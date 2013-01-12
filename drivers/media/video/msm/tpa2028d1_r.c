/*
 * Copyright (C) 2009 HUAWEI Corporation.
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
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/earlysuspend.h>
#include <linux/audio_amplifier.h>
#include <linux/delay.h>
#include <mach/vreg.h>
#include <linux/err.h>

#define REG1_DEFAULT_VALUE 0xc3
#define TPA_DEBUG
#ifdef TPA_DEBUG
#define TPA_DEBUG_TPA(fmt, args...) printk(KERN_INFO fmt, ##args)
#else
#define TPA_DEBUG_TPA(fmt, args...)
#endif
#define TPA2028D1_I2C_NAME "tpa2028d1_r"
static struct i2c_client *g_client;
static struct vreg *v_gp2 = NULL;

#include <asm/mach-types.h>

/* for voice */
static char en_data_4voice[] = 
{
    /* 26dB open agc */
    /* reg  val  */
    0x01, 0x83,
    0x02, 0x05,
    0x03, 0x0a,
    0x04, 0x00,
    0x05, 0x1a,
    0x06, 0x3c,
    0x07, 0x82,
    0x01, 0xc3
};
static char en_data_4voice_u8820[] = 
{
    /* 2010.12.31 renyanhui tuning for U8820 */
    /* reg  val  */
    0x01, 0x83,
    0x02, 0x05,
    0x03, 0x04,
    0x04, 0x00,
    0x05, 0x16,
    0x06, 0x7e,
    0x07, 0x30,
    0x01, 0xc3
};

/* for music */
static char en_data_4music[] = 
{
    /* open AGC 30db for playing music only */
    /* accordingly, need to check if iir filter need modify and volume percentage in hwVolumeFactor.cfg */
    /* reg  val  */
    0x01, 0x83,
    0x02, 0x05,
    0x03, 0x0a,
    0x04, 0x00,
    0x05, 0x1e,
    0x06, 0x5c,
    0x07, 0xc2,
    0x01, 0xc3
};

static char en_data_4music_u8820[] = 
{
    /* 2010.12.31 renyanhui tuning for U8820 */
    /* reg  val  */
    0x01, 0x83,
    0x02, 0x05,
    0x03, 0x04,
    0x04, 0x00,
    0x05, 0x16,
    0x06, 0x7e,
    0x07, 0x30,
    0x01, 0xc3
};

/* data  pointer */
static char* pen_data_4voice = &(en_data_4voice[0]);
static int     pen_data_4voice_size = 0;

static char* pen_data_4music = &(en_data_4music[0]);
static int     pen_data_4music_size = 0;



static int tpa2028d1_i2c_write(char *txData, int length)
{

	struct i2c_msg msg[] = {
		{
		 .addr = g_client->addr,
		 .flags = 0,
		 .len = length,
		 .buf = txData,
		 },
	};

	if (i2c_transfer(g_client->adapter, msg, 1) < 0) 
    {
		TPA_DEBUG_TPA("tpa2028d1_i2c_r_write: transfer error\n");
		return -EIO;
	} 
    else
    {
        return 0;
    }
}
static int tpa2028d1_i2c_read(char * reg, char *rxData)
{
    
	struct i2c_msg msgs[] = {
		{
		 .addr = g_client->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = reg,
		 },
		{
		 .addr = g_client->addr,
		 .flags = I2C_M_RD,
		 .len = 1,
		 .buf = rxData,
		 },
	};

	if (i2c_transfer(g_client->adapter, msgs, 2) < 0) 
    {
		TPA_DEBUG_TPA("tpa2028d1_i2c_read: transfer error\n");
		return -EIO;
	} 
    else
	{
        TPA_DEBUG_TPA("reg(0x%x)'s value:0x%x\n",*reg, *rxData);
		return 0;
    }
}
/* move up static char en_data[8][2]  */

static void tpa2028d1_amplifier_poweron(void)
{
    int ret = 0;
    v_gp2 = vreg_get(NULL,"gp2");   
    ret = IS_ERR(v_gp2); 
    if(ret)         
        goto err_power_on_failed;    
    ret = vreg_set_level(v_gp2,1800);        
    if (ret)        
        goto err_power_on_failed;    
    ret = vreg_enable(v_gp2);
    TPA_DEBUG_TPA("the power is ok\n");
    if (ret)       
        goto err_power_on_failed;
    return;
    
    err_power_on_failed:
        TPA_DEBUG_TPA("THE POWER IS FAILED!!!\n");
   
}

static void tpa2028d1_amplifier_poweroff(void)
{
    int ret = 0;
    if(NULL != v_gp2)
    {
        ret = vreg_disable(v_gp2);
        TPA_DEBUG_TPA(KERN_ERR "the power is off: gp2 = %d \n ", ret);	
    }
}



/* power on tpa2028d1 amplifier by type */
static int tpa2028d1_amplifier_on_by_type(char* pdata, int size)
{
    char* pd = pdata;
    int ret = 0;
    char i = 0;

    /*enable amplifier*/
    tpa2028d1_amplifier_poweron();

    gpio_set_value(125, 1);/* enable spkr poweramp */
    msleep(10);
    for (i = 0; i < (size/2); i++)
    {
    	 ret = tpa2028d1_i2c_write(pd, 2);
    	 if(ret)
    		break;
    	 pd += 2;
    }

    return ret;
}

static void tpa2028d1_amplifier_on(void)
{
    int ret = 0;

    /* power on tpa2028d1 amplifier by type */
    if (0 == pen_data_4voice_size)
    {
        TPA_DEBUG_TPA("failed to turn on tpa2028d1_amplifier, pen_data_4voice_size = 0.\n");
        return;
    }

    ret = tpa2028d1_amplifier_on_by_type(pen_data_4voice, pen_data_4voice_size);
    if(ret)
        TPA_DEBUG_TPA("failed to turn on tpa2028d1_amplifier\n");
    else
        TPA_DEBUG_TPA("tpa2028d1_amplifier_on\n");
    /*
    for(i = 1; i<8; i++)
    {
        tpa2028d1_i2c_read(&i, &r_data);
    }
    */
}

static void tpa2028d1_amplifier_off(void)
{
    tpa2028d1_amplifier_poweroff();
    gpio_set_value(125, 0);	/* disable spkr poweramp */
    TPA_DEBUG_TPA("tpa2028d1_r_amplifier_off\n");
}

/* move up static char en_4music_data[8][2] */

static void tpa2028d1_amplifier_4music_on(void)
{
    int ret = 0;

    /* power on tpa2028d1 amplifier by type */
    if (0 == pen_data_4music_size)
    {
        TPA_DEBUG_TPA("failed to turn on tpa2028d1_amplifier_4music, pen_data_4music_size = 0.\n");
        return;
    }

    ret = tpa2028d1_amplifier_on_by_type(pen_data_4music, pen_data_4music_size);
    if(ret)
        TPA_DEBUG_TPA("failed to turn on tpa2028d1_amplifier\n");
    else
        TPA_DEBUG_TPA("tpa2028d1_amplifier_4music_on\n");

}

static int tpa2028d1_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
       
	char r_data = 0;
	char reg = 0x01;
    int ret = 0;
    struct amplifier_platform_data *pdata = client->dev.platform_data;

	TPA_DEBUG_TPA("tpa2028d1_r_probe\n");
	
	g_client = client;

    /* power on tpa2028d1 amplifier by type */
    if (machine_is_msm7x30_u8820())
    {
        pen_data_4voice = &(en_data_4voice_u8820[0]);
        pen_data_4voice_size = ARRAY_SIZE(en_data_4voice_u8820);
        pen_data_4music = &(en_data_4music_u8820[0]);
        pen_data_4music_size = ARRAY_SIZE(en_data_4music_u8820);

        TPA_DEBUG_TPA("tpa2028d1_probe machine_is_msm7x30_u8820\n");
    }
    else
    {
        pen_data_4voice = &(en_data_4voice[0]);
        pen_data_4voice_size = ARRAY_SIZE(en_data_4voice);
        pen_data_4music = &(en_data_4music[0]);
        pen_data_4music_size = ARRAY_SIZE(en_data_4music);

        TPA_DEBUG_TPA("tpa2028d1_probe default.\n");
    }


	gpio_tlmm_config(GPIO_CFG(125, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
    gpio_set_value(125, 1);/* enable spkr poweramp */
    msleep(10);
    /*identify if this is  tpa2028d1*/
    ret = tpa2028d1_i2c_read(&reg, &r_data);
    
    gpio_set_value(125, 0);	/* disable spkr poweramp */
    if(!ret && (REG1_DEFAULT_VALUE == r_data) && pdata)
    {
        pdata->amplifier_on = tpa2028d1_amplifier_on;
        pdata->amplifier_off = tpa2028d1_amplifier_off;
        #ifdef CONFIG_HUAWEI_KERNEL
        pdata->amplifier_4music_on = tpa2028d1_amplifier_4music_on;
        #endif
    }
    return ret;
}


static int tpa2028d1_remove(struct i2c_client *client)
{
    struct amplifier_platform_data *pdata = client->dev.platform_data;
    if(pdata)
    {
        pdata->amplifier_on = NULL;
        pdata->amplifier_off = NULL;
    }
	return 0;
}


static const struct i2c_device_id tpa2028d1_id[] = {
	{TPA2028D1_I2C_NAME, 0},
	{ }
};

static struct i2c_driver tpa2028d1_driver = {
	.probe		= tpa2028d1_probe,
	.remove		= tpa2028d1_remove,
	.id_table	= tpa2028d1_id,
	.driver = {
	    .name	= TPA2028D1_I2C_NAME,
	},
};

static int __devinit tpa2028d1_r_init(void)
{
    TPA_DEBUG_TPA("add tpa2028d1_r driver\n");
	return i2c_add_driver(&tpa2028d1_driver);
}

static void __exit tpa2028d1_r_exit(void)
{
	i2c_del_driver(&tpa2028d1_driver);
}

module_init(tpa2028d1_r_init);
module_exit(tpa2028d1_r_exit);

MODULE_DESCRIPTION("tpa2028d1_r Driver");
MODULE_LICENSE("GPL");

