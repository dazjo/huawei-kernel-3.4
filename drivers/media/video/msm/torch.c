/* Add new device node for torch */
/*
 * Copyright (c) 2010, HUAWEI. All rights reserved.
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

#include<linux/module.h>
#include<linux/platform_device.h>
#include<linux/miscdevice.h>
#include<linux/fs.h>
#include<asm/atomic.h>
#include<mach/camera.h>
#include <asm/mach-types.h>

#define PRINT_BUG

#ifdef  PRINT_BUG
#undef CDBG
#define CDBG(fmt,args...) printk(KERN_INFO "torch.c :" fmt,##args);
#endif

#define CAMERA_LED_GET 0
#define CAMERA_LED_SET 1

static atomic_t flag = ATOMIC_INIT(0);
static atomic_t camera_led_flag = ATOMIC_INIT(0);

static int hw_camera_led_open(struct inode * inode, struct file *file);
static long hw_camera_led_ioctl(struct file *filep,unsigned int cmd,unsigned long arg);
static int hw_camera_led_release(struct inode * inode, struct file *file);


const struct file_operations hw_camera_led_fops =
{
	.owner = THIS_MODULE,
	.open = hw_camera_led_open,
	.unlocked_ioctl = hw_camera_led_ioctl,
	.release = hw_camera_led_release,
};

static struct miscdevice hw_camera_led_device =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "hw_led",
	.fops = &hw_camera_led_fops,
};

struct msm_camera_sensor_flash_src msm_camera_led_src =
{
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_PWM,
	._fsrc.pwm_src.freq  = 500,/*pwm freq*/
	._fsrc.pwm_src.max_load = 300,
	._fsrc.pwm_src.low_load = 100,/*low level*/
	._fsrc.pwm_src.high_load = 300,/*high level*/
	._fsrc.pwm_src.channel = 0,/*chanel id -> gpio num 24*/
};

struct msm_camera_sensor_flash_data hw_camera_led_data = 
{	
	.flash_type = MSM_CAMERA_FLASH_LED,
	.flash_src = & msm_camera_led_src,
};

static int hw_camera_led_open(struct inode *inode,struct file *file)
{	
	int ret = 0;
	
	CDBG("function %s enterence\n",__func__);
	if(atomic_read(&flag)) ret = -1;
	else atomic_set(&flag,1);

	return ret;
}

static long hw_camera_led_ioctl(struct file *filep ,unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	unsigned int camera_led_state;
	
	CDBG("function %s enterence\n",__func__);
	switch(cmd)
	{
		case CAMERA_LED_GET :
			
			camera_led_state = atomic_read(&camera_led_flag);
			if(copy_to_user((void __user *)arg,&camera_led_state,sizeof(camera_led_state))) 
			{
				pr_err("function copy_to_user fail");
				ret = -EFAULT;
			}
			break;
		case CAMERA_LED_SET :
			if(copy_from_user(&camera_led_state,(void __user *)arg,sizeof(camera_led_state)))
			{
				pr_err("function copy_from_user fail");
				ret = -EFAULT;
			}
			else
			{
				/*these handset use tps61310 as flash*/
				if( machine_is_msm8x25_U8825()
				|| machine_is_msm8x25_U8825D()
                || machine_is_msm8x25_U8833D()
                || machine_is_msm8x25_U8833()                
				|| machine_is_msm8x25_C8825D()
				|| machine_is_msm8x25_C8950D()
				|| machine_is_msm8x25_U8950D()
				|| machine_is_msm8x25_U8951()
                || machine_is_msm8x25_C8813()
                || machine_is_msm8x25_H881C()
				||machine_is_msm8x25_U8950())
				{
			
					tps61310_set_flash(camera_led_state);
				}
				else
				{
					ret = msm_camera_flash_set_led_state(&hw_camera_led_data, camera_led_state);
				}
				
				if(!ret)
				{
					atomic_set(&camera_led_flag,camera_led_state);
				}
			}
			break;
		default:
			pr_err("hw_camera_led_ioctl:error ioctl cmd");
			ret = -EINVAL;
		
	}
	
	return ret;
}

static int hw_camera_led_release(struct inode *inode,struct file *file)
{
	int ret = 0;

	CDBG("function %s enterence\n",__func__);
	if(atomic_read(&flag)) atomic_set(&flag,0);
	else ret = -1;
	
	return ret;
}


static int __init hw_camera_led_init(void)
{
	int ret;
	CDBG("init %s start\n",__func__);

	ret = misc_register(&hw_camera_led_device);
	if(ret)
	{	
		pr_err("%s:Unable to register misc device.\n",__func__);
	}	

	return ret;
}

static void __exit hw_camera_led_exit(void)
{
	 CDBG("function %s enterence\n",__func__);
	 misc_deregister(&hw_camera_led_device);
}

module_init(hw_camera_led_init);
module_exit(hw_camera_led_exit);

