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
#define SENSOR_NAME "mt9e013"
#define PLATFORM_DRIVER_NAME "msm_camera_mt9e013"
#define mt9e013_obj mt9e013_##obj

#include "./msm.h"
#include "./actuators/msm_actuator.h"
#define MODEL_LITEON 1
#define MODEL_SUNNY 2
static uint16_t mt9e013_model_id = MODEL_LITEON;
static bool OTP_READ = FALSE;
DEFINE_MUTEX(mt9e013_mut);
static struct msm_sensor_ctrl_t mt9e013_s_ctrl;

static struct msm_camera_i2c_reg_conf mt9e013_groupon_settings[] = {
	{0x0104, 0x01},
};

static struct msm_camera_i2c_reg_conf mt9e013_groupoff_settings[] = {
	{0x0104, 0x00},
};

/*modify the initialization settings according to ICS*/
static struct msm_camera_i2c_reg_conf mt9e013_prev_settings[] = {
	/*Output Size (1632x1224)*/
	{0x0344, 0x0008},/*X_ADDR_START*/
	{0x0348, 0x0CC9},/*X_ADDR_END*/
	{0x0346, 0x0008},/*Y_ADDR_START*/
	{0x034A, 0x0999},/*Y_ADDR_END*/
	{0x034C, 0x0660},/*X_OUTPUT_SIZE*/
	{0x034E, 0x04C8},/*Y_OUTPUT_SIZE*/
	{0x306E, 0xFCB0},/*DATAPATH_SELECT*/
	{0x3040, 0xC4C3},/*READ_MODE*/
	{0x3178, 0x0000},/*ANALOG_CONTROL5*/
	{0x3ED0, 0x1E24},/*DAC_LD_4_5*/
	{0x0400, 0x0002},/*SCALING_MODE*/
	{0x0404, 0x0010},/*SCALE_M*/
	/*Timing configuration*/
	{0x0342, 0x1018},/*LINE_LENGTH_PCK*/
	{0x0340, 0x055B},/*FRAME_LENGTH_LINES*/
	{0x0202, 0x0557},/*COARSE_INTEGRATION_TIME*/
	{0x3014, 0x0846},/*FINE_INTEGRATION_TIME_*/
	{0x3010, 0x0130},/*FINE_CORRECTION*/
};

static struct msm_camera_i2c_reg_conf mt9e013_snap_settings[] = {
	/*Output Size (3264x2448)*/
	{0x0344, 0x0000},/*X_ADDR_START */
	{0x0348, 0x0CCF},/*X_ADDR_END*/
	{0x0346, 0x0000},/*Y_ADDR_START */
	{0x034A, 0x099F},/*Y_ADDR_END*/
	{0x034C, 0x0CD0},/*X_OUTPUT_SIZE*/
	{0x034E, 0x09A0},/*Y_OUTPUT_SIZE*/
	{0x306E, 0xFC80},/*DATAPATH_SELECT*/
	{0x3040, 0xC041},/*READ_MODE*/
	{0x3178, 0x0000},/*ANALOG_CONTROL5*/
	{0x3ED0, 0x1E24},/*DAC_LD_4_5*/
	{0x0400, 0x0000},/*SCALING_MODE*/
	{0x0404, 0x0010},/*SCALE_M*/
	/*Timing configuration*/
	{0x0342, 0x13F8},/*LINE_LENGTH_PCK*/
	{0x0340, 0x0A2F},/*FRAME_LENGTH_LINES*/
	{0x0202, 0x0A1F},/*COARSE_INTEGRATION_TIME*/
	{0x3014, 0x03F6},/*FINE_INTEGRATION_TIME_ */
	{0x3010, 0x0078},/*FINE_CORRECTION*/
};

static struct msm_camera_i2c_reg_conf mt9e013_hfr60_settings[] = {
	{0x0300, 0x0005},/*VT_PIX_CLK_DIV*/
	{0x0302, 0x0001},/*VT_SYS_CLK_DIV*/
	{0x0304, 0x0002},/*PRE_PLL_CLK_DIV*/
	{0x0306, 0x0029},/*PLL_MULTIPLIER*/
	{0x0308, 0x000A},/*OP_PIX_CLK_DIV*/
	{0x030A, 0x0001},/*OP_SYS_CLK_DIV*/
	{0x0344, 0x0008},/*X_ADDR_START*/
	{0x0348, 0x0685},/*X_ADDR_END*/
	{0x0346, 0x013a},/*Y_ADDR_START*/
	{0x034A, 0x055B},/*Y_ADDR_END*/
	{0x034C, 0x0340},/*X_OUTPUT_SIZE*/
	{0x034E, 0x0212},/*Y_OUTPUT_SIZE*/
	{0x306E, 0xFC80},/*DATAPATH_SELECT*/
	{0x3040, 0xC0C3},/*READ_MODE*/
	{0x3178, 0x0000},/*ANALOG_CONTROL5*/
	{0x3ED0, 0x1E24},/*DAC_LD_4_5*/
	{0x0400, 0x0000},/*SCALING_MODE*/
	{0x0404, 0x0010},/*SCALE_M*/
	/*Timing configuration*/
	{0x0342, 0x0970},/*LINE_LENGTH_PCK*/
	{0x0340, 0x02A1},/*FRAME_LENGTH_LINES*/
	{0x0202, 0x02A1},/*COARSE_INTEGRATION_TIME*/
	{0x3014, 0x03F6},/*FINE_INTEGRATION_TIME_*/
	{0x3010, 0x0078},/*FINE_CORRECTION*/
};

static struct msm_camera_i2c_reg_conf mt9e013_hfr90_settings[] = {
	{0x0300, 0x0005},/*VT_PIX_CLK_DIV*/
	{0x0302, 0x0001},/*VT_SYS_CLK_DIV*/
	{0x0304, 0x0002},/*PRE_PLL_CLK_DIV*/
	{0x0306, 0x003D},/*PLL_MULTIPLIER*/
	{0x0308, 0x000A},/*OP_PIX_CLK_DIV*/
	{0x030A, 0x0001},/*OP_SYS_CLK_DIV*/
	{0x0344, 0x0008},/*X_ADDR_START*/
	{0x0348, 0x0685},/*X_ADDR_END*/
	{0x0346, 0x013a},/*Y_ADDR_START*/
	{0x034A, 0x055B},/*Y_ADDR_END*/
	{0x034C, 0x0340},/*X_OUTPUT_SIZE*/
	{0x034E, 0x0212},/*Y_OUTPUT_SIZE*/
	{0x306E, 0xFC80},/*DATAPATH_SELECT*/
	{0x3040, 0xC0C3},/*READ_MODE*/
	{0x3178, 0x0000},/*ANALOG_CONTROL5*/
	{0x3ED0, 0x1E24},/*DAC_LD_4_5*/
	{0x0400, 0x0000},/*SCALING_MODE*/
	{0x0404, 0x0010},/*SCALE_M*/
	/*Timing configuration*/
	{0x0342, 0x0970},/*LINE_LENGTH_PCK*/
	{0x0340, 0x02A1},/*FRAME_LENGTH_LINES*/
	{0x0202, 0x02A1},/*COARSE_INTEGRATION_TIME*/
	{0x3014, 0x03F6},/*FINE_INTEGRATION_TIME_*/
	{0x3010, 0x0078},/*FINE_CORRECTION*/
};

static struct msm_camera_i2c_reg_conf mt9e013_hfr120_settings[] = {
	{0x0300, 0x0005},/*VT_PIX_CLK_DIV*/
	{0x0302, 0x0001},/*VT_SYS_CLK_DIV*/
	{0x0304, 0x0002},/*PRE_PLL_CLK_DIV*/
	{0x0306, 0x0052},/*PLL_MULTIPLIER*/
	{0x0308, 0x000A},/*OP_PIX_CLK_DIV*/
	{0x030A, 0x0001},/*OP_SYS_CLK_DIV*/
	{0x0344, 0x0008},/*X_ADDR_START*/
	{0x0348, 0x0685},/*X_ADDR_END*/
	{0x0346, 0x013a},/*Y_ADDR_START*/
	{0x034A, 0x055B},/*Y_ADDR_END*/
	{0x034C, 0x0340},/*X_OUTPUT_SIZE*/
	{0x034E, 0x0212},/*Y_OUTPUT_SIZE*/
	{0x306E, 0xFC80},/*DATAPATH_SELECT*/
	{0x3040, 0xC0C3},/*READ_MODE*/
	{0x3178, 0x0000},/*ANALOG_CONTROL5*/
	{0x3ED0, 0x1E24},/*DAC_LD_4_5*/
	{0x0400, 0x0000},/*SCALING_MODE*/
	{0x0404, 0x0010},/*SCALE_M*/
	/*Timing configuration*/
	{0x0342, 0x0970},/*LINE_LENGTH_PCK*/
	{0x0340, 0x02A1},/*FRAME_LENGTH_LINES*/
	{0x0202, 0x02A1},/*COARSE_INTEGRATION_TIME*/
	{0x3014, 0x03F6},/*FINE_INTEGRATION_TIME_*/
	{0x3010, 0x0078},/*FINE_CORRECTION*/
};

static struct msm_camera_i2c_reg_conf mt9e013_recommend_settings_mipi[] = {
	/*Disable embedded data*/
	{0x3064, 0x7800},/*SMIA_TEST*/
	/*configure 2-lane MIPI*/
	{0x31AE, 0x0202},/*SERIAL_FORMAT*/
	{0x31B8, 0x0E3F},/*MIPI_TIMING_2*/
	/*set data to RAW10 format*/
	{0x0112, 0x0A0A},/*CCP_DATA_FORMAT*/
	{0x30F0, 0x8000},/*VCM CONTROL*/
};
static struct msm_camera_i2c_reg_conf mt9e013_recommend_settings[] = {
	{0x3044, 0x0590},
	{0x306E, 0xFC80},
	{0x30B2, 0xC000},
	{0x30D6, 0x0800},
	{0x316C, 0xB42F},
	{0x316E, 0x869C},
	{0x3170, 0x210E},
	{0x317A, 0x010E},
	{0x31E0, 0x1FB9},
	{0x31E6, 0x07FC},
	{0x37C0, 0x0000},
	{0x37C2, 0x0000},
	{0x37C4, 0x0000},
	{0x37C6, 0x0000},
	{0x3E02, 0x8801},
	{0x3E04, 0x2301},
	{0x3E06, 0x8449},
	{0x3E08, 0x6841},
	{0x3E0A, 0x400C},
	{0x3E0C, 0x1001},
	{0x3E0E, 0x2103},
	{0x3E10, 0x4B41},
	{0x3E12, 0x4B26},
	{0x3E16, 0x8802},
	{0x3E18, 0x84FF},
	{0x3E1A, 0x8601},
	{0x3E1C, 0x8401},
	{0x3E1E, 0x840A},
	{0x3E20, 0xFF00},
	{0x3E22, 0x8401},
	{0x3E24, 0x00FF},
	{0x3E26, 0x0088},
	{0x3E28, 0x2E8A},
	{0x3E32, 0x8801},
	{0x3E34, 0x4024},
	{0x3E38, 0x8469},
	{0x3E3C, 0x2301},
	{0x3E3E, 0x3E25},
	{0x3E40, 0x1C01},
	{0x3E42, 0x8486},
	{0x3E44, 0x8401},
	{0x3E46, 0x00FF},
	{0x3E48, 0x8401},
	{0x3E4A, 0x8601},
	{0x3E4C, 0x8402},
	{0x3E4E, 0x00FF},
	{0x3E50, 0x6623},
	{0x3E52, 0x8340},
	{0x3E54, 0x00FF},
	{0x3E56, 0x4A42},
	{0x3E58, 0x2203},
	{0x3E5A, 0x674D},
	{0x3E5C, 0x3F25},
	{0x3E5E, 0x846A},
	{0x3E60, 0x4C01},
	{0x3E62, 0x8401},
	{0x3E66, 0x3901},
	{0x3ECC, 0x00EB},
	{0x3ED0, 0x1E24},
	{0x3ED4, 0xAFC4},
	{0x3ED6, 0x909B},
	{0x3ED8, 0x0006},
	{0x3EDA, 0xCFC6},
	{0x3EDC, 0x4FE4},
	{0x3EE0, 0x2424},
	//{0x3EE2, 0x9797},
	{0x3EE4, 0xC100},
	{0x3EE6, 0x0540},
};
static struct msm_camera_i2c_reg_conf mt9e013_recommend_settings_shading[] = {
	/*delete one line*/
	{0x3600, 0x0370},
	{0x3602, 0x102E},
	{0x3604, 0x30D0},
	{0x3606, 0xAC2D},
	{0x3608, 0xB950},
	{0x360A, 0x0370},
	{0x360C, 0x88AE},
	{0x360E, 0x1130},
	{0x3610, 0x518E},
	{0x3612, 0x8590},
	{0x3614, 0x03D0},
	{0x3616, 0x344E},
	{0x3618, 0x388F},
	{0x361A, 0x954E},
	{0x361C, 0xBF2F},
	{0x361E, 0x02F0},
	{0x3620, 0xA02E},
	{0x3622, 0x30D0},
	{0x3624, 0x3F0E},
	{0x3626, 0xBE90},
	{0x3640, 0xDC2D},
	{0x3642, 0x926E},
	{0x3644, 0x316A},
	{0x3646, 0x1DEE},
	{0x3648, 0x3DCC},
	{0x364A, 0x964D},
	{0x364C, 0x16CE},
	{0x364E, 0x0A8B},
	{0x3650, 0xD88E},
	{0x3652, 0x186C},
	{0x3654, 0x1ECD},
	{0x3656, 0x656E},
	{0x3658, 0x38EE},
	{0x365A, 0x972F},
	{0x365C, 0xB10F},
	{0x365E, 0x7EEC},
	{0x3660, 0xAB8E},
	{0x3662, 0x772E},
	{0x3664, 0x61AE},
	{0x3666, 0xE2AF},
	{0x3680, 0x0471},
	{0x3682, 0x3F2E},
	{0x3684, 0x8C53},
	{0x3686, 0xA6EF},
	{0x3688, 0x3493},
	{0x368A, 0x0AB1},
	{0x368C, 0xA1CD},
	{0x368E, 0xCF32},
	{0x3690, 0xB68D},
	{0x3692, 0x7BF2},
	{0x3694, 0x70D0},
	{0x3696, 0x0A4F},
	{0x3698, 0xD2D2},
	{0x369A, 0xB80F},
	{0x369C, 0x0533},
	{0x369E, 0x0471},
	{0x36A0, 0x024D},
	{0x36A2, 0x89D3},
	{0x36A4, 0x8EAF},
	{0x36A6, 0x2EF3},
	{0x36C0, 0x744D},
	{0x36C2, 0x4EAE},
	{0x36C4, 0x904D},
	{0x36C6, 0xB90F},
	{0x36C8, 0xB290},
	{0x36CA, 0x42CE},
	{0x36CC, 0xDF0E},
	{0x36CE, 0xC78F},
	{0x36D0, 0x0470},
	{0x36D2, 0x9A2F},
	{0x36D4, 0x948B},
	{0x36D6, 0xAA6F},
	{0x36D8, 0xA2D0},
	{0x36DA, 0x1570},
	{0x36DC, 0x5830},
	{0x36DE, 0x612B},
	{0x36E0, 0x70EE},
	{0x36E2, 0xE410},
	{0x36E4, 0xD9AF},
	{0x36E6, 0x16B1},
	{0x3700, 0xDC71},
	{0x3702, 0x9CB0},
	{0x3704, 0x5E93},
	{0x3706, 0x15F1},
	{0x3708, 0xAF53},
	{0x370A, 0xCE71},
	{0x370C, 0x420F},
	{0x370E, 0x1FB3},
	{0x3710, 0xCA90},
	{0x3712, 0xAAB2},
	{0x3714, 0xD151},
	{0x3716, 0xE570},
	{0x3718, 0x3353},
	{0x371A, 0x4431},
	{0x371C, 0xF092},
	{0x371E, 0xD851},
	{0x3720, 0x45EE},
	{0x3722, 0x56D3},
	{0x3724, 0x8E50},
	{0x3726, 0x9FB3},
	{0x3782, 0x062C},
	{0x3784, 0x0494},
	{0x37C0, 0x8FCB},
	{0x37C2, 0xE20A},
	{0x37C4, 0xDB8A},
	{0x37C6, 0x822B},
	/*delete one line*/
};
static struct msm_camera_i2c_reg_conf mt9e013_recommend_settings_pll[] = {
	/* PLL settings */
	{0x0300, 0x0004},/*VT_PIX_CLK_DIV*/
	{0x0302, 0x0001},/*VT_SYS_CLK_DIV*/
	{0x0304, 0x0002},/*PRE_PLL_CLK_DIV*/
	{0x0306, 0x003A},/*PLL_MULTIPLIER*/
	{0x0308, 0x000A},/*OP_PIX_CLK_DIV*/
	{0x030A, 0x0001},/*OP_SYS_CLK_DIV*/
};
/*OTP array*/
static struct msm_camera_i2c_reg_conf mt9e013_OTP_settings[] = {
	{0x3800, 0x0000},
	{0x3802, 0x0000},
	{0x3804, 0x0000},
	{0x3806, 0x0000},
	{0x3808, 0x0000},
	{0x380a, 0x0000},
	{0x380c, 0x0000},
	{0x380e, 0x0000},
	{0x3810, 0x0000},
	{0x3812, 0x0000},
	{0x3814, 0x0000},
	{0x3816, 0x0000},
	{0x3818, 0x0000},
	{0x381a, 0x0000},
	{0x381c, 0x0000},
	{0x381e, 0x0000},
	{0x3820, 0x0000},
	{0x3822, 0x0000},
	{0x3824, 0x0000},
	{0x3826, 0x0000},
	{0x3828, 0x0000},
	{0x382a, 0x0000},
	{0x382c, 0x0000},
	{0x382e, 0x0000},
	{0x3830, 0x0000},
	{0x3832, 0x0000},
	{0x3834, 0x0000},
	{0x3836, 0x0000},
	{0x3838, 0x0000},
	{0x383a, 0x0000},
	{0x383c, 0x0000},
	{0x383e, 0x0000},
	{0x3840, 0x0000},
	{0x3842, 0x0000},
	{0x3844, 0x0000},
	{0x3846, 0x0000},
	{0x3848, 0x0000},
	{0x384a, 0x0000},
	{0x384c, 0x0000},
	{0x384e, 0x0000},
	{0x3850, 0x0000},
	{0x3852, 0x0000},
	{0x3854, 0x0000},
	{0x3856, 0x0000},
	{0x3858, 0x0000},
	{0x385a, 0x0000},
	{0x385c, 0x0000},
	{0x385e, 0x0000},
	{0x3860, 0x0000},
	{0x3862, 0x0000},
	{0x3864, 0x0000},
	{0x3866, 0x0000},
	{0x3868, 0x0000},
	{0x386a, 0x0000},
	{0x386c, 0x0000},
	{0x386e, 0x0000},
	{0x3870, 0x0000},
	{0x3872, 0x0000},
	{0x3874, 0x0000},
	{0x3876, 0x0000},
	{0x3878, 0x0000},
	{0x387a, 0x0000},
	{0x387c, 0x0000},
	{0x387e, 0x0000},
	{0x3880, 0x0000},
	{0x3882, 0x0000},
	{0x3884, 0x0000},
	{0x3886, 0x0000},
	{0x3888, 0x0000},
	{0x388a, 0x0000},
	{0x388c, 0x0000},
	{0x388e, 0x0000},
	{0x3890, 0x0000},
	{0x3892, 0x0000},
	{0x3894, 0x0000},
	{0x3896, 0x0000},
	{0x3898, 0x0000},
	{0x389a, 0x0000},
	{0x389c, 0x0000},
	{0x389e, 0x0000},
	{0x38a0, 0x0000},
	{0x38a2, 0x0000},
	{0x38a4, 0x0000},
	{0x38a6, 0x0000},
	{0x38a8, 0x0000},
	{0x38aa, 0x0000},
	{0x38ac, 0x0000},
	{0x38ae, 0x0000},
	{0x38b0, 0x0000},
	{0x38b2, 0x0000},
	{0x38b4, 0x0000},
	{0x38b6, 0x0000},
	{0x38b8, 0x0000},
	{0x38ba, 0x0000},
	{0x38bc, 0x0000},
	{0x38be, 0x0000},
	{0x38c0, 0x0000},
	{0x38c2, 0x0000},
	{0x38c4, 0x0000},
	{0x38c6, 0x0000},
	{0x38c8, 0x0000},
	{0x38ca, 0x0000},
	{0x38cc, 0x0000},
	{0x38ce, 0x0000},
	{0x38d0, 0x0000},
	{0x38d2, 0x0000},
	{0x38d4, 0x0000},
	{0x38d6, 0x0000},
	{0x38d8, 0x0000},
	{0x38da, 0x0000},
	{0x38dc, 0x0000},
	{0x38de, 0x0000},
	{0x38e0, 0x0000},
	{0x38e2, 0x0000},
	{0x38e4, 0x0000},
	{0x38e6, 0x0000},
	{0x38e8, 0x0000},
	{0x38ea, 0x0000},
	{0x38ec, 0x0000},
	{0x38ee, 0x0000},
	{0x38f0, 0x0000},
	{0x38f2, 0x0000},
	{0x38f4, 0x0000},
	{0x38f6, 0x0000}
};

static struct v4l2_subdev_info mt9e013_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array mt9e013_init_conf[] = {
	{&mt9e013_recommend_settings_mipi[0],
	ARRAY_SIZE(mt9e013_recommend_settings_mipi), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9e013_recommend_settings[0],
	ARRAY_SIZE(mt9e013_recommend_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9e013_recommend_settings_shading[0],
	ARRAY_SIZE(mt9e013_recommend_settings_shading), 0, MSM_CAMERA_I2C_WORD_DATA},	
	{&mt9e013_recommend_settings_pll[0],
	ARRAY_SIZE(mt9e013_recommend_settings_pll), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9e013_confs[] = {
	{&mt9e013_snap_settings[0],
	ARRAY_SIZE(mt9e013_snap_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9e013_prev_settings[0],
	ARRAY_SIZE(mt9e013_prev_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9e013_hfr60_settings[0],
	ARRAY_SIZE(mt9e013_hfr60_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9e013_hfr90_settings[0],
	ARRAY_SIZE(mt9e013_hfr90_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9e013_hfr120_settings[0],
	ARRAY_SIZE(mt9e013_hfr120_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_sensor_output_info_t mt9e013_dimensions[] = {
	{
		.x_output = 0xCD0,
		.y_output = 0x9A0,
		.line_length_pclk = 0x13F8,
		.frame_length_lines = 0xA2F,
		.vt_pixel_clk = 174000000,
		.op_pixel_clk = 174000000,
		.binning_factor = 1,
	},
	{
		.x_output = 0x660,
		.y_output = 0x4C8,
		.line_length_pclk = 0x1018,
		.frame_length_lines = 0x55B,
		.vt_pixel_clk = 174000000,
		.op_pixel_clk = 174000000,
		.binning_factor = 1,
	},
	{
		.x_output = 0x340,
		.y_output = 0x212,
		.line_length_pclk = 0x970,
		.frame_length_lines = 0x2A1,
		.vt_pixel_clk = 98400000,
		.op_pixel_clk = 98400000,
		.binning_factor = 1,
	},
	{
		.x_output = 0x340,
		.y_output = 0x212,
		.line_length_pclk = 0x970,
		.frame_length_lines = 0x2A1,
		.vt_pixel_clk = 146400000,
		.op_pixel_clk = 146400000,
		.binning_factor = 1,
	},
	{
		.x_output = 0x340,
		.y_output = 0x212,
		.line_length_pclk = 0x970,
		.frame_length_lines = 0x2A1,
		.vt_pixel_clk = 196800000,
		.op_pixel_clk = 196800000,
		.binning_factor = 1,
	},
};

static struct msm_camera_csi_params mt9e013_csi_params = {
	.data_format = CSI_10BIT,
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x18,
};

static struct msm_camera_csi_params *mt9e013_csi_params_array[] = {
	&mt9e013_csi_params,
	&mt9e013_csi_params,
	&mt9e013_csi_params,
	&mt9e013_csi_params,
	&mt9e013_csi_params,
};

static struct msm_sensor_output_reg_addr_t mt9e013_reg_addr = {
	.x_output = 0x34C,
	.y_output = 0x34E,
	.line_length_pclk = 0x342,
	.frame_length_lines = 0x340,
};

static struct msm_sensor_id_info_t mt9e013_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x4B00,
};

static struct msm_sensor_exp_gain_info_t mt9e013_exp_gain_info = {
	.coarse_int_time_addr = 0x202,
	.global_gain_addr = 0x305E,
	.vert_offset = 0,
};
#define MT9E013_FRAMELINE_INDEX  13
/*use the set fps function instead of defaut function*/
static int32_t mt9e013_set_fps(struct msm_sensor_ctrl_t *s_ctrl,struct fps_cfg *fps)
{
	uint16_t total_lines_per_frame = 0;
	int32_t rc = 0;
	enum msm_sensor_resolution_t res = 0;

	s_ctrl->fps_divider = fps->fps_div;
	printk("%s fps_divider=%d\n",__func__,s_ctrl->fps_divider);
	if(s_ctrl->curr_res == MSM_SENSOR_INVALID_RES)
	{
		res = MSM_SENSOR_RES_QTR;
	}
	else
	{
		res = s_ctrl->curr_res;
	}

	s_ctrl->curr_frame_length_lines = s_ctrl->msm_sensor_reg->output_settings[res].frame_length_lines;
	
	total_lines_per_frame = (uint16_t)
		((s_ctrl->curr_frame_length_lines * s_ctrl->fps_divider) / Q10);

	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);

	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			total_lines_per_frame,
			MSM_CAMERA_I2C_WORD_DATA);

	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);

	if(res == MSM_SENSOR_RES_QTR)
	{
		mt9e013_prev_settings[MT9E013_FRAMELINE_INDEX].reg_data = total_lines_per_frame;
	}

	printk("%s: res = %d, curr_frame_length_lines = %d, total_lines_per_frame = %d\n",__func__,res,s_ctrl->curr_frame_length_lines,total_lines_per_frame);

	return rc;
}

static int32_t mt9e013_write_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	uint32_t fl_lines;
	fl_lines =
		(s_ctrl->curr_frame_length_lines * s_ctrl->fps_divider) / Q10;

	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr, gain | 0x1000,
		MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr, line,
		MSM_CAMERA_I2C_WORD_DATA);
	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	if(is_first_preview_frame)
	{
		msleep(50);
		is_first_preview_frame = 0;
	}
	return 0;
}

static int32_t mt9e013_write_exp_snapshot_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	uint32_t fl_lines;
	fl_lines =
		(s_ctrl->curr_frame_length_lines * s_ctrl->fps_divider) / Q10;

	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr, gain | 0x1000,
		MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr, line,
		MSM_CAMERA_I2C_WORD_DATA);
	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x301A, (0x065C|0x2), MSM_CAMERA_I2C_WORD_DATA);

	return 0;
}
static void mt9e013_start_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x301A, 0x8250, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x301A, 0x8650, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x301A, 0x8658, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x0104, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x301A, 0x065C, MSM_CAMERA_I2C_WORD_DATA);
}

static void mt9e013_stop_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x301A, 0x0058, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x301A, 0x0050, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x0104, 0x01, MSM_CAMERA_I2C_BYTE_DATA);
}

static int32_t mt9e013_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;

	struct msm_cam_media_controller *pmctl = NULL;
	struct msm_actuator_ctrl_t *a_ctrl = NULL;
	int16_t stored_step_pos=-1;
	extern uint32_t my_mctl_handle;

	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
		printk("Register INIT\n");
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
		csi_config = 0;
		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		printk("PERIODIC : %d\n", res);

		/* here we need to get the actuator ctrl to do something to actuator
		 * before writing registers of preview or snapshot, move the actuator
		 * to init position 0, after sensor switched mode successfully, move 
		 * to original position to get clear image
		 */
		pmctl = msm_camera_get_mctl(my_mctl_handle);
		a_ctrl = get_actrl(pmctl->act_sdev);
		stored_step_pos= a_ctrl->curr_step_pos;
		printk("nowt the pos is %d\n", stored_step_pos);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			0x30F2, 0,
			MSM_CAMERA_I2C_WORD_DATA);

		s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
		msm_sensor_write_conf_array(
			s_ctrl->sensor_i2c_client,
			s_ctrl->msm_sensor_reg->mode_settings, res);
		msleep(30);
		if (!csi_config) {
			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
			CDBG("CSI config in progress\n");
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIC_CFG,
				s_ctrl->curr_csic_params);
			CDBG("CSI config is done\n");
			mb();
			msleep(30);
			csi_config = 1;
		}
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE,
			&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);
		
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			0x30F2, a_ctrl->step_position_table[stored_step_pos],
			MSM_CAMERA_I2C_WORD_DATA);
		
		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		msleep(50);
		if(res == MSM_SENSOR_RES_QTR)
		    is_first_preview_frame = 1;
	}
	return rc;
}
/*read OTP value function*/
inline int32_t reading(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = -1;
	unsigned short i = 0;
	unsigned short addr = 0x3800;
	unsigned short value = 0;

	//read the value and put them in the mt9e013_OTP_settings array.
	for( i=0;i<(ARRAY_SIZE(mt9e013_OTP_settings)-8);i++)
	{
		addr = 0x3800 +2 * i;
		rc = msm_camera_i2c_read(
					s_ctrl->sensor_i2c_client,
					addr, &value,
					MSM_CAMERA_I2C_WORD_DATA);
		if (rc < 0)
		{
			printk("%s: can't read %0x address \n", __func__, addr);
		}
		else
		{
			mt9e013_OTP_settings[i].reg_data = value;
		}
	}
	return rc;
}

static int32_t Auto_reading(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t j = 0;
	int32_t bsuccess = -1;
	unsigned short value = 0;

	for(j = 0; j<5; j++)//POLL Register 0x304A [6:5] = 11 //auto read success
		{
			msleep(5);
			msm_camera_i2c_read(
					s_ctrl->sensor_i2c_client,
					0x304A, &value,
					MSM_CAMERA_I2C_WORD_DATA);
		if(0xFFFF == (value |0xFF9F))//finish
		{
			bsuccess =1;
			CDBG("mt9e013_OTP_reading reading bsuccess = %dstart! \n",bsuccess);
			break;
		}
	}
	if(1 == bsuccess)
		reading(s_ctrl);
	
	return bsuccess;
}


static int32_t mt9e013_OTP_reading (struct msm_sensor_ctrl_t *s_ctrl)
{	
	int32_t rc = -1;

	//get the type that we have the params
	//Do the OTP reading, From Type 35 to 30. read the data from the right type
	CDBG("mt9e013_OTP_reading reading start! \n");
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x301A, 0x0610, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3134, 0xCD95, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x304C, 0x3500, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x304A, 0x0010, MSM_CAMERA_I2C_WORD_DATA);
	rc = Auto_reading(s_ctrl);
	if(rc > 0)
	{
		CDBG("the right type is 35\n");
		goto otp_probe_check;
	}

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x304C, 0x3400, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x304A, 0x0010, MSM_CAMERA_I2C_WORD_DATA);
	rc = Auto_reading(s_ctrl);
	if(rc > 0)
	{
		CDBG("the right type is 34\n");
		goto otp_probe_check;
	}

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x304C, 0x3300, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x304A, 0x0010, MSM_CAMERA_I2C_WORD_DATA);
	rc = Auto_reading(s_ctrl);
	if(rc > 0)
	{
		CDBG("the right type is 33\n");
		goto otp_probe_check;
	}

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x304C, 0x3200, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x304A, 0x0010, MSM_CAMERA_I2C_WORD_DATA);
	rc = Auto_reading(s_ctrl);
	if(rc > 0)
	{
		CDBG("the right type is 32\n");
		goto otp_probe_check;
	}

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x304C, 0x3100, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x304A, 0x0010, MSM_CAMERA_I2C_WORD_DATA);
	rc = Auto_reading(s_ctrl);
	if(rc > 0)
	{
		CDBG("the right type is 31\n");
		goto otp_probe_check;
	}

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x304C, 0x3000, MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x304A, 0x0010, MSM_CAMERA_I2C_WORD_DATA);
	rc = Auto_reading(s_ctrl);
	if(rc > 0)
	{
		CDBG("the right type is 30\n");
		goto otp_probe_check;
	}

otp_probe_check:
	if(rc < 0)
	{
		printk("The OTP reading failed\n");
		return rc;
	}

	//This is for OTP verify. check the 0x3800's first value is 2
	//check the AWB data is OK level
	//check the 0x38e2 is 0xFFFF
	if(0 != mt9e013_OTP_settings[7].reg_data)
	{
		CDBG("The OTP reading failed addr = %0x, data = %0x\n", mt9e013_OTP_settings[7].reg_addr, mt9e013_OTP_settings[7].reg_data);
		rc = -1;
		return rc;
	}
	//check the 0x38e2 is 0xFFFF
	if(0xFFFF != mt9e013_OTP_settings[114].reg_data)
	{
		CDBG("The OTP reading failed addr = %0x, data = %0x\n", mt9e013_OTP_settings[114].reg_addr, mt9e013_OTP_settings[114].reg_data);
		rc = -1;
		return rc;
	}
	//if OTP is right, we will set the OTP_READ to TRUE
	
	OTP_READ = TRUE;
	CDBG("The OTP reading success\n");
	return rc;
}

int32_t mt9e013_sensor_model_match(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc=0;

	/*read and write some otp before model read*/
	mt9e013_OTP_reading(s_ctrl);
	rc = msm_camera_i2c_read(
				s_ctrl->sensor_i2c_client,
				0x3806, &mt9e013_model_id,
				MSM_CAMERA_I2C_WORD_DATA);
	printk("mt9e013 model is :0x%x \n", mt9e013_model_id);
	mt9e013_model_id = (mt9e013_model_id & 0xF000) >> 12;

	if(MODEL_SUNNY == mt9e013_model_id)
	{
		strncpy((char *)s_ctrl->sensor_name, "23060068FA-MT-S", strlen("23060068FA-MT-S"));
	}
	else
	{
		strncpy((char *)s_ctrl->sensor_name, "23060068FA-MT-L", strlen("23060068FA-MT-L"));
	}
	printk("mt9e013.c name is %s \n", s_ctrl->sensor_name);
	
	return rc;
}
int32_t mt9e013_shading_setting(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	int32_t i=0;
	CDBG("mt9e013_shading_setting write start!reg_shading_size=%d \n", ARRAY_SIZE(mt9e013_recommend_settings_shading));

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3780, 0,MSM_CAMERA_I2C_WORD_DATA);
	/*write our default array not OTP*/
	if((0 == mt9e013_OTP_settings[8].reg_data) || (FALSE == OTP_READ))
	{
		CDBG("shading invalid, write the default\n");
		rc = msm_camera_i2c_write_tbl(
				s_ctrl->sensor_i2c_client,
				&mt9e013_recommend_settings_shading[0],
				ARRAY_SIZE(mt9e013_recommend_settings_shading), 
				MSM_CAMERA_I2C_WORD_DATA);
		return rc;
	}
	
	/*write the otp array we read from sensor*/
	for(i=0;i< ARRAY_SIZE(mt9e013_recommend_settings_shading);i++)
	{
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
					mt9e013_recommend_settings_shading[i].reg_addr, mt9e013_OTP_settings[8+i].reg_data,
					MSM_CAMERA_I2C_WORD_DATA);
		if(rc < 0)
		{
			CDBG("Write shading register failed!address = %0x, wirte again!\n",mt9e013_recommend_settings_shading[i].reg_addr);
			msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
					mt9e013_recommend_settings_shading[i].reg_addr, mt9e013_OTP_settings[8+i].reg_data,
					MSM_CAMERA_I2C_WORD_DATA);
		}
		else
		{
			CDBG("address = %0x, value = %0x sucess\n",
				mt9e013_recommend_settings_shading[i].reg_addr, mt9e013_OTP_settings[8+i].reg_data);
		}
	}
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3780, 0x8000,MSM_CAMERA_I2C_WORD_DATA);
	CDBG("mt9e013_shading_setting  OTP write success! \n");
	
	return rc;
}
int32_t mt9e013_write_init_settings(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc=0;

	printk("%s is called !\n", __func__);

	msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
		&mt9e013_recommend_settings_mipi[0],
		ARRAY_SIZE(mt9e013_recommend_settings_mipi),
		MSM_CAMERA_I2C_WORD_DATA); 
	msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
		&mt9e013_recommend_settings[0],
		ARRAY_SIZE(mt9e013_recommend_settings),
		MSM_CAMERA_I2C_WORD_DATA); 
	mt9e013_shading_setting(s_ctrl);
	msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
		&mt9e013_recommend_settings_pll[0],
		ARRAY_SIZE(mt9e013_recommend_settings_pll),
		MSM_CAMERA_I2C_WORD_DATA); 

	return rc;
}
int mt9e013_otp_reading(struct sensor_cfg_data *cfg)
{
	int32_t rc = 0;

	//Get the AWB cabliate data from OTP, if OTP failed, then return
	CDBG(" mt9e013_read_awb_data, start \n");
	if((0==mt9e013_OTP_settings[4].reg_data) || (FALSE == OTP_READ))
	{
		CDBG("AWB invalid, no need to get\n");
		return rc;
	}	

	if( MODEL_SUNNY == mt9e013_model_id)
	{
		cfg->cfg.calib_info.r_over_g = mt9e013_OTP_settings[4].reg_data;
		cfg->cfg.calib_info.b_over_g = mt9e013_OTP_settings[5].reg_data;
		cfg->cfg.calib_info.gr_over_gb = mt9e013_OTP_settings[6].reg_data;
	}
	else
	{
		cfg->cfg.calib_info.r_over_g = mt9e013_OTP_settings[4].reg_data;
		cfg->cfg.calib_info.gr_over_gb = mt9e013_OTP_settings[5].reg_data;
		cfg->cfg.calib_info.b_over_g = mt9e013_OTP_settings[6].reg_data;
	}
	CDBG(" mt9e013_read_awb_data, end rc = %d \n",rc);

	return rc;
}
static const struct i2c_device_id mt9e013_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&mt9e013_s_ctrl},
	{ }
};

static struct i2c_driver mt9e013_i2c_driver = {
	.id_table = mt9e013_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client mt9e013_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
	/*the i2c addr we registerd in board-msm7627a-camera for mt9e013 moves right for 2 bits*/
	.addr_pos = 2,
	.addr_dir = -1,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&mt9e013_i2c_driver);
}

static struct v4l2_subdev_core_ops mt9e013_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops mt9e013_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops mt9e013_subdev_ops = {
	.core = &mt9e013_subdev_core_ops,
	.video  = &mt9e013_subdev_video_ops,
};

static struct msm_sensor_fn_t mt9e013_func_tbl = {
	.sensor_start_stream = mt9e013_start_stream,
	.sensor_stop_stream = mt9e013_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = mt9e013_set_fps,
	.sensor_write_exp_gain = mt9e013_write_exp_gain,
	.sensor_write_snapshot_exp_gain = mt9e013_write_exp_snapshot_gain,
	.sensor_csi_setting = mt9e013_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
	.sensor_model_match = mt9e013_sensor_model_match,
	.sensor_write_init_settings = mt9e013_write_init_settings,
	.sensor_otp_reading = mt9e013_otp_reading,
};

static struct msm_sensor_reg_t mt9e013_regs = {
	.default_data_type = MSM_CAMERA_I2C_WORD_DATA,
	.group_hold_on_conf = mt9e013_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(mt9e013_groupon_settings),
	.group_hold_off_conf = mt9e013_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(mt9e013_groupoff_settings),
	.init_settings = &mt9e013_init_conf[0],
	.init_size = ARRAY_SIZE(mt9e013_init_conf),
	.mode_settings = &mt9e013_confs[0],
	.output_settings = &mt9e013_dimensions[0],
	.num_conf = ARRAY_SIZE(mt9e013_confs),
};

static struct msm_sensor_ctrl_t mt9e013_s_ctrl = {
	.msm_sensor_reg = &mt9e013_regs,
	.sensor_i2c_client = &mt9e013_sensor_i2c_client,
	.sensor_i2c_addr = 0x6C >> 2,
	.sensor_output_reg_addr = &mt9e013_reg_addr,
	.sensor_id_info = &mt9e013_id_info,
	.sensor_exp_gain_info = &mt9e013_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &mt9e013_csi_params_array[0],
	.msm_sensor_mutex = &mt9e013_mut,
	.sensor_i2c_driver = &mt9e013_i2c_driver,
	.sensor_v4l2_subdev_info = mt9e013_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9e013_subdev_info),
	.sensor_v4l2_subdev_ops = &mt9e013_subdev_ops,
	.func_tbl = &mt9e013_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
	.sensor_name = {0},
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Aptina 8MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");


