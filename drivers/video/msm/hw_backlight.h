/* drivers\video\msm\hw_backlight.h
 * backlight driver for 7x30 platform
 *
 * Copyright (C) 2010 HUAWEI Technology Co., ltd.
 * 
 * Date: 2010/12/07
 * By lijianzhao
 * 
 */
#ifndef HW_BACKLIGHT_H
#define HW_BACKLIGHT_H
#define LCD_MIN_BACKLIGHT_LEVEL 0
#define LCD_MAX_BACKLIGHT_LEVEL	255
void pwm_set_backlight (struct msm_fb_data_type * mfd);

#endif
