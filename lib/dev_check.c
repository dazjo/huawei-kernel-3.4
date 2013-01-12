/*
 * FileName:       dev_check.c
 * Author:         w00126555  Version: v1.0  Date: 2012-7-10
 * Description:    Check device state, if the device abnormal, send msg to exception node
 * Version:        v1.0
 * Function List:  
 *                 1.static int __init init_dev_check(void)
                   2.static void __exit exit_dev_check(void)
                   3.void dc_register_handler(struct dc_handler *handler)
                   4.void dc_unregister_handler(struct dc_handler *handler)
                   5.static int dev_info_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
                   6.static int dev_ctrl_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
                   7.static int dev_ctrl_write_proc (struct file *file, const char *buffer, 
                   8.static int dc_thread(void *data)
                   9.static void dc_node_handler(struct dc_handler *handler)
                   10.static int dc_node_handler_need(struct dc_handler *handler)
                   11.static void dc_coupling_node_handler(void)
                   12.static void dc_coupling_node_handler_action(int node_num)
                   13.static void dc_send_abnormal2expetion(char *node_name)
                   14.int dc_send_abnormal2expetion_logger_write(const enum logidx index,
                                                                 const unsigned char prio,
                                                                 const char __kernel * const tag,
                                                                 const char __kernel * const fmt,
                                                                 ...)
 * History:        
 *     <author>   <time>    <version >   <desc>
 *     w00126555  09.19     v1.0         Init
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/hardirq.h>

#include <linux/dev_check.h>

//#define DEV_CHECK_DEBUG

#ifdef DEV_CHECK_DEBUG
#define dcprintk(format, args...) printk(format, ##args)
#define FUNC_BEGIN printk("\n[%s] Begin\n", __FUNCTION__)
#define FUNC_END printk("[%s] End\n\n", __FUNCTION__)
#else
#define dcprintk(format, args...)
#define FUNC_BEGIN
#define FUNC_END
#endif

static LIST_HEAD(dev_check_list);
static DEFINE_MUTEX(dc_mutex);
static struct hlist_head dc_hash[DC_COUPLING_SIZE];

struct proc_dir_entry *dev_check_dir = NULL;

#define DC_SCHD_TIME (5 * MSEC_PER_SEC)
static struct task_struct * g_dc_thread = NULL;
static wait_queue_head_t dc_waitq;

/* every bit stand for a coupling relation. Notes: bit0 not used */
static int g_dc_coupling_flag = 0;

/* save the value of /proc/dev_ctrl, the cmd range:[1,9],[a,z],[A,Z] */
static char cmd_from_user;

/* cmd_from_user */
#define CMDFU_1 '1'
#define CMDFU_2 '2'
#define CMDFU_3 '3'
#define CMDFU_4 '4'
#define CMDFU_5 '5'
#define CMDFU_6 '6'
#define CMDFU_7 '7'
#define CMDFU_8 '8'
#define CMDFU_9 '9'

/*
 * Function:       static inline int time_converter(int time_val)
 * Description:    check the interval time
 * Input:          int timek_val: input interval time
 * Output:         No
 * Return:         the time by calc
 */
static inline int time_converter(int time_val)
{
    int tmp_poll_time_secs = 0;
    
    if (time_val >= TIME_VAL_LOW_MIN && time_val <= TIME_VAL_LOW_MAX)
    {
        tmp_poll_time_secs = TIME_VAL_LOW_POLL_TIME;
    }
    else if (time_val >= TIME_VAL_MID_MIN && time_val <= TIME_VAL_MID_MAX)
    {
        tmp_poll_time_secs = TIME_VAL_MID_POLL_TIME;
    }
    else if (time_val >= TIME_VAL_HIG_MIN)
    {
        tmp_poll_time_secs = TIME_VAL_HIG_POLL_TIME;
    }
    else
    {
        tmp_poll_time_secs = TIME_VAL_MID_POLL_TIME;
    }
    
    return tmp_poll_time_secs;
}

#ifdef CONFIG_PROC_FS
#define DC_PROCFS_DEV_INFO_NAME "Device Name"
#define DC_PROCFS_DEV_INFO_IN_CNT "Input Cnt"
#define DC_PROCFS_DEV_INFO_OUT_CNT "Output Cnt"

/*
 * Function:       static int dev_info_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
 * Description:    read /proc/dev_check/dev_info
 * Input:          the standerd of proc file
 * Output:         the standerd of proc file
 * Return:         the standerd of proc file
 */
static int dev_info_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    struct dc_handler *handler;
    struct list_head *i;
    
    len = sprintf(page, "%32s\t%16s\t%16s\n", DC_PROCFS_DEV_INFO_NAME, DC_PROCFS_DEV_INFO_IN_CNT, DC_PROCFS_DEV_INFO_OUT_CNT);
    
    mutex_lock(&dc_mutex);
    
    list_for_each(i, &dev_check_list)
    {
        handler = list_entry(i, struct dc_handler, node);
        len += sprintf(page+len, "%32s\t%16lu\t%16lu\n", handler->name, handler->old_value_input, handler->old_value_output);
    }
    
    mutex_unlock(&dc_mutex);
    
    #ifdef DEV_CHECK_DEBUG
    len += sprintf(page + len, "%32s\t%16lu\n", "jiffies<test>", jiffies);
    len += sprintf(page + len, "%32s\t0x%16x\n", "global coupling flag", g_dc_coupling_flag);
    #endif	
    
    if (len <= off + count)
    {
        *eof = 1;
    }
    
    *start = page + off;
    len -= off;
    if (len > count)
    {
        len = count;
    }
    
    if (len < 0)
    {
        len = 0;
    }
    
    return len;
}

/*
 * Function:       static int dev_ctrl_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
 * Description:    read /proc/dev_check/dev_ctrl
 * Input:          the standerd of proc file
 * Output:         the standerd of proc file
 * Return:         the standerd of proc file
 */
static int dev_ctrl_read_proc(char *page, char **start, off_t off, int count,  int *eof, void *data)
{
    int len = 0;
    
    len = sprintf(page, "%c\n", cmd_from_user);
    if (len <= off + count)
    {
        *eof = 1;
    }

    *start = page + off;
    len -= off;
    if (len > count)
    {
        len = count;
    }
    
    if (len < 0)
    {
        len = 0;
    }
    
    return len;
}

/*
 * Function:       static int dev_ctrl_write_proc (struct file *file, const char *buffer, unsigned long count, void *data)
 * Description:    write /proc/dev_check/dev_ctrl
 * Input:          the standerd of proc file
 * Output:         the standerd of proc file
 * Return:         the standerd of proc file
 */
static int dev_ctrl_write_proc (struct file *file, const char *buffer, unsigned long count, void *data)
{
    char flag = 0;
    
    if (buffer && !copy_from_user(&flag, buffer, 1))
    {
        dcprintk("[%s] flag:%c,count %ld\n", __FUNCTION__, flag, count);
        cmd_from_user = flag;
    }
    
    return count;
}
#else
static int dev_info_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    return 0;
}

static int dev_ctrl_read_proc(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
    return 0;
}

static int dev_ctrl_write_proc (struct file *file, const char *buffer, unsigned long count, void *data)
{
    return 0;
}
#endif

/*
 * Function:       void dc_register_handler(struct dc_handler *handler)
 * Description:    register function,and export to kernal
 * Input:          struct dc_handler * handler 
 * Output:         No
 * Return:         No
 */
void dc_register_handler(struct dc_handler *handler)
{
    struct list_head *i = NULL;
    struct dc_handler *tmp_handler = NULL;
    struct list_head *find_node = NULL;
    struct list_head *add_before_node = NULL;
    struct hlist_node *hpos_tmp = NULL;
    struct hlist_node *hpos = NULL;
    int list_is_null = 0;
    unsigned long current_time = jiffies;
    
    if (!handler)
    {
        return;
    }
    
    handler->old_value_input = 0;
    handler->old_value_output = 0;
    handler->real_time = 0;
    handler->poll_time_secs = time_converter(handler->time_val_secs);
    handler->poll_time_jiffies = msecs_to_jiffies(handler->poll_time_secs * 1000);
    handler->real_time = handler->poll_time_jiffies + current_time;
    if (DC_COUPLING_NONE != handler->coupling_flag)
    {
        g_dc_coupling_flag = g_dc_coupling_flag | (1 << handler->coupling_flag);
    }
    
    mutex_lock(&dc_mutex);
    
    list_is_null = list_empty(&dev_check_list);
    
    list_for_each(i, &dev_check_list)
    {
        tmp_handler = list_entry(i, struct dc_handler, node);
        tmp_handler->real_time = tmp_handler->poll_time_jiffies + current_time;
        if (handler->poll_time_secs <= tmp_handler->poll_time_secs && find_node == NULL)
        {
            find_node = i;
        }
    }
    
    add_before_node = find_node ? find_node : &dev_check_list;
    list_add_tail(&handler->node, add_before_node);
    
    if (handler->coupling_flag != DC_COUPLING_NONE
        && handler->coupling_master == DC_COUPLING_MASTER
        && handler->coupling_slave == DC_COUPLING_SLAVE_NONE)
    {
        hlist_add_head(&handler->hnode, &dc_hash[handler->coupling_flag]);
    }
    
    if (handler->coupling_flag != DC_COUPLING_NONE
        && handler->coupling_master == DC_COUPLING_MASTER_NONE
        && handler->coupling_slave == DC_COUPLING_SLAVE)
    {
        if (hlist_empty(&dc_hash[handler->coupling_flag]))
        {
            hlist_add_head(&handler->hnode, &dc_hash[handler->coupling_flag]);
        }
        else
        {
            hlist_for_each(hpos_tmp, &dc_hash[handler->coupling_flag])
            {
                hpos = hpos_tmp;
            }
            
            hlist_add_after(hpos, &handler->hnode);
        }
    }
    
    mutex_unlock(&dc_mutex);

    /* wake up the thread to poll the chain */
    wake_up_interruptible(&dc_waitq);
    
    dcprintk("[DC_CHECK_OK %s] handler->name: %s,poll_time_jiffies:%lu,register!\n", __FUNCTION__, handler->name, handler->poll_time_jiffies);
}
EXPORT_SYMBOL(dc_register_handler);

/*
 * Function:       void dc_unregister_handler(struct dc_handler *handler)
 * Description:    unregister function,and export to kernal
 * Input:          struct dc_handler * handler
 * Output:         No
 * Return:         No
 * Others:         No
 */
void dc_unregister_handler(struct dc_handler *handler)
{
    struct dc_handler *tmp_handler = NULL;
    struct list_head *i = NULL;
    struct hlist_head *head = NULL;
    int hlist_is_null = 0;
    
    mutex_lock(&dc_mutex);
    
    head = &dc_hash[handler->coupling_flag];
    
    list_for_each(i, &dev_check_list)
    {
        tmp_handler = list_entry(i, struct dc_handler, node);
        if ((!strcmp(handler->name, tmp_handler->name))
        && (handler->time_val_secs == tmp_handler->time_val_secs))
        {
            list_del_init(&handler->node);
            hlist_del_init(&handler->hnode);
            break;
        }
    }
    
    hlist_is_null = hlist_empty(head);
    
    if (DC_COUPLING_NONE != handler->coupling_flag && 1 == hlist_is_null)
    {
        g_dc_coupling_flag = g_dc_coupling_flag & (~(1 << handler->coupling_flag));
    }
    
    mutex_unlock(&dc_mutex);
    
    dcprintk("[DC_CHECK_OK %s] handler->name: %s,unregister...\n", __FUNCTION__, handler->name);
}
EXPORT_SYMBOL(dc_unregister_handler);

/*
 * Function:       int dc_send_abnormal2expetion_logger_write(const enum logidx index,
                                                          const unsigned char prio,
                                                          const char __kernel * const tag,
                                                          const char __kernel * const fmt,
                                                          ...)
 * Description:    send msg to /dev/log/exception :copy from kernel
 * Input:          index: node index;
 * Output:         No
 * Return:         0:success;other:fail
 */
int dc_send_abnormal2expetion_logger_write(const enum logidx index,
                                                          const unsigned char prio,
                                                          const char __kernel * const tag,
                                                          const char __kernel * const fmt,
                                                          ...)
{
    int ret = 0;
    va_list vargs;
    struct file *filp = (struct file *)-ENOENT;
    mm_segment_t oldfs;
    struct iovec vec[3];
    int tag_bytes = strlen(tag) + 1, msg_bytes;
    char *msg;

    va_start(vargs, fmt);
    msg = kvasprintf(GFP_ATOMIC, fmt, vargs);
    va_end(vargs);
    if (!msg)
    {
        return -ENOMEM;
    }
    if (in_interrupt()) 
    {
        /* we have no choice since aio_write may be blocked */
        printk(KERN_ALERT "%s",  msg);
        goto out_free_message;
    }
    msg_bytes = strlen(msg) + 1;/* add "\0" */
    if (msg_bytes <= 1) /* empty message? */
    {
        goto out_free_message; /* don't bother, then */
    }
    if ((msg_bytes + tag_bytes + 1) > 2048)
    {
        ret = -E2BIG;
        goto out_free_message;
    }
            
    vec[0].iov_base = (unsigned char *) &prio;
    vec[0].iov_len = 1;
    vec[1].iov_base = (void *) tag;
    vec[1].iov_len = strlen(tag) + 1;
    vec[2].iov_base = (void *) msg;
    vec[2].iov_len = strlen(msg) + 1; 

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    do 
    {
        filp = filp_open(EXCEPTION_NODE_STR, O_WRONLY, S_IRUSR);
        if (IS_ERR(filp) || !filp->f_op)
        {
            printk("%s: filp_open %s error\n", __FUNCTION__,EXCEPTION_NODE_STR);
            ret = -ENOENT;
            break;
        }

        if (filp->f_op->aio_write)
        {
            int nr_segs = sizeof(vec) / sizeof(vec[0]);
            int len = vec[0].iov_len + vec[1].iov_len + vec[2].iov_len;
            struct kiocb kiocb;
            init_sync_kiocb(&kiocb, filp);
            kiocb.ki_pos = 0;
            kiocb.ki_left = len;
            kiocb.ki_nbytes = len;
            ret = filp->f_op->aio_write(&kiocb, vec, nr_segs, kiocb.ki_pos);
        }
    } while (0);

    if (!IS_ERR(filp))
    {
        filp_close(filp, NULL);
    }
    set_fs(oldfs);
out_free_message:
    if (msg)
    {
        kfree(msg);
    }
    return ret;
}

EXPORT_SYMBOL(dc_send_abnormal2expetion_logger_write);

/*
 * Function:       static void dc_send_abnormal2expetion(char *node_name)
 * Description:    send msg to /dev/log/exception
 * Input:          char *node_name
 * Output:         No
 * Return:         No
 */
static void dc_send_abnormal2expetion(struct dc_handler *handler)
{
    if (NULL == handler)
    {
        printk("dc_handler is null,return\n");
        return;
    }
    
    DC_PRINT2EXCEPTION("Device_Check", "Device_Check: %s %d abnormal,handler!!!", handler->name, handler->coupling_flag);
}

/*
 * Function:       static void dc_coupling_node_handler_action(int node_num)
 * Description:    handle slave node
 * Input:          int node_num
 * Output:         No
 * Return:         No
 */
static void dc_coupling_node_handler_action(int node_num)
{
    struct hlist_head *head = &dc_hash[node_num];
    struct hlist_node *master_node = NULL;
    struct dc_handler *master_handler = NULL;
    struct hlist_node *slave_node_first = NULL;
    struct dc_handler *slave_handler = NULL;
    
    if (hlist_empty(head))
    {
        dcprintk("[%s]dc_hash[%d] empty,out!", __FUNCTION__, node_num);
        goto out;
    }
    
    master_node = head->first;
    master_handler = hlist_entry(master_node, struct dc_handler, hnode);
    if (master_handler->coupling_flag != DC_COUPLING_NONE
        && master_handler->coupling_master != DC_COUPLING_MASTER
    )
    {
        dcprintk("[%s]First node in not master!out\n", __FUNCTION__);
        goto out;
    }
    
    if (master_handler->coupling_flag != DC_COUPLING_NONE
        && master_handler->coupling_master == DC_COUPLING_MASTER
        && master_handler->coupling_slave == DC_COUPLING_SLAVE_NONE
    )
    {
        if (master_handler->active_stat != CUR_STAT_ACTIVITY
            || master_handler->coupling_master_no_input == DC_COUPLING_MASTER_NO_INPUT)
        {
            dcprintk("[%s]First master,but not active or no Input!out\n", __FUNCTION__);
            goto out;
        }
    }
    
    if (!master_handler->hnode.next)
    {
        dcprintk("[%s]next pointer of first node is null!out\n", __FUNCTION__);
        goto out;
    }
    
    slave_node_first = master_handler->hnode.next;
    hlist_for_each_entry_from(slave_handler, slave_node_first, hnode)
    {
        if (CUR_STAT_ABNORMAL == slave_handler->active_stat
            && DC_DEV_ACTIVITY == slave_handler->abnormal_flag)
        {
            slave_handler->active_stat = CUR_STAT_INIT;
            slave_handler->abnormal_flag = DC_DEV_ABNORMAL;
            dcprintk("[%s]slave node abnormal,Send Msg to Exception...!\n", __FUNCTION__);
            dc_send_abnormal2expetion(slave_handler);
        }
        dcprintk("[%s]Slave:<%s> Handler\n", __FUNCTION__, slave_handler->name);
    }

    out:
    return;
}

/*
 * Function:       static void dc_coupling_node_handler(void)
 * Description:    handle coupling device
 * Input:          No
 * Output:         No
 * Return:         No
 */
static void dc_coupling_node_handler(void)
{
    int pending = g_dc_coupling_flag;
    int i = 1;
    pending >>= 1;
    
    do
    {
        if (pending & 1)
        {
            dcprintk("[coupling]pending : 0x%x,i:%d\n", pending, i);
            dc_coupling_node_handler_action(i);
        }
        
        pending >>= 1;
        i++;
    } while (pending);
}

/*
 * Function:       static int dc_node_handler_need(struct dc_handler *handler)
 * Description:    judge the device need to handle
 * Input:          struct dc_handler *handler
 * Output:         No
 * Return:         1: not handle  0: need handle
 */
static int dc_node_handler_need(struct dc_handler *handler)
{
    unsigned long get_value_input;
    int ret = 0;

    if (handler->get_var_input
        && ( DC_COUPLING_NONE == handler->coupling_flag
        || (DC_COUPLING_NONE != handler->coupling_flag
        && DC_COUPLING_MASTER == handler->coupling_master
        && DC_COUPLING_SLAVE_NONE == handler->coupling_slave)))
    {
        get_value_input = handler->get_var_input();
        if (get_value_input == handler->old_value_input)
        {
            dcprintk("[%s]Input No Change,do not handler...return 1\n", __FUNCTION__);
            
            if (DC_COUPLING_NONE != handler->coupling_flag
                && DC_COUPLING_MASTER == handler->coupling_master
                && DC_COUPLING_SLAVE_NONE == handler->coupling_slave)
            {
                handler->coupling_master_no_input = DC_COUPLING_MASTER_NO_INPUT;
            }

            ret = 1;
        }
        else
        {
            handler->old_value_input = get_value_input;

            if (DC_COUPLING_NONE != handler->coupling_flag
                && DC_COUPLING_MASTER == handler->coupling_master
                && DC_COUPLING_SLAVE_NONE == handler->coupling_slave)
            {
                handler->coupling_master_no_input = DC_COUPLING_MASTER_INPUT_OK;
            }
        }
    }
    
    return ret;
}

/*
 * Function:       static void dc_node_handler(struct dc_handler *handler)
 * Description:    handler device in chain, if the device abnormal, send msg to exception
 * Input:          struct dc_handler *handler
 * Output:         No
 * Return:         No
 */
static void dc_node_handler(struct dc_handler *handler)
{
    unsigned long get_value = 0;
    FUNC_BEGIN;
    
    if (dc_node_handler_need(handler))
    {
        return;
    }
    
    if (handler->get_var_output)
    {
        get_value = handler->get_var_output();
    }
    
    dcprintk("[%s]get_value:%lu,old_value_output:%lu\n", __FUNCTION__, get_value, handler->old_value_output);
    
    if (get_value != handler->old_value_output)
    {
        dcprintk("FSM : <%s> activity!\n", handler->name);
        handler->old_value_output = get_value;
        handler->active_stat = CUR_STAT_ACTIVITY;
        handler->abnormal_flag = DC_DEV_ACTIVITY;
    }
    else
    {
        dcprintk("FSM : <%s> cur stat : 0x%x\n", handler->name, handler->active_stat);
        switch(handler->active_stat)
        {
            case CUR_STAT_INIT:
            case CUR_STAT_ACTIVITY:
                dcprintk("FSM : <%s> set CUR_STAT_DEACTIVITY\n", handler->name);
                handler->active_stat = CUR_STAT_DEACTIVITY;
                break;
            case CUR_STAT_DEACTIVITY:
                dcprintk("FSM : <%s> set CUR_STAT_ABNORMAL\n", handler->name);
                handler->active_stat = CUR_STAT_ABNORMAL;
                break;
            case CUR_STAT_ABNORMAL:
                dcprintk("FSM : <%s> set CUR_STAT_INIT\n", handler->name);
                handler->active_stat = CUR_STAT_INIT;
                break;
            default:
                break;
        }
    }
    
    if (CUR_STAT_ABNORMAL == handler->active_stat
        && DC_DEV_ACTIVITY == handler->abnormal_flag
        && ( DC_COUPLING_NONE == handler->coupling_flag 
        || (DC_COUPLING_NONE != handler->coupling_flag
        && DC_COUPLING_MASTER == handler->coupling_master
        && DC_COUPLING_MASTER_NONE == handler->coupling_slave)))
    {
        handler->active_stat = CUR_STAT_INIT;
        handler->abnormal_flag = DC_DEV_ABNORMAL;

        /* Send msg to exception */
        dcprintk("[%s]<%s> abnormal,Send Msg to Exception...\n", __FUNCTION__, handler->name);
        dc_send_abnormal2expetion(handler);
    }
    
    FUNC_END;
}

/*
 * Function:       static int dc_thread(void *data)
 * Description:    kernel thread
 * Input:          void *data  目前未用到，留接口
 * Output:         No
 * Return:         0:线程退出
 */
static int dc_thread(void *data)
{
    struct list_head *i;
    struct dc_handler *handler;
    int compare_ret;
    unsigned long current_time;
    
    init_waitqueue_head(&dc_waitq);
    
    while (1)
    {
        if (kthread_should_stop())
        {
            break;
        }
    
        current_time = jiffies;/* get current time */
        
        mutex_lock(&dc_mutex);
        
        if (list_empty(&dev_check_list))/* check chain is null or not */
        {
            goto null_list_schedule;
        }
        
        #ifdef DEV_CHECK_DEBUG
        list_for_each(i, &dev_check_list)
        {
            handler = list_entry(i, struct dc_handler, node);
            printk("Node name:%s\treal_time:%lu\n", handler->name, handler->real_time);
        }
        #endif
        
        list_for_each(i, &dev_check_list)
        {
            handler = list_entry(i, struct dc_handler, node);
            compare_ret = time_before_eq(handler->real_time, current_time);
            
            if (compare_ret)
            {
                dcprintk("%s handler.real_time lt current time %lu\n", handler->name, current_time);/* lt : litter than */
                dc_node_handler(handler);
                handler->real_time = current_time + handler->poll_time_jiffies;
            }
        }
        
        /* coupling handle */
        dc_coupling_node_handler();
        
        /*normal_sched:*/
        mutex_unlock(&dc_mutex);
        
        dcprintk("[%s]Schedule time:5 * HZ\n", __FUNCTION__);
        interruptible_sleep_on_timeout(&dc_waitq, msecs_to_jiffies(DC_SCHD_TIME));
        continue;
        
        null_list_schedule:
        mutex_unlock(&dc_mutex);
        interruptible_sleep_on(&dc_waitq);/* sleep but interruptible <->  wake up func : wake_up_interruptible()*/
        dcprintk("[%s]wake up by signal or process...\n", __FUNCTION__);
    }

    return 0;
}

/*
 * Function:       static inline int dc_init_proc(void)
 * Description:    Init proc file
 * Input:          No
 * Output:         No
 * Return:         0: success, other: fail
 */
static inline int dc_init_proc(void)
{
    int ret = 0;
    struct proc_dir_entry *dev_ctrl_file = NULL;
    
    dev_check_dir = proc_mkdir("dev_check", NULL);
    if (dev_check_dir == NULL)
    {
        ret = -ENOMEM;
        printk("DC_CHECK_ERR:Creat dev_check dir fail\n");		
        goto dev_check_dir_fail;
    }
    
    if (create_proc_read_entry("dev_info", 0755, dev_check_dir, dev_info_read_proc, NULL) == NULL)
    {
        ret  = -ENOMEM;
        printk("DC_CHECK_ERR:Creat dev_info file fail\n");
        goto dev_info_fail;
    }
    
    dev_ctrl_file = create_proc_entry("dev_ctrl", 0755, dev_check_dir);
    if (dev_ctrl_file == NULL)
    {
        ret = -ENOMEM;
        printk("DC_CHECK_ERR:Creat dev_ctrl file fail\n");
        goto dev_ctrl_fail;
    }
    
    dev_ctrl_file->read_proc = dev_ctrl_read_proc;
    dev_ctrl_file->write_proc = dev_ctrl_write_proc;
    
    printk("DC_CHECK_OK:Creat all proc file success!\n");
    return 0;
    
    dev_ctrl_fail:
    remove_proc_entry("dev_info", dev_check_dir);
    
    dev_info_fail:
    remove_proc_entry("dev_check", NULL);
    
    dev_check_dir_fail:
    
    return ret;
}

/*
 * Function:       static int __init init_dev_check(void)
 * Description:    module init function
 * Input:          No
 * Output:         No
 * Return:         0: success, other: fail
 */
static int __init init_dev_check(void)
{
    int ret = -1;
    ret = dc_init_proc();
    if (ret)
    {
        printk("DC_CHECK_ERR:init proc fail\n");
        return ret;
    }

    /* start a kernel thread */
    g_dc_thread = kthread_run(dc_thread, NULL, "kDevCheckd");
    
    printk("Start init_dev_check success\n");
    return 0;
}

/*
 * Function:       static void __exit exit_dev_check(void)
 * Description:    module exit function
 * Input:          No
 * Output:         No
 * Return:         No
 */
static void __exit exit_dev_check(void)
{
    remove_proc_entry("dev_info", dev_check_dir);
    remove_proc_entry("dev_ctrl", dev_check_dir);
    remove_proc_entry("dev_check", NULL);
    kthread_stop(g_dc_thread);
    
    printk("Exit exit_dev_check success\n");
}

early_initcall(init_dev_check);
module_exit(exit_dev_check);
MODULE_AUTHOR("Huawei");
MODULE_DESCRIPTION("Device check");
MODULE_LICENSE("Dual BSD/GPL");

