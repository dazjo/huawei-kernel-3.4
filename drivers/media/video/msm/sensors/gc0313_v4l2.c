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
#define SENSOR_NAME "gc0313"
#define PLATFORM_DRIVER_NAME "msm_camera_gc0313"
#define gc0313_obj gc0313_##obj

DEFINE_MUTEX(gc0313_mut);
static struct msm_sensor_ctrl_t gc0313_s_ctrl;

static struct msm_camera_i2c_reg_conf gc0313_start_settings[] = {
};

static struct msm_camera_i2c_reg_conf gc0313_stop_settings[] = {
};

static struct msm_camera_i2c_reg_conf gc0313_reset_settings[] = {
	{0xfe, 0x80},
	{0xfe, 0x80},
	{0xfe, 0x80},
	{0xfe, 0x10},
	{0xfe, 0x10},
	{0xf1, 0xf0},
	{0xf2, 0x00},
	{0xf6, 0x03},
	{0xf7, 0x03},
	{0xfc, 0x1e},
	{0xfe, 0x00},

	{0x42, 0xff},
};

static struct msm_camera_i2c_conf_array gc0313_reset_confs[] = {
	{&gc0313_reset_settings[0], ARRAY_SIZE(gc0313_reset_settings), 50,
		MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_i2c_reg_conf gc0313_init_settings[] =
{
	/////////////////////////////////////////////////////
	////////////////// Window Setting ///////////////////
	/////////////////////////////////////////////////////
	{0xfe, 0x00},
	{0x0d, 0x01},
	{0x0e, 0xe8},
	{0x0f, 0x02},
	{0x10, 0x88},
	{0x05, 0x00},
	{0x06, 0xde},
	{0x07, 0x00},
	{0x08, 0xa7},
	{0x09, 0x00},
	{0x0a, 0x00},
	{0x0b, 0x00},
	{0x0c, 0x04},

	/////////////////////////////////////////////////////
	////////////////// Analog & CISCTL //////////////////
	/////////////////////////////////////////////////////
	{0x17, 0x14},
	{0x19, 0x04},
	{0x1b, 0x48},
	{0x1f, 0x08},
	{0x20, 0x01},
	{0x21, 0x48},
	{0x22, 0x9a},
	{0x23, 0x07},
	{0x24, 0x16},

	/////////////////////////////////////////////////////
	/////////////////// ISP Realated ////////////////////
	/////////////////////////////////////////////////////
	//////////////////////////////////
	//{0x40, 0xdf},
	//{0x41, 0x24},
	//{0x42, 0xff},
	{0x44, 0x20}, //0x20
	{0x45, 0x00},
	{0x46, 0x02},
	{0x4d, 0x01},
	//{0x4f, 0x01},
	{0x50, 0x01},
	//{0x70, 0x70},

	/////////////////////////////////////////////////////
	/////////////////////// BLK /////////////////////////
	/////////////////////////////////////////////////////
	{0x26, 0xf7},
	{0x27, 0x01},
	{0x28, 0x7f},
	{0x29, 0x38},
	{0x33, 0x1a},
	{0x34, 0x1a},
	{0x35, 0x1a},
	{0x36, 0x1a},
	{0x37, 0x1a},
	{0x38, 0x1a},
	{0x39, 0x1a},
	{0x3a, 0x1a},

	////////////////////////////////////////////////////
	//////////////////// Y Gamma ///////////////////////
	////////////////////////////////////////////////////
	{0xfe, 0x00},
	{0x63, 0x00},
	{0x64, 0x06},
	{0x65, 0x0f},
	{0x66, 0x21},
	{0x67, 0x34},
	{0x68, 0x47},
	{0x69, 0x59},
	{0x6a, 0x6c},
	{0x6b, 0x8e},
	{0x6c, 0xab},
	{0x6d, 0xc5},
	{0x6e, 0xe0},
	{0x6f, 0xfa},

	////////////////////////////////////////////////////
	////////////////// YUV to RGB //////////////////////
	////////////////////////////////////////////////////
	{0xb0, 0x13},
	{0xb1, 0x27},
	{0xb2, 0x07},
	{0xb3, 0xf6},
	{0xb4, 0xe0},
	{0xb5, 0x29},
	{0xb6, 0x24},
	{0xb7, 0xdf},
	{0xb8, 0xfd},

	////////////////////////////////////////////////////
	/////////////////////// DNDD ///////////////////////
	////////////////////////////////////////////////////
	{0x7e, 0x12},
	{0x7f, 0xc3},
	//{0x82, 0x78},
	{0x84, 0x02},
	{0x89, 0xa4},

	////////////////////////////////////////////////////
	////////////////////// INTPEE //////////////////////
	////////////////////////////////////////////////////
	//{0x90, 0xbc},
	{0x92, 0x08},
	{0x94, 0x08},
	//{0x95, 0x64},

	////////////////////////////////////////////////////
	/////////////////////// ASDE ///////////////////////
	////////////////////////////////////////////////////
	{0xfe, 0x01},
	{0x18, 0x01},
	{0xfe, 0x00},
	{0x9a, 0x20},
	{0x9c, 0x98},
	{0x9e, 0x08},
	{0xa2, 0x32},
	{0xa4, 0x40},
	{0xaa, 0x50},

	////////////////////////////////////////////////////
	//////////////////// RGB Gamma /////////////////////
	////////////////////////////////////////////////////
#if 0
	{0xbf, 0x0e},
	{0xc0, 0x1c},
	{0xc1, 0x34},
	{0xc2, 0x48},
	{0xc3, 0x5a},
	{0xc4, 0x6b},
	{0xc5, 0x7b},
	{0xc6, 0x95},
	{0xc7, 0xab},
	{0xc8, 0xbf},
	{0xc9, 0xce},
	{0xca, 0xd9},
	{0xcb, 0xe4},
	{0xcc, 0xec},
	{0xcd, 0xf7},
	{0xce, 0xfd},
	{0xcf, 0xff},
#endif

	////////////////////////////////////////////////////
	/////////////////////// AEC ////////////////////////
	////////////////////////////////////////////////////
	{0xfe, 0x01},
	{0x10, 0x08},
	{0x11, 0x11},
	{0x12, 0x13},
	//{0x13, 0x40},
	{0x16, 0x18},
	{0x17, 0x88},
	{0x29, 0x00},
	{0x2a, 0x83},
	{0x2b, 0x02},
	{0x2c, 0x8f},
	{0x2d, 0x03},
	{0x2e, 0x95},
	{0x2f, 0x06},
	{0x30, 0x24},
	{0x31, 0x0c},
	{0x32, 0x48},
	{0x33, 0x20},
	{0x3c, 0x60},
	{0x3e, 0x40},

	////////////////////////////////////////////////////
	/////////////////////// YCP ////////////////////////
	////////////////////////////////////////////////////
	{0xfe, 0x00},
	//{0xd1, 0x30},
	//{0xd2, 0x30},
	{0xde, 0x38},
	{0xd8, 0x15},
	{0xdd, 0x00},
	{0xe4, 0x8f},
	{0xe5, 0x50},

	////////////////////////////////////////////////////
	//////////////////// DARK & RC /////////////////////
	////////////////////////////////////////////////////
	{0xfe, 0x01},
	{0x40, 0x8f},
	{0x41, 0x83},
	//{0x42, 0xff},
	{0x43, 0x06},
	{0x44, 0x1f},
	{0x45, 0xff},
	{0x46, 0xff},
	{0x47, 0x04},

	////////////////////////////////////////////////////
	////////////////////// AWB /////////////////////////
	////////////////////////////////////////////////////
	{0x06, 0x0d},
	{0x07, 0x06},
	{0x08, 0xa4},
	{0x09, 0xf2},
	{0x50, 0xfd},
	{0x51, 0x20},
	{0x52, 0x24},
	{0x53, 0x08},
	{0x54, 0x0b},
	{0x55, 0x0f},
	{0x56, 0x0b},
	{0x57, 0x20},
	{0x58, 0xf6},
	{0x59, 0x0b},
	{0x5a, 0x11},
	{0x5b, 0xf0},
	{0x5c, 0xe8},
	{0x5d, 0x10},
	{0x5e, 0x20},
	{0x5f, 0xe0},
	{0x67, 0x00},
	{0x6d, 0x32},
	{0x6e, 0x08},
	{0x6f, 0x08},
	{0x70, 0x40},
	{0x71, 0x83},
	{0x72, 0x26},
	{0x73, 0x62},
	{0x74, 0x03},
	{0x75, 0x48},
	{0x76, 0x40},
	{0x77, 0xc2},
	{0x78, 0xa5},
	{0x79, 0x18},
	{0x7a, 0x40},
	{0x7b, 0xb0},
	{0x7c, 0xf5},
	{0x81, 0x80},
	{0x82, 0x60},
	{0x83, 0x80},
	{0x92, 0x00},
	{0xd5, 0x0C},
	{0xd6, 0x02},
	{0xd7, 0x06},
	{0xd8, 0x05},
	{0xdd, 0x00},
	{0xfe, 0x00},

	////////////////////////////////////////////////////
	////////////////////// LSC /////////////////////////
	////////////////////////////////////////////////////
	{0xfe, 0x01},
	{0xa0, 0x00},
	{0xa1, 0x3c},
	{0xa2, 0x50},
	{0xa3, 0x00},
	{0xa4, 0x00},
	{0xa5, 0x00},
	{0xa6, 0x00},
	{0xa7, 0x00},
	{0xa8, 0x00},
	{0xa9, 0x00},
	{0xaa, 0x00},
	{0xab, 0x00},
	{0xac, 0x00},
	{0xad, 0x00},
	{0xae, 0x00},
	{0xaf, 0x00},
	{0xb0, 0x00},
	{0xb1, 0x00},
	{0xb2, 0x00},
	{0xb3, 0x00},
	{0xb4, 0x21},
	{0xb5, 0x1e},
	{0xb6, 0x18},
	{0xba, 0x28},
	{0xbb, 0x24},
	{0xbc, 0x1c},
	{0xc0, 0x15},
	{0xc1, 0x14},
	{0xc2, 0x11},
	{0xc6, 0x12},
	{0xc7, 0x12},
	{0xc8, 0x11},
	{0xb7, 0x20},
	{0xb8, 0x20},
	{0xb9, 0x20},
	{0xbd, 0x20},
	{0xbe, 0x20},
	{0xbf, 0x20},
	{0xc3, 0x00},
	{0xc4, 0x00},
	{0xc5, 0x00},
	{0xc9, 0x00},
	{0xca, 0x00},
	{0xcb, 0x00},

	//////////////////////////////////////////////////
	////////////////////// MIPI //////////////////////
	//////////////////////////////////////////////////
	{0xfe, 0x03},
	{0x01, 0x03},
	{0x02, 0x21},
	{0x03, 0x10},
	{0x04, 0x80},
	{0x05, 0x02},
	{0x06, 0x80},
	{0x11, 0x1e},
	{0x12, 0x00},
	{0x13, 0x05},
	{0x15, 0x12},
	{0x17, 0x00},
	{0x21, 0x01},
	{0x22, 0x02},
	{0x23, 0x01},
	{0x29, 0x02},
	{0x2a, 0x01},
	{0x10, 0x94},
	{0xfe, 0x00},
	{0x17, 0x14},
};

static struct msm_camera_i2c_reg_conf gc0313_wb_auto_reg_config[] =
{
	{0x77, 0x57},{0x78, 0x4d},{0x79, 0x45}, 
};
static struct msm_camera_i2c_reg_conf gc0313_wb_incandescent_reg_config[] =
{
	{0x77, 0x48},{0x78, 0x40},{0x79, 0x5c},
};
static struct msm_camera_i2c_reg_conf gc0313_wb_fluorescent_reg_config[] =
{
	{0x77, 0x40},{0x78, 0x42},{0x79, 0x50},
};
static struct msm_camera_i2c_reg_conf gc0313_wb_daylight_reg_config[] =
{
	{0x77, 0x74},{0x78, 0x48},{0x79, 0x64},
};
static struct msm_camera_i2c_reg_conf gc0313_wb_cloudy_reg_config[] =
{
	{0x77, 0x8c},{0x78, 0x50},{0x79, 0x40},
};

static struct msm_camera_i2c_reg_conf gc0313_effect_off_reg_config[] =
{
	{0xfe,0x00},{0x43,0x00},{0x40,0xff},{0x41,0x22},{0x96,0x82},{0x90,0xbc},
	{0x4f,0x01},{0xd4,0x80},{0xda,0x00},{0xdb,0x00},{0xbf,0x0e},{0xc0,0x1c},
	{0xc1,0x34},{0xc2,0x48},{0xc3,0x5a},{0xc4,0x6b},{0xc5,0x7b},{0xc6,0x95},{0xc7,0xab},{0xc8,0xbf},
	{0xc9,0xce},{0xca,0xd9},{0xcb,0xe4},{0xcc,0xec},{0xcd,0xf7},{0xce,0xfd},{0xcf,0xff},{0xfe,0x00},
	{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0x82,0x78},
};

static struct msm_camera_i2c_reg_conf gc0313_effect_mono_reg_config[] =
{
	{0xfe,0x00},{0x43,0x02},{0x40,0xff},{0x41,0x22},{0x42,0xff},{0x96,0x82},{0x90,0xbc},
	{0x4f,0x01},{0xd4,0x80},{0xda,0x00},{0xdb,0x00},{0xbf,0x0e},{0xc0,0x1c},
	{0xc1,0x34},{0xc2,0x48},{0xc3,0x5a},{0xc4,0x6b},{0xc5,0x7b},{0xc6,0x95},{0xc7,0xab},{0xc8,0xbf},
	{0xc9,0xce},{0xca,0xd9},{0xcb,0xe4},{0xcc,0xec},{0xcd,0xf7},{0xce,0xfd},{0xcf,0xff},{0xfe,0x00},
	{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0x82,0x78},
};

static struct msm_camera_i2c_reg_conf gc0313_effect_negative_reg_config[] =
{
	{0xfe,0x00},{0x43,0x01},{0x40,0xff},{0x41,0x22},{0x42,0xff},{0x96,0x82},{0x90,0xbc},
	{0x4f,0x01},{0xd4,0x80},{0xda,0x00},{0xdb,0x00},{0xbf,0x0e},{0xc0,0x1c},
	{0xc1,0x34},{0xc2,0x48},{0xc3,0x5a},{0xc4,0x6b},{0xc5,0x7b},{0xc6,0x95},{0xc7,0xab},{0xc8,0xbf},
	{0xc9,0xce},{0xca,0xd9},{0xcb,0xe4},{0xcc,0xec},{0xcd,0xf7},{0xce,0xfd},{0xcf,0xff},{0xfe,0x00},
	{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0x82,0x78},
};

static struct msm_camera_i2c_reg_conf gc0313_effect_sepia_reg_config[] =
{
	{0xfe,0x00},{0x43,0x02},{0x40,0xff},{0x41,0x22},{0x42,0xff},{0x96,0x82},{0x90,0xbc},
	{0x4f,0x01},{0xd4,0x80},{0xda,0xd0},{0xdb,0x28},{0xbf,0x0e},{0xc0,0x1c},
	{0xc1,0x34},{0xc2,0x48},{0xc3,0x5a},{0xc4,0x6b},{0xc5,0x7b},{0xc6,0x95},{0xc7,0xab},{0xc8,0xbf},
	{0xc9,0xce},{0xca,0xd9},{0xcb,0xe4},{0xcc,0xec},{0xcd,0xf7},{0xce,0xfd},{0xcf,0xff},{0xfe,0x00},
	{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0x82,0x78},
};

static struct msm_camera_i2c_reg_conf gc0313_effect_aqua_reg_config[] =
{
	{0xfe,0x00},{0x43,0x02},{0x40,0xff},{0x41,0x22},{0x42,0xff},{0x96,0x82},{0x90,0xbc},
	{0x4f,0x01},{0xd4,0x80},{0xda,0xd0},{0xdb,0x48},{0xbf,0x0e},{0xc0,0x1c},
	{0xc1,0x34},{0xc2,0x48},{0xc3,0x5a},{0xc4,0x6b},{0xc5,0x7b},{0xc6,0x95},{0xc7,0xab},{0xc8,0xbf},
	{0xc9,0xce},{0xca,0xd9},{0xcb,0xe4},{0xcc,0xec},{0xcd,0xf7},{0xce,0xfd},{0xcf,0xff},{0xfe,0x00},
	{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0xfe,0x00},{0x82,0x78},
};
static struct v4l2_subdev_info gc0313_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};


static struct msm_camera_i2c_conf_array gc0313_init_conf[] = {
	{&gc0313_init_settings[0],
	ARRAY_SIZE(gc0313_init_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_i2c_conf_array gc0313_confs[] = {
	//{NULL,0, 0, MSM_CAMERA_I2C_BYTE_DATA},
	//{NULL,0, 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t gc0313_dimensions[] = {
	{
		.x_output = 0x280,
		.y_output = 0x1E0,
		.line_length_pclk = 0x290,
		.frame_length_lines = 0x1EC,
		.vt_pixel_clk = 24000000,
		.op_pixel_clk = 24000000,
		.binning_factor = 1,
	},
	{
		.x_output = 0x280,
		.y_output = 0x1E0,
		.line_length_pclk = 0x290,
		.frame_length_lines = 0x1EC,
		.vt_pixel_clk = 24000000,
		.op_pixel_clk = 24000000,
		.binning_factor = 1,
	},
};

static struct msm_camera_csi_params gc0313_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x14,
};

static struct msm_camera_csi_params *gc0313_csi_params_array[] = {
	&gc0313_csi_params,
	&gc0313_csi_params,
};

static struct msm_sensor_output_reg_addr_t gc0313_reg_addr = {
	.x_output = 0xCC,
	.y_output = 0xCE,
	.line_length_pclk = 0xC8,
	.frame_length_lines = 0xCA,
};
static struct msm_sensor_id_info_t gc0313_id_info = {
	.sensor_id_reg_addr = 0xf0,
	.sensor_id = 0xd0,
};

static const struct i2c_device_id gc0313_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&gc0313_s_ctrl},
	{ }
};

static struct i2c_driver gc0313_i2c_driver = {
	.id_table = gc0313_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client gc0313_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&gc0313_i2c_driver);
}


int32_t gc0313_write_init_settings(struct msm_sensor_ctrl_t *s_ctrl)
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

	msleep(20);
	return rc;
}


int32_t gc0313_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	uint16_t chipid = 0;
	/*second match id*/
	rc = msm_camera_i2c_read(
			s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_id_info->sensor_id_reg_addr, &chipid,
			MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
		pr_err("%s: %s: read id failed\n", __func__,
			s_ctrl->sensordata->sensor_name);
		return rc;
	}

	printk("msm_sensor id: %d\n", chipid);
	if (chipid != s_ctrl->sensor_id_info->sensor_id) {
		pr_err("msm_sensor_match_id chip id doesnot match\n");
		return -ENODEV;
	}

	return rc;
}
/*func for gc0313 to set effect*/
int32_t gc0313_sensor_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int effect)
{
	struct msm_camera_i2c_reg_conf *reg_conf_tbl = NULL;
	uint16_t num_of_items_in_table = 0;
	int rc = 0;

	printk("%s, to set effect = %d \n", __func__,effect);
	switch (effect)
	{
		case CAMERA_EFFECT_OFF:
			reg_conf_tbl = &gc0313_effect_off_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(gc0313_effect_off_reg_config);
			break;
		case CAMERA_EFFECT_MONO:
			reg_conf_tbl = &gc0313_effect_mono_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(gc0313_effect_mono_reg_config);
			break;
		case CAMERA_EFFECT_NEGATIVE:
			reg_conf_tbl = &gc0313_effect_negative_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(gc0313_effect_negative_reg_config);
			break;
		case CAMERA_EFFECT_SEPIA:
			reg_conf_tbl = &gc0313_effect_sepia_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(gc0313_effect_sepia_reg_config);
			break;
		case CAMERA_EFFECT_AQUA:
			reg_conf_tbl = &gc0313_effect_aqua_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(gc0313_effect_aqua_reg_config);
			break;
		default:
			return 0;
	}
	
	rc = msm_camera_i2c_write_tbl(
		s_ctrl->sensor_i2c_client,
		reg_conf_tbl,
		num_of_items_in_table, 
		MSM_CAMERA_I2C_BYTE_DATA);

	return rc;
}
int32_t gc0313_sensor_set_wb(struct msm_sensor_ctrl_t *s_ctrl, int wb)
{
	struct msm_camera_i2c_reg_conf *reg_conf_tbl = NULL;
	uint16_t num_of_items_in_table = 0;
	int rc = 0;

	printk("%s, to set wb = %d \n", __func__,wb);
	switch (wb)
	{
		case CAMERA_WB_AUTO:
			reg_conf_tbl = &gc0313_wb_auto_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(gc0313_wb_auto_reg_config);
			break;
		case CAMERA_WB_INCANDESCENT:
			reg_conf_tbl = &gc0313_wb_incandescent_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(gc0313_wb_incandescent_reg_config);
			break;
		case CAMERA_WB_FLUORESCENT:
			reg_conf_tbl = &gc0313_wb_fluorescent_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(gc0313_wb_fluorescent_reg_config);
			break;
		case CAMERA_WB_DAYLIGHT:
			reg_conf_tbl = &gc0313_wb_daylight_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(gc0313_wb_daylight_reg_config);
			break;
		case CAMERA_WB_CLOUDY_DAYLIGHT:
			reg_conf_tbl = &gc0313_wb_cloudy_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(gc0313_wb_cloudy_reg_config);
			break;
		default:
			return 0;
	}
	
	rc = msm_camera_i2c_write_tbl(
		s_ctrl->sensor_i2c_client,
		reg_conf_tbl,
		num_of_items_in_table, 
		MSM_CAMERA_I2C_BYTE_DATA);

	return rc;
}

static struct v4l2_subdev_core_ops gc0313_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops gc0313_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops gc0313_subdev_ops = {
	.core = &gc0313_subdev_core_ops,
	.video  = &gc0313_subdev_video_ops,
};

int32_t gc0313_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
	int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;

	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		CDBG("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;

		msm_sensor_write_conf_array(
			s_ctrl->sensor_i2c_client,
			gc0313_reset_confs, 0);

		msm_sensor_enable_debugfs(s_ctrl);
		
		csi_config = 0;
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		CDBG("PERIODIC : %d\n", res);
		if (!csi_config) {
			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
			CDBG("CSI config in progress\n");
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIC_CFG,
				s_ctrl->curr_csic_params);
			CDBG("CSI config is done\n");
			mb();
			msleep(50);
			msm_sensor_write_conf_array(
				s_ctrl->sensor_i2c_client,
				s_ctrl->msm_sensor_reg->init_settings, res);
			msleep(20);
			csi_config = 1;
		}

		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE,
			&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);

		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		msleep(50);
	}
	return rc;
}

static struct msm_sensor_fn_t gc0313_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_csi_setting = gc0313_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id = gc0313_match_id,
	.sensor_set_wb = gc0313_sensor_set_wb,
	.sensor_set_effect = gc0313_sensor_set_effect,
};
 
static struct msm_sensor_reg_t gc0313_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = gc0313_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(gc0313_start_settings),
	.stop_stream_conf = gc0313_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(gc0313_stop_settings),
	.init_settings = &gc0313_init_conf[0],
	.init_size = ARRAY_SIZE(gc0313_init_conf),
	.mode_settings = &gc0313_confs[0],
	.output_settings = &gc0313_dimensions[0],
	.num_conf = ARRAY_SIZE(gc0313_dimensions),
};

static struct msm_sensor_ctrl_t gc0313_s_ctrl = {
	.msm_sensor_reg = &gc0313_regs,
	.sensor_i2c_client = &gc0313_sensor_i2c_client,
	.sensor_i2c_addr = 0x42, 
	.sensor_output_reg_addr = &gc0313_reg_addr,
	.sensor_id_info = &gc0313_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &gc0313_csi_params_array[0],
	.msm_sensor_mutex = &gc0313_mut,
	.sensor_i2c_driver = &gc0313_i2c_driver,
	.sensor_v4l2_subdev_info = gc0313_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(gc0313_subdev_info),
	.sensor_v4l2_subdev_ops = &gc0313_subdev_ops,
	.func_tbl = &gc0313_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
	.sensor_name = "23060075FF-GC-F",
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("GC VGA YUV sensor driver");
MODULE_LICENSE("GPL v2");
