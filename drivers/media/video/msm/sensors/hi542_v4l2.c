
#include "msm_sensor.h"
#include "msm.h"
#include <linux/gpio.h>


#define SENSOR_NAME "hi542"
#define PLATFORM_DRIVER_NAME "msm_camera_hi542"
#define hi542_obj hi542_##obj

#define HI542_REG_MODEL_ID	      0x0004 /*Chip ID read register*/
#define HI542_MODEL_ID	      0xB1 /*Hynix HI542 Chip ID*/

DEFINE_MUTEX(hi542_mut);
static struct msm_sensor_ctrl_t hi542_s_ctrl;

static struct msm_camera_i2c_reg_conf hi542_start_settings[] = {
	{0x0001, 0x00},
};

static struct msm_camera_i2c_reg_conf hi542_stop_settings[] = {
	{0x0001, 0x01},
	{0x0617, 0x01},
	{0x0001, 0x00},
	{0x0001, 0x01},
	{0x0617, 0x00},
};

static struct msm_camera_i2c_reg_conf hi542_groupon_settings[] = {
	//{0x104, 0x01},
};

static struct msm_camera_i2c_reg_conf hi542_groupoff_settings[] = {
	//{0x104, 0x00},
};

static struct msm_camera_i2c_reg_conf hi542_prev_settings[] = {
	///////////////////////////////////////////
	// Preview Setting
	///////////////////////////////////////////
	{0x0010, 0x41},
	{0x0011, 0x00}, //jk 0x00},
	{0x0034, 0x03},
	{0x0035, 0xD4},
	
	//{0x0042,0x00},
	//{0x0043,0x14},
	
	{0x0500, 0x1b},// 1B}, LSC ON
	
	{0x0630, 0x05},
	{0x0631, 0x18},//08},
	{0x0632, 0x03},
	{0x0633, 0xD4},//C8};

};

static struct msm_camera_i2c_reg_conf hi542_snap_settings[] = {
	{0x0010, 0x40},
	{0x0011, 0x00}, //00}, exposure method change

	{0x0034, 0x07},
	{0x0035, 0xA8},

	{0x0500, 0x13}, //13}, LSC ON

	{0x0630, 0x0A},
	{0x0631, 0x30},
	{0x0632, 0x07},
	{0x0633, 0xA8},
};

static struct msm_camera_i2c_reg_conf hi542_recommend_settings[] = {			
	{0x0011,0x90},  //90},//exposure method change
	{0x0020,0x00},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0021,0x00},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0022,0x00},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0023,0x00},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0038,0x02},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0039,0x2c},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x003C,0x00},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x003D,0x0C},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x003E,0x00},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x003F,0x0C},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0040,0x00},  //Hblank H
	{0x0041,0x35},  ////2e} Hblank L
	{0x0042,0x00},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0043,0x14},  /*jk 0x20},*/  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0045,0x07},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0046,0x01},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0047,0xD0},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0050,0x00},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0052,0x10},   /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0053,0x10},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0054,0x10},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0055,0x08},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0056,0x80},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0057,0x08},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0058,0x08},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0059,0x08},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x005A,0x08},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x005B,0x02},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0070,0x00},  //03},EMI OFF
	{0x0081,0x01},  //09},//0B},BLC scheme
	{0x0082,0x23},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0083,0x00},  /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0085,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0086,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x008c,0x02},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x00A0,0x0f},//0C},//0B},RAMP DC OFFSET
	{0x00A1,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00A2,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00A3,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00A4,0xFF},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00A5,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00A6,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00A8,0x7F},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00A9,0x7F},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00AA,0x7F},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x00B4,0x00},//08},BLC offset
	{0x00B5,0x00},//08},
	{0x00B6,0x02},//07},
	{0x00B7,0x01},//07},
	
	{0x00D4,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00D5,0xaa},//a9},RAMP T1
	{0x00D6,0x01},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00D7,0xc9},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00D8,0x05},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00D9,0x59},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00DA,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00DB,0xb0},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00DC,0x01},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x00DD,0xc9},//c5},/*rp_rst_flg_on1*/
	
	{0x011C,0x1F},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x011D,0xFF},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x011E,0xFF},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x011F,0xFF},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x012A,0xFF},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x012B,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0129,0x40},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x0210,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0212,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0213,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0216,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0219,0x33},//   66},Pixel bias
	{0x021B,0x55},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x021C,0x85},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x021D,0xFF},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x021E,0x01},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x021F,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x0220,0x02},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0221,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0222,0xA0},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0227,0x0A},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0228,0x5C},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0229,0x2d},//41},//00},//2C},RAMP swing range jwryu120120
	{0x022A,0x04},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x022C,0x01},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x022D,0x23},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x0237,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0238,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0239,0xA5},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x023A,0x20},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x023B,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x023C,0x22},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x023F,0x80},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x0240,0x04},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0241,0x07},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0242,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0243,0x01},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0244,0x80},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0245,0xE0},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0246,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0247,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x024A,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x024B,0x14},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x024D,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x024E,0x03},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x024F,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x0250,0x53},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0251,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0252,0x07},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0253,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0254,0x4F},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0255,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0256,0x07},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0257,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0258,0x4F},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0259,0x0C},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x025A,0x0C},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x025B,0x0C},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x026C,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x026D,0x09},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x026E,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x026F,0x4B},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x0270,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0271,0x09},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0272,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0273,0x4B},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0274,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0275,0x09},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0276,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0277,0x4B},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0278,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0279,0x01},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x027A,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x027B,0x55},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x027C,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x027D,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x027E,0x05},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x027F,0x5E},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x0280,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0281,0x03},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0282,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0283,0x45},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0284,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0285,0x03},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0286,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0287,0x45},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0288,0x05},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0289,0x5C},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x028A,0x05},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x028B,0x60},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x02A0,0x01},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02A1,0xe0},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02A2,0x02},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02A3,0x22},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02A4,0x05},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02A5,0x5C},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02A6,0x05},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02A7,0x60},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02A8,0x05},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02A9,0x5C},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02AA,0x05},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02AB,0x60},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x02D2,0x0F},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02DB,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02DC,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02DD,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02DE,0x0C},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02DF,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x02E0,0x04},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02E1,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02E2,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02E3,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02E4,0x0F},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x02F0,0x05},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x02F1,0x05},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x0310,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0311,0x01},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0312,0x05},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0313,0x5A},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0314,0x00},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0315,0x01},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0316,0x05},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	{0x0317,0x5A},/*man_spec_edof_crtl_edof_fw_spare_0 Gain x7*/
	
	{0x0318,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0319,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x031A,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x031B,0x2F},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x031C,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/	
	{0x031D,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x031E,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x031F,0x2F},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0320,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0321,0xAB},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0322,0x02},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0323,0x55},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0324,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0325,0xAB},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0326,0x02},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0327,0x55},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0328,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0329,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x032A,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x032B,0x10},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x032C,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x032D,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x032E,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x032F,0x10},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	
	{0x0330,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0331,0x02},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0332,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0333,0x2E},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0334,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0335,0x02},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0336,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0337,0x2E},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	{0x0358,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0359,0x46},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x035A,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x035B,0x59},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x035C,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x035D,0x46},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x035E,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x035F,0x59},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	////10
	
	{0x0360,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0361,0x46},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0362,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0363,0xa4},//a2},Black sun
	{0x0364,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0365,0x46},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0366,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0367,0xa4},//a2},Black sun
	{0x0368,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0369,0x46},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x036A,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x036B,0xa6},//a9},S2 off
	{0x036C,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x036D,0x46},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x036E,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x036F,0xa6},//a9},S2 off
	
	
	{0x0370,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0371,0xb0},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0372,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0373,0x59},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0374,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0375,0xb0},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0376,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0377,0x59},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0378,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0379,0x45},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x037A,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x037B,0xaa},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x037C,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x037D,0x99},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x037E,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x037F,0xAE},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	{0x0380,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0381,0xB1},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0382,0x02},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0383,0x56},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0384,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0385,0x6D},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0386,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0387,0xDC},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	
	//PAGE11
	
	{0x03A0,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03A1,0x5E},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03A2,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03A3,0x62},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03A4,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03A5,0xC9},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03A6,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03A7,0x27},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03A8,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03A9,0x59},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03AA,0x02},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03AB,0x55},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03AC,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03AD,0xC5},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03AE,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03AF,0x27},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	{0x03B0,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03B1,0x55},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03B2,0x02},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03B3,0x55},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03B4,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03B5,0x0A},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	{0x03D3,0x08},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03D5,0x44},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03D6,0x51},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03D7,0x56},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03D8,0x44},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03D9,0x06},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	{0x0580,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0581,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0582,0x80},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0583,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0584,0x80},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0585,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0586,0x80},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0587,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0588,0x80},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0589,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x058A,0x80},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	{0x05C2,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x05C3,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	
	{0x0080,0xC7},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0119,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x011A,0x15},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x011B,0xC0},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0115,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0116,0x2A},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0117,0x4C},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0118,0x20},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0223,0xED},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0224,0xE4},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0225,0x09},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0226,0x36},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	{0x023E,0x80},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x05B0,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03D2,0xAD},//PLL reset 20120418 ryu add
	{0x0616,0x00},//D-PHY reset 20120418 ryu add
	{0x0616,0x01},//D-PHY reset disable 20120418 ryu add
	{0x03D2,0xAC},//PLL reset disable 20120418 ryu add
	
	{0x03D0,0xE9},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x03D1,0x75},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	{0x0800,0x07},//0F},EMI disable
	{0x0801,0x08},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0802,0x02},//00},apb clock speed down
	
	{0x0012,0x04},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0013,0x40},//00},exposure method change
	
	{0x0024,0x07},/*windowing*/
	{0x0025,0xA8},/*jk 0x90},//A8},*//*windowing*/
	{0x0026,0x0A},/*windowing*/
	{0x0027,0x30},/*jk 0x10},//30},*//*windowing*/
	
	{0x0030,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0031,0x03},//jk 0xFF},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0032,0x07},//jk 0x06},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0033,0xAC},//jk 0xB0},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0034,0x03},//jk 0x02},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0035,0xD4},//jk 0xD8},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	{0x003A,0x00}, /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x003B,0x2E},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x004A,0x03},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x004B,0xD4},//JK 0xC8/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x004C,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x004D,0x18},//08
	{0x0C98,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0C99,0x5E},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0C9A,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0C9B,0x62},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x05A0,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	 
	{0x0084,0x30},//10},  blc CONTROL
	{0x008D,0xFF},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0090,0x02},//0B },BLC defect pixel th
	{0x00A7,0x80},//ff }, /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x021A,0x15},//05},  CDS bias
	{0x022B,0xB0},//f0}, RMAP filter
	{0x0232,0x37},//17}, black sun enable
	{0x0010,0x41},//jk 0x01 /*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	
	{0x0740,0x1A},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0742,0x1A},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0743,0x1A},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0744,0x1A},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0745,0x04},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0746,0x32},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0747,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0748,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0749,0x90},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x074A,0x1A},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x074B,0xB1},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/

	
	{0x0500,0x1b},//1b LSC enable
	{0x0510,0x10},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	
	
	{0x0217,0x44},//adaptive NCP on
	{0x0218,0x00},//scn_sel
	
	{0x02ac,0x00},//outdoor on
	{0x02ad,0x00},
	{0x02ae,0x00},//outdoor off
	{0x02af,0x00},
	{0x02b0,0x00},//indoor on
	{0x02b1,0x00},
	{0x02b2,0x00},//indoor off
	{0x02b3,0x00},
	{0x02b4,0x60},//dark1 on
	{0x02b5,0x21},
	{0x02b6,0x66},//dark1 off
	{0x02b7,0x8a},
	
	{0x02c0,0x36},//outdoor NCP en
	{0x02c1,0x36},//indoor NCP en
	{0x02c2,0x36},//dark1 NCP en
	{0x02c3,0x36},//3f  dark2 NCP disable
	{0x02c4,0xE4},//outdoor NCP voltage
	{0x02c5,0xE4},//indoor  NCP voltage
	{0x02c6,0xE4},//dark1  NCP voltage
	{0x02c7,0xdb},//24 dark2	NCP voltage
	
	{0x061A,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x061B,0x03},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x061C,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x061D,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x061E,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x061F,0x03},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0613,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0615,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0616,0x01},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0617,0x00},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0619,0x01},//continue clk mode  20120418 ryu add
	
	{0x0008,0x0F},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0630,0x05},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0631,0x18},// jk 08/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0632,0x03},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0633,0xd4},//jk c8},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	//LSC
	{0x0540, 0x10}, /* man_spec_edof_ctrl_edof_fw_spare_0 Gain x7 */
	{0x0541, 0x1f}, /* man_spec_edof_ctrl_edof_fw_spare_0 Gain x7 */
	{0x0550, 0x80}, /* man_spec_edof_ctrl_edof_fw_spare_0 Gain x7 */
	{0x0551, 0x80}, /* man_spec_edof_ctrl_edof_fw_spare_0 Gain x7 */
	{0x0552, 0x80}, /* man_spec_edof_ctrl_edof_fw_spare_0 Gain x7 */
	{0x0553, 0x80}, /* man_spec_edof_ctrl_edof_fw_spare_0 Gain x7 */
	{0x0554, 0x52}, /* man_spec_edof_ctrl_edof_fw_spare_0 Gain x7 */
	{0x0555, 0x52}, /* man_spec_edof_ctrl_edof_fw_spare_0 Gain x7 */
	{0x0556, 0x52}, /* man_spec_edof_ctrl_edof_fw_spare_0 Gain x7 */
	{0x0571, 0x60}, //60}, /* man_spec_edof_ctrl_edof_fw_spare_0 Gain x7 */
	{0x0663,0x05},//0a}, trail time/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/
	{0x0660,0x03},/*man_spec_edof_ctrl_edof_fw_spare_0 Gain x7*/

};

static struct v4l2_subdev_info hi542_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
};

static struct msm_camera_i2c_conf_array hi542_init_conf[] = {
	{&hi542_recommend_settings[0],
	ARRAY_SIZE(hi542_recommend_settings), 
	0, 
	MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_i2c_conf_array hi542_confs[] = {
	{&hi542_snap_settings [0],
	ARRAY_SIZE(hi542_snap_settings), 
	0, 
	MSM_CAMERA_I2C_BYTE_DATA},

	{&hi542_prev_settings[0],
	ARRAY_SIZE(hi542_prev_settings), 
	0, 
	MSM_CAMERA_I2C_BYTE_DATA},

};

static struct msm_sensor_output_info_t hi542_dimensions[] = {
	/* snapshot */
	{
		.x_output = 2608,
		.y_output = 1960,
		.line_length_pclk = 2791,
		.frame_length_lines = 1995,
		.vt_pixel_clk = 84000000,//210000000,//192400000
		.op_pixel_clk = 84000000,//182300000
		.binning_factor = 0,
	},

	
	/* preview */
	{
		.x_output = 1304,
		.y_output = 980,
		.line_length_pclk = 2791,
		.frame_length_lines = 1003,
		.vt_pixel_clk = 84000000,//210000000,//216000000
		.op_pixel_clk = 84000000,//108000000
		.binning_factor = 0,
	},

};


static struct msm_camera_csid_vc_cfg hi542_cid_cfg[] = {
	{0, CSI_RAW10,CSI_DECODE_10BIT},
	{1,CSI_EMBED_DATA,CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params hi542_csi_params = {
	.csid_params = {
		.lane_cnt = 2,
		.lut_params = {
			.num_cid = ARRAY_SIZE(hi542_cid_cfg),
			.vc_cfg = hi542_cid_cfg,
		},
	},
	.csiphy_params = { 
		.lane_cnt = 2, 
		.settle_cnt = 0x21,
	},
};


static struct msm_camera_csi2_params *hi542_csi_params_array[]={

	&hi542_csi_params,
	&hi542_csi_params,
};

static struct msm_camera_csi_params hi542_csic_params = {			
	.data_format = CSI_10BIT,		
	.lane_cnt = 2,			
	.lane_assign = 0xe4,		
	.dpcm_scheme = 0,		
	.settle_cnt = 0x21, 	
	};			


static struct msm_camera_csi_params *hi542_csic_params_array[] = {					  
	&hi542_csic_params,				  
	&hi542_csic_params,	
	};	  



static struct msm_sensor_output_reg_addr_t hi542_reg_addr = {
	.x_output = 0x0630 ,
	.y_output = 0x0632,
	.line_length_pclk = 0x0641, //
	.frame_length_lines = 0x0643,
};

static struct msm_sensor_id_info_t hi542_id_info = {
	.sensor_id_reg_addr = HI542_REG_MODEL_ID ,
	.sensor_id = HI542_MODEL_ID,
};

static struct msm_sensor_exp_gain_info_t hi542_exp_gain_info = {
	.coarse_int_time_addr = 0x0115, //reg_coarse_integration_time_hb 
	.global_gain_addr =0x0129, //Again address 
	.vert_offset = 0,
};


static const struct i2c_device_id hi542_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&hi542_s_ctrl},
	{ }
};

static struct i2c_driver hi542_i2c_driver = {
	.id_table = hi542_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};
static struct msm_camera_i2c_client hi542_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .addr_pos = 0,
    .addr_dir = 0,
};
int32_t hi542_sensor_write_exp_gain1(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{ 
#if 1
	uint16_t max_legal_gain = 0x00;//0x0200;
	int32_t rc = 0;
	uint32_t min_line = 4;
	uint32_t pixels_line = 0;
	uint32_t fixed_line = 0; 
	uint8_t i = 0, mask = 0xFF;
	uint8_t values_1[] = { 0, 0, 0, 0, 0 }; 
	uint8_t values_2[] = { 0, 0, 0, 0, 0 }; 
		
		/*HI542's Max gain x8 = 0x00h, if gain < 0x00h then gain = 0x00h*/
	if (gain < max_legal_gain){
		CDBG("Max legal gain line:%d\n",__LINE__);
		gain = max_legal_gain;
	} 
		/* HI542's min line = 4*/ 
	if (line < min_line){
		CDBG("Max legal gain line:%d\n",__LINE__);
		line= min_line;
	} 
		
		pixels_line = line * 2791; //s_ctrl->curr_line_length_pclk ; //2791;
		fixed_line = pixels_line + 5582; //(s_ctrl->curr_line_length_pclk * 2);//5582 = 2791 * 2
	
		
	for ( i = 1 ; i < 5; i++ ) {
		values_1[i] = ( mask & pixels_line ); 
		values_2[i] = ( mask & fixed_line); 
	  	pixels_line >>= 8;
		fixed_line >>= 8;	}
		
		values_1[0] = gain;
	
	pr_info("hi542_write_prev_exp_gain :%d%d\n",gain,line);
	//offset=s_ctrl->hi542_exp_gain_info->vert_offset;

	/*HI542 fixed time update*/
	
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x0120, //(s_ctrl->sensor_exp_gain_info->coarse_int_time_addr+11),
		values_2[4],
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x0121, //(s_ctrl->sensor_exp_gain_info->coarse_int_time_addr+12),
		values_2[3],
		MSM_CAMERA_I2C_BYTE_DATA);
	
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x0122, //(s_ctrl->sensor_exp_gain_info->coarse_int_time_addr+13),
		values_2[2],
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x0123, //(s_ctrl->sensor_exp_gain_info->coarse_int_time_addr+14),
		values_2[1],
		MSM_CAMERA_I2C_BYTE_DATA);
	
	
	/*HI542 max time update*/
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x011c, //(s_ctrl->sensor_exp_gain_info->coarse_int_time_addr+7),
		values_1[4],
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x011d, //(s_ctrl->sensor_exp_gain_info->coarse_int_time_addr+8),
		values_1[3],
		MSM_CAMERA_I2C_BYTE_DATA);
	
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x011e, //(s_ctrl->sensor_exp_gain_info->coarse_int_time_addr+9),
		values_1[2],
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x011f, //(s_ctrl->sensor_exp_gain_info->coarse_int_time_addr+10),
		values_1[1],
		MSM_CAMERA_I2C_BYTE_DATA);
	
	/*HI542 Analog gain update*/
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
             0x0129, //s_ctrl->sensor_exp_gain_info->global_gain_addr,
             values_1[0],
             MSM_CAMERA_I2C_BYTE_DATA);

      /*HI542 Exposure time update*/
      msm_camera_i2c_write(s_ctrl -> sensor_i2c_client,
             0x0115, //(s_ctrl->sensor_exp_gain_info->coarse_int_time_addr),
             values_1[4],
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
            0x0116, //(s_ctrl->sensor_exp_gain_info->coarse_int_time_addr+1),
            values_1[3],
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
            0x0117, //(s_ctrl->sensor_exp_gain_info->coarse_int_time_addr+2),
            values_1[2],
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
            0x0118, //(s_ctrl->sensor_exp_gain_info->coarse_int_time_addr+3),
            values_1[1],
		MSM_CAMERA_I2C_BYTE_DATA);	
	

#else
	uint16_t max_legal_gain = 0x00;//0x0200;
	int32_t rc = 0;
//	static uint32_t fl_lines, offset;
	uint32_t min_line = 4;
	uint32_t pixels_line = 0;
	uint8_t i = 0, mask = 0xFF;	uint8_t values[] = { 0, 0, 0, 0, 0};


	/* HI542's Max gain x8 = 0x00h, if gain < 0x00h then gain = 0x00h*/
	if (gain < max_legal_gain){
		CDBG("Max legal gain line:%d\n",__LINE__);
		gain = max_legal_gain;
	} 
	/* HI542's min line = 4*/
	if (line < min_line){
		CDBG("Max legal gain line:%d\n",__LINE__);
		line= min_line;
	} 
	
	pixels_line = line*2791;//2791 = above struct hi542_dimensions[] .line_length_pclk

	for ( i = 1 ; i < 5; i++ ) {
	   values[i]  = ( mask & pixels_line );	   pixels_line >>= 8;	}
	values[0] = gain;

	pr_info("hi542_write_prev_exp_gain :%d%d\n",gain,line);
//	offset = s_ctrl->hi542_exp_gain_info->vert_offset;


	/*HI542 Analog gain update*/
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr,
		values[0],
		MSM_CAMERA_I2C_BYTE_DATA);

	/*HI542 Exposure time update*/
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		values[4],
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
		values[3],
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 2,
		values[2],
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 3,
		values[1],
		MSM_CAMERA_I2C_BYTE_DATA);	
	

#endif

	if(is_first_preview_frame)
	{
		msleep(70);
		is_first_preview_frame = 0;
	}

return rc;
}
int32_t hi542_sensor_match_id(struct msm_sensor_ctrl_t * s_ctrl)
{
	int32_t rc = 0;
	uint16_t chipid = 0;
	rc = msm_camera_i2c_read(
			s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_id_info->sensor_id_reg_addr, &chipid,
			MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
		pr_err("%s: %s: read id failed\n", __func__,
			s_ctrl->sensordata->sensor_name);
		printk("Brayden:hi542_sensor_match_id2:sensor_id_reg_addr is %x\n", s_ctrl->sensor_id_info->sensor_id_reg_addr);
		return rc;
	}

	printk("msm_sensor id: %d\n", chipid);
	if (chipid != s_ctrl->sensor_id_info->sensor_id) {
		pr_err("hi542_sensor_match_id2 chip id doesnot match\n");
		return -ENODEV;
	}
	return rc;
}


static int __init hi542_init_module(void)
{
	return i2c_add_driver(&hi542_i2c_driver);
}

static struct v4l2_subdev_core_ops hi542_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops hi542_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops hi542_subdev_ops = {
	.core = &hi542_subdev_core_ops,
	.video  = &hi542_subdev_video_ops,
};

static struct msm_sensor_fn_t hi542_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = hi542_sensor_write_exp_gain1,
	.sensor_write_snapshot_exp_gain = hi542_sensor_write_exp_gain1,
	.sensor_setting = msm_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
//	.sensor_power_up = hi542_sensor_power_up,
//	.sensor_power_down = hi542_sensor_power_down,
    .sensor_power_up = msm_sensor_power_up,
    .sensor_power_down = msm_sensor_power_down,
    .sensor_match_id =hi542_sensor_match_id,
	.sensor_csi_setting = msm_sensor_setting1,
};

static struct msm_sensor_reg_t hi542_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = hi542_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(hi542_start_settings),
	.stop_stream_conf = hi542_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(hi542_stop_settings),
	.group_hold_on_conf = hi542_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(hi542_groupon_settings),
	.group_hold_off_conf = hi542_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(hi542_groupoff_settings),
	.init_settings = &hi542_init_conf[0],
	.init_size = ARRAY_SIZE(hi542_init_conf),
	.mode_settings = &hi542_confs[0],
	.output_settings = &hi542_dimensions[0],
	.num_conf = ARRAY_SIZE(hi542_confs),
};

static struct msm_sensor_ctrl_t hi542_s_ctrl = {
	.msm_sensor_reg = &hi542_regs,
	.sensor_i2c_client = &hi542_sensor_i2c_client,
	.sensor_i2c_addr = 0x40,  
	.sensor_output_reg_addr = &hi542_reg_addr,
	.sensor_id_info = &hi542_id_info,
	.sensor_exp_gain_info = &hi542_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &hi542_csic_params_array[0],
	.csi_params = &hi542_csi_params_array[0],
	.msm_sensor_mutex = &hi542_mut,
	.sensor_i2c_driver = &hi542_i2c_driver,
	.sensor_v4l2_subdev_info = hi542_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(hi542_subdev_info),
	.sensor_v4l2_subdev_ops = &hi542_subdev_ops,
	.func_tbl = &hi542_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
	.sensor_name = "23060084FF-HY-S",

};


module_init(hi542_init_module);
MODULE_DESCRIPTION("Hynix 5MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");

