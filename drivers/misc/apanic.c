/* drivers/misc/apanic.c
 *
 * Copyright (C) 2009 Google, Inc.
 * Author: San Mehat <san@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/mtd/mtd.h>
#include <linux/notifier.h>
#include <linux/mtd/mtd.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/preempt.h>

/* Add meminfo head files */
#ifdef CONFIG_HUAWEI_APANIC_EXTEND
#include <linux/hugetlb.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/mmzone.h>
#include <linux/quicklist.h>
#include <linux/seq_file.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <asm/atomic.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include "../../fs/proc/internal.h"

/* Add /proc/stat info head files,xiemingliang,2011.05.30 */
#include <linux/cpumask.h>
#include <linux/gfp.h>
#include <linux/kernel_stat.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/irqnr.h>
#include <asm/cputime.h>

#include <linux/err.h>
#include <linux/cpu.h>

/*vmalloc info head files,xiemingliang,2011.05.30*/
#include <linux/vmalloc.h>
#include <linux/kallsyms.h>
#endif

/* 2 macros are here to define a block number region
 * that can be marked as bad block, to make sure that 
 * the data in the block region can be protected from 
 * touching by the apanic mechanism.
 * But if you have a seperate partition to store the apanic
 * info, thse macros and the added functions below are
 * useless and should be deleted.
 */
#ifdef CONFIG_HUAWEI_APANIC
#define HUAWEI_PROTECT_BLOCK_NUMBER_BEGIN 0
#define HUAWEI_PROTECT_BLOCK_NUMBER_END 15
#endif	
extern void ram_console_enable_console(int);

struct panic_header {
	u32 magic;
#define PANIC_MAGIC 0xdeadf00d

	u32 version;
#define PHDR_VERSION   0x01

	u32 console_offset;
	u32 console_length;

	u32 threads_offset;
	u32 threads_length;

#ifdef CONFIG_HUAWEI_APANIC_EXTEND
	u32 sysinfo_offset;
	u32 sysinfo_length;
#endif
};

struct apanic_data {
	struct mtd_info		*mtd;
	struct panic_header	curr;
	void			*bounce;
	struct proc_dir_entry	*apanic_console;
	struct proc_dir_entry	*apanic_threads;
#ifdef CONFIG_HUAWEI_APANIC_EXTEND
	struct proc_dir_entry	*apanic_sysinfo;
#endif
	
#ifdef CONFIG_HUAWEI_KERNEL
	struct proc_dir_entry   *modem_panic;	
	char *modem_panic_ptr;
	int modem_panic_len;
#endif	
};

static struct apanic_data drv_ctx;
static struct work_struct proc_removal_work;
static DEFINE_MUTEX(drv_mutex);

static unsigned int *apanic_bbt;
static unsigned int apanic_erase_blocks;
static unsigned int apanic_good_blocks;

#ifdef CONFIG_HUAWEI_APANIC
/*
 * Function block_isprotected
 *
 * Description: 
 *   to return a value shows that if a block should be protected 
 *   from touching by the apainc, if a value 1 is returned, the 
 *   block is marked as a bad block, and the data in these area can 
 *   be protected
 *
 * Return values:
 *    1: this block should be protected 
 *    0: this block can be used to store the apanic info
 *
 */
static int block_isprotected(int block_number)
{
    if( (block_number >= HUAWEI_PROTECT_BLOCK_NUMBER_BEGIN)
        && (block_number <= HUAWEI_PROTECT_BLOCK_NUMBER_END) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif	
static void set_bb(unsigned int block, unsigned int *bbt)
{
	unsigned int flag = 1;

	BUG_ON(block >= apanic_erase_blocks);

	flag = flag << (block%32);
	apanic_bbt[block/32] |= flag;
	apanic_good_blocks--;
}

static unsigned int get_bb(unsigned int block, unsigned int *bbt)
{
	unsigned int flag;

	BUG_ON(block >= apanic_erase_blocks);

	flag = 1 << (block%32);
	return apanic_bbt[block/32] & flag;
}

static void alloc_bbt(struct mtd_info *mtd, unsigned int *bbt)
{
	int bbt_size;
	apanic_erase_blocks = (mtd->size)>>(mtd->erasesize_shift);
	bbt_size = (apanic_erase_blocks+32)/32;

	apanic_bbt = kmalloc(bbt_size*4, GFP_KERNEL);
	memset(apanic_bbt, 0, bbt_size*4);
	apanic_good_blocks = apanic_erase_blocks;
}
static void scan_bbt(struct mtd_info *mtd, unsigned int *bbt)
{
	int i;

	for (i = 0; i < apanic_erase_blocks; i++) {
    #ifndef CONFIG_HUAWEI_APANIC
		if (mtd->block_isbad(mtd, i*mtd->erasesize))
			set_bb(i, apanic_bbt);
    #else
		if (block_isprotected(i))
        {
			set_bb(i, apanic_bbt);
        }
    #endif	
	}
}

#define APANIC_INVALID_OFFSET 0xFFFFFFFF

static unsigned int phy_offset(struct mtd_info *mtd, unsigned int offset)
{
	unsigned int logic_block = offset>>(mtd->erasesize_shift);
	unsigned int phy_block;
	unsigned good_block = 0;

	for (phy_block = 0; phy_block < apanic_erase_blocks; phy_block++) {
		if (!get_bb(phy_block, apanic_bbt))
			good_block++;
		if (good_block == (logic_block + 1))
			break;
	}

	if (good_block != (logic_block + 1))
		return APANIC_INVALID_OFFSET;

	return offset + ((phy_block-logic_block)<<mtd->erasesize_shift);
}

static void apanic_erase_callback(struct erase_info *done)
{
	wait_queue_head_t *wait_q = (wait_queue_head_t *) done->priv;
	wake_up(wait_q);
}

static int apanic_proc_read(char *buffer, char **start, off_t offset,
			       int count, int *peof, void *dat)
{
	struct apanic_data *ctx = &drv_ctx;
	size_t file_length;
	off_t file_offset;
	unsigned int page_no;
	off_t page_offset;
	int rc;
	size_t len;

	if (!count)
		return 0;

	mutex_lock(&drv_mutex);

	switch ((int) dat) {
	case 1:	/* apanic_console */
		file_length = ctx->curr.console_length;
		file_offset = ctx->curr.console_offset;
		break;
	case 2:	/* apanic_threads */
		file_length = ctx->curr.threads_length;
		file_offset = ctx->curr.threads_offset;
		break;        
#ifdef CONFIG_HUAWEI_APANIC_EXTEND
	case 3:	/* apanic_sysinfo */ 
		file_length = ctx->curr.sysinfo_length;
		file_offset = ctx->curr.sysinfo_offset;
		break;
#endif
	default:
		pr_err("Bad dat (%d)\n", (int) dat);
		mutex_unlock(&drv_mutex);
		return -EINVAL;
	}

	if ((offset + count) > file_length) {
		mutex_unlock(&drv_mutex);
		return 0;
	}

	/* We only support reading a maximum of a flash page */
	if (count > ctx->mtd->writesize)
		count = ctx->mtd->writesize;

	page_no = (file_offset + offset) / ctx->mtd->writesize;
	page_offset = (file_offset + offset) % ctx->mtd->writesize;


	if (phy_offset(ctx->mtd, (page_no * ctx->mtd->writesize))
		== APANIC_INVALID_OFFSET) {
		pr_err("apanic: reading an invalid address\n");
		mutex_unlock(&drv_mutex);
		return -EINVAL;
	}
	rc = ctx->mtd->read(ctx->mtd,
		phy_offset(ctx->mtd, (page_no * ctx->mtd->writesize)),
		ctx->mtd->writesize,
		&len, ctx->bounce);

	if (page_offset)
		count -= page_offset;
	memcpy(buffer, ctx->bounce + page_offset, count);

	*start = (char*)count;
	
	if ((offset + count) == file_length)
		*peof = 1;

	mutex_unlock(&drv_mutex);
	return count;
}

static void mtd_panic_erase(void)
{
	struct apanic_data *ctx = &drv_ctx;
	struct erase_info erase;
	DECLARE_WAITQUEUE(wait, current);
	wait_queue_head_t wait_q;
	int rc, i;

	init_waitqueue_head(&wait_q);
	erase.mtd = ctx->mtd;
	erase.callback = apanic_erase_callback;
	erase.len = ctx->mtd->erasesize;
	erase.priv = (u_long)&wait_q;
	for (i = 0; i < ctx->mtd->size; i += ctx->mtd->erasesize) {
		erase.addr = i;
		set_current_state(TASK_INTERRUPTIBLE);
		add_wait_queue(&wait_q, &wait);

		if (get_bb(erase.addr>>ctx->mtd->erasesize_shift, apanic_bbt)) {
			printk(KERN_WARNING
			       "apanic: Skipping erase of bad "
			       "block @%llx\n", erase.addr);
			set_current_state(TASK_RUNNING);
			remove_wait_queue(&wait_q, &wait);
			continue;
		}

		rc = ctx->mtd->erase(ctx->mtd, &erase);
		if (rc) {
			set_current_state(TASK_RUNNING);
			remove_wait_queue(&wait_q, &wait);
			printk(KERN_ERR
			       "apanic: Erase of 0x%llx, 0x%llx failed\n",
			       (unsigned long long) erase.addr,
			       (unsigned long long) erase.len);
			if (rc == -EIO) {
				if (ctx->mtd->block_markbad(ctx->mtd,
							    erase.addr)) {
					printk(KERN_ERR
					       "apanic: Err marking blk bad\n");
					goto out;
				}
				printk(KERN_INFO
				       "apanic: Marked a bad block"
				       " @%llx\n", erase.addr);
				set_bb(erase.addr>>ctx->mtd->erasesize_shift,
					apanic_bbt);
				continue;
			}
			goto out;
		}
		schedule();
		remove_wait_queue(&wait_q, &wait);
	}

    /* write the erased mtd virtual flash to the mmc panic partition */
#ifdef CONFIG_HUAWEI_KERNEL
    if (ctx->mtd->sync)
    {
        ctx->mtd->sync(ctx->mtd);
    }
#endif
	
	printk(KERN_DEBUG "apanic: %s partition erased\n",
	       CONFIG_APANIC_PLABEL);
out:
	return;
}

static void apanic_remove_proc_work(struct work_struct *work)
{
	struct apanic_data *ctx = &drv_ctx;

	mutex_lock(&drv_mutex);
	mtd_panic_erase();
	memset(&ctx->curr, 0, sizeof(struct panic_header));
	if (ctx->apanic_console) {
		remove_proc_entry("apanic_console", NULL);
		ctx->apanic_console = NULL;
	}
	if (ctx->apanic_threads) {
		remove_proc_entry("apanic_threads", NULL);
		ctx->apanic_threads = NULL;
	}

#ifdef CONFIG_HUAWEI_APANIC_EXTEND
	if (ctx->apanic_sysinfo) {
		remove_proc_entry("apanic_sysinfo", NULL);
		ctx->apanic_sysinfo = NULL;
	}
#endif
	
	mutex_unlock(&drv_mutex);
}

static int apanic_proc_write(struct file *file, const char __user *buffer,
				unsigned long count, void *data)
{
	schedule_work(&proc_removal_work);
	return count;
}

#ifdef CONFIG_HUAWEI_KERNEL
static ssize_t modem_proc_read(struct file *file, char __user *buf,
			size_t len, loff_t *offset)
{
	struct apanic_data *ctx = &drv_ctx;
	loff_t pos = *offset;
	ssize_t count;

	if (pos >= ctx->modem_panic_len)
		return 0;

	count = min(len, (size_t)(ctx->modem_panic_len - pos));
	if (copy_to_user(buf, ctx->modem_panic_ptr + pos, count)) {
		pr_err("%s: copy to user failed\n", __func__);
		return -EFAULT;
	}

	*offset += count;
	return count;
}

static struct file_operations modem_crash_log_fops = {
	.read = modem_proc_read
};
extern int detect_modem_crash_log(void **ppcrash_log,int *plog_len);
#endif
	
static void mtd_panic_notify_add(struct mtd_info *mtd)
{
	struct apanic_data *ctx = &drv_ctx;
	struct panic_header *hdr = ctx->bounce;
	size_t len;
	int rc;
	int    proc_entry_created = 0;

	if (strcmp(mtd->name, CONFIG_APANIC_PLABEL))
		return;

#ifdef CONFIG_HUAWEI_KERNEL
	if(detect_modem_crash_log((void **)&ctx->modem_panic_ptr,&ctx->modem_panic_len)) {
		ctx->modem_panic= create_proc_entry("modem_panic",
						      S_IFREG | S_IRUGO, NULL);
		if (!ctx->modem_panic)
			printk(KERN_ERR "%s: failed creating modem procfile\n",__func__);
		else {
			ctx->modem_panic->proc_fops = &modem_crash_log_fops;
			ctx->modem_panic->size = ctx->modem_panic_len;
		}	
	}
#endif	

	ctx->mtd = mtd;

	alloc_bbt(mtd, apanic_bbt);
	scan_bbt(mtd, apanic_bbt);

	if (apanic_good_blocks == 0) {
		printk(KERN_ERR "apanic: no any good blocks?!\n");
		goto out_err;
	}

	rc = mtd->read(mtd, phy_offset(mtd, 0), mtd->writesize,
			&len, ctx->bounce);
	if (rc && rc == -EBADMSG) {
		printk(KERN_WARNING
		       "apanic: Bad ECC on block 0 (ignored)\n");
	} else if (rc && rc != -EUCLEAN) {
		printk(KERN_ERR "apanic: Error reading block 0 (%d)\n", rc);
		goto out_err;
	}

	if (len != mtd->writesize) {
		printk(KERN_ERR "apanic: Bad read size (%d)\n", rc);
		goto out_err;
	}

	printk(KERN_INFO "apanic: Bound to mtd partition '%s'\n", mtd->name);

	if (hdr->magic != PANIC_MAGIC) {
		printk(KERN_INFO "apanic: No panic data available\n");
		mtd_panic_erase();
		return;
	}

	if (hdr->version != PHDR_VERSION) {
		printk(KERN_INFO "apanic: Version mismatch (%d != %d)\n",
		       hdr->version, PHDR_VERSION);
		mtd_panic_erase();
		return;
	}

	memcpy(&ctx->curr, hdr, sizeof(struct panic_header));

	printk(KERN_INFO "apanic: c(%u, %u) t(%u, %u)\n",
	       hdr->console_offset, hdr->console_length,
	       hdr->threads_offset, hdr->threads_length);

	if (hdr->console_length) {
		ctx->apanic_console = create_proc_entry("apanic_console",
						      S_IFREG | S_IRUGO, NULL);
		if (!ctx->apanic_console)
			printk(KERN_ERR "%s: failed creating procfile\n",
			       __func__);
		else {
			ctx->apanic_console->read_proc = apanic_proc_read;
			ctx->apanic_console->write_proc = apanic_proc_write;
			ctx->apanic_console->size = hdr->console_length;
			ctx->apanic_console->data = (void *) 1;
			proc_entry_created = 1;
		}
	}

	if (hdr->threads_length) {
		ctx->apanic_threads = create_proc_entry("apanic_threads",
						       S_IFREG | S_IRUGO, NULL);
		if (!ctx->apanic_threads)
			printk(KERN_ERR "%s: failed creating procfile\n",
			       __func__);
		else {
			ctx->apanic_threads->read_proc = apanic_proc_read;
			ctx->apanic_threads->write_proc = apanic_proc_write;
			ctx->apanic_threads->size = hdr->threads_length;
			ctx->apanic_threads->data = (void *) 2;
			proc_entry_created = 1;
		}
	}
    
#ifdef CONFIG_HUAWEI_APANIC_EXTEND
	if (hdr->sysinfo_length) {
		ctx->apanic_sysinfo = create_proc_entry("apanic_sysinfo",
						       S_IFREG | S_IRUGO, NULL);
		if (!ctx->apanic_sysinfo)
			printk(KERN_ERR "%s: failed creating apanic_sysinfo procfile\n",
			       __func__);
		else {
			ctx->apanic_sysinfo->read_proc = apanic_proc_read;
			ctx->apanic_sysinfo->write_proc = apanic_proc_write;
			ctx->apanic_sysinfo->size = hdr->sysinfo_length;
			/* "1" is console, "2" is threads, "3" is sysinfo */
			ctx->apanic_sysinfo->data = (void *) 3;
			proc_entry_created = 1;
		}
	}
#endif

	if (!proc_entry_created)
		mtd_panic_erase();

	return;
out_err:
	ctx->mtd = NULL;
}

static void mtd_panic_notify_remove(struct mtd_info *mtd)
{
	struct apanic_data *ctx = &drv_ctx;
	if (mtd == ctx->mtd) {
		ctx->mtd = NULL;
		printk(KERN_INFO "apanic: Unbound from %s\n", mtd->name);
	}
}

static struct mtd_notifier mtd_panic_notifier = {
	.add	= mtd_panic_notify_add,
	.remove	= mtd_panic_notify_remove,
};

static int in_panic = 0;

static int apanic_writeflashpage(struct mtd_info *mtd, loff_t to,
				 const u_char *buf)
{
	int rc;
	size_t wlen;
	int panic = in_interrupt() | in_atomic();

	if (panic && !mtd->panic_write) {
		printk(KERN_EMERG "%s: No panic_write available\n", __func__);
		return 0;
	} else if (!panic && !mtd->write) {
		printk(KERN_EMERG "%s: No write available\n", __func__);
		return 0;
	}

	to = phy_offset(mtd, to);
	if (to == APANIC_INVALID_OFFSET) {
		printk(KERN_EMERG "apanic: write to invalid address\n");
		return 0;
	}

	if (panic)
		rc = mtd->panic_write(mtd, to, mtd->writesize, &wlen, buf);
	else
		rc = mtd->write(mtd, to, mtd->writesize, &wlen, buf);

	if (rc) {
		printk(KERN_EMERG
		       "%s: Error writing data to flash (%d)\n",
		       __func__, rc);
		return rc;
	}

	return wlen;
}

extern int log_buf_copy(char *dest, int idx, int len);
extern void log_buf_clear(void);

/*
 * Writes the contents of the console to the specified offset in flash.
 * Returns number of bytes written
 */
static int apanic_write_console(struct mtd_info *mtd, unsigned int off)
{
	struct apanic_data *ctx = &drv_ctx;
	int saved_oip;
	int idx = 0;
	int rc, rc2;
	unsigned int last_chunk = 0;

	while (!last_chunk) {
		saved_oip = oops_in_progress;
		oops_in_progress = 1;
		rc = log_buf_copy(ctx->bounce, idx, mtd->writesize);
		if (rc < 0)
			break;

		if (rc != mtd->writesize)
			last_chunk = rc;

		oops_in_progress = saved_oip;
		if (rc <= 0)
			break;
		if (rc != mtd->writesize)
			memset(ctx->bounce + rc, 0, mtd->writesize - rc);

		rc2 = apanic_writeflashpage(mtd, off, ctx->bounce);
		if (rc2 <= 0) {
			printk(KERN_EMERG
			       "apanic: Flash write failed (%d)\n", rc2);
			return idx;
		}
		if (!last_chunk)
			idx += rc2;
		else
			idx += last_chunk;
		off += rc2;
	}
	return idx;
}

#ifdef CONFIG_HUAWEI_APANIC_EXTEND

#define DATA_BUF_LEN (64*1024)/* 64KB */

/* Use for sava sysinfo data ,xiemingliang,20110804*/
static char data_buf[DATA_BUF_LEN];

/* Index into log_buf: most-recently-written-char + 1 ,xiemingliang,20110804*/
static int data_end = 0;

/*
 * Copy a range of characters from the sysinfo data buffer.
 * xiemingliang,20110804
 */
static int buf_copy(char *dest, int idx, int len)
{
	int ret = 0;
	int max = data_end;

	if (idx < 0 || idx >= max) {
		ret = -1;
	} else {
		if (len > max - idx){
			len = max - idx;
		}
		ret = len;
		while (len-- > 0){
			dest[len] = data_buf[idx + len];
		}
	}

	return ret;
}


/*
 * Writes the contents of the sysinfo to the specified offset in flash.
 * Returns number of bytes written
 * xiemingliang,20110804
 */
static int apanic_write_sysinfo(struct mtd_info *mtd, unsigned int off)
{
	struct apanic_data *ctx = &drv_ctx;
	int saved_oip;
	int idx = 0;
	int rc = 0;
	int rc2 = 0;
	unsigned int last_chunk = 0;

	while (!last_chunk) {
		saved_oip = oops_in_progress;
		oops_in_progress = 1;
		rc = buf_copy(ctx->bounce, idx, mtd->writesize);
		if (rc < 0)
			break;

		if (rc != mtd->writesize)
			last_chunk = rc;

		oops_in_progress = saved_oip;
		if (rc <= 0)
			break;
		if (rc != mtd->writesize)
			memset(ctx->bounce + rc, 0, mtd->writesize - rc);

		rc2 = apanic_writeflashpage(mtd, off, ctx->bounce);
		if (rc2 <= 0) {
			printk(KERN_EMERG
			       "apanic: Flash write failed (%d)\n", rc2);
			return idx;
		}
		if (!last_chunk)
			idx += rc2;
		else
			idx += last_chunk;
		off += rc2;
	}
	return idx;
}


/*
 * Get /proc/meminfo
 * Write by xiemingliang,2011.05.27
 * Modified by xiemingliang,20110810
 */
static int apanic_meminfo_proc_get(void)
{
	struct sysinfo i;
	unsigned long committed;
	unsigned long allowed;
	struct vmalloc_info vmi;
	long cached;
	unsigned long pages[NR_LRU_LISTS];
	int lru;

/*
 * display in kilobytes.
 */
#define K(x) ((x) << (PAGE_SHIFT - 10))
	si_meminfo(&i);
	si_swapinfo(&i);
	committed = percpu_counter_read_positive(&vm_committed_as);
	allowed = ((totalram_pages - hugetlb_total_pages())
		* sysctl_overcommit_ratio / 100) + total_swap_pages;

	cached = global_page_state(NR_FILE_PAGES) -
			total_swapcache_pages - i.bufferram;
	if (cached < 0)
		cached = 0;

	get_vmalloc_info(&vmi);

	for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++){
		pages[lru] = global_page_state(NR_LRU_BASE + lru);
    }
	/*
	 * Tagged format, for easy grepping and expansion.
	 */
    data_end += sprintf(&data_buf[data_end],"%s","\n---------- MEMINFO -----------\n");
    data_end += sprintf(&data_buf[data_end],
		"MemTotal:       %8lu kB\n"
		"MemFree:        %8lu kB\n"
		"Buffers:        %8lu kB\n"
		"Cached:         %8lu kB\n"
		"SwapCached:     %8lu kB\n"
		"Active:         %8lu kB\n"
		"Inactive:       %8lu kB\n"
		"Active(anon):   %8lu kB\n"
		"Inactive(anon): %8lu kB\n"
		"Active(file):   %8lu kB\n"
		"Inactive(file): %8lu kB\n"
		"Unevictable:    %8lu kB\n"
		"Mlocked:        %8lu kB\n"
#ifdef CONFIG_HIGHMEM
		"HighTotal:      %8lu kB\n"
		"HighFree:       %8lu kB\n"
		"LowTotal:       %8lu kB\n"
		"LowFree:        %8lu kB\n"
#endif
#ifndef CONFIG_MMU
		"MmapCopy:       %8lu kB\n"
#endif
		"SwapTotal:      %8lu kB\n"
		"SwapFree:       %8lu kB\n"
		"Dirty:          %8lu kB\n"
		"Writeback:      %8lu kB\n"
		"AnonPages:      %8lu kB\n"
		"Mapped:         %8lu kB\n"
		"Shmem:          %8lu kB\n"
		"Slab:           %8lu kB\n"
		"SReclaimable:   %8lu kB\n"
		"SUnreclaim:     %8lu kB\n"
		"KernelStack:    %8lu kB\n"
		"PageTables:     %8lu kB\n"
#ifdef CONFIG_QUICKLIST
		"Quicklists:     %8lu kB\n"
#endif
		"NFS_Unstable:   %8lu kB\n"
		"Bounce:         %8lu kB\n"
		"WritebackTmp:   %8lu kB\n"
		"CommitLimit:    %8lu kB\n"
		"Committed_AS:   %8lu kB\n"
		"VmallocTotal:   %8lu kB\n"
		"VmallocUsed:    %8lu kB\n"
		"VmallocChunk:   %8lu kB\n"
#ifdef CONFIG_MEMORY_FAILURE
		"HardwareCorrupted: %5lu kB\n"
#endif
		,
		K(i.totalram),
		K(i.freeram),
		K(i.bufferram),
		K(cached),
		K(total_swapcache_pages),
		K(pages[LRU_ACTIVE_ANON]   + pages[LRU_ACTIVE_FILE]),
		K(pages[LRU_INACTIVE_ANON] + pages[LRU_INACTIVE_FILE]),
		K(pages[LRU_ACTIVE_ANON]),
		K(pages[LRU_INACTIVE_ANON]),
		K(pages[LRU_ACTIVE_FILE]),
		K(pages[LRU_INACTIVE_FILE]),
		K(pages[LRU_UNEVICTABLE]),
		K(global_page_state(NR_MLOCK)),
#ifdef CONFIG_HIGHMEM
		K(i.totalhigh),
		K(i.freehigh),
		K(i.totalram-i.totalhigh),
		K(i.freeram-i.freehigh),
#endif
#ifndef CONFIG_MMU
		K((unsigned long) atomic_long_read(&mmap_pages_allocated)),
#endif
		K(i.totalswap),
		K(i.freeswap),
		K(global_page_state(NR_FILE_DIRTY)),
		K(global_page_state(NR_WRITEBACK)),
		K(global_page_state(NR_ANON_PAGES)),
		K(global_page_state(NR_FILE_MAPPED)),
		K(global_page_state(NR_SHMEM)),
		K(global_page_state(NR_SLAB_RECLAIMABLE) +
				global_page_state(NR_SLAB_UNRECLAIMABLE)),
		K(global_page_state(NR_SLAB_RECLAIMABLE)),
		K(global_page_state(NR_SLAB_UNRECLAIMABLE)),
		global_page_state(NR_KERNEL_STACK) * THREAD_SIZE / 1024,
		K(global_page_state(NR_PAGETABLE)),
#ifdef CONFIG_QUICKLIST
		K(quicklist_total_size()),
#endif
		K(global_page_state(NR_UNSTABLE_NFS)),
		K(global_page_state(NR_BOUNCE)),
		K(global_page_state(NR_WRITEBACK_TEMP)),
		K(allowed),
		K(committed),
		(unsigned long)VMALLOC_TOTAL >> 10,
		vmi.used >> 10,
		vmi.largest_chunk >> 10
#ifdef CONFIG_MEMORY_FAILURE
		,atomic_long_read(&mce_bad_pages) << (PAGE_SHIFT - 10)
#endif
		);

	return 0;
#undef K
}

/* ******************************************
 * Show proc/stat info when system is panic.
 * Add by xiemingliang,2011.05.30
 */
#ifndef arch_irq_stat_cpu
#define arch_irq_stat_cpu(cpu) 0
#endif
#ifndef arch_irq_stat
#define arch_irq_stat() 0
#endif
#ifndef arch_idle_time
#define arch_idle_time(cpu) 0
#endif

/*
 * Modified by xiemingliang,20110810
 */
static int apanic_stat_proc_get(void)
{
	int i, j;
	unsigned long jif;
	cputime64_t user, nice, system, idle, iowait, irq, softirq, steal;
	cputime64_t guest;
	u64 sum = 0;
	u64 sum_softirq = 0;
	unsigned int per_softirq_sums[NR_SOFTIRQS] = {0};
	struct timespec boottime;
	unsigned int per_irq_sum;

	user = nice = system = idle = iowait =
		irq = softirq = steal = cputime64_zero;
	guest = cputime64_zero;
	getboottime(&boottime);
	jif = boottime.tv_sec;

	for_each_possible_cpu(i) {
		user = cputime64_add(user, kstat_cpu(i).cpustat.user);
		nice = cputime64_add(nice, kstat_cpu(i).cpustat.nice);
		system = cputime64_add(system, kstat_cpu(i).cpustat.system);
		idle = cputime64_add(idle, kstat_cpu(i).cpustat.idle);
		idle = cputime64_add(idle, arch_idle_time(i));
		iowait = cputime64_add(iowait, kstat_cpu(i).cpustat.iowait);
		irq = cputime64_add(irq, kstat_cpu(i).cpustat.irq);
		softirq = cputime64_add(softirq, kstat_cpu(i).cpustat.softirq);
		steal = cputime64_add(steal, kstat_cpu(i).cpustat.steal);
		guest = cputime64_add(guest, kstat_cpu(i).cpustat.guest);
		for_each_irq_nr(j) {
			sum += kstat_irqs_cpu(j, i);
		}
		sum += arch_irq_stat_cpu(i);

		for (j = 0; j < NR_SOFTIRQS; j++) {
			unsigned int softirq_stat = kstat_softirqs_cpu(j, i);

			per_softirq_sums[j] += softirq_stat;
			sum_softirq += softirq_stat;
		}
	}
	sum += arch_irq_stat();
    data_end += sprintf(&data_buf[data_end],"%s","\n------ STAT INFO ------\n");
    data_end += sprintf(&data_buf[data_end],
        "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu\n",
		(unsigned long long)cputime64_to_clock_t(user),
		(unsigned long long)cputime64_to_clock_t(nice),
		(unsigned long long)cputime64_to_clock_t(system),
		(unsigned long long)cputime64_to_clock_t(idle),
		(unsigned long long)cputime64_to_clock_t(iowait),
		(unsigned long long)cputime64_to_clock_t(irq),
		(unsigned long long)cputime64_to_clock_t(softirq),
		(unsigned long long)cputime64_to_clock_t(steal),
		(unsigned long long)cputime64_to_clock_t(guest));
	for_each_online_cpu(i) {

		/* Copy values here to work around gcc-2.95.3, gcc-2.96 */
		user = kstat_cpu(i).cpustat.user;
		nice = kstat_cpu(i).cpustat.nice;
		system = kstat_cpu(i).cpustat.system;
		idle = kstat_cpu(i).cpustat.idle;
		idle = cputime64_add(idle, arch_idle_time(i));
		iowait = kstat_cpu(i).cpustat.iowait;
		irq = kstat_cpu(i).cpustat.irq;
		softirq = kstat_cpu(i).cpustat.softirq;
		steal = kstat_cpu(i).cpustat.steal;
		guest = kstat_cpu(i).cpustat.guest;
		data_end += sprintf(&data_buf[data_end],
			"cpu%d %llu %llu %llu %llu %llu %llu %llu %llu %llu\n",
			i,
			(unsigned long long)cputime64_to_clock_t(user),
			(unsigned long long)cputime64_to_clock_t(nice),
			(unsigned long long)cputime64_to_clock_t(system),
			(unsigned long long)cputime64_to_clock_t(idle),
			(unsigned long long)cputime64_to_clock_t(iowait),
			(unsigned long long)cputime64_to_clock_t(irq),
			(unsigned long long)cputime64_to_clock_t(softirq),
			(unsigned long long)cputime64_to_clock_t(steal),
			(unsigned long long)cputime64_to_clock_t(guest));
	}
    data_end += sprintf(&data_buf[data_end],"intr %llu\n", (unsigned long long)sum);

	/* sum again ? it could be updated? */
	for_each_irq_nr(j) {
		per_irq_sum = 0;
		for_each_possible_cpu(i){
			per_irq_sum += kstat_irqs_cpu(j, i);
        }
		data_end += sprintf(&data_buf[data_end]," %8u", per_irq_sum);
		if((j%8)==7)
			data_end += sprintf(&data_buf[data_end],"%s","\n");
	}

	data_end += sprintf(&data_buf[data_end],
		"\nctxt %llu\n"
		"btime %lu\n"
		"processes %lu\n"
		"procs_running %lu\n"
		"procs_blocked %lu\n",
		nr_context_switches(),
		(unsigned long)jif,
		total_forks,
		nr_running(),
		nr_iowait());

	data_end += sprintf(&data_buf[data_end],"softirq %llu", (unsigned long long)sum_softirq);

	for (i = 0; i < NR_SOFTIRQS; i++){
		data_end += sprintf(&data_buf[data_end]," %u", per_softirq_sums[i]);
	}
    data_end += sprintf(&data_buf[data_end],"%s","\n");

	return 0;
}
/* ****************************************** */

/*===============vmalloc info==================*/
/* 
 * Get /proc/zoneinfo
 * Write by xiemingliang,20110530
 */
static int apanic_s_show(void *p)
{
	struct vm_struct *v = p;
    
    unsigned long addr_s ;
    unsigned long addr_e ;
    addr_s = (unsigned long)v->addr;
    addr_e = addr_s + v->size;

	data_end += sprintf(&data_buf[data_end],"0x%08lx-0x%08lx %8ld",addr_s, addr_e, v->size);
    
	if (v->caller) {
		char buff[KSYM_SYMBOL_LEN];

        data_end += sprintf(&data_buf[data_end],"%s"," ");
		sprint_symbol(buff, (unsigned long)v->caller);
        data_end += sprintf(&data_buf[data_end],"%s",buff);
	}

	if (v->nr_pages)
		data_end += sprintf(&data_buf[data_end]," pages=%d", v->nr_pages);

    if (v->phys_addr)
		data_end += sprintf(&data_buf[data_end]," phys=%lx", (unsigned long)v->phys_addr);

	if (v->flags & VM_IOREMAP)
		data_end += sprintf(&data_buf[data_end],"%s"," ioremap");

	if (v->flags & VM_ALLOC)
		data_end += sprintf(&data_buf[data_end],"%s"," vmalloc");

	if (v->flags & VM_MAP)
		data_end += sprintf(&data_buf[data_end],"%s"," vmap");

	if (v->flags & VM_USERMAP)
		data_end += sprintf(&data_buf[data_end],"%s"," user");

	if (v->flags & VM_VPAGES)
		data_end += sprintf(&data_buf[data_end],"%s"," vpages");

    if (NUMA_BUILD) {
		unsigned int nr;
        unsigned int counters[nr_node_ids] = {0};
		for (nr = 0; nr < v->nr_pages; nr++){
			counters[page_to_nid(v->pages[nr])]++;
		}
		for_each_node_state(nr, N_HIGH_MEMORY){
			if (counters[nr])
				data_end += sprintf(&data_buf[data_end]," N%u=%u", nr, counters[nr]);
		}
	}
	data_end += sprintf(&data_buf[data_end],"%s","\n");
	return 0;
}
extern struct vm_struct *vmlist;
static int apanic_vmalloc_proc_get(void)
{
    struct vm_struct *v;

	read_lock(&vmlist_lock);
	v = vmlist;
    data_end += sprintf(&data_buf[data_end],"%s","\n---------- VMALLOC INFO -----------\n");
    do{
        apanic_s_show(v);
        v = v->next;
    }while(v);
    read_unlock(&vmlist_lock);
    return 0;
}


/* 
 * Get /proc/zoneinfo xiemingliang,2011.05.31
 * Modified by xiemingliang,20110804
 */
static int apanic_zoneinfo_proc_get(void)
{
    struct   file   *file   =   NULL; 
    loff_t pos = 0;
    int cnt = 1;
    char *buff = NULL;
    mm_segment_t   old_fs; 
    int buflen = 4096;/* 4KB */
    char *ptr = NULL;
    int index = 0;

    buff = (char *)kmalloc(buflen, GFP_KERNEL);
    if(buff==NULL) {
        printk("alllocate mem for file fail?\n");
        return -1;
    }
    memset(buff,0x00,buflen);

    old_fs = get_fs();   
    set_fs(KERNEL_DS);
    
	file = filp_open("/proc/zoneinfo", O_RDONLY, 0); 
    if(IS_ERR(file))
    {
        kfree(buff);
        printk(KERN_INFO "Open /proc/zoneinfo failure.\n");
        data_end += sprintf(&data_buf[data_end],"%s","Open /proc/zoneinfo failure.\n");
        return PTR_ERR(file);
    }

    data_end += sprintf(&data_buf[data_end],"%s","\n---------- ZONEINFO -----------\n");

    while(1){
        cnt = file->f_op->read(file, buff, buflen, &pos);
        /*No data read or  data buffer is full,break */
        if( (cnt<=0) || ( (data_end + cnt) > DATA_BUF_LEN )){
            break;
        }
        ptr = buff;
        index = cnt;
        while(index-->0)
        {   /* Modified,20110804
            printk("%c",*ptr);
            ptr++;
            */
            data_buf[data_end] = *ptr;
            ptr++;
			data_end++;
        }
    }
    
    set_fs(old_fs); 
    filp_close(file,   NULL);
    kfree(buff);
    return 0;
}

/* 
 * Get /proc/vmstat xiemingliang,2011.05.31
 * Modified by xiemingliang,20110804
 */
static int apanic_vmstat_proc_get(void)
{
    struct   file   *file   =   NULL; 
    loff_t pos = 0;
    int cnt = 1;
    char *buff = NULL;
    mm_segment_t   old_fs; 
    int buflen = 4096;/* 4KB */
    char *ptr = NULL;
    int index = 0;

    buff = (char *)kmalloc(buflen, GFP_KERNEL);
    if(buff==NULL) {
        printk("alllocate mem for file fail?\n");
        return -1;
    }
    memset(buff,0x00,buflen);

    old_fs = get_fs();   
    set_fs(KERNEL_DS);
    
	file = filp_open("/proc/vmstat", O_RDONLY, 0); 
    if(IS_ERR(file))
    {
        kfree(buff);
        printk(KERN_INFO "Open /proc/vmstat failure.\n");
        data_end += sprintf(&data_buf[data_end],"%s","Open /proc/vmstat failure.\n");
        return PTR_ERR(file);
    }

    data_end += sprintf(&data_buf[data_end],"%s","\n---------- VIRTUAL MEMORY STATS -----------\n");

    while(1){
        cnt = file->f_op->read(file, buff, buflen, &pos);
        /*No data read or  data buffer is full,break */
        if( (cnt<=0) || ( (data_end + cnt) > DATA_BUF_LEN )){
            break;
        }
        ptr = buff;
        index = cnt;
        while(index-->0)
        {   /* 2011.05.31
            printk("%c",*ptr);
            ptr++;
            */
            /* 20110804 */
            data_buf[data_end] = *ptr;
            ptr++;
			data_end++;
        }
    }
    
    set_fs(old_fs); 
    filp_close(file,   NULL);
    kfree(buff);
    return 0;
}


/* 
 * Get /proc/slabinfo
 * Write by xiemingliang,20110804
 */
static int apanic_slabinfo_proc_get(void)
{
    struct   file   *file   =   NULL; 
    loff_t pos = 0;
    int cnt = 1;
    char *buff = NULL;
    mm_segment_t   old_fs; 
    int buflen = 4096;/* 4KB */
    char *ptr = NULL;
    int index = 0;

    buff = (char *)kmalloc(buflen, GFP_KERNEL);
    if(buff==NULL) {
        printk("alllocate mem for file fail?\n");
        return -1;
    }
    memset(buff,0x00,buflen);

    old_fs = get_fs();   
    set_fs(KERNEL_DS);
    
	file = filp_open("/proc/slabinfo", O_RDONLY, 0); 
    if(IS_ERR(file))
    {
        kfree(buff);
        printk(KERN_INFO "Open /proc/slabinfo failure.\n");
        data_end += sprintf(&data_buf[data_end],"%s","Open /proc/slabinfo failure.\n");
        return PTR_ERR(file);
    }
    /*2011.05.31
    printk("\n---------- SLAB INFO -----------\n");
    */
    data_end += sprintf(&data_buf[data_end],"%s","\n---------- SLAB INFO -----------\n");

    while(1){
        cnt = file->f_op->read(file, buff, buflen, &pos);
        /*No data read */
        if( cnt <= 0){
            break;
        }
        if( (data_end + cnt) > DATA_BUF_LEN )
        {
            printk("data_buf is full,breadk!\n");
            break;
        }
        ptr = buff;
        index = cnt;
        while(index-->0)
        {   /* 2011.05.31
            printk("%c",*ptr);
            ptr++;
            */
            /* 20110804 */
            data_buf[data_end] = *ptr;
            ptr++;
			data_end++;
        }
    }
    
    set_fs(old_fs); 
    filp_close(file,   NULL);
    kfree(buff);
    return 0;
}
#endif

static int apanic(struct notifier_block *this, unsigned long event,
			void *ptr)
{
	struct apanic_data *ctx = &drv_ctx;
	struct panic_header *hdr = (struct panic_header *) ctx->bounce;
	int console_offset = 0;
	int console_len = 0;
	int threads_offset = 0;
	int threads_len = 0;

#ifdef CONFIG_HUAWEI_APANIC_EXTEND
	int sysinfo_offset = 0;
	int sysinfo_len = 0;
#endif
	
	int rc;

	if (in_panic)
		return NOTIFY_DONE;
	in_panic = 1;
#ifdef CONFIG_PREEMPT
	/* Ensure that cond_resched() won't try to preempt anybody */
	add_preempt_count(PREEMPT_ACTIVE);
#endif
	touch_softlockup_watchdog();

	if (!ctx->mtd) {
        printk(KERN_EMERG "No mtd partition in use!\n");
        goto out;
    }

	if (ctx->curr.magic) {
		printk(KERN_EMERG "Crash partition in use!\n");
		goto out;
	}
	console_offset = ctx->mtd->writesize;

	/*
	 * Write out the console
	 */
	console_len = apanic_write_console(ctx->mtd, console_offset);
	if (console_len < 0) {
		printk(KERN_EMERG "Error writing console to panic log! (%d)\n",
		       console_len);
		console_len = 0;
	}

	/*
	 * Write out all threads
	 */
	threads_offset = ALIGN(console_offset + console_len,
			       ctx->mtd->writesize);
	if (!threads_offset)
		threads_offset = ctx->mtd->writesize;

	ram_console_enable_console(0);

	log_buf_clear();
	show_state_filter(0);
	threads_len = apanic_write_console(ctx->mtd, threads_offset);
	if (threads_len < 0) {
		printk(KERN_EMERG "Error writing threads to panic log! (%d)\n",
		       threads_len);
		threads_len = 0;
	}

#ifdef CONFIG_HUAWEI_APANIC_EXTEND
    memset(data_buf,0x00,sizeof(data_buf));
	data_end = 0;
	/*
	 * Write out all added info
	 */
	sysinfo_offset = ALIGN(console_offset + console_len + threads_offset + threads_len,
			       ctx->mtd->writesize);
	if (!sysinfo_offset){
		sysinfo_offset = ctx->mtd->writesize;
	}
   
    /* get data to buffer*/
    apanic_meminfo_proc_get();
    apanic_stat_proc_get();
    apanic_zoneinfo_proc_get();
    apanic_vmstat_proc_get();
    apanic_vmalloc_proc_get();
    apanic_slabinfo_proc_get();

    data_end += sprintf(&data_buf[data_end],"%s","-------- apanic_sysinfo ---------\n");
    /* save buffer data to flash */
	sysinfo_len = apanic_write_sysinfo(ctx->mtd,sysinfo_offset);
	if (sysinfo_len < 0) {
		printk(KERN_EMERG "Error writing sysinfo to panic log! (%d)\n",
		       sysinfo_len);
		sysinfo_len = 0;
	}
#endif

	/*
	 * Finally write the panic header
	 */
	memset(ctx->bounce, 0, PAGE_SIZE);
	hdr->magic = PANIC_MAGIC;
	hdr->version = PHDR_VERSION;

	hdr->console_offset = console_offset;
	hdr->console_length = console_len;

	hdr->threads_offset = threads_offset;
	hdr->threads_length = threads_len;

/* add sysinfo file header */
#ifdef CONFIG_HUAWEI_APANIC_EXTEND
	hdr->sysinfo_offset = sysinfo_offset;
	hdr->sysinfo_length = sysinfo_len;
#endif

	rc = apanic_writeflashpage(ctx->mtd, 0, ctx->bounce);
	if (rc <= 0) {
		printk(KERN_EMERG "apanic: Header write failed (%d)\n",
		       rc);
		goto out;
	}

	/*we should sync the log to mmc*/
#ifdef CONFIG_HUAWEI_KERNEL
    if (ctx->mtd->sync)
    {
        ctx->mtd->sync(ctx->mtd);
    }
#endif
	printk(KERN_EMERG "apanic: Panic dump sucessfully written to flash\n");

 out:
#ifdef CONFIG_PREEMPT
	sub_preempt_count(PREEMPT_ACTIVE);
#endif
	in_panic = 0;
	return NOTIFY_DONE;
}

static struct notifier_block panic_blk = {
	.notifier_call	= apanic,
};

static int panic_dbg_get(void *data, u64 *val)
{
	apanic(NULL, 0, NULL);
	return 0;
}

static int panic_dbg_set(void *data, u64 val)
{
	BUG();
	return -1;
}

DEFINE_SIMPLE_ATTRIBUTE(panic_dbg_fops, panic_dbg_get, panic_dbg_set, "%llu\n");

int __init apanic_init(void)
{
	register_mtd_user(&mtd_panic_notifier);
	atomic_notifier_chain_register(&panic_notifier_list, &panic_blk);
	debugfs_create_file("apanic", 0644, NULL, NULL, &panic_dbg_fops);
	memset(&drv_ctx, 0, sizeof(drv_ctx));
	drv_ctx.bounce = (void *) __get_free_page(GFP_KERNEL);
	INIT_WORK(&proc_removal_work, apanic_remove_proc_work);
	printk(KERN_INFO "Android kernel panic handler initialized (bind=%s)\n",
	       CONFIG_APANIC_PLABEL);
	return 0;
}

module_init(apanic_init);
