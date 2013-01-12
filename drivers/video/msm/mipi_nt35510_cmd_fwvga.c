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

#define LCD_DEVICE_NAME "mipi_cmd_nt35510_fwvga"

static lcd_panel_type lcd_panel_fwvga = LCD_NONE;

/* increase the DSI bit clock to 490 MHz */
/*mipi dsi register setting , help qualcomm to set.*/
static struct mipi_dsi_phy_ctrl dsi_cmd_mode_phy_db_nt35510_fwvga = 
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

static struct dsi_buf nt35510_fwvga_tx_buf;
static struct sequence * nt35510_fwvga_lcd_init_table_debug = NULL;

/*Read register in order to solve LCD can't enter into black panel in ESD testing */
static struct dsi_buf nt35510_fwvga_rx_buf;

static char register_id[2] = {0x0A, 0x00}; /* DTYPE_DCS_READ */

static struct dsi_cmd_desc nt35510_register_id_cmd = {
	DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(register_id), register_id};

static uint32 mipi_nt35510_read_register(struct msm_fb_data_type *mfd)
{
	struct dsi_buf *rp = NULL;
	struct dsi_buf *tp = NULL;
	struct dsi_cmd_desc *cmd = NULL;
	char *lp = NULL;

	tp = &nt35510_fwvga_tx_buf;
	rp = &nt35510_fwvga_rx_buf;
	cmd = &nt35510_register_id_cmd;
	mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 1);
	lp = (char *)rp->data;	
	pr_info("%s: register_id=%02x\n", __func__, *lp);
	
	return *lp;
}

/*LCD init code*/
static const struct sequence nt35510_fwvga_standby_enter_table[]= 
{
	{0x00028,MIPI_DCS_COMMAND,0}, //28h
	{0x00010,MIPI_DCS_COMMAND,20},
	{0x00029,MIPI_TYPE_END,120}, // add new command for 
};
static const struct sequence nt35510_fwvga_standby_exit_table[]= 
{
	{0x00011,MIPI_DCS_COMMAND,0}, //29h
	{0x00029,MIPI_DCS_COMMAND,120},
	{0x00029,MIPI_TYPE_END,20}, // add new command for 
};

#ifdef CONFIG_FB_DYNAMIC_GAMMA
static const struct sequence nt35510_fwvga_gamma_25[]= 
{	
	{0x000D1,MIPI_GEN_COMMAND,0},  
	{0x00000,TYPE_PARAMETER,0}, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0007F,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000AF,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000CC,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E2,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0000A,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00028,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00057,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0007B,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000b7,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000E3,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0002C,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00067,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0009B,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000D6,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000F7,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00025,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0007B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00097,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000B5,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000EA,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FD,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FE,TYPE_PARAMETER,0},

	{0x000D2,MIPI_GEN_COMMAND,0},  
	{0x00000,TYPE_PARAMETER,0}, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0007F,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000AF,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000CC,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E2,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0000A,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00028,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00057,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0007B,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000b7,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000E3,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0002C,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00067,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0009B,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000D6,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000F7,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00025,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0007B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00097,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000B5,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000EA,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FD,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FE,TYPE_PARAMETER,0},
	
	{0x000D3,MIPI_GEN_COMMAND,0},  
	{0x00000,TYPE_PARAMETER,0}, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0007F,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000AF,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000CC,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E2,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0000A,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00028,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00057,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0007B,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000b7,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000E3,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0002C,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00067,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0009B,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000D6,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000F7,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00025,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0007B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00097,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000B5,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000EA,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FD,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FE,TYPE_PARAMETER,0},

	{0x000D4,MIPI_GEN_COMMAND,0},  
	{0x00000,TYPE_PARAMETER,0}, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0007F,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000AF,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000CC,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E2,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0000A,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00028,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00057,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0007B,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000b7,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000E3,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0002C,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00067,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0009B,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000D6,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000F7,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00025,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0007B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00097,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000B5,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000EA,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FD,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FE,TYPE_PARAMETER,0},
	{0x000D5,MIPI_GEN_COMMAND,0},  
	{0x00000,TYPE_PARAMETER,0}, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0007F,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000AF,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000CC,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E2,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0000A,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00028,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00057,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0007B,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000b7,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000E3,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0002C,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00067,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0009B,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000D6,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000F7,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00025,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0007B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00097,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000B5,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000EA,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FD,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FE,TYPE_PARAMETER,0},

	{0x000D6,MIPI_GEN_COMMAND,0},  
	{0x00000,TYPE_PARAMETER,0}, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0007F,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000AF,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000CC,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000E2,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0000A,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00028,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00057,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0007B,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000b7,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000E3,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0002C,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00067,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0009B,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000D6,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000F7,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00025,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00044,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00065,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0007B,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00097,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000B5,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000EA,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FD,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FE,TYPE_PARAMETER,0},

	{0x00029,MIPI_TYPE_END,0},
};
	
static const struct sequence nt35510_fwvga_gamma_22[]= 
{
	{0x000D1,MIPI_GEN_COMMAND,0},  
	{0x00000,TYPE_PARAMETER,0}, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0007e,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000a0,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000c2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000cb,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000fd,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0001b,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00049,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0006b,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000a5,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000d1,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0001a,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00056,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00089,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000bd,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000e0,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0000d,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00028,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0004f,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00062,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0007e,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00093,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000a6,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000ef,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FE,TYPE_PARAMETER,0},

	{0x000D2,MIPI_GEN_COMMAND,0},  
	{0x00000,TYPE_PARAMETER,0}, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0007e,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000a0,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000c2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000cb,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000fd,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0001b,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00049,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0006b,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000a5,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000d1,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0001a,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00056,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00089,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000bd,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000e0,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0000d,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00028,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0004f,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00062,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0007e,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00093,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000a6,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000ef,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FE,TYPE_PARAMETER,0},

	{0x000D3,MIPI_GEN_COMMAND,0},  
	{0x00000,TYPE_PARAMETER,0}, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0007e,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000a0,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000c2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000cb,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000fd,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0001b,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00049,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0006b,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000a5,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000d1,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0001a,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00056,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00089,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000bd,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000e0,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0000d,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00028,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0004f,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00062,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0007e,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00093,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000a6,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000ef,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FE,TYPE_PARAMETER,0},

	{0x000D4,MIPI_GEN_COMMAND,0},  
	{0x00000,TYPE_PARAMETER,0}, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0007e,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000a0,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000c2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000cb,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000fd,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0001b,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00049,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0006b,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000a5,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000d1,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0001a,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00056,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00089,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000bd,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000e0,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0000d,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00028,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0004f,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00062,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0007e,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00093,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000a6,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000ef,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FE,TYPE_PARAMETER,0},

	{0x000D5,MIPI_GEN_COMMAND,0},  
	{0x00000,TYPE_PARAMETER,0}, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0007e,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000a0,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000c2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000cb,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000fd,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0001b,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00049,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0006b,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000a5,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000d1,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0001a,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00056,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00089,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000bd,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000e0,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0000d,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00028,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0004f,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00062,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0007e,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00093,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000a6,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000ef,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FE,TYPE_PARAMETER,0},

	{0x000D6,MIPI_GEN_COMMAND,0},  
	{0x00000,TYPE_PARAMETER,0}, 
	{0x00001,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x0007e,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000a0,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000c2,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000cb,TYPE_PARAMETER,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x000fd,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0001b,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x00049,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x0006b,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000a5,TYPE_PARAMETER,0},
	{0x00001,TYPE_PARAMETER,0},
	{0x000d1,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x0001a,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00054,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00056,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x00089,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000bd,TYPE_PARAMETER,0},
	{0x00002,TYPE_PARAMETER,0},
	{0x000e0,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0000d,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00028,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0004f,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00062,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x0007e,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x00093,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000a6,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000ef,TYPE_PARAMETER,0},
	{0x00003,TYPE_PARAMETER,0},
	{0x000FE,TYPE_PARAMETER,0},

	{0x00029,MIPI_TYPE_END,0},
};
	
int mipi_nt35510_fwvga_set_dynamic_gamma(enum danymic_gamma_mode gamma_mode,struct msm_fb_data_type *mfd)
{
	int ret = 0;

	
	switch(gamma_mode)
	{
		case GAMMA25:
			process_mipi_table(mfd,&nt35510_fwvga_tx_buf,(struct sequence*)&nt35510_fwvga_gamma_25,
						ARRAY_SIZE(nt35510_fwvga_gamma_25), lcd_panel_fwvga);
			break ;
		case GAMMA22:
			process_mipi_table(mfd,&nt35510_fwvga_tx_buf,(struct sequence*)&nt35510_fwvga_gamma_22,
						ARRAY_SIZE(nt35510_fwvga_gamma_22), lcd_panel_fwvga);
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
static int mipi_nt35510_fwvga_lcd_on(struct platform_device *pdev)
{
	boolean para_debug_flag = FALSE;
	uint32 para_num = 0;
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL; 

	para_debug_flag = lcd_debug_malloc_get_para( "nt35510_fwvga_lcd_init_table_debug", 
			(void**)&nt35510_fwvga_lcd_init_table_debug,&para_num);

	if( (TRUE == para_debug_flag) && (NULL != nt35510_fwvga_lcd_init_table_debug))
	{
		process_mipi_table(mfd,&nt35510_fwvga_tx_buf,nt35510_fwvga_lcd_init_table_debug,
		     para_num, lcd_panel_fwvga);
	}
	else
	{
		mipi_set_tx_power_mode(1);
		process_mipi_table(mfd,&nt35510_fwvga_tx_buf,(struct sequence*)&nt35510_fwvga_standby_exit_table,
		 	ARRAY_SIZE(nt35510_fwvga_standby_exit_table), lcd_panel_fwvga);
		mipi_set_tx_power_mode(0);
	}

	if((TRUE == para_debug_flag)&&(NULL != nt35510_fwvga_lcd_init_table_debug))
	{
		lcd_debug_free_para((void *)nt35510_fwvga_lcd_init_table_debug);
	}
	
	pr_info("leave mipi_nt35510_fwvga_lcd_on \n");
	return 0;
}

/*lcd suspend function*/
static int mipi_nt35510_fwvga_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	
	/* clean up ack_err_status */
	mipi_nt35510_read_register(mfd);

	process_mipi_table(mfd,&nt35510_fwvga_tx_buf,(struct sequence*)&nt35510_fwvga_standby_enter_table,
		 ARRAY_SIZE(nt35510_fwvga_standby_enter_table), lcd_panel_fwvga);
	pr_info("leave mipi_nt35510_fwvga_lcd_off \n");
	return 0;
}

#ifdef CONFIG_FB_AUTO_CABC
static struct sequence nt35510_fwvga_auto_cabc_set_table[] =
{
	 
	/* Delete two lines,These lines are set lcd backlight max brightness*/
	/* change cabc mode from video to UI */
	{0x00053,MIPI_DCS_COMMAND,0},
	{0x00024,TYPE_PARAMETER,0},//open cabc
	{0x00055,MIPI_DCS_COMMAND,0},
	{0x00000,TYPE_PARAMETER,0},
	{0x00029,MIPI_TYPE_END,0},
};
/***************************************************************
Function: nt35510_config_cabc
Description: Set CABC configuration
Parameters:
    struct msmfb_cabc_config cabc_cfg: CABC configuration struct
Return:
    0: success
***************************************************************/
static int nt35510_fwvga_config_auto_cabc(struct msmfb_cabc_config cabc_cfg,struct msm_fb_data_type *mfd)
{
	int ret = 0;

	switch(cabc_cfg.mode)
	{
		case CABC_MODE_UI:
			/* the value of cabc register should be 24 in UI mode */
			nt35510_fwvga_auto_cabc_set_table[1].reg=0x00024;
			nt35510_fwvga_auto_cabc_set_table[3].reg=0x00001;
			break;
		case CABC_MODE_MOVING:
		case CABC_MODE_STILL:
			/* the value of cabc register should be 2C in moving mode and still mode */
			nt35510_fwvga_auto_cabc_set_table[1].reg=0x0002C;
			nt35510_fwvga_auto_cabc_set_table[3].reg=0x00003;
			break;
		default:
			LCD_DEBUG("%s: invalid cabc mode: %d\n", __func__, cabc_cfg.mode);
	        ret = -EINVAL;
			break;
	}
	if(likely(0 == ret))
	{
		process_mipi_table(mfd,&nt35510_fwvga_tx_buf,(struct sequence*)&nt35510_fwvga_auto_cabc_set_table,
			 ARRAY_SIZE(nt35510_fwvga_auto_cabc_set_table), lcd_panel_fwvga);
	}

	LCD_DEBUG("%s: change cabc mode to %d\n",__func__,cabc_cfg.mode);
	return ret;
}
#endif // CONFIG_FB_AUTO_CABC



static int __devinit mipi_nt35510_fwvga_lcd_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);
	return 0;
}

static struct sequence nt35510_fwvga_write_cabc_brightness_table[]= 
{
	{0x00051,MIPI_DCS_COMMAND,0},
	{0x000FF,TYPE_PARAMETER,0},//max brightness
	{0x00029,MIPI_TYPE_END,0}
};
/*lcd cabc control function*/
void nt35510_fwvga_set_cabc_backlight(struct msm_fb_data_type *mfd,uint32 bl_level)
{
    /* clean up ack_err_status */
	mipi_nt35510_read_register(mfd);
	nt35510_fwvga_write_cabc_brightness_table[1].reg = bl_level; 

	process_mipi_table(mfd,&nt35510_fwvga_tx_buf,(struct sequence*)&nt35510_fwvga_write_cabc_brightness_table,
		 ARRAY_SIZE(nt35510_fwvga_write_cabc_brightness_table), lcd_panel_fwvga);
}

static struct platform_driver this_driver = {
	.probe  = mipi_nt35510_fwvga_lcd_probe,
	.driver = {
		.name   = LCD_DEVICE_NAME,
	},
};
static struct msm_fb_panel_data nt35510_fwvga_panel_data = {
	.on		= mipi_nt35510_fwvga_lcd_on,
	.off	= mipi_nt35510_fwvga_lcd_off,
	.set_backlight = pwm_set_backlight,
	/*add cabc control backlight*/
	.set_cabc_brightness = nt35510_fwvga_set_cabc_backlight,	
#ifdef CONFIG_FB_AUTO_CABC
		.config_cabc = nt35510_fwvga_config_auto_cabc,
#endif
#ifdef CONFIG_FB_DYNAMIC_GAMMA
		.set_dynamic_gamma = mipi_nt35510_fwvga_set_dynamic_gamma,
#endif
};
static struct platform_device this_device = {
	.name   = LCD_DEVICE_NAME,
	.id	= 0,
	.dev	= {
		.platform_data = &nt35510_fwvga_panel_data,
	}
};

static int __init mipi_cmd_nt35510_fwvga_init(void)
{
	int ret = 0;
	struct msm_panel_info *pinfo = NULL;
	lcd_panel_fwvga = get_lcd_panel_type();
	if (MIPI_CMD_NT35510_BOE_FWVGA != lcd_panel_fwvga )
	{
		return 0;
	}
	LCD_DEBUG("enter mipi_cmd_nt35510_fwvga_init \n");
	mipi_dsi_buf_alloc(&nt35510_fwvga_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&nt35510_fwvga_rx_buf, DSI_BUF_SIZE);

	ret = platform_driver_register(&this_driver);
	if (!ret)
	{
		pinfo = &nt35510_fwvga_panel_data.panel_info;
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
		pinfo->mipi.t_clk_post = 0xB0;// min 60 + 128*UI
		pinfo->mipi.t_clk_pre = 0x2f;// min 8*UI
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
		pinfo->mipi.dsi_phy_db = &dsi_cmd_mode_phy_db_nt35510_fwvga;
		pinfo->mipi.tx_eot_append = 0x01;
		pinfo->mipi.rx_eot_ignore = 0;
		pinfo->mipi.dlane_swap = 0x1;

		ret = platform_device_register(&this_device);
		if (ret)
			LCD_DEBUG("%s: failed to register device!\n", __func__);
	}

	return ret;
}

module_init(mipi_cmd_nt35510_fwvga_init);
