/*
 * leds-msm-pmic.c - MSM PMIC LEDs driver.
 *
 * Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>

#include <mach/pmic.h>
#include <mach/rpc_pmapp.h>

#include <linux/module.h>
#ifdef CONFIG_HUAWEI_LEDS_PMIC
#include <linux/mfd/pmic8058.h>
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <linux/gpio.h>
#include <linux/pwm.h>
#include <mach/pmic.h>

#include <linux/hardware_self_adapt.h>
#include <asm/mach-types.h>
#endif
#define MAX_KEYPAD_BL_LEVEL	16

/*U8860lp use GPIO 25 Of PIMIC to driver the led backlight, Then configure it as PWM to driver LED*/
#ifdef CONFIG_HUAWEI_LEDS_PMIC
#define LED_PWM_PERIOD ( NSEC_PER_SEC / ( 22 * 1000 ) )	/* ns, period of 22Khz */
#define LED_PWM_LEVEL 255
#define LED_PWM_DUTY_LEVEL (LED_PWM_PERIOD / LED_PWM_LEVEL)
#define LED_PM_GPIO25_PWM_ID  1
#define LED_ADD_VALUE			4
#define LED_PWM_LEVEL_ADJUST	226
#define LED_BL_MIN_LEVEL 	    30
#ifdef CONFIG_HUAWEI_KERNEL
#define LED_BRIGHTNESS_LEVEL  2
#define LED_BRIGHTNESS_LEVEL_U8680 12
#define LED_BRIGHTNESS_LEVEL_U8667 20
#define LED_BRIGHTNESS_OFF    0
#endif

static struct pwm_device *bl_pwm;

/*configure GPIO 25 Of PIMIC as PWM to driver LED*/
int led_pwm_gpio_config(void)
{
    int rc;
    struct pm_gpio backlight_drv = 
    {
        .direction      = PM_GPIO_DIR_OUT,
        .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
        .output_value   = 0,
        .pull           = PM_GPIO_PULL_NO,
        .vin_sel        = 0,
        .out_strength   = PM_GPIO_STRENGTH_HIGH,
        .function       = PM_GPIO_FUNC_2,
        .inv_int_pol 	= 1,
    };
    if(machine_is_msm8255_u8860lp()
    || machine_is_msm8255_u8860_r()
	 ||machine_is_msm8255_u8860_51())
    {
        rc = pm8xxx_gpio_config( 24, &backlight_drv);
    }
    else
    {
        rc = -1;
    }
	
    if (rc) 
    {
        pr_err("%s LED backlight GPIO config failed\n", __func__);
        return rc;
    }
    return 0;
}
#endif

static void msm_keypad_bl_led_set(struct led_classdev *led_cdev,
	enum led_brightness value)
{
#ifdef CONFIG_HUAWEI_LEDS_PMIC
    int ret = 0;
/* 7x27a platform use mpp7 as keypad backlight */
	#ifdef CONFIG_ARCH_MSM7X27A
        /* use pwm to control the brightness of keypad backlight*/
		/* make sure the led is drived by pwm when */
        /* the system sleep indicator switch is on */
        pmapp_button_backlight_init();

        ret = pmapp_button_backlight_set_brightness(value);
	#else
	    if(machine_is_msm7x30_u8800() || machine_is_msm7x30_u8800_51() || machine_is_msm8255_u8800_pro() ) 
	    {
	      ret = pmic_set_led_intensity(LED_KEYPAD, !( ! value));
	    }
	    else if( machine_is_msm8255_u8860lp()	
        || machine_is_msm8255_u8860_r()
		       ||machine_is_msm8255_u8860_51())
	    {
	        pwm_config(bl_pwm, LED_PWM_DUTY_LEVEL*value/NSEC_PER_USEC, LED_PWM_PERIOD/NSEC_PER_USEC);
	        pwm_enable(bl_pwm);
	    }
	    else if(machine_is_msm7x30_u8820()
		    || (machine_is_msm8255_u8730()))
	    {   
	      ret = pmic_set_mpp6_led_intensity(!( ! value));
	    }
		/*< when the value between 0 and 255,set the key brightness is LED_BRIGHRNESS_LEVEL or set the brightness is 0 */
		else if( machine_is_msm8255_u8860() 
		      || machine_is_msm8255_c8860() 
			  || machine_is_msm8255_u8860_92())
		{
	       if(LED_BRIGHTNESS_OFF >= value || LED_PWM_LEVEL < value )
	       {
		   	   ret = pmic_set_keyled_intensity(LED_KEYPAD,LED_BRIGHTNESS_OFF  );
	       }
		   else 
		   {
		   	   ret = pmic_set_keyled_intensity(LED_KEYPAD, LED_BRIGHTNESS_LEVEL);
		   }
		}
    else if(machine_is_msm8255_u8680())
    {   
	    /* Set keypad led brightness level 12 for U8680 */
        if(LED_BRIGHTNESS_OFF >= value || LED_PWM_LEVEL < value)
        {
            ret = pmic_set_keyled_intensity(LED_KEYPAD,LED_BRIGHTNESS_OFF);
        }
        else 
        {
            ret = pmic_set_keyled_intensity(LED_KEYPAD, LED_BRIGHTNESS_LEVEL_U8680);
        }	
    }
    else if(machine_is_msm8255_u8667())
    {   
        /* Set keypad led brightness level 16 for U8667 */
        if(LED_BRIGHTNESS_OFF >= value || LED_PWM_LEVEL < value)
        {
            ret = pmic_set_keyled_intensity(LED_KEYPAD, LED_BRIGHTNESS_OFF);
        }
        else 
        {
            ret = pmic_set_keyled_intensity(LED_KEYPAD, LED_BRIGHTNESS_LEVEL_U8667);
        }	
    }
	#endif
    if (ret)
		dev_err(led_cdev->dev, "can't set keypad backlight\n");
#else
	int ret;

	ret = pmic_set_led_intensity(LED_KEYPAD, value / MAX_KEYPAD_BL_LEVEL);
	if (ret)
		dev_err(led_cdev->dev, "can't set keypad backlight\n");
#endif
}

static struct led_classdev msm_kp_bl_led = {
#ifdef CONFIG_HUAWEI_LEDS_PMIC
	.name			= "button-backlight",
#else
	.name			= "keyboard-backlight",
#endif
	.brightness_set		= msm_keypad_bl_led_set,
	.brightness		= LED_OFF,
};

static int msm_pmic_led_probe(struct platform_device *pdev)
{
	int rc;

	rc = led_classdev_register(&pdev->dev, &msm_kp_bl_led);
	if (rc) {
		dev_err(&pdev->dev, "unable to register led class driver\n");
		return rc;
	}
#ifdef CONFIG_HUAWEI_LEDS_PMIC
    if( machine_is_msm8255_u8860lp()	
    || machine_is_msm8255_u8860_r()
	  ||machine_is_msm8255_u8860_51())
    {
        led_pwm_gpio_config();   
        bl_pwm = pwm_request(LED_PM_GPIO25_PWM_ID, "keypad backlight");
    }
#endif
    /* use pwm to control the brightness of keypad backlight*/
    pmapp_button_backlight_init();
	msm_keypad_bl_led_set(&msm_kp_bl_led, LED_OFF);
	return rc;
}

static int __devexit msm_pmic_led_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&msm_kp_bl_led);

	return 0;
}

#ifdef CONFIG_PM
static int msm_pmic_led_suspend(struct platform_device *dev,
		pm_message_t state)
{
/* if phone is set in system sleep indicator mode and is sleepping,bl_pwm must be free for GPIO_24 is being controled by modem*/
#ifdef CONFIG_HUAWEI_LEDS_PMIC
    if( machine_is_msm8255_u8860lp()
    || machine_is_msm8255_u8860_r()
	 ||machine_is_msm8255_u8860_51())
    {
        pwm_free(bl_pwm);
    }
#endif
	led_classdev_suspend(&msm_kp_bl_led);

	return 0;
}

static int msm_pmic_led_resume(struct platform_device *dev)
{
/* if phone is set in system sleep indicator mode and awoke,GPIO_24 is relseased so it should be requested*/
#ifdef CONFIG_HUAWEI_LEDS_PMIC
    if( machine_is_msm8255_u8860lp()
    || machine_is_msm8255_u8860_r()
	 ||machine_is_msm8255_u8860_51())
    {
        led_pwm_gpio_config();
        bl_pwm = pwm_request(LED_PM_GPIO25_PWM_ID, "keypad backlight");
    }
#endif
	led_classdev_resume(&msm_kp_bl_led);

	return 0;
}
#else
#define msm_pmic_led_suspend NULL
#define msm_pmic_led_resume NULL
#endif

static struct platform_driver msm_pmic_led_driver = {
	.probe		= msm_pmic_led_probe,
	.remove		= __devexit_p(msm_pmic_led_remove),
	.suspend	= msm_pmic_led_suspend,
	.resume		= msm_pmic_led_resume,
	.driver		= {
		.name	= "pmic-leds",
		.owner	= THIS_MODULE,
	},
};

static int __init msm_pmic_led_init(void)
{
	return platform_driver_register(&msm_pmic_led_driver);
}
module_init(msm_pmic_led_init);

static void __exit msm_pmic_led_exit(void)
{
	platform_driver_unregister(&msm_pmic_led_driver);
}
module_exit(msm_pmic_led_exit);

MODULE_DESCRIPTION("MSM PMIC LEDs driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:pmic-leds");
