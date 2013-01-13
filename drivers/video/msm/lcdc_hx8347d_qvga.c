/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * Alternatively, provided that this notice is retained in full, this software
 * may be relicensed by the recipient under the terms of the GNU General Public
 * License version 2 ("GPL") and only version 2, in which case the provisions of
 * the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
 * software under the GPL, then the identification text in the MODULE_LICENSE
 * macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a
 * recipient changes the license terms to the GPL, subsequent recipients shall
 * not relicense under alternate licensing terms, including the BSD or dual
 * BSD/GPL terms.  In addition, the following license statement immediately
 * below and between the words START and END shall also then apply when this
 * software is relicensed under the GPL:
 *
 * START
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 and only version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * END
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <linux/delay.h>
#include <mach/gpio.h>
#include "msm_fb.h"
#include "hw_lcd_common.h"

#define LCD_DRIVER_NAME "lcdc_hx8347d_qvga"

static int lcd_reset_gpio;
static struct lcd_state_type hx8347d_qvga_state = { 0 };
static struct msm_panel_common_pdata *lcdc_hx8347d_qvga_pdata;
static lcd_panel_type lcd_panel_qvga = LCD_NONE;

struct sequence* hx8347d_qvga_init_table = NULL;

static const struct sequence hx8347d_chimei_qvga_disp_off[] = 
{
    {0x28, 0x38, 40},
    {0x1F, 0x89, 40},
    {0x28, 0x04, 40},
    {0x28, 0x24, 40},
    {0x19, 0x00, 5},
};
static const struct sequence hx8347d_chimei_qvga_disp_on[] = 
{
    { 0x18, 0x66, 0},
    { 0x19, 0x01, 0},
    { 0x1F, 0x88, 5},
    { 0x1F, 0x80, 5},
    { 0x1F, 0x90, 5},
    { 0x1F, 0xD4, 5},
    { 0x28, 0x38, 40},
    { 0x28, 0x3C, 120},
};
static const struct sequence hx8347d_truly_qvga_disp_off[] = 
{
    {0x28, 0x38, 40},
    {0x1F, 0x89, 40},
    {0x28, 0x04, 40},
    {0x28, 0x24, 40},
    {0x19, 0x00, 5},
};
static const struct sequence hx8347d_truly_qvga_disp_on[] = 
{
    { 0x18, 0x36, 0},
    { 0x19, 0x01, 0},
    { 0x1F, 0x88, 5},
    { 0x1F, 0x80, 5},
    { 0x1F, 0x90, 5},
    { 0x1F, 0xD0, 5},
    { 0x28, 0x38, 40},
    { 0x28, 0x3C, 0},
};

static void hx8347d_qvga_disp_powerup(void)
{
    if (!hx8347d_qvga_state.disp_powered_up && !hx8347d_qvga_state.display_on) 
	{
        /* Reset the hardware first */
        /* Include DAC power up implementation here */
        hx8347d_qvga_state.disp_powered_up = TRUE;
    }
}

static void hx8347d_qvga_disp_on(void)
{
    if (hx8347d_qvga_state.disp_powered_up && !hx8347d_qvga_state.display_on) 
    {
        LCD_DEBUG("%s: disp on lcd\n", __func__);
        /* Initialize LCD */
        hx8347d_qvga_state.display_on = TRUE;
    }
}


static void hx8347d_qvga_reset(void)
{
    /* Reset LCD*/
    lcdc_hx8347d_qvga_pdata->panel_config_gpio(1);
    lcd_reset_gpio = *(lcdc_hx8347d_qvga_pdata->gpio_num + 4);
}

static int hx8347d_qvga_panel_on(struct platform_device *pdev)
{
    boolean para_debug_flag = FALSE;
    uint32 para_num = 0;
    /* open debug file and read the para */
    switch(lcd_panel_qvga)
    {
        case LCD_HX8347D_CHIMEI_QVGA:
            para_debug_flag = lcd_debug_malloc_get_para( "hx8347d_chimei_qvga_init_table", 
                                               (void**)&hx8347d_qvga_init_table,&para_num);
            break;
        case LCD_HX8347D_TRULY_QVGA:
			para_debug_flag = lcd_debug_malloc_get_para( "hx8347d_truly_qvga_init_table", 
                                               (void**)&hx8347d_qvga_init_table,&para_num);
        	break;
        default:
            break;
    }    
       
    if (!hx8347d_qvga_state.disp_initialized) 
    {
        hx8347d_qvga_reset();
        lcd_spi_init(lcdc_hx8347d_qvga_pdata);	/* LCD needs SPI */
        hx8347d_qvga_disp_powerup();
        hx8347d_qvga_disp_on();
        hx8347d_qvga_state.disp_initialized = TRUE;
        if( (TRUE == para_debug_flag) && (NULL != hx8347d_qvga_init_table))
        {
            // lcd_reset();
            process_lcdc_table(hx8347d_qvga_init_table, para_num,lcd_panel_qvga);
        }
        LCD_DEBUG("%s: hx8347d lcd initialized\n", __func__);
    } 
    else if (!hx8347d_qvga_state.display_on) 
    {
    	switch(lcd_panel_qvga)
		{
			case LCD_HX8347D_CHIMEI_QVGA:
                if( (TRUE == para_debug_flag)&&(NULL != hx8347d_qvga_init_table))
                {
                     //  lcd_reset();
                     process_lcdc_table(hx8347d_qvga_init_table, para_num,lcd_panel_qvga);
                }
                else
                {
                    /* Exit Standby Mode */
                     process_lcdc_table((struct sequence*)&hx8347d_chimei_qvga_disp_on, 
                                ARRAY_SIZE(hx8347d_chimei_qvga_disp_on),lcd_panel_qvga);
                }
                break;
             case LCD_HX8347D_TRULY_QVGA:
                if( (TRUE == para_debug_flag)&&(NULL != hx8347d_qvga_init_table))
                {
                     //  lcd_reset();
                     process_lcdc_table(hx8347d_qvga_init_table, para_num,lcd_panel_qvga);
                }
                else
                {
                    /* Exit Standby Mode */
                     process_lcdc_table((struct sequence*)&hx8347d_truly_qvga_disp_on, 
                                ARRAY_SIZE(hx8347d_truly_qvga_disp_on),lcd_panel_qvga);
                }
                break;
            default:
                break;
		}
        LCD_DEBUG("%s: Exit Standby Mode\n", __func__);	
        hx8347d_qvga_state.display_on = TRUE;
    } 
/* Must malloc before,then you can call free */
	if((TRUE == para_debug_flag)&&(NULL != hx8347d_qvga_init_table))
	{
		lcd_debug_free_para((void *)hx8347d_qvga_init_table);
	}
    return 0;
}

static int hx8347d_qvga_panel_off(struct platform_device *pdev)
{
    if (hx8347d_qvga_state.disp_powered_up && hx8347d_qvga_state.display_on) 
	{
        /* Enter Standby Mode */
		switch (lcd_panel_qvga)
		{
			case LCD_HX8347D_CHIMEI_QVGA:
		        process_lcdc_table((struct sequence*)&hx8347d_chimei_qvga_disp_off, ARRAY_SIZE(hx8347d_chimei_qvga_disp_off),lcd_panel_qvga);
				break;	
			case LCD_HX8347D_TRULY_QVGA:
		        process_lcdc_table((struct sequence*)&hx8347d_truly_qvga_disp_off, ARRAY_SIZE(hx8347d_truly_qvga_disp_off),lcd_panel_qvga);
				break;			
			default:
				break;
		}
		
        hx8347d_qvga_state.display_on = FALSE;
        LCD_DEBUG("%s: Enter Standby Mode\n", __func__);
    }
    return 0;
}

static struct sequence hx8347d_pwm_set_table[] =
{
	//CABC pwm 
    {0x3C, 0xFF, 10},
};
void hx8347d_set_cabc_backlight(struct msm_fb_data_type *mfd,uint32 bl_level)
{
	/*warning reg :0x3c*/
	hx8347d_pwm_set_table[0].value = bl_level;

	process_lcdc_table((struct sequence*)&hx8347d_pwm_set_table, ARRAY_SIZE(hx8347d_pwm_set_table),lcd_panel_qvga);
}

static int __devinit hx8347d_qvga_probe(struct platform_device *pdev)
{
    if (pdev->id == 0) 
	{
        lcdc_hx8347d_qvga_pdata = pdev->dev.platform_data;
        return 0;
    }
    msm_fb_add_device(pdev);
    return 0;
}

static struct platform_driver this_driver = {
    .probe  = hx8347d_qvga_probe,
    .driver = {
    	.name   = LCD_DRIVER_NAME,
    },
};

static struct msm_fb_panel_data hx8347d_qvga_panel_data = {
    .on = hx8347d_qvga_panel_on,
    .off = hx8347d_qvga_panel_off,
    .set_backlight = pwm_set_backlight,
    .set_cabc_brightness = hx8347d_set_cabc_backlight,
};

static struct platform_device this_device = {
    .name   = LCD_DRIVER_NAME,
    .id		= 1,
    .dev	= {
    	.platform_data = &hx8347d_qvga_panel_data,
    }
};

static int __init hx8347d_qvga_panel_init(void)
{
    int ret;
    struct msm_panel_info *pinfo;

    lcd_panel_qvga = get_lcd_panel_type();
	
    if((LCD_HX8347D_CHIMEI_QVGA!= lcd_panel_qvga) && (LCD_HX8347D_TRULY_QVGA!= lcd_panel_qvga) && (msm_fb_detect_client(LCD_DRIVER_NAME)) )
    {
        return 0;
    }
    LCD_DEBUG(" lcd_type=%s, lcd_panel_qvga = %d\n", LCD_DRIVER_NAME, lcd_panel_qvga);
    
    ret = platform_driver_register(&this_driver);
    if (ret)
        return ret;

    pinfo = &hx8347d_qvga_panel_data.panel_info;
    pinfo->xres = 240;
    pinfo->yres = 320;
    pinfo->type = LCDC_PANEL;
    pinfo->pdest = DISPLAY_1;
    pinfo->wait_cycle = 0;
    pinfo->bpp = 18;
    pinfo->fb_num = 2;
    pinfo->bl_max = LCD_MAX_BACKLIGHT_LEVEL;
    pinfo->bl_min = LCD_MIN_BACKLIGHT_LEVEL;

    pinfo->clk_rate = 6125000;  /*for QVGA pixel clk*/   
    pinfo->lcdc.h_back_porch = 2;
    pinfo->lcdc.h_front_porch = 2;
    pinfo->lcdc.h_pulse_width = 2;
    pinfo->lcdc.v_back_porch = 2;
    pinfo->lcdc.v_front_porch = 2;
    pinfo->lcdc.v_pulse_width = 2;

    pinfo->lcdc.border_clr = 0;     /* blk */
    pinfo->lcdc.underflow_clr = 0xff;       /* blue */
    pinfo->lcdc.hsync_skew = 0;

    ret = platform_device_register(&this_device);
    if (ret)
        platform_driver_unregister(&this_driver);

    return ret;
}

module_init(hx8347d_qvga_panel_init);
