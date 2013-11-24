/*
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 * Author: Brian Swetland <swetland@google.com>
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

#include <linux/platform_device.h>
#include <linux/gpio_event.h>

#include <asm/mach-types.h>
#ifdef CONFIG_HUAWEI_KERNEL
#include <linux/gpio.h>
#endif

/* del umts_surf keypad configure  */

/* change array name form hw_u8815_keypad_col_gpios to default_keypad_col_gpios,
 * form hw_u8815_keypad_row_gpios to default_keypad_row_gpios,
 * form KEYMAP_U8815_INDEX to KEYMAP_DEFAULT_INDEX,
 * form hw_u8815_keypad_keymap to default_keypad_keymap,
 * form u8815_keypad_matrix_info to default_keypad_matrix_info,
 * form u8815_keypad_info to default_keypad_info,
 * form u8815_keypad_data to default_keypad_data,
 * form keypad_device_u8815 to keypad_device_default,
 * for tending to promote code unity.
 */
/* default keypad begin*/

static unsigned int default_keypad_col_gpios[] = { 42, 41 };
static unsigned int default_keypad_row_gpios[] = { 255 };

#define KEYMAP_default_INDEX(row, col) ((row)*ARRAY_SIZE(default_keypad_col_gpios) + (col))

static const unsigned short default_keypad_keymap[ARRAY_SIZE(default_keypad_col_gpios) *
					  ARRAY_SIZE(default_keypad_row_gpios)] = 
{
    /* the volume keys reverse */
	[KEYMAP_default_INDEX(0, 0)] = KEY_VOLUMEUP,
	[KEYMAP_default_INDEX(0, 1)] = KEY_VOLUMEDOWN
};

/* umts_surf keypad platform device information */
static struct gpio_event_matrix_info default_keypad_matrix_info = {
	.info.func	= gpio_event_matrix_func,
	.keymap		= default_keypad_keymap,
	.output_gpios	= default_keypad_row_gpios,
	.input_gpios	= default_keypad_col_gpios,
	.noutputs	= ARRAY_SIZE(default_keypad_row_gpios),
	.ninputs	= ARRAY_SIZE(default_keypad_col_gpios),
	.settle_time.tv64 = 40 * NSEC_PER_USEC,
	.poll_time.tv64 = 20 * NSEC_PER_MSEC,
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_PRINT_UNMAPPED_KEYS |
			  GPIOKPF_DRIVE_INACTIVE
};

static struct gpio_event_info *default_keypad_info[] = {
	&default_keypad_matrix_info.info
};

static struct gpio_event_platform_data default_keypad_data = {
	.name		= "default_keypad",
	.info		= default_keypad_info,
	.info_count	= ARRAY_SIZE(default_keypad_info)
};

struct platform_device keypad_device_default = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &default_keypad_data,
	},
};
/* default keypad end */
/* because all production KEY_VOLUMEUP and KEY_VOLUMEDOWN sameness,
 * so use keypad_device_default ,
 * del keypad_device_u8655, del 36 row,
 * for tending to promote code unity.
 */

/* U8185 keypad begin*/

static unsigned int u8185_keypad_col_gpios[] = { 42, 41, 40 };
static unsigned int u8185_keypad_row_gpios[] = { 255 };

#define KEYMAP_U8185_INDEX(row, col) ((row)*ARRAY_SIZE(u8185_keypad_col_gpios) + (col))

static const unsigned short u8185_keypad_keymap[ARRAY_SIZE(u8185_keypad_col_gpios) *
					  ARRAY_SIZE(u8185_keypad_row_gpios)] = 
{
    /* the volume keys reverse */
	[KEYMAP_U8185_INDEX(0, 0)] = KEY_VOLUMEUP,
	[KEYMAP_U8185_INDEX(0, 1)] = KEY_VOLUMEDOWN,
	[KEYMAP_U8185_INDEX(0, 2)] = KEY_HOME
};

/* umts_surf keypad platform device information */
static struct gpio_event_matrix_info u8185_keypad_matrix_info = {
	.info.func	= gpio_event_matrix_func,
	.keymap		= u8185_keypad_keymap,
	.output_gpios	= u8185_keypad_row_gpios,
	.input_gpios	= u8185_keypad_col_gpios,
	.noutputs	= ARRAY_SIZE(u8185_keypad_row_gpios),
	.ninputs	= ARRAY_SIZE(u8185_keypad_col_gpios),
	.settle_time.tv64 = 40 * NSEC_PER_USEC,
	.poll_time.tv64 = 20 * NSEC_PER_MSEC,
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_PRINT_UNMAPPED_KEYS |
			  GPIOKPF_DRIVE_INACTIVE
};

static struct gpio_event_info *u8185_keypad_info[] = {
	&u8185_keypad_matrix_info.info
};

static struct gpio_event_platform_data u8185_keypad_data = {
	.name		= "u8185_keypad",
	.info		= u8185_keypad_info,
	.info_count	= ARRAY_SIZE(u8185_keypad_info)
};

struct platform_device keypad_device_u8185 = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &u8185_keypad_data,
	},
};
/* U8185 keypad end */
static unsigned int y300j1_keypad_col_gpios[] = { 42, 41, 36 };
static unsigned int y300j1_keypad_row_gpios[] = { 255 };

#define KEYMAP_Y300J1_INDEX(row, col) ((row)*ARRAY_SIZE(y300j1_keypad_col_gpios) + (col))

static const unsigned short y300j1_keypad_keymap[ARRAY_SIZE(y300j1_keypad_col_gpios) *
					  ARRAY_SIZE(y300j1_keypad_row_gpios)] = 
{
	/* the volume keys reverse */
	[KEYMAP_Y300J1_INDEX(0, 0)] = KEY_VOLUMEUP,
	[KEYMAP_Y300J1_INDEX(0, 1)] = KEY_VOLUMEDOWN,
	[KEYMAP_Y300J1_INDEX(0, 2)] = KEY_F10
};

/* umts_surf keypad platform device information */
static struct gpio_event_matrix_info y300j1_keypad_matrix_info = {
	.info.func	= gpio_event_matrix_func,
	.keymap		= y300j1_keypad_keymap,
	.output_gpios	= y300j1_keypad_row_gpios,
	.input_gpios	= y300j1_keypad_col_gpios,
	.noutputs	= ARRAY_SIZE(y300j1_keypad_row_gpios),
	.ninputs	= ARRAY_SIZE(y300j1_keypad_col_gpios),
	.settle_time.tv64 = 40 * NSEC_PER_USEC,
	.poll_time.tv64 = 20 * NSEC_PER_MSEC,
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_PRINT_UNMAPPED_KEYS |
			  GPIOKPF_DRIVE_INACTIVE
};
#ifdef CONFIG_HUAWEI_KERNEL
static int mute_mode = 0;
static struct platform_device *mute_key_state = NULL;
static ssize_t mute_mode_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
	return count;
}
static ssize_t mute_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct gpio_event_input_info *data = platform_get_drvdata(mute_key_state);
	mute_mode = gpio_get_value(data->keymap[0].gpio) ^ 
				!(data->flags & GPIOEDF_ACTIVE_HIGH);
	return sprintf(buf, "%u\n", mute_mode);
}
static DEVICE_ATTR(mute_mode,0664,mute_mode_show, mute_mode_store);
int get_init_mute_mode(struct gpio_event_input_devs *input_devs,struct gpio_event_input_info *data)
{
	int ret = 0;
	int dev = 0;

	dev = data->keymap[0].dev;
	mute_key_state = platform_device_register_simple("mute_key_state",-1,NULL,0);
	if(IS_ERR(mute_key_state))
	{
		printk(KERN_ALERT "mute_key_state: platform_device_register_simple error\n");
		return -1;
	}
	platform_set_drvdata(mute_key_state,data);
	ret = device_create_file(&mute_key_state->dev,&dev_attr_mute_mode);
	if(ret)
	{
		printk(KERN_ALERT "get mute mode: device_create_file failed!\n");
		return -1;
	}
	return ret;
}
#endif

static const struct gpio_event_direct_entry y300j1_gpio_keymap[] = {
	{
		.gpio = 38,
		.code = SW_MUTE,
		.dev  = 1,
	},
};
static struct gpio_event_input_info y300j1_key_gpio = {
	.info.func	= gpio_event_input_func,
	.info.no_suspend = true,		//set true so the irq will wake up the device
	.debounce_time.tv64 = 10 * NSEC_PER_MSEC,
	.flags = GPIOEDF_ACTIVE_HIGH,    //change to ACTIVE HIGH
	.type = EV_SW,
	.keymap = y300j1_gpio_keymap,
	.keymap_size = ARRAY_SIZE(y300j1_gpio_keymap),
#ifdef CONFIG_HUAWEI_KERNEL
	.func = get_init_mute_mode
#endif
};
static struct gpio_event_info *y300j1_keypad_info[] = {
	&y300j1_keypad_matrix_info.info,
	&y300j1_key_gpio.info
};

static struct gpio_event_platform_data y300j1_keypad_data = {
	.names		= {
		"y300j1_keypad",
		"y300j1_mute",
		NULL
	},
	.info		= y300j1_keypad_info,
	.info_count	= ARRAY_SIZE(y300j1_keypad_info)
};

struct platform_device keypad_device_y300j1 = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &y300j1_keypad_data,
	},
};
