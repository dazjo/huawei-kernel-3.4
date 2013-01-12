/*add by zhangtao for ptt leds*/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/device.h>
#include <mach/board.h>
#include <linux/uaccess.h>
	
#include <linux/workqueue.h>
#include <linux/string.h>
#include <linux/leds.h>
#include <mach/vreg.h>
#include <mach/mpp.h>
#include <linux/platform_device.h>
#include <mach/pmic.h>
#include <linux/hardware_self_adapt.h>
//#define PTT_LED_DEBUG
#ifdef PTT_LED_DEBUG
#define PTT_PRINT(x...) do{ \
		printk(KERN_INFO "[PTT_LED] "x); \
	}while(0)
#else
#define PTT_PRINT(x...) do{}while(0)
#endif
/*we use the set function to contrl the light is on or off*/
static void set_ptt_brightness(struct led_classdev *led_cdev,enum led_brightness OnOff)

									
{
	int ret = 0;

	PTT_PRINT("%s: OnOff = %d\n",__func__, OnOff);
	ret = pmic_set_ptt_current_led_intensity(OnOff);


	if(ret)
	{
	PTT_PRINT("%s: failed\n",__func__);
	return;
	}

	PTT_PRINT("%s: success\n",__func__);

}
static int ptt_leds_probe(struct platform_device *pdev)
{
	int rc = -ENODEV;
	
	struct led_classdev *ptt_data = NULL;

	ptt_data = kzalloc(sizeof(*ptt_data), GFP_KERNEL);
	
	if (ptt_data == NULL) {
		rc = -ENOMEM;
		goto err_alloc_failed;
	}

	platform_set_drvdata(pdev, ptt_data);
	
	ptt_data->name= "ptt_led";
	ptt_data->brightness = LED_OFF;
	ptt_data->brightness_set = set_ptt_brightness;

	rc = led_classdev_register(&pdev->dev, ptt_data);
	if (rc < 0) {
		printk(KERN_ERR "ptt_led: led_classdev_register failed\n");
		goto err_led0_classdev_register_failed;
	}
	printk("led_classdev_register sucess\n");
	
	return 0;
err_led0_classdev_register_failed:
err_alloc_failed:
	kfree(ptt_data);
	return rc;

}
static struct platform_driver ptt_leds_driver = {
	.probe = ptt_leds_probe,
	.driver = {
		   .name = "ptt-led",
		   },
};

int __init ptt_leds_init(void)
{
	int rc = -ENODEV;
	if (platform_driver_register(&ptt_leds_driver))
			return rc;
	return 0;
}

static void __exit ptt_leds_exit(void)
{
	platform_driver_unregister(&ptt_leds_driver);
}

module_init(ptt_leds_init);
module_exit(ptt_leds_exit);


