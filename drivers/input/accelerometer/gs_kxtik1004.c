/* drivers/input/accelerometer/gs_kxtik1004.c*/
/*
 * Copyright (C) 2012 HUAWEI, Inc.
 * Author: zhangmin/195861 
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

#include <linux/gs_kxtik1004.h>
#include "linux/hardware_self_adapt.h"

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <linux/hw_dev_dec.h>
#endif
#include <linux/sensors.h>
#ifdef GS_DEBUG
#define GS_DEBUG(fmt, args...) printk(KERN_ERR fmt, ##args)
#else
#define GS_DEBUG(fmt, args...)
#endif
/*DBG */
/*This is the classcial Delay_time from framework and the units is ms*/
#define DELAY_FASTEST  10
#define DELAY_GAME     20
#define DELAY_UI       68
#define DELAY_NORMAL  200
#define DELAY_ERROR 10000
/*
 * The following table lists the maximum appropriate poll interval for each
 * available output data rate.
 */
static const struct {
	unsigned int cutoff;
	u8 mask;
} kxtik_odr_table[] = {
	{ DELAY_FASTEST,ODR200F},
	{ DELAY_GAME,   ODR100F},
	{ DELAY_UI,      ODR25F},
	{ DELAY_NORMAL,ODR12_5F},
	{ DELAY_ERROR, ODR12_5F},
};
static int kxtik_debug_mask ;
module_param_named(kxtik_debug, kxtik_debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);

#define kxtik_DBG(x...) do {\
	if (kxtik_debug_mask) \
		printk(KERN_DEBUG x);\
	} while (0)
	
#define GS_POLLING   1

static struct workqueue_struct *gs_wq;
extern struct input_dev *sensor_dev;

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
static int accel_delay = GS_KX_TIMRER;     /*1s*/
static atomic_t a_flag;
static atomic_t kxtik_status_flag;
static compass_gs_position_type  compass_gs_position=COMPASS_TOP_GS_TOP;

#ifdef CONFIG_HAS_EARLYSUSPEND
static void gs_early_suspend(struct early_suspend *h);
static void gs_late_resume(struct early_suspend *h);
#endif

static inline int reg_read(struct gs_data *gs , int reg)
{
	int val;
	mutex_lock(&gs->mlock);
	val = i2c_smbus_read_byte_data(gs->client, reg);
	if (val < 0)
	{
		printk(KERN_ERR "kxtik chip i2c %s failed! reg=0x%x, value=0x%x\n", __FUNCTION__, reg, val);
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
		printk(KERN_ERR "kxtik chip i2c %s failed! reg=0x%x, value=0x%x, ret=%d\n", __FUNCTION__, reg, val, ret);
	}
	mutex_unlock(&gs->mlock);

    return ret;
}

static signed short gs_sensor_data[3];
static char kxtik_device_id[] = "kxtik_1004";

static int gs_data_to_compass(signed short accel_data [3])
{
	memset((void*)accel_data, 0, sizeof(accel_data));
	accel_data[0]=gs_sensor_data[0];
	accel_data[1]=gs_sensor_data[1];
	accel_data[2]=gs_sensor_data[2];
	return 0;
}
static void gs_kxtik_update_odr(struct gs_data  *gs)
{
	int i;
	int reg = 0;
	int ret = 0;
	short time_reg;
	for (i = 0; i < ARRAY_SIZE(kxtik_odr_table); i++) 
	{
		time_reg = kxtik_odr_table[i].mask;
		if (accel_delay <= kxtik_odr_table[i].cutoff)
		{
			accel_delay = kxtik_odr_table[i].cutoff;
			break;
		}
	}
	/*kxtik doesn't need to use mask,this register's fuction is independence*/
	reg  = reg_read(gs, CTRL_REG1);
	ret  = reg_write(gs, CTRL_REG1,0x00);
	ret |= reg_write(gs,DATA_CTRL,time_reg);
	ret |= reg_write(gs,CTRL_REG1,reg);
	if(ret < 0)
	{
		printk("register write failed \n ");
	}
}
static int gs_init_reg(struct gs_data  *gs)
{
	int ret = 0;
	ret  = reg_write(gs, CTRL_REG1, 0x00);
	mdelay(10);
	ret |= reg_write(gs, CTRL_REG1, 0xD0);
	if(ret < 0)
	{
		printk("register write failed \n ");
		return ret;
	}
	return ret;
}

static int gs_kxtik_open(struct inode *inode, struct file *file)
{	
	gs_init_reg(this_gs_data);
	atomic_set(&kxtik_status_flag, GS_RESUME);
	if (this_gs_data->use_irq)
		enable_irq(this_gs_data->client->irq);
	else
		hrtimer_start(&this_gs_data->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	return nonseekable_open(inode, file);
}

static int gs_kxtik_release(struct inode *inode, struct file *file)
{
	int ret;
	ret  = reg_write(this_gs_data, CTRL_REG1, 0x00);
	atomic_set(&kxtik_status_flag, GS_SUSPEND);
	if (this_gs_data->use_irq)
		disable_irq(this_gs_data->client->irq);
	else
		hrtimer_cancel(&this_gs_data->timer);
	accel_delay = GS_KX_TIMRER;	
	return 0;
}

static long
gs_kxtik_ioctl(struct file *file, unsigned int cmd,
		unsigned long arg)
{
	/*
	struct kxtik_data *tik = file->private_data;
	struct input_dev *input_dev = tik->input_dev;
	*/
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
			/*
			 * Set current interval to the greater of the minimum interval or
			 * the requested interval
			 */	
			gs_kxtik_update_odr(this_gs_data);
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
			if (copy_to_user(argp, kxtik_device_id, sizeof(kxtik_device_id)))
				return -EFAULT;
			break;
		default:
			break;
	}
	return 0;
}

static struct file_operations gs_kxtik_fops = {
	.owner = THIS_MODULE,
	.open = gs_kxtik_open,
	.release = gs_kxtik_release,
	.unlocked_ioctl = gs_kxtik_ioctl,
};

static struct miscdevice gsensor_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "accel",
	.fops = &gs_kxtik_fops,
};

static void gs_work_func(struct work_struct *work)
{
    int x, y, z;
    s16 s14x, s14y, s14z;
    u8 u8xl, u8xh, u8yl, u8yh, u8zl, u8zh;
    int sesc = accel_delay / 1000;
    int nsesc = (accel_delay % 1000) * 1000000;
    struct gs_data *gs = container_of(work, struct gs_data, work);
    u8xl = reg_read(gs, XOUT_L_REG);
    u8xh = reg_read(gs, XOUT_H_REG);
    s14x = (s16)((u8xh << 8) | u8xl) >> 2;
    u8yl = reg_read(gs, YOUT_L_REG);
    u8yh = reg_read(gs, YOUT_H_REG);
    s14y = (s16)((u8yh << 8) | u8yl )>> 2;
    u8zl = reg_read(gs, ZOUT_L_REG);
    u8zh = reg_read(gs, ZOUT_H_REG);
    s14z = (s16)((u8zh << 8) | u8zl )>> 2;
    x = ((int)s14x * 720 )/ 1024;
    y = ((int)s14y * 720 )/ 1024;
    z = ((int)s14z * 720 )/ 1024;
    kxtik_DBG("A  x:%+5d y:%+5d z:%+5d sec:%d nsec:%d\n", s14x, s14y, s14z, sesc, nsesc);
    if((compass_gs_position==COMPASS_TOP_GS_BOTTOM)||(compass_gs_position==COMPASS_BOTTOM_GS_BOTTOM)||(compass_gs_position==COMPASS_NONE_GS_BOTTOM))
    {
        //inverse
        x *=(-1);
        y *=(-1);
    }
    else
    {
        //obverse
        y *=(-1);
        z *=(-1);
    }
    input_report_abs(gs->input_dev, ABS_X,  x);
    input_report_abs(gs->input_dev, ABS_Y,  y);
    input_report_abs(gs->input_dev, ABS_Z,  z);
    input_sync(gs->input_dev);
    /*
     * There is a transform formula between ABS_X, ABS_Y, ABS_Z
     * and Android_X, Android_Y, Android_Z.
     *                        -          -
     *                        |  0 -1  0 |
     * [ABS_X ABS_Y ABS_Z] *  |  1  0  0 | = [Android_X, Android_Y, Android_Z]
     *                        |  0  0 -1 |
     *                        -          -
     * compass uses Android_X, Andorid_Y, Android_Z
     */
    memset(gs_sensor_data, 0, sizeof(gs_sensor_data));

    gs_sensor_data[0]= -x;
    gs_sensor_data[1]= y;
    gs_sensor_data[2]= -z;

    if (gs->use_irq)
    {
        enable_irq(gs->client->irq);
    }
    else
    {
        if(GS_RESUME == atomic_read(&kxtik_status_flag))
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

static int gs_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	struct gs_data *gs;
	struct gs_platform_data *pdata = NULL;
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "gs_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}
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
				printk(KERN_ERR "gs_kxtik probe failed, because the othe gsensor has been probed.\n");
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
	gs->sub_type = reg_read(gs,WHO_AM_I);
	if(0x05 != gs->sub_type)
		goto err_detect_failed;
	ret  = reg_write(gs, CTRL_REG1, 0x50);
	if(ret <0)
	{
		printk("ret is %d\n",ret);
	}
	atomic_set(&kxtik_status_flag, GS_SUSPEND);
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
	gs->input_dev->id.vendor = GS_KXTIK1004;

	set_bit(EV_ABS,gs->input_dev->evbit);
	//provide by company
	input_set_abs_params(gs->input_dev, ABS_X, -11520, 11520, 3, 3);
	input_set_abs_params(gs->input_dev, ABS_Y, -11520, 11520, 3, 3);
	input_set_abs_params(gs->input_dev, ABS_Z, -11520, 11520, 3, 3);
	
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
	if(pdata && pdata->init_flag)
		*(pdata->init_flag) = 1;
	ret = set_sensor_input(ACC, gs->input_dev->dev.kobj.name);
	if (ret) {
		dev_err(&client->dev, "%s set_sensor_input failed\n", __func__);
		goto err_create_workqueue_failed;
	}
	printk("My G-sensor is KXTIK\n");
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
	ret  = reg_write(gs, CTRL_REG1, 0x00);
	if(ret < 0)
	{
		printk("register write failed \n ");
		return ret;
	}
	atomic_set(&kxtik_status_flag, GS_SUSPEND);
	if (gs->use_irq)
		disable_irq(client->irq);
	else
		hrtimer_cancel(&gs->timer);
	ret = cancel_work_sync(&gs->work);
	if (ret && gs->use_irq) 
		enable_irq(client->irq);
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
	gs_init_reg(gs);
	atomic_set(&kxtik_status_flag, GS_RESUME);
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
	{ /*GS_I2C_NAME*/"gs_kxtik", 0 },
	{ },
};


static struct i2c_driver gs_driver = {
	.probe		= gs_probe,
	.remove		= gs_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= gs_suspend,
	.resume		= gs_resume,
#endif
	.id_table	= gs_id,
	.driver = {
		.name	= "gs_kxtik",
	},
};

static int __devinit gs_kxtik_init(void)
{
	return i2c_add_driver(&gs_driver);
}

static void __exit gs_kxtik_exit(void)
{
	i2c_del_driver(&gs_driver);
	if (gs_wq)
		destroy_workqueue(gs_wq);
}

module_init(gs_kxtik_init);
module_exit(gs_kxtik_exit);

MODULE_DESCRIPTION("accessor  Driver");
MODULE_LICENSE("GPL");
