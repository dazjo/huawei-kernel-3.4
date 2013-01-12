/*
 * FileName:       dev_check.h
 * Author:         w00126555  Version: v1.0  Date: 2012-9-19
 * Description:    Header file for Device Check
 * Version:        v1.0
 * Function List:  1.void dc_register_handler(struct dc_handler *handler)
                   2.void dc_unregister_handler(struct dc_handler *handler)
 * History:        
 *     <author>   <time>    <version >   <desc>
 *     w00126555  09.19     v1.0         Init
 */

#ifndef __DEV_CHECK_H_
#define __DEV_CHECK_H_

struct dc_handler
{
    /* Device Init */
    char *name;/* Device Name */
    int time_val_secs;/* Interval time(second) of polling the device */
    int coupling_flag;/* coupling flag:the relation of the two devices */
    int coupling_master;/* master device of coupling*/
    int coupling_slave;/* slave device of coupling */
    
    /* Device Check Module: if i changed,o should be changed, if not, the device abnormal */
    /*      --------       */
    /*--i-->|device| --o-> */
    /*      --------       */
    unsigned long (*get_var_input) (void);/* Func Pointer: get_var_input */
    unsigned long (*get_var_output) (void);/* Func Pointer: get_var_output */
    
    /* Used in Module */
    int poll_time_secs;/* Polling time, get from time_val_secs */
    unsigned long poll_time_jiffies;/* transfer poll_time_secs to value of jiffies */
    unsigned long real_time;/* absolute time: real time + poll_time_jiffies */
    
    unsigned long old_value_input;/* save the old value of i node */
    int coupling_master_no_input;/* no input flag of master: 1: no input  0: normal */
    unsigned long old_value_output;/* save the old value of o node */
    
    int active_stat;/* current device state */
    int abnormal_flag;/* the device abnormal flag, set or clear in FSM */
    struct list_head node;
    struct hlist_node hnode;
};

/* 3 stage of time_val_secs : [0,7],[8,13]; [14,...]*/
#define TIME_VAL_LOW_MIN 0
#define TIME_VAL_LOW_MAX 7
#define TIME_VAL_LOW_POLL_TIME 5
#define TIME_VAL_MID_MIN 8
#define TIME_VAL_MID_MAX 13
#define TIME_VAL_MID_POLL_TIME 10
#define TIME_VAL_HIG_MIN 14
#define TIME_VAL_HIG_POLL_TIME 15



/* active_stat */
#define CUR_STAT_MASK 0x3/* mask */
#define CUR_STAT_INIT 0x0/* Init stat */
#define CUR_STAT_ACTIVITY 0x2/* Normal stat */
#define CUR_STAT_DEACTIVITY 0x3/* First Abnormal stat */
#define CUR_STAT_ABNORMAL 0x1/* Abnormal stat */

/* abnormal_flag */
#define DC_DEV_ABNORMAL 1/* the node stat is abnormal, and send message to exception node */
#define DC_DEV_ACTIVITY 0/* the node stat is not abnormal */

/* coupling_flag */
#define DC_COUPLING_SIZE 32/* MAX coupling device */
#define DC_COUPLING_MASK 0xffffffff/* coupling relation mask */
#define DC_COUPLING_NONE 0/* no coupling */
#define DC_COUPLING_TP_DISP 1/* coupling: TP and DISP */
#define DC_COUPLING_OTHER 2/* other coupling of devices */


/* coupling_master */
#define DC_COUPLING_MASTER 1/* coupling + master */
#define DC_COUPLING_MASTER_NONE 0/* coupling but not master */

/* coupling_slave */
#define DC_COUPLING_SLAVE 1/* coupling + slave */
#define DC_COUPLING_SLAVE_NONE 0/* coupling but not slave */

/* coupling_master_no_input */
#define DC_COUPLING_MASTER_NO_INPUT 1/* master node no input,so not judge slave node */
#define DC_COUPLING_MASTER_INPUT_OK 0/* master node input, need to judge slave node */

void dc_register_handler(struct dc_handler *handler);
void dc_unregister_handler(struct dc_handler *handler);

/*
* Send data to Exception node
*/
extern int android_logger_lv(void* module, int mask);

enum logidx 
{
    LOG_DC2EXCEPTION_IDX = 0,
};

#define DC2EXCEPTION_INFO 4
#define DC2EXCEPTION_ERR  6
#define EXCEPTION_NODE_STR "/dev/log/exception"

int dc_send_abnormal2expetion_logger_write(const enum logidx index,
                                                          const unsigned char prio,
                                                          const char __kernel * const tag,
                                                          const char __kernel * const fmt,
                                                          ...);

#define DC_PRINT2EXCEPTION(tags, args...) do { \
    dc_send_abnormal2expetion_logger_write(LOG_DC2EXCEPTION_IDX, DC2EXCEPTION_ERR, tags, args); \
} while (0)

#endif /* end of __DEV_CHECK_H_ */

