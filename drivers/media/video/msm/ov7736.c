
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
#include "ov7736.h"

#include "linux/hardware_self_adapt.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
 #include <linux/hw_dev_dec.h>
#endif
#undef CDBG
#define CDBG(fmt, args...) printk(KERN_ERR "ov7736.c: " fmt, ## args)

/*=============================================================
    SENSOR REGISTER DEFINES
==============================================================*/
#define OV7736_REG_CHIP_ID_LOW 0x300b
#define OV7736_REG_CHIP_ID_HIGH 0x300a
#define OV7736_CHIP_ID 0x7736
#define OV7736_REG_RESET_REGISTER 0x0022

enum ov7736_test_mode_t
{
    TEST_OFF,
    TEST_1,
    TEST_2,
    TEST_3
};

enum ov7736_resolution_t
{
    QTR_SIZE,
    FULL_SIZE,
    INVALID_SIZE
};

enum ov7736_reg_update_t
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

enum ov7736_setting_t
{
    RES_PREVIEW,
    RES_CAPTURE
};

/* for 30 fps preview */
#define OV7736_DEFAULT_CLOCK_RATE 24000000

#define OV7736_ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

/* FIXME: Changes from here */
struct ov7736_work_t
{
    struct work_struct work;
};

struct ov7736_ctrl_t
{
    const struct  msm_camera_sensor_info *sensordata;

    int      sensormode;
    uint32_t fps_divider; /* init to 1 * 0x00000400 */
    uint32_t pict_fps_divider; /* init to 1 * 0x00000400 */

    uint16_t curr_lens_pos;
    uint16_t init_curr_lens_pos;
    uint16_t my_reg_gain;
    uint32_t my_reg_line_count;

    enum ov7736_resolution_t prev_res;
    enum ov7736_resolution_t pict_res;
    enum ov7736_resolution_t curr_res;
    enum ov7736_test_mode_t  set_test;

    unsigned short imgaddr;
};

struct ov7736_i2c_reg_conf
{
    unsigned short waddr;
    unsigned char  wdata;
};

static struct ov7736_i2c_reg_conf ov7736_init_reg_config_comm[] =
{
    /*******************************************************
     * setting based on ¡°OV7736_demo_A06.db¡±
     * status after initialization
     * light mode: auto
     * color saturation: 0
     * EV: 0
     * contrast: 0
     * brightness: 0
     * effect: normal
     * MCLK = 24Mh, PCLK = 24Mhz, 30fps, night mode off
     * banding: manual 50Hz
     * Output sequence YUYV
     *********************************************************/

    //SCCB_slave_Address = 0x78;
    {0x3008, 0x82}, // software reset
    // delay 5ms here
    {0x3008, 0x42}, // software power down
    {0x3630, 0x11},
    {0x3104, 0x03},
    {0x3017, 0x7f}, // output enable
    {0x3018, 0xfc}, // output enable
    {0x3600, 0x1c},
    {0x3602, 0x04},
    {0x3611, 0x44},
    {0x3612, 0x63},
    {0x3631, 0x22},
    {0x3622, 0x00}, // binning
    {0x3633, 0x25},
    {0x370b, 0x43},
    {0x401c, 0x00},
    {0x401e, 0x11},
    {0x4702, 0x01},
    {0x3a00, 0x7a}, // night mode off
    {0x3a18, 0x00}, // gain ceiling
    {0x3a19, 0x3f}, // gain ceiling
    {0x300f, 0x88}, // PLL
    {0x3011, 0x08}, // PLL
    //--- format ---
    {0x4303, 0xff}, // Y max
    {0x4307, 0xff}, // U max
    {0x430b, 0xff}, // V max
    {0x4305, 0x00}, // Y min
    {0x4309, 0x00}, // U min
    {0x430d, 0x00}, // V min
    {0x4001, 0x02},
    {0x4004, 0x06},

    //--- timing ---
    {0x3800, 0x00}, // H start
    {0x3801, 0x8e}, // H start
    {0x3810, 0x08}, // H off
    {0x3811, 0x02}, // V off
    {0x380c, 0x03}, // HTS
    {0x380d, 0x20}, // HTS
    {0x380e, 0x01}, // VTS
    {0x380f, 0xf4}, // VTS
    //--- banding filter ---
    {0x3a09, 0x96}, // B50 step
    {0x3a0b, 0x7d}, // B60 step
    {0x4300, 0x30}, // YUV 422
    {0x501f, 0x01}, // YUV 422
    {0x5000, 0x4f}, // ISP lenc off, gamma on, awb gain on, white pixel on, black pixel on,

    {0x5001, 0x47}, // ISP SDE on, UV average on, color matrix on, awb on
    {0x370d, 0x0b}, // vertical binning
    {0x3715, 0x1a},
    {0x370e, 0x00},
    {0x3713, 0x08},
    {0x3703, 0x2c},
    {0x3620, 0xc2},
    {0x3714, 0x36},
    {0x3716, 0x01},
    {0x3623, 0x03},

    //--- 50/60 ---
    {0x3c00, 0x04}, // manual 50hz
    {0x3c01, 0xb2}, // auto detection off
    {0x3c04, 0x12}, // detection TH sum1
    {0x3c05, 0x60}, // detection TH sum2
    {0x3c06, 0x00}, // TH luminance 1
    {0x3c07, 0x20}, // TH luminance 1
    {0x3c08, 0x00}, // TH luminance 2
    {0x3c09, 0xc2}, // TH luminance 2
    {0x300d, 0x22},
    {0x3c0a, 0x9c}, // sample number
    {0x3c0b, 0x40}, // sample number
    {0x3008, 0x02}, // wake up from software power down
    {0x5180, 0x02}, // awb
    {0x5181, 0x02}, // awb

    //--- AE target ---
    {0x3a0f, 0x3c}, // stable in high
    {0x3a10, 0x34}, // stable in low
    {0x3a1b, 0x3c}, // stable out high
    {0x3a1e, 0x34}, // stable out low
    {0x3a11, 0x70}, // fast zone high
    {0x3a1f, 0x18}, // fast zone low
    {0x5000, 0xcf}, // ISP lenc on, gamma on, awb gain on, white pixel on, black pixel on,

    //--- gamma ---
    {0x5481, 0x0a},
    {0x5482, 0x13},
    {0x5483, 0x23},
    {0x5484, 0x40},
    {0x5485, 0x4d},
    {0x5486, 0x58},
    {0x5487, 0x64},
    {0x5488, 0x6e},
    {0x5489, 0x78},
    {0x548a, 0x81},
    {0x548b, 0x92},
    {0x548c, 0xa1},
    {0x548d, 0xbb},
    {0x548e, 0xcf},
    {0x548f, 0xe3},
    {0x5490, 0x26},

    //--- color matrix ---
    {0x5380, 0x42},
    {0x5381, 0x33},
    {0x5382, 0x0f},
    {0x5383, 0x0b},
    {0x5384, 0x42},
    {0x5385, 0x4d},
    {0x5392, 0x1e},

    //--- lens correction ---
    //--- Sekonix Lens ---
    {0x5801, 0x00},
    {0x5802, 0x50},
    {0x5803, 0x40},
    {0x5804, 0x1c},
    {0x5805, 0x12},
    {0x5806, 0x10},

    //--- special effects ---
    {0x5001, 0xc7},
    {0x5580, 0x06},
    {0x5583, 0x40},
    {0x5584, 0x26},
    {0x5585, 0x20},
    {0x5589, 0x10},
    {0x558a, 0x00},
    {0x558b, 0x3e},

    //--- shaprness & de-noise ---
    {0x5300, 0x0f}, // sharpen mt th1
    {0x5301, 0x30}, // sharpen mt th2
    {0x5302, 0x0d}, // sharpen offset 1
    {0x5303, 0x02}, // sharpen offset 2
    {0x5304, 0x0e}, // de-noise th1
    {0x5305, 0x30}, // de-noise th2
    {0x5306, 0x06}, // de-noise offset 1
    {0x5307, 0x40}, // de-noise offset 2
    {0x5680, 0x00},
    {0x5681, 0x50},
    {0x5682, 0x00},
    {0x5683, 0x3c},
    {0x5684, 0x11},
    {0x5685, 0xe0},
    {0x5686, 0x0d},
    {0x5687, 0x68},
    {0x5688, 0x03},
    {0x3008, 0x02},
    {0x4708, 0x03},
    {0x3818, 0x40}, //mirror and flip
};

/*add the effect setting*/
static struct ov7736_i2c_reg_conf ov7736_effect_off_reg_config[] =
{
    {0x5580,0x00},
};

static struct ov7736_i2c_reg_conf ov7736_effect_mono_reg_config[] =
{
    {0x5580,0x18},
    {0x5583,0x80},    
    {0x5584,0x80}, 

};

static struct ov7736_i2c_reg_conf ov7736_effect_negative_reg_config[] =
{
    {0x5580,0x40},

};
static struct ov7736_i2c_reg_conf ov7736_effect_solarize_reg_config[] =
{
    {0x5580,0x18},
    {0x5583,0x80},
    {0x5584,0xc0},

};
static struct ov7736_i2c_reg_conf ov7736_effect_sepia_reg_config[] =
{
    {0x5580,0x18},
    {0x5583,0x40},
    {0x5584,0xa0},
};

static struct ov7736_i2c_reg_conf ov7736_effect_aqua_reg_config[] =
{
    {0x5580,0x18},
    {0x5583,0x60},
    {0x5584,0x60},

};

/*add the wb setting*/
static struct ov7736_i2c_reg_conf ov7736_WB_auto_reg_config[] =
{
    {0x5186,0x02},

};
static struct ov7736_i2c_reg_conf ov7736_wb_incandescent_reg_config[] =
{
    {0x5186,0x03}, 
    {0x504e,0x06}, 
    {0x504f,0x2a},
    {0x5050,0x04}, 
    {0x5051,0x00},
    {0x5052,0x07}, 
    {0x5053,0x24},

};
static struct ov7736_i2c_reg_conf ov7736_wb_fluorescent_reg_config[] =
{
    {0x5186,0x03}, 
    {0x504e,0x04}, 
    {0x504f,0x58},
    {0x5050,0x04}, 
    {0x5051,0x00},
    {0x5052,0x08}, 
    {0x5053,0x40},

};
static struct ov7736_i2c_reg_conf ov7736_wb_daylight_reg_config[] =
{
    {0x5186,0x03}, 
    {0x504e,0x07}, 
    {0x504f,0x02},
    {0x5050,0x04}, 
    {0x5051,0x00},
    {0x5052,0x05}, 
    {0x5053,0x15},

};
static struct ov7736_i2c_reg_conf ov7736_wb_cloudy_reg_config[] =
{
    {0x5186,0x03}, 
    {0x504e,0x07}, 
    {0x504f,0x88},
    {0x5050,0x04}, 
    {0x5051,0x00},
    {0x5052,0x05}, 
    {0x5053,0x00},

};

static struct  ov7736_work_t *ov7736sensorw = NULL;

static struct  i2c_client *ov7736_client = NULL;
static struct ov7736_ctrl_t *ov7736_ctrl = NULL;

static DECLARE_WAIT_QUEUE_HEAD(ov7736_wait_queue);
DEFINE_SEMAPHORE(ov7736_sem);

static int ov7736_i2c_rxdata(unsigned short saddr,
                             unsigned char *rxdata, int length)
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

    if (i2c_transfer(ov7736_client->adapter, msgs, 2) < 0)
    {
        CDBG("ov7736_i2c_rxdata failed!\n");
        return -EIO;
    }

    return 0;
}

static int32_t ov7736_i2c_read_w(unsigned short raddr, unsigned char *rdata)
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

    rc = ov7736_i2c_rxdata(ov7736_client->addr, buf, 1);
    if (rc < 0)
    {
        return rc;
    }

    *rdata = buf[0];

    if (rc < 0)
    {
        CDBG("ov7736_i2c_read failed!\n");
    }

    return rc;
}

static int32_t ov7736_i2c_txdata(unsigned short saddr,
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

    if (i2c_transfer(ov7736_client->adapter, msg, 1) < 0)
    {
        CDBG("ov7736_i2c_txdata faild\n");
        return -EIO;
    }

    return 0;
}

static int32_t ov7736_i2c_write_w(unsigned short waddr, unsigned char wdata)
{
    int32_t rc = -EFAULT;
    unsigned char buf[3];
    int32_t i = 0;

    memset(buf, 0, sizeof(buf));
    buf[0] = (waddr & 0xFF00) >> 8;
    buf[1] = (waddr & 0x00FF);
    buf[2] = wdata;

    //  buf[3] = (wdata & 0x00FF);

    /*write three times, if error, return -EIO*/
    for (i = 0; i < 10; i++)
    {
        rc = ov7736_i2c_txdata(ov7736_client->addr, buf, 3);
        if (0 <= rc)
        {
            return 0;
        }
    }

    if (10 == i)
    {
        CDBG("i2c_write_w failed, addr = 0x%x, val = 0x%x!\n", waddr, wdata);
        return -EIO;
    }

    return 0;
}

static int32_t ov7736_i2c_write_w_table(struct ov7736_i2c_reg_conf const *reg_conf_tbl,
                                        int                               num_of_items_in_table)
{
    int i;
    int32_t rc = -EFAULT;

    for (i = 0; i < num_of_items_in_table; i++)
    {
        rc = ov7736_i2c_write_w(reg_conf_tbl->waddr, reg_conf_tbl->wdata);
        if (rc < 0)
        {
            break;
        }

        reg_conf_tbl++;
    }

    return rc;
}

int32_t ov7736_set_default_focus(uint8_t af_step)
{
    int32_t rc = 0;

    return rc;
}

/*add the effect setting*/
int32_t ov7736_set_effect(int32_t effect)
{
	struct ov7736_i2c_reg_conf  *reg_conf_tbl = NULL;
    int num_of_items_in_table = 0;
	long rc = 0;
    
	switch (effect) {
	case CAMERA_EFFECT_OFF:
        reg_conf_tbl = ov7736_effect_off_reg_config;
        num_of_items_in_table = OV7736_ARRAY_SIZE(ov7736_effect_off_reg_config);
        break;

	case CAMERA_EFFECT_MONO:
        reg_conf_tbl = ov7736_effect_mono_reg_config;
        num_of_items_in_table = OV7736_ARRAY_SIZE(ov7736_effect_mono_reg_config);
		break;

	case CAMERA_EFFECT_NEGATIVE:
        reg_conf_tbl = ov7736_effect_negative_reg_config;
        num_of_items_in_table = OV7736_ARRAY_SIZE(ov7736_effect_negative_reg_config);
		break;

	case CAMERA_EFFECT_SOLARIZE:
        reg_conf_tbl = ov7736_effect_solarize_reg_config;
        num_of_items_in_table = OV7736_ARRAY_SIZE(ov7736_effect_solarize_reg_config);
		break;

	case CAMERA_EFFECT_SEPIA:
        reg_conf_tbl = ov7736_effect_sepia_reg_config;
        num_of_items_in_table = OV7736_ARRAY_SIZE(ov7736_effect_sepia_reg_config);
		break;
        
	case CAMERA_EFFECT_AQUA:
        reg_conf_tbl = ov7736_effect_aqua_reg_config;
        num_of_items_in_table = OV7736_ARRAY_SIZE(ov7736_effect_aqua_reg_config);
		break;
              
	default: 
		return 0;
	}

    rc = ov7736_i2c_write_w_table(reg_conf_tbl, num_of_items_in_table);
    return rc;

}
/*add the wb setting*/
int32_t ov7736_set_wb(int32_t wb)
{
	struct ov7736_i2c_reg_conf *reg_conf_tbl = NULL;
    int num_of_items_in_table = 0;
	long rc = 0;
    
	switch (wb) {
	case CAMERA_WB_AUTO:
        reg_conf_tbl = ov7736_WB_auto_reg_config;
        num_of_items_in_table = OV7736_ARRAY_SIZE(ov7736_WB_auto_reg_config);
        break;

	case CAMERA_WB_INCANDESCENT:
        reg_conf_tbl = ov7736_wb_incandescent_reg_config;
        num_of_items_in_table = OV7736_ARRAY_SIZE(ov7736_wb_incandescent_reg_config);
		break;

	case CAMERA_WB_CUSTOM:       
	case CAMERA_WB_FLUORESCENT:
        reg_conf_tbl = ov7736_wb_fluorescent_reg_config;
        num_of_items_in_table = OV7736_ARRAY_SIZE(ov7736_wb_fluorescent_reg_config);
		break;

	case CAMERA_WB_DAYLIGHT:
        reg_conf_tbl = ov7736_wb_daylight_reg_config;
        num_of_items_in_table = OV7736_ARRAY_SIZE(ov7736_wb_daylight_reg_config);
		break;
        
	case CAMERA_WB_CLOUDY_DAYLIGHT:
        reg_conf_tbl = ov7736_wb_cloudy_reg_config;
        num_of_items_in_table = OV7736_ARRAY_SIZE(ov7736_wb_cloudy_reg_config);
		break;
              
	default: 
		return 0;
	}

    rc = ov7736_i2c_write_w_table(reg_conf_tbl, num_of_items_in_table);
    return rc;

}
int32_t ov7736_set_fps(struct fps_cfg    *fps)
{
    /* input is new fps in Q8 format */
    int32_t rc = 0;

    CDBG("ov7736_set_fps\n");
    return rc;
}

int32_t ov7736_write_exp_gain(uint16_t gain, uint32_t line)
{
    CDBG("ov7736_write_exp_gain\n");
    return 0;
}

int32_t ov7736_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
    int32_t rc = 0;

    CDBG("ov7736_set_pict_exp_gain\n");

    mdelay(10);

    /* camera_timed_wait(snapshot_wait*exposure_ratio); */
    return rc;
}

int32_t ov7736_setting(enum ov7736_reg_update_t rupdate,
                       enum ov7736_setting_t    rt)
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
        rc = ov7736_i2c_write_w_table(ov7736_init_reg_config_comm,
                                      OV7736_ARRAY_SIZE(ov7736_init_reg_config_comm));

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

int32_t ov7736_video_config(int mode, int res)
{
    int32_t rc;

    switch (res)
    {
    case QTR_SIZE:
        rc = ov7736_setting(UPDATE_PERIODIC, RES_PREVIEW);
        if (rc < 0)
        {
            return rc;
        }

        CDBG("sensor configuration done!\n");
        break;

    case FULL_SIZE:
        rc = ov7736_setting(UPDATE_PERIODIC, RES_CAPTURE);
        if (rc < 0)
        {
            return rc;
        }

        break;

    default:
        return 0;
    } /* switch */

    ov7736_ctrl->prev_res   = res;
    ov7736_ctrl->curr_res   = res;
    ov7736_ctrl->sensormode = mode;

    return rc;
}

int32_t ov7736_snapshot_config(int mode)
{
    int32_t rc = 0;

    CDBG("ov7736_snapshot_config in\n");
    rc = ov7736_setting(UPDATE_PERIODIC, RES_CAPTURE);
    msleep(50);
    if (rc < 0)
    {
        return rc;
    }

    ov7736_ctrl->curr_res = ov7736_ctrl->pict_res;

    ov7736_ctrl->sensormode = mode;

    return rc;
}

int32_t ov7736_power_down(void)
{
    int32_t rc = 0;

    mdelay(5);

    return rc;
}

int32_t ov7736_move_focus(int direction, int32_t num_steps)
{
    return 0;
}

static int ov7736_sensor_init_done(const struct msm_camera_sensor_info *data)
{
    gpio_direction_output(data->sensor_reset, 0);
    gpio_free(data->sensor_reset);
    gpio_direction_output(data->sensor_pwd, 1);
    gpio_free(data->sensor_pwd);
    mdelay(5);

    if (data->vreg_disable_func)
    {
        data->vreg_disable_func(0);
    }

    return 0;
}

static int ov7736_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
    int rc;
    unsigned short chipid;
    unsigned char chiplow, chiphigh;

    /* pull down power down */
    rc = gpio_request(data->sensor_pwd, "ov7736");
    if (!rc || (rc == -EBUSY))
    {
        gpio_direction_output(data->sensor_pwd, 1);
    }
    else
    {
        goto init_probe_fail;
    }

    rc = gpio_request(data->sensor_reset, "ov7736");
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

    mdelay(5);
    rc = gpio_direction_output(data->sensor_pwd, 0);
    if (rc < 0)
    {
        goto init_probe_fail;
    }

    mdelay(20);

    /*hardware reset*/
    rc = gpio_direction_output(data->sensor_reset, 1);
    if (rc < 0)
    {
        goto init_probe_fail;
    }

    mdelay(20);

    /* 3. Read sensor Model ID: */
    rc = ov7736_i2c_read_w(OV7736_REG_CHIP_ID_LOW, &chiplow);
    CDBG("ov7736 chipidlow = 0x%x\n", chiplow);
    if (rc < 0)
    {
        CDBG("ov7736_i2c_read_w Model_ID failed!! rc=%d", rc);
        goto init_probe_fail;
    }

    rc = ov7736_i2c_read_w(OV7736_REG_CHIP_ID_HIGH, &chiphigh);
    CDBG("ov7736 chipidhigh = 0x%x\n", chiphigh);
    if (rc < 0)
    {
        CDBG("ov7736_i2c_read_w Model_ID failed!! rc=%d", rc);
        goto init_probe_fail;
    }

    chipid = (chiphigh << 8) | chiplow;
    CDBG("ov7736 chipid = 0x%x\n", chipid);

    if (chipid != OV7736_CHIP_ID)
    {
        rc = -ENODEV;
        CDBG("ov7736 Model_ID error!!");
        goto init_probe_fail;
    }

    goto init_probe_done;

init_probe_fail:
    ov7736_sensor_init_done(data);
init_probe_done:
    return rc;
}

int ov7736_sensor_open_init(const struct msm_camera_sensor_info *data)
{
    int32_t rc;

    ov7736_ctrl = kzalloc(sizeof(struct ov7736_ctrl_t), GFP_KERNEL);
    if (!ov7736_ctrl)
    {
        CDBG("ov7736_sensor_open_init failed!\n");
        rc = -ENOMEM;
        goto init_done;
    }

    ov7736_ctrl->fps_divider = 1 * 0x00000400;
    ov7736_ctrl->pict_fps_divider = 1 * 0x00000400;
    ov7736_ctrl->set_test = TEST_OFF;
    ov7736_ctrl->prev_res = QTR_SIZE;
    ov7736_ctrl->pict_res = FULL_SIZE;

    if (data)
    {
        ov7736_ctrl->sensordata = data;
    }

    /* enable mclk first */
    msm_camio_clk_rate_set(OV7736_DEFAULT_CLOCK_RATE);
    msleep(20);

    msm_camio_camif_pad_reg_reset();
    msleep(20);

    rc = ov7736_probe_init_sensor(data);
    if (rc < 0)
    {
        goto init_fail;
    }

    if (ov7736_ctrl->prev_res == QTR_SIZE)
    {
        rc = ov7736_setting(REG_INIT, RES_PREVIEW);
    }
    else
    {
        rc = ov7736_setting(REG_INIT, RES_CAPTURE);
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
    kfree(ov7736_ctrl);
init_done:
    return rc;
}

int ov7736_init_client(struct i2c_client *client)
{
    /* Initialize the MSM_CAMI2C Chip */
    init_waitqueue_head(&ov7736_wait_queue);
    return 0;
}

int32_t ov7736_set_sensor_mode(int mode, int res)
{
    int32_t rc = 0;

    switch (mode)
    {
    case SENSOR_PREVIEW_MODE:
        CDBG("SENSOR_PREVIEW_MODE\n");
        rc = ov7736_video_config(mode, res);
        break;

    case SENSOR_SNAPSHOT_MODE:
    case SENSOR_RAW_SNAPSHOT_MODE:
        CDBG("SENSOR_SNAPSHOT_MODE\n");
        rc = ov7736_snapshot_config(mode);
        break;

    default:
        rc = -EINVAL;
        break;
    }

    return rc;
}

int ov7736_sensor_config(void __user *argp)
{
    struct sensor_cfg_data cdata;
    long rc = 0;

    if (copy_from_user(&cdata,
                       (void *)argp,
                       sizeof(struct sensor_cfg_data)))
    {
        return -EFAULT;
    }

    down(&ov7736_sem);

    CDBG("ov7736_sensor_config: cfgtype = %d\n",
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
        rc = ov7736_set_fps(&(cdata.cfg.fps));
        break;

    case CFG_SET_EXP_GAIN:
        rc =
            ov7736_write_exp_gain(
            cdata.cfg.exp_gain.gain,
            cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_PICT_EXP_GAIN:
        rc =
            ov7736_set_pict_exp_gain(
            cdata.cfg.exp_gain.gain,
            cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_MODE:
        rc = ov7736_set_sensor_mode(cdata.mode,
                                    cdata.rs);
        break;

    case CFG_PWR_DOWN:
        rc = ov7736_power_down();
        break;

    case CFG_MOVE_FOCUS:
        rc =
            ov7736_move_focus(
            cdata.cfg.focus.dir,
            cdata.cfg.focus.steps);
        break;

    case CFG_SET_DEFAULT_FOCUS:
        rc =
            ov7736_set_default_focus(
            cdata.cfg.focus.steps);
        break;

    case CFG_SET_EFFECT:
        rc = ov7736_set_effect(
            cdata.cfg.effect);
        break;

    case CFG_SET_WB:
        rc = ov7736_set_wb(
            cdata.cfg.effect);
        break;

    default:
        rc = -EFAULT;
        break;
    }

    up(&ov7736_sem);

    return rc;
}

int ov7736_sensor_release(void)
{
    int rc = -EBADF;

    down(&ov7736_sem);

    ov7736_power_down();

    ov7736_sensor_init_done(ov7736_ctrl->sensordata);

    kfree(ov7736_ctrl);

    up(&ov7736_sem);
    CDBG("ov7736_release completed!\n");
    return rc;
}

static int ov7736_i2c_probe(struct i2c_client *         client,
                            const struct i2c_device_id *id)
{
    int rc = 0;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        rc = -ENOTSUPP;
        goto probe_failure;
    }

    ov7736sensorw =
        kzalloc(sizeof(struct ov7736_work_t), GFP_KERNEL);
    if (!ov7736sensorw)
    {
        rc = -ENOMEM;
        goto probe_failure;
    }

    i2c_set_clientdata(client, ov7736sensorw);
    ov7736_init_client(client);
    ov7736_client = client;
    client->addr = 0x3C;

    //ov7736_client->addr = ov7736_client->addr >> 1;
    msleep(50);
    CDBG("i2c probe ok\n");
    return 0;

probe_failure:
    kfree(ov7736sensorw);
    ov7736sensorw = NULL;
    pr_err("i2c probe failure %d\n", rc);
    return rc;
}

static const struct i2c_device_id ov7736_i2c_id[] =
{
    { "ov7736", 0},
    { }
};

static struct i2c_driver ov7736_i2c_driver =
{
    .id_table = ov7736_i2c_id,
    .probe    = ov7736_i2c_probe,
    .remove   = __exit_p(ov7736_i2c_remove),
    .driver   = {
        .name = "ov7736",
    },
};

static int ov7736_sensor_probe(const struct msm_camera_sensor_info *info,
                               struct msm_sensor_ctrl *             s)
{
    /* We expect this driver to match with the i2c device registered
     * in the board file immediately. */
    int rc = i2c_add_driver(&ov7736_i2c_driver);

    if ((rc < 0) || (ov7736_client == NULL))
    {
        rc = -ENOTSUPP;
        goto probe_done;
    }

    /* enable mclk first */
    msm_camio_clk_rate_set(OV7736_DEFAULT_CLOCK_RATE);
    msleep(20);

    CDBG("ov7736_sensor_probe!!!!!!!!!!!!!\n");
    rc = ov7736_probe_init_sensor(info);

    /*probe failed*/
    if (rc < 0)
    {
        i2c_del_driver(&ov7736_i2c_driver);
        CDBG("camera sensor ov7736 probe is failed!!!\n");
        goto probe_done;
    }
    /*probe succeed*/
    else
    {
        CDBG("camera sensor ov7736 probe is succeed!!!\n");
    }

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
    /* detect current device successful, set the flag as present */
    set_hw_dev_flag(DEV_I2C_CAMERA_SLAVE);
#endif
    s->s_init = ov7736_sensor_open_init;
    s->s_release = ov7736_sensor_release;
    s->s_config = ov7736_sensor_config;
    s->s_camera_type = FRONT_CAMERA_2D;
    s->s_mount_angle = 0;
    ov7736_sensor_init_done(info);

probe_done:
    return rc;
}

static int __ov7736_probe(struct platform_device *pdev)
{
    return msm_camera_drv_start(pdev, ov7736_sensor_probe);
}

static struct platform_driver msm_camera_driver =
{
    .probe     = __ov7736_probe,
    .driver    = {
        .name  = "msm_camera_ov7736",
        .owner = THIS_MODULE,
    },
};

static int __init ov7736_init(void)
{
    return platform_driver_register(&msm_camera_driver);
}

module_init(ov7736_init);

