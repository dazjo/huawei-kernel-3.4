
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
#include <linux/slab.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/camera.h>
#include "s5k5ca.h"
#include "linux/hardware_self_adapt.h"

#include <asm/mach-types.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
 #include <linux/hw_dev_dec.h>
#endif

#ifdef CONFIG_HUAWEI_CAMERA_SENSOR_S5K5CA
 #undef CDBG
 #define CDBG(fmt, args...) printk(KERN_INFO "s5k5ca.c: " fmt, ## args)
#endif

/*=============================================================
    SENSOR REGISTER DEFINES
==============================================================*/
#define S5K5CA_CHIP_ID 0x05ca

enum s5k5ca_test_mode_t
{
    TEST_OFF,
    TEST_1,
    TEST_2,
    TEST_3
};

enum s5k5ca_resolution_t
{
    QTR_SIZE,
    FULL_SIZE,
    INVALID_SIZE
};

enum s5k5ca_reg_update_t
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

enum s5k5ca_setting_t
{
    RES_PREVIEW,
    RES_CAPTURE
};

/*
 * Time in milisecs for waiting for the sensor to reset.
 */
#define S5K5CA_RESET_DELAY_MSECS 66

/* for 30 fps preview */
#define S5K5CA_DEFAULT_CLOCK_RATE 24500000

#define S5K5CA_ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

/* FIXME: Changes from here */
struct s5k5ca_work_t
{
    struct work_struct work;
};

struct s5k5ca_ctrl_t
{
    const struct  msm_camera_sensor_info *sensordata;

    int      sensormode;
    uint32_t fps_divider; /* init to 1 * 0x00000400 */
    uint32_t pict_fps_divider; /* init to 1 * 0x00000400 */

    uint16_t curr_lens_pos;
    uint16_t init_curr_lens_pos;
    uint16_t my_reg_gain;
    uint32_t my_reg_line_count;

    enum s5k5ca_resolution_t prev_res;
    enum s5k5ca_resolution_t pict_res;
    enum s5k5ca_resolution_t curr_res;
    enum s5k5ca_test_mode_t  set_test;

    unsigned short imgaddr;
};

/*delete antibanding enum*/
const static char s5k5ca_supported_effect[] = "none,mono,negative,sepia,aqua";
static bool CSI_CONFIG;
#define MODEL_TRULY 0
#define MODEL_SUNNY 1
#define S5K5CA_IS_ON 1

static uint16_t s5k5ca_model_id = MODEL_SUNNY;

static uint8_t s5k5ca_init_flag = false;

#define M660_SENSOR_PWD 32
static int s5k5ca_pwd = 119; //pwd for camera
static struct s5k5ca_i2c_reg_conf * p_s5k5ca_init_reg_config;
static unsigned int reg_num;

static struct  s5k5ca_work_t *s5k5casensorw = NULL;

static struct  i2c_client *s5k5ca_client = NULL;
static struct s5k5ca_ctrl_t *s5k5ca_ctrl = NULL;
static enum s5k5ca_reg_update_t last_rupdate = -1;
static enum s5k5ca_setting_t last_rt = -1;
static DECLARE_WAIT_QUEUE_HEAD(s5k5ca_wait_queue);
DEFINE_MUTEX(s5k5ca_sem);

static int s5k5ca_i2c_rxdata(unsigned short saddr,
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

    if (i2c_transfer(s5k5ca_client->adapter, msgs, 2) < 0)
    {
        CDBG("s5k5ca_i2c_rxdata failed!\n");
        return -EIO;
    }

    return 0;
}

static int32_t s5k5ca_i2c_read_w(unsigned short raddr, unsigned short *rdata)
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

    rc = s5k5ca_i2c_rxdata(s5k5ca_client->addr, buf, 2);
    if (rc < 0)
    {
        return rc;
    }

    *rdata = buf[0] << 8 | buf[1];

    if (rc < 0)
    {
        CDBG("s5k5ca_i2c_read failed!\n");
    }

    return rc;
}

static int32_t s5k5ca_i2c_txdata(unsigned short saddr,
                                 unsigned char *txdata, int length)
{
    int32_t i  = 0;
    int32_t rc = -EFAULT;
    struct i2c_msg msg[] =
    {
        {
            .addr  = saddr,
            .flags = 0,
            .len = length,
            .buf = txdata,
        },
    };

    for (i = 0; i < 3; i++)
    {
        rc = i2c_transfer(s5k5ca_client->adapter, msg, 1);
        if (0 <= rc)
        {
            return 0;
        }
    }

    if (3 == i)
    {
        CDBG("s5k5ca_i2c_txdata faild\n");
        return -EIO;
    }

    return 0;
}

static int32_t s5k5ca_i2c_write_w(unsigned short waddr, unsigned short wdata)
{
    int32_t rc = -EFAULT;
    unsigned char buf[4];

    memset(buf, 0, sizeof(buf));
    buf[0] = (waddr & 0xFF00) >> 8;
    buf[1] = (waddr & 0x00FF);
    buf[2] = (wdata & 0xFF00) >> 8;
    buf[3] = (wdata & 0x00FF);

    rc = s5k5ca_i2c_txdata(s5k5ca_client->addr, buf, 4);

    if (rc < 0)
    {
        CDBG("i2c_write_w failed, addr = 0x%x, val = 0x%x!\n",
             waddr, wdata);
    }

    return rc;
}

static int32_t s5k5ca_i2c_write_w_table(struct s5k5ca_i2c_reg_conf const *reg_conf_tbl,
                                        int                               num_of_items_in_table)
{
    int i;
    int32_t rc = -EFAULT;

    for (i = 0; i < num_of_items_in_table; i++)
    {
        rc = s5k5ca_i2c_write_w(reg_conf_tbl->waddr, reg_conf_tbl->wdata);
        if (rc < 0)
        {
            break;
        }

        reg_conf_tbl++;
    }

    return rc;
}

int32_t s5k5ca_set_default_focus(uint8_t af_step)
{
    CDBG("s5k4cdgx_set_default_focus:\n");

    return 0;
}

int32_t s5k5ca_set_fps(struct fps_cfg    *fps)
{
    /* input is new fps in Q8 format */
    int32_t rc = 0;

    CDBG("s5k5ca_set_fps\n");
    return rc;
}

int32_t s5k5ca_write_exp_gain(uint16_t gain, uint32_t line)
{
    CDBG("s5k5ca_write_exp_gain\n");
    return 0;
}

int32_t s5k5ca_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
    int32_t rc = 0;

    CDBG("s5k5ca_set_pict_exp_gain\n");

    mdelay(10);

    /* camera_timed_wait(snapshot_wait*exposure_ratio); */
    return rc;
}
/* Mirror from kernel space */ 
static int32_t s5k5ca_set_mirror_mode(void)
{
    int32_t rc = 0;

    if (HW_MIRROR_AND_FLIP == get_hw_camera_mirror_type()) 
    {
        rc = s5k5ca_i2c_write_w_table(s5k5ca_regs.s5k5ca_mirror_mode_reg_config,
                                     s5k5ca_regs.s5k5ca_mirror_mode_reg_config_size);
    }

    return rc;
}
int32_t s5k5ca_setting(enum s5k5ca_reg_update_t rupdate,
                       enum s5k5ca_setting_t    rt)
{
    struct msm_camera_csi_params s5k5ca_csi_params;
    int32_t rc = 0;

    mutex_lock(&s5k5ca_sem);
    if ((rupdate == last_rupdate) && (rt == last_rt))
    {
        CDBG("s5k5ca_setting exit\n");
        mutex_unlock(&s5k5ca_sem);
        return rc;
    }

    CDBG("s5k5ca_setting in rupdate=%d,rt=%d\n", rupdate, rt);
    switch (rupdate)
    {
    case UPDATE_PERIODIC:

        /*preview setting*/
        if (rt == RES_PREVIEW)
        {
            CDBG("s5k5ca:  sensor: init preview reg.\n");
            rc = s5k5ca_i2c_write_w_table(s5k5ca_regs.s5k5ca_preview_reg_config,
                                          s5k5ca_regs.s5k5ca_preview_reg_config_size);
            if (!CSI_CONFIG)
            {
                CDBG("s5k5ca: init CSI  config!\n");
                msm_camio_vfe_clk_rate_set(192000000);
                s5k5ca_csi_params.data_format = CSI_8BIT;
                s5k5ca_csi_params.lane_cnt = 1;
                s5k5ca_csi_params.lane_assign = 0xe4;
                s5k5ca_csi_params.dpcm_scheme = 0;
                s5k5ca_csi_params.settle_cnt = 0x18;
                rc = msm_camio_csi_config(&s5k5ca_csi_params);
                CSI_CONFIG = 1;
            }
        }
        /*snapshot setting*/
        else
        {
            CDBG("s5k5ca:  sensor: init snapshot reg.\n");
            rc = s5k5ca_i2c_write_w_table(s5k5ca_regs.s5k5ca_snapshot_reg_config,
                                          s5k5ca_regs.s5k5ca_snapshot_reg_config_size);
        }

        /*increase the delay after register writing*/
        mdelay(50);
        break;

    case REG_INIT:

        /*delete one line*/
        CDBG("s5k5ca  model is %d : init sensor!\n", s5k5ca_model_id);

        /* Write init sensor register */

        rc = s5k5ca_i2c_write_w_table(s5k5ca_regs.s5k5ca_init_reg_sensor_start,
                                      s5k5ca_regs.s5k5ca_init_reg_sensor_start_size);
        mdelay(100);

        rc = s5k5ca_i2c_write_w_table(p_s5k5ca_init_reg_config, reg_num);
        /*add a 10ms delay between registers writing*/
        mdelay(10);
        rc = s5k5ca_i2c_write_w_table(s5k5ca_regs.s5k5ca_init_reg_config_sunny_2,  
                                s5k5ca_regs.s5k5ca_init_reg_config_sunny_2_size);
        //           mdelay(100);
        CDBG("s5k5ca model is %d: init sensor done!\n", s5k5ca_model_id);
        if(rc >= 0)
        {
            rc = s5k5ca_set_mirror_mode();
        } 
        break;

    default:
        rc = -EFAULT;
        break;
    } /* switch (rupdate) */
    if (rc == 0)
    {
        last_rupdate = rupdate;
        last_rt = rt;
    }

    mutex_unlock(&s5k5ca_sem);
    return rc;
}

int32_t s5k5ca_video_config(int mode, int res)
{
    int32_t rc;

    switch (res)
    {
    case QTR_SIZE:
        rc = s5k5ca_setting(UPDATE_PERIODIC, RES_PREVIEW);
        if (rc < 0)
        {
            return rc;
        }

        CDBG("sensor configuration done!\n");
        break;

    case FULL_SIZE:
        rc = s5k5ca_setting(UPDATE_PERIODIC, RES_CAPTURE);
        if (rc < 0)
        {
            return rc;
        }

        break;

    default:
        return 0;
    } /* switch */

    s5k5ca_ctrl->prev_res   = res;
    s5k5ca_ctrl->curr_res   = res;
    s5k5ca_ctrl->sensormode = mode;

    return rc;
}

int32_t s5k5ca_snapshot_config(int mode)
{
    int32_t rc = 0;

    CDBG("s5k5ca_snapshot_config in\n");
    rc = s5k5ca_setting(UPDATE_PERIODIC, RES_CAPTURE);
    /*delete one line*/
    if (rc < 0)
    {
        return rc;
    }

    s5k5ca_ctrl->curr_res = s5k5ca_ctrl->pict_res;

    s5k5ca_ctrl->sensormode = mode;

    return rc;
}

int32_t s5k5ca_power_down(void)
{
    int32_t rc = 0;

    mdelay(5);

    return rc;
}

int32_t s5k5ca_move_focus(int direction, int32_t num_steps)
{
    return 0;
}

static int s5k5ca_sensor_init_done(const struct msm_camera_sensor_info *data)
{
    /* Set the sensor reset when camera is not initialization. */
    if (false == s5k5ca_init_flag)
    {
        gpio_direction_output(data->sensor_reset, 0);
        gpio_free(data->sensor_reset);
    }

    gpio_direction_output(s5k5ca_pwd, 1);
    gpio_free(s5k5ca_pwd);

    if (false == s5k5ca_init_flag)
    {
        /*disable the power*/
        if (data->vreg_disable_func)
        {
            data->vreg_disable_func(0);
        }
    }

    last_rupdate = -1;
    last_rt = -1;
    return 0;
}

static int s5k5ca_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
    int rc;
    unsigned short chipid;

    /* pull down power down */
    rc = gpio_request(s5k5ca_pwd, "s5k5ca");
    if (!rc || (rc == -EBUSY))
    {
        gpio_direction_output(s5k5ca_pwd, 1);
    }
    else
    {
        goto init_probe_fail;
    }

    /* Set the sensor reset when camera is not initialization. */
    if (false == s5k5ca_init_flag)
    {
        rc = gpio_request(data->sensor_reset, "s5k5ca");
        if (!rc)
        {
            rc = gpio_direction_output(data->sensor_reset, 0);
        }
        else
        {
            goto init_probe_fail;
        }
        mdelay(10);
        /*enable the power*/
        if (data->vreg_enable_func)
        {
           data->vreg_enable_func(1);
        } 
    }

    mdelay(20);

    {
        rc = gpio_direction_output(s5k5ca_pwd, 0);
        if (rc < 0)
        {
            goto init_probe_fail;
        }

        mdelay(20);

        /*hardware reset*/
        /* Set the sensor reset when camera is not initialization. */
        if (false == s5k5ca_init_flag)
        {
            rc = gpio_direction_output(data->sensor_reset, 1);
            if (rc < 0)
            {
                goto init_probe_fail;
            }
        }

        mdelay(20);
    }

    /* Set the soft reset to reset the chip and read the chip ID when camera is not initialization. */
    if (false == s5k5ca_init_flag)
    {
        rc = s5k5ca_i2c_write_w(0x0010, 0x0001);
        if (rc < 0)
        {
            CDBG("s5k5ca_i2c_write_w 0x0010 0x0001 rc=%d", rc);
            goto init_probe_fail;
        }

        mdelay(10);

        rc = s5k5ca_i2c_write_w(0x0010, 0x0000);
        if (rc < 0)
        {
            CDBG("s5k5ca_i2c_write_w 0x0010 0x0000 rc=%d", rc);
            goto init_probe_fail;
        }

        mdelay(10);

        rc = s5k5ca_i2c_write_w(0x002c, 0x0000);
        if (rc < 0)
        {
            CDBG("s5k5ca_i2c_write_w 0x002c 0x0000 rc=%d", rc);
            goto init_probe_fail;
        }

        rc = s5k5ca_i2c_write_w(0x002e, 0x0040);
        if (rc < 0)
        {
            CDBG("s5k5ca_i2c_write_w 0x002e 0x0040 rc=%d", rc);
            goto init_probe_fail;
        }

        /* 3. Read sensor Model ID: */
        rc = s5k5ca_i2c_read_w(0x0f12, &chipid);
        if (rc < 0)
        {
            CDBG("s5k5ca_i2c_read_w Model_ID failed!! rc=%d", rc);
            goto init_probe_fail;
        }

        CDBG("s5k5ca chipid = 0x%x\n", chipid);

        /* 4. Compare sensor ID to S5K5CA ID: */
        if (chipid != S5K5CA_CHIP_ID)
        {
            CDBG("s5k5ca Model_ID error!!");
            rc = -ENODEV;
            goto init_probe_fail;
        }

        {
            /* Change the method of reading model id to fit socket and FPC packing models
             * Socket model : 0--3
             * FPC model : 8--11
             */
            rc = s5k5ca_i2c_write_w(0xFCFC, 0xD000);
            if (rc < 0)
            {
                goto init_probe_fail;
            }

            rc = s5k5ca_i2c_write_w(0x108E, 0x3333);
            if (rc < 0)
            {
                goto init_probe_fail;
            }

            mdelay(2);

            rc = s5k5ca_i2c_write_w(0x1090, 0x8888);
            if (rc < 0)
            {
                goto init_probe_fail;
            }

            rc = s5k5ca_i2c_read_w(0x100C, &s5k5ca_model_id);
            if (rc < 0)
            {
                CDBG("s5k5ca_i2c_read_w 0x002e rc=%d", rc);
            }

            CDBG("s5k5ca model = 0x%x\n", s5k5ca_model_id);

            /* If ID out of range ,set model_id MODEL_TRULY_FPC as default */
            if (s5k5ca_model_id > MODEL_SUNNY)
            {
                s5k5ca_model_id = MODEL_SUNNY;
            }

            rc = s5k5ca_i2c_write_w(0x108E, 0x0000);
            if (rc < 0)
            {
                goto init_probe_fail;
            }

            switch (s5k5ca_model_id)
            {
            case MODEL_TRULY:

            case MODEL_SUNNY:
                p_s5k5ca_init_reg_config = (struct s5k5ca_i2c_reg_conf *)(s5k5ca_regs.s5k5ca_init_reg_config_sunny);
                reg_num = s5k5ca_regs.s5k5ca_init_reg_config_sunny_size;

                //              strncpy((char *)data->sensor_name, "23060043SF-SAM-S", strlen("23060043SF-SAM-S"));
                CDBG("s5k5ca probe is  MODEL_SUNNY.");
                break;

            default:
                goto init_probe_fail;
                CDBG("s5k5ca is no this sensor model.\n");
                break;
            }
        }
        CDBG("sensor name is %s.", data->sensor_name);
    }

    goto init_probe_done;

init_probe_fail:
    s5k5ca_sensor_init_done(data);
init_probe_done:
    return rc;
}

int s5k5ca_sensor_open_init(const struct msm_camera_sensor_info *data)
{
    int32_t rc;

    s5k5ca_ctrl = kzalloc(sizeof(struct s5k5ca_ctrl_t), GFP_KERNEL);
    if (!s5k5ca_ctrl)
    {
        CDBG("s5k5ca_sensor_open_init failed!\n");
        rc = -ENOMEM;
        goto init_done;
    }

    CDBG("s5k5ca_sensor_open_*****************!\n");
    s5k5ca_ctrl->fps_divider = 1 * 0x00000400;
    s5k5ca_ctrl->pict_fps_divider = 1 * 0x00000400;
    s5k5ca_ctrl->set_test = TEST_OFF;
    s5k5ca_ctrl->prev_res = QTR_SIZE;
    s5k5ca_ctrl->pict_res = FULL_SIZE;

    if (data)
    {
        s5k5ca_ctrl->sensordata = data;
    }

    /* enable mclk first */
    msm_camio_clk_rate_set(S5K5CA_DEFAULT_CLOCK_RATE);
    mdelay(20);

    rc = s5k5ca_probe_init_sensor(data);
    if (rc < 0)
    {
        CDBG("s5k5ca init failed!!!!!\n");
        goto init_fail;
    }
    else
    {
        /*delete one line*/
        CSI_CONFIG = 0;
        CDBG("s5k5ca init succeed!!!!! rc = %d \n", rc);
        goto init_done;
    }

    /* Don't write sensor init register at open camera. */

init_fail:
    kfree(s5k5ca_ctrl);
init_done:
    return rc;
}

int s5k5ca_init_client(struct i2c_client *client)
{
    /* Initialize the MSM_CAMI2C Chip */
    init_waitqueue_head(&s5k5ca_wait_queue);
    return 0;
}

int32_t s5k5ca_set_sensor_mode(int mode, int res)
{
    int32_t rc = 0;

    switch (mode)
    {
    case SENSOR_PREVIEW_MODE:
        CDBG("SENSOR_PREVIEW_MODE,res=%d\n", res);
        rc = s5k5ca_video_config(mode, res);
        break;

    case SENSOR_SNAPSHOT_MODE:
    case SENSOR_RAW_SNAPSHOT_MODE:
        CDBG("SENSOR_SNAPSHOT_MODE\n");
        rc = s5k5ca_snapshot_config(mode);
        break;

    default:
        rc = -EINVAL;
        break;
    }

    return rc;
}

static long s5k5ca_set_effect(int mode, int effect)
{
    struct s5k5ca_i2c_reg_conf const *reg_conf_tbl = NULL;
    int num_of_items_in_table = 0;
    long rc = 0;

    switch (effect)
    {
    case CAMERA_EFFECT_OFF:
        reg_conf_tbl = s5k5ca_regs.s5k5ca_effect_off_reg_config;
        num_of_items_in_table = s5k5ca_regs.s5k5ca_effect_off_reg_config_size;
        break;

    case CAMERA_EFFECT_MONO:
        reg_conf_tbl = s5k5ca_regs.s5k5ca_effect_mono_reg_config;
        num_of_items_in_table = s5k5ca_regs.s5k5ca_effect_mono_reg_config_size;
        break;

    case CAMERA_EFFECT_NEGATIVE:
        reg_conf_tbl = s5k5ca_regs.s5k5ca_effect_negative_reg_config;
        num_of_items_in_table = s5k5ca_regs.s5k5ca_effect_negative_reg_config_size;
        break;

    case CAMERA_EFFECT_SEPIA:
        reg_conf_tbl = s5k5ca_regs.s5k5ca_effect_sepia_reg_config;
        num_of_items_in_table = s5k5ca_regs.s5k5ca_effect_sepia_reg_config_size;
        break;

    case CAMERA_EFFECT_AQUA:
        reg_conf_tbl = s5k5ca_regs.s5k5ca_effect_aqua_reg_config;
        num_of_items_in_table = s5k5ca_regs.s5k5ca_effect_aqua_reg_config_size;
        break;

    /*the effect we need is solarize and posterize*/
    case CAMERA_EFFECT_SOLARIZE:
        reg_conf_tbl = s5k5ca_regs.s5k5ca_effect_solarize_reg_config;
        num_of_items_in_table = s5k5ca_regs.s5k5ca_effect_solarize_reg_config_size;
        break;

    case CAMERA_EFFECT_POSTERIZE:
        reg_conf_tbl = s5k5ca_regs.s5k5ca_effect_posterize_reg_config;
        num_of_items_in_table = s5k5ca_regs.s5k5ca_effect_posterize_reg_config_size;
        break;

    default:
        return 0;
    }
    rc = s5k5ca_i2c_write_w_table(reg_conf_tbl, num_of_items_in_table);

    return rc;
}

static long s5k5ca_set_wb(int wb)
{
    struct s5k5ca_i2c_reg_conf const *reg_conf_tbl = NULL;
    int num_of_items_in_table = 0;
    long rc = 0;

    CDBG("s5k5ca_set_wb FF wb:%d", wb);
    switch (wb)
    {
    case CAMERA_WB_AUTO:
        reg_conf_tbl = s5k5ca_regs.s5k5ca_wb_auto_reg_config;
        num_of_items_in_table = s5k5ca_regs.s5k5ca_wb_auto_reg_config_size;
        break;

    case CAMERA_WB_INCANDESCENT:
        reg_conf_tbl = s5k5ca_regs.s5k5ca_wb_a_reg_config;
        num_of_items_in_table = s5k5ca_regs.s5k5ca_wb_a_reg_config_size;
        break;

    case CAMERA_WB_CUSTOM:
        reg_conf_tbl = s5k5ca_regs.s5k5ca_wb_f_reg_config;
        num_of_items_in_table = s5k5ca_regs.s5k5ca_wb_f_reg_config_size;
        break;
    case CAMERA_WB_FLUORESCENT:
        reg_conf_tbl = s5k5ca_regs.s5k5ca_wb_tl84_reg_config;
        num_of_items_in_table = s5k5ca_regs.s5k5ca_wb_tl84_reg_config_size;
        break;

    case CAMERA_WB_DAYLIGHT:
        reg_conf_tbl = s5k5ca_regs.s5k5ca_wb_d65_reg_config;
        num_of_items_in_table = s5k5ca_regs.s5k5ca_wb_d65_reg_config_size;
        break;

    case CAMERA_WB_CLOUDY_DAYLIGHT:
        reg_conf_tbl = s5k5ca_regs.s5k5ca_wb_d50_reg_config;
        num_of_items_in_table = s5k5ca_regs.s5k5ca_wb_d50_reg_config_size;
        break;

    case CAMERA_WB_TWILIGHT:
        return 0;
        break;

    case CAMERA_WB_SHADE:
        return 0;
        break;

    default:
        return 0;
    }
    rc = s5k5ca_i2c_write_w_table(reg_conf_tbl, num_of_items_in_table);

    return rc;
}
/*delete some lines*/

int s5k5ca_sensor_config(void __user *argp)
{
    struct sensor_cfg_data cdata;
    long rc = 0;

    if (copy_from_user(&cdata,
                       (void *)argp,
                       sizeof(struct sensor_cfg_data)))
    {
        return -EFAULT;
    }

    CDBG("s5k5ca_sensor_config: cfgtype = %d\n",
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
        rc = s5k5ca_set_fps(&(cdata.cfg.fps));
        break;

    case CFG_SET_EXP_GAIN:
        rc =
            s5k5ca_write_exp_gain(
            cdata.cfg.exp_gain.gain,
            cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_PICT_EXP_GAIN:
        rc =
            s5k5ca_set_pict_exp_gain(
            cdata.cfg.exp_gain.gain,
            cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_MODE:
        rc = s5k5ca_set_sensor_mode(cdata.mode,
                                    cdata.rs);
        break;

    case CFG_PWR_DOWN:
        rc = s5k5ca_power_down();
        break;

    case CFG_MOVE_FOCUS:
        rc =
            s5k5ca_move_focus(
            cdata.cfg.focus.dir,
            cdata.cfg.focus.steps);
        break;

    case CFG_SET_DEFAULT_FOCUS:
        rc =
            s5k5ca_set_default_focus(
            cdata.cfg.focus.steps);
        break;
    case CFG_SET_EFFECT:
        rc = s5k5ca_set_effect(cdata.mode,
                               cdata.cfg.effect);
        break;

    case CFG_SET_WB:
        rc = s5k5ca_set_wb(cdata.cfg.effect);
        break;

    case CFG_SET_ANTIBANDING:
        /*delete antibanding handling*/
        break;

    case CFG_MAX:

        break;

    default:
        rc = -EFAULT;
        break;
    }

    return rc;
}

int s5k5ca_sensor_release(void)
{
    int rc = -EBADF;

    mutex_lock(&s5k5ca_sem);

    s5k5ca_power_down();

    s5k5ca_sensor_init_done(s5k5ca_ctrl->sensordata);

    msleep(150);
    kfree(s5k5ca_ctrl);

    mutex_unlock(&s5k5ca_sem);
    CDBG("s5k5ca_release completed!\n");
    return rc;
}

static int s5k5ca_i2c_probe(struct i2c_client *         client,
                            const struct i2c_device_id *id)
{
    int rc = 0;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        rc = -ENOTSUPP;
        goto probe_failure;
    }

    s5k5casensorw =
        kzalloc(sizeof(struct s5k5ca_work_t), GFP_KERNEL);

    if (!s5k5casensorw)
    {
        rc = -ENOMEM;
        goto probe_failure;
    }

    i2c_set_clientdata(client, s5k5casensorw);
    s5k5ca_init_client(client);
    s5k5ca_client = client;

    //s5k5ca_client->addr = s5k5ca_client->addr >> 1;
    mdelay(50);

    CDBG("i2c probe ok\n");
    return 0;

probe_failure:
    kfree(s5k5casensorw);
    s5k5casensorw = NULL;
    pr_err("i2c probe failure %d\n", rc);
    return rc;
}

static const struct i2c_device_id s5k5ca_i2c_id[] =
{
    { "s5k5ca", 0},
    { }
};

static struct i2c_driver s5k5ca_i2c_driver =
{
    .id_table = s5k5ca_i2c_id,
    .probe    = s5k5ca_i2c_probe,
    .remove   = __exit_p(s5k5ca_i2c_remove),
    .driver   = {
        .name = "s5k5ca",
    },
};

static int s5k5ca_sensor_probe(const struct msm_camera_sensor_info *info,
                               struct msm_sensor_ctrl *             s)
{
    /* We expect this driver to match with the i2c device registered
     * in the board file immediately. */
    int rc = i2c_add_driver(&s5k5ca_i2c_driver);

    if ((rc < 0) || (s5k5ca_client == NULL))
    {
        rc = -ENOTSUPP;
        goto probe_done;
    }

    /* enable mclk first */
    msm_camio_clk_rate_set(S5K5CA_DEFAULT_CLOCK_RATE);
    mdelay(20);

    /*camera shutdown is GPIO32 for M660*/
    if(machine_is_msm7x27a_M660())
    {
        s5k5ca_pwd = M660_SENSOR_PWD;
    }
    rc = s5k5ca_probe_init_sensor(info);
    if (rc < 0)
    {
        CDBG("s5k5ca probe failed!!!!\n");
        i2c_del_driver(&s5k5ca_i2c_driver);
        goto probe_done;
    }
    else
    {
        /*s5k5ca probe succeed, use the func to sign power should always on*/
        info->set_s5k5ca_is_on(S5K5CA_IS_ON);
        CDBG("s5k5ca probe succeed!!!!\n");
    }
    /*initialize the registers to save the time of open camera*/
    rc = s5k5ca_setting(REG_INIT, RES_PREVIEW);
    if (rc < 0) 
    {
        CDBG("s5k5ca init sensor failed!!!!\n");
        i2c_del_driver(&s5k5ca_i2c_driver);
        goto probe_done;
    }
    else
    {
        /*we use the variable to sign reset should't be set any more*/
        s5k5ca_init_flag = true;
        CDBG("s5k5ca init sensor succeed!!!!\n");
    }

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
    /* detect current device successful, set the flag as present */
    set_hw_dev_flag(DEV_I2C_CAMERA_MAIN);
#endif

    s->s_init = s5k5ca_sensor_open_init;
    s->s_release = s5k5ca_sensor_release;
    s->s_config = s5k5ca_sensor_config;
    /*set the s_mount_angle value of sensor*/
    s->s_mount_angle = info->sensor_platform_info->mount_angle;
    s5k5ca_sensor_init_done(info);

    /* For go to sleep mode, follow the datasheet */
    msleep(150);
    set_camera_support(true);
probe_done:
    return rc;
}

static int __s5k5ca_probe(struct platform_device *pdev)
{
    return msm_camera_drv_start(pdev, s5k5ca_sensor_probe);
}

static struct platform_driver msm_camera_driver =
{
    .probe     = __s5k5ca_probe,
    .driver    = {
        .name  = "msm_camera_s5k5ca",
        .owner = THIS_MODULE,
    },
};

static int __init s5k5ca_init(void)
{
    return platform_driver_register(&msm_camera_driver);
}

module_init(s5k5ca_init);

