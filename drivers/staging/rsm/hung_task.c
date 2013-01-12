/*
 * Detect Hung Task
 *
 * kernel/hung_task.c - kernel thread for detecting tasks stuck in D state
 *
 */
#include <linux/mm.h>
#include <linux/cpu.h>
#include <linux/nmi.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/lockdep.h>
#include <linux/module.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#include <linux/sched.h>

#include "rsm_find_func_addr.h"

/*
 * The number of tasks checked:
 */

#define NAME_NUM  16

#define TIMEOUT   120

#define HUNG_TASK_BATCHING 1024

#define RSM_OK       0x0		/*OK                          */

#define RSM_ERDIR    0x4		/*Directory operation error   */
#define RSM_ERENTRY  0x5		/*File entry error            */

/* storage names and last switch counts of process in D status */
typedef struct tagSwitchCount
{
	char p_name[TASK_COMM_LEN];
	unsigned long last_swithc_count;
}SwitchCount;

static SwitchCount last_switch_count_table[NAME_NUM];

unsigned long __read_mostly sysctl_hung_task_check_count = PID_MAX_LIMIT;

static char p_name[TASK_COMM_LEN*NAME_NUM]={0};

/* storage proccess not dectected in hung task mechanism */
static char p_name_table[NAME_NUM][TASK_COMM_LEN];



static unsigned long hung_task_timeout_secs= TIMEOUT;


/*
 * Limit number of tasks checked in a batch.
 *
 * This value controls the preemptibility of khungtaskd since preemption
 * is disabled during the critical section. It also controls the size of
 * the RCU grace period. So it needs to be upper-bound.
 */


/*
 * Zero means infinite timeout - no checking done:
 */
unsigned long __read_mostly sysctl_hung_task_warnings = 10;

static int __read_mostly did_panic;

static struct task_struct *watchdog_task;

struct proc_dir_entry *zc_procdir;


/*
 * Should we panic (and reboot, if panic_timeout= is set) when a
 * hung task is detected:
 */
unsigned int __read_mostly sysctl_hung_task_panic = 1;

static int
hung_task_panic(struct notifier_block *this, unsigned long event, void *ptr)
{
	did_panic = 1;

	return NOTIFY_DONE;
}

static struct notifier_block panic_block = {
	.notifier_call = hung_task_panic,
};

static void check_hung_task(struct task_struct *t, unsigned long timeout)
{
   	unsigned long switch_count = t->nvcsw + t->nivcsw;
	int idx;
	int first_empty_item = -1;
	int i = 0;


	while(('\0' != p_name_table[i][0]) && (i < NAME_NUM)) 
	{
		if (strncmp(p_name_table[i], t->comm, strlen(t->comm)) == 0)
			return;
		i++;
	}

	/*
	 * Ensure the task is not frozen.
	 * Also, when a freshly created task is scheduled once, changes
	 * its state to TASK_UNINTERRUPTIBLE without having ever been
	 * switched out once, it musn't be checked.
	 */

	
	if (unlikely(t->flags & PF_FROZEN || !switch_count))
		return;

	/* find last swich count record in last_switch_count_table */
	for(idx = 0; idx < NAME_NUM; idx++)
	{
		if ('\0' == last_switch_count_table[idx].p_name[0])
		{
			if (-1 == first_empty_item)
				first_empty_item = idx;
		}
		else if (strcmp(last_switch_count_table[idx].p_name, t->comm) == 0)
		{
			break;
		}
	}

	/* if current proccess is not in last switch count table, insert a new record */
	if (NAME_NUM == idx)
	{
		strcpy(last_switch_count_table[first_empty_item].p_name, t->comm);
		last_switch_count_table[first_empty_item].last_swithc_count = 0;
		idx = first_empty_item;
	}
	
	if (switch_count != last_switch_count_table[idx].last_swithc_count) {
		last_switch_count_table[idx].last_swithc_count = switch_count;
		return;
	}
	
	if (!sysctl_hung_task_warnings)
		return;
	sysctl_hung_task_warnings--;

	/*
	 * Ok, the task did not get scheduled for more than 2 minutes,
	 * complain:
	 */
	
	printk(KERN_ERR "INFO: task %s:%d blocked for more than "
			"%ld seconds.\n", t->comm, t->pid, timeout);
	printk(KERN_ERR "\"echo 0 > /proc/sys/kernel/hung_task_timeout_secs\""
			" disables this message.\n");
	/* 
	 * in our module, use sched_show_task_macro replace of sched_show_task because
     * "sched_show_task" this function is not an export symbol in kernel.
	 */		
	sched_show_task_macro(t);
	__debug_show_held_locks(t);

	touch_nmi_watchdog();

	if (sysctl_hung_task_panic)
		panic("hung_task: blocked tasks");
}

/*
 * To avoid extending the RCU grace period for an unbounded amount of time,
 * periodically exit the critical section and enter a new one.
 *
 * For preemptible RCU it is sufficient to call rcu_read_unlock in order
 * exit the grace period. For classic RCU, a reschedule is required.
 */
static void rcu_lock_break(struct task_struct *g, struct task_struct *t)
{
	get_task_struct(g);
	get_task_struct(t);
	rcu_read_unlock();
	cond_resched();
	rcu_read_lock();
	/* 
	 * in our module, use __put_task_struct_macro replace of put_task_struct because
     * "put_task_struct" this function is not an export symbol in kernel.
	 */
	__put_task_struct_macro(t);
	__put_task_struct_macro(g);
}

/*
 * Check whether a TASK_UNINTERRUPTIBLE does not get woken up for
 * a really long time (120 seconds). If that happens, print out
 * a warning.
 */
static void check_hung_uninterruptible_tasks(unsigned long timeout)
{
	int max_count = sysctl_hung_task_check_count;
	int batch_count = HUNG_TASK_BATCHING;
	struct task_struct *g, *t;

	/*
	 * If the system crashed already then all bets are off,
	 * do not report extra hung tasks:
	 */
	if (test_taint(TAINT_DIE) || did_panic)
		return;

	rcu_read_lock();
	do_each_thread(g, t) {
		if (!max_count--)
			goto unlock;
		if (!--batch_count) {
			batch_count = HUNG_TASK_BATCHING;
			rcu_lock_break(g, t);
			/* Exit if t or g was unhashed during refresh. */
			if (t->state == TASK_DEAD || g->state == TASK_DEAD)
				goto unlock;
		}
		/* use "==" to skip the TASK_KILLABLE tasks waiting on NFS */
		if (t->state == TASK_UNINTERRUPTIBLE)
			check_hung_task(t, timeout);
	} while_each_thread(g, t);
 unlock:
	rcu_read_unlock();
}

static unsigned long timeout_jiffies(unsigned long timeout)
{
	/* timeout of 0 will disable the watchdog */
	return timeout ? timeout * HZ : MAX_SCHEDULE_TIMEOUT;
}

/*
 * Process updating of timeout sysctl
 */
int proc_dohung_task_timeout_secs(struct ctl_table *table, int write,
				  void __user *buffer,
				  size_t *lenp, loff_t *ppos)
{
	int ret;

	ret = proc_doulongvec_minmax(table, write, buffer, lenp, ppos);

	if (ret || !write)
		goto out;

	wake_up_process(watchdog_task);

 out:
	return ret;
}

/*
 * kthread which checks for tasks stuck in D state
 */
static int watchdog(void *dummy)
{
	
	set_user_nice(current, 0);

	/* 
	 * When "watchdog" thread is stopped by other process, 
	 * we should exit watchdog.
	 */
	while (!kthread_should_stop())
    {
		unsigned long timeout = hung_task_timeout_secs;

		while (schedule_timeout_interruptible(timeout_jiffies(timeout)))
			timeout = hung_task_timeout_secs;
		
		check_hung_uninterruptible_tasks(timeout);
	}

	return 0;
}

/*  
 * rsm_proc_timeout_read  -  Called when 'cat' method is used on entry 'timeout' in /proc fs.
 * most of the parameters is created by kernel.
 */
int rsm_proc_timeout_read(char *page,char **start,off_t off,int count,int *eof,void *data)
{
	int len;
    char *buf = page;

    len = 0;
	
	len = sprintf(buf,"%ld\n", hung_task_timeout_secs);
    buf += len;
    
    len = buf - page;
    if (len < off+count) {
        *eof=1;
    }

    *start = page+off;
    len -= off;

    if (len>count) {
        len=count;
    }

    return len;
}

/*  
 * rsm_proc_timeout_write  -  Called when 'write' method is used on entry 'timeout' in /proc fs.
 */
int rsm_proc_timeout_write(struct file *file,
			const char __user * buffer, unsigned long count, void *data)
{
	long timeout = 0;
	char tmp[16];
	if (count > sizeof(tmp)){
		printk(KERN_ERR "RSM: input string is too long\n");
		return -EINVAL;
	}	
	if (copy_from_user(tmp,buffer,count)) 
		return -EFAULT;
	timeout = simple_strtol(tmp, NULL, 10);
	if(timeout <= 0){
		hung_task_timeout_secs = TIMEOUT;
	}
	else{
		hung_task_timeout_secs = timeout;
	}
	return count; 
}

/*  
 * rsm_proc_pname_read  -  Called when 'cat' method is used on entry 'pname' in /proc fs.
 * most of the parameters is created by kernel.
 */
int rsm_proc_pname_read(char *page,char **start,off_t off,int count,int *eof,void *data)
{
	int len;
    char *buf = page;

    len = 0;

	len = sprintf(buf,"%s\n", p_name);
    buf += len;
    
    len = buf - page;
    if (len < off+count) {
        *eof=1;
    }

    *start = page+off;
    len -= off;

    if (len>count) {
        len=count;
    }

    return len;
}


/* 
 * storage proccess names in [pname] to [pname_table], and return 
 * the numbers of process
 */
static int rebuild_name_table(char (* pname_table)[TASK_COMM_LEN], char *pname, int pname_len)
{
	int count = 0;
	int proc_name_len;
	const char *curr = pname;
	char *curr_table;

	/* reset the table to empty */

    memset((void *) pname_table, 0x00, sizeof(p_name_table));
	
	while ('\0' != *curr && pname_len )
	{
		/* proccess names are seperated by comma or space */
		while ((',' == *curr || ' ' == *curr) && pname_len )
		{
			curr ++;
			pname_len --;
		}

		/* check if the number of proccess exceed the limit, pointer [curr] not an end symbol indicates that the after [NAME_NUM] proccess, the [NAME_NUM + 1]th proccess was found */
		if (NAME_NUM == count && '\0' != *curr)
		{
			goto err_proc_num;
		}

		/* if the user input [NAME_NUM] proccess name, but just end his input by a space or comma, we just jump out the loop */
		if (NAME_NUM == count)
		{
			break;
		}

		/* the [count]th name should be storage in corresponding item in table, and [proc_name_len] is set to count the length of process name */
		proc_name_len = 0;
		curr_table = pname_table[count];

		while (',' != *curr &&  ' ' != *curr && '\0' != *curr && pname_len)
		{
			*curr_table = *curr;
			curr_table ++;
			curr ++;
			proc_name_len ++;

			/* check if the length of proccess name exceed the limit */
			if (TASK_COMM_LEN == proc_name_len)
			{
				goto err_proc_name;
			}
			pname_len --;
		}
		*curr_table = '\0';

		printk("\n rsm: build_name_table: %d, %s, proc_name_len: %d \n", count, pname_table[count], proc_name_len);

		/* count how many proccess, only when [proc_name_len] is not zero, one new proccess was added into [pname_table] */
		if (proc_name_len)
		{
			count ++;
		}
	}

	return count;
	
err_proc_name:
	memset(p_name_table,0x00,sizeof(p_name_table));
	memset(p_name,0x00,sizeof(p_name));
    printk(" rebuild_name_table: Error: process name is invallid, set /proc/rsm/pname failed.\n");

	return 0;

err_proc_num:
	/* more than 16 processes,remove it */
	printk(" rebuild_name_table: Warnig: too many processess, leave it and do nothing.\n");
	return count;
}

/* 
 * since the proccess name written into [pname_table] may be different from 
 * original input, [p_name] should be modified to adjust [pname_table]
 */
static int modify_pname(int num_count)
{
	int i, len_count;

	memset((void *)p_name, 0x00, sizeof(p_name));

	for (i = 0; i < num_count; i ++)
	{
		strcat(p_name, p_name_table[i]);
		
		/* seperate different proccess by a comma and a space */
		if (i != num_count - 1){
			strcat(p_name, ",");
		}
	}
	printk("the buffer now is: %s", p_name);

	len_count = strlen(p_name);
	return len_count;
}

/*  
 * rsm_proc_timeout_write  -  Called when 'write' method is used on 
 * entry 'timeout' in /proc fs.
 */
int rsm_proc_pname_write(struct file *file,
			const char __user * buffer, unsigned long count, void *data)
{
	int num_count;

	/* TASK_COMM_LEN * NAME_NUM * 2 might be larger than 128, put it into the static area is a better choice */
	static char tmp[TASK_COMM_LEN*NAME_NUM]={0};

	/* reset p_name to NULL */
	memset(p_name,0x00,sizeof(p_name));
	
	if (count > sizeof(tmp)){
		printk(KERN_ERR "RSM: input string is too long\n");
		return -EINVAL;
	}	
	
	if (copy_from_user(tmp,buffer,count)){
		return -EFAULT;
	}

	/* -1: remove '\n'  */
	strncpy(p_name, tmp, count-1);

	/* convert [p_name] to a table [p_name_table], and refresh the buffer [p_name] */
	num_count = rebuild_name_table(p_name_table, p_name, count - 1);
	modify_pname(num_count);
	
	return count; 
}


/*  
 * create proc node in /proc fs.
 */
int create_proc(void)
{
	struct proc_dir_entry *res;

	/*Create zcopy directory*/
	zc_procdir = proc_mkdir("rsm",NULL); 
	if (!zc_procdir) {
		return RSM_ERDIR;
	}
	/*Create attributes files*/
	res = create_proc_read_entry("timeout", 0, zc_procdir, rsm_proc_timeout_read,0);
	if (!res) {
		remove_proc_entry("rsm",NULL);
		return RSM_ERENTRY;
	}
	res->write_proc = (write_proc_t *)rsm_proc_timeout_write;

	res = create_proc_read_entry("pname", 0, zc_procdir, rsm_proc_pname_read,0);
    if (!res) {
		remove_proc_entry("timeout",zc_procdir);
	    remove_proc_entry("rsm",NULL);
        return RSM_ERENTRY;
    }
	res->write_proc = (write_proc_t *)rsm_proc_pname_write;
	return RSM_OK;
}

/*  
 * remove proc node in /proc fs.
 */
void remove_proc(void)
{
	remove_proc_entry("timeout",zc_procdir);
    remove_proc_entry("pname",zc_procdir);
    remove_proc_entry("rsm",NULL);
	return;
}

/*
 * hung_task_init  -  Initial hung_task
 */
int hung_task_init(void)
{
	/* create directory and entry for rsm */
	create_proc();
	atomic_notifier_chain_register(&panic_notifier_list, &panic_block);
	watchdog_task = kthread_run(watchdog, NULL, "khungtaskd");
	return 0;
}


/*
 *  hung_task_exit  -  hung_task Exit
 */
int hung_task_exit(void)
{
 	hung_task_timeout_secs = 1;
	
	if (watchdog_task){
         kthread_stop(watchdog_task);
	}

	atomic_notifier_chain_unregister(&panic_notifier_list, &panic_block);	
	remove_proc();
	return 0;
}


