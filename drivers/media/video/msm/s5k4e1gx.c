/*
Copyright (c) 2010, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
 * Neither the name of Code Aurora Forum, Inc. nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Alternatively, and instead of the terms immediately above, this
software may be relicensed by the recipient at their option under the
terms of the GNU General Public License version 2 ("GPL") and only
version 2.  If the recipient chooses to relicense the software under
the GPL, then the recipient shall replace all of the text immediately
above and including this paragraph with the text immediately below
and between the words START OF ALTERNATE GPL TERMS and END OF
ALTERNATE GPL TERMS and such notices and license terms shall apply
INSTEAD OF the notices and licensing terms given above.

START OF ALTERNATE GPL TERMS

Copyright (c) 2010, Code Aurora Forum. All rights reserved.

This software was originally licensed under the Code Aurora Forum
Inc. Dual BSD/GPL License version 1.1 and relicensed as permitted
under the terms thereof by a recipient under the General Public
License Version 2.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 and
only version 2 as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

END OF ALTERNATE GPL TERMS
 */

#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/camera.h>
#include "s5k4e1gx.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
 #include <linux/hw_dev_dec.h>
#endif
#include <asm/mach-types.h>
#undef CDBG
#define CDBG(fmt, args...) printk(KERN_INFO "s5k4e1gx: " fmt, ## args)

static bool CSI_CONFIG;

#define S5K4E1GX_REG_MODEL_ID 0x0000
#define S5K4E1GX_MODEL_ID 0x4E10

/* Frame Fotmat */
#define REG_FRAME_LENGTH_LINES_MSB 0x0340
#define REG_FRAME_LENGTH_LINES_LSB 0x0341
#define REG_LINE_LENGTH_PCK_MSB 0x0342
#define REG_LINE_LENGTH_PCK_LSB 0x0343

/*ADLC SETTING*/
#define REG_ANALOGUE_GAIN_CODE_GLOBAL_MSB 0x0204
#define REG_ANALOGUE_GAIN_CODE_GLOBAL_LSB 0x0205
#define REG_FINE_INTEGRATION_TIME 0x0200
#define REG_COARSE_INTEGRATION_TIME 0x0202
#define REG_COARSE_INTEGRATION_TIME_LSB 0x0203

/* Mode select register */
#define S5K4E1GX_REG_MODE_SELECT 0x0100
#define S5K4E1GX_MODE_SELECT_STREAM 0x01    /* start streaming */
#define S5K4E1GX_MODE_SELECT_SW_STANDBY 0x00    /* software standby */

#define S5K4E1GX_REG_SOFTWARE_RESET 0x0103
#define S5K4E1GX_SOFTWARE_RESET 0x01

#define S5K4E1GX_REG_GROUP_PARAMETER_HOLD 0x0104
#define S5K4E1GX_GROUP_PARAMETER_HOLD 0x01
#define S5K4E1GX_GROUP_PARAMETER_UNHOLD 0x00

#define S5K4E1GX_MIRROR_AND_FLIP 0x0101
#define MIRROR 0x1
#define FLIP 0x2
#define MIRROR_AND_FLIP 0x3

#define REG_TEST_PATTERN_MODE 0x0601

#define S5K4E1GX_AF_I2C_ADDR 0x18

/*optimize the preview and snapshot*/
#define S5K4E1GX_STEPS_NEAR_TO_CLOSEST_INF 44 //48
#define S5K4E1GX_TOTAL_STEPS_NEAR_TO_FAR 44 //48
#define S5K4E1GX_SW_DAMPING_STEP 10
#define S5K4E1GX_MAX_FPS 30
#define S5K4E1GX_MAX_SNAPSHOT_EXP_LC 3961

struct reg_struct
{
    uint8_t  pre_pll_clk_div;               /* 0x0305 */
    uint8_t  pll_multiplier_msb;            /* 0x0306 */
    uint8_t  pll_multiplier_lsb;            /* 0x0307 */
    uint8_t  vt_sys_clk_div;                /* 0x30B5 */
    uint8_t  x_output_size_msb;             /* 0x034C */
    uint8_t  x_output_size_lsb;             /* 0x034D */
    uint8_t  y_output_size_msb;             /* 0x034E */
    uint8_t  y_output_size_lsb;             /* 0x034F */
    uint8_t  x_even_inc;                    /* 0x0381 */
    uint8_t  x_odd_inc;                     /* 0x0383 */
    uint8_t  y_even_inc;                    /* 0x0385 */
    uint8_t  y_odd_inc;                     /* 0x0387 */
    uint8_t  h_binning;                     /* 0x30A9 */
    uint8_t  v_binning;                     /* 0x300E */
    uint8_t  frame_length_lines_msb;        /* 0x0340 */
    uint8_t  frame_length_lines_lsb;        /* 0x0341 */
    uint8_t  line_length_pck_msb;           /* 0x0342 */
    uint8_t  line_length_pck_lsb;           /* 0x0343 */
    uint8_t  ld_start;                      /* 0x3000 */
    uint8_t  sl_start;                      /* 0x3001 */
    uint8_t  rx_start;                      /* 0x3002 */
    uint8_t  cds_start;                     /* 0x3003 */
    uint8_t  smp_width;                     /* 0x3004 */
    uint8_t  az_width;                      /* 0x3005 */
    uint8_t  s1r_width;                     /* 0x3006 */
    uint8_t  tx_start;                      /* 0x3007 */
    uint8_t  tx_width;                      /* 0x3008 */
    uint8_t  stx_width;                     /* 0x3009 */
    uint8_t  dtx_width;                     /* 0x300A */
    uint8_t  rmp_rst_start;                 /* 0x300B */
    uint8_t  rmp_sig_start;                 /* 0x300C */
    uint8_t  rmp_lat;                       /* 0x300D */
    uint8_t  smp_en;                        /* 0x3010 */
    uint8_t  rst_mx;                        /* 0x3011 */
    uint8_t  sig_offset1;                   /* 0x3012 */
    uint8_t  rst_offset1;                   /* 0x3013 */
    uint8_t  sig_offset2;                   /* 0x3014 */
    uint8_t  rst_offset2;                   /* 0x3015 */
    uint8_t  adc_sat;                       /* 0x3016 */
    uint8_t  rmp_init;                      /* 0x3017 */
    uint8_t  rmp_option;                    /* 0x3018 */
    uint8_t  reg_0x301B;                        /* 0x301B */
    uint8_t  clp_level;                     /* 0x301D */
    uint8_t  inrush_ctrl;                   /* 0x3021 */
    uint8_t  pump_ring_osc;                 /* 0x3022 */
    uint8_t  pix_voltage;                   /* 0x3024 */
    uint8_t  ntg_voltage;                   /* 0x3027 */
    uint8_t  pixel_bias;                    /* 0x301C */
    uint8_t  all_tx_off;                    /* 0x30D8 */
    uint8_t  l_adlc_bpr;                    /* 0x3070 */
    uint8_t  f_l_adlc_max;                  /* 0x3071 */
    uint8_t  f_adlc_filter_a;               /* 0x3080 */
    uint8_t  f_adlc_filter_b;               /* 0x3081 */
    uint8_t  sync_mode;                     /* 0x3084 */
    uint8_t  pclk_inv;                      /* 0x3110 */
    uint8_t  pclk_delay;                    /* 0x3117 */
    uint8_t  v_h_strength;                  /* 0x3119 */
    uint8_t  data_pclk_strength;            /* 0x311A */
    uint8_t  analogue_gain_code_global_msb; /* 0x0204 */
    uint8_t  analogue_gain_code_global_lsb; /* 0x0205 */
    uint8_t  fine_integration_time;         /* 0x0200 */
    uint8_t  coarse_integration_time;       /* 0x0202 */
    uint8_t  flip_and_mirror;                           /*0x0101*/
    uint32_t size_h;
    uint32_t blk_l;
    uint32_t size_w;
    uint32_t blk_p;
};

struct reg_struct s5k4e1gx_reg_pat[2] =
{
    {/*Preview*/
        0x06, /* pre_pll_clk_div               REG=0x0305 */
        0x00, /* pll_multiplier_msb            REG=0x0306 */
        0x64, // pll_multiplier_lsb            REG=0x0307
        0x00, /* vt_sys_clk_div                REG=0x30B5 */
        0x05, /* x_output_size_msb             REG=0x034C */
        0x18, /* x_output_size_lsb             REG=0x034D */
        0x03, /* y_output_size_msb             REG=0x034E */
        0xD4, /* y_output_size_lsb             REG=0x034F */
        0x01, /* x_even_inc                    REG=0x0381 */
        0x01, /* x_odd_inc                     REG=0x0383 */
        0x01, /* y_even_inc                    REG=0x0385 */
        0x03, /* y_odd_inc                     REG=0x0387 */
        0x02, /* h_binning                     REG=0x30A9 */
        0xAB, /* v_binning                     REG=0x300E */
        0x03, /* frame_length_lines_msb        REG=0x0340 */
        0xE0, /* frame_length_lines_lsb        REG=0x0341 */
        0x0A, /* line_length_pck_msb           REG=0x0342 */
        0xB2, /* line_length_pck_lsb           REG=0x0343 */
        0x05, //0x04,  // ld_start;                     REG=0x3000
        0x03, //0x02,  // sl_start;                     REG=0x3001
        0x08, //0x0C,  // rx_start;                     REG=0x3002
        0x09, //0x0E,  // cds_start;                    REG=0x3003
        0x2E, //0x2C,  // smp_width;                    REG=0x3004
        0x06, //0x0D,  // az_width;                     REG=0x3005
        0x34, //0x39,  // s1r_width;                    REG=0x3006
        0x00, //0x02,  // tx_start;                     REG=0x3007
        0x3C, /* tx_width;                     REG=0x3008 */
        0x3C, /* stx_width;                    REG=0x3009 */
        0x28, /* dtx_width;                    REG=0x300A */
        0x04, //0x15,  // rmp_rst_start;                REG=0x300B
        0x0A, //0x15,  // rmp_sig_start;                REG=0x300C
        0x02, /* rmp_lat;                      REG=0x300D */
        0x00, /* smp_en;                       REG=0x3010 */
        0x4C, //0x7A,  // rst_mx;                       REG=0x3011
        0x30, /* sig_offset1;                  REG=0x3012 */
        0xC0, //0x90,  // rst_offset1;                  REG=0x3013
        0x00, /* sig_offset2;                  REG=0x3014 */
        0x00, /* rst_offset2;                  REG=0x3015 */
        0x2C, //0x02,  // adc_sat;                      REG=0x3016
        0x94, //0x84,  // rmp_init;                     REG=0x3017
        0x78, /* rmp_option;                   REG=0x3018 */
        0x83, // reg_0x301B;                   REG=0x301B
        0xD4, /* clp_level;                    REG=0x301D */
        0x02, /* inrush_ctrl;                  REG=0x3021 */
        0x24, //0x44,  // pump_ring_osc;                REG=0x3022
        0x40, /* pix_voltage;                  REG=0x3024 */
        0x08, /* ntg_voltage;                  REG=0x3027 */
        0x04, //0x05,  // pixel_bias;                   REG=0x301C
        0x3F, /* all_tx_off;                   REG=0x30D8 */
        0x5F, /* l_adlc_bpr;                   REG=0x3070 */
        0x00, /* f_l_adlc_max;                 REG=0x3071 */
        0x04, /* f_adlc_filter_a;              REG=0x3080 */
        0x38, /* f_adlc_filter_b;              REG=0x3081 */
        0x15, /* sync_mode;                    REG=0x3084 */
        0x10, /* pclk_inv;                     REG=0x3110 */
        0x05, /* pclk_delay;                   REG=0x3117 */ //0x06
        0x0A, /* v_h_strength;                 REG=0x3119 */
        0xFA, /* data_pclk_strength;           REG=0x311A */ //0xFA
        0x00, /* analogue_gain_code_global_msb REG=0x0204 */
        0x80, /* analogue_gain_code_global_lsb REG=0x0205 */
        0x02, /* fine_integration_time         REG=0x0200 */
        0x03, // coarse_integration_time       REG=0x0202
        0x03,
        980,
        12,
        1304,
        1434
    },
    { /*Snapshot*/
        0x06, /* pre_pll_clk_div               REG=0x0305 */
        0x00, /* pll_multiplier_msb            REG=0x0306 */
        0x64, // pll_multiplier_lsb            REG=0x0307
        0x00, /* vt_sys_clk_div                REG=0x30B5 */
        0x0A, /* x_output_size_msb             REG=0x034C */
        0x30, /* x_output_size_lsb             REG=0x034D */
        0x07, /* y_output_size_msb             REG=0x034E */
        0xA8, /* y_output_size_lsb             REG=0x034F */
        0x01, /* x_even_inc                    REG=0x0381 */
        0x01, /* x_odd_inc                     REG=0x0383 */
        0x01, /* y_even_inc                    REG=0x0385 */
        0x01, /* y_odd_inc                     REG=0x0387 */
        0x03, /* h_binning                     REG=0x30A9 */
        0xA8, /* v_binning                     REG=0x300E */
        0x07, /* frame_length_lines_msb        REG=0x0340 */
        0xB4, /* frame_length_lines_lsb        REG=0x0341 */
        0x0A, /* line_length_pck_msb           REG=0x0342 */
        0xB2, /* line_length_pck_lsb           REG=0x0343 */
        0x05, //0x04,  // ld_start;                     REG=0x3000
        0x03, //0x02,  // sl_start;                     REG=0x3001
        0x08, //0x0C,  // rx_start;                     REG=0x3002
        0x09, //0x0E,  // cds_start;                    REG=0x3003
        0x2E, //0x2C,  // smp_width;                    REG=0x3004
        0x06, //0x0D,  // az_width;                     REG=0x3005
        0x34, //0x39,  // s1r_width;                    REG=0x3006
        0x00, //0x02,  // tx_start;                     REG=0x3007
        0x3C, /* tx_width;                     REG=0x3008 */
        0x3C, /* stx_width;                    REG=0x3009 */
        0x28, /* dtx_width;                    REG=0x300A */
        0x04, //0x15,  // rmp_rst_start;                REG=0x300B
        0x0A, //0x15,  // rmp_sig_start;                REG=0x300C
        0x02, /* rmp_lat;                      REG=0x300D */
        0x00, /* smp_en;                       REG=0x3010 */
        0x4C, //0x7A,  // rst_mx;                       REG=0x3011
        0x30, /* sig_offset1;                  REG=0x3012 */
        0xC0, //0xA0,  // rst_offset1;                  REG=0x3013
        0x00, /* sig_offset2;                  REG=0x3014 */
        0x00, /* rst_offset2;                  REG=0x3015 */
        0x2C, //0x02,  // adc_sat;                      REG=0x3016
        0x94, /* rmp_init;                     REG=0x3017 */
        0x78, /* rmp_option;                   REG=0x3018 */
        0x71, // reg_0x301B;                   REG=0x301B
        0xD4, /* clp_level;                    REG=0x301D */
        0x02, /* inrush_ctrl;                  REG=0x3021 */
        0x24, //0x44,  // pump_ring_osc;                REG=0x3022
        0x40, /* pix_voltage;                  REG=0x3024 */
        0x08, /* ntg_voltage;                  REG=0x3027 */
        0x04, //0x05,  // pixel_bias;                   REG=0x301C
        0x3F, /* all_tx_off;                   REG=0x30D8 */
        0x5F, /* l_adlc_bpr;                   REG=0x3070 */
        0x00, /* f_l_adlc_max;                 REG=0x3071 */
        0x04, /* f_adlc_filter_a;              REG=0x3080 */
        0x38, /* f_adlc_filter_b;              REG=0x3081 */
        0x15, /* sync_mode;                    REG=0x3084 */
        0x10, /* pclk_inv;                     REG=0x3110 */
        0x05, /* pclk_delay;                   REG=0x3117 */ //06,0d
        0x0A, /* v_h_strength;                 REG=0x3119 */
        0xFA, /* data_pclk_strength;           REG=0x311A */ //AA
        0x00, /* analogue_gain_code_global_msb REG=0x0204 */
        0x80, /* analogue_gain_code_global_lsb REG=0x0205 */
        0x02, /* fine_integration_time         REG=0x0200 */
        0x03, // coarse_integration_time       REG=0x0202
        0x03,
        1960,
        12,
        2608,
        130
    }
};

struct s5k4e1gx_work
{
    struct work_struct work;
};
static struct s5k4e1gx_work *s5k4e1gx_sensorw;
static struct i2c_client *s5k4e1gx_client;
static uint16_t s5k4e1gx_pos_tbl[S5K4E1GX_TOTAL_STEPS_NEAR_TO_FAR + 1];

struct s5k4e1gx_ctrl
{
    const struct msm_camera_sensor_info *sensordata;

    int      sensormode;
    uint32_t fps_divider; /* init to 1 * 0x00000400 */
    uint32_t pict_fps_divider; /* init to 1 * 0x00000400 */
    uint16_t curr_step_pos;
    uint16_t curr_lens_pos;
    uint16_t init_curr_lens_pos;
    uint16_t my_reg_gain;
    uint32_t my_reg_line_count;

    enum msm_s_resolution prev_res;
    enum msm_s_resolution pict_res;
    enum msm_s_resolution curr_res;
    enum msm_s_test_mode  set_test;
};

static struct s5k4e1gx_ctrl *s5k4e1gx_ctrl;
static DECLARE_WAIT_QUEUE_HEAD(s5k4e1gx_wait_queue);
DEFINE_MUTEX(s5k4e1gx_mutex);
static uint8_t s5k4e1gx_mode_mask   = 0;
uint16_t s5k4e1gx_damping_threshold = 10;
uint16_t s5k4e1gx_sw_damping_step = 10;
uint16_t s5k4e1gx_sw_damping_time_wait = 1;
uint16_t debug_focus_method = 0;
uint16_t s5k4e1gx_debug_total_step = 40;

static int s5k4e1gx_i2c_rxdata(unsigned short saddr, unsigned char *rxdata,
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

    if (i2c_transfer(s5k4e1gx_client->adapter, msgs, 2) < 0)
    {
        CDBG("s5k4e1gx_i2c_rxdata failed!\n");
        return -EIO;
    }

    return 0;
}

static int32_t s5k4e1gx_i2c_txdata(unsigned short saddr,
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

    if (i2c_transfer(s5k4e1gx_client->adapter, msg, 1) < 0)
    {
        CDBG("s5k4e1gx_i2c_txdata failed\n");
        return -EIO;
    }

    return 0;
}

static int32_t s5k4e1gx_i2c_write_b(unsigned short saddr, unsigned short waddr,
                                    unsigned char bdata)
{
    int32_t rc = -EIO;
    unsigned char buf[4];

    memset(buf, 0, sizeof(buf));
    buf[0] = (waddr & 0xFF00) >> 8;
    buf[1] = (waddr & 0x00FF);
    buf[2] = bdata;

    rc = s5k4e1gx_i2c_txdata(saddr, buf, 3);

    if (rc < 0)
    {
        CDBG("i2c_write_w failed, addr = 0x%x, val = 0x%x!\n",
             waddr, bdata);
    }

    return rc;
}

static int32_t s5k4e1gx_i2c_write_table(struct s5k4e1gx_i2c_reg_conf *reg_cfg_tbl, int num)
{
    int i;
    int32_t rc = -EIO;

    for (i = 0; i < num; i++)
    {
        rc = s5k4e1gx_i2c_write_b(s5k4e1gx_client->addr,
                                  reg_cfg_tbl->waddr, reg_cfg_tbl->bdata);
        if (rc < 0)
        {
            break;
        }

        reg_cfg_tbl++;
    }

    return rc;
}

static int32_t s5k4e1gx_i2c_read_w(unsigned short saddr, unsigned short raddr,
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

    rc = s5k4e1gx_i2c_rxdata(saddr, buf, 2);
    if (rc < 0)
    {
        return rc;
    }

    *rdata = buf[0] << 8 | buf[1];

    if (rc < 0)
    {
        CDBG("s5k4e1gx_i2c_read failed!\n");
    }

    return rc;
}

static int s5k4e1gx_probe_init_done(const struct msm_camera_sensor_info *data)
{
    CDBG("s5k4e1gx_probe_init_done start\n");
    gpio_direction_output(data->sensor_reset, 0);
    gpio_free(data->sensor_reset);
    gpio_free(data->sensor_pwd);
    gpio_free(data->vcm_pwd);

    /*probe finish ,power down camera*/
    if (data->vreg_disable_func)
    {
        data->vreg_disable_func(0);
    }

    return 0;
}

static int s5k4e1gx_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
    int32_t rc;
    uint16_t chipid;

    CDBG("s5k4e1gx_probe_init_sensor\n");

    rc = gpio_request(data->sensor_pwd, "s5k4e1gx");
    if (!rc)
    {
        gpio_direction_output(data->sensor_pwd, 1);
    }
    else
    {
        CDBG("gpio_request(data->sensor_pwd, s5k4e1g) Failed \n");
        goto init_probe_done;
    }

    if (data->vreg_enable_func)
    {
        data->vreg_enable_func(1);
    }

    rc = gpio_request(data->sensor_reset, "s5k4e1gx");
    if (!rc)
    {
        gpio_direction_output(data->sensor_reset, 1);
    }
    else
    {
        gpio_free(data->sensor_reset);
        gpio_request(data->sensor_reset, "s5k4e1gx");
        rc = gpio_direction_output(data->sensor_reset, 1);
        CDBG("gpio_direction_output(data->sensor_reset, 0) rc=%d \n", rc);
        goto init_probe_done;
    }

    mdelay(1);

    rc = gpio_request(data->vcm_pwd, "s5k4e1gx");
    if (!rc)
    {
        gpio_direction_output(data->vcm_pwd, 1);
    }
    else
    {
        gpio_free(data->vcm_pwd);
        gpio_request(data->vcm_pwd, "s5k4e1gx");
        rc = gpio_direction_output(data->vcm_pwd, 1);
        CDBG("gpio_direction_output(data->sensor_reset, 0) rc=%d \n", rc);
        goto init_probe_done;
    }

    msleep(66);

    /* RESET the sensor image part via I2C command */
    CDBG("s5k4e1gx_sensor_init(): reseting sensor.\n");
    rc = s5k4e1gx_i2c_write_b(s5k4e1gx_client->addr,
                              0x0100, 0x00);
    if (rc < 0)
    {
        CDBG("sensor reset failed. rc = %d\n", rc);
        goto init_probe_fail;
    }

    rc = s5k4e1gx_i2c_write_b(s5k4e1gx_client->addr,
                              0x0103, 0x01);
    if (rc < 0)
    {
        CDBG("sensor reset failed. rc = %d\n", rc);
        goto init_probe_fail;
    }

    msleep(10);

    /* 3. Read sensor Model ID: */
    rc = s5k4e1gx_i2c_read_w(s5k4e1gx_client->addr,
                             S5K4E1GX_REG_MODEL_ID, &chipid);
    if (rc < 0)
    {
        goto init_probe_fail;
    }

    CDBG("s5k4e1gx  model_id = 0x%x\n", chipid);

    /* 4. Compare sensor ID to S5K4E1GX ID: */
    if (chipid != S5K4E1GX_MODEL_ID)
    {
        CDBG("s5k4e1gx wrong model_id = 0x%x\n", chipid);
        rc = -ENODEV;
        goto init_probe_fail;
    }

    goto init_probe_done;

    CDBG("init_probe_done start\n");
init_probe_fail:
    s5k4e1gx_probe_init_done(data);
init_probe_done:
    return rc;
}

static int s5k4e1gx_init_client(struct i2c_client *client)
{
    /* Initialize the MSM_CAMI2C Chip */
    init_waitqueue_head(&s5k4e1gx_wait_queue);
    return 0;
}

static const struct i2c_device_id s5k4e1gx_i2c_id[] =
{
    { "s5k4e1gx", 0},
    { }
};

/*optimize snapshot*/
static void s5k4e1gx_setup_af_tbl(void)
{
    int i;
    uint16_t s5k4e1gx_nl_region_boundary1 = 1;
    uint16_t s5k4e1gx_nl_region_boundary2 = 4;
    uint16_t s5k4e1gx_nl_region_code_per_step1 = 60;
    uint16_t s5k4e1gx_nl_region_code_per_step2 = 20;
    uint16_t s5k4e1gx_l_region_code_per_step = 8;

    s5k4e1gx_pos_tbl[0] = 60;
    for (i = 1; i <= S5K4E1GX_TOTAL_STEPS_NEAR_TO_FAR; i++)
    {
        if (i <= s5k4e1gx_nl_region_boundary1)
        {
            s5k4e1gx_pos_tbl[i] = s5k4e1gx_pos_tbl[i - 1] + s5k4e1gx_nl_region_code_per_step1;
        }
        else if (i <= s5k4e1gx_nl_region_boundary2)
        {
            s5k4e1gx_pos_tbl[i] = s5k4e1gx_pos_tbl[i - 1] + s5k4e1gx_nl_region_code_per_step2;
        }
        else
        {
            s5k4e1gx_pos_tbl[i] = s5k4e1gx_pos_tbl[i - 1] + s5k4e1gx_l_region_code_per_step;
        }
    }
}

static int s5k4e1gx_i2c_probe(struct i2c_client *         client,
                              const struct i2c_device_id *id)
{
    int rc = 0;

    CDBG("s5k4e1gx_probe called!\n");

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        CDBG("i2c_check_functionality failed\n");
        goto probe_failure;
    }

    s5k4e1gx_sensorw = kzalloc(sizeof(struct s5k4e1gx_work), GFP_KERNEL);
    if (!s5k4e1gx_sensorw)
    {
        CDBG("kzalloc failed.\n");
        rc = -ENOMEM;
        goto probe_failure;
    }

    i2c_set_clientdata(client, s5k4e1gx_sensorw);
    s5k4e1gx_init_client(client);
    s5k4e1gx_client = client;

    mdelay(50);

    CDBG("s5k4e1gx_probe successed! rc = %d\n", rc);
    return 0;

probe_failure:
    CDBG("s5k4e1gx_probe failed! rc = %d\n", rc);
    return rc;
}

static struct i2c_driver s5k4e1gx_i2c_driver =
{
    .id_table = s5k4e1gx_i2c_id,
    .probe    = s5k4e1gx_i2c_probe,
    .remove   = __exit_p(s5k4e1gx_i2c_remove),
    .driver   = {
        .name = "s5k4e1gx",
    },
};

static void s5k4e1gx_stop_stream(void)
{
    s5k4e1gx_i2c_write_b(s5k4e1gx_client->addr, 0x0100, 0x00);/* streaming off */
}

static void s5k4e1gx_reset_sensor(void)
{
    s5k4e1gx_i2c_write_b(s5k4e1gx_client->addr, 0x103, 0x1);
}

static int32_t s5k4e1gx_setting(enum msm_s_reg_update rupdate,
                                enum msm_s_setting    rt)
{
    int32_t rc = 0;
    struct msm_camera_csi_params s5k4e1_csi_params;

    s5k4e1gx_stop_stream();
    msleep(30);

    switch (rupdate)
    {
    case S_UPDATE_PERIODIC:

        if (rt == S_RES_PREVIEW)
        {
            CDBG("sensor preview!\n");
            s5k4e1gx_i2c_write_table(s5k4e1_regs.reg_prev,
                                     s5k4e1_regs.reg_prev_size);
        }
        else if (rt == S_RES_CAPTURE)
        {
            CDBG("sensor capture!\n");
            s5k4e1gx_i2c_write_table(s5k4e1_regs.reg_snap,
                                     s5k4e1_regs.reg_snap_size);
        }

        msleep(20);
        if (!CSI_CONFIG)
        {
            CDBG("MIPI CONFIG!\n");
            msm_camio_vfe_clk_rate_set(192000000);
            s5k4e1_csi_params.data_format = CSI_10BIT;
            s5k4e1_csi_params.lane_cnt = 1;
            s5k4e1_csi_params.lane_assign = 0xe4;
            s5k4e1_csi_params.dpcm_scheme = 0;
            s5k4e1_csi_params.settle_cnt = 24;
            rc = msm_camio_csi_config(&s5k4e1_csi_params);
            msleep(20);
            CSI_CONFIG = 1;
        }
        s5k4e1gx_i2c_write_b(s5k4e1gx_client->addr, 0x0100, 0x01);/* streaming on */
        msleep(30);

        break; /* UPDATE_PERIODIC */

    case S_REG_INIT:
        CDBG("sensor init!\n");
        s5k4e1gx_reset_sensor();
        s5k4e1gx_i2c_write_table(s5k4e1_regs.reg_mipi,
                                 s5k4e1_regs.reg_mipi_size);
        s5k4e1gx_i2c_write_table(s5k4e1_regs.rec_settings,
                                 s5k4e1_regs.rec_size);
        s5k4e1gx_i2c_write_table(s5k4e1_regs.reg_pll_p,
                                 s5k4e1_regs.reg_pll_p_size);
        CSI_CONFIG = 0;

        mdelay(5);
        break; /* case REG_INIT: */
    default:
        rc = -EINVAL;
        break;
    } /* switch (rupdate) */
    return rc;
}

static int s5k4e1gx_sensor_open_init(const struct msm_camera_sensor_info *data)
{
    int32_t rc;

    CDBG("open init\n");
    s5k4e1gx_ctrl = kzalloc(sizeof(struct s5k4e1gx_ctrl), GFP_KERNEL);
    if (!s5k4e1gx_ctrl)
    {
        CDBG("s5k4e1gx_init failed!\n");
        rc = -ENOMEM;
        goto init_done;
    }

    s5k4e1gx_ctrl->fps_divider = 1 * 0x00000400;
    s5k4e1gx_ctrl->pict_fps_divider = 1 * 0x00000400;
    s5k4e1gx_ctrl->set_test = S_TEST_OFF;
    s5k4e1gx_ctrl->prev_res = S_QTR_SIZE;
    s5k4e1gx_ctrl->pict_res = S_FULL_SIZE;
    s5k4e1gx_ctrl->curr_step_pos = 0;
    s5k4e1gx_ctrl->curr_lens_pos = 0;

    if (data)
    {
        s5k4e1gx_ctrl->sensordata = data;
    }

    CDBG("open init\n");

    /* enable mclk first */
    msm_camio_clk_rate_set(24000000);
    mdelay(20);
    CDBG("open init\n");

    CDBG("open init\n");
    rc = s5k4e1gx_probe_init_sensor(data);
    if (rc < 0)
    {
        goto init_fail1;
    }

    if (s5k4e1gx_ctrl->prev_res == S_QTR_SIZE)
    {
        rc = s5k4e1gx_setting(S_REG_INIT, S_RES_PREVIEW);
    }
    else
    {
        rc = s5k4e1gx_setting(S_REG_INIT, S_RES_CAPTURE);
    }

    if (rc < 0)
    {
        CDBG("s5k4e1gx_setting failed. rc = %d\n", rc);
        goto init_fail1;
    }

    CDBG("setting done");

    /* set up lens position talbe */
    s5k4e1gx_setup_af_tbl();

    goto init_done;

init_fail1:
    s5k4e1gx_probe_init_done(data);
    kfree(s5k4e1gx_ctrl);
init_done:
    return rc;
}

/*quit camera application ,power down camera*/
static int32_t s5k4e1gx_power_down(void)
{
    int32_t rc = 0;

    if (s5k4e1gx_ctrl->sensordata->vreg_disable_func)
    {
        s5k4e1gx_ctrl->sensordata->vreg_disable_func(0);
    }

    return rc;
}

static int s5k4e1gx_sensor_release(void)
{
    int rc = -EBADF;

    mutex_lock(&s5k4e1gx_mutex);

    gpio_direction_output(s5k4e1gx_ctrl->sensordata->sensor_reset,
                          0);
    gpio_free(s5k4e1gx_ctrl->sensordata->sensor_reset);
    gpio_free(s5k4e1gx_ctrl->sensordata->sensor_pwd);
    rc = s5k4e1gx_power_down();

    kfree(s5k4e1gx_ctrl);
    s5k4e1gx_ctrl = NULL;

    CDBG("s5k4e1gx_release completed\n");

    mutex_unlock(&s5k4e1gx_mutex);
    return rc;
}

static void s5k4e1gx_get_pict_fps(uint16_t fps, uint16_t *pfps)
{
    /* input fps is preview fps in Q8 format */
    uint32_t divider;   /*Q10 */
    uint32_t d1;
    uint32_t d2;

    d1 =
        (uint32_t)(
        ((s5k4e1gx_reg_pat[S_RES_PREVIEW].size_h +
          s5k4e1gx_reg_pat[S_RES_PREVIEW].blk_l) *
         0x00000400) /
        (s5k4e1gx_reg_pat[S_RES_CAPTURE].size_h +
         s5k4e1gx_reg_pat[S_RES_CAPTURE].blk_l));

    d2 =
        (uint32_t)(
        ((s5k4e1gx_reg_pat[S_RES_PREVIEW].size_w +
          s5k4e1gx_reg_pat[S_RES_PREVIEW].blk_p) *
         0x00000400) /
        (s5k4e1gx_reg_pat[S_RES_CAPTURE].size_w +
         s5k4e1gx_reg_pat[S_RES_CAPTURE].blk_p));

    divider = (uint32_t) (d1 * d2) / 0x00000400;

    /* Verify PCLK settings and frame sizes. */
    *pfps = (uint16_t)(fps * divider / 0x00000400);
}

static uint16_t s5k4e1gx_get_prev_lines_pf(void)
{
    return s5k4e1gx_reg_pat[S_RES_PREVIEW].size_h +
           s5k4e1gx_reg_pat[S_RES_PREVIEW].blk_l;
}

static uint16_t s5k4e1gx_get_prev_pixels_pl(void)
{
    return s5k4e1gx_reg_pat[S_RES_PREVIEW].size_w +
           s5k4e1gx_reg_pat[S_RES_PREVIEW].blk_p;
}

static uint16_t s5k4e1gx_get_pict_lines_pf(void)
{
    return s5k4e1gx_reg_pat[S_RES_CAPTURE].size_h +
           s5k4e1gx_reg_pat[S_RES_CAPTURE].blk_l;
}

static uint16_t s5k4e1gx_get_pict_pixels_pl(void)
{
    return s5k4e1gx_reg_pat[S_RES_CAPTURE].size_w +
           s5k4e1gx_reg_pat[S_RES_CAPTURE].blk_p;
}

static uint32_t s5k4e1gx_get_pict_max_exp_lc(void)
{
    uint32_t snapshot_lines_per_frame;

    if (s5k4e1gx_ctrl->pict_res == S_QTR_SIZE)
    {
        snapshot_lines_per_frame =
            s5k4e1gx_reg_pat[S_RES_PREVIEW].size_h +
            s5k4e1gx_reg_pat[S_RES_PREVIEW].blk_l;
    }
    else
    {
        snapshot_lines_per_frame = S5K4E1GX_MAX_SNAPSHOT_EXP_LC * 3;
    }

    return snapshot_lines_per_frame;
}

static int32_t s5k4e1gx_set_fps(struct fps_cfg *fps)
{
    /* input is new fps in Q10 format */
    int32_t rc = 0;

    s5k4e1gx_ctrl->fps_divider = fps->fps_div;

    rc = s5k4e1gx_i2c_write_b(s5k4e1gx_client->addr,
                              REG_FRAME_LENGTH_LINES_MSB,
                              (((s5k4e1gx_reg_pat[S_RES_PREVIEW].size_h +
                                 s5k4e1gx_reg_pat[S_RES_PREVIEW].blk_l) *
                                s5k4e1gx_ctrl->fps_divider / 0x400) & 0xFF00) >> 8);
    if (rc < 0)
    {
        goto set_fps_done;
    }

    rc = s5k4e1gx_i2c_write_b(s5k4e1gx_client->addr,
                              REG_FRAME_LENGTH_LINES_LSB,
                              (((s5k4e1gx_reg_pat[S_RES_PREVIEW].size_h +
                                 s5k4e1gx_reg_pat[S_RES_PREVIEW].blk_l) *
                                s5k4e1gx_ctrl->fps_divider / 0x400) & 0x00FF));

set_fps_done:
    return rc;
}

/*optimize AF default pos*/
static int32_t s5k4e1gx_write_exp_gain(uint16_t gain, uint32_t line)
{
    int32_t rc = 0;

    uint16_t max_legal_gain = 0x0200;
    uint32_t ll_ratio; /* Q10 */
    uint32_t ll_pck, fl_lines;
    uint16_t offset = 4;
    uint32_t gain_msb, gain_lsb;
    uint32_t intg_t_msb, intg_t_lsb;
    uint32_t ll_pck_msb, ll_pck_lsb;

    struct s5k4e1gx_i2c_reg_conf tbl[3];

    CDBG("Line:%d s5k4e1gx_write_exp_gain \n", __LINE__);
    CDBG("Gain is %d, Line count is %d\n", gain, line);
    if (s5k4e1gx_ctrl->sensormode == SENSOR_PREVIEW_MODE)
    {
        s5k4e1gx_ctrl->my_reg_gain = gain;
        s5k4e1gx_ctrl->my_reg_line_count = (uint16_t)line;

        fl_lines = s5k4e1gx_reg_pat[S_RES_PREVIEW].size_h +
                   s5k4e1gx_reg_pat[S_RES_PREVIEW].blk_l;

        ll_pck = s5k4e1gx_reg_pat[S_RES_PREVIEW].size_w +
                 s5k4e1gx_reg_pat[S_RES_PREVIEW].blk_p;
    }
    else
    {
        fl_lines = s5k4e1gx_reg_pat[S_RES_CAPTURE].size_h +
                   s5k4e1gx_reg_pat[S_RES_CAPTURE].blk_l;

        ll_pck = s5k4e1gx_reg_pat[S_RES_CAPTURE].size_w +
                 s5k4e1gx_reg_pat[S_RES_CAPTURE].blk_p;
    }

    if (gain > max_legal_gain)
    {
        gain = max_legal_gain;
    }

    if (s5k4e1gx_ctrl->sensormode != SENSOR_PREVIEW_MODE)
    {
        CDBG("ll_pck :0x%x, gain : 0x%x, line : 0x%x \n", ll_pck, gain, line);
    }
    else
    {
        //CDBG("ll_pck :0x%x, gain : 0x%x, line : 0x%x \n", ll_pck, gain, line);
    }

    line = (line * s5k4e1gx_ctrl->fps_divider);

    if (fl_lines < (line / 0x400))
    {
        ll_ratio = (line / (fl_lines - offset));
    }
    else
    {
        ll_ratio = 0x400;
    }

    /* update gain registers */
    gain_msb = (gain & 0xFF00) >> 8;
    gain_lsb = gain & 0x00FF;
    tbl[0].waddr = S5K4E1GX_REG_GROUP_PARAMETER_HOLD;
    tbl[0].bdata = S5K4E1GX_GROUP_PARAMETER_HOLD;
    tbl[1].waddr = REG_ANALOGUE_GAIN_CODE_GLOBAL_MSB;
    tbl[1].bdata = gain_msb;
    tbl[2].waddr = REG_ANALOGUE_GAIN_CODE_GLOBAL_LSB;
    tbl[2].bdata = gain_lsb;
    rc = s5k4e1gx_i2c_write_table(&tbl[0], ARRAY_SIZE(tbl));
    if (rc < 0)
    {
        goto write_gain_done;
    }

    ll_pck = ll_pck * ll_ratio;
    ll_pck_msb   = ((ll_pck / 0x400) & 0xFF00) >> 8;
    ll_pck_lsb   = (ll_pck / 0x400) & 0x00FF;
    tbl[0].waddr = REG_LINE_LENGTH_PCK_MSB;
    tbl[0].bdata = ll_pck_msb;
    tbl[1].waddr = REG_LINE_LENGTH_PCK_LSB;
    tbl[1].bdata = ll_pck_lsb;
    rc = s5k4e1gx_i2c_write_table(&tbl[0], ARRAY_SIZE(tbl));
    if (rc < 0)
    {
        goto write_gain_done;
    }

    if (s5k4e1gx_ctrl->sensormode != SENSOR_PREVIEW_MODE)
    {
        CDBG("ll_pck :0x%x, gain : 0x%x, line : 0x%x \n", ll_pck, gain, line);
    }
    else
    {
        //CDBG("ll_pck :0x%x, gain : 0x%x, line : 0x%x \n", ll_pck, gain, line);
    }

    line = line / ll_ratio;

    //line = line / 0x400;
    intg_t_msb   = (line & 0xFF00) >> 8;
    intg_t_lsb   = (line & 0x00FF);
    tbl[0].waddr = REG_COARSE_INTEGRATION_TIME;
    tbl[0].bdata = intg_t_msb;
    tbl[1].waddr = REG_COARSE_INTEGRATION_TIME_LSB;
    tbl[1].bdata = intg_t_lsb;
    tbl[2].waddr = S5K4E1GX_REG_GROUP_PARAMETER_HOLD;
    tbl[2].bdata = S5K4E1GX_GROUP_PARAMETER_UNHOLD;
    rc = s5k4e1gx_i2c_write_table(&tbl[0], ARRAY_SIZE(tbl));

    if (s5k4e1gx_ctrl->sensormode != SENSOR_PREVIEW_MODE)
    {
        CDBG("ll_pck :0x%x, gain : 0x%x, line : 0x%x \n", ll_pck, gain, line);
    }

write_gain_done:
    return rc;
}

static int32_t s5k4e1gx_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
    int32_t rc = 0;

    CDBG("Line:%d s5k4e1gx_set_pict_exp_gain \n", __LINE__);

    rc =
        s5k4e1gx_write_exp_gain(gain, line);

    return rc;
}

static int32_t s5k4e1gx_video_config(int mode, int res)
{
    int32_t rc;

    switch (res)
    {
    case S_QTR_SIZE:
        rc = s5k4e1gx_setting(S_UPDATE_PERIODIC, S_RES_PREVIEW);
        if (rc < 0)
        {
            return rc;
        }

        CDBG("s5k4e1gx sensor configuration done!\n");
        break;

    case S_FULL_SIZE:
        rc = s5k4e1gx_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);
        if (rc < 0)
        {
            return rc;
        }

        break;

    default:
        return 0;
    } /* switch */

    s5k4e1gx_ctrl->prev_res   = res;
    s5k4e1gx_ctrl->curr_res   = res;
    s5k4e1gx_ctrl->sensormode = mode;

    rc =
        s5k4e1gx_write_exp_gain(s5k4e1gx_ctrl->my_reg_gain,
                                s5k4e1gx_ctrl->my_reg_line_count);

    return rc;
}

static int32_t s5k4e1gx_snapshot_config(int mode)
{
    int32_t rc = 0;

    rc = s5k4e1gx_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);
    if (rc < 0)
    {
        return rc;
    }

    s5k4e1gx_ctrl->curr_res   = s5k4e1gx_ctrl->pict_res;
    s5k4e1gx_ctrl->sensormode = mode;

    return rc;
}

static int32_t s5k4e1gx_raw_snapshot_config(int mode)
{
    int32_t rc = 0;

    rc = s5k4e1gx_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);
    if (rc < 0)
    {
        return rc;
    }

    s5k4e1gx_ctrl->curr_res   = s5k4e1gx_ctrl->pict_res;
    s5k4e1gx_ctrl->sensormode = mode;

    return rc;
}

static int32_t s5k4e1gx_set_sensor_mode(int mode, int res)
{
    int32_t rc = 0;

    switch (mode)
    {
    case SENSOR_PREVIEW_MODE:
        rc = s5k4e1gx_video_config(mode, res);
        break;

    case SENSOR_SNAPSHOT_MODE:
        rc = s5k4e1gx_snapshot_config(mode);
        break;

    case SENSOR_RAW_SNAPSHOT_MODE:
        rc = s5k4e1gx_raw_snapshot_config(mode);
        break;

    default:
        rc = -EINVAL;
        break;
    }

    return rc;
}

static int32_t s5k4e1gx_af_i2c_write(uint16_t data)
{
    uint8_t code_val_msb, code_val_lsb;
    int32_t rc = 0;
    unsigned char buf[2];

    code_val_msb  = data >> 4;
    code_val_lsb  = (data & 0x000F) << 4;
    code_val_lsb |= s5k4e1gx_mode_mask;
    buf[0] = code_val_msb;
    buf[1] = code_val_lsb;
    rc = s5k4e1gx_i2c_txdata(S5K4E1GX_AF_I2C_ADDR >> 1, buf, 2);
    if (rc < 0)
    {
        CDBG("i2c_write failed, saddr = 0x%x addr = 0x%x, val =0x%x!\n", S5K4E1GX_AF_I2C_ADDR >> 1, code_val_msb,
             code_val_msb);
    }

    return rc;
}

static int32_t s5k4e1gx_move_focus(int direction, int32_t num_steps)
{
    int16_t step_direction, dest_lens_position, dest_step_position;
    int16_t target_dist, small_step, next_lens_position;
    int32_t rc = 0;

    if (direction == MOVE_NEAR)
    {
        step_direction = 1;
    }
    else if (direction == MOVE_FAR)
    {
        step_direction = -1;
        CDBG("%s:%d\n", __func__, __LINE__);
    }
    else
    {
        CDBG("%s:%d\n", __func__, __LINE__);
        return -EINVAL;
    }

    dest_step_position = s5k4e1gx_ctrl->curr_step_pos + (step_direction * num_steps);
    if (dest_step_position < 0)
    {
        CDBG("%s:%d\n", __func__, __LINE__);
        dest_step_position = 0;
    }
    else if (dest_step_position > S5K4E1GX_TOTAL_STEPS_NEAR_TO_FAR)
    {
        CDBG("%s:%d\n", __func__, __LINE__);
        dest_step_position = S5K4E1GX_TOTAL_STEPS_NEAR_TO_FAR;
    }

    if (dest_step_position == s5k4e1gx_ctrl->curr_step_pos)
    {
        CDBG("%s:%d\n", __func__, __LINE__);
        return rc;
    }

    dest_lens_position = s5k4e1gx_pos_tbl[dest_step_position];
    target_dist = step_direction * (dest_lens_position - s5k4e1gx_ctrl->curr_lens_pos);

    CDBG("dest_step_position =%d,dest_lens_position=%d \n", dest_step_position, dest_lens_position);

    /* SW damping */

    if ((step_direction < 0) && (target_dist >= s5k4e1gx_pos_tbl[s5k4e1gx_damping_threshold])) //change to variable
    {
        small_step = (uint16_t)(target_dist / 10);
        s5k4e1gx_sw_damping_time_wait = 1;
    }
    else
    {
        small_step = (uint16_t)(target_dist / 4);
        s5k4e1gx_sw_damping_time_wait = 4;
    }

    for (next_lens_position = s5k4e1gx_ctrl->curr_lens_pos + (step_direction * small_step);
         (step_direction * next_lens_position) <= (step_direction * dest_lens_position);
         next_lens_position += (step_direction * small_step))
    {
        if (s5k4e1gx_af_i2c_write(next_lens_position) < 0)
        {
            return rc;
        }

        s5k4e1gx_ctrl->curr_lens_pos = next_lens_position;
        mdelay(s5k4e1gx_sw_damping_time_wait);
    }

    if (s5k4e1gx_ctrl->curr_lens_pos != dest_lens_position)
    {
        if (s5k4e1gx_af_i2c_write(dest_lens_position) < 0)
        {
            return rc;
        }

        mdelay(s5k4e1gx_sw_damping_time_wait);
    }

    s5k4e1gx_ctrl->curr_lens_pos = dest_lens_position;
    s5k4e1gx_ctrl->curr_step_pos = dest_step_position;
    return rc;
}

static int32_t s5k4e1gx_set_default_focus(void)
{
    int32_t rc = 0;

    if (s5k4e1gx_ctrl->curr_step_pos != 0)
    {
        rc = s5k4e1gx_move_focus(MOVE_FAR, s5k4e1gx_ctrl->curr_step_pos);
        if (rc < 0)
        {
            CDBG("s5k4e1gx_set_default_focus Failed!!!\n");
            return rc;
        }
    }
    else
    {
        CDBG("%s:%d\n", __func__, __LINE__);
        rc = s5k4e1gx_af_i2c_write(120);
        if (rc < 0)
        {
            CDBG("s5k4e1gx_go_to_position Failed!!!\n");
            return rc;
        }
    }

    s5k4e1gx_ctrl->curr_lens_pos = 120;
    s5k4e1gx_ctrl->curr_step_pos = 2;

    return rc;
}

static int s5k4e1gx_sensor_config(void __user *argp)
{
    struct sensor_cfg_data cdata;
    long rc = 0;

    if (copy_from_user(&cdata,
                       (void *)argp,
                       sizeof(struct sensor_cfg_data)))
    {
        return -EFAULT;
    }

    mutex_lock(&s5k4e1gx_mutex);

    CDBG("%s: cfgtype = %d\n", __func__, cdata.cfgtype);
    switch (cdata.cfgtype)
    {
    case CFG_GET_PICT_FPS:
        s5k4e1gx_get_pict_fps(cdata.cfg.gfps.prevfps,
                              &(cdata.cfg.gfps.pictfps));

        if (copy_to_user((void *)argp, &cdata,
                         sizeof(struct sensor_cfg_data)))
        {
            rc = -EFAULT;
        }

        break;

    case CFG_GET_PREV_L_PF:
        cdata.cfg.prevl_pf = s5k4e1gx_get_prev_lines_pf();

        if (copy_to_user((void *)argp,
                         &cdata,
                         sizeof(struct sensor_cfg_data)))
        {
            rc = -EFAULT;
        }

        break;

    case CFG_GET_PREV_P_PL:
        cdata.cfg.prevp_pl = s5k4e1gx_get_prev_pixels_pl();

        if (copy_to_user((void *)argp,
                         &cdata,
                         sizeof(struct sensor_cfg_data)))
        {
            rc = -EFAULT;
        }

        break;

    case CFG_GET_PICT_L_PF:
        cdata.cfg.pictl_pf = s5k4e1gx_get_pict_lines_pf();

        if (copy_to_user((void *)argp,
                         &cdata,
                         sizeof(struct sensor_cfg_data)))
        {
            rc = -EFAULT;
        }

        break;

    case CFG_GET_PICT_P_PL:
        cdata.cfg.pictp_pl = s5k4e1gx_get_pict_pixels_pl();

        if (copy_to_user((void *)argp,
                         &cdata,
                         sizeof(struct sensor_cfg_data)))
        {
            rc = -EFAULT;
        }

        break;

    case CFG_GET_PICT_MAX_EXP_LC:
        cdata.cfg.pict_max_exp_lc =
            s5k4e1gx_get_pict_max_exp_lc();

        if (copy_to_user((void *)argp,
                         &cdata,
                         sizeof(struct sensor_cfg_data)))
        {
            rc = -EFAULT;
        }

        break;

    case CFG_SET_FPS:
    case CFG_SET_PICT_FPS:
        rc = s5k4e1gx_set_fps(&(cdata.cfg.fps));
        break;

    case CFG_SET_EXP_GAIN:
        rc =
            s5k4e1gx_write_exp_gain(cdata.cfg.exp_gain.gain,
                                    cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_PICT_EXP_GAIN:
        CDBG("Line:%d CFG_SET_PICT_EXP_GAIN \n", __LINE__);
        rc =
            s5k4e1gx_set_pict_exp_gain(
            cdata.cfg.exp_gain.gain,
            cdata.cfg.exp_gain.line);
        break;

    case CFG_SET_MODE:
        rc =
            s5k4e1gx_set_sensor_mode(
            cdata.mode, cdata.rs);
        break;

    case CFG_PWR_DOWN:
        rc = s5k4e1gx_power_down();
        break;

    case CFG_MOVE_FOCUS:
        rc =
            s5k4e1gx_move_focus(
            cdata.cfg.focus.dir,
            cdata.cfg.focus.steps);
        break;

    case CFG_SET_DEFAULT_FOCUS:
        rc =
            s5k4e1gx_set_default_focus();
        break;

    case CFG_GET_AF_MAX_STEPS:
    case CFG_SET_EFFECT:
    case CFG_SET_LENS_SHADING:
    default:
        rc = -EINVAL;
        break;
    }

    mutex_unlock(&s5k4e1gx_mutex);
    return rc;
}

static int s5k4e1gx_sensor_probe(const struct msm_camera_sensor_info *info,
                                 struct msm_sensor_ctrl *             s)
{
    int rc = 0;

    rc = i2c_add_driver(&s5k4e1gx_i2c_driver);
    if ((rc < 0) || (s5k4e1gx_client == NULL))
    {
        rc = -ENOTSUPP;
        goto probe_fail;
    }

    CDBG("Line:%d  s5k4e1gx_sensor_probe rc=%d \n", __LINE__, rc);

    rc = s5k4e1gx_probe_init_sensor(info);
    if (rc < 0)
    {
        goto probe_fail;
    }

    CDBG("Line:%d  s5k4e1gx_probe_init_sensor rc=%d \n", __LINE__, rc);
    s->s_init = s5k4e1gx_sensor_open_init;
    s->s_release = s5k4e1gx_sensor_release;
    s->s_config = s5k4e1gx_sensor_config;
    s->s_camera_type = BACK_CAMERA_2D;
	/* if machine is U8680, rotate the camera with 180 degree */
	if (machine_is_msm8255_u8680())
	{
        s->s_mount_angle = 180;
	}
	else
	{
        s->s_mount_angle = 0;
	}
    s5k4e1gx_probe_init_done(info);
    CDBG("Line:%d  s5k4e1gx_probe_init_done \n", __LINE__);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
    /* detect current device successful, set the flag as present */
    set_hw_dev_flag(DEV_I2C_CAMERA_MAIN);
#endif

    return rc;

probe_fail:
    CDBG("SENSOR PROBE FAILS!\n");
    return rc;
}

static int __s5k4e1gx_probe(struct platform_device *pdev)
{
    return msm_camera_drv_start(pdev, s5k4e1gx_sensor_probe);
}

static struct platform_driver msm_camera_driver =
{
    .probe     = __s5k4e1gx_probe,
    .driver    = {
        .name  = "msm_camera_s5k4e1gx",
        .owner = THIS_MODULE,
    },
};

static int __init s5k4e1gx_init(void)
{
    return platform_driver_register(&msm_camera_driver);
}

module_init(s5k4e1gx_init);
