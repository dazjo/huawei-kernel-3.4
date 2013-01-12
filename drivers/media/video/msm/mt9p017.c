
/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/camera.h>
#include "mt9p017.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <linux/hw_dev_dec.h>
#endif
#undef CDBG
#define CDBG(fmt, args...) printk(KERN_INFO "mt9p017.c: " fmt, ## args)

#define MT9P017_OTP_SUPPORT

/*=============================================================
    SENSOR REGISTER DEFINES
==============================================================*/
#define MT9P017_REG_MODEL_ID 0x0000
#define MT9P017_MODEL_ID 0x4800
#define REG_GROUPED_PARAMETER_HOLD 0x0104
#define GROUPED_PARAMETER_HOLD 0x0001
#define GROUPED_PARAMETER_UPDATE 0x0000
#define REG_COARSE_INT_TIME 0x3012
#define REG_VT_PIX_CLK_DIV 0x0300
#define REG_VT_SYS_CLK_DIV 0x0302
#define REG_PRE_PLL_CLK_DIV 0x0304
#define REG_PLL_MULTIPLIER 0x0306
#define REG_OP_PIX_CLK_DIV 0x0308
#define REG_OP_SYS_CLK_DIV 0x030A
#define REG_SCALE_M 0x0404
#define REG_FRAME_LENGTH_LINES 0x300A
#define REG_LINE_LENGTH_PCK 0x300C
#define REG_X_ADDR_START 0x3004
#define REG_Y_ADDR_START 0x3002
#define REG_X_ADDR_END 0x3008
#define REG_Y_ADDR_END 0x3006
#define REG_X_OUTPUT_SIZE 0x034C
#define REG_Y_OUTPUT_SIZE 0x034E
#define REG_FINE_INTEGRATION_TIME 0x3014
#define REG_ROW_SPEED 0x3016
#define MT9P017_REG_RESET_REGISTER 0x301A
#define MT9P017_RESET_REGISTER_PWON 0x10CC
#define MT9P017_RESET_REGISTER_PWOFF 0x10C8
#define REG_READ_MODE 0x3040
#define REG_GLOBAL_GAIN 0x305E
#define REG_TEST_PATTERN_MODE 0x3070
#define REG_FINE_CORRECTION 0x3010
#define REG_VCM_CONTROL 0x30F0
#define REG_VCM_NEW_CODE 0x30F2

#define MT9P017_REG_MODE_SELECT 0x0100
#define MT9P017_START_STREAM 0x01
#define MT9P017_STOP_STREAM 0x00

#define MT9P017_REV_7

enum mt9p017_test_mode
{
    TEST_OFF,
    TEST_1,
    TEST_2,
    TEST_3
};

enum mt9p017_resolution
{
    QTR_SIZE,
    FULL_SIZE,
    INVALID_SIZE
};

enum mt9p017_reg_update
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

enum mt9p017_setting
{
    RES_PREVIEW,
    RES_CAPTURE
};

enum mt9p017_move_focus_dir
{
    CAMSENSOR_MOVE_FOCUS_NEAR,
    CAMSENSOR_MOVE_FOCUS_FAR
};

/* actuator's Slave Address */
#define MT9P017_AF_I2C_ADDR 0x18

/* AF Total steps parameters */

/* Change the af step */
#define MT9P017_STEPS_NEAR_TO_CLOSEST_INF 40
#define MT9P017_TOTAL_STEPS_NEAR_TO_FAR 40

#define MT9P017_MU5M0_PREVIEW_DUMMY_PIXELS 0
#define MT9P017_MU5M0_PREVIEW_DUMMY_LINES 0

/* Time in milisecs for waiting for the sensor to reset.*/
#define MT9P017_RESET_DELAY_MSECS 66

/* for 20 fps preview */
#define MT9P017_DEFAULT_CLOCK_RATE 24000000
#define MT9P017_DEFAULT_MAX_FPS 26  /* ???? */

#define LOG10_1P_03 0.012837224705f

struct mt9p017_work
{
    struct work_struct work;
};
static struct mt9p017_work *mt9p017_sensorw;
static struct i2c_client *mt9p017_client;

struct mt9p017_ctrl
{
    const struct msm_camera_sensor_info *sensordata;

    int      sensormode;
    uint32_t fps_divider;   /* init to 1 * 0x00000400 */
    uint32_t pict_fps_divider;  /* init to 1 * 0x00000400 */

    uint16_t curr_lens_pos;
    uint16_t curr_step_pos;
    uint16_t my_reg_gain;
    uint32_t my_reg_line_count;

    enum mt9p017_resolution prev_res;
    enum mt9p017_resolution pict_res;
    enum mt9p017_resolution curr_res;
    enum mt9p017_test_mode  set_test;
};
static uint16_t update_type = UPDATE_PERIODIC;
static struct mt9p017_ctrl *mt9p017_ctrl;
static DECLARE_WAIT_QUEUE_HEAD(mt9p017_wait_queue);
DEFINE_MUTEX(mt9p017_mut);

uint16_t mt9p017_step_position_table[MT9P017_TOTAL_STEPS_NEAR_TO_FAR + 1];
const uint16_t mt9p017_nl_region_boundary1 = 1;
const uint16_t mt9p017_nl_region_boundary2 = 20;
const uint16_t mt9p017_nl_region_boundary3 = 28;
const uint16_t mt9p017_nl_region_boundary4 = 32;
const uint16_t mt9p017_nl_region_code_per_step1 = 62;
const uint16_t mt9p017_l_region_code_per_step1 = 2;
const uint16_t mt9p017_l_region_code_per_step2 = 3;
const uint16_t mt9p017_l_region_code_per_step3 = 6;
const uint16_t mt9p017_l_region_code_per_step4 = 9;
const uint16_t mt9p017_damping_threshold = 10;
uint16_t mt9p017_sw_damping_time_wait = 1;
uint16_t use_stream_damping = 1;

static int32_t mt9p017_move_focus(enum mt9p017_move_focus_dir direction,
                                  int32_t                     num_steps);

/*=============================================================*/

static int mt9p017_i2c_rxdata(unsigned short saddr, unsigned char *rxdata,
                              int length)
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

    if (i2c_transfer(mt9p017_client->adapter, msgs, 2) < 0)
    {
        CDBG("mt9p017_i2c_rxdata failed!\n");
        return -EIO;
    }

    return 0;
}

static int32_t mt9p017_i2c_read_w(unsigned short saddr, unsigned short raddr,
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

    rc = mt9p017_i2c_rxdata(saddr >> 1, buf, 2);
    if (rc < 0)
    {
        return rc;
    }

    *rdata = buf[0] << 8 | buf[1];

    if (rc < 0)
    {
        CDBG("mt9p017_i2c_read_w failed!\n");
    }
    else
    {
        CDBG("mt9p017_i2c_read_w succeeded:	saddr = 0x%x addr = 0x%x, val =0x%x!\n",
             saddr, raddr, *rdata);
    }

    return rc;
}

static int32_t mt9p017_i2c_txdata(unsigned short saddr, unsigned char *txdata,
                                  int length)
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

    if (i2c_transfer(mt9p017_client->adapter, msg, 1) < 0)
    {
        CDBG("mt9p017_i2c_txdata failed\n");
        return -EIO;
    }

    return 0;
}

static int32_t mt9p017_i2c_write_b(unsigned short saddr, unsigned short waddr,
                                   unsigned short bdata)
{
    int32_t rc = -EIO;
    unsigned char buf[3];

    memset(buf, 0, sizeof(buf));
    buf[0] = (waddr & 0xFF00) >> 8;
    buf[1] = (waddr & 0x00FF);
    buf[2] = bdata;
    rc = mt9p017_i2c_txdata(saddr >> 1, buf, 3);

    if (rc < 0)
    {
        CDBG("i2c_write_b failed, saddr = 0x%x addr = 0x%x, val =0x%x!\n",
             saddr, waddr, bdata);
    }

    /*
    else
    {
        CDBG("i2c_write_b succeeded: saddr = 0x%x addr = 0x%x, val =0x%x!\n",
             saddr, waddr, bdata);
    }*/

    return rc;
}

static int32_t mt9p017_i2c_write_w(unsigned short saddr, unsigned short waddr,
                                   unsigned short wdata)
{
    int32_t rc = -EIO;
    unsigned char buf[4];

    memset(buf, 0, sizeof(buf));
    buf[0] = (waddr & 0xFF00) >> 8;
    buf[1] = (waddr & 0x00FF);
    buf[2] = (wdata & 0xFF00) >> 8;
    buf[3] = (wdata & 0x00FF);

    rc = mt9p017_i2c_txdata(saddr >> 1, buf, 4);

    if (rc < 0)
    {
        CDBG("i2c_write_w failed, saddr = 0x%x addr = 0x%x, val = 0x%x!\n",
             saddr, waddr, wdata);
    }

    /*
    else
    {
        CDBG("i2c_write_w succeeded: saddr = 0x%x addr = 0x%x, val = 0x%x!\n",
             saddr, waddr, wdata);
    }*/

    //mdelay(20);
    //mt9p017_i2c_read_w(saddr, waddr, &temp_data);
    //printk("WriteW: ADDR: 0x%x VAL: 0x%x\n", waddr, temp_data);
    return rc;
}

static int32_t mt9p017_i2c_write_w_table(struct mt9p017_i2c_reg_conf const
                                         *reg_conf_tbl, int num)
{
    int i;
    int32_t rc = -EIO;

    for (i = 0; i < num; i++)
    {
        rc = mt9p017_i2c_write_w(mt9p017_client->addr,
                                 reg_conf_tbl->waddr,
                                 reg_conf_tbl->wdata);
        if (rc < 0)
        {
            break;
        }

        reg_conf_tbl++;
    }

    return rc;
}

static int32_t mt9p017_lens_shading_enable(uint8_t is_enable)
{
    int32_t rc = 0;

    CDBG("%s: entered. enable = %d\n", __func__, is_enable);

    rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                             REG_GROUPED_PARAMETER_HOLD,
                             GROUPED_PARAMETER_HOLD);
    if (rc < 0)
    {
        return rc;
    }

    rc = mt9p017_i2c_write_w(mt9p017_client->addr, 0x3780,
                             ((uint16_t) is_enable) << 15);
    if (rc < 0)
    {
        return rc;
    }

    rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                             REG_GROUPED_PARAMETER_HOLD,
                             GROUPED_PARAMETER_UPDATE);

    CDBG("%s: exiting. rc = %d\n", __func__, rc);
    return rc;
}

static int32_t mt9p017_set_lc(void)
{
    int32_t rc;

    rc = mt9p017_i2c_write_w_table(mt9p017_regs.rftbl,
                                   mt9p017_regs.rftbl_size);

    return rc;
}

static int32_t mt9p017_load_pixel_timing(void)
{
    int32_t rc;

    rc = mt9p017_i2c_write_w_table(mt9p017_regs.pttbl,
                                   mt9p017_regs.pttbl_size);
    return rc;
}

static int32_t mt9p017_load_settings(void)
{
    int32_t rc;

    rc = mt9p017_i2c_write_w_table(mt9p017_regs.rec_settings,
                                   mt9p017_regs.rec_size);
    return rc;
}

static void mt9p017_get_pict_fps(uint16_t fps, uint16_t *pfps)
{
    /* input fps is preview fps in Q8 format */
    uint32_t divider;   /*Q10 */
    uint32_t pclk_mult; /*Q10 */
    uint32_t d1;
    uint32_t d2;

    d1 = (uint32_t)((mt9p017_regs.reg_pat[RES_PREVIEW].frame_length_lines * 0x00000400) /
                        mt9p017_regs.reg_pat[RES_CAPTURE].frame_length_lines);

    d2 = (uint32_t)((mt9p017_regs.reg_pat[RES_PREVIEW].line_length_pck * 0x00000400) /
                        mt9p017_regs.reg_pat[RES_CAPTURE].line_length_pck);

    divider = (uint32_t) (d1 * d2) / 0x00000400;

    pclk_mult = (uint32_t) ((mt9p017_regs.reg_pat[RES_CAPTURE].pll_multiplier * 0x00000400) /
                        (mt9p017_regs.reg_pat[RES_PREVIEW].pll_multiplier));

    /* Verify PCLK settings and frame sizes. */
    *pfps = (uint16_t) (fps * divider * pclk_mult / 0x00000400 /
                        0x00000400);
}

static uint16_t mt9p017_get_prev_lines_pf(void)
{
    if (mt9p017_ctrl->prev_res == QTR_SIZE)
    {
        return mt9p017_regs.reg_pat[RES_PREVIEW].frame_length_lines;
    }
    else
    {
        return mt9p017_regs.reg_pat[RES_CAPTURE].frame_length_lines;
    }
}

static uint16_t mt9p017_get_prev_pixels_pl(void)
{
    if (mt9p017_ctrl->prev_res == QTR_SIZE)
    {
        return mt9p017_regs.reg_pat[RES_PREVIEW].line_length_pck;
    }
    else
    {
        return mt9p017_regs.reg_pat[RES_CAPTURE].line_length_pck;
    }
}

static uint16_t mt9p017_get_pict_lines_pf(void)
{
    return mt9p017_regs.reg_pat[RES_CAPTURE].frame_length_lines;
}

static uint16_t mt9p017_get_pict_pixels_pl(void)
{
    return mt9p017_regs.reg_pat[RES_CAPTURE].line_length_pck;
}

static uint32_t mt9p017_get_pict_max_exp_lc(void)
{
    uint16_t snapshot_lines_per_frame;

    if (mt9p017_ctrl->pict_res == QTR_SIZE)
    {
        snapshot_lines_per_frame =
            mt9p017_regs.reg_pat[RES_PREVIEW].frame_length_lines - 1;
    }
    else
    {
        snapshot_lines_per_frame =
            mt9p017_regs.reg_pat[RES_CAPTURE].frame_length_lines - 1;
    }

    return snapshot_lines_per_frame * 24;
}

static int32_t mt9p017_set_fps(struct fps_cfg *fps)
{
    /* input is new fps in Q10 format */
    int32_t rc = 0;
    enum mt9p017_setting setting;

    mt9p017_ctrl->fps_divider = fps->fps_div;
    mt9p017_ctrl->pict_fps_divider = fps->pict_fps_div;

    rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                             REG_GROUPED_PARAMETER_HOLD,
                             GROUPED_PARAMETER_HOLD);
    if (rc < 0)
    {
        return -EBUSY;
    }

    if (mt9p017_ctrl->sensormode == SENSOR_PREVIEW_MODE)
    {
        setting = RES_PREVIEW;
    }
    else
    {
        setting = RES_CAPTURE;
    }

    rc = mt9p017_i2c_write_w(mt9p017_client->addr,
                             REG_FRAME_LENGTH_LINES,
                             (mt9p017_regs.reg_pat[setting].frame_length_lines *
                              fps->fps_div / 0x00000400));
    if (rc < 0)
    {
        return rc;
    }

    rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                             REG_GROUPED_PARAMETER_HOLD,
                             GROUPED_PARAMETER_UPDATE);

    return rc;
}

static int32_t mt9p017_write_exp_gain(uint16_t gain, uint32_t line)
{
    uint16_t max_legal_gain = 0x1E7F;
    uint32_t line_length_ratio = 0x00000400;
    enum mt9p017_setting setting;
    int32_t rc = 0;

    CDBG("Line:%d mt9p017_write_exp_gain\n", __LINE__);

    if (mt9p017_ctrl->sensormode == SENSOR_PREVIEW_MODE)
    {
        mt9p017_ctrl->my_reg_gain = gain;
        mt9p017_ctrl->my_reg_line_count = (uint16_t) line;
    }

    if (gain > max_legal_gain)
    {
        CDBG("Max legal gain Line:%d\n", __LINE__);
        gain = max_legal_gain;
    }

    /* Verify no overflow */
    if (mt9p017_ctrl->sensormode == SENSOR_PREVIEW_MODE)
    {
        line = (uint32_t) (line * mt9p017_ctrl->fps_divider /
                           0x00000400);
        setting = RES_PREVIEW;
    }
    else
    {
        line = (uint32_t) (line * mt9p017_ctrl->pict_fps_divider /
                           0x00000400);
        setting = RES_CAPTURE;
    }

    /* Set digital gain to 1 */
#ifdef MT9P017_REV_7
    gain |= 0x1000;
#else
    gain |= 0x0200;
#endif

    if ((mt9p017_regs.reg_pat[setting].frame_length_lines - 1) < line)
    {
        line_length_ratio = (uint32_t) (line * 0x00000400) /
                            (mt9p017_regs.reg_pat[setting].frame_length_lines - 1);
    }
    else
    {
        line_length_ratio = 0x00000400;
    }

    rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                             REG_GROUPED_PARAMETER_HOLD,
                             GROUPED_PARAMETER_HOLD);
    if (rc < 0)
    {
        return rc;
    }

    rc = mt9p017_i2c_write_w(mt9p017_client->addr, REG_GLOBAL_GAIN, gain);
    if (rc < 0)
    {
        CDBG("mt9p017_i2c_write_w failed... Line:%d \n", __LINE__);
        return rc;
    }

    rc = mt9p017_i2c_write_w(mt9p017_client->addr,
                             REG_COARSE_INT_TIME, line);
    if (rc < 0)
    {
        CDBG("mt9p017_i2c_write_w failed... Line:%d \n", __LINE__);
        return rc;
    }

    rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                             REG_GROUPED_PARAMETER_HOLD,
                             GROUPED_PARAMETER_UPDATE);
    if (rc < 0)
    {
        return rc;
    }

    printk("mt9p017_write_exp_gain: gain = %d, line = %d\n", gain, line);

    return rc;
}

static int32_t mt9p017_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
    int32_t rc = 0;

    CDBG("Line:%d mt9p017_set_pict_exp_gain\n", __LINE__);

    rc = mt9p017_write_exp_gain(gain, line);
    if (rc < 0)
    {
        CDBG("Line:%d mt9p017_set_pict_exp_gain failed...\n",
             __LINE__);
        return rc;
    }

    //	rc = mt9p017_i2c_write_w(mt9p017_client->addr,
    //							 MT9P017_REG_RESET_REGISTER, 0x0018 | 0x0002);
    if (rc < 0)
    {
        CDBG("mt9p017_i2c_write_w failed... Line:%d\n", __LINE__);
        return rc;
    }

    mdelay(5);

    return rc;
}

static int32_t mt9p017_setting(enum mt9p017_reg_update rupdate,
                               enum mt9p017_setting    rt)
{
    int32_t rc = 0;
    uint16_t restored_step_pos = mt9p017_ctrl->curr_step_pos;
    struct msm_camera_csi_params mt9p017_csi_params;

    switch (rupdate)
    {
    case UPDATE_PERIODIC:
        if ((rt == RES_PREVIEW) || (rt == RES_CAPTURE))
        {
            struct mt9p017_i2c_reg_conf ppc_tbl[] =
            {
                {REG_X_ADDR_START,
                 mt9p017_regs.reg_pat[rt].x_addr_start},
                {REG_X_ADDR_END,
                 mt9p017_regs.reg_pat[rt].x_addr_end},
                {REG_Y_ADDR_START,
                 mt9p017_regs.reg_pat[rt].y_addr_start},
                {REG_Y_ADDR_END,
                 mt9p017_regs.reg_pat[rt].y_addr_end},
                {REG_READ_MODE,
                 mt9p017_regs.reg_pat[rt].read_mode},
                {REG_X_OUTPUT_SIZE,
                 mt9p017_regs.reg_pat[rt].x_output_size},
                {REG_Y_OUTPUT_SIZE,
                 mt9p017_regs.reg_pat[rt].y_output_size},
                {REG_LINE_LENGTH_PCK,
                 mt9p017_regs.reg_pat[rt].line_length_pck},
                {REG_FRAME_LENGTH_LINES,
                 (mt9p017_regs.reg_pat[rt].frame_length_lines *
                  mt9p017_ctrl->fps_divider / 0x00000400)},
                {REG_COARSE_INT_TIME,
                 mt9p017_regs.reg_pat[rt].coarse_int_time},
                {REG_FINE_INTEGRATION_TIME,
                 mt9p017_regs.reg_pat[rt].fine_int_time},
                {REG_FINE_CORRECTION,
                 mt9p017_regs.reg_pat[rt].fine_correction},
                {REG_VCM_CONTROL,
                 mt9p017_regs.reg_pat[rt].vcm_control},
            };
            if (use_stream_damping)
            {
                if (mt9p017_move_focus(CAMSENSOR_MOVE_FOCUS_FAR,
                                       mt9p017_ctrl->curr_step_pos) < 0)
                {
                    return rc;
                }
            }

            rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                                     MT9P017_REG_MODE_SELECT,
                                     MT9P017_STOP_STREAM);
            if (rc < 0)
            {
                return rc;
            }

            rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                                     REG_GROUPED_PARAMETER_HOLD,
                                     GROUPED_PARAMETER_HOLD);
            if (rc < 0)
            {
                CDBG("error\n");
            }

            if (update_type == REG_INIT)
            {
                CDBG("MIPI CONFIG!\n");
                msm_camio_vfe_clk_rate_set(192000000);
                mt9p017_csi_params.data_format = CSI_10BIT;
                mt9p017_csi_params.lane_cnt = 1;
                mt9p017_csi_params.lane_assign = 0xe4;
                mt9p017_csi_params.dpcm_scheme = 0;
                mt9p017_csi_params.settle_cnt = 24;
                rc = msm_camio_csi_config(&mt9p017_csi_params);
                update_type = rupdate;
            }
            else
            {
                rc = mt9p017_i2c_write_w_table(&ppc_tbl[0],
                                               ARRAY_SIZE(ppc_tbl));
                if (rc < 0)
                {
                    return rc;
                }
            }

            rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                                     REG_GROUPED_PARAMETER_HOLD,
                                     GROUPED_PARAMETER_UPDATE);
            if (rc < 0)
            {
                CDBG("error\n");
            }

            if (use_stream_damping)
            {
                if (mt9p017_move_focus(CAMSENSOR_MOVE_FOCUS_NEAR,
                                       restored_step_pos) < 0)
                {
                    return rc;
                }
            }

            rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                                     MT9P017_REG_MODE_SELECT,
                                     MT9P017_START_STREAM);
            if (rc < 0)
            {
                return rc;
            }

            mdelay(5);
        }

        break;      /* UPDATE_PERIODIC */

    case REG_INIT:
        CDBG("mt9p017_setting: case REG_INIT !!\n");
        if ((rt == RES_PREVIEW) || (rt == RES_CAPTURE))
        {
            struct mt9p017_i2c_reg_conf ipc_tbl1[] =
            {
                {0x301A, 0x0018}, //reset_register
                {0x3064, 0x5840}, //smia_test_2lane_mipi
                {0x31AE, 0x0201}, //dual_lane_MIPI_interface
                {REG_VT_PIX_CLK_DIV,
                 mt9p017_regs.reg_pat[rt].vt_pix_clk_div},
                {REG_VT_SYS_CLK_DIV,
                 mt9p017_regs.reg_pat[rt].vt_sys_clk_div},
                {REG_PRE_PLL_CLK_DIV,
                 mt9p017_regs.reg_pat[rt].pre_pll_clk_div},
                {REG_PLL_MULTIPLIER,
                 mt9p017_regs.reg_pat[rt].pll_multiplier},
                {REG_OP_PIX_CLK_DIV,
                 mt9p017_regs.reg_pat[rt].op_pix_clk_div},
                {REG_OP_SYS_CLK_DIV,
                 mt9p017_regs.reg_pat[rt].op_sys_clk_div},
            };

            struct mt9p017_i2c_reg_conf ipc_tbl3[] =
            {
                /* Set preview or snapshot mode */
                {REG_X_ADDR_START,
                 mt9p017_regs.reg_pat[rt].x_addr_start},
                {REG_X_ADDR_END,
                 mt9p017_regs.reg_pat[rt].x_addr_end},
                {REG_Y_ADDR_START,
                 mt9p017_regs.reg_pat[rt].y_addr_start},
                {REG_Y_ADDR_END,
                 mt9p017_regs.reg_pat[rt].y_addr_end},
                {REG_READ_MODE,
                 mt9p017_regs.reg_pat[rt].read_mode},
                {REG_X_OUTPUT_SIZE,
                 mt9p017_regs.reg_pat[rt].x_output_size},
                {REG_Y_OUTPUT_SIZE,
                 mt9p017_regs.reg_pat[rt].y_output_size},
                {REG_LINE_LENGTH_PCK,
                 mt9p017_regs.reg_pat[rt].line_length_pck},
                {REG_FRAME_LENGTH_LINES,
                 mt9p017_regs.reg_pat[rt].frame_length_lines},
                {REG_COARSE_INT_TIME,
                 mt9p017_regs.reg_pat[rt].coarse_int_time},
                {REG_FINE_INTEGRATION_TIME,
                 mt9p017_regs.reg_pat[rt].fine_int_time},
                {REG_FINE_CORRECTION,
                 mt9p017_regs.reg_pat[rt].fine_correction},
                {REG_VCM_CONTROL,
                 mt9p017_regs.reg_pat[rt].vcm_control},
            };

            /* reset fps_divider */
            mt9p017_ctrl->fps_divider = 1 * 0x0400;

            CDBG("------------RIGISTER-WRITE-BEGAIN---------------\n");

            /* software reset sensor */

            //            rc = mt9p017_i2c_write_b(mt9p017_client->addr,0x0103,0x01);
            //            msleep(300);
            rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                                     MT9P017_REG_MODE_SELECT,
                                     MT9P017_STOP_STREAM);
            if (rc < 0)
            {
                CDBG("register failed. rc = %d\n", rc);
            }

            msleep(10);

            rc = mt9p017_i2c_write_w_table(&ipc_tbl1[0],
                                           ARRAY_SIZE(ipc_tbl1));
            if (rc < 0)
            {
                CDBG("error\n");
            }

            msleep(10);
            rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                                     REG_GROUPED_PARAMETER_HOLD,
                                     GROUPED_PARAMETER_HOLD);

            rc = mt9p017_i2c_write_w_table(&ipc_tbl3[0],
                                           ARRAY_SIZE(ipc_tbl3));
            if (rc < 0)
            {
                return rc;
            }

            rc = mt9p017_set_lc();
            if (rc < 0)
            {
                CDBG("mt9p017_set_lc failed. rc = %d\n", rc);
            }

            rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                                     REG_GROUPED_PARAMETER_HOLD,
                                     GROUPED_PARAMETER_UPDATE);
            if (rc < 0)
            {
                CDBG("error\n");
            }

            rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                                     MT9P017_REG_MODE_SELECT,
                                     MT9P017_START_STREAM);
            if (rc < 0)
            {
                CDBG("error\n");
            }

            CDBG("------------RIGISTER-WRITE-END---------------\n");
        }

        update_type = rupdate;
        break;      /* case REG_INIT: */

    default:
        rc = -EINVAL;
        break;
    }           /* switch (rupdate) */

    return rc;
}

static int32_t mt9p017_video_config(int mode, int res)
{
    int32_t rc;

    switch (res)
    {
    case QTR_SIZE:
        rc = mt9p017_setting(UPDATE_PERIODIC, RES_PREVIEW);
        if (rc < 0)
        {
            return rc;
        }

        CDBG("mt9p017 sensor configuration done!\n");
        break;

    case FULL_SIZE:
        rc = mt9p017_setting(UPDATE_PERIODIC, RES_CAPTURE);
        if (rc < 0)
        {
            return rc;
        }

        break;

    default:
        return 0;
    }           /* switch */

    mt9p017_ctrl->prev_res   = res;
    mt9p017_ctrl->curr_res   = res;
    mt9p017_ctrl->sensormode = mode;

    rc = mt9p017_write_exp_gain(mt9p017_ctrl->my_reg_gain,
                                mt9p017_ctrl->my_reg_line_count);

    //	rc = mt9p017_i2c_write_w(mt9p017_client->addr,
    //							 MT9P017_REG_RESET_REGISTER, 0x0018 | 0x0002);

    return rc;
}

static int32_t mt9p017_snapshot_config(int mode)
{
    int32_t rc = 0;

    rc = mt9p017_setting(UPDATE_PERIODIC, RES_CAPTURE);
    if (rc < 0)
    {
        return rc;
    }

    mt9p017_ctrl->curr_res = mt9p017_ctrl->pict_res;

    mt9p017_ctrl->sensormode = mode;

    return rc;
}

static int32_t mt9p017_raw_snapshot_config(int mode)
{
    int32_t rc = 0;

    rc = mt9p017_setting(UPDATE_PERIODIC, RES_CAPTURE);
    if (rc < 0)
    {
        return rc;
    }

    mt9p017_ctrl->curr_res = mt9p017_ctrl->pict_res;

    mt9p017_ctrl->sensormode = mode;

    return rc;
}

static int32_t mt9p017_power_down(void)
{
    int32_t rc = 0;

    rc = mt9p017_i2c_write_w(mt9p017_client->addr,
                             MT9P017_REG_RESET_REGISTER,
                             MT9P017_RESET_REGISTER_PWOFF);

    mdelay(5);
    return rc;
}

static void mt9p017_set_af_init(void)
{
    uint8_t i;

    mt9p017_step_position_table[0] = 0;
    for (i = 1; i <= MT9P017_TOTAL_STEPS_NEAR_TO_FAR; i++)
    {
        if (i <= mt9p017_nl_region_boundary1)
        {
            mt9p017_step_position_table[i] = mt9p017_step_position_table[i - 1] + mt9p017_nl_region_code_per_step1;
        }
        else if (i <= mt9p017_nl_region_boundary2)
        {
            mt9p017_step_position_table[i] = mt9p017_step_position_table[i - 1] + mt9p017_l_region_code_per_step1;
        }
        else if (i <= mt9p017_nl_region_boundary3)
        {
            mt9p017_step_position_table[i] = mt9p017_step_position_table[i - 1] + mt9p017_l_region_code_per_step2;
        }
        else if (i <= mt9p017_nl_region_boundary4)
        {
            mt9p017_step_position_table[i] = mt9p017_step_position_table[i - 1] + mt9p017_l_region_code_per_step3;
        }
        else
        {
            mt9p017_step_position_table[i] = mt9p017_step_position_table[i - 1] + mt9p017_l_region_code_per_step4;
        }

        //CDBG("Step Pos Tbl Value: %hd\n", mt9p017_step_position_table[i]);
    }
}

//jkoh
#define DIV_CEIL(x, y) (x / y + (x % y) ? 1 : 0)

//--

static int32_t mt9p017_move_focus(enum mt9p017_move_focus_dir direction, int32_t num_steps)
{
    int16_t step_direction;
    int16_t dest_lens_position;
    int16_t dest_step_position;
    int16_t target_dist;
    int16_t small_step;
    int16_t next_lens_position;

    if (direction == CAMSENSOR_MOVE_FOCUS_NEAR)
    {
        step_direction = 1;
    }
    else if (direction == CAMSENSOR_MOVE_FOCUS_FAR)
    {
        step_direction = -1;
    }
    else
    {
        CDBG("mt9p017_move_focus: illegal focus direction");
        return -EINVAL;
    }

    dest_step_position = mt9p017_ctrl->curr_step_pos + (step_direction * num_steps);
    if (dest_step_position < 0)
    {
        dest_step_position = 0;
    }
    else if (dest_step_position > MT9P017_TOTAL_STEPS_NEAR_TO_FAR)
    {
        dest_step_position = MT9P017_TOTAL_STEPS_NEAR_TO_FAR;
    }

    /* No move */
    if (dest_step_position == mt9p017_ctrl->curr_step_pos)
    {
        return 0;
    }

    CDBG("Cur Step: %hd Step Direction: %hd Dest Step Pos: %hd Num Step: %hd\n", mt9p017_ctrl->curr_step_pos,
         step_direction, dest_step_position, num_steps);

    dest_lens_position = mt9p017_step_position_table[dest_step_position];
    target_dist = step_direction * (dest_lens_position - mt9p017_ctrl->curr_lens_pos);

    CDBG("Target Dist: %hd\n", target_dist);

    //jkoh - take care of ceil !!!

    /* SW damping */
    if ((step_direction < 0) && (target_dist >= mt9p017_step_position_table[mt9p017_damping_threshold]))
    {
        //change to variable
#if 0
        //small_step = (uint16_t)ceil((float)target_dist/(float)10);
        small_step = (uint16_t) DIV_CEIL(target_dist, 10);
        mt9p017_sw_damping_time_wait = 1;
#endif
        small_step = (uint16_t)((target_dist / 10));
        if (small_step == 0)
        {
            small_step = 1;
        }

        mt9p017_sw_damping_time_wait = 1;
    }
    else
    {
        small_step = (uint16_t)(target_dist / 4); // 10

        if (small_step == 0)
        {
            small_step = 1;
        }

        mt9p017_sw_damping_time_wait = 4;
    }

    CDBG("Cur Len Pos: %hd Step Direction: %hd Dest Len Pos: %hd Small Step: %hd\n", mt9p017_ctrl->curr_lens_pos,
         step_direction, dest_lens_position, small_step);
    for (next_lens_position = mt9p017_ctrl->curr_lens_pos + (step_direction * small_step);
         (step_direction * next_lens_position) < (step_direction * dest_lens_position);
         next_lens_position += (step_direction * small_step))
    {
        CDBG("Moving Lens - Next Len Pos: %hd, Step Direction: %hd Dest Len Pos: %hd Small Step: %hd\n",
             next_lens_position, step_direction, dest_lens_position, small_step);

        /* Write sensor rigister, not af chip */
        if (mt9p017_i2c_write_w(mt9p017_client->addr, REG_VCM_NEW_CODE,
                                next_lens_position) < 0)
        {
            CDBG("mt9p017_move_focus failed at line %d ...\n", __LINE__);
            return -EBUSY;
        }

        mt9p017_ctrl->curr_lens_pos = next_lens_position;
        usleep(mt9p017_sw_damping_time_wait * 50);
    }

    if (mt9p017_ctrl->curr_lens_pos != dest_lens_position)
    {
        if (mt9p017_i2c_write_w(mt9p017_client->addr, REG_VCM_NEW_CODE,
                                dest_lens_position) < 0)
        {
            CDBG("mt9p017_move_focus failed at line %d ...\n", __LINE__);
            return -EBUSY;
        }

        usleep(mt9p017_sw_damping_time_wait * 50);
    }

    /* Storing the current lens Position */
    mt9p017_ctrl->curr_lens_pos = dest_lens_position;
    mt9p017_ctrl->curr_step_pos = dest_step_position;

    return 0;
}

static int32_t mt9p017_set_default_focus(void)
{
    int32_t rc = 0;

    /* Write sensor rigister, not af chip */
    if (mt9p017_ctrl->curr_step_pos != 0)
    {
        rc = mt9p017_move_focus(CAMSENSOR_MOVE_FOCUS_FAR,
                                mt9p017_ctrl->curr_step_pos);
    }
    else
    {
        /* rewrite in case sensor is changed */
        rc = mt9p017_i2c_write_w(mt9p017_client->addr,
                                 REG_VCM_NEW_CODE, 0x00);
    }

    mt9p017_ctrl->curr_lens_pos = 0x00;
    mt9p017_ctrl->curr_step_pos = 0x00;
    return rc;
}

static int mt9p017_probe_init_done(const struct msm_camera_sensor_info *data)
{
    /*
    gpio_direction_output(data->sensor_reset, 0);
    gpio_free(data->sensor_reset);

    gpio_direction_output(data->sensor_pwd, 0);
    gpio_free(data->sensor_pwd);

    gpio_direction_output(data->vcm_pwd, 0);
    gpio_free(data->vcm_pwd);

    if (data->vreg_disable_func)
    {
        data->vreg_disable_func(data->sensor_vreg, data->vreg_num);
    }*/
    CDBG("probe done\n");
    gpio_free(data->sensor_reset);

    /*disable the power*/
    if (data->vreg_disable_func)
    {
        data->vreg_disable_func(0);
    }
    return 0;
}

static int mt9p017_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
    int32_t rc;
    uint16_t chipid;

    CDBG(" mt9p017_probe_init_sensor is called\n");
    rc = gpio_request(data->sensor_reset, "mt9p017");
    if (!rc)
    {
        gpio_direction_output(data->sensor_reset, 1);
    }
    else
    {
        goto init_probe_done;
    }

    msleep(20);

    {
        /*hardware reset*/
        rc = gpio_direction_output(data->sensor_reset, 0);
        if (rc < 0)
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
        rc = gpio_direction_output(data->sensor_reset, 1);
        if (rc < 0)
        {
            goto init_probe_fail;
        }
    }

    msleep(20);

    /* 3. Read sensor Model ID: */
    rc = mt9p017_i2c_read_w(mt9p017_client->addr,
                            MT9P017_REG_MODEL_ID, &chipid);

    if (rc < 0)
    {
        goto init_probe_fail;
    }

    CDBG("mt9p017 chipid = 0x%x\n", chipid);

    /* 4. Compare sensor ID to MT9T012VC ID: */
    if (chipid != MT9P017_MODEL_ID)
    {
        CDBG("mt9p017 wrong model_id = 0x%x, doesnot match .\n", chipid);
        rc = -ENODEV;
        goto init_probe_fail;
    }

    goto init_probe_done;

init_probe_fail:
    CDBG(" mt9p017_probe_init_sensor fails\n");
    mt9p017_probe_init_done(data);
init_probe_done:
    return rc;
}

static int mt9p017_sensor_open_init(const struct msm_camera_sensor_info *data)
{
    int32_t rc;

    CDBG("mt9p017_sensor_open_init called !\n");

    mt9p017_ctrl = kzalloc(sizeof(struct mt9p017_ctrl), GFP_KERNEL);
    if (!mt9p017_ctrl)
    {
        CDBG("mt9p017_init failed!\n");
        rc = -ENOMEM;
        goto init_done;
    }

    mt9p017_ctrl->fps_divider = 1 * 0x00000400;
    mt9p017_ctrl->pict_fps_divider = 1 * 0x00000400;
    mt9p017_ctrl->set_test = TEST_OFF;
    mt9p017_ctrl->prev_res = QTR_SIZE;
    mt9p017_ctrl->pict_res = FULL_SIZE;
    mt9p017_ctrl->curr_step_pos = 0;

    if (data)
    {
        mt9p017_ctrl->sensordata = data;
    }

    msm_camio_clk_rate_set(24000000);
    mdelay(20);
    rc = mt9p017_probe_init_sensor(data);
    if (rc < 0)
    {
        goto init_fail1;
    }

    rc = mt9p017_load_settings();
    if (rc < 0)
    {
        CDBG("mt9p017_load_settings failed. rc = %d\n", rc);
        goto init_fail1;
    }

    CDBG("mt9p017_load_settings succeed. rc = %d\n", rc);

    rc = mt9p017_load_pixel_timing();
    if (rc < 0)
    {
        CDBG("mt9p017_load_pixel_timing failed. rc = %d\n", rc);
        goto init_fail1;
    }

    CDBG("mt9p017_load_pixel_timing succeed. rc = %d\n", rc);

    if (mt9p017_ctrl->prev_res == QTR_SIZE)
    {
        rc = mt9p017_setting(REG_INIT, RES_PREVIEW);
    }
    else
    {
        rc = mt9p017_setting(REG_INIT, RES_CAPTURE);
    }

    if (rc < 0)
    {
        CDBG("mt9p017_setting failed. rc = %d\n", rc);
        goto init_fail1;
    }

    /* sensor : output enable */
    CDBG("mt9p017_sensor_open_init(): enabling output.\n");

    rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                             MT9P017_REG_MODE_SELECT,
                             MT9P017_STOP_STREAM);
    if (rc < 0)
    {
        CDBG("sensor output enable failed. rc = %d\n", rc);
        goto init_fail1;
    }

    rc = mt9p017_i2c_write_w(mt9p017_client->addr,
                             0x31E0,
                             0x1F01);
    if (rc < 0)
    {
        CDBG("sensor output enable failed. rc = %d\n", rc);
        goto init_fail1;
    }

    rc = mt9p017_i2c_write_b(mt9p017_client->addr,
                             MT9P017_REG_MODE_SELECT,
                             MT9P017_START_STREAM);
    if (rc < 0)
    {
        CDBG("sensor output enable failed. rc = %d\n", rc);
        goto init_fail1;
    }

    /* enable AF actuator */
    if (mt9p017_ctrl->sensordata->vcm_enable)
    {
        CDBG("AF Table INIT\n");
        mt9p017_set_af_init();
    }

    if (rc >= 0)
    {
        goto init_done;
    }

    CDBG("----------open---err---\n");
init_fail1:
    mt9p017_probe_init_done(data);
    kfree(mt9p017_ctrl);
init_done:
    CDBG("----------open--ok----\n");
    return rc;
}

static int mt9p017_init_client(struct i2c_client *client)
{
    /* Initialize the MSM_CAMI2C Chip */
    init_waitqueue_head(&mt9p017_wait_queue);
    return 0;
}

static int32_t mt9p017_set_sensor_mode(int mode, int res)
{
    int32_t rc = 0;

    switch (mode)
    {
    case SENSOR_PREVIEW_MODE:
        rc = mt9p017_video_config(mode, res);
        break;

    case SENSOR_SNAPSHOT_MODE:
        rc = mt9p017_snapshot_config(mode);
        break;

    case SENSOR_RAW_SNAPSHOT_MODE:
        rc = mt9p017_raw_snapshot_config(mode);
        break;

    default:
        rc = -EINVAL;
        break;
    }

    return rc;
}

int mt9p017_sensor_config(void __user *argp)
{
    struct sensor_cfg_data cdata;
    int rc = 0;

    if (copy_from_user(&cdata,
                       (void *)argp, sizeof(struct sensor_cfg_data)))
    {
        return -EFAULT;
    }

    mutex_lock(&mt9p017_mut);

    CDBG("%s: cfgtype = %d\n", __func__, cdata.cfgtype);
    switch (cdata.cfgtype)
    {
    case CFG_GET_PICT_FPS:
        mt9p017_get_pict_fps(cdata.cfg.gfps.prevfps,
                             &(cdata.cfg.gfps.pictfps));

        if (copy_to_user((void *)argp, &cdata,
                         sizeof(struct sensor_cfg_data)))
        {
            rc = -EFAULT;
        }

        break;

    case CFG_GET_PREV_L_PF:
        cdata.cfg.prevl_pf = mt9p017_get_prev_lines_pf();

        if (copy_to_user((void *)argp,
                         &cdata, sizeof(struct sensor_cfg_data)))
        {
            rc = -EFAULT;
        }

        break;

    case CFG_GET_PREV_P_PL:
        cdata.cfg.prevp_pl = mt9p017_get_prev_pixels_pl();

        if (copy_to_user((void *)argp,
                         &cdata, sizeof(struct sensor_cfg_data)))
        {
            rc = -EFAULT;
        }

        break;

    case CFG_GET_PICT_L_PF:
        cdata.cfg.pictl_pf = mt9p017_get_pict_lines_pf();

        if (copy_to_user((void *)argp,
                         &cdata, sizeof(struct sensor_cfg_data)))
        {
            rc = -EFAULT;
        }

        break;

    case CFG_GET_PICT_P_PL:
        cdata.cfg.pictp_pl = mt9p017_get_pict_pixels_pl();

        if (copy_to_user((void *)argp,
                         &cdata, sizeof(struct sensor_cfg_data)))
        {
            rc = -EFAULT;
        }

        break;

    case CFG_GET_PICT_MAX_EXP_LC:
        cdata.cfg.pict_max_exp_lc = mt9p017_get_pict_max_exp_lc();

        if (copy_to_user((void *)argp,
                         &cdata, sizeof(struct sensor_cfg_data)))
        {
            rc = -EFAULT;
        }

        break;

    case CFG_SET_FPS:
    case CFG_SET_PICT_FPS:
        rc = mt9p017_set_fps(&(cdata.cfg.fps));
        break;

    case CFG_SET_EXP_GAIN:
        rc = mt9p017_write_exp_gain(cdata.cfg.exp_gain.gain,
                                    cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_PICT_EXP_GAIN:
        CDBG("Line:%d CFG_SET_PICT_EXP_GAIN \n", __LINE__);
        rc = mt9p017_set_pict_exp_gain(cdata.cfg.exp_gain.gain,
                                       cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_MODE:
        rc = mt9p017_set_sensor_mode(cdata.mode, cdata.rs);
        break;

    case CFG_PWR_DOWN:
        rc = mt9p017_power_down();
        break;

    case CFG_MOVE_FOCUS:
        CDBG("mt9p017_ioctl: CFG_MOVE_FOCUS: cdata.cfg.focus.dir=%d \
				cdata.cfg.focus.steps=%d\n"                                                          ,
             cdata.cfg.focus.dir, cdata.cfg.focus.steps);
        rc = mt9p017_move_focus((enum mt9p017_move_focus_dir) cdata.cfg.focus.dir,
                                cdata.cfg.focus.steps);
        break;

    case CFG_SET_DEFAULT_FOCUS:
        rc = mt9p017_set_default_focus();
        break;

    case CFG_SET_LENS_SHADING:
        CDBG("%s: CFG_SET_LENS_SHADING\n", __func__);
        rc = mt9p017_lens_shading_enable(cdata.cfg.lens_shading);
        break;

    case CFG_GET_AF_MAX_STEPS:
        cdata.max_steps = MT9P017_STEPS_NEAR_TO_CLOSEST_INF;
        if (copy_to_user((void *)argp,
                         &cdata, sizeof(struct sensor_cfg_data)))
        {
            rc = -EFAULT;
        }

        break;

    case CFG_SET_EFFECT:
    default:
        rc = -EINVAL;
        break;
    }

    mutex_unlock(&mt9p017_mut);
    return rc;
}

int mt9p017_sensor_release(void)
{
    int rc = -EBADF;

    mutex_lock(&mt9p017_mut);

    mt9p017_power_down();

    gpio_direction_output(mt9p017_ctrl->sensordata->sensor_reset, 0);
    gpio_free(mt9p017_ctrl->sensordata->sensor_reset);

    gpio_direction_output(mt9p017_ctrl->sensordata->sensor_pwd, 0);
    gpio_free(mt9p017_ctrl->sensordata->sensor_pwd);

    gpio_direction_output(mt9p017_ctrl->sensordata->vcm_pwd, 0);
    gpio_free(mt9p017_ctrl->sensordata->vcm_pwd);

    /*disable the power*/
    if (mt9p017_ctrl->sensordata->vreg_disable_func)
    {
        mt9p017_ctrl->sensordata->vreg_disable_func(0);
    }
    kfree(mt9p017_ctrl);
    mt9p017_ctrl = NULL;

    CDBG("mt9p017_release completed\n");

    mutex_unlock(&mt9p017_mut);
    return rc;
}

static int mt9p017_i2c_probe(struct i2c_client *         client,
                             const struct i2c_device_id *id)
{
    int rc = 0;

    CDBG("mt9p017_probe called!\n");

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        CDBG("i2c_check_functionality failed\n");
        goto probe_failure;
    }

    mt9p017_sensorw = kzalloc(sizeof(struct mt9p017_work), GFP_KERNEL);
    if (!mt9p017_sensorw)
    {
        CDBG("kzalloc failed.\n");
        rc = -ENOMEM;
        goto probe_failure;
    }

    i2c_set_clientdata(client, mt9p017_sensorw);
    mt9p017_init_client(client);
    mt9p017_client = client;

    mdelay(50);

    CDBG("mt9p017_probe successed! rc = %d\n", rc);
    return 0;

probe_failure:
    CDBG("mt9p017_probe failed! rc = %d\n", rc);
    return rc;
}

static const struct i2c_device_id mt9p017_i2c_id[] =
{
    {"mt9p017", 0},
    {}
};

static struct i2c_driver mt9p017_i2c_driver =
{
    .id_table = mt9p017_i2c_id,
    .probe    = mt9p017_i2c_probe,
    .remove   = __exit_p(mt9p017_i2c_remove),
    .driver   = {
        .name = "mt9p017",
    },
};

static int mt9p017_sensor_probe(const struct msm_camera_sensor_info *info,
                                struct msm_sensor_ctrl *             s)
{
    int rc = i2c_add_driver(&mt9p017_i2c_driver);

    if ((rc < 0) || (mt9p017_client == NULL))
    {
        rc = -ENOTSUPP;
        CDBG("i2c driver add failed .");
        goto probe_done;
    }

    msm_camio_clk_rate_set(MT9P017_DEFAULT_CLOCK_RATE);

    mdelay(20);

    rc = mt9p017_probe_init_sensor(info);
    if (rc < 0)
    {
        i2c_del_driver(&mt9p017_i2c_driver);
        CDBG("mt9p017_probe_init_sensor failed .");
        goto probe_done;
    }
    else
    {
        CDBG("camera sensor mt9p017 probe is succeed!!!\n");
    }
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
    /* detect current device successful, set the flag as present */
    set_hw_dev_flag(DEV_I2C_CAMERA_MAIN);
#endif
    s->s_init = mt9p017_sensor_open_init;
    s->s_release = mt9p017_sensor_release;
    s->s_config = mt9p017_sensor_config;
#ifdef CONFIG_ARCH_MSM7X27A
    /*set the s_mount_angle value of sensor*/
    s->s_mount_angle = info->sensor_platform_info->mount_angle;
#else
    s->s_mount_angle = 0 ;
#endif
    mt9p017_probe_init_done(info);

probe_done:
    CDBG("%s:%d\n", __func__, __LINE__);

    return rc;
}

static int __mt9p017_probe(struct platform_device *pdev)
{
    return msm_camera_drv_start(pdev, mt9p017_sensor_probe);
}

static struct platform_driver msm_camera_driver =
{
    .probe     = __mt9p017_probe,
    .driver    = {
        .name  = "msm_camera_mt9p017",
        .owner = THIS_MODULE,
    },
};

static int __init mt9p017_init(void)
{
    return platform_driver_register(&msm_camera_driver);
}

module_init(mt9p017_init);

