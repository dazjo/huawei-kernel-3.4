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
#include "mt9t113.h"
#include "linux/hardware_self_adapt.h"
#include <asm/mach-types.h>

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
 #include <linux/hw_dev_dec.h>
#endif

#ifdef CONFIG_HUAWEI_CAMERA_SENSOR_MT9T113
 #undef CDBG
 #define CDBG(fmt, args...) printk(KERN_INFO "mt9t113.c: " fmt, ## args)
#endif

/*=============================================================
    SENSOR REGISTER DEFINES
==============================================================*/
#define MT9T113_CHIP_ID 0x4680

enum mt9t113_test_mode_t
{
    TEST_OFF,
    TEST_1,
    TEST_2,
    TEST_3
};

enum mt9t113_resolution_t
{
    QTR_SIZE,
    FULL_SIZE,
    INVALID_SIZE
};

enum mt9t113_reg_update_t
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

enum mt9t113_setting_t
{
    RES_PREVIEW,
    RES_CAPTURE
};

/*
 * Time in milisecs for waiting for the sensor to reset.
 */
#define MT9T113_RESET_DELAY_MSECS 66

/* for 30 fps preview */
#define MT9T113_DEFAULT_CLOCK_RATE 24000000

#define MT9T113_ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

/* FIXME: Changes from here */
struct mt9t113_work_t
{
    struct work_struct work;
};

struct mt9t113_ctrl_t
{
    const struct  msm_camera_sensor_info *sensordata;

    int      sensormode;
    uint32_t fps_divider; /* init to 1 * 0x00000400 */
    uint32_t pict_fps_divider; /* init to 1 * 0x00000400 */

    uint16_t curr_lens_pos;
    uint16_t init_curr_lens_pos;
    uint16_t my_reg_gain;
    uint32_t my_reg_line_count;

    enum mt9t113_resolution_t prev_res;
    enum mt9t113_resolution_t pict_res;
    enum mt9t113_resolution_t curr_res;
    enum mt9t113_test_mode_t  set_test;

    unsigned short imgaddr;
};

const static char mt9t113_supported_effect[] = "none,mono,negative,sepia,aqua";
static bool CSI_CONFIG;
#define MODEL_TRULY 0
#define MODEL_SUNNY 1

static uint16_t mt9t113_model_id = MODEL_SUNNY;
#define REGISTER_WRITE_WAIT_FW 1
#define REGISTER_WRITE_WAIT_PATCH 2
#define REGISTER_WRITE_WAIT_PREVIEW 3
#define REGISTER_WRITE_WAIT_SNAPSHOT 4
#define MT9T113_IS_ON 1
static struct  mt9t113_work_t *mt9t113sensorw = NULL;

static struct  i2c_client *mt9t113_client = NULL;
static struct mt9t113_ctrl_t *mt9t113_ctrl = NULL;
static enum mt9t113_reg_update_t last_rupdate = -1;
static enum mt9t113_setting_t last_rt = -1;
static DECLARE_WAIT_QUEUE_HEAD(mt9t113_wait_queue);
DEFINE_MUTEX(mt9t113_sem);

static int mt9t113_i2c_rxdata(unsigned short saddr,
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

    if (i2c_transfer(mt9t113_client->adapter, msgs, 2) < 0)
    {
        CDBG("mt9t113_i2c_rxdata failed!\n");
        return -EIO;
    }

    return 0;
}

static int32_t mt9t113_i2c_read_w(unsigned short raddr, unsigned short *rdata)
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

    rc = mt9t113_i2c_rxdata(mt9t113_client->addr, buf, 2);
    if (rc < 0)
    {
        return rc;
    }

    *rdata = buf[0] << 8 | buf[1];

    if (rc < 0)
    {
        CDBG("mt9t113_i2c_read failed!\n");
    }

    return rc;
}

static int32_t mt9t113_i2c_txdata(unsigned short saddr,
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
        rc = i2c_transfer(mt9t113_client->adapter, msg, 1);
        if (0 <= rc)
        {
            return 0;
        }
    }

    if (3 == i)
    {
        CDBG("mt9t113_i2c_txdata faild\n");
        return -EIO;
    }

    return 0;
}

static int32_t mt9t113_i2c_write_w(unsigned short waddr, unsigned short wdata)
{
    int32_t rc = -EFAULT;
    unsigned char buf[4];

    memset(buf, 0, sizeof(buf));
    buf[0] = (waddr & 0xFF00) >> 8;
    buf[1] = (waddr & 0x00FF);
    buf[2] = (wdata & 0xFF00) >> 8;
    buf[3] = (wdata & 0x00FF);

    rc = mt9t113_i2c_txdata(mt9t113_client->addr, buf, 4);

    if (rc < 0)
    {
        CDBG("i2c_write_w failed, addr = 0x%x, val = 0x%x!\n",
             waddr, wdata);
    }

    return rc;
}

static int32_t mt9t113_i2c_write_w_table(struct mt9t113_i2c_reg_conf const *reg_conf_tbl,
                                        int                               num_of_items_in_table)
{
    int i;
    int32_t rc = -EFAULT;

    for (i = 0; i < num_of_items_in_table; i++)
    {
        rc = mt9t113_i2c_write_w(reg_conf_tbl->waddr, reg_conf_tbl->wdata);
        if (rc < 0)
        {
            break;
        }

        reg_conf_tbl++;
    }

    return rc;
}

int32_t mt9t113_set_default_focus(uint8_t af_step)
{
    CDBG("s5k4cdgx_set_default_focus:\n");

    return 0;
}

int32_t mt9t113_set_fps(struct fps_cfg    *fps)
{
    /* input is new fps in Q8 format */
    int32_t rc = 0;

    CDBG("mt9t113_set_fps\n");
    return rc;
}

int32_t mt9t113_write_exp_gain(uint16_t gain, uint32_t line)
{
    CDBG("mt9t113_write_exp_gain\n");
    return 0;
}

int32_t mt9t113_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
    int32_t rc = 0;

    CDBG("mt9t113_set_pict_exp_gain\n");

    mdelay(10);

    /* camera_timed_wait(snapshot_wait*exposure_ratio); */
    return rc;
}
/* Mirror from kernel space */ 
static int32_t mt9t113_set_mirror_mode(void)
{
    int32_t rc = 0;

    if (HW_MIRROR_AND_FLIP == get_hw_camera_mirror_type()) 
    {
        rc = mt9t113_i2c_write_w_table(mt9t113_regs.mt9t113_mirror_mode_reg_config,
                    				  mt9t113_regs.mt9t113_mirror_mode_reg_config_size);
    }

    return rc;
}
static int mt9t113_register_waiting(int time)
{
    int rc = 0, i;
    unsigned short r_value;

    /* wait for FW initialization complete
     * bit[14] of register 0x0018 is the STANDBY_CONTROL_AND_STATUS bit
     * the bit is 0 when initialization done
     * read the register per 5ms, timeout after 100ms
     */
    if (REGISTER_WRITE_WAIT_FW == time)
    {
        for(i = 20; i > 0; i --)
        {
            CDBG("time = %d, i =%d\n", time ,i);
            rc = mt9t113_i2c_read_w(0x0018, &r_value);
            if(0 == (r_value & 0x4000))
            {
                return rc;
            }
            mdelay(5);
        }
    }

    /* wait for Patch loading complete
     * variable 0x0018 is the MON_RAM_PATCH_ID status
     * the status is 1 when initialization done
     * read the statue per 5ms, timeout after 100ms
     */
    if (REGISTER_WRITE_WAIT_PATCH == time)
    {
        for(i = 20; i > 0; i --)
        {
            CDBG("time = %d, i =%d\n", time ,i);
            rc = mt9t113_i2c_write_w(0x098E, 0x800C);
            rc = mt9t113_i2c_read_w(0x0990, &r_value);
            CDBG("====rc = %d, r_value =%d \n", rc, r_value);
            if(1 == r_value)
            {
                return rc;
            }
            mdelay(10);
        }
    }

    /* wait for sensor to enter the Preview mode
     * variable 0x8401 is the sensor MODE status
     * the status is 0x0003 when sensor in Preview mode
     * read the statue per 20ms, timeout after 200ms
     */
    if (REGISTER_WRITE_WAIT_PREVIEW == time)
    {
        mdelay(30);
        for(i = 0; i< 10; i++)
        {
            rc = mt9t113_i2c_write_w(0x098E, 0x8401);
            rc = mt9t113_i2c_read_w(0x0990, &r_value);
            if(0x0003 == r_value)
            {
                break;
            }
            mdelay(20);
        }
    }

    /* wait for sensor to enter the Snapshot mode
     * variable 0x8401 is the sensor MODE status
     * the status is 0x0007 when sensor in Snapshot mode
     * read the statue per 20ms, timeout after 200ms
     */
    if (REGISTER_WRITE_WAIT_SNAPSHOT == time)
    {
        mdelay(30);
        for(i = 0; i< 10; i++)
        {
            rc = mt9t113_i2c_write_w(0x098E, 0x8401);
            rc = mt9t113_i2c_read_w(0x0990, &r_value);
            if(0x0007 == r_value)
            {
                break;
            }
            mdelay(20);
        }
    }

    return rc;
}
int32_t mt9t113_setting(enum mt9t113_reg_update_t rupdate,
                       enum mt9t113_setting_t    rt)
{
    struct msm_camera_csi_params mt9t113_csi_params;
    int32_t rc = 0;
    unsigned short mode_value;
  //  mutex_lock(&mt9t113_sem);
    if ((rupdate == last_rupdate) && (rt == last_rt))
    {
        CDBG("mt9t113_setting exit\n");
   //     mutex_unlock(&mt9t113_sem);
        return rc;
    }

    CDBG("mt9t113_setting in rupdate=%d,rt=%d\n", rupdate, rt);
    switch (rupdate)
    {
    case UPDATE_PERIODIC:

        /*preview setting*/
        if (rt == RES_PREVIEW)
        {
            rc = mt9t113_i2c_write_w(0x098E, 0x8401);
            rc = mt9t113_i2c_read_w(0x0990, &mode_value);
            if(0x0003 != mode_value)
            {
                CDBG("mt9t113:  sensor: init preview reg.\n");
                rc = mt9t113_i2c_write_w_table(mt9t113_regs.mt9t113_preview_reg_config,
                                          mt9t113_regs.mt9t113_preview_reg_config_size);
                if (rc)
                {
                   CDBG("       write mt9t113_preview_reg_config error!!!!!!");
                }
                mt9t113_register_waiting(REGISTER_WRITE_WAIT_PREVIEW);
            }
            if(!CSI_CONFIG)
            {
                CDBG("mt9t113: init CSI  config!\n");
             //   msm_camio_vfe_clk_rate_set(192000000);
                mt9t113_csi_params.data_format = CSI_8BIT;
                mt9t113_csi_params.lane_cnt = 1;
                mt9t113_csi_params.lane_assign = 0xe4;
                mt9t113_csi_params.dpcm_scheme = 0;
                mt9t113_csi_params.settle_cnt = 0x18;
                rc = msm_camio_csi_config(&mt9t113_csi_params);
                CSI_CONFIG = 1;
            }
        }
        /*snapshot setting*/
        else
        {
            CDBG("mt9t113:  sensor: init snapshot reg.\n");
            rc = mt9t113_i2c_write_w_table(mt9t113_regs.mt9t113_snapshot_reg_config,
                                          mt9t113_regs.mt9t113_snapshot_reg_config_size);
            mt9t113_register_waiting(REGISTER_WRITE_WAIT_SNAPSHOT);
        }
        mdelay(5);
        break;

    case REG_INIT:

        CDBG("mt9t113  model is %d : init sensor!\n", mt9t113_model_id);

        /* Write init sensor register */

        rc = mt9t113_i2c_write_w_table(mt9t113_regs.mt9t113_init_reg_sensor_start,
                                      mt9t113_regs.mt9t113_init_reg_sensor_start_size);
        mt9t113_register_waiting(REGISTER_WRITE_WAIT_FW);
        if (rc)
        {
            CDBG("       write mt9t113_init_reg_sensor_start error!!!!!!");
        }
        rc = mt9t113_i2c_write_w_table(mt9t113_regs.mt9t113_init_reg_config_sunny,
                                       mt9t113_regs.mt9t113_init_reg_config_sunny_size);
        mt9t113_register_waiting(REGISTER_WRITE_WAIT_PATCH);
        if (rc)
        {
            CDBG("       write mt9t113_init_reg_config_sunny error!!!!!!");
        }
        rc = mt9t113_set_mirror_mode();
        rc = mt9t113_i2c_write_w_table(mt9t113_regs.mt9t113_init_reg_config_sunny_2,
                                       mt9t113_regs.mt9t113_init_reg_config_sunny_2_size);
        if (rc)
        {
            CDBG("       write mt9t113_init_reg_config_sunny_2 error!!!!!!");
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

  //  mutex_unlock(&mt9t113_sem);
    return rc;
}

int32_t mt9t113_video_config(int mode, int res)
{
    int32_t rc;

    switch (res)
    {
    case QTR_SIZE:
        rc = mt9t113_setting(UPDATE_PERIODIC, RES_PREVIEW);
        if (rc < 0)
        {
            return rc;
        }

        CDBG("sensor configuration done!\n");
        break;

    case FULL_SIZE:
        rc = mt9t113_setting(UPDATE_PERIODIC, RES_CAPTURE);
        if (rc < 0)
        {
            return rc;
        }

        break;

    default:
        return 0;
    } /* switch */

    mt9t113_ctrl->prev_res   = res;
    mt9t113_ctrl->curr_res   = res;
    mt9t113_ctrl->sensormode = mode;

    return rc;
}

int32_t mt9t113_snapshot_config(int mode)
{
    int32_t rc = 0;

    CDBG("mt9t113_snapshot_config in\n");
    rc = mt9t113_setting(UPDATE_PERIODIC, RES_CAPTURE);
    /*delete one line*/
    if (rc < 0)
    {
        return rc;
    }

    /*delete one line*/
    mt9t113_ctrl->curr_res = mt9t113_ctrl->pict_res;

    mt9t113_ctrl->sensormode = mode;

    return rc;
}

int32_t mt9t113_power_down(void)
{
    int32_t rc = 0;

    mdelay(1);

    return rc;
}

int32_t mt9t113_move_focus(int direction, int32_t num_steps)
{
    return 0;
}

static int mt9t113_sensor_init_done(const struct msm_camera_sensor_info *data)
{
    unsigned short r_standby;
    /* Set the sensor reset when camera is not initialization. */
    /*delete one line*/
    {
        gpio_direction_output(data->sensor_reset, 0);
        gpio_free(data->sensor_reset);
    }

    /*as the FAE suggest, vdd_dis_soft setted ON, R0x0028[0] = 1*/
    mt9t113_i2c_read_w(0x0028,&r_standby);
    r_standby |= 0x0001;
    mt9t113_i2c_write_w(0x0028,r_standby);
    gpio_direction_output(data->sensor_pwd, 1);
    gpio_free(data->sensor_pwd);

    /*delete one line*/
    {
        if (data->vreg_disable_func)
        {
            data->vreg_disable_func(0);
        }
    }
    last_rupdate = -1;
    last_rt = -1;
    return 0;
}

static int mt9t113_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
    int rc;
    unsigned short chipid;

    /* pull down power down */
    rc = gpio_request(data->sensor_pwd, "mt9t113");
    if (!rc || (rc == -EBUSY))
    {
        gpio_direction_output(data->sensor_pwd, 0);
    }
    else
    {
        goto init_probe_fail;
    }

    mdelay(1);
    /* Set the sensor reset when camera is not initialization. */
    /*delete one line*/
    {
        rc = gpio_request(data->sensor_reset, "mt9t113");
        if (!rc || (rc == -EBUSY))
        {
            rc = gpio_direction_output(data->sensor_reset, 0);
        }
        else
        {
            goto init_probe_fail;
        }
        mdelay(2);

       if (data->vreg_enable_func)
       {
           data->vreg_enable_func(1);
       }
        mdelay(60);

        /*hardware reset*/
        /* Set the sensor reset when camera is not initialization. */
        rc = gpio_direction_output(data->sensor_reset, 1);
        if (rc < 0)
        {
            goto init_probe_fail;
        }
        mdelay(5);
        /* Set the soft reset to reset the chip and read the chip ID when camera is not initialization. */
        /* 3. Read sensor Model ID: */
        rc = mt9t113_i2c_read_w(0x0000, &chipid);
        if (rc < 0)
        {
            CDBG("mt9t113_i2c_read_w Model_ID failed!! rc=%d", rc);
            goto init_probe_fail;
        }

        CDBG("mt9t113 chipid = 0x%x\n", chipid);

        /* 4. Compare sensor ID to MT9T113 ID: */
        if (chipid != MT9T113_CHIP_ID)
        {
            CDBG("mt9t113 Model_ID error!!");
            rc = -ENODEV;
            goto init_probe_fail;
        }
        CDBG("sensor name is %s.", data->sensor_name);
    }

    goto init_probe_done;

init_probe_fail:
    mt9t113_sensor_init_done(data);
init_probe_done:
    return rc;
}

int mt9t113_sensor_open_init(const struct msm_camera_sensor_info *data)
{
    int32_t rc;

    mt9t113_ctrl = kzalloc(sizeof(struct mt9t113_ctrl_t), GFP_KERNEL);
    if (!mt9t113_ctrl)
    {
        CDBG("mt9t113_sensor_open_init failed!\n");
        rc = -ENOMEM;
        goto init_done;
    }

    CDBG("mt9t113_sensor_open_*****************!\n");
    mt9t113_ctrl->fps_divider = 1 * 0x00000400;
    mt9t113_ctrl->pict_fps_divider = 1 * 0x00000400;
    mt9t113_ctrl->set_test = TEST_OFF;
    mt9t113_ctrl->prev_res = QTR_SIZE;
    mt9t113_ctrl->pict_res = FULL_SIZE;

    if (data)
    {
        mt9t113_ctrl->sensordata = data;
    }

    /* enable mclk first */
    msm_camio_clk_rate_set(MT9T113_DEFAULT_CLOCK_RATE);
    mdelay(1);

    rc = mt9t113_probe_init_sensor(data);
    if (rc < 0)
    {
        CDBG("mt9t113 init failed!!!!!\n");
        goto init_fail;
    }
    else
    {
        CSI_CONFIG = 0;
        CDBG("mt9t113 init succeed!!!!! rc = %d \n", rc);
        rc = mt9t113_setting(REG_INIT, RES_PREVIEW);
        if (rc < 0)
        {
            CDBG("mt9t113 init failed!!!!!\n");
            goto init_fail;
        }
        goto init_done;
    }

    /* Don't write sensor init register at open camera. */

init_fail:
    kfree(mt9t113_ctrl);
init_done:
    return rc;
}

int mt9t113_init_client(struct i2c_client *client)
{
    /* Initialize the MSM_CAMI2C Chip */
    init_waitqueue_head(&mt9t113_wait_queue);
    return 0;
}

int32_t mt9t113_set_sensor_mode(int mode, int res)
{
    int32_t rc = 0;

    switch (mode)
    {
    case SENSOR_PREVIEW_MODE:
        CDBG("SENSOR_PREVIEW_MODE,res=%d\n", res);
        rc = mt9t113_video_config(mode, res);
        break;

    case SENSOR_SNAPSHOT_MODE:
    case SENSOR_RAW_SNAPSHOT_MODE:
        CDBG("SENSOR_SNAPSHOT_MODE\n");
        rc = mt9t113_snapshot_config(mode);
        break;

    default:
        rc = -EINVAL;
        break;
    }

    return rc;
}

static long mt9t113_set_effect(int mode, int effect)
{
    struct mt9t113_i2c_reg_conf const *reg_conf_tbl = NULL;
    int num_of_items_in_table = 0;
    long rc = 0;
    unsigned short effect_value;
    printk("mt9t113_set_effect \n ");
    /* variable 0xE887 is the key of effect
     * if the value of register is already the value
     * we are to set, return from the func
     */
    rc = mt9t113_i2c_write_w(0x098E, 0xE887);
    rc = mt9t113_i2c_read_w(0x0990, &effect_value);
    switch (effect)
    {
    case CAMERA_EFFECT_OFF:
        if(0 == effect_value)
        {
            return 0;
        }
        reg_conf_tbl = mt9t113_regs.mt9t113_effect_off_reg_config;
        num_of_items_in_table = mt9t113_regs.mt9t113_effect_off_reg_config_size;
        break;

    case CAMERA_EFFECT_MONO:
        if(1 == effect_value)
        {
            return 0;
        }
        reg_conf_tbl = mt9t113_regs.mt9t113_effect_mono_reg_config;
        num_of_items_in_table = mt9t113_regs.mt9t113_effect_mono_reg_config_size;
        break;

    case CAMERA_EFFECT_NEGATIVE:
        if(3 == effect_value)
        {
            return 0;
        }
        reg_conf_tbl = mt9t113_regs.mt9t113_effect_negative_reg_config;
        num_of_items_in_table = mt9t113_regs.mt9t113_effect_negative_reg_config_size;
        break;

    case CAMERA_EFFECT_SEPIA:
        if(2 == effect_value)
        {
            rc = mt9t113_i2c_write_w(0x098E, 0xE889);
            rc = mt9t113_i2c_read_w(0x0990, &effect_value);
            if(0x1E == effect_value)
            {
                return 0;
            }
        }
        reg_conf_tbl = mt9t113_regs.mt9t113_effect_sepia_reg_config;
        num_of_items_in_table = mt9t113_regs.mt9t113_effect_sepia_reg_config_size;
        break;

    case CAMERA_EFFECT_AQUA:
        if(2 == effect_value)
        {
            rc = mt9t113_i2c_write_w(0x098E, 0xE889);
            rc = mt9t113_i2c_read_w(0x0990, &effect_value);
            if(0xE0 == effect_value)
            {
                return 0;
            }
        }
        reg_conf_tbl = mt9t113_regs.mt9t113_effect_aqua_reg_config;
        num_of_items_in_table = mt9t113_regs.mt9t113_effect_aqua_reg_config_size;
        break;

    /*the effect we need is solarize and posterize*/
    case CAMERA_EFFECT_SOLARIZE:
        reg_conf_tbl = mt9t113_regs.mt9t113_effect_solarize_reg_config;
        num_of_items_in_table = mt9t113_regs.mt9t113_effect_solarize_reg_config_size;
        break;

    case CAMERA_EFFECT_POSTERIZE:
        reg_conf_tbl = mt9t113_regs.mt9t113_effect_posterize_reg_config;
        num_of_items_in_table = mt9t113_regs.mt9t113_effect_posterize_reg_config_size;
        break;

    default:
        return 0;
    }
    /*delete some lines*/
    rc = mt9t113_i2c_write_w_table(reg_conf_tbl, num_of_items_in_table);
    mdelay(50);

    return rc;
}

static long mt9t113_set_wb(int wb)
{
    struct mt9t113_i2c_reg_conf const *reg_conf_tbl = NULL;
    int num_of_items_in_table = 0;
    long rc = 0;
    unsigned short wb_value;

    /* variable 0x6848 is the key of whitebalance
     * if the value of register is already the value
     * we are to set, return from the func
     */
    rc = mt9t113_i2c_write_w(0x098E, 0x6848);
    rc = mt9t113_i2c_read_w(0x0990, &wb_value);
    CDBG("mt9t113_set_wb FF wb:%d", wb);
    switch (wb)
    {
    case CAMERA_WB_AUTO:
        if(0x003F == wb_value)
        {
            return 0;
        }
        reg_conf_tbl = mt9t113_regs.mt9t113_wb_auto_reg_config;
        num_of_items_in_table = mt9t113_regs.mt9t113_wb_auto_reg_config_size;
        break;

    case CAMERA_WB_INCANDESCENT:
        if(0x0000 == wb_value)
        {
            rc = mt9t113_i2c_write_w(0x098E, 0xAC3B);
            rc = mt9t113_i2c_read_w(0x0990, &wb_value);
            if(0x0060 == wb_value)
            {
                return 0;
            }
        }
        reg_conf_tbl = mt9t113_regs.mt9t113_wb_a_reg_config;
        num_of_items_in_table = mt9t113_regs.mt9t113_wb_a_reg_config_size;
        break;

    case CAMERA_WB_CUSTOM:
        reg_conf_tbl = mt9t113_regs.mt9t113_wb_f_reg_config;
        num_of_items_in_table = mt9t113_regs.mt9t113_wb_f_reg_config_size;
        break;
    case CAMERA_WB_FLUORESCENT:
        if(0x0000 == wb_value)
        {
            rc = mt9t113_i2c_write_w(0x098E, 0xAC3B);
            rc = mt9t113_i2c_read_w(0x0990, &wb_value);
            if(0x0050 == wb_value)
            {
                return 0;
            }
        }
        reg_conf_tbl = mt9t113_regs.mt9t113_wb_tl84_reg_config;
        num_of_items_in_table = mt9t113_regs.mt9t113_wb_tl84_reg_config_size;
        break;

    case CAMERA_WB_DAYLIGHT:
        if(0x0000 == wb_value)
        {
            rc = mt9t113_i2c_write_w(0x098E, 0xAC3B);
            rc = mt9t113_i2c_read_w(0x0990, &wb_value);
            if(0x0044 == wb_value)
            {
                return 0;
            }
        }
        /* Daylight should be D50 */
        reg_conf_tbl = mt9t113_regs.mt9t113_wb_d50_reg_config;
        num_of_items_in_table = mt9t113_regs.mt9t113_wb_d50_reg_config_size;
        break;

    case CAMERA_WB_CLOUDY_DAYLIGHT:
        if(0x0000 == wb_value)
        {
            rc = mt9t113_i2c_write_w(0x098E, 0xAC3B);
            rc = mt9t113_i2c_read_w(0x0990, &wb_value);
            if(0x003A == wb_value)
            {
                return 0;
            }
        }
        /* Cloudy should be D65 */
        reg_conf_tbl = mt9t113_regs.mt9t113_wb_d65_reg_config;
        num_of_items_in_table = mt9t113_regs.mt9t113_wb_d65_reg_config_size;
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
    rc = mt9t113_i2c_write_w_table(reg_conf_tbl, num_of_items_in_table);
    mdelay(50);
    return rc;
}

/*delete the unused antibanding func*/
int mt9t113_sensor_config(void __user *argp)
{
    struct sensor_cfg_data cdata;
    long rc = 0;
    if (copy_from_user(&cdata,
                       (void *)argp,
                       sizeof(struct sensor_cfg_data)))
    {
        return -EFAULT;
    }
    mutex_lock(&mt9t113_sem);
    CDBG("mt9t113_sensor_config: cfgtype = %d\n",
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
        rc = mt9t113_set_fps(&(cdata.cfg.fps));
        break;

    case CFG_SET_EXP_GAIN:
        rc =
            mt9t113_write_exp_gain(
            cdata.cfg.exp_gain.gain,
            cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_PICT_EXP_GAIN:
        rc =
            mt9t113_set_pict_exp_gain(
            cdata.cfg.exp_gain.gain,
            cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_MODE:
        rc = mt9t113_set_sensor_mode(cdata.mode,
                                    cdata.rs);
        break;

    case CFG_PWR_DOWN:
        rc = mt9t113_power_down();
        break;

    case CFG_MOVE_FOCUS:
        rc =
            mt9t113_move_focus(
            cdata.cfg.focus.dir,
            cdata.cfg.focus.steps);
        break;

    case CFG_SET_DEFAULT_FOCUS:
        rc =
            mt9t113_set_default_focus(
            cdata.cfg.focus.steps);
        break;
    case CFG_SET_EFFECT:
        rc = mt9t113_set_effect(cdata.mode,
                               cdata.cfg.effect);
        break;

    case CFG_SET_WB:
        rc = mt9t113_set_wb(cdata.cfg.effect);
        break;

    case CFG_SET_ANTIBANDING:
        /*delete the antibanding handling*/
        break;

    case CFG_MAX:

        break;

    default:
        rc = -EFAULT;
        break;
    }
    mutex_unlock(&mt9t113_sem);

    return rc;
}

int mt9t113_sensor_release(void)
{
    int rc = -EBADF;

    mutex_lock(&mt9t113_sem);

    mt9t113_power_down();

    mt9t113_sensor_init_done(mt9t113_ctrl->sensordata);

    kfree(mt9t113_ctrl);

    mutex_unlock(&mt9t113_sem);
    CDBG("mt9t113_release completed!\n");
    return rc;
}

static int mt9t113_i2c_probe(struct i2c_client *         client,
                            const struct i2c_device_id *id)
{
    int rc = 0;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        rc = -ENOTSUPP;
        goto probe_failure;
    }

    mt9t113sensorw =
        kzalloc(sizeof(struct mt9t113_work_t), GFP_KERNEL);

    if (!mt9t113sensorw)
    {
        rc = -ENOMEM;
        goto probe_failure;
    }

    i2c_set_clientdata(client, mt9t113sensorw);
    mt9t113_init_client(client);
    mt9t113_client = client;

    //mt9t113_client->addr = mt9t113_client->addr >> 1;
    mdelay(50);

    CDBG("i2c probe ok\n");
    return 0;

probe_failure:
    kfree(mt9t113sensorw);
    mt9t113sensorw = NULL;
    pr_err("i2c probe failure %d\n", rc);
    return rc;
}

static const struct i2c_device_id mt9t113_i2c_id[] =
{
    { "mt9t113", 0},
    { }
};

static struct i2c_driver mt9t113_i2c_driver =
{
    .id_table = mt9t113_i2c_id,
    .probe    = mt9t113_i2c_probe,
    .remove   = __exit_p(mt9t113_i2c_remove),
    .driver   = {
        .name = "mt9t113",
    },
};

static int mt9t113_sensor_probe(const struct msm_camera_sensor_info *info,
                               struct msm_sensor_ctrl *             s)
{
    /* We expect this driver to match with the i2c device registered
     * in the board file immediately. */
    int rc = i2c_add_driver(&mt9t113_i2c_driver);

    if ((rc < 0) || (mt9t113_client == NULL))
    {
        rc = -ENOTSUPP;
        goto probe_done;
    }

    /* enable mclk first */
    msm_camio_clk_rate_set(MT9T113_DEFAULT_CLOCK_RATE);
    mdelay(20);

    /*delete some lines*/
    rc = mt9t113_probe_init_sensor(info);
    if (rc < 0)
    {
        CDBG("mt9t113 probe failed!!!!\n");
        i2c_del_driver(&mt9t113_i2c_driver);
        goto probe_done;
    }
    else
    {
        /*delete one line*/
        /* delete one line */
        CDBG("mt9t113 probe succeed!!!!\n");
    }
    /*initialize the registers to save the time of open camera*/
    /*don't use standby mode anymore, delete the initiation when probe*/

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
    /* detect current device successful, set the flag as present */
    set_hw_dev_flag(DEV_I2C_CAMERA_MAIN);
#endif

    s->s_init = mt9t113_sensor_open_init;
    s->s_release = mt9t113_sensor_release;
    s->s_config = mt9t113_sensor_config;
    /*set the s_mount_angle value of sensor*/
    s->s_mount_angle = info->sensor_platform_info->mount_angle;
    mt9t113_sensor_init_done(info);

    /* For go to sleep mode, follow the datasheet */
    msleep(150);
    set_camera_support(true);
probe_done:
    return rc;
}

static int __mt9t113_probe(struct platform_device *pdev)
{
    return msm_camera_drv_start(pdev, mt9t113_sensor_probe);
}

static struct platform_driver msm_camera_driver =
{
    .probe     = __mt9t113_probe,
    .driver    = {
        .name  = "msm_camera_mt9t113",
        .owner = THIS_MODULE,
    },
};

static int __init mt9t113_init(void)
{
    return platform_driver_register(&msm_camera_driver);
}

module_init(mt9t113_init);


