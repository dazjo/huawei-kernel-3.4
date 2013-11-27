/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
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

#define LCD_DEVICE_NAME "mipi_cmd_otm8009a_fwvga"

static lcd_panel_type lcd_panel_fwvga = LCD_NONE;

/* increase the DSI bit clock to 490 MHz */
static struct mipi_dsi_phy_ctrl dsi_cmd_mode_phy_db_otm8009a_fwvga = {
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

static struct dsi_buf otm8009a_fwvga_tx_buf;
#if LCD_OTM8009A_CMI_ESD_SIGN
static struct dsi_buf otm8009a_fwvga_rx_buf;
static struct hrtimer lcd_esd_timer;
static struct msm_fb_data_type *otm8009a_g_mfd;
static struct work_struct otm8009a_work;
static struct workqueue_struct *lcd_esd_wq;
#endif
static struct sequence * otm8009a_lcd_init_table_debug = NULL;

static const struct sequence otm8009a_fwvga_standby_enter_table[]= 
{
	{0x00028,MIPI_DCS_COMMAND,0},
    {0x00010,MIPI_DCS_COMMAND,20},
    {0x00010,MIPI_TYPE_END,120},
};
static const struct sequence otm8009a_fwvga_standby_exit_table[]= 
{
	{0x00011,MIPI_DCS_COMMAND,0},
    {0x00029,MIPI_DCS_COMMAND,120},
	{0x00029,MIPI_TYPE_END,20},
};
#if LCD_OTM8009A_CMI_ESD_SIGN
static struct sequence otm8009a_chimei_lcd_init_table[] =
{
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000FF,MIPI_GEN_COMMAND,0},
	{0x00080,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00080,TYPE_PARAMETER,0},
	{0x000FF,MIPI_GEN_COMMAND,0},
	{0x00080,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,1},

	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00080,TYPE_PARAMETER,0},
	{0x000F5,MIPI_GEN_COMMAND,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00010,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x0000F,TYPE_PARAMETER,0},
	{0x00020,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00090,TYPE_PARAMETER,0},
	{0x000F5,MIPI_GEN_COMMAND,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00008,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00006,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x0000D,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x0000B,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000A0,TYPE_PARAMETER,0},
	{0x000F5,MIPI_GEN_COMMAND,0},
	{0x00010,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00014,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00014,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000B0,TYPE_PARAMETER,0},
	{0x000F5,MIPI_GEN_COMMAND,0},
	{0x00014,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00012,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00013,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00011,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00013,TYPE_PARAMETER,0},
	{0x00018,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},

	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000B4,TYPE_PARAMETER,0},
	{0x000C0,MIPI_GEN_COMMAND,0},
	{0x00055,TYPE_PARAMETER,0},

	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00082,TYPE_PARAMETER,0},
	{0x000C5,MIPI_GEN_COMMAND,0},
	{0x000A3,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00090,TYPE_PARAMETER,0},
	{0x000C5,MIPI_GEN_COMMAND,0},
	{0x000D6,TYPE_PARAMETER,0},
	{0x00076,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000D8,MIPI_GEN_COMMAND,0},
	{0x000A7,TYPE_PARAMETER,0},
	{0x000A7,TYPE_PARAMETER,0},

	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00081,TYPE_PARAMETER,0},
	{0x000C1,MIPI_GEN_COMMAND,0},
	{0x00055,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000A1,TYPE_PARAMETER,0},
	{0x000C1,MIPI_GEN_COMMAND,0},
	{0x00008,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000A3,TYPE_PARAMETER,0},
	{0x000C0,MIPI_GEN_COMMAND,0},
	{0x0001B,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00081,TYPE_PARAMETER,0},
	{0x000C4,MIPI_GEN_COMMAND,0},
	{0x00083,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00092,TYPE_PARAMETER,0},
	{0x000C5,MIPI_GEN_COMMAND,0},
	{0x00001,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000B1,TYPE_PARAMETER,0},
	{0x000C5,MIPI_GEN_COMMAND,0},
	{0x000A9,TYPE_PARAMETER,0},

	/* set LCD_backlight PWM frequency 22.786kHz */	
	{0x00, MIPI_DCS_COMMAND,0},		/* set address mode */
	{0xB0, TYPE_PARAMETER,0},		/* set address 0xC6B0 */		

	{0xC6, MIPI_GEN_COMMAND,0},		/* the register set LCD_backlight PWM frequency*/ 
	{0x03, TYPE_PARAMETER,0},
	{0x05, TYPE_PARAMETER,0},
	{0x00, TYPE_PARAMETER,0},
	{0x5F, TYPE_PARAMETER,0},
	{0x12, TYPE_PARAMETER,0},
	{0x00, TYPE_PARAMETER,0},
	
	{0x00,MIPI_DCS_COMMAND,0},		/* set address mode */
	{0xC0,TYPE_PARAMETER,0},		/* set address 0xB3C0 */

	{0xB3,MIPI_GEN_COMMAND,0},
	{0x09,TYPE_PARAMETER,0},		
	{0x10,TYPE_PARAMETER,0},		/* turn on SRAM power during sleep in mode when IM = X101 */
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00080,TYPE_PARAMETER,0},
	{0x000CE,MIPI_GEN_COMMAND,0},
	{0x00084,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00083,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},

	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00090,TYPE_PARAMETER,0},
	{0x000b3,MIPI_GEN_COMMAND,0},
	{0x00002,TYPE_PARAMETER,0},

	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00092,TYPE_PARAMETER,0},
	{0x000b3,MIPI_GEN_COMMAND,0},
	{0x00045,TYPE_PARAMETER,0},

	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00080,TYPE_PARAMETER,0},
	{0x000C0,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00057,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00015,TYPE_PARAMETER,0},
	{0x00015,TYPE_PARAMETER,0},

	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00090,TYPE_PARAMETER,0},
	{0x000CE,MIPI_GEN_COMMAND,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x0005d,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x0005e,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000A0,TYPE_PARAMETER,0},
	{0x000CE,MIPI_GEN_COMMAND,0},
	{0x00038,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00057,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00038,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00058,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000B0,TYPE_PARAMETER,0},
	{0x000CE,MIPI_GEN_COMMAND,0},
	{0x00038,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00059,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00030,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0005a,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000C0,TYPE_PARAMETER,0},
	{0x000CE,MIPI_GEN_COMMAND,0},
	{0x00030,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0005b,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00030,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0005c,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000D0,TYPE_PARAMETER,0},
	{0x000CE,MIPI_GEN_COMMAND,0},
	{0x00030,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0005d,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00030,TYPE_PARAMETER,0},
	{0x00004,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0005e,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},

	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000C7,TYPE_PARAMETER,0},
	{0x000CF,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000C0,TYPE_PARAMETER,0},
	{0x000CB,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000D0,TYPE_PARAMETER,0},
	{0x000CB,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000E0,TYPE_PARAMETER,0},
	{0x000CB,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00080,TYPE_PARAMETER,0},
	{0x000CC,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0000C,TYPE_PARAMETER,0},
	{0x0000A,TYPE_PARAMETER,0},
	{0x00010,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00090,TYPE_PARAMETER,0},
	{0x000CC,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00006,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0000B,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000A0,TYPE_PARAMETER,0},
	{0x000CC,MIPI_GEN_COMMAND,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x0000F,TYPE_PARAMETER,0},
	{0x0000D,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00005,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000B0,TYPE_PARAMETER,0},
	{0x000CC,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0000d,TYPE_PARAMETER,0},
	{0x0000f,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x0000b,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00005,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000C0,TYPE_PARAMETER,0},
	{0x000CC,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00036,MIPI_DCS_COMMAND,0},
	{0x00040,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x000D0,TYPE_PARAMETER,0},
	{0x000CC,MIPI_GEN_COMMAND,0},
	{0x00010,TYPE_PARAMETER,0},
	{0x0000A,TYPE_PARAMETER,0},
	{0x0000C,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00006,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},

	/* E1 */
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E1,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00011,TYPE_PARAMETER,0},
	{0x00017,TYPE_PARAMETER,0},
	{0x0000f,TYPE_PARAMETER,0},
	{0x00007,TYPE_PARAMETER,0},
	{0x00010,TYPE_PARAMETER,0},
	{0x0000a,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x00004,TYPE_PARAMETER,0},
	{0x00007,TYPE_PARAMETER,0},
	{0x0000f,TYPE_PARAMETER,0},
	{0x0000c,TYPE_PARAMETER,0},
	{0x0000d,TYPE_PARAMETER,0},
	{0x0000e,TYPE_PARAMETER,0},
	{0x00008,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	/* E2 */
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E2,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00011,TYPE_PARAMETER,0},
	{0x00017,TYPE_PARAMETER,0},
	{0x0000f,TYPE_PARAMETER,0},
	{0x00007,TYPE_PARAMETER,0},
	{0x00010,TYPE_PARAMETER,0},
	{0x0000a,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x00004,TYPE_PARAMETER,0},
	{0x00007,TYPE_PARAMETER,0},
	{0x0000f,TYPE_PARAMETER,0},
	{0x0000c,TYPE_PARAMETER,0},
	{0x0000d,TYPE_PARAMETER,0},
	{0x0000e,TYPE_PARAMETER,0},
	{0x00008,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	/* EC */
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000EC,MIPI_GEN_COMMAND,0},
	{0x00040,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00034,TYPE_PARAMETER,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	/* ED */
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000ED,MIPI_GEN_COMMAND,0},
	{0x00040,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00004,TYPE_PARAMETER,0},
	/* EE */
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000EE,MIPI_GEN_COMMAND,0},
	{0x00040,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00004,TYPE_PARAMETER,0},

	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000FF,MIPI_GEN_COMMAND,0},
	{0x000FF,TYPE_PARAMETER,0},
	{0x000FF,TYPE_PARAMETER,0},
	{0x000FF,TYPE_PARAMETER,0},

	/* delete 26h for already set in gamma */
 
	{0x00000,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0002B,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00056,TYPE_PARAMETER,0},

	/* move 11h before 29h */
	{0x00011,MIPI_DCS_COMMAND,0},
	{0x00000,MIPI_DCS_COMMAND,120},
	{0x00000,TYPE_PARAMETER,0},
	{0x00029,MIPI_DCS_COMMAND,0},
 
	{0x0002c,MIPI_DCS_COMMAND,20},
	
	{0x00051,MIPI_DCS_COMMAND,20},
	{0x0007f,TYPE_PARAMETER,0},
	
	{0x00053,MIPI_DCS_COMMAND,0}, // ctrldisplay1
	{0x00024,TYPE_PARAMETER,0}, 
	{0x00055,MIPI_DCS_COMMAND,0}, // ctrldisplay2
	{0x00001,TYPE_PARAMETER,0},
 
	{0x00000,MIPI_TYPE_END,20}, //end flag
};


static const struct read_sequence otm8009a_esd_read_table_0A[] = 
{
	{0x0A,MIPI_DCS_COMMAND,1},
};
static const struct read_sequence otm8009a_esd_read_table_0B[] = 
{
	{0x0B,MIPI_DCS_COMMAND,1},
};

static const struct read_sequence otm8009a_esd_read_table_0C[] = 
{
	{0x0C,MIPI_DCS_COMMAND,1},
};

static const struct read_sequence otm8009a_esd_read_table_0D[] = 
{
	{0x0D,MIPI_DCS_COMMAND,1},
};

static int otm8009a_fwvga_monitor_reg_status(struct msm_fb_data_type *mfd) 
{
	
    struct dsi_buf *rp, *tp;
    uint8 err = 0;//ok
    uint8 read_value[3]={0, 0, 0};
	static uint32 count=0;
    tp = &otm8009a_fwvga_tx_buf; 
    rp = &otm8009a_fwvga_rx_buf; 
	
    mipi_dsi_buf_init(tp); 
    mipi_dsi_buf_init(rp); 
	
	process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&otm8009a_esd_read_table_0A);
	read_value[0] = *((unsigned char *)rp->data);
	if (0x9c != read_value[0])
	{
		LCD_DEBUG("0x0a value0 = %02x\n",read_value[0]);
		process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&otm8009a_esd_read_table_0A);
		read_value[1] = *((unsigned char *)rp->data);
		if (0x9c != read_value[1])
		{
			LCD_DEBUG("0x0a value1 = %02x\n",read_value[1]);
			process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&otm8009a_esd_read_table_0A);	
			read_value[2] = *((unsigned char *)rp->data);
			if (0x9c != read_value[2])
			{
				LCD_DEBUG("0x0a value2 = %02x\n",read_value[2]);
				err = 1;
			}
		}
	}

	process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&otm8009a_esd_read_table_0B);
	read_value[0] = *((unsigned char *)rp->data);		
	if (0x40 != read_value[0])
	{
		LCD_DEBUG("0x0b value0 = %02x\n",read_value[0]);
		process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&otm8009a_esd_read_table_0B);
		read_value[1] = *((unsigned char *)rp->data);
		if (0x40 != read_value[1])
		{
			LCD_DEBUG("0x0b value1 = %02x\n",read_value[1]);
			process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&otm8009a_esd_read_table_0B);	
			read_value[2] = *((unsigned char *)rp->data);
			if (0x40 != read_value[2])
			{
				LCD_DEBUG("0x0b value2 = %02x\n",read_value[2]);
				err = 2;
			}
		}
	}
	process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&otm8009a_esd_read_table_0C);
	read_value[0] = *((unsigned char *)rp->data);		
	if (0x07 != read_value[0])
	{
		LCD_DEBUG("0x0c value0 = %02x\n",read_value[0]);
		process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&otm8009a_esd_read_table_0C);
		read_value[1] = *((unsigned char *)rp->data);
		if (0x07 != read_value[1])
		{
			LCD_DEBUG("0x0c value1 = %02x\n",read_value[1]);
			process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&otm8009a_esd_read_table_0C);	
			read_value[2] = *((unsigned char *)rp->data);
			if (0x07 != read_value[2])
			{
				LCD_DEBUG("0x0c value2 = %02x\n",read_value[2]);
				err = 3;
			}
		}
	}
	process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&otm8009a_esd_read_table_0D);
	read_value[0] = *((unsigned char *)rp->data);		
	if (0x00 != read_value[0])
	{
		LCD_DEBUG("0x0d value0 = %02x\n",read_value[0]);
		process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&otm8009a_esd_read_table_0D);
		read_value[1] = *((unsigned char *)rp->data);
		if (0x00 != read_value[1])
		{
			LCD_DEBUG("0x0d value1 = %02x\n",read_value[1]);
			process_mipi_read_table(mfd,tp,rp,(struct read_sequence*)&otm8009a_esd_read_table_0D);	
			read_value[2] = *((unsigned char *)rp->data);
			if (0x00 != read_value[2])
			{
				LCD_DEBUG("0x0d value2 = %02x\n",read_value[2]);
				err = 4;
			}
		}
	}
	
	return ((count++ < 3) ? 0 : err);	
}

static void lcd_esd_check(struct msm_fb_data_type *mfd)
{
	int ret = 0;
	
	/* Read status of registers begin */
	
	ret = otm8009a_fwvga_monitor_reg_status(mfd);
	
    if(ret)
	{ 
       
	   lcd_reset();
	   mipi_set_tx_power_mode(1);
	  
	   process_mipi_table(mfd,&otm8009a_fwvga_tx_buf,(struct sequence*)&otm8009a_chimei_lcd_init_table,
			ARRAY_SIZE(otm8009a_chimei_lcd_init_table), lcd_panel_fwvga);
	   mipi_set_tx_power_mode(0);
	      
       LCD_DEBUG("LCD reset and initial again.\n"); 
    } 
	/*Read status of registers end */
}


static enum hrtimer_restart lcd_esd_timer_func(struct hrtimer *timer)
{	
	queue_work(lcd_esd_wq, &otm8009a_work);

	return HRTIMER_NORESTART;
}

static void lcd_esd_func(struct work_struct *work)
{
	if (!otm8009a_g_mfd)
		return ;
	if (otm8009a_g_mfd->key != MFD_KEY)
		return ;

	down(&otm8009a_g_mfd->dma->mutex);
	down(&otm8009a_g_mfd->sem);
	
	lcd_esd_check(otm8009a_g_mfd);
	
	up(&otm8009a_g_mfd->sem);
	up(&otm8009a_g_mfd->dma->mutex);	

	hrtimer_start(&lcd_esd_timer, ktime_set(3,0), HRTIMER_MODE_REL);
	//LCD_DEBUG("leave read_timer_func \n");	
}
#endif
/*lcd resume function*/
static int mipi_otm8009a_fwvga_lcd_on(struct platform_device *pdev)
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
	para_debug_flag = lcd_debug_malloc_get_para( "otm8009a_lcd_init_table_debug", 
            (void**)&otm8009a_lcd_init_table_debug,&para_num);

    if( (TRUE == para_debug_flag) && (NULL != otm8009a_lcd_init_table_debug))
    {
        process_mipi_table(mfd,&otm8009a_fwvga_tx_buf,otm8009a_lcd_init_table_debug,
		     para_num, lcd_panel_fwvga);
    }
	else
	{
        /* low power mode*/
        mipi_set_tx_power_mode(1);
		process_mipi_table(mfd,&otm8009a_fwvga_tx_buf,(struct sequence*)&otm8009a_fwvga_standby_exit_table,
		 	ARRAY_SIZE(otm8009a_fwvga_standby_exit_table), lcd_panel_fwvga);
		mipi_set_tx_power_mode(0);
		/*delete some lines */
	}

	if((TRUE == para_debug_flag)&&(NULL != otm8009a_lcd_init_table_debug))
	{
		lcd_debug_free_para((void *)otm8009a_lcd_init_table_debug);
	}
#if LCD_OTM8009A_CMI_ESD_SIGN
	otm8009a_g_mfd = mfd;
	hrtimer_start(&lcd_esd_timer, ktime_set(3, 0), HRTIMER_MODE_REL);
#endif

	LCD_DEBUG("leave mipi_otm8009a_fwvga_lcd_on \n");
	return 0;
}
/*lcd suspend function*/
static int mipi_otm8009a_fwvga_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
#if LCD_OTM8009A_CMI_ESD_SIGN
	hrtimer_cancel(&lcd_esd_timer);
#endif

	process_mipi_table(mfd,&otm8009a_fwvga_tx_buf,(struct sequence*)&otm8009a_fwvga_standby_enter_table,
		 ARRAY_SIZE(otm8009a_fwvga_standby_enter_table), lcd_panel_fwvga);
	LCD_DEBUG("leave mipi_otm8009a_fwvga_lcd_off \n");
	return 0;
}

#ifdef CONFIG_FB_AUTO_CABC
static struct sequence otm8009a_fwvga_auto_cabc_set_table[] =
{	
	{0x00053,MIPI_DCS_COMMAND,0},
	{0x00024,TYPE_PARAMETER,0},

    {0x00055,MIPI_DCS_COMMAND,0}, 
	{0x00001,TYPE_PARAMETER,0},

	{0xFFFFF,MIPI_TYPE_END,0}, /* the end flag,it don't sent to driver IC */
};
/***************************************************************
Function: otm8009a_fwvga_config_auto_cabc
Description: Set CABC configuration
Parameters:
	struct msmfb_cabc_config cabc_cfg: CABC configuration struct
Return:
	0: success
***************************************************************/
static int otm8009a_fwvga_config_auto_cabc(struct msmfb_cabc_config cabc_cfg, struct msm_fb_data_type *mfd)
{
	int ret = 0;

	switch(cabc_cfg.mode)
	{
		case CABC_MODE_UI:
			otm8009a_fwvga_auto_cabc_set_table[1].reg=0x00024;
			otm8009a_fwvga_auto_cabc_set_table[3].reg=0x00001;
			break;
		case CABC_MODE_MOVING:
		case CABC_MODE_STILL:
			otm8009a_fwvga_auto_cabc_set_table[1].reg=0x0002C;
			otm8009a_fwvga_auto_cabc_set_table[3].reg=0x00003;
			break;
		default:
			LCD_DEBUG("%s: invalid cabc mode: %d\n", __func__, cabc_cfg.mode);
	        ret = -EINVAL;
			break;
	}
	if(likely(0 == ret))
	{
		process_mipi_table(mfd,&otm8009a_fwvga_tx_buf,(struct sequence*)&otm8009a_fwvga_auto_cabc_set_table,
			                ARRAY_SIZE(otm8009a_fwvga_auto_cabc_set_table), lcd_panel_fwvga);
	}

	LCD_DEBUG("%s: change cabc mode to %d\n",__func__,cabc_cfg.mode);
	return ret;
}
#endif /* CONFIG_FB_AUTO_CABC */

static int __devinit mipi_otm8009a_fwvga_lcd_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);

	return 0;
}

static struct sequence otm8009a_fwvga_cabc_enable_table[] =
{	
	{0x00051,MIPI_DCS_COMMAND,0}, 		
	{0x000ff,TYPE_PARAMETER,0},
	
	{0x00029,MIPI_TYPE_END,0},
};
void otm8009a_fwvga_set_cabc_backlight(struct msm_fb_data_type *mfd,uint32 bl_level)
{	
	otm8009a_fwvga_cabc_enable_table[1].reg = bl_level; // 1 will be changed if modify init code

	process_mipi_table(mfd,&otm8009a_fwvga_tx_buf,(struct sequence*)&otm8009a_fwvga_cabc_enable_table,
		 ARRAY_SIZE(otm8009a_fwvga_cabc_enable_table), lcd_panel_fwvga);
}

static struct platform_driver this_driver = {
	.probe  = mipi_otm8009a_fwvga_lcd_probe,
	.driver = {
		.name   = LCD_DEVICE_NAME,
	},
};
static struct msm_fb_panel_data otm8009a_fwvga_panel_data = {
	.on					= mipi_otm8009a_fwvga_lcd_on,
	.off					= mipi_otm8009a_fwvga_lcd_off,
	.set_backlight 		= pwm_set_backlight,
	/*add cabc control backlight*/
	.set_cabc_brightness 	= otm8009a_fwvga_set_cabc_backlight,
	#ifdef CONFIG_FB_AUTO_CABC
	.config_cabc = otm8009a_fwvga_config_auto_cabc,
    #endif
};
static struct platform_device this_device = {
	.name   = LCD_DEVICE_NAME,
	.id	= 0,
	.dev	= {
		.platform_data = &otm8009a_fwvga_panel_data,
	}
};
static int __init mipi_cmd_otm8009a_fwvga_init(void)
{
	int ret = 0;
	struct msm_panel_info *pinfo = NULL;
	
	lcd_panel_fwvga = get_lcd_panel_type();
	if (MIPI_CMD_OTM8009A_CHIMEI_FWVGA != lcd_panel_fwvga)
	{
		return 0;
	}
	LCD_DEBUG("enter mipi_cmd_otm8009a_fwvga_init \n");
	mipi_dsi_buf_alloc(&otm8009a_fwvga_tx_buf, DSI_BUF_SIZE);
#if LCD_OTM8009A_CMI_ESD_SIGN
	mipi_dsi_buf_alloc(&otm8009a_fwvga_rx_buf, DSI_BUF_SIZE);
#endif

	ret = platform_driver_register(&this_driver);
	if (!ret)
	{
	 	pinfo = &otm8009a_fwvga_panel_data.panel_info;
		pinfo->xres = 480;
		pinfo->yres = 854;
		pinfo->type = MIPI_CMD_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 24;		
		pinfo->bl_max = 255;
		pinfo->bl_min = 30;		
		pinfo->fb_num = 3;
        /* increase the DSI bit clock to 490 MHz */
        pinfo->clk_rate = 490000000;
		pinfo->lcd.refx100 = 6000; /* adjust refx100 to prevent tearing */

		pinfo->mipi.mode = DSI_CMD_MODE;
		pinfo->mipi.dst_format = DSI_CMD_DST_FORMAT_RGB888;
		pinfo->mipi.vc = 0;
		pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RGB;
		pinfo->mipi.data_lane0 = TRUE;
		pinfo->mipi.data_lane1 = TRUE;
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
		pinfo->mipi.dsi_phy_db = &dsi_cmd_mode_phy_db_otm8009a_fwvga;
		pinfo->mipi.tx_eot_append = 0x01;
		pinfo->mipi.rx_eot_ignore = 0;
		pinfo->mipi.dlane_swap = 0x1;

		ret = platform_device_register(&this_device);
		if (ret)
			LCD_DEBUG("%s: failed to register device!\n", __func__);
	}
#if LCD_OTM8009A_CMI_ESD_SIGN
	hrtimer_init(&lcd_esd_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	lcd_esd_timer.function = lcd_esd_timer_func;
	lcd_esd_wq = create_singlethread_workqueue("lcd_esd_wq");
	INIT_WORK(&otm8009a_work, lcd_esd_func);
#endif
	return ret;
}

module_init(mipi_cmd_otm8009a_fwvga_init);
