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

#define LCD_DEVICE_NAME "mipi_cmd_hx8369a_fwvga"

static lcd_panel_type lcd_panel_wvga = LCD_NONE;

/* increase the DSI bit clock to 490 MHz */
/*mipi dsi register setting , help qualcomm to set.*/
static struct mipi_dsi_phy_ctrl dsi_cmd_mode_phy_db_hx8369a_fwvga = 
{
	/* DSI Bit Clock at 490 MHz, 2 lane, RGB888 */ 
	/* regulator */ 
	{0x03, 0x01, 0x01, 0x00, 0x00}, 
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

static struct dsi_buf hx8369a_fwvga_tx_buf;
#if LCD_HX8369A_TIANMA_ESD_SIGN
static struct dsi_buf hx8369a_fwvga_rx_buf;
static struct hrtimer lcd_esd_timer;
static struct msm_fb_data_type *g_mfd;
static struct work_struct work;
static struct workqueue_struct *lcd_esd_wq;
#endif
static struct sequence *hx8369a_fwvga_lcd_init_table_debug = NULL;

static struct sequence hx8369a_fwvga_write_cabc_brightness_table[]= 
{
	{0x00051,MIPI_DCS_COMMAND,0},
	{0x000FF,TYPE_PARAMETER,0},
	{0x00029,MIPI_TYPE_END,0},
};

static const struct sequence hx8369a_fwvga_standby_enter_table[]= 
{
	{0x00028,MIPI_DCS_COMMAND,0}, //28h
	{0x00010,MIPI_DCS_COMMAND,20},
	{0x00029,MIPI_TYPE_END,120}, // add new command for 
};

static const struct sequence hx8369a_fwvga_standby_exit_table[]= 
{
	{0x00011,MIPI_DCS_COMMAND,0}, //29h
	{0x00029,MIPI_DCS_COMMAND,120},
#if LCD_HX8369A_TIANMA_ESD_SIGN	
	{0xC9,MIPI_GEN_COMMAND,20},
	{0x3E,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x01,TYPE_PARAMETER,0},
	{0x0F,TYPE_PARAMETER,0},
	{0x02,TYPE_PARAMETER,0},
	{0x1E,TYPE_PARAMETER,0},
	/* optimise the CABC function */
	{0xCA,MIPI_GEN_COMMAND,0},
	{0x24,TYPE_PARAMETER,0},
	{0x24,TYPE_PARAMETER,0},
	{0x24,TYPE_PARAMETER,0},
	{0x23,TYPE_PARAMETER,0},
	{0x23,TYPE_PARAMETER,0},
	{0x23,TYPE_PARAMETER,0},
	{0x22,TYPE_PARAMETER,0},
	{0x22,TYPE_PARAMETER,0},
	{0x22,TYPE_PARAMETER,0},
	{0x00029,MIPI_TYPE_END,0}, 
#else
	{0x00029,MIPI_TYPE_END,20}, // add new command for 
#endif
};

#if LCD_HX8369A_TIANMA_ESD_SIGN	
static struct sequence hx8369a_fwvga_lcd_init_table[] =
{
	{0xB9,MIPI_GEN_COMMAND,0},
	{0xFF,TYPE_PARAMETER,0},
	{0x83,TYPE_PARAMETER,0},
	{0x69,TYPE_PARAMETER,0},

	{0xB1,MIPI_GEN_COMMAND,5},
	{0x01,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x44,TYPE_PARAMETER,0},
	{0x0A,TYPE_PARAMETER,0}, 
	{0x00,TYPE_PARAMETER,0},
	{0x0F,TYPE_PARAMETER,0},
	{0x0F,TYPE_PARAMETER,0},
	{0x2C,TYPE_PARAMETER,0},
	{0x2C,TYPE_PARAMETER,0},
	{0x3F,TYPE_PARAMETER,0},
	{0x3F,TYPE_PARAMETER,0},
	{0x01,TYPE_PARAMETER,0},
	{0x3A,TYPE_PARAMETER,0},
	{0x01,TYPE_PARAMETER,0},
	{0xE6,TYPE_PARAMETER,0},
	{0xE6,TYPE_PARAMETER,0},
	{0xE6,TYPE_PARAMETER,0},
	{0xE6,TYPE_PARAMETER,0},
	{0xE6,TYPE_PARAMETER,0},

	{0xB2,MIPI_GEN_COMMAND,0},//B2 set display related register
	{0x00,TYPE_PARAMETER,0},
	{0x10,TYPE_PARAMETER,0},
	{0x05,TYPE_PARAMETER,0},
	{0x0A,TYPE_PARAMETER,0}, 
	{0x70,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0xff,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x03,TYPE_PARAMETER,0},
	{0x03,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x01,TYPE_PARAMETER,0},

	{0xB4,MIPI_GEN_COMMAND,5},//B4 set display waveform cycle
	{0x02,TYPE_PARAMETER,0}, //inversion type
	{0x1D,TYPE_PARAMETER,0},
	{0x80,TYPE_PARAMETER,0},
	{0x06,TYPE_PARAMETER,0},
	{0x02,TYPE_PARAMETER,0},

	{0x36,MIPI_DCS_COMMAND,5},//set address mode
	{0xD0,TYPE_PARAMETER,0},
	
	{0xCC,MIPI_DCS_COMMAND,5},//set address mode
	{0x02,TYPE_PARAMETER,0},

	{0xB6,MIPI_DCS_COMMAND,5},//set address mode
	{0x55,TYPE_PARAMETER,0},
	{0x55,TYPE_PARAMETER,0},
	{0xD5,MIPI_GEN_COMMAND,5}, // SET GIP	
	{0x00,TYPE_PARAMETER,0},
	{0x01,TYPE_PARAMETER,0},
	{0x03,TYPE_PARAMETER,0},
	{0x28,TYPE_PARAMETER,0},
	{0x01,TYPE_PARAMETER,0},
	{0x04,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x70,TYPE_PARAMETER,0},
	{0x11,TYPE_PARAMETER,0},
	{0x13,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x60,TYPE_PARAMETER,0},
	{0x04,TYPE_PARAMETER,0},
	{0x71,TYPE_PARAMETER,0},
	{0x05,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x71,TYPE_PARAMETER,0},
	{0x05,TYPE_PARAMETER,0},
	{0x60,TYPE_PARAMETER,0},
	{0x04,TYPE_PARAMETER,0},
	{0x07,TYPE_PARAMETER,0},
	{0x0F,TYPE_PARAMETER,0},
	{0x04,TYPE_PARAMETER,0},
	{0x04,TYPE_PARAMETER,0},//00

	{0xE0,MIPI_GEN_COMMAND,10},
	{0x00,TYPE_PARAMETER,0},
	{0x20,TYPE_PARAMETER,0},
	{0x26,TYPE_PARAMETER,0},
	{0x34,TYPE_PARAMETER,0},
	{0x38,TYPE_PARAMETER,0}, 
	{0x3F,TYPE_PARAMETER,0},
	{0x33,TYPE_PARAMETER,0},
	{0x4B,TYPE_PARAMETER,0},
	{0x09,TYPE_PARAMETER,0}, 
	{0x13,TYPE_PARAMETER,0}, 
	{0x0e,TYPE_PARAMETER,0}, 
	{0x15,TYPE_PARAMETER,0}, 
	{0x16,TYPE_PARAMETER,0}, 
	{0x14,TYPE_PARAMETER,0}, 
	{0x15,TYPE_PARAMETER,0},
	{0x11,TYPE_PARAMETER,0},
	{0x17,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x20,TYPE_PARAMETER,0},
	{0x26,TYPE_PARAMETER,0}, 
	{0x34,TYPE_PARAMETER,0}, 
	{0x38,TYPE_PARAMETER,0}, 
	{0x3F,TYPE_PARAMETER,0},
	{0x33,TYPE_PARAMETER,0},
	{0x4B,TYPE_PARAMETER,0},
	{0x09,TYPE_PARAMETER,0},
	{0x13,TYPE_PARAMETER,0},
	{0x0e,TYPE_PARAMETER,0},
	{0x15,TYPE_PARAMETER,0},
	{0x16,TYPE_PARAMETER,0},
	{0x14,TYPE_PARAMETER,0},
	{0x15,TYPE_PARAMETER,0}, 
	{0x11,TYPE_PARAMETER,0}, 
	{0x17,TYPE_PARAMETER,0},
		
	/* for mipi */
	{0xBA,MIPI_GEN_COMMAND,10},
	{0x00,TYPE_PARAMETER,0},
	{0xa0,TYPE_PARAMETER,0},
	{0xc6,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x0a,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x10,TYPE_PARAMETER,0},
	{0x30,TYPE_PARAMETER,0},
	{0x6f,TYPE_PARAMETER,0},
	{0x02,TYPE_PARAMETER,0},
	{0x11,TYPE_PARAMETER,0},
	{0x18,TYPE_PARAMETER,0},
	{0x40,TYPE_PARAMETER,0},
	
	{0x51,MIPI_DCS_COMMAND,0},
	{0x7f,TYPE_PARAMETER,0},
	
	{0x53,MIPI_DCS_COMMAND,0}, //29h
	{0x24,TYPE_PARAMETER,0},
	
	{0x55,MIPI_DCS_COMMAND,0},
	{0x01,TYPE_PARAMETER,0},

	{0x5E,MIPI_DCS_COMMAND,0},  //change cabc minimun brightness
	{0xD0,TYPE_PARAMETER,0},
			
	{0xC9,MIPI_GEN_COMMAND,0},
	{0x3E,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x00,TYPE_PARAMETER,0},
	{0x01,TYPE_PARAMETER,0},
	{0x0F,TYPE_PARAMETER,0},
	{0x02,TYPE_PARAMETER,0},
	{0x1E,TYPE_PARAMETER,0},
	/* optimise the CABC function */
	{0xCA,MIPI_GEN_COMMAND,0},
	{0x24,TYPE_PARAMETER,0},
	{0x24,TYPE_PARAMETER,0},
	{0x24,TYPE_PARAMETER,0},
	{0x23,TYPE_PARAMETER,0},
	{0x23,TYPE_PARAMETER,0},
	{0x23,TYPE_PARAMETER,0},
	{0x22,TYPE_PARAMETER,0},
	{0x22,TYPE_PARAMETER,0},
	{0x22,TYPE_PARAMETER,0},
	
	/* SET DISPLAY ON */
	{0x11,MIPI_DCS_COMMAND,0},
	{0x29,MIPI_DCS_COMMAND,120},
	{0x2C,MIPI_DCS_COMMAND,20},// 2C memory start  
	{0x29,MIPI_TYPE_END,0}, //the end flag,it don't sent to driver IC
};
static const struct read_sequence hx8369a_esd_read_table_0B[] = 
{
	{0x0B,MIPI_DCS_COMMAND,2},
};
static const struct read_sequence hx8369a_esd_read_table_0C[] = 
{
	{0x0C,MIPI_DCS_COMMAND,2},
};
static const struct read_sequence hx8369a_esd_read_table_0D[] = 
{
	{0x0D,MIPI_DCS_COMMAND,2},
};

int hx8369a_monitor_reg_status(struct msm_fb_data_type *mfd) 
{
	
    struct dsi_buf *rp, *tp;
	uint8 ret = 0; 
    uint8 err = 0;//ok

	
    tp = &hx8369a_fwvga_tx_buf; 
    rp = &hx8369a_fwvga_rx_buf; 
	
    mipi_dsi_buf_init(tp); 
    mipi_dsi_buf_init(rp); 
	
	process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&hx8369a_esd_read_table_0B);
		
	ret = *((unsigned char *)rp->data);
		
	if (0xd0 != ret)
	{
		err = 1;
		LCD_DEBUG("0x0b value = %02x\n",ret);
	}
	
	process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&hx8369a_esd_read_table_0C);
		
	ret = *((unsigned char *)rp->data);
		
	if (0x07 != ret)
	{
		err = 2;
		LCD_DEBUG("0x0c value = %02x\n",ret);
	}
	
	process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&hx8369a_esd_read_table_0D);
		
	ret = *((unsigned char *)rp->data);
	
	if (0x00 != ret)
	{
		err = 3;
		LCD_DEBUG("0x0d value = %02x\n",ret);
	}
	return err;
}

void lcd_esd_check(struct msm_fb_data_type *mfd)
{
	int ret = 0;
	
	/* Read status of registers begin */
	
	ret = hx8369a_monitor_reg_status(mfd);
	
    if(ret)
	{ 
       
	   lcd_reset();
	   mipi_set_tx_power_mode(1);
	  
	   process_mipi_table(mfd,&hx8369a_fwvga_tx_buf,(struct sequence*)&hx8369a_fwvga_lcd_init_table,
			ARRAY_SIZE(hx8369a_fwvga_lcd_init_table), lcd_panel_wvga);
	   mipi_set_tx_power_mode(0);
	      
       LCD_DEBUG("LCD reset and initial again.\n"); 
    } 
	/*Read status of registers end */
}
static const struct sequence hx8369a_fwvga_ESD_table[]= 
{
	{0x00038,MIPI_DCS_COMMAND,0}, //29h
	{0x00020,MIPI_DCS_COMMAND,0}, //29h
	{0x000BD,MIPI_DCS_COMMAND,0},
	{0x00006,TYPE_PARAMETER,0}, 
	{0x000C0,TYPE_PARAMETER,0},
	{0x00004,TYPE_PARAMETER,0},
	{0x000DB,TYPE_PARAMETER,0},
	{0x00029,MIPI_TYPE_END,0}, // add new command for 
};

static enum hrtimer_restart lcd_esd_timer_func(struct hrtimer *timer)
{	
	queue_work(lcd_esd_wq, &work);

	return HRTIMER_NORESTART;
}

static void lcd_esd_func(struct work_struct *work)
{
	if (!g_mfd)
		return ;
	if (g_mfd->key != MFD_KEY)
		return ;
	down(&g_mfd->dma->mutex);
	down(&g_mfd->sem);
	
	process_mipi_table(g_mfd,&hx8369a_fwvga_tx_buf,(struct sequence*)&hx8369a_fwvga_ESD_table,
		 ARRAY_SIZE(hx8369a_fwvga_ESD_table), lcd_panel_wvga);
	
	lcd_esd_check(g_mfd);
	
	up(&g_mfd->sem);
	up(&g_mfd->dma->mutex);	

	hrtimer_start(&lcd_esd_timer, ktime_set(3,0), HRTIMER_MODE_REL);
	//LCD_DEBUG("leave Read_registers_timer_func \n");	
}
#endif
/*lcd resume function*/
static int mipi_hx8369a_fwvga_lcd_on(struct platform_device *pdev)
{
	boolean para_debug_flag = FALSE;
	uint32 para_num = 0;
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL; 

	para_debug_flag = lcd_debug_malloc_get_para( "hx8369a_fwvga_lcd_init_table_debug", 
		(void**)&hx8369a_fwvga_lcd_init_table_debug,&para_num);

	if( (TRUE == para_debug_flag) && (NULL != hx8369a_fwvga_lcd_init_table_debug))
	{
		process_mipi_table(mfd,&hx8369a_fwvga_tx_buf,hx8369a_fwvga_lcd_init_table_debug,
			para_num, lcd_panel_wvga);
	}
	else
	{
		mipi_set_tx_power_mode(1);
		process_mipi_table(mfd,&hx8369a_fwvga_tx_buf,(struct sequence*)&hx8369a_fwvga_standby_exit_table,
		 	ARRAY_SIZE(hx8369a_fwvga_standby_exit_table), lcd_panel_wvga);
		mipi_set_tx_power_mode(0);
	}

	if((TRUE == para_debug_flag)&&(NULL != hx8369a_fwvga_lcd_init_table_debug))
	{
		lcd_debug_free_para((void *)hx8369a_fwvga_lcd_init_table_debug);
	}
	
	LCD_DEBUG("leave mipi_hx8369a_fwvga_lcd_on \n");

#if LCD_HX8369A_TIANMA_ESD_SIGN
	g_mfd = mfd;
	hrtimer_start(&lcd_esd_timer, ktime_set(3, 0), HRTIMER_MODE_REL);
#endif

	return 0;
}

/*lcd suspend function*/
static int mipi_hx8369a_fwvga_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
		
#if LCD_HX8369A_TIANMA_ESD_SIGN
	hrtimer_cancel(&lcd_esd_timer);
#endif

	process_mipi_table(mfd,&hx8369a_fwvga_tx_buf,(struct sequence*)&hx8369a_fwvga_standby_enter_table,
		 ARRAY_SIZE(hx8369a_fwvga_standby_enter_table), lcd_panel_wvga);
	LCD_DEBUG("leave mipi_hx8369a_fwvga_lcd_off \n");
	return 0;
}
/* Add auto cabc function */
#ifdef CONFIG_FB_AUTO_CABC
static struct sequence hx8369a_fwvga_auto_cabc_set_table[] =
{
	{0x00055,MIPI_DCS_COMMAND,0 }, 
	{0x00001,TYPE_PARAMETER,0},
	{0xFFFFF,MIPI_TYPE_END,0}, //the end flag,it don't sent to driver IC
};

/***************************************************************
Function: hx8369a_fwvga_config_auto_cabc
Description: Set CABC configuration
Parameters:
	struct msmfb_cabc_config cabc_cfg: CABC configuration struct
Return:
	0: success
***************************************************************/
static int hx8369a_fwvga_config_auto_cabc(struct msmfb_cabc_config cabc_cfg,struct msm_fb_data_type *mfd)
{
	int ret = 0;

	switch(cabc_cfg.mode)
	{
		case CABC_MODE_UI:
			hx8369a_fwvga_auto_cabc_set_table[1].reg=0x00001;
			break;
		case CABC_MODE_MOVING:
		case CABC_MODE_STILL:
			hx8369a_fwvga_auto_cabc_set_table[1].reg=0x00003;
			break;
		default:
			LCD_DEBUG("%s: invalid cabc mode: %d\n", __func__, cabc_cfg.mode);
	        ret = -EINVAL;
			break;
	}
	if(likely(0 == ret))
	{
		process_mipi_table(mfd,&hx8369a_fwvga_tx_buf,(struct sequence*)&hx8369a_fwvga_auto_cabc_set_table,
			 ARRAY_SIZE(hx8369a_fwvga_auto_cabc_set_table), lcd_panel_wvga);
	}

	LCD_DEBUG("%s: change cabc mode to %d\n",__func__,cabc_cfg.mode);
	return ret;
}
#endif // CONFIG_FB_AUTO_CABC

static int __devinit mipi_hx8369a_fwvga_lcd_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);

	return 0;
}
/*lcd cabc control function*/
void hx8369a_fwvga_set_cabc_backlight(struct msm_fb_data_type *mfd,uint32 bl_level)
{	
	hx8369a_fwvga_write_cabc_brightness_table[1].reg = bl_level; 

	process_mipi_table(mfd,&hx8369a_fwvga_tx_buf,(struct sequence*)&hx8369a_fwvga_write_cabc_brightness_table,
		ARRAY_SIZE(hx8369a_fwvga_write_cabc_brightness_table), lcd_panel_wvga);
}

static struct platform_driver this_driver = {
	.probe = mipi_hx8369a_fwvga_lcd_probe,
	.driver = {
		.name   = LCD_DEVICE_NAME,
	},
};
static struct msm_fb_panel_data hx8369a_fwvga_panel_data = {
	.on = mipi_hx8369a_fwvga_lcd_on,
	.off = mipi_hx8369a_fwvga_lcd_off,
	.set_backlight = pwm_set_backlight,
	/*add cabc control backlight*/
	.set_cabc_brightness = hx8369a_fwvga_set_cabc_backlight,
/* Add auto cabc function */
#ifdef CONFIG_FB_AUTO_CABC
	.config_cabc = hx8369a_fwvga_config_auto_cabc,
#endif
};
static struct platform_device this_device = {
	.name = LCD_DEVICE_NAME,
	.id	= 0,
	.dev = {
		.platform_data = &hx8369a_fwvga_panel_data,
	}
};

static int __init mipi_cmd_hx8369a_fwvga_init(void)
{
	int ret = 0;
	struct msm_panel_info *pinfo = NULL;

	lcd_panel_wvga = get_lcd_panel_type();
	if ((MIPI_CMD_HX8369A_TIANMA_FWVGA!= lcd_panel_wvga ))
	{
		return 0;
	}
	LCD_DEBUG("enter mipi_cmd_hx8369a_fwvga_init \n");
	mipi_dsi_buf_alloc(&hx8369a_fwvga_tx_buf, DSI_BUF_SIZE);
#if LCD_HX8369A_TIANMA_ESD_SIGN
	mipi_dsi_buf_alloc(&hx8369a_fwvga_rx_buf, DSI_BUF_SIZE);
#endif

	ret = platform_driver_register(&this_driver);
	if (!ret)
	{
	 	pinfo = &hx8369a_fwvga_panel_data.panel_info;
		pinfo->xres = 480;
		pinfo->yres = 854;
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
		pinfo->lcd.hw_vsync_mode = TRUE;
		pinfo->lcd.vsync_enable = TRUE;		
		pinfo->mipi.te_sel = 1; /* TE from vsync gpio */
		pinfo->mipi.interleave_max = 1;
		pinfo->mipi.insert_dcs_cmd = TRUE;
		pinfo->mipi.wr_mem_continue = 0x3c;
		pinfo->mipi.wr_mem_start = 0x2c;
		pinfo->mipi.dsi_phy_db = &dsi_cmd_mode_phy_db_hx8369a_fwvga;
		pinfo->mipi.tx_eot_append = 0x01;
		pinfo->mipi.rx_eot_ignore = 0;
		pinfo->mipi.dlane_swap = 0x1;

		ret = platform_device_register(&this_device);
		if (ret)
			LCD_DEBUG("%s: failed to register device!\n", __func__);
	}
#if LCD_HX8369A_TIANMA_ESD_SIGN
	hrtimer_init(&lcd_esd_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	lcd_esd_timer.function = lcd_esd_timer_func;
	lcd_esd_wq = create_singlethread_workqueue("lcd_esd_wq");
	INIT_WORK(&work, lcd_esd_func);
#endif

	return ret;
}

module_init(mipi_cmd_hx8369a_fwvga_init);
