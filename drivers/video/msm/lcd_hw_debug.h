/* kernel\drivers\video\msm\lcd_hw_debug.h
 * this file is used by the driver team to change the 
 * LCD init parameters by putting a config file in the mobile,
 * this function can make the LCD parameter debug easier.
 * 
 * Copyright (C) 2010 HUAWEI Technology Co., ltd.
 * 
 * Date: 2010/12/10
 * By genghua
 * 
 */

#ifndef __HW_LCD_DEBUG__
#define __HW_LCD_DEBUG__

#include <linux/syscalls.h>
/* modify for 4125 baseline */
#include <linux/slab.h>
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct sequence{
    uint32_t reg;
    uint32_t value;
    uint32_t time; //unit is ms
};
/* close hx8369a ESD control macro */
#define LCD_HX8369A_TIANMA_ESD_SIGN		0
/* close otm8009a ESD control macro */
#define LCD_OTM8009A_CMI_ESD_SIGN	1
#if (LCD_HX8369A_TIANMA_ESD_SIGN || LCD_OTM8009A_CMI_ESD_SIGN)
struct read_sequence{
	uint32_t reg;  //register
	uint32_t value; //register's type
	uint32_t len; // length of read parameters
};
#endif
#define HW_LCD_INIT_TEST_PARAM "/data/hw_lcd_init_param.txt"
#define HW_LCD_CONFIG_TABLE_MAX_NUM 600
#define HW_LCD_CONFIGLINE_MAX 100

bool lcd_debug_malloc_get_para(char *para_name, void **para_table,uint32_t *para_num);
bool lcd_debug_free_para(void *para_table);

#endif 

