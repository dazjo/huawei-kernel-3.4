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
#define SENSOR_NAME "mt9p017"
#define PLATFORM_DRIVER_NAME "msm_camera_mt9p017"
#define mt9p017_obj mt9p017_##obj
#define MSB                             1
#define LSB                             0
#define MT9P017_OTP_SUPPORT
#define MSM_SENSOR_MCLK_12HZ 12000000

#include "./msm.h"
#include "./actuators/msm_actuator.h"
DEFINE_MUTEX(mt9p017_mut);
static struct msm_sensor_ctrl_t mt9p017_s_ctrl;

static struct msm_camera_i2c_reg_conf mt9p017_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf mt9p017_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf mt9p017_groupon_settings[] = {
	{0x0104, 0x01},
};

static struct msm_camera_i2c_reg_conf mt9p017_groupoff_settings[] = {
	{0x0104, 0x00},
};

static struct msm_camera_i2c_reg_conf mt9p017_prev_settings[] = {
	{0x3004,0x0000},
	{0x3008,0x0A2D},
	{0x3002,0x0000},
	{0x3006,0x07A5},
	{0x3040,0xC4C3},
	{0x034C,0x0518},
	{0x034E,0x03D4},
	{0x300C,0x0C6A},
	{0x300A,0x041D},
	{0x3012,0x041C},
	{0x3014,0x0926},
	{0x3010,0x0184},
	{0x30F0,0x8000},
};

static struct msm_camera_i2c_reg_conf mt9p017_snap_settings[] = {
	{0x3004,0x0000},
	{0x3008,0x0A2F},
	{0x3002,0x0000},
	{0x3006,0x07A7},
	{0x3040,0xC041},
	{0x034C,0x0A30},
	{0x034E,0x07A8},
	{0x300C,0x167C},
	{0x300A,0x07F5},
	{0x3012,0x07F4},
	{0x3014,0x149A},
	{0x3010,0x00A0},
	{0x30F0,0x8000},
};

static struct msm_camera_i2c_reg_conf mt9p017_recommend_settings_1[] = {
	//recomm setting
	{0x3100, 0x0000},
 	{0x316A, 0x8400}, 
	{0x316C, 0x8400},
	{0x316E, 0x8400},
	{0x3EFA, 0x1A1F},
	{0x3ED2, 0xD965},
	{0x3ED8, 0x7F1B},
	{0x3EDA, 0xAF11},
	{0x3EE2, 0x0060},
	{0x3EF2, 0xD965},
	{0x3EF8, 0x797F},
	{0x3EFC, 0xA8EF},
	{0x30d4, 0x9200},
	{0x30b2, 0xC000},
	{0x30bc, 0x0400}, 
	{0x306E, 0xB480},
	{0x3EFE, 0x1F0F},
	{0x31E0, 0x1F01},
};

static struct msm_camera_i2c_reg_conf mt9p017_recommend_settings_2[] = {
	//pixel timing
	{0x3E00, 0x042F},
	{0x3E02, 0xFFFF},
	{0x3E04, 0xFFFF},
	{0x3E06, 0xFFFF},
	{0x3E08, 0x8071},
	{0x3E0A, 0x7281},
	{0x3E0C, 0x4011},
	{0x3E0E, 0x8010},
	{0x3E10, 0x60A5},
	{0x3E12, 0x4080},
	{0x3E14, 0x4180},
	{0x3E16, 0x0018},
	{0x3E18, 0x46B7},
	{0x3E1A, 0x4994},
	{0x3E1C, 0x4997},
	{0x3E1E, 0x4682},
	{0x3E20, 0x0018},
	{0x3E22, 0x4241},
	{0x3E24, 0x8000},
	{0x3E26, 0x1880},
	{0x3E28, 0x4785},
	{0x3E2A, 0x4992},
	{0x3E2C, 0x4997},
	{0x3E2E, 0x4780},
	{0x3E30, 0x4D80},
	{0x3E32, 0x100C},
	{0x3E34, 0x8000},
	{0x3E36, 0x184A},
	{0x3E38, 0x8042},
	{0x3E3A, 0x001A},
	{0x3E3C, 0x9610},
	{0x3E3E, 0x0C80},
	{0x3E40, 0x4DC6},
	{0x3E42, 0x4A80},
	{0x3E44, 0x0018},
	{0x3E46, 0x8042},
	{0x3E48, 0x8041},
	{0x3E4A, 0x0018},
	{0x3E4C, 0x804B},
	{0x3E4E, 0xB74B},
	{0x3E50, 0x8010},
	{0x3E52, 0x6056},
	{0x3E54, 0x001C},
	{0x3E56, 0x8211},
	{0x3E58, 0x8056},
	{0x3E5A, 0x827C},
	{0x3E5C, 0x0970},
	{0x3E5E, 0x8082},
	{0x3E60, 0x7281},
	{0x3E62, 0x4C40},
	{0x3E64, 0x8E4D},
	{0x3E66, 0x8110},
	{0x3E68, 0x0CAF},
	{0x3E6A, 0x4D80},
	{0x3E6C, 0x100C},
	{0x3E6E, 0x8440},
	{0x3E70, 0x4C81},
	{0x3E72, 0x7C5F},
	{0x3E74, 0x7000},
	{0x3E76, 0x0000},
	{0x3E78, 0x0000},
	{0x3E7A, 0x0000},
	{0x3E7C, 0x0000},
	{0x3E7E, 0x0000},
	{0x3E80, 0x0000},
	{0x3E82, 0x0000},
	{0x3E84, 0x0000},
	{0x3E86, 0x0000},
	{0x3E88, 0x0000},
	{0x3E8A, 0x0000},
	{0x3E8C, 0x0000},
	{0x3E8E, 0x0000},
	{0x3E90, 0x0000},
	{0x3E92, 0x0000},
	{0x3E94, 0x0000},
	{0x3E96, 0x0000},
	{0x3E98, 0x0000},
	{0x3E9A, 0x0000},
	{0x3E9C, 0x0000},
	{0x3E9E, 0x0000},
	{0x3EA0, 0x0000},
	{0x3EA2, 0x0000},
	{0x3EA4, 0x0000},
	{0x3EA6, 0x0000},
	{0x3EA8, 0x0000},
	{0x3EAA, 0x0000},
	{0x3EAC, 0x0000},
	{0x3EAE, 0x0000},
	{0x3EB0, 0x0000},
	{0x3EB2, 0x0000},
	{0x3EB4, 0x0000},
	{0x3EB6, 0x0000},
	{0x3EB8, 0x0000},
	{0x3EBA, 0x0000},
	{0x3EBC, 0x0000},
	{0x3EBE, 0x0000},
	{0x3EC0, 0x0000},
	{0x3EC2, 0x0000},
	{0x3EC4, 0x0000},
	{0x3EC6, 0x0000},
	{0x3EC8, 0x0000},
	{0x3ECA, 0x0000},
	{0x3170, 0x2150},
	{0x317A, 0x0150},
	{0x3ECC, 0x2200},
	{0x3174, 0x0000},
	{0x3176, 0X0000},
	{0x31B0, 0x00C4},
	{0x31B2, 0x0064},
	{0x31B4, 0x0E77},
	{0x31B6, 0x0D24},
	{0x31B8, 0x020E},
	{0x31BA, 0x0710},
	{0x31BC, 0x2A0D},
	{0x31BE, 0xC007},
};

/* Change 0x030A to 0x02 to match  MCLK_24HZ*/
static struct msm_camera_i2c_reg_conf mt9p017_recommend_settings_3[] = {
	//init 1
	{0x301A, 0x0018}, //reset_register
	{0x3064, 0xB800}, //smia_test_2lane_mipi
	{0x31AE, 0x0202}, //dual_lane_MIPI_interface
	{0x0300, 0x0008},	// VT_PIX_CLK_DIV
	{0x0302, 0x0001},	// VT_SYS_CLK_DIV
	{0x0304, 0x0002},	// PRE_PLL_CLK_DIV
	{0x0306, 0x0046},	// PLL_MULTIPLIER
	{0x0308, 0x000a},	// OP_PIX_CLK_DIV
	{0x030A, 0x0002},	// OP_SYS_CLK_DIV

	{0x0404, 0x0010},	//scale_m
	{0x3016, 0x0111},	//row_speed
};

static struct msm_camera_i2c_reg_conf mt9p017_recommend_settings_4[] = {
	//init 3
	{0x3004,0x0000},
	{0x3008,0x0A2D},
	{0x3002,0x0000},
	{0x3006,0x07A5},
	{0x3040,0xC4C3},
	{0x034C,0x0518},
	{0x034E,0x03D4},
	{0x300C,0x0D09},	// LINE_LENGTH_PCK
	{0x300A,0x041D},
	{0x3012,0x041C},
	{0x3014,0x0926},
	{0x3010,0x0184},
	{0x30F0,0x8000},

	{0x31E0,0x1F01},
};

static struct msm_camera_i2c_reg_conf mt9p017_recommend_settings_5[] = {
	{0x3600, 0x00D0},
	{0x3602, 0x332B},
	{0x3604, 0x00F1},
	{0x3606, 0x872B},
	{0x3608, 0xC38E},
	{0x360A, 0x0450},
	{0x360C, 0xC08E},
	{0x360E, 0x77F0},
	{0x3610, 0x61AE},
	{0x3612, 0x910E},
	{0x3614, 0x0690},
	{0x3616, 0x1B8E},
	{0x3618, 0x07F0},
	{0x361A, 0x8A6F},
	{0x361C, 0x4DCD},
	{0x361E, 0x0470},
	{0x3620, 0xED2E},
	{0x3622, 0x21D1},
	{0x3624, 0x49AD},
	{0x3626, 0x8590},
	{0x3640, 0xA7AC},
	{0x3642, 0xB1AC},
	{0x3644, 0xE9CF},
	{0x3646, 0x84CF},
	{0x3648, 0x4F10},
	{0x364A, 0xE32B},
	{0x364C, 0x718D},
	{0x364E, 0x862E},
	{0x3650, 0x9A2E},
	{0x3652, 0x140D},
	{0x3654, 0x4AED},
	{0x3656, 0x314E},
	{0x3658, 0x0E2E},
	{0x365A, 0x8B6F},
	{0x365C, 0xFFCE},
	{0x365E, 0x1BED},
	{0x3660, 0xD60E},
	{0x3662, 0x09F0},
	{0x3664, 0x088F},
	{0x3666, 0x8171},
	{0x3680, 0x0FF1},
	{0x3682, 0x24EE},
	{0x3684, 0x6291},
	{0x3686, 0x00D1},
	{0x3688, 0xAB73},
	{0x368A, 0x0051},
	{0x368C, 0xCC6F},
	{0x368E, 0x5992},
	{0x3690, 0x67F0},
	{0x3692, 0xF553},
	{0x3694, 0x4E90},
	{0x3696, 0x774D},
	{0x3698, 0x1551},
	{0x369A, 0x63B0},
	{0x369C, 0xBDD2},
	{0x369E, 0x0DD1},
	{0x36A0, 0xA00E},
	{0x36A2, 0x466C},
	{0x36A4, 0x064E},
	{0x36A6, 0xB731},
	{0x36C0, 0x070E},
	{0x36C2, 0x8C0E},
	{0x36C4, 0x8ACD},
	{0x36C6, 0x1DD1},
	{0x36C8, 0x97EF},
	{0x36CA, 0x44CD},
	{0x36CC, 0x52AD},
	{0x36CE, 0xF74E},
	{0x36D0, 0x51EE},
	{0x36D2, 0x1FB0},
	{0x36D4, 0xC0CD},
	{0x36D6, 0xD36E},
	{0x36D8, 0x124C},
	{0x36DA, 0x6D10},
	{0x36DC, 0xB06E},
	{0x36DE, 0x030D},
	{0x36E0, 0x59EE},
	{0x36E2, 0xEA30},
	{0x36E4, 0x070E},
	{0x36E6, 0x3251},
	{0x3700, 0xF6AF},
	{0x3702, 0x256F},
	{0x3704, 0x81B4},
	{0x3706, 0xF232},
	{0x3708, 0x0E35},
	{0x370A, 0x1070},
	{0x370C, 0x2D30},
	{0x370E, 0xD6F4},
	{0x3710, 0xE9D1},
	{0x3712, 0x3A75},
	{0x3714, 0xF7ED},
	{0x3716, 0xCA2D},
	{0x3718, 0x9D73},
	{0x371A, 0xEE31},
	{0x371C, 0x2DF4},
	{0x371E, 0xFEAF},
	{0x3720, 0x154B},
	{0x3722, 0xD4F2},
	{0x3724, 0x7411},
	{0x3726, 0x7632},
	{0x3782, 0x04A4},
	{0x3784, 0x03A0},
	{0x37C0, 0xBBA9},
	{0x37C2, 0xAA08},
	{0x37C4, 0xACEA},
	{0x37C6, 0xD8EA},
	{0x3780, 0x8000},
};
static struct v4l2_subdev_info mt9p017_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array mt9p017_init_conf[] = {
	{&mt9p017_recommend_settings_1[0],
	ARRAY_SIZE(mt9p017_recommend_settings_1), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9p017_recommend_settings_2[0],
	ARRAY_SIZE(mt9p017_recommend_settings_2), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9p017_recommend_settings_3[0],
	ARRAY_SIZE(mt9p017_recommend_settings_3), 10, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9p017_recommend_settings_4[0],
	ARRAY_SIZE(mt9p017_recommend_settings_4), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9p017_confs[] = {
	{&mt9p017_snap_settings[0],
	ARRAY_SIZE(mt9p017_snap_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9p017_prev_settings[0],
	ARRAY_SIZE(mt9p017_prev_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_sensor_output_info_t mt9p017_dimensions[] = {
	{
		.x_output = 0xA30,
		.y_output = 0x7A8,
		.line_length_pclk = 0x167C,
		.frame_length_lines = 0x7F5,
		.vt_pixel_clk = 81600000,
		.op_pixel_clk = 81600000,
		.binning_factor = 0,
	},
	{
		.x_output = 0x518,
		.y_output = 0x3D4,
		.line_length_pclk = 0xD09,
		.frame_length_lines = 0x41D,
		.vt_pixel_clk = 81600000,
		.op_pixel_clk = 81600000,
		.binning_factor = 1,
	},
};
 
static struct msm_camera_csi_params mt9p017_csi_params = {
	.data_format = CSI_10BIT,
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 24,
};

static struct msm_camera_csi_params *mt9p017_csi_params_array[] = {
	&mt9p017_csi_params,
	&mt9p017_csi_params,
};

static struct msm_sensor_output_reg_addr_t mt9p017_reg_addr = {
	.x_output = 0x034C,
	.y_output = 0x034E,
	.line_length_pclk = 0x300C,
	.frame_length_lines = 0x300A,
};

static struct msm_sensor_id_info_t mt9p017_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x4800,
};

static struct msm_sensor_exp_gain_info_t mt9p017_exp_gain_info = {
	.coarse_int_time_addr = 0x3012,
	.global_gain_addr = 0x305E,
	.vert_offset = 4,
};

static inline uint8_t mt9p017_byte(uint16_t word, uint8_t offset)
{
	return word >> (offset * BITS_PER_BYTE);
}
#define MT9P017_FRAMELINE_INDEX 8
/*use the set fps function instead of defaut function*/
static int32_t mt9p017_set_fps(struct msm_sensor_ctrl_t *s_ctrl,struct fps_cfg *fps)
{
	uint16_t total_lines_per_frame = 0;
	int32_t rc = 0;
	enum msm_sensor_resolution_t res = 0;

	s_ctrl->fps_divider = fps->fps_div;

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
		mt9p017_prev_settings[MT9P017_FRAMELINE_INDEX].reg_data = total_lines_per_frame;
	}

	printk("%s: res = %d, curr_frame_length_lines = %d, total_lines_per_frame = %d\n",__func__,res,s_ctrl->curr_frame_length_lines,total_lines_per_frame);

	return rc;
}
static int32_t mt9p017_write_prev_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
						uint16_t gain, uint32_t line)
{
    uint16_t max_legal_gain = 0x1E7F;
    int32_t rc = 0;

    printk("Line:%d mt9p017_write_exp_gain\n", __LINE__);

    if (gain > max_legal_gain)
    {
        CDBG("Max legal gain Line:%d\n", __LINE__);
        gain = max_legal_gain;
    }

    gain |= 0x1000;

    s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);

    msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr, gain ,
		MSM_CAMERA_I2C_WORD_DATA);
    msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr, line,
		MSM_CAMERA_I2C_WORD_DATA);

    s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);

    printk("mt9p017_write_exp_gain: gain = %d, line = %d\n", gain, line);

    return rc;
}

static int32_t mt9p017_write_pict_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	uint16_t max_legal_gain = 0x1E7F;
    int32_t rc = 0;

    printk("Line:%d mt9p017_write_exp_gain\n", __LINE__);

    if (gain > max_legal_gain)
    {
        CDBG("Max legal gain Line:%d\n", __LINE__);
        gain = max_legal_gain;
    }

    gain |= 0x1000;

    s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);

    msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr, gain ,
		MSM_CAMERA_I2C_WORD_DATA);
    msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr, line,
		MSM_CAMERA_I2C_WORD_DATA);

    s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	
	/* add 30 ms delay as FAE suggest */
	msleep(30);
    printk("mt9p017_write_exp_gain: gain = %d, line = %d\n", gain, line);

    return rc;
}

#ifdef MT9P017_OTP_SUPPORT
#define TRUE    1
#define FALSE   0
//for read lens shading from eeprom 
#define LC_TABLE_SIZE 106//equal to the size of mt9p017_recommend_settings_5 - 1.
static const unsigned short mt9p017_eeprom_table[LC_TABLE_SIZE] = {
	0x3800,
	0x3802,
	0x3804,
	0x3806,
	0x3808,
	0x380A,
	0x380C,
	0x380E,
	0x3810,
	0x3812,
	0x3814,
	0x3816,
	0x3818,
	0x381A,
	0x381C,
	0x381E,
	0x3820,
	0x3822,
	0x3824,
	0x3826,
	0x3828,
	0x382A,
	0x382C,
	0x382E,
	0x3830,
	0x3832,
	0x3834,
	0x3836,
	0x3838,
	0x383A,
	0x383C,
	0x383E,
	0x3840,
	0x3842,
	0x3844,
	0x3846,
	0x3848,
	0x384A,
	0x384C,
	0x384E,
	0x3850,
	0x3852,
	0x3854,
	0x3856,
	0x3858,
	0x385A,
	0x385C,
	0x385E,
	0x3860,
	0x3862,
	0x3864,
	0x3866,
	0x3868,
	0x386A,
	0x386C,
	0x386E,
	0x3870,
	0x3872,
	0x3874,
	0x3876,
	0x3878,
	0x387A,
	0x387C,
	0x387E,
	0x3880,
	0x3882,
	0x3884,
	0x3886,
	0x3888,
	0x388A,
	0x388C,
	0x388E,
	0x3890,
	0x3892,
	0x3894,
	0x3898,
	0x389A,
	0x389C,
	0x389E,
	0x38A0,
	0x38A2,
	0x38A4,
	0x38A6,
	0x38A8,
	0x38AA,
	0x38AC,
	0x38B0,
	0x38B2,
	0x38B4,
	0x38B6,
	0x38B8,
	0x38BA,
	0x38BC,
	0x38BE,
	0x38C0,
	0x38C2,
	0x38C4,
	0x38C6,
	0x38C8,
	0x38CA,
	0x38CC,
	0x38CE,
	0x38D0,
	0x38D2,
	0x38D4,
	0x38D6,
};
static int32_t mt9p017_set_lc(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc;
	bool bSuccess = FALSE;
	bool bRWFinished = FALSE;
	bool bRWSuccess = FALSE;
	unsigned short j, i;
	unsigned short OTPCheckValue = 0;
	unsigned short DataStartType = 0x3100;
	unsigned short mt9p017_reg_data[LC_TABLE_SIZE];

	memset(mt9p017_reg_data, 0, sizeof(unsigned short)*LC_TABLE_SIZE);

	CDBG("%s: Before read dataStartType = 0x%x\n", __func__, DataStartType);
	while(!bSuccess)
	{
		rc = msm_camera_i2c_write(
					s_ctrl->sensor_i2c_client, 
					0x301A,0x0610, 
					MSM_CAMERA_I2C_WORD_DATA); //disable Stream
		rc = msm_camera_i2c_write(
					s_ctrl->sensor_i2c_client, 
					0x3134,0xCD95, 
					MSM_CAMERA_I2C_WORD_DATA); //timing parameters for OTPM read
		rc = msm_camera_i2c_write(
					s_ctrl->sensor_i2c_client, 
					0x304C,DataStartType, 
					MSM_CAMERA_I2C_WORD_DATA);//0x304C [15:8] for record type
		rc = msm_camera_i2c_write(
					s_ctrl->sensor_i2c_client, 
					0x304A,0x0200, 
					MSM_CAMERA_I2C_WORD_DATA);//Only Read Single Record at a time
		rc = msm_camera_i2c_write(
					s_ctrl->sensor_i2c_client, 
					0x304A,0x0210, 
					MSM_CAMERA_I2C_WORD_DATA);//auto Read Start

		bRWFinished = FALSE;
		bRWSuccess = FALSE;

		for(j = 0; j<10; j++)//POLL Register 0x304A [6:5] = 11 //auto read success
		{
			msleep(10);
				rc = msm_camera_i2c_read(
					s_ctrl->sensor_i2c_client,
					0x304A, &OTPCheckValue,
					MSM_CAMERA_I2C_WORD_DATA);

			CDBG("%s:read count=%d, CheckValue=0x%x", __func__, j, OTPCheckValue);
			if(0xFFFF == (OTPCheckValue |0xFFDF))//finish
			{
				bRWFinished = TRUE;
				if(0xFFFF == (OTPCheckValue |0xFFBF))//success
				{
					bRWSuccess = TRUE;
				}
				break;
			}
		}
		CDBG("%s: read DataStartType = 0x%x, bRWFinished = %d, bRWSuccess = %d", __func__,DataStartType, bRWFinished, bRWSuccess);

		if(!bRWFinished)
		{
			CDBG("%s: read DataStartType Fail!", __func__);
			goto OTPERR;
		}
		else
		{
			if(bRWSuccess)
			{
				switch(DataStartType)
				{
					case 0x3000:
					case 0x3200:
					bSuccess = TRUE;
					break;

					case 0x3100:
					bSuccess = FALSE;
					DataStartType = 0x3000;
					break;
					
					default:
						break;
				}
			}
			else
			{
				switch(DataStartType)
				{
					case 0x3200:
					{
						bSuccess = FALSE;
						CDBG("%s: read DataStartType Error Times Twice!", __func__);
						goto OTPERR;
					}
					break;
					case 0x3100:
					{
						bSuccess = FALSE;
						DataStartType = 0x3200;
					}
					break;
					case 0x3000:
					{
						bSuccess = TRUE;
						DataStartType = 0x3100;
					}
					break;
					default:
						break;
				}
			}
		}
	}
	CDBG("%s: after read DataStartType = 0x%x", __func__,DataStartType);

	//read data
	bSuccess = FALSE;
	rc = msm_camera_i2c_write(
					s_ctrl->sensor_i2c_client, 
					0x301A,0x0610, 
					MSM_CAMERA_I2C_WORD_DATA); //disable Stream
	rc = msm_camera_i2c_write(
					s_ctrl->sensor_i2c_client, 
					0x3134,0xCD95, 
					MSM_CAMERA_I2C_WORD_DATA); //timing parameters for OTPM read
	rc = msm_camera_i2c_write(
					s_ctrl->sensor_i2c_client, 
					0x304C,DataStartType, 
					MSM_CAMERA_I2C_WORD_DATA);//0x304C [15:8] for record type
	rc = msm_camera_i2c_write(
					s_ctrl->sensor_i2c_client, 
					0x304A,0x0200, 
					MSM_CAMERA_I2C_WORD_DATA);//Only Read Single Record at a time
	rc = msm_camera_i2c_write(
					s_ctrl->sensor_i2c_client, 
					0x304A,0x0210, 
					MSM_CAMERA_I2C_WORD_DATA);//auto Read Start

	for(j = 0; j<10; j++)//POLL Register 0x304A [6:5] = 11 //auto read success
	{
		msleep(10);
		rc = msm_camera_i2c_read(
				s_ctrl->sensor_i2c_client,
				0x304A, &OTPCheckValue,
				MSM_CAMERA_I2C_WORD_DATA);

		CDBG("%s:read count=%d, CheckValue=0x%x", __func__, j, OTPCheckValue);
		if(0xFFFF == (OTPCheckValue |0xFFDF))//finish
		{
			bRWFinished = TRUE;
			if(0xFFFF == (OTPCheckValue |0xFFBF))//success
			{
				bRWSuccess = TRUE;
			}
			break;
		}
	}

	CDBG("%s:: read DataStartType(step2) = 0x%x, bRWFinished = %d, bRWSuccess = %d", __func__,DataStartType, bRWFinished, bRWSuccess);
	if(!bRWFinished ||!bRWSuccess)
	{
		CDBG("%s: read DataStartType Error!Failed!", __func__);
		goto OTPERR;
	}
	else 
	{
		CDBG("%s:read 0x3800 to 0x39FE for the written data", __func__);
		for(i = 0; i<LC_TABLE_SIZE; i++)//READ 0x3800 to 0x39FE for the written data
		{	
		 rc = msm_camera_i2c_read(
				s_ctrl->sensor_i2c_client,
				mt9p017_eeprom_table[i], &OTPCheckValue,
				MSM_CAMERA_I2C_WORD_DATA);
			mt9p017_reg_data[i] = OTPCheckValue;
			CDBG("%s:read:mt9p017_eeprom_table[%d]=0x%x,mt9p017_reg_data[%d]=0x%x",__func__,i,mt9p017_eeprom_table[i], i,mt9p017_reg_data[i]);
		}
		
		bSuccess = TRUE;
	}
OTPERR:
	if (bSuccess)
	{
		//write lens shading to sensor registers
		for(i=0;i<LC_TABLE_SIZE;i++)
		{
			/*delete one line of log*/
			rc = msm_camera_i2c_write(
					s_ctrl->sensor_i2c_client, 
					mt9p017_recommend_settings_5[i].reg_addr,
					mt9p017_reg_data[i],
					MSM_CAMERA_I2C_WORD_DATA);
		}
		rc = msm_camera_i2c_write(
					s_ctrl->sensor_i2c_client, 
					0x3780,0x8000, 
					MSM_CAMERA_I2C_WORD_DATA);		

		CDBG("%s: OTP Check OK!!!  rc = %d", __func__,rc);
	}
	else
	{
		CDBG("%s: OTP Check Fail Write Fail!", __func__);
	rc = msm_camera_i2c_write_tbl(
		s_ctrl->sensor_i2c_client,
		&mt9p017_recommend_settings_5[0],
		ARRAY_SIZE(mt9p017_recommend_settings_5), 
		MSM_CAMERA_I2C_WORD_DATA);

		CDBG("%s: OTP Check Fail rc = %d", __func__,rc);
	} 
	return rc;
}
#endif

int32_t mt9p017_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
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
	}
	return rc;
}
int32_t mt9p017_write_init_settings(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc=0, i;

	printk("%s is called !\n", __func__);
	
	for (i = 0; i < s_ctrl->msm_sensor_reg->init_size; i++) 
	{
		rc = msm_sensor_write_conf_array(
			s_ctrl->sensor_i2c_client, 
			s_ctrl->msm_sensor_reg->init_settings, i);
		if (rc < 0)
			break;
	}
#ifndef MT9P017_OTP_SUPPORT
	rc = msm_camera_i2c_write_tbl(
		s_ctrl->sensor_i2c_client,
		&mt9p017_recommend_settings_5[0],
		ARRAY_SIZE(mt9p017_recommend_settings_5), 
		MSM_CAMERA_I2C_WORD_DATA);
#else
	mt9p017_set_lc(s_ctrl);
#endif

	return rc;
}
static const struct i2c_device_id mt9p017_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&mt9p017_s_ctrl},
	{ }
};

static struct i2c_driver mt9p017_i2c_driver = {
	.id_table = mt9p017_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client mt9p017_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
	.addr_pos = 0,
	.addr_dir = 0,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&mt9p017_i2c_driver);
}

static struct v4l2_subdev_core_ops mt9p017_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops mt9p017_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops mt9p017_subdev_ops = {
	.core = &mt9p017_subdev_core_ops,
	.video  = &mt9p017_subdev_video_ops,
};

static struct msm_sensor_fn_t mt9p017_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = mt9p017_set_fps,//msm_sensor_set_fps,
	.sensor_write_exp_gain = mt9p017_write_prev_exp_gain,
	.sensor_write_snapshot_exp_gain = mt9p017_write_pict_exp_gain,
	.sensor_csi_setting = mt9p017_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_write_init_settings = mt9p017_write_init_settings,
};

static struct msm_sensor_reg_t mt9p017_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = mt9p017_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(mt9p017_start_settings),
	.stop_stream_conf = mt9p017_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(mt9p017_stop_settings),
	.group_hold_on_conf = mt9p017_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(mt9p017_groupon_settings),
	.group_hold_off_conf = mt9p017_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(mt9p017_groupoff_settings),
	.init_settings = &mt9p017_init_conf[0],
	.init_size = ARRAY_SIZE(mt9p017_init_conf),
	.mode_settings = &mt9p017_confs[0],
	.output_settings = &mt9p017_dimensions[0],
	.num_conf = ARRAY_SIZE(mt9p017_confs),
};

static struct msm_sensor_ctrl_t mt9p017_s_ctrl = {
	.msm_sensor_reg = &mt9p017_regs,
	.sensor_i2c_client = &mt9p017_sensor_i2c_client,
	.sensor_i2c_addr = 0x6C,
	.sensor_output_reg_addr = &mt9p017_reg_addr,
	.sensor_id_info = &mt9p017_id_info,
	.sensor_exp_gain_info = &mt9p017_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &mt9p017_csi_params_array[0],
	.msm_sensor_mutex = &mt9p017_mut,
	.sensor_i2c_driver = &mt9p017_i2c_driver,
	.sensor_v4l2_subdev_info = mt9p017_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9p017_subdev_info),
	.sensor_v4l2_subdev_ops = &mt9p017_subdev_ops,
	.func_tbl = &mt9p017_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
	.sensor_name = "23060069FA-MT-S",
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Aptina 5MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
