/* ====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 * 
 *                     PN544  Near Field Communication (NFC) driver
 * 
 * GENERAL DESCRIPTION
 *   driver for PN544 NFC on Huawei Sonic

 * REFERENCES
 * 
 * EXTERNALIZED FUNCTIONS
 *   None.
 * 
 * INITIALIZATION AND SEQUENCING REQUIREMENTS
 * 
 * Copyright (c) 2011 by HUAWEI, Incorporated.  All Rights Reserved.
 * ====*====*====*====*====*====*====*====*====*====*====*====*====*====*====
 * ===========================================================================
 * 
 *                       EDIT HISTORY FOR FILE
 * 
 *  This section contains comments describing changes made to this file.
 *   Notice that changes are listed in reverse chronological order.
 * 
 * 
 * when       who      what, where, why
 * -------------------------------------------------------------------------------
 * 20110106  genghua  create  SUPPORT PN544 NFC on Sonic
 */

/*
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

#include <linux/uaccess.h>
#include <asm/gpio.h>
#include <linux/module.h>
#include <linux/nfc/pn544.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/slab.h> 
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/crc-ccitt.h>
#include <asm/mach-types.h>

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <linux/hw_dev_dec.h>
#endif

int pn544_debug_mask = 0;
int pn544_debug_control = 1;
#define PN544_RESET_CMD 	0
#define PN544_DOWNLOAD_CMD	2

module_param_named(debug_mask, pn544_debug_mask, int, 
				   S_IRUGO | S_IWUSR | S_IWGRP);

int pn544_use_read_irq = 0;
module_param_named(use_read_irq, pn544_use_read_irq, int, 
				   S_IRUGO | S_IWUSR | S_IWGRP);

/* we add pn544_debug_control to let the 
 * driver can control the printk more easier.
 */
#define PN544_DEBUG(message, ...) \
	do { \
	if (pn544_debug_mask && pn544_debug_control) \
		printk(message, ## __VA_ARGS__); \
	} while (0)

/* the public values for the HCI base software for temp use
 * these values will be moved when the HAL and Upper-level code
 * for nfc added to our project 
 */
struct i2c_client pn544_client;
struct pn544_info * pn544_info=NULL;
struct pn544_nfc_platform_data *pdata = NULL; 
static bool update = false;

/* the pn544 i2c_device id table */
static struct i2c_device_id pn544_id_table[] = 
{
	{ PN544_DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pn544_id_table);


/*it is used in pn544_i2c_write, and this func is only used by us,
*so the illegal verify is not necessarily
*/ 
/*del 29 lines*/

enum pn544_irq pn544_irq_state(struct pn544_info *info) 
{ 
	enum pn544_irq irq=PN544_NONE; 
	if (0!=pn544_use_read_irq)
	{
		mutex_lock(&(info->read_mutex) ); 
		irq = info->read_irq; 
		mutex_unlock(&(info->read_mutex)); 
	}
	return irq; 
} 

/* this function is the irq_disable handler */
static void pn544_disable_irq(struct pn544_info *info)
{
	unsigned long flags;
	PN544_DEBUG("%s:entered\n",__func__);
	
	spin_lock_irqsave(&pn544_info->irq_enabled_lock, flags);
	/* use irq enable flag*/
	if (pn544_info->irq_enabled) {
		disable_irq_nosync(pn544_info->i2c_dev->irq);
		pn544_info->irq_enabled = false;
		PN544_DEBUG("%s:disable pn544irq\n", __func__);
	}
	spin_unlock_irqrestore(&pn544_info->irq_enabled_lock, flags);	
}

/* this function is used the irq handler*/
static irqreturn_t pn544_irq_thread_fn(int irq, void *dev_id) 
{ 
	PN544_DEBUG("%s:enter\n",__func__);

	pn544_disable_irq(pn544_info);
	pn544_info->read_irq = PN544_INT;
	
	/* Wake up waiting readers */
	wake_up(&pn544_info->read_wait);
	
	return IRQ_HANDLED;

} 

static int pn544_open(struct inode *inode, struct file *file) 
{ 

	int ret = 0; 
	/*use global var instead of container_of funcation*/
    struct pn544_info *info = pn544_info;	


	PN544_DEBUG("%s:entered info: %p, client %p\n", __func__, 
		info, info->i2c_dev); 
	mutex_lock(&info->mutex); 
	if (info->state != PN544_ST_COLD) 
	{ 
		ret = -EBUSY; 
		goto out; 
	} 
        
	info->state = PN544_ST_READY;

	file->f_pos = info->read_offset; 
out: 
	mutex_unlock(&info->mutex); 
	PN544_DEBUG("%s:exit,ret=%d\n",__func__,ret);

	return ret; 
}

static int pn544_close(struct inode *inode, struct file *file) 
{
    struct pn544_info *info = pn544_info;
	/* maybe we need add something in this function later */
	PN544_DEBUG("%s:entered\n",__func__);
    /* set state of pn544 after dev closed */
	/*use global var instead of container_of funcation*/
	/* the declaration of var should be in the front of this function */
	mutex_lock(&info->mutex);
	info->state = PN544_ST_COLD;
	mutex_unlock(&info->mutex); 
	PN544_DEBUG("%s:exit\n",__func__);
	return 0; 
} 

int pn544_i2c_read(struct i2c_client *client, u8 *buf, int buflen) 
{ 
	int ret=0; 
	u8 len=0; 
	int i = 0;

	PN544_DEBUG("%s:entered\n",__func__);

	/* You could read a packet in one go, but then you'd need to read 
	 * max size and rest would be 0xff fill, so we do split reads. 
	 */ 

	ret = i2c_master_recv(client, &len, sizeof(len)); 
	PN544_DEBUG("%s:recv1: ret=%d,len=%d\n",
		__func__, ret , len); 

	if (0==pn544_use_read_irq)
	{
		/* if we do not use irq when the data is not ready we need to delay 
		 * sometime to receive again
		 */
		if(len==PN544_NODATA)
		{
			mdelay(10);
			ret = i2c_master_recv(client, &len, sizeof(len)); 
			PN544_DEBUG("%s:recv2: ret=%d,len=%d\n",
				__func__, ret , len); 
		}
	}

	if (ret != 1) 
		return -EREMOTEIO; 

	if(PN544_NODATA == len)
		return -EREMOTEIO; 

	/* we make sure the length is legal for the LLC Layer*/
	if (len < PN544_LLC_HCI_OVERHEAD) 
		len = PN544_LLC_HCI_OVERHEAD; 
	else if (len > (PN544_MSG_MAX_SIZE - 1)) 
		len = PN544_MSG_MAX_SIZE - 1; 

	if (1 + len > buflen) /* len+(data+crc16) */ 
		return -EMSGSIZE; 

	buf[0] = len; 

	ret = i2c_master_recv(client, buf + 1, len); 

	if (ret != len) 
		return -EREMOTEIO; 

	usleep_range(3000, 6000); 
	PN544_DEBUG("IFD->PC:  ");
	for(i = 0; i < len; i++)
	{
		PN544_DEBUG("%02X", buf[i]);
	}
	PN544_DEBUG("\n");
	return ret + 1; 
} 

int pn544_i2c_write(struct i2c_client *client, u8 *buf, int len) 
{ 
	int ret=0; 
	int i = 0;

    PN544_DEBUG("%s:entered, len=%d, buf0+1 = %d\n",__func__, len, buf[0] + 1);
	
	//this func is only used by us, so the illegal verify is not necessarily
	if (len < 3) 
	{ 
		printk("%s: [ERROR]Illegal message length: %d\n", 
			__func__, len); 
		return -EINVAL; 
	} 

	usleep_range(3000, 6000); 

	ret = i2c_master_send(client, buf, len); 
	PN544_DEBUG("%s:send msg to pn544 ret=%d\n", __func__,ret); 
	/* if the PN544 enter the standby mode, we receive EIO from i2c core
	 * NOT EREMOTEIO which we imported from the open-source code.
	 */
	/* Retry if chip was in standby */ 
	if (ret == -EIO) 
	{ 
		usleep_range(6000, 10000); 
		ret = i2c_master_send(client, buf, len); 
		PN544_DEBUG("%s:chip was in standby, retry sending,ret=%d\n",
			__func__,ret); 
	} 

	if (ret != len) 
		return -EREMOTEIO; 

    PN544_DEBUG("IFD->PC: ");
	for(i = 0; i < len; i++)
	{
		PN544_DEBUG("%02X", buf[i]);
	}
	PN544_DEBUG("\n");
	return ret; 
} 

static ssize_t pn544_read(struct file *file, char __user *buf, 
						  size_t count, loff_t *offset) 
{ 
	/*use global var instead of container_of funcation*/
    struct pn544_info *info = pn544_info;	
    /*unused var variable 'len'*/
	int ret = -1;
	int i = 0;
	char tmp[PN544_MAX_PACK_LEN];

	PN544_DEBUG("%s:entered\n",__func__);

	if (count > PN544_MAX_PACK_LEN)
		count = PN544_MAX_PACK_LEN;

	PN544_DEBUG("%s : reading %zu bytes.\n", __func__, count);

	//mutex_lock(&info->mutex);
	/*if gpio value is high we should enable irq*/
	PN544_DEBUG("%s:GPIO_NFC_INT value %d\n", __func__, gpio_get_value(GPIO_NFC_INT)); 
	
	if(!gpio_get_value(GPIO_NFC_INT))
	{
		PN544_DEBUG("%s:enable pn544irq !\n", __func__); 
		pn544_info->irq_enabled = true;
		enable_irq(pn544_info->i2c_dev ->irq);
		ret = wait_event_interruptible(pn544_info->read_wait, 
		                               gpio_get_value(GPIO_NFC_INT));
		pn544_disable_irq(pn544_info);
		PN544_DEBUG("%s:enable pn544irq! ret=%d!\n", __func__, ret);
		if (ret)
		{
			goto out;
		}	
	}
	if (update)
	{
		PN544_DEBUG("updateing****!\n");
	}

	/* Read data */
	ret = i2c_master_recv(info->i2c_dev, tmp, count);
	PN544_DEBUG("%s:read datasize: ret=%d count=%d\n", __func__, ret, count);
	//mutex_unlock(&info->mutex);
	
	if (ret < 0) {
		printk("%s: receive error! i2c_master_recv returned %d\n", __func__, ret);
		return ret;
	}
	if (ret > count) {
		printk("%s: received too many bytes from i2c (%d)\n",
			__func__, ret);
		return -EIO;
	}
	PN544_DEBUG("IFD->PC: ");
	for(i = 0; i < count; i++)
	{
		PN544_DEBUG("%02X", tmp[i]);
	}
	PN544_DEBUG("\n");
	if (copy_to_user(buf, tmp, ret)) {
		printk("%s : failed to copy to user space\n", __func__);
		return -EFAULT;
	}
	PN544_DEBUG("%s:exit ret=%d\n",__func__,ret);
	return ret;

out: 
	//mutex_unlock(&info->mutex);
	return ret;
} 

static ssize_t pn544_write(struct file *file, const char __user *buf, 
						   size_t count, loff_t *ppos) 
{ 
	/*use global var instead of container_of funcation*/
    struct pn544_info *info = pn544_info;	
    /*unused var variable 'len'*/
	int ret=0; 
	char tmp[PN544_MAX_PACK_LEN];
	int i = 0;

	PN544_DEBUG("%s:entered\n",__func__);
	if (count > PN544_MAX_PACK_LEN)
		count = PN544_MAX_PACK_LEN;

	if (copy_from_user(tmp, buf, count)) {
		printk("%s : failed to copy from user space\n", __func__);
		return -EFAULT;
	}

	/* Write data */
	PN544_DEBUG("ready to send , client addr=0x%02x,count=%d,tmp[0]=%02x\n",
		info->i2c_dev->addr,count,tmp[0]);
	
	usleep_range(3000,6000); 
	PN544_DEBUG("PC->IFD: ");
	for(i = 0; i < count; i++)
	{
		PN544_DEBUG("%02X", tmp[i]);
	}
	PN544_DEBUG("\n");
	//if send failed, we will retry it 3 times
    for(i = 0; i < 3; i++)
    {
        ret = i2c_master_send(info->i2c_dev, tmp, count);
	
        if(ret < 0)
        {
            usleep_range(6000,10000); 
		
            //ret = i2c_master_send(info->i2c_dev, tmp, count); 
            PN544_DEBUG("%s:chip was in standby, retry sending,ret=%d\n",
                __func__,ret); 
        }
        else
        {
		    break;
        }
    }
	
	if (ret != count) {
		printk("%s :send error! i2c_master_send returned %d\n", __func__, ret);
		ret = -EIO;
	}	
	PN544_DEBUG("%s:exit,ret=%d\n",__func__,ret);
	return ret;
} 

int pn544_send_for_mmi(char * pszBuf,unsigned short SendCnt)
{
	int ret=0;	
	mutex_lock(&pn544_info->mutex_mmi);
	ret = pn544_i2c_write(&pn544_client, pszBuf, SendCnt); 
	mutex_unlock(&pn544_info->mutex_mmi);
	return ret;
}
int pn544_read_for_mmi(char * pszBuf)
{	
	int ret=0;
	mutex_lock(&pn544_info->mutex_mmi);
	ret = pn544_i2c_read(&pn544_client, pszBuf, PN544_MAX_PACK_LEN);
	mutex_unlock(&pn544_info->mutex_mmi);
	return ret;
}

//for reset and firmware download
static long pn544_dev_ioctl(struct file *file, unsigned int cmd, unsigned long level)
{
	long ret = 0;

//On platform 7X30, if GPIO04 is used, the fuction  machine_is_msm8255_u8860_r() will return true	
#ifdef CONFIG_ARCH_MSM7X30
	if (!machine_is_msm8255_u8860_r())
	{
	    printk("%s:the cmd is not supported on this board.\n",__func__);
	    return ret;
	}
#endif

	//Now we only deal with cmd PN544_RESET_CMD and PN544_DOWNLOAD_CMD
	switch (cmd) {
	case PN544_SET_PWR:
	    //It will be used for power on and off	      
	    if( PN544_RESET_CMD == level )
	    {
	        printk("ioctl: pn544_reset VEN level:%ld\n", level);
	        if (!pdata->pn544_ven_reset || !pdata->pn544_fw_download_pull_down) 
	        { 
	            printk("%s:func pull_down missing\n",__func__); 
	            ret = -1;
	            break;
	        }
	
	        pdata->pn544_fw_download_pull_down();
	        ret = pdata->pn544_ven_reset();	
	        update = false;
	        ret = PN544_RESET_CMD;
				
	    }
	    //Only for firmware download		
	    else if(PN544_DOWNLOAD_CMD == level )
	    {
	        printk("ioctl: pn544_download nfc_load level:%ld\n", level);
	        if (!pdata->pn544_ven_reset || !pdata->pn544_fw_download_pull_high)
	        {
	            printk("%s:func pull_high missing\n",__func__);
	            ret = -1;
	            break;
	        } 

	        pdata->pn544_fw_download_pull_high();
	        pdata->pn544_ven_reset();
	        update = true;
	        printk("%s:down load!\n",__func__); 

	        ret = PN544_DOWNLOAD_CMD;
	    }
	    else
	    {
	        printk("ioctl: unknown level %ld",level);	
	    }
	    break;
	default:
	    printk("ioctl: unknown_cmd %d",cmd);
	    ret = -1;
	    break;
	}

	return ret;
}

static const struct file_operations pn544_fops = 
{ 
	.owner		= THIS_MODULE, 
	.llseek		= no_llseek, 
	.read		= pn544_read, 
	.write		= pn544_write, 
	.open		= pn544_open, 
	.release	= pn544_close, 
	.unlocked_ioctl      = pn544_dev_ioctl,
}; 

//nto all phone download pin is down, we need pull down the NFC_DOWNLOAD pin first, this func is used for firmware download
static int init_fw_download(void)
{
	int ret=0;
	int gpio_config=0;
	
	printk("pn544:fw_download gpio_request GPIO%d\n", GPIO_NFC_LOAD);
	ret = gpio_request(GPIO_NFC_LOAD, "gpio for NFC pn544 download");
	if(ret) {
		printk("pn544:fw_download gpio_request failed\n");
	}
	
	gpio_config = GPIO_CFG(GPIO_NFC_LOAD, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA);
	ret = gpio_tlmm_config(gpio_config, GPIO_CFG_ENABLE);
	ret = gpio_direction_output(GPIO_NFC_LOAD,0);
	return 0;
}

//to check if the i2c device is ok
static int pn544_i2c_device_probe(struct i2c_client *client)
{
    int ret = 0;
    int i = 0;
    int cmd_send_result = 0;
    u8 *cmd_reset = NULL;
    u8 *cmd_fwdld_first = NULL;
    u8 *cmd_receive = NULL;

    /* the following code is used for the pn544 reset cmd test
     * according to the pn544 SPEC 
     */
    cmd_reset = kzalloc(sizeof(u8) * PN544_RESET_SEND_SIZE, GFP_KERNEL);
    if(NULL == cmd_reset)
    {
        PN544_DEBUG("%s:cmd_reset alloc failed.\n", __func__);
        ret = -ENOMEM;
        goto err_cmd_reset_alloc; 
    }

    cmd_receive = kzalloc(sizeof(u8) * PN544_RESET_RECEIVE_SIZE, GFP_KERNEL);
    if(NULL == cmd_receive)
    {
        PN544_DEBUG("%s:cmd_receive alloc failed.\n", __func__);
        ret = -ENOMEM;
        goto err_cmd_receive_alloc; 
    }

    for (i = 0; i < PN544_RESET_SEND_TIMES; i++)
    {
        cmd_reset[0] = 0x05;
        cmd_reset[1] = 0xf9;
        cmd_reset[2] = 0x04;
        cmd_reset[3] = 0x00;
        cmd_reset[4] = 0xc3;
        cmd_reset[5] = 0xe5;
		 
        /* here we send a HCI based reset command to pn544 to reset it */	
        mdelay(100);	

        ret = pn544_i2c_write(client, cmd_reset, PN544_RESET_SEND_SIZE);
        if (ret < 0)
        {
            PN544_DEBUG("%s:write i2c failed, continue.\n", __func__);
            continue;
        }
        mdelay(5);

        PN544_DEBUG("%s:send cmd_reset to pn544\n", __func__);

        cmd_receive[0] = 0x00;
        cmd_receive[1] = 0x00;
        cmd_receive[2] = 0x00;
        cmd_receive[3] = 0x00;
		
	
        PN544_DEBUG("%s:ready to receive data\n", __func__);
        ret = pn544_i2c_read(client, cmd_receive, PN544_RESET_RECEIVE_SIZE);
        if(ret < 0)
        {
            PN544_DEBUG("%s:read i2c failed, continue.\n", __func__);
            continue;         
        }
              
        PN544_DEBUG("pn544 cmd_receive[0]=0x%x\n", cmd_receive[0]);
        PN544_DEBUG("pn544 cmd_receive[1]=0x%x\n", cmd_receive[1]);
        PN544_DEBUG("pn544 cmd_receive[2]=0x%x\n", cmd_receive[2]);
        PN544_DEBUG("pn544 cmd_receive[3]=0x%x\n", cmd_receive[3]);
	 

        /* the following check is according to pn544 SPEC */
        if ((cmd_receive[0]!= 0x03)
			|| (cmd_receive[1]!= 0xE6) 
			|| (cmd_receive[2]!= 0x17) 
			|| (cmd_receive[3]!= 0xA7))
        {
            PN544_DEBUG("%s:The reset cmd is not exec successful\n", __func__);
            ret = -ENODEV; 
            continue;
        }

        cmd_send_result = 1; 
        ret = 0;
        break;
    }

    if (1 == cmd_send_result)
    {
        PN544_DEBUG("%s, pn544 reset cmd response successfully.\n", __func__);
        ret = 0;
        goto err_cmd_reset;
    }

    //no response with reset cmd, we go to dlowad mode to test the i2c 
    if (!pdata->pn544_ven_reset || !pdata->pn544_fw_download_pull_high)
    {
        PN544_DEBUG("%s:func pull_high and ven_reset missing\n", __func__);

        ret = -ENODEV;
        goto err_cmd_reset;     
    } 

    pdata->pn544_fw_download_pull_high();
    pdata->pn544_ven_reset();

    cmd_fwdld_first = kzalloc(sizeof(u8) * PN544_FWDLD_FIRST_SEND_SIZE, GFP_KERNEL);
    if(NULL == cmd_fwdld_first)
    {
        PN544_DEBUG("%s:cmd_fwdld_first alloc failed.\n", __func__);

        ret = -ENOMEM; 
        goto err_cmd_fwdld_first_alloc; 
    }

    //the test cmd 0x010000
    cmd_fwdld_first[0] = 0x01;
    cmd_fwdld_first[1] = 0x00;
    cmd_fwdld_first[2] = 0x00;

    mdelay(100);
	
    ret = pn544_i2c_write(client, cmd_fwdld_first, PN544_FWDLD_FIRST_SEND_SIZE);
    if (ret < 0)
    {
        PN544_DEBUG("%s:pn544_i2c_write download cmd failed.\n", __func__);

        goto err_cmd_fwdld_first_send;
    }
    mdelay(5);

    if (!pdata->pn544_fw_download_pull_down)
    {
	 PN544_DEBUG("%s:func pull_down missing.\n", __func__);
        goto err_cmd_fwdld_first_send;
    }

    //here we should exit the download mode
    pdata->pn544_fw_download_pull_down();
    pdata->pn544_ven_reset();


err_cmd_fwdld_first_send:
	kfree(cmd_fwdld_first);
	
err_cmd_fwdld_first_alloc:	

err_cmd_reset:
	kfree(cmd_receive);
	
err_cmd_receive_alloc:
	kfree(cmd_reset);
	
err_cmd_reset_alloc:	

    return ret;			
}
	
static int __devinit pn544_probe(struct i2c_client *client,
								 const struct i2c_device_id *id)
{
	int ret=0;
    /*del 10 lines*/
	
	PN544_DEBUG("%s:entered\n",__func__);

	/* private data allocation */ 
	pn544_info = kzalloc(sizeof(struct pn544_info), GFP_KERNEL); 
	if (!pn544_info) 
	{ 
		printk("%s:Cannot allocate memory for pn544_info.\n",__func__); 
		ret = -ENOMEM; 
		goto err_info_alloc; 
	} 

	pn544_info->buflen = max(PN544_MSG_MAX_SIZE, PN544_MAX_I2C_TRANSFER); 
	pn544_info->buf = kzalloc(pn544_info->buflen, GFP_KERNEL);    
	if (!pn544_info->buf) 
	{ 
		printk("%s:Cannot allocate memory for pn544_info->buf.\n",__func__); 
		ret = -ENOMEM; 
		goto err_buf_alloc; 
	} 

	pn544_info->i2c_dev = client; 
	pn544_info->state = PN544_ST_COLD; 
	pn544_info->read_irq = PN544_NONE; 
	mutex_init(&pn544_info->read_mutex); 
	mutex_init(&pn544_info->mutex); 
	mutex_init(&pn544_info->mutex_mmi); 
	init_waitqueue_head(&pn544_info->read_wait); 
	spin_lock_init(&pn544_info->irq_enabled_lock);
	i2c_set_clientdata(client, pn544_info); 
//On platform 7X30 we should identify whether GPIO04 is used
#ifdef CONFIG_ARCH_MSM7X30
    if (machine_is_msm8255_u8860_r())
	{
	    //GPIO04 is needed for firmware download
	    ret = init_fw_download();
	    if(ret) {
		    printk("%s:nfc init_fw_download failed\n",__func__);
	    }
	}
#else	
	ret = init_fw_download();
	if(ret) {
		printk("%s:nfc init_fw_download failed\n",__func__);
	}
#endif
	pdata = client->dev.platform_data; 
	if (!pdata) 
	{ 
		printk("%s:No platform data\n",__func__); 
		ret = -EINVAL; 
		goto err_no_platform_data; 
	} 
//The clock control of nfc is different between 7x27A and 7x30, we don't need to do following operation on 7X30 
#ifdef CONFIG_ARCH_MSM7X30
	/* nothing todo */
#else
	if (!pdata->pn544_clock_output_ctrl) 
	{ 
		printk("%s:pn544_clock_output_ctrl  missing\n",__func__); 
		ret = -EINVAL; 
		goto err_no_clock_ctrl; 
	} 
	else
	{
		ret = pdata->pn544_clock_output_ctrl(1);
		printk("pn544_clock_output_ctrl:%d \n",ret);
		if(ret)
		{		       
		    goto err_no_clock_ctrl; 
		}
	}    
	//It is a fixed operation sequence, just follow it   
	if (!pdata->pn544_fw_download_pull_down) 
	{ 
		printk("%s:fw_download  missing\n",__func__); 
		ret = -EINVAL; 
		goto err_no_fw_download; 
	} 
	else
	{
		ret = pdata->pn544_fw_download_pull_down();
		if(ret)
		{
			goto err_no_fw_download; 
		}
	}
#endif

	if (!pdata->pn544_ven_reset) 
	{ 
		printk("%s:ven reset missing\n",__func__); 
		ret = -EINVAL; 
		goto err_no_ven_reset; 
	} 
	else
	{
		ret = pdata->pn544_ven_reset();
		if(ret)
		{
			goto err_no_ven_reset; 
		}
	}
    /* we need to request read irq to support the pn544 
     *read function anyway , so we delete "if" here
     */
	if (!pdata->pn544_interrupt_gpio_config) 
	{ 
		printk("%s:request_resources() missing\n",__func__); 
		ret = -EINVAL; 
		goto err_gpio_config; 
	} 
	else
	{
		ret=pdata->pn544_interrupt_gpio_config();
		if(ret)
		{
			goto err_gpio_config; 
		}
	}

	/* irq request move down */
    /* we need to request read irq to support the pn544 
     *read function anyway , so we delete "if" here
     */

    //we use a func to deal the device probe
    if (pn544_i2c_device_probe(client) < 0)
    {
        printk("%s:unable to probe device.\n", __func__); 
		ret = -ENODEV; 
        goto err_cmd_resp;
    }
	/*kfree delete*/
	memcpy(&pn544_client,client,sizeof(struct i2c_client));

	if (0!=pn544_use_read_irq)
	{
		pn544_info->read_irq=PN544_NONE;
		enable_irq(pn544_client.irq);
		pn544_info->use_read_irq=1;
	}
	else
	{
		pn544_info->use_read_irq=0;
	}
	/* add irq request*/
	pn544_info->irq_enabled = true;
	ret = request_irq(client->irq, pn544_irq_thread_fn, 
		IRQF_TRIGGER_HIGH, PN544_DRIVER_NAME, pn544_info); 

	if (ret < 0) 
	{ 
		printk("%s:Unable to register IRQ handler\n", __func__); 
		goto err_irq_req; 
	} 
	pn544_disable_irq(pn544_info);
	pn544_info->miscdev.minor = MISC_DYNAMIC_MINOR; 
	pn544_info->miscdev.name = PN544_DRIVER_NAME; 
	pn544_info->miscdev.fops = &pn544_fops; 
	pn544_info->miscdev.parent = &client->dev; 
	ret = misc_register(&pn544_info->miscdev); 
	if (ret < 0) 
	{ 
		printk("%s:Device registration failed\n",__func__); 
		goto err_misc_dev; 
	} 

	/* remove code because the HAL and the up-level
	 * code added to our project
	 */
	printk("%s success finished: info: %p, client %p\n", 
		__func__, pn544_info, client); 

    #ifdef CONFIG_HUAWEI_HW_DEV_DCT
    /* detect current device successful, set the flag as present */
    set_hw_dev_flag(DEV_I2C_NFC);
    #endif

	ret = pn544_mmi_init();
	if (ret)
	{
		goto err_mmi_error;
	}

//following operation is for power save	
#ifdef CONFIG_ARCH_MSM7X30
	/* nothing todo */
#else
	ret = pdata->pn544_clock_output_ctrl(0);
	printk("pn544_clock_output_ctrl:%d \n",ret);
	if(ret)
	{		       
		goto err_clock_mode_ctrl_err; 
	}

	//It is another clock out put mode, we can see the difference with pn544_clock_output_ctrl() in board-msm7x27a.c
	if (!pdata->pn544_clock_output_mode_ctrl) 
	{ 
		printk("%s:pn544_clock_output_mode_ctrl  missing\n",__func__); 
		ret = -EINVAL; 
		goto err_clock_mode_ctrl_err; 
	} 
	else
	{
		// 1 - Set for clock PMU request mode , 0 - Close for clock PMU request mode
		ret = pdata->pn544_clock_output_mode_ctrl(1);
		printk("pn544_clock_output_mode_ctrl:%d \n",ret);
		if(ret)
		{		       
			goto err_clock_mode_ctrl_err; 
		}
	}
#endif
    
    /*del 4 lines*/
	return 0; 		
#ifdef CONFIG_ARCH_MSM7X30
	/* nothing todo */
#else
err_clock_mode_ctrl_err:	
#endif
err_mmi_error:
	misc_deregister(&pn544_info->miscdev);
err_misc_dev:
	free_irq(client->irq, pn544_info);
err_irq_req:
err_cmd_resp:
/*del 14 lines*/
err_gpio_config:

err_no_ven_reset:
#ifdef CONFIG_ARCH_MSM7X30
	/* nothing todo */
#else
err_no_fw_download:
	pdata->pn544_clock_output_ctrl(0);
    printk("pn544_clock_output_ctrl  close when error\n");
err_no_clock_ctrl:    
#endif
err_no_platform_data:
	mutex_destroy(&pn544_info->read_mutex); 
	mutex_destroy(&pn544_info->mutex); 
	mutex_destroy(&pn544_info->mutex_mmi); 
	kfree(pn544_info->buf); 

err_buf_alloc: 
	kfree(pn544_info); 

err_info_alloc: 

	return ret;
}

// New function, Close for clock PMU request mode 
static int pn544_suspend(struct i2c_client *client, pm_message_t mesg)
{
#ifdef CONFIG_ARCH_MSM7X30
	/* nothing todo */
#else
	int ret = 0;
	
	PN544_DEBUG("%s:entered\n",__func__);
	
	pdata = client->dev.platform_data; 
	
	if (!pdata->pn544_clock_output_mode_ctrl) 
	{ 
		printk(" <NFC> %s:pn544_clock_output_mode_ctrl  missing\n",__func__); 
		return -EINVAL; 
	} 
	else
	{
		// 0 - close for clock pmu request mode, 1 - set for clock pmu request mode
		ret = pdata->pn544_clock_output_mode_ctrl(0);
		printk(" <NFC> pn544_clock_output_mode_ctrl: ret = %d  \n",ret );
		if(ret)
		{		       
			return -EINVAL; 
		}
	}
#endif

	return 0;
}


// New function, Set for clock PMU request mode 
static int pn544_resume(struct i2c_client *client)
{
#ifdef CONFIG_ARCH_MSM7X30
	/* nothing todo */
#else
	int ret = 0;

	PN544_DEBUG("%s:entered\n",__func__);

	pdata = client->dev.platform_data; 

	if (!pdata->pn544_clock_output_mode_ctrl) 
	{ 
		printk(" <NFC> %s:pn544_clock_output_mode_ctrl  missing\n",__func__); 
		return -EINVAL; 
	} 
	else
	{
		// 0 - close for clock pmu request mode, 1 - set for clock pmu request mode
		ret = pdata->pn544_clock_output_mode_ctrl(1);
		printk(" <NFC> pn544_clock_output_mode_ctrl: ret = %d  \n",ret );
		if(ret)
		{		       
			return -EINVAL; 
		}
	}
#endif
	
	return 0;
}

static __devexit int pn544_remove(struct i2c_client *client)
{
	misc_deregister(&pn544_info->miscdev);
	mutex_destroy(&pn544_info->read_mutex); 
	mutex_destroy(&pn544_info->mutex); 
	mutex_destroy(&pn544_info->mutex_mmi); 
	kfree(pn544_info->buf); 
	kfree(pn544_info); 
	
	return 0;
}

static struct i2c_driver pn544_driver = 
{
	.driver = 
	{
		.name = PN544_DRIVER_NAME,
	},
	.probe = pn544_probe,
	.id_table = pn544_id_table,
	.suspend = pn544_suspend,
	.resume = pn544_resume,
	.remove = __devexit_p(pn544_remove),
};


static int __init pn544_init(void)
{
	int ret;

	ret = i2c_add_driver(&pn544_driver);
	if (ret) 
	{
		printk(PN544_DRIVER_NAME ": driver registration failed\n");
		return ret;
	}
	return 0;
}

static void __exit pn544_exit(void)
{
	i2c_del_driver(&pn544_driver);
	PN544_DEBUG("%s:%s -Exiting.\n",__func__,PN544_DRIVER_DESC);
}

module_init(pn544_init);
module_exit(pn544_exit);

MODULE_LICENSE("GPL");

