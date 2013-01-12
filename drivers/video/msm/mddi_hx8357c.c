/* drivers\video\msm\mddi_rsp61408.c
 * rsp61408 LCD driver for 7x27 platform
 *
 * Copyright (C) 2010 HUAWEI Technology Co., ltd.
 * 
 * Date: 2011/09/01
 * By jiaoshuangwei
 * 
 */

#include "msm_fb.h"
#include "mddihost.h"
#include "mddihosti.h"
#include <linux/mfd/pmic8058.h>
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/hardware_self_adapt.h>
#include <linux/pwm.h>
#include <mach/pmic.h>
#include "hw_backlight.h"
#include "hw_lcd_common.h"
#include "lcd_hw_debug.h"
struct sequence* hx8357c_hvga_init_table = NULL;
static lcd_panel_type lcd_panel_hvga = LCD_NONE;
#define PM_GPIO_24 24
#define PM_GPIO_HIGH_VALUE 1 

/*delate the initialize sequence */


static const struct sequence hx8357c_hvga_standby_exit_table[]= 
{
	{0x00011,TYPE_COMMAND,0}, //11 
	{0x00000,TYPE_PARAMETER,0},
	{0x000CC,TYPE_COMMAND,120},  //Set Panel 
	{0x00007,TYPE_PARAMETER,0},
	{0x00029,TYPE_COMMAND,0}, //29  
	{0x00000,TYPE_PARAMETER,0},
	{0x0020,TYPE_COMMAND,20},
	{0x00000,TYPE_PARAMETER,0},
	{MDDI_MULTI_WRITE_END,TYPE_COMMAND,20},
};
static const struct sequence hx8357c_hvga_standby_enter_table[]= 
{
	{0x0021,TYPE_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000CC,TYPE_COMMAND,0},  //Set Panel
	{0x00005,TYPE_PARAMETER,0},
	{0x00028,TYPE_COMMAND,0}, //29h
	{0x00000,TYPE_PARAMETER,0},
	{0x00010,TYPE_COMMAND,20},
	{0x00000,TYPE_PARAMETER,0},
	{MDDI_MULTI_WRITE_END,TYPE_COMMAND,120}, //the end flag,it don't sent to driver IC
};

static struct sequence hx8357c_hvga_write_cabc_brightness_table[]= 
{
	{0x00051,TYPE_COMMAND,0},
	{0x000ff,TYPE_PARAMETER,0},
	{MDDI_MULTI_WRITE_END,TYPE_COMMAND,0},
};
/* gamma 2.2 */
static const struct sequence hx8357c_hvga_dynamic_gamma22_table[] = {};
/* gamma1.9 */
static const struct sequence hx8357c_hvga_dynamic_gamma19_table[] = {};
/* gamma2.5 */
static const struct sequence hx8357c_hvga_dynamic_gamma25_table[] = {};

/* add the function  to set different gama by different mode */
int hx8357c_set_dynamic_gamma(enum danymic_gamma_mode  gamma_mode)
{
    int ret = 0;
	
    if (LOW_LIGHT == gamma_mode)
    {
        printk(KERN_ERR "the dynamic_gamma_setting is wrong\n");
    }

    switch(gamma_mode)
    {
        case GAMMA25:
            ret = process_mddi_table((struct sequence*)&hx8357c_hvga_dynamic_gamma25_table,
                        ARRAY_SIZE(hx8357c_hvga_dynamic_gamma25_table), lcd_panel_hvga);			
            break ;
        case GAMMA22:
			 ret = process_mddi_table((struct sequence*)&hx8357c_hvga_dynamic_gamma22_table,
                        ARRAY_SIZE(hx8357c_hvga_dynamic_gamma22_table), lcd_panel_hvga);	
            break;
        case HIGH_LIGHT:
            ret = process_mddi_table((struct sequence*)&hx8357c_hvga_dynamic_gamma19_table,
                        ARRAY_SIZE(hx8357c_hvga_dynamic_gamma19_table), lcd_panel_hvga);
            break;
        default:
            ret= -1;
            break;
    }
	LCD_DEBUG("%s: change gamma mode to %d\n",__func__,gamma_mode);
    return ret;
}

static int hx8357c_lcd_on(struct platform_device *pdev)
{
	boolean para_debug_flag = FALSE;
    uint32 para_num = 0;
	int ret = 0;

 	/* open debug file and read the para */
	para_debug_flag = lcd_debug_malloc_get_para( "hx8357c_hvga_init_table", 
		(void**)&hx8357c_hvga_init_table,&para_num);

	/* If exist the init file ,then init lcd with it for debug */
    if( (TRUE == para_debug_flag)&&(NULL != hx8357c_hvga_init_table))
    {
		ret = process_mddi_table(hx8357c_hvga_init_table, para_num, lcd_panel_hvga);
    }
    else
    {
		/* Exit Standby Mode */
		ret = process_mddi_table((struct sequence*)&hx8357c_hvga_standby_exit_table, 
			ARRAY_SIZE(hx8357c_hvga_standby_exit_table), lcd_panel_hvga);
    }
       
	/* Must malloc before,then you can call free */
	if((TRUE == para_debug_flag)&&(NULL != hx8357c_hvga_init_table))
	{
		lcd_debug_free_para((void *)hx8357c_hvga_init_table);
	}
	
    LCD_DEBUG("%s: hx8357c_lcd exit sleep mode ,on_ret=%d\n",__func__,ret);
	
	return ret;
}

static int hx8357c_lcd_off(struct platform_device *pdev)
{
	int ret = 0;
	ret = process_mddi_table((struct sequence*)&hx8357c_hvga_standby_enter_table, 
    	      		ARRAY_SIZE(hx8357c_hvga_standby_enter_table), lcd_panel_hvga);
    LCD_DEBUG("%s: hx8357c_lcd enter sleep mode ,off_ret=%d\n",__func__,ret);
	return ret;
}

static void hx8357c_set_cabc_brightness(struct msm_fb_data_type *mfd,uint32 bl_level)
{
	int ret = 0;
	hx8357c_hvga_write_cabc_brightness_table[1].reg = bl_level;
	ret = process_mddi_table((struct sequence*)&hx8357c_hvga_write_cabc_brightness_table,
                    ARRAY_SIZE(hx8357c_hvga_write_cabc_brightness_table), lcd_panel_hvga);
    return;
}

static int __devinit hx8357c_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);
 	return 0;
}

static struct platform_driver this_driver = {
	.probe  = hx8357c_probe,
	.driver = {
		.name   = "mddi_hx8357c_hvga",
	},
};

static struct msm_fb_panel_data hx8357c_panel_data = {
	.on = hx8357c_lcd_on,
	.off = hx8357c_lcd_off,
	.set_backlight = pwm_set_backlight,
	.set_cabc_brightness = hx8357c_set_cabc_brightness,
    .set_dynamic_gamma = hx8357c_set_dynamic_gamma,
};

static struct platform_device this_device = {
	.name   = "mddi_hx8357c_hvga",
	.id	= 0,
	.dev	= {
		.platform_data = &hx8357c_panel_data,
	}
};
static int __init hx8357c_init(void)
{
	int ret = 0;
	struct msm_panel_info *pinfo = NULL;
	bpp_type bpp = MDDI_OUT_24BPP;		
	hw_lcd_interface_type mddi_port_type = get_hw_lcd_interface_type();

	lcd_panel_hvga = get_lcd_panel_type();
	
	if( (MDDI_HX8357C_CHIMEI_HVGA != lcd_panel_hvga) 
		&& (MDDI_HX8357C_TIANMA_HVGA != lcd_panel_hvga)
		&& (MDDI_HX8357C_CHIMEI_IPS_HVGA != lcd_panel_hvga))
	{
		return 0;
	}

	LCD_DEBUG("%s:start init %s\n",__func__,this_device.name);
	/* Select which bpp accroding MDDI port type */
	if(LCD_IS_MDDI_TYPE1 == mddi_port_type)
	{
		bpp = MDDI_OUT_16BPP;
	}
	else if(LCD_IS_MDDI_TYPE2 == mddi_port_type)
	{
		bpp = MDDI_OUT_24BPP;
	}
	else
	{
		bpp = MDDI_OUT_16BPP;
	}
	
	ret = platform_driver_register(&this_driver);
	if (!ret) 
	{
		pinfo = &hx8357c_panel_data.panel_info;
		pinfo->xres = 320;
		pinfo->yres = 480;
		pinfo->type = MDDI_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->mddi.vdopkt = MDDI_DEFAULT_PRIM_PIX_ATTR;
		pinfo->wait_cycle = 0;
		pinfo->bpp = (uint32)24;
		pinfo->fb_num = 2;
        pinfo->clk_rate = 160000000;
	    pinfo->clk_min = 160000000;
	    pinfo->clk_max = 160000000;
        pinfo->lcd.vsync_enable = TRUE;
        pinfo->lcd.refx100 = 6000;
		pinfo->lcd.v_back_porch = 0;
		pinfo->lcd.v_front_porch = 0;
		pinfo->lcd.v_pulse_width = 22;
		pinfo->lcd.hw_vsync_mode = TRUE;
		pinfo->lcd.vsync_notifier_period = 0;
		pinfo->bl_max = 255;

		ret = platform_device_register(&this_device);
		if (ret)
		{
			platform_driver_unregister(&this_driver);
		}
	}

	return ret;
}
module_init(hx8357c_init);

