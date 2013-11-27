/*
 * drivers/input/gs_st_lis3xh.c
 * derived from gs_st.c
 *
 * Copyright (C) 2010-2011  Huawei.
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


#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <mach/vreg.h>

#include <linux/gs_st_lis3xh.h>
#include "linux/hardware_self_adapt.h"

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <linux/hw_dev_dec.h>
#endif
#include <linux/sensors.h>
//#define GS_DEBUG
//#undef GS_DEBUG 

#ifdef GS_DEBUG
#define GS_DEBUG(fmt, args...) printk(KERN_ERR fmt, ##args)
#else
#define GS_DEBUG(fmt, args...)
#endif

#define GS_POLLING   1
/* DATA_CTRL_REG: controls the output data rate of the part */
#define ODR10F       0x20   //Period  100ms
#define ODR25F       0x30   //Period   40ms
#define ODR50F       0x40   //Period   20ms
#define ODR100F      0x50   //Period   10ms
#define ODR200F      0x60   //Period    5ms
#define ODR400F      0x70   //Period  2.5ms
#define ODR_MASK     0x0F
/*This is the classcial Delay_time from framework and the units is ms*/
#define DELAY_FASTEST  10
#define DELAY_GAME     20
#define DELAY_UI       68
#define DELAY_NORMAL  200
#define DELAY_ERROR 10000
/*
 * The following table lists the maximum appropriate poll interval for each
 * available output data rate.
 * Make sure the status still have proper timer.
 */
 
static const struct {
	unsigned int cutoff;
	u8 mask;
} st_odr_table[] = {
	{ DELAY_FASTEST,ODR200F},
	{ DELAY_GAME,   ODR100F},
	{ DELAY_UI,      ODR25F},
	{ DELAY_NORMAL,  ODR10F},
	{ DELAY_ERROR,   ODR10F},
};
static struct workqueue_struct *gs_wq;
extern struct input_dev *sensor_dev;
static atomic_t st_status_flag;

struct gs_data {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	int use_irq;
	int sub_type;
	struct mutex  mlock;
	struct hrtimer timer;
	struct work_struct  work;	
	uint32_t flags;
	int (*power)(int on);
	struct early_suspend early_suspend;
};

static struct gs_data  *this_gs_data;

static int accel_delay = GS_ST_TIMRER;     /*1s*/

static atomic_t a_flag;

static compass_gs_position_type  compass_gs_position=COMPASS_TOP_GS_TOP;

#ifdef CONFIG_HAS_EARLYSUSPEND
static void gs_early_suspend(struct early_suspend *h);
static void gs_late_resume(struct early_suspend *h);
#endif

static inline int reg_read(struct gs_data *gs , int reg);
static int lis3xh_debug_mask;
module_param_named(lis3xh_debug, lis3xh_debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);

#define lis3xh_DBG(x...) do {\
	if (lis3xh_debug_mask) \
		printk(KERN_DEBUG x);\
	} while (0)
#define lis3xh_PRINT_PER_TIMES 100
unsigned int list3xh_times = 0;

void lis3xh_print_debug(int start_reg,int end_reg)
{
	int reg, ret;

	for(reg = start_reg ; reg <= end_reg ; reg ++)
	{
		/* read reg value */
		ret = reg_read(this_gs_data,reg);
		/* print reg info */
		lis3xh_DBG("lis3xh reg 0x%x values 0x%x\n",reg,ret);
	}
}

static inline int reg_read(struct gs_data *gs , int reg)
{
	int val;

	mutex_lock(&gs->mlock);

	val = i2c_smbus_read_byte_data(gs->client, reg);
	if (val < 0)
	{
		printk(KERN_ERR "i2c_smbus_read_byte_data failed! reg=0x%x, value=0x%x\n", reg, val);
	}

	mutex_unlock(&gs->mlock);

	return val;
}
static inline int reg_write(struct gs_data *gs, int reg, uint8_t val)
{
	int ret;

	mutex_lock(&gs->mlock);
	ret = i2c_smbus_write_byte_data(gs->client, reg, val);
	if(ret < 0)
	{
		printk(KERN_ERR "i2c_smbus_write_byte_data failed! reg=0x%x, value=0x%x, ret=%d\n", reg, val, ret);
	}
	mutex_unlock(&gs->mlock);

	return ret;
}

#define MG_PER_SAMPLE   	720            /*HAL: 720=1g*/                       
#define FILTER_SAMPLE_NUMBER  	65536          /*256LSB =1g*/  

static signed short gs_sensor_data[3];
/*adjust device name */
static char st_device_id[] = "st_lis3xh";

static int gs_data_to_compass(signed short accel_data [3])
{
	memset((void*)accel_data, 0, sizeof(accel_data));
	accel_data[0]=gs_sensor_data[0];
	accel_data[1]=gs_sensor_data[1];
	accel_data[2]=gs_sensor_data[2];
	return 0;
}
static void gs_st_update_odr(struct gs_data  *gs)
{
	int i;
	int reg = 0;
	int ret = 0;
	short time_reg;
	for (i = 0; i < ARRAY_SIZE(st_odr_table); i++) 
	{
		time_reg = st_odr_table[i].mask;
		if (accel_delay <= st_odr_table[i].cutoff)
		{
			accel_delay = st_odr_table[i].cutoff;
			break;
		}
	}
	printk("Update G-sensor Odr ,delay_time is %d\n",accel_delay);
	reg  = reg_read(gs, GS_ST_REG_CTRL1);
	if( reg < 0 )
	{
		printk("Update_odr read register error \n");
		return;
	}
	reg  = reg & ODR_MASK;
	time_reg = time_reg | reg ;
	ret  = reg_write(gs,GS_ST_REG_CTRL1,time_reg);
	if(ret < 0)
	{
		printk("register write failed is gs_mma_update_odr\n ");
	}
}
static int gs_st_open(struct inode *inode, struct file *file)
{
	reg_read(this_gs_data, GS_ST_REG_STATUS ); /* read status */
	/* enable x, y, z; low power mode 50 HZ */	
	reg_write(this_gs_data, GS_ST_REG_CTRL1, GS_ST_CTRL1_PD|
		GS_ST_CTRL1_Zen|
		GS_ST_CTRL1_Yen|
		GS_ST_CTRL1_Xen);
	reg_write(this_gs_data, GS_ST_REG_CTRL3, GS_INTMODE_DATA_READY);

	reg_read(this_gs_data, GS_ST_REG_OUT_XL ); /* read X */
	reg_read(this_gs_data, GS_ST_REG_OUT_XH ); /* read X */
	reg_read(this_gs_data, GS_ST_REG_OUT_YL ); /* read Y */
	reg_read(this_gs_data, GS_ST_REG_OUT_YH ); /* read Y */
	reg_read(this_gs_data, GS_ST_REG_OUT_ZL ); /* read Z*/
	reg_read(this_gs_data, GS_ST_REG_OUT_ZH ); /* read Z*/
	atomic_set(&st_status_flag, GS_RESUME);
	if (this_gs_data->use_irq)
		enable_irq(this_gs_data->client->irq);
	else
		hrtimer_start(&this_gs_data->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	return nonseekable_open(inode, file);
}
static int gs_st_release(struct inode *inode, struct file *file)
{
	
	reg_write(this_gs_data, GS_ST_REG_CTRL1, 0x00);
	atomic_set(&st_status_flag, GS_SUSPEND);
	if (this_gs_data->use_irq)
		disable_irq(this_gs_data->client->irq);
	else
		hrtimer_cancel(&this_gs_data->timer);
	
	accel_delay = GS_ST_TIMRER;	

	return 0;
}

static long
gs_st_ioctl(struct file *file, unsigned int cmd,
		unsigned long arg)
{
	
	void __user *argp = (void __user *)arg;
	signed short accel_buf[3];
	
	short flag;

	switch (cmd) 
	{
		case ECS_IOCTL_APP_SET_AFLAG:     /*set open acceleration sensor flag*/
			if (copy_from_user(&flag, argp, sizeof(flag)))
				return -EFAULT;
				break;
		case ECS_IOCTL_APP_SET_DELAY:
			if (copy_from_user(&flag, argp, sizeof(flag)))
				return -EFAULT;
				break;
		
			default:
				break;
	}

	switch (cmd) 
	{
		case ECS_IOCTL_APP_SET_AFLAG:
			atomic_set(&a_flag, flag);
			break;

		case ECS_IOCTL_APP_GET_AFLAG:  /*get open acceleration sensor flag*/
			flag = atomic_read(&a_flag);
			break;

		case ECS_IOCTL_APP_SET_DELAY:
			if(flag)
				accel_delay = flag;
			else
				accel_delay = 10;   /*10ms*/
			gs_st_update_odr(this_gs_data);
			break;

		case ECS_IOCTL_APP_GET_DELAY:
			flag = accel_delay;
			break;

		case ECS_IOCTL_READ_ACCEL_XYZ:
			gs_data_to_compass(accel_buf);
			break;

		default:
			break;
	}

	switch (cmd) 
	{
		case ECS_IOCTL_APP_GET_AFLAG:
			if (copy_to_user(argp, &flag, sizeof(flag)))
				return -EFAULT;
			break;

		case ECS_IOCTL_APP_GET_DELAY:
			if (copy_to_user(argp, &flag, sizeof(flag)))
				return -EFAULT;
			break;

		case ECS_IOCTL_READ_ACCEL_XYZ:
			if (copy_to_user(argp, &accel_buf, sizeof(accel_buf)))
				return -EFAULT;
			break;

		case ECS_IOCTL_READ_DEVICEID:
			if (copy_to_user(argp, st_device_id, sizeof(st_device_id)))
				return -EFAULT;
			break;

		default:
			break;
	}

	return 0;
}

static struct file_operations gs_st_fops = {
	.owner = THIS_MODULE,
	.open = gs_st_open,
	.release = gs_st_release,
	.unlocked_ioctl = gs_st_ioctl,
};

static struct miscdevice gsensor_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "accel",
	.fops = &gs_st_fops,
};

static void gs_work_func(struct work_struct *work)
{
    int status; 
    int x = 0, y = 0, z = 0;
    u16 u16x, u16y, u16z;
    u8 u8xl, u8xh, u8yl, u8yh, u8zl, u8zh;
    int sesc = accel_delay / 1000;
    int nsesc = (accel_delay % 1000) * 1000000;
    struct gs_data *gs = container_of(work, struct gs_data, work);

    status = reg_read(gs, GS_ST_REG_STATUS ); /* read status */
    
    if(status & (1<<3))
    {
        u8xl = reg_read(gs, GS_ST_REG_OUT_XL);
        u8xh = reg_read(gs, GS_ST_REG_OUT_XH);
        u16x = (u8xh << 8) | u8xl;
        
        u8yl = reg_read(gs, GS_ST_REG_OUT_YL);
        u8yh = reg_read(gs, GS_ST_REG_OUT_YH);
        u16y = (u8yh << 8) | u8yl;
        
        u8zl = reg_read(gs, GS_ST_REG_OUT_ZL);
        u8zh = reg_read(gs, GS_ST_REG_OUT_ZH);
        u16z = (u8zh << 8) | u8zl;
         
        if(u16x & 0x8000)
        {
            x = u16x - 65536;
        }
        else
        {
            x = u16x;
        }
                    
        if(u16y & 0x8000)
        {
            y = u16y - 65536;
        }
        else
        {
            y = u16y;
        }
                
        if(u16z & 0x8000)
        {
            z = u16z - 65536;
        }
        else
        {
            z = u16z;
        }

        /*(Decimal value/ 65536) * 4 g,For (0g ~+2.3g)*/    
        x = (MG_PER_SAMPLE*40*(s16)x)/FILTER_SAMPLE_NUMBER/10;
        y = (MG_PER_SAMPLE*40*(s16)y)/FILTER_SAMPLE_NUMBER/10;
        z = (MG_PER_SAMPLE*40*(s16)z)/FILTER_SAMPLE_NUMBER/10;

        lis3xh_DBG("%s, line %d: gs_lis3xh, x:%d y:%d z:%d sec:%d nsec:%d\n", __func__, __LINE__, x, y, z, sesc, nsesc);
        /*report different values by machines*/
        if((compass_gs_position==COMPASS_TOP_GS_BOTTOM)||(compass_gs_position==COMPASS_BOTTOM_GS_BOTTOM)||(compass_gs_position==COMPASS_NONE_GS_BOTTOM))
        {
            //inverse
            x *=(-1);
            y *=(-1);
        }
        else
        {
            /*
            if((compass_gs_position==0)||(compass_gs_position==2))
            */
            //obverse
            y *=(-1);
            z *=(-1);
        }
        input_report_abs(gs->input_dev, ABS_X, x);
        input_report_abs(gs->input_dev, ABS_Y, y);
        input_report_abs(gs->input_dev, ABS_Z, z);
        input_sync(gs->input_dev);

        /*
         * There is a transform formula between ABS_X, ABS_Y, ABS_Z
         * and Android_X, Android_Y, Android_Z.
         *                      -          -
         *                      |  0 -1  0 |
         * [ABS_X ABS_Y ABS_Z]* |  1  0  0 | = [Android_X, Android_Y, Android_Z]
         *                      |  0  0 -1 |
         *                      -          -
         * compass uses Android_X, Andorid_Y, Android_Z
         */

        memset(gs_sensor_data, 0, sizeof(gs_sensor_data));
        /*gs_sensor_data[0]= x;
        gs_sensor_data[1]= -y;
        gs_sensor_data[2]= -z;*/

        gs_sensor_data[0]= -x;
        gs_sensor_data[1]= y;
        gs_sensor_data[2]= -z;
    }
    else
    {
        printk(KERN_ERR"%s, line %d,GS_ST_REG_CTRL1 is 0x%x,status = 0x%x \n",__func__, __LINE__,reg_read(gs, GS_ST_REG_CTRL1),status);
    }
    if(lis3xh_debug_mask)
    {
        /* print reg info in such times */
        if(!(++list3xh_times%lis3xh_PRINT_PER_TIMES))  
        {
            /* count return to 0 */
            list3xh_times = 0;
            lis3xh_print_debug(GS_ST_REG_STATUS_AUX,GS_ST_REG_WHO_AM_I);
            lis3xh_print_debug(GS_ST_REG_TEMP_CFG_REG,GS_ST_REG_OUT_ZH);
            lis3xh_print_debug(GS_ST_REG_FF_WU_CFG_1,GS_ST_REG_CLICK_SRC);
            lis3xh_print_debug(GS_ST_REG_CLICK_THSY_X,GS_ST_REG_CLICK_WINDOW);
        }
    }
    if (gs->use_irq)
    {
        enable_irq(gs->client->irq);
    }
    else
    {
        if(GS_RESUME == atomic_read(&st_status_flag))
            if (0 != hrtimer_start(&gs->timer, ktime_set(sesc, nsesc), HRTIMER_MODE_REL) )
                printk(KERN_ERR "%s, line %d: hrtimer_start fail! sec=%d, nsec=%d\n", __func__, __LINE__, sesc, nsesc);
    }
}


static enum hrtimer_restart gs_timer_func(struct hrtimer *timer)
{
	struct gs_data *gs = container_of(timer, struct gs_data, timer);		
	queue_work(gs_wq, &gs->work);
	return HRTIMER_NORESTART;
}

#ifndef   GS_POLLING 	
static irqreturn_t gs_irq_handler(int irq, void *dev_id)
{
	struct gs_data *gs = dev_id;
	disable_irq(gs->client->irq);
	queue_work(gs_wq, &gs->work);
	return IRQ_HANDLED;
}

static int gs_config_int_pin(void)
{
	int err;

	err = gpio_request(GPIO_INT1, "gpio_gs_int");
	if (err)
	{
		printk(KERN_ERR "gpio_request failed for st gs int\n");
		return -1;
	}
	err = gpio_configure(GPIO_INT1, GPIOF_INPUT | IRQF_TRIGGER_HIGH);
	if (err)
	{
			gpio_free(GPIO_INT1);
		printk(KERN_ERR "gpio_config failed for gs int HIGH\n");
		return -1;
	}

	return 0;
}

static void gs_free_int(void)
{

	gpio_free(GPIO_INT1);

}

#endif
static int gs_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{	
	int ret;
	struct gs_data *gs;
	int reg_st;
	struct gs_platform_data *pdata = NULL;

	/*delete 19 lines*/
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "gs_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}
	
	/*turn on the power*/
	pdata = client->dev.platform_data;
	if (pdata){
#ifdef CONFIG_ARCH_MSM7X30
		if(pdata->gs_power != NULL){
			ret = pdata->gs_power(IC_PM_ON);
			if(ret < 0 ){
				goto err_check_functionality_failed;
			}
		}
#endif
		/*adapt_fn is out of style,delete*/
		
		if(pdata->get_compass_gs_position != NULL){
			compass_gs_position=pdata->get_compass_gs_position();
		}
			

		if(pdata->init_flag != NULL){
			if(*(pdata->init_flag)){
				printk(KERN_ERR "gs_list3xh probe failed, because the othe gsensor has been probed.\n");
				ret = -ENODEV;
				goto err_power_failed;
			}
		}
	} 
#ifndef   GS_POLLING 	
	ret = gs_config_int_pin();
	if(ret <0)
	{
		goto err_power_failed;
	}
#endif

	gs = kzalloc(sizeof(*gs), GFP_KERNEL);
	if (gs == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	mutex_init(&gs->mlock);

	INIT_WORK(&gs->work, gs_work_func);
	gs->client = client;
	i2c_set_clientdata(client, gs);

	gs->sub_type = reg_read(gs, 0x0f);
	/*this plan is provided by ST company,detail is attaching on the dts website*/
	reg_st = reg_read(gs, 0x1E);
	reg_st = reg_st | 0x80 ;
	ret = reg_write(gs, 0x1E, reg_st);
	if (ret < 0) 
	{
		printk(KERN_ERR "write 0x1E fail at first time!\n");
		ret = reg_write(gs, 0x1E, reg_st);
		if (ret < 0) 
		{
			printk(KERN_ERR "write 0x1E fail at second time!\n");
		}
		else
		{
			printk(KERN_ERR "write 0x1E success at second time!\n");
		}
	}
	else
	{
	printk(KERN_ERR "write 0x1E success at first time!\n");
	}
	reg_st = reg_read(gs, 0x1E);
	if(0x80 == (reg_st & 0x80))
	{
		printk("set and read success!\n");
	}
	else
	{
		printk("set failed!\n");
	}
	ret = reg_write(gs, GS_ST_REG_CTRL2, 0x00); /* device command = ctrl_reg2 */
	if (ret < 0) {
		printk(KERN_ERR "i2c_smbus_write_byte_data failed\n");
		/* fail? */
		goto err_detect_failed;
	}
	atomic_set(&st_status_flag, GS_SUSPEND);
	#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_G_SENSOR);
	#endif

	if (sensor_dev == NULL)
	{
		gs->input_dev = input_allocate_device();
		if (gs->input_dev == NULL) {
			ret = -ENOMEM;
			printk(KERN_ERR "gs_probe: Failed to allocate input device\n");
			goto err_input_dev_alloc_failed;
		}
		
		gs->input_dev->name = "sensors";
		sensor_dev = gs->input_dev;
		
	}else{

		gs->input_dev = sensor_dev;
	}
	
	gs->input_dev->id.vendor = GS_STLIS3XH;

	set_bit(EV_ABS,gs->input_dev->evbit);
	
	/* < DTS20111208XXXXX  liujinggang 20111208 begin */
	/* modify for ES-version*/
	input_set_abs_params(gs->input_dev, ABS_X, -11520, 11520, 0, 0);
	input_set_abs_params(gs->input_dev, ABS_Y, -11520, 11520, 0, 0);
	input_set_abs_params(gs->input_dev, ABS_Z, -11520, 11520, 0, 0);
	/* DTS20111208XXXXX  liujinggang 20111208 end > */
	
	set_bit(EV_SYN,gs->input_dev->evbit);


	gs->input_dev->id.bustype = BUS_I2C;
	
	input_set_drvdata(gs->input_dev, gs);
	
	ret = input_register_device(gs->input_dev);
	if (ret) {
		printk(KERN_ERR "gs_probe: Unable to register %s input device\n", gs->input_dev->name);
		goto err_input_register_device_failed;
	}
	
	ret = misc_register(&gsensor_device);
	if (ret) {
		printk(KERN_ERR "gs_probe: gsensor_device register failed\n");
		goto err_misc_device_register_failed;
	}

#ifndef   GS_POLLING 
	if (client->irq) {
		ret = request_irq(client->irq, gs_irq_handler, 0, client->name, gs);
		
		if (ret == 0)
			gs->use_irq = 1;
		else
			dev_err(&client->dev, "request_irq failed\n");
	}
#endif 

	if (!gs->use_irq) {
		hrtimer_init(&gs->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		gs->timer.function = gs_timer_func;
		
	}
	
#ifdef CONFIG_HAS_EARLYSUSPEND
	gs->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	gs->early_suspend.suspend = gs_early_suspend;
	gs->early_suspend.resume = gs_late_resume;
	register_early_suspend(&gs->early_suspend);
#endif

	gs_wq = create_singlethread_workqueue("gs_wq");
	if (!gs_wq)
	{
		ret = -ENOMEM;
		printk(KERN_ERR "%s, line %d: create_singlethread_workqueue fail!\n", __func__, __LINE__);
		goto err_create_workqueue_failed;
	}
	this_gs_data =gs;

	if (pdata && pdata->init_flag)
		*(pdata->init_flag) = 1;
	ret = set_sensor_input(ACC, gs->input_dev->dev.kobj.name);
	if (ret) {
		dev_err(&client->dev, "%s set_sensor_input failed\n", __func__);
		goto err_create_workqueue_failed;
	}
	printk("My G-sensor is ST-LIS35DE\n");

	set_sensors_list(G_SENSOR);
	return 0;

err_create_workqueue_failed:
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&gs->early_suspend);
#endif

	if (gs->use_irq)
	{
		free_irq(client->irq, gs);
	}
	else
	{
		hrtimer_cancel(&gs->timer);
	}
err_misc_device_register_failed:
		misc_deregister(&gsensor_device);
		
err_input_register_device_failed:
	input_free_device(gs->input_dev);

err_input_dev_alloc_failed:

err_detect_failed:
	kfree(gs);


err_alloc_data_failed:
#ifndef   GS_POLLING 
	gs_free_int();
#endif
err_power_failed:
err_check_functionality_failed:
	return ret;
}

static int gs_remove(struct i2c_client *client)
{
	struct gs_data *gs = i2c_get_clientdata(client);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&gs->early_suspend);
#endif
	if (gs->use_irq)
		free_irq(client->irq, gs);
	else
		hrtimer_cancel(&gs->timer);
	
	misc_deregister(&gsensor_device);
	
	input_unregister_device(gs->input_dev);

	kfree(gs);
	return 0;
}

static int gs_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret;
	struct gs_data *gs = i2c_get_clientdata(client);
	atomic_set(&st_status_flag, GS_SUSPEND);
	if (gs->use_irq)
		disable_irq(client->irq);
	else
		hrtimer_cancel(&gs->timer);
	ret = cancel_work_sync(&gs->work);
	if (ret && gs->use_irq) 
		enable_irq(client->irq);

	reg_write(gs, GS_ST_REG_CTRL3, 0); /* disable interrupt */
	
	reg_write(gs, GS_ST_REG_CTRL1, 0x00); /* deep sleep */

	if (gs->power) {
		ret = gs->power(0);
		if (ret < 0)
			printk(KERN_ERR "gs_resume power off failed\n");
	}
	return 0;
}

static int gs_resume(struct i2c_client *client)
{
	struct gs_data *gs = i2c_get_clientdata(client);
	/*Make ODR to 200HZ as if some apk does't update ODR when resume*/
	reg_write(gs, GS_ST_REG_CTRL1, ODR200F|
						GS_ST_CTRL1_Zen|
						GS_ST_CTRL1_Yen|
						GS_ST_CTRL1_Xen); /* enable abs int */
	reg_write(gs, GS_ST_REG_CTRL3, GS_INTMODE_DATA_READY);/*active mode*/
	atomic_set(&st_status_flag, GS_RESUME);
	if (!gs->use_irq)
		hrtimer_start(&gs->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	else
		enable_irq(client->irq);
	
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void gs_early_suspend(struct early_suspend *h)
{
	struct gs_data *gs;
	gs = container_of(h, struct gs_data, early_suspend);
	gs_suspend(gs->client, PMSG_SUSPEND);
}

static void gs_late_resume(struct early_suspend *h)
{
	struct gs_data *gs;
	gs = container_of(h, struct gs_data, early_suspend);
	gs_resume(gs->client);
}
#endif

static const struct i2c_device_id gs_id[] = {
	{ /*GS_I2C_NAME*/"gs_st_lis3xh", 0 },
	{ }
};

static struct i2c_driver gs_driver = {
	.probe		=gs_probe,
	.remove		= gs_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= gs_suspend,
	.resume		= gs_resume,
#endif
	.id_table	= gs_id,
	.driver = {
		.name	="gs_st_lis3xh",
	},
};

static int __devinit gs_init(void)
{
	return i2c_add_driver(&gs_driver);
}

static void __exit gs_exit(void)
{
	i2c_del_driver(&gs_driver);
	if (gs_wq)
		destroy_workqueue(gs_wq);
}

module_init(gs_init);
module_exit(gs_exit);

MODULE_DESCRIPTION("accessor  Driver");
MODULE_LICENSE("GPL");
