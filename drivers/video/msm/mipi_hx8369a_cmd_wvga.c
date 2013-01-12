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

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "hw_lcd_common.h"

#define LCD_DEVICE_NAME "mipi_cmd_hx8369a_wvga"

static lcd_panel_type lcd_panel_wvga = LCD_NONE;

/* increase the DSI bit clock to 490 MHz */
/*mipi dsi register setting , help qualcomm to set.*/
static struct mipi_dsi_phy_ctrl dsi_cmd_mode_phy_db = 
{
    /* DSI Bit Clock at 490 MHz, 2 lane, RGB888 */ 
	/* regulator */ 
	{0x03, 0x01, 0x01, 0x00}, 
	/* timing */ 
	{0x88, 0x32, 0x14, 0x00, 0x44, 0x4F, 0x18, 0x35, 
	0x17, 0x3, 0x04, 0x00}, 
	/* phy ctrl */ 
	{0x7f, 0x00, 0x00, 0x00}, 
	/* strength */ 
	{0xbb, 0x02, 0x06, 0x00}, 
	/* pll control */ 
	{0x1, 0xE3, 0x31, 0xd2, 0x00, 0x40, 0x37, 0x62, 
	0x01, 0x0f, 0x07, 
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0}, 
};


static struct dsi_buf hx8369a_tx_buf;
static struct sequence *hx8369a_lcd_init_table_debug = NULL;

static struct sequence hx8369a_wvga_write_cabc_brightness_table[]= 
{
	{0x00051,MIPI_DCS_COMMAND,0},
	{0x000FF,TYPE_PARAMETER,0},
	{0x00029,MIPI_TYPE_END,0},
};

static const struct sequence hx8369a_wvga_standby_enter_table[]= 
{
	{0x00028,MIPI_DCS_COMMAND,0}, //28h
	{0x0010,MIPI_DCS_COMMAND,20},
	{0x00029,MIPI_TYPE_END,120}, // add new command for 
};
static const struct sequence hx8369a_wvga_standby_exit_table[]= 
{
	{0x00011,MIPI_DCS_COMMAND,0}, //29h
	{0x00029,MIPI_DCS_COMMAND,120},
	{0x00029,MIPI_TYPE_END,20}, // add new command for 
};


/*lcd resume function*/
static int mipi_hx8369a_lcd_on(struct platform_device *pdev)
{
	boolean para_debug_flag = FALSE;
	uint32 para_num = 0;
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL; 

	para_debug_flag = lcd_debug_malloc_get_para( "hx8369a_lcd_init_table_debug", 
            (void**)&hx8369a_lcd_init_table_debug,&para_num);

    if( (TRUE == para_debug_flag) && (NULL != hx8369a_lcd_init_table_debug))
    {
        process_mipi_table(mfd,&hx8369a_tx_buf,hx8369a_lcd_init_table_debug,
		     para_num, lcd_panel_wvga);
    }
	else
	{
		mipi_set_tx_power_mode(1);
		process_mipi_table(mfd,&hx8369a_tx_buf,(struct sequence*)&hx8369a_wvga_standby_exit_table,
		 	ARRAY_SIZE(hx8369a_wvga_standby_exit_table), lcd_panel_wvga);
		mipi_set_tx_power_mode(0);
	}

	if((TRUE == para_debug_flag)&&(NULL != hx8369a_lcd_init_table_debug))
	{
		lcd_debug_free_para((void *)hx8369a_lcd_init_table_debug);
	}
	
	pr_info("leave mipi_hx8369a_lcd_on \n");
	return 0;
}

/*lcd suspend function*/
static int mipi_hx8369a_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	process_mipi_table(mfd,&hx8369a_tx_buf,(struct sequence*)&hx8369a_wvga_standby_enter_table,
		 ARRAY_SIZE(hx8369a_wvga_standby_enter_table), lcd_panel_wvga);
	pr_info("leave mipi_hx8369a_lcd_off \n");
	return 0;
}
#ifdef CONFIG_FB_AUTO_CABC

static struct sequence hx8369a_auto_cabc_set_table[] =
{
	{0x00055,MIPI_DCS_COMMAND,0}, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00029,MIPI_TYPE_END,0}, 
    {0xFFFFF,MIPI_TYPE_END,0},
};

/***************************************************************
Function: hx8369a_config_auto_cabc
Description: Set CABC configuration
Parameters:
    struct msmfb_cabc_config cabc_cfg: CABC configuration struct
Return:
    0: success
***************************************************************/
static int hx8369a_config_auto_cabc(struct msmfb_cabc_config cabc_cfg,struct msm_fb_data_type *mfd)
{
    int ret = 0;

	switch(cabc_cfg.mode)
	{
		case CABC_MODE_UI:
			hx8369a_auto_cabc_set_table[1].reg = 0x0001;
			break;
		case CABC_MODE_MOVING:
		case CABC_MODE_STILL:
			hx8369a_auto_cabc_set_table[1].reg = 0x0003;
			break;
		default:
			LCD_DEBUG("%s: invalid cabc mode: %d\n", __func__, cabc_cfg.mode);
	        ret = -EINVAL;
			break;
	}
	if(likely(0 == ret))
	{
		process_mipi_table(mfd,&hx8369a_tx_buf,(struct sequence*)&hx8369a_auto_cabc_set_table,
			 ARRAY_SIZE(hx8369a_auto_cabc_set_table), lcd_panel_wvga);
	}

    LCD_DEBUG("%s: change cabc mode to %d\n",__func__,cabc_cfg.mode);
    return ret;
}
#endif // CONFIG_FB_AUTO_CABC

static int __devinit mipi_hx8369a_lcd_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);

	return 0;
}
/*lcd cabc control function*/
void hx8369a_set_cabc_backlight(struct msm_fb_data_type *mfd,uint32 bl_level)
{	
	hx8369a_wvga_write_cabc_brightness_table[1].reg = bl_level; 

	process_mipi_table(mfd,&hx8369a_tx_buf,(struct sequence*)&hx8369a_wvga_write_cabc_brightness_table,
		 ARRAY_SIZE(hx8369a_wvga_write_cabc_brightness_table), lcd_panel_wvga);
}

static struct platform_driver this_driver = {
	.probe  = mipi_hx8369a_lcd_probe,
	.driver = {
		.name   = LCD_DEVICE_NAME,
	},
};
static struct msm_fb_panel_data hx8369a_panel_data = {
	.on		= mipi_hx8369a_lcd_on,
	.off	= mipi_hx8369a_lcd_off,
	.set_backlight = pwm_set_backlight,
	/*add cabc control backlight*/
	.set_cabc_brightness = hx8369a_set_cabc_backlight,
#ifdef CONFIG_FB_AUTO_CABC
    .config_cabc = hx8369a_config_auto_cabc,
#endif
};
static struct platform_device this_device = {
	.name   = LCD_DEVICE_NAME,
	.id	= 0,
	.dev	= {
		.platform_data = &hx8369a_panel_data,
	}
};

static int __init mipi_cmd_hx8369a_wvga_init(void)
{
	int ret = 0;
	struct msm_panel_info *pinfo = NULL;

	lcd_panel_wvga = get_lcd_panel_type();
	if ((MIPI_CMD_HX8369A_TIANMA_WVGA!= lcd_panel_wvga ))
	{
		return 0;
	}
	pr_info("enter mipi_cmd_hx8369a_wvga_init \n");
	mipi_dsi_buf_alloc(&hx8369a_tx_buf, DSI_BUF_SIZE);

	ret = platform_driver_register(&this_driver);
	if (!ret)
	{
	 	pinfo = &hx8369a_panel_data.panel_info;
		pinfo->xres = 480;
		pinfo->yres = 800;
		pinfo->type = MIPI_CMD_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 24;		
		
		pinfo->bl_max = 255;
		pinfo->bl_min = 30;		
		
		pinfo->fb_num = 2;

        /* increase the DSI bit clock to 490 MHz */		
        pinfo->clk_rate = 490000000;
		
		pinfo->lcd.refx100 = 6000; /* adjust refx100 to prevent tearing */

		pinfo->mipi.mode = DSI_CMD_MODE;
		pinfo->mipi.dst_format = DSI_CMD_DST_FORMAT_RGB888;
		pinfo->mipi.vc = 0;
		pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RGB;
		pinfo->mipi.data_lane0 = TRUE;
		pinfo->mipi.data_lane1 = TRUE;
		pinfo->mipi.t_clk_post = 0xB0;
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

module_init(mipi_cmd_hx8369a_wvga_init);
