/* drivers/i2c/gyroscope/l3g4200d.c
 *
 * Copyright (C) 2007-2010  Huawei.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/l3g4200d.h>
#include <mach/vreg.h>
#include "linux/hardware_self_adapt.h"
#include <linux/sensors.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <linux/hw_dev_dec.h>
#endif

/* l3g4200d gyroscope registers */
#define WHO_AM_I    0x0F

#define CTRL_REG1       0x20    /* power control reg */
#define CTRL_REG2       0x21    /* power control reg */
#define CTRL_REG3       0x22    /* power control reg */
#define CTRL_REG4       0x23    /* interrupt control reg */
#define CTRL_REG5       0x24    /* interrupt control reg */
#define AXISDATA_REG    0x28
/*use FAE adviced value*/
#define MAX_VAL         11000    
#define MIN_VAL         2800
#define NORMAL_TM       10000000  /*10 HZ*/
#define SUSPEND_VAL     10000000    
#define VENDOR          0x11
#define GYRO_ENABLE 1   
#define GYRO_DISABLE  -1
#define MAX_VALUE  2147483647
#define MIN_VALUE  -2147483647

static int gyro_debug_mask = 0;
module_param_named(gyro_debug, gyro_debug_mask, int,
		S_IRUGO | S_IWUSR | S_IWGRP);

#define GYRO_DBG(x...) do {\
	if (gyro_debug_mask) \
		printk(KERN_DEBUG x);\
	} while (0)

static struct workqueue_struct *gy_wq;
static struct input_dev *sensor_dev;
/*
 * L3G4200D gyroscope data
 * brief structure containing gyroscope values for yaw, pitch and roll in
 * signed short
 */
struct l3g4200d_t {
	short	x,	/* x-axis angular rate data. Range -2048 to 2047. */
		y,	/* y-axis angluar rate data. Range -2048 to 2047. */
		z;	/* z-axis angular rate data. Range -2048 to 2047. */
};

static struct l3g4200d_t sensor_data;
/* static struct i2c_client *l3g4200d_client; */

struct l3g4200d_data {
	struct i2c_client *client;
	struct gyro_platform_data *pdata;
    struct input_dev *input_dev;
	struct mutex  mlock;
	struct hrtimer timer;
	struct work_struct  work;	
	int flags;  
	struct early_suspend early_suspend;
};

static struct l3g4200d_data *gyro;
static short userdata[3];
static int fusiondata[10];
static atomic_t a_flag;

#ifdef CONFIG_HAS_EARLYSUSPEND
static void gy_early_suspend(struct early_suspend *h);
static void gy_late_resume(struct early_suspend *h);
#endif

static char l3g4200d_i2c_write(unsigned char reg_addr,
				    unsigned char *data,
				    unsigned char len);

static char l3g4200d_i2c_read(unsigned char reg_addr,
				   unsigned char *data,
				   unsigned char len);

/* set l3g4200d digital gyroscope bandwidth */
int l3g4200d_set_bandwidth(char bw)
{
	int res = 0;
	unsigned char data;

	res = i2c_smbus_read_word_data(gyro->client, CTRL_REG1);
	if (res >= 0)
		data = res & 0x000F;

	data = data + bw;
	res = l3g4200d_i2c_write(CTRL_REG1, &data, 1);
	return res;
}

/* read selected bandwidth from l3g4200d */
int l3g4200d_get_bandwidth(unsigned char *bw)
{
	int res = 1;
	/* TO DO */
	return res;
}

int l3g4200d_set_mode(char mode)
{
	int res = 0;
	unsigned char data;

	res = i2c_smbus_read_word_data(gyro->client, CTRL_REG1);
	if (res >= 0)
		data = res & 0x00F7;

	data = mode + data;

	res = l3g4200d_i2c_write(CTRL_REG1, &data, 1);
	return res;
}

int l3g4200d_set_range(char range)
{
	int res = 0;
	unsigned char data;
	
	res = i2c_smbus_read_word_data(gyro->client, CTRL_REG4);
	if (res >= 0)
		data = res & 0x00CF;

	data = range + data;
	res = l3g4200d_i2c_write(CTRL_REG4, &data, 1);
	return res;

}
int l3g4200d_disable_selftest(void)
{
	int res = 0;
	unsigned char data;
	
	res = i2c_smbus_read_word_data(gyro->client, CTRL_REG4);
	if (res >= 0)
		data = res & 0x00F1;
	res = l3g4200d_i2c_write(CTRL_REG4, &data, 1);
	return res;		
}
int l3g4200d_set_selftest(char status)
{
	int res = 0;
	unsigned char data;
	
	res = i2c_smbus_read_word_data(gyro->client, CTRL_REG4);
	if (res >= 0)
		data = res & 0x00F1;

	data = status + data;
	res = l3g4200d_i2c_write(CTRL_REG4, &data, 1);
	return res;	
}
/* gyroscope data readout */
int l3g4200d_read_gyro_values(struct l3g4200d_t *data)
{
	int res;
	unsigned char gyro_data[6];
	/* x,y,z hardware data */
	int hw_d[3] = { 0 };

	res = l3g4200d_i2c_read(AXISDATA_REG, &gyro_data[0], 6);

	hw_d[0] = (short) (((gyro_data[1]) << 8) | gyro_data[0]);
	hw_d[1] = (short) (((gyro_data[3]) << 8) | gyro_data[2]);
	hw_d[2] = (short) (((gyro_data[5]) << 8) | gyro_data[4]);

	/* hw_d[0] >>= gyro->sensitivity;
	hw_d[1] >>= gyro->sensitivity;
	hw_d[2] >>= gyro->sensitivity; */

	data->x = ((gyro->pdata->negate_x) ? (-hw_d[gyro->pdata->axis_map_x])
		   : (hw_d[gyro->pdata->axis_map_x]));
	data->y = ((gyro->pdata->negate_y) ? (-hw_d[gyro->pdata->axis_map_y])
		   : (hw_d[gyro->pdata->axis_map_y]));
	data->z = ((gyro->pdata->negate_z) ? (-hw_d[gyro->pdata->axis_map_z])
		   : (hw_d[gyro->pdata->axis_map_z]));

	userdata[0] = data->x;
	userdata[1] = data->y;
	userdata[2] = data->z;
	return res;
}


/* Device Initialization  */
static int device_init(void)
{
	int res = -1;
	#if 0
	unsigned char buf[5];
	buf[0] = 0x27;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf[4] = 0x00;
	res = l3g4200d_i2c_write(CTRL_REG1, &buf[0], 5);
	#endif
	res = l3g4200d_set_mode(PM_NORMAL);
	if(res < 0)
	{
		printk(KERN_ERR "open device i2c write mode error\n");	
		goto out;
	}
	res = l3g4200d_set_range(L3G4200D_FS_2000DPS);
	if(res < 0)
	{
		printk(KERN_ERR "open device i2c write range error\n");
		goto out;	
	}
	res = l3g4200d_set_bandwidth(ODR100_BW25);
	if(res < 0)
	{
		printk(KERN_ERR "open device i2c write bandwidth error\n");
		goto out;	
	}
out:
	return res;
}

/*  i2c write routine for l3g4200d digital gyroscope */
static char l3g4200d_i2c_write(unsigned char reg_addr,
				    unsigned char *data,
				    unsigned char len)
{
	int dummy;
	int i;

	if (gyro->client == NULL)  /*  No global client pointer? */
		return -1;
	for (i = 0; i < len; i++) {
		dummy = i2c_smbus_write_byte_data(gyro->client,
						  reg_addr++, data[i]);
		if (dummy) {
			GYRO_DBG("i2c write error\n");
			return dummy;
		}
	}
	return 0;
}

static void gy_work_func(struct work_struct *work)
{
	int sesc = 0;
	struct l3g4200d_data *gy = container_of(work, struct l3g4200d_data, work);
	memset(&sensor_data, 0, sizeof(sensor_data));
	if (gy->client == NULL)
	{
		printk(KERN_ERR "gy client is null");
		return;
	}
	sesc = 0;
	l3g4200d_read_gyro_values(&sensor_data);
	GYRO_DBG( "X axis: %d Y axis: %d Z axis: %d\n", sensor_data.x,sensor_data.y,sensor_data.z);
	if (atomic_read(&a_flag)) 
	{
		GYRO_DBG("report fusion event to input dev!");
		input_report_abs(gy->input_dev, ABS_RX, fusiondata[0]);		
		input_report_abs(gy->input_dev, ABS_RY, fusiondata[1]);		
		input_report_abs(gy->input_dev, ABS_RZ, fusiondata[2]);
		input_report_abs(gy->input_dev, ABS_THROTTLE, fusiondata[3]);				
		input_report_abs(gy->input_dev, ABS_RUDDER, fusiondata[4]);		
		input_report_abs(gy->input_dev, ABS_WHEEL, fusiondata[5]);
		input_report_abs(gy->input_dev, ABS_GAS, fusiondata[6]);			
		input_report_abs(gy->input_dev, ABS_BRAKE, fusiondata[7]);		
		input_report_abs(gy->input_dev, ABS_HAT0X, fusiondata[8]);
		input_report_abs(gy->input_dev, ABS_HAT0Y, fusiondata[9]);				
		atomic_set(&a_flag, 0);
	}
	input_report_abs(gy->input_dev, ABS_X, sensor_data.x);//cross x,y adapter hal sensors_akm8973.c			
	input_report_abs(gy->input_dev, ABS_Y, sensor_data.y);		
	input_report_abs(gy->input_dev, ABS_Z, sensor_data.z);
	input_sync(gy->input_dev);
	/*initalize timer os gyro*/
	if(gyro->flags > 0)
	{
		hrtimer_start(&gy->timer, ktime_set(sesc, NORMAL_TM), HRTIMER_MODE_REL);
	}
	else
	{
		hrtimer_start(&gy->timer, ktime_set(SUSPEND_VAL, 0), HRTIMER_MODE_REL);
	}
}
/*  i2c read routine for l3g4200d digital gyroscope */
static char l3g4200d_i2c_read(unsigned char reg_addr,
				   unsigned char *data,
				   unsigned char len)
{
	int dummy = 0;
	int i = 0;
	if (gyro->client == NULL)  /*  No global client pointer? */
		return -1;

	while (i < len) {
		dummy = i2c_smbus_read_word_data(gyro->client, reg_addr++);
		if (dummy >= 0) {
			data[i] = dummy & 0x00ff;
			//printk(KERN_ERR" i2c read %d %d %x\n ",i, data[i], reg_addr-1);
			i++;
		} else {
			GYRO_DBG(" i2c read error\n ");
			return dummy;
		}
		dummy = len;
	        //usleep_range(3000, 6000);
	}
	return dummy;
}
static enum hrtimer_restart gy_timer_func(struct hrtimer *timer)
{
	struct l3g4200d_data *gy = container_of(timer, struct l3g4200d_data, timer);		
	queue_work(gy_wq, &gy->work);
	//hrtimer_start(&gs->timer, ktime_set(0, 500000000), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

/*  read command for l3g4200d device file  */
static ssize_t l3g4200d_read(struct file *file, char __user *buf,
				  size_t count, loff_t *offset)
{
	struct l3g4200d_t data;
	if (gyro->client == NULL)
		return -1;
	l3g4200d_read_gyro_values(&data);
	GYRO_DBG("X axis: %d\n", data.x);
	GYRO_DBG("Y axis: %d\n", data.y);
	GYRO_DBG("Z axis: %d\n", data.z);
	return 0;
}

/*  write command for l3g4200d device file */
static ssize_t l3g4200d_write(struct file *file, const char __user *buf,
				   size_t count, loff_t *offset)
{
	if (gyro->client == NULL)
		return -1;
	
	GYRO_DBG("l3g4200d should be accessed with ioctl command\n");
	return 0;
}

/*  open command for l3g4200d device file  */
static int l3g4200d_open(struct inode *inode, struct file *file)
{
	if (gyro->client == NULL) {
		GYRO_DBG( "I2C driver not install\n");
		return -1;
	}
	device_init();
	hrtimer_start(&gyro->timer, ktime_set(0, NORMAL_TM), HRTIMER_MODE_REL);
	gyro->flags = 1;
	GYRO_DBG("l3g4200d has been opened\n");
	return nonseekable_open(inode, file);
}

/*  release command for l3g4200d device file */
static int l3g4200d_close(struct inode *inode, struct file *file)
{
	int ret = -1;
	if (gyro == NULL || gyro->client == NULL) {
		GYRO_DBG( "I2C driver not install\n");
		return -1;
	}
	hrtimer_cancel(&gyro->timer);
	ret = l3g4200d_set_mode(PM_OFF);
	if(ret < 0)
	{
		printk(KERN_ERR "close device i2c set mode PM_OFF err!\n");	
	}
	gyro->flags = -1;
	GYRO_DBG("L3G4200D has been closed\n");
	return 0;
}


/*  ioctl command for l3g4200d device file */
static long l3g4200d_ioctl(struct file *file,
			       unsigned int cmd, unsigned long arg)
{
	int err = 0;
	unsigned char data[6];
	short flag = 1;
	int val = 0;
	void __user *argp = (void __user *)arg;
	/* check l3g4200d_client */
	if (gyro->client == NULL) {
		GYRO_DBG( "I2C driver not install\n");
		return -EFAULT;
	}

	/* cmd mapping */

	switch (cmd) {
	case L3G4200D_SET_RANGE:
		if (copy_from_user(data, argp, 1) != 0) {
			GYRO_DBG( "copy_from_user error\n");
			return -EFAULT;
		}
		err = l3g4200d_set_range(*data);
		return err;
		
	case L3G4200D_SET_MODE:
		if (copy_from_user(data, argp, 1) != 0) {
			GYRO_DBG( "copy_to_user error\n");
			return -EFAULT;
		}
		/*enable or disable timer*/
		if( PM_NORMAL == *data )
 		{
			gyro->flags = GYRO_ENABLE;		
			hrtimer_start(&gyro->timer, ktime_set(0, NORMAL_TM), HRTIMER_MODE_REL);
		}	
		else
		{
			err = 0;
			gyro->flags = GYRO_DISABLE;		
		}	
		GYRO_DBG("L3G4200D_SET_MODE %d\n", *data);
		err = l3g4200d_set_mode(*data);
		return err;

	case L3G4200D_SET_BANDWIDTH:
		if (copy_from_user(data, argp, 1) != 0) {
			GYRO_DBG("copy_from_user error\n");
			return -EFAULT;
		}
		err = l3g4200d_set_bandwidth(*data);
		return err;

	case L3G4200D_READ_GYRO_VALUES:
		err = l3g4200d_read_gyro_values(
				(struct l3g4200d_t *)data);

		if (copy_to_user((struct l3g4200d_t *)arg,
				 (struct l3g4200d_t *)data, 6) != 0) {
			GYRO_DBG("copy_to error\n");			
			return -EFAULT;
		}
		return err;
	case L3G4200D_SET_FUSION_DATA:
		if(copy_from_user((void*)fusiondata, argp, sizeof(fusiondata)) != 0){
			return -EFAULT;
		}
		atomic_set(&a_flag, 1);
		GYRO_DBG("L3G4200D_SET_FUSION_DATA copy_from error\n");
		return err;
	case L3G4200D_GET_GYRO_DATA:
		if(copy_to_user(argp, &userdata[0], sizeof(userdata))!= 0)
		{
			return -EFAULT;
		}
		GYRO_DBG("L3G4200D_GET_GYRO_DATA copy_to error\n");
		return 0; 
	case ECS_IOCTL_APP_GET_GYRO_DATA:
		hrtimer_cancel(&gyro->timer);
		memset(&sensor_data, 0, sizeof(sensor_data));
	 	err = l3g4200d_read_gyro_values(&sensor_data);
	 	if(err < 0)
		{
			GYRO_DBG("L3G4200D ECS_IOCTL_APP_GET_GYRO_DATA TEST error!\n");
			return err;
		}
	 	GYRO_DBG(" before X axis: %d Y axis: %d Z axis: %d\n", sensor_data.x,sensor_data.y,sensor_data.z);
		if(copy_to_user(argp, &sensor_data, sizeof(sensor_data))!= 0)
		{
			GYRO_DBG("ECS_IOCTL_APP_GET_GYRO_DATA copy to user error\n");
			return -EFAULT;
		}
		return 0; 
	case ECS_IOCTL_APP_GET_GYRO_CAL:	
		GYRO_DBG("L3G4200D mmi get ret!\n");
		/* self-test flowchart update*/
		hrtimer_cancel(&gyro->timer);
		memset(&sensor_data, 0, sizeof(sensor_data));
		err = l3g4200d_read_gyro_values(&sensor_data);
		l3g4200d_set_selftest(L3G4200D_SELFTEST_EN);
		msleep(800);
		err = l3g4200d_read_gyro_values(&sensor_data);
		if(err < 0)
		{
			GYRO_DBG("L3G4200D SELF-TEST read error!\n");
			return err;
		}
		/*put data to user*/
		if (copy_to_user(argp, &sensor_data, sizeof(sensor_data))) 
		{
			return -EFAULT;
		}
		l3g4200d_disable_selftest();
		return 0; 		
	case ECS_IOCTL_APP_GET_CAL:	
		GYRO_DBG("L3G4200D mmi get ret!\n");
		/* self-test flowchart update*/
		hrtimer_cancel(&gyro->timer);
		memset(&sensor_data, 0, sizeof(sensor_data));
		err = l3g4200d_read_gyro_values(&sensor_data);
		l3g4200d_set_selftest(L3G4200D_SELFTEST_EN);
		msleep(800);
		err = l3g4200d_read_gyro_values(&sensor_data);
		if(err < 0)
		{
			GYRO_DBG("L3G4200D SELF-TEST read error!\n");
			return err;
		}
		
		val = abs(sensor_data.x);
		if( (val > MAX_VAL) || (val < MIN_VAL) )
		{
			flag = -1;
			GYRO_DBG("L3G4200D SELF-TEST x %d error!\n", val);	
		}
		
		val = abs(sensor_data.y);
		if( (val > MAX_VAL) || (val < MIN_VAL) )
		{
			flag = -1;
			GYRO_DBG("L3G4200D SELF-TEST y %d error!\n", val);
		}	
		
		val = abs(sensor_data.z);
		if( (val > MAX_VAL) || (val < MIN_VAL) )
		{
			flag = -1;
			GYRO_DBG("L3G4200D SELF-TEST z %d error!\n", val);	
		}
	    if (copy_to_user(argp, &flag, sizeof(flag))) {
			return -EFAULT;
		}
		l3g4200d_disable_selftest();
		return 0; 		
	default:
		return 0;
	}
	return err;
}





static int l3g4200d_validate_pdata(struct l3g4200d_data *gyro)
{
	if (gyro->pdata->axis_map_x > 2 ||
	    gyro->pdata->axis_map_y > 2 ||
	    gyro->pdata->axis_map_z > 2) {
		dev_err(&gyro->client->dev,
			"invalid axis_map value x:%u y:%u z%u\n",
			gyro->pdata->axis_map_x, gyro->pdata->axis_map_y,
			gyro->pdata->axis_map_z);
		return -EINVAL;
	}

	/* Only allow 0 and 1 for negation boolean flag */
	if (gyro->pdata->negate_x > 1 ||
	    gyro->pdata->negate_y > 1 ||
	    gyro->pdata->negate_z > 1) {
		dev_err(&gyro->client->dev,
			"invalid negate value x:%u y:%u z:%u\n",
			gyro->pdata->negate_x, gyro->pdata->negate_y,
			gyro->pdata->negate_z);
		return -EINVAL;
	}

	return 0;
}

static const struct file_operations l3g4200d_fops = {
	.owner = THIS_MODULE,
	.read = l3g4200d_read,
	.write = l3g4200d_write,
	.open = l3g4200d_open,
	.release = l3g4200d_close,
	.unlocked_ioctl = l3g4200d_ioctl,
};
static struct miscdevice gysensor_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gyro",
	.fops = &l3g4200d_fops,
};
static int l3g4200d_probe(struct i2c_client *client,
			       const struct i2c_device_id *devid)
{
	struct l3g4200d_data *data;
	struct gyro_platform_data *platform_data = NULL;
	int ret = -1;
	int tempvalue;
	GYRO_DBG("l3g4200d_probe start!\n");
	
	if (client->dev.platform_data == NULL) {
		dev_err(&client->dev, "platform data is NULL. exiting.\n");
		ret = -ENODEV;
		goto exit;
	}
	/*gyro mate power*/
	platform_data = client->dev.platform_data;
	if(platform_data->gyro_power)
	{
		ret = platform_data->gyro_power(IC_PM_ON);
		if( ret < 0)
		{
			dev_err(&client->dev, "gyro power on error!\n");
			goto exit;
		}
	}  
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		ret = -ENODEV;
		goto exit_pm_off;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_I2C_BLOCK)){
		ret = -ENODEV;
		goto exit_pm_off;
	}
	
	/*
	 * OK. For now, we presume we have a valid client. We now create the
	 * client structure, even though we cannot fill it completely yet.
	 */
	data = kzalloc(sizeof(struct l3g4200d_data), GFP_KERNEL);

	if (NULL == data) {
		dev_err(&client->dev,
			"failed to allocate memory for module data\n");
		ret = -ENOMEM;
		goto exit_pm_off;
	}
	mutex_init(&data->mlock);

	INIT_WORK(&data->work, gy_work_func);
	i2c_set_clientdata(client, data);
	data->client = client;
	data->pdata = platform_data;
	
	ret = l3g4200d_validate_pdata(data);
	if (ret < 0) {
		dev_err(&client->dev, "failed to validate platform data\n");
		goto exit_kfree;
	}

    ret = i2c_smbus_read_byte(client);
	if ( ret < 0) {
		GYRO_DBG("i2c_smbus_read_byte error!!\n");
		goto err_detect_failed;
	} else {
		GYRO_DBG("L3G4200D Device detected!\n");
	}

	/* read chip id */
	tempvalue = i2c_smbus_read_word_data(client, WHO_AM_I);
	if ((tempvalue & 0x00FF) == 0x00D3) {
		GYRO_DBG("I2C driver registered!\n");
	} else {
		data->client = NULL;
		ret = -ENODEV;
		goto err_detect_failed;
	}
	if (sensor_dev == NULL)
	{
		data->input_dev = input_allocate_device();
		if (data->input_dev == NULL) {
			ret = -ENOMEM;
			printk(KERN_ERR "gs_probe: Failed to allocate input device\n");
			goto err_input_dev_alloc_failed;
		}
       
		data->input_dev->name = "gy_sensors";
		sensor_dev = data->input_dev;

	}else{
		data->input_dev = sensor_dev;
	}
	data->input_dev->id.vendor = VENDOR;
	#if 0
	set_bit(EV_REL,data->input_dev->evbit);
	set_bit(REL_RX, data->input_dev->absbit);
	set_bit(REL_RY, data->input_dev->absbit);
	set_bit(REL_RZ, data->input_dev->absbit);
	#endif
	set_bit(EV_ABS,data->input_dev->evbit);
	/* modify the func of init */
	input_set_abs_params(data->input_dev, ABS_RX, MIN_VALUE, MAX_VALUE, 0, 0);
	input_set_abs_params(data->input_dev, ABS_RY, MIN_VALUE, MAX_VALUE, 0, 0);
	input_set_abs_params(data->input_dev, ABS_RZ, MIN_VALUE, MAX_VALUE, 0, 0);
	input_set_abs_params(data->input_dev, ABS_X, MIN_VALUE, MAX_VALUE, 0, 0);
	input_set_abs_params(data->input_dev, ABS_Y, MIN_VALUE, MAX_VALUE, 0, 0);
	input_set_abs_params(data->input_dev, ABS_Z, MIN_VALUE, MAX_VALUE, 0, 0);
	
	input_set_abs_params(data->input_dev, ABS_THROTTLE, MIN_VALUE, MAX_VALUE, 0, 0);
	input_set_abs_params(data->input_dev, ABS_RUDDER, MIN_VALUE, MAX_VALUE, 0, 0);
	input_set_abs_params(data->input_dev, ABS_WHEEL, MIN_VALUE, MAX_VALUE, 0, 0);
	
	input_set_abs_params(data->input_dev, ABS_GAS, MIN_VALUE, MAX_VALUE, 0, 0);
	input_set_abs_params(data->input_dev, ABS_HAT0X, MIN_VALUE, MAX_VALUE, 0, 0);
	input_set_abs_params(data->input_dev, ABS_HAT0Y, MIN_VALUE, MAX_VALUE, 0, 0);
	input_set_abs_params(data->input_dev, ABS_BRAKE, MIN_VALUE, MAX_VALUE, 0, 0);
	set_bit(EV_SYN,data->input_dev->evbit);
	data->input_dev->id.bustype = BUS_I2C;
	input_set_drvdata(data->input_dev, data);
	ret = input_register_device(data->input_dev);
	if (ret) {
		printk(KERN_ERR "gy_probe: Unable to register %s input device\n", data->input_dev->name);
	/* create l3g-dev device class */
		goto err_input_register_device_failed;
	}
	ret = misc_register(&gysensor_device);

	if (ret) {
		printk(KERN_ERR "gy_probe: gysensor_device register failed\n");
		goto err_misc_device_register_failed;
	}

	hrtimer_init(&data->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	data->timer.function = gy_timer_func;
	atomic_set(&a_flag, 0);
	data->flags = -1;
#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = gy_early_suspend;
	data->early_suspend.resume = gy_late_resume;
	register_early_suspend(&data->early_suspend);
#endif
	GYRO_DBG("L3G4200D device created successfully\n");
	gy_wq = create_singlethread_workqueue("gy_wq");
	/* log for create workqueue fail */
	if (!gy_wq)
	{
		ret = -ENOMEM;
		printk(KERN_ERR "%s, line %d: create_singlethread_workqueue fail!\n", __func__, __LINE__);
		goto err_gy_create_workqueue_failed;
	}

	gyro = data;
//	hrtimer_start(&this_gs_data->timer, ktime_set(0, 500000000), HRTIMER_MODE_REL);
	#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_GYROSCOPE);
	#endif
	
	ret = set_sensor_input(GYRO, data->input_dev->dev.kobj.name);
	if (ret) {
		dev_err(&client->dev, "%s set_sensor_input failed\n", __func__);
		goto err_misc_device_register_failed;
	}
	printk(KERN_DEBUG "l3g4200d_probe   successful");

	set_sensors_list(GY_SENSOR);
	return 0;
/*add gyro exception process*/
err_gy_create_workqueue_failed:
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif

	hrtimer_cancel(&data->timer);
err_misc_device_register_failed:
	misc_deregister(&gysensor_device);
err_input_register_device_failed:
	input_free_device(gyro->input_dev);
err_input_dev_alloc_failed:
err_detect_failed:
exit_kfree:
	kfree(gyro);
exit_pm_off:
/*No need to power down*/
exit:
	return ret;
}

static int l3g4200d_remove(struct i2c_client *client)
{
	struct l3g4200d_data *lis = i2c_get_clientdata(client);
	unregister_early_suspend(&lis->early_suspend);
	hrtimer_cancel(&lis->timer);
	misc_deregister(&gysensor_device);
	input_unregister_device(lis->input_dev);
	kfree(lis);
	GYRO_DBG("L3G4200D driver removing\n");
	return 0;
}

static int l3g4200d_suspend(struct i2c_client *client, pm_message_t state)
{
	int ret;
	struct l3g4200d_data *lis = i2c_get_clientdata(client);
	/*add mutex if gyro suspend*/
	mutex_lock(&lis->mlock);
	ret = l3g4200d_set_mode(PM_OFF);
	if(ret < 0)
	{
		printk(KERN_ERR "open device i2c set mode PM_OFF err!\n");	
	}
	hrtimer_cancel(&lis->timer);
	ret = cancel_work_sync(&lis->work);
	mutex_unlock(&lis->mlock);
	return 0;
}

static int l3g4200d_resume(struct i2c_client *client)
{
	int ret;
	struct l3g4200d_data *lis = i2c_get_clientdata(client);
	/*if gyro is opened, need to set power reg bit*/
	/*add mutex if gyro resume*/
	mutex_lock(&lis->mlock);
	if(gyro->flags > 0)
	{
		ret = l3g4200d_set_mode(PM_NORMAL);
		if(ret < 0)
		{
			printk(KERN_ERR "open device i2c set mode PM_NORMAL err!\n");	
		}
		hrtimer_start(&lis->timer, ktime_set(0, NORMAL_TM), HRTIMER_MODE_REL);
	}
	else
	{
		hrtimer_start(&lis->timer, ktime_set(SUSPEND_VAL, 0), HRTIMER_MODE_REL);
	}
	mutex_unlock(&lis->mlock);
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void gy_early_suspend(struct early_suspend *h)
{
	struct l3g4200d_data *gy;
	gy = container_of(h, struct l3g4200d_data, early_suspend);
	l3g4200d_suspend(gy->client, PMSG_SUSPEND);
	GYRO_DBG("L3G4200D driver gy_early_suspend\n");
}
static void gy_late_resume(struct early_suspend *h)
{
	struct l3g4200d_data *gy;
	gy = container_of(h, struct l3g4200d_data, early_suspend);
	l3g4200d_resume(gy->client);
	GYRO_DBG("L3G4200D driver l3g4200d_resume\n");
}
#endif

static const struct i2c_device_id l3g4200d_id[] = {
	{ "l3g4200d", 0 },
	{},
};

MODULE_DEVICE_TABLE(i2c, l3g4200d_id);

static struct i2c_driver l3g4200d_driver = {
	.class = I2C_CLASS_HWMON,
	.probe = l3g4200d_probe,
	.remove = __devexit_p(l3g4200d_remove),
	.id_table = l3g4200d_id,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = l3g4200d_suspend,
	.resume = l3g4200d_resume,
#endif
	.driver = {
	.owner = THIS_MODULE,
	.name = "l3g4200d",
	},
};

static int __init l3g4200d_init(void)
{
	return i2c_add_driver(&l3g4200d_driver);
}

static void __exit l3g4200d_exit(void)
{

	i2c_del_driver(&l3g4200d_driver);
	if(gy_wq)
	   destroy_workqueue(gy_wq);
	return;
}

module_init(l3g4200d_init);
module_exit(l3g4200d_exit);

MODULE_DESCRIPTION("l3g4200d digital gyroscope driver");
MODULE_LICENSE("GPL");

