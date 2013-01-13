/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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
#include<linux/gpio.h>
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "hw_lcd_common.h"


#define LCD_DEVICE_NAME "mipi_cmd_hx8357c_hvga"

static lcd_panel_type lcd_panel_hvga = LCD_NONE;

/*mipi dsi register setting , help qualcomm to set.*/
static struct mipi_dsi_phy_ctrl dsi_cmd_mode_phy_db = 
{
    /* DSI Bit Clock at 250 MHz, 1 lane, RGB888 */ 
    /* regulator */ 
    {0x03, 0x01, 0x01, 0x00}, 
    /* timing */ 
    {0x51, 0x25, 0x9, 0x00, 0x2E, 0x36, 0xD, 0x29, 
    0xB, 0x3, 0x04}, 
    /* phy ctrl */ 
    {0x7f, 0x00, 0x00, 0x00}, 
    /* strength */ 
    {0xbb, 0x02, 0x06, 0x00}, 
    /* pll control */ 
    {0x01, 0xF6, 0x30, 0xd2, 0x00, 0x40, 0x37, 0x62, 
    0x01, 0x0f, 0x0f, 
    0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0},
};

static struct dsi_buf hx8357c_tx_buf;
static struct sequence * hx8357c_lcd_init_table_debug = NULL;

/*LCD init code*/
static const struct sequence hx8357c_hvga_standby_enter_table[]= 
{
	/*set the delay time 100ms*/
	/* Fixed LCD Flicker */
	{0x0021,MIPI_DCS_COMMAND,0},
	{0x00028,MIPI_DCS_COMMAND,0}, //28h
	{0x0010,MIPI_DCS_COMMAND,20},


	{0x00029,MIPI_TYPE_END,120}, // add new command for 
};
/* let the reset go ,so remove this code */

static const struct sequence hx8357c_hvga_standby_exit_table[]= 
{
	/*set the delay time 100ms*/
	{0x00011,MIPI_DCS_COMMAND,0}, //29h
	{0x0029,MIPI_DCS_COMMAND,120},
	/* Fixed LCD Flicker */
	{0x0020,MIPI_DCS_COMMAND,20},
	{0x00029,MIPI_TYPE_END,20}, // add new command for 
};
/*lcd resume function*/
static int mipi_hx8357c_lcd_on(struct platform_device *pdev)
{
	
	/*delete some lines */
	boolean para_debug_flag = FALSE;
	uint32 para_num = 0;
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL; 

	/*delete some lines */
	para_debug_flag = lcd_debug_malloc_get_para( "hx8357c_lcd_init_table_debug", 
            (void**)&hx8357c_lcd_init_table_debug,&para_num);

    if( (TRUE == para_debug_flag) && (NULL != hx8357c_lcd_init_table_debug))
    {
        process_mipi_table(mfd,&hx8357c_tx_buf,hx8357c_lcd_init_table_debug,
		     para_num, lcd_panel_hvga);
    }
	else
	{
		//mipi_set_tx_power_mode(1);
		process_mipi_table(mfd,&hx8357c_tx_buf,(struct sequence*)&hx8357c_hvga_standby_exit_table,
		 	ARRAY_SIZE(hx8357c_hvga_standby_exit_table), lcd_panel_hvga);
		//mipi_set_tx_power_mode(0);
		/*delete some lines */
	}

	if((TRUE == para_debug_flag)&&(NULL != hx8357c_lcd_init_table_debug))
	{
		lcd_debug_free_para((void *)hx8357c_lcd_init_table_debug);
	}
	
	pr_info("leave mipi_hx8357c_lcd_on \n");
	return 0;
}
/*lcd suspend function*/
static int mipi_hx8357c_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	process_mipi_table(mfd,&hx8357c_tx_buf,(struct sequence*)&hx8357c_hvga_standby_enter_table,
		 ARRAY_SIZE(hx8357c_hvga_standby_enter_table), lcd_panel_hvga);
	pr_info("leave mipi_hx8357c_lcd_off \n");
	return 0;
}

static int __devinit mipi_hx8357c_lcd_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);

	return 0;
}

static struct sequence hx8357c_cabc_enable_table[] =
{	
	{0x00051,MIPI_DCS_COMMAND,0}, 		
	{0x000ff,TYPE_PARAMETER,0},
	
	{0x00029,MIPI_TYPE_END,0},
};
void hx8357c_set_cabc_backlight(struct msm_fb_data_type *mfd,uint32 bl_level)
{	
	hx8357c_cabc_enable_table[1].reg = bl_level; // 1 will be changed if modify init code

	process_mipi_table(mfd,&hx8357c_tx_buf,(struct sequence*)&hx8357c_cabc_enable_table,
		 ARRAY_SIZE(hx8357c_cabc_enable_table), lcd_panel_hvga);
}

static struct platform_driver this_driver = {
	.probe  = mipi_hx8357c_lcd_probe,
	.driver = {
		.name   = LCD_DEVICE_NAME,
	},
};
static struct msm_fb_panel_data hx8357c_panel_data = {
	.on		= mipi_hx8357c_lcd_on,
	.off	= mipi_hx8357c_lcd_off,
	.set_backlight = pwm_set_backlight,
	/*add cabc control backlight*/
	.set_cabc_brightness = hx8357c_set_cabc_backlight,
};
static struct platform_device this_device = {
	.name   = LCD_DEVICE_NAME,
	.id	= 0,
	.dev	= {
		.platform_data = &hx8357c_panel_data,
	}
};

static int __init mipi_cmd_hx8357c_hvga_init(void)
{
	int ret = 0;
	struct msm_panel_info *pinfo = NULL;

	lcd_panel_hvga = get_lcd_panel_type();
	if ((MIPI_CMD_HX8357C_CHIMEI_HVGA != lcd_panel_hvga )
		&& (MIPI_CMD_HX8357C_CHIMEI_IPS_HVGA != lcd_panel_hvga )
		&& (MIPI_CMD_HX8357C_TIANMA_HVGA != lcd_panel_hvga )
		&& (MIPI_CMD_HX8357C_TIANMA_IPS_HVGA != lcd_panel_hvga))
	{
		return 0;
	}
	pr_info("enter mipi_cmd_hx8357c_hvga_init \n");
	mipi_dsi_buf_alloc(&hx8357c_tx_buf, DSI_BUF_SIZE);

	ret = platform_driver_register(&this_driver);
	if (!ret)
	{
	 	pinfo = &hx8357c_panel_data.panel_info;
		pinfo->xres = 320;
		pinfo->yres = 480;
		pinfo->type = MIPI_CMD_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 24;
		pinfo->bl_max = 255;
		pinfo->bl_min = 30;
		pinfo->fb_num = 2;
        pinfo->clk_rate = 250000000;/* 60fps */
		pinfo->lcd.refx100 = 6000; /* adjust refx100 to prevent tearing */

		pinfo->mipi.mode = DSI_CMD_MODE;
		pinfo->mipi.dst_format = DSI_CMD_DST_FORMAT_RGB888;
		pinfo->mipi.vc = 0;
		pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RGB;
		pinfo->mipi.data_lane0 = TRUE;
		pinfo->mipi.t_clk_post = 0x7f;
		pinfo->mipi.t_clk_pre = 0x2f;
		pinfo->mipi.stream = 0; /* dma_p */
		pinfo->mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
		pinfo->mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
		/*set hw te sync*/
		pinfo->lcd.hw_vsync_mode = TRUE;
		pinfo->lcd.vsync_enable = TRUE;
		pinfo->mipi.te_sel = 1; /* TE from vsync gpio */
		pinfo->mipi.interleave_max = 1;
		pinfo->mipi.insert_dcs_cmd = TRUE;
		pinfo->mipi.wr_mem_continue = 0x3c;
		pinfo->mipi.wr_mem_start = 0x2c;
		pinfo->mipi.dsi_phy_db = &dsi_cmd_mode_phy_db;
		pinfo->mipi.tx_eot_append = 0x01;
		pinfo->mipi.rx_eot_ignore = 0;
		pinfo->mipi.dlane_swap = 0x1;

		ret = platform_device_register(&this_device);
		if (ret)
			pr_err("%s: failed to register device!\n", __func__);
	}


	return ret;
}

module_init(mipi_cmd_hx8357c_hvga_init);
