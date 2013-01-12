/* mtk6252.c
 *
 * mtk6252 Management Routines, mtk6252 as a modem of system
 *
 * Copyright (c) 2012-2015 Huawei. All rights reserved.
 * Author: someone
 * Email:   someone@huawei.com
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
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/vmalloc.h>
#include <linux/wakelock.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/pm.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/hardware_self_adapt.h>
#include <linux/mtk6252_dev.h>
#include <asm/processor.h>
#include <asm-arm/huawei/usb_switch_huawei.h>

/********************************************************************************
* structure define
********************************************************************************/
struct mtk6252_driver{
	unsigned gpio_pwron;
	unsigned gpio_reset;
	unsigned gpio_pwrstat;
	unsigned gpio_softwarestate;
	unsigned gpio_download_en;
	int irq_softwarestate;								/*from gpio status*/
	int irq_enabled; /* if irq enabled*/
	unsigned gpio_usb_sel;
  unsigned gpio_sim_swap;
	
	atomic_t mtk_modem_softwarestate;	/* store the gpio status's value */

    /* wakelock */
	struct wake_lock mtk_irq_wakelock;

	/* work */
	struct work_struct  work_irq_softwarestate;
	struct work_struct  work_start;

	/* mutex */
	struct mutex dev_lock;
	spinlock_t		lock;

	wait_queue_head_t monitor;  /* monitor mtk modem's software state */

	int mtk_start; /* mtk is start or not, 0--not start, 1--yes*/

};

/********************************************************************************
* macro define
********************************************************************************/
#define MTK_MODEM_POLL_PWRON_TIMES			20		/* 20 times */
#define MTK_MODEM_POLL_PWRON_INTERVAL	100	/* 100ms */
#define MTK_MODEM_POLL_PWRON_WAITTING_TIME	1000	/* 1000ms */
#define MTK_MODEM_POLL_PWRON_PLUS_TIME			100		/* 100ms */
#define MTK_MODEM_POLL_PWROFF_TIMES		50	/* 50 times */
#define MTK_MODEM_POLL_PWROFF_INTERVAL	100	/* 100ms */
#define MTK_MODEM_PWRON_TIME		100					/* power on, 100ms */
#define MTK_MODEM_DOWNLOAD_EN_TIME		100		/* download en, 100ms */
#define MTK_MODEM_USB_SEL_TIME		100		/* download en, 100ms */
#define MTK_MODEM_RESET_TIME			1000					/* reset, 1000ms */
#define MTK_MODEM_RESET_WAITTING_TIME			100					/* reset, wait stabilization, 100ms */
#define MTK_MODEM_LONGPRESS_TIME			3000	/* reset, 3000ms */

#define MTK_MODEM_ABNORMAL_HANDLE_TIME (HZ * 5)	/* wakelock 5s to waiting for handling */
#define STRING_CMP(buf, str) memcmp(buf, str, strlen(str))

#define GPIO_HIGH_LEVEL   1
#define GPIO_LOW_LEVEL    0
#define MTK_MODEM_CHECK_GPIO_STATES			10		/* 10ms */

/********************************************************************************
* data define
********************************************************************************/
static struct  mtk6252_driver *gdriver = NULL;

/********************************************************************************
* function define
********************************************************************************/
/******************************************************************************
  Function: 			mtk6252_get_softwarestate
  Description:		get pin's state
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0--abnormal or inactive, 1--normal
  Others:				
******************************************************************************/
static int mtk6252_get_softwarestate(void)
{
    /*no need to pull low the ap_wakeup_modem pin*/
	if ((!gdriver) || !(gdriver->gpio_softwarestate)) {
		printk(KERN_ERR "%s gdriver\n", __func__);
		return 0;
	}
	/* get state of MODEM_SOFTWARE_STATE */
	return gpio_get_value(gdriver->gpio_softwarestate);
}

/******************************************************************************
  Function: 			mtk6252_get_pwrstat
  Description:		get pin's state
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0--powerdown, 1--active
  Others:				
******************************************************************************/
static int mtk6252_get_pwrstat(void)
{
    /*no need to pull low the ap_wakeup_modem pin*/
	if ((!gdriver) || !(gdriver->gpio_pwrstat)) {
		printk(KERN_ERR "%s gdriver\n", __func__);
		return 0;
	}
	/* get state of GPIO_MODEM_PWRSTAT */
	return gpio_get_value(gdriver->gpio_pwrstat);
}

/******************************************************************************
  Function: 			mtk6252_get_download_en
  Description:		get pin's state
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0--disable, 1--enable
  Others:				
******************************************************************************/
static int mtk6252_get_download_en(void)
{
	if ((!gdriver) || !(gdriver->gpio_download_en)) {
		printk(KERN_ERR "%s gdriver\n", __func__);
		return 0;
	}
	/* get state of GPIO_MODEM_DOWNLOAD_EN */
	return gpio_get_value(gdriver->gpio_download_en);
}

/* delete 20 lines, get gpio status directly */

/******************************************************************************
  Function: 			mtk6252_reset_mtk
  Description:		reset mtk, go into shutdown
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_reset_mtk(void)
{
	if ((!gdriver) || !(gdriver->gpio_reset)) {
		printk(KERN_ERR "%s gdriver\n", __func__);
		return;
	}
	/* be sure pwrkey is off */
	/* output high, wait for 200ms, then output low */
	gpio_set_value(gdriver->gpio_reset, 1);
	msleep(MTK_MODEM_RESET_TIME); 
	gpio_set_value(gdriver->gpio_reset, 0);
	msleep(MTK_MODEM_RESET_WAITTING_TIME); 	/* wait for power off*/
	gdriver->mtk_start = 0;
}

/******************************************************************************
  Function: 			mtk6252_usb_sel_ap
  Description:		    switch usb to ap side
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_usb_sel_ap(void)
{    
    int gpio_value = 0;
    
    gpio_value = gpio_get_value(gdriver->gpio_usb_sel);
    if (!gpio_value){
        printk(KERN_INFO "%s: switch blocked for usb is already in qcom side\n", __func__);
        return;
    }
        
    /* pull GPIO_USB_SEL low to swtich usb to qcom side */    	
    printk(KERN_INFO "%s: switch usb to qcom side\n", __func__);
    gpio_set_value(gdriver->gpio_usb_sel, 0);
	msleep(MTK_MODEM_USB_SEL_TIME);  
    
    /* force reenumeration for qcom usb */
    android_usb_force_reset();       
}

/******************************************************************************
  Function: 			mtk6252_usb_sel_mtk
  Description:		    switch usb to mtk side
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_usb_sel_mtk(void)
{        
    /* pull GPIO_USB_SEL low to swtich usb to mtk side */        
    printk(KERN_INFO "%s: switch usb to mtk side\n", __func__);
    gpio_set_value(gdriver->gpio_usb_sel, 1);
    msleep(MTK_MODEM_USB_SEL_TIME);  
}

/******************************************************************************
  Function: 			mtk6252_download_enable
  Description:		enable mtk in download mode
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_download_enable(void)
{
	if ((!gdriver) || !(gdriver->gpio_download_en)) {
		printk(KERN_ERR "%s gdriver\n", __func__);
		return;
	}
	/* output high wait for 50ms at least */
	gpio_set_value(gdriver->gpio_download_en, 1);
	msleep(MTK_MODEM_DOWNLOAD_EN_TIME);  
}

/******************************************************************************
  Function: 			mtk6252_download_disable
  Description:		disable mtk in download mode
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_download_disable(void)
{
	if ((!gdriver) || !(gdriver->gpio_download_en)) {
		printk(KERN_ERR "%s gdriver\n", __func__);
		return;
	}
	/* output low wait for 50ms at least */
	gpio_set_value(gdriver->gpio_download_en, 0);
	msleep(MTK_MODEM_DOWNLOAD_EN_TIME);  
}

/******************************************************************************
  Function: 			mtk6252_poweron_high
  Description:		put power-on pin high
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_poweron_high(void)
{
	if ((!gdriver) || !(gdriver->gpio_pwron)) {
		printk(KERN_ERR "%s gdriver\n", __func__);
		return;
	}
	/* output high wait for 50ms at least */
	gpio_set_value(gdriver->gpio_pwron, 1);
}

/******************************************************************************
  Function: 			mtk6252_poweron_low
  Description:		put power-on pin low
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_poweron_low(void)
{
	if ((!gdriver) || !(gdriver->gpio_pwron)) {
		printk(KERN_ERR "%s gdriver\n", __func__);
		return;
	}
	/* output high wait for 50ms at least */
	gpio_set_value(gdriver->gpio_pwron, 0);
}

/******************************************************************************
  Function: 			mtk6252_poweron_nocheck
  Description:		power on mtk, not check mtk's state
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_poweron_nocheck(void)
{
	mtk6252_poweron_high();
	msleep(MTK_MODEM_POLL_PWRON_PLUS_TIME); /* wait 100 ms for power on */
	mtk6252_poweron_low();
}

/*remove the mtk6252_poweroff_bylongpresspwrkey function, because of useless*/

/******************************************************************************
  Function: 			mtk6252_poweron_mtkmodem
  Description:		power on mtk
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0 -- ok, else fail
  Others:				
******************************************************************************/
static int mtk6252_poweron_mtkmodem(void)
{
	int times = MTK_MODEM_POLL_PWRON_TIMES; 

	/* press power key */
	mtk6252_poweron_high();

	msleep(MTK_MODEM_POLL_PWRON_WAITTING_TIME);	/* 1s for mtk ready */

	while (times--) {
		/* need mtk give a ok signal */
		if (mtk6252_get_pwrstat()) {
			/* release power key */
			mtk6252_poweron_low();
			pr_debug("%s power on success, return \n", __func__);
			return 0;	/* return sucess, active */
		}
		msleep(MTK_MODEM_POLL_PWRON_INTERVAL);	/* check each some-time, such as 100ms */
		printk(KERN_INFO "%s power on failed, retry times %d \n", __func__, times);
	}

	/* release power key */
	mtk6252_poweron_low();
	printk(KERN_ERR"%s power on failed, return \n", __func__);
	return -1;
}

/*remove the mtk_config_gpio function, because of useless*/

/******************************************************************************
  Function: 			mtk6252_power_off
  Description:		power off mtk
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_power_off(void)
{
	int timeout = MTK_MODEM_POLL_PWROFF_TIMES; /* 2 seconds, temp */

	printk(KERN_INFO"%s enter\n", __func__);
    /*remove mtk_config_gpio() function, becaue GPIO 35 cause pwrstate incorrect,
      so set it low before check power states everytime in mtk6252_get_pwrstat()*/

	/* check status of mtk6252 GPIO_MODEM_PWRSTAT, if not shutdown, wait for 5 seconds for at+EPOF*/
	/* if at failed before, here maybe only waste time */
	while (timeout--) {
        /*ap_wakeup_modem pin will affect the status of gpio_pwrstat,
        so set it to low before read the status of gpio_pwrstat*/
        gpio_set_value(GPIO_MSM_WAKE_MTK, GPIO_LOW_LEVEL);
		if (0==mtk6252_get_pwrstat()) {
			printk(KERN_INFO "%s at+EPOF shut down ok \n", __func__);
			/*remove 1 line.if mtk have been powered off, exit this function,do not need to reset again*/
			return;
		}
		msleep(MTK_MODEM_POLL_PWROFF_INTERVAL);	/* check each 100 ms */
	}

    /*remove 12 lines. mtk can not support long press key power off, so remove this operation*/
	printk(KERN_ERR "%s at+EPOF shut down fail,need to force to shut down \n", __func__);

    /* force power off, reset mtk */
	mtk6252_reset_mtk();
    /*ap_wakeup_modem pin will affect the status of gpio_pwrstat,
      so set it to low before read the status of gpio_pwrstat*/
    gpio_set_value(GPIO_MSM_WAKE_MTK, GPIO_LOW_LEVEL);
    msleep(MTK_MODEM_CHECK_GPIO_STATES);
	if (0==mtk6252_get_pwrstat()) 
		printk(KERN_INFO "%s reset suc \n", __func__);
	else
		printk(KERN_ERR"%s reset fail \n", __func__);		
}

/******************************************************************************
  Function: 			mtk6252_power_on
  Description:		power on mtk
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0 --ok, else fail
  Others:				
******************************************************************************/
static int mtk6252_power_on(void)
{
	pr_debug("%s enter\n", __func__);

	/* power on mtk modem */
	if (!mtk6252_poweron_mtkmodem()) {
		gdriver->mtk_start = 1;
		printk(KERN_INFO "%s power on success, return \n", __func__);
		return 0;	/* return sucess, active */
	}
	gdriver->mtk_start = 0;

	/* power on fail, return -1 */
	printk(KERN_ERR "%s mtk power on fail \n", __func__);
	return -1;	/* fail */
}

/******************************************************************************
  Function: 			mtk6252_power_on_force
  Description:		force power on mtk
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_power_on_force(void)
{
	/* force mtk power down at first */
	mtk6252_reset_mtk();
	/* disable download */
	mtk6252_download_disable();
	/* then power on */
	(void)mtk6252_power_on();
}

/******************************************************************************
  Function: 			mtk6252_disable_softwarestate_irq
  Description:		disable softwarestate pin's irq
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_disable_softwarestate_irq(void)
{
	unsigned long flags = 0;
	if ((!gdriver) || !(gdriver->irq_softwarestate)) {
		printk(KERN_ERR "%s gdriver\n", __func__);
		return ;
	}
	spin_lock_irqsave(&gdriver->lock, flags);
	if (gdriver->irq_enabled) {
		disable_irq_nosync(gdriver->irq_softwarestate);
		gdriver->irq_enabled = 0;
	}
	spin_unlock_irqrestore(&gdriver->lock, flags);
}

/******************************************************************************
  Function: 			mtk6252_enable_softwarestate_irq
  Description:		enable softwarestate pin's irq
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_enable_softwarestate_irq(void)
{
	unsigned long flags = 0;
	if ((!gdriver) || !(gdriver->irq_softwarestate)) {
		printk(KERN_ERR "%s gdriver\n", __func__);
		return ;
	}
	spin_lock_irqsave(&gdriver->lock, flags);
	if (!gdriver->irq_enabled) {
		enable_irq(gdriver->irq_softwarestate);
		gdriver->irq_enabled = 1;
	}
	spin_unlock_irqrestore(&gdriver->lock, flags);
}

/******************************************************************************
  Function: 			do_mtk6252_download_mode
  Description:		order mtk enter download mode
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void do_mtk6252_download_mode(void)
{
	/* disable irq */
	mtk6252_disable_softwarestate_irq();
	/* shut down */
	mtk6252_power_off();
	/* reset again,  ensure power down*/
	mtk6252_reset_mtk();
	msleep(MTK_MODEM_RESET_WAITTING_TIME);  /* stabilization */
	/* download en */
	mtk6252_download_enable();
	/* power on */
	(void)mtk6252_power_on();
}

/******************************************************************************
  Function: 			do_mtk6252_download_mode_quick
  Description:		order mtk enter download mode quickly and switch usb to mtk at same time
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void do_mtk6252_download_mode_quick(void)
{
	/* disable irq */
	mtk6252_disable_softwarestate_irq();
	/* reset */
	mtk6252_reset_mtk();
	/* download en */
	mtk6252_download_enable();
	/* usb switch */
	mtk6252_usb_sel_mtk();
	/* power on, need optimization */
	//(void)mtk6252_power_on();
	mtk6252_poweron_nocheck();
}

/******************************************************************************
  Function: 			do_mtk6252_power_off
  Description:		power off mtk
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void do_mtk6252_power_off(void)
{
	/* check status of mtk6252 GPIO_MODEM_PWRSTAT, if off, return 0 */
	if (!mtk6252_get_pwrstat()) {
		printk(KERN_INFO "%s already power off, return \n", __func__);
		return ;
	}
	/* disable irq */
	mtk6252_disable_softwarestate_irq();
	/* shut down */
	mtk6252_power_off();
}

/******************************************************************************
  Function: 			do_mtk6252_power_on
  Description:		power on mtk
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void do_mtk6252_power_on(void)
{
	/* check status of mtk6252 GPIO_MODEM_PWRSTAT, if on, return 0 */
	if (mtk6252_get_pwrstat()) {
		printk(KERN_INFO "%s already power on, return \n", __func__);
		return ; /* return, already active */
	}
	/* disable download */
	mtk6252_download_disable();
	/* power on */
	(void)mtk6252_power_on();
	/* enable irq */
	mtk6252_enable_softwarestate_irq();
}

/******************************************************************************
  Function: 			do_mtk6252_power_on_force
  Description:		force power on mtk
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void do_mtk6252_power_on_force(void)
{
	/* check status of mtk6252 GPIO_MODEM_PWRSTAT, if on, return 0 */
	if (mtk6252_get_pwrstat()) {
		printk(KERN_INFO "%s already power on, return \n", __func__);
		return ; /* return, already active */
	}
	/* disable irq */
	mtk6252_disable_softwarestate_irq();
	/* power on */
	(void)mtk6252_power_on_force();
	/* enable irq */
	mtk6252_enable_softwarestate_irq();
}

/******************************************************************************
  Function: 			do_mtk6252_power_off_force
  Description:		force power off mtk
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void do_mtk6252_power_off_force(void)
{
	/* disable irq */
	mtk6252_disable_softwarestate_irq();
	/* force mtk power down at first */
	mtk6252_reset_mtk();
}

/********************************************************************************
* sysfs begin
********************************************************************************/
/******************************************************************************
  Function: 			mtk6252_usb_sel_show
  Description:		    get usb select satus
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static ssize_t mtk6252_usb_sel_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
    int gpio_value = 0;
    ssize_t ret =0;
    
	if ((!gdriver) || !(gdriver->gpio_usb_sel)) {
		printk(KERN_ERR "%s gdriver\n", __func__);
		return 0;
	}

	/* get gpio stauts */
	mutex_lock(&gdriver->dev_lock);
    gpio_value = gpio_get_value(gdriver->gpio_usb_sel);
	printk(KERN_INFO "%s : %d\n", __func__, gpio_value);
    ret = snprintf(buf, PAGE_SIZE, "%d\n", gpio_value);
    mutex_unlock(&gdriver->dev_lock);
    return ret;     
}

/******************************************************************************
  Function: 			mtk6252_usb_sel_store
  Description:		    usb switch to ap or mtk
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				"0" - switch usb to qcom 
                        "1" - switch usb to usb
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static ssize_t mtk6252_usb_sel_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{   
    int input_value = 0;
    
	if (!buf || !count) {
		printk(KERN_ERR "%s bad args\n", __func__);
		return -EINVAL;
	}
	
	if ((!gdriver) || !(gdriver->gpio_usb_sel)) {
		printk(KERN_ERR "%s gdriver\n", __func__);
		return 0;
	}

	mutex_lock(&gdriver->dev_lock);
	
    sscanf(buf, "%d\n", &input_value);   
    printk(KERN_INFO "%s: input_value = %d\n", __func__, input_value);
      
    if (input_value) { //switch usb to mtk side
        mtk6252_usb_sel_mtk();
    } else { //switch usb to qcom side
        mtk6252_usb_sel_ap();
    }
    
	mutex_unlock(&gdriver->dev_lock);
	return count;
}

/******************************************************************************
  Function: 			mtk6252_download_mode_show
  Description:		no permission
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static ssize_t mtk6252_download_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int state = 0;
	ssize_t ret = 0;

	mutex_lock(&gdriver->dev_lock);
	state = mtk6252_get_download_en();
	printk(KERN_INFO "%s : %d\n", __func__, state);
	if (state) 
		ret = sprintf(buf, "%s\n", "enable");
	else
		ret = sprintf(buf, "%s\n", "disable");
	mutex_unlock(&gdriver->dev_lock);
	return ret;
}

/******************************************************************************
  Function: 			mtk6252_download_mode_store
  Description:		download mode operation
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				"enabel" or "disable" or "enter"
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static ssize_t mtk6252_download_mode_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	mutex_lock(&gdriver->dev_lock);
	if (!buf || !count) {
		printk(KERN_ERR "%s bad args\n", __func__);
		mutex_unlock(&gdriver->dev_lock);
		return -EINVAL;
	}

	printk(KERN_INFO "%s args: %s\n", __func__, buf);

	if (0 == STRING_CMP(buf, "enable")) {
		mtk6252_download_enable();
	} else if (0 == STRING_CMP(buf, "disable")) {
		mtk6252_download_disable();
	} else if (0 == STRING_CMP(buf, "enter")) {
		do_mtk6252_download_mode();
	} else if (0 == STRING_CMP(buf, "quickenter")){
		do_mtk6252_download_mode_quick();
	} else {
		printk(KERN_ERR "%s bad args: %s\n", __func__, buf);
		mutex_unlock(&gdriver->dev_lock);
		return -EINVAL;		
	}
	mutex_unlock(&gdriver->dev_lock);
	return count;
}

/******************************************************************************
  Function: 			mtk6252_onoff_show
  Description:		no permission
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static ssize_t mtk6252_onoff_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	mutex_lock(&gdriver->dev_lock);
	printk(KERN_INFO "%s : %d\n", __func__, gdriver->mtk_start);
	if (gdriver->mtk_start) 
		ret = sprintf(buf, "%s\n", "on");
	else
		ret = sprintf(buf, "%s\n", "off");
	mutex_unlock(&gdriver->dev_lock);
	return ret;
}

/******************************************************************************
  Function: 			mtk6252_onoff_store
  Description:		mtk power on off operation
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				"on" "off" "forceon" "forceoff"
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static ssize_t mtk6252_onoff_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	mutex_lock(&gdriver->dev_lock);
	if (!buf || !count) {
		printk(KERN_ERR "%s bad args\n", __func__);
		mutex_unlock(&gdriver->dev_lock);
		return -EINVAL;
	}

	printk(KERN_INFO "%s args: %s\n", __func__, buf);

	if (0 == STRING_CMP(buf, "on")) {
		do_mtk6252_power_on();
	} else if (0 == STRING_CMP(buf, "off")) {
		do_mtk6252_power_off();
	} else  if (0 == STRING_CMP(buf, "forceon")) {
		do_mtk6252_power_on_force();
	} else  if (0 == STRING_CMP(buf, "forceoff")) {
		do_mtk6252_power_off_force();
	} else {
		printk(KERN_ERR "%s bad args: %s\n", __func__, buf);
		mutex_unlock(&gdriver->dev_lock);
		return -EINVAL;		
	}
	mutex_unlock(&gdriver->dev_lock);
	return count;
}

/******************************************************************************
  Function: 			mtk6252_softwarestate_show
  Description:		get softwarestate pin's state
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static ssize_t mtk6252_softwarestate_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int state = 0;

	mutex_lock(&gdriver->dev_lock);
	state = mtk6252_get_softwarestate();
	printk(KERN_INFO "%s : %d\n", __func__, state);
	mutex_unlock(&gdriver->dev_lock);
	return sprintf(buf, "%d\n", state);
}

/******************************************************************************
  Function: 			mtk6252_softwarestate_store
  Description:		no permission
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static ssize_t mtk6252_softwarestate_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	/* Status is RO so we shouldn't do anything if the user
	app writes to the sysfs file. */
	printk(KERN_ERR "%s not alow\n", __func__);
	return -EPERM;
}

/******************************************************************************
  Function: 			mtk6252_pwrstat_show
  Description:		get pwrstat pin's state
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static ssize_t mtk6252_pwrstat_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int state = 0;

	mutex_lock(&gdriver->dev_lock);
	state = mtk6252_get_pwrstat();
	printk(KERN_INFO "%s : %d\n", __func__, state);
	mutex_unlock(&gdriver->dev_lock);
	return sprintf(buf, "%d\n", state);
}

/******************************************************************************
  Function: 			mtk6252_pwrstat_store
  Description:		no permission
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static ssize_t mtk6252_pwrstat_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	/* Status is RO so we shouldn't do anything if the user
	app writes to the sysfs file. */
	printk(KERN_ERR "%s not alow\n", __func__);
	return -EPERM;
}

/******************************************************************************
  Function: 			mtk6252_get_simswap_status
  Description:		get pin's state
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0--disable, 1--enable
  Others:				
******************************************************************************/
static int mtk6252_get_simswap_status(void)
{
  if ((!gdriver) || !(gdriver->gpio_sim_swap)) 
  {
    printk(KERN_ERR "%s gdriver\n", __func__);
    return 0;
  }
  /* get state of GPIO_MODEM_DOWNLOAD_EN */
  return gpio_get_value(gdriver->gpio_sim_swap);
}

/******************************************************************************
  Function: 			mtk6252_simswap_show
  Description:		no permission
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static ssize_t mtk6252_simswap_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int state = 0;
	ssize_t ret = 0;

	mutex_lock(&gdriver->dev_lock);
	state = mtk6252_get_simswap_status();
	printk(KERN_INFO "%s : %d\n", __func__, state);
	if (state) 
		ret = sprintf(buf, "%s\n", "disable");
	else
		ret = sprintf(buf, "%s\n", "enable");
	mutex_unlock(&gdriver->dev_lock);
	return ret;
}

/******************************************************************************
  Function: 			mtk6252_simswap_disable
  Description:		disable the sim swap
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static void mtk6252_simswap_disable(void)
{
  pr_debug("%s enter\n", __func__);
  if ((!gdriver) || !(gdriver->gpio_sim_swap)) 
  {
    printk(KERN_ERR "%s gdriver\n", __func__);
    return;
  }
  printk(KERN_ERR "dis gpio_sim_swap %d \n", gdriver->gpio_sim_swap);
  /* output high to set to default dual sim connection */
  gpio_set_value(gdriver->gpio_sim_swap, 1);
}

/******************************************************************************
  Function: 			mtk6252_simswap_enable
  Description:		enable the sim swap
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static void mtk6252_simswap_enable(void)
{
  pr_debug("%s enter\n", __func__);
  if ((!gdriver) || !(gdriver->gpio_sim_swap)) 
  {
    printk(KERN_ERR "%s gdriver\n", __func__);
    return;
  }
  
  printk(KERN_ERR "enable gpio_sim_swap %d \n", gdriver->gpio_sim_swap);
  /* output low to swap the dual sim */
  gpio_set_value(gdriver->gpio_sim_swap, 0);
}

/******************************************************************************
  Function: 			mtk6252_simswap_store
  Description:		sim swap operation
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				"enabel" or "disable"
  Output:				
  Return:				ssize_t
  Others:				
******************************************************************************/
static ssize_t mtk6252_simswap_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
  mutex_lock(&gdriver->dev_lock);
  if (!buf || !count) 
  {
    printk(KERN_ERR "%s bad args\n", __func__);
    mutex_unlock(&gdriver->dev_lock);
    return -EINVAL;
  }

  printk(KERN_INFO "%s args: %s\n", __func__, buf);

  if (0 == STRING_CMP(buf, "enable")) 
  {
    mtk6252_simswap_enable();
  } 
  else if (0 == STRING_CMP(buf, "disable")) 
  {
    mtk6252_simswap_disable();
  } 
  else 
  {
    printk(KERN_ERR "%s bad args: %s\n", __func__, buf);
    mutex_unlock(&gdriver->dev_lock);
    return -EINVAL;		
  }
  mutex_unlock(&gdriver->dev_lock);
  return count;
}

DEVICE_ATTR(usb_sel, 0664, mtk6252_usb_sel_show, mtk6252_usb_sel_store);  /* WR attr */
DEVICE_ATTR(download_mode, 0664, mtk6252_download_mode_show, mtk6252_download_mode_store);  /* WR attr */
DEVICE_ATTR(onoff,  0664, mtk6252_onoff_show,  mtk6252_onoff_store);     /* WO attr */
DEVICE_ATTR(softwarestate, 0444, mtk6252_softwarestate_show, mtk6252_softwarestate_store);  /* RO attr */
DEVICE_ATTR(pwrstat, 0444, mtk6252_pwrstat_show, mtk6252_pwrstat_store);  /* RO attr */
DEVICE_ATTR(simswap, 0664, mtk6252_simswap_show, mtk6252_simswap_store);  /* WR attr */

/* sysfs's sttrabition array */
static struct device_attribute *mtk_attrs[] = {
	&dev_attr_usb_sel,
	&dev_attr_download_mode,
	&dev_attr_onoff,
	&dev_attr_softwarestate,
	&dev_attr_pwrstat,
    &dev_attr_simswap,
	NULL,
};

/******************************************************************************
  Function: 			mtk6252_remove_sysfs
  Description:		remove sysfs
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_remove_sysfs(struct platform_device *pdev)
{
	struct device_attribute **attr = NULL;

	for (attr = mtk_attrs; *attr; ++attr)
		device_remove_file(&pdev->dev, *attr);
}

/******************************************************************************
  Function: 			mtk6252_create_sysfs
  Description:		create sysfs
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0--ok, else fail
  Others:				
******************************************************************************/
static int mtk6252_create_sysfs(struct platform_device *pdev)
{
	int ret= 0;
	struct device_attribute **attr = NULL;

	/* create sysfs*/
	for (attr = mtk_attrs; *attr; ++attr) {
		ret = device_create_file(&pdev->dev, *attr);
		if (ret) {
			printk(KERN_ERR "%s: Failed to create sysfs file: %s/%s\n", 
							__func__, pdev->name,  (*attr)->attr.name);
			goto rem_attr;
		}
	}

	return 0;

rem_attr:
	for (attr = mtk_attrs; *attr; ++attr)
		device_remove_file(&pdev->dev, *attr);

	return -ENOMEM;
}

/********************************************************************************
* start
********************************************************************************/
/******************************************************************************
  Function: 			mtk6252_can_poweron
  Description:		check if or not to power mtk
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0--no need, 1-yes
  Others:				not need to power-on when boot reason is powerdown_charging or alarm-rtc boot
******************************************************************************/
static int mtk6252_can_poweron(void)
{
	/* if power-on reason is powerdown-charging,return 0, else 1 */
#ifdef CONFIG_HUAWEI_POWER_DOWN_CHARGE
	if(get_charge_flag())
		return 0; /* not need to start mtk modem */
#endif
    /* Delete 3 lines, RTC alarm also need power on mtk modem */
	return 1;		/* start mtk modem */
}

static int is_swtype_factory;
static int __init androidboot_swtype(char *str)
{
	if (0==strncmp(str, "factory", strlen("factory")))
		is_swtype_factory = 1;
	else
		is_swtype_factory = 0;
	return 1;
}
__setup("androidboot.swtype=", androidboot_swtype);

/******************************************************************************
  Function: 			mtk6252_poweron_start
  Description:		power on mtk
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0--ok, else fail
  Others:				
******************************************************************************/
static int mtk6252_poweron_start(void)
{
	/* if start mtk6252 in oemsbl, please return */
#ifdef HUAWEI_FEATURE_MTKBOOT_AT_OEMSBL
	return 0;
#endif

  /* delete 8 lines code.*/ 
  /*in factory test, do not need to power on mtk base on pwrstates pin status*/

	/* firstly, check mtk modem's state, if has on, force shutdown */
	if (mtk6252_get_pwrstat()) {
		printk(KERN_WARNING "%s mtk maybe not shutdown before, reset it \n", __func__);
		mtk6252_reset_mtk();
	}

	/* start mtk6252 modem according the power-on reason */
	if (mtk6252_can_poweron()) {
		/*disable irq */
		mtk6252_disable_softwarestate_irq();
		/*power on */
		if (mtk6252_power_on()) {
			printk(KERN_ERR "%s mtk power on fail \n", __func__);
			return -EIO;	/* power fail */
		}
		printk(KERN_INFO"%s mtk power on ok \n", __func__);
		/*enable irq */
		mtk6252_enable_softwarestate_irq();
		return 0;
	}

	printk(KERN_INFO "%s not need to start mtk modem \n", __func__);
	return 0;	/* not need power  */
}

/******************************************************************************
  Function: 			mtk6252_work_start_func
  Description:		the work to start mtk
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_work_start_func(struct work_struct *work)
{
	printk(KERN_INFO "%s  \n", __func__);
	/* power on mtk modem */
	if (mtk6252_poweron_start()) {
		printk(KERN_ERR "%s: power on mtk failed\n", __func__);
	} 
}

/********************************************************************************
* misc device
********************************************************************************/
/******************************************************************************
  Function: 			mtk6252_work_irq_softwarestate_func
  Description:		the work to handler softwarestate irq
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				mtk abnormal
******************************************************************************/
static void mtk6252_work_irq_softwarestate_func(struct work_struct *work)
{
	printk(KERN_INFO "%s  \n", __func__);
	if (gdriver) {
		mutex_lock(&gdriver->dev_lock);
		/* set mkt_modem_softwarestate to notify abnormal happened */
		atomic_set(&gdriver->mtk_modem_softwarestate, 0);
		mutex_unlock(&gdriver->dev_lock);
		/* wake up monitor */
		wake_up_interruptible(&gdriver->monitor);
	}
}

/* comment interrupt handler temperary */
/******************************************************************************
  Function: 			mtk6252_irq_softewarestate_handler
  Description:		the irq handler of softwarestate pin
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				irqreturn_t: IRQ_HANDLED
  Others:				
******************************************************************************/
static irqreturn_t mtk6252_irq_softewarestate_handler(int irq, void *dev_id)
{
	struct mtk6252_driver *priv = dev_id;

	//printk(KERN_INFO "%s  \n", __func__);
	/* remove spin_lock according to the reviwer's opinion */
	/* spin_lock(&priv->lock); */
	/* only once */
	if (priv->irq_enabled) {
		disable_irq_nosync(priv->irq_softwarestate);
		priv->irq_enabled = 0;
	}
	if (priv && priv->mtk_start) {		
		wake_lock_timeout(&priv->mtk_irq_wakelock, MTK_MODEM_ABNORMAL_HANDLE_TIME); 
		schedule_work(&priv->work_irq_softwarestate);
	}
	/* spin_unlock(&priv->lock); */
	return IRQ_HANDLED;
}

/******************************************************************************
  Function: 			mtk6252_open
  Description:		misc device's open function
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0
  Others:				not handle
******************************************************************************/
static int mtk6252_open(struct inode *inode, struct file *file)
{
	return 0;
}

/******************************************************************************
  Function: 			mtk6252_release
  Description:		misc device's release function
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0
  Others:				not handle
******************************************************************************/
static int mtk6252_release(struct inode *inode, struct file *file)
{
	return 0;	
}

/******************************************************************************
  Function: 			mtk6252_ioctl
  Description:		misc device's ioctl function
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				long: 
  Others:				mainly used to monitor mtk's state
******************************************************************************/
static long mtk6252_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int state = 0;
	int ret = 0;

	mutex_lock(&gdriver->dev_lock);

	/* check arguments */
	if (!argp) {
		ret = -EINVAL;
		goto result;
	}

	if (!gdriver) {
		ret = -EINVAL;
		goto result;
	}

	printk(KERN_INFO "%s cmd = 0x%x\n", __func__, cmd);

	switch (cmd) 
	{
	case MTKMODEM_MONITOR_STATE:
		/* if mtk modem has shutdown, return 0 */
        /*get the mtk power states*/
        state = mtk6252_get_pwrstat();
		if (!state) {
			state = MTKMODEM_MONITOR_STATE_SHUTDOWN;
			if (copy_to_user(argp, &state, sizeof(state))) {
				printk(KERN_ERR "%s copy_to_user fail\n", __func__);
				ret = -EFAULT;
			}
			printk(KERN_INFO"%s state=%d \n", __func__, state);
			break;
		}
		mutex_unlock(&gdriver->dev_lock);
		/* monitor state change */
		atomic_set(&gdriver->mtk_modem_softwarestate, 1);
		if (wait_event_interruptible(gdriver->monitor, atomic_read(&gdriver->mtk_modem_softwarestate) == 0))
			return -ERESTARTSYS;  /* signal: tell the fs layer to handle it */
		mutex_lock(&gdriver->dev_lock);
		/* notify the user */
		state = MTKMODEM_MONITOR_STATE_ABNORMAL;
		if (copy_to_user(argp, &state, sizeof(state))) {
			printk(KERN_ERR "%s copy_to_user fail\n", __func__);
			ret = -EFAULT;
		}
		printk(KERN_INFO"%s state=%d \n", __func__, state);
		break;

	default:
		printk(KERN_ERR "%s\n unknown cmd %d\n", __func__, cmd);
		ret = -EINVAL;
		break;
	}
result:
	mutex_unlock(&gdriver->dev_lock);
	return ret;
}

/* misc device's ops */
static struct file_operations mtk6252_fops = {
	.owner = THIS_MODULE,
	.open = mtk6252_open,
	.release = mtk6252_release,
	.unlocked_ioctl = mtk6252_ioctl,
};

/* misc device */
static struct miscdevice mtk6252_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mtk6252_dev",
	.fops = &mtk6252_fops,
};

/******************************************************************************
  Function: 			mtk6252_probe
  Description:		mtk6252 driver's probe function
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				long: 
  Others:				mainly used to monitor mtk's state
******************************************************************************/
static int __devinit mtk6252_probe(struct platform_device *pdev)
{
	struct msm_device_mtk6252_data *pdata = pdev->dev.platform_data;
	struct mtk6252_driver *priv = NULL;
	int ret = 0;
	unsigned long flags = 0;

	pr_debug("%s enter\n", __func__);

	/* init driver data */
	if (pdata) {
		priv = kzalloc(sizeof(struct mtk6252_driver),
				GFP_KERNEL);
		if (!priv) {
			printk(KERN_ERR "%s: kzalloc failed\n", __func__);
			return -ENOMEM;
		}

		priv->gpio_pwron = pdata->gpio_pwron;
		priv->gpio_reset = pdata->gpio_reset;
		priv->gpio_pwrstat = pdata->gpio_pwrstat;
		priv->gpio_softwarestate = pdata->gpio_softwarestate;
		priv->gpio_download_en = pdata->gpio_download_en;
		priv->irq_softwarestate = gpio_to_irq(pdata->gpio_softwarestate);
		priv->gpio_usb_sel = pdata->gpio_usb_sel;
		priv->gpio_sim_swap = pdata->gpio_sim_swap;
	}

	/* set drv data */
	platform_set_drvdata(pdev, priv);

	gdriver = priv;

	pr_debug("%s line %d\n", __func__, __LINE__);

	/* create sys fs */
	ret = mtk6252_create_sysfs(pdev);
	if (ret) {
		printk(KERN_ERR "%s: create sysfs failed, ret = %d\n", __func__, ret);
		goto err_mtk6252_device_create_sysfs_failed;
	}

	/* init monitor */
	init_waitqueue_head(&(priv->monitor));

	/* init mutex */
	mutex_init(&priv->dev_lock);

	/* create work for irq handle*/
	INIT_WORK(&priv->work_irq_softwarestate, mtk6252_work_irq_softwarestate_func);
	INIT_WORK(&priv->work_start, mtk6252_work_start_func);

	/* wakelock init */
	wake_lock_init(&priv->mtk_irq_wakelock, WAKE_LOCK_SUSPEND, "mtk_state_detect");

	spin_lock_init(&priv->lock);

	pr_debug("%s line %d\n", __func__, __LINE__);

/* comment interrupt handler temperary */
	/* status irq */
	if (priv->irq_softwarestate) {
		/* change trigger to level low */
		priv->irq_enabled = 1;
		ret = request_irq(priv->irq_softwarestate, mtk6252_irq_softewarestate_handler,
				IRQF_TRIGGER_LOW, pdev->name, priv);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to request_irq %d\n, ret = %d", __func__, priv->irq_softwarestate, ret);
			goto err_mtk6252_irq_request_failed;
		}
		spin_lock_irqsave(&priv->lock, flags);
		if (priv->irq_enabled) {
			disable_irq_nosync(priv->irq_softwarestate);
			priv->irq_enabled = 0;
		}
		spin_unlock_irqrestore(&priv->lock, flags);
		ret = irq_set_irq_wake(priv->irq_softwarestate, 1);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to set IRQ wake, ret = %d\n", __func__, ret);
			goto err_mtk6252_set_irq_wake_failed;
		}
	}
	pr_debug("%s line %d\n", __func__, __LINE__);

	/* launch work to start mtk modem */
	schedule_work(&priv->work_start);

	pr_debug("%s line %d\n", __func__, __LINE__);

	/* create misc device*/
	ret = misc_register(&mtk6252_device);
	if (ret) {
		printk(KERN_ERR "%s: mtk6252_device register failed, ret = %d\n", __func__, ret);
		goto err_mtk6252_device_register_failed;
	}

	pr_debug("%s exit ok\n", __func__);

	return 0;

err_mtk6252_device_register_failed:
/* comment interrupt handler temperary */
err_mtk6252_set_irq_wake_failed:

	irq_set_irq_wake(priv->irq_softwarestate, 0);
	free_irq(priv->irq_softwarestate, priv);
err_mtk6252_irq_request_failed:
	wake_lock_destroy(&priv->mtk_irq_wakelock);
	mutex_destroy(&priv->dev_lock);
	mtk6252_remove_sysfs(pdev);
err_mtk6252_device_create_sysfs_failed:
	gdriver = NULL;
	platform_set_drvdata(pdev, NULL);
	kfree(priv);
	printk(KERN_ERR"%s exit fail %d\n", __func__, ret);
	return ret;	
}

/******************************************************************************
  Function: 			mtk6252_remove
  Description:		mtk6252 driver's remove function
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0
  Others:				
******************************************************************************/
static int __devexit mtk6252_remove(struct platform_device *pdev)
{
	struct mtk6252_driver *priv = dev_get_drvdata(&pdev->dev);

	/* cancel irq */
	irq_set_irq_wake(priv->irq_softwarestate, 0);
	free_irq(priv->irq_softwarestate, priv);

	/* cancel work */
	cancel_work_sync(&priv->work_start);
	cancel_work_sync(&priv->work_irq_softwarestate);

	/* cancel mutex */
	mutex_destroy(&priv->dev_lock);
	
	/* cancel wakelock */
	wake_lock_destroy(&priv->mtk_irq_wakelock);

	/* unregister misc dev */
	misc_deregister(&mtk6252_device);	

	/* unregister sysfs */
	mtk6252_remove_sysfs(pdev);

	/* release driver data */
	gdriver = NULL;
	platform_set_drvdata(pdev, NULL);
	kfree(priv);

	return 0;
}

/******************************************************************************
  Function: 			mtk6252_shutdown
  Description:		mtk6252 driver's shutdown function
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void mtk6252_shutdown(struct platform_device *pdev)
{
	printk(KERN_INFO"%s enter\n", __func__);
	/* disable irq */
	mtk6252_disable_softwarestate_irq();
	/* power off */
	mtk6252_power_off();
}

/* mkt6252's driver define */
static struct platform_driver mtk6252_driver = {
	.probe		= mtk6252_probe,
	.remove		= __devexit_p(mtk6252_remove),
	.shutdown	= mtk6252_shutdown,
	.driver		= {
		.name	= "mtk6252",
		.owner	= THIS_MODULE,
	},
};

MODULE_ALIAS("platform:mtk6252");

/******************************************************************************
  Function: 			mtk6252_drvier_init
  Description:		mtk6252 driver's register function
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int
  Others:				
******************************************************************************/
static int __init mtk6252_drvier_init(void)
{
	return platform_driver_register(&mtk6252_driver);
}

/******************************************************************************
  Function: 			mtk6252_driver_exit
  Description:		mtk6252 driver's exit function
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void  __exit mtk6252_driver_exit(void)
{
	platform_driver_unregister(&mtk6252_driver);
}

/* module init define */
module_init(mtk6252_drvier_init);
/* module exit define */
module_exit(mtk6252_driver_exit);
/* module decsription define */
MODULE_DESCRIPTION("Mtk modem Driver");
/* module license define */
MODULE_LICENSE("GPL");

