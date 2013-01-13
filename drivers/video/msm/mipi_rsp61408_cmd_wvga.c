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

#define LCD_DEVICE_NAME "mipi_cmd_rsp61408_wvga"

static lcd_panel_type lcd_panel_wvga = LCD_NONE;

/*mipi dsi register setting , help qualcomm to set.*/
static struct mipi_dsi_phy_ctrl dsi_cmd_mode_phy_db = 
{
    /* DSI Bit Clock at 300 MHz, 2 lane, RGB888 */ 
	/* regulator */ 
	{0x03, 0x01, 0x01, 0x00}, 
	/* timing */ 
	{0x5D, 0x28, 0xB, 0x00, 0x33, 0x38, 0x10, 0x2C, 
	0xE, 0x3, 0x04}, 
	/* phy ctrl */ 
	{0x7f, 0x00, 0x00, 0x00}, 
	/* strength */ 
	{0xbb, 0x02, 0x06, 0x00}, 
	/* pll control */ 
	{0x01, 0x27, 0x31, 0xd2, 0x00, 0x40, 0x37, 0x62, 
	0x01, 0x0f, 0x07, 
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0}, 
};

static struct dsi_buf rsp61408_tx_buf;
static struct sequence * rsp61408_lcd_init_table_debug = NULL;

/*LCD init code*/
static const struct sequence rsp61408_wvga_standby_enter_table[]= 
{
	/*close Vsync singal,when lcd sleep in*/
	{0x00034,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00028,MIPI_DCS_COMMAND,0}, //28h
	{0x00000,TYPE_PARAMETER,0},
	/*delay time is not very correctly right*/
	{0x0010,MIPI_DCS_COMMAND,20},
	{0x0000,TYPE_PARAMETER,0},
	{0x00029,MIPI_TYPE_END,150}, // add new command for 
};
static const struct sequence rsp61408_wvga_standby_exit_table[]= 
{
	/* solve losing control of the backlight */
	{0x00011,MIPI_DCS_COMMAND,0}, //29h
	{0x00000,TYPE_PARAMETER,0},
	/*open Vsync singal,when lcd sleep out*/
	{0x00035,MIPI_DCS_COMMAND,150},
	{0x00000,TYPE_PARAMETER,0},
	{0x00029,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00029,MIPI_TYPE_END,0}, // add new command for 
};

#ifdef CONFIG_FB_DYNAMIC_GAMMA
static const struct sequence rsp61408_gamma_25[]= 
{
	{0x000C8,MIPI_GEN_COMMAND,0}, //C8h
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x0001D,TYPE_PARAMETER,0}, 
    {0x00029,TYPE_PARAMETER,0}, 
    {0x00039,TYPE_PARAMETER,0}, 
    {0x00054,TYPE_PARAMETER,0}, 
    {0x00030,TYPE_PARAMETER,0}, 
    {0x00020,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x00013,TYPE_PARAMETER,0}, 
    {0x00008,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x0001D,TYPE_PARAMETER,0}, 
    {0x00029,TYPE_PARAMETER,0}, 
    {0x00039,TYPE_PARAMETER,0}, 
    {0x00054,TYPE_PARAMETER,0}, 
    {0x00030,TYPE_PARAMETER,0}, 
    {0x00020,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x00013,TYPE_PARAMETER,0}, 
    {0x00008,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0},
    
    {0x000C9,MIPI_GEN_COMMAND,0}, //C9h
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0},
    {0x0001D,TYPE_PARAMETER,0},  
    {0x00029,TYPE_PARAMETER,0}, 
    {0x00039,TYPE_PARAMETER,0}, 
    {0x00054,TYPE_PARAMETER,0}, 
    {0x00030,TYPE_PARAMETER,0}, 
    {0x00020,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x00013,TYPE_PARAMETER,0}, 
    {0x00008,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x0001D,TYPE_PARAMETER,0}, 
    {0x00029,TYPE_PARAMETER,0}, 
    {0x00039,TYPE_PARAMETER,0}, 
    {0x00054,TYPE_PARAMETER,0}, 
    {0x00030,TYPE_PARAMETER,0}, 
    {0x00020,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x00013,TYPE_PARAMETER,0}, 
    {0x00008,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0},
    
    {0x000CA,MIPI_GEN_COMMAND,0}, //CAh
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x0001D,TYPE_PARAMETER,0}, 
    {0x00029,TYPE_PARAMETER,0}, 
    {0x00039,TYPE_PARAMETER,0}, 
    {0x00054,TYPE_PARAMETER,0}, 
    {0x00030,TYPE_PARAMETER,0}, 
    {0x00020,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x00013,TYPE_PARAMETER,0}, 
    {0x00008,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x0001D,TYPE_PARAMETER,0}, 
    {0x00029,TYPE_PARAMETER,0}, 
    {0x00039,TYPE_PARAMETER,0}, 
    {0x00054,TYPE_PARAMETER,0}, 
    {0x00030,TYPE_PARAMETER,0}, 
    {0x00020,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x00013,TYPE_PARAMETER,0}, 
    {0x00008,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0xFFFFF,MIPI_TYPE_END,0},

};


static const struct sequence rsp61408_gamma_22[]= 
{
	{0x000C8,MIPI_GEN_COMMAND,0}, //C8h
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x0001D,TYPE_PARAMETER,0}, 
    {0x00029,TYPE_PARAMETER,0}, 
    {0x00036,TYPE_PARAMETER,0}, 
    {0x00050,TYPE_PARAMETER,0}, 
    {0x00033,TYPE_PARAMETER,0}, 
    {0x00021,TYPE_PARAMETER,0}, 
    {0x00016,TYPE_PARAMETER,0}, 
    {0x00011,TYPE_PARAMETER,0}, 
    {0x00008,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x0001D,TYPE_PARAMETER,0}, 
    {0x00029,TYPE_PARAMETER,0}, 
    {0x00036,TYPE_PARAMETER,0}, 
    {0x00050,TYPE_PARAMETER,0}, 
    {0x00033,TYPE_PARAMETER,0}, 
    {0x00021,TYPE_PARAMETER,0}, 
    {0x00016,TYPE_PARAMETER,0}, 
    {0x00011,TYPE_PARAMETER,0}, 
    {0x00008,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0},
    
    {0x000C9,MIPI_GEN_COMMAND,0}, //C9h
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0},
    {0x0001D,TYPE_PARAMETER,0},  
    {0x00029,TYPE_PARAMETER,0}, 
    {0x00036,TYPE_PARAMETER,0}, 
    {0x00050,TYPE_PARAMETER,0}, 
    {0x00033,TYPE_PARAMETER,0}, 
    {0x00021,TYPE_PARAMETER,0}, 
    {0x00016,TYPE_PARAMETER,0}, 
    {0x00011,TYPE_PARAMETER,0}, 
    {0x00008,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x0001D,TYPE_PARAMETER,0}, 
    {0x00029,TYPE_PARAMETER,0}, 
    {0x00039,TYPE_PARAMETER,0}, 
    {0x00054,TYPE_PARAMETER,0}, 
    {0x00030,TYPE_PARAMETER,0}, 
    {0x00020,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x00013,TYPE_PARAMETER,0}, 
    {0x00008,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0},
    
    {0x000CA,MIPI_GEN_COMMAND,0}, //CAh
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x0001D,TYPE_PARAMETER,0}, 
    {0x00029,TYPE_PARAMETER,0}, 
    {0x00036,TYPE_PARAMETER,0}, 
    {0x00050,TYPE_PARAMETER,0}, 
    {0x00033,TYPE_PARAMETER,0}, 
    {0x00021,TYPE_PARAMETER,0}, 
    {0x00016,TYPE_PARAMETER,0}, 
    {0x00011,TYPE_PARAMETER,0}, 
    {0x00008,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0x00017,TYPE_PARAMETER,0}, 
    {0x0001D,TYPE_PARAMETER,0}, 
    {0x00029,TYPE_PARAMETER,0}, 
    {0x00036,TYPE_PARAMETER,0}, 
    {0x00050,TYPE_PARAMETER,0}, 
    {0x00033,TYPE_PARAMETER,0}, 
    {0x00021,TYPE_PARAMETER,0}, 
    {0x00016,TYPE_PARAMETER,0}, 
    {0x00011,TYPE_PARAMETER,0}, 
    {0x00008,TYPE_PARAMETER,0}, 
    {0x00002,TYPE_PARAMETER,0}, 
    {0xFFFFF,MIPI_TYPE_END,0},

};

int mipi_rsp61408_set_dynamic_gamma(enum danymic_gamma_mode  gamma_mode,struct msm_fb_data_type *mfd)
{
    int ret = 0;

    switch(gamma_mode)
    {
        case GAMMA25:
            process_mipi_table(mfd,&rsp61408_tx_buf,(struct sequence*)&rsp61408_gamma_25,
                        ARRAY_SIZE(rsp61408_gamma_25), lcd_panel_wvga);
            break ;
        case GAMMA22:
            process_mipi_table(mfd,&rsp61408_tx_buf,(struct sequence*)&rsp61408_gamma_22,
                        ARRAY_SIZE(rsp61408_gamma_22), lcd_panel_wvga);
            break;
        default:
			LCD_DEBUG("%s: invalid dynamic_gamma: %d\n", __func__,gamma_mode);
	        ret = -EINVAL;
            break;
    }
    LCD_DEBUG("%s: change gamma mode to %d\n",__func__,gamma_mode);
    return ret;
}

#endif

/*lcd resume function*/
static int mipi_rsp61408_lcd_on(struct platform_device *pdev)
{
	boolean para_debug_flag = FALSE;
	uint32 para_num = 0;
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL; 

	para_debug_flag = lcd_debug_malloc_get_para( "rsp61408_lcd_init_table_debug", 
            (void**)&rsp61408_lcd_init_table_debug,&para_num);

    if( (TRUE == para_debug_flag) && (NULL != rsp61408_lcd_init_table_debug))
    {
        process_mipi_table(mfd,&rsp61408_tx_buf,rsp61408_lcd_init_table_debug,
		     para_num, lcd_panel_wvga);
    }
	else
	{
		mipi_set_tx_power_mode(1);
		process_mipi_table(mfd,&rsp61408_tx_buf,(struct sequence*)&rsp61408_wvga_standby_exit_table,
		 	ARRAY_SIZE(rsp61408_wvga_standby_exit_table), lcd_panel_wvga);
		mipi_set_tx_power_mode(0);
	}

	if((TRUE == para_debug_flag)&&(NULL != rsp61408_lcd_init_table_debug))
	{
		lcd_debug_free_para((void *)rsp61408_lcd_init_table_debug);
	}
	
	pr_info("leave mipi_rsp61408_lcd_on \n");
	return 0;
}

/*lcd suspend function*/
static int mipi_rsp61408_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	process_mipi_table(mfd,&rsp61408_tx_buf,(struct sequence*)&rsp61408_wvga_standby_enter_table,
		 ARRAY_SIZE(rsp61408_wvga_standby_enter_table), lcd_panel_wvga);
	pr_info("leave mipi_rsp61408_lcd_off \n");
	return 0;
}
#ifdef CONFIG_FB_AUTO_CABC
static struct sequence rsp61408_auto_cabc_set_table[] =
{
	{0x000B8,MIPI_GEN_COMMAND,0 }, 
	{0x00000,TYPE_PARAMETER,0},
	
	{0x000B8,MIPI_GEN_COMMAND,0 }, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0000d,TYPE_PARAMETER,0},
	{0x000FF,TYPE_PARAMETER,0},
	{0x000FF,TYPE_PARAMETER,0},
	/* change the low brightness */
	{0x000F9,TYPE_PARAMETER,0},
	{0x000d0,TYPE_PARAMETER,0},
	{0x0000f,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00008,TYPE_PARAMETER,0},
	{0x00005,TYPE_PARAMETER,0},
	{0x00037,TYPE_PARAMETER,0},
	{0x0005A,TYPE_PARAMETER,0},
	{0x00087,TYPE_PARAMETER,0},
	{0x000B9,TYPE_PARAMETER,0},
	{0x000FF,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	
	{0xFFFFF,MIPI_TYPE_END,0}, //the end flag,it don't sent to driver IC
};


/***************************************************************
Function: rsp61408_config_cabc
Description: Set CABC configuration
Parameters:
    struct msmfb_cabc_config cabc_cfg: CABC configuration struct
Return:
    0: success
***************************************************************/
static int rsp61408_config_auto_cabc(struct msmfb_cabc_config cabc_cfg,struct msm_fb_data_type *mfd)
{
    int ret = 0;

	switch(cabc_cfg.mode)
	{
		case CABC_MODE_UI:
			rsp61408_auto_cabc_set_table[3].reg=0x00001;
			rsp61408_auto_cabc_set_table[5].reg=0x0000d;
			break;
		case CABC_MODE_MOVING:
		case CABC_MODE_STILL:
			rsp61408_auto_cabc_set_table[3].reg=0x00003;
			rsp61408_auto_cabc_set_table[5].reg=0x00019;
			break;
		default:
			LCD_DEBUG("%s: invalid cabc mode: %d\n", __func__, cabc_cfg.mode);
	        ret = -EINVAL;
			break;
	}
	if(likely(0 == ret))
	{
		process_mipi_table(mfd,&rsp61408_tx_buf,(struct sequence*)&rsp61408_auto_cabc_set_table,
			 ARRAY_SIZE(rsp61408_auto_cabc_set_table), lcd_panel_wvga);
	}

    LCD_DEBUG("%s: change cabc mode to %d\n",__func__,cabc_cfg.mode);
    return ret;
}
#endif // CONFIG_FB_AUTO_CABC

static int __devinit mipi_rsp61408_lcd_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);

	return 0;
}
static struct sequence rsp61408_wvga_write_cabc_brightness_table[]= 
{
	/* solve losing control of the backlight */
	{0x000B9,MIPI_GEN_COMMAND,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
    {0x00002,TYPE_PARAMETER,0},
    {0x00018,TYPE_PARAMETER,0},
	{0x00,MIPI_TYPE_END,0},
};
/*lcd cabc control function*/
void rsp61408_set_cabc_backlight(struct msm_fb_data_type *mfd,uint32 bl_level)
{	
	rsp61408_wvga_write_cabc_brightness_table[2].reg = bl_level; 

	process_mipi_table(mfd,&rsp61408_tx_buf,(struct sequence*)&rsp61408_wvga_write_cabc_brightness_table,
		 ARRAY_SIZE(rsp61408_wvga_write_cabc_brightness_table), lcd_panel_wvga);
}

static struct platform_driver this_driver = {
	.probe  = mipi_rsp61408_lcd_probe,
	.driver = {
		.name   = LCD_DEVICE_NAME,
	},
};
static struct msm_fb_panel_data rsp61408_panel_data = {
	.on		= mipi_rsp61408_lcd_on,
	.off	= mipi_rsp61408_lcd_off,
	.set_backlight = pwm_set_backlight,
	/*add cabc control backlight*/
	.set_cabc_brightness = rsp61408_set_cabc_backlight,
#ifdef CONFIG_FB_AUTO_CABC
    .config_cabc = rsp61408_config_auto_cabc,
#endif
#ifdef CONFIG_FB_DYNAMIC_GAMMA
    .set_dynamic_gamma = mipi_rsp61408_set_dynamic_gamma,
#endif
};
static struct platform_device this_device = {
	.name   = LCD_DEVICE_NAME,
	.id	= 0,
	.dev	= {
		.platform_data = &rsp61408_panel_data,
	}
};

static int __init mipi_cmd_rsp61408_wvga_init(void)
{
	int ret = 0;
	struct msm_panel_info *pinfo = NULL;

	lcd_panel_wvga = get_lcd_panel_type();
	if ((MIPI_CMD_RSP61408_CHIMEI_WVGA!= lcd_panel_wvga )&&(MIPI_CMD_RSP61408_BYD_WVGA!= lcd_panel_wvga )
		&&(MIPI_CMD_RSP61408_TRULY_WVGA!= lcd_panel_wvga))
	{
		return 0;
	}
	pr_info("enter mipi_cmd_rsp61408_wvga_init \n");
	mipi_dsi_buf_alloc(&rsp61408_tx_buf, DSI_BUF_SIZE);

	ret = platform_driver_register(&this_driver);
	if (!ret)
	{
	 	pinfo = &rsp61408_panel_data.panel_info;
		pinfo->xres = 480;
		pinfo->yres = 800;
		pinfo->type = MIPI_CMD_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 24;		
		pinfo->bl_max = 255;
		pinfo->bl_min = 30;		
		pinfo->fb_num = 2;
        pinfo->clk_rate = 300000000;
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

module_init(mipi_cmd_rsp61408_wvga_init);
