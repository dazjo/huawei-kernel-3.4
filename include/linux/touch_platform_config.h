/* Optimize code */
/*
 * include/linux/touch_platfrom_config.h - platform data structure for touchscreen
 *
 * Copyright (C) 2010 Google, Inc.
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
#ifndef _TOUCH_PLATFORM_CONFIG_H
#define IRQ_GPIO_START 64
#define irq_to_gpio(irq)	((irq) - IRQ_GPIO_START)
/*define some tp type*/
#define LCD_X_QVGA  	   240
#define LCD_Y_QVGA    	   320
#define LCD_X_HVGA     320
#define LCD_Y_HVGA     480
#define LCD_X_WVGA     480
#define LCD_Y_WVGA     800
#define LCD_X_QHD   540
#define LCD_Y_QHD   960
/* reduce the value */
#define LCD_ALL_QHD_45INCHTP     1037
#define LCD_ALL_FWVGA_45INCHTP     922  
#define LCD_X_FWVGA 480
#define LCD_Y_FWVGA 854
#define LCD_ALL_QVGA        347
#define LCD_ALL_HVGA_35INCHTP     538 /* 3.5 INCH TP */
#define LCD_ALL_HVGA_32INCHTP     521  /* 3.2 INCH TP */
#define LCD_ALL_FWVGA 958
#define LCD_ALL_WVGA_4INCHTP     882  /* U8610 use this TP */

/*independent button num*/
#define MAX_BUTTON_NUM   4  

/*add this function for judge the tp type*/
struct tp_resolution_conversion{
    int lcd_x;
    int lcd_y;
    int lcd_all;
};

/*add for independent button*/
struct tp_button_map{
	int button_num;
	int button_map[MAX_BUTTON_NUM];
};

struct touch_hw_platform_data {
	int (*touch_power)(int on);	/* Only valid in first array entry */
	int (*set_touch_interrupt_gpio)(void);/*it will config the gpio*/
    void (*set_touch_probe_flag)(int detected);/*we use this to detect the probe is detected*/
    int (*read_touch_probe_flag)(void);/*when the touch is find we return a value*/
	int (*touch_reset)(void);
	int (*get_touch_reset_gpio)(void);
    int (*get_touch_resolution)(struct tp_resolution_conversion *tp_resolution_type);/*add this function for judge the tp type*/
	int (*read_button_flag)(void);
	int (*get_touch_button_map)(struct tp_button_map *tp_button_map);
};
#endif /*_TOUCH_PLATFORM_CONFIG_H */
