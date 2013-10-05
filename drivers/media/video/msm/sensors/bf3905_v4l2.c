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
#define SENSOR_NAME "bf3905"
#define PLATFORM_DRIVER_NAME "msm_camera_bf3905"
#define bf3905_obj bf3905_##obj
#include "linux/hardware_self_adapt.h"

DEFINE_MUTEX(bf3905_mut);
static struct msm_sensor_ctrl_t bf3905_s_ctrl;

static struct msm_camera_i2c_reg_conf bf3905_start_settings[] = {
};

static struct msm_camera_i2c_reg_conf bf3905_stop_settings[] = {
};
static struct msm_camera_i2c_reg_conf bf3905_init_settings[] =
{
	{0x12,0x80},
	{0x15,0x12},
	{0x3e,0x83},
	{0x09,0x01},
	{0x12,0x00},
	{0x3a,0x20},
	{0x1b,0x0e},
	{0x2a,0x00},
	{0x2b,0x10},
	{0x92,0x60},//9 de c6
	{0x93,0x06},//0 3 7
	{0x8a,0x96},
	{0x8b,0x7d},
	{0x13,0x00},
	{0x01,0x15},
	{0x02,0x23},
	{0x9d,0x20},
	{0x8c,0x02},
	{0x8d,0xee},
	{0x13,0x07},
	{0x5d,0xb3},
	{0xbf,0x08},
	{0xc3,0x08},
	{0xca,0x10},
	{0x62,0x00},
	{0x63,0x00},
	{0xb9,0x00},
	{0x64,0x00},
	
	{0x0e,0x10},
	{0x22,0x12},
	
	{0xbb,0x10},
	{0x08,0x02},
	{0x20,0x09},
	{0x21,0x4f},
	{0x2f,0x84},
	{0x7e,0x84},
	{0x7f,0x3c},
	{0x60,0xe5},
	{0x61,0xf2},
	{0x6d,0xc0},
	{0x1e,0x40},
	{0xd9,0x25},
	{0xdf,0x26},
	{0x71,0x3f},
	{0x16,0xaf},
	{0x17,0x00},
	{0x18,0xa0},
	{0x19,0x00},
	{0x1a,0x78},
	{0x03,0x00},
	{0x4a,0x0c},
	{0xda,0x00},
	{0xdb,0xa2},
	{0xdc,0x00},
	{0xdd,0x7a},
	{0xde,0x00},
	{0x33,0x10},
	{0x34,0x08},
	{0x36,0xc5},
	{0x6e,0x20},
	{0xbc,0x0d},
	{0x35,0x8f}, //0x82   0x8f 92    
	{0x65,0x7f}, //84    7f
	{0x66,0x7a}, //80  7a    
	{0xbd,0xec}, //0xe8
	{0xbe,0x49}, 
	{0x9b,0xe4},
	{0x9c,0x4c},
	{0x37,0xe4},
	{0x38,0x50}, 
	{0x70,0x0b},
	{0x71,0x0e},
	{0x72,0x4c},
	{0x73,0x28},
	{0x74,0x6d},
	{0x75,0x8a},
	{0x76,0x98},
	{0x77,0x2a},
	{0x78,0xff},
	{0x79,0x24},
	{0x7a,0x12},
	{0x7b,0x58},
	{0x7c,0x55},
	{0x7d,0x00},
	{0x13,0x07},
	{0x24,0x48},//0x4a 0x4a
	{0x25,0x88},
	{0x80,0x92},
	{0x81,0x00},
	{0x82,0x2a},
	{0x83,0x54},
	{0x84,0x39},
	{0x85,0x5d},
	{0x86,0x88},
	{0x89,0x73},  //63
	{0x8e,0x2c},
	{0x8f,0x82},
	{0x94,0x22},
	{0x95,0x84},
	{0x96,0xb3},
	{0x97,0x3c},
	{0x98,0x8a}, 
	{0x99,0x10},
	{0x9a,0x50},
	{0x9f,0x64},
	{0x39,0x98}, 
	{0x3f,0x98}, 
	{0x90,0x20},
	{0x91,0x70},
	{0x40,0x36}, 
	{0x41,0x33}, 
	{0x42,0x2a}, 
    {0x43,0x22}, 
	{0x44,0x1b}, 
	{0x45,0x16}, 
	{0x46,0x13}, 
	{0x47,0x10}, 
	{0x48,0x0e}, 
	{0x49,0x0c}, 
	{0x4b,0x0b}, 
	{0x4c,0x0a}, 
	{0x4e,0x09}, 
	{0x4f,0x08}, 
	{0x50,0x08}, 
	{0x5a,0x56},
	{0x51,0x12},
	{0x52,0x0d},
	{0x53,0x92},
	{0x54,0x7d},
	{0x57,0x97},
	{0x58,0x43},
	{0x5a,0xd6},
	/* avoiding too red skin */
	{0x51,0x39},
	{0x52,0x0f}, 
	{0x53,0x3b}, 
	{0x54,0x55}, 
	{0x57,0x7e},
	{0x58,0x05}, 
	{0x5b,0x02},
	{0x5c,0x30},
	{0x6a,0x81},
	{0x23,0x55},
	{0xa0,0x00},
	{0xa1,0x31},
	{0xa2,0x0b}, 
	{0xa3,0x27},
	{0xa4,0x0a},
	{0xa5,0x27},
	{0xa6,0x04},
	{0xa7,0x1a}, 
	{0xa8,0x15}, //16  15
	{0xa9,0x13}, 
	{0xaa,0x18}, 
	{0xab,0x1c}, 
	{0xac,0x3c},
	{0xad,0xe8}, 
	{0xae,0x7b},//7c   0x7b  0x7c 0x7b
	{0xc5,0x55},//0xaa
	{0xc6,0x88},
	{0xc7,0x10},
	{0xc8,0x0d},
	{0xc9,0x10},
	{0xd0,0x69}, //0xb7
	{0xd1,0x00},//00
	{0xd2,0x58},
	{0xd3,0x09},
	{0xd4,0x24},
	{0xee,0x30},
	{0xb0,0xe0},
	{0xb3,0x48},
	{0xb4,0xa3}, 
	{0xb1,0xff},
	{0xb2,0xff}, 
	{0xb4,0x63}, 
	{0xb1,0xca}, //0xba  c0 c8
	{0xb2,0xaa},  //0xa8
	{0x55,0x00},
	{0x56,0x40},
};
static struct msm_camera_i2c_reg_conf bf3905_wb_auto_reg_config[] =
{
	{0x13,0x07},
	{0x01,0x15},
	{0x02,0x23},
};
static struct msm_camera_i2c_reg_conf bf3905_wb_incandescent_reg_config[] =
{
    {0x13,0x05},
    {0x6a,0x81},
	{0x23,0x55},
	{0x01,0x25},
	{0x02,0x0d},
};
static struct msm_camera_i2c_reg_conf bf3905_wb_fluorescent_reg_config[] =
{
        
    {0x13,0x05},
    {0x6a,0x81},
	{0x23,0x55},
	{0x01,0x1c},
	{0x02,0x1a},
};
static struct msm_camera_i2c_reg_conf bf3905_wb_daylight_reg_config[] =
{
    {0x13,0x05},
    {0x6a,0x81},
	{0x23,0x55},
	{0x01,0x14},
	{0x02,0x1a},
};
static struct msm_camera_i2c_reg_conf bf3905_wb_cloudy_reg_config[] =
{
    {0x13,0x05},
    {0x6a,0x81},
	{0x23,0x55},
	{0x01,0x10},
	{0x02,0x22},
};

static struct msm_camera_i2c_reg_conf bf3905_effect_off_reg_config[] =
{
	 {0x70,0x0b},
	 {0x69,0x00},
	 {0x67,0x80},
	 {0x68,0x80},
	 {0xb4,0x03}, 
	 {0x56,0x40},
};

static struct msm_camera_i2c_reg_conf bf3905_effect_mono_reg_config[] =
{
	 {0x70,0x0b},
	 {0x69,0x20},
	 {0x67,0x80},
	 {0x68,0x80},
	 {0xb4,0x03},
	 {0x56,0x40},
	 	  
};

static struct msm_camera_i2c_reg_conf bf3905_effect_negative_reg_config[] =
{
	 {0x70,0x0b},
	 {0x69,0x21},
	 {0x67,0x80},
	 {0x68,0x80},
	 {0xb4,0x03},
	 {0x56,0x40},
};

static struct msm_camera_i2c_reg_conf bf3905_effect_sepia_reg_config[] =
{
	 {0x70,0x0b},
	 {0x69,0x20},
	 {0x67,0x60},
	 {0x68,0xa0},
	 {0xb4,0x03},
	 {0x56,0x40},
};

static struct msm_camera_i2c_reg_conf bf3905_effect_aqua_reg_config[] =
{
     {0x70,0x0b},
	 {0x69,0x20},
	 {0x67,0xe0},
	 {0x68,0x60},
	 {0xb4,0x03},
	 {0x56,0x40},
};
static struct v4l2_subdev_info bf3905_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};
static struct msm_camera_i2c_conf_array bf3905_init_conf[] = {
	{&bf3905_init_settings[0],
	ARRAY_SIZE(bf3905_init_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_i2c_conf_array bf3905_confs[] = {
	{NULL,0, 0, MSM_CAMERA_I2C_BYTE_DATA},
	{NULL,0, 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t bf3905_dimensions[] = {
	{
		.x_output = 0x280,
		.y_output = 0x1E0,
		/*.line_length_pclk = 0x034A,
		.frame_length_lines = 0x022A,
		.vt_pixel_clk = 21000000,
		.op_pixel_clk = 16800000,
		.binning_factor = 1,*/
	},
	{
		.x_output = 0x280,
		.y_output = 0x1E0,
		/*.line_length_pclk = 0x034A,
		.frame_length_lines = 0x022A,
		.vt_pixel_clk = 21000000,
		.op_pixel_clk = 16800000,
		.binning_factor = 1,*/
	},
};

static struct msm_camera_csi_params bf3905_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x18,
};

static struct msm_camera_csi_params *bf3905_csi_params_array[] = {
	&bf3905_csi_params,
	&bf3905_csi_params,
};

static struct msm_sensor_id_info_t bf3905_id_info = {
	.sensor_id_reg_addr = 0x1c,
	.sensor_id = 0x7fa2,
};

static const struct i2c_device_id bf3905_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&bf3905_s_ctrl},
	{ }
};

static struct i2c_driver bf3905_i2c_driver = {
	.id_table = bf3905_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client bf3905_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
	.addr_pos = 0,
	.addr_dir = 0,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&bf3905_i2c_driver);
}


int32_t bf3905_write_init_settings(struct msm_sensor_ctrl_t *s_ctrl)
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
	return rc;
}

int32_t bf3905_mirrorandflip_self_adapt(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	CDBG("%s is called !\n", __func__);
	/*must mend 0x40 if 0x1e is mended in intializtion register sequence*/	
	/*the 4 and 5 bit are used to control mirror and flip */
	if((HW_MIRROR_AND_FLIP << 1) == get_hw_camera_mirror_type())
	{
		rc = msm_camera_i2c_write(
		s_ctrl->sensor_i2c_client,
		0x1e, 0x70,
		MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) 
		{
			pr_err("%s: write register error\n", __func__);
		}
	}	
	return rc;
}

int32_t bf3905_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	uint16_t chipid = 0;
	uint16_t chipidL , chippidM;
	chipidL = chippidM = 0;
	
	rc = msm_camera_i2c_read(
			s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_id_info->sensor_id_reg_addr, &chippidM,
			MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
		pr_err("%s: %s: read id failed\n", __func__,
			s_ctrl->sensordata->sensor_name);
		return rc;
	}

	rc = msm_camera_i2c_read(
			s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_id_info->sensor_id_reg_addr+1, &chipidL,
			MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
		pr_err("%s: %s: read id failed\n", __func__,
			s_ctrl->sensordata->sensor_name);
		return rc;
	}
	chipid = (chippidM << 8) | chipidL;
	printk("reg_addr:%x;bf3905_match_id: %d\n", s_ctrl->sensor_id_info->sensor_id_reg_addr,chipid);
	if (chipid != s_ctrl->sensor_id_info->sensor_id) {
		pr_err("msm_sensor_match_id chip id doesnot match\n");
		return -ENODEV;
	}
	return rc;
}



/*func for bf3905 to set effect*/
int32_t bf3905_sensor_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int effect)
{
	struct msm_camera_i2c_reg_conf *reg_conf_tbl = NULL;
	uint16_t num_of_items_in_table = 0;
	int rc = 0;

	printk("%s, to set effect = %d \n", __func__,effect);
	switch (effect)
	{
		case CAMERA_EFFECT_OFF:
			reg_conf_tbl = &bf3905_effect_off_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(bf3905_effect_off_reg_config);
			break;
		case CAMERA_EFFECT_MONO:
			reg_conf_tbl = &bf3905_effect_mono_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(bf3905_effect_mono_reg_config);
			break;
		case CAMERA_EFFECT_NEGATIVE:
			reg_conf_tbl = &bf3905_effect_negative_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(bf3905_effect_negative_reg_config);
			break;
		case CAMERA_EFFECT_SEPIA:
			reg_conf_tbl = &bf3905_effect_sepia_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(bf3905_effect_sepia_reg_config);
			break;
		case CAMERA_EFFECT_AQUA:
			reg_conf_tbl = &bf3905_effect_aqua_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(bf3905_effect_aqua_reg_config);
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
int32_t bf3905_sensor_set_wb(struct msm_sensor_ctrl_t *s_ctrl, int wb)
{
	struct msm_camera_i2c_reg_conf *reg_conf_tbl = NULL;
	uint16_t num_of_items_in_table = 0;
	int rc = 0;

	printk("%s, to set wb = %d \n", __func__,wb);
	switch (wb)
	{
		case CAMERA_WB_AUTO:
			reg_conf_tbl = &bf3905_wb_auto_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(bf3905_wb_auto_reg_config);
			break;
		case CAMERA_WB_INCANDESCENT:
			reg_conf_tbl = &bf3905_wb_incandescent_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(bf3905_wb_incandescent_reg_config);
			break;
		case CAMERA_WB_FLUORESCENT:
			reg_conf_tbl = &bf3905_wb_fluorescent_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(bf3905_wb_fluorescent_reg_config);
			break;
		case CAMERA_WB_DAYLIGHT:
			reg_conf_tbl = &bf3905_wb_daylight_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(bf3905_wb_daylight_reg_config);
			break;
		case CAMERA_WB_CLOUDY_DAYLIGHT:
			reg_conf_tbl = &bf3905_wb_cloudy_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(bf3905_wb_cloudy_reg_config);
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

static struct v4l2_subdev_core_ops bf3905_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops bf3905_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops bf3905_subdev_ops = {
	.core = &bf3905_subdev_core_ops,
	.video  = &bf3905_subdev_video_ops,
};

static struct msm_sensor_fn_t bf3905_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_csi_setting = msm_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_write_init_settings = bf3905_write_init_settings,
	.sensor_match_id = bf3905_match_id,
	.sensor_set_wb = bf3905_sensor_set_wb,
	.sensor_set_effect = bf3905_sensor_set_effect,
	.sensor_mirrorandflip_self_adapt = bf3905_mirrorandflip_self_adapt,
};
 
static struct msm_sensor_reg_t bf3905_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = bf3905_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(bf3905_start_settings),
	.stop_stream_conf = bf3905_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(bf3905_stop_settings),
	.group_hold_on_conf_size = 0,
	.group_hold_off_conf_size = 0,
	.init_settings = &bf3905_init_conf[0],
	.init_size = ARRAY_SIZE(bf3905_init_conf),
	.mode_settings = &bf3905_confs[0],
	.output_settings = &bf3905_dimensions[0],
	.num_conf = ARRAY_SIZE(bf3905_dimensions),
};

static struct msm_sensor_ctrl_t bf3905_s_ctrl = {
	.msm_sensor_reg = &bf3905_regs,
	.sensor_i2c_client = &bf3905_sensor_i2c_client,
	.sensor_i2c_addr = 0xDC, 
	.sensor_id_info = &bf3905_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &bf3905_csi_params_array[0],
	.msm_sensor_mutex = &bf3905_mut,
	.sensor_i2c_driver = &bf3905_i2c_driver,
	.sensor_v4l2_subdev_info = bf3905_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(bf3905_subdev_info),
	.sensor_v4l2_subdev_ops = &bf3905_subdev_ops,
	.func_tbl = &bf3905_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
	.sensor_name = "23060075FF-BYD-B",
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("BYD VGA YUV sensor driver");
MODULE_LICENSE("GPL v2");

