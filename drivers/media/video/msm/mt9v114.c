
/*
 * Copyright (c) 2008-2009 QUALCOMM USA, INC.
 *
 * All source code in this file is licensed under the following license
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can find it at http://www.fsf.org
 */

#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/camera.h>
#include <linux/slab.h>
#include "mt9v114.h"

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
 #include <linux/hw_dev_dec.h>
#endif

#undef CDBG
#define CDBG(fmt, args...) printk(KERN_INFO "mt9v114_sunny.c: " fmt, ## args)

/*=============================================================
    SENSOR REGISTER DEFINES
==============================================================*/
#define MT9V114_REG_MODEL_ID 0x0000
#define MT9V114_MODEL_ID 0x2283
#define MT9V114_REG_RESET_REGISTER 0x001A

/* Add effect status flag for setEffect */
static int current_effect  = CAMERA_EFFECT_OFF;
enum mt9v114_test_mode_t
{
    TEST_OFF,
    TEST_1,
    TEST_2,
    TEST_3
};

enum mt9v114_resolution_t
{
    QTR_SIZE,
    FULL_SIZE,
    INVALID_SIZE
};


enum mt9v114_reg_update_t
{
    /* Sensor egisters that need to be updated during initialization */
    REG_INIT,

    /* Sensor egisters that needs periodic I2C writes */
    UPDATE_PERIODIC,

    /* All the sensor Registers will be updated */
    UPDATE_ALL,

    /* Not valid update */
    UPDATE_INVALID
};

enum mt9v114_setting_t
{
    RES_PREVIEW,
    RES_CAPTURE
};

/* actuator's Slave Address */
#define MT9V114_AF_I2C_ADDR 0x7A

/*
 * AF Total steps parameters
 */
#define MT9V114_TOTAL_STEPS_NEAR_TO_FAR 30 /* 28 */

/*
 * Time in milisecs for waiting for the sensor to reset.
 */
#define MT9V114_RESET_DELAY_MSECS 66

/* for 30 fps preview */
#define MT9V114_DEFAULT_CLOCK_RATE 12000000

/* FIXME: Changes from here */
struct mt9v114_work_t
{
    struct work_struct work;
};

struct mt9v114_ctrl_t
{
    const struct  msm_camera_sensor_info *sensordata;

    int      sensormode;
    uint32_t fps_divider; /* init to 1 * 0x00000400 */
    uint32_t pict_fps_divider; /* init to 1 * 0x00000400 */

    uint16_t curr_lens_pos;
    uint16_t init_curr_lens_pos;
    uint16_t my_reg_gain;
    uint32_t my_reg_line_count;

    enum mt9v114_resolution_t prev_res;
    enum mt9v114_resolution_t pict_res;
    enum mt9v114_resolution_t curr_res;
    enum mt9v114_test_mode_t  set_test;

    unsigned short imgaddr;
};

typedef enum
{
    E_REGISTER_END,
    E_REGISTER_CMD_8BIT,
    E_REGISTER_CMD_16BIT,
    E_REGISTER_WAIT,
    E_REGISTER_MAX,
} e_cmd_type;

struct mt9v114_i2c_reg_conf
{
    e_cmd_type     type;
    unsigned short reg;
    unsigned short value;
};

static struct mt9v114_i2c_reg_conf mt9v114_init_reg_config[] =
{
    // Step 1  Reset
    //{E_REGISTER_CMD_16BIT, 0x001A, 0x0306},	// RESET_AND_MISC_CONTROL
    //{E_REGISTER_CMD_16BIT, 0x001A, 0x0124},	// RESET_AND_MISC_CONTROL

    //PLL setting
    // 12MHz  12MHz    Frame rate_Max = 15.8fps
    {E_REGISTER_CMD_16BIT, 0x0010, 0x0010}, //pll_dividers = 16
    {E_REGISTER_CMD_16BIT, 0x0012, 0x0700}, //pll_p_dividers = 1792
    {E_REGISTER_CMD_16BIT, 0x001E, 0x0707}, // PAD_SLEW
    {E_REGISTER_CMD_16BIT, 0x0018, 0x0006}, // STANDBY_CONTROL_AND_STATUS
    //  POLL  STANDBY_CONTROL_AND_STATUS::FW_IN_STANDBY =>  0x00

    {E_REGISTER_WAIT, 0x0000, 100}, //Delay =100       

    // Patch
    // patch driver table
    {E_REGISTER_CMD_16BIT, 0x098A, 0x0000}, // PHYSICAL_ADDRESS_ACCESS
    {E_REGISTER_CMD_16BIT, 0x8082, 0x0194},
    {E_REGISTER_CMD_16BIT, 0x8084, 0x0163},
    {E_REGISTER_CMD_16BIT, 0x8086, 0x0107},
    {E_REGISTER_CMD_16BIT, 0x8088, 0x01C7},
    {E_REGISTER_CMD_16BIT, 0x808A, 0x01A1},
    {E_REGISTER_CMD_16BIT, 0x808C, 0x022A},
    {E_REGISTER_CMD_16BIT, 0x098E, 0x0000}, // LOGICAL_ADDRESS_ACCESS

    //patch SISR
    {E_REGISTER_CMD_16BIT, 0x0982, 0x0000}, // ACCESS_CTL_STAT
    {E_REGISTER_CMD_16BIT, 0x098A, 0x0000}, // PHYSICAL_ADDRESS_ACCESS
    {E_REGISTER_CMD_16BIT, 0x8098, 0x3C3C},
    {E_REGISTER_CMD_16BIT, 0x809A, 0x1300},
    {E_REGISTER_CMD_16BIT, 0x809C, 0x0147},
    {E_REGISTER_CMD_16BIT, 0x809E, 0xCC31},
    {E_REGISTER_CMD_16BIT, 0x80A0, 0x8230},
    {E_REGISTER_CMD_16BIT, 0x80A2, 0xED02},
    {E_REGISTER_CMD_16BIT, 0x80A4, 0xCC00},
    {E_REGISTER_CMD_16BIT, 0x80A6, 0x90ED},
    {E_REGISTER_CMD_16BIT, 0x80A8, 0x00C6},
    {E_REGISTER_CMD_16BIT, 0x80AA, 0x04BD},
    {E_REGISTER_CMD_16BIT, 0x80AC, 0xDDBD},
    {E_REGISTER_CMD_16BIT, 0x80AE, 0x5FD7},
    {E_REGISTER_CMD_16BIT, 0x80B0, 0x8E86},
    {E_REGISTER_CMD_16BIT, 0x80B2, 0x90BD},
    {E_REGISTER_CMD_16BIT, 0x80B4, 0x0330},
    {E_REGISTER_CMD_16BIT, 0x80B6, 0xDD90},
    {E_REGISTER_CMD_16BIT, 0x80B8, 0xCC92},
    {E_REGISTER_CMD_16BIT, 0x80BA, 0x02BD},
    {E_REGISTER_CMD_16BIT, 0x80BC, 0x0330},
    {E_REGISTER_CMD_16BIT, 0x80BE, 0xDD92},
    {E_REGISTER_CMD_16BIT, 0x80C0, 0xCC94},
    {E_REGISTER_CMD_16BIT, 0x80C2, 0x04BD},
    {E_REGISTER_CMD_16BIT, 0x80C4, 0x0330},
    {E_REGISTER_CMD_16BIT, 0x80C6, 0xDD94},
    {E_REGISTER_CMD_16BIT, 0x80C8, 0xCC96},
    {E_REGISTER_CMD_16BIT, 0x80CA, 0x00BD},
    {E_REGISTER_CMD_16BIT, 0x80CC, 0x0330},
    {E_REGISTER_CMD_16BIT, 0x80CE, 0xDD96},
    {E_REGISTER_CMD_16BIT, 0x80D0, 0xCC07},
    {E_REGISTER_CMD_16BIT, 0x80D2, 0xFFDD},
    {E_REGISTER_CMD_16BIT, 0x80D4, 0x8ECC},
    {E_REGISTER_CMD_16BIT, 0x80D6, 0x3180},
    {E_REGISTER_CMD_16BIT, 0x80D8, 0x30ED},
    {E_REGISTER_CMD_16BIT, 0x80DA, 0x02CC},
    {E_REGISTER_CMD_16BIT, 0x80DC, 0x008E},
    {E_REGISTER_CMD_16BIT, 0x80DE, 0xED00},
    {E_REGISTER_CMD_16BIT, 0x80E0, 0xC605},
    {E_REGISTER_CMD_16BIT, 0x80E2, 0xBDDE},
    {E_REGISTER_CMD_16BIT, 0x80E4, 0x13BD},
    {E_REGISTER_CMD_16BIT, 0x80E6, 0x03FA},
    {E_REGISTER_CMD_16BIT, 0x80E8, 0x3838},
    {E_REGISTER_CMD_16BIT, 0x80EA, 0x3913},
    {E_REGISTER_CMD_16BIT, 0x80EC, 0x0001},
    {E_REGISTER_CMD_16BIT, 0x80EE, 0x0109},
    {E_REGISTER_CMD_16BIT, 0x80F0, 0xBC01},
    {E_REGISTER_CMD_16BIT, 0x80F2, 0x9D26},
    {E_REGISTER_CMD_16BIT, 0x80F4, 0x0813},
    {E_REGISTER_CMD_16BIT, 0x80F6, 0x0004},
    {E_REGISTER_CMD_16BIT, 0x80F8, 0x0108},
    {E_REGISTER_CMD_16BIT, 0x80FA, 0xFF02},
    {E_REGISTER_CMD_16BIT, 0x80FC, 0xEF7E},
    {E_REGISTER_CMD_16BIT, 0x80FE, 0xC278},
    {E_REGISTER_CMD_16BIT, 0x8330, 0x364F},
    {E_REGISTER_CMD_16BIT, 0x8332, 0x36CE},
    {E_REGISTER_CMD_16BIT, 0x8334, 0x02F3},
    {E_REGISTER_CMD_16BIT, 0x8336, 0x3AEC},
    {E_REGISTER_CMD_16BIT, 0x8338, 0x00CE},
    {E_REGISTER_CMD_16BIT, 0x833A, 0x0018},
    {E_REGISTER_CMD_16BIT, 0x833C, 0xBDE4},
    {E_REGISTER_CMD_16BIT, 0x833E, 0x5197},
    {E_REGISTER_CMD_16BIT, 0x8340, 0x8F38},
    {E_REGISTER_CMD_16BIT, 0x8342, 0xEC00},
    {E_REGISTER_CMD_16BIT, 0x8344, 0x938E},
    {E_REGISTER_CMD_16BIT, 0x8346, 0x2C02},
    {E_REGISTER_CMD_16BIT, 0x8348, 0x4F5F},
    {E_REGISTER_CMD_16BIT, 0x834A, 0x3900},
    {E_REGISTER_CMD_16BIT, 0x83E4, 0x3C13},
    {E_REGISTER_CMD_16BIT, 0x83E6, 0x0001},
    {E_REGISTER_CMD_16BIT, 0x83E8, 0x0CCC},
    {E_REGISTER_CMD_16BIT, 0x83EA, 0x3180},
    {E_REGISTER_CMD_16BIT, 0x83EC, 0x30ED},
    {E_REGISTER_CMD_16BIT, 0x83EE, 0x00CC},
    {E_REGISTER_CMD_16BIT, 0x83F0, 0x87FF},
    {E_REGISTER_CMD_16BIT, 0x83F2, 0xBDDD},
    {E_REGISTER_CMD_16BIT, 0x83F4, 0xF5BD},
    {E_REGISTER_CMD_16BIT, 0x83F6, 0xC2A9},
    {E_REGISTER_CMD_16BIT, 0x83F8, 0x3839},
    {E_REGISTER_CMD_16BIT, 0x83FA, 0xFE02},
    {E_REGISTER_CMD_16BIT, 0x83FC, 0xEF7E},
    {E_REGISTER_CMD_16BIT, 0x83FE, 0x00EB},
    {E_REGISTER_CMD_16BIT, 0x098E, 0x0000}, // LOGICAL_ADDRESS_ACCESS

    //SISR patch enable
    {E_REGISTER_CMD_16BIT, 0x098A, 0x0000}, // PHYSICAL_ADDRESS_ACCESS
    {E_REGISTER_CMD_16BIT, 0x83E0, 0x0098},
    {E_REGISTER_CMD_16BIT, 0x83E2, 0x03E4},
    {E_REGISTER_CMD_16BIT, 0x098E, 0x0000}, // LOGICAL_ADDRESS_ACCESS

    // Timing setting
    //REG = 0x98E, 0x1000
    {E_REGISTER_CMD_16BIT, 0x300A, 0x020B}, //frame_length_lines = 523
    {E_REGISTER_CMD_16BIT, 0x300C, 0x02D6}, //line_length_pck = 726
    {E_REGISTER_CMD_16BIT, 0x3010, 0x0012}, //fine_correction = 18
    {E_REGISTER_CMD_8BIT,  0x9803, 0x04  }, //stat_fd_zone_height = 4
    {E_REGISTER_CMD_16BIT, 0xA06E, 0x0050}, //cam_fd_config_fdperiod_50hz = 80
    {E_REGISTER_CMD_16BIT, 0xA070, 0x0043}, //cam_fd_config_fdperiod_60hz = 67
    {E_REGISTER_CMD_8BIT,  0xA072, 0x0F  }, //cam_fd_config_search_f1_50 = 15
    {E_REGISTER_CMD_8BIT,  0xA073, 0x11  }, //cam_fd_config_search_f2_50 = 17
    {E_REGISTER_CMD_8BIT,  0xA074, 0x13  }, //cam_fd_config_search_f1_60 = 19
    {E_REGISTER_CMD_8BIT,  0xA075, 0x15  }, //cam_fd_config_search_f2_60 = 21
    {E_REGISTER_CMD_16BIT, 0xA076, 0x0006}, //cam_fd_config_max_fdzone_50hz = 6
    {E_REGISTER_CMD_16BIT, 0xA078, 0x0007}, //cam_fd_config_max_fdzone_60hz = 7
    {E_REGISTER_CMD_16BIT, 0xA01A, 0x0006}, //cam_ae_config_target_fdzone = 6

    // Chart settings

    {E_REGISTER_CMD_16BIT, 0x3E22, 0x3307}, // SAMP_BOOST_ROW
    {E_REGISTER_CMD_16BIT, 0x3ECE, 0x4311}, // DAC_LD_2_3
    {E_REGISTER_CMD_16BIT, 0x3ED0, 0x16AF}, // DAC_LD_4_5

    // AWB-CCM
    {E_REGISTER_CMD_16BIT, 0xA02F, 0x042B}, // CAM_AWB_CONFIG_CCM_L_0
    {E_REGISTER_CMD_16BIT, 0xA045, 0x0111}, // CAM_AWB_CONFIG_CCM_RL_0
    {E_REGISTER_CMD_16BIT, 0xA031, 0xFE56}, // CAM_AWB_CONFIG_CCM_L_1
    {E_REGISTER_CMD_16BIT, 0xA047, 0xFE94}, // CAM_AWB_CONFIG_CCM_RL_1
    {E_REGISTER_CMD_16BIT, 0xA033, 0xFEB7}, // CAM_AWB_CONFIG_CCM_L_2
    {E_REGISTER_CMD_16BIT, 0xA049, 0x0069}, // CAM_AWB_CONFIG_CCM_RL_2
    {E_REGISTER_CMD_16BIT, 0xA035, 0xFF70}, // CAM_AWB_CONFIG_CCM_L_3
    {E_REGISTER_CMD_16BIT, 0xA04B, 0xFFE0}, // CAM_AWB_CONFIG_CCM_RL_3
    {E_REGISTER_CMD_16BIT, 0xA037, 0x0123}, // CAM_AWB_CONFIG_CCM_L_4
    {E_REGISTER_CMD_16BIT, 0xA04D, 0x0072}, // CAM_AWB_CONFIG_CCM_RL_4
    {E_REGISTER_CMD_16BIT, 0xA039, 0x00A2}, // CAM_AWB_CONFIG_CCM_L_5
    {E_REGISTER_CMD_16BIT, 0xA04F, 0xFFBE}, // CAM_AWB_CONFIG_CCM_RL_5
    {E_REGISTER_CMD_16BIT, 0xA03B, 0xFE08}, // CAM_AWB_CONFIG_CCM_L_6
    {E_REGISTER_CMD_16BIT, 0xA051, 0x016B}, // CAM_AWB_CONFIG_CCM_RL_6
    {E_REGISTER_CMD_16BIT, 0xA03D, 0xFB70}, // CAM_AWB_CONFIG_CCM_L_7
    {E_REGISTER_CMD_16BIT, 0xA053, 0x01A3}, // CAM_AWB_CONFIG_CCM_RL_7
    {E_REGISTER_CMD_16BIT, 0xA03F, 0x07D1}, // CAM_AWB_CONFIG_CCM_L_8
    {E_REGISTER_CMD_16BIT, 0xA055, 0xFCF0}, // CAM_AWB_CONFIG_CCM_RL_8
    {E_REGISTER_CMD_16BIT, 0xA041, 0x0021}, // CAM_AWB_CONFIG_CCM_L_9
    {E_REGISTER_CMD_16BIT, 0xA043, 0x004A}, // CAM_AWB_CONFIG_CCM_L_10
    {E_REGISTER_CMD_16BIT, 0xA057, 0x0010}, // CAM_AWB_CONFIG_CCM_RL_9
    {E_REGISTER_CMD_16BIT, 0xA059, 0xFFDD}, // CAM_AWB_CONFIG_CCM_RL_10
    {E_REGISTER_CMD_16BIT, 0x940A, 0x0000}, // AWB_X_START
    {E_REGISTER_CMD_16BIT, 0x940C, 0x0000}, // AWB_Y_START
    {E_REGISTER_CMD_16BIT, 0x940E, 0x027F}, // AWB_X_END
    {E_REGISTER_CMD_16BIT, 0xA061, 0x002A}, // CAM_AWB_CONFIG_X_SHIFT_PRE_ADJ
    {E_REGISTER_CMD_16BIT, 0xA063, 0x0038}, // CAM_AWB_CONFIG_Y_SHIFT_PRE_ADJ
    {E_REGISTER_CMD_8BIT,  0xA065,   0x04}, // CAM_AWB_CONFIG_X_SCALE
    {E_REGISTER_CMD_8BIT,  0xA066,   0x02}, // CAM_AWB_CONFIG_Y_SCALE
    {E_REGISTER_CMD_8BIT,  0x9409,   0xF0}, // AWB_LUMA_THRESH_HIGH
    {E_REGISTER_CMD_8BIT,  0x9416,   0x2D}, // AWB_R_SCENE_RATIO_LOWER
    {E_REGISTER_CMD_8BIT,  0x9417,   0x8C}, // AWB_R_SCENE_RATIO_UPPER
    {E_REGISTER_CMD_8BIT,  0x9418,   0x16}, // AWB_B_SCENE_RATIO_LOWER
    {E_REGISTER_CMD_8BIT,  0x9419,   0x78}, // AWB_B_SCENE_RATIO_UPPER
    {E_REGISTER_CMD_16BIT, 0x2112, 0x0000}, // AWB_WEIGHT_R0
    {E_REGISTER_CMD_16BIT, 0x2114, 0x0000}, // AWB_WEIGHT_R1
    {E_REGISTER_CMD_16BIT, 0x2116, 0x0000}, // AWB_WEIGHT_R2
    {E_REGISTER_CMD_16BIT, 0x2118, 0x0F80}, // AWB_WEIGHT_R3
    {E_REGISTER_CMD_16BIT, 0x211A, 0x2A80}, // AWB_WEIGHT_R4
    {E_REGISTER_CMD_16BIT, 0x211C, 0x0B40}, // AWB_WEIGHT_R5
    {E_REGISTER_CMD_16BIT, 0x211E, 0x01AC}, // AWB_WEIGHT_R6
    {E_REGISTER_CMD_16BIT, 0x2120, 0x0038}, // AWB_WEIGHT_R7
    
    //LSC
    {E_REGISTER_CMD_16BIT, 0x3210, 0x00B0},
    {E_REGISTER_CMD_16BIT, 0x3640, 0x0190},
    {E_REGISTER_CMD_16BIT, 0x3642, 0x94CB},
    {E_REGISTER_CMD_16BIT, 0x3644, 0x51F0},
    {E_REGISTER_CMD_16BIT, 0x3646, 0x94CB},
    {E_REGISTER_CMD_16BIT, 0x3648, 0xB830},
    {E_REGISTER_CMD_16BIT, 0x364A, 0x0170},
    {E_REGISTER_CMD_16BIT, 0x364C, 0x802C},
    {E_REGISTER_CMD_16BIT, 0x364E, 0x68D0},
    {E_REGISTER_CMD_16BIT, 0x3650, 0x022B},
    {E_REGISTER_CMD_16BIT, 0x3652, 0x8450},
    {E_REGISTER_CMD_16BIT, 0x3654, 0x0210},
    {E_REGISTER_CMD_16BIT, 0x3656, 0xC52B},
    {E_REGISTER_CMD_16BIT, 0x3658, 0x4930},
    {E_REGISTER_CMD_16BIT, 0x365A, 0x366D},
    {E_REGISTER_CMD_16BIT, 0x365C, 0xA470},
    {E_REGISTER_CMD_16BIT, 0x365E, 0x0390},
    {E_REGISTER_CMD_16BIT, 0x3660, 0xDF6A},
    {E_REGISTER_CMD_16BIT, 0x3662, 0x5570},
    {E_REGISTER_CMD_16BIT, 0x3664, 0x820A},
    {E_REGISTER_CMD_16BIT, 0x3666, 0xBB10},
    {E_REGISTER_CMD_16BIT, 0x3680, 0x926D},
    {E_REGISTER_CMD_16BIT, 0x3682, 0x8E44},
    {E_REGISTER_CMD_16BIT, 0x3684, 0x436E},
    {E_REGISTER_CMD_16BIT, 0x3686, 0x62EC},
    {E_REGISTER_CMD_16BIT, 0x3688, 0x8D4F},
    {E_REGISTER_CMD_16BIT, 0x368A, 0xD4CD},
    {E_REGISTER_CMD_16BIT, 0x368C, 0xAF6B},
    {E_REGISTER_CMD_16BIT, 0x368E, 0x304E},
    {E_REGISTER_CMD_16BIT, 0x3690, 0x354D},
    {E_REGISTER_CMD_16BIT, 0x3692, 0xB36F},
    {E_REGISTER_CMD_16BIT, 0x3694, 0xDFAD},
    {E_REGISTER_CMD_16BIT, 0x3696, 0x7A68},
    {E_REGISTER_CMD_16BIT, 0x3698, 0x598E},
    {E_REGISTER_CMD_16BIT, 0x369A, 0x1B65},
    {E_REGISTER_CMD_16BIT, 0x369C, 0xE88E},
    {E_REGISTER_CMD_16BIT, 0x369E, 0x96AD},
    {E_REGISTER_CMD_16BIT, 0x36A0, 0xFC2A},
    {E_REGISTER_CMD_16BIT, 0x36A2, 0x578E},
    {E_REGISTER_CMD_16BIT, 0x36A4, 0x68CD},
    {E_REGISTER_CMD_16BIT, 0x36A6, 0x984F},
    {E_REGISTER_CMD_16BIT, 0x36C0, 0x4D30},
    {E_REGISTER_CMD_16BIT, 0x36C2, 0x000E},
    {E_REGISTER_CMD_16BIT, 0x36C4, 0xDA32},
    {E_REGISTER_CMD_16BIT, 0x36C6, 0xF08F},
    {E_REGISTER_CMD_16BIT, 0x36C8, 0x5F13},
    {E_REGISTER_CMD_16BIT, 0x36CA, 0x67D0},
    {E_REGISTER_CMD_16BIT, 0x36CC, 0x882D},
    {E_REGISTER_CMD_16BIT, 0x36CE, 0xA632},
    {E_REGISTER_CMD_16BIT, 0x36D0, 0x556E},
    {E_REGISTER_CMD_16BIT, 0x36D2, 0x1C93},
    {E_REGISTER_CMD_16BIT, 0x36D4, 0x4890},
    {E_REGISTER_CMD_16BIT, 0x36D6, 0xE46C},
    {E_REGISTER_CMD_16BIT, 0x36D8, 0xCEB2},
    {E_REGISTER_CMD_16BIT, 0x36DA, 0x008F},
    {E_REGISTER_CMD_16BIT, 0x36DC, 0x45B3},
    {E_REGISTER_CMD_16BIT, 0x36DE, 0x4730},
    {E_REGISTER_CMD_16BIT, 0x36E0, 0x42AE},
    {E_REGISTER_CMD_16BIT, 0x36E2, 0xDC12},
    {E_REGISTER_CMD_16BIT, 0x36E4, 0x9E30},
    {E_REGISTER_CMD_16BIT, 0x36E6, 0x6BD3},
    {E_REGISTER_CMD_16BIT, 0x3700, 0xF7EC},
    {E_REGISTER_CMD_16BIT, 0x3702, 0x9D2C},
    {E_REGISTER_CMD_16BIT, 0x3704, 0x96AD},
    {E_REGISTER_CMD_16BIT, 0x3706, 0x9710},
    {E_REGISTER_CMD_16BIT, 0x3708, 0x97D2},
    {E_REGISTER_CMD_16BIT, 0x370A, 0x50ED},
    {E_REGISTER_CMD_16BIT, 0x370C, 0x3A2D},
    {E_REGISTER_CMD_16BIT, 0x370E, 0xEFAF},
    {E_REGISTER_CMD_16BIT, 0x3710, 0xA990},
    {E_REGISTER_CMD_16BIT, 0x3712, 0xE991},
    {E_REGISTER_CMD_16BIT, 0x3714, 0x79CE},
    {E_REGISTER_CMD_16BIT, 0x3716, 0x93ED},
    {E_REGISTER_CMD_16BIT, 0x3718, 0xA930},
    {E_REGISTER_CMD_16BIT, 0x371A, 0x80CF},
    {E_REGISTER_CMD_16BIT, 0x371C, 0x8D12},
    {E_REGISTER_CMD_16BIT, 0x371E, 0xEA4C},
    {E_REGISTER_CMD_16BIT, 0x3720, 0x20ED},
    {E_REGISTER_CMD_16BIT, 0x3722, 0x8F0D},
    {E_REGISTER_CMD_16BIT, 0x3724, 0x8711},
    {E_REGISTER_CMD_16BIT, 0x3726, 0xA6B2},
    {E_REGISTER_CMD_16BIT, 0x3740, 0xC6D0},
    {E_REGISTER_CMD_16BIT, 0x3742, 0xD16F},
    {E_REGISTER_CMD_16BIT, 0x3744, 0x25F4},
    {E_REGISTER_CMD_16BIT, 0x3746, 0x1B92},
    {E_REGISTER_CMD_16BIT, 0x3748, 0x9115},
    {E_REGISTER_CMD_16BIT, 0x374A, 0xB350},
    {E_REGISTER_CMD_16BIT, 0x374C, 0x0C70},
    {E_REGISTER_CMD_16BIT, 0x374E, 0x7D93},
    {E_REGISTER_CMD_16BIT, 0x3750, 0xABD1},
    {E_REGISTER_CMD_16BIT, 0x3752, 0xBF14},
    {E_REGISTER_CMD_16BIT, 0x3754, 0xDDF0},
    {E_REGISTER_CMD_16BIT, 0x3756, 0x1950},
    {E_REGISTER_CMD_16BIT, 0x3758, 0x1D34},
    {E_REGISTER_CMD_16BIT, 0x375A, 0xB531},
    {E_REGISTER_CMD_16BIT, 0x375C, 0x87D5},
    {E_REGISTER_CMD_16BIT, 0x375E, 0xBEF0},
    {E_REGISTER_CMD_16BIT, 0x3760, 0x9290},
    {E_REGISTER_CMD_16BIT, 0x3762, 0x2A74},
    {E_REGISTER_CMD_16BIT, 0x3764, 0x2B72},
    {E_REGISTER_CMD_16BIT, 0x3766, 0xA9B5},
    {E_REGISTER_CMD_16BIT, 0x3782, 0x00FC},
    {E_REGISTER_CMD_16BIT, 0x3784, 0x0144},
    {E_REGISTER_CMD_16BIT, 0x3210, 0x00B8},    

    // Step 7

    {E_REGISTER_CMD_16BIT, 0x326E, 0x0006}, // LOW_PASS_YUV_FILTER
    {E_REGISTER_CMD_16BIT, 0x33F4, 0x000B}, // KERNEL_CONFIG
    //REG= 0x098E, 0xA087 	// LOGICAL_ADDRESS_ACCESS [CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_0]
    {E_REGISTER_CMD_8BIT,  0xA087,   0x00}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_0
    {E_REGISTER_CMD_8BIT,  0xA088,   0x07}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_1
    {E_REGISTER_CMD_8BIT,  0xA089,   0x16}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_2
    {E_REGISTER_CMD_8BIT,  0xA08A,   0x30}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_3
    {E_REGISTER_CMD_8BIT,  0xA08B,   0x52}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_4
    {E_REGISTER_CMD_8BIT,  0xA08C,   0x6D}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_5
    {E_REGISTER_CMD_8BIT,  0xA08D,   0x86}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_6
    {E_REGISTER_CMD_8BIT,  0xA08E,   0x9B}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_7
    {E_REGISTER_CMD_8BIT,  0xA08F,   0xAB}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_8
    {E_REGISTER_CMD_8BIT,  0xA090,   0xB9}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_9
    {E_REGISTER_CMD_8BIT,  0xA091,   0xC5}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_10
    {E_REGISTER_CMD_8BIT,  0xA092,   0xCF}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_11
    {E_REGISTER_CMD_8BIT,  0xA093,   0xD8}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_12
    {E_REGISTER_CMD_8BIT,  0xA094,   0xE0}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_13
    {E_REGISTER_CMD_8BIT,  0xA095,   0xE7}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_14
    {E_REGISTER_CMD_8BIT,  0xA096,   0xEE}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_15
    {E_REGISTER_CMD_8BIT,  0xA097,   0xF4}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_16
    {E_REGISTER_CMD_8BIT,  0xA098,   0xFA}, // CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_17
    {E_REGISTER_CMD_16BIT, 0xA0AD, 0x0001}, // CAM_LL_CONFIG_GAMMA_START_BM
    {E_REGISTER_CMD_16BIT, 0xA0AF, 0x0338}, // CAM_LL_CONFIG_GAMMA_STOP_BM
    {E_REGISTER_CMD_8BIT,  0xA0B1,   0x10}, // CAM_LL_CONFIG_NR_RED_START
    {E_REGISTER_CMD_8BIT,  0xA0B2,   0x2D}, // CAM_LL_CONFIG_NR_RED_STOP
    {E_REGISTER_CMD_8BIT,  0xA0B3,   0x10}, // CAM_LL_CONFIG_NR_GREEN_START
    {E_REGISTER_CMD_8BIT,  0xA0B4,   0x2D}, // CAM_LL_CONFIG_NR_GREEN_STOP
    {E_REGISTER_CMD_8BIT,  0xA0B5,   0x10}, // CAM_LL_CONFIG_NR_BLUE_START
    {E_REGISTER_CMD_8BIT,  0xA0B6,   0x2D}, // CAM_LL_CONFIG_NR_BLUE_STOP
    {E_REGISTER_CMD_8BIT,  0xA0B7,   0x10}, // CAM_LL_CONFIG_NR_MIN_MAX_START
    {E_REGISTER_CMD_8BIT,  0xA0B8,   0x2D}, // CAM_LL_CONFIG_NR_MIN_MAX_STOP
    {E_REGISTER_CMD_16BIT, 0xA0B9, 0x0040}, // CAM_LL_CONFIG_START_GAIN_METRIC
    {E_REGISTER_CMD_16BIT, 0xA0BB, 0x00C8}, // CAM_LL_CONFIG_STOP_GAIN_METRIC
    {E_REGISTER_CMD_8BIT,  0xA07A,   0x04}, // CAM_LL_CONFIG_AP_THRESH_START
    {E_REGISTER_CMD_8BIT,  0xA07B,   0x0F}, // CAM_LL_CONFIG_AP_THRESH_STOP
    {E_REGISTER_CMD_8BIT,  0xA07C,   0x03}, // CAM_LL_CONFIG_AP_GAIN_START
    {E_REGISTER_CMD_8BIT,  0xA07D,   0x00}, // CAM_LL_CONFIG_AP_GAIN_STOP
    {E_REGISTER_CMD_16BIT, 0xA07E, 0x0078}, // CAM_LL_CONFIG_CDC_THRESHOLD_BM
    {E_REGISTER_CMD_8BIT,  0xA080,   0x05}, // CAM_LL_CONFIG_CDC_GATE_PERCENTAGE
    {E_REGISTER_CMD_8BIT,  0xA081,   0x28}, // CAM_LL_CONFIG_DM_EDGE_TH_START
    {E_REGISTER_CMD_8BIT,  0xA082,   0x32}, // CAM_LL_CONFIG_DM_EDGE_TH_STOP
    {E_REGISTER_CMD_16BIT, 0xA083, 0x000F}, // CAM_LL_CONFIG_FTB_AVG_YSUM_START
    {E_REGISTER_CMD_16BIT, 0xA085, 0x0000}, // CAM_LL_CONFIG_FTB_AVG_YSUM_STOP
    {E_REGISTER_CMD_8BIT,  0xA020,   0x4B}, // CAM_AE_CONFIG_BASE_TARGET
    {E_REGISTER_CMD_16BIT, 0xA027, 0x0050}, // CAM_AE_CONFIG_MIN_VIRT_AGAIN
    {E_REGISTER_CMD_16BIT, 0xA029, 0x00C0}, // CAM_AE_CONFIG_MAX_VIRT_AGAIN
    {E_REGISTER_CMD_16BIT, 0xA025, 0x0080}, // CAM_AE_CONFIG_MAX_VIRT_DGAIN
    {E_REGISTER_CMD_16BIT, 0xA01C, 0x00C8}, // CAM_AE_CONFIG_TARGET_AGAIN
    {E_REGISTER_CMD_16BIT, 0xA01E, 0x0080}, // CAM_AE_CONFIG_TARGET_DGAIN
    {E_REGISTER_CMD_16BIT, 0xA01A, 0x000B}, // CAM_AE_CONFIG_TARGET_FDZONE
    {E_REGISTER_CMD_8BIT,  0xA05F,   0xA0}, // CAM_AWB_CONFIG_START_SATURATION
    {E_REGISTER_CMD_8BIT,  0xA060,   0x28}, // CAM_AWB_CONFIG_END_SATURATION
    {E_REGISTER_CMD_16BIT, 0xA05B, 0x0005}, // CAM_AWB_CONFIG_START_BRIGHTNESS_BM
    {E_REGISTER_CMD_16BIT, 0xA05D, 0x0023}, // CAM_AWB_CONFIG_STOP_BRIGHTNESS_BM
    {E_REGISTER_CMD_16BIT, 0x9801, 0x000F}, // STAT_CLIP_MAX
    {E_REGISTER_CMD_8BIT,  0x9C02,   0x02}, // LL_GAMMA_SELECT
    {E_REGISTER_CMD_8BIT,  0x9001,   0x05}, // AE_TRACK_MODE
    {E_REGISTER_CMD_8BIT,  0x9007,   0x05}, // AE_TRACK_TARGET_GATE
    {E_REGISTER_CMD_8BIT,  0x9003,   0x12}, // AE_TRACK_BLACK_LEVEL_MAX
    {E_REGISTER_CMD_8BIT,  0x9004,   0x02}, // AE_TRACK_BLACK_LEVEL_STEP_SIZE
    {E_REGISTER_CMD_8BIT,  0x9005,   0x23}, // AE_TRACK_BLACK_CLIP_PERCENT
    {E_REGISTER_CMD_8BIT,  0x8C03,   0x01}, // FD_STAT_MIN
    {E_REGISTER_CMD_8BIT,  0x8C04,   0x03}, // FD_STAT_MAX
    {E_REGISTER_CMD_8BIT,  0x8C05,   0x05}, // FD_MIN_AMPLITUDE
    {E_REGISTER_CMD_16BIT, 0x3040, 0x0041}, //{E_REGISTER_CMD_16BIT, 0x3040, 0x4041}, // READ_MODE Mirror
    {E_REGISTER_CMD_16BIT, 0x098E, 0xA05F}, // LOGICAL_ADDRESS_ACCESS [CAM_AWB_CONFIG_START_SATURATION]
    {E_REGISTER_CMD_8BIT,  0xA05F,   0x40}, // CAM_AWB_CONFIG_START_SATURATION
    {E_REGISTER_CMD_8BIT,  0xA060,   0x00}, // CAM_AWB_CONFIG_END_SATURATION

    {E_REGISTER_CMD_16BIT, 0x098E, 0x2029}, // LOGICAL_ADDRESS_ACCESS [CAM_AE_CONFIG_MAX_VIRT_AGAIN]
    {E_REGISTER_CMD_16BIT, 0xA029, 0x00C0}, // CAM_AE_CONFIG_MAX_VIRT_AGAIN

    {E_REGISTER_CMD_16BIT, 0x0018, 0x0002}, // STANDBY_CONTROL_AND_STATUS

    {E_REGISTER_WAIT, 0x0000, 30}, //Delay =30

    /*
    // ===  output size
    //[ VGA]
    {E_REGISTER_CMD_16BIT, 0x098E, 0x8400}, 	// LOGICAL_ADDRESS_ACCESS [SEQ_CMD]
    {E_REGISTER_CMD_8BIT, 0x8400, 0x01}, 	// SEQ_CMD
    {E_REGISTER_CMD_16BIT, 0xA006, 0x01E0}, 	// CAM_FOV_HEIGHT
    {E_REGISTER_CMD_16BIT, 0xA000, 0x0280}, 	// CAM_IMAGE_WIDTH
    {E_REGISTER_CMD_16BIT, 0xA002, 0x01E0}, 	// CAM_IMAGE_HEIGHT
    {E_REGISTER_CMD_8BIT, 0x8400, 0x02}, 	// SEQ_CMD     
    */
};

/*add the effect setting*/
/* avert with the effect can not open the front camera*/
static struct mt9v114_i2c_reg_conf mt9v114_effect_off_reg_config[] =
{
    {E_REGISTER_CMD_16BIT, 0x098E, 0x8400},	// LOGICAL_ADDRESS_ACCESS [SEQ_CMD]
   // {E_REGISTER_CMD_8BIT,  0x8400,   0x01},// SEQ_CMD
    {E_REGISTER_CMD_8BIT,  0xA010,   0x00},// CAM_SELECT_FX
    {E_REGISTER_CMD_8BIT,  0x8400,   0x02},// SEQ_CMD
};

static struct mt9v114_i2c_reg_conf mt9v114_effect_mono_reg_config[] =
{
    {E_REGISTER_CMD_16BIT, 0x098E, 0x8400},	// LOGICAL_ADDRESS_ACCESS [SEQ_CMD]
   // {E_REGISTER_CMD_8BIT,  0x8400,   0x01},// SEQ_CMD
    {E_REGISTER_CMD_8BIT,  0xA010,   0x01},// CAM_SELECT_FX
    {E_REGISTER_CMD_8BIT,  0x8400,   0x02},// SEQ_CMD
};

static struct mt9v114_i2c_reg_conf mt9v114_effect_negative_reg_config[] =
{
    {E_REGISTER_CMD_16BIT, 0x098E, 0x8400},	// LOGICAL_ADDRESS_ACCESS [SEQ_CMD]
   // {E_REGISTER_CMD_8BIT,  0x8400,   0x01},// SEQ_CMD
    {E_REGISTER_CMD_8BIT,  0xA010,   0x03},// CAM_SELECT_FX
    {E_REGISTER_CMD_8BIT,  0x8400,   0x02},// SEQ_CMD
};

static struct mt9v114_i2c_reg_conf mt9v114_effect_solarize_reg_config[] =
{
    {E_REGISTER_CMD_16BIT, 0x098E, 0x8400}, 	// LOGICAL_ADDRESS_ACCESS [SEQ_CMD]
    //{E_REGISTER_CMD_8BIT,  0x8400,   0x01}, 	// SEQ_CMD
    {E_REGISTER_CMD_8BIT,  0xA010,   0x04}, 	// CAM_SELECT_FX
    {E_REGISTER_CMD_8BIT,  0x8400,   0x02}, 	// SEQ_CMD
};

static struct mt9v114_i2c_reg_conf mt9v114_effect_sepia_reg_config[] =
{
    {E_REGISTER_CMD_16BIT, 0x098E, 0x8400}, 	// LOGICAL_ADDRESS_ACCESS [SEQ_CMD]
   // {E_REGISTER_CMD_8BIT,  0x8400,   0x01},// SEQ_CMD
    {E_REGISTER_CMD_8BIT,  0xA010,   0x02},// CAM_SELECT_FX
    {E_REGISTER_CMD_8BIT,  0xA012,   0x1E},// CAM_SEPIA_CR
    {E_REGISTER_CMD_8BIT,  0xA013,   0xD8},// CAM_SEPIA_CB
    {E_REGISTER_CMD_8BIT,  0x8400,   0x02},// SEQ_CMD
};

static struct mt9v114_i2c_reg_conf mt9v114_effect_aqua_reg_config[] =
{
    {E_REGISTER_CMD_16BIT, 0x098E, 0x8400},   // LOGICAL_ADDRESS_ACCESS [SEQ_CMD]
    // {E_REGISTER_CMD_8BIT,  0x8400,   0x01},   // SEQ_CMD
    {E_REGISTER_CMD_8BIT,  0xA010,   0x02},   // CAM_SELECT_FX
    {E_REGISTER_CMD_8BIT,  0xA012,   0xCA},   // CAM_SEPIA_CR
    {E_REGISTER_CMD_8BIT,  0xA013,   0x28},   // CAM_SEPIA_CB
    {E_REGISTER_CMD_8BIT,  0x8400,   0x02},   // SEQ_CMD


};

/*add the wb setting*/
static struct mt9v114_i2c_reg_conf mt9v114_wb_auto_reg_config[] =
{
    {E_REGISTER_CMD_8BIT,  0x9401, 0x0D}, // AWB_MODE
};

static struct mt9v114_i2c_reg_conf mt9v114_wb_incandescent_reg_config[] =
{
    {E_REGISTER_CMD_8BIT,  0x9401, 0x0C}, // AWB_MODE
    {E_REGISTER_CMD_8BIT,  0x9436, 0x5E},     // AWB_R_RATIO_PRE_AWB
    {E_REGISTER_CMD_8BIT,  0x9437, 0x33},     // AWB_B_RATIO_PRE_AWB

};

/*modify the R_Ration for fluorescent whitebalance*/
static struct mt9v114_i2c_reg_conf mt9v114_wb_fluorescent_reg_config[] =
{
    {E_REGISTER_CMD_8BIT,  0x9401, 0x0C}, // AWB_MODE
    {E_REGISTER_CMD_8BIT,  0x9436, 0x57},     // AWB_R_RATIO_PRE_AWB
    {E_REGISTER_CMD_8BIT,  0x9437, 0x3C},     // AWB_B_RATIO_PRE_AWB

};

/* this config should used for cloudy,not daylight, so change it to cloudy */
static struct mt9v114_i2c_reg_conf mt9v114_wb_cloudy_reg_config[] =
{

    {E_REGISTER_CMD_8BIT,  0x9401, 0x0C}, // AWB_MODE
    {E_REGISTER_CMD_8BIT,  0x9436, 0x42},     // AWB_R_RATIO_PRE_AWB
    {E_REGISTER_CMD_8BIT,  0x9437, 0x56}, // AWB_B_RATIO_PRE_AWB

};

/* this config should used for daylight,not cloudy,so change it to daylight */
static struct mt9v114_i2c_reg_conf mt9v114_wb_daylight_reg_config[] =
{

    {E_REGISTER_CMD_8BIT,  0x9401, 0x0C}, // AWB_MODE
    {E_REGISTER_CMD_8BIT,  0x9436, 0x4B},     // AWB_R_RATIO_PRE_AWB
    {E_REGISTER_CMD_8BIT,  0x9437, 0x49}, // AWB_B_RATIO_PRE_AWB

};

static struct  mt9v114_work_t *mt9v114sensorw = NULL;
static struct  i2c_client *mt9v114_client  = NULL;
static struct mt9v114_ctrl_t *mt9v114_ctrl = NULL;

static DECLARE_WAIT_QUEUE_HEAD(mt9v114_wait_queue);
DEFINE_SEMAPHORE(mt9v114_sem);

static int mt9v114_i2c_rxdata
(   unsigned short saddr,
    unsigned char *rxdata,
    int            length)
{
    struct i2c_msg msgs[] =
    {
        {
            .addr  = saddr,
            .flags = 0,
            .len = 2,
            .buf = rxdata,
        },
        {
            .addr  = saddr,
            .flags = I2C_M_RD,
            .len = length,
            .buf = rxdata,
        },
    };

    if (i2c_transfer(mt9v114_client->adapter, msgs, 2) < 0)
    {
        CDBG("mt9v114_i2c_rxdata failed!\n");
        return -EIO;
    }

    return 0;
}

static int32_t mt9v114_i2c_read_word
(   unsigned short  saddr,
    unsigned short  raddr,
    unsigned short *rdata)
{
    int32_t rc = 0;
    unsigned char buf[4];

    if (!rdata)
    {
        return -EIO;
    }

    memset(buf, 0, sizeof(buf));

    buf[0] = (raddr & 0xFF00) >> 8;
    buf[1] = (raddr & 0x00FF);

    rc = mt9v114_i2c_rxdata(saddr, buf, 2);
    if (rc < 0)
    {
        return rc;
    }

    *rdata = buf[0] << 8 | buf[1];

    if (rc < 0)
    {
        CDBG("mt9v114_i2c_read_word failed!\n");
    }

    return rc;
}

static int32_t mt9v114_i2c_txdata
(   unsigned short saddr,
    unsigned char *txdata,
    int            length)
{
    struct i2c_msg msg[] =
    {
        {
            .addr  = saddr,
            .flags = 0,
            .len = length,
            .buf = txdata,
        },
    };

    if (i2c_transfer(mt9v114_client->adapter, msg, 1) < 0)
    {
        CDBG("mt9v114_i2c_txdata faild\n");
        return -EIO;
    }

    return 0;
}
/* Add i2c write function for reset i2c */
static int32_t mt9v114_i2c_write_word_sendonetime
(
    unsigned short saddr,
    unsigned short waddr, 
    unsigned short wdata)
{
	int32_t rc = -EFAULT;
	unsigned char buf[4];

	memset(buf, 0, sizeof(buf));
	buf[0] = (waddr & 0xFF00)>>8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = (wdata & 0xFF00)>>8;
	buf[3] = (wdata & 0x00FF);
	  rc = mt9v114_i2c_txdata(saddr, buf, 4);
	return rc; 
}

static int32_t mt9v114_i2c_write_word
(   unsigned short saddr,
    unsigned short waddr,
    unsigned short wdata)
{
    int32_t rc = -EFAULT;
    unsigned char buf[4];
    int32_t i = 0;

    memset(buf, 0, sizeof(buf));
    buf[0] = (waddr & 0xFF00) >> 8;
    buf[1] = (waddr & 0x00FF);
    buf[2] = (wdata & 0xFF00) >> 8;
    buf[3] = (wdata & 0x00FF);

    /*write 3 times, if error, return -EIO*/
    for (i = 0; i < 3; i++)
    {
        rc = mt9v114_i2c_txdata(saddr, buf, 4);
        if (0 <= rc)
        {
            return 0;
        }
    }

    if (3 == i)
    {
        CDBG("mt9v114_i2c_write_word failed, addr = 0x%x, val = 0x%x!\n", waddr, wdata);
        return -EIO;
    }

    return 0;
}

static int mt9v114_i2c_write_byte
(   unsigned short saddr,
    unsigned short waddr,
    unsigned char  cdata)
{
    int32_t rc = -EFAULT;
    unsigned char buf[3];
    int32_t i = 0;

    memset(buf, 0, sizeof(buf));
    buf[0] = (waddr & 0xFF00) >> 8;
    buf[1] = (waddr & 0x00FF);
    buf[2] = (cdata & 0x00FF);

    /*write 3 times, if error, return -EIO*/
    for (i = 0; i < 3; i++)
    {
        rc = mt9v114_i2c_txdata(saddr, buf, 3);
        if (0 <= rc)
        {
            return 0;
        }
    }

    if (3 == i)
    {
        CDBG("mt9v114_i2c_write_byte failed, addr = 0x%x, val = 0x%x!\n", waddr, cdata);
        return -EIO;
    }

    return 0;
}

int32_t mt9v114_i2c_write_table(struct mt9v114_i2c_reg_conf *reg_conf_tbl, int num_of_items_in_table)
{
    int i;
    int32_t rc = -EFAULT;

    CDBG("mt9v114_i2c_write_table.\n");

    for (i = 0; i < num_of_items_in_table; i++)
    {
        if (E_REGISTER_CMD_8BIT == reg_conf_tbl->type)
        {
            rc = mt9v114_i2c_write_byte(mt9v114_client->addr,
                                        reg_conf_tbl->reg, reg_conf_tbl->value);
            if (rc < 0)
            {
                CDBG("mt9v114_i2c_write_table,reg[%d] write failed.\n", i);
                break;
            }
        }
        else if (E_REGISTER_CMD_16BIT == reg_conf_tbl->type)
        {
            rc = mt9v114_i2c_write_word(mt9v114_client->addr,
                                        reg_conf_tbl->reg, reg_conf_tbl->value);
            if (rc < 0)
            {
                CDBG("mt9v114_i2c_write_table,reg[%d] write failed.\n", i);
                break;
            }
        }
        else if (E_REGISTER_WAIT == reg_conf_tbl->type)
        {
            mdelay(reg_conf_tbl->value);
        }
        else
        {
            CDBG("mt9v114_i2c_write_table,reg[%d].type is wrong.\n", i);
        }

        reg_conf_tbl++;
    }

    return rc;
}

int32_t mt9v114_set_default_focus(uint8_t af_step)
{
    CDBG("mt9v114_set_default_focus,af_step:%d\n", af_step);
    return 0;
}

/*add the effect setting*/
int32_t mt9v114_set_effect(int32_t effect)
{
	struct mt9v114_i2c_reg_conf  *reg_conf_tbl = NULL;
    int num_of_items_in_table = 0;
	long rc = 0;

	CDBG("mt9v114_set_effect,af_step:current_effect:%d, effect:%d\n", current_effect, effect);    
	switch (effect) {
	case CAMERA_EFFECT_OFF:
        reg_conf_tbl = mt9v114_effect_off_reg_config;
        num_of_items_in_table = sizeof(mt9v114_effect_off_reg_config) / sizeof(mt9v114_effect_off_reg_config[0]);
        break;

	case CAMERA_EFFECT_MONO:
        reg_conf_tbl = mt9v114_effect_mono_reg_config;
        num_of_items_in_table = sizeof(mt9v114_effect_mono_reg_config) / sizeof(mt9v114_effect_mono_reg_config[0]);
		break;

	case CAMERA_EFFECT_NEGATIVE:
        reg_conf_tbl = mt9v114_effect_negative_reg_config;
        num_of_items_in_table = sizeof(mt9v114_effect_negative_reg_config) / sizeof(mt9v114_effect_negative_reg_config[0]);
		break;

	case CAMERA_EFFECT_SOLARIZE:
        reg_conf_tbl = mt9v114_effect_solarize_reg_config;
        num_of_items_in_table = sizeof(mt9v114_effect_solarize_reg_config) / sizeof(mt9v114_effect_solarize_reg_config[0]);
		break;

	case CAMERA_EFFECT_SEPIA:
        reg_conf_tbl = mt9v114_effect_sepia_reg_config;
        num_of_items_in_table = sizeof(mt9v114_effect_sepia_reg_config) / sizeof(mt9v114_effect_sepia_reg_config[0]);
		break;
        
	case CAMERA_EFFECT_AQUA:
        reg_conf_tbl = mt9v114_effect_aqua_reg_config;
        num_of_items_in_table = sizeof(mt9v114_effect_aqua_reg_config) / sizeof(mt9v114_effect_aqua_reg_config[0]);
		break;
              
	default: 
		return 0;
	}

	if(current_effect != effect)
	{
		rc = mt9v114_i2c_write_table(reg_conf_tbl, num_of_items_in_table);
		current_effect = effect;
	}
    
    return rc;

}

/*add the wb setting*/
int32_t mt9v114_set_wb(int32_t wb)
{
	struct mt9v114_i2c_reg_conf *reg_conf_tbl = NULL;
    int num_of_items_in_table = 0;
	long rc = 0;
    
	switch (wb) {
	case CAMERA_WB_AUTO:
        reg_conf_tbl = mt9v114_wb_auto_reg_config;
        num_of_items_in_table = sizeof(mt9v114_wb_auto_reg_config) / sizeof(mt9v114_wb_auto_reg_config[0]);
        break;

	case CAMERA_WB_INCANDESCENT:
        reg_conf_tbl = mt9v114_wb_incandescent_reg_config;
        num_of_items_in_table = sizeof(mt9v114_wb_incandescent_reg_config) / sizeof(mt9v114_wb_incandescent_reg_config[0]);
		break;

	case CAMERA_WB_CUSTOM:       
	case CAMERA_WB_FLUORESCENT:
        reg_conf_tbl = mt9v114_wb_fluorescent_reg_config;
        num_of_items_in_table = sizeof(mt9v114_wb_fluorescent_reg_config) / sizeof(mt9v114_wb_fluorescent_reg_config[0]);
		break;

	case CAMERA_WB_DAYLIGHT:
        reg_conf_tbl = mt9v114_wb_daylight_reg_config;
        num_of_items_in_table = sizeof(mt9v114_wb_daylight_reg_config) / sizeof(mt9v114_wb_daylight_reg_config[0]);
		break;
        
	case CAMERA_WB_CLOUDY_DAYLIGHT:
        reg_conf_tbl = mt9v114_wb_cloudy_reg_config;
        num_of_items_in_table = sizeof(mt9v114_wb_cloudy_reg_config) / sizeof(mt9v114_wb_cloudy_reg_config[0]);
		break;
              
	default: 
		return 0;
	}

    rc = mt9v114_i2c_write_table(reg_conf_tbl, num_of_items_in_table);
    return rc;

}


int32_t mt9v114_set_fps(struct fps_cfg *fps)
{
    /* input is new fps in Q8 format */
    int32_t rc = 0;

    CDBG("mt9v114_set_fps\n");
    return rc;
}

int32_t mt9v114_write_exp_gain(uint16_t gain, uint32_t line)
{
    CDBG("mt9v114_write_exp_gain\n");
    return 0;
}

int32_t mt9v114_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
    int32_t rc = 0;

    CDBG("mt9v114_set_pict_exp_gain\n");


    /* camera_timed_wait(snapshot_wait*exposure_ratio); */
    return rc;
}

int32_t mt9v114_setting(enum mt9v114_reg_update_t rupdate,
                        enum mt9v114_setting_t    rt)
{
    int32_t rc = 0;

    switch (rupdate)
    {
    case UPDATE_PERIODIC:
        if (rt == RES_PREVIEW)
        {
            return rc;
        }
        else
        {}

        break;

    case REG_INIT:

        rc = mt9v114_i2c_write_table(mt9v114_init_reg_config,
                                     sizeof(mt9v114_init_reg_config) / sizeof(mt9v114_init_reg_config[0]));
        if (rc < 0)
        {
            CDBG("mt9v114_setting,reg init failed.\n");
        }

        mdelay(5);
        return rc;
        break;

    default:
        rc = -EFAULT;
        break;
    } /* switch (rupdate) */

    return rc;
}

int32_t mt9v114_video_config(int mode, int res)
{
    int32_t rc;

    switch (res)
    {
    case QTR_SIZE:
        rc = mt9v114_setting(UPDATE_PERIODIC, RES_PREVIEW);
        if (rc < 0)
        {
            CDBG("mt9v114_video_config:sensor configuration fail!\n");
            return rc;
        }

        CDBG("sensor configuration done!\n");
        break;

    case FULL_SIZE:
        rc = mt9v114_setting(UPDATE_PERIODIC, RES_CAPTURE);
        if (rc < 0)
        {
            CDBG("mt9v114_video_config:sensor configuration fail!\n");
            return rc;
        }

        break;

    default:
        return 0;
    } /* switch */

    mt9v114_ctrl->prev_res   = res;
    mt9v114_ctrl->curr_res   = res;
    mt9v114_ctrl->sensormode = mode;

    return rc;
}

int32_t mt9v114_snapshot_config(int mode)
{
    int32_t rc = 0;

    rc = mt9v114_setting(UPDATE_PERIODIC, RES_CAPTURE);
    if (rc < 0)
    {
        return rc;
    }

    mt9v114_ctrl->curr_res = mt9v114_ctrl->pict_res;

    mt9v114_ctrl->sensormode = mode;

    return rc;
}

int32_t mt9v114_power_down(void)
{
    int32_t rc = 0;

    return rc;
}

int32_t mt9v114_move_focus(int direction, int32_t num_steps)
{
 
    return 0;
}

static int mt9v114_sensor_init_done(const struct msm_camera_sensor_info *data)
{
    gpio_direction_output(data->sensor_pwd, 1);
    gpio_free(data->sensor_pwd);

	mdelay(5);

    if (data->vreg_disable_func)
    {
        data->vreg_disable_func(0);
    }

    return 0;
}

static int mt9v114_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
    int rc;
    unsigned short chipid;

    if (data->vreg_enable_func)
    {
        data->vreg_enable_func(1);
    }

    mdelay(5);
    rc = gpio_request(data->sensor_pwd, "mt9v114");
    if (!rc)
    {
        gpio_direction_output(data->sensor_pwd, 0);
    }
    else
    {
        goto init_probe_fail;
    }

    mdelay(MT9V114_RESET_DELAY_MSECS);

    /* RESET the sensor image part via I2C command */
    rc = mt9v114_i2c_write_word_sendonetime(mt9v114_client->addr,
                                MT9V114_REG_RESET_REGISTER, 0x0306);
    mdelay(5);
    rc = mt9v114_i2c_write_word(mt9v114_client->addr,
                                MT9V114_REG_RESET_REGISTER, 0x0124);
    mdelay(5);

    /* Read sensor Model ID: */
    rc = mt9v114_i2c_read_word(mt9v114_client->addr,
                               MT9V114_REG_MODEL_ID, &chipid);
    if (rc < 0)
    {
        CDBG("mt9v114_i2c_read_word Model_ID failed!! rc=%d", rc);
        goto init_probe_fail;
    }

    CDBG("mt9v114 model_id = 0x%x\n", chipid);

    /* Compare sensor ID to MT9T012VC ID: */
    if (chipid != MT9V114_MODEL_ID)
    {
        CDBG("mt9v114 Model_ID  error!!");
        rc = -ENODEV;
        goto init_probe_fail;
    }


    goto init_probe_done;

init_probe_fail:
    mt9v114_sensor_init_done(data);
init_probe_done:
    return rc;
}

int mt9v114_sensor_open_init(const struct msm_camera_sensor_info *data)
{
    int32_t rc;
    current_effect  = CAMERA_EFFECT_OFF;
    mt9v114_ctrl = kzalloc(sizeof(struct mt9v114_ctrl_t), GFP_KERNEL);
    if (!mt9v114_ctrl)
    {
        CDBG("mt9v114_sensor_open_init failed!\n");
        rc = -ENOMEM;
        goto init_done;
    }

    mt9v114_ctrl->fps_divider = 1 * 0x00000400;
    mt9v114_ctrl->pict_fps_divider = 1 * 0x00000400;
    mt9v114_ctrl->set_test = TEST_OFF;
    mt9v114_ctrl->prev_res = QTR_SIZE;
    mt9v114_ctrl->pict_res = FULL_SIZE;

    if (data)
    {
        mt9v114_ctrl->sensordata = data;
    }

    /* enable mclk first */
    msm_camio_clk_rate_set(MT9V114_DEFAULT_CLOCK_RATE);
    mdelay(20);

    msm_camio_camif_pad_reg_reset();
    mdelay(20);

    rc = mt9v114_probe_init_sensor(data);
    if (rc < 0)
    {
        goto init_fail;
    }

    if (mt9v114_ctrl->prev_res == QTR_SIZE)
    {
        rc = mt9v114_setting(REG_INIT, RES_PREVIEW);
    }
    else
    {
        rc = mt9v114_setting(REG_INIT, RES_CAPTURE);
    }

    mdelay(10);

    if (rc < 0)
    {
        goto init_fail;
    }
    else
    {
        goto init_done;
    }

init_fail:
    kfree(mt9v114_ctrl);
init_done:
    return rc;
}

int mt9v114_init_client(struct i2c_client *client)
{
    /* Initialize the MSM_CAMI2C Chip */
    init_waitqueue_head(&mt9v114_wait_queue);
    return 0;
}

int32_t mt9v114_set_sensor_mode(int mode, int res)
{
    int32_t rc = 0;

    CDBG("mt9v114_set_sensor_mode: mode=%d, res=%d.\n", mode, res);

    switch (mode)
    {
    case SENSOR_PREVIEW_MODE:
        CDBG("SENSOR_PREVIEW_MODE\n");
        rc = mt9v114_video_config(mode, res);
        break;

    case SENSOR_SNAPSHOT_MODE:
    case SENSOR_RAW_SNAPSHOT_MODE:
        CDBG("SENSOR_SNAPSHOT_MODE\n");
        rc = mt9v114_snapshot_config(mode);
        break;

    default:
        rc = -EINVAL;
        break;
    }

    return rc;
}

int mt9v114_sensor_config(void __user *argp)
{
    struct sensor_cfg_data cdata;
    long rc = 0;

    if (copy_from_user(&cdata,
                       (void *)argp,
                       sizeof(struct sensor_cfg_data)))
    {
        return -EFAULT;
    }

    down(&mt9v114_sem);

    CDBG("mt9v114_sensor_config: cfgtype = %d\n",
         cdata.cfgtype);
    switch (cdata.cfgtype)
    {
    case CFG_GET_PICT_FPS:
        break;

    case CFG_GET_PREV_L_PF:
        break;

    case CFG_GET_PREV_P_PL:
        break;

    case CFG_GET_PICT_L_PF:
        break;

    case CFG_GET_PICT_P_PL:
        break;

    case CFG_GET_PICT_MAX_EXP_LC:
        break;

    case CFG_SET_FPS:
    case CFG_SET_PICT_FPS:
        rc = mt9v114_set_fps(&(cdata.cfg.fps));
        break;

    case CFG_SET_EXP_GAIN:
        rc =
            mt9v114_write_exp_gain(
            cdata.cfg.exp_gain.gain,
            cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_PICT_EXP_GAIN:
        rc =
            mt9v114_set_pict_exp_gain(
            cdata.cfg.exp_gain.gain,
            cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_MODE:
        rc = mt9v114_set_sensor_mode(cdata.mode,
                                     cdata.rs);
        break;

    case CFG_PWR_DOWN:
        rc = mt9v114_power_down();
        break;

    case CFG_MOVE_FOCUS:
        rc =
            mt9v114_move_focus(
            cdata.cfg.focus.dir,
            cdata.cfg.focus.steps);
        break;

    case CFG_SET_DEFAULT_FOCUS:
        rc =
            mt9v114_set_default_focus(
            cdata.cfg.focus.steps);
        break;

    case CFG_SET_EFFECT:
        rc = mt9v114_set_effect(
            cdata.cfg.effect);
        break;

    case CFG_SET_WB:
    	/*turn on the whitebalance setting*/
        rc = mt9v114_set_wb(
            cdata.cfg.effect);
        break;
    default:
        rc = -EFAULT;
        break;
    }

    up(&mt9v114_sem);

    return rc;
}

int mt9v114_sensor_release(void)
{
    int rc = -EBADF;

    down(&mt9v114_sem);

    mt9v114_power_down();

    mt9v114_sensor_init_done(mt9v114_ctrl->sensordata);

    kfree(mt9v114_ctrl);

    up(&mt9v114_sem);
    CDBG("mt9v114_release completed!\n");
    return rc;
}

static int mt9v114_i2c_probe(struct i2c_client *         client,
                             const struct i2c_device_id *id)
{
    int rc = 0;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        rc = -ENOTSUPP;
        goto probe_failure;
    }

    mt9v114sensorw =
        kzalloc(sizeof(struct mt9v114_work_t), GFP_KERNEL);

    if (!mt9v114sensorw)
    {
        rc = -ENOMEM;
        goto probe_failure;
    }

    i2c_set_clientdata(client, mt9v114sensorw);
    mt9v114_init_client(client);
    mt9v114_client = client;

    //mt9v114_client->addr = mt9v114_client->addr >> 1;

    CDBG("i2c probe ok\n");
    return 0;

probe_failure:
    kfree(mt9v114sensorw);
    mt9v114sensorw = NULL;
    pr_err("i2c probe failure %d\n", rc);
    return rc;
}

static const struct i2c_device_id mt9v114_i2c_id[] =
{
    { "mt9v114_sunny", 0},
    { }
};

static struct i2c_driver mt9v114_i2c_driver =
{
    .id_table = mt9v114_i2c_id,
    .probe    = mt9v114_i2c_probe,
    .remove   = __exit_p(mt9v114_i2c_remove),
    .driver   = {
        .name = "mt9v114_sunny",
    },
};

static int mt9v114_sensor_probe(const struct msm_camera_sensor_info *info,
                                struct msm_sensor_ctrl *s)
{
    /* We expect this driver to match with the i2c device registered
     * in the board file immediately. */
    int rc = i2c_add_driver(&mt9v114_i2c_driver);

    if ((rc < 0) || (mt9v114_client == NULL))
    {
        rc = -ENOTSUPP;
        goto probe_done;
    }

    mdelay(5);

    /* enable mclk first */
    msm_camio_clk_rate_set(MT9V114_DEFAULT_CLOCK_RATE);
    mdelay(20);

    rc = mt9v114_probe_init_sensor(info);
    if (rc < 0)
    {
        i2c_del_driver(&mt9v114_i2c_driver);
        goto probe_done;
    }

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
    set_hw_dev_flag(DEV_I2C_CAMERA_SLAVE);
#endif

    s->s_init = mt9v114_sensor_open_init;
    s->s_release = mt9v114_sensor_release;
    s->s_config = mt9v114_sensor_config;
    s->s_camera_type = FRONT_CAMERA_2D;
    s->s_mount_angle = 270;
    mt9v114_sensor_init_done(info);

probe_done:
    return rc;
}

static int __mt9v114_probe(struct platform_device *pdev)
{
    printk("__mt9v114_probe\n"); //CAMERALOG h00144661 add
    return msm_camera_drv_start(pdev, mt9v114_sensor_probe);
}

static struct platform_driver msm_camera_driver =
{
    .probe     = __mt9v114_probe,
    .driver    = {
        .name  = "msm_camera_mt9v114_sunny",
        .owner = THIS_MODULE,
    },
};

static int __init mt9v114_init(void)
{
    return platform_driver_register(&msm_camera_driver);
}

module_init(mt9v114_init);

