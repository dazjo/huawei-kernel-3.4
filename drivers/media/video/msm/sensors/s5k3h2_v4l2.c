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
//#include <hsad/config_interface.h>
#define SENSOR_NAME "s5k3h2"
#define PLATFORM_DRIVER_NAME "msm_camera_s5k3h2"
#define s5k3h2_obj s5k3h2_##obj
#define VIDEO_1080P_RECORD 1

DEFINE_MUTEX(s5k3h2_mut);
static struct msm_sensor_ctrl_t s5k3h2_s_ctrl;

static struct msm_camera_i2c_reg_conf s5k3h2_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf s5k3h2_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf s5k3h2_groupon_settings[] = {
	{0x104, 0x01},
};

static struct msm_camera_i2c_reg_conf s5k3h2_groupoff_settings[] = {
	{0x104, 0x00},
};


static struct msm_camera_i2c_reg_conf s5k3h2_prev_settings[] = {
	/*Timing configuration*/
	{0x0200, 0x02},/*FINE_INTEGRATION_TIME_*/
	{0x0201, 0x50},
	{0x0202, 0x04},/*COARSE_INTEGRATION_TIME*/
	{0x0203, 0xDB},
	{0x0204, 0x00},/*ANALOG_GAIN*/
	{0x0205, 0x20},
	{0x0342, 0x0D},/*LINE_LENGTH_PCK*/
	{0x0343, 0x8E},
	{0x0340, 0x04},/*FRAME_LENGTH_LINES*/
	{0x0341, 0xE0},
	/*Output Size (1640x1232)*/
	{0x0344, 0x00},/*X_ADDR_START*/
	{0x0345, 0x00},
	{0x0346, 0x00},/*Y_ADDR_START*/
	{0x0347, 0x00},
	{0x0348, 0x0C},/*X_ADDR_END*/
	{0x0349, 0xCD},
	{0x034A, 0x09},/*Y_ADDR_END*/
	{0x034B, 0x9F},
	{0x0381, 0x01},/*X_EVEN_INC*/
	{0x0383, 0x03},/*X_ODD_INC*/
	{0x0385, 0x01},/*Y_EVEN_INC*/
	{0x0387, 0x03},/*Y_ODD_INC*/
	{0x0401, 0x00},/*DERATING_EN*/
	{0x0405, 0x10},
	{0x0700, 0x05},/*FIFO_WATER_MARK_PIXELS*/
	{0x0701, 0x30},
	{0x034C, 0x06},/*X_OUTPUT_SIZE*/
	{0x034D, 0x68},
	{0x034E, 0x04},/*Y_OUTPUT_SIZE*/
	{0x034F, 0xD0},
	/*Manufacture Setting*/
	{0x300E, 0xED},
	{0x301D, 0x80},
	{0x301A, 0x77},

/*zhanglijun  add begin*/
	   /* PLL */
    {0x0305, 0x08},/*PRE_PLL_CLK_DIV*/
    {0x0306, 0x00},/*PLL_MULTIPLIER*/
    {0x0307, 0x6C},/*PLL_MULTIPLIER*/
    {0x0303, 0x01},/*VT_SYS_CLK_DIV*/
    {0x0301, 0x05},/*VT_PIX_CLK_DIV*/
    {0x030B, 0x01},/*OP_SYS_CLK_DIV*/
    {0x0309, 0x05},/*OP_PIX_CLK_DIV*/
    {0x30CC, 0xB0},/*DPHY_BAND_CTRL*/
    {0x31A1, 0x56},/*BINNING*/
/*zhanglijun  add end*/
};

static struct msm_camera_i2c_reg_conf s5k3h2_snap_settings[] = {
	/*Timing configuration*/
	{0x0200, 0x02},/*FINE_INTEGRATION_TIME_*/
	{0x0201, 0x50},
	{0x0202, 0x04},/*COARSE_INTEGRATION_TIME*/
	{0x0203, 0xE7},
	{0x0204, 0x00},/*ANALOG_GAIN*/
	{0x0205, 0x20},
	{0x0342, 0x0D},/*LINE_LENGTH_PCK*/
	{0x0343, 0x8E},
	{0x0340, 0x09},/*FRAME_LENGTH_LINES*/
	{0x0341, 0xC0},
	/*Output Size (3280x2464)*/
	{0x0344, 0x00},/*X_ADDR_START*/
	{0x0345, 0x00},
	{0x0346, 0x00},/*Y_ADDR_START*/
	{0x0347, 0x00},
	{0x0348, 0x0C},/*X_ADDR_END*/
	{0x0349, 0xCF},
	{0x034A, 0x09},/*Y_ADDR_END*/
	{0x034B, 0x9F},
	{0x0381, 0x01},/*X_EVEN_INC*/
	{0x0383, 0x01},/*X_ODD_INC*/
	{0x0385, 0x01},/*Y_EVEN_INC*/
	{0x0387, 0x01},/*Y_ODD_INC*/
	{0x0401, 0x00},/*DERATING_EN*/
	{0x0405, 0x10},
	{0x0700, 0x05},/*FIFO_WATER_MARK_PIXELS*/
	{0x0701, 0x30},
	{0x034C, 0x0C},/*X_OUTPUT_SIZE*/
	{0x034D, 0xD0},
	{0x034E, 0x09},/*Y_OUTPUT_SIZE*/
	{0x034F, 0xA0},
	/*Manufacture Setting*/
	{0x300E, 0xE9},
	{0x301D, 0x00},
	{0x301A, 0x77},

/*zhanglijun  add begin*/
	   /* PLL */
    {0x0305, 0x08},/*PRE_PLL_CLK_DIV*/
    {0x0306, 0x00},/*PLL_MULTIPLIER*/
    {0x0307, 0x6C},/*PLL_MULTIPLIER*/
    {0x0303, 0x01},/*VT_SYS_CLK_DIV*/
    {0x0301, 0x05},/*VT_PIX_CLK_DIV*/
    {0x030B, 0x01},/*OP_SYS_CLK_DIV*/
    {0x0309, 0x05},/*OP_PIX_CLK_DIV*/
    {0x30CC, 0xB0},/*DPHY_BAND_CTRL*/
    {0x31A1, 0x56},/*BINNING*/
/*zhanglijun  add end*/
};


#ifdef VIDEO_1080P_RECORD
static struct msm_camera_i2c_reg_conf s5k3h2_1080p_settings[] = {
//{0x0100 ,0x00},										
									

			//Readout							
			//Address	Data	Comment					
{0x0344 ,0x00},		//X addr start 140d				
{0x0345 ,0x8C},				
{0x0346 ,0x01},		//Y addr start 396d                                                                                                              
{0x0347 ,0x8C},		 					
{0x0348 ,0x0C},		//X addr end 3139d					
{0x0349 ,0x43},							
{0x034A ,0x08},		//Y addr end 2075d                                                                                                             
{0x034B ,0x1B},		 					
						
{0x0381 ,0x01},		//x_even_inc = 1					
{0x0383 ,0x01},		//x_odd_inc = 1					
{0x0385 ,0x01},		//y_even_inc = 1					
{0x0387 ,0x01},		//y_odd_inc = 1					
						
{0x0401 ,0x00},		//Derating_en  = 0 (disable)					
{0x0405 ,0x10},							
{0x0700 ,0x05},		//fifo_water_mark_pixels = 1328					
{0x0701 ,0x30},							
						
{0x034C ,0x0B},		//x_output_size = 3000				
{0x034D ,0xB8},							
{0x034E ,0x06},		//y_output_size = 1680                                                                                                       
{0x034F ,0x90},		    					
						
{0x0200 ,0x02},		//fine integration time	Fixed value				
{0x0201 ,0x50},							
{0x0202 ,0x06},		//Coarse integration time	  Frame_length_lines - 8d  //0x06D0				
{0x0203 ,0xD0},							
{0x0204 ,0x00},		//Analog gain					
{0x0205 ,0x20},							
{0x0342 ,0x0D},		//Line_length_pck 3470d					
{0x0343 ,0x8E},							
{0x0340 ,0x06},		//Frame_length_lines 1752d					
{0x0341 ,0xD8},						
							
			//Manufacture Setting							
			//Address	Data	Comment					
{0x300E ,0x29},				
{0x31A3 ,0x00},					
{0x301A ,0xA7},	
{0x3053 ,0xCB},		//CF for full/preview/ ,CB for HD/FHD/QVGA120fps		
//{0x0100 ,0x01},		

			//PLL³]©w		EXCLK 24Mhz, vt_pix_clk_freq_mhz=182.4Mhz,op_sys_clk_freq_mhz=912Mhz					
			//Address	Data	Comment			
{0x0305 ,0x08}, //pre_pll_clk_div = 4 
{0x0306 ,0x00}, //pll_multiplier
{0x0307 ,0x98}, //pll_multiplier = 152
{0x0303 ,0x01}, //vt_sys_clk_div = 1
{0x0301 ,0x05}, //vt_pix_clk_div = 5
{0x030B ,0x01}, //op_sys_clk_div = 1
{0x0309 ,0x05}, //op_pix_clk_div = 5
{0x30CC ,0xe0}, //DPHY_band_ctrl 870 MHz ~ 950 MHz
{0x31A1 ,0x58},
};


#endif
static struct msm_camera_i2c_reg_conf s5k3h2_recommend_settings[] = {			
			//Manufacture Setting		                         
			//Address	Data	Comment                                  
{0x3000 ,0x08},		
{0x3001 ,0x05},		
{0x3002 ,0x0D},		
{0x3003 ,0x21},		
{0x3004 ,0x62},		
{0x3005 ,0x0B},		
{0x3006 ,0x6D},		
{0x3007 ,0x02},		
{0x3008 ,0x62},		
{0x3009 ,0x62},		
{0x300A ,0x41},		
{0x300B ,0x10},		
{0x300C ,0x21},		
{0x300D ,0x04},		
{0x307E ,0x03},		
{0x307F ,0xA5},		
{0x3080 ,0x04},		
{0x3081 ,0x29},		
{0x3082 ,0x03},		
{0x3083 ,0x21},		
{0x3011 ,0x5F},		
{0x3156 ,0xE2},		
{0x3027 ,0xBE},		//DBR_CLK enable for EMI	
{0x300f ,0x02},						
{0x3010 ,0x10},						
{0x3017 ,0x74},						
{0x3018 ,0x00},						
{0x3020 ,0x02},						
{0x3021 ,0x00},		//EMI 		
{0x3023 ,0x80},						
{0x3024 ,0x08},						
{0x3025 ,0x08},						
{0x301C ,0xD4},						
{0x315D ,0x00},						 			
{0x3054 ,0x00},						
{0x3055 ,0x35},						
{0x3062 ,0x04},						
{0x3063 ,0x38},						
{0x31A4 ,0x04},						
{0x3016 ,0x54},						
{0x3157 ,0x02},						
{0x3158 ,0x00},						
{0x315B ,0x02},						
{0x315C ,0x00},						
{0x301B ,0x05},						
{0x3028 ,0x41},						
{0x302A ,0x10},		//20100503 00 	
{0x3060 ,0x00},						
{0x302D ,0x19}, 						
{0x302B ,0x05},						
{0x3072 ,0x13},						
{0x3073 ,0x21},						
{0x3074 ,0x82},						
{0x3075 ,0x20},						
{0x3076 ,0xA2},						
{0x3077 ,0x02},						
{0x3078 ,0x91},						
{0x3079 ,0x91},						
{0x307A ,0x61},						
{0x307B ,0x28},						
{0x307C ,0x31},						
  			
			//black level =64 @ 10bit 
{0x304E ,0x40},		//Pedestal
{0x304F ,0x01},		//Pedestal
{0x3050 ,0x00},		//Pedestal
{0x3088 ,0x01},		//Pedestal
{0x3089 ,0x00},		//Pedestal
{0x3210 ,0x01},		//Pedestal
{0x3211 ,0x00},		//Pedestal
{0x308E ,0x01},		//110512 update		 
{0x308F ,0x8F},		
{0x3064 ,0x03},		//110323 update 
{0x31A7 ,0x0F},		//110323 update

			//Flip/Mirror Setting		                 
			//Address	Data	Comment      
{0x0101 ,0x00},		//Flip/Mirror ON 0x03      OFF 0x00
						
			//MIPI Setting		                         
			//Address	Data	Comment                          
{0x3065 ,0x35},				
{0x310E ,0x00},				
{0x3098 ,0xAB},				
{0x30C7 ,0x0A},				
{0x309A ,0x01},				
{0x310D ,0xC6},				
{0x30c3 ,0x40},				
{0x30BB ,0x02},		
{0x30BC ,0x70},		//According to MCLK, these registers should be changed.
{0x30BD ,0x80},			
{0x3110 ,0xE0},			
{0x3111 ,0x00},			
{0x3112 ,0xF7},			
{0x3113 ,0x80},			
{0x30C7 ,0x2A},			
						


};
static struct v4l2_subdev_info s5k3h2_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array s5k3h2_init_conf[] = {
	{&s5k3h2_recommend_settings[0],
	ARRAY_SIZE(s5k3h2_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_i2c_conf_array s5k3h2_confs[] = {
	{&s5k3h2_snap_settings [0],
	ARRAY_SIZE(s5k3h2_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
#if 1
	{&s5k3h2_prev_settings[0],
	ARRAY_SIZE(s5k3h2_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
#ifdef   VIDEO_1080P_RECORD 
            {&s5k3h2_1080p_settings[0],
            ARRAY_SIZE(s5k3h2_1080p_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
            ///////////////////////////////////////////////////////////////
#endif
    
	#else
#ifdef   VIDEO_1080P_RECORD 
                {&s5k3h2_1080p_settings[0],
                ARRAY_SIZE(s5k3h2_1080p_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
#endif
    {&s5k3h2_prev_settings[0],
    ARRAY_SIZE(s5k3h2_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},

#endif
};

static struct msm_sensor_output_info_t s5k3h2_dimensions[] = {
	/* snapshot */
	{
		.x_output = 0x0CD0, /* 3280 */
		.y_output = 0x09a0, /* 2464 */
		.line_length_pclk = 0x0d8e, /* 3470 */
		.frame_length_lines = 0x09c0, /* 2496*/
		.vt_pixel_clk = 129600000,
		.op_pixel_clk = 129600000,
		.binning_factor = 1,
	},
	
	/* preview */
	{
		.x_output = 0x0668, /* 1640 */
		.y_output = 0x04d0, /* 1232 */
		.line_length_pclk = 0x0d8e, /* 3470*/
		.frame_length_lines = 0x04e0, /* 1248 */
		.vt_pixel_clk = 129600000,
		.op_pixel_clk = 129600000,
		.binning_factor = 1,
	},
#ifdef  VIDEO_1080P_RECORD 
        /* 1080p */
        {
		.x_output = 0x0BB8, /* 3000 */
		.y_output = 0x0690, /* 1680 */
		.line_length_pclk = 0x0d8e, /* 3470 */
		.frame_length_lines = 0x06D8, /* 1752*/
            .vt_pixel_clk = 182400000,
            .op_pixel_clk = 182400000,
		.binning_factor = 1,
        },
#endif

        
};
#if 0
static struct msm_camera_csid_vc_cfg s5k3h2_cid_cfg[] = {
	{0, CSI_RAW10, CSI_DECODE_10BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params s5k3h2_csi_params = {
	.csid_params = {
		.lane_assign = 0xe4,
		.lane_cnt = 2,
		.lut_params = {
			.num_cid = ARRAY_SIZE(s5k3h2_cid_cfg),
			.vc_cfg = s5k3h2_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 2,
		.settle_cnt = 0x1B,
	},
};

static struct msm_camera_csi2_params *s5k3h2_csi_params_array[] = {
#ifdef  VIDEO_1080P_RECORD 
	&s5k3h2_csi_params,
	#endif
	&s5k3h2_csi_params,
	&s5k3h2_csi_params,
};
#endif
static struct msm_camera_csi_params s5k3h2_liteon_csic_params = {
	.data_format = CSI_10BIT,
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x1B,
};

static struct msm_camera_csi_params *s5k3h2_liteon_csic_params_array[] = {
	&s5k3h2_liteon_csic_params,
	&s5k3h2_liteon_csic_params,
	&s5k3h2_liteon_csic_params,
	&s5k3h2_liteon_csic_params,
	&s5k3h2_liteon_csic_params,
};
static struct msm_sensor_output_reg_addr_t s5k3h2_reg_addr = {
	.x_output = 0x34C,
	.y_output = 0x34E,
	.line_length_pclk = 0x342,
	.frame_length_lines = 0x340,
};

static struct msm_sensor_id_info_t s5k3h2_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x382B,
};

static struct msm_sensor_exp_gain_info_t s5k3h2_exp_gain_info = {
	.coarse_int_time_addr = 0x202,
	.global_gain_addr = 0x204,
	.vert_offset = 8,
};
#if 0
int32_t s5k3h2_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
    int rc=0;
	int reset_gpio;
	int vcm_pwd_gpio;

	msm_sensor_expand_power_up(s_ctrl);
	
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

	rc = gpio_request(reset_gpio,"s5k3h2y_8960");
	if (rc) 
	{
		gpio_free(reset_gpio);
		rc = gpio_request(reset_gpio,"s5k3h2y_8960");
		if(rc)
		{
		    printk("%s gpio_request(%d) again fail \n",__func__,reset_gpio);
			return rc;
		}
		printk("%s gpio_request(%d) again success\n",__func__,reset_gpio);
	}

	rc = gpio_request(vcm_pwd_gpio, "s5k3h2y_8960");
	if (rc) 
	{
		gpio_free(vcm_pwd_gpio);
		rc = gpio_request(vcm_pwd_gpio, "s5k3h2y_8960");
		if(rc)
		{
			printk("%s gpio_request(%d) again fail \n",__func__,vcm_pwd_gpio);
		}
		printk("%s gpio_request(%d) again success\n",__func__,vcm_pwd_gpio);
	}
	gpio_direction_output(reset_gpio, 1);
	gpio_direction_output(vcm_pwd_gpio, 1);
    msleep(10);        
	gpio_direction_output(reset_gpio, 0);
	gpio_direction_output(vcm_pwd_gpio, 0);
    msleep(10);
	gpio_direction_output(reset_gpio, 1);
	gpio_direction_output(vcm_pwd_gpio, 1);
    msleep(10);

    return 0;
}

int32_t s5k3h2_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
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
#if 0 /* not use temply */
static struct sensor_calib_data s5k3h2_calib_data;
#endif

static const struct i2c_device_id s5k3h2_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&s5k3h2_s_ctrl},
	{ }
};

static struct i2c_driver s5k3h2_i2c_driver = {
	.id_table = s5k3h2_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};
static struct msm_camera_i2c_client s5k3h2_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .addr_pos = 0,
    .addr_dir = 0,
};

#if 0 /* not use temply */
static struct msm_camera_i2c_client s5k3h2_eeprom_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
};

static struct msm_camera_eeprom_read_t s5k3h2_eeprom_read_tbl[] = {
	{0x10, &s5k3h2_calib_data.r_over_g, 2, 1},
	{0x12, &s5k3h2_calib_data.b_over_g, 2, 1},
	{0x14, &s5k3h2_calib_data.gr_over_gb, 2, 1},
};

static struct msm_camera_eeprom_data_t s5k3h2_eeprom_data_tbl[] = {
	{&s5k3h2_calib_data, sizeof(struct sensor_calib_data)},
};

static struct msm_camera_eeprom_client s5k3h2_eeprom_client = {
	.i2c_client = &s5k3h2_eeprom_i2c_client,
	.i2c_addr = 0xA4,

	.func_tbl = {
		.eeprom_set_dev_addr = NULL,
		.eeprom_init = msm_camera_eeprom_init,
		.eeprom_release = msm_camera_eeprom_release,
		.eeprom_get_data = msm_camera_eeprom_get_data,
	},

	.read_tbl = s5k3h2_eeprom_read_tbl,
	.read_tbl_size = ARRAY_SIZE(s5k3h2_eeprom_read_tbl),
	.data_tbl = s5k3h2_eeprom_data_tbl,
	.data_tbl_size = ARRAY_SIZE(s5k3h2_eeprom_data_tbl),
};
#endif

static int __init s5k3h2y_liteon_init_module(void)
{
	return i2c_add_driver(&s5k3h2_i2c_driver);
}

static struct v4l2_subdev_core_ops s5k3h2_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops s5k3h2_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops s5k3h2_subdev_ops = {
	.core = &s5k3h2_subdev_core_ops,
	.video  = &s5k3h2_subdev_video_ops,
};

static struct msm_sensor_fn_t s5k3h2_func_tbl = {
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
//	.sensor_power_up = s5k3h2_sensor_power_up,
//	.sensor_power_down = s5k3h2_sensor_power_down,
    .sensor_power_up = msm_sensor_power_up,
    .sensor_power_down = msm_sensor_power_down,
	.sensor_csi_setting = msm_sensor_setting1,
};

static struct msm_sensor_reg_t s5k3h2_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = s5k3h2_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(s5k3h2_start_settings),
	.stop_stream_conf = s5k3h2_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(s5k3h2_stop_settings),
	.group_hold_on_conf = s5k3h2_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(s5k3h2_groupon_settings),
	.group_hold_off_conf = s5k3h2_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(s5k3h2_groupoff_settings),
	.init_settings = &s5k3h2_init_conf[0],
	.init_size = ARRAY_SIZE(s5k3h2_init_conf),
	.mode_settings = &s5k3h2_confs[0],
	.output_settings = &s5k3h2_dimensions[0],
	.num_conf = ARRAY_SIZE(s5k3h2_confs),
};

static struct msm_sensor_ctrl_t s5k3h2_s_ctrl = {
	.msm_sensor_reg = &s5k3h2_regs,
	.sensor_i2c_client = &s5k3h2_sensor_i2c_client,
	.sensor_i2c_addr = 0x6f,
	#if 0 /* not use temply */
	.sensor_eeprom_client = &s5k3h2_eeprom_client,
	#endif
	.sensor_output_reg_addr = &s5k3h2_reg_addr,
	.sensor_id_info = &s5k3h2_id_info,
	.sensor_exp_gain_info = &s5k3h2_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &s5k3h2_liteon_csic_params_array[0],
	.msm_sensor_mutex = &s5k3h2_mut,
	.sensor_i2c_driver = &s5k3h2_i2c_driver,
	.sensor_v4l2_subdev_info = s5k3h2_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(s5k3h2_subdev_info),
	.sensor_v4l2_subdev_ops = &s5k3h2_subdev_ops,
	.func_tbl = &s5k3h2_func_tbl,
   	.clk_rate = MSM_SENSOR_MCLK_48HZ,
	.sensor_name = "23060093FA-SAM-L",

};

module_init(s5k3h2y_liteon_init_module);
MODULE_DESCRIPTION("SAMSUNG S5K3H2YX Bayer sensor driver");
MODULE_LICENSE("GPL v2");
