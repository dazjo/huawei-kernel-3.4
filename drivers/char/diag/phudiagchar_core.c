/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * Alternatively, provided that this notice is retained in full, this software
 * may be relicensed by the recipient under the terms of the GNU General Public
 * License version 2 ("GPL") and only version 2, in which case the provisions of
 * the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
 * software under the GPL, then the identification text in the MODULE_LICENSE
 * macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a
 * recipient changes the license terms to the GPL, subsequent recipients shall
 * not relicense under alternate licensing terms, including the BSD or dual
 * BSD/GPL terms.  In addition, the following license statement immediately
 * below and between the words START and END shall also then apply when this
 * software is relicensed under the GPL:
 *
 * START
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 and only version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * END
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/diagchar.h>
#include <linux/sched.h>
#include <mach/usbdiag.h>
#include <asm/current.h>
#include "phudiagchar.h"
#include <linux/timer.h>

MODULE_DESCRIPTION("PHU Diag Char Driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0");

struct phudiag_dev *phudriver;


static int phudiag_open(struct inode *inode, struct file *file)
{	
	if (phudriver && file ) 
	{
		mutex_lock(&phudriver->diagchar_mutex);

		if(((file->f_flags & O_ACCMODE) == O_RDONLY)
			&& 0 == phudriver->read_flag)
		{
			printk("phudiag_open  read only!\n");
			phudriver->read_flag = 1;
		}
		else if(((file->f_flags & O_ACCMODE) == O_WRONLY)
			&& 0 == phudriver->write_flag)
		{
			printk("phudiag_open  write only!\n");
			phudriver->write_flag = 1;
		}
		else if(((file->f_flags & O_ACCMODE) == O_RDWR)
			&& 0 == phudriver->write_flag 
			&&  0 == phudriver->read_flag) 
		{
			printk("phudiag_open  read write!\n");
			phudriver->read_flag = 1;
			phudriver->write_flag = 1;			
		}
		else
		{
			printk("phudiag_open phudiag has be opened yet !\n");
			mutex_unlock(&phudriver->diagchar_mutex);
			return -ENOMEM;
		}

		if(phudriver->read_flag && phudriver->write_flag)
		{
			printk("phudiag_open ok!. \n");

			phudiag_ring_buf_clear(phudriver->in_buf);
			phudiag_ring_buf_clear(phudriver->out_buf);
			phudriver->opened = 1;
		}
		mutex_unlock(&phudriver->diagchar_mutex);
		return 0;
	}
	printk("phudiagchar : phudiag_open failed!. \n");
	
	return -ENOMEM;
}

static int phudiag_close(struct inode *inode, struct file *file)
{

	if (phudriver)
	{
		mutex_lock(&phudriver->diagchar_mutex);
		phudriver->opened = 0;
		phudriver->read_flag = 0;
		phudriver->write_flag = 0;
		mutex_unlock(&phudriver->diagchar_mutex);
		
		printk("phudiag_close ok! \n");

		return 0;
	}
	return 0;
}

/*
static int phudiag_ioctl(struct inode *inode, struct file *filp,
			   unsigned int iocmd, unsigned long ioarg)
{
	int len = 0;

	#ifdef PHUDIAG_DEBUG
	printk("Enter phudiag_ioctl(). \n");
	#endif
	
	switch (iocmd) {
	//case FIONREAD:
	case 0x541B:
		if(phudriver->in_buf_busy)
		{
			return 0;
		}
		
		phudriver->in_buf_busy = 1;
		len = phudiagfwd_ring_buf_get_data_length(phudriver->in_buf);
		phudriver->in_buf_busy = 0;
		
		#ifdef PHUDIAG_DEBUG
		printk("phudiag_ioctl() : len=%d \n",len);
		#endif
		
		if(len >= 0)
		{
			return put_user(len, (int __user *)ioarg);	
		}
		else
		{
			return -EINVAL;
		}
	default:
		printk("phudiag_ioctl() : return -EINVAL. \n");
		return -EINVAL;
	}
}
*/
static int phudiag_read(struct file *file, char __user *buf, size_t count,
			  loff_t *ppos)
{	
	int ret = 0;
	#ifdef PHUDIAG_DEBUG
	printk("Enter phudiag_read(). \n");
	#endif
	
	

	ret = phudiagfwd_user_get_data(buf,count);
	queue_work(phudriver->diag_wq, phudriver->diag_read_smd_work);
	return  ret;
	
}

static int phudiag_write(struct file *file, const char __user *buf,
			      size_t count, loff_t *ppos)
{
	int ret = -1;
	int i = 0;
	//#ifdef PHUDIAG_DEBUG
	printk("phudiagchar : Enter phudiag_write(). count=%4d ",count);
	//#endif

	for(i=0;i<count;i++)
	{
		printk(" %x",buf[i]);
	}
	printk(" \n");

	ret =  phudiagfwd_user_set_data(buf, count);

	queue_work(phudriver->diag_wq , &(phudriver->phudiag_write_work));
	
	#ifdef PHUDIAG_DEBUG
	printk("exit phudiag_write(). \n");
	#endif
	
	return ret;
}

static const struct file_operations phudiagfops = {
	.owner = THIS_MODULE,
	.read = phudiag_read,
	.write = phudiag_write,
	//.ioctl = phudiag_ioctl,
	.open = phudiag_open,
	.release = phudiag_close
};

static int phudiag_setup_cdev(dev_t devno)
{

	int err;
	
	#ifdef PHUDIAG_DEBUG
	printk("Enter phudiag_setup_cdev(). \n");
	#endif

	cdev_init(phudriver->cdev, &phudiagfops);

	phudriver->cdev->owner = THIS_MODULE;
	phudriver->cdev->ops = &phudiagfops;

	err = cdev_add(phudriver->cdev, devno, 1);

	if (err) {
		printk("phu diagchar cdev registration failed !\n\n");
		return -1;
	}

	phudriver->phudiagchar_class = class_create(THIS_MODULE, "phudiag");

	if (IS_ERR(phudriver->phudiagchar_class)) {
		printk(KERN_ERR "Error creating phu diagchar class.\n");
		return -1;
	}

	device_create(phudriver->phudiagchar_class, NULL, devno,
				  (void *)phudriver, "phudiag");

	return 0;

}

static int phudiag_cleanup(void)
{
	#ifdef PHUDIAG_DEBUG
	printk("Enter phudiag_cleanup(). \n");
	#endif
	
	if (phudriver) {
		if(phudriver->in_buf)
		{
			phudiag_ring_buf_free(phudriver->in_buf);
		}

		if(phudriver->out_buf)
		{
			phudiag_ring_buf_free(phudriver->out_buf);
		}
		

		
		if(phudriver->smd_buf)
		{
			kfree(phudriver->smd_buf);
		}

		if (phudriver->cdev) {
			/* TODO - Check if device exists before deleting */
			device_destroy(phudriver->phudiagchar_class,
				       MKDEV(phudriver->major,
					     phudriver->minor_start));
			cdev_del(phudriver->cdev);
		}
		if (!IS_ERR(phudriver->phudiagchar_class))
			class_destroy(phudriver->phudiagchar_class);
		kfree(phudriver);
	}
	return 0;
}


//static int __init phudiag_init(void)
int phudiag_init(void)
{
	dev_t dev;
	int error;
	
	#ifdef PHUDIAG_DEBUG
	printk("Entering phudiag_init...\n");
	#endif
	
	phudriver = kzalloc(sizeof(struct phudiag_dev) + 8, GFP_KERNEL);

	if (phudriver) {
	       phudriver->used = 0;
		phudriver->opened = 0;
		phudriver->read_flag = 0;
		phudriver->write_flag = 0;

	       #ifdef PHUDIAG_DEBUG
		printk("phudiag char initializing ..\n");
	       #endif
		phudriver->num = 1;
		phudriver->name = ((void *)phudriver) + sizeof(struct phudiag_dev);
		strlcpy(phudriver->name, "phudiag", 7);

		phudriver->in_buf = phudiag_ring_buf_malloc(PHU_DIAG_IN_BUF_SIZE);
		if(!phudriver->in_buf)
		{
			goto fail;
		}	

		phudriver->out_buf = phudiag_ring_buf_malloc(PHU_DIAG_OUT_BUF_SIZE);
		if(!phudriver->out_buf)
		{
			goto fail;
		}
		

		
		phudriver->smd_buf = kzalloc(PHU_DIAG_SMD_BUF_MAX, GFP_KERNEL);
		if(!phudriver->smd_buf)
		{
			printk("phudiag_init :phudriver->smd_buf  kzalloc fail!\n");
			goto fail;
		}
		mutex_init(&phudriver->diagchar_mutex);
	
		INIT_WORK(&(phudriver->phudiag_write_work), phudiagfwd_write_to_smd_work_fn);

		/* Get major number from kernel and initialize */
		error = alloc_chrdev_region(&dev, phudriver->minor_start,
					    phudriver->num, phudriver->name);
		if (!error) {
			phudriver->major = MAJOR(dev);
			phudriver->minor_start = MINOR(dev);
		} else {
			printk("phudiag_init : Major number not allocated \n");
			goto fail;
		}
		phudriver->cdev = cdev_alloc();
		error = phudiag_setup_cdev(dev);
		if (error)
			goto fail;
	} else {
		printk("phudiag_init : kzalloc failed\n");
		goto fail;
	}
	
	#ifdef PHUDIAG_DEBUG
	printk("phudiag_init : phudiag initialized\n");
	#endif
	return 0;

fail:
	phudiag_cleanup();
	return -1;

}

//static void __exit phudiag_exit(void)
void phudiag_exit(void)
{
	#ifdef PHUDIAG_DEBUG
	printk("phudiag_exit : PHU diagchar exiting ..\n");
	#endif
	phudiag_cleanup();
	
	#ifdef PHUDIAG_DEBUG
	printk("phudiag_exit : done PHU diagchar exit\n");
	#endif
}

//module_init(phudiag_init);
//module_exit(phudiag_exit);
