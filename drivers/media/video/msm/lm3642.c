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
#include <asm/gpio.h>
#include <linux/gpio.h>
#include <linux/types.h>
#include <linux/platform_device.h> 
#include <mach/camera.h>
#include <asm/mach-types.h>
#include <linux/hardware_self_adapt.h> 

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <linux/hw_dev_dec.h>
#endif

#define lm3642_strobe	111
#define lm3642_torch    117
#define FLASH_CHIP_ID 0x00
#define FLASH_CHIP_ID_MASK 0x07

#define LM3642_ERROR -1
#define LM3642_NORMAL 0

struct i2c_client *lm3642_client;

static int lm3642_i2c_write(struct i2c_client *client, uint8_t reg, uint8_t val)
{
	int ret = LM3642_NORMAL;

	if (NULL == lm3642_client)
	{ 
		printk("%s : lm3642_client is NULL!\n", __func__);
		ret = LM3642_ERROR;
	}
	else
	{	
		ret = i2c_smbus_write_byte_data(lm3642_client, reg, val);
		
		if (ret < LM3642_NORMAL) 
		{
			printk("%s : I2C write error.\n", __func__);
		}
	}

	return ret;
}

static int lm3642_i2c_read(struct i2c_client *client, uint8_t reg)
{
	int ret = LM3642_NORMAL;
	
	if (NULL == lm3642_client)
	{ 
		printk("%s : lm3642_client is NULL!\n", __func__);
		ret = LM3642_ERROR;
	}
	else
	{
		ret = i2c_smbus_read_byte_data(lm3642_client, reg);
		
		if (ret >= LM3642_NORMAL) 
		{
			CDBG("%s : reg[%x] = %x\n", __func__, reg, ret);
		} 
		else 
		{
			printk("%s : I2C read error.\n", __func__);
		}
	}
		
	return ret;
}

/*set flash's states*/
int lm3642_set_flash(unsigned led_state)
{
    int rc = LM3642_NORMAL;

    CDBG("%s : led_state = %d\n", __func__, led_state);

    switch (led_state)
    {
	//50mA
	case MSM_CAMERA_LED_TORCH:
	case MSM_CAMERA_LED_TORCH_LOW:
		rc = lm3642_i2c_write( lm3642_client, 0x09, 0x00 );
		rc = lm3642_i2c_write( lm3642_client, 0x0A, 0x12 );
		gpio_set_value(lm3642_torch, 1);
        break;
	//100mA
	case MSM_CAMERA_LED_TORCH_MIDDLE: 
		rc = lm3642_i2c_write( lm3642_client, 0x09, 0x10 );
		rc = lm3642_i2c_write( lm3642_client, 0x0A, 0x12 );
		gpio_set_value(lm3642_torch, 1);
        break;
	//150mA
	case MSM_CAMERA_LED_TORCH_HIGH: 
		rc = lm3642_i2c_write( lm3642_client, 0x09, 0x20 );
		rc = lm3642_i2c_write( lm3642_client, 0x0A, 0x12 );
		gpio_set_value(lm3642_torch, 1);
        break;

	//100mA
	case MSM_CAMERA_LED_LOW:
		rc = lm3642_i2c_write( lm3642_client, 0x09, 0x10 );
		rc = lm3642_i2c_write( lm3642_client, 0x0A, 0x12 );
		gpio_set_value(lm3642_torch, 1);
        break;
		
	//650mA
	case MSM_CAMERA_LED_HIGH:
		rc = lm3642_i2c_write( lm3642_client, 0x09, 0x06 );
		rc = lm3642_i2c_write( lm3642_client, 0x08, 0x07 );
		rc = lm3642_i2c_write( lm3642_client, 0x0A, 0x23 );
		gpio_set_value(lm3642_strobe, 1);
        break;

	//650mA
	case MSM_CAMERA_LED_FIRST_MMI:
		rc = lm3642_i2c_write( lm3642_client, 0x09, 0x06 );
		rc = lm3642_i2c_write( lm3642_client, 0x08, 0x04 );
		rc = lm3642_i2c_write( lm3642_client, 0x0A, 0x23 );
		gpio_set_value(lm3642_strobe, 1);
		usleep(1000*200);
		
		//turn off
		rc = lm3642_i2c_write( lm3642_client, 0x0A, 0x00 );  
		gpio_set_value(lm3642_strobe, 0);
        break;

	//Turn off torch and flash
	case MSM_CAMERA_LED_OFF:
	default:
		rc = lm3642_i2c_write( lm3642_client, 0x0A, 0x00 );
		gpio_set_value(lm3642_strobe, 0);
		gpio_set_value(lm3642_torch, 0);
        break;
    }

    return rc;
}

static int lm3642_device_init(void)
{	
	
	int err = LM3642_NORMAL;
	int gpio_config;

	//flash gpio config
	err = gpio_request(lm3642_strobe, "lm3642_strobe");
	if(err)
	{
		printk(KERN_ERR "%s : gpio_requset gpio %d failed, err=%d!\n", __func__, lm3642_strobe, err);
	}
	else
	{
		/*config the lm3642_strobe gpio to output state*/
		gpio_config = GPIO_CFG(lm3642_strobe, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA);
		err = gpio_tlmm_config(gpio_config, GPIO_CFG_ENABLE);
		if(err)
		{
			printk(KERN_ERR "%s : gpio_tlmm_config(#%d)=%d\n", __func__, lm3642_strobe, err);
		}
	}	

	//torch gpio config
	err = gpio_request(lm3642_torch, "lm3642_strobe");
	if(err)
	{
		printk(KERN_ERR "%s : gpio_requset gpio %d failed, err=%d!\n", __func__, lm3642_torch, err);
	}
	else
	{
		/*config the lm3642_torch gpio to output state*/
		gpio_config = GPIO_CFG(lm3642_torch, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA);
		err = gpio_tlmm_config(gpio_config, GPIO_CFG_ENABLE);
		if (err) 
		{
			printk(KERN_ERR "%s : gpio_tlmm_config(#%d)=%d\n", __func__, lm3642_torch, err);
		}
	}	
				
	return err;
}

static int lm3642_probe(struct i2c_client *client,
			       const struct i2c_device_id *devid)
{
	int ret = LM3642_ERROR;
	int tempvalue = LM3642_ERROR;
	
	CDBG("%s : probe start!\n", __func__);

	lm3642_client = client;

	/*device init of gpio init*/
	if ( lm3642_device_init() )
	{
		printk("%s : Device init error!\n", __func__);	
	}
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		printk("%s : I2C error1!\n", __func__);
		ret = -ENODEV;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_I2C_BLOCK))
	{
		printk("%s : I2C error2!\n", __func__);
		ret = -ENODEV;
	}
	
	/* read chip id */
	tempvalue = lm3642_i2c_read(lm3642_client, 0x00);

	if ( FLASH_CHIP_ID == (tempvalue & FLASH_CHIP_ID_MASK) )
	{
		CDBG("%s : Read chip id ok!Chip ID is %d.\n", __func__, tempvalue);
		register_led_set_state(lm3642_set_flash);
		
		#ifdef CONFIG_HUAWEI_HW_DEV_DCT
		/* detect current device successful, set the flag as present */
		set_hw_dev_flag(DEV_I2C_FLASH);
		#endif
		
		CDBG("%s : probe succeed!\n", __func__);
		ret = LM3642_NORMAL;
	} 
	else 
	{
		printk("%s : read chip id error!Chip ID is %d.\n", __func__, tempvalue);
		ret = -ENODEV;
	}

	return ret;
}

static int lm3642_remove(struct i2c_client *client)
{
	CDBG("%s : lm3642 driver removing\n", __func__);
	gpio_free(lm3642_strobe);
	gpio_free(lm3642_torch);
	return 0;
}

static const struct i2c_device_id lm3642_id[] = {
	{ "lm3642", 0 },
	{},
};

MODULE_DEVICE_TABLE(i2c, lm3642_id);

static struct i2c_driver lm3642_driver = {
	.probe = lm3642_probe,
	.remove = __devexit_p(lm3642_remove),
	.id_table = lm3642_id,
	.driver = {
	.owner = THIS_MODULE,
	.name = "lm3642",
	},
};

static int __init lm3642_init(void)
{
	CDBG(KERN_INFO "lm3642 init driver\n");
	return i2c_add_driver(&lm3642_driver);
}

static void __exit lm3642_exit(void)
{
	CDBG(KERN_INFO "lm3642 exit\n");
	i2c_del_driver(&lm3642_driver);
	return;
}

module_init(lm3642_init);
module_exit(lm3642_exit);

MODULE_DESCRIPTION("lm3642 light driver");
MODULE_LICENSE("GPL");
