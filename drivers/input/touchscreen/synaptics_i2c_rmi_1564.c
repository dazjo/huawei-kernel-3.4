/* Optimize synaptics driver */

/* drivers/input/keyboard/synaptics_i2c_rmi.c
 *
 * Copyright (C) 2007 Google, Inc.
 * Copyright (C) 2008 Texas Instrument Inc.
 * Copyright (C) 2009 Synaptics, Inc.
 *
 * provides device files /dev/input/event#
 * for named device files, use udev
 * 2D sensors report ABS_X_FINGER(0), ABS_Y_FINGER(0) through ABS_X_FINGER(7), ABS_Y_FINGER(7)
 * NOTE: requires updated input.h, which should be included with this driver
 * 1D/Buttons report BTN_0 through BTN_0 + button_count
 * TODO: report REL_X, REL_Y for flick, BTN_TOUCH for tap (on 1D/0D; done for 2D)
 * TODO: check ioctl (EVIOCGABS) to query 2D max X & Y, 1D button count
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/synaptics_i2c_rmi_1564.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/earlysuspend.h>
#include <linux/device.h>
#include <mach/vreg.h>
#include <mach/gpio.h>

#include <linux/proc_fs.h>
#include <linux/touch_platform_config.h>

/* New add a head file */
#include <linux/hw_tp_config.h>

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <linux/hw_dev_dec.h>
#endif
#ifndef CONFIG_SYNAPTICS_UPDATE_RMI_TS_FIRMWARE
#define CONFIG_SYNAPTICS_UPDATE_RMI_TS_FIRMWARE
#endif
#ifdef CONFIG_SYNAPTICS_UPDATE_RMI_TS_FIRMWARE
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/namei.h>
#include <linux/vmalloc.h>
#endif
#define DEV_ATTR(_pre, _name, _mode) \
DEVICE_ATTR(_pre##_##_name,_mode,_pre##_##_name##_show,_pre##_##_name##_store)

#include <linux/kernel.h>
#include <asm/mach-types.h>
#include "linux/hardware_self_adapt.h"
#include <asm/system.h>

/* É¾³ý´Ë¶Î´úÂë */

/*
* TP_ID0 and TP_ID1: get moudle message .
* SYN_I2C_RETRY_TIMES: use i2c read times.
* SYN_F01DATA_BASEADDR: Addr that judge synaptics chip exist or not exist 
* syanptics_pr_version: get UI FW  version and store
*/
/* delete FILE_LEN, don't need to use*/

#define SYN_I2C_RETRY_TIMES 10

/* upgrade fw file path */
#define TP_FW_COB_FILE_NAME  "/tp/1294018.img"
#define TP_FW_FILE_NAME "/sdcard/update/synaptics.img"
/*syn_version init */
struct syn_version_config syn_version = 
{
	.syn_firmware_version = 0, 
	.syn_moudel_version = 0,  
	.syn_ic_name = 0,
};

/* É¾³ý´Ë¶Î´úÂë */

#define BTN_F30 BTN_0


/* Define to open debug_log / Undef to close debug_log */
//#define TS_RMI_DEBUG
#undef TS_RMI_DEBUG 
#ifdef TS_RMI_DEBUG
#define TS_DEBUG_RMI(fmt, args...) printk(KERN_INFO fmt, ##args)
#else
#define TS_DEBUG_RMI(fmt, args...)
#endif

#ifdef CONFIG_SYNAPTICS_UPDATE_RMI_TS_FIRMWARE
#define SYNAPITICS_DEBUG(fmt, args...) printk(KERN_DEBUG fmt, ##args)
static struct i2c_client *g_client = NULL;
static ssize_t update_firmware_show(struct kobject *kobj, struct kobj_attribute *attr,char *buf);
static ssize_t update_firmware_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

static int ts_firmware_file(void);
/*choose upgrade file name by the filename  */
static int i2c_update_firmware(struct i2c_client *client, char *filename);
/* delete */


/* change the firmware file authority 664 */
static struct kobj_attribute update_firmware_attribute = {
	.attr = {.name = "update_firmware", .mode = 0664},
	.show = update_firmware_show,
	.store = update_firmware_store,
};
#endif /* CONFIG_SYNAPTICS_UPDATE_RMI_TS_FIRMWARE */

/*use this to contrl the debug message*/
static int synaptics_debug_mask;
module_param_named(synaptics_debug, synaptics_debug_mask, int,
		S_IRUGO | S_IWUSR | S_IWGRP);

#define DBG_MASK(x...) do {\
	if (synaptics_debug_mask) \
		printk(KERN_DEBUG x);\
	} while (0)


static struct workqueue_struct *synaptics_wq;
	
static struct synaptics_rmi4 *g_ts = NULL;


/* Register: EGR_0 */
#define EGR_PINCH_REG		0
#define EGR_PINCH 		(1 << 6)
#define EGR_PRESS_REG 		0
#define EGR_PRESS 		(1 << 5)
#define EGR_FLICK_REG 		0
#define EGR_FLICK 		(1 << 4)
#define EGR_EARLY_TAP_REG	0
#define EGR_EARLY_TAP		(1 << 3)
#define EGR_DOUBLE_TAP_REG	0
#define EGR_DOUBLE_TAP		(1 << 2)
#define EGR_TAP_AND_HOLD_REG	0
#define EGR_TAP_AND_HOLD	(1 << 1)
#define EGR_SINGLE_TAP_REG	0
#define EGR_SINGLE_TAP		(1 << 0)
/* Register: EGR_1 */
#define EGR_PALM_DETECT_REG	1
#define EGR_PALM_DETECT		(1 << 0)




struct synaptics_function_descriptor {
	__u8 queryBase;
	__u8 commandBase;
	__u8 controlBase;
	__u8 dataBase;
	__u8 intSrc;
#define INTERRUPT_SOURCE_COUNT(x) (x & 7)

	__u8 functionNumber;
};
#define FD_ADDR_MAX 0xE9
#define FD_ADDR_MIN 0x05
#define FD_BYTE_COUNT 6


/* synaptics module name and id table*/
#define BYD     1
#define CMI     2
#define TRULY   3
#define TPK     4
#define LENSONE 5
#define OFILM   6
#define EELY    7
#define SUCCESS 8
/*Add new module info  */
#define MUTTO   9
#define JUNDA   10
#define BIEL    11
#define JDI     12


/* delete */
static char touch_info[50] = {0};

static int RMI4_enable_program(struct i2c_client *client);
int RMI4_disable_program(struct i2c_client *client);
static struct synaptics_function_descriptor fd_34;

static struct synaptics_function_descriptor fd_01;
static struct i2c_msg query_i2c_msg_name[2];
static __u8 query_name[8];

static int ts_x_max = 0;
static int ts_y_max = 0;
static int lcd_x = 0;
static int lcd_y = 0;
static int lcd_all = 0;
static __u8 point_supported_huawei = 0;





/* define in platform/board file(s) */
extern struct i2c_device_id synaptics_rmi4_id[];

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_rmi4_early_suspend(struct early_suspend *h);
static void synaptics_rmi4_late_resume(struct early_suspend *h);
#endif

/*check the scope of X  axes*/
u12 check_scope_X(u12 x)
{
	u12 temp = x;
	if (x >= lcd_x -1)
	{
		temp = lcd_x -2;
	}
	if (x <= 1)
	{
		temp = 1;
	}

	return temp;
}

static int synaptics_rmi4_read_pdt(struct synaptics_rmi4 *ts)
{
	int ret = 0;
	int nFd = 0;
	int interruptCount = 0;
	__u8 data_length = 0;

	struct i2c_msg fd_i2c_msg[2];
	__u8 fd_reg;
	struct synaptics_function_descriptor fd;

	struct i2c_msg query_i2c_msg[2];
	__u8 query[14];
	__u8 *egr;
    /*check if rmi page is 0*/
    ret = i2c_smbus_write_byte_data(ts->client, 0xff, 0x00);
    if(ret < 0)
    {
        printk(KERN_ERR "failed to set rmi page\n");
    }
    else
    {
        printk("set rmi page to zero successfull\n");
    }
	fd_i2c_msg[0].addr = ts->client->addr;
	fd_i2c_msg[0].flags = 0;
	fd_i2c_msg[0].buf = &fd_reg;
	fd_i2c_msg[0].len = 1;

	fd_i2c_msg[1].addr = ts->client->addr;
	fd_i2c_msg[1].flags = I2C_M_RD;
	fd_i2c_msg[1].buf = (__u8 *)(&fd);
	fd_i2c_msg[1].len = FD_BYTE_COUNT;

	query_i2c_msg[0].addr = ts->client->addr;
	query_i2c_msg[0].flags = 0;
	query_i2c_msg[0].buf = &fd.queryBase;
	query_i2c_msg[0].len = 1;

	query_i2c_msg[1].addr = ts->client->addr;
	query_i2c_msg[1].flags = I2C_M_RD;
	query_i2c_msg[1].buf = query;
	query_i2c_msg[1].len = sizeof(query);


	ts->hasF11 = false;
	ts->hasF19 = false;
	ts->hasF30 = false;
	ts->data_reg = 0xff;
	ts->data_length = 0;


	for (fd_reg = FD_ADDR_MAX; fd_reg >= FD_ADDR_MIN; fd_reg -= FD_BYTE_COUNT)     
    {
		ret = i2c_transfer(ts->client->adapter, fd_i2c_msg, 2);
		if (ret < 0) 
		{
			printk(KERN_ERR "I2C read failed querying RMI4 $%02X capabilities\n", ts->client->addr);
			return ret;
		}
		if (!fd.functionNumber) 
        {
			/* End of PDT */
			ret = nFd;
			TS_DEBUG_RMI("Read %d functions from PDT\n", fd.functionNumber);
			break;
		}

		++nFd;

		switch (fd.functionNumber) 
        {
            case 0x34:
                fd_34.queryBase = fd.queryBase;
                fd_34.dataBase = fd.dataBase;
                fd_34.controlBase = fd.controlBase;
                break;
			case 0x01: /* Interrupt */
				ts->f01.data_offset = fd.dataBase;
                fd_01.queryBase = fd.queryBase;
                fd_01.dataBase = fd.dataBase;
                fd_01.commandBase = fd.commandBase;
                fd_01.controlBase = fd.controlBase;
				/*
				 * Can't determine data_length
				 * until whole PDT has been read to count interrupt sources
				 * and calculate number of interrupt status registers.
				 * Setting to 0 safely "ignores" for now.
				 */
				data_length = 0;
				break;
			case 0x11: /* 2D */
				ts->hasF11 = true;

				ts->f11.data_offset = fd.dataBase;
				ts->f11.interrupt_offset = interruptCount / 8;
				ts->f11.interrupt_mask = ((1 << INTERRUPT_SOURCE_COUNT(fd.intSrc)) - 1) << (interruptCount % 8);

				ret = i2c_transfer(ts->client->adapter, query_i2c_msg, 2);
				if (ret < 0)
					printk(KERN_ERR "Error reading F11 query registers\n");

				ts->f11.points_supported = (query[1] & 7) + 1;
				if (ts->f11.points_supported == 6)
					ts->f11.points_supported = 10;

				ts->f11_fingers = kcalloc(ts->f11.points_supported,
				                          sizeof(*ts->f11_fingers), 0);

				TS_DEBUG_RMI("%d fingers\n", ts->f11.points_supported);

				ts->f11_has_gestures = (query[1] >> 5) & 1;
				ts->f11_has_relative = (query[1] >> 3) & 1;
                ts->f11_has_Sensitivity_Adjust = (query[1] >> 6) & 1;
				egr = &query[7];

 
				TS_DEBUG_RMI("EGR features:\n");
				ts->hasEgrPinch = egr[EGR_PINCH_REG] & EGR_PINCH;
				TS_DEBUG_RMI("\tpinch: %u\n", ts->hasEgrPinch);
				ts->hasEgrPress = egr[EGR_PRESS_REG] & EGR_PRESS;
				TS_DEBUG_RMI("\tpress: %u\n", ts->hasEgrPress);
				ts->hasEgrFlick = egr[EGR_FLICK_REG] & EGR_FLICK;
				TS_DEBUG_RMI("\tflick: %u\n", ts->hasEgrFlick);
				ts->hasEgrEarlyTap = egr[EGR_EARLY_TAP_REG] & EGR_EARLY_TAP;
				TS_DEBUG_RMI("\tearly tap: %u\n", ts->hasEgrEarlyTap);
				ts->hasEgrDoubleTap = egr[EGR_DOUBLE_TAP_REG] & EGR_DOUBLE_TAP;
				TS_DEBUG_RMI("\tdouble tap: %u\n", ts->hasEgrDoubleTap);
				ts->hasEgrTapAndHold = egr[EGR_TAP_AND_HOLD_REG] & EGR_TAP_AND_HOLD;
				TS_DEBUG_RMI("\ttap and hold: %u\n", ts->hasEgrTapAndHold);
				ts->hasEgrSingleTap = egr[EGR_SINGLE_TAP_REG] & EGR_SINGLE_TAP;
				TS_DEBUG_RMI("\tsingle tap: %u\n", ts->hasEgrSingleTap);
				ts->hasEgrPalmDetect = egr[EGR_PALM_DETECT_REG] & EGR_PALM_DETECT;
				TS_DEBUG_RMI("\tpalm detect: %u\n", ts->hasEgrPalmDetect);


				query_i2c_msg[0].buf = &fd.controlBase;
				ret = i2c_transfer(ts->client->adapter, query_i2c_msg, 2);
				if (ret < 0)
					printk(KERN_ERR "Error reading F11 control registers\n");

				query_i2c_msg[0].buf = &fd.queryBase;

				ts->f11_max_x = ((query[7] & 0x0f) * 0x100) | query[6];
				ts->f11_max_y = ((query[9] & 0x0f) * 0x100) | query[8];

				TS_DEBUG_RMI("max X: %d; max Y: %d\n", ts->f11_max_x, ts->f11_max_y);

				ts->f11.data_length = data_length =
					/* finger status, four fingers per register */
					((ts->f11.points_supported + 3) / 4)
					/* absolute data, 5 per finger */
					+ 5 * ts->f11.points_supported
					/* two relative registers */
					+ (ts->f11_has_relative ? 2 : 0)
					/* F11_2D_Data8 is only present if the egr_0 register is non-zero. */
					+ (egr[0] ? 1 : 0)
					/* F11_2D_Data9 is only present if either egr_0 or egr_1 registers are non-zero. */
					+ ((egr[0] || egr[1]) ? 1 : 0)
					/* F11_2D_Data10 is only present if EGR_PINCH or EGR_FLICK of egr_0 reports as 1. */
					+ ((ts->hasEgrPinch || ts->hasEgrFlick) ? 1 : 0)
					/* F11_2D_Data11 and F11_2D_Data12 are only present if EGR_FLICK of egr_0 reports as 1. */
					+ (ts->hasEgrFlick ? 2 : 0)
					;

				break;
 			case 0x30: /* GPIO */
				ts->hasF30 = true;

				ts->f30.data_offset = fd.dataBase;
				ts->f30.interrupt_offset = interruptCount / 8;
				ts->f30.interrupt_mask = ((1 << INTERRUPT_SOURCE_COUNT(fd.intSrc)) - 1) << (interruptCount % 8);

				ret = i2c_transfer(ts->client->adapter, query_i2c_msg, 2);
				if (ret < 0)
					printk(KERN_ERR "Error reading F30 query registers\n");


				ts->f30.points_supported = query[1] & 0x1F;
				ts->f30.data_length = data_length = (ts->f30.points_supported + 7) / 8;

				break;
			default:
				goto pdt_next_iter;
		}

		/* Change to end address for comparison
		  NOTE: make sure final value of ts->data_reg is subtracted */
		data_length += fd.dataBase;
		if (data_length > ts->data_length) 
        {
			ts->data_length = data_length;
		}

		if (fd.dataBase < ts->data_reg) 
        {
			ts->data_reg = fd.dataBase;
		}

pdt_next_iter:
		interruptCount += INTERRUPT_SOURCE_COUNT(fd.intSrc);
	}

	/* Now that PDT has been read, interrupt count determined, F01 data length can be determined.*/
	ts->f01.data_length = data_length = 1 + ((interruptCount + 7) / 8);
	/* Change to end address for comparison
	NOTE: make sure final value of ts->data_reg is subtracted*/
	data_length += ts->f01.data_offset;
	if (data_length > ts->data_length) 
    {
		ts->data_length = data_length;
	}

	/*Change data_length back from end address to length*/
	/*NOTE: make sure this was an address*/
	ts->data_length -= ts->data_reg;
    // I want to read the register from F01_data_reg
    ts->data_reg = ts->f01.data_offset; 
    //only need to read F01 data and F11 data per interrupt
    ts->data_length = (ts->f01.data_length) + (ts->f11.data_length);
	/*Change all data offsets to be relative to first register read */
 	ts->f01.data_offset -= ts->data_reg;
	ts->f11.data_offset -= ts->data_reg;
	ts->f19.data_offset -= ts->data_reg;
	ts->f30.data_offset -= ts->data_reg;

	ts->data = kcalloc(ts->data_length, sizeof(*ts->data), 0);
	if (ts->data == NULL) 
    {
		printk(KERN_ERR "Not enough memory to allocate space for RMI4 data\n");
		ret = -ENOMEM;
	}

	ts->data_i2c_msg[0].addr = ts->client->addr;
	ts->data_i2c_msg[0].flags = 0;
	ts->data_i2c_msg[0].len = 1;
	ts->data_i2c_msg[0].buf = &ts->data_reg;

	ts->data_i2c_msg[1].addr = ts->client->addr;
	ts->data_i2c_msg[1].flags = I2C_M_RD;
	ts->data_i2c_msg[1].len = ts->data_length;
	ts->data_i2c_msg[1].buf = ts->data;

	printk(KERN_ERR "RMI4 $%02X data read: $%02X + %d\n",
        	ts->client->addr, ts->data_reg, ts->data_length);

	return ret;
}

static void synaptics_rmi4_work_func(struct work_struct *work)
{
	int ret;
    __u8 finger_status = 0x00;
    
    __u8 reg = 0;
    __u8 *finger_reg = NULL;
    u12 x = 0;
    u12 y = 0;
    u4 wx = 0;
    u4 wy = 0;
    u8 z = 0 ;
	u8 finger_pressed_count = 0;
/* Add new variable */
	__u8 prev_state = 0;


	__u8 *interrupt = NULL;
	struct synaptics_rmi4 *ts = NULL;
	ts = container_of(work, struct synaptics_rmi4, work);

	ret = i2c_transfer(ts->client->adapter, ts->data_i2c_msg, 2);

	if (ret < 0) 
    {
		printk(KERN_ERR "%s: i2c_transfer failed\n", __func__);
	}
    else /* else with "i2c_transfer's return value"*/
	{
		interrupt = &ts->data[ts->f01.data_offset + 1];
		if (ts->hasF11 && interrupt[ts->f11.interrupt_offset] & ts->f11.interrupt_mask) 
        {
			__u8 *f11_data = NULL;
            int f = 0;
			__u8 finger_status_reg = 0;
			__u8 fsr_len = 0;
			
            f11_data = &ts->data[ts->f11.data_offset];

			fsr_len = (ts->f11.points_supported + 3) / 4;
            TS_DEBUG_RMI("f11.points_supported is %d\n",ts->f11.points_supported);
            if(ts->is_support_multi_touch)
            {
                for (f = 0; f < point_supported_huawei; ++f)
                {

                	if (!(f % 4))
                        
                	finger_status_reg = f11_data[f / 4];

                	finger_status = (finger_status_reg >> ((f % 4) * 2)) & 3;

                	reg = fsr_len + 5 * f;
                	finger_reg = &f11_data[reg];

                	x = (finger_reg[0] * 0x10) | (finger_reg[2] % 0x10);
                	y = (finger_reg[1] * 0x10) | (finger_reg[2] / 0x10);
                	wx = finger_reg[3] % 0x10;
                	wy = finger_reg[3] / 0x10;
                	z = finger_reg[4];

                    if(machine_is_msm7x30_u8820())
                    {
					    /* don't read the interrupt again  */
                        if(interrupt[ts->f11.interrupt_offset] & 0x02)
                        {
                            ret = i2c_smbus_read_byte_data(ts->client, fd_01.dataBase);
                            if(ret & 0x03)
                            {
                                
                                ret = i2c_smbus_write_byte_data(ts->client, fd_01.commandBase, 0x01);
                                printk("the touchscreen is reset yet!\n");
                            }
                        }
                    }
                    x = x * lcd_x / ts_x_max;
					/*Coordinates is the opposite in S2000 IC for U8661*/
					if (machine_is_msm7x27a_U8661())
                    {
                        y = ((ts_y_max - y) * lcd_all ) / ts_y_max;
                   	}
                    else
                    {
                        y = ( y * lcd_all ) / ts_y_max;
                    }
					/*check the scope of X  axes*/
                    x = check_scope_X(x);

                    DBG_MASK("the x is %d the y is %d the stauts is %d!\n",x,y,finger_status);
                	/* Linux 2.6.31 multi-touch */
					/* Modify the tp_report information to adapt the framework change */
					prev_state = ts->f11_fingers[f].status;

					if (prev_state && (!finger_status))
					{
						/* this is a release */
						z = wx = wy = 0;
					}
					else if ((!prev_state) && (!finger_status)) 
					{
						/* nothing to report */
						continue;
					}

                    input_report_abs(ts->input_dev, ABS_MT_PRESSURE, z);
                	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, max(wx, wy));
					input_report_abs(ts->input_dev, ABS_MT_TOUCH_MINOR, min(wx, wy));
					input_report_abs(ts->input_dev, ABS_MT_ORIENTATION, (wx > wy ? 1 : 0));
					input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
                	input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
					input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, f);

                    input_mt_sync(ts->input_dev);  

                    ts->f11_fingers[f].status = finger_status;
					if (finger_status > 0)
						finger_pressed_count++;
                                        
                    
                }
				/* Report if there is any fingure on the TP */
				input_report_key(ts->input_dev, BTN_TOUCH, finger_pressed_count);
            }
            else /* else with "if(ts->is_support_multi_touch)"*/
            {
    			finger_status_reg = f11_data[0];
                finger_status = (finger_status_reg & 3);
                TS_DEBUG_RMI("the finger_status is %2d!\n",finger_status);
          
                reg = fsr_len;
                finger_reg = &f11_data[reg];
                x = (finger_reg[0] * 0x10) | (finger_reg[2] % 0x10);
                y = (finger_reg[1] * 0x10) | (finger_reg[2] / 0x10);
				wx = finger_reg[3] % 0x10;
				wy = finger_reg[3] / 0x10;
				z = finger_reg[4];

                x = x * lcd_x / ts_x_max;
                y = y * lcd_all / ts_y_max;
				/*check the scope of X  axes*/
                x = check_scope_X(x);

                TS_DEBUG_RMI(KERN_ERR "the x_sig is %2d ,the y_sig is %2d \n",x, y);

                input_report_abs(ts->input_dev, ABS_X, x);
				input_report_abs(ts->input_dev, ABS_Y, y);

				input_report_abs(ts->input_dev, ABS_PRESSURE, z);
                input_report_key(ts->input_dev, BTN_TOUCH, finger_status);
                input_sync(ts->input_dev);
            
            }


        }

		if (ts->hasF19 && interrupt[ts->f19.interrupt_offset] & ts->f19.interrupt_mask) 
        {
			int reg;
			int touch = 0;
			for (reg = 0; reg < ((ts->f19.points_supported + 7) / 8); reg++)
			{
				if (ts->data[ts->f19.data_offset + reg]) 
                {
					touch = 1;
				   	break;
				}
			}
			input_report_key(ts->input_dev, BTN_DEAD, touch);

		}
    	input_sync(ts->input_dev);
	}
	if (ts->use_irq)
	{
       enable_irq(ts->client->irq);
    }
    
/* É¾³ý´Ë¶Î´úÂë */

}

static enum hrtimer_restart synaptics_rmi4_timer_func(struct hrtimer *timer)
{
	struct synaptics_rmi4 *ts = container_of(timer, \
					struct synaptics_rmi4, timer);

	queue_work(synaptics_wq, &ts->work);

	hrtimer_start(&ts->timer, ktime_set(0, 12 * NSEC_PER_MSEC), HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}

irqreturn_t synaptics_rmi4_irq_handler(int irq, void *dev_id)
{
	struct synaptics_rmi4 *ts = dev_id;

	disable_irq_nosync(ts->client->irq);
	
/* É¾³ý´Ë¶Î´úÂë */
	
	queue_work(synaptics_wq, &ts->work);

	return IRQ_HANDLED;
}

static void synaptics_rmi4_enable(struct synaptics_rmi4 *ts)
{  
	if (ts->use_irq)
		enable_irq(ts->client->irq);
	else
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	ts->enable = 1;
}

static void synaptics_rmi4_disable(struct synaptics_rmi4 *ts)
{

	if (ts->use_irq)
		disable_irq_nosync(ts->client->irq);
	else
		hrtimer_cancel(&ts->timer);

	cancel_work_sync(&ts->work);

	ts->enable = 0;
}

static ssize_t synaptics_rmi4_enable_show(struct device *dev,
                                         struct device_attribute *attr, char *buf)
{
	struct synaptics_rmi4 *ts = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", ts->enable);
}

static ssize_t synaptics_rmi4_enable_store(struct device *dev,
                                          struct device_attribute *attr,
                                          const char *buf, size_t count)
{
	struct synaptics_rmi4 *ts = dev_get_drvdata(dev);
	unsigned long val;
	int error;

	error = strict_strtoul(buf, 10, &val);

	if (error)
		return error;

	val = !!val;

	if (val != ts->enable) 
    {
		if (val)
			synaptics_rmi4_enable(ts);
		else
			synaptics_rmi4_disable(ts);
	}

	return count;
}

DEV_ATTR(synaptics_rmi4, enable, 0664);
static char * get_touch_module_name(u8 module_id)
{
	switch(module_id)
	{
		case BYD:
			return "BYD";
		case CMI:
			return "CMI";
		case TRULY:
			return "TRULY";
		case TPK:
			return "TPK";
		case LENSONE:
			return "LENSONE";
		case OFILM:
			return "OFILM";
		case EELY:
			return "EELY";
		case SUCCESS:
			return "SUCCESS";
		/*show module info */			
		case MUTTO:
			return "MUTTO";
		case JUNDA:
			return "JUNDA";
		case BIEL:
			return "BIEL";
		case JDI:
			return "JDI";
		default:
			return "unknow";
	}

	return NULL;
}
/* named Rule
 * 2202 3200 : syanptics-IC-Module.ver
 * for example: syanptics-3200-tpk.2
 *
 * 2000 2100 3000 :syanptics-Module.ver
 * for example: syanptics-tpk.2
 */
char * get_synaptics_touch_info(void)
{
	u32 config_id = 0;
	char * module_name = NULL;
	
	/* return NULL if client is unknow */
	if (g_client == NULL)
		return NULL;


	module_name = get_touch_module_name(query_name[2]);
	if (module_name == NULL)
	{
		return NULL;
	}
	if (IC_TYPE_2202 == syn_version.syn_ic_name)
	{
		config_id = query_name[3];
		sprintf(touch_info,"synaptics-2202-%s.%d",module_name,config_id);		
	}
	else if (IC_TYPE_3200 == syn_version.syn_ic_name)
	{
		config_id = query_name[3];
		sprintf(touch_info,"synaptics-3200-%s.%d",module_name,config_id);	
	}
	else
	{
		config_id = query_name[3];
		sprintf(touch_info,"synaptics-%s.%d",module_name,config_id);	
	}

	return touch_info;
}

static void get_ic_name(void)
{
    struct i2c_msg msg[2];
    uint8_t ic_name_buffer[2] = {0};
    int ret = 0;
    u8 addr = fd_01.queryBase+17;

    msg[0].addr = g_ts->client->addr;
	msg[0].flags = 0;
	msg[0].buf = &addr;
	msg[0].len = 1;

	msg[1].addr = g_ts->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = ic_name_buffer;
	msg[1].len = sizeof(ic_name_buffer);

    ret = i2c_transfer(g_ts->client->adapter, msg, 2);
	if (ret < 0)
    {
		printk("Failed to read IC name.\n");
        return;
	}
    syn_version.syn_ic_name = ic_name_buffer[1] * 0x100 + ic_name_buffer[0];
}
static u8 get_module_id(void)
{
	struct i2c_msg msg[2];
	char productid[11];
	int ret ;
	unsigned long module_id = 0;
	u8 querybase = 0;
	
	ret = RMI4_enable_program(g_ts->client);
    if( ret != 0)
	{
		printk("%s:%d:RMI enable program error,return...\n",__FUNCTION__,__LINE__);
		goto get_module_id_error;
	}
	querybase = fd_01.queryBase + 11;

    msg[0].addr = g_ts->client->addr;
	msg[0].flags = 0;
	msg[0].buf = &querybase;
	msg[0].len = 1;

	msg[1].addr = g_ts->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = productid;
	msg[1].len = 10;

	ret = i2c_transfer(g_ts->client->adapter, msg, 2);
	if (ret < 0) 
	{
		printk(KERN_ERR "%s: i2c_transfer failed\n", __func__);
        goto get_module_id_error;
	}
		
	productid[10] = '\0';
	ret = strict_strtoul(&productid[9], 10, &module_id);
	if (ret)
	{
		pr_err("%s : transfer error\n",__func__);
        goto get_module_id_error;
	}

	RMI4_disable_program(g_ts->client);
	return (u8)module_id;

get_module_id_error:
    RMI4_disable_program(g_ts->client);
    return -1;
}

static u8 get_config_version(void)
{
	struct i2c_msg msg[2];
	char configver[5];
	int ret ;
	unsigned long config_ver = 0;
					
	msg[0].addr = g_ts->client->addr;
	msg[0].flags = 0;
	msg[0].buf = &fd_34.controlBase;
	msg[0].len = 1;

	msg[1].addr = g_ts->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = configver;
	msg[1].len = 4;

	ret = i2c_transfer(g_ts->client->adapter, msg, 2);
	if (ret < 0) 
	{
		printk(KERN_ERR "%s: i2c_transfer failed\n", __func__);
		return -1;
	}

	configver[4] = '\0';
	ret = strict_strtoul(configver, 10, &config_ver);
	if (ret < 0) 
	{
		pr_err("%s : transfer fail\n",__func__);
		return -1;
	}

	return (u8)config_ver;
}
/* same as in proc_misc.c */
static int
proc_calc_metrics(char *page, char **start, off_t off, int count, int *eof, int len)
{
	if (len <= off + count)
		*eof = 1;
	*start = page + off;
	len -= off;
	if (len > count)
		len = count;
	if (len < 0)
		len = 0;
	return len;
}
static void tp_read_fn34_input_name(void)
{
	/* set random number for query_name[0] and query_name[1] because we don't have the real value */
    query_name[0] = 1;
    query_name[1] = 1;
    query_name[2] = get_module_id();
    query_name[3] = get_config_version();
}
static int tp_read_input_name(void)
{
    int ret;
    query_i2c_msg_name[0].addr = g_ts->client->addr;
	query_i2c_msg_name[0].flags = 0;
	query_i2c_msg_name[0].buf = &fd_01.queryBase;
	query_i2c_msg_name[0].len = 1;

	query_i2c_msg_name[1].addr = g_ts->client->addr;
	query_i2c_msg_name[1].flags = I2C_M_RD;
	query_i2c_msg_name[1].buf = query_name;
	query_i2c_msg_name[1].len = sizeof(query_name);
    
    ret = i2c_transfer(g_ts->client->adapter, query_i2c_msg_name, 2);
	if (ret < 0) 
    {
		printk(KERN_ERR "%s: i2c_transfer failed\n", __func__);
	}
	
    return ret;

}
static int tp_read_proc(
	char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;
	len = snprintf(page, PAGE_SIZE, "TP_TYPE:"
	"%s\n"
	"Manufacturer ID:"
	"%x\n"
    " Product Properties:"
	"%x\n"
	"Customer Family:%x\n"
	"Firmware Revision:%x\n",
	"synapitcs",query_name[0], query_name[1], query_name[2], query_name[3]);

    return proc_calc_metrics(page, start, off, count, eof, len);
    
}

/* moved here, In order to better function calls*/
struct RMI4_FDT
{
	unsigned char m_QueryBase;
	unsigned char m_CommandBase;
	unsigned char m_ControlBase;
	unsigned char m_DataBase;
	unsigned char m_IntSourceCount;
	unsigned char m_ID;
};

static int RMI4_read_PDT(struct i2c_client *client)
{
	// Read config data
	struct RMI4_FDT temp_buf;
	struct RMI4_FDT m_PdtF34Flash;
	struct RMI4_FDT m_PdtF01Common;
	struct i2c_msg msg[2];
	unsigned short start_addr; 
	int ret = 0;

	memset(&m_PdtF34Flash,0,sizeof(struct RMI4_FDT));
	memset(&m_PdtF01Common,0,sizeof(struct RMI4_FDT));

	for(start_addr = 0xe9; start_addr > 10; start_addr -= sizeof(struct RMI4_FDT))
	{
		msg[0].addr = client->addr;
		msg[0].flags = 0;
		msg[0].len = 1;
		msg[0].buf = (unsigned char *)&start_addr;
		msg[1].addr = client->addr;
		msg[1].flags = I2C_M_RD;
		msg[1].len = sizeof(struct RMI4_FDT);
		msg[1].buf = (unsigned char *)&temp_buf;
		
		if(i2c_transfer(client->adapter, msg, 2) < 0)
		{
			printk("%s:%d: read RIM4 PDT error!\n", __FUNCTION__, __LINE__);
			return -1;
		}

		if(temp_buf.m_ID == 0x34)
		{
			memcpy(&m_PdtF34Flash,&temp_buf,sizeof(struct RMI4_FDT ));
		}
		else if(temp_buf.m_ID == 0x01)
		{
			memcpy(&m_PdtF01Common,&temp_buf,sizeof(struct RMI4_FDT ));
		}
		else if (temp_buf.m_ID == 0)  //end of PDT
		{		
			break;
		}
	}

	if((m_PdtF01Common.m_CommandBase != fd_01.commandBase) || (m_PdtF34Flash.m_QueryBase != fd_34.queryBase))
	{
		printk("%s:%d: RIM4 PDT has changed!!!\n",__FUNCTION__,__LINE__);
		
		ret = synaptics_rmi4_read_pdt(g_ts);
		if(ret < 0)
		{
			printk("read pdt error:!\n");
			return -1;
		}
		
		return 0;
	}

	return 0;

}

//to be improved .......
int RMI4_wait_attn(struct i2c_client * client,int udleay)
{
	int loop_count=0;
	int ret=0;

	do{
		mdelay(udleay);
		
		ret = i2c_smbus_read_byte_data(client,fd_34.dataBase+18);//read Flash Control
		//printk("RMI4_wait_attn is %d \n",(fd_34.dataBase+18));
		/* Clear the attention assertion by reading the interrupt status register */
		i2c_smbus_read_byte_data(client,fd_01.dataBase+1);//read the irq Interrupt Status
	}while(loop_count++ < 0x10 && (ret != 0x80));

	if(loop_count >= 0x10)
	{
		SYNAPITICS_DEBUG("RMI4 wait attn timeout:ret=0x%x\n",ret);
		return -1;
	}
	return 0;
}

/* i2c read function by use synaptics IC*/
static int i2c_syn_read(struct i2c_client *client, uint16_t addr, uint8_t *data, uint16_t length)
{
	uint8_t retry = 0;
	uint8_t buf = 0;

	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0, 
			.len = 1,
			.buf = &buf,
		},
		{
			.addr = client->addr,
			.flags = I2C_M_RD, 
			.len = length,
			.buf = data,
		}
	};
	buf = addr & 0xFF; 

	for (retry = 0; retry < SYN_I2C_RETRY_TIMES; retry++) {
		if (i2c_transfer(client->adapter, msg, 2) == 2)
			break;
		msleep(10);
	}

	if (retry == SYN_I2C_RETRY_TIMES) {
		printk(KERN_INFO "[TP] i2c_read retry over %d\n",
			SYN_I2C_RETRY_TIMES);
		return -EIO;
	}
	return 0;
}

/* i2c write function by use synaptics IC */
static int i2c_syn_write(struct i2c_client *client, uint16_t addr, uint8_t *data, uint16_t length)
{
	uint8_t retry = 0;
	uint8_t buf[length + 1];

	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = length + 1,
			.buf = buf,
		}
	};

	buf[0] = addr & 0xFF;
	memcpy(&buf[1], &data[0], length);

	for (retry = 0; retry < SYN_I2C_RETRY_TIMES; retry++) {
		if (i2c_transfer(client->adapter, msg, 1) == 1)
			break;
		mdelay(10);
	}

	if (retry == SYN_I2C_RETRY_TIMES) {
		printk(KERN_ERR "[TP] i2c_write retry over %d\n",
			SYN_I2C_RETRY_TIMES);
		return -EIO;
	}

	return 0;
}

/*get UI FW version and  config version*/
static int syn_get_version(void)
{
    uint8_t data[4] = {0};
    int ret = 0;
    
	ret = i2c_syn_read(g_ts->client,fd_01.queryBase+ 18, data, 3);
	if (ret < 0)
	{
		printk("Failed to read FW_PR_NUM.\n");
		goto err_handle_fail;
    }
	syn_version.syn_firmware_version = data[2] << 16 | data[1] << 8 | data[0];
	printk(KERN_INFO "[TP] %s: packrat_number: %d\n", __func__, syn_version.syn_firmware_version);

    ret = i2c_syn_read(g_ts->client, fd_34.controlBase,data, 4);
	if (ret < 0)
	{
		printk("Failed to read FW_PR_NUM.\n");
		goto err_handle_fail;
    }	
	syn_version.syn_moudel_version = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
	printk(KERN_INFO "[TP] %s: config version: %x\n", __func__, syn_version.syn_moudel_version);

	return 0;
err_handle_fail:
   		ret = -EIO;
	return ret;
}

/*get value of the config data, use to confirm which module show */
static int tp_show_module_info(void)
{
    uint8_t data[5] = {0};
    int ret = 0;

	ret = synaptics_rmi4_read_pdt(g_ts);
	if (ret <= 0) 
	{
		if (ret == 0)
			printk(KERN_ERR "Empty PDT\n");
			
		printk(KERN_ERR "Error identifying device (%d)\n", ret);
		return -1;
	}

	ret = i2c_syn_read(g_ts->client, fd_34.controlBase,data, 4);
	if (ret < 0)
	{
		printk("Failed to read config_info.\n");
		return -1;		
	}

	data[4] = '\0';
	query_name[0] = 1;
	query_name[1] = 1;
	query_name[2] = data[2];
	query_name[3] = data[3];
	return 0;
	
}
/*synaptics crc Algorithm */
static uint32_t syn_crc(uint16_t *data, uint16_t len)
{
	
	//len: 32*12/2-2
	uint32_t sum1, sum2;
	sum1 = sum2 = 0xFFFF;

	if(NULL == data)
	{
		printk(KERN_INFO "syn_crc: data is null\n");
		return -1;
	}
	
	while (len--) {
	//date: 0x00
		sum1 += *data++; //0xFFFF+0x00
		sum2 += sum1;//0xFFFF+0xFFFF
		sum1 = (sum1 & 0xFFFF) + (sum1 >> 16);
		sum2 = (sum2 & 0xFFFF) + (sum2 >> 16);
/*		printk("Data: %x, Sum1: %x, Sum2: %x\n", *data, sum1, sum2); */
	}
	return sum1 | (sum2 << 16);
}

/* loacl crc value compare IC crc value*/
static int crc_comparison(struct synaptics_rmi4 *ts, uint32_t config_crc)
{
	int ret = 0;
	uint8_t data[17] = {0};
	uint32_t flash_crc = 0;

	data[0] = 0x1F;
	data[1] = 0x00;

	if (NULL == ts)
	{
		printk(KERN_INFO "crc_comparison: ts is null\n");
		goto err_handle_fail;
	}

	ret = i2c_syn_write(ts->client,fd_34.dataBase, data, 2);
	if (ret < 0)
		goto err_handle_fail; 
		
	ret = i2c_smbus_write_byte_data(ts->client,fd_34.dataBase + 18,0x05);
	if (ret < 0)
		goto err_handle_fail;

	ret = RMI4_wait_attn(ts->client,12);
	if (ret < 0)
		goto err_handle_fail;
	
	ret = i2c_syn_read(ts->client,fd_34.dataBase + 2,data,17);
	if (ret < 0)
		goto err_handle_fail;

	memcpy(&flash_crc, &data[12], 4);

    printk("flash_crc = %X \n",flash_crc);
	if (flash_crc == config_crc)
	{
		ret = 0;
	}
	else
	{
		ret = 1;
	}
	return ret;
	
err_handle_fail:
   	ret = -EIO;	
   	return ret;
}

/*upgrade moudule config  */
static int program_config(struct synaptics_rmi4 *ts, uint8_t *config)
{
	int ret = 0;
	uint8_t data[19] = {0};
	uint16_t i = 0;

    if (NULL == ts)
	{
		printk(KERN_INFO "program_config: ts is null\n");
		goto err_handle_fail;
	}
	
	ret = i2c_syn_read(ts->client, fd_34.queryBase,data,2);
	if (ret < 0)
		goto err_handle_fail;

	ret = i2c_syn_write(ts->client,fd_34.dataBase + 2,data,2);
	if (ret < 0)
		goto err_handle_fail;

	ret = i2c_smbus_write_byte_data(ts->client,fd_34.dataBase + 18,0x07);
	if (ret < 0)
		goto err_handle_fail;

	ret = RMI4_wait_attn(ts->client,300);
	if (ret < 0)
		goto err_handle_fail;
		
	for (i = 0; i < 0x20; i++) {
		data[0] = i & 0xFF;
		data[1] = (i & 0xFF00) >> 8;
		memcpy(&data[2], &config[16 * i], 16);
		data[18] = 0x06;
		ret = i2c_syn_write(ts->client,fd_34.dataBase,data,19);
		if (ret < 0)
			goto err_handle_fail;

		ret = RMI4_wait_attn(ts->client,1);
		if (ret < 0)
			goto err_handle_fail;
	}

	return 0;
err_handle_fail:
        ret = -EIO; 

	return ret;
}

/*check tp whether need to update lock down */
static int syn_bootloader_is_lockdown(struct synaptics_rmi4 *ts)
{
    int ret = 0;
    uint8_t data = 0;

    if(NULL == ts)
    {
    	printk(KERN_INFO "syn_bootloader_is_lockdown: ts is null\n");
    	ret = -1;
    	return ret;
    }
    ret = i2c_syn_read(ts->client,fd_34.queryBase + 2,&data,1);
    
    if (ret < 0)
        return ret;
        
    if (data & 0x02)
    {
       /* Need to update lock down data*/ 
       return 0;
    }
    return 1;
}

/*
* update the lock down 
* return value if 1, device is already lock 
*/
static int syn_write_lockdown(struct synaptics_rmi4 *ts, uint8_t *config)
{
	uint8_t data[19] = {0};
	int ret = 0;
    uint16_t uBlockNum = 0;

    if((NULL == ts) || (NULL == config))
    {
    	printk(KERN_INFO "syn_write_lockdown: ts or config is null\n");
    	ret = -1;
    	return ret;
    }
    
	for(uBlockNum = 0; uBlockNum < 5; ++uBlockNum)
	{
		data[0] = uBlockNum & 0xff;
		data[1] = (uBlockNum & 0xff00) >> 8;
		memcpy(&data[2], &config[16 * uBlockNum], 16);
		data[18] = 0x04;
		ret = i2c_syn_write(ts->client,fd_34.dataBase,data,19);
		if (ret < 0)
			return ret;

		ret = RMI4_wait_attn(ts->client,12);
		if (ret < 0)
			return ret;
	}
    printk("Write device locked successfully.\n");	
	return 0;
}	

/*update config function*/
static int syn_config_update(struct synaptics_rmi4 *ts, uint8_t *config)
{
	uint8_t retry ;
	uint32_t crc_checksum ;
	int ret = 0;

 	
	memcpy(&crc_checksum,&config[SYN_CONFIG_SIZE - 4], 4);
	printk(KERN_INFO "[TP] CRC = %X\n" , syn_crc((uint16_t *)config, SYN_CONFIG_SIZE / 2 - 2));

	ret = RMI4_enable_program(ts->client);
	if (ret < 0) {
		printk(KERN_INFO "[TP] syn_config_update: Enable flash programming fail!\n");
		return RMI4_disable_program(ts->client);
	}
     
	if ((config[0] << 24 | config[1] << 16 |
		config[2] << 8 | config[3]) == syn_version.syn_moudel_version) {
		//compare the checksum, if same or different 
		ret = crc_comparison(ts, crc_checksum);
		if (ret < 0) {
			printk(KERN_INFO "[TP] syn_config_update: CRC comparison fail!\n");
			RMI4_disable_program(ts->client);
			return -1;
		} else if (ret == 0)
		{
		    RMI4_disable_program(ts->client);
			return 1;
		}
	}

	for (retry = 0; retry < 3; retry++) {
		ret = program_config(ts, config);
		if (ret < 0) {
			printk(KERN_INFO "[TP] syn_config_update: Program config fail %d!\n", retry + 1);
			//continue;
		}
		else
			break;
	}
    ret = RMI4_disable_program(ts->client);
    if (ret < 0)
	{
        printk(KERN_INFO "[TP] syn_config_update: Disable flash programming fail %d\n", retry + 1);
		return ret;
	}
	if (retry == 3) {
		printk(KERN_INFO "[TP] syn_config_update: Program config fail 3 times\n");
		return ret;
	}
	return 0;
}
/* Not use, retain  donot delete */
#if 0
/* int change to char */
static void itoa (uint32_t n, char s[])
{
	uint32_t tmp,i = 0;
	int num = 0;
	do{
	
	   s[i++] = n % 10 + '0';
	   
	}while((n /= 10) > 0);

    for(num = 0; num < i/2; num++)
    {
       tmp = s[i-1-num];
       s[i-1-num] = s[num];
       s[num] = tmp;       
    }
    s[i] = '\0';
	return;
}
#endif
/*upgrade synaptics firmware */
static int synaptics_cob_upgrade(struct i2c_client *client)
{
    int ret = 0;
    int i = 0;
    
    struct touch_hw_platform_data *touch_pdata = NULL;
	
    touch_pdata = client->dev.platform_data;

    if (NULL == client)
    {
    	printk(KERN_INFO "synaptics_cob_upgrade: client is null\n");
		goto err_handle_fail;
    }

    if(NULL == touch_pdata)
    {
        printk("the touch_pdata is NULL please check the init code !\n");
        goto err_handle_fail;  
    }   
    
	for (i = 0 ; i < 2; i++) 
	{
		/* manual upgrade */
		ret= i2c_update_firmware(client,TP_FW_COB_FILE_NAME);  
		if (0 == ret)
		{
			break;
		}
	}
	
	if (ret < 0)
	{
		printk(KERN_ERR "FW updated,Error\n");
		goto err_handle_fail;
	}
	
	if (touch_pdata->touch_reset)
	{
		ret = touch_pdata->touch_reset();
		if (ret < 0)
		{
			printk(KERN_ERR "%s: reset failed \n", __func__);
			goto err_handle_fail;
		}
	}
	
	ret = synaptics_rmi4_read_pdt(g_ts);
	if (ret <= 0) 
	{
		if (ret == 0)
			printk(KERN_ERR "Empty PDT\n");
			
		printk(KERN_ERR "FW updated,Error identifying device (%d)\n", ret);
		goto err_handle_fail;
	}
	
	if (syn_get_version() < 0)
	{
		printk(KERN_ERR "[TP] TOUCH_ERR: get_ic_fw_config_version fail \n");
		goto err_handle_fail;
	}

	return 0;
err_handle_fail:
      ret = -1;  
	return ret;
}

/*COB handle*/
static int synaptics_cob_handle(struct i2c_client *client)
{
    int ret = 0;
    int i = 0;
	uint8_t data = 0;
	int module_id = 0;
	uint8_t *p_config = NULL;
	uint8_t *p_lockdown_config = NULL;
    
	if(NULL == client)
	{
		printk(KERN_INFO "[TP] synaptics client not exits\n");
		goto err_handle_fail;
	}
    for (i = 0; i < 5; i++) 
	{
		ret = i2c_syn_read(client, fd_01.dataBase, &data, 1);
		if (ret < 0) {
			printk(KERN_INFO "[TP] synaptics f01_data fail\n");
			goto err_handle_fail;
		}
		
		if ((data & RMI_DEV_DEVICE_FAILURE) == RMI_DEV_DEVICE_FAILURE) 
		{
			printk(KERN_INFO "[TP] synaptics tp fail\n");
			goto err_handle_fail;
		}
		else if (data & (BIT_IS_BOOTLOADER_MODE|RMI_DEV_CFG_CRC_FAILURE))
		{
			msleep(20);
		}
		else
			break;
	}
	/*state in bootloade mode, need upgrade fw */
	if (i == 5)
	{
		if (data & (BIT_IS_BOOTLOADER_MODE|RMI_DEV_CFG_CRC_FAILURE))
		{
			ret = synaptics_cob_upgrade(client);
			if(ret < 0)
			{
				printk(KERN_ERR "%s: cob_upgrade failed \n", __func__);
				goto err_handle_fail;
			}
		}
		else
		{
			printk(KERN_INFO "[TP] TP error can not update\n");
			goto err_handle_fail;
		}
		
	}
     
    get_ic_name();
    printk(KERN_INFO "[TP] IC_NAME = %d\n",syn_version.syn_ic_name);
    	
	module_id = get_tp_id();
	printk("gpio_value is %d \n",module_id);
    if(module_id < 0)
    {
        printk(KERN_INFO "Get tp gpio value [%d] fail! \n",module_id);
    }
	
	p_config  = get_tp_version_config(module_id,syn_version.syn_ic_name);

	if(NULL == p_config)
	{
	    printk(KERN_INFO "Get config  fail!");
	} 

    if (syn_get_version() < 0)
	{
		printk(KERN_ERR "[TP] TOUCH_ERR: get_ic_fw_version fail \n");
		goto err_handle_fail;
	}

	/*Default need upgrade fw*/
	if (NEED_UPDATE_FW == is_need_update_fw())
	{
	    if(CURRENT_PR_VERSION != syn_version.syn_firmware_version)
	    {
		    ret = synaptics_cob_upgrade(client);
			if(ret < 0)
			{
				printk(KERN_ERR "%s: cob_upgrade failed \n", __func__);
				goto err_handle_fail;
			}
		}
	}

	/* update module config, enable touchscreen was operator */
	if((NULL != p_config) && (CURRENT_PR_VERSION == syn_version.syn_firmware_version))
	{
		ret = syn_config_update(g_ts, p_config);
		if (ret < 0) 
		{
			printk(KERN_ERR "[TP] TOUCH_ERR: syn_config_update fail\n");
			goto err_handle_fail;
		} 
		else if (ret == 0)
		{
			printk(KERN_INFO "[TP] syn_config_update success\n");
		}				
		else
		{
			printk(KERN_INFO "[TP] Warning: syn_config_update: the same "
			"config version and CRC but touch controller always stay in bootloader mode\n");
		}
		if (syn_get_version() < 0)
		{
			printk(KERN_ERR "[TP] TOUCH_ERR: get_ic_fw_version fail \n");
			goto err_handle_fail;
		}
	}
	else
	{
		printk(KERN_INFO "syn_config_update fail!");
		goto err_handle_fail;
	}

	/*write lockdow date */
    if(CURRENT_PR_VERSION == syn_version.syn_firmware_version)
    {
        ret = RMI4_enable_program(client);
		if (ret < 0) 
		{
			printk(KERN_INFO "[TP] syn_config_update: Enable flash programming fail!\n");
		    RMI4_disable_program(client);
			goto err_handle_fail;
		}
		ret = syn_bootloader_is_lockdown(g_ts);
		if (ret > 0)
		{
			printk(KERN_INFO "Device already locked.\n");
		}
		else if (ret == 0)
		{
			p_lockdown_config = get_tp_lockdown_config();
			if(NULL != p_lockdown_config)
			{
				ret = syn_write_lockdown(g_ts,p_lockdown_config);
				if (ret < 0)
				{
				    printk(KERN_INFO "Write device lockdown fail .\n");
					goto err_handle_fail;
				}
				RMI4_disable_program(client);	
			}
			else
			{
				printk(KERN_INFO "p_lockdown_config is null \n");
				goto err_handle_fail;
			}
		    
		}
		else
		{
			printk(KERN_INFO "lockdown info query fail .\n");
			goto err_handle_fail;
		}	
		RMI4_disable_program(client);
    }
    /* update config and lockdown info successful, Need to renew create pdt */
	ret = synaptics_rmi4_read_pdt(g_ts);
	if (ret <= 0) 
	{
		if (ret == 0)
			printk(KERN_ERR "Empty PDT\n");
			
		printk(KERN_ERR "FW updated,Error identifying device (%d)\n", ret);
		goto err_handle_fail;
	}
    return 0;
err_handle_fail:
    ret = -1;

    return ret;
}

static int synaptics_rmi4_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
    int i ;
    int ret = 0;

    /* move other */
	
	struct proc_dir_entry *d_entry;
    /*when the probe is come in we first detect the probe for touch is ready?*/
    struct touch_hw_platform_data *touch_pdata = NULL;
    struct tp_resolution_conversion tp_type_self_check = {0};
    
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
    {
		printk(KERN_ERR "%s: need I2C_FUNC_I2C\n", __func__);
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}
    TS_DEBUG_RMI("the i2c_check_functionality is ok \n");
    touch_pdata = client->dev.platform_data;

    if(NULL == touch_pdata)
    {
        printk("the touch_pdata is NULL please check the init code !\n");
        ret = -ENOMEM;
        goto err_platform_data_init_failed;
    }

    if(touch_pdata->read_touch_probe_flag)
    {
        ret = touch_pdata->read_touch_probe_flag();
    	if(ret)
    	{
        	printk(KERN_ERR "%s: the touch driver has detected! \n", __func__);
        	return -1;
    	}
    	else
    	{
        	printk(KERN_ERR "%s: it's the first touch driver! \n", __func__);
    	}
    }
    if(touch_pdata->touch_power)
    {
        ret = touch_pdata->touch_power(1);
    	if(ret)
    	{
        	printk(KERN_ERR "%s: power on failed \n", __func__);
        	ret = -ENOMEM;
       	    goto err_power_on_failed;
    	}
    }
    
	if (touch_pdata->touch_reset())
	{
		ret = touch_pdata->touch_reset();
		if (ret)
		{
			printk(KERN_ERR "%s: reset failed \n", __func__);
			goto err_power_on_failed;
		}
	}
    if(touch_pdata->get_touch_resolution)
    {
        ret = touch_pdata->get_touch_resolution(&tp_type_self_check);
        if(ret < 0)
        {
            printk(KERN_ERR "%s: reset failed \n", __func__);
            goto err_power_on_failed;
        }
        else
        {
            lcd_x = tp_type_self_check.lcd_x;
            lcd_y = tp_type_self_check.lcd_y;
            lcd_all = tp_type_self_check.lcd_all;
        }
    }
	g_ts = kzalloc(sizeof(*g_ts), GFP_KERNEL);
	if (g_ts == NULL) 
    {
        printk(KERN_ERR "%s: check zalloc failed!\n", __func__);
        ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
    synaptics_wq = create_singlethread_workqueue("synaptics_wq");
    if (!synaptics_wq)
    {
        printk(KERN_ERR "Could not create work queue synaptics_wq: no memory");
        ret = -ENOMEM;
        goto error_wq_creat_failed; 
    }
	INIT_WORK(&g_ts->work, synaptics_rmi4_work_func);
	/*delete some lines we do not used*/


	g_ts->is_support_multi_touch = client->flags;

 
	g_ts->client = client;
	i2c_set_clientdata(client, g_ts);

	ret = synaptics_rmi4_read_pdt(g_ts);
    
	if (ret <= 0) 
    {
		if (ret == 0)
			printk(KERN_ERR "Empty PDT\n");

		printk(KERN_ERR "Error identifying device (%d)\n", ret);
		ret = -ENODEV;
		goto err_pdt_read_failed;
	}
    if(touch_pdata->set_touch_probe_flag)
    {
        touch_pdata->set_touch_probe_flag(ret);
    }
/*create the right file used for update*/
#ifdef CONFIG_SYNAPTICS_UPDATE_RMI_TS_FIRMWARE
    g_client = client;  
    for (i = 0 ; i < 3; i++) 
    {
        ret= ts_firmware_file();   
        if (!ret)
        {
          break;
        }
    }
#endif /* CONFIG_SYNAPTICS_UPDATE_RMI_TS_FIRMWARE */
	
    /* to optimize the function */
   	if(TP_COB == get_touch_type())
   	{
		ret = synaptics_cob_handle(client);
		if(ret < 0)
		{
			printk(KERN_ERR "%s: cob handle failed \n", __func__);
			goto err_cob_handle_failed;
		}
   	}
   	else
   	{
   		get_ic_name();
   	}
	
   
    ts_x_max =  g_ts->f11_max_x;
    ts_y_max =  g_ts->f11_max_y;
        
/* move to get_ic_name()*/
/* if IC name is 3200 or 2202, we should use a different way to read the touch_info */
    if ((IC_TYPE_3200 == syn_version.syn_ic_name)||(IC_TYPE_2202 == syn_version.syn_ic_name))
    {
		/*show module info in COB */
		if (TP_COB == get_touch_type())
		{
			ret = tp_show_module_info();
			if(ret < 0)
			{
				printk(KERN_ERR "%s: cob handle failed \n", __func__);
				goto err_cob_handle_failed;
			}
		}
		else
		{
        	tp_read_fn34_input_name();
        }
    }
    else
    {
        ret = tp_read_input_name();
        if(!ret)
        {
            printk("the tp input name is query error!\n ");
        }
    }
	d_entry = create_proc_entry("tp_hw_type", S_IRUGO | S_IWUSR | S_IWGRP, NULL);
	if (d_entry) 
    {
		d_entry->read_proc = tp_read_proc;
		d_entry->data = NULL;
	}
    
    {
        TS_DEBUG_RMI("the ReportingMode is changged ok!\n");
    }
    
	g_ts->input_dev = input_allocate_device();
	if (!g_ts->input_dev)
    {
		printk(KERN_ERR "failed to allocate input device.\n");
		ret = -EBUSY;
		goto err_alloc_dev_failed;
	}

	g_ts->input_dev->name = "synaptics";
	dev_set_drvdata(&(g_ts->input_dev->dev), g_ts);
	g_ts->input_dev->phys = client->name;
	set_bit(EV_ABS, g_ts->input_dev->evbit);
	set_bit(EV_SYN, g_ts->input_dev->evbit);
	set_bit(EV_KEY, g_ts->input_dev->evbit);
	set_bit(BTN_TOUCH, g_ts->input_dev->keybit);
	set_bit(ABS_X, g_ts->input_dev->absbit);
	set_bit(ABS_Y, g_ts->input_dev->absbit);
    set_bit(KEY_NUMLOCK, g_ts->input_dev->keybit);
	set_bit(INPUT_PROP_DIRECT,g_ts->input_dev->propbit);
/*we removed it to here to register the touchscreen first */
	ret = input_register_device(g_ts->input_dev);
	if (ret) 
    {
		printk(KERN_ERR "synaptics_rmi4_probe: Unable to register %s \
				input device\n", g_ts->input_dev->name);
		ret = -ENODEV;
		goto err_input_register_device_failed;
	} 
    else 
	{
		TS_DEBUG_RMI("synaptics input device registered\n");
	}
	
	/*All products support five points ,delete */	
	if (g_ts->hasF11) 
	{
		if (g_ts->f11.points_supported > 5) 
		{
			point_supported_huawei = 5;
		}  
		else
		{
	    	point_supported_huawei = g_ts->f11.points_supported;
		}
		if(g_ts->is_support_multi_touch)
		{

			/* Linux 2.6.31 multi-touch */
			input_set_abs_params(g_ts->input_dev, ABS_MT_TRACKING_ID, 1,
                    			g_ts->f11.points_supported, 0, 0);
			input_set_abs_params(g_ts->input_dev, ABS_MT_POSITION_X, 0, lcd_x - 1, 0, 0);
			input_set_abs_params(g_ts->input_dev, ABS_MT_POSITION_Y, 0, lcd_y - 1, 0, 0);
			input_set_abs_params(g_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 0xF, 0, 0);
			input_set_abs_params(g_ts->input_dev, ABS_MT_TOUCH_MINOR, 0, 0xF, 0, 0);
			input_set_abs_params(g_ts->input_dev, ABS_MT_ORIENTATION, 0, 1, 0, 0);
			input_set_abs_params(g_ts->input_dev, ABS_MT_PRESSURE, 0,	255, 0, 0);
		}
		else
		{
            input_set_abs_params(g_ts->input_dev, ABS_X, 0, lcd_x-1, 0, 0);
            input_set_abs_params(g_ts->input_dev, ABS_Y, 0, lcd_y-1, 0, 0);
            input_set_abs_params(g_ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);
                
		}

		if (g_ts->hasEgrPalmDetect)
			set_bit(BTN_DEAD, g_ts->input_dev->keybit);
		if (g_ts->hasEgrFlick) 
		{
			set_bit(REL_X, g_ts->input_dev->keybit);
			set_bit(REL_Y, g_ts->input_dev->keybit);
		}
		if (g_ts->hasEgrSingleTap)
			set_bit(BTN_TOUCH, g_ts->input_dev->keybit);
		if (g_ts->hasEgrDoubleTap)
			set_bit(BTN_TOOL_DOUBLETAP, g_ts->input_dev->keybit);
	}
	if (g_ts->hasF19) 
	{
		set_bit(BTN_DEAD, g_ts->input_dev->keybit);
	}
	if (g_ts->hasF30) 
	{
		for (i = 0; i < g_ts->f30.points_supported; ++i) 
		{
			set_bit(BTN_F30 + i, g_ts->input_dev->keybit);
		}
	}

	if (client->irq) 
	{
    	if(touch_pdata->set_touch_interrupt_gpio)
    	{
			ret = touch_pdata->set_touch_interrupt_gpio();
    	}

		TS_DEBUG_RMI("Requesting IRQ...\n");

		if (request_irq(client->irq, synaptics_rmi4_irq_handler,
				IRQF_TRIGGER_LOW, client->name, g_ts) >= 0) 
		{
			TS_DEBUG_RMI("Received IRQ!\n");
			g_ts->use_irq = 1;
			#if 0
			if (set_irq_wake(client->irq, 1) < 0)
				printk(KERN_ERR "failed to set IRQ wake\n");
			#endif
		} 
		else 
		{
			TS_DEBUG_RMI("Failed to request IRQ!\n");
		}
	}

	if (!g_ts->use_irq) 
	{
		printk(KERN_ERR "Synaptics RMI4 device %s in polling mode\n", client->name);
		hrtimer_init(&g_ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		g_ts->timer.function = synaptics_rmi4_timer_func;
		hrtimer_start(&g_ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}

	/*
	 * Device will be /dev/input/event#
	 * For named device files, use udev
	 */

	g_ts->enable = 1;

	dev_set_drvdata(&g_ts->input_dev->dev, g_ts);

	if (sysfs_create_file(&g_ts->input_dev->dev.kobj, &dev_attr_synaptics_rmi4_enable.attr) < 0)
		printk("failed to create sysfs file for input device\n");

	#ifdef CONFIG_HAS_EARLYSUSPEND
	g_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	g_ts->early_suspend.suspend = synaptics_rmi4_early_suspend;
	g_ts->early_suspend.resume = synaptics_rmi4_late_resume;
	register_early_suspend(&g_ts->early_suspend);
	#endif
	printk(KERN_ERR "probing for Synaptics RMI4 device %s at $%02X...\n", client->name, client->addr);

    #ifdef CONFIG_HUAWEI_HW_DEV_DCT
    /* detect current device successful, set the flag as present */
    set_hw_dev_flag(DEV_I2C_TOUCH_PANEL);
    #endif
    
    /* set  normal operation in sleep mode, 00 is normal operation*/
    ret = i2c_smbus_write_byte_data(client, fd_01.controlBase, 0x00);
    if(ret < 0)
    {
        printk(KERN_ERR "synaptics_rmi_probe: the touch can't normal operation in sleep mode \n");
    }
	return 0;
err_input_register_device_failed:
    if(NULL != g_ts->input_dev)
	    input_free_device(g_ts->input_dev);
/* delete */

err_pdt_read_failed:
/*add error handle */
err_cob_handle_failed:
err_alloc_dev_failed:
error_wq_creat_failed:
    if (synaptics_wq)
    {   
        destroy_workqueue(synaptics_wq);
    }
    if(NULL != g_ts)
        kfree(g_ts);
err_alloc_data_failed:
err_check_functionality_failed:

    touch_pdata->touch_power(0);
err_platform_data_init_failed:

err_power_on_failed:
    TS_DEBUG_RMI("THE POWER IS FAILED!!!\n");

	return ret;
}


static int synaptics_rmi4_remove(struct i2c_client *client)
{
struct synaptics_rmi4 *ts = i2c_get_clientdata(client);
	unregister_early_suspend(&ts->early_suspend);
	if (ts->use_irq)
		free_irq(client->irq, ts);
	else
		hrtimer_cancel(&ts->timer);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static int synaptics_rmi4_suspend(struct i2c_client *client, pm_message_t mesg)
{
    int ret;
	struct synaptics_rmi4 *ts = i2c_get_clientdata(client);
	/* if use interrupt disable the irq ,else disable timer */ 
    if (ts->use_irq)
	    disable_irq_nosync(client->irq);
	else
		hrtimer_cancel(&ts->timer);

	ret = cancel_work_sync(&ts->work);    
	if (ret && ts->use_irq) /* if work was pending disable-count is now 2 */
    {   
        enable_irq(client->irq);
        printk(KERN_ERR "synaptics_ts_suspend: can't cancel the work ,so enable the irq \n");
    }
    ret = i2c_smbus_write_byte_data(client, fd_01.controlBase, 0x01); //use control base to set tp sleep
    if(ret < 0)
    {
        printk(KERN_ERR "synaptics_ts_suspend: the touch can't get into deep sleep \n");
    }

	ts->enable = 0;

	return 0;
}

static int synaptics_rmi4_resume(struct i2c_client *client)
{
    int ret;
	struct synaptics_rmi4 *ts = i2c_get_clientdata(client);
    
    
    ret = i2c_smbus_write_byte_data(ts->client, fd_01.controlBase, 0x00); //use control base to set tp wakeup
    if(ret < 0)
    {
        printk(KERN_ERR "synaptics_ts_resume: the touch can't resume! \n");
    }
    mdelay(50);
    if (ts->use_irq) 
	{
		enable_irq(client->irq);
	}
	else
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
    printk(KERN_ERR "synaptics_rmi4_touch is resume!\n");

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_rmi4_early_suspend(struct early_suspend *h)
{
	struct synaptics_rmi4 *ts;
	ts = container_of(h, struct synaptics_rmi4, early_suspend);
	synaptics_rmi4_suspend(ts->client, PMSG_SUSPEND);
}

static void synaptics_rmi4_late_resume(struct early_suspend *h)
{
	struct synaptics_rmi4 *ts;
	ts = container_of(h, struct synaptics_rmi4, early_suspend);
	synaptics_rmi4_resume(ts->client);
}
#endif

/*add the update firmware progrom*/
#ifdef CONFIG_SYNAPTICS_UPDATE_RMI_TS_FIRMWARE
/* move to up */
int RMI4_disable_program(struct i2c_client *client)
{
	unsigned char cdata; 
	unsigned int loop_count=0;
  
	printk("RMI4 disable program...\n");
	// Issue a reset command
	i2c_smbus_write_byte_data(client,fd_01.commandBase,0x01);

	// Wait for ATTN to be asserted to see if device is in idle state
	RMI4_wait_attn(client,20);

	// Read F01 Status flash prog, ensure the 6th bit is '0'
	do{
		cdata = i2c_smbus_read_byte_data(client,fd_01.dataBase);
		udelay(2);
	} while(((cdata & 0x40) != 0) && (loop_count++ < 10));

	//Rescan the Page Description Table
	return RMI4_read_PDT(client);
}

static int RMI4_enable_program(struct i2c_client *client)
{
	unsigned short bootloader_id = 0 ;
	int ret = -1;
	printk("RMI4 enable program...\n");
	 // Read and write bootload ID
	bootloader_id = i2c_smbus_read_word_data(client,fd_34.queryBase);
	i2c_smbus_write_word_data(client,fd_34.dataBase+2,bootloader_id);//write Block Data 0

	  // Issue Enable flash command
	if(i2c_smbus_write_byte_data(client, fd_34.dataBase+18, 0x0F) < 0) //write Flash Control
	{
		SYNAPITICS_DEBUG("RMI enter flash mode error\n");
		return -1;
	}
	ret = RMI4_wait_attn(client,12);

	//Rescan the Page Description Table
	RMI4_read_PDT(client);
	return ret;
}

static unsigned long ExtractLongFromHeader(const unsigned char* SynaImage) 
{
  	return((unsigned long)SynaImage[0] +
         (unsigned long)SynaImage[1]*0x100 +
         (unsigned long)SynaImage[2]*0x10000 +
         (unsigned long)SynaImage[3]*0x1000000);
}

static int RMI4_check_firmware(struct i2c_client *client,const unsigned char *pgm_data)
{
	unsigned long checkSumCode;
	unsigned long m_firmwareImgSize;
	unsigned long m_configImgSize;
	unsigned short m_bootloadImgID; 
	unsigned short bootloader_id;
	const unsigned char *SynaFirmware;
	unsigned char m_firmwareImgVersion;
	unsigned short UI_block_count;
	unsigned short CONF_block_count;
	unsigned short fw_block_size;

  	SynaFirmware = pgm_data;
	checkSumCode = ExtractLongFromHeader(&(SynaFirmware[0]));
	m_bootloadImgID = (unsigned int)SynaFirmware[4] + (unsigned int)SynaFirmware[5]*0x100;
	m_firmwareImgVersion = SynaFirmware[7];
	m_firmwareImgSize    = ExtractLongFromHeader(&(SynaFirmware[8]));
	m_configImgSize      = ExtractLongFromHeader(&(SynaFirmware[12]));
 
	UI_block_count  = i2c_smbus_read_word_data(client,fd_34.queryBase+5);//read Firmware Block Count 0
	fw_block_size = i2c_smbus_read_word_data(client,fd_34.queryBase+3);//read Block Size 0
	CONF_block_count = i2c_smbus_read_word_data(client,fd_34.queryBase+7);//read Configuration Block Count 0
	bootloader_id = i2c_smbus_read_word_data(client,fd_34.queryBase);

	return (m_firmwareImgVersion != 0 || bootloader_id == m_bootloadImgID) ? 0 : -1;

}


static int RMI4_write_image(struct i2c_client *client,unsigned char type_cmd,const unsigned char *pgm_data)
{
	unsigned short block_size;
	unsigned short img_blocks;
	unsigned short block_index;
	const unsigned char * p_data;
	int i ;
    
	unsigned char configdate[17] = {0};
    int ret = 0;
   
    
	block_size = i2c_smbus_read_word_data(client,fd_34.queryBase+3);//read Block Size 0
	
	switch(type_cmd )
	{
		case 0x02:
			img_blocks = i2c_smbus_read_word_data(client,fd_34.queryBase+5);	//read UI Firmware
			break;
		case 0x06:
			img_blocks = i2c_smbus_read_word_data(client,fd_34.queryBase+7);	//read Configuration Block Count 0	
			break;
		default:
			SYNAPITICS_DEBUG("image type error\n");
			goto error;
	}

	p_data = pgm_data;


  	if(i2c_smbus_write_word_data(client, fd_34.dataBase,0) < 0)
	{
		SYNAPITICS_DEBUG("write block number error\n");
		goto error;
	}
	
	for(block_index = 0; block_index < img_blocks; ++block_index)
	{
	      
		configdate[0] = fd_34.dataBase+2;      
		for(i=0; i<16 ; i++)      
		{          
		    configdate[i+1] = *(p_data+i);     
		}       
		ret = i2c_master_send(client, configdate, 17);    
		if (ret < 0)    
		{          
		  printk("RMI4_write_image i2c_master_send error");      
		}      
	     		
		p_data += block_size;			
		if(i2c_smbus_write_word_data(client, fd_34.dataBase+18, type_cmd) < 0) //write Flash Control		
		{			
		     SYNAPITICS_DEBUG("issue write command error\n");
			 goto error;		
		}		
		// Wait ATTN. Read Flash Command register and check error      		
		if(RMI4_wait_attn(client,1) != 0)
		{
			goto error;
		}
	}

	return 0;
error:
	return -1;
}


static int RMI4_program_configuration(struct i2c_client *client,const unsigned char *pgm_data )
{
	int ret;
	unsigned short block_size;
	unsigned short ui_blocks;

	printk("\nRMI4 program Config firmware...\n");
	block_size = i2c_smbus_read_word_data(client,fd_34.queryBase+3);//read Block Size 0
	ui_blocks = i2c_smbus_read_word_data(client,fd_34.queryBase+5);	//read Firmware Block Count 0

	if(RMI4_write_image(client, 0x06,pgm_data+ui_blocks*block_size ) < 0)
	{
		SYNAPITICS_DEBUG("write configure image error\n");
		return -1;
	}
	
	ret = i2c_smbus_read_byte_data(client,fd_34.dataBase+18);	//read Flash Control
	return ((ret & 0xF0) == 0x80 ? 0 : ret);
}

static int RMI4_program_firmware(struct i2c_client *client,const unsigned char *pgm_data)
{
	int ret=0;
	unsigned short bootloader_id;

	printk("RMI4 program UI firmware...\n");

	//read and write back bootloader ID
	bootloader_id = i2c_smbus_read_word_data(client,fd_34.queryBase);
	i2c_smbus_write_word_data(client,fd_34.dataBase+2, bootloader_id ); //write Block Data0

	//issue erase commander
	if(i2c_smbus_write_byte_data(client, fd_34.dataBase+18, 0x03) < 0) //write Flash Control
	{
		SYNAPITICS_DEBUG("RMI4_program_firmware error, erase firmware error \n");
		return -1;
	}
	RMI4_wait_attn(client,300);

	//check status
	if((ret = i2c_smbus_read_byte_data(client,fd_34.dataBase+18)) != 0x80) //check Flash Control
	{
		return -1;
	}

	//write firmware
	if( RMI4_write_image(client,0x02,pgm_data) <0 )
	{
		SYNAPITICS_DEBUG("write UI firmware error!\n");
		return -1;
	}

	ret = i2c_smbus_read_byte_data(client,fd_34.dataBase+18); //read Flash Control
	return ((ret & 0xF0) == 0x80 ? 0 : ret);
}

static int synaptics_download(struct i2c_client *client,const unsigned char *pgm_data)
{
	int ret;

	ret = RMI4_read_PDT(client);
	if(ret != 0)
	{
		printk("RMI page func check error\n");
		return -1;
	}

	ret = RMI4_enable_program(client);
	if( ret != 0)
	{
		printk("%s:%d:RMI enable program error,return...\n",__FUNCTION__,__LINE__);
		goto error;
	}

	ret = RMI4_check_firmware(client,pgm_data);
	if( ret != 0)
	{
		printk("%s:%d:RMI check firmware error,return...\n",__FUNCTION__,__LINE__);
		goto error;
	}

	ret = RMI4_program_firmware(client, pgm_data + 0x100);
	if( ret != 0)
	{
		printk("%s:%d:RMI program firmware error,return...",__FUNCTION__,__LINE__);
		goto error;
	}

	RMI4_program_configuration(client, pgm_data +  0x100);
	return RMI4_disable_program(client);

error:
	RMI4_disable_program(client);
	printk("%s:%d:error,return ....",__FUNCTION__,__LINE__);
	return -1;

}

static int i2c_update_firmware(struct i2c_client *client, char *filename) 
{
	char *buf;
	struct file	*filp;
	struct inode *inode = NULL;
	mm_segment_t oldfs;
	uint16_t	length;
	int ret = 0;
    /* add different upgrade file path */
	if ((NULL == client) || (NULL == filename))
	{
		printk(KERN_INFO "i2c_update_firmware: client or filename is null\n");
		return -1;
	}
	/* open file */
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	filp = filp_open(filename, O_RDONLY, S_IRUSR);
	if (IS_ERR(filp))
	{
            printk("%s: file %s filp_open error\n", __FUNCTION__,filename);
            set_fs(oldfs);
            return -1;
	}

	if (!filp->f_op)
	{
            printk("%s: File Operation Method Error\n", __FUNCTION__);
            filp_close(filp, NULL);
            set_fs(oldfs);
            return -1;
	}

    inode = filp->f_path.dentry->d_inode;
    if (!inode) 
    {
        printk("%s: Get inode from filp failed\n", __FUNCTION__);
        filp_close(filp, NULL);
        set_fs(oldfs);
        return -1;
    }

    /* file's size */
    length = i_size_read(inode->i_mapping->host);
    if (!( length > 0 && length < 62*1024 ))
    {
        printk("file size error\n");
        filp_close(filp, NULL);
        set_fs(oldfs);
        return -1;
    }

	/* allocation buff size */
	buf = vmalloc(length+(length%2));		/* buf size if even */
	if (!buf) 
	{
		printk("alloctation memory failed\n");
		filp_close(filp, NULL);
		set_fs(oldfs);
		return -1;
	}

    /* read data */
    if (filp->f_op->read(filp, buf, length, &filp->f_pos) != length)
    {
        printk("%s: file read error\n", __FUNCTION__);
        filp_close(filp, NULL);
        set_fs(oldfs);
        vfree(buf);
        return -1;
    }

	ret = synaptics_download(client,buf);

 	filp_close(filp, NULL);
	set_fs(oldfs);
	vfree(buf);
	return ret;
}


/* delete */


static int ts_firmware_file(void)
{
	int ret;
	struct kobject *kobject_ts;
	kobject_ts = kobject_create_and_add("touch_screen", NULL);
	if (!kobject_ts)
	{
		printk("create kobjetct error!\n");
		return -1;
	}
	
	ret = sysfs_create_file(kobject_ts, &update_firmware_attribute.attr);
	if (ret) 
	{
		kobject_put(kobject_ts);
		printk("create file error\n");
		return -1;
	}
	return 0;	
}

/*
 * The "update_firmware" file where a static variable is read from and written to.
 */
static ssize_t update_firmware_show(struct kobject *kobj, struct kobj_attribute *attr,char *buf)
{
	return 1;
}

static ssize_t update_firmware_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	char ret = -1;

	printk("#################update_firmware_store######################\n");

	if ( (buf[0] == '2')&&(buf[1] == '\0') )
	{
		/* driver detect its device  */
		ret = i2c_smbus_read_byte_data(g_client, fd_01.queryBase);
		printk("The if of synaptics device is : %d\n",ret);

		disable_irq(g_client->irq);

		/*update firmware*/
		ret = i2c_update_firmware(g_client,TP_FW_FILE_NAME);
		enable_irq(g_client->irq);
 
		if( 0 != ret )
		{
			printk("Update firmware failed!\n");
			ret = -1;
		} 
		else 
		{
			printk("Update firmware success!\n");
			arm_pm_restart(0,&ret);
			ret = 1;
		}
	}
	
	return ret;
 }
#endif 

static const struct i2c_device_id synaptics_ts_id[] = {
	{ "Synaptics_rmi", 0 },
	{ }
};
static struct i2c_driver synaptics_rmi4_driver = {
	.probe		= synaptics_rmi4_probe,
	.remove		= synaptics_rmi4_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= synaptics_rmi4_suspend,
	.resume		= synaptics_rmi4_resume,
#endif
    .id_table   = synaptics_ts_id,
	.driver = {
		.name	= "Synaptics_rmi",
	},
};
static int __devinit synaptics_rmi4_init(void)
{
/* É¾³ý´Ë¶Î´úÂë */
    
	return i2c_add_driver(&synaptics_rmi4_driver);
}

static void __exit synaptics_rmi4_exit(void)
{
/* É¾³ý´Ë¶Î´úÂë */
    
	i2c_del_driver(&synaptics_rmi4_driver);
	if (synaptics_wq)
		destroy_workqueue(synaptics_wq);
}

module_init(synaptics_rmi4_init);
module_exit(synaptics_rmi4_exit);

MODULE_DESCRIPTION("Synaptics RMI4 Driver");
MODULE_LICENSE("GPL");

