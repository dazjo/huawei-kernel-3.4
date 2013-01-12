
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
#include "mt9d113.h"
#include "linux/hardware_self_adapt.h"
#include <asm/mach-types.h>

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
 #include <linux/hw_dev_dec.h>
#endif

#ifdef CONFIG_HUAWEI_CAMERA_SENSOR_MT9D113
 #undef CDBG
 #define CDBG(fmt, args...) printk(KERN_INFO "mt9d113.c: " fmt, ## args)
#endif

/*=============================================================
    SENSOR REGISTER DEFINES
==============================================================*/
#define MT9D113_CHIP_ID 0x2580

enum mt9d113_test_mode_t
{
    TEST_OFF,
    TEST_1,
    TEST_2,
    TEST_3
};

enum mt9d113_resolution_t
{
    QTR_SIZE,
    FULL_SIZE,
    INVALID_SIZE
};

enum mt9d113_reg_update_t
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

enum mt9d113_setting_t
{
    RES_PREVIEW,
    RES_CAPTURE
};

/*
 * Time in milisecs for waiting for the sensor to reset.
 */
#define MT9D113_RESET_DELAY_MSECS 66

/* for 30 fps preview */
#define MT9D113_DEFAULT_CLOCK_RATE 24000000

#define MT9D113_ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define HW_MT9D113_VER_2   2
#define HW_MT9D113_VER_3   3
static uint16_t mt9d113_version_id = HW_MT9D113_VER_3;
/* FIXME: Changes from here */
struct mt9d113_work_t
{
    struct work_struct work;
};

struct mt9d113_ctrl_t
{
    const struct  msm_camera_sensor_info *sensordata;

    int      sensormode;
    uint32_t fps_divider; /* init to 1 * 0x00000400 */
    uint32_t pict_fps_divider; /* init to 1 * 0x00000400 */

    uint16_t curr_lens_pos;
    uint16_t init_curr_lens_pos;
    uint16_t my_reg_gain;
    uint32_t my_reg_line_count;

    enum mt9d113_resolution_t prev_res;
    enum mt9d113_resolution_t pict_res;
    enum mt9d113_resolution_t curr_res;
    enum mt9d113_test_mode_t  set_test;

    unsigned short imgaddr;
};

const static char mt9d113_supported_effect[] = "none,mono,negative,sepia,aqua";
static bool CSI_CONFIG;
#define MODEL_TRULY 0
#define MODEL_SUNNY 1

#define U8185_SENSOR_PWD 37
static unsigned int sensor_pwd = 119;

static uint16_t mt9d113_model_id = MODEL_SUNNY;

static struct  mt9d113_work_t *mt9d113sensorw = NULL;

static struct  i2c_client *mt9d113_client  = NULL;
static struct mt9d113_ctrl_t *mt9d113_ctrl = NULL;
static enum mt9d113_reg_update_t last_rupdate = -1;
static enum mt9d113_setting_t last_rt = -1;
static DECLARE_WAIT_QUEUE_HEAD(mt9d113_wait_queue);
DEFINE_MUTEX(mt9d113_sem);

static int mt9d113_i2c_rxdata(unsigned short saddr,
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

    if (i2c_transfer(mt9d113_client->adapter, msgs, 2) < 0)
    {
        CDBG("mt9d113_i2c_rxdata failed!\n");
        return -EIO;
    }

    return 0;
}

static int32_t mt9d113_i2c_read_w(unsigned short raddr, unsigned short *rdata)
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

    rc = mt9d113_i2c_rxdata(mt9d113_client->addr << 1, buf, 2);
    if (rc < 0)
    {
        return rc;
    }

    *rdata = buf[0] << 8 | buf[1];

    if (rc < 0)
    {
        CDBG("mt9d113_i2c_read failed!\n");
    }

    return rc;
}

static int32_t mt9d113_i2c_txdata(unsigned short saddr,
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
        rc = i2c_transfer(mt9d113_client->adapter, msg, 1);
        if (0 <= rc)
        {
            return 0;
        }
    }

    if (3 == i)
    {
        CDBG("mt9d113_i2c_txdata faild\n");
        return -EIO;
    }

    return 0;
}

static int32_t mt9d113_i2c_write_w(unsigned short waddr, unsigned short wdata)
{
    int32_t rc = -EFAULT;
    unsigned char buf[4];

    memset(buf, 0, sizeof(buf));
    buf[0] = (waddr & 0xFF00) >> 8;
    buf[1] = (waddr & 0x00FF);
    buf[2] = (wdata & 0xFF00) >> 8;
    buf[3] = (wdata & 0x00FF);

    rc = mt9d113_i2c_txdata(mt9d113_client->addr << 1, buf, 4);

    if (rc < 0)
    {
        CDBG("i2c_write_w failed, addr = 0x%x, val = 0x%x!\n",
             waddr, wdata);
    }

    return rc;
}

static int32_t mt9d113_i2c_write_w_table(struct mt9d113_i2c_reg_conf const *reg_conf_tbl,
                                         int                                num_of_items_in_table)
{
    int i;
    int32_t rc = -EFAULT;

    for (i = 0; i < num_of_items_in_table; i++)
    {
        rc = mt9d113_i2c_write_w(reg_conf_tbl->waddr, reg_conf_tbl->wdata);
        if (rc < 0)
        {
            break;
        }

        reg_conf_tbl++;
    }

    return rc;
}

int32_t mt9d113_set_default_focus(uint8_t af_step)
{
    CDBG("s5k4cdgx_set_default_focus:\n");

    return 0;
}

int32_t mt9d113_set_fps(struct fps_cfg    *fps)
{
    /* input is new fps in Q8 format */
    int32_t rc = 0;

    CDBG("mt9d113_set_fps\n");
    return rc;
}

int32_t mt9d113_write_exp_gain(uint16_t gain, uint32_t line)
{
    CDBG("mt9d113_write_exp_gain\n");
    return 0;
}

int32_t mt9d113_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
    int32_t rc = 0;

    CDBG("mt9d113_set_pict_exp_gain\n");

    mdelay(10);

    /* camera_timed_wait(snapshot_wait*exposure_ratio); */
    return rc;
}

static int32_t mt9d113_wait(uint16_t reg, uint16_t mode_value)
{
    uint16_t i =100;
    uint16_t mode_data = 0;
    while(i)
    {
        if(mt9d113_i2c_write_w(0x098C, reg) < 0)
        {
            return -EFAULT;
        }
        if(mt9d113_i2c_read_w(0x0990, &mode_data) < 0)
        {
            return -EFAULT;
        }
        if(mode_value == mode_data)
        {
            return 0;
        }
        i--;
        msleep(10);
    }
    CDBG("mt9d113_wait fail,reg=0x%x,mode_value=0x%x,mode_data=0x%x\n",reg,mode_value,mode_data);
    return -EFAULT;
}
int32_t mt9d113_setting(enum mt9d113_reg_update_t rupdate,
                        enum mt9d113_setting_t    rt)
{
    struct msm_camera_csi_params mt9d113_csi_params;
    int32_t rc = 0;

    if ((rupdate == last_rupdate) && (rt == last_rt))
    {
        CDBG("mt9d113_setting exit\n");
        return rc;
    }

    CDBG("mt9d113_setting in rupdate=%d,rt=%d\n", rupdate, rt);
    switch (rupdate)
    {
    case UPDATE_PERIODIC:

        /*preview setting*/
        if (rt == RES_PREVIEW)
        {
            CDBG("~~~~mt9d113:  sensor: init preview reg.\n");
            rc = mt9d113_i2c_write_w_table(mt9d113_regs.mt9d113_preview_reg_config,
                                           mt9d113_regs.mt9d113_preview_reg_config_size);
            if (rc)
            {
                CDBG("       write mt9d113_preview_reg_config error!!!!!!");
            }
            if(mt9d113_wait(0xA104,0x0003) < 0)
            {
                return -EFAULT;
            } 
            msleep(10);
            if (!CSI_CONFIG)
            {
                CDBG("mt9d113: init CSI  config!\n");

                //  msm_camio_vfe_clk_rate_set(192000000);
                mt9d113_csi_params.data_format = CSI_8BIT;
                mt9d113_csi_params.lane_cnt = 1;
                mt9d113_csi_params.lane_assign = 0xe4;
                mt9d113_csi_params.dpcm_scheme = 0;
                mt9d113_csi_params.settle_cnt = 0x18;
                rc = msm_camio_csi_config(&mt9d113_csi_params);
                CSI_CONFIG = 1;
            }
        }
        /*snapshot setting*/
        else
        {
            CDBG("mt9d113:  sensor: init snapshot reg.\n");
            rc = mt9d113_i2c_write_w_table(mt9d113_regs.mt9d113_snapshot_reg_config,
                                           mt9d113_regs.mt9d113_snapshot_reg_config_size);
            if(mt9d113_wait(0xA104,0x0007) < 0)
            {
                CDBG("mt9d113_wait(0xA104,0x0007) fail\n");
                return -EFAULT;
            }
        }
        break;

    case REG_INIT:

        CSI_CONFIG = 0;

        /* Write init sensor register */
        rc = mt9d113_i2c_write_w_table(mt9d113_regs.mt9d113_init_reg_sensor_start,
                                       mt9d113_regs.mt9d113_init_reg_sensor_start_size);
        if (rc)
        {
            CDBG("       write mt9d113_init_reg_sensor_start error!!!!!!");
        }

        mdelay(100);
        rc = mt9d113_i2c_write_w_table(mt9d113_regs.mt9d113_init_reg_config_byd_2,
                                       mt9d113_regs.mt9d113_init_reg_config_byd_2_size);
        if (rc)
        {
            CDBG("       write mt9d113_init_reg_config_byd_2 error!!!!!!");
        }

        mdelay(100);
        rc = mt9d113_i2c_write_w(0x0018, 0x0028);
        mdelay(100);
        rc = mt9d113_i2c_write_w_table(mt9d113_regs.mt9d113_init_reg_config_byd_3,
                                       mt9d113_regs.mt9d113_init_reg_config_byd_3_size);
        if (rc)
        {
            CDBG("       write mt9d113_init_reg_config_byd_3 error!!!!!!");
        }

        mdelay(100);
        /*flip and mirror the output of U8185 camera*/
        if(machine_is_msm7x27a_U8185())
        {
            rc = mt9d113_i2c_write_w(0x098C, 0x2717 );	// MCU_ADDRESS [MODE_SENSOR_READ_MODE_A]
            rc = mt9d113_i2c_write_w(0x0990, 0x046F );	// MCU_DATA_0
            rc = mt9d113_i2c_write_w(0x098C, 0x272D );	// MCU_ADDRESS [MODE_SENSOR_READ_MODE_B]
            rc = mt9d113_i2c_write_w(0x0990, 0x0027 );	// MCU_DATA_0
            rc = mt9d113_i2c_write_w(0x098C, 0xA103 );	// MCU_ADDRESS [SEQ_CMD]
            rc = mt9d113_i2c_write_w(0x0990, 0x0006 );
            /*add a delay for sensor to refresh registers*/
            mdelay(100);
        }
        rc = mt9d113_i2c_write_w(0x098C, 0xA103);
        rc = mt9d113_i2c_write_w(0x0990, 0x0006);
        rc = mt9d113_i2c_write_w(0x3400, 0x7A28);
        CDBG("mt9d113 model is %d: init sensor done!\n", mt9d113_model_id);
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

    //  mutex_unlock(&mt9d113_sem);
    return rc;
}

int32_t mt9d113_video_config(int mode, int res)
{
    int32_t rc;

    switch (res)
    {
    case QTR_SIZE:
        printk("%s :QTR_SIZE\n", __func__);
        rc = mt9d113_setting(UPDATE_PERIODIC, RES_PREVIEW);
        if (rc < 0)
        {
            return rc;
        }

        CDBG("sensor configuration done!\n");
        break;

    case FULL_SIZE:
        printk("%s :FULL_SIZE\n", __func__);
        rc = mt9d113_setting(UPDATE_PERIODIC, RES_CAPTURE);
        if (rc < 0)
        {
            return rc;
        }

        break;

    default:
        return 0;
    } /* switch */

    mt9d113_ctrl->prev_res   = res;
    mt9d113_ctrl->curr_res   = res;
    mt9d113_ctrl->sensormode = mode;

    return rc;
}

int32_t mt9d113_snapshot_config(int mode)
{
    int32_t rc = 0;

    CDBG("mt9d113_snapshot_config in\n");
    rc = mt9d113_setting(UPDATE_PERIODIC, RES_CAPTURE);
    mdelay(50);
    if (rc < 0)
    {
        return rc;
    }

    mt9d113_ctrl->curr_res = mt9d113_ctrl->pict_res;

    mt9d113_ctrl->sensormode = mode;

    return rc;
}

int32_t mt9d113_power_down(void)
{
    int32_t rc = 0;

    mdelay(5);

    return rc;
}

int32_t mt9d113_move_focus(int direction, int32_t num_steps)
{
    return 0;
}

static int mt9d113_sensor_init_done(const struct msm_camera_sensor_info *data)
{
    /* Set the sensor reset when camera is not initialization. */

    gpio_direction_output(data->sensor_reset, 0);
    gpio_free(data->sensor_reset);

    gpio_direction_output(sensor_pwd, 1);
    gpio_free(sensor_pwd);

    /*disable the power*/
    if (data->vreg_disable_func)
    {
        data->vreg_disable_func(0);
    }
    last_rupdate = -1;
    last_rt = -1;
    return 0;
}

static int mt9d113_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
    int rc;
    unsigned short chipid;
  
    CDBG("sensor_pwd is =%d", sensor_pwd);
    /* pull down power down */
    rc = gpio_request(sensor_pwd, "mt9d113");
    if (!rc || (rc == -EBUSY))
    {
        gpio_direction_output(sensor_pwd, 1);
    }
    else
    {
        goto init_probe_fail;
    }

    /* Set the sensor reset when camera is not initialization. */

    rc = gpio_request(data->sensor_reset, "mt9d113");
    if (!rc)
    {
        rc = gpio_direction_output(data->sensor_reset, 0);
    }
    else
    {
        goto init_probe_fail;
    }
    /*enable the power*/
    mdelay(10);
    if (data->vreg_enable_func)
    {
       data->vreg_enable_func(1);
    }

    mdelay(20);

    {
        rc = gpio_direction_output(sensor_pwd, 0);
        if (rc < 0)
        {
            goto init_probe_fail;
        }

        mdelay(50);

        /*hardware reset*/
        /* Set the sensor reset when camera is not initialization. */

        rc = gpio_direction_output(data->sensor_reset, 1);
        if (rc < 0)
        {
            goto init_probe_fail;
        }

        mdelay(100);
    }

    /* Set the soft reset to reset the chip and read the chip ID when camera is not initialization. */

    /* 3. Read sensor Model ID: */
    rc = mt9d113_i2c_read_w(0x0000, &chipid);
    if (rc < 0)
    {
        CDBG("mt9d113_i2c_read_w Model_ID failed!! rc=%d", rc);
        goto init_probe_fail;
    }

    CDBG("mt9d113 chipid = 0x%x\n", chipid);

    /* 4. Compare sensor ID to MT9D113 ID: */
    if (chipid != MT9D113_CHIP_ID)
    {
        CDBG("mt9d113 Model_ID error!!");
        rc = -ENODEV;
        goto init_probe_fail;
    }

    CDBG("sensor name is %s.", data->sensor_name);

    goto init_probe_done;

init_probe_fail:
    mt9d113_sensor_init_done(data);
init_probe_done:
    return rc;
}

int mt9d113_sensor_open_init(const struct msm_camera_sensor_info *data)
{
    int32_t rc;

    mt9d113_ctrl = kzalloc(sizeof(struct mt9d113_ctrl_t), GFP_KERNEL);
    if (!mt9d113_ctrl)
    {
        CDBG("mt9d113_sensor_open_init failed!\n");
        rc = -ENOMEM;
        goto init_done;
    }

    CDBG("mt9d113_sensor_open_init!\n");
    mt9d113_ctrl->fps_divider = 1 * 0x00000400;
    mt9d113_ctrl->pict_fps_divider = 1 * 0x00000400;
    mt9d113_ctrl->set_test = TEST_OFF;
    mt9d113_ctrl->prev_res = QTR_SIZE;
    mt9d113_ctrl->pict_res = FULL_SIZE;

    if (data)
    {
        mt9d113_ctrl->sensordata = data;
    }

    /* enable mclk first */
    msm_camio_clk_rate_set(MT9D113_DEFAULT_CLOCK_RATE);
    mdelay(20);

    rc = mt9d113_probe_init_sensor(data);
    if (rc < 0)
    {
        CDBG("mt9d113 init failed!!!!!\n");
        goto init_fail;
    }
    else
    {
        rc = mt9d113_setting(REG_INIT, RES_PREVIEW);
        CDBG("mt9d113 init succeed!!!!! rc = %d \n", rc);
        goto init_done;
    }

    /* Don't write sensor init register at open camera. */

init_fail:
    kfree(mt9d113_ctrl);
init_done:
    return rc;
}

int mt9d113_init_client(struct i2c_client *client)
{
    /* Initialize the MSM_CAMI2C Chip */
    init_waitqueue_head(&mt9d113_wait_queue);
    return 0;
}

int32_t mt9d113_set_sensor_mode(int mode, int res)
{
    int32_t rc = 0;

    switch (mode)
    {
    case SENSOR_PREVIEW_MODE:
        CDBG("SENSOR_PREVIEW_MODE,res=%d\n", res);
        rc = mt9d113_video_config(mode, res);
        break;

    case SENSOR_SNAPSHOT_MODE:
    case SENSOR_RAW_SNAPSHOT_MODE:
        CDBG("SENSOR_SNAPSHOT_MODE\n");
        rc = mt9d113_snapshot_config(mode);
        break;

    default:
        rc = -EINVAL;
        break;
    }

    return rc;
}

static long mt9d113_set_effect(int mode, int effect)
{
    struct mt9d113_i2c_reg_conf const *reg_conf_tbl = NULL;
    int num_of_items_in_table = 0;
    long rc = 0;

    printk("mt9d113_set_effect \n ");
    switch (effect)
    {
    case CAMERA_EFFECT_OFF:
        reg_conf_tbl = mt9d113_regs.mt9d113_effect_off_reg_config;
        num_of_items_in_table = mt9d113_regs.mt9d113_effect_off_reg_config_size;
        break;

    case CAMERA_EFFECT_MONO:
        reg_conf_tbl = mt9d113_regs.mt9d113_effect_mono_reg_config;
        num_of_items_in_table = mt9d113_regs.mt9d113_effect_mono_reg_config_size;
        break;

    case CAMERA_EFFECT_NEGATIVE:
        reg_conf_tbl = mt9d113_regs.mt9d113_effect_negative_reg_config;
        num_of_items_in_table = mt9d113_regs.mt9d113_effect_negative_reg_config_size;
        break;

    case CAMERA_EFFECT_SEPIA:
        reg_conf_tbl = mt9d113_regs.mt9d113_effect_sepia_reg_config;
        num_of_items_in_table = mt9d113_regs.mt9d113_effect_sepia_reg_config_size;
        break;

    case CAMERA_EFFECT_AQUA:
        reg_conf_tbl = mt9d113_regs.mt9d113_effect_aqua_reg_config;
        num_of_items_in_table = mt9d113_regs.mt9d113_effect_aqua_reg_config_size;
        break;

    case CAMERA_EFFECT_WHITEBOARD:
        reg_conf_tbl = mt9d113_regs.mt9d113_effect_whiteboard_reg_config;
        num_of_items_in_table = mt9d113_regs.mt9d113_effect_whiteboard_reg_config_size;
        break;

    case CAMERA_EFFECT_BLACKBOARD:
        reg_conf_tbl = mt9d113_regs.mt9d113_effect_blackboard_reg_config;
        num_of_items_in_table = mt9d113_regs.mt9d113_effect_blackboard_reg_config_size;
        break;

    default:
        return 0;
    }
    rc = mt9d113_i2c_write_w_table(reg_conf_tbl, num_of_items_in_table);

    return rc;
}

static long mt9d113_set_wb(int wb)
{
    long rc = 0;

    CDBG("mt9d113_set_wb FF wb:%d", wb);
    if(mt9d113_i2c_read_w(0x31FE, &mt9d113_version_id) <0)
    {
        rc = -ENODEV;
        CDBG("mt9d113_set_wb,mt9d113_version_id failed\n");
        return rc;
    }
    CDBG("mt9d113_set_wb,mt9d113_version_id = %d\n", mt9d113_version_id);

    switch (wb) 
    {
    case CAMERA_WB_AUTO:
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xAB22))){return rc;} // MCU_ADDRESS [HG_LL_APCORR1]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0003))){return rc;} // MCU_DATA_0
        if(HW_MT9D113_VER_2 == mt9d113_version_id)
        {
            if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xAB20))){return rc;} // MCU_ADDRESS [HG_LL_SAT1]
            if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0043))){return rc;} // MCU_DATA_0
        }
        else
        {
            if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xAB20 ))){return rc;} 	// MCU_ADDRESS [HG_LL_SAT1]
            if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0080 ))){return rc;}	// MCU_DATA_0 0~255,normal use 120,150——————这个寄存器多处用到，请杨海民在每一处都判断V2和V3，是V3就修改成0x0080，具体可以咨询吴晓金
        }
    
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0x271F))){return rc;}// MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_A]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0293))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA11F))){return rc;}// MCU_ADDRESS [SEQ_PREVIEW_1_AWB]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0001))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA103))){return rc;}// MCU_ADDRESS [SEQ_CMD]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0005))){return rc;}// MCU_DATA_0
        msleep(100);
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA355))){return rc;}// MCU_ADDRESS [AWB_MODE]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x000A))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34A))){return rc;}// MCU_ADDRESS [AWB_GAIN_MIN]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0059))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34B))){return rc;}// MCU_ADDRESS [AWB_GAIN_MAX]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x00C8))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34C))){return rc;}// MCU_ADDRESS [AWB_GAINMIN_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0059))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34D))){return rc;}// MCU_ADDRESS [AWB_GAINMAX_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x00A6))){return rc;}// MCU_DATA_0
        break;
    
    case CAMERA_WB_INCANDESCENT:
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA11F))){return rc;}// MCU_ADDRESS [SEQ_PREVIEW_1_AWB]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0000))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA103))){return rc;}// MCU_ADDRESS [SEQ_CMD]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0005))){return rc;}// MCU_DATA_0
        msleep(100);
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA353))){return rc;}// MCU_ADDRESS
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0000))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34E))){return rc;}// MCU_ADDRESS [AWB_GAIN_R]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0086))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34F))){return rc;}// MCU_ADDRESS [AWB_GAIN_G]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0080))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA350))){return rc;}// MCU_ADDRESS [AWB_GAIN_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0092))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34A))){return rc;}// MCU_ADDRESS [AWB_GAIN_MIN]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0072))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34B))){return rc;}// MCU_ADDRESS [AWB_GAIN_MAX]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0072))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34C))){return rc;}// MCU_ADDRESS [AWB_GAINMIN_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x00A0))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34D))){return rc;}// MCU_ADDRESS [AWB_GAINMAX_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x00A0))){return rc;}// MCU_DATA_0
        break;
        
    case CAMERA_WB_CUSTOM:
    case CAMERA_WB_FLUORESCENT:
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA11F))){return rc;}// MCU_ADDRESS [SEQ_PREVIEW_1_AWB]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0000))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA103))){return rc;}// MCU_ADDRESS [SEQ_CMD]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0005))){return rc;}// MCU_DATA_0
        msleep(100);
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA353))){return rc;}// MCU_ADDRESS
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0032))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34E))){return rc;}// MCU_ADDRESS [AWB_GAIN_R]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x008C))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34F))){return rc;}// MCU_ADDRESS [AWB_GAIN_G]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0080))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA350))){return rc;}// MCU_ADDRESS [AWB_GAIN_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0080))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34A))){return rc;}// MCU_ADDRESS [AWB_GAIN_MIN]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x008C))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34B))){return rc;}// MCU_ADDRESS [AWB_GAIN_MAX]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x008C))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34C))){return rc;}// MCU_ADDRESS [AWB_GAINMIN_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0080))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34D))){return rc;}// MCU_ADDRESS [AWB_GAINMAX_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0080))){return rc;}// MCU_DATA_0	
        break;
             
    case CAMERA_WB_DAYLIGHT:
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA11F))){return rc;}// MCU_ADDRESS [SEQ_PREVIEW_1_AWB]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0000))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA103))){return rc;}// MCU_ADDRESS [SEQ_CMD]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0005))){return rc;}// MCU_DATA_0
        msleep(100);
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA353))){return rc;}// MCU_ADDRESS
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x007F))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34E))){return rc;}// MCU_ADDRESS [AWB_GAIN_R]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x009A))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34F))){return rc;}// MCU_ADDRESS [AWB_GAIN_G]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0080))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA350))){return rc;}// MCU_ADDRESS [AWB_GAIN_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x007A))){return rc;}// MCU_DATA_0 
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34A))){return rc;}// MCU_ADDRESS [AWB_GAIN_MIN]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x007D))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34B))){return rc;}// MCU_ADDRESS [AWB_GAIN_MAX]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x007D))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34C))){return rc;}// MCU_ADDRESS [AWB_GAINMIN_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0078))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34D))){return rc;}// MCU_ADDRESS [AWB_GAINMAX_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0078))){return rc;}// MCU_DATA_0
        break;
           
    case CAMERA_WB_CLOUDY_DAYLIGHT:
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA11F))){return rc;}// MCU_ADDRESS [SEQ_PREVIEW_1_AWB]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0000))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA103))){return rc;}// MCU_ADDRESS [SEQ_CMD]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0005))){return rc;}// MCU_DATA_0
        msleep(100);
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA353))){return rc;}// MCU_ADDRESS
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x007F))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34E))){return rc;}// MCU_ADDRESS [AWB_GAIN_R]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x00AF))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34F))){return rc;}// MCU_ADDRESS [AWB_GAIN_G]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0081))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA350))){return rc;}// MCU_ADDRESS [AWB_GAIN_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0064))){return rc;}// MCU_DATA_0 
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34A))){return rc;}// MCU_ADDRESS [AWB_GAIN_MIN]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0080))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34B))){return rc;}// MCU_ADDRESS [AWB_GAIN_MAX]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x0080))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34C))){return rc;}// MCU_ADDRESS [AWB_GAINMIN_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x006E))){return rc;}// MCU_DATA_0
        if(0 > (rc = mt9d113_i2c_write_w( 0x098C, 0xA34D))){return rc;}// MCU_ADDRESS [AWB_GAINMAX_B]
        if(0 > (rc = mt9d113_i2c_write_w( 0x0990, 0x006E))){return rc;}// MCU_DATA_0
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

    return rc;
}

int mt9d113_sensor_config(void __user *argp)
{
    struct sensor_cfg_data cdata;
    long rc = 0;

    if (copy_from_user(&cdata,
                       (void *)argp,
                       sizeof(struct sensor_cfg_data)))
    {
        return -EFAULT;
    }

    mutex_lock(&mt9d113_sem);
    CDBG("mt9d113_sensor_config: cfgtype = %d\n",
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
        rc = mt9d113_set_fps(&(cdata.cfg.fps));
        break;

    case CFG_SET_EXP_GAIN:
        rc =
            mt9d113_write_exp_gain(
            cdata.cfg.exp_gain.gain,
            cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_PICT_EXP_GAIN:
        rc =
            mt9d113_set_pict_exp_gain(
            cdata.cfg.exp_gain.gain,
            cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_MODE:
        rc = mt9d113_set_sensor_mode(cdata.mode,
                                     cdata.rs);
        break;

    case CFG_PWR_DOWN:
        rc = mt9d113_power_down();
        break;

    case CFG_MOVE_FOCUS:
        rc =
            mt9d113_move_focus(
            cdata.cfg.focus.dir,
            cdata.cfg.focus.steps);
        break;

    case CFG_SET_DEFAULT_FOCUS:
        rc =
            mt9d113_set_default_focus(
            cdata.cfg.focus.steps);
        break;
    case CFG_SET_EFFECT:
        rc = mt9d113_set_effect(cdata.mode,
                                cdata.cfg.effect);
        break;

    case CFG_SET_WB:
        rc = mt9d113_set_wb(cdata.cfg.effect);
        break;

    case CFG_SET_ANTIBANDING:
        break;

    case CFG_MAX:

        break;

    default:
        rc = -EFAULT;
        break;
    }

    mutex_unlock(&mt9d113_sem);

    return rc;
}

int mt9d113_sensor_release(void)
{
    int rc = -EBADF;

    mutex_lock(&mt9d113_sem);

    mt9d113_power_down();

    mt9d113_sensor_init_done(mt9d113_ctrl->sensordata);

    msleep(150);
    kfree(mt9d113_ctrl);

    mutex_unlock(&mt9d113_sem);
    CDBG("mt9d113_release completed!\n");
    return rc;
}

static int mt9d113_i2c_probe(struct i2c_client *         client,
                             const struct i2c_device_id *id)
{
    int rc = 0;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        rc = -ENOTSUPP;
        goto probe_failure;
    }

    mt9d113sensorw =
        kzalloc(sizeof(struct mt9d113_work_t), GFP_KERNEL);

    if (!mt9d113sensorw)
    {
        rc = -ENOMEM;
        goto probe_failure;
    }

    i2c_set_clientdata(client, mt9d113sensorw);
    mt9d113_init_client(client);
    mt9d113_client = client;

    //mt9d113_client->addr = mt9d113_client->addr >> 1;
    mdelay(50);

    CDBG("i2c probe ok\n");
    return 0;

probe_failure:
    kfree(mt9d113sensorw);
    mt9d113sensorw = NULL;
    pr_err("i2c probe failure %d\n", rc);
    return rc;
}

static const struct i2c_device_id mt9d113_i2c_id[] =
{
    { "mt9d113", 0},
    { }
};

static struct i2c_driver mt9d113_i2c_driver =
{
    .id_table = mt9d113_i2c_id,
    .probe    = mt9d113_i2c_probe,
    .remove   = __exit_p(mt9d113_i2c_remove),
    .driver   = {
        .name = "mt9d113",
    },
};

static int mt9d113_sensor_probe(const struct msm_camera_sensor_info *info,
                                struct msm_sensor_ctrl *             s)
{
    /* We expect this driver to match with the i2c device registered
     * in the board file immediately. */
    int rc = i2c_add_driver(&mt9d113_i2c_driver);

    if ((rc < 0) || (mt9d113_client == NULL))
    {
        rc = -ENOTSUPP;
        goto probe_done;
    }

    /* enable mclk first */
    msm_camio_clk_rate_set(MT9D113_DEFAULT_CLOCK_RATE);
    mdelay(20);

    if (machine_is_msm7x27a_U8185())
    {
        sensor_pwd = U8185_SENSOR_PWD;
    }
	
    rc = mt9d113_probe_init_sensor(info);
    if (rc < 0)
    {
        CDBG("mt9d113 probe failed!!!!\n");
        i2c_del_driver(&mt9d113_i2c_driver);
        goto probe_done;
    }
    else
    {
        CDBG("mt9d113 probe succeed!!!!\n");
    }

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
    /* detect current device successful, set the flag as present */
    set_hw_dev_flag(DEV_I2C_CAMERA_MAIN);
#endif

    s->s_init = mt9d113_sensor_open_init;
    s->s_release = mt9d113_sensor_release;
    s->s_config = mt9d113_sensor_config;

    /*set the s_mount_angle value of sensor*/
    s->s_mount_angle = info->sensor_platform_info->mount_angle;
    mt9d113_sensor_init_done(info);

    /* For go to sleep mode, follow the datasheet */
    msleep(150);
    set_camera_support(true);
probe_done:
    return rc;
}

static int __mt9d113_probe(struct platform_device *pdev)
{
    return msm_camera_drv_start(pdev, mt9d113_sensor_probe);
}

static struct platform_driver msm_camera_driver =
{
    .probe     = __mt9d113_probe,
    .driver    = {
        .name  = "msm_camera_mt9d113",
        .owner = THIS_MODULE,
    },
};

static int __init mt9d113_init(void)
{
    return platform_driver_register(&msm_camera_driver);
}

module_init(mt9d113_init);

