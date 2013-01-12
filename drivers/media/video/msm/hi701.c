
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

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/camera.h>
#include "hi701.h"

#include "linux/hardware_self_adapt.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
 #include <linux/hw_dev_dec.h>
#endif
#undef CDBG
#define CDBG(fmt, args...) printk(KERN_INFO "hi701.c: " fmt, ## args)

/*=============================================================
    SENSOR REGISTER DEFINES
==============================================================*/
#define HI701_REG_CHIP_ID 0x04
#define HI701_CHIP_ID 0x81
#define HI701_REG_RESET_REGISTER 0x01

enum hi701_test_mode_t
{
    TEST_OFF,
    TEST_1,
    TEST_2,
    TEST_3
};

enum hi701_resolution_t
{
    QTR_SIZE,
    FULL_SIZE,
    INVALID_SIZE
};

enum hi701_reg_update_t
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

enum hi701_setting_t
{
    RES_PREVIEW,
    RES_CAPTURE
};


/* for 15 fps preview */
#define HI701_DEFAULT_CLOCK_RATE 24000000

#define HI701_ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

/* FIXME: Changes from here */
struct hi701_work_t
{
    struct work_struct work;
};

struct hi701_ctrl_t
{
    const struct  msm_camera_sensor_info *sensordata;

    int      sensormode;
    uint32_t fps_divider; /* init to 1 * 0x00000400 */
    uint32_t pict_fps_divider; /* init to 1 * 0x00000400 */

    uint16_t curr_lens_pos;
    uint16_t init_curr_lens_pos;
    uint16_t my_reg_gain;
    uint32_t my_reg_line_count;

    enum hi701_resolution_t prev_res;
    enum hi701_resolution_t pict_res;
    enum hi701_resolution_t curr_res;
    enum hi701_test_mode_t  set_test;

    unsigned short imgaddr;
};

struct hi701_i2c_reg_conf
{
    unsigned char waddr;
    unsigned char wdata;
};

const static char hi701_supported_effect[] = "none,mono,negative,solarize,sepia,aqua";
static struct hi701_i2c_reg_conf hi701_init_reg_config_comm[] =
{
    {0x03, 0x00}, {0x01, 0xf1}, {0x01, 0xf3}, {0x01, 0xf1},
    {0x03, 0x20}, {0x10, 0x1c}, {0x03, 0x22}, {0x10, 0x6b},
    {0x03, 0x00}, {0x10, 0x00}, {0x11, 0x93}, {0x12, 0x04},
    {0x20, 0x00}, {0x21, 0x05}, {0x22, 0x00}, {0x23, 0x07},
    {0x24, 0x01}, {0x25, 0xe2}, {0x26, 0x02}, {0x27, 0x80},
    {0x40, 0x01}, {0x41, 0x68}, {0x42, 0x00}, {0x43, 0x14},
    {0x80, 0x0e}, {0x81, 0x93}, {0x83, 0x03}, {0x84, 0x91},
    {0x90, 0x0a}, {0x91, 0x0a}, {0x92, 0x70}, {0x93, 0x60},
    {0xa0, 0x03}, {0xa1, 0x03}, {0xa2, 0x03}, {0xa3, 0x03},
    {0xa4, 0x03}, {0xa5, 0x03}, {0xa6, 0x03}, {0xa7, 0x03},
    {0xa8, 0x44}, {0xa9, 0x44}, {0xaa, 0x44}, {0xab, 0x44},
    {0xac, 0x46}, {0xad, 0x46}, {0xae, 0x44}, {0xaf, 0x44},
    {0x03, 0x02}, {0x1a, 0x21}, {0x1c, 0x00}, {0x1d, 0x03},
    {0x20, 0x33}, {0x21, 0x77}, {0x22, 0xad}, {0x34, 0xff},
    {0x54, 0x30}, {0x62, 0x78}, {0x63, 0x7a}, {0x64, 0x7d},
    {0x65, 0x88}, {0x72, 0x78}, {0x73, 0x8b}, {0x74, 0x78},
    {0x75, 0x8b}, {0xa0, 0x03}, {0xa8, 0x03}, {0xaa, 0x03},
    {0x03, 0x10}, {0x10, 0x01}, {0x12, 0x30}, {0x40, 0x2a},
    {0x41, 0x05}, {0x50, 0x90}, {0x60, 0x1f}, {0x61, 0xb5},
    {0x62, 0xb5}, {0x63, 0x90}, {0x64, 0x80}, {0x03, 0x11},
    {0x10, 0x1d}, {0x11, 0x0a}, {0x60, 0x12}, {0x62, 0x43},
    {0x63, 0x53}, {0x03, 0x12}, {0x40, 0x21}, {0x41, 0x07},
    {0x50, 0x0d}, {0x70, 0x1d}, {0x74, 0x04}, {0x75, 0x06},
    {0x90, 0x5d}, {0x91, 0x10}, {0xb0, 0xc9}, {0x03, 0x13},
    {0x10, 0x19}, {0x11, 0x07}, {0x12, 0x01}, {0x13, 0x02},
    {0x20, 0x04}, {0x21, 0x05}, {0x23, 0x18}, {0x24, 0x03},
    {0x24, 0x03}, {0x80, 0x0d}, {0x81, 0x01}, {0x83, 0x5d},
    {0x90, 0x05}, {0x91, 0x04}, {0x93, 0x19}, {0x94, 0x03},
    {0x95, 0x00}, {0x03, 0x14}, {0x10, 0x07}, {0x20, 0x80},
    {0x21, 0x80}, {0x22, 0x6b}, {0x23, 0x5b}, {0x24, 0x4e},
    {0x25, 0x5a}, {0x26, 0x50}, {0x03, 0x15}, {0x10, 0x0f},
    {0x14, 0x36}, {0x16, 0x28}, {0x17, 0x2f}, {0x30, 0x5c},
    {0x31, 0x26}, {0x32, 0x0a}, {0x33, 0x11}, {0x34, 0x65},
    {0x35, 0x14}, {0x36, 0x01}, {0x37, 0x33}, {0x38, 0x74},
    {0x40, 0x00}, {0x41, 0x00}, {0x42, 0x00}, {0x43, 0x8b},
    {0x44, 0x07}, {0x45, 0x04}, {0x46, 0x84}, {0x47, 0xa1},
    {0x48, 0x25}, {0x03, 0x16}, {0x10, 0x01}, {0x30, 0x00},
    {0x31, 0x03}, {0x32, 0x0b}, {0x33, 0x22}, {0x34, 0x46},
    {0x35, 0x63}, {0x36, 0x78}, {0x37, 0x87}, {0x38, 0x96},
    {0x39, 0xa1}, {0x3a, 0xad}, {0x3b, 0xbd}, {0x3c, 0xc8},
    {0x3d, 0xcf}, {0x3e, 0xd2}, {0x03, 0x17}, {0x10, 0x03},
    {0xc4, 0x3c}, {0xc5, 0x32}, {0xc6, 0x02}, {0xc7, 0x20},
    {0x03, 0x20}, {0x11, 0x00}, {0x20, 0x01}, {0x28, 0x1f},
    {0x29, 0xa3}, {0x2a, 0xf0}, {0x2b, 0x34}, {0x30, 0x78},
    {0x60, 0x6a}, {0x70, 0x48}, {0x78, 0x11}, {0x79, 0x24},
    {0x7A, 0x24}, {0x83, 0x00}, {0x84, 0xb4}, {0x85, 0x00},
    {0x86, 0x02}, {0x87, 0x00}, {0x88, 0x02}, {0x89, 0x58},
    {0x8a, 0x00}, {0x8b, 0x3c}, {0x8c, 0x00}, {0x8d, 0x32},
    {0x8e, 0x00}, {0x8f, 0xc4}, {0x90, 0x68}, {0x91, 0x02},
    {0x92, 0xda}, {0x93, 0x77}, {0x98, 0x8C}, {0x99, 0x23},
    {0x9c, 0x06}, {0x9d, 0xd6}, {0x9e, 0x00}, {0x9f, 0xfa},
    {0xb0, 0x10}, {0xb1, 0x18}, {0xb2, 0x60}, {0xb3, 0x18},
    {0xb4, 0x18}, {0xb5, 0x40}, {0xb6, 0x2c}, {0xb7, 0x25},
    {0xb8, 0x22}, {0xb9, 0x20}, {0xba, 0x1f}, {0xbb, 0x1e},
    {0xbc, 0x1d}, {0xbd, 0x1c}, {0xc0, 0x18}, {0xc3, 0x60},
    {0xc4, 0x58}, {0xc8, 0xb0}, {0xc9, 0x80}, {0x03, 0x22},
    {0x11, 0x2c}, {0x21, 0x01}, {0x30, 0x80}, {0x31, 0x80},
    {0x38, 0x12}, {0x39, 0x46}, {0x40, 0xf3}, {0x41, 0xaa},
    {0x42, 0x44}, {0x46, 0x09}, {0x80, 0x38}, {0x81, 0x20},
    {0x82, 0x38}, {0x83, 0x65}, {0x84, 0x16}, {0x85, 0x63},
    {0x86, 0x1a}, {0x87, 0x6e}, {0x88, 0x30}, {0x89, 0x4e},
    {0x8a, 0x1a}, {0x8b, 0x08}, {0x8d, 0x14}, {0x8e, 0x61},
    {0x8f, 0x50}, {0x90, 0x50}, {0x91, 0x50}, {0x92, 0x50},
    {0x93, 0x4c}, {0x94, 0x48}, {0x95, 0x45}, {0x96, 0x3e},
    {0x97, 0x35}, {0x98, 0x2e}, {0x99, 0x2a}, {0x9a, 0x2a},
    {0x9b, 0x08}, {0xb4, 0xea}, {0x03, 0x22}, {0x10, 0xeb},
    {0x03, 0x20}, {0x10, 0xdc}, {0x01, 0xf0},
};

static struct hi701_i2c_reg_conf hi701_effect_off_reg_config[] =
{
    {0x03, 0x10}, {0x11, 0x03}, {0x12, 0x30},
    {0x13, 0x00}, {0x44, 0x80}, {0x45, 0x80},
    {0x47, 0x7f}, {0x20, 0x00}, {0x21, 0x00},
};

static struct hi701_i2c_reg_conf hi701_effect_mono_reg_config[] =
{
    {0x03, 0x10}, {0x11, 0x03}, {0x12, 0x33},
    {0x13, 0x00}, {0x44, 0x80}, {0x45, 0x80},
    {0x47, 0x7f}, {0x20, 0x07}, {0x21, 0x03},
};

static struct hi701_i2c_reg_conf hi701_effect_negative_reg_config[] =
{
    {0x03, 0x10}, {0x11, 0x03}, {0x12, 0x38},
    {0x13, 0x00}, {0x44, 0x80}, {0x45, 0x80},
    {0x47, 0x7f}, {0x20, 0x07}, {0x21, 0x03},
};

static struct hi701_i2c_reg_conf hi701_effect_solarize_reg_config[] =
{
    {0x03, 0x10}, {0x11, 0x03}, {0x12, 0x30},
    {0x13, 0x01}, {0x44, 0x80}, {0x45, 0x80},
};

static struct hi701_i2c_reg_conf hi701_effect_sepia_reg_config[] =
{
    {0x03, 0x10}, {0x11, 0x03}, {0x12, 0x23},
    {0x13, 0x00}, {0x44, 0x40}, {0x45, 0x98},
    {0x47, 0x7f}, {0x20, 0x07}, {0x21, 0x03},
};

static struct hi701_i2c_reg_conf hi701_effect_aqua_reg_config[] =
{
    {0x03,0x10},  {0x11,0x03},  {0x12,0x23},
    {0x13,0x00},  {0x44,0xb0},  {0x45,0x40},
    {0x47,0x7f},  {0x20,0x07},  {0x21,0x03},
};

//awb
static struct hi701_i2c_reg_conf hi701_awb_auto_reg_config[] =
{
    {0x03, 0x22}, {0x10, 0x6a},
    {0x83, 0x58}, {0x84, 0x10},
    {0x85, 0x70}, {0x86, 0x10},
    {0x10, 0xea},
};

static struct hi701_i2c_reg_conf hi701_awb_incandescent_reg_config[] =
{
    {0x03, 0x22}, {0x10, 0x6a},
    {0x80, 0x25}, {0x81, 0x20},
    {0x82, 0x44}, {0x83, 0x24},
    {0x84, 0x1e}, {0x85, 0x50},
    {0x86, 0x45},
};
static struct hi701_i2c_reg_conf hi701_awb_fluorescent_reg_config[] =
{
    {0x03, 0x22}, {0x10, 0x6a},
    {0x80, 0x35}, {0x81, 0x20},
    {0x82, 0x32}, {0x83, 0x3c},
    {0x84, 0x2c}, {0x85, 0x45},
    {0x86, 0x35},
};
static struct hi701_i2c_reg_conf hi701_awb_daylight_reg_config[] =
{
    {0x03, 0x22}, {0x10, 0x6a},
    {0x80, 0x40}, {0x81, 0x20},
    {0x82, 0x28}, {0x83, 0x45},
    {0x84, 0x35}, {0x85, 0x2d},
    {0x86, 0x1c},
};
static struct hi701_i2c_reg_conf hi701_awb_cloudy_daylight_reg_config[] =
{
    {0x03, 0x22}, {0x10, 0x6a},
    {0x80, 0x40}, {0x81, 0x20},
    {0x82, 0x28}, {0x83, 0x45},
    {0x84, 0x35}, {0x85, 0x2d},
    {0x86, 0x1c},
};

static struct hi701_i2c_reg_conf hi701_awb_shade_reg_config[] =
{
    {0x03, 0x22}, {0x10, 0x6a},
    {0x80, 0x50}, {0x81, 0x20},
    {0x82, 0x24}, {0x83, 0x6d},
    {0x84, 0x65}, {0x85, 0x24},
    {0x86, 0x1c},
};
static struct  hi701_work_t *hi701sensorw = NULL;

static struct  i2c_client *hi701_client = NULL;
static struct hi701_ctrl_t *hi701_ctrl = NULL;

static DECLARE_WAIT_QUEUE_HEAD(hi701_wait_queue);
DEFINE_SEMAPHORE(hi701_sem);

/*static int hi701_i2c_rxdata(unsigned short saddr,
    unsigned char *rxdata, int length)
{
    struct i2c_msg msgs[] = {
    {
        .addr   = saddr,
        .flags = 0,
        .len   = 2,
        .buf   = rxdata,
    },
    {
        .addr  = saddr,
        .flags = I2C_M_RD,
        .len   = length,
        .buf   = rxdata,
    },
    };

    if (i2c_transfer(hi701_client->adapter, msgs, 2) < 0) {
        CDBG("hi701_i2c_rxdata failed!\n");
        return -EIO;
    }

    return 0;
}*/
static int32_t hi701_i2c_read_w(unsigned char raddr, unsigned char *rdata)
{
    unsigned char buf;

    struct i2c_msg msgs[] =
    {
        {
            .addr  = hi701_client->addr,
            .flags = 0,
            .len = 1,
            .buf = &buf,
        },
        {
            .addr  = hi701_client->addr,
            .flags = I2C_M_RD,
            .len = 1,
            .buf = &buf,
        },
    };

    buf = raddr;

    if (i2c_transfer(hi701_client->adapter, msgs, 2) < 0)
    {
        CDBG("ov7690_i2c_read failed!\n");
        return -EIO;
    }

    *rdata = buf;

    return 0;
}

static int32_t hi701_i2c_txdata(unsigned short saddr,
                                unsigned char *txdata, int length)
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

    if (i2c_transfer(hi701_client->adapter, msg, 1) < 0)
    {
        CDBG("hi701_i2c_txdata faild\n");
        return -EIO;
    }

    return 0;
}

static int32_t hi701_i2c_write_w(unsigned char waddr, unsigned char wdata)
{
    unsigned char buf[2];

    buf[0] = waddr;
    buf[1] = wdata;
    if (hi701_i2c_txdata(hi701_client->addr, buf, 2) < 0)
    {
        CDBG("hi701_client_i2c_write faild\n");
        return -EIO;
    }

    return 0;
}

static int32_t hi701_i2c_write_w_table(struct hi701_i2c_reg_conf const *reg_conf_tbl,
                                                                int num_of_items_in_table)
{
    int i;
    int32_t rc = -EFAULT;

    for (i = 0; i < num_of_items_in_table; i++)
    {
        rc = hi701_i2c_write_w(reg_conf_tbl->waddr, reg_conf_tbl->wdata);
        if (rc < 0)
        {
            break;
        }

        reg_conf_tbl++;
    }

    return rc;
}

int32_t hi701_set_default_focus(uint8_t af_step)
{
    int32_t rc = 0;

    return rc;
}

int32_t hi701_set_effect(int32_t effect)
{
    struct hi701_i2c_reg_conf const *reg_conf_tbl = NULL;
    int num_of_items_in_table = 0;
    long rc = 0;

    CDBG("hi701_set_effect: effect = %d\n", effect);
    switch (effect)
    {
    case CAMERA_EFFECT_OFF:
        reg_conf_tbl = hi701_effect_off_reg_config;
        num_of_items_in_table = HI701_ARRAY_SIZE(hi701_effect_off_reg_config);
        break;

    case CAMERA_EFFECT_MONO:
        reg_conf_tbl = hi701_effect_mono_reg_config;
        num_of_items_in_table = HI701_ARRAY_SIZE(hi701_effect_mono_reg_config);
        break;

    case CAMERA_EFFECT_NEGATIVE:
        reg_conf_tbl = hi701_effect_negative_reg_config;
        num_of_items_in_table = HI701_ARRAY_SIZE(hi701_effect_negative_reg_config);
        break;

    case CAMERA_EFFECT_SOLARIZE:
        reg_conf_tbl = hi701_effect_solarize_reg_config;
        num_of_items_in_table = HI701_ARRAY_SIZE(hi701_effect_solarize_reg_config);
        break;

    case CAMERA_EFFECT_SEPIA:
        reg_conf_tbl = hi701_effect_sepia_reg_config;
        num_of_items_in_table = HI701_ARRAY_SIZE(hi701_effect_sepia_reg_config);
        break;

    case CAMERA_EFFECT_AQUA:
        reg_conf_tbl = hi701_effect_aqua_reg_config;
        num_of_items_in_table = HI701_ARRAY_SIZE(hi701_effect_aqua_reg_config);
        break;

    default:
        return 0;
    }

    rc = hi701_i2c_write_w_table(reg_conf_tbl, num_of_items_in_table);
    return rc;
}

static long hi701_set_wb(int wb)
{
    struct hi701_i2c_reg_conf const *reg_wb_tbl = NULL;
    int num_items_in_table = 0;
    long rc = 0;

    CDBG("hi701_set_wb: wb = %d\n", wb);
    switch (wb)
    {
    case CAMERA_WB_AUTO:
        reg_wb_tbl = hi701_awb_auto_reg_config;
        num_items_in_table = HI701_ARRAY_SIZE(hi701_awb_auto_reg_config);
        break;

    case CAMERA_WB_INCANDESCENT:
        reg_wb_tbl = hi701_awb_incandescent_reg_config;
        num_items_in_table = HI701_ARRAY_SIZE(hi701_awb_incandescent_reg_config);
        break;

    case CAMERA_WB_CUSTOM:
    case CAMERA_WB_FLUORESCENT:
        reg_wb_tbl = hi701_awb_fluorescent_reg_config;
        num_items_in_table = HI701_ARRAY_SIZE(hi701_awb_fluorescent_reg_config);
        break;

    case CAMERA_WB_DAYLIGHT:
        reg_wb_tbl = hi701_awb_daylight_reg_config;
        num_items_in_table = HI701_ARRAY_SIZE(hi701_awb_daylight_reg_config);
        break;

    case CAMERA_WB_CLOUDY_DAYLIGHT:
        reg_wb_tbl = hi701_awb_cloudy_daylight_reg_config;
        num_items_in_table = HI701_ARRAY_SIZE(hi701_awb_cloudy_daylight_reg_config);
        break;

    case CAMERA_WB_TWILIGHT:
        return 0;
        break;

    case CAMERA_WB_SHADE:
        reg_wb_tbl = hi701_awb_shade_reg_config;
        num_items_in_table = HI701_ARRAY_SIZE(hi701_awb_shade_reg_config);
        break;

    default:
        return 0;
    }

    rc = hi701_i2c_write_w_table(reg_wb_tbl, num_items_in_table);

    return rc;
}

int32_t hi701_set_fps(struct fps_cfg    *fps)
{
    /* input is new fps in Q8 format */
    int32_t rc = 0;

    CDBG("hi701_set_fps\n");
    return rc;
}

int32_t hi701_write_exp_gain(uint16_t gain, uint32_t line)
{
    CDBG("hi701_write_exp_gain\n");
    return 0;
}

int32_t hi701_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
    int32_t rc = 0;

    CDBG("hi701_set_pict_exp_gain\n");

    mdelay(10);

    /* camera_timed_wait(snapshot_wait*exposure_ratio); */
    return rc;
}

int32_t hi701_setting(enum hi701_reg_update_t rupdate,
                      enum hi701_setting_t    rt)
{
    int32_t rc = 0;

    switch (rupdate)
    {
    case UPDATE_PERIODIC:

        break;

    case REG_INIT:
        CDBG("set init reg!\n");
        rc = hi701_i2c_write_w_table(hi701_init_reg_config_comm,
                                     HI701_ARRAY_SIZE(hi701_init_reg_config_comm));
        if (rc < 0)
        {
            return rc;
        }

        msleep(10);
        break;

    default:
        rc = -EFAULT;
        break;
    } /* switch (rupdate) */

    return rc;
}

int32_t hi701_video_config(int mode, int res)
{
    int32_t rc;

    switch (res)
    {
    case QTR_SIZE:
        rc = hi701_setting(UPDATE_PERIODIC, RES_PREVIEW);
        if (rc < 0)
        {
            return rc;
        }

        CDBG("sensor configuration done!\n");
        break;

    case FULL_SIZE:
        rc = hi701_setting(UPDATE_PERIODIC, RES_CAPTURE);
        if (rc < 0)
        {
            return rc;
        }

        break;

    default:
        return 0;
    } /* switch */

    hi701_ctrl->prev_res   = res;
    hi701_ctrl->curr_res   = res;
    hi701_ctrl->sensormode = mode;

    return rc;
}

int32_t hi701_snapshot_config(int mode)
{
    int32_t rc = 0;

    CDBG("hi701_snapshot_config in\n");
    rc = hi701_setting(UPDATE_PERIODIC, RES_CAPTURE);
    msleep(50);
    if (rc < 0)
    {
        return rc;
    }

    hi701_ctrl->curr_res = hi701_ctrl->pict_res;

    hi701_ctrl->sensormode = mode;

    return rc;
}

int32_t hi701_power_down(void)
{
    int32_t rc = 0;

    mdelay(5);

    return rc;
}

int32_t hi701_move_focus(int direction, int32_t num_steps)
{
    return 0;
}

static int hi701_sensor_init_done(const struct msm_camera_sensor_info *data)
{
    gpio_direction_output(data->sensor_reset, 0);
    gpio_free(data->sensor_reset);

    gpio_direction_output(data->sensor_pwd, 1);
    gpio_free(data->sensor_pwd);
    msleep(100);
    if (data->vreg_disable_func)
    {
        data->vreg_disable_func(0);
    }

    return 0;
}

static int hi701_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
    int rc;
    unsigned char chipid;

    /* pull down power down */
    rc = gpio_request(data->sensor_pwd, "hi701");
    if (!rc || (rc == -EBUSY))
    {
        gpio_direction_output(data->sensor_pwd, 0);
    }
    else
    {
        goto init_probe_fail;
    }

    rc = gpio_request(data->sensor_reset, "hi701");
    if (!rc)
    {
        rc = gpio_direction_output(data->sensor_reset, 0);
    }
    else
    {
        goto init_probe_fail;
    }

    mdelay(5);

    if (data->vreg_enable_func)
    {
        data->vreg_enable_func(1);
    }

    msleep(20);

    // if(data->master_init_control_slave == NULL
    //    || data->master_init_control_slave(data) != 0
    //    )
    {
        rc = gpio_direction_output(data->sensor_pwd, 1);
        if (rc < 0)
        {
            goto init_probe_fail;
        }

        msleep(20);

        /*hardware reset*/
        rc = gpio_direction_output(data->sensor_reset, 1);
        if (rc < 0)
        {
            goto init_probe_fail;
        }

        msleep(20);
    }

    //hi701 soft reset
    rc = hi701_i2c_write_w(0x03, 0x00);
    if (rc < 0)
    {
        CDBG("hi701_i2c_write_w 0x03 0x00 !! rc=%d", rc);
        goto init_probe_fail;
    }

    /* 3. Read sensor Model ID: */
    rc = hi701_i2c_read_w(HI701_REG_CHIP_ID, &chipid);
    if (rc < 0)
    {
        CDBG("hi701_i2c_read_w Model_ID failed!! rc=%d", rc);
        goto init_probe_fail;
    }

    CDBG("hi701 chipid = 0x%x\n", chipid);

    if (chipid != HI701_CHIP_ID)
    {
        rc = -ENODEV;
        CDBG("hi701 Model_ID error!!");
        goto init_probe_fail;
    }

    goto init_probe_done;

init_probe_fail:
    hi701_sensor_init_done(data);
init_probe_done:
    return rc;
}

int hi701_sensor_open_init(const struct msm_camera_sensor_info *data)
{
    int32_t rc;

    hi701_ctrl = kzalloc(sizeof(struct hi701_ctrl_t), GFP_KERNEL);
    if (!hi701_ctrl)
    {
        CDBG("hi701_sensor_open_init failed!\n");
        rc = -ENOMEM;
        goto init_done;
    }

    hi701_ctrl->fps_divider = 1 * 0x00000400;
    hi701_ctrl->pict_fps_divider = 1 * 0x00000400;
    hi701_ctrl->set_test = TEST_OFF;
    hi701_ctrl->prev_res = QTR_SIZE;
    hi701_ctrl->pict_res = FULL_SIZE;

    if (data)
    {
        hi701_ctrl->sensordata = data;
    }

    /* enable mclk first */
    msm_camio_clk_rate_set(HI701_DEFAULT_CLOCK_RATE);
    msleep(20);

    msm_camio_camif_pad_reg_reset();
    msleep(20);

    rc = hi701_probe_init_sensor(data);
    if (rc < 0)
    {
        goto init_fail;
    }

    if (hi701_ctrl->prev_res == QTR_SIZE)
    {
        rc = hi701_setting(REG_INIT, RES_PREVIEW);
    }
    else
    {
        rc = hi701_setting(REG_INIT, RES_CAPTURE);
    }

    if (rc < 0)
    {
        goto init_fail;
    }
    else
    {
        goto init_done;
    }

init_fail:
    kfree(hi701_ctrl);
init_done:
    return rc;
}

int hi701_init_client(struct i2c_client *client)
{
    /* Initialize the MSM_CAMI2C Chip */
    init_waitqueue_head(&hi701_wait_queue);
    return 0;
}

int32_t hi701_set_sensor_mode(int mode, int res)
{
    int32_t rc = 0;

    switch (mode)
    {
    case SENSOR_PREVIEW_MODE:
        CDBG("SENSOR_PREVIEW_MODE\n");
        rc = hi701_video_config(mode, res);
        break;

    case SENSOR_SNAPSHOT_MODE:
    case SENSOR_RAW_SNAPSHOT_MODE:
        CDBG("SENSOR_SNAPSHOT_MODE\n");
        rc = hi701_snapshot_config(mode);
        break;

    default:
        rc = -EINVAL;
        break;
    }

    return rc;
}

int hi701_sensor_config(void __user *argp)
{
    struct sensor_cfg_data cdata;
    long rc = 0;

    if (copy_from_user(&cdata,
                       (void *)argp,
                       sizeof(struct sensor_cfg_data)))
    {
        return -EFAULT;
    }

    down(&hi701_sem);

    CDBG("hi701_sensor_config: cfgtype = %d\n",
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
        rc = hi701_set_fps(&(cdata.cfg.fps));
        break;

    case CFG_SET_EXP_GAIN:
        rc =
            hi701_write_exp_gain(
            cdata.cfg.exp_gain.gain,
            cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_PICT_EXP_GAIN:
        rc =
            hi701_set_pict_exp_gain(
            cdata.cfg.exp_gain.gain,
            cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_MODE:
        rc = hi701_set_sensor_mode(cdata.mode,
                                   cdata.rs);
        break;

    case CFG_PWR_DOWN:
        rc = hi701_power_down();
        break;

    case CFG_MOVE_FOCUS:
        rc =
            hi701_move_focus(
            cdata.cfg.focus.dir,
            cdata.cfg.focus.steps);
        break;

    case CFG_SET_DEFAULT_FOCUS:
        rc =
            hi701_set_default_focus(
            cdata.cfg.focus.steps);
        break;

    case CFG_SET_EFFECT:
        rc = hi701_set_effect(
            cdata.cfg.effect);
        break;

    case CFG_SET_WB:
        rc = hi701_set_wb(
            cdata.cfg.effect);
        break;

    case CFG_MAX:
        if (copy_to_user((void *)(cdata.cfg.pict_max_exp_lc),
                         hi701_supported_effect,
                         HI701_ARRAY_SIZE(hi701_supported_effect)))
        {
            CDBG("copy hi701_supported_effect to user fail\n");
            rc = -EFAULT;
        }
        else
        {
            rc = 0;
        }

        break;

    default:
        rc = -EFAULT;
        break;
    }

    up(&hi701_sem);

    return rc;
}

int hi701_sensor_release(void)
{
    int rc = -EBADF;

    down(&hi701_sem);

    hi701_power_down();

    hi701_sensor_init_done(hi701_ctrl->sensordata);

    kfree(hi701_ctrl);

    up(&hi701_sem);
    CDBG("hi701_release completed!\n");
    return rc;
}

static int hi701_i2c_probe(struct i2c_client *         client,
                           const struct i2c_device_id *id)
{
    int rc = 0;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        rc = -ENOTSUPP;
        goto probe_failure;
    }

    hi701sensorw =
        kzalloc(sizeof(struct hi701_work_t), GFP_KERNEL);

    if (!hi701sensorw)
    {
        rc = -ENOMEM;
        goto probe_failure;
    }

    i2c_set_clientdata(client, hi701sensorw);
    hi701_init_client(client);
    hi701_client = client;

    //hi701_client->addr = hi701_client->addr >> 1;
    msleep(50);

    CDBG("i2c probe ok\n");
    return 0;

probe_failure:
    kfree(hi701sensorw);
    hi701sensorw = NULL;
    pr_err("i2c probe failure %d\n", rc);
    return rc;
}

static const struct i2c_device_id hi701_i2c_id[] =
{
    { "hi701", 0},
    { }
};

static struct i2c_driver hi701_i2c_driver =
{
    .id_table = hi701_i2c_id,
    .probe    = hi701_i2c_probe,
    .remove   = __exit_p(hi701_i2c_remove),
    .driver   = {
        .name = "hi701",
    },
};

static int hi701_sensor_probe(const struct msm_camera_sensor_info *info,
                              struct msm_sensor_ctrl *             s)
{
    /* We expect this driver to match with the i2c device registered
     * in the board file immediately. */
    int rc = i2c_add_driver(&hi701_i2c_driver);

    if ((rc < 0) || (hi701_client == NULL))
    {
        rc = -ENOTSUPP;
        goto probe_done;
    }

    /* enable mclk first */
    msm_camio_clk_rate_set(HI701_DEFAULT_CLOCK_RATE);
    msleep(20);

    rc = hi701_probe_init_sensor(info);

    /*probe failed*/
    if (rc < 0)
    {
        i2c_del_driver(&hi701_i2c_driver);
        CDBG("camera sensor hi701 probe is failed!!!\n");
        goto probe_done;
    }
    /*probe succeed*/
    else
    {
        CDBG("camera sensor hi701 probe is succeed!!!\n");
    }

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
    /* detect current device successful, set the flag as present */
    set_hw_dev_flag(DEV_I2C_CAMERA_MAIN);
#endif
    s->s_init = hi701_sensor_open_init;
    s->s_release = hi701_sensor_release;
    s->s_config = hi701_sensor_config;
    s->s_camera_type = FRONT_CAMERA_2D;
    s->s_mount_angle = 0;
    hi701_sensor_init_done(info);

    //set_camera_support(true);
probe_done:
    return rc;
}

static int __hi701_probe(struct platform_device *pdev)
{
    return msm_camera_drv_start(pdev, hi701_sensor_probe);
}

static struct platform_driver msm_camera_driver =
{
    .probe     = __hi701_probe,
    .driver    = {
        .name  = "msm_camera_hi701",
        .owner = THIS_MODULE,
    },
};

static int __init hi701_init(void)
{
    return platform_driver_register(&msm_camera_driver);
}

module_init(hi701_init);

