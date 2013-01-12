/* Optimize the MMI code */
/*
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
 * 20120211  sunkai  create  SUPPORT MIMI key test ON C8828
 */
/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
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

#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/pci.h>
#include <linux/input.h>

#include <linux/module.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#include <linux/platform_device.h>
#include <linux/platform_device.h>
#include <linux/hardware_self_adapt.h>

static struct platform_device *mmi_key_dev;
/* MMI test start flag  */
#define ON 1
char value[8]={0};
bool mmi_keystate[255]={MMI_KEY_UP};

static ssize_t write_fun(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
	int flag=0;
	int key_code=0,keyup_code=0;
	
	sscanf(buf, "%d", &flag);
	
	/* For the upper open button, and then for by mmi_keystate [] 
	 * press the state key_code stored in the array value []
	 */
	if( ON == flag)
	{
		for(key_code=0;key_code<255;key_code++)
		{
			if (mmi_keystate[key_code] == MMI_KEY_DOWN && keyup_code < 8)
			{
				value[keyup_code] = key_code;
				keyup_code++;
			}	
		}
	}
	/* used to empty the value[] */
	else
	{
		for(key_code=0;key_code<8;key_code++)
		{
			value[key_code]=0;	
		}
	}
	return count;
}

static ssize_t read_fun(struct device *dev, struct device_attribute *attr, char *buf)
{
	/* report the value[] to APP */
	memcpy(buf, (char *) value, sizeof(value));
	return sizeof(value);
}

static DEVICE_ATTR(keytest,0664,read_fun, write_fun);

static int __init mmi_key_init(void)
{
	int rc=0;
	
	/* register a platform device */
	mmi_key_dev = platform_device_register_simple("mmi_key_dev",-1,NULL,0);
	if(IS_ERR(mmi_key_dev))
	{
		printk(KERN_ALERT "mmi_key_dev: platform_device_register_simple error\n");
	}
	
	rc = device_create_file(&mmi_key_dev->dev,&dev_attr_keytest);
	if(rc)
	{
		printk(KERN_ALERT "mmi_key_dev: sysfs_create_file error\n");	
	}
	return rc;	
}

static void __exit mmi_key_exit(void)
{
	device_remove_file(&mmi_key_dev->dev,&dev_attr_keytest);
	platform_device_unregister(mmi_key_dev);
	return;
}

module_init(mmi_key_init);
module_exit(mmi_key_exit);
