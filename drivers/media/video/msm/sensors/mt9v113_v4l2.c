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
#define SENSOR_NAME "mt9v113"
#define PLATFORM_DRIVER_NAME "msm_camera_mt9v113"
#define mt9v113_obj mt9v113_##obj
#define MODEL_SUNNY 0
#define MODEL_BYD 1
#include "linux/hardware_self_adapt.h"
DEFINE_MUTEX(mt9v113_mut);
static struct msm_sensor_ctrl_t mt9v113_s_ctrl;
void  mt9v113_set_mirror_mode(struct msm_sensor_ctrl_t *s_ctrl);

static struct msm_camera_i2c_reg_conf mt9v113_start_settings[] = {
	{ 0x301A, 0x121C},
};

static struct msm_camera_i2c_reg_conf mt9v113_stop_settings[] = {
	{0x301A, 0x1218},
};

/*modify the initialization settings to avoid exceptional output*/
static struct msm_camera_i2c_reg_conf mt9v113_recommend_settings_1[] =
{
	{0x001A, 0x0011}, 	
	{0x001A, 0x0018}, 	
	{0x0014, 0x2145}, 	
	{0x0014, 0x2145}, 	
	{0x0010, 0x0631}, 	
	{0x0012, 0x0000}, 	
	{0x0014, 0x244B}, 
};

static struct msm_camera_i2c_reg_conf mt9v113_recommend_settings_8Mmclk[] =
{
	{0x001A, 0x0011}, 	
	{0x001A, 0x0018}, 	
	{0x0014, 0x2145}, 	
	{0x0014, 0x2145}, 	
	{0x0010, 0x0015}, 	
	{0x0012, 0x0000}, 	
	{0x0014, 0x244B}, 
};

static struct msm_camera_i2c_reg_conf mt9v113_recommend_settings_2[] =
{
	{0x0014, 0x304B},
};
static struct msm_camera_i2c_reg_conf mt9v113_recommend_settings_3[] =
{
	{0x0014, 0xB04A}, 	
	{0x0018, 0x402C}, 
};    
static struct msm_camera_i2c_reg_conf mt9v113_recommend_settings_4[] =
{
	{0x3400, 0x7A38}, 	
	{0x321C, 0x0003}, 	

	{0x098C, 0x02F0}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x02F2}, 	
	{0x0990, 0x0210}, 	
	{0x098C, 0x02F4}, 	
	{0x0990, 0x001A}, 	
	{0x098C, 0x2145}, 	
	{0x0990, 0x02F4}, 	
	{0x098C, 0xA134}, 	
	{0x0990, 0x0001}, 	
	{0x31E0, 0x0001}, 	

	{0x098C, 0x2703}, 	
	{0x0990, 0x0280}, 	
	{0x098C, 0x2705}, 	
	{0x0990, 0x01E0}, 	
	{0x098C, 0x2707}, 	
	{0x0990, 0x0280}, 	
	{0x098C, 0x2709}, 	
	{0x0990, 0x01E0}, 	
	{0x098C, 0x270D}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x270F}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x2711}, 	
	{0x0990, 0x01E7}, 	
	{0x098C, 0x2713}, 	
	{0x0990, 0x0287}, 	
	{0x098C, 0x2715}, 	
	{0x0990, 0x0001}, 	
	{0x098C, 0x2717}, 	
	{0x0990, 0x0025}, 	
	{0x098C, 0x2719}, 	
	{0x0990, 0x001A}, 	
	{0x098C, 0x271B}, 	
	{0x0990, 0x006B}, 	
	{0x098C, 0x271D}, 	
	{0x0990, 0x006B}, 	
	{0x098C, 0x271F}, // MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_A]
	{0x0990, 0x032A}, // MCU_DATA_0
	{0x098C, 0x2721}, 	
	{0x0990, 0x0364}, 	
	{0x098C, 0x2723}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x2725}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x2727}, 	
	{0x0990, 0x01E7}, 	
	{0x098C, 0x2729}, 	
	{0x0990, 0x0287}, 	
	{0x098C, 0x272B}, 	
	{0x0990, 0x0001}, 	
	{0x098C, 0x272D}, 	
	{0x0990, 0x0025}, 	
	{0x098C, 0x272F}, 	
	{0x0990, 0x001A}, 	
	{0x098C, 0x2731}, 	
	{0x0990, 0x006B}, 	
	{0x098C, 0x2733}, 	
	{0x0990, 0x006B}, 	
	{0x098C, 0x2735}, 	
	{0x0990, 0x0426}, 	
	{0x098C, 0x2737}, 	
	{0x0990, 0x0363}, 	
	{0x098C, 0x2739}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x273B}, 	
	{0x0990, 0x027F}, 	
	{0x098C, 0x273D}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x273F}, 	
	{0x0990, 0x01DF}, 	
	{0x098C, 0x2747}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x2749}, 	
	{0x0990, 0x027F}, 	
	{0x098C, 0x274B}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x274D}, 	
	{0x0990, 0x01DF}, 	
	{0x098C, 0x222D}, 
	{0x0990, 0x0088}, 
	{0x098C, 0xA408}, 
	{0x0990, 0x0020}, 
	{0x098C, 0xA409}, 
	{0x0990, 0x0023}, 
	{0x098C, 0xA40A}, 
	{0x0990, 0x0027}, 
	{0x098C, 0xA40B}, 
	{0x0990, 0x002A}, 
	{0x098C, 0x2411}, 
	{0x0990, 0x0088}, 
	{0x098C, 0x2413}, 
	{0x0990, 0x00A4}, 
	{0x098C, 0x2415}, 
	{0x0990, 0x0088}, 
	{0x098C, 0x2417}, 
	{0x0990, 0x00A4}, 
	{0x098C, 0xA404}, 
	{0x0990, 0x0010}, 
	{0x098C, 0xA40D}, 
	{0x0990, 0x0002}, 
	{0x098C, 0xA40E}, 
	{0x0990, 0x0003},
	{0x098C, 0xA410}, 	
	{0x0990, 0x000A}, 	

	{0x364E, 0x0350}, 
	{0x3650, 0x22ED}, 
	{0x3652, 0x0513}, 
	{0x3654, 0x6C70}, 
	{0x3656, 0x5015}, 
	{0x3658, 0x0130}, 
	{0x365A, 0x444D}, 
	{0x365C, 0x18D3}, 
	{0x365E, 0x5FB1}, 
	{0x3660, 0x6415}, 
	{0x3662, 0x00D0}, 
	{0x3664, 0x014C}, 
	{0x3666, 0x7BB2}, 
	{0x3668, 0x31B1}, 
	{0x366A, 0x46D5}, 
	{0x366C, 0x0130}, 
	{0x366E, 0x338D}, 
	{0x3670, 0x0593}, 
	{0x3672, 0x13D1}, 
	{0x3674, 0x4875}, 
	{0x3676, 0x992E}, 
	{0x3678, 0x910E}, 
	{0x367A, 0xAF92}, 
	{0x367C, 0x1732}, 
	{0x367E, 0x7BD3}, 
	{0x3680, 0x98EE}, 
	{0x3682, 0xEF4D}, 
	{0x3684, 0x8872}, 
	{0x3686, 0x0352}, 
	{0x3688, 0x2792}, 
	{0x368A, 0xDB6D}, 
	{0x368C, 0xF52D}, 
	{0x368E, 0xA532}, 
	{0x3690, 0x0213}, 
	{0x3692, 0x10D5}, 
	{0x3694, 0x8BCE}, 
	{0x3696, 0xFC2D}, 
	{0x3698, 0xA532}, 
	{0x369A, 0x67F1}, 
	{0x369C, 0x1034}, 
	{0x369E, 0x1113}, 
	{0x36A0, 0x2EF3}, 
	{0x36A2, 0x39F7}, 
	{0x36A4, 0xB097}, 
	{0x36A6, 0x81BA}, 
	{0x36A8, 0x2CF3}, 
	{0x36AA, 0x1373}, 
	{0x36AC, 0x4457}, 
	{0x36AE, 0xFAF6}, 
	{0x36B0, 0xEC19}, 
	{0x36B2, 0x0E73}, 
	{0x36B4, 0x0873}, 
	{0x36B6, 0x34F7}, 
	{0x36B8, 0x9EB7}, 
	{0x36BA, 0x9B9A}, 
	{0x36BC, 0x0E33}, 
	{0x36BE, 0x2013}, 
	{0x36C0, 0x3C37}, 
	{0x36C2, 0xA0D7}, 
	{0x36C4, 0x935A}, 
	{0x36C6, 0xCD11}, 
	{0x36C8, 0x0353}, 
	{0x36CA, 0x2516}, 
	{0x36CC, 0x8437}, 
	{0x36CE, 0xA01A}, 
	{0x36D0, 0xFCF1}, 
	{0x36D2, 0xAD91}, 
	{0x36D4, 0x29D4}, 
	{0x36D6, 0x0AB6}, 
	{0x36D8, 0x9436}, 
	{0x36DA, 0xA872}, 
	{0x36DC, 0xD00F}, 
	{0x36DE, 0x2B36}, 
	{0x36E0, 0x0714}, 
	{0x36E2, 0xFEB9}, 
	{0x36E4, 0xD211}, 
	{0x36E6, 0x22F1}, 
	{0x36E8, 0x5BD5}, 
	{0x36EA, 0x98D3}, 
	{0x36EC, 0xF4D9}, 
	{0x36EE, 0x78B5}, 
	{0x36F0, 0xAA57}, 
	{0x36F2, 0xF65A}, 
	{0x36F4, 0x52DB}, 
	{0x36F6, 0x4CDE}, 
	{0x36F8, 0x7475}, 
	{0x36FA, 0xD936}, 
	{0x36FC, 0xEDFA}, 
	{0x36FE, 0x7DDA}, 
	{0x3700, 0x46DE}, 
	{0x3702, 0x4775}, 
	{0x3704, 0xE5F6}, 
	{0x3706, 0xF9DA}, 
	{0x3708, 0x281B}, 
	{0x370A, 0x4C1E}, 
	{0x370C, 0x0396}, 
	{0x370E, 0xF016}, 
	{0x3710, 0x85DB}, 
	{0x3712, 0x239B}, 
	{0x3714, 0x6B9E}, 
	{0x3644, 0x0154}, 
	{0x3642, 0x00DC}, 
	{0x3210, 0x09B8}, 
	{0x0018, 0x0028}, 	
};

static struct msm_camera_i2c_reg_conf mt9v113_recommend_settings_4_mirror_flip[] =
{
	{0x3400, 0x7A38}, 	
	{0x321C, 0x0003}, 	

	{0x098C, 0x02F0}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x02F2}, 	
	{0x0990, 0x0210}, 	
	{0x098C, 0x02F4}, 	
	{0x0990, 0x001A}, 	
	{0x098C, 0x2145}, 	
	{0x0990, 0x02F4}, 	
	{0x098C, 0xA134}, 	
	{0x0990, 0x0001}, 	
	{0x31E0, 0x0001}, 	

	{0x098C, 0x2703}, 	
	{0x0990, 0x0280}, 	
	{0x098C, 0x2705}, 	
	{0x0990, 0x01E0}, 	
	{0x098C, 0x2707}, 	
	{0x0990, 0x0280}, 	
	{0x098C, 0x2709}, 	
	{0x0990, 0x01E0}, 	
	{0x098C, 0x270D}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x270F}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x2711}, 	
	{0x0990, 0x01E7}, 	
	{0x098C, 0x2713}, 	
	{0x0990, 0x0287}, 	
	{0x098C, 0x2715}, 	
	{0x0990, 0x0001}, 	
	{0x098C, 0x2717}, 	
	{0x0990, 0x0026}, 	
	{0x098C, 0x2719}, 	
	{0x0990, 0x001A}, 	
	{0x098C, 0x271B}, 	
	{0x0990, 0x006B}, 	
	{0x098C, 0x271D}, 	
	{0x0990, 0x006B}, 	
	{0x098C, 0x271F}, // MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_A]
	{0x0990, 0x032A}, // MCU_DATA_0
	{0x098C, 0x2721}, 	
	{0x0990, 0x0364}, 	
	{0x098C, 0x2723}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x2725}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x2727}, 	
	{0x0990, 0x01E7}, 	
	{0x098C, 0x2729}, 	
	{0x0990, 0x0287}, 	
	{0x098C, 0x272B}, 	
	{0x0990, 0x0001}, 	
	{0x098C, 0x272D}, 	
	{0x0990, 0x0026}, 	
	{0x098C, 0x272F}, 	
	{0x0990, 0x001A}, 	
	{0x098C, 0x2731}, 	
	{0x0990, 0x006B}, 	
	{0x098C, 0x2733}, 	
	{0x0990, 0x006B}, 	
	{0x098C, 0x2735}, 	
	{0x0990, 0x0426}, 	
	{0x098C, 0x2737}, 	
	{0x0990, 0x0363}, 	
	{0x098C, 0x2739}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x273B}, 	
	{0x0990, 0x027F}, 	
	{0x098C, 0x273D}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x273F}, 	
	{0x0990, 0x01DF}, 	
	{0x098C, 0x2747}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x2749}, 	
	{0x0990, 0x027F}, 	
	{0x098C, 0x274B}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0x274D}, 	
	{0x0990, 0x01DF}, 	
	{0x098C, 0x222D}, 
	{0x0990, 0x0088}, 
	{0x098C, 0xA408}, 
	{0x0990, 0x0020}, 
	{0x098C, 0xA409}, 
	{0x0990, 0x0023}, 
	{0x098C, 0xA40A}, 
	{0x0990, 0x0027}, 
	{0x098C, 0xA40B}, 
	{0x0990, 0x002A}, 
	{0x098C, 0x2411}, 
	{0x0990, 0x0088}, 
	{0x098C, 0x2413}, 
	{0x0990, 0x00A4}, 
	{0x098C, 0x2415}, 
	{0x0990, 0x0088}, 
	{0x098C, 0x2417}, 
	{0x0990, 0x00A4}, 
	{0x098C, 0xA404}, 
	{0x0990, 0x0010}, 
	{0x098C, 0xA40D}, 
	{0x0990, 0x0002}, 
	{0x098C, 0xA40E}, 
	{0x0990, 0x0003},
	{0x098C, 0xA410}, 	
	{0x0990, 0x000A}, 	

	{0x364E, 0x0350}, 
	{0x3650, 0x22ED}, 
	{0x3652, 0x0513}, 
	{0x3654, 0x6C70}, 
	{0x3656, 0x5015}, 
	{0x3658, 0x0130}, 
	{0x365A, 0x444D}, 
	{0x365C, 0x18D3}, 
	{0x365E, 0x5FB1}, 
	{0x3660, 0x6415}, 
	{0x3662, 0x00D0}, 
	{0x3664, 0x014C}, 
	{0x3666, 0x7BB2}, 
	{0x3668, 0x31B1}, 
	{0x366A, 0x46D5}, 
	{0x366C, 0x0130}, 
	{0x366E, 0x338D}, 
	{0x3670, 0x0593}, 
	{0x3672, 0x13D1}, 
	{0x3674, 0x4875}, 
	{0x3676, 0x992E}, 
	{0x3678, 0x910E}, 
	{0x367A, 0xAF92}, 
	{0x367C, 0x1732}, 
	{0x367E, 0x7BD3}, 
	{0x3680, 0x98EE}, 
	{0x3682, 0xEF4D}, 
	{0x3684, 0x8872}, 
	{0x3686, 0x0352}, 
	{0x3688, 0x2792}, 
	{0x368A, 0xDB6D}, 
	{0x368C, 0xF52D}, 
	{0x368E, 0xA532}, 
	{0x3690, 0x0213}, 
	{0x3692, 0x10D5}, 
	{0x3694, 0x8BCE}, 
	{0x3696, 0xFC2D}, 
	{0x3698, 0xA532}, 
	{0x369A, 0x67F1}, 
	{0x369C, 0x1034}, 
	{0x369E, 0x1113}, 
	{0x36A0, 0x2EF3}, 
	{0x36A2, 0x39F7}, 
	{0x36A4, 0xB097}, 
	{0x36A6, 0x81BA}, 
	{0x36A8, 0x2CF3}, 
	{0x36AA, 0x1373}, 
	{0x36AC, 0x4457}, 
	{0x36AE, 0xFAF6}, 
	{0x36B0, 0xEC19}, 
	{0x36B2, 0x0E73}, 
	{0x36B4, 0x0873}, 
	{0x36B6, 0x34F7}, 
	{0x36B8, 0x9EB7}, 
	{0x36BA, 0x9B9A}, 
	{0x36BC, 0x0E33}, 
	{0x36BE, 0x2013}, 
	{0x36C0, 0x3C37}, 
	{0x36C2, 0xA0D7}, 
	{0x36C4, 0x935A}, 
	{0x36C6, 0xCD11}, 
	{0x36C8, 0x0353}, 
	{0x36CA, 0x2516}, 
	{0x36CC, 0x8437}, 
	{0x36CE, 0xA01A}, 
	{0x36D0, 0xFCF1}, 
	{0x36D2, 0xAD91}, 
	{0x36D4, 0x29D4}, 
	{0x36D6, 0x0AB6}, 
	{0x36D8, 0x9436}, 
	{0x36DA, 0xA872}, 
	{0x36DC, 0xD00F}, 
	{0x36DE, 0x2B36}, 
	{0x36E0, 0x0714}, 
	{0x36E2, 0xFEB9}, 
	{0x36E4, 0xD211}, 
	{0x36E6, 0x22F1}, 
	{0x36E8, 0x5BD5}, 
	{0x36EA, 0x98D3}, 
	{0x36EC, 0xF4D9}, 
	{0x36EE, 0x78B5}, 
	{0x36F0, 0xAA57}, 
	{0x36F2, 0xF65A}, 
	{0x36F4, 0x52DB}, 
	{0x36F6, 0x4CDE}, 
	{0x36F8, 0x7475}, 
	{0x36FA, 0xD936}, 
	{0x36FC, 0xEDFA}, 
	{0x36FE, 0x7DDA}, 
	{0x3700, 0x46DE}, 
	{0x3702, 0x4775}, 
	{0x3704, 0xE5F6}, 
	{0x3706, 0xF9DA}, 
	{0x3708, 0x281B}, 
	{0x370A, 0x4C1E}, 
	{0x370C, 0x0396}, 
	{0x370E, 0xF016}, 
	{0x3710, 0x85DB}, 
	{0x3712, 0x239B}, 
	{0x3714, 0x6B9E}, 
	{0x3644, 0x0154}, 
	{0x3642, 0x00DC}, 
	{0x3210, 0x09B8}, 
	{0x0018, 0x0028}, 	
};
static struct msm_camera_i2c_reg_conf mt9v113_recommend_settings_5[] =
{      
	{0x098C, 0xA20C}, 
	{0x0990, 0x0010}, 
	{0x098C, 0xA215}, 
	{0x0990, 0x0010}, 
	{0x098C, 0x2212}, 
	{0x0990, 0x0180}, 
	{0x098C, 0xA24F}, 
	{0x0990, 0x0038}, 

	{0x098C, 0x2306}, 
	{0x0990, 0x0616}, 
	{0x098C, 0x231C}, 
	{0x0990, 0x00B0}, 
	{0x098C, 0x2308}, 
	{0x0990, 0xFAEB}, 
	{0x098C, 0x231E}, 
	{0x0990, 0xFF57}, 
	{0x098C, 0x230A}, 
	{0x0990, 0x005A}, 
	{0x098C, 0x2320}, 
	{0x0990, 0x0014}, 
	{0x098C, 0x230C}, 
	{0x0990, 0xFE50}, 
	{0x098C, 0x2322}, 
	{0x0990, 0x0066}, 
	{0x098C, 0x230E}, 
	{0x0990, 0x0503}, 
	{0x098C, 0x2324}, 
	{0x0990, 0x0008}, 
	{0x098C, 0x2310}, 
	{0x0990, 0xFE31}, 
	{0x098C, 0x2326}, 
	{0x0990, 0xFFAD}, 
	{0x098C, 0x2312}, 
	{0x0990, 0xFE37}, 
	{0x098C, 0x2328}, 
	{0x0990, 0x0130}, 
	{0x098C, 0x2314}, 
	{0x0990, 0xFBD4}, 
	{0x098C, 0x232A}, 
	{0x0990, 0x01BA}, 
	{0x098C, 0x2316}, 
	{0x0990, 0x0766}, 
	{0x098C, 0x232C}, 
	{0x0990, 0xFD2D}, 
	{0x098C, 0x2318}, 	
	{0x0990, 0x001C}, 	
	{0x098C, 0x231A}, 	
	{0x0990, 0x0039}, 	
	{0x098C, 0x232E}, 	
	{0x0990, 0x0001}, 	
	{0x098C, 0x2330}, 	
	{0x0990, 0xFFEF}, 	
	{0x098C, 0xA366}, 
	{0x0990, 0x0080}, 
	{0x098C, 0xA367}, 
	{0x0990, 0x0080}, 
	{0x098C, 0xA368}, 
	{0x0990, 0x0080}, 
	{0x098C, 0xA369}, 
	{0x0990, 0x0080}, 
	{0x098C, 0xA36A}, 
	{0x0990, 0x0080}, 
	{0x098C, 0xA36B}, 
	{0x0990, 0x0080}, 
	{0x098C, 0xA348}, 	
	{0x0990, 0x0008}, 	
	{0x098C, 0xA349}, 	
	{0x0990, 0x0002}, 	
	{0x098C, 0xA34A}, 	
	{0x0990, 0x0090}, 	
	{0x098C, 0xA34B}, 	
	{0x0990, 0x00FF}, 	
	{0x098C, 0xA34C}, 	
	{0x0990, 0x0075}, 	
	{0x098C, 0xA34D}, 	
	{0x0990, 0x00EF}, 	
	{0x098C, 0xA351}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0xA352}, 	
	{0x0990, 0x007F}, 	
	{0x098C, 0xA354}, 	
	{0x0990, 0x0043}, 	
	{0x098C, 0xA355}, 	
	{0x0990, 0x0001}, 	
	{0x098C, 0xA35D}, 	
	{0x0990, 0x0078}, 	
	{0x098C, 0xA35E}, 	
	{0x0990, 0x0086}, 	
	{0x098C, 0xA35F}, 	
	{0x0990, 0x007E}, 	
	{0x098C, 0xA360}, 	
	{0x0990, 0x0082}, 	
	{0x098C, 0x2361}, 	
	{0x0990, 0x0040}, 	
	{0x098C, 0xA363}, 	
	{0x0990, 0x00D2}, 	
	{0x098C, 0xA364}, 	
	{0x0990, 0x00F6}, 	
	{0x098C, 0xA302}, 	
	{0x0990, 0x0000}, 	
	{0x098C, 0xA303}, 	
	{0x0990, 0x00EF}, 	
	{0x098C, 0x274F}, 	
	{0x0990, 0x0004}, 	
	{0x098C, 0x2741}, 	
	{0x0990, 0x0004}, 	
	{0x098C, 0xAB1F}, 	
	{0x0990, 0x00C7}, 	
	{0x098C, 0xAB31}, 	
	{0x0990, 0x001E}, 	
	{0x098C, 0xAB20}, 	
	{0x0990, 0x0024}, 	
	{0x098C, 0xAB21}, 	
	{0x0990, 0x0046}, 	
	{0x098C, 0xAB22}, 	
	{0x0990, 0x0002}, 	
	{0x098C, 0xAB24}, 	
	{0x0990, 0x0000}, 
	{0x098C, 0x2B28}, 	
	{0x0990, 0x170C}, 	
	{0x098C, 0x2B2A}, 	
	{0x0990, 0x3E80}, 	
	{0x098C, 0xA103}, 	
	{0x0990, 0x0006}, 
};
static struct msm_camera_i2c_reg_conf mt9v113_recommend_settings_6[] =
{     
	{0x098C, 0xA103}, 	
	{0x0990, 0x0005},  
};

static struct msm_camera_i2c_reg_conf mt9v113_wb_auto_reg_config[] =
{
	{0x098C, 0xA34a},
	{0x0990, 0x0090},
	{0x098C, 0xA34b},
	{0x0990, 0x00ff},
	{0x098C, 0xA34c},
	{0x0990, 0x0075},
	{0x098C, 0xA34d},
	{0x0990, 0x00ef},
	{0x098C, 0xA351},
	{0x0990, 0x0000},
	{0x098C, 0xA352},
	{0x0990, 0x007f},
	//Refresh
	{0x098C, 0xA103},
	{0x0990, 0x0005},
};
static struct msm_camera_i2c_reg_conf mt9v113_wb_incandescent_reg_config[] =
{
	{0x098C, 0xA34a},   // MCU_ADDRESS 
	/*tune the R-gain and B-gain for whitebalance*/
	{0x0990, 0x0094},   // MCU_DATA_0  
	{0x098C, 0xA34b},
	{0x0990, 0x0094},    // MCU_DATA_0 9e->0x008D
	{0x098C, 0xA34c},    // MCU_ADDRESS 
	{0x0990, 0x0084},    // MCU_DATA_0
	{0x098C, 0xA34d},
	{0x0990, 0x0084},    // MCU_DATA_0 93->83
	{0x098C, 0xA351},    // MCU_ADDRESS 
	{0x0990, 0x0000},    // MCU_DATA_0
	{0x098C, 0xA352},    // MCU_ADDRESS 
	{0x0990, 0x0000},    // MCU_DATA_0
	//Refresh
	{0x098C, 0xA103},
	{0x0990, 0x0005},
};
static struct msm_camera_i2c_reg_conf mt9v113_wb_fluorescent_reg_config[] =
{
	{0x098C, 0xA34a},   // MCU_ADDRESS []	   
	{0x0990, 0x00af},   // MCU_DATA_0	 c8   
	{0x098C, 0xA34b},   // MCU
	{0x0990, 0x00af},   // MCU_DATA_0	 c8   
	{0x098C, 0xA34c},   // MCU_ADDRESS []	   
	{0x0990, 0x0070},   // MCU_DATA_0	 81  
	{0x098C, 0xA34d},   // MCU
	{0x0990, 0x0070},   // MCU_DATA_0	 81  
	{0x098C, 0xA351},   // MCU_ADDRESS []	   
	{0x0990, 0x000c},   // MCU_DATA_0	   
	{0x098C, 0xA352},   // MCU_ADDRESS []	   
	{0x0990, 0x000c},   // MCU_DATA_0	   
	//Refresh
	{0x098C, 0xA103},
	{0x0990, 0x0005},
};
static struct msm_camera_i2c_reg_conf mt9v113_wb_daylight_reg_config[] =
{
	{0x098C, 0xA34a},     // MCU_ADDRESS []  
	{0x0990, 0x00c8},     // MCU_DATA_0  ff 
	{0x098C, 0xA34b},
	{0x0990, 0x00c8},     // MCU_DATA_0  ff 
	{0x098C, 0xA34c},     // MCU_ADDRESS []  
	{0x0990, 0x008a},     // MCU_DATA_0	7a 
	{0x098C, 0xA34d},
	{0x0990, 0x008a},     // MCU_DATA_0	7a  
	{0x098C, 0xA351},     // MCU_ADDRESS []  
	{0x0990, 0x007f},     // MCU_DATA_0	  
	{0x098C, 0xA352},     // MCU_ADDRESS []  
	{0x0990, 0x007f},     // MCU_DATA_0	  
	//Refresh
	{0x098C, 0xA103},
	{0x0990, 0x0005},
};
static struct msm_camera_i2c_reg_conf mt9v113_wb_cloudy_reg_config[] =
{
	{0x098C, 0xA34a},   // MCU_ADDRESS []	  
	{0x0990, 0x00ff},	  // MCU_DATA_0	  
	{0x098C, 0xA34b},				 // MC
	{0x0990, 0x00f0},   // MCU_DATA_0	  
	{0x098C, 0xA34c},   // MCU_ADDRESS []	  
	{0x0990, 0x006d},   // MCU_DATA_0		  
	{0x098C, 0xA34d},				 // MC
	{0x0990, 0x007a},   // MCU_DATA_0	  
	{0x098C, 0xA351},   // MCU_ADDRESS []	  
	{0x0990, 0x007f},   // MCU_DATA_0	  
	{0x098C, 0xA352},   // MCU_ADDRESS []	  
	{0x0990, 0x007f},   // MCU_DATA_0	  
	//Refresh
	{0x098C, 0xA103},
	{0x0990, 0x0005},
};

static struct msm_camera_i2c_reg_conf mt9v113_effect_off_reg_config[] =
{
	{0x098C, 0x2759},     // MCU_ADDRESS [MODE_SPEC_EFFECTS_A]
	{0x0990, 0x6440},     // MCU_DATA_0
	{0x098C, 0x275B},     // MCU_ADDRESS [MODE_SPEC_EFFECTS_B]
	{0x0990, 0x6440},     // MCU_DATA_0
	{0x098C, 0x2763},     // MCU_ADDRESS [MODE_COMMONMODESETTINGS_FX_SEPIA_SETTINGS]
	{0x0990, 0x6440},     // MCU_DATA_0
	{0x098C, 0xA103},     // MCU_ADDRESS [SEQ_CMD]
	{0x0990, 0x0005},     // MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf mt9v113_effect_mono_reg_config[] =
{
	{0x098C, 0x2759},     // MCU_ADDRESS [MODE_SPEC_EFFECTS_A]
	{0x0990, 0x6441},     // MCU_DATA_0
	{0x098C, 0x275B},     // MCU_ADDRESS [MODE_SPEC_EFFECTS_B]
	{0x0990, 0x6441},     // MCU_DATA_0
	{0x098C, 0xA103},     // MCU_ADDRESS [SEQ_CMD]
	{0x0990, 0x0005},     // MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf mt9v113_effect_negative_reg_config[] =
{
	{0x098C, 0x2759},     // MCU_ADDRESS [MODE_SPEC_EFFECTS_A]
	{0x0990, 0x0043},     // MCU_DATA_0
	{0x098C, 0x275B},     // MCU_ADDRESS [MODE_SPEC_EFFECTS_B]
	{0x0990, 0x0943},     // MCU_DATA_0
	{0x098C, 0xA103},     // MCU_ADDRESS [SEQ_CMD]
	{0x0990, 0x0005},     // MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf mt9v113_effect_sepia_reg_config[] =
{
	{0x098C, 0x2759},     // MCU_ADDRESS [MODE_SPEC_EFFECTS_A]
	{0x0990, 0x0042},     // MCU_DATA_0
	{0x098C, 0x275B},     // MCU_ADDRESS [MODE_SPEC_EFFECTS_B]
	{0x0990, 0x0942},     // MCU_DATA_0
	{0x098C, 0x2763},     // MCU_ADDRESS [MODE_COMMONMODESETTINGS_FX_SEPIA_SETTINGS]
	{0x0990, 0xB023},     // MCU_DATA_0
	{0x098C, 0xA103},     // MCU_ADDRESS [SEQ_CMD]
	{0x0990, 0x0005},     // MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf mt9v113_effect_aqua_reg_config[] =
{
 
	{0x098C, 0x2759},     // MCU_ADDRESS [MODE_SPEC_EFFECTS_A]
	{0x0990, 0x0042},     // MCU_DATA_0
	{0x098C, 0x275B},     // MCU_ADDRESS [MODE_SPEC_EFFECTS_B]
	{0x0990, 0x0942},     // MCU_DATA_0
	{0x098C, 0x2763 },	// MCU_ADDRESS [MODE_COMMONMODESETTINGS_FX_SEPIA_SETTINGS]
	{0x0990, 0x28ca },	// MCU_DATA_0
	{0x098C, 0xA103},     // MCU_ADDRESS [SEQ_CMD]
	{0x0990, 0x0005},     // MCU_DATA_0
};
static struct v4l2_subdev_info mt9v113_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array mt9v113_init_conf[] = {
	{&mt9v113_recommend_settings_1[0],
	ARRAY_SIZE(mt9v113_recommend_settings_1), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9v113_recommend_settings_2[0],
	ARRAY_SIZE(mt9v113_recommend_settings_2), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9v113_recommend_settings_3[0],
	ARRAY_SIZE(mt9v113_recommend_settings_3), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9v113_recommend_settings_4[0],
	ARRAY_SIZE(mt9v113_recommend_settings_4), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9v113_recommend_settings_5[0],
	ARRAY_SIZE(mt9v113_recommend_settings_5), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9v113_recommend_settings_6[0],
	ARRAY_SIZE(mt9v113_recommend_settings_6), 0, MSM_CAMERA_I2C_WORD_DATA},
	};

static struct msm_camera_i2c_conf_array mt9v113_confs[] = {
	{NULL,0, 0, MSM_CAMERA_I2C_WORD_DATA},
	{NULL,0, 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_sensor_output_info_t mt9v113_dimensions[] = {
	{
		.x_output = 0x280,
		.y_output = 0x1E0,
		.line_length_pclk = 0x034A,
		.frame_length_lines = 0x022A,
		.vt_pixel_clk = 21000000,
		.op_pixel_clk = 16800000,
		.binning_factor = 1,
	},
	{
		.x_output = 0x280,
		.y_output = 0x1E0,
		.line_length_pclk = 0x034A,
		.frame_length_lines = 0x022A,
		.vt_pixel_clk = 21000000,
		.op_pixel_clk = 16800000,
		.binning_factor = 1,
	},
};

static struct msm_camera_csi_params mt9v113_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x18,
};

static struct msm_camera_csi_params *mt9v113_csi_params_array[] = {
	&mt9v113_csi_params,
	&mt9v113_csi_params,
};

static struct msm_sensor_id_info_t mt9v113_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x2280,
};

static const struct i2c_device_id mt9v113_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&mt9v113_s_ctrl},
	{ }
};

static struct i2c_driver mt9v113_i2c_driver = {
	.id_table = mt9v113_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client mt9v113_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
	.addr_pos = 0,
	.addr_dir = 0,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&mt9v113_i2c_driver);
}


int32_t mt9v113_wait(struct msm_sensor_ctrl_t *s_ctrl,  int time)
{
	int            count = 0;
	unsigned short r_value = 0;
	unsigned short bit15 = 0;
	/* modify the delay time for CPU 1.4G */
	/*modify delays and polls after register writing*/
	switch(time){
		case 0:
		case 2:
		case 3:
			mdelay(30);
			break;
		case 1:
			for(count = 50; count > 0; count --)
			{
				msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x0014, &bit15,MSM_CAMERA_I2C_WORD_DATA);
				bit15 = bit15 & 0x8000;
				printk("count = %d, bit15 = 0x%x,\n", count,bit15);
				 
				if(0x8000 == bit15) 
				{
				    break;
				}
				mdelay(10);
			}
			break;
		case 4:
		case 5:
			for(count = 50; count > 0; count --)
			{
				msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x098C, 0xA103,MSM_CAMERA_I2C_WORD_DATA);
				msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x0990, &r_value,  MSM_CAMERA_I2C_WORD_DATA);
				printk("count = %d, value = %d\n", count, r_value);
				 
				if(0 == r_value) 
				{
					if( 5 == time)
						mdelay(100);
					break;
				}
				mdelay(10);
			}
			break;
		default:
			break;
	}

	return 0;
}

int32_t mt9v113_write_init_settings(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc=0, i;

	printk("%s is called !\n", __func__);
	mt9v113_set_mirror_mode(s_ctrl);
	
	for (i = 0; i < s_ctrl->msm_sensor_reg->init_size; i++) 
	{
		rc = msm_sensor_write_conf_array(
			s_ctrl->sensor_i2c_client, 
			s_ctrl->msm_sensor_reg->init_settings, i);
		if (rc < 0)
			break;
		mt9v113_wait(s_ctrl, i);
	}

	return rc;
}
void  mt9v113_set_mirror_mode(struct msm_sensor_ctrl_t *s_ctrl)
{

	if (HW_MIRROR_AND_FLIP == (get_hw_camera_mirror_type() >> 1)) 
	{
		s_ctrl->msm_sensor_reg->init_settings[3].conf = &mt9v113_recommend_settings_4_mirror_flip[0];
		s_ctrl->msm_sensor_reg->init_settings[3].size = ARRAY_SIZE(mt9v113_recommend_settings_4_mirror_flip);
	}
}
int32_t mt9v113_sensor_model_match(struct msm_sensor_ctrl_t *s_ctrl)
{
	unsigned short model_id = 0;
	int32_t rc=0;
	
	rc = mt9v113_write_init_settings(s_ctrl);
	mdelay(50);
	/*
	* after sensor is initialized ,we distinguish the model by CAM_ID gpio:
	* sunny model's GPIO[1] connects DGND, byd model's GPIO[1] connects to DOVDD
	* bit[9] of register 0x1070 is the value of GPIO[1] signal, so we read
	* the value of 0x1070 and move right 9 bits to get the GPIO[1] value
	*/

	rc = msm_camera_i2c_write(
					s_ctrl->sensor_i2c_client, 
					0x098C,0x1070, 
					MSM_CAMERA_I2C_WORD_DATA);
	rc = msm_camera_i2c_read(
					s_ctrl->sensor_i2c_client,
					0x0990, &model_id,
					MSM_CAMERA_I2C_WORD_DATA);
	
	model_id = (model_id & 0x0200) >> 9;
	CDBG("cam_id gpio[1]= %d\n", model_id);

	if(MODEL_SUNNY == model_id)
	{
		strncpy((char *)s_ctrl->sensor_name, "23060075FF-MT-S", strlen("23060075FF-MT-S"));
	}
	else if( MODEL_BYD == model_id) 
	{
		strncpy((char *)s_ctrl->sensor_name, "23060075FF-MT-B", strlen("23060075FF-MT-B"));
	}
	else
	{
		strncpy((char *)s_ctrl->sensor_name, "23060075FF-MT", strlen("23060075FF-MT"));
	}
	printk("mt9v113.c name is %s \n", s_ctrl->sensor_name);
	
	return rc;
}
/*func for mt9v113 to set wb*/
int32_t mt9v113_sensor_set_wb(struct msm_sensor_ctrl_t *s_ctrl, int wb)
{
	struct msm_camera_i2c_reg_conf *reg_conf_tbl = NULL;
	uint16_t num_of_items_in_table = 0;
	int rc = 0;

	printk("%s, to set wb = %d \n", __func__,wb);
	switch (wb)
	{
		case CAMERA_WB_AUTO:
			reg_conf_tbl = &mt9v113_wb_auto_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9v113_wb_auto_reg_config);
			break;
		case CAMERA_WB_INCANDESCENT:
			reg_conf_tbl = &mt9v113_wb_incandescent_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9v113_wb_incandescent_reg_config);
			break;
		case CAMERA_WB_FLUORESCENT:
			reg_conf_tbl = &mt9v113_wb_fluorescent_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9v113_wb_fluorescent_reg_config);
			break;
		case CAMERA_WB_DAYLIGHT:
			reg_conf_tbl = &mt9v113_wb_daylight_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9v113_wb_daylight_reg_config);
			break;
		case CAMERA_WB_CLOUDY_DAYLIGHT:
			reg_conf_tbl = &mt9v113_wb_cloudy_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9v113_wb_cloudy_reg_config);
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

/*func for mt9v113 to set effect*/
int32_t mt9v113_sensor_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int effect)
{
	struct msm_camera_i2c_reg_conf *reg_conf_tbl = NULL;
	uint16_t num_of_items_in_table = 0;
	int rc = 0;

	printk("%s, to set effect = %d \n", __func__,effect);
	switch (effect)
	{
		case CAMERA_EFFECT_OFF:
			reg_conf_tbl = &mt9v113_effect_off_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9v113_effect_off_reg_config);
			break;
		case CAMERA_EFFECT_MONO:
			reg_conf_tbl = &mt9v113_effect_mono_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9v113_effect_mono_reg_config);
			break;
		case CAMERA_EFFECT_NEGATIVE:
			reg_conf_tbl = &mt9v113_effect_negative_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9v113_effect_negative_reg_config);
			break;
		case CAMERA_EFFECT_SEPIA:
			reg_conf_tbl = &mt9v113_effect_sepia_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9v113_effect_sepia_reg_config);
			break;
		case CAMERA_EFFECT_AQUA:
			reg_conf_tbl = &mt9v113_effect_aqua_reg_config[0];
			num_of_items_in_table = ARRAY_SIZE(mt9v113_effect_aqua_reg_config);
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
void mt9v113_sensor_mclk_self_adapt(struct msm_sensor_ctrl_t *s_ctrl)
{
    if(machine_is_msm8x25_C8950D()
    || machine_is_msm8x25_U8950D()
    || machine_is_msm8x25_U8950())
    {
        s_ctrl->clk_rate = MSM_SENSOR_MCLK_8HZ;
        s_ctrl->msm_sensor_reg->init_settings[0].conf = &mt9v113_recommend_settings_8Mmclk[0];
        s_ctrl->msm_sensor_reg->init_settings[0].size = ARRAY_SIZE(mt9v113_recommend_settings_8Mmclk);
    }
    
}
int32_t mt9v113_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
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

	if (s_ctrl->clk_rate != 0)
		clk_info->clk_rate = s_ctrl->clk_rate;

	rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(clk_info), 1);
	if (rc < 0) {
		pr_err("%s: clk enable failed\n", __func__);
		goto enable_clk_failed;
	}
      if(data->standby_is_supported)
      {
           csi_config = 0;
      }
	/* power up one time in standby mode */
      if((false == data->standby_is_supported) 
         || (0 == strcmp(data->sensor_name, ""))
         || (false == standby_mode))
      {
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
       }
	usleep_range(2000,3000);
	rc = msm_camera_config_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: config gpio failed\n", __func__);
		goto config_gpio_failed;
	}
	usleep_range(2000,3000);
	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(1);

	if (data->sensor_platform_info->i2c_conf &&
		data->sensor_platform_info->i2c_conf->use_i2c_mux)
		msm_sensor_enable_i2c_mux(data->sensor_platform_info->i2c_conf);

	return rc;
	
config_gpio_failed:
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
	msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(clk_info), 0);	
enable_clk_failed:
	msm_camera_request_gpio_table(data, 0);
request_gpio_failed:
	kfree(s_ctrl->reg_ptr);
	return rc;
}


static struct v4l2_subdev_core_ops mt9v113_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops mt9v113_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops mt9v113_subdev_ops = {
	.core = &mt9v113_subdev_core_ops,
	.video  = &mt9v113_subdev_video_ops,
};

static struct msm_sensor_fn_t mt9v113_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_csi_setting = msm_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = mt9v113_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_write_init_settings = mt9v113_write_init_settings,
	.sensor_model_match = mt9v113_sensor_model_match,
	.sensor_set_wb = mt9v113_sensor_set_wb,
	.sensor_set_effect = mt9v113_sensor_set_effect,
    .sensor_mclk_self_adapt = mt9v113_sensor_mclk_self_adapt,
};
 
static struct msm_sensor_reg_t mt9v113_regs = {
	.default_data_type = MSM_CAMERA_I2C_WORD_DATA,
	.start_stream_conf = mt9v113_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(mt9v113_start_settings),
	.stop_stream_conf = mt9v113_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(mt9v113_stop_settings),
	.group_hold_on_conf_size = 0,
	.group_hold_off_conf_size = 0,
	.init_settings = &mt9v113_init_conf[0],
	.init_size = ARRAY_SIZE(mt9v113_init_conf),
	.mode_settings = &mt9v113_confs[0],
	.output_settings = &mt9v113_dimensions[0],
	.num_conf = ARRAY_SIZE(mt9v113_dimensions),
};

static struct msm_sensor_ctrl_t mt9v113_s_ctrl = {
	.msm_sensor_reg = &mt9v113_regs,
	.sensor_i2c_client = &mt9v113_sensor_i2c_client,
	.sensor_i2c_addr = 0x7A, 
	.sensor_id_info = &mt9v113_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &mt9v113_csi_params_array[0],
	.msm_sensor_mutex = &mt9v113_mut,
	.sensor_i2c_driver = &mt9v113_i2c_driver,
	.sensor_v4l2_subdev_info = mt9v113_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9v113_subdev_info),
	.sensor_v4l2_subdev_ops = &mt9v113_subdev_ops,
	.func_tbl = &mt9v113_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
	.sensor_name = {0},
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Aptian VGA YUV sensor driver");
MODULE_LICENSE("GPL v2");

