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

#define LCD_DEVICE_NAME "mipi_cmd_nt35560_fwvga"

static lcd_panel_type lcd_panel_hvga = LCD_NONE;

/*mipi dsi register setting , help qualcomm to set.*/
static struct mipi_dsi_phy_ctrl dsi_cmd_mode_phy_db = 
{
    /* DSI Bit Clock at 491.90 MHz, 2 lane, RGB888 */ 
    /* regulator */ 
    {0x03, 0x01, 0x01, 0x00}, 
    /* timing */ 
    {0x66, 0x26, 0x13, 0x00, 0x14, 0x89, 0x1e, 0x8a, 
    0x14, 0x03, 0x04}, 
    /* phy ctrl */ 
    {0x7f, 0x00, 0x00, 0x00}, 
    /* strength */ 
    {0xbb, 0x02, 0x06, 0x00}, 
    /* pll control */ 
    {0x01, 0x26, 0x31, 0xda, 0x00, 0x40, 0x37, 0x62, 
    0x01, 0x0f, 0x07, 
    0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0}, 
};

static struct dsi_buf nt35560_tx_buf;

/*LCD init code*/
static char displayoff[2] = {0x28,0x00};
static char sleepin[2] = {0x10, 0x00};
static char displayon[2] = {0x29,0x00};
static char sleepout[2] = {0x11,0x00};

static char cabcdisplaybright[2] = {0x51,0xff};
static char cabcctrldisplay1[2] = {0x53,0x24};
static char cabcctrldisplay2[2] = {0x55,0x01};
static char cabcminbright[2] = {0x5e,0x30};

static char tearon[2] = {0x35,0x00};
static char tearline[3] = {0x44,0x01,0x90};
static char addressmode[2] = {0x36,0x00};
static char writestartcmd[2] = {0x2c,0x00};


static struct dsi_cmd_desc nt35560_display_off_cmds[] = 
{
    {DTYPE_DCS_WRITE, 1, 0, 0, 0,
		sizeof(displayoff), displayoff },
};
static struct dsi_cmd_desc nt35560_sleep_in_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(sleepin), sleepin},	
};

static struct dsi_cmd_desc nt35560_sleep_out_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(sleepout), sleepout },
};
static struct dsi_cmd_desc nt35560_display_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,
		sizeof(displayon), displayon},	
};

//static struct dsi_cmd_desc nt35560_lcd_gamma_cmds[] = {
//};
static struct dsi_cmd_desc nt35560_lcd_init_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(tearon), tearon},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,
		sizeof(tearline), tearline},	
	{DTYPE_DCS_WRITE1, 1, 0, 0, 10,
		sizeof(addressmode), addressmode},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,
		sizeof(writestartcmd), writestartcmd},
};
static struct dsi_cmd_desc nt35560_lcd_cabc_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(cabcdisplaybright), cabcdisplaybright},	
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(cabcctrldisplay1), cabcctrldisplay1},	
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(cabcctrldisplay2), cabcctrldisplay2},			
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(cabcminbright), cabcminbright},			
};

/*lcd resume function*/
static int mipi_nt35560_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL; 

	mipi_set_tx_power_mode(1);
    /* < lishubin update baseline change this begin */
	mipi_dsi_cmds_tx( &nt35560_tx_buf, nt35560_sleep_out_cmds,
			ARRAY_SIZE(nt35560_sleep_out_cmds));
	
	mipi_dsi_cmds_tx( &nt35560_tx_buf, nt35560_lcd_init_cmds,
			ARRAY_SIZE(nt35560_lcd_init_cmds));
	mipi_dsi_cmds_tx( &nt35560_tx_buf, nt35560_lcd_cabc_cmds,
			ARRAY_SIZE(nt35560_lcd_cabc_cmds));
	mipi_dsi_cmds_tx( &nt35560_tx_buf, nt35560_display_on_cmds,
			ARRAY_SIZE(nt35560_display_on_cmds));
    /* lishubin update baseline change this end > */
	mipi_set_tx_power_mode(0);

	pr_info("leave mipi_nt35560_lcd_on \n");
	return 0;
}

/*lcd suspend function*/
static int mipi_nt35560_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

   /* < lishubin update baseline change this begin */
	mipi_dsi_cmds_tx( &nt35560_tx_buf, nt35560_display_off_cmds,
			ARRAY_SIZE(nt35560_display_off_cmds));
	mipi_dsi_cmds_tx( &nt35560_tx_buf, nt35560_sleep_in_cmds,
			ARRAY_SIZE(nt35560_sleep_in_cmds));
    /* lishubin update baseline change this end > */
	pr_info("leave mipi_nt35560_lcd_off \n");
	return 0;
}

static int __devinit mipi_nt35560_lcd_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);

	return 0;
}

static void mipi_nt35560_set_backlight(struct msm_fb_data_type *mfd)
{	
}

static struct platform_driver this_driver = {
	.probe  = mipi_nt35560_lcd_probe,
	.driver = {
		.name   = LCD_DEVICE_NAME,
	},
};
static struct msm_fb_panel_data nt35560_panel_data = {
	.on		= mipi_nt35560_lcd_on,
	.off	= mipi_nt35560_lcd_off,
	.set_backlight = mipi_nt35560_set_backlight,
};
static struct platform_device this_device = {
	.name   = LCD_DEVICE_NAME,
	.id	= 0,
	.dev	= {
		.platform_data = &nt35560_panel_data,
	}
};

static int __init mipi_cmd_nt35560_fwvga_init(void)
{
	int ret = 0;
	struct msm_panel_info *pinfo = NULL;

	lcd_panel_hvga = get_lcd_panel_type();
	if(MIPI_CMD_NT35560_TOSHIBA_FWVGA != lcd_panel_hvga) 
	{
		return 0;
	}
	pr_info("enter mipi_cmd_nt35560_fwvga_init \n");
	mipi_dsi_buf_alloc(&nt35560_tx_buf, DSI_BUF_SIZE);

	ret = platform_driver_register(&this_driver);
	if (!ret)
	{
	 	pinfo = &nt35560_panel_data.panel_info;
		pinfo->xres = 480;
		pinfo->yres = 854;
		pinfo->type = MIPI_CMD_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 24;		
		pinfo->bl_max = 255;
		pinfo->bl_min = 30;
		pinfo->fb_num = 2;

		pinfo->clk_rate = 499000000;
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

module_init(mipi_cmd_nt35560_fwvga_init);
