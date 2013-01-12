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

#define LCD_DEVICE_NAME "mipi_video_otm8018b_wvga"

static lcd_panel_type lcd_panel_wvga = LCD_NONE;
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
	/* DSI Bit Clock at 490 MHz, 2 lane, RGB888 */ 
	/* regulator */ 
	{0x03, 0x01, 0x01, 0x00}, 
	/* timing */ 
	{0x88, 0x32, 0x14, 0x00, 0x44, 0x4f, 0x18, 0x35, 
	0x17, 0x3, 0x04}, 
	/* phy ctrl */ 
	{0x7f, 0x00, 0x00, 0x00}, 
	/* strength */ 
	{0xbb, 0x02, 0x06, 0x00}, 
	/* pll control */ 
	{0x01, 0xE3, 0x31, 0xd2, 0x00, 0x40, 0x37, 0x62, 
	0x01, 0x0f, 0x07, 
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0}, 
};
static struct dsi_buf otm8018b_tx_buf;
static struct sequence * otm8018b_lcd_init_table_debug = NULL;

#if 0
static const struct sequence otm8018b_wvga_standby_enter_table[]= 
{
	{0x00028,MIPI_DCS_COMMAND,0},
    {0x00010,MIPI_DCS_COMMAND,20},
    {0x00010,MIPI_TYPE_END,150},
};
static const struct sequence otm8018b_wvga_standby_exit_table[]= 
{
	{0x00011,MIPI_DCS_COMMAND,0},
    {0x00029,MIPI_DCS_COMMAND,150},
	{0x00029,MIPI_TYPE_END,20},
};
#endif

static struct sequence otm8018b_cabc_enable_table[] =
{	
	{0x00051,MIPI_DCS_COMMAND,0},
	{0x000ff,TYPE_PARAMETER,0},
	
	{0x00029,MIPI_TYPE_END,0},
};

/* CHIMEI OTM8018B LCD init code */
static struct sequence otm8018b_chimei_lcd_init_table[] =
{
	{0x00051,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0}, 

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

	{0x00011,MIPI_DCS_COMMAND,0},

    {0x00000,MIPI_DCS_COMMAND,120},
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
	{0x000B5,TYPE_PARAMETER,0},
	{0x000C0,MIPI_GEN_COMMAND,0},
	{0x00008,TYPE_PARAMETER,0},

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
	{0x000AF,TYPE_PARAMETER,0},
	{0x000AF,TYPE_PARAMETER,0},

    {0x00000,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000D9,MIPI_GEN_COMMAND,0},
	{0x00085,TYPE_PARAMETER,0},

    {0x00000,MIPI_DCS_COMMAND,0},
	{0x00081,TYPE_PARAMETER,0},
	{0x000C1,MIPI_GEN_COMMAND,0},
	{0x00066,TYPE_PARAMETER,0},

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

	/* set LCD_backlight PWM frequency 22.786kHz and solve the bug of LCD display random image when Wake-up */	
	{0x00000, MIPI_DCS_COMMAND,0},		/* set address mode */
	{0x000B0, TYPE_PARAMETER,0},		/* set address 0xC6B0 */		
	{0x000C6, MIPI_GEN_COMMAND,0},		/* the register set LCD_backlight PWM frequency*/ 
	{0x00003, TYPE_PARAMETER,0},
	{0x00005, TYPE_PARAMETER,0},
	{0x00000, TYPE_PARAMETER,0},
	{0x0005F, TYPE_PARAMETER,0},
	{0x00012, TYPE_PARAMETER,0},
	{0x00000, TYPE_PARAMETER,0},

	{0x00000,MIPI_DCS_COMMAND,0},		/* set address mode */
	{0x000C0,TYPE_PARAMETER,0},		/* set address 0xB3C0 */
	{0x000B3,MIPI_GEN_COMMAND,0},
	{0x00009,TYPE_PARAMETER,0},		
	{0x00010,TYPE_PARAMETER,0},		/* turn on SRAM power during sleep in mode when IM = X101 */

    {0x00000,MIPI_DCS_COMMAND,0},
	{0x00090,TYPE_PARAMETER,0},
	{0x000C0,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},

    {0x00000,MIPI_DCS_COMMAND,0},
	{0x000A6,TYPE_PARAMETER,0},
	{0x000C1,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},

    {0x00000,MIPI_DCS_COMMAND,0},
	{0x00080,TYPE_PARAMETER,0},
	{0x000CE,MIPI_GEN_COMMAND,0},
	{0x00085,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00084,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},

    {0x00000,MIPI_DCS_COMMAND,0},
	{0x00090,TYPE_PARAMETER,0},
	{0x000CE,MIPI_GEN_COMMAND,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x00026,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00033,TYPE_PARAMETER,0},
	{0x00027,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},

    {0x00000,MIPI_DCS_COMMAND,0},
	{0x000A0,TYPE_PARAMETER,0},
	{0x000CE,MIPI_GEN_COMMAND,0},
	{0x00038,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00020,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00038,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00021,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},

    {0x00000,MIPI_DCS_COMMAND,0},
	{0x000B0,TYPE_PARAMETER,0},
	{0x000CE,MIPI_GEN_COMMAND,0},
	{0x00038,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00022,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00038,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00023,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},

    {0x00000,MIPI_DCS_COMMAND,0},
	{0x000C0,TYPE_PARAMETER,0},
	{0x000CE,MIPI_GEN_COMMAND,0},
	{0x00030,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00024,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00030,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00025,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},

    {0x00000,MIPI_DCS_COMMAND,0},
	{0x000D0,TYPE_PARAMETER,0},
	{0x000CE,MIPI_GEN_COMMAND,0},
	{0x00030,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00026,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00030,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00027,TYPE_PARAMETER,0},
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
	{0x0000D,TYPE_PARAMETER,0},
	{0x0000F,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x0000B,TYPE_PARAMETER,0},
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

    {0x00000,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E1,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00007,TYPE_PARAMETER,0},
	{0x0000F,TYPE_PARAMETER,0},
	{0x00011,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x00005,TYPE_PARAMETER,0},
	{0x00006,TYPE_PARAMETER,0},
	{0x00008,TYPE_PARAMETER,0},
	{0x0000D,TYPE_PARAMETER,0},
	{0x00006,TYPE_PARAMETER,0},
	{0x0001D,TYPE_PARAMETER,0},
	{0x00017,TYPE_PARAMETER,0},
	{0x0000D,TYPE_PARAMETER,0},
	{0x00006,TYPE_PARAMETER,0},

    {0x00000,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E2,MIPI_GEN_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00007,TYPE_PARAMETER,0},
	{0x0000F,TYPE_PARAMETER,0},
	{0x00011,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x0000C,TYPE_PARAMETER,0},
	{0x0000E,TYPE_PARAMETER,0},
	{0x0000F,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00006,TYPE_PARAMETER,0},
	{0x0000F,TYPE_PARAMETER,0},
	{0x0000C,TYPE_PARAMETER,0},
	{0x00013,TYPE_PARAMETER,0},
	{0x0000D,TYPE_PARAMETER,0},
	{0x00009,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},

    {0x00000,MIPI_DCS_COMMAND,0}, 
	{0x00000,TYPE_PARAMETER,0}, 
	{0x000E5,MIPI_GEN_COMMAND,0}, 
	{0x00000,TYPE_PARAMETER,0}, 
	{0x0000B,TYPE_PARAMETER,0}, 
	{0x00011,TYPE_PARAMETER,0}, 
	{0x0000F,TYPE_PARAMETER,0}, 
	{0x00007,TYPE_PARAMETER,0}, 
	{0x0000D,TYPE_PARAMETER,0}, 
	{0x00006,TYPE_PARAMETER,0}, 
	{0x00003,TYPE_PARAMETER,0}, 
	{0x00009,TYPE_PARAMETER,0}, 
	{0x0000B,TYPE_PARAMETER,0}, 
	{0x0000E,TYPE_PARAMETER,0}, 
	{0x00002,TYPE_PARAMETER,0}, 
	{0x00010,TYPE_PARAMETER,0}, 
	{0x0000E,TYPE_PARAMETER,0}, 
	{0x00007,TYPE_PARAMETER,0}, 
	{0x00006,TYPE_PARAMETER,0}, 

    {0x00000,MIPI_DCS_COMMAND,0}, 
	{0x00000,TYPE_PARAMETER,0}, 
	{0x000E6,MIPI_GEN_COMMAND,0}, 
	{0x00000,TYPE_PARAMETER,0}, 
	{0x0000D,TYPE_PARAMETER,0}, 
	{0x00011,TYPE_PARAMETER,0}, 
	{0x0000D,TYPE_PARAMETER,0}, 
	{0x00006,TYPE_PARAMETER,0}, 
	{0x0000D,TYPE_PARAMETER,0}, 
	{0x0000B,TYPE_PARAMETER,0}, 
	{0x0000B,TYPE_PARAMETER,0}, 
	{0x00004,TYPE_PARAMETER,0}, 
	{0x00009,TYPE_PARAMETER,0}, 
	{0x0000D,TYPE_PARAMETER,0}, 
	{0x00000,TYPE_PARAMETER,0}, 
	{0x00007,TYPE_PARAMETER,0}, 
	{0x00009,TYPE_PARAMETER,0}, 
	{0x00003,TYPE_PARAMETER,0}, 
	{0x00001,TYPE_PARAMETER,0}, 

	{0x00053,MIPI_DCS_COMMAND,0}, // ctrldisplay1
	{0x00024,TYPE_PARAMETER,0}, 

	{0x00055,MIPI_DCS_COMMAND,0}, // ctrldisplay2
	{0x00001,TYPE_PARAMETER,0},

	{0x00026,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},

	{0x0002B,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00056,TYPE_PARAMETER,0},

	{0x00029,MIPI_DCS_COMMAND,0},
	{0x0002c,MIPI_DCS_COMMAND,20},
	{0x00000,MIPI_TYPE_END,10}, //end flag
};

/*setting for support continuous splash */
/*lcd resume function*/
static int mipi_otm8018b_lcd_on(struct platform_device *pdev)
{
	boolean para_debug_flag = FALSE;
	uint32 para_num = 0;
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);
	
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL; 

	para_debug_flag = lcd_debug_malloc_get_para( "otm8018b_lcd_init_table_debug", 
			(void**)&otm8018b_lcd_init_table_debug,&para_num);

	lcd_reset();
	mipi_set_tx_power_mode(1);//Low power mode 
	
	if( (TRUE == para_debug_flag) && (NULL != otm8018b_lcd_init_table_debug))
	{
		process_mipi_table(mfd,&otm8018b_tx_buf,otm8018b_lcd_init_table_debug,
			para_num, lcd_panel_wvga);
	}
	else
	{
		process_mipi_table(mfd,&otm8018b_tx_buf,(struct sequence*)&otm8018b_chimei_lcd_init_table,
			ARRAY_SIZE(otm8018b_chimei_lcd_init_table), lcd_panel_wvga);
		/*delete some lines */
	}
	mipi_set_tx_power_mode(0);//High speed mode 
	
	if((TRUE == para_debug_flag)&&(NULL != otm8018b_lcd_init_table_debug))
	{
		lcd_debug_free_para((void *)otm8018b_lcd_init_table_debug);
	}
	
	if (!mfd->cont_splash_done) 
	{
		mfd->cont_splash_done = 1;
		otm8018b_cabc_enable_table[1].reg = 0x00064;
		process_mipi_table(mfd,&otm8018b_tx_buf,(struct sequence*)&otm8018b_cabc_enable_table,
			ARRAY_SIZE(otm8018b_cabc_enable_table), lcd_panel_wvga);
	}
	
	LCD_DEBUG("leave mipi_otm8018b_lcd_on \n");
	return 0;
}
/*lcd suspend function*/
static int mipi_otm8018b_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
#if 0
	process_mipi_table(mfd,&otm8018b_tx_buf,(struct sequence*)&otm8018b_wvga_standby_enter_table,
		 ARRAY_SIZE(otm8018b_wvga_standby_enter_table), lcd_panel_wvga);
#endif
	lcd_reset();
	LCD_DEBUG("leave mipi_otm8018b_lcd_off \n");
	return 0;
}

#ifdef CONFIG_FB_AUTO_CABC
static struct sequence otm8018b_wvga_auto_cabc_set_table[] =
{	
	{0x00053,MIPI_DCS_COMMAND,0},
	{0x00024,TYPE_PARAMETER,0},

	{0x00055,MIPI_DCS_COMMAND,0}, 
	{0x00001,TYPE_PARAMETER,0},

	{0xFFFFF,MIPI_TYPE_END,0}, /* the end flag,it don't sent to driver IC */
};

/***************************************************************
Function: otm8018b_wvga_config_auto_cabc
Description: Set CABC configuration
Parameters:
	struct msmfb_cabc_config cabc_cfg: CABC configuration struct
Return:
	0: success
***************************************************************/
static int otm8018b_wvga_config_auto_cabc(struct msmfb_cabc_config cabc_cfg, struct msm_fb_data_type *mfd)
{
	int ret = 0;

	switch(cabc_cfg.mode)
	{
		case CABC_MODE_UI:
			otm8018b_wvga_auto_cabc_set_table[1].reg=0x00024;
			otm8018b_wvga_auto_cabc_set_table[3].reg=0x00001;
			break;
		case CABC_MODE_MOVING:
		case CABC_MODE_STILL:
			otm8018b_wvga_auto_cabc_set_table[1].reg=0x0002C;
			otm8018b_wvga_auto_cabc_set_table[3].reg=0x00003;
			break;
		default:
			LCD_DEBUG("%s: invalid cabc mode: %d\n", __func__, cabc_cfg.mode);
			ret = -EINVAL;
			break;
	}
	if(likely(0 == ret))
	{
		process_mipi_table(mfd,&otm8018b_tx_buf,(struct sequence*)&otm8018b_wvga_auto_cabc_set_table,
			ARRAY_SIZE(otm8018b_wvga_auto_cabc_set_table), lcd_panel_wvga);
	}

	LCD_DEBUG("%s: change cabc mode to %d\n",__func__,cabc_cfg.mode);
	return ret;
}
#endif /* CONFIG_FB_AUTO_CABC */

static int __devinit mipi_otm8018b_lcd_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);

	return 0;
}
	/*delete some lines */

void otm8018b_set_cabc_backlight(struct msm_fb_data_type *mfd,uint32 bl_level)
{	
	otm8018b_cabc_enable_table[1].reg = bl_level; // 1 will be changed if modify init code

	process_mipi_table(mfd,&otm8018b_tx_buf,(struct sequence*)&otm8018b_cabc_enable_table,
		 ARRAY_SIZE(otm8018b_cabc_enable_table), lcd_panel_wvga);
}

static struct platform_driver this_driver = {
	.probe  = mipi_otm8018b_lcd_probe,
	.driver = {
		.name   = LCD_DEVICE_NAME,
	},
};
static struct msm_fb_panel_data otm8018b_panel_data = {
	.on					= mipi_otm8018b_lcd_on,
	.off					= mipi_otm8018b_lcd_off,
	.set_backlight 		= pwm_set_backlight,
	/*add cabc control backlight*/
	.set_cabc_brightness 	= otm8018b_set_cabc_backlight,
	#ifdef CONFIG_FB_AUTO_CABC
	.config_cabc = otm8018b_wvga_config_auto_cabc,
	#endif
};
static struct platform_device this_device = {
	.name   = LCD_DEVICE_NAME,
	.id	= 0,
	.dev	= {
		.platform_data = &otm8018b_panel_data,
	}
};

static __init int mipi_video_otm8018b_wvga_init(void)
{
	int ret = 0;
	struct msm_panel_info *pinfo = NULL;
	
	lcd_panel_wvga = get_lcd_panel_type();
	if (MIPI_VIDEO_OTM8018B_CHIMEI_WVGA != lcd_panel_wvga)
	{
		return 0;
	}
	LCD_DEBUG("enter mipi_video_otm8018b_wvga_init \n");
	mipi_dsi_buf_alloc(&otm8018b_tx_buf, DSI_BUF_SIZE);

	ret = platform_driver_register(&this_driver);
	if (!ret)
	{
		pinfo = &otm8018b_panel_data.panel_info;
		pinfo->xres = 480;
		pinfo->yres = 800;
		pinfo->type = MIPI_VIDEO_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 24;		
		pinfo->lcdc.h_back_porch = 165;
		pinfo->lcdc.h_front_porch = 165;
		pinfo->lcdc.h_pulse_width = 8;
		pinfo->lcdc.v_back_porch = 20;
		pinfo->lcdc.v_front_porch = 20;
		pinfo->lcdc.v_pulse_width = 1;
		pinfo->lcdc.border_clr = 0;	/* blk */
		pinfo->lcdc.underflow_clr = 0xff;	/* blue */
		/* number of dot_clk cycles HSYNC active edge is
		delayed from VSYNC active edge */
		pinfo->lcdc.hsync_skew = 0;
		pinfo->bl_max = 255;
		pinfo->bl_min = 30;		
		pinfo->fb_num = 3;
		pinfo->clk_rate = 419000000;

		pinfo->mipi.mode = DSI_VIDEO_MODE;
		pinfo->mipi.pulse_mode_hsa_he = TRUE;
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
		pinfo->mipi.t_clk_post = 0x7f;
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

module_init(mipi_video_otm8018b_wvga_init);
