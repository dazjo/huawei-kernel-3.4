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

#include <linux/gpio.h>

#define LCD_DEVICE_NAME "mipi_video_hx8369b_wvga"

static lcd_panel_type lcd_panel_wvga = LCD_NONE;

/*mipi dsi register setting , help qualcomm to set.*/
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = 
{
	/* DSI Bit Clock at 360 MHz, 2 lane, RGB888 */
	/* regulator */
	{0x03, 0x01, 0x01, 0x00, 0x00}, 
	/* timing */ 
	{0x6B, 0x2C, 0x0E, 0x00, 0x38, 0x43, 0x12, 0x2F, 
	0x11, 0x3, 0x04, 0x00}, 
	/* phy ctrl */ 
	{0x7f, 0x00, 0x00, 0x00}, 
	/* strength */ 
	{0xbb, 0x02, 0x06, 0x00}, 
	/* pll control */ 
	{0x01, 0x63, 0x31, 0xd2, 0x00, 0x40, 0x37, 0x62, 
	0x01, 0x0f, 0x07, 
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0},
};

static struct dsi_buf hx8369b_tx_buf;
static struct sequence *hx8369b_lcd_init_table_debug = NULL;

static struct sequence hx8369b_wvga_write_cabc_brightness_table[]= 
{
	{0x00051,MIPI_DCS_COMMAND,0},
	{0x000FF,TYPE_PARAMETER,0},
	{0x00029,MIPI_TYPE_END,0},
};
#if 0
static const struct sequence hx8369b_wvga_standby_enter_table[]= 
{
	{0x00028,MIPI_DCS_COMMAND,0}, //28h
	{0x00010,MIPI_DCS_COMMAND,20},
	{0x00029,MIPI_TYPE_END,120}, // add new command for 
};

static const struct sequence hx8369b_wvga_standby_exit_table[]= 
{
	{0x00011,MIPI_DCS_COMMAND,0}, //29h
	{0x00029,MIPI_DCS_COMMAND,120},
	{0x00005,MIPI_TYPE_END,20}, // add new command for 
};
#endif

/* modify init sequence
 * 0xD5 reg
 * modify GAMMA
 */
static const struct sequence hx8369b_tianma_lcd_init_table[] =
{
	{0x000B9,MIPI_GEN_COMMAND,0}, //B9 set extension command
	{0x000FF,TYPE_PARAMETER,0},
	{0x00083,TYPE_PARAMETER,0},
	{0x00069,TYPE_PARAMETER,0},

	{0x000D5,MIPI_GEN_COMMAND,0},	//GIP
	{0x00000,TYPE_PARAMETER,0}, //1/	  PANSEL
	{0x00000,TYPE_PARAMETER,0}, //2/	  SHR_0[11:8]
	{0x00012,TYPE_PARAMETER,0}, //3/	  SHR_0[7:0]
	{0x00003,TYPE_PARAMETER,0}, //4/	  SHR_1[11:8]
	{0x00031,TYPE_PARAMETER,0}, //5	  SHR_1[7:0]
	{0x00003,TYPE_PARAMETER,0}, //6	  SHR_2[11:8]
	{0x00035,TYPE_PARAMETER,0}, //7	  SHR_2[7:0]               
	{0x00012,TYPE_PARAMETER,0}, //8	  SHR0_1[3:0], SHR0_2[3:0] 
	{0x00001,TYPE_PARAMETER,0},	//9	  SHR0_3[3:0], SHR1_1[3:0] 
	{0x00020,TYPE_PARAMETER,0}, //10  SHR1_2[3:0], SHR1_3[3:0] 
	{0x00012,TYPE_PARAMETER,0}, //11  SHR2_1[3:0], SHR2_2[3:0] 
	{0x00000,TYPE_PARAMETER,0}, //12  SHR2_3[3:0]              
    {0x00021,TYPE_PARAMETER,0}, //13  SPON[7:0]             
	{0x00060,TYPE_PARAMETER,0}, //14  SPOFF[7:0]        
	{0x0001B,TYPE_PARAMETER,0}, //15  SHP[3:0], SCP[3:0]
	{0x00000,TYPE_PARAMETER,0}, //16  GTO[5:0]          
	{0x00000,TYPE_PARAMETER,0}, //17  GNO[7:0]          
	{0x00010,TYPE_PARAMETER,0}, //18  CHR[7:0]          
	{0x00030,TYPE_PARAMETER,0}, //19  CON[7:0] *         
	{0x0002E,TYPE_PARAMETER,0}, //20  COFF[7:0]*         
	{0x00013,TYPE_PARAMETER,0}, //21  CHP[3:0], CCP[3:0] 
	{0x00000,TYPE_PARAMETER,0}, //22  EQ_DELAY[7:0]      
	{0x00000,TYPE_PARAMETER,0}, //23  EQ_DELAY_HSYNC[7:0]
	{0x00000,TYPE_PARAMETER,0},	//24  CGTS[18:16]        	
	{0x000C3,TYPE_PARAMETER,0}, //25  CGTS[15:8]                
	{0x00000,TYPE_PARAMETER,0}, //26  CGTS[7:0]                 
	{0x00000,TYPE_PARAMETER,0}, //27  CGTS_INV[18:16]           
	{0x00000,TYPE_PARAMETER,0}, //28  CGTS_INV[15:8]            
	{0x00000,TYPE_PARAMETER,0}, //29  CGTS_INV[7:0]             
	{0x00003,TYPE_PARAMETER,0}, //30  CGTS_ZERO_1ST_FRAME[18:16]
	{0x00000,TYPE_PARAMETER,0}, //31  CGTS_ZERO_1ST_FRAME[15:8]
	{0x00000,TYPE_PARAMETER,0}, //32  CGTS_ZERO_1ST_FRAME[7:0] 
	{0x00003,TYPE_PARAMETER,0}, //33                           
	{0x00000,TYPE_PARAMETER,0}, //34                           
	{0x00000,TYPE_PARAMETER,0}, //35                           
	{0x00004,TYPE_PARAMETER,0},	//36  USER_GIP_GATE[7:0]       	
	{0x00000,TYPE_PARAMETER,0}, //37  COS1_L_GS[3:0], COS2_L_GS[3:0]  
	{0x00000,TYPE_PARAMETER,0}, //38  COS3_L_GS[3:0], COS4_L_GS[3:0]  
	{0x00089,TYPE_PARAMETER,0}, //39  COS5_L_GS[3:0], COS6_L_GS[3:0]  
	{0x00000,TYPE_PARAMETER,0}, //40  COS7_L_GS[3:0], COS8_L_GS[3:0]  
	{0x000AA,TYPE_PARAMETER,0}, //41  COS9_L_GS[3:0], COS10_L_GS[3:0] 
	{0x00011,TYPE_PARAMETER,0}, //42  COS11_L_GS[3:0], COS12_L_GS[3:0]
	{0x00033,TYPE_PARAMETER,0}, //43  COS13_L_GS[3:0], COS14_L_GS[3:0]
	{0x00011,TYPE_PARAMETER,0}, //44  COS15_L_GS[3:0], COS16_L_GS[3:0]
	{0x00000,TYPE_PARAMETER,0}, //45  COS17_L_GS[3:0], COS18_L_GS[3:0]
	{0x00000,TYPE_PARAMETER,0}, //46  COS1_L[3:0], COS2_L[3:0]        
	{0x00000,TYPE_PARAMETER,0}, //47  COS3_L[3:0], COS4_L[3:0]        
	{0x00098,TYPE_PARAMETER,0}, //48  COS5_L[3:0], COS6_L[3:0]        
	{0x00000,TYPE_PARAMETER,0}, //49  COS7_L[3:0], COS8_L[3:0]  
	{0x00000,TYPE_PARAMETER,0}, //50  COS9_L[3:0], COS10_L[3:0] 
	{0x00022,TYPE_PARAMETER,0}, //51  COS11_L[3:0], COS12_L[3:0]
	{0x00000,TYPE_PARAMETER,0}, //52  COS13_L[3:0], COS14_L[3:0]
	{0x00044,TYPE_PARAMETER,0}, //53  COS15_L[3:0], COS16_L[3:0]
	{0x00000,TYPE_PARAMETER,0},	//54  COS17_L[3:0], COS18_L[3:0]	
	{0x00000,TYPE_PARAMETER,0}, //55  COS1_R_GS[3:0], COS2_R_GS[3:0]  
	{0x00000,TYPE_PARAMETER,0}, //56  COS3_R_GS[3:0], COS4_R_GS[3:0]  
	{0x00089,TYPE_PARAMETER,0}, //57  COS5_R_GS[3:0], COS6_R_GS[3:0]  
	{0x00000,TYPE_PARAMETER,0}, //58  COS7_R_GS[3:0], COS8_R_GS[3:0]  
	{0x00099,TYPE_PARAMETER,0}, //59  COS9_R_GS[3:0], COS10_R_GS[3:0] 
	{0x00000,TYPE_PARAMETER,0}, //60  COS11_R_GS[3:0], COS12_R_GS[3:0]
	{0x00022,TYPE_PARAMETER,0}, //61  COS13_R_GS[3:0], COS14_R_GS[3:0]
	{0x00000,TYPE_PARAMETER,0}, //62  COS15_R_GS[3:0], COS16_R_GS[3:0]
	{0x00000,TYPE_PARAMETER,0}, //63  COS17_R_GS[3:0], COS18_R_GS[3:0]
	{0x00000,TYPE_PARAMETER,0}, //64  COS1_R[3:0], COS2_R[3:0]        
	{0x00000,TYPE_PARAMETER,0}, //65  COS3_R[3:0], COS4_R[3:0]        
	{0x00098,TYPE_PARAMETER,0}, //66  COS5_R[3:0], COS6_R[3:0]        
	{0x00000,TYPE_PARAMETER,0}, //67  COS7_R[3:0], COS8_R[3:0]  
	{0x00011,TYPE_PARAMETER,0}, //68  COS9_R[3:0], COS10_R[3:0] 
	{0x00033,TYPE_PARAMETER,0}, //69  COS11_R[3:0], COS12_R[3:0]
	{0x00011,TYPE_PARAMETER,0}, //70  COS13_R[3:0], COS14_R[3:0]
	{0x00055,TYPE_PARAMETER,0}, //71  COS15_R[3:0], COS16_R[3:0]
	{0x00000,TYPE_PARAMETER,0}, //72  COS17_R[3:0], COS18_R[3:0]
	{0x00000,TYPE_PARAMETER,0}, //73  TCON_OPT[7:0] 
	{0x00000,TYPE_PARAMETER,0}, //74  GIP_OPT[22:16]
	{0x00000,TYPE_PARAMETER,0}, //75  GIP_OPT[15:8] 
	{0x00001,TYPE_PARAMETER,0}, //76  GIP_OPT[7:0]  
	{0x00000,TYPE_PARAMETER,0}, //77                
	{0x00000,TYPE_PARAMETER,0},	//78                	
	{0x00000,TYPE_PARAMETER,0}, //79                           
	{0x0000F,TYPE_PARAMETER,0}, //80  HZ_L[17:16], HZ_UL[17:16]
	{0x00000,TYPE_PARAMETER,0}, //81  HZ_L[15:8]               
	{0x000CF,TYPE_PARAMETER,0}, //82  HZ_L[7:0]                
	{0x000FF,TYPE_PARAMETER,0}, //83  HZ_UL[15:8]              
	{0x000FF,TYPE_PARAMETER,0},	//84  HZ_UL[7:0]               	  
	{0x0000F,TYPE_PARAMETER,0}, //85  HZ_R[17:16], HZ_UR[17:16]              
	{0x00000,TYPE_PARAMETER,0}, //86  HZ_R[15:8]                             
	{0x000CF,TYPE_PARAMETER,0}, //87  HZ_R[7:0]                              
	{0x000FF,TYPE_PARAMETER,0}, //88  HZ_UR[15:8]                            
	{0x000FF,TYPE_PARAMETER,0}, //89  HZ_UR[7:0]                             
	{0x00000,TYPE_PARAMETER,0}, //90  EQ_ENB_B,EQ_ENB_F, HSYNC_TO_CL1_CNT9[8] 
	{0x0002A,TYPE_PARAMETER,0}, //91  HSYNC_TO_CL1_CNT9[7:0]
	{0x0005A,TYPE_PARAMETER,0}, //92  COFF_TCON[7:0]        

    {0x000B1,MIPI_GEN_COMMAND,0}, //B1 set power	
	{0x00012,TYPE_PARAMETER,0},
	{0x00083,TYPE_PARAMETER,0},
	{0x00077,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},//07         
	{0x00090,TYPE_PARAMETER,0},             
	{0x00010,TYPE_PARAMETER,0},//BTP[4:0]0D 
	{0x0001C,TYPE_PARAMETER,0},//BTN[4:0]0D 
	{0x0001C,TYPE_PARAMETER,0},
	{0x0000C,TYPE_PARAMETER,0},
	{0x00012,TYPE_PARAMETER,0},
	
    {0x000B3,MIPI_GEN_COMMAND,0},
	{0x00083,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00031,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},

	{0x000B4,MIPI_GEN_COMMAND,0}, //B4 set display waveform cycle 
	{0x00002,TYPE_PARAMETER,0},   //40 *                          

    {0x0003A,MIPI_GEN_COMMAND,0}, //24BIT 
	{0x00070,TYPE_PARAMETER,0},

    {0x000CC,MIPI_GEN_COMMAND,0}, //REVERSE SCAN
	{0x0000E,TYPE_PARAMETER,0},

	{0x000B5,MIPI_GEN_COMMAND,0},
	{0x0000B,TYPE_PARAMETER,0},
	{0x0000B,TYPE_PARAMETER,0},
	{0x00024,TYPE_PARAMETER,0},
	
	{0x000CB,MIPI_GEN_COMMAND,0}, //24BIT 
	{0x0006D,TYPE_PARAMETER,0},

    {0x000E3,MIPI_GEN_COMMAND,0},
	{0x00006,TYPE_PARAMETER,0},
	{0x00006,TYPE_PARAMETER,0},
	{0x00006,TYPE_PARAMETER,0},
	{0x00006,TYPE_PARAMETER,0},

    {0x000C0,MIPI_GEN_COMMAND,0},
	{0x00073,TYPE_PARAMETER,0},
	{0x00050,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00034,TYPE_PARAMETER,0},
	{0x000C4,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},

    {0x000C1,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},

    {0x000BA,MIPI_GEN_COMMAND,0},
	{0x00031,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00016,TYPE_PARAMETER,0},
	{0x000CA,TYPE_PARAMETER,0},
	{0x000B0,TYPE_PARAMETER,0},
	{0x0000A,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00010,TYPE_PARAMETER,0},
	{0x00028,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00021,TYPE_PARAMETER,0},
	{0x00021,TYPE_PARAMETER,0},
	{0x0009A,TYPE_PARAMETER,0},
	{0x0001A,TYPE_PARAMETER,0},
	{0x0008F,TYPE_PARAMETER,0},

    {0x000C6,MIPI_GEN_COMMAND,0},
	{0x00041,TYPE_PARAMETER,0},
	{0x000FF,TYPE_PARAMETER,0},
	{0x0007D,TYPE_PARAMETER,0},
	{0x000FF,TYPE_PARAMETER,0},

    {0x000E0,MIPI_GEN_COMMAND,0},
//GAMMA=2.5
/*
	{0x00000,TYPE_PARAMETER,0},
	{0x00020,TYPE_PARAMETER,0},
	{0x00025,TYPE_PARAMETER,0},
	{0x0003E,TYPE_PARAMETER,0},
	{0x0003E,TYPE_PARAMETER,0},
	{0x0003F,TYPE_PARAMETER,0},
	{0x00035,TYPE_PARAMETER,0},
	{0x0004F,TYPE_PARAMETER,0},
	{0x00007,TYPE_PARAMETER,0},
	{0x0000D,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x00012,TYPE_PARAMETER,0},
	{0x00014,TYPE_PARAMETER,0},
	{0x00013,TYPE_PARAMETER,0},
	{0x00014,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x00017,TYPE_PARAMETER,0},

	{0x00000,TYPE_PARAMETER,0},
	{0x00020,TYPE_PARAMETER,0},
	{0x00025,TYPE_PARAMETER,0},
	{0x0003E,TYPE_PARAMETER,0},
	{0x0003E,TYPE_PARAMETER,0},
	{0x0003F,TYPE_PARAMETER,0},
	{0x00032,TYPE_PARAMETER,0},
	{0x0004B,TYPE_PARAMETER,0},
	{0x00007,TYPE_PARAMETER,0},
	{0x0000D,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x00012,TYPE_PARAMETER,0},
	{0x00014,TYPE_PARAMETER,0},
	{0x00013,TYPE_PARAMETER,0},
	{0x00014,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x00017,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
*/
//GAMMA=2.2
	{0x00000,TYPE_PARAMETER,0},
	{0x0001E,TYPE_PARAMETER,0},
	{0x0001D,TYPE_PARAMETER,0},
	{0x00036,TYPE_PARAMETER,0},
	{0x0003E,TYPE_PARAMETER,0},
	{0x0003F,TYPE_PARAMETER,0},
	{0x00030,TYPE_PARAMETER,0},
	{0x0004A,TYPE_PARAMETER,0},
	{0x00008,TYPE_PARAMETER,0},
	{0x0000C,TYPE_PARAMETER,0},
	{0x00010,TYPE_PARAMETER,0},
	{0x00015,TYPE_PARAMETER,0},
	{0x00017,TYPE_PARAMETER,0},
	{0x00015,TYPE_PARAMETER,0},
	{0x00016,TYPE_PARAMETER,0},
	{0x00012,TYPE_PARAMETER,0},
	{0x00017,TYPE_PARAMETER,0},

	{0x00000,TYPE_PARAMETER,0},
	{0x0001E,TYPE_PARAMETER,0},
	{0x0001D,TYPE_PARAMETER,0},
	{0x00036,TYPE_PARAMETER,0},
	{0x0003E,TYPE_PARAMETER,0},
	{0x0003F,TYPE_PARAMETER,0},
	{0x00030,TYPE_PARAMETER,0},
	{0x0004A,TYPE_PARAMETER,0},
	{0x00008,TYPE_PARAMETER,0},
	{0x0000C,TYPE_PARAMETER,0},
	{0x00010,TYPE_PARAMETER,0},
	{0x00015,TYPE_PARAMETER,0},
	{0x00017,TYPE_PARAMETER,0},
	{0x00015,TYPE_PARAMETER,0},
	{0x00016,TYPE_PARAMETER,0},
	{0x00012,TYPE_PARAMETER,0},
	{0x00017,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},

	{0x000EA,MIPI_GEN_COMMAND,0},//PWM ON
	{0x0007A,TYPE_PARAMETER,0},

	{0x000C9,MIPI_GEN_COMMAND,0}, // pwm FQ
	{0x0000F,TYPE_PARAMETER,0}, //32kHZ
	{0x00000,TYPE_PARAMETER,0},

	{0x00051,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0}, 

	{0x00053,MIPI_DCS_COMMAND,0}, // dimming
	{0x00024,TYPE_PARAMETER,0}, 
	
	{0x00055,MIPI_DCS_COMMAND,0}, // UI MODE
	{0x00001,TYPE_PARAMETER,0},

	{0x00011,MIPI_DCS_COMMAND,0},
	{0x00029,MIPI_DCS_COMMAND,120},			
    {0x00000,MIPI_TYPE_END,20}, //the end flag,it don't sent to driver IC

};

/*lcd resume function*/
/*setting for support continuous splash */
static int mipi_hx8369b_lcd_on(struct platform_device *pdev)
{

	boolean para_debug_flag = FALSE;
	uint32 para_num = 0;
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);
	
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL; 

	para_debug_flag = lcd_debug_malloc_get_para( "hx8369b_lcd_init_table_debug", 
			(void**)&hx8369b_lcd_init_table_debug,&para_num);
	
	lcd_reset();
	mipi_set_tx_power_mode(1);//Low power mode 
	
	if( (TRUE == para_debug_flag) && (NULL != hx8369b_lcd_init_table_debug))
	{
		process_mipi_table(mfd,&hx8369b_tx_buf,hx8369b_lcd_init_table_debug,
			para_num, lcd_panel_wvga);
	}
	else
	{
		process_mipi_table(mfd,&hx8369b_tx_buf,(struct sequence*)&hx8369b_tianma_lcd_init_table,
			ARRAY_SIZE(hx8369b_tianma_lcd_init_table), lcd_panel_wvga);
	}
	
	mipi_set_tx_power_mode(0);//High speed mode 
	
	if((TRUE == para_debug_flag)&&(NULL != hx8369b_lcd_init_table_debug))
	{
		lcd_debug_free_para((void *)hx8369b_lcd_init_table_debug);
	}
	
	if (!mfd->cont_splash_done) {
		mfd->cont_splash_done = 1;
		hx8369b_wvga_write_cabc_brightness_table[1].reg = 0x00064;
		process_mipi_table(mfd,&hx8369b_tx_buf,(struct sequence*)&hx8369b_wvga_write_cabc_brightness_table,
			ARRAY_SIZE(hx8369b_wvga_write_cabc_brightness_table), lcd_panel_wvga);
	}
	
	LCD_DEBUG("leave mipi_otm8018b_lcd_on \n");
	return 0;
}

/*lcd suspend function*/
static int mipi_hx8369b_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	lcd_reset();
	
	LCD_DEBUG("leave mipi_hx8369b_lcd_off \n");
	return 0;
}

/* Add auto cabc function */
#ifdef CONFIG_FB_AUTO_CABC
static struct sequence hx8369b_auto_cabc_set_table[] =
{
	{0x00055,MIPI_DCS_COMMAND,0 }, 
	{0x00001,TYPE_PARAMETER,0},
	{0xFFFFF,MIPI_TYPE_END,0}, //the end flag,it don't sent to driver IC
};

/***************************************************************
Function: hx8369b_config_auto_cabc
Description: Set CABC configuration
Parameters:
    struct msmfb_cabc_config cabc_cfg: CABC configuration struct
Return:
    0: success
***************************************************************/
static int hx8369b_config_auto_cabc(struct msmfb_cabc_config cabc_cfg,struct msm_fb_data_type *mfd)
{
	int ret = 0;

	switch(cabc_cfg.mode)
	{
		case CABC_MODE_UI:
			hx8369b_auto_cabc_set_table[1].reg=0x00001;
			break;
		case CABC_MODE_MOVING:
		case CABC_MODE_STILL:
			hx8369b_auto_cabc_set_table[1].reg=0x00003;
			break;
		default:
			LCD_DEBUG("%s: invalid cabc mode: %d\n", __func__, cabc_cfg.mode);
	        ret = -EINVAL;
			break;
	}
	if(likely(0 == ret))
	{
		process_mipi_table(mfd,&hx8369b_tx_buf,(struct sequence*)&hx8369b_auto_cabc_set_table,
			 ARRAY_SIZE(hx8369b_auto_cabc_set_table), lcd_panel_wvga);
	}

	LCD_DEBUG("%s: change cabc mode to %d\n",__func__,cabc_cfg.mode);
	return ret;
}
#endif // CONFIG_FB_AUTO_CABC

static int __devinit mipi_hx8369b_lcd_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);

	return 0;
}
/*lcd cabc control function*/
void hx8369b_set_cabc_backlight(struct msm_fb_data_type *mfd,uint32 bl_level)
{	
	hx8369b_wvga_write_cabc_brightness_table[1].reg = bl_level; 

	process_mipi_table(mfd,&hx8369b_tx_buf,(struct sequence*)&hx8369b_wvga_write_cabc_brightness_table,
		 ARRAY_SIZE(hx8369b_wvga_write_cabc_brightness_table), lcd_panel_wvga);
}

static struct platform_driver this_driver = {
	.probe  = mipi_hx8369b_lcd_probe,
	.driver = {
		.name   = LCD_DEVICE_NAME,
	},
};
static struct msm_fb_panel_data hx8369b_panel_data = {
	.on		= mipi_hx8369b_lcd_on,
	.off	= mipi_hx8369b_lcd_off,
	.set_backlight = pwm_set_backlight,
	/*add cabc control backlight*/
	.set_cabc_brightness = hx8369b_set_cabc_backlight,
#ifdef CONFIG_FB_AUTO_CABC
    .config_cabc = hx8369b_config_auto_cabc,
#endif
};
static struct platform_device this_device = {
	.name   = LCD_DEVICE_NAME,
	.id	= 0,
	.dev	= {
		.platform_data = &hx8369b_panel_data,
	}
};

static int __init mipi_video_hx8369b_wvga_init(void)
{
	int ret = 0;
	struct msm_panel_info *pinfo = NULL;

	lcd_panel_wvga = get_lcd_panel_type();
	if ((MIPI_VIDEO_HX8369B_TIANMA_WVGA!= lcd_panel_wvga ))
	{
		return 0;
	}
	LCD_DEBUG("enter mipi_video_hx8369b_wvga_init \n");
	mipi_dsi_buf_alloc(&hx8369b_tx_buf, DSI_BUF_SIZE);

	ret = platform_driver_register(&this_driver);
	if (!ret)
	{
	 	pinfo = &hx8369b_panel_data.panel_info;
		pinfo->xres = 480;
		pinfo->yres = 800;
		pinfo->type = MIPI_VIDEO_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 24;		
		/*modify setting parameter*/
		pinfo->lcdc.h_back_porch = 55;
		pinfo->lcdc.h_front_porch = 55;
		pinfo->lcdc.h_pulse_width = 20;
		pinfo->lcdc.v_back_porch = 21;
		pinfo->lcdc.v_front_porch = 17;
		pinfo->lcdc.v_pulse_width = 4;
		pinfo->lcdc.border_clr = 0;	/* blk */
		pinfo->lcdc.underflow_clr = 0xff;	/* blue */
		/* number of dot_clk cycles HSYNC active edge is
		delayed from VSYNC active edge */
		pinfo->lcdc.hsync_skew = 0;
		pinfo->bl_max = 255;
		pinfo->bl_min = 30;		
		
		pinfo->fb_num = 3;
		
        pinfo->clk_rate = 360000000;

		pinfo->mipi.mode = DSI_VIDEO_MODE;
		pinfo->mipi.pulse_mode_hsa_he = TRUE;
		/* to false*/
		pinfo->mipi.hfp_power_stop = TRUE; /* LP-11 during the HFP period */
		pinfo->mipi.hbp_power_stop = TRUE; /* LP-11 during the HBP period */
		pinfo->mipi.hsa_power_stop = TRUE; /* LP-11 during the HSA period */
		/* LP-11 or let Command Mode Engine send packets in
		HS or LP mode for the BLLP of the last line of a frame */
		pinfo->mipi.eof_bllp_power_stop = TRUE;
		/* LP-11 or let Command Mode Engine send packets in
		HS or LP mode for packets sent during BLLP period */
		pinfo->mipi.bllp_power_stop = TRUE;

		pinfo->mipi.traffic_mode = DSI_BURST_MODE;
		pinfo->mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
		pinfo->mipi.vc = 0;
		pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RGB;
		pinfo->mipi.data_lane0 = TRUE;
		pinfo->mipi.data_lane1 = TRUE;
		pinfo->mipi.t_clk_post = 0xB0;
		pinfo->mipi.t_clk_pre = 0x2f;
		pinfo->mipi.stream = 0; /* dma_p */
		pinfo->mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
		pinfo->mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
		pinfo->mipi.frame_rate = 60; /* FIXME */

		pinfo->mipi.dsi_phy_db = &dsi_video_mode_phy_db;
		pinfo->mipi.tx_eot_append = 0x01;
		pinfo->mipi.dlane_swap = 0x1;

		ret = platform_device_register(&this_device);
		if (ret)
			LCD_DEBUG("%s: failed to register device!\n", __func__);
	}

	return ret;
}

module_init(mipi_video_hx8369b_wvga_init);
