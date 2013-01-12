/* mtk6252_dev.c
 *
 * mtk6252 devices Routines, mtk6252 as a modem of system
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
 
#include <asm/mach-types.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/hardware_self_adapt.h>
#include <linux/mtk6252_dev.h>

/* moves some macros of GPIO no. to mtk6252_dev.h */
#define SIM_SWAP 36									/* the pin to control the sim swap */
static struct msm_device_mtk6252_data mtk6252_data = {
	.gpio_pwron		= MTK_PWRON,
	.gpio_reset			= AP_RESET_MTK,
	.gpio_pwrstat		= GPIO_MODEM_PWRSTAT,
	.gpio_softwarestate	= MODEM_SOFTWARE_STATE,
	.gpio_download_en		= DOWNLOAD_EN,
	.gpio_usb_sel		= USB_SEL,
    .gpio_sim_swap		= SIM_SWAP,
};

static struct platform_device msm_device_mtk6252_modem = {
	.name	= "mtk6252",
	.dev		= {
		.platform_data = &mtk6252_data,
	},
	.id		= -1,
};

static struct msm_gpiosleep_data gpiosleep_data = {
	.host_wake		= GPIO_MTK_WAKE_MSM,
	.ext_wake		= GPIO_MSM_WAKE_MTK,
};

static struct platform_device msm_gpiosleep_device = {
	.name	= "gpiosleep",
	.dev		= {
		.platform_data = &gpiosleep_data,
	},
	.id		= -1,
};

/* gpio tabel */
static uint32_t mtk6252_gpio_table[] = {
	GPIO_CFG(MODEM_SOFTWARE_STATE,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), 
	GPIO_CFG(GPIO_MODEM_PWRSTAT,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), 
	GPIO_CFG(AP_RESET_MTK,  0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), 
	GPIO_CFG(MTK_PWRON,  0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), 
	GPIO_CFG(DOWNLOAD_EN,  0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), 
	GPIO_CFG(USB_SEL,  0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), 
	GPIO_CFG(GPIO_MTK_WAKE_MSM,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), 
	GPIO_CFG(GPIO_MSM_WAKE_MTK,  0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), 
    /*the sim swap gpio can not be configured in ap,it have been configured in modem base on the sim swap menu*/
};

/******************************************************************************
  Function: 			config_gpio_table
  Description:		config gpio of mtk modem related
  Calls:					mtk6252_dev_init
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				void
  Others:				
******************************************************************************/
static void config_gpio_table(uint32_t *table, int len)
{
	int n, rc;
	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}

/******************************************************************************
  Function: 			mtk6252_dev_support
  Description:		check if support mtk6252 modem
  Calls:					mtk6252_dev_init
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0 - support, 1 -- not support
  Others:				
******************************************************************************/
static int  mtk6252_dev_support(void)
{
	/* only C8950D support mtk modem, remove U8950D */
	if ( machine_is_msm8x25_C8950D() )
	{
		pr_debug("%s support mach\n", __func__);
		return 1; /* support */
	}	

	pr_debug("%s not support mach\n", __func__);
	return 0; /* not support */
}

/******************************************************************************
  Function: 			mtk6252_dev_init
  Description:		mtk6252 device init
  Calls:					
  Data Accessed:	
  Data Updated:	
  Input:				
  Output:				
  Return:				int: 0 - ok, else fail
  Others:				
******************************************************************************/
 int mtk6252_dev_init(void)
{
	int ret;

	pr_debug("%s enter\n", __func__);

	/* judge mach if support, if not, return */
	if(!mtk6252_dev_support()){
		printk("%s not support\n", __func__);
		return -1;
	}

	/* config gpio */
	config_gpio_table(mtk6252_gpio_table, ARRAY_SIZE(mtk6252_gpio_table));

	/* device register */
	ret = platform_device_register(&msm_device_mtk6252_modem);
	if (ret)
	{
		printk(KERN_ERR"%s fail\n", __func__);
	}

    ret = platform_device_register(&msm_gpiosleep_device);
	if (ret)
	{
		printk(KERN_ERR"%s fail\n", __func__);
	}
    

	pr_debug("%s exit\n", __func__);

	return ret;
}

