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

