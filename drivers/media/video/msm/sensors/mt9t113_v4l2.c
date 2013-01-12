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
 *
 */

#include "msm_sensor.h"
#include "msm.h"

#define SENSOR_NAME "mt9t113"
#define PLATFORM_DRIVER_NAME "msm_camera_mt9t113"
#define mt9t113_obj mt9t113_##obj

DEFINE_MUTEX(mt9t113_mut);
static struct msm_sensor_ctrl_t mt9t113_s_ctrl;

#if 0
 #undef CDBG
 #define CDBG(fmt, args...) printk(KERN_INFO "mt9t113: " fmt, ## args)
#endif

static int mt9t113_csi_config=0;

#define REGISTER_WRITE_WAIT_FW			0
#define REGISTER_WRITE_WAIT_PATCH		1
#define REGISTER_WRITE_WAIT_PREVIEW		10
#define REGISTER_WRITE_WAIT_SNAPSHOT	11

#define REGISTER_WRITE_RETRY		2

enum mt9t113_setting_t
{
	RES_CAPTURE,
	RES_PREVIEW,
};

static int32_t mt9t113_wait(struct msm_sensor_ctrl_t *s_ctrl,  int time);

static struct msm_camera_i2c_reg_conf mt9t113_prev_settings[] = {
	//================================================================================================
	// APPLY PREVIEW CONFIGURATION & RUN PREVIEW
	//================================================================================================
    {0x098E, 0xEC09},   // MCU_ADDRESS [PRI_B_NUM_OF_FRAMES_RUN]
    {0x0990, 0x0005},   // MCU_DATA_0
    {0x098E, 0x8400},   // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0001},   // MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf mt9t113_snap_settings[] = {
    {0x098E, 0xEC09},   // MCU_ADDRESS [PRI_B_NUM_OF_FRAMES_RUN]
    {0x0990, 0x00FF},   // MCU_DATA_0
    {0x098E, 0x8400},   // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0002},   // MCU_DATA_0
    {0x3400, 0x7A28},   // MIPI_CONTROL
};

static struct msm_camera_i2c_reg_conf mt9t113_recommend_settings_1[] = {
    {0x0018, 0x4129}, 	// STANDBY_CONTROL_AND_STATUS
    {0x0018, 0x4029}, 	// STANDBY_CONTROL_AND_STATUS
    {0x0010, 0x0318}, 	// PLL_DIVIDERS
    {0x0012, 0x0070}, 	// PLL_P_DIVIDERS
    {0x002A, 0x76A9}, 	// PLL_P4_P5_P6_DIVIDERS
    {0x0028, 0x0000},
    // enable mipi
    {0x001A, 0x0014}, 	// RESET_AND_MISC_CONTROL 

    {0x0018, 0x402E}, 	// STANDBY_CONTROL_AND_STATUS  
};
static struct msm_camera_i2c_reg_conf mt9t113_recommend_settings_2[] = {
    {0x0022, 0x0140},   // VDD_DIS_COUNTER
    {0x001E, 0x0701},   // PAD_SLEW_PAD_CONFIG
    {0x0112, 0x0012},   // RX_FIFO_CONTROL
    {0x3B84, 0x0062},   // I2C_MASTER_FREQUENCY_DIVIDER
    {0x3400, 0x7A28},   // MIPI_CONTROL
    {0x301a, 0x10E8},     // for register 6, clear the first 4 bits
    
    {0x098E, 0xA805},   // MCU_ADDRESS
    {0x0990, 0x0010},   // MCU_DATA_0
    {0x098E, 0x2803},   // MCU_ADDRESS
    {0x0990, 0x0000},   // MCU_DATA_0
    {0x098E, 0x4800},   // MCU_ADDRESS
    {0x0990, 0x0010},   // MCU_DATA_0
    {0x098E, 0x4802},   // MCU_ADDRESS
    {0x0990, 0x0010},   // MCU_DATA_0
    {0x098E, 0x4804},   // MCU_ADDRESS
    {0x0990, 0x062D},   // MCU_DATA_0
    {0x098E, 0x4806},   // MCU_ADDRESS
    {0x0990, 0x082D},   // MCU_DATA_0
    {0x098E, 0x4808},   // MCU_ADDRESS
    {0x0990, 0x0359},   // MCU_DATA_0
    {0x098E, 0x480A},   // MCU_ADDRESS
    {0x0990, 0x0AB7},   // MCU_DATA_0
    {0x098E, 0x480C},   // MCU_ADDRESS
    {0x0990, 0x0399},   // MCU_DATA_0
    {0x098E, 0x480E},   // MCU_ADDRESS
    {0x0990, 0x0111},   // MCU_DATA_0
    {0x098E, 0x4810},   // MCU_ADDRESS
    {0x0990, 0x046C},   // MCU_DATA_0
    {0x098E, 0x4812},   // MCU_ADDRESS
    {0x0990, 0x0510},   // MCU_DATA_0
    {0x098E, 0x4814},   // MCU_ADDRESS
    {0x0990, 0x01BA},   // MCU_DATA_0
    {0x098E, 0x482D},   // MCU_ADDRESS
    {0x0990, 0x0018},   // MCU_DATA_0
    {0x098E, 0x482F},   // MCU_ADDRESS
    {0x0990, 0x0018},   // MCU_DATA_0
    {0x098E, 0x4831},   // MCU_ADDRESS
    {0x0990, 0x0627},   // MCU_DATA_0
    {0x098E, 0x4833},   // MCU_ADDRESS
    {0x0990, 0x0827},   // MCU_DATA_0
    {0x098E, 0x4835},   // MCU_ADDRESS
    {0x0990, 0x065D},   // MCU_DATA_0
    {0x098E, 0x4837},   // MCU_ADDRESS
    {0x0990, 0x0E8A},   // MCU_DATA_0
    {0x098E, 0x4839},   // MCU_ADDRESS
    {0x0990, 0x019F},   // MCU_DATA_0
    {0x098E, 0x483B},   // MCU_ADDRESS
    {0x0990, 0x0111},   // MCU_DATA_0
    {0x098E, 0x483D},   // MCU_ADDRESS
    {0x0990, 0x0024},   // MCU_DATA_0
    {0x098E, 0x483F},   // MCU_ADDRESS
    {0x0990, 0x0266},   // MCU_DATA_0
    {0x098E, 0x4841},   // MCU_ADDRESS
    {0x0990, 0x010A},   // MCU_DATA_0
    {0x098E, 0xB81A},   // MCU_ADDRESS
    {0x0990, 0x0005},   // MCU_DATA_0
    {0x098E, 0x481A},   // MCU_ADDRESS
    {0x0990, 0x00D2},   // MCU_DATA_0
    {0x098E, 0x481C},   // MCU_ADDRESS
    {0x0990, 0x00AF},   // MCU_DATA_0
    {0x098E, 0xC81E},   // MCU_ADDRESS
    {0x0990, 0x0022},   // MCU_DATA_0
    {0x098E, 0xC81F},   // MCU_ADDRESS
    {0x0990, 0x0024},   // MCU_DATA_0
    {0x098E, 0xC820},   // MCU_ADDRESS
    {0x0990, 0x0029},   // MCU_DATA_0
    {0x098E, 0xC821},   // MCU_ADDRESS
    {0x0990, 0x002B},   // MCU_DATA_0
    {0x098E, 0x4847},   // MCU_ADDRESS
    {0x0990, 0x009B},   // MCU_DATA_0
    {0x098E, 0x4849},   // MCU_ADDRESS
    {0x0990, 0x0081},   // MCU_DATA_0
    {0x098E, 0xC84B},   // MCU_ADDRESS
    {0x0990, 0x0018},   // MCU_DATA_0
    {0x098E, 0xC84C},   // MCU_ADDRESS
    {0x0990, 0x001A},   // MCU_DATA_0
    {0x098E, 0xC84D},   // MCU_ADDRESS
    {0x0990, 0x001E},   // MCU_DATA_0
    {0x098E, 0xC84E},   // MCU_ADDRESS
    {0x0990, 0x0020},   // MCU_DATA_0
    {0x098E, 0x6800},   // MCU_ADDRESS
    {0x0990, 0x0400},   // MCU_DATA_0
    {0x098E, 0x6802},   // MCU_ADDRESS
    {0x0990, 0x0300},   // MCU_DATA_0
    {0x098E, 0x6804},   // MCU_ADDRESS
    {0x0990, 0x0400},   // MCU_DATA_0
    {0x098E, 0x6806},   // MCU_ADDRESS
    {0x0990, 0x0300},   // MCU_DATA_0
    {0x098E, 0x6C00},   // MCU_ADDRESS
    {0x0990, 0x0800},   // MCU_DATA_0
    {0x098E, 0x6C02},   // MCU_ADDRESS
    {0x0990, 0x0600},   // MCU_DATA_0
    {0x098E, 0x6C04},   // MCU_ADDRESS
    {0x0990, 0x0800},   // MCU_DATA_0
    {0x098E, 0x6C06},   // MCU_ADDRESS
    {0x0990, 0x0600},   // MCU_DATA_0
    {0x098E, 0x6CA6},   // MCU_ADDRESS
    {0x0990, 0x082D},   // MCU_DATA_0
    {0x098E, 0xECA5},   // MCU_ADDRESS
    {0x0990, 0x0000},   // MCU_DATA_0
    {0x098E, 0x6C94},   // MCU_ADDRESS
    {0x0990, 0x0C34},   // MCU_DATA_0
    {0x3C86, 0x00E1},   // OB_PCLK1_CONFIG
    {0x3C20, 0x0000},   // TX_SS_CONTROL
    {0x098E, 0x6820},   // MCU_ADDRESS
    {0x0990, 0x0007},   // MCU_DATA_0
    {0x098E, 0x6822},   // MCU_ADDRESS
    {0x0990, 0x0064},   // MCU_DATA_0
    {0x098E, 0x6824},   // MCU_ADDRESS
    {0x0990, 0x0080},   // MCU_DATA_0
    {0x098E, 0xE826},   // MCU_ADDRESS
    {0x0990, 0x003c},   // MCU_DATA_0
    {0x098E, 0x6829},   // MCU_ADDRESS
    {0x0990, 0x0080},   // MCU_DATA_0
    {0x098E, 0x682B},   // MCU_ADDRESS
    {0x0990, 0x0080},   // MCU_DATA_0
    {0x098E, 0x682D},   // MCU_ADDRESS
    {0x0990, 0x0038},   // MCU_DATA_0
    {0x098E, 0x486F},   // MCU_ADDRESS
    {0x0990, 0x0180},   // MCU_DATA_0
    {0x098E, 0x4871},   // MCU_ADDRESS
    {0x0990, 0x0038},   // MCU_DATA_0
    {0x098E, 0x682F},   // MCU_ADDRESS
    {0x0990, 0x0180},   // MCU_DATA_0
    {0x098E, 0x6815},   // MCU_ADDRESS
    {0x0990, 0x0011},   // MCU_DATA_0
    {0x098E, 0x6817},   // MCU_ADDRESS
    {0x0990, 0x0014},   // MCU_DATA_0
    {0x098E, 0xA005},   // MCU_ADDRESS [FD_FDPERIOD_SELECT]
    {0x0990, 0x0001},   // MCU_DATA_0
    {0x098E, 0x680F},   // MCU_ADDRESS
    {0x0990, 0x0003},   // MCU_DATA_0
    {0x098E, 0xA006},   // MCU_ADDRESS
    {0x0990, 0x0008},   // MCU_DATA_0
    {0x098E, 0xA007},   // MCU_ADDRESS
    {0x0990, 0x0003},   // MCU_DATA_0
    {0x098E, 0xA008},   // MCU_ADDRESS
    {0x0990, 0x0005},   // MCU_DATA_0
    {0x098E, 0xA00A},   // MCU_ADDRESS
    {0x0990, 0x0000},   // MCU_DATA_0

    {0x098E, 0x4873}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_0]
    {0x0990, 0x01EA}, 	// MCU_DATA_0
    {0x098E, 0x4889}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_0]
    {0x0990, 0x005F}, 	// MCU_DATA_0
    {0x098E, 0x489F}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_0]
    {0x0990, 0x0100}, 	// MCU_DATA_0
    {0x098E, 0x4875}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_1]
    {0x0990, 0xFF76}, 	// MCU_DATA_0
    {0x098E, 0x488B}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_1]
    {0x0990, 0xFF8A}, 	// MCU_DATA_0
    {0x098E, 0x48A1}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_1]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x4877}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_2]
    {0x0990, 0xFFC4}, 	// MCU_DATA_0
    {0x098E, 0x488D}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_2]
    {0x0990, 0x0018}, 	// MCU_DATA_0
    {0x098E, 0x48A3}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_2]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x4879}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_3]
    {0x0990, 0xFFA6}, 	// MCU_DATA_0
    {0x098E, 0x488F}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_3]
    {0x0990, 0x0015}, 	// MCU_DATA_0
    {0x098E, 0x48A5}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_3]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x487B}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_4]
    {0x0990, 0x01B1}, 	// MCU_DATA_0
    {0x098E, 0x4891}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_4]
    {0x0990, 0xFFEA}, 	// MCU_DATA_0
    {0x098E, 0x48A7}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_4]
    {0x0990, 0x0100}, 	// MCU_DATA_0
    {0x098E, 0x487D}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_5]
    {0x0990, 0xFFDF}, 	// MCU_DATA_0
    {0x098E, 0x4893}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_5]
    {0x0990, 0xFFF9},   	// MCU_DATA_0
    {0x098E, 0x48A9}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_5]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x487F}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_6]
    {0x0990, 0xFFE4}, 	// MCU_DATA_0
    {0x098E, 0x4895}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_6]
    {0x0990, 0xFFFD}, 	// MCU_DATA_0
    {0x098E, 0x48AB}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_6]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x4881}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_7]
    {0x0990, 0xFF05}, 	// MCU_DATA_0
    {0x098E, 0x4897}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_7]
    {0x0990, 0x0087}, 	// MCU_DATA_0
    {0x098E, 0x48AD}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_7]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x4883}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_8]
    {0x0990, 0x0258}, 	// MCU_DATA_0
    {0x098E, 0x4899}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_8]
    {0x0990, 0xFF66}, 	// MCU_DATA_0
    {0x098E, 0x48AF}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_8]
    {0x0990, 0x0100}, 	// MCU_DATA_0
    {0x098E, 0x4873}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_0]
    {0x0990, 0x01EA}, 	// MCU_DATA_0
    {0x098E, 0x4889}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_0]
    {0x0990, 0x005F}, 	// MCU_DATA_0
    {0x098E, 0x489F}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_0]
    {0x0990, 0x0100}, 	// MCU_DATA_0
    {0x098E, 0x4875}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_1]
    {0x0990, 0xFF76}, 	// MCU_DATA_0
    {0x098E, 0x488B}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_1]
    {0x0990, 0xFF8A}, 	// MCU_DATA_0
    {0x098E, 0x48A1}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_1]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x4877}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_2]
    {0x0990, 0xFFC4}, 	// MCU_DATA_0
    {0x098E, 0x488D}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_2]
    {0x0990, 0x0018}, 	// MCU_DATA_0
    {0x098E, 0x48A3}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_2]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x4879}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_3]
    {0x0990, 0xFFA6}, 	// MCU_DATA_0
    {0x098E, 0x488F}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_3]
    {0x0990, 0x0015}, 	// MCU_DATA_0
    {0x098E, 0x48A5}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_3]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x487B}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_4]
    {0x0990, 0x01B1}, 	// MCU_DATA_0
    {0x098E, 0x4891}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_4]
    {0x0990, 0xFFEA}, 	// MCU_DATA_0
    {0x098E, 0x48A7}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_4]
    {0x0990, 0x0100}, 	// MCU_DATA_0
    {0x098E, 0x487D}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_5]
    {0x0990, 0xFFDF}, 	// MCU_DATA_0
    {0x098E, 0x4893}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_5]
    {0x0990, 0xFFF9},  	// MCU_DATA_0
    {0x098E, 0x48A9}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_5]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x487F}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_6]
    {0x0990, 0xFFE4}, 	// MCU_DATA_0
    {0x098E, 0x4895}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_6]
    {0x0990, 0xFFFD}, 	// MCU_DATA_0
    {0x098E, 0x48AB}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_6]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x4881}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_7]
    {0x0990, 0xFF05}, 	// MCU_DATA_0
    {0x098E, 0x4897}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_7]
    {0x0990, 0x0087}, 	// MCU_DATA_0
    {0x098E, 0x48AD}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_7]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x4883}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_8]
    {0x0990, 0x0258}, 	// MCU_DATA_0
    {0x098E, 0x4899}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_8]
    {0x0990, 0xFF66}, 	// MCU_DATA_0
    {0x098E, 0x48AF}, 	// MCU_ADDRESS [CAM1_AWB_LL_CCM_8]
    {0x0990, 0x0100}, 	// MCU_DATA_0
    {0x098E, 0x4885},   // MCU_ADDRESS [CAM1_AWB_CCM_L_9]
    {0x0990, 0x0022},   // MCU_DATA_0
    {0x098E, 0x4887},   // MCU_ADDRESS [CAM1_AWB_CCM_L_10]
    {0x0990, 0x004E},   // MCU_DATA_0
    {0x098E, 0x489B},   // MCU_ADDRESS [CAM1_AWB_CCM_RL_9]
    {0x0990, 0x0016},   // MCU_DATA_0
    {0x098E, 0x489D},   // MCU_ADDRESS [CAM1_AWB_CCM_RL_10]
    {0x0990, 0xFFE1},   // MCU_DATA_0

    {0x098E, 0x48B8}, 	// MCU_ADDRESS [CAM1_AWB_X_SHIFT]
    {0x0990, 0x002D}, 	// MCU_DATA_0
    {0x098E, 0x48BA}, 	// MCU_ADDRESS [CAM1_AWB_Y_SHIFT]
    {0x0990, 0x0011}, 	// MCU_DATA_0
    {0x098E, 0x48BC}, 	// MCU_ADDRESS [CAM1_AWB_RECIP_XSCALE]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098E, 0x48BE}, 	// MCU_ADDRESS [CAM1_AWB_RECIP_YSCALE]
    {0x0990, 0x00AB}, 	// MCU_DATA_0
    {0x098E, 0x48C0}, 	// MCU_ADDRESS [CAM1_AWB_ROT_CENTER_X]
    {0x0990, 0x03F2}, 	// MCU_DATA_0
    {0x098E, 0x48C2}, 	// MCU_ADDRESS [CAM1_AWB_ROT_CENTER_Y]
    {0x0990, 0x03E2}, 	// MCU_DATA_0
    {0x098E, 0xC8C4}, 	// MCU_ADDRESS [CAM1_AWB_ROT_SIN]
    {0x0990, 0x0038}, 	// MCU_DATA_0
    {0x098E, 0xC8C5}, 	// MCU_ADDRESS [CAM1_AWB_ROT_COS]
    {0x0990, 0x001F}, 	// MCU_DATA_0
    {0x098E, 0x48C6}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x48C8}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_1]
    {0x0990, 0x0001}, 	// MCU_DATA_0
    {0x098E, 0x48CA}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_2]
    {0x0990, 0x1100}, 	// MCU_DATA_0
    {0x098E, 0x48CC}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_3]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x48CE}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_4]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x48D0}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_5]
    {0x0990, 0x0011}, 	// MCU_DATA_0
    {0x098E, 0x48D2}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_6]
    {0x0990, 0x1111}, 	// MCU_DATA_0
    {0x098E, 0x48D4}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_7]
    {0x0990, 0x1100}, 	// MCU_DATA_0
    {0x098E, 0x48D6}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_8]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0x48D8}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_9]
    {0x0990, 0x0011}, 	// MCU_DATA_0
    {0x098E, 0x48DA}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_10]
    {0x0990, 0x1111}, 	// MCU_DATA_0
    {0x098E, 0x48DC}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_11]
    {0x0990, 0x1110}, 	// MCU_DATA_0
    {0x098E, 0x48DE}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_12]
    {0x0990, 0x0111}, 	// MCU_DATA_0
    {0x098E, 0x48E0}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_13]
    {0x0990, 0x1111}, 	// MCU_DATA_0
    {0x098E, 0x48E2}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_14]
    {0x0990, 0x2122}, 	// MCU_DATA_0
    {0x098E, 0x48E4}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_15]
    {0x0990, 0x2110}, 	// MCU_DATA_0
    {0x098E, 0x48E6}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_16]
    {0x0990, 0x0123}, 	// MCU_DATA_0
    {0x098E, 0x48E8}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_17]
    {0x0990, 0x3332}, 	// MCU_DATA_0
    {0x098E, 0x48EA}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_18]
    {0x0990, 0x2123}, 	// MCU_DATA_0
    {0x098E, 0x48EC}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_19]
    {0x0990, 0x3321}, 	// MCU_DATA_0
    {0x098E, 0x48EE}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_20]
    {0x0990, 0x0134}, 	// MCU_DATA_0
    {0x098E, 0x48F0}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_21]
    {0x0990, 0x4443}, 	// MCU_DATA_0
    {0x098E, 0x48F2}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_22]
    {0x0990, 0x2112}, 	// MCU_DATA_0
    {0x098E, 0x48F4}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_23]
    {0x0990, 0x3321}, 	// MCU_DATA_0
    {0x098E, 0x48F6}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_24]
    {0x0990, 0x0123}, 	// MCU_DATA_0
    {0x098E, 0x48F8}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_25]
    {0x0990, 0x3432}, 	// MCU_DATA_0
    {0x098E, 0x48FA}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_26]
    {0x0990, 0x1111}, 	// MCU_DATA_0
    {0x098E, 0x48FC}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_27]
    {0x0990, 0x2221}, 	// MCU_DATA_0
    {0x098E, 0x48FE}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_28]
    {0x0990, 0x0111}, 	// MCU_DATA_0
    {0x098E, 0x4900}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_29]
    {0x0990, 0x1111}, 	// MCU_DATA_0
    {0x098E, 0x4902}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_30]
    {0x0990, 0x1111}, 	// MCU_DATA_0
    {0x098E, 0x4904}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_31]
    {0x0990, 0x1111}, 	// MCU_DATA_0
    {0x098E, 0xE851}, 	// MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_L]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098E, 0xE852}, 	// MCU_ADDRESS [PRI_A_CONFIG_AWB_K_G_L]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098E, 0xE853}, 	// MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_L]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098E, 0xE854}, 	// MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_R]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098E, 0xE855}, 	// MCU_ADDRESS [PRI_A_CONFIG_AWB_K_G_R]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098E, 0xE856}, 	// MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_R]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098E, 0xEC51}, 	// MCU_ADDRESS [PRI_B_CONFIG_AWB_K_R_L]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098E, 0xEC52}, 	// MCU_ADDRESS [PRI_B_CONFIG_AWB_K_G_L]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098E, 0xEC53}, 	// MCU_ADDRESS [PRI_B_CONFIG_AWB_K_B_L]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098E, 0xEC54}, 	// MCU_ADDRESS [PRI_B_CONFIG_AWB_K_R_R]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098E, 0xEC55}, 	// MCU_ADDRESS [PRI_B_CONFIG_AWB_K_G_R]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098E, 0xEC56}, 	// MCU_ADDRESS [PRI_B_CONFIG_AWB_K_B_R]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098E, 0xE877}, 	// MCU_ADDRESS [PRI_A_CONFIG_LL_END_SATURATION]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0xEC77}, 	// MCU_ADDRESS [PRI_B_CONFIG_LL_END_SATURATION]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0xC8B6},   // MCU_ADDRESS
    {0x0990, 0x0008},   // MCU_DATA_0
    {0x098E, 0xC8B5},   // MCU_ADDRESS
    {0x0990, 0x00F5},   // MCU_DATA_0
    {0x098E, 0xAC37},   // MCU_ADDRESS
    {0x0990, 0x0036},   // MCU_DATA_0
    {0x098E, 0xAC38},   // MCU_ADDRESS
    {0x0990, 0x005E},   // MCU_DATA_0
    {0x098E, 0xAC39},   // MCU_ADDRESS
    {0x0990, 0x0026},   // MCU_DATA_0
    {0x098E, 0xAC3A},   // MCU_ADDRESS
    {0x0990, 0x004B},   // MCU_DATA_0
    {0x098E, 0xAC3B},   // MCU_ADDRESS
    {0x0990, 0x0040},   // MCU_DATA_0
    {0x098E, 0xAC3C},   // MCU_ADDRESS
    {0x0990, 0x002B},   // MCU_DATA_0
    {0x098E, 0xAC32},   // MCU_ADDRESS
    {0x0990, 0x001C},   // MCU_DATA_0
    {0x0982, 0x0000},   // ACCESS_CTL_STAT
    {0x098A, 0x0A80},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x3C3C},
    {0x0992, 0xCE05},
    {0x0994, 0x1F1F},
    {0x0996, 0x0204},
    {0x0998, 0x0CCC},
    {0x099A, 0x33D4},
    {0x099C, 0x30ED},
    {0x099E, 0x00FC},
    {0x098A, 0x0A90},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x0590},
    {0x0992, 0xBDA8},
    {0x0994, 0x93CE},
    {0x0996, 0x051F},
    {0x0998, 0x1F02},
    {0x099A, 0x0110},
    {0x099C, 0xCC33},
    {0x099E, 0xD830},
    {0x098A, 0x0AA0},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xED02},
    {0x0992, 0xCC05},
    {0x0994, 0xB8ED},
    {0x0996, 0x00C6},
    {0x0998, 0x06BD},
    {0x099A, 0xA8B1},
    {0x099C, 0xCE05},
    {0x099E, 0x1F1F},
    {0x098A, 0x0AB0},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x0208},
    {0x0992, 0x0CCC},
    {0x0994, 0x33D6},
    {0x0996, 0x30ED},
    {0x0998, 0x00FC},
    {0x099A, 0x0592},
    {0x099C, 0xBDA8},
    {0x099E, 0x93CC},
    {0x098A, 0x0AC0},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x33F4},
    {0x0992, 0x30ED},
    {0x0994, 0x02CC},
    {0x0996, 0xFFE9},
    {0x0998, 0xED00},
    {0x099A, 0xFC05},
    {0x099C, 0x94C4},
    {0x099E, 0x164F},
    {0x098A, 0x0AD0},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xBDA9},
    {0x0992, 0x0ACE},
    {0x0994, 0x051F},
    {0x0996, 0x1F02},
    {0x0998, 0x020A},
    {0x099A, 0xCC32},
    {0x099C, 0x1030},
    {0x099E, 0xED00},
    {0x098A, 0x0AE0},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x4FBD},
    {0x0992, 0xA8E4},
    {0x0994, 0x3838},
    {0x0996, 0x393C},
    {0x0998, 0x3CFC},
    {0x099A, 0x0322},
    {0x099C, 0xB303},
    {0x099E, 0x2030},
    {0x098A, 0x0AF0},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xED02},
    {0x0992, 0xCE03},
    {0x0994, 0x141F},
    {0x0996, 0x0408},
    {0x0998, 0x3ECE},
    {0x099A, 0x0314},
    {0x099C, 0x1F0B},
    {0x099E, 0x0134},
    {0x098A, 0x0B00},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x30EC},
    {0x0992, 0x0227},
    {0x0994, 0x2F83},
    {0x0996, 0x0000},
    {0x0998, 0x2C18},
    {0x099A, 0xF603},
    {0x099C, 0x244F},
    {0x099E, 0xED00},
    {0x098A, 0x0B10},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xFC03},
    {0x0992, 0x20A3},
    {0x0994, 0x00B3},
    {0x0996, 0x0322},
    {0x0998, 0x241A},
    {0x099A, 0xFC03},
    {0x099C, 0x22FD},
    {0x099E, 0x0320},
    {0x098A, 0x0B20},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x2012},
    {0x0992, 0xF603},
    {0x0994, 0x244F},
    {0x0996, 0xF303},
    {0x0998, 0x20B3},
    {0x099A, 0x0322},
    {0x099C, 0x2306},
    {0x099E, 0xFC03},
    {0x098A, 0x0B30},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x22FD},
    {0x0992, 0x0320},
    {0x0994, 0xBD7D},
    {0x0996, 0x9038},
    {0x0998, 0x3839},
    {0x099A, 0x3C3C},
    {0x099C, 0xFC07},
    {0x099E, 0x4327},
    {0x098A, 0x0B40},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x5FDE},
    {0x0992, 0x431F},
    {0x0994, 0xB410},
    {0x0996, 0x563C},
    {0x0998, 0xFC07},
    {0x099A, 0x4130},
    {0x099C, 0xED00},
    {0x099E, 0x3CCC},
    {0x098A, 0x0B50},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x0008},
    {0x0992, 0x30ED},
    {0x0994, 0x00FC},
    {0x0996, 0x0743},
    {0x0998, 0xBDAA},
    {0x099A, 0x7C38},
    {0x099C, 0x38BD},
    {0x099E, 0xE9E4},
    {0x098A, 0x0B60},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x30ED},
    {0x0992, 0x02CC},
    {0x0994, 0x0064},
    {0x0996, 0xED00},
    {0x0998, 0xCC01},
    {0x099A, 0x00BD},
    {0x099C, 0xAA7C},
    {0x099E, 0xFD03},
    {0x098A, 0x0B70},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x103C},
    {0x0992, 0xFC07},
    {0x0994, 0x4530},
    {0x0996, 0xED00},
    {0x0998, 0x3CCC},
    {0x099A, 0x0008},
    {0x099C, 0x30ED},
    {0x099E, 0x00FC},
    {0x098A, 0x0B80},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x0743},
    {0x0992, 0xBDAA},
    {0x0994, 0x7C38},
    {0x0996, 0x38BD},
    {0x0998, 0xE9E4},
    {0x099A, 0x30ED},
    {0x099C, 0x02CC},
    {0x099E, 0x0064},
    {0x098A, 0x0B90},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xED00},
    {0x0992, 0xCC01},
    {0x0994, 0x00BD},
    {0x0996, 0xAA7C},
    {0x0998, 0xFD03},
    {0x099A, 0x1220},
    {0x099C, 0x03BD},
    {0x099E, 0x7993},
    {0x098A, 0x0BA0},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x3838},
    {0x0992, 0x390F},
    {0x0994, 0xF601},
    {0x0996, 0x05C1},
    {0x0998, 0x0326},
    {0x099A, 0x14F6},
    {0x099C, 0x0106},
    {0x099E, 0xC106},
    {0x098A, 0x0BB0},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x260D},
    {0x0992, 0xF630},
    {0x0994, 0x4DC4},
    {0x0996, 0xF0CA},
    {0x0998, 0x08F7},
    {0x099A, 0x304D},
    {0x099C, 0xBD0B},
    {0x099E, 0xC10E},
    {0x098A, 0x0BC0},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x39F6},
    {0x0992, 0x304D},
    {0x0994, 0xC4F0},
    {0x0996, 0xCA09},
    {0x0998, 0xF730},
    {0x099A, 0x4DDE},
    {0x099C, 0xF218},
    {0x099E, 0xCE0A},
    {0x098A, 0x0BD0},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x00CC},
    {0x0992, 0x001D},
    {0x0994, 0xBDB5},
    {0x0996, 0x31DE},
    {0x0998, 0xA818},
    {0x099A, 0xCE0A},
    {0x099C, 0x1ECC},
    {0x099E, 0x001D},
    {0x098A, 0x0BE0},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xBDB5},
    {0x0992, 0x31DE},
    {0x0994, 0xA618},
    {0x0996, 0xCE0A},
    {0x0998, 0x3CCC},
    {0x099A, 0x0013},
    {0x099C, 0xBDB5},
    {0x099E, 0x31CC},
    {0x098A, 0x0BF0},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x0A80},
    {0x0992, 0xFD0A},
    {0x0994, 0x0ECC},
    {0x0996, 0x0AE7},
    {0x0998, 0xFD0A},
    {0x099A, 0x30CC},
    {0x099C, 0x0B3A},
    {0x099E, 0xFD0A},
    {0x098A, 0x0C00},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x4CCC},
    {0x0992, 0x0A00},
    {0x0994, 0xDDF2},
    {0x0996, 0xCC0A},
    {0x0998, 0x1EDD},
    {0x099A, 0xA8CC},
    {0x099C, 0x0A3C},
    {0x099E, 0xDDA6},
    {0x098A, 0x0C10},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xC601},
    {0x0992, 0xF701},
    {0x0994, 0x0CF7},
    {0x0996, 0x010D},
    {0x098A, 0x8C18},   // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x0039},   // MCU_DATA_0
    {0x098E, 0x0012},   // MCU_ADDRESS [MON_ADDR]
    {0x0990, 0x0BA3},   // MCU_DATA_0
    {0x098E, 0x0003},   // MCU_ADDRESS [MON_ALGO]
    {0x0990, 0x0004},   // MCU_DATA_0

};
static struct msm_camera_i2c_reg_conf mt9t113_recommend_settings_4[] = {
    {0x098E, 0x8400},      // MCU_ADDRESS
    {0x0990, 0x0006},
    {0x3ED6, 0x0F00},   // DAC_LD_10_11
    {0x3EF2, 0xD965},   // DAC_LP_6_7
    {0x3FD2, 0xD965},
    {0x3EF8, 0x7F7F},   // DAC_LD_TXHI
    {0x3ED8, 0x7F1D},   // DAC_LD_12_13
    {0x3172, 0x0033},   // ANALOG_CONTROL2
    {0x3EEA, 0x0200},   // DAC_LD_30_31
    {0x3EE2, 0x0050},   // DAC_LD_22_23
    {0x316A, 0x8200},   // DAC_FBIAS
    {0x316C, 0x8200},   // DAC_TXLO
    {0x3EFC, 0xA8E8},   // DAC_LD_FBIAS
    {0x3EFE, 0x130D},   // DAC_LD_TXLO
    {0x3180, 0xB3FF},   // FINE_DIG_CORRECTION_CONTROL
    {0x30B2, 0xC000},   // CALIB_TIED_OFFSET
    {0x30BC, 0x0384},   // CALIB_GLOBAL
    {0x30C0, 0x1220},   // CALIB_CONTROL
    {0x3170, 0x000A},   // ANALOG_CONTROL
    {0x3174, 0x8060},   // ANALOG_CONTROL3
    {0x3ECC, 0x22B0},   // DAC_LD_0_1
    {0x098E, 0x482B},   // MCU_ADDRESS
    {0x0990, 0x22B0},   // MCU_DATA_0
    {0x098E, 0x4858},   // MCU_ADDRESS
    {0x0990, 0x22B0},   // MCU_DATA_0
    {0x317A, 0x000A},   // ANALOG_CONTROL6
    {0x098E, 0x4822},   // MCU_ADDRESS
    {0x0990, 0x000A},   // MCU_DATA_0
    {0x098E, 0x4824},   // MCU_ADDRESS
    {0x0990, 0x000A},   // MCU_DATA_0
    {0x098E, 0x484F},   // MCU_ADDRESS
    {0x0990, 0x000A},   // MCU_DATA_0
    {0x098E, 0x4851},   // MCU_ADDRESS
    {0x0990, 0x000A},   // MCU_DATA_0
    {0x3210, 0x01B0},   // COLOR_PIPELINE_CONTROL
    {0x3640, 0x00F0},
    {0x3642, 0x308D},
    {0x3644, 0x55B1},
    {0x3646, 0xFF8C},
    {0x3648, 0x398E},
    {0x364A, 0x0230},
    {0x364C, 0x99EE},
    {0x364E, 0x3E31},
    {0x3650, 0x0B6E},
    {0x3652, 0x552F},
    {0x3654, 0x0270},
    {0x3656, 0x240E},
    {0x3658, 0x1231},
    {0x365A, 0xD7AE},
    {0x365C, 0x4FB0},
    {0x365E, 0x0550},
    {0x3660, 0xC7EE},
    {0x3662, 0x70B1},
    {0x3664, 0x956D},
    {0x3666, 0xF50C},
    {0x3680, 0xC5AB},
    {0x3682, 0x954E},
    {0x3684, 0x82F0},
    {0x3686, 0x034D},
    {0x3688, 0x14B1},
    {0x368A, 0xE1EC},
    {0x368C, 0x164E},
    {0x368E, 0xDFCD},
    {0x3690, 0xB16F},
    {0x3692, 0x86EB},
    {0x3694, 0x492D},
    {0x3696, 0x43CE},
    {0x3698, 0x644F},
    {0x369A, 0x67EA},
    {0x369C, 0x8DB1},
    {0x369E, 0x6A8D},
    {0x36A0, 0xFBCE},
    {0x36A2, 0x1BF0},
    {0x36A4, 0x492E},
    {0x36A6, 0x96D1},
    {0x36C0, 0x73B1},
    {0x36C2, 0x46EF},
    {0x36C4, 0x6B11},
    {0x36C6, 0x1BCC},
    {0x36C8, 0x8093},
    {0x36CA, 0x7F31},
    {0x36CC, 0xFA8F},
    {0x36CE, 0x2E11},
    {0x36D0, 0x6F6D},
    {0x36D2, 0xAC52},
    {0x36D4, 0x4EB1},
    {0x36D6, 0x0FB0},
    {0x36D8, 0x6B72},
    {0x36DA, 0xAC89},
    {0x36DC, 0x86B4},
    {0x36DE, 0x7191},
    {0x36E0, 0x8B30},
    {0x36E2, 0x6551},
    {0x36E4, 0x6BCE},
    {0x36E6, 0xAE92},
    {0x3700, 0xC30F},
    {0x3702, 0x22AD},
    {0x3704, 0x58B0},
    {0x3706, 0x86CC},
    {0x3708, 0x8CF3},
    {0x370A, 0x848E},
    {0x370C, 0x0B4E},
    {0x370E, 0x8391},
    {0x3710, 0x1B90},
    {0x3712, 0xCD0E},
    {0x3714, 0xD72F},
    {0x3716, 0x78AE},
    {0x3718, 0xFD4F},
    {0x371A, 0x8592},
    {0x371C, 0x2991},
    {0x371E, 0x85AF},
    {0x3720, 0x15B0},
    {0x3722, 0x8952},
    {0x3724, 0xA3B1},
    {0x3726, 0x7BB2},
    {0x3740, 0xD691},
    {0x3742, 0xC8AF},
    {0x3744, 0xEA53},
    {0x3746, 0xA113},
    {0x3748, 0x0396},
    {0x374A, 0x8FF2},
    {0x374C, 0x28D1},
    {0x374E, 0x8530},
    {0x3750, 0xDBD4},
    {0x3752, 0x0B76},
    {0x3754, 0xB131},
    {0x3756, 0xB4F1},
    {0x3758, 0x8935},
    {0x375A, 0x8773},
    {0x375C, 0x6336},
    {0x375E, 0xDB71},
    {0x3760, 0x6F91},
    {0x3762, 0xFCB3},
    {0x3764, 0x8BD4},
    {0x3766, 0x3D75},
    {0x3782, 0x0320},
    {0x3784, 0x0400},
    {0x3210, 0x01B8},   // COLOR_PIPELINE_CONTROL
    
    {0x098E, 0x4918},   // MCU_ADDRESS
    {0x0990, 0x0039},   // MCU_DATA_0
    {0x098E, 0x491A},   // MCU_ADDRESS
    {0x0990, 0x0100},   // MCU_DATA_0
    {0x098E, 0x6872},   // MCU_ADDRESS
    {0x0990, 0x0005},   // MCU_DATA_0
    {0x098E, 0x6874},   // MCU_ADDRESS
    {0x0990, 0x008C},   // MCU_DATA_0
    {0x098E, 0x4956},   // MCU_ADDRESS
    {0x0990, 0x0040},   // MCU_DATA_0
    {0x098E, 0x4958},   // MCU_ADDRESS
    {0x0990, 0x0100},   // MCU_DATA_0
    {0x098E, 0x495A},   // MCU_ADDRESS
    {0x0990, 0x0000},   // MCU_DATA_0
    {0x098E, 0x495C},   // MCU_ADDRESS
    {0x0990, 0x0000},   // MCU_DATA_0
    {0x098E, 0x495E},   // MCU_ADDRESS
    {0x0990, 0x0040},   // MCU_DATA_0
    {0x098E, 0x4960},   // MCU_ADDRESS
    {0x0990, 0x0100},   // MCU_DATA_0
    {0x098E, 0xC962},   // MCU_ADDRESS
    {0x0990, 0x0000},   // MCU_DATA_0
    {0x098E, 0xC963},   // MCU_ADDRESS
    {0x0990, 0x0003},   // MCU_DATA_0
    {0x098E, 0x4964},   // MCU_ADDRESS
    {0x0990, 0x0040},   // MCU_DATA_0
    {0x098E, 0x4966},   // MCU_ADDRESS
    {0x0990, 0x0100},   // MCU_DATA_0
    {0x098E, 0x4968},   // MCU_ADDRESS
    {0x0990, 0x0001},   // MCU_DATA_0
    {0x098E, 0x496A},   // MCU_ADDRESS
    {0x0990, 0x0001},   // MCU_DATA_0
    {0x098E, 0x496C},   // MCU_ADDRESS
    {0x0990, 0x0014},   // MCU_DATA_0
    {0x098E, 0x496E},   // MCU_ADDRESS
    {0x0990, 0x000C},   // MCU_DATA_0
    {0x098E, 0xC970},   // MCU_ADDRESS
    {0x0990, 0x0004},   // MCU_DATA_0
    {0x098E, 0xC971},   // MCU_ADDRESS
    {0x0990, 0x000F},   // MCU_DATA_0
    {0x098E, 0x4972},   // MCU_ADDRESS
    {0x0990, 0x0040},   // MCU_DATA_0
    {0x098E, 0x4974},   // MCU_ADDRESS
    {0x0990, 0x0100},   // MCU_DATA_0
    {0x098E, 0x4976},   // MCU_ADDRESS
    {0x0990, 0x0001},   // MCU_DATA_0
    {0x098E, 0x4978},   // MCU_ADDRESS
    {0x0990, 0x0001},   // MCU_DATA_0
    {0x098E, 0x497A},   // MCU_ADDRESS
    {0x0990, 0x00C8},   // MCU_DATA_0
    {0x098E, 0x497C},   // MCU_ADDRESS
    {0x0990, 0x003C},   // MCU_DATA_0
    {0x098E, 0xC97E},   // MCU_ADDRESS
    {0x0990, 0x0004},   // MCU_DATA_0
    {0x098E, 0xC97F},   // MCU_ADDRESS
    {0x0990, 0x000F},   // MCU_DATA_0
    {0x098E, 0x491C},   // MCU_ADDRESS
    {0x0990, 0x0040},   // MCU_DATA_0
    {0x098E, 0x491E},   // MCU_ADDRESS
    {0x0990, 0x0100},   // MCU_DATA_0
    {0x098E, 0xC920},   // MCU_ADDRESS
    {0x0990, 0x000B},   // MCU_DATA_0
    {0x098E, 0xC921},   // MCU_ADDRESS
    {0x0990, 0x002C},   // MCU_DATA_0
    {0x098E, 0xC922},   // MCU_ADDRESS
    {0x0990, 0x0007},   // MCU_DATA_0
    {0x098E, 0xC923},   // MCU_ADDRESS
    {0x0990, 0x001D},   // MCU_DATA_0
    {0x098E, 0x4926},   // MCU_ADDRESS
    {0x0990, 0x0039},   // MCU_DATA_0
    {0x098E, 0x4928},   // MCU_ADDRESS
    {0x0990, 0x00A0},   // MCU_DATA_0
    {0x098E, 0x492A},   // MCU_ADDRESS
    {0x0990, 0x0082},   // MCU_DATA_0
    {0x098E, 0x492C},   // MCU_ADDRESS
    {0x0990, 0x0040},   // MCU_DATA_0
    {0x098E, 0x492E},   // MCU_ADDRESS
    {0x0990, 0x0015},   // MCU_DATA_0
    {0x098E, 0x4930},   // MCU_ADDRESS
    {0x0990, 0x0015},   // MCU_DATA_0
    {0x098E, 0x4932},   // MCU_ADDRESS
    {0x0990, 0x0002},   // MCU_DATA_0
    {0x098E, 0x4934},   // MCU_ADDRESS
    {0x0990, 0x0004},   // MCU_DATA_0
    {0x098E, 0x4936},   // MCU_ADDRESS
    {0x0990, 0x0008},   // MCU_DATA_0
    {0x098E, 0x4938},   // MCU_ADDRESS
    {0x0990, 0x0009},   // MCU_DATA_0
    {0x098E, 0x493A},   // MCU_ADDRESS
    {0x0990, 0x000C},   // MCU_DATA_0
    {0x098E, 0x493C},   // MCU_ADDRESS
    {0x0990, 0x000D},   // MCU_DATA_0
    {0x098E, 0x493E},   // MCU_ADDRESS
    {0x0990, 0x0015},   // MCU_DATA_0
    {0x098E, 0x4940},   // MCU_ADDRESS
    {0x0990, 0x0013},   // MCU_DATA_0
    {0x098E, 0xC944},   // MCU_ADDRESS
    {0x0990, 0x0023},   // MCU_DATA_0
    {0x098E, 0xC945},   // MCU_ADDRESS
    {0x0990, 0x007F},   // MCU_DATA_0
    {0x098E, 0xC946},   // MCU_ADDRESS
    {0x0990, 0x0007},   // MCU_DATA_0
    {0x098E, 0xC947},   // MCU_ADDRESS
    {0x0990, 0x0001},   // MCU_DATA_0
    {0x098E, 0xC948},   // MCU_ADDRESS
    {0x0990, 0x0002},   // MCU_DATA_0
    {0x098E, 0xC949},   // MCU_ADDRESS
    {0x0990, 0x000C},   // MCU_DATA_0
    {0x098E, 0xC94A},   // MCU_ADDRESS
    {0x0990, 0x00FF},   // MCU_DATA_0
    {0x098E, 0xC94B},   // MCU_ADDRESS
    {0x0990, 0x00FF},   // MCU_DATA_0
    {0x098E, 0xC906},   // MCU_ADDRESS
    {0x0990, 0x0006},   // MCU_DATA_0
    {0x098E, 0xC907},   // MCU_ADDRESS
    {0x0990, 0x00FF},   // MCU_DATA_0
    {0x098E, 0xBC02},   // MCU_ADDRESS
    {0x0990, 0x0005},   // MCU_DATA_0
    {0x098E, 0xC908},   // MCU_ADDRESS
    {0x0990, 0x0006},   // MCU_DATA_0
    {0x098E, 0xC909},   // MCU_ADDRESS
    {0x0990, 0x0028},   // MCU_DATA_0
    {0x098E, 0xC90A},   // MCU_ADDRESS
    {0x0990, 0x0007},   // MCU_DATA_0
    
    {0x326C, 0x140B},   // APERTURE_PARAMETERS_2D
    {0x098E, 0xC94C},   // MCU_ADDRESS
    {0x0990, 0x0003},   // MCU_DATA_0
    {0x098E, 0xC94E},   // MCU_ADDRESS
    {0x0990, 0x003C},   // MCU_DATA_0
    {0x098E, 0xC94F},   // MCU_ADDRESS
    {0x0990, 0x0064},   // MCU_DATA_0
    {0x098E, 0xE877},   // MCU_ADDRESS
    {0x0990, 0x0050},   // MCU_DATA_0
    {0x098E, 0x4912},   // MCU_ADDRESS
    {0x0990, 0x0000},   // MCU_DATA_0
    {0x098E, 0x4914},   // MCU_ADDRESS
    {0x0990, 0x0001},   // MCU_DATA_0
    {0x098E, 0x4916},   // MCU_ADDRESS
    {0x0990, 0x0037},   // MCU_DATA_0
    {0x098E, 0xBC06},   // MCU_ADDRESS
    {0x0990, 0x0002},   // MCU_DATA_0
    //GAMMA
    {0x098E, 0xBC1C}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098E, 0xBC1D}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_1]
    {0x0990, 0x0008}, 	// MCU_DATA_0
    {0x098E, 0xBC1E}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_2]
    {0x0990, 0x001c}, 	// MCU_DATA_0
    {0x098E, 0xBC1F}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_3]
    {0x0990, 0x0036}, 	// MCU_DATA_0
    {0x098E, 0xBC20}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_4]
    {0x0990, 0x005a}, 	// MCU_DATA_0
    {0x098E, 0xBC21}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_5]
    {0x0990, 0x0078}, 	// MCU_DATA_0
    {0x098E, 0xBC22}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_6]
    {0x0990, 0x0090}, 	// MCU_DATA_0
    {0x098E, 0xBC23}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_7]
    {0x0990, 0x00a2}, 	// MCU_DATA_0
    {0x098E, 0xBC24}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_8]
    {0x0990, 0x00b1}, 	// MCU_DATA_0
    {0x098E, 0xBC25}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_9]
    {0x0990, 0x00Bc}, 	// MCU_DATA_0
    {0x098E, 0xBC26}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_10]
    {0x0990, 0x00C6}, 	// MCU_DATA_0
    {0x098E, 0xBC27}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_11]
    {0x0990, 0x00Cd}, 	// MCU_DATA_0
    {0x098E, 0xBC28}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_12]
    {0x0990, 0x00D5}, 	// MCU_DATA_0
    {0x098E, 0xBC29}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_13]
    {0x0990, 0x00Dc}, 	// MCU_DATA_0
    {0x098E, 0xBC2A}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_14]
    {0x0990, 0x00E2}, 	// MCU_DATA_0
    {0x098E, 0xBC2B}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_15]
    {0x0990, 0x00E8}, 	// MCU_DATA_0
    {0x098E, 0xBC2C}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_16]
    {0x0990, 0x00ee}, 	// MCU_DATA_0
    {0x098E, 0xBC2D}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_17]
    {0x0990, 0x00F4}, 	// MCU_DATA_0
    {0x098E, 0xBC2E}, 	// MCU_ADDRESS [LL_GAMMA_NEUTRAL_CURVE_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    
    {0x098E, 0x3C42},   // MCU_ADDRESS
    {0x0990, 0x0032},   // MCU_DATA_0
    
    {0x098E, 0x3C44},   // MCU_ADDRESS
    
    {0x0990, 0x0000},   // MCU_DATA_0
    {0x0018, 0x002A},   // MCU_DATA_0	

};
static struct msm_camera_i2c_reg_conf mt9t113_recommend_settings_3[] =
{
     {0x098E, 0x4810},  //MCU_ADDRESS [CAM1_CTX_A_READ_MODE]
     {0x0990, 0x046F}, // MCU_DATA_0
     {0x098E, 0x483D}, // MCU_ADDRESS [CAM1_CTX_B_READ_MODE]
     {0x0990, 0x0027}, // MCU_DATA_0
    
};

static struct msm_camera_i2c_reg_conf mt9t113_wb_auto_reg_config[] =
{
    {0x098E, 0x6848},
    {0x0990, 0x003F},
    {0x098E, 0x6865},
    {0x0990, 0x801F},
    {0x098E, 0x6867},
    {0x0990, 0x12F7},
    {0x098E, 0x6881},
    {0x0990, 0x000B},
    {0x098E, 0x6883},
    {0x0990, 0x000B},
    {0x098E, 0x8400},
    {0x0990, 0x0006},
};
static struct msm_camera_i2c_reg_conf mt9t113_wb_incandescent_reg_config[] =
{
    {0x098E, 0x6848},
    {0x0990, 0x0000},
    {0x098E, 0x6865},
    {0x0990, 0x0000},
    {0x098E, 0x6867},
    {0x0990, 0x0000},
    {0x098E, 0x6881},
    {0x0990, 0x0009},
    {0x098E, 0x6883},
    {0x0990, 0x0009},
    {0x098E, 0x8400},
    {0x0990, 0x0006},
    
    {0x098E, 0xAC3B},
    {0x0990, 0x005a},//R_Gain
    {0x098E, 0xAC3C},
    {0x0990, 0x0025},//B_Gain
};
static struct msm_camera_i2c_reg_conf mt9t113_wb_fluorescent_reg_config[] =
{
    {0x098E, 0x6848},
    {0x0990, 0x0000},
    {0x098E, 0x6865},
    {0x0990, 0x0000},
    {0x098E, 0x6867},
    {0x0990, 0x0000},
    {0x098E, 0x6881},
    {0x0990, 0x0009},
    {0x098E, 0x6883},
    {0x0990, 0x0009},
    {0x098E, 0x8400},
    {0x0990, 0x0006},
    {0x098E, 0xAC3B},
    {0x0990, 0x004c},//R_Gain
    {0x098E, 0xAC3C},
    {0x0990, 0x0030},
};
static struct msm_camera_i2c_reg_conf mt9t113_wb_daylight_reg_config[] =
{
    {0x098E, 0x6848},
    {0x0990, 0x0000},
    {0x098E, 0x6865},
    {0x0990, 0x0000},
    {0x098E, 0x6867},
    {0x0990, 0x0000},
    {0x098E, 0x6881},
    {0x0990, 0x0009},
    {0x098E, 0x6883},
    {0x0990, 0x0009},
    {0x098E, 0x8400},
    {0x0990, 0x0006},
    
    {0x098E, 0xAC3B},
    {0x0990, 0x0043},//R_Gain
    {0x098E, 0xAC3C},
    {0x0990, 0x0040},//B_Gain
};
static struct msm_camera_i2c_reg_conf mt9t113_wb_cloudy_reg_config[] =
{
    {0x098E, 0x6848},
    {0x0990, 0x0000},
    {0x098E, 0x6865},
    {0x0990, 0x0000},
    {0x098E, 0x6867},
    {0x0990, 0x0000},
    {0x098E, 0x6881},
    {0x0990, 0x0009},
    {0x098E, 0x6883},
    {0x0990, 0x0009},
    {0x098E, 0x8400},
    {0x0990, 0x0006},
    
    {0x098E, 0xAC3B},
    {0x0990, 0x003d},//R_Gain
    {0x098E, 0xAC3C},
    {0x0990, 0x004e},//B_Gain  50
};

static struct msm_camera_i2c_reg_conf mt9t113_effect_off_reg_config[] =
{
    {0x098E, 0xE887},
    {0x0990, 0x0000},
    {0x098E, 0xEC87},
    {0x0990, 0x0000},
    {0x098E, 0x8400},
    {0x0990, 0x0006},
};

static struct msm_camera_i2c_reg_conf mt9t113_effect_mono_reg_config[] =
{
    {0x098E, 0xE887},
    {0x0990, 0x0001},
    {0x098E, 0xEC87},
    {0x0990, 0x0001},
    {0x098E, 0x8400},
    {0x0990, 0x0006},
};

static struct msm_camera_i2c_reg_conf mt9t113_effect_negative_reg_config[] =
{
    {0x098E, 0xE887},
    {0x0990, 0x0003},
    {0x098E, 0xEC87},
    {0x0990, 0x0003},
    {0x098E, 0x8400},
    {0x0990, 0x0006},
};

static struct msm_camera_i2c_reg_conf mt9t113_effect_sepia_reg_config[] =
{
    {0x098E, 0xE887},
    {0x0990, 0x0002},
    {0x098E, 0xEC87},
    {0x0990, 0x0002},
    
    {0x098E, 0xE889},
    {0x0990, 0x001E},
    {0x098E, 0xE88A},
    {0x0990, 0x009C},
    {0x098E, 0xEC89},
    {0x0990, 0x001E},
    {0x098E, 0xEC8A},
    {0x0990, 0x009C},
    {0x098E, 0x8400},
    {0x0990, 0x0006},
};

static struct msm_camera_i2c_reg_conf mt9t113_effect_aqua_reg_config[] =
{
    {0x098E, 0xE887},
    {0x0990, 0x0002},
    {0x098E, 0xEC87},
    {0x0990, 0x0002},
    
    {0x098E, 0xE889},
    {0x0990, 0x00CA},
    {0x098E, 0xE88A},
    {0x0990, 0x0028},
    {0x098E, 0xEC89},
    {0x0990, 0x00CA},
    {0x098E, 0xEC8A},
    {0x0990, 0x0028},
    {0x098E, 0x8400},
    {0x0990, 0x0006},
};
static struct v4l2_subdev_info mt9t113_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array mt9t113_init_conf[] = {
	{&mt9t113_recommend_settings_1[0],
		ARRAY_SIZE(mt9t113_recommend_settings_1), 100, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9t113_recommend_settings_2[0],
		ARRAY_SIZE(mt9t113_recommend_settings_2), 10, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9t113_recommend_settings_3[0],
		ARRAY_SIZE(mt9t113_recommend_settings_3), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9t113_recommend_settings_4[0],
		ARRAY_SIZE(mt9t113_recommend_settings_4), 300, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9t113_confs[] = {
	{&mt9t113_snap_settings[0],
	ARRAY_SIZE(mt9t113_snap_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9t113_prev_settings[0],
	ARRAY_SIZE(mt9t113_prev_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_sensor_output_info_t mt9t113_dimensions[] = {
	{
		.x_output = 0x800,
		.y_output = 0x600,
		//.line_length_pclk = 0xAB2,
		//.frame_length_lines = 0x7B4,
		//.vt_pixel_clk = 81600000,
		//.op_pixel_clk = 81600000,
		.binning_factor = 0,
	},
	{
		.x_output = 0x400,
		.y_output = 0x300,
		//.line_length_pclk = 0xAB2,
		//.frame_length_lines = 0x3E0,
		//.vt_pixel_clk = 81600000,
		//.op_pixel_clk = 81600000,
		.binning_factor = 1,
	},
};

static struct msm_camera_csi_params mt9t113_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 18,
};

static struct msm_camera_csi_params *mt9t113_csi_params_array[] = {
	&mt9t113_csi_params,
	&mt9t113_csi_params,
};
static struct msm_sensor_id_info_t mt9t113_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x4680,
};

static int32_t mt9t113_write_prev_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
						uint16_t gain, uint32_t line)
{
	return 0;
}

static int32_t mt9t113_write_pict_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	mdelay(10);
	return 0;
}

int32_t mt9t113_sensor_set_wb(struct msm_sensor_ctrl_t *s_ctrl, int wb)
{
	struct msm_camera_i2c_reg_conf *reg_conf_tbl = NULL;
	uint16_t num_of_items_in_table = 0;
	int rc = 0;

	printk("%s, to set wb = %d \n", __func__,wb);
	switch (wb)
	{
		case CAMERA_WB_AUTO:
			reg_conf_tbl = &mt9t113_wb_auto_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9t113_wb_auto_reg_config);
			break;
		case CAMERA_WB_INCANDESCENT:
			reg_conf_tbl = &mt9t113_wb_incandescent_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9t113_wb_incandescent_reg_config);
			break;
		case CAMERA_WB_FLUORESCENT:
			reg_conf_tbl = &mt9t113_wb_fluorescent_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9t113_wb_fluorescent_reg_config);
			break;
		case CAMERA_WB_DAYLIGHT:
			reg_conf_tbl = &mt9t113_wb_daylight_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9t113_wb_daylight_reg_config);
			break;
		case CAMERA_WB_CLOUDY_DAYLIGHT:
			reg_conf_tbl = &mt9t113_wb_cloudy_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9t113_wb_cloudy_reg_config);
			break;
		default:
			return 0;
	}
	
	rc = msm_camera_i2c_write_tbl(
		s_ctrl->sensor_i2c_client,
		reg_conf_tbl,
		num_of_items_in_table, 
		MSM_CAMERA_I2C_WORD_DATA);

	return rc;
}

int32_t mt9t113_sensor_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int effect)
{
	struct msm_camera_i2c_reg_conf *reg_conf_tbl = NULL;
	uint16_t num_of_items_in_table = 0;
	int rc = 0;

	printk("%s, to set effect = %d \n", __func__,effect);
	switch (effect)
	{
		case CAMERA_EFFECT_OFF:
			reg_conf_tbl = &mt9t113_effect_off_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9t113_effect_off_reg_config);
			break;
		case CAMERA_EFFECT_MONO:
			reg_conf_tbl = &mt9t113_effect_mono_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9t113_effect_mono_reg_config);
			break;
		case CAMERA_EFFECT_NEGATIVE:
			reg_conf_tbl = &mt9t113_effect_negative_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9t113_effect_negative_reg_config);
			break;
		case CAMERA_EFFECT_SEPIA:
			reg_conf_tbl = &mt9t113_effect_sepia_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9t113_effect_sepia_reg_config);
			break;
		case CAMERA_EFFECT_AQUA:
			reg_conf_tbl = &mt9t113_effect_aqua_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9t113_effect_aqua_reg_config);
			break;
		default:
			return 0;
	}
	
	rc = msm_camera_i2c_write_tbl(
		s_ctrl->sensor_i2c_client,
		reg_conf_tbl,
		num_of_items_in_table, 
		MSM_CAMERA_I2C_WORD_DATA);

	return rc;
}

static int32_t mt9t113_wait(struct msm_sensor_ctrl_t *s_ctrl,  int time)
{
	int rc = -EFAULT, count = 0;
	unsigned short r_value = 0;
	/*modify delays and polls after register writing*/
	switch(time){
		case REGISTER_WRITE_WAIT_FW:
		{
			/* wait for FW initialization complete
			 * bit[14] of register 0x0018 is the STANDBY_CONTROL_AND_STATUS bit
			 * the bit is 0 when initialization done
			 * read the register per 5ms, timeout after 100ms
			 */
			for(count = 0; count < 20; count ++)
			{
				CDBG("time = %d, count =%d\n", time ,count);
				rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x0018, &r_value, MSM_CAMERA_I2C_WORD_DATA);
				if(0 == (r_value & 0x4000))
				{
					break;
				}
				mdelay(5);
			}
		}
			break;
		case REGISTER_WRITE_WAIT_PATCH:
		{
			/* wait for Patch loading complete
			 * variable 0x800C is the MON_RAM_PATCH_ID status
			 * the status is 1 when initialization done
			 * read the statue per 10ms, timeout after 200ms
			 */
			for(count = 0; count < 20; count ++)
			{
				CDBG("time = %d, count =%d,", time ,count);
				msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x098E, 0x800C, MSM_CAMERA_I2C_WORD_DATA);
				rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x0990, &r_value,  MSM_CAMERA_I2C_WORD_DATA);
				CDBG("====rc = %d, r_value =%d \n", rc, r_value);
				if(1 == r_value)
				{
					break;
				}
				mdelay(10);
			}
		}
			break;
		case REGISTER_WRITE_WAIT_PREVIEW:
		{
			/* wait for sensor to enter the Preview mode
			 * variable 0x8401 is the sensor MODE status
			 * the status is 0x0003 when sensor in Preview mode
			 * read the statue per 20ms, timeout after 300ms
			 */
			mdelay(30);
			for(count = 0; count < 15; count ++)
			{
				printk("time = %d, count =%d,", time ,count);
				msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x098E, 0x8401, MSM_CAMERA_I2C_WORD_DATA);
				msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x0990, &r_value,  MSM_CAMERA_I2C_WORD_DATA);
				printk("====rc = %d, r_value =%d \n", rc, r_value);
				if(0x0003 == r_value)
				{
					rc = 0;
					break;
				}
				mdelay(20);
			}
		}
			break;
		case REGISTER_WRITE_WAIT_SNAPSHOT:
		{
			/* wait for sensor to enter the Snapshot mode
			 * variable 0x8401 is the sensor MODE status
			 * the status is 0x0007 when sensor in Snapshot mode
			 * read the statue per 20ms, timeout after 300ms
			 */
			mdelay(30);
			for(count = 0; count < 15; count ++)
			{
				printk("time = %d, count =%d,", time ,count);
				msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x098E, 0x8401, MSM_CAMERA_I2C_WORD_DATA);
				rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x0990, &r_value,  MSM_CAMERA_I2C_WORD_DATA);
				printk("====rc = %d, r_value =%d \n", rc, r_value);
				if(0x0007 == r_value)
				{
					rc = 0;
					break;
				}
				mdelay(20);
			}
		}
			break;
		default:
			rc = -EFAULT;
			break;
	}
	return rc;
}

static int32_t mt9t113_write_init_settings(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc=0, i;

	CDBG("%s is called !\n", __func__);

	for (i = 0; i < s_ctrl->msm_sensor_reg->init_size; i++)
	{
		rc = msm_sensor_write_conf_array(
			s_ctrl->sensor_i2c_client,
			s_ctrl->msm_sensor_reg->init_settings, i);
		if(rc < 0)
			break;
		mt9t113_wait(s_ctrl, i);
	}

	return rc;
}

static int32_t mt9t113_sensor_mode_setting(struct msm_sensor_ctrl_t *s_ctrl, int res)
{
	unsigned short r_value = 0;
	int rc = 0, i;
	/*preview setting*/
	if (RES_PREVIEW == res)
	{
		CDBG("%s: sensor: init preview reg.\n", __func__);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x098E, 0x8401, MSM_CAMERA_I2C_WORD_DATA);
		msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x0990, &r_value,  MSM_CAMERA_I2C_WORD_DATA);
		printk("%s: r_value = %d\n", __func__, r_value);

		if(0x0003 != r_value)
		{
			for(i=0; i < REGISTER_WRITE_RETRY; i++)
			{
				rc =msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, s_ctrl->msm_sensor_reg->mode_settings, res);
				if (rc)
				{
					CDBG("%s: init preview reg error!!!!!!\n", __func__);
				}

				rc = mt9t113_wait(s_ctrl, REGISTER_WRITE_WAIT_PREVIEW);
				if(!rc)
					break;
			}
		}
	}
        /*snapshot setting*/
	else
	{
		CDBG("%s: sensor: init snapshot reg.\n", __func__);

		for(i=0; i < REGISTER_WRITE_RETRY; i++)
		{
			rc =msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, s_ctrl->msm_sensor_reg->mode_settings, res);
			if (rc)
			{
				CDBG("%s: init snapshot reg error!!!!!!\n", __func__);
			}

			rc = mt9t113_wait(s_ctrl, REGISTER_WRITE_WAIT_SNAPSHOT);
			if(!rc)
				break;
		}
	}

	mdelay(5);
	return rc;
}

static int32_t mt9t113_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;

	if(s_ctrl->func_tbl->sensor_stop_stream)
	{
		s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	}
	msleep(30);

	if (MSM_SENSOR_REG_INIT == update_type)
	{
		CDBG("%s:Register INIT\n", __func__);

		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);

		if(s_ctrl->func_tbl->sensor_write_init_settings)
		{
			s_ctrl->func_tbl->sensor_write_init_settings(s_ctrl);
		}
		else
		{
			msm_sensor_write_init_settings(s_ctrl);
		}
		mt9t113_csi_config = 0;
	}
	else if (MSM_SENSOR_UPDATE_PERIODIC == update_type)
	{
		CDBG("%s:PERIODIC : %d\n", __func__, res);

		mt9t113_sensor_mode_setting(s_ctrl, res);

		if(!mt9t113_csi_config) {
			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,	NOTIFY_CSIC_CFG, s_ctrl->curr_csic_params);
			CDBG("CSI config is done\n");
			mb();
			msleep(30);
			mt9t113_csi_config = 1;
		}

		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,	NOTIFY_PCLK_CHANGE, &s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);

		if(s_ctrl->func_tbl->sensor_start_stream)
		{
			s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		}
		msleep(50);
	}

	return rc;
}


static int mt9t113_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	static struct msm_cam_clk_info clk_info[] = {
		{"cam_clk", MSM_SENSOR_MCLK_24HZ},
	};
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;

	CDBG("%s: %d\n", __func__, __LINE__);
	s_ctrl->reg_ptr = kzalloc(sizeof(struct regulator *)
			* data->sensor_platform_info->num_vreg, GFP_KERNEL);
	if (!s_ctrl->reg_ptr) {
		pr_err("%s: could not allocate mem for regulators\n",
			__func__);
		return -ENOMEM;
	}

	rc = msm_camera_request_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: request gpio failed\n", __func__);
		goto request_gpio_failed;
	}

	rc = msm_camera_config_vreg(&s_ctrl->sensor_i2c_client->client->dev,
		s_ctrl->sensordata->sensor_platform_info->cam_vreg,
		s_ctrl->sensordata->sensor_platform_info->num_vreg,
		s_ctrl->reg_ptr, 1);
	if (rc < 0) {
		pr_err("%s: regulator on failed\n", __func__);
		goto config_vreg_failed;
	}

	rc = msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
			s_ctrl->sensordata->sensor_platform_info->cam_vreg,
			s_ctrl->sensordata->sensor_platform_info->num_vreg,
			s_ctrl->reg_ptr, 1);
	if (rc < 0) {
		pr_err("%s: enable regulator failed\n", __func__);
		goto enable_vreg_failed;
	}

	/*the delay between VDD and MCLK*/
	usleep_range(50*1000, 55*1000);

	if (s_ctrl->clk_rate != 0)
		clk_info->clk_rate = s_ctrl->clk_rate;
	rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(clk_info), 1);
	if (rc < 0) {
		pr_err("%s: clk enable failed\n", __func__);
		goto enable_clk_failed;
	}

	rc = msm_camera_config_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: config gpio failed\n", __func__);
		goto config_gpio_failed;
	}

	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(1);

	if (data->sensor_platform_info->i2c_conf &&
		data->sensor_platform_info->i2c_conf->use_i2c_mux)
		msm_sensor_enable_i2c_mux(data->sensor_platform_info->i2c_conf);

	return rc;

config_gpio_failed:
	msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(clk_info), 0);
enable_clk_failed:
	msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
		s_ctrl->sensordata->sensor_platform_info->cam_vreg,
		s_ctrl->sensordata->sensor_platform_info->num_vreg,
		s_ctrl->reg_ptr, 0);
enable_vreg_failed:
	msm_camera_config_vreg(&s_ctrl->sensor_i2c_client->client->dev,
		s_ctrl->sensordata->sensor_platform_info->cam_vreg,
		s_ctrl->sensordata->sensor_platform_info->num_vreg,
		s_ctrl->reg_ptr, 0);
config_vreg_failed:
	msm_camera_request_gpio_table(data, 0);
request_gpio_failed:
	kfree(s_ctrl->reg_ptr);
	return rc;
}

static const struct i2c_device_id mt9t113_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&mt9t113_s_ctrl},
	{ }
};

static struct i2c_driver mt9t113_i2c_driver = {
	.id_table = mt9t113_i2c_id,
	.probe  =  msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client mt9t113_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
	.addr_pos = 0,
	.addr_dir = 0,  
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&mt9t113_i2c_driver);
}

static struct v4l2_subdev_core_ops mt9t113_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops mt9t113_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops mt9t113_subdev_ops = {
	.core = &mt9t113_subdev_core_ops,
	.video  = &mt9t113_subdev_video_ops,
};

static struct msm_sensor_fn_t mt9t113_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_write_exp_gain = mt9t113_write_prev_exp_gain,
	.sensor_write_snapshot_exp_gain = mt9t113_write_pict_exp_gain,
	.sensor_csi_setting = mt9t113_sensor_setting,
	.sensor_write_init_settings = mt9t113_write_init_settings,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = mt9t113_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_set_wb = mt9t113_sensor_set_wb,
	.sensor_set_effect = mt9t113_sensor_set_effect,
};

static struct msm_sensor_reg_t mt9t113_regs = {
	.default_data_type = MSM_CAMERA_I2C_WORD_DATA,
	.start_stream_conf_size = 0,
	.stop_stream_conf_size = 0,
	.group_hold_on_conf_size = 0,
	.group_hold_off_conf_size = 0,
	.init_settings = &mt9t113_init_conf[0],
	.init_size = ARRAY_SIZE(mt9t113_init_conf),
	.mode_settings = &mt9t113_confs[0],
	.output_settings = &mt9t113_dimensions[0],
	.num_conf = ARRAY_SIZE(mt9t113_confs),
};

static struct msm_sensor_ctrl_t mt9t113_s_ctrl = {
	.msm_sensor_reg = &mt9t113_regs,
	.sensor_i2c_client = &mt9t113_sensor_i2c_client,
	.sensor_i2c_addr = 0x78,
	.sensor_id_info = &mt9t113_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &mt9t113_csi_params_array[0],
	.msm_sensor_mutex = &mt9t113_mut,
	.sensor_i2c_driver = &mt9t113_i2c_driver,
	.sensor_v4l2_subdev_info = mt9t113_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9t113_subdev_info),
	.sensor_v4l2_subdev_ops = &mt9t113_subdev_ops,
	.func_tbl = &mt9t113_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_48HZ,
	.sensor_name = "23060073FF-MT-B",
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Aptina 3MP YUV sensor driver");
MODULE_LICENSE("GPL v2");
