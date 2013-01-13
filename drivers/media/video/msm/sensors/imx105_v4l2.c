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
 *
 */
#include "msm_sensor.h"
#include "msm.h" 
//#include <hsad/config_interface.h>
#define SENSOR_NAME "imx105_sunny"
#define PLATFORM_DRIVER_NAME "msm_camera_imx105_sunny"
#define imx105_sunny_obj imx105_sunny_##obj
int imx105_sunny_otp_read_flag = 0;
unsigned short  otp_id[8] = {0};
static unsigned short  imx105_sunny_otp_awb[3] = {0};
unsigned short  imx105_sunny_otp_af[2] = {0};

DEFINE_MUTEX(imx105_sunny_mut);
static struct msm_sensor_ctrl_t imx105_sunny_s_ctrl;

static struct msm_camera_i2c_reg_conf imx105_sunny_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf imx105_sunny_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf imx105_sunny_groupon_settings[] = {
	{0x104, 0x01},
};

static struct msm_camera_i2c_reg_conf imx105_sunny_groupoff_settings[] = {
	{0x104, 0x00},
};

static struct msm_camera_i2c_reg_conf imx105_sunny_prev_settings[] = {

	{0x0305,0x04},     //pre_pll_clk_div[7:0]
	{0x0307,0x38},     //pll_multiplier[7:0]
	{0x30A4,0x02},
	{0x303C,0x96},
#if 0
	{0x0340,0x04},     //frame_length_lines[15:8]
	{0x0341,0xF2},     //frame_length_lines[7:0]
#else
	//29fps
	//{0x0340,0x05},     //frame_length_lines[15:8]
	//{0x0341,0x1E},     //frame_length_lines[7:0]
	//29.5fps change for AEC unstable
	{0x0340,0x05},     //frame_length_lines[15:8]
	{0x0341,0x08},     //frame_length_lines[7:0]
#endif
	{0x0342,0x0D},     //line_length_pck[15:8]
	{0x0343,0xD0},     //line_length_pck[7:0]
	{0x0344,0x00},     //x_addr_start[15:8]
	{0x0345,0x04},     //x_addr_start[7:0]
	{0x0346,0x00},     //y_addr_start[15:8]
	{0x0347,0x24},     //y_addr_start[7:0]
	{0x0348,0x0C},     //x_addr_end[15:8]
	{0x0349,0xD3},     //x_addr_end[7:0]
	{0x034A,0x09},     //y_addr_end[15:8]
	{0x034B,0xC3},     //y_addr_end[7:0]
	{0x034C,0x06},     //x_output_size[15:8]
	{0x034D,0x68},     //x_output_size[7:0]
	{0x034E,0x04},     //y_output_size[15:8]
	{0x034F,0xD0},     //y_output_size[7:0]
	{0x0381,0x01},     //x_even_inc[3:0]
	{0x0383,0x03},     //x_odd_inc[3:0]
	{0x0385,0x01},     //y_even_inc[7:0]
	{0x0387,0x03},     //y_odd_inc[7:0]
	{0x3040,0x08},
	{0x3041,0x97},
	{0x3048,0x01},
	{0x309B,0x08},
	{0x309C,0x34},
	{0x30D5,0x09},
	{0x30D6,0x01},
	{0x30D7,0x01},
	{0x30DE,0x02},
	{0x3318,0x72},

	//test register
//	{0x3282,0x01},
//	{0x3032,0x3C},
//	{0x0600,0x00},
//	{0x0601,0x02},
};

static struct msm_camera_i2c_reg_conf imx105_sunny_snap_settings[] = { 

	//full frame mode
	{0x0305,0x04},     //pre_pll_clk_div[7:0]
	{0x0307,0x38},     //pll_multiplier[7:0]
	{0x30A4,0x02},
	{0x303C,0x96},

	{0x0340,0x09},     //frame_length_lines[15:8]
	{0x0341,0xE6},     //frame_length_lines[7:0]
	{0x0342,0x0D},     //line_length_pck[15:8]
	{0x0343,0xD0},     //line_length_pck[7:0]
	{0x0344,0x00},     //x_addr_start[15:8]
	{0x0345,0x04},     //x_addr_start[7:0]
	{0x0346,0x00},     //y_addr_start[15:8]
	{0x0347,0x24},     //y_addr_start[7:0]
	{0x0348,0x0C},     //x_addr_end[15:8]
	{0x0349,0xD3},     //x_addr_end[7:0]
	{0x034A,0x09},     //y_addr_end[15:8]
	{0x034B,0xC3},     //y_addr_end[7:0]
	{0x034C,0x0C},     //x_output_size[15:8]
	{0x034D,0xD0},     //x_output_size[7:0]
	{0x034E,0x09},     //y_output_size[15:8]
	{0x034F,0xA0},     //y_output_size[7:0]
	{0x0381,0x01},     //x_even_inc[3:0]
	{0x0383,0x01},     //x_odd_inc[3:0]
	{0x0385,0x01},     //y_even_inc[7:0]
	{0x0387,0x01},     //y_odd_inc[7:0]
	{0x3040,0x08},
	{0x3041,0x97},
	{0x3048,0x00},
	{0x309B,0x00},
	{0x309C,0x34},
	{0x30D5,0x00},
	{0x30D6,0x85},
	{0x30D7,0x2A},
	{0x30DE,0x00},
	{0x3318,0x62},

	//test register
//	{0x3282,0x01},
//	{0x3032,0x3C},
//	{0x0600,0x00},
//	{0x0601,0x02},
};

static struct msm_camera_i2c_reg_conf imx105_sunny_1080p_settings[] = {
{0x0305,0x04},                                         
{0x0307,0x3A},                                         
{0x30A4,0x02},                                         
{0x303C,0x96},   

{0x104, 0x01},

{ 0x0340, 0x06 },        // frame_length_lines[15:8]
{ 0x0341, 0x5E },        // frame_length_lines[7:0]
{ 0x0342, 0x0d },        // line_length_pck[15:8]
{ 0x0343, 0x48 },        // line_length_pck[7:0]

{ 0x0344, 0x00 },        // x_addr_start[15:8]
{ 0x0345, 0xE4 },        // x_addr_start[7:0]
{ 0x0346, 0x01 },        // y_addr_start[15:8]
{ 0x0347, 0xD8 },        // y_addr_start[7:0]
{ 0x0348, 0x0B },        // x_addr_end[15:8]
{ 0x0349, 0xF3 },        // x_addr_end[7:0]
{ 0x034A, 0x08 },        // y_addr_end[15:8]
{ 0x034B, 0x0F },        // y_addr_end[7:0]

{ 0x034C, 0x0B },        // x_output_size[15:8]
{ 0x034D, 0x10 },        // x_output_size[7:0]
{ 0x034E, 0x06 },        // y_output_size[15:8]
{ 0x034F, 0x38 },        // y_output_size[7:0]

{ 0x0381, 0x01 },        // x_even_inc[3:0]
{ 0x0383, 0x01 },        // x_odd_inc[3:0]
{ 0x0385, 0x01 },        // y_even_inc[7:0]
{ 0x0387, 0x01 },        // y_odd_inc[7:0]
{ 0x303E, 0x41 },
{ 0x3040, 0x08 },
{ 0x3041, 0x97 },
{ 0x3048, 0x00 },
{ 0x304C, 0x50 },
{ 0x304D, 0x03 },
{ 0x309B, 0x00 },
{ 0x309C, 0x34 },
{ 0x30D5, 0x00 },
{ 0x30D6, 0x85 },
{ 0x30D7, 0x2A },
{ 0x30DE, 0x00 },
{ 0x3102, 0x10 },
{ 0x3103, 0x44 },
{ 0x3104, 0x40 },
{ 0x3105, 0x00 },
{ 0x3106, 0x10 },
{ 0x3107, 0x01 },
{ 0x315C, 0x76 },
{ 0x315D, 0x75 },
{ 0x316E, 0x77 },
{ 0x316F, 0x76 },
{ 0x3318, 0x62 },
{0x104, 0x00},


};

static struct msm_camera_i2c_reg_conf imx105_sunny_recommend_settings[] = {

	//Global setting
	{0x3031,0x10},
	{0x3064,0x12},
	{0x3087,0x57},
	{0x308A,0x35},
	{0x3091,0x41},
	{0x3098,0x03},
	{0x3099,0xC0},
	{0x309A,0xA3},
	{0x309D,0x94},
	{0x30AB,0x01},
	{0x30AD,0x08},
	{0x30B1,0x03},
	{0x30C4,0x13},
	{0x30F3,0x03},
	{0x3116,0x31},
	{0x3117,0x38},
	{0x3138,0x28},
	{0x3137,0x14},
	{0x3139,0x2E},
	{0x314D,0x2A},
	{0x328F,0x00},
	{0x3343,0x04},
	//Black level Setting
	{0x3032,0x40},
	{0x0101,0x00},	//mirror and flip off
	//test register
//	{0x3282,0x01},
//	{0x3032,0x3C},
//	{0x0600,0x00},
//	{0x0601,0x02},
};
static struct v4l2_subdev_info imx105_sunny_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array imx105_sunny_init_conf[] = {
	{&imx105_sunny_recommend_settings[0],
	ARRAY_SIZE(imx105_sunny_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array imx105_sunny_confs[] = {
	{&imx105_sunny_snap_settings[0],
	ARRAY_SIZE(imx105_sunny_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx105_sunny_prev_settings[0],
	ARRAY_SIZE(imx105_sunny_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx105_sunny_1080p_settings[0],
	ARRAY_SIZE(imx105_sunny_1080p_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t imx105_sunny_dimensions[] = {
	/* snapshot */
    {   //3280 * 2464
		.x_output = 0X0CD0,
		.y_output = 0X09AE,//0X09A0,
		.line_length_pclk = 0X0DD0,
		.frame_length_lines = 0X09E6,
		.vt_pixel_clk = 134400000,
		.op_pixel_clk = 134400000,
		.binning_factor = 1,
	},

	/* preview */
    {    //1640 * 1232
		.x_output = 0X668,
		.y_output = 0X4D6,//0X4D0,
		.line_length_pclk = 0XDD0,
//		.frame_length_lines = 0X4F2,
//		.frame_length_lines = 0X51E,    //29fps
		.frame_length_lines = 0X508,    //29.5fps
		.vt_pixel_clk = 134400000,
		.op_pixel_clk = 134400000,
		.binning_factor = 1,
	},


	/* 1080p 2832*1592 */
    {    //
		.x_output = 0XB10,  //2832   
		.y_output = 0X63E, //0X638,  //1592
		.line_length_pclk = 0Xd48,   //3400   
		.frame_length_lines = 0X65E,  //  1630
		.vt_pixel_clk = 139200000,
		.op_pixel_clk = 139200000,
		.binning_factor = 1,
	},

};
#if 0
static struct msm_camera_csid_vc_cfg imx105_sunny_cid_cfg[] = {
	{0, CSI_RAW10, CSI_DECODE_10BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params imx105_sunny_csi_params = {
	.csid_params = {
		.lane_assign = 0xe4,
		.lane_cnt = 2,
		.lut_params = {
			.num_cid = 2,
			.vc_cfg = imx105_sunny_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 2,
		.settle_cnt = 0x1B,
	},
};

static struct msm_camera_csi2_params *imx105_sunny_csi_params_array[] = {
	&imx105_sunny_csi_params,
	&imx105_sunny_csi_params,
	&imx105_sunny_csi_params,
};
#endif
static struct msm_camera_csi_params imx105_sunny_csic_params = {
	.data_format = CSI_10BIT,
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x1B,
};

static struct msm_camera_csi_params *imx105_sunny_csic_params_array[] = {
	&imx105_sunny_csic_params,
	&imx105_sunny_csic_params,
	&imx105_sunny_csic_params,
	&imx105_sunny_csic_params,
	&imx105_sunny_csic_params,
};
static struct msm_sensor_output_reg_addr_t imx105_sunny_reg_addr = {
	.x_output = 0x34C,
	.y_output = 0x34E,
	.line_length_pclk = 0x342,
	.frame_length_lines = 0x340,
};

static struct msm_sensor_id_info_t imx105_sunny_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x0105,
};

static struct msm_sensor_exp_gain_info_t imx105_sunny_exp_gain_info = {
	.coarse_int_time_addr = 0x202,
	.global_gain_addr = 0x204,
	.vert_offset = 5,        //((frame_length_lines)-(vert_offset)),the max value of coarse_int_time
};

static int imx105_sunny_otp_data_read(void)
{
	int32_t rc = 0;
	int32_t count= 0;
	uint16_t otpdata = 0;
	uint16_t addr = 0x3572;    //Bank E  MAX CODE
	uint16_t bank_set = 0x0E;

	for(count = 0; count <5; count++)
	{
		rc = msm_camera_i2c_write((&imx105_sunny_s_ctrl)->sensor_i2c_client, 0x34C9, bank_set, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) 
		{
			CDBG("%s: Error Setting 0x34C9 values!\n", __func__);
			return rc;
		}
		rc = msm_camera_i2c_read((&imx105_sunny_s_ctrl)->sensor_i2c_client, addr, &otpdata, MSM_CAMERA_I2C_WORD_DATA);
		if (rc < 0) 
		{
			CDBG("%s: Error Reading OTP data @ 0x%x\n", __func__, addr);
			return rc;
		}
		printk("imx105_sunny ======= kernel otp addr = 0x%x, otpdata = 0x%x, bank_set = 0x%x\n",addr,otpdata, bank_set);
		if(0x0 == otpdata)
		{
			addr = addr - 0x18;  //three banks
			bank_set = bank_set - 0x3;
		}
		else
		{
			break;
		}
	}
	
	if(5 == count)
	{
		CDBG("%s: OTP data doesn't exit!\n", __func__);
		printk("imx105_sunny ===========  enter%s: OTP data doesn't exit!\n", __func__);
		imx105_sunny_otp_read_flag = -1;
		rc = -ENXIO;
		return rc;
	}

	printk("imx105_sunny ===========  enter%s: addr = 0x%x,  go on to read otp data!\n", __func__, addr);
	
	/******************read AF information *******************/
	/******************start code*******************/
	addr = addr -0x2;
	rc = msm_camera_i2c_read((&imx105_sunny_s_ctrl)->sensor_i2c_client, addr, &otpdata, MSM_CAMERA_I2C_WORD_DATA);
	if(rc < 0) 
	{
		CDBG("%s: Error Reading OTP data @ 0x%x\n", __func__, addr);
		return rc;
	}
	imx105_sunny_otp_af[0] = otpdata&0xFFFF;


	/*max code*/
	addr += 0x2;
	rc = msm_camera_i2c_read((&imx105_sunny_s_ctrl)->sensor_i2c_client, addr, &otpdata, MSM_CAMERA_I2C_WORD_DATA);
	if(rc < 0) 
	{
		CDBG("%s: Error Reading OTP data @ 0x%x\n", __func__, addr);
		return rc;
	}
	imx105_sunny_otp_af[1] = otpdata&0xFFFF;

	bank_set = bank_set -0x1;
	printk("imx105_sunny ============RGB  bank_set 0x%x\n", bank_set);
	msm_camera_i2c_write((&imx105_sunny_s_ctrl)->sensor_i2c_client, 0x34C9, bank_set, MSM_CAMERA_I2C_BYTE_DATA);

	
	/*R/G data*/
	addr = addr - 0xA;
	rc = msm_camera_i2c_read((&imx105_sunny_s_ctrl)->sensor_i2c_client, addr, &otpdata, MSM_CAMERA_I2C_WORD_DATA);
	if(rc < 0) 
	{
		CDBG("%s: Error Reading OTP data @ 0x%x\n", __func__, addr);
		return rc;
	}
	imx105_sunny_otp_awb[0] = otpdata&0xFFFF;


	/*B/G data*/
	addr += 0x2;
	rc = msm_camera_i2c_read((&imx105_sunny_s_ctrl)->sensor_i2c_client, addr, &otpdata, MSM_CAMERA_I2C_WORD_DATA);
	if(rc < 0) 
	{
		CDBG("%s: Error Reading OTP data @ 0x%x\n", __func__, addr);
		return rc;
	}
	imx105_sunny_otp_awb[1] = otpdata&0xFFFF;


	/*GB/GR data*/
	addr += 0x2;
	rc = msm_camera_i2c_read((&imx105_sunny_s_ctrl)->sensor_i2c_client, addr, &otpdata, MSM_CAMERA_I2C_WORD_DATA);
	if(rc < 0) 
	{
		CDBG("%s: Error Reading OTP data @ 0x%x\n", __func__, addr);
		return rc;
	}
	imx105_sunny_otp_awb[2] = otpdata&0xFFFF;

	printk("imx105_sunny==== r_over_g=0x%x, %d.\n \t\t\t\tb_over_g=0x%x, %d, \n \t\t\t\tgr_over_gb=0x%x,  %d\n \t\t\t imx105_sunny_otp_af[0] = %d, imx105_sunny_otp_af[1] = %d.\n",
        imx105_sunny_otp_awb[0], imx105_sunny_otp_awb[0], 
        imx105_sunny_otp_awb[1], imx105_sunny_otp_awb[1], 
        imx105_sunny_otp_awb[2], imx105_sunny_otp_awb[2],
        imx105_sunny_otp_af[0], imx105_sunny_otp_af[1]);
	
	return rc;
}

static int imx105_sunny_otp_data_update(struct sensor_cfg_data *cfg)
{
    cfg->cfg.calib_info.r_over_g = imx105_sunny_otp_awb[0];
    cfg->cfg.calib_info.b_over_g = imx105_sunny_otp_awb[1];
    cfg->cfg.calib_info.gr_over_gb = imx105_sunny_otp_awb[2];
    cfg->cfg.calib_info.af_start_code= imx105_sunny_otp_af[0];
    cfg->cfg.calib_info.af_max_code= imx105_sunny_otp_af[1];

    printk("imx105_sunny==== r_over_g=0x%x  b_over_g=0x%x  gr_over_gb=0x%x\n ,imx105_sunny_otp_af[0] is 0x%x, imx105_sunny_otp_af[1] is 0x%x.\n   "
        ,cfg->cfg.calib_info.r_over_g,cfg->cfg.calib_info.b_over_g,cfg->cfg.calib_info.gr_over_gb, imx105_sunny_otp_af[0], imx105_sunny_otp_af[1]);
    
    return imx105_sunny_otp_read_flag;
//	return -1;
}
#if 0

int32_t imx105_sunny_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
    int rc=0;
	int reset_gpio;
	int vcm_pwd_gpio;

	//msm_sensor_expand_power_up(s_ctrl);
	
	reset_gpio = get_gpio_num_by_name("CAM_RST");
	vcm_pwd_gpio = get_gpio_num_by_name("CAM_AF_SHDN");
	if(reset_gpio < 0)
	{
		printk(KERN_ERR"%s get_gpio_num_by_name fail\n",__func__);
		return reset_gpio;
	}
	if(vcm_pwd_gpio < 0)
	{
		printk(KERN_ERR"%s get_gpio_num_by_name fail\n",__func__);
		return vcm_pwd_gpio;
	}
		
	rc = gpio_request(reset_gpio,"imx105_sunny");
	if (rc) 
	{
		gpio_free(reset_gpio);
		rc = gpio_request(reset_gpio,"imx105_sunny");
		if(rc)
		{
		    printk("%s gpio_request(%d) again fail \n",__func__,reset_gpio);
			return rc;
		}
		printk("%s gpio_request(%d) again success\n",__func__,reset_gpio);
	}
    
    rc = gpio_request(vcm_pwd_gpio,"imx105_sunny");
	if (rc) 
	{
		gpio_free(vcm_pwd_gpio);
		rc = gpio_request(vcm_pwd_gpio,"imx105_sunny");
		if(rc)
		{
		    printk("%s gpio_request(%d) again fail \n",__func__,vcm_pwd_gpio);
			return rc;
		}
		printk("%s gpio_request(%d) again success\n",__func__,vcm_pwd_gpio);
	}
	gpio_direction_output(reset_gpio, 1);
    gpio_direction_output(vcm_pwd_gpio, 1);
    msleep(10);        
    
    gpio_direction_output(vcm_pwd_gpio, 0);
    msleep(10);
    
    gpio_direction_output(vcm_pwd_gpio, 1);
    msleep(10);

    return 0;
}

int32_t imx105_sunny_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	int reset_gpio;
	int vcm_pwd_gpio;

	reset_gpio = get_gpio_num_by_name("CAM_RST");
	vcm_pwd_gpio = get_gpio_num_by_name("CAM_AF_SHDN");
	if(reset_gpio < 0)
	{
		printk(KERN_ERR"%s get_gpio_num_by_name fail\n",__func__);
		return reset_gpio;
	}
	if(vcm_pwd_gpio < 0)
	{
		printk(KERN_ERR"%s get_gpio_num_by_name fail\n",__func__);
		return vcm_pwd_gpio;
	}
	
    gpio_direction_output(reset_gpio, 0);
	gpio_direction_output(vcm_pwd_gpio, 0);
    gpio_free(reset_gpio);
    gpio_free(vcm_pwd_gpio);
    
    msm_sensor_expand_power_down(s_ctrl);
    return 0;
}
#endif
int32_t imx105_sunny_msm_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	int ret = 0;
	struct msm_sensor_ctrl_t *s_ctrl;
	CDBG("%s_i2c_probe called\n", client->name);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		CDBG("i2c_check_functionality failed\n");
		rc = -EFAULT;
		return rc;
	}

	s_ctrl = (struct msm_sensor_ctrl_t *)(id->driver_data);
	if (s_ctrl->sensor_i2c_client != NULL) {
		s_ctrl->sensor_i2c_client->client = client;
		if (s_ctrl->sensor_i2c_addr != 0)
			s_ctrl->sensor_i2c_client->client->addr =
				s_ctrl->sensor_i2c_addr;
	} else {
		rc = -EFAULT;
		return rc;
	}

	s_ctrl->sensordata = client->dev.platform_data;
	if (s_ctrl->sensordata == NULL) {
		pr_err("%s: NULL sensor data\n", __func__);
		return -EFAULT;
	}

	rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
	if (rc < 0)
		goto probe_fail;

	rc = msm_sensor_match_id(s_ctrl);
	if (rc < 0)
		goto probe_fail;

	/*zhanglijun add for otp begin*/
	ret= imx105_sunny_otp_data_read();
    if(ret < 0)
        printk("imx105_sunny ========= otp read data error!\n");
	/*zhanglijun add for otp end*/
#if 0
	if (s_ctrl->sensor_eeprom_client != NULL) {
		struct msm_camera_eeprom_client *eeprom_client =
			s_ctrl->sensor_eeprom_client;
		if (eeprom_client->func_tbl.eeprom_init != NULL &&
			eeprom_client->func_tbl.eeprom_release != NULL) {
			rc = eeprom_client->func_tbl.eeprom_init(
				eeprom_client,
				s_ctrl->sensor_i2c_client->client->adapter);
			if (rc < 0)
				goto probe_fail;

			rc = msm_camera_eeprom_read_tbl(eeprom_client,
			eeprom_client->read_tbl, eeprom_client->read_tbl_size);
			eeprom_client->func_tbl.eeprom_release(eeprom_client);
			if (rc < 0)
				goto probe_fail;
		}
	}
#endif
	snprintf(s_ctrl->sensor_v4l2_subdev.name,
		sizeof(s_ctrl->sensor_v4l2_subdev.name), "%s", id->name);
	v4l2_i2c_subdev_init(&s_ctrl->sensor_v4l2_subdev, client,
		s_ctrl->sensor_v4l2_subdev_ops);
    
    if(s_ctrl->sensor_name)
    {
		strncpy((char *)s_ctrl->sensordata->sensor_name, s_ctrl->sensor_name, CAMERA_NAME_LEN -1);
	    printk("the name for project menu is %s\n", s_ctrl->sensordata->sensor_name);
    }

	msm_sensor_register(&s_ctrl->sensor_v4l2_subdev);
	goto power_down;
probe_fail:
	CDBG("%s_i2c_probe failed\n", client->name);
power_down:
	if (rc > 0)
		rc = 0;
	s_ctrl->func_tbl->sensor_power_down(s_ctrl);
	return rc;
}

#if 0 /* not use temply */
static struct sensor_calib_data imx105_sunny_calib_data;
#endif

static const struct i2c_device_id imx105_sunny_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&imx105_sunny_s_ctrl},
	{ }
};

static struct i2c_driver imx105_sunny_i2c_driver = {
	.id_table = imx105_sunny_i2c_id,
//	.probe  = imx105_sunny_msm_sensor_i2c_probe,
    .probe = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client imx105_sunny_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .addr_pos = 1,
	.addr_dir = -1,
};

#if 0 /* not use temply */
static struct msm_camera_i2c_client imx105_sunny_eeprom_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
};

static struct msm_camera_eeprom_read_t imx105_sunny_eeprom_read_tbl[] = {
	{0x10, &imx105_sunny_calib_data.r_over_g, 2, 1},
	{0x12, &imx105_sunny_calib_data.b_over_g, 2, 1},
	{0x14, &imx105_sunny_calib_data.gr_over_gb, 2, 1},
};

static struct msm_camera_eeprom_data_t imx105_sunny_eeprom_data_tbl[] = {
	{&imx105_sunny_calib_data, sizeof(struct sensor_calib_data)},
};

static struct msm_camera_eeprom_client imx105_sunny_eeprom_client = {
	.i2c_client = &imx105_sunny_eeprom_i2c_client,
	.i2c_addr = 0xA4,

	.func_tbl = {
		.eeprom_set_dev_addr = NULL,
		.eeprom_init = msm_camera_eeprom_init,
		.eeprom_release = msm_camera_eeprom_release,
		.eeprom_get_data = msm_camera_eeprom_get_data,
	},

	.read_tbl = imx105_sunny_eeprom_read_tbl,
	.read_tbl_size = ARRAY_SIZE(imx105_sunny_eeprom_read_tbl),
	.data_tbl = imx105_sunny_eeprom_data_tbl,
	.data_tbl_size = ARRAY_SIZE(imx105_sunny_eeprom_data_tbl),
};
#endif

static int __init imx105_sunny_init_module(void)
{
    printk(KERN_ERR "%s: E\n", __func__);
	return i2c_add_driver(&imx105_sunny_i2c_driver);
}

static struct v4l2_subdev_core_ops imx105_sunny_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops imx105_sunny_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops imx105_sunny_subdev_ops = {
	.core = &imx105_sunny_subdev_core_ops,
	.video  = &imx105_sunny_subdev_video_ops,
};

static struct msm_sensor_fn_t imx105_sunny_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = msm_sensor_write_exp_gain1,
	.sensor_write_snapshot_exp_gain = msm_sensor_write_exp_gain1, 
	.sensor_setting = msm_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
//	.sensor_power_up = imx105_sunny_sensor_power_up,
//	.sensor_power_down = imx105_sunny_sensor_power_down,
	.sensor_otp_reading = imx105_sunny_otp_data_update,
	.sensor_csi_setting = msm_sensor_setting1,
};

static struct msm_sensor_reg_t imx105_sunny_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = imx105_sunny_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(imx105_sunny_start_settings),
	.stop_stream_conf = imx105_sunny_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(imx105_sunny_stop_settings),
	.group_hold_on_conf = imx105_sunny_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(imx105_sunny_groupon_settings),
	.group_hold_off_conf = imx105_sunny_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(imx105_sunny_groupoff_settings),
	.init_settings = &imx105_sunny_init_conf[0],
	.init_size = ARRAY_SIZE(imx105_sunny_init_conf),
	.mode_settings = &imx105_sunny_confs[0],
	.output_settings = &imx105_sunny_dimensions[0],
	.num_conf = ARRAY_SIZE(imx105_sunny_confs),
};

static struct msm_sensor_ctrl_t imx105_sunny_s_ctrl = {
	.msm_sensor_reg = &imx105_sunny_regs,
	.sensor_i2c_client = &imx105_sunny_sensor_i2c_client,
	.sensor_i2c_addr = 0x6C >> 1,
	#if 0 /* not use temply */
	.sensor_eeprom_client = &imx105_sunny_eeprom_client,
	#endif
	.sensor_output_reg_addr = &imx105_sunny_reg_addr,
	.sensor_id_info = &imx105_sunny_id_info,
	.sensor_exp_gain_info = &imx105_sunny_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	//.csi_params = &imx105_sunny_csi_params_array[0],
	.csic_params = &imx105_sunny_csic_params_array[0],
	.msm_sensor_mutex = &imx105_sunny_mut,
	.sensor_i2c_driver = &imx105_sunny_i2c_driver,
	.sensor_v4l2_subdev_info = imx105_sunny_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(imx105_sunny_subdev_info),
	.sensor_v4l2_subdev_ops = &imx105_sunny_subdev_ops,
	.func_tbl = &imx105_sunny_func_tbl,
    .clk_rate = MSM_SENSOR_MCLK_48HZ,
	.sensor_name = "23060093FA-IMX-S",

};

module_init(imx105_sunny_init_module);
MODULE_DESCRIPTION("sunny 8MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
