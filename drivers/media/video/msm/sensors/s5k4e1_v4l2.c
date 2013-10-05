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

#include <linux/module.h>
#include "msm_sensor.h"
#define SENSOR_NAME "s5k4e1"
#define PLATFORM_DRIVER_NAME "msm_camera_s5k4e1"
#define s5k4e1_obj s5k4e1_##obj
#define MSB                             1
#define LSB                             0

#include <linux/gpio.h>
/*added struct is no use if the number of module is more than two*/
#define S5k4E1_CAMERA_ID_GPIO 9

DEFINE_MUTEX(s5k4e1_mut);
static struct msm_sensor_ctrl_t s5k4e1_s_ctrl;

static struct msm_camera_i2c_reg_conf s5k4e1_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf s5k4e1_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf s5k4e1_groupon_settings[] = {
	{0x0104, 0x01},
};

static struct msm_camera_i2c_reg_conf s5k4e1_groupoff_settings[] = {
	{0x0104, 0x00},
};

static struct msm_camera_i2c_reg_conf s5k4e1_prev_settings[] = {
	/* output size (1304 x 980) */
	{0x30A9, 0x02},/* Horizontal Binning On */
	{0x300E, 0xEB},/* Vertical Binning On */
	{0x0387, 0x03},/* y_odd_inc 03(10b AVG) */
	{0x0344, 0x00},/* x_addr_start 0 */
	{0x0345, 0x00},
	{0x0348, 0x0A},/* x_addr_end 2607 */
	{0x0349, 0x2F},
	{0x0346, 0x00},/* y_addr_start 0 */
	{0x0347, 0x00},
	{0x034A, 0x07},/* y_addr_end 1959 */
	{0x034B, 0xA7},
	{0x0380, 0x00},/* x_even_inc 1 */
	{0x0381, 0x01},
	{0x0382, 0x00},/* x_odd_inc 1 */
	{0x0383, 0x01},
	{0x0384, 0x00},/* y_even_inc 1 */
	{0x0385, 0x01},
	{0x0386, 0x00},/* y_odd_inc 3 */
	{0x0387, 0x03},
	{0x034C, 0x05},/* x_output_size 1304 */
	{0x034D, 0x18},
	{0x034E, 0x03},/* y_output_size 980 */
	{0x034F, 0xd4},
	{0x30BF, 0xAB},/* outif_enable[7], data_type[5:0](2Bh = bayer 10bit} */
	{0x30C0, 0xA0},/* video_offset[7:4] 3260%12 */
	{0x30C8, 0x06},/* video_data_length 1600 = 1304 * 1.25 */
	{0x30C9, 0x5E},
	/* Timing Configuration */
	{0x0202, 0x03},
	{0x0203, 0x14},
	{0x0204, 0x00},
	{0x0205, 0x80},
	{0x0340, 0x03},/* Frame Length */
	{0x0341, 0xE0},
	{0x0342, 0x0A},/* 2738  Line Length */
	{0x0343, 0xB2},
	/*preview lens shading*/
	{0x3096, 0x40},
	{0x3097, 0x52},
	{0x3098, 0x3e},
	{0x3099, 0x03},
	{0x309a, 0x1f},
	{0x309b, 0x04},
	{0x309c, 0x21},
	{0x309d, 0x00},
	{0x309e, 0x00},
	{0x309f, 0x00},
	{0x30a0, 0x00},
	{0x30a1, 0x00},
	{0x30a2, 0x00},
	{0x30a3, 0x00},
	{0x30a4, 0x00},
	{0x30a5, 0x01},
	{0x30a6, 0x00},
};

static struct msm_camera_i2c_reg_conf s5k4e1_snap_settings[] = {
	/*Output Size (2608x1960)*/
	{0x30A9, 0x03},/* Horizontal Binning Off */
	{0x300E, 0xE8},/* Vertical Binning Off */
	{0x0387, 0x01},/* y_odd_inc */
	{0x034C, 0x0A},/* x_output size */
	{0x034D, 0x30},
	{0x034E, 0x07},/* y_output size */
	{0x034F, 0xA8},
	{0x30BF, 0xAB},/* outif_enable[7], data_type[5:0](2Bh = bayer 10bit} */
	{0x30C0, 0x80},/* video_offset[7:4] 3260%12 */
	{0x30C8, 0x0C},/* video_data_length 3260 = 2608 * 1.25 */
	{0x30C9, 0xBC},
	/*Timing configuration*/
	{0x0202, 0x06},
	{0x0203, 0x28},
	{0x0204, 0x00},
	{0x0205, 0x80},
	{0x0340, 0x07},/* Frame Length */
	{0x0341, 0xB4},
	{0x0342, 0x0A},/* 2738 Line Length */
	{0x0343, 0xB2},
	{0x3096, 0x40},
	{0x3097, 0x52},
	{0x3098, 0x7b},
	{0x3099, 0x03},
	{0x309a, 0x1f},
	{0x309b, 0x02},
	{0x309c, 0x15},
	{0x309d, 0x00},
	{0x309e, 0x00},
	{0x309f, 0x00},
	{0x30a0, 0x00},
	{0x30a1, 0x00},
	{0x30a2, 0x00},
	{0x30a3, 0x00},
	{0x30a4, 0x00},
	{0x30a5, 0x01},
	{0x30a6, 0x00},
};

/*modify the initialization settings*/
static struct msm_camera_i2c_reg_conf s5k4e1_recommend_settings_1[] = {
	/* Reset setting */
	{0x0103, 0x01},

	/* REC Settings */
	/*CDS timing setting ... */
	{0x3000, 0x05},
	{0x3001, 0x03},
	{0x3002, 0x08},
	{0x3003, 0x09},
	{0x3004, 0x2E},
	{0x3005, 0x06},
	{0x3006, 0x34},
	{0x3007, 0x00},
	{0x3008, 0x3C},
	{0x3009, 0x3C},
	{0x300A, 0x28},
	{0x300B, 0x04},
	{0x300C, 0x0A},
	{0x300D, 0x02},
	{0x300E, 0xE8},
	{0x300F, 0x82},

	/* CDS option setting ... */
	{0x3010, 0x00},
	{0x3011, 0x4C},
	{0x3012, 0x30},
	{0x3013, 0xC0},
	{0x3014, 0x00},
	{0x3015, 0x00},
	{0x3016, 0x2C},
	{0x3017, 0x94},
	{0x3018, 0x78},
	{0x301B, 0x83},
	{0x301C, 0x04},
	{0x301D, 0xD4},
	{0x3021, 0x02},
	{0x3022, 0x24},
	{0x3024, 0x40},
	{0x3027, 0x08},

	/* Pixel option setting ...   */
	{0x3029, 0xC6},
	{0x30BC, 0xB0},
	{0x302B, 0x01},
	{0x30D8, 0x3F},
	{0x3070, 0x5F},
	{0x3071, 0x00},
	{0x3080, 0x04},
	{0x3081, 0x38},
};

static struct msm_camera_i2c_reg_conf s5k4e1_recommend_settings_2[] = {
	/* MIPI settings */
	{0x30BD, 0x00},/* SEL_CCP[0] */
	{0x3084, 0x15},/* SYNC Mode */
	{0x30BE, 0x1A},/* M_PCLKDIV_AUTO[4], M_DIV_PCLK[3:0] */
	{0x30C1, 0x01},/* pack video enable [0] */
	{0x30EE, 0x02},/* DPHY enable [ 1] */
	{0x3111, 0x86},/* Embedded data off [5] */
};

static struct msm_camera_i2c_reg_conf s5k4e1_recommend_settings_3[] = {
	/* PLL settings */
	{0x0305,0x06},//PLL P = 6
	{0x0306,0x00},//PLL M[8] = 0
	{0x0307,0x65},//PLL M = 101
	{0x30B5,0x01},//PLL S = 1 
	{0x30E2,0x02},//num lanes[1:0] = 2
	{0x30F1,0x70},//DPHY BANDCTRL 404MHz=40.4MHz
};

static struct msm_camera_i2c_reg_conf s5k4e1_recommend_settings_4[] = {
	{0x3096, 0x40},
	
	{0x3097, 0x52},
	{0x3098, 0x3e},
	{0x3099, 0x03},
	{0x309a, 0x1f},
	{0x309b, 0x04},
	{0x309c, 0x21},
	{0x309d, 0x00},
	{0x309e, 0x00},
	{0x309f, 0x00},
	{0x30a0, 0x00},
	{0x30a1, 0x00},
	{0x30a2, 0x00},
	{0x30a3, 0x00},
	{0x30a4, 0x00},
	
	{0x30a5, 0x01},
	{0x30a6, 0x00},
	
	{0x3200, 0x00},
	{0x3201, 0x74},
	{0x3202, 0xcf},
	{0x3203, 0x0f},
	{0x3204, 0xf1},
	{0x3205, 0x31},
	{0x3206, 0x0f},
	{0x3207, 0xff},
	{0x3208, 0xeb},
	{0x3209, 0x00},
	{0x320a, 0x0d},
	{0x320b, 0xed},
	{0x320c, 0x0f},
	{0x320d, 0xf3},
	{0x320e, 0x3c},
	{0x320f, 0x00},
	{0x3210, 0x06},
	{0x3211, 0x6b},
	{0x3212, 0x0f},
	{0x3213, 0xf4},
	{0x3214, 0xa5},
	{0x3215, 0x0f},
	{0x3216, 0xf7},
	{0x3217, 0xf0},
	{0x3218, 0x0f},
	{0x3219, 0xfd},
	{0x321a, 0xa9},
	{0x321b, 0x00},
	{0x321c, 0x0b},
	{0x321d, 0x28},
	{0x321e, 0x0f},
	{0x321f, 0xfd},
	{0x3220, 0xf8},
	{0x3221, 0x0f},
	{0x3222, 0xf7},
	{0x3223, 0x83},
	{0x3224, 0x0f},
	{0x3225, 0xef},
	{0x3226, 0xaf},
	{0x3227, 0x00},
	{0x3228, 0x04},
	{0x3229, 0x46},
	{0x322a, 0x00},
	{0x322b, 0x0a},
	{0x322c, 0x79},
	{0x322d, 0x0f},
	{0x322e, 0xe9},
	{0x322f, 0x08},
	{0x3230, 0x00},
	{0x3231, 0x03},
	{0x3232, 0xdc},
	{0x3233, 0x00},
	{0x3234, 0x14},
	{0x3235, 0x6d},
	{0x3236, 0x00},
	{0x3237, 0x1f},
	{0x3238, 0xb4},
	{0x3239, 0x00},
	{0x323a, 0x00},
	{0x323b, 0xaa},
	{0x323c, 0x0f},
	{0x323d, 0xfe},
	{0x323e, 0x85},
	{0x323f, 0x00},
	{0x3240, 0x0c},
	{0x3241, 0xbe},
	{0x3242, 0x0f},
	{0x3243, 0xff},
	{0x3244, 0x82},
	{0x3245, 0x0f},
	{0x3246, 0xe5},
	{0x3247, 0xe0},
	{0x3248, 0x0f},
	{0x3249, 0xf7},
	{0x324a, 0x6d},
	{0x324b, 0x0f},
	{0x324c, 0xf9},
	{0x324d, 0xd9},
	{0x324e, 0x0f},
	{0x324f, 0xef},
	{0x3250, 0xac},
	{0x3251, 0x00},
	{0x3252, 0x08},
	{0x3253, 0xbe},
	{0x3254, 0x00},
	{0x3255, 0x07},
	{0x3256, 0x6e},
	{0x3257, 0x00},
	{0x3258, 0x07},
	{0x3259, 0x1a},
	{0x325a, 0x0f},
	{0x325b, 0xf2},
	{0x325c, 0xf5},
	{0x325d, 0x00},
	{0x325e, 0x0a},
	{0x325f, 0x7c},
	{0x3260, 0x00},
	{0x3261, 0x0b},
	{0x3262, 0x9c},
	{0x3263, 0x0f},
	{0x3264, 0xf9},
	{0x3265, 0x5d},
	{0x3266, 0x0f},
	{0x3267, 0xec},
	{0x3268, 0x3b},
	{0x3269, 0x00},
	{0x326a, 0x0e},
	{0x326b, 0x97},
	{0x326c, 0x00},
	{0x326d, 0x84},
	{0x326e, 0x02},
	{0x326f, 0x0f},
	{0x3270, 0xef},
	{0x3271, 0x5e},
	{0x3272, 0x0f},
	{0x3273, 0xfb},
	{0x3274, 0xd5},
	{0x3275, 0x00},
	{0x3276, 0x18},
	{0x3277, 0x16},
	{0x3278, 0x0f},
	{0x3279, 0xe8},
	{0x327a, 0xde},
	{0x327b, 0x00},
	{0x327c, 0x0c},
	{0x327d, 0x01},
	{0x327e, 0x0f},
	{0x327f, 0xfa},
	{0x3280, 0x2c},
	{0x3281, 0x0f},
	{0x3282, 0xf2},
	{0x3283, 0xbd},
	{0x3284, 0x0f},
	{0x3285, 0xfa},
	{0x3286, 0x4e},
	{0x3287, 0x00},
	{0x3288, 0x15},
	{0x3289, 0xd5},
	{0x328a, 0x0f},
	{0x328b, 0xfa},
	{0x328c, 0xee},
	{0x328d, 0x0f},
	{0x328e, 0xf0},
	{0x328f, 0x0c},
	{0x3290, 0x0f},
	{0x3291, 0xdc},
	{0x3292, 0x7b},
	{0x3293, 0x00},
	{0x3294, 0x0a},
	{0x3295, 0x94},
	{0x3296, 0x00},
	{0x3297, 0x0b},
	{0x3298, 0x9a},
	{0x3299, 0x0f},
	{0x329a, 0xe0},
	{0x329b, 0x23},
	{0x329c, 0x0f},
	{0x329d, 0xfc},
	{0x329e, 0x51},
	{0x329f, 0x00},
	{0x32a0, 0x2e},
	{0x32a1, 0x03},
	{0x32a2, 0x00},
	{0x32a3, 0x38},
	{0x32a4, 0x37},
	{0x32a5, 0x0f},
	{0x32a6, 0xf8},
	{0x32a7, 0x46},
	{0x32a8, 0x00},
	{0x32a9, 0x03},
	{0x32aa, 0x5b},
	{0x32ab, 0x00},
	{0x32ac, 0x12},
	{0x32ad, 0xa4},
	{0x32ae, 0x00},
	{0x32af, 0x0a},
	{0x32b0, 0xe8},
	{0x32b1, 0x0f},
	{0x32b2, 0xc1},
	{0x32b3, 0x4d},
	{0x32b4, 0x0f},
	{0x32b5, 0xf0},
	{0x32b6, 0x45},
	{0x32b7, 0x00},
	{0x32b8, 0x04},
	{0x32b9, 0xe1},
	{0x32ba, 0x0f},
	{0x32bb, 0xe0},
	{0x32bc, 0xa6},
	{0x32bd, 0x00},
	{0x32be, 0x0b},
	{0x32bf, 0x33},
	{0x32c0, 0x00},
	{0x32c1, 0x02},
	{0x32c2, 0x63},
	{0x32c3, 0x00},
	{0x32c4, 0x20},
	{0x32c5, 0x67},
	{0x32c6, 0x0f},
	{0x32c7, 0xea},
	{0x32c8, 0x0d},
	{0x32c9, 0x00},
	{0x32ca, 0x02},
	{0x32cb, 0xf9},
	{0x32cc, 0x00},
	{0x32cd, 0x1c},
	{0x32ce, 0x1e},
	{0x32cf, 0x0f},
	{0x32d0, 0xee},
	{0x32d1, 0xad},
	{0x32d2, 0x0f},
	{0x32d3, 0xec},
	{0x32d4, 0xf2},
	{0x32d5, 0x00},
	{0x32d6, 0x0a},
	{0x32d7, 0x0b},
	{0x32d8, 0x00},
	{0x32d9, 0x63},
	{0x32da, 0x64},
	{0x32db, 0x0f},
	{0x32dc, 0xf4},
	{0x32dd, 0x9d},
	{0x32de, 0x00},
	{0x32df, 0x01},
	{0x32e0, 0xe5},
	{0x32e1, 0x00},
	{0x32e2, 0x07},
	{0x32e3, 0x69},
	{0x32e4, 0x0f},
	{0x32e5, 0xfa},
	{0x32e6, 0xd4},
	{0x32e7, 0x00},
	{0x32e8, 0x02},
	{0x32e9, 0x3f},
	{0x32ea, 0x0f},
	{0x32eb, 0xfe},
	{0x32ec, 0x5d},
	{0x32ed, 0x0f},
	{0x32ee, 0xf5},
	{0x32ef, 0x85},
	{0x32f0, 0x0f},
	{0x32f1, 0xff},
	{0x32f2, 0x56},
	{0x32f3, 0x00},
	{0x32f4, 0x0a},
	{0x32f5, 0x8b},
	{0x32f6, 0x0f},
	{0x32f7, 0xff},
	{0x32f8, 0xb2},
	{0x32f9, 0x0f},
	{0x32fa, 0xf4},
	{0x32fb, 0x33},
	{0x32fc, 0x0f},
	{0x32fd, 0xea},
	{0x32fe, 0xbf},
	{0x32ff, 0x00},
	{0x3300, 0x10},
	{0x3301, 0xfc},
	{0x3302, 0x0f},
	{0x3303, 0xfd},
	{0x3304, 0x4a},
	{0x3305, 0x0f},
	{0x3306, 0xf1},
	{0x3307, 0x4b},
	{0x3308, 0x0f},
	{0x3309, 0xfb},
	{0x330a, 0x1b},
	{0x330b, 0x00},
	{0x330c, 0x1f},
	{0x330d, 0x0c},
	{0x330e, 0x00},
	{0x330f, 0x20},
	{0x3310, 0x11},
	{0x3311, 0x0f},
	{0x3312, 0xee},
	{0x3313, 0x89},
	{0x3314, 0x00},
	{0x3315, 0x0b},
	{0x3316, 0x17},
	{0x3317, 0x00},
	{0x3318, 0x10},
	{0x3319, 0x16},
	{0x331a, 0x0f},
	{0x331b, 0xfa},
	{0x331c, 0x80},
	{0x331d, 0x0f},
	{0x331e, 0xde},
	{0x331f, 0xee},
	{0x3320, 0x0f},
	{0x3321, 0xfd},
	{0x3322, 0x71},
	{0x3323, 0x00},
	{0x3324, 0x03},
	{0x3325, 0xce},
	{0x3326, 0x0f},
	{0x3327, 0xea},
	{0x3328, 0xe4},
	{0x3329, 0x0f},
	{0x332a, 0xfd},
	{0x332b, 0x25},
	{0x332c, 0x00},
	{0x332d, 0x12},
	{0x332e, 0x8d},
	{0x332f, 0x00},
	{0x3330, 0x0c},
	{0x3331, 0xb2},
	{0x3332, 0x0f},
	{0x3333, 0xea},
	{0x3334, 0x9f},
	{0x3335, 0x00},
	{0x3336, 0x0a},
	{0x3337, 0xad},
	{0x3338, 0x00},
	{0x3339, 0x0d},
	{0x333a, 0xa9},
	{0x333b, 0x0f},
	{0x333c, 0xfa},
	{0x333d, 0x4a},
	{0x333e, 0x0f},
	{0x333f, 0xef},
	{0x3340, 0x00},
	{0x3341, 0x00},
	{0x3342, 0x05},
	{0x3343, 0xf9},
	{0x3344, 0x00},
	{0x3345, 0x72},
	{0x3346, 0x14},
	{0x3347, 0x0f},
	{0x3348, 0xf0},
	{0x3349, 0x88},
	{0x334a, 0x00},
	{0x334b, 0x07},
	{0x334c, 0x96},
	{0x334d, 0x00},
	{0x334e, 0x01},
	{0x334f, 0x8e},
	{0x3350, 0x00},
	{0x3351, 0x00},
	{0x3352, 0xc6},
	{0x3353, 0x0f},
	{0x3354, 0xfe},
	{0x3355, 0x9a},
	{0x3356, 0x0f},
	{0x3357, 0xf5},
	{0x3358, 0xc4},
	{0x3359, 0x0f},
	{0x335a, 0xf5},
	{0x335b, 0xf9},
	{0x335c, 0x00},
	{0x335d, 0x00},
	{0x335e, 0xb6},
	{0x335f, 0x00},
	{0x3360, 0x08},
	{0x3361, 0xdb},
	{0x3362, 0x0f},
	{0x3363, 0xfd},
	{0x3364, 0x52},
	{0x3365, 0x0f},
	{0x3366, 0xf9},
	{0x3367, 0x28},
	{0x3368, 0x0f},
	{0x3369, 0xed},
	{0x336a, 0xdd},
	{0x336b, 0x00},
	{0x336c, 0x0c},
	{0x336d, 0x59},
	{0x336e, 0x0f},
	{0x336f, 0xfa},
	{0x3370, 0x1a},
	{0x3371, 0x0f},
	{0x3372, 0xf9},
	{0x3373, 0x3e},
	{0x3374, 0x0f},
	{0x3375, 0xfc},
	{0x3376, 0xf1},
	{0x3377, 0x00},
	{0x3378, 0x14},
	{0x3379, 0x77},
	{0x337a, 0x00},
	{0x337b, 0x24},
	{0x337c, 0x02},
	{0x337d, 0x0f},
	{0x337e, 0xf5},
	{0x337f, 0x9d},
	{0x3380, 0x00},
	{0x3381, 0x0f},
	{0x3382, 0x9c},
	{0x3383, 0x00},
	{0x3384, 0x00},
	{0x3385, 0x06},
	{0x3386, 0x0f},
	{0x3387, 0xfe},
	{0x3388, 0x10},
	{0x3389, 0x0f},
	{0x338a, 0xec},
	{0x338b, 0x76},
	{0x338c, 0x0f},
	{0x338d, 0xf6},
	{0x338e, 0xd5},
	{0x338f, 0x0f},
	{0x3390, 0xfe},
	{0x3391, 0xba},
	{0x3392, 0x0f},
	{0x3393, 0xeb},
	{0x3394, 0x4f},
	{0x3395, 0x00},
	{0x3396, 0x0a},
	{0x3397, 0x3f},
	{0x3398, 0x00},
	{0x3399, 0x0a},
	{0x339a, 0x13},
	{0x339b, 0x00},
	{0x339c, 0x04},
	{0x339d, 0x4b},
	{0x339e, 0x0f},
	{0x339f, 0xef},
	{0x33a0, 0x1c},
	{0x33a1, 0x00},
	{0x33a2, 0x0c},
	{0x33a3, 0xf1},
	{0x33a4, 0x00},
	{0x33a5, 0x06},
	{0x33a6, 0xb5},
	{0x33a7, 0x0f},
	{0x33a8, 0xfa},
	{0x33a9, 0xe1},
	{0x33aa, 0x0f},
	{0x33ab, 0xf3},
	{0x33ac, 0x1b},
	{0x33ad, 0x00},
	{0x33ae, 0x06},
	{0x33af, 0xf4},
	
	{0x3096, 0x60},
	{0x3096, 0x40},
	
	{0x0101, 0x03}, //mirror and flip
};

static struct v4l2_subdev_info s5k4e1_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array s5k4e1_init_conf[] = {
	{&s5k4e1_recommend_settings_1[0],
	ARRAY_SIZE(s5k4e1_recommend_settings_1), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&s5k4e1_recommend_settings_2[0],
	ARRAY_SIZE(s5k4e1_recommend_settings_2), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&s5k4e1_recommend_settings_3[0],
	ARRAY_SIZE(s5k4e1_recommend_settings_3), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&s5k4e1_recommend_settings_4[0],
	ARRAY_SIZE(s5k4e1_recommend_settings_4), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_i2c_conf_array s5k4e1_confs[] = {
	{&s5k4e1_snap_settings[0],
	ARRAY_SIZE(s5k4e1_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&s5k4e1_prev_settings[0],
	ARRAY_SIZE(s5k4e1_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t s5k4e1_dimensions[] = {
	{
		.x_output = 0xA30,
		.y_output = 0x7A8,
		.line_length_pclk = 0xAB2,
		.frame_length_lines = 0x7B4,
		.vt_pixel_clk = 81600000,
		.op_pixel_clk = 81600000,
		.binning_factor = 0,
	},
	{
		.x_output = 0x518,
		.y_output = 0x3D4,
		.line_length_pclk = 0xAB2,
		.frame_length_lines = 0x3E0,
		.vt_pixel_clk = 81600000,
		.op_pixel_clk = 81600000,
		.binning_factor = 1,
	},
};

static struct msm_camera_csi_params s5k4e1_csi_params = {
	.data_format = CSI_10BIT,
	/*we use 2 lane for s5k4e1*/
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 24,
};

static struct msm_camera_csi_params *s5k4e1_csi_params_array[] = {
	&s5k4e1_csi_params,
	&s5k4e1_csi_params,
};

static struct msm_sensor_output_reg_addr_t s5k4e1_reg_addr = {
	.x_output = 0x034C,
	.y_output = 0x034E,
	.line_length_pclk = 0x0342,
	.frame_length_lines = 0x0340,
};

static struct msm_sensor_id_info_t s5k4e1_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x4E10,
};

static struct msm_sensor_exp_gain_info_t s5k4e1_exp_gain_info = {
	.coarse_int_time_addr = 0x0202,
	.global_gain_addr = 0x0204,
	.vert_offset = 4,
};

static inline uint8_t s5k4e1_byte(uint16_t word, uint8_t offset)
{
	return word >> (offset * BITS_PER_BYTE);
}
#define LINE_NUM_30FPS 992
static uint16_t cur_min_frame_lines = LINE_NUM_30FPS;
/*use the set fps function instead of defaut function*/
static int32_t s5k4e1_set_fps(struct msm_sensor_ctrl_t *s_ctrl,struct fps_cfg *fps)
{
	uint16_t total_lines_per_frame = 0;
	int32_t rc = 0;
	enum msm_sensor_resolution_t res = 0;
	if(fps->fps_div == s_ctrl->fps_divider)
	{
		return rc;
	}
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
		((s_ctrl->curr_frame_length_lines * s_ctrl->fps_divider) / 0x400);

	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			s5k4e1_byte(total_lines_per_frame, MSB),
			MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
			s5k4e1_byte(total_lines_per_frame, LSB),
			MSM_CAMERA_I2C_BYTE_DATA);

	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);

	printk("%s: res = %d, curr_frame_length_lines = %d, total_lines_per_frame = %d\n",__func__,res,s_ctrl->curr_frame_length_lines,total_lines_per_frame);

	s_ctrl->curr_frame_length_lines = total_lines_per_frame;
	/*only use in preview mode*/
	if(res == MSM_SENSOR_RES_QTR)
		cur_min_frame_lines = s_ctrl->curr_frame_length_lines;
	return rc;
}
static int32_t s5k4e1_write_prev_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
						uint16_t gain, uint32_t line)
{
	uint16_t max_legal_gain = 0x0200;
	int32_t rc = 0;
	uint32_t fl_lines, offset;
	fl_lines = s_ctrl->curr_frame_length_lines;
	pr_info("s5k4e1_write_prev_exp_gain :%d %d\n", gain, line);
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;
	fl_lines = (fl_lines * s_ctrl->fps_divider) / Q10;
	if (gain > max_legal_gain) {
		CDBG("Max legal gain Line:%d\n", __LINE__);
		gain = max_legal_gain;
	}

	/* Analogue Gain */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr,
		s5k4e1_byte(gain, MSB),
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr + 1,
		s5k4e1_byte(gain, LSB),
		MSM_CAMERA_I2C_BYTE_DATA);
	/*if Line is smaller than 1985, then the fps will be larger than 15fps*/
	if(s_ctrl->curr_frame_length_lines < cur_min_frame_lines)
	{
		s_ctrl->curr_frame_length_lines = cur_min_frame_lines;
	}
	/*modify for unnormal preview*/
	if (line > (s_ctrl->curr_frame_length_lines - offset)) {
		fl_lines = line ; //+ offset;
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			s5k4e1_byte(fl_lines, MSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
			s5k4e1_byte(fl_lines, LSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		/* Coarse Integration Time */
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
			s5k4e1_byte(line, MSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
			s5k4e1_byte(line, LSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	} else if (line < (s_ctrl->curr_frame_length_lines - offset)) {
		fl_lines = line ;//+ offset;
		if (fl_lines < s_ctrl->curr_frame_length_lines)
			fl_lines = s_ctrl->curr_frame_length_lines;

		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
		/* Coarse Integration Time */
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
			s5k4e1_byte(line, MSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
			s5k4e1_byte(line, LSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			s5k4e1_byte(fl_lines, MSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
			s5k4e1_byte(fl_lines, LSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	} else {
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			s5k4e1_byte(fl_lines, MSB),
			MSM_CAMERA_I2C_BYTE_DATA);
			msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
			s5k4e1_byte(fl_lines, LSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		/* Coarse Integration Time */
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
			s5k4e1_byte(line, MSB),
			MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
			s5k4e1_byte(line, LSB),
			MSM_CAMERA_I2C_BYTE_DATA);
			s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	}
	if(is_first_preview_frame)
	{
		msleep(50);
		is_first_preview_frame = 0;
	}
	return rc;
}

static int32_t s5k4e1_write_pict_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	uint16_t max_legal_gain = 0x0200;
	uint32_t ll_pck, fl_lines;
	uint8_t gain_msb, gain_lsb;
	uint8_t intg_time_msb, intg_time_lsb;
	uint8_t ll_pck_msb, ll_pck_lsb;

	if (gain > max_legal_gain) {
		CDBG("Max legal gain Line:%d\n", __LINE__);
		gain = max_legal_gain;
	}

	fl_lines = s_ctrl->curr_frame_length_lines * s_ctrl->fps_divider / Q10;
	ll_pck = s_ctrl->curr_line_length_pclk;
	CDBG("s5k4e1_write_exp_gain : gain = %d line = %d  fl_lines = %d, ll_pck = %d\n", gain, line, fl_lines, ll_pck);

	if (line > (fl_lines -12))
	{
		fl_lines = line +12;
	}

	gain_msb = (uint8_t) ((gain & 0xFF00) >> 8);
	gain_lsb = (uint8_t) (gain & 0x00FF);

	intg_time_msb = (uint8_t) ((line & 0xFF00) >> 8);
	intg_time_lsb = (uint8_t) (line & 0x00FF);

	ll_pck_msb = (uint8_t) ((fl_lines & 0xFF00) >> 8);
	ll_pck_lsb = (uint8_t) (fl_lines & 0x00FF);

	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr,
		gain_msb,
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr + 1,
		gain_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines,
		ll_pck_msb,
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
		ll_pck_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	/* Coarse Integration Time */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		intg_time_msb,
		MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
		intg_time_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);

	return 0;
}

int32_t s5k4e1_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	struct msm_camera_sensor_info *s_info;
	struct msm_sensor_ctrl_t *s_ctrl;

	/* sensor_i2c_addr maybe different for different module*/
	/* set sensor_i2c_addr to corresponding value to probe the same sensor repeatedly*/
	s_ctrl = (struct msm_sensor_ctrl_t *)(id->driver_data);
	s_ctrl->sensor_i2c_addr=0x6E;
	if((rc = msm_sensor_i2c_probe(client, id)) < 0)
	{
		s_ctrl->sensor_i2c_addr=0x20;
		if((rc = msm_sensor_i2c_probe(client, id)) < 0)
		{
			return rc;
		}
	}

	s_info = client->dev.platform_data;
	if (s_info == NULL) {
		pr_err("%s %s NULL sensor data\n", __func__, client->name);
		return -EFAULT;
	}

	if (s_info->actuator_info)
	{
		if(s_info->actuator_info->vcm_enable)
		{
		rc = gpio_request(s_info->actuator_info->vcm_pwd,
				"msm_actuator");
		if (rc < 0)
			pr_err("%s: gpio_request:msm_actuator %d failed\n",
				__func__, s_info->actuator_info->vcm_pwd);
		rc = gpio_direction_output(s_info->actuator_info->vcm_pwd, 0);
		if (rc < 0)
			pr_err("%s: gpio:msm_actuator %d direction can't be set\n",
				__func__, s_info->actuator_info->vcm_pwd);
		gpio_free(s_info->actuator_info->vcm_pwd);
	}
	}

	return rc;
}

static const struct i2c_device_id s5k4e1_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&s5k4e1_s_ctrl},
	{ }
};

static struct i2c_driver s5k4e1_i2c_driver = {
	.id_table = s5k4e1_i2c_id,
	.probe  = s5k4e1_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client s5k4e1_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
	.addr_pos = 0,
	.addr_dir = 0,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&s5k4e1_i2c_driver);
}

static struct v4l2_subdev_core_ops s5k4e1_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops s5k4e1_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops s5k4e1_subdev_ops = {
	.core = &s5k4e1_subdev_core_ops,
	.video  = &s5k4e1_subdev_video_ops,
};

int32_t s5k4e1_mirrorandflip_self_adapt(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	CDBG("%s is called !\n", __func__);
	rc = msm_camera_i2c_write(
		s_ctrl->sensor_i2c_client,
		0x0101, 0x00,
		MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) 
{
		pr_err("%s: write register error\n", __func__);
	}
	
	return rc;
}

static int32_t s5k4e1_sensor_model_match(struct msm_sensor_ctrl_t *s_ctrl)
{
	/*the sensor_i2c_addr is the same for liteon and  samsuny,different for others*/
	/*so camera_id is used to distinguish liteon and samsuny and sensor_i2c_addr is used to distinguish others */
	switch(s_ctrl->sensor_i2c_addr)
	{
		case 0x6E:
		{
	if(!gpio_request(S5k4E1_CAMERA_ID_GPIO,  "s5k4e1"))
	{
		/* if the moudle is liteon, the value is 0, if the moudle is samsuny,the value is 1 */
				if(1 == gpio_get_value(S5k4E1_CAMERA_ID_GPIO))
		{
					if(check_product_y300_for_camera())
					{
						strncpy((char *)s_ctrl->sensor_name, "23060110FA-SAM-3-Y300", sizeof("23060110FA-SAM-3-Y300"));
					}
					else
					{
						strncpy((char *)s_ctrl->sensor_name, "23060110FA-SAM-3", sizeof("23060110FA-SAM-3"));
					}
		}
		else
		{
					strncpy((char *)s_ctrl->sensor_name, "23060069FA-SAM-L", sizeof("23060069FA-SAM-L"));
		}
		gpio_free(S5k4E1_CAMERA_ID_GPIO);
	}
	else
	{
		printk("%s: gpio request fail\n",__func__);
				strncpy((char *)s_ctrl->sensor_name, "23060069FA-SAM-L", sizeof("23060069FA-SAM-L"));
	}

			break;
    }
		case 0x20:
    {
			strncpy((char *)s_ctrl->sensor_name, "23060084FF-SAM-F", sizeof("23060084FF-SAM-F"));
			/*the image rotate 180 for foxcom module*/
			/*foxcom module need write 0x0101,liteon and samsuny  module not */
			s_ctrl->func_tbl->sensor_mirrorandflip_self_adapt=s5k4e1_mirrorandflip_self_adapt;
			/* the Foxcom module is FF*/
			s_ctrl->sensordata->actuator_info = NULL;
			break;
		}
		default:
		break;
    }
	
	return 0;
}
static struct msm_sensor_fn_t s5k4e1_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = s5k4e1_set_fps,//msm_sensor_set_fps,
	.sensor_write_exp_gain = s5k4e1_write_prev_exp_gain,
	.sensor_write_snapshot_exp_gain = s5k4e1_write_pict_exp_gain,
	.sensor_csi_setting = msm_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
	.sensor_model_match = s5k4e1_sensor_model_match,
};

static struct msm_sensor_reg_t s5k4e1_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = s5k4e1_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(s5k4e1_start_settings),
	.stop_stream_conf = s5k4e1_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(s5k4e1_stop_settings),
	.group_hold_on_conf = s5k4e1_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(s5k4e1_groupon_settings),
	.group_hold_off_conf = s5k4e1_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(s5k4e1_groupoff_settings),
	.init_settings = &s5k4e1_init_conf[0],
	.init_size = ARRAY_SIZE(s5k4e1_init_conf),
	.mode_settings = &s5k4e1_confs[0],
	.output_settings = &s5k4e1_dimensions[0],
	.num_conf = ARRAY_SIZE(s5k4e1_confs),
};

static struct msm_sensor_ctrl_t s5k4e1_s_ctrl = {
	.msm_sensor_reg = &s5k4e1_regs,
	.sensor_i2c_client = &s5k4e1_sensor_i2c_client,
	.sensor_i2c_addr = 0x6E,
	.sensor_output_reg_addr = &s5k4e1_reg_addr,
	.sensor_id_info = &s5k4e1_id_info,
	.sensor_exp_gain_info = &s5k4e1_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &s5k4e1_csi_params_array[0],
	.msm_sensor_mutex = &s5k4e1_mut,
	.sensor_i2c_driver = &s5k4e1_i2c_driver,
	.sensor_v4l2_subdev_info = s5k4e1_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(s5k4e1_subdev_info),
	.sensor_v4l2_subdev_ops = &s5k4e1_subdev_ops,
	.func_tbl = &s5k4e1_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
	.sensor_name = "23060069FA-SAM-L",
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Samsung 5MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");

