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
#define SENSOR_NAME "ov5647"
#define PLATFORM_DRIVER_NAME "msm_camera_ov5647"
#define ov5647_obj ov5647_##obj

#define OV_SUNNY_AF 0x45
#define OV_SUNNY_FF 0x54
#define OV_SUNNY_AF_0110 0x6E
static int camera_model_id = 0x1234;

static struct msm_sensor_ctrl_t ov5647_s_ctrl;

DEFINE_MUTEX(ov5647_mut);
struct otp_struct {
    int iProduct_Year;
	int iProduct_Month;
	int iProduct_Date;
	int iCamera_Id;
	int iSupplier_Version_Id;
	int iWB_RG_H;
	int iWB_RG_L;
	int iWB_BG_H;
	int iWB_BG_L;
	int iWB_GbGr_H;
	int iWB_GbGr_L;
	int iVCM_Start;
	int iVCM_End;
};
static struct msm_camera_i2c_reg_conf ov5647_start_settings[] = {
	{0x301a, 0xf0},  /* streaming on */
};

static struct msm_camera_i2c_reg_conf ov5647_stop_settings[] = {
	{0x301a, 0xf1},  /* streaming off*/
};

static struct msm_camera_i2c_reg_conf ov5647_groupon_settings[] = {
	{0x3208, 0x0},
};

static struct msm_camera_i2c_reg_conf ov5647_groupoff_settings[] = {
	{0x3208, 0x10},
	{0x3208, 0xa0},
};

/* data format modify to 10 bit, so we need modify the regsiter */
static struct msm_camera_i2c_reg_conf ov5647_prev_settings[] = {
	/*1280*960 Reference Setting 24M MCLK 2lane 280Mbps/lane 30fps
	for back to preview*/
	{0x3035, 0x21},//PLL   0x21 30fps   0x41 15fps
	{0x3036, 0x66},//PLL,  0x64
	{0x303c, 0x11},//PLL   
	
	{0x3821,0x01},//ISP mirror on, Sensor mirror on
    {0x3820,0x47},//0x41},//} bit[1:2] = 0

    {0x3612, 0x59}, 
    {0x3618, 0x00}, 
    {0x380c, 0x09}, 
    {0x380d, 0x70}, 
    {0x380e, 0x04}, 
    {0x380f, 0x66}, 
    {0x3814, 0x31}, 
    {0x3815, 0x31}, 
    {0x3708, 0x64}, 
    {0x3709, 0x52}, 
    {0x3808, 0x05}, //X OUTPUT SIZE = 1296
    {0x3809, 0x00}, 
    {0x380a, 0x03}, //Y OUTPUT SIZE = 972
    {0x380b, 0xc0}, 
    {0x3800, 0x00}, 
    {0x3801, 0x08}, 
    {0x3802, 0x00}, 
    {0x3803, 0x00}, 
    {0x3804, 0x0a}, 
    {0x3805, 0x37}, 
    {0x3806, 0x07}, 
    {0x3807, 0xa7}, 
	
    {0x4004, 0x02},//black line number

	{0x4005, 0x18},//add
    {0x4051, 0x8F},//add
    
    {0x4837, 0x19},     // 0x3035, 0x41    0x4837 37
};

static struct msm_camera_i2c_reg_conf ov5647_snap_settings[] = {
	/*2608*1952 Reference Setting 24M MCLK 2lane 280Mbps/lane 30fps*/
    {0x3208, 0x02},
    {0x3035, 0x21},
    {0x3036, 0x66},
    {0x303c, 0x11},

   	{0x3821, 0x00},//ISP mirror of, Sensor mirror of 0x00  
    {0x3820, 0x06},//0x0}, bit[1:2] = 0
	
    {0x3612, 0x5b},
    {0x3618, 0x04},
    {0x380c, 0x0a},
    {0x380d, 0xc0},
    {0x380e, 0x07},
    {0x380f, 0xb6},
    {0x3814, 0x11},
    {0x3815, 0x11},
    {0x3708, 0x64},
    {0x3709, 0x12},

    /*2608 * 1952 */
	{0x3808, 0x0a}, //X OUTPUT SIZE = 2608
    {0x3809, 0x30},
    {0x380a, 0x07}, //Y OUTPUT SIZE = 1952
    {0x380b, 0xa0},
    {0x3800, 0x00},
    {0x3801, 0x04},
    {0x3802, 0x00},
    {0x3803, 0x00},
    {0x3804, 0x0a},
    {0x3805, 0x3b},
    {0x3806, 0x07},
    {0x3807, 0xa3},

    {0x4004, 0x04},//black line number

	{0x4005, 0x1a},// add
	
    {0x4837, 0x19},//MIPI pclk period
    {0x3208, 0x12},
    {0x3208, 0xa2},
};

static struct msm_camera_i2c_reg_conf ov5647_video_60fps_settings[] = {
	{0x3035, 0x21},
	{0x3036, 0x38},
	{0x3821, 0x07},
	{0x3820, 0x41},
	{0x3612, 0x49},
	{0x3618, 0x00},
	{0x380c, 0x07},
	{0x380d, 0x30},
	{0x380e, 0x01},
	{0x380f, 0xf8},
	{0x3814, 0x71},
	{0x3815, 0x71},
	{0x3709, 0x52},
	{0x3808, 0x02},
	{0x3809, 0x80},
	{0x380a, 0x01},
	{0x380b, 0xe0},
	{0x3800, 0x00},
	{0x3801, 0x10},
	{0x3802, 0x00},
	{0x3803, 0x00},
	{0x3804, 0x0a},
	{0x3805, 0x2f},
	{0x3806, 0x07},
	{0x3807, 0x9f},
	{0x4004, 0x02},
};

static struct msm_camera_i2c_reg_conf ov5647_video_90fps_settings[] = {
	{0x3035, 0x11},
	{0x3036, 0x2a},
	{0x3821, 0x07},
	{0x3820, 0x41},
	{0x3612, 0x49},
	{0x3618, 0x00},
	{0x380c, 0x07},
	{0x380d, 0x30},
	{0x380e, 0x01},
	{0x380f, 0xf8},
	{0x3814, 0x71},
	{0x3815, 0x71},
	{0x3709, 0x52},
	{0x3808, 0x02},
	{0x3809, 0x80},
	{0x380a, 0x01},
	{0x380b, 0xe0},
	{0x3800, 0x00},
	{0x3801, 0x10},
	{0x3802, 0x00},
	{0x3803, 0x00},
	{0x3804, 0x0a},
	{0x3805, 0x2f},
	{0x3806, 0x07},
	{0x3807, 0x9f},
	{0x4004, 0x02},
};

static struct msm_camera_i2c_reg_conf ov5647_zsl_settings[] = {
	{0x3035, 0x21},
	{0x3036, 0x4f},
	{0x3821, 0x01},
	{0x3820, 0x47},

	{0x3612, 0x0b},
	{0x3618, 0x04},
	{0x380c, 0x0a},
	{0x380d, 0x8c},
	{0x380e, 0x07},
	{0x380f, 0xb0},
	{0x3814, 0x11},
	{0x3815, 0x11},
	{0x3709, 0x12},
	{0x3808, 0x0a},
	{0x3809, 0x30},
	{0x380a, 0x07},
	{0x380b, 0xa0},
	{0x3800, 0x00},
	{0x3801, 0x04},
	{0x3802, 0x00},
	{0x3803, 0x00},
	{0x3804, 0x0a},
	{0x3805, 0x3b},
	{0x3806, 0x07},
	{0x3807, 0xa3},
	{0x4004, 0x04},
};

static struct msm_camera_i2c_reg_conf ov5647_recommend_settings[] = {
	{0x0100, 0x00},
	{0x370c, 0x03},
	{0x5000, 0x06},
	{0x5003, 0x08},
	{0x5a00, 0x08},
	{0x3000, 0xff},
	{0x3001, 0xff},
	{0x3002, 0xff},
	{0x301d, 0xf0},
	{0x3a18, 0x00},
    {0x3a19, 0xf8},
	{0x3c01, 0x80},
	{0x3b07, 0x0c},

	/*analog control*/
	{0x3630, 0x2e},
	{0x3632, 0xe2},
	{0x3633, 0x23},
	{0x3634, 0x44},
	{0x3620, 0x64},
	{0x3621, 0xe0},
	{0x3600, 0x37},
	{0x3704, 0xa0},
	{0x3703, 0x5a},
	{0x3715, 0x78},
	{0x3717, 0x01},
	{0x3731, 0x02},
	{0x370b, 0x60},
	{0x3705, 0x1a},
	{0x3f05, 0x02},
	{0x3f06, 0x10},
	{0x3f01, 0x0a},
	/*AGAE target*/
	{0x3a0f, 0x58},
	{0x3a10, 0x50},
	{0x3a1b, 0x58},
	{0x3a1e, 0x50},
	{0x3a11, 0x60},
	{0x3a1f, 0x28},
	{0x4001, 0x02},
	{0x4000, 0x09},
    {0x4005, 0x18},//add blc
    {0x4051, 0x8F},//add 
	{0x3000, 0x00},
	{0x3001, 0x00},
	{0x3002, 0x00},
	{0x3017, 0xe0},
	{0x301c, 0xfc},
	{0x3636, 0x06},
	{0x3016, 0x08},
	{0x3827, 0xec},
	{0x3018, 0x44},
	{0x3035, 0x21},
	{0x3106, 0xf5},
    {0x3034, 0x1a},//PLL
	{0x301c, 0xf8},
		
    //16# lens
    {0x5800,0x10},
    {0x5801,0xa },
    {0x5802,0x8 },
    {0x5803,0x8 },
    {0x5804,0x9 },
    {0x5805,0x10},
    {0x5806,0x7 },
    {0x5807,0x5 },
    {0x5808,0x3 },
    {0x5809,0x3 },
    {0x580a,0x5 },
    {0x580b,0x8 },
    {0x580c,0x5 },
    {0x580d,0x2 },
    {0x580e,0x0 },
    {0x580f,0x0 },
    {0x5810,0x2 },
    {0x5811,0x5 },
    {0x5812,0x5 },
    {0x5813,0x2 },
    {0x5814,0x0 },
    {0x5815,0x0 },
    {0x5816,0x2 },
    {0x5817,0x5 },
    {0x5818,0x7 },
    {0x5819,0x5 },
    {0x581a,0x3 },
    {0x581b,0x3 },
    {0x581c,0x5 },
    {0x581d,0x7 },
    {0x581e,0xf },
    {0x581f,0x9 },
    {0x5820,0x8 },
    {0x5821,0x8 },
    {0x5822,0x9 },
    {0x5823,0x10},
    {0x5824,0x4a},
    {0x5825,0x2a},
    {0x5826,0x2c},
    {0x5827,0x2c},
    {0x5828,0x46},
    {0x5829,0x2a},
    {0x582a,0x46},
    {0x582b,0x44},
    {0x582c,0x46},
    {0x582d,0x4a},
    {0x582e,0x2a},
    {0x582f,0x62},
    {0x5830,0x60},
    {0x5831,0x62},
    {0x5832,0x28},
    {0x5833,0x2a},
    {0x5834,0x46},
    {0x5835,0x44},
    {0x5836,0x46},
    {0x5837,0x28},
    {0x5838,0x8 },
    {0x5839,0xa },
    {0x583a,0xc },
    {0x583b,0xa },
    {0x583c,0x26},
    {0x583d,0xae},
	/* manual AWB,manual AE,close Lenc,open WBC*/
	{0x3503, 0x03}, /*manual AE*/
	{0x3501, 0x10},
	{0x3502, 0x80},
	{0x350a, 0x00},
	{0x350b, 0x7f},
	{0x5001, 0x01}, /*manual AWB*/
	{0x5180, 0x08},
	{0x5186, 0x04},
	{0x5187, 0x00},
	{0x5188, 0x04},
	{0x5189, 0x00},
	{0x518a, 0x04},
	{0x518b, 0x00},
	{0x5000, 0x86}, /*lenc on, WBC on*/
    {0x0100, 0x01},
};
static struct msm_camera_i2c_reg_conf ov5647_recommend_settings_ff[] = {
	{0x370c, 0x03},
	{0x5000, 0x06},
	{0x5003, 0x08},
	{0x5a00, 0x08},
	{0x3000, 0xff},
	{0x3001, 0xff},
	{0x3002, 0xff},
	{0x301d, 0xf0},
	{0x3a18, 0x00},
	{0x3a19, 0xf8},
	{0x3c01, 0x80},
	{0x3b07, 0x0c},

	/*analog control*/
	{0x3630, 0x2e},
	{0x3632, 0xe2},
	{0x3633, 0x23},
	{0x3634, 0x44},
	{0x3620, 0x64},
	{0x3621, 0xe0},
	{0x3600, 0x37},
	{0x3704, 0xa0},
	{0x3703, 0x5a},
	{0x3715, 0x78},
	{0x3717, 0x01},
	{0x3731, 0x02},
	{0x370b, 0x60},
	{0x3705, 0x1a},
	{0x3f05, 0x02},
	{0x3f06, 0x10},
	{0x3f01, 0x0a},
	/*AGAE target*/
	{0x3a0f, 0x58},
	{0x3a10, 0x50},
	{0x3a1b, 0x58},
	{0x3a1e, 0x50},
	{0x3a11, 0x60},
	{0x3a1f, 0x28},
	{0x4001, 0x02},
	{0x4000, 0x09},
	{0x4005, 0x18},//add blc
	{0x4051, 0x8F},//add 
	{0x3000, 0x00},
	{0x3001, 0x00},
	{0x3002, 0x00},
	{0x3017, 0xe0},
	{0x301c, 0xfc},
	{0x3636, 0x06},
	{0x3016, 0x08},
	{0x3827, 0xec},
	{0x3018, 0x44},
	{0x3035, 0x21},
	{0x3106, 0xf5},
	{0x3034, 0x1a},//PLL
	{0x301c, 0xf8},
		
    /* module 7 setting */
    {0x5800, 0x0d},
    {0x5801, 0x08},
    {0x5802, 0x06},
    {0x5803, 0x06},
    {0x5804, 0x08},
    {0x5805, 0x0c},
    {0x5806, 0x06},
    {0x5807, 0x03},
    {0x5808, 0x02},
    {0x5809, 0x02},
    {0x580a, 0x03},
    {0x580b, 0x05},
    {0x580c, 0x04},
    {0x580d, 0x01},
    {0x580e, 0x00},
    {0x580f, 0x00},
    {0x5810, 0x01},
    {0x5811, 0x03},
    {0x5812, 0x03},
    {0x5813, 0x01},
    {0x5814, 0x00},
    {0x5815, 0x00},
    {0x5816, 0x01},
    {0x5817, 0x03},
    {0x5818, 0x06},
    {0x5819, 0x03},
    {0x581a, 0x02},
    {0x581b, 0x02},
    {0x581c, 0x03},
    {0x581d, 0x05},
    {0x581e, 0x0b},
    {0x581f, 0x0a},
    {0x5820, 0x06},
    {0x5821, 0x07},
    {0x5822, 0x09},
    {0x5823, 0x0c},
    {0x5824, 0x44},
    {0x5825, 0x24},
    {0x5826, 0x06},
    {0x5827, 0x24},
    {0x5828, 0x44},
    {0x5829, 0x24},
    {0x582a, 0x22},
    {0x582b, 0x22},
    {0x582c, 0x22},
    {0x582d, 0x06},
    {0x582e, 0x02},
    {0x582f, 0x22},
    {0x5830, 0x42},
    {0x5831, 0x42},
    {0x5832, 0x04},
    {0x5833, 0x06},
    {0x5834, 0x22},
    {0x5835, 0x22},
    {0x5836, 0x24},
    {0x5837, 0x06},
    {0x5838, 0x44},
    {0x5839, 0x24},
    {0x583a, 0x26},
    {0x583b, 0x24},
    {0x583c, 0x42},
    {0x583d, 0xcf},
    
    /* manual AWB,manual AE,close Lenc,open WBC*/
    {0x3503, 0x03}, /*manual AE*/
    {0x3501, 0x10},
    {0x3502, 0x80},
    {0x350a, 0x00},
    {0x350b, 0x7f},
    {0x5001, 0x01}, /*manual AWB*/
    {0x5180, 0x08},
    {0x5186, 0x04},
    {0x5187, 0x00},
    {0x5188, 0x04},
    {0x5189, 0x00},
    {0x518a, 0x04},
    {0x518b, 0x00},
    {0x5000, 0x86}, /*lenc on, WBC on*/
};

static struct msm_camera_i2c_conf_array ov5647_init_conf[] = {
	{&ov5647_recommend_settings[0],
	ARRAY_SIZE(ov5647_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};
static struct msm_camera_i2c_conf_array ov5647_init_conf_ff[] = {
	{&ov5647_recommend_settings_ff[0],
	ARRAY_SIZE(ov5647_recommend_settings_ff), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array ov5647_confs[] = {
	{&ov5647_snap_settings[0],
	ARRAY_SIZE(ov5647_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov5647_prev_settings[0],
	ARRAY_SIZE(ov5647_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov5647_video_60fps_settings[0],
	ARRAY_SIZE(ov5647_video_60fps_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov5647_video_90fps_settings[0],
	ARRAY_SIZE(ov5647_video_90fps_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov5647_zsl_settings[0],
	ARRAY_SIZE(ov5647_zsl_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};
static struct msm_camera_csi_params ov5647_csi_params = {
	.data_format = CSI_10BIT,
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x18,
};

static struct v4l2_subdev_info ov5647_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
	/* more can be supported, to be added later */
};

/* sensor data format modify to 10 bit, the line_length_pclk & 
 * frame_length_lines need modify as the same time */
static struct msm_sensor_output_info_t ov5647_dimensions[] = {
	{ /* For SNAPSHOT */
		.x_output = 0xA30,  /*2608*/  /*for 5Mp*/
		.y_output = 0x7A0,   /*1952*/
		.line_length_pclk = 0xAC0,
		.frame_length_lines = 0x7B6,
		.vt_pixel_clk = 81600000,
		.op_pixel_clk = 159408000,
		.binning_factor = 0x0,
	},
	{ /* For PREVIEW */
		.x_output = 0x500, /*1280*/
		.y_output = 0x3C0, /*960*/
		.line_length_pclk = 0x970,
		.frame_length_lines = 0x466,
		.vt_pixel_clk = 81600000,
		.op_pixel_clk = 159408000,
		.binning_factor = 0x0,
	},
	{ /* For 60fps */
		.x_output = 0x280,  /*640*/
		.y_output = 0x1E0,   /*480*/
		.line_length_pclk = 0x73C,
		.frame_length_lines = 0x1F8,
		.vt_pixel_clk = 44800000,
		.op_pixel_clk = 159408000,
		.binning_factor = 0x0,
	},
	{ /* For 90fps */
		.x_output = 0x280,  /*640*/
		.y_output = 0x1E0,   /*480*/
		.line_length_pclk = 0x73C,
		.frame_length_lines = 0x1F8,
		.vt_pixel_clk = 67200000,
		.op_pixel_clk = 159408000,
		.binning_factor = 0x0,
	},
	{ /* For ZSL */
		.x_output = 0xA30,  /*2608*/  /*for 5Mp*/
		.y_output = 0x7A0,   /*1952*/
		.line_length_pclk = 0xA8C,
		.frame_length_lines = 0x7B0,
		.vt_pixel_clk = 37600000,
		.op_pixel_clk = 159408000,
		.binning_factor = 0x0,
	},

};

static struct msm_sensor_output_reg_addr_t ov5647_reg_addr = {
	.x_output = 0x3808,
	.y_output = 0x380A,
	.line_length_pclk = 0x380C,
	.frame_length_lines = 0x380E,
};

static struct msm_camera_csi_params *ov5647_csi_params_array[] = {
	&ov5647_csi_params, /* Snapshot */
	&ov5647_csi_params, /* Preview */
	&ov5647_csi_params, /* 60fps */
	&ov5647_csi_params, /* 90fps */
	&ov5647_csi_params, /* ZSL */
};

static struct msm_sensor_id_info_t ov5647_id_info = {
	.sensor_id_reg_addr = 0x300a,
	.sensor_id = 0x5647,
};

static struct msm_sensor_exp_gain_info_t ov5647_exp_gain_info = {
	.coarse_int_time_addr = 0x3500,
	.global_gain_addr = 0x350A,
	.vert_offset = 4,
};
// R/G and B/G of typical camera module is defined here
/*use new AWB golden module: #16*/
int RG_Ratio_Typical = 0x2C6;//0x293;//;//0x29E;//use #64 module 0x2A8;
int BG_Ratio_Typical = 0x2C6;//0x2C8;//;//0x2C8;//use #64 module 0x280;
int32_t OV5647_read_i2c(uint16_t addr)
{
    uint16_t data;
    int rc;
    
    rc = msm_camera_i2c_read(ov5647_s_ctrl.sensor_i2c_client, 
            addr, &data, 
            MSM_CAMERA_I2C_BYTE_DATA);
    if(rc < 0)
    {
        printk(KERN_ERR "%s fail, rc = %d! addr = 0x%x\n", __func__, rc, addr);
        return rc;
    }
    return data;
}
int32_t OV5647_write_i2c(uint16_t addr, uint16_t data)
{
    int rc = -EFAULT;

    rc = msm_camera_i2c_write(ov5647_s_ctrl.sensor_i2c_client, 
            addr, data, 
            MSM_CAMERA_I2C_BYTE_DATA);
    if(rc < 0)
        printk(KERN_ERR "%s fail,rc = %d! addr = 0x%x, data = 0x%x\n", 
            __func__, rc, addr, data);
    return rc;
}
//index: index of otp group.(0,1)
//Return value : 0 : group 0
//				 1 : group 1
//				 2 : empty
int check_otp_group(void)
{
	int temp_h,temp_l,temp;
	int i;
	int address;
	int index;

	// check group 1 first
	index =  1;

	// read otp into buffer
	OV5647_write_i2c(0x3d21, 0x01);
    mdelay(5);

	// read R/G data from OTP to judge data empty or not
	address = 0x3d05 + index*13 + 5;
	temp_h = OV5647_read_i2c(address);
	address = address + 1;
	temp_l = OV5647_read_i2c(address);
	temp = (temp_h<< 8) + temp_l;

	// disable otp read
	OV5647_write_i2c(0x3d21, 0x00);

	// clear otp buffer
	for (i=0;i<32;i++) {
		OV5647_write_i2c(0x3d00 + i, 0x00);
	}
	
	if(!temp == 0)
	{
			return 1;
	}

	// check group 2 data
	index =  0;

	// read otp into buffer
	OV5647_write_i2c(0x3d21, 0x01);
    mdelay(5);

	// read R/G data from OTP to judge data empty or not
	address = 0x3d05 + index*13 + 5;
	temp_h = OV5647_read_i2c(address);

	address = address + 1;
	temp_l = OV5647_read_i2c(address);

	temp = (temp_h<< 8) + temp_l;

	// disable otp read
	OV5647_write_i2c(0x3d21, 0x00);

	// clear otp buffer
	for (i=0;i<32;i++) {
		OV5647_write_i2c(0x3d00 + i, 0x00);
	}
	
	if(!temp == 0)
	{
			return 0;
	}else
	{
		return 2;
	}
}

// index: index of otp group. (0, 1)
// return: 	0, 
int read_otp(int index, struct otp_struct * otp_ptr)
{
	int i;
	int address;

	// read otp into buffer
	OV5647_write_i2c(0x3d21, 0x01);
    mdelay(5);

	address = 0x3d05 + index*13;
	(*otp_ptr).iProduct_Year=OV5647_read_i2c(address);
	(*otp_ptr).iProduct_Month = OV5647_read_i2c(address+1);
	(*otp_ptr).iProduct_Date = OV5647_read_i2c(address+2);
	(*otp_ptr).iCamera_Id = OV5647_read_i2c(address + 3);
	(*otp_ptr).iSupplier_Version_Id = OV5647_read_i2c(address + 4);
	(*otp_ptr).iWB_RG_H = OV5647_read_i2c(address + 5);
	(*otp_ptr).iWB_RG_L = OV5647_read_i2c(address + 6);
	(*otp_ptr).iWB_BG_H = OV5647_read_i2c(address + 7);
	(*otp_ptr).iWB_BG_L = OV5647_read_i2c(address + 8);
	(*otp_ptr).iWB_GbGr_H = OV5647_read_i2c(address + 9);
	(*otp_ptr).iWB_GbGr_L = OV5647_read_i2c(address + 10);
	(*otp_ptr).iVCM_Start = OV5647_read_i2c(address + 11);
	(*otp_ptr).iVCM_End = OV5647_read_i2c(address + 12);
	
	// disable otp read
	OV5647_write_i2c(0x3d21, 0x00);

	// clear otp buffer
	for (i=0;i<32;i++) {
		OV5647_write_i2c(0x3d00 + i, 0x00);
	}
	/*print awb otp info*/
	printk("iProduct_Year = %d iProduct_Month = %d, iProduct_Date = %d,\n \
		iCamera_Id = %d, iSupplier_Version_Id = %d, \n \
		iWB_RG_H = %d, iWB_RG_L = %d, iWB_BG_H = %d, iWB_BG_L = %d,\n \
		iWB_GbGr_H = %d, iWB_GbGr_L = %d, iVCM_Start = %d, iVCM_End = %d\n ", 
		(*otp_ptr).iProduct_Year,(*otp_ptr).iProduct_Month, (*otp_ptr).iProduct_Date,
		(*otp_ptr).iCamera_Id,(*otp_ptr).iSupplier_Version_Id,
		(*otp_ptr).iWB_RG_H, (*otp_ptr).iWB_RG_L, (*otp_ptr).iWB_BG_H,(*otp_ptr).iWB_BG_L,
		(*otp_ptr).iWB_GbGr_H,(*otp_ptr).iWB_GbGr_L,(*otp_ptr).iVCM_Start,(*otp_ptr).iVCM_End);
	return 0;	
}

int update_awb_gain(int R_gain, int G_gain, int B_gain)
{
    if (R_gain>0x400) {
        OV5647_write_i2c(0x5186, R_gain>>8);
        OV5647_write_i2c(0x5187, R_gain & 0x00ff);
    }

    if (G_gain>0x400) {
        OV5647_write_i2c(0x5188, G_gain>>8);
        OV5647_write_i2c(0x5189, G_gain & 0x00ff);
    }

    if (B_gain>0x400) {
        OV5647_write_i2c(0x518a, B_gain>>8);
        OV5647_write_i2c(0x518b, B_gain & 0x00ff);
    }
    
    return 0;
}

// call this function after OV5647 initialization
// return value: 0, update success
//		         1, no OTP
int update_otp(void)
{
	struct otp_struct current_otp;
	int otp_index;
	int R_gain, G_gain, B_gain, G_gain_R, G_gain_B;
	int rg,bg;
	

	
	// Check OTP group
	otp_index = check_otp_group();
	if(otp_index == 2)
	{
		// no data in OTP
		printk("%s no data in OTP!\n", __func__);
		return 1;
	}

	// R/G and B/G of current camera module is read out from sensor OTP
	read_otp(otp_index, &current_otp);

	rg = (current_otp.iWB_RG_H << 8) + current_otp.iWB_RG_L;
	bg = (current_otp.iWB_BG_H << 8) + current_otp.iWB_BG_L;

	//calculate G gain
	//0x400 = 1x gain
	if(bg < BG_Ratio_Typical) {
		if (rg< RG_Ratio_Typical) {
			// current_otp.bg_ratio < BG_Ratio_typical &&  
			// current_otp.rg_ratio < RG_Ratio_typical
   			G_gain = 0x400;
			B_gain = 0x400 * BG_Ratio_Typical / bg;
    		R_gain = 0x400 * RG_Ratio_Typical / rg; 
		}
		else {
			// current_otp.bg_ratio < BG_Ratio_typical &&  
			// current_otp.rg_ratio >= RG_Ratio_typical
    		R_gain = 0x400;
   	 		G_gain = 0x400 * rg / RG_Ratio_Typical;
    		B_gain = G_gain * BG_Ratio_Typical /bg;
		}
	}
	else {
		if (rg < RG_Ratio_Typical) {
			// current_otp.bg_ratio >= BG_Ratio_typical &&  
			// current_otp.rg_ratio < RG_Ratio_typical
    		B_gain = 0x400;
    		G_gain = 0x400 * bg / BG_Ratio_Typical;
    		R_gain = G_gain * RG_Ratio_Typical / rg;
		}
		else {
			// current_otp.bg_ratio >= BG_Ratio_typical &&  
			// current_otp.rg_ratio >= RG_Ratio_typical
    		G_gain_B = 0x400 * bg / BG_Ratio_Typical;
   	 		G_gain_R = 0x400 * rg / RG_Ratio_Typical;

    		if(G_gain_B > G_gain_R ) {
        				B_gain = 0x400;
        				G_gain = G_gain_B;
 	     			R_gain = G_gain * RG_Ratio_Typical /rg;
  			}
    		else {
        			R_gain = 0x400;
       				G_gain = G_gain_R;
        			B_gain = G_gain * BG_Ratio_Typical / bg;
			}
    	}    
	}

	update_awb_gain(R_gain, G_gain, B_gain);

	return 0;

}

// return value: 0, no otp
//		         1, otp r/g value
int get_otp_rg(void)
{
	struct otp_struct current_otp;
	int otp_index;
	int rg;
	
	// Check OTP group
	otp_index = check_otp_group();
	if(otp_index == 2)
	{
		// no data in OTP
		return 0;
	}

	read_otp(otp_index, &current_otp);

	rg =  (current_otp.iWB_RG_H<<8) + current_otp.iWB_RG_L;

	return rg;
}

// return value: 0 no otp
//		> 0: otp r/g value
int get_otp_bg(void)
{
	struct otp_struct current_otp;
	int otp_index;
	int bg;
	
	// Check OTP group
	otp_index = check_otp_group();
	if(otp_index == 2)
	{
		// no data in OTP
		return 0;
	}

	read_otp(otp_index, &current_otp);
	
	bg = (current_otp.iWB_BG_H << 8) + current_otp.iWB_BG_L;

	return bg;
}

// convert OTP WB data to ratio 
/* the real value need divide 1024 */
int get_awb_ratio(int value)
{
	int ratio;

	if(value == 0)
	{
		ratio = 1 * 1024;
	}
	else
	{
 		ratio = value;
	}

	return ratio;
}
void ov5647_sensor_reset_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
	msm_camera_i2c_write(
		s_ctrl->sensor_i2c_client,
		0x103, 0x1,
		MSM_CAMERA_I2C_BYTE_DATA);
}

static int32_t ov5647_write_pict_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{

	static uint16_t max_line = 1964;
	uint8_t gain_lsb, gain_hsb;
	u8 intg_time_hsb, intg_time_msb, intg_time_lsb;

	gain_lsb = (uint8_t) (gain);
	gain_hsb = (uint8_t)((gain & 0x300)>>8);

	CDBG(KERN_ERR "snapshot exposure seting 0x%x, 0x%x, %d"
		, gain, line, line);
	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	if (line > 1964) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			(uint8_t)((line+4) >> 8),
			MSM_CAMERA_I2C_BYTE_DATA);

		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
			(uint8_t)((line+4) & 0x00FF),
			MSM_CAMERA_I2C_BYTE_DATA);
		max_line = line + 4;
	} else if (max_line > 1968) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			(uint8_t)(1968 >> 8),
			MSM_CAMERA_I2C_BYTE_DATA);

		 msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
			(uint8_t)(1968 & 0x00FF),
			MSM_CAMERA_I2C_BYTE_DATA);
			max_line = 1968;
	}


	line = line<<4;
	/* ov5647 need this operation */
	intg_time_hsb = (u8)(line>>16);
	intg_time_msb = (u8) ((line & 0xFF00) >> 8);
	intg_time_lsb = (u8) (line & 0x00FF);

	/* FIXME for BLC trigger */
	/* Coarse Integration Time */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		intg_time_hsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
		intg_time_msb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 2,
		intg_time_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	/* gain */

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr,
		gain_hsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr + 1,
		gain_lsb^0x1,
		MSM_CAMERA_I2C_BYTE_DATA);

	/* Coarse Integration Time */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		intg_time_hsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
		intg_time_msb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 2,
		intg_time_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	/* gain */

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr,
		gain_hsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr + 1,
		gain_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);


	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	return 0;

}


static int32_t ov5647_write_prev_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
						uint16_t gain, uint32_t line)
{
	u8 intg_time_hsb, intg_time_msb, intg_time_lsb;
	uint8_t gain_lsb, gain_hsb;
	uint32_t fl_lines = s_ctrl->curr_frame_length_lines;
	uint8_t offset = s_ctrl->sensor_exp_gain_info->vert_offset;

	CDBG(KERN_ERR "preview exposure setting 0x%x, 0x%x, %d",
		 gain, line, line);

	gain_lsb = (uint8_t) (gain);
	gain_hsb = (uint8_t)((gain & 0x300)>>8);

	fl_lines = (fl_lines * s_ctrl->fps_divider) / Q10;

	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);

	/* adjust frame rate */
	if ((s_ctrl->curr_res < MSM_SENSOR_RES_2) &&
		(line > (fl_lines - offset)))
		fl_lines = line + offset;

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines,
		(uint8_t)(fl_lines >> 8),
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
		(uint8_t)(fl_lines & 0x00FF),
		MSM_CAMERA_I2C_BYTE_DATA);

	line = line<<4;
	/* ov5647 need this operation */
	intg_time_hsb = (u8)(line>>16);
	intg_time_msb = (u8) ((line & 0xFF00) >> 8);
	intg_time_lsb = (u8) (line & 0x00FF);


	/* Coarse Integration Time */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		intg_time_hsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
		intg_time_msb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 2,
		intg_time_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	/* gain */

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr,
		gain_hsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr + 1,
		gain_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	if(is_first_preview_frame)
	{
		msleep(50);
		is_first_preview_frame = 0;
	}
	return 0;
}

static const struct i2c_device_id ov5647_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&ov5647_s_ctrl},
	{ }
};
int32_t ov5647_sensor_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int32_t rc = 0;
	struct  msm_camera_sensor_info * sinfo = NULL;
	
	rc = msm_sensor_i2c_probe(client, id);
	if(rc < 0)
	{	
		pr_err("%s: original ov5647 probe failed, to probe another ov5647\n", __func__);

		sinfo = (struct  msm_camera_sensor_info *)client->dev.platform_data;
		sinfo->sensor_platform_info->gpio_conf->get_correct_gpio_set();
			
		/*after get the gpio setting ,we try to probe another ov5647*/
		rc = msm_sensor_i2c_probe(client, id);
	}
	
	return rc;
}

static struct i2c_driver ov5647_i2c_driver = {
	.id_table = ov5647_i2c_id,
	.probe  = ov5647_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};



static struct msm_camera_i2c_client ov5647_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&ov5647_i2c_driver);
}

static struct v4l2_subdev_core_ops ov5647_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops ov5647_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops ov5647_subdev_ops = {
	.core = &ov5647_subdev_core_ops,
	.video  = &ov5647_subdev_video_ops,
};

int32_t ov5647_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	struct msm_camera_sensor_info *info = NULL;
	unsigned short rdata;
	int rc;

	info = s_ctrl->sensordata;
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x4202, 0xf,
		MSM_CAMERA_I2C_BYTE_DATA);
	msleep(20);
	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x3018,
			&rdata, MSM_CAMERA_I2C_WORD_DATA);
	CDBG("ov5647_sensor_power_down: %d\n", rc);
	rdata |= 0x18;
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x3018, rdata,
		MSM_CAMERA_I2C_WORD_DATA);
	msleep(20);
	gpio_direction_output(info->sensor_pwd, 1);
	usleep_range(5000, 5100);
	msm_sensor_power_down(s_ctrl);
	return 0;
}

int32_t ov5647_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	struct msm_camera_sensor_info *info = NULL;

	info = s_ctrl->sensordata;
	gpio_direction_output(info->sensor_pwd, 1);
	gpio_direction_output(info->sensor_reset, 0);
	usleep_range(10000, 11000);
	rc = msm_sensor_power_up(s_ctrl);
	if (rc < 0) {
		CDBG("%s: msm_sensor_power_up failed\n", __func__);
		return rc;
	}

	/* turn on ldo and vreg */

	gpio_direction_output(info->sensor_pwd, 0);
	msleep(20);
	gpio_direction_output(info->sensor_reset, 1);
	msleep(25);

	return rc;

}

static int32_t vfe_clk = 266667000;

int32_t ov5647_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(10);

	msm_camera_i2c_write(
			s_ctrl->sensor_i2c_client,
			0x301c, 0xfc,
			MSM_CAMERA_I2C_BYTE_DATA);
	msleep(5);
	if (update_type == MSM_SENSOR_REG_INIT) {
		CDBG("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_camera_i2c_write(
				s_ctrl->sensor_i2c_client,
				0x103, 0x1,
				MSM_CAMERA_I2C_BYTE_DATA);
		msm_sensor_enable_debugfs(s_ctrl);
		msm_sensor_write_init_settings(s_ctrl);
		csi_config = 0;
        update_otp();
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		CDBG("PERIODIC : %d\n", res);
		msm_sensor_write_conf_array(
			s_ctrl->sensor_i2c_client,
			s_ctrl->msm_sensor_reg->mode_settings, res);
		msleep(10);
		if (!csi_config) {
			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
			CDBG("CSI config in progress\n");
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIC_CFG,
				s_ctrl->curr_csic_params);
			CDBG("CSI config is done\n");
			mb();
			msleep(20);
			csi_config = 1;
		msm_camera_i2c_write(
			s_ctrl->sensor_i2c_client,
			0x100, 0x1,
			MSM_CAMERA_I2C_BYTE_DATA);
		}
		msm_camera_i2c_write(
			s_ctrl->sensor_i2c_client,
			0x301c, 0xf8,
			MSM_CAMERA_I2C_BYTE_DATA);
		msleep(5);
		if (res == MSM_SENSOR_RES_4)
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
					NOTIFY_PCLK_CHANGE,
					&vfe_clk);
		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		msleep(30);
		if(res == MSM_SENSOR_RES_QTR)
		    is_first_preview_frame = 1;
	}
	return rc;
}

int32_t ov5647_sensor_model_match(struct msm_sensor_ctrl_t *s_ctrl)
{

	int otp_index = -1;
	
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,	0x100, 0x1,MSM_CAMERA_I2C_BYTE_DATA);
	
	// Check OTP group
	otp_index = check_otp_group();
	if(otp_index == 2)
	{
		// no data in OTP
		printk("%s:no data in OTP!\n", __func__);
		return -1;
	}

	OV5647_write_i2c(0x3d21, 0x01);
	mdelay(5);
	//read the 0x3D08 or 0x3D15 to save camera model coding
	camera_model_id = OV5647_read_i2c(0x3d05 + otp_index*13 + 3);
	OV5647_write_i2c(0x3d21, 0x00);

	// clear otp buffer
	OV5647_write_i2c(0x3d05 + otp_index*13 + 3, 0x00);

	printk("%s:camera_model_id=0x%x\n", __func__, camera_model_id);	
	if((OV_SUNNY_AF == camera_model_id)||(OV_SUNNY_AF_0110 == camera_model_id))
	{
		if(check_product_y300_for_camera())
		{
			strncpy((char *)s_ctrl->sensor_name, "23060110FA-OV-S-Y300", sizeof("23060110FA-OV-S-Y300"));
		}
		else
		{
		strncpy((char *)s_ctrl->sensor_name, "23060110FA-OV-S", sizeof("23060110FA-OV-S"));
		}
		RG_Ratio_Typical = 0x2A8;
        BG_Ratio_Typical = 0x280;
	}
	else if(OV_SUNNY_FF == camera_model_id)
	{
		strncpy((char *)s_ctrl->sensor_name, "23060084FF-OV-S", sizeof("23060084FF-OV-S"));
		s_ctrl->sensordata->actuator_info = NULL;
		s_ctrl->msm_sensor_reg->init_settings = &ov5647_init_conf_ff[0];
		s_ctrl->msm_sensor_reg->init_size = ARRAY_SIZE(ov5647_init_conf_ff);
		RG_Ratio_Typical = 0x29F;
        BG_Ratio_Typical = 0x2C6;
	}
	else
	{
		strncpy((char *)s_ctrl->sensor_name, "23060084FF-OV-S", sizeof("23060084FF-OV-S"));
		s_ctrl->sensordata->actuator_info = NULL;
		s_ctrl->msm_sensor_reg->init_settings = &ov5647_init_conf_ff[0];
		s_ctrl->msm_sensor_reg->init_size = ARRAY_SIZE(ov5647_init_conf_ff);
		RG_Ratio_Typical = 0x29F;
        BG_Ratio_Typical = 0x2C6;
	}
	
	return 0;
}

static struct msm_sensor_fn_t ov5647_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = ov5647_write_prev_exp_gain,
	.sensor_write_snapshot_exp_gain = ov5647_write_pict_exp_gain,
	.sensor_csi_setting = ov5647_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
//	.sensor_power_up = ov5647_sensor_power_up,
	.sensor_power_up = msm_sensor_power_up,
//	.sensor_power_down = ov5647_sensor_power_down,
	.sensor_power_down = msm_sensor_power_down,
   	.sensor_get_csi_params = msm_sensor_get_csi_params,
   	.sensor_model_match = ov5647_sensor_model_match,
};

static struct msm_sensor_reg_t ov5647_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = ov5647_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(ov5647_start_settings),
	.stop_stream_conf = ov5647_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(ov5647_stop_settings),
	.group_hold_on_conf = ov5647_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(ov5647_groupon_settings),
	.group_hold_off_conf = ov5647_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(ov5647_groupoff_settings),
	.init_settings = &ov5647_init_conf[0],
	.init_size = ARRAY_SIZE(ov5647_init_conf),
	.mode_settings = &ov5647_confs[0],
	.output_settings = &ov5647_dimensions[0],
	.num_conf = ARRAY_SIZE(ov5647_confs),
};

static struct msm_sensor_ctrl_t ov5647_s_ctrl = {
	.msm_sensor_reg = &ov5647_regs,
	.sensor_i2c_client = &ov5647_sensor_i2c_client,
	.sensor_i2c_addr =  0x36 << 1 ,
	.sensor_output_reg_addr = &ov5647_reg_addr,
	.sensor_id_info = &ov5647_id_info,
	.sensor_exp_gain_info = &ov5647_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &ov5647_csi_params_array[0],
	.msm_sensor_mutex = &ov5647_mut,
	.sensor_i2c_driver = &ov5647_i2c_driver,
	.sensor_v4l2_subdev_info = ov5647_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov5647_subdev_info),
	.sensor_v4l2_subdev_ops = &ov5647_subdev_ops,
	.func_tbl = &ov5647_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
	.sensor_name = "23060049FF-OV-S",
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Omnivision WXGA Bayer sensor driver");
MODULE_LICENSE("GPL v2");
