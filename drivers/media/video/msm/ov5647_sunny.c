/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
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
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/camera.h>
#include "ov5647_sunny.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <linux/hw_dev_dec.h>
#endif

/*=============================================================
    SENSOR REGISTER DEFINES
==============================================================*/
#define OV5647_SUNNY_REG_MODEL_ID 0x300A
#define OV5647_SUNNY_MODEL_ID 0x5647
#define OV5647_SUNNY_FULL_SIZE_WIDTH 2608
#define OV5647_SUNNY_FULL_SIZE_HEIGHT 1960
#define OV5647_SUNNY_SNAPSHOT_DUMMY_PIXELS 0
#define OV5647_SUNNY_SNAPSHOT_DUMMY_LINES 0

#define OV5647_SUNNY_QTR_SIZE_WIDTH 1296
#define OV5647_SUNNY_QTR_SIZE_HEIGHT 972
#define OV5647_SUNNY_PREVIEW_DUMMY_PIXELS 0
#define OV5647_SUNNY_PREVIEW_DUMMY_LINES 0

#define OV5647_SUNNY_OFFSET 8
#define OV5647_SUNNY_MAX_SNAPSHOT_EXPOSURE_LINE_COUNT 4001

#define OV5647_SUNNY_HRZ_FULL_BLK_PIXELS 92
#define OV5647_SUNNY_VER_FULL_BLK_LINES 16
#define OV5647_SUNNY_HRZ_QTR_BLK_PIXELS 592
#define OV5647_SUNNY_VER_QTR_BLK_LINES 12

#define CAMERA_FAILED -1
#define Q10 0x00000400

enum ov5647_sunny_test_mode
{
    TEST_OFF,
    TEST_1,
    TEST_2,
    TEST_3
};

enum ov5647_sunny_resolution
{
    QTR_SIZE,
    FULL_SIZE,
    INVALID_SIZE
};

enum ov5647_sunny_reg_update
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

enum ov5647_sunny_setting
{
    RES_PREVIEW,
    RES_CAPTURE
};


struct ov5647_sunny_work
{
    struct work_struct work;
};
static struct ov5647_sunny_work *ov5647_sunny_sensorw;
static struct i2c_client *ov5647_sunny_client;

struct ov5647_sunny_ctrl
{
    const struct msm_camera_sensor_info *sensordata;

    int      sensormode;
    uint32_t fps_divider;   /* init to 1 * 0x00000400 */
    uint32_t pict_fps_divider;  /* init to 1 * 0x00000400 */

    uint16_t curr_lens_pos;
    uint16_t curr_step_pos;
    uint16_t init_curr_lens_pos;
    uint16_t my_reg_gain;
    uint32_t my_reg_line_count;

    enum ov5647_sunny_resolution prev_res;
    enum ov5647_sunny_resolution pict_res;
    enum ov5647_sunny_resolution curr_res;
    enum ov5647_sunny_test_mode  set_test;
};

static struct ov5647_sunny_ctrl *ov5647_sunny_ctrl;
static DECLARE_WAIT_QUEUE_HEAD(ov5647_sunny_wait_queue);
DEFINE_MUTEX(ov5647_sunny_mut);

/*========================================*/
// Preview:

// Input Clock - 12MHz

// Output Clock - 56MHz

// Active Pixels - 1296 x 972

// Bayer Pattern - BGGR

// Frame Size -

// Frame Rate -

// 30fps preview
/*========================================*/
struct register_address_value_pair const
ov5647_sunny_preview[] =
{
    {0x0100, 0x00}, {0x0103, 0x01}, {0x0100, 0x00}, {0x0100, 0x00}, 
		
    {0x0100, 0x00}, {0x0100, 0x00}, {0x3035, 0x11}, {0x3036, 0x46},

	{0x303c, 0x11}, {0x3821, 0x07}, {0x3820, 0x41}, {0x370c, 0x03},

	{0x3612, 0x09}, {0x3618, 0x00}, {0x5000, 0x06}, {0x5003, 0x08},

	{0x5a00, 0x08}, {0x3000, 0xff}, {0x3001, 0xff}, {0x3002, 0xff},

	{0x3a18, 0x01}, {0x3a19, 0xe0}, {0x3c01, 0x80}, {0x3b07, 0x0c}, 

	{0x380c, 0x07}, {0x380d, 0x60}, {0x380e, 0x03}, {0x380f, 0xd8},

	{0x3814, 0x31}, {0x3815, 0x31}, {0x3708, 0x22}, {0x3709, 0x52},

	{0x3815, 0x31}, {0x3808, 0x05}, {0x3809, 0x10}, {0x380a, 0x03},

	{0x380b, 0xcc}, {0x3800, 0x00}, {0x3801, 0x08}, {0x3802, 0x00}, 

	{0x3803, 0x02}, {0x3804, 0x0a}, {0x3805, 0x37}, {0x3806, 0x07},
	{0x3807, 0xa1}, {0x3630, 0x2e}, {0x3632, 0xe2}, {0x3633, 0x23},
	{0x3634, 0x44}, {0x3620, 0x64}, {0x3621, 0xe0}, {0x3600, 0x37},

	{0x3704, 0xa0}, {0x3703, 0x5a}, {0x3715, 0x78}, {0x3717, 0x01},

	{0x3731, 0x02}, {0x370b, 0x60}, {0x3705, 0x1a}, {0x3f05, 0x02},

	{0x3f06, 0x10}, {0x3f01, 0x0a}, {0x3a08, 0x01}, {0x3a09, 0x27}, 

	{0x3a0a, 0x00}, {0x3a0b, 0xf6}, {0x3a0d, 0x04}, {0x3a0e, 0x03},

	{0x3821, 0x01}, {0x3820, 0x47}, {0x3503, 0x03}, {0x3501, 0x3d},

	{0x3502, 0x40}, {0x350a, 0x01}, {0x350b, 0xe0}, {0x5001, 0x01},

	{0x5180, 0x08}, {0x5186, 0x04}, {0x5187, 0x00}, {0x5188, 0x04},

	{0x5189, 0x00}, {0x518a, 0x04}, {0x518b, 0x00}, {0x5000, 0x06},

/* kernel29 -> kernel32 driver modify*/
	{0x3011, 0xe2}, {0x4001, 0x02}, {0x4004, 0x02}, {0x4000, 0x09}, 
	{0x0100, 0x01}, {0x5000, 0x86},
	{0x5800, 0x15}, {0x5801, 0x8 }, {0x5802, 0x8 },	{0x5803, 0x8 },
	
	{0x5804, 0x8 },	{0x5805, 0xc },	{0x5806, 0x5 },	{0x5807, 0x4 },
	
	{0x5808, 0x3 },	{0x5809, 0x3 },	{0x580a, 0x4 },	{0x580b, 0x5 },
	
	{0x580c, 0x5 },	{0x580d, 0x1 },	{0x580e, 0x0 },	{0x580f, 0x0 },
	
	{0x5810, 0x1 },	{0x5811, 0x3 },	{0x5812, 0x4 },	{0x5813, 0x1 },
	
	{0x5814, 0x0 },	{0x5815, 0x0 },	{0x5816, 0x1 },	{0x5817, 0x3 },
	
	{0x5818, 0x6 },	{0x5819, 0x4 },	{0x581a, 0x2 },	{0x581b, 0x2 },
	
	{0x581c, 0x3 },	{0x581d, 0x4 },	{0x581e, 0xb },	{0x581f, 0x7 },
	
	{0x5820, 0x6 },	{0x5821, 0x6 },	{0x5822, 0x7 },	{0x5823, 0x9 },
	
	{0x5824, 0x8a},	{0x5825, 0x64},	{0x5826, 0x66},	{0x5827, 0x44},
	
	{0x5828, 0xa6},	{0x5829, 0x26},	{0x582a, 0x46},	{0x582b, 0x43},
	
	{0x582c, 0x44},	{0x582d, 0x8 },	{0x582e, 0x46},	{0x582f, 0x64},
	
	{0x5830, 0x61},	{0x5831, 0x62},	{0x5832, 0x6 },	{0x5833, 0x46},
	
	{0x5834, 0x66},	{0x5835, 0x44},	{0x5836, 0x65},	{0x5837, 0x8 },
	
	{0x5838, 0x66},	{0x5839, 0x28},	{0x583a, 0x2a},	{0x583b, 0x28},
	
	{0x583c, 0x28},	{0x583d, 0xae},

};

/*========================================*/
//snapshot
//PAD I/O driver {0x3011, 0x62},

//{0x3036, 0x46}

// Snapshot:

// Input Clock - 24MHz

// Output Clock -

// Active Pixels - 2608 x 1952

// Bayer Pattern - BGGR

// Frame Size -

// Frame Rate -

//15fps
/*========================================*/
struct register_address_value_pair const
ov5647_sunny_snapshot[] =
{
	{0x0100, 0x00}, {0x3036, 0x64},	{0x3821, 0x00}, {0x3820, 0x06},
		
	{0x370c, 0x00}, {0x3612, 0x0b},	{0x3618, 0x04},	{0x380c, 0x0a},

	{0x380d, 0x8c},	{0x380e, 0x07},	{0x380f, 0xb0},	{0x3814, 0x11},

	{0x3815, 0x11},	{0x3708, 0x24},	{0x3709, 0x12},	{0x3815, 0x11},

	{0x3808, 0x0a},	{0x3809, 0x30},	{0x380a, 0x07},	{0x380b, 0xa0},

	{0x3801, 0x04},	{0x3803, 0x00},	{0x3805, 0x3b},	{0x3807, 0xa3},

	{0x3a0d, 0x08},	{0x3a0e, 0x06},	{0x4004, 0x04},	{0x3a08, 0x00},

	{0x3a09, 0x4a},	{0x3a0a, 0x00},	{0x3a0b, 0x3d},	{0x3a0d, 0x20},

	{0x3a0e, 0x1a}, {0x3036, 0x46}, {0x0100, 0x01}, {0x4202, 0x01},
};

struct ov5647_sunny_reg ov5647_sunny_regs =
{
    .prev_reg_settings		= &ov5647_sunny_preview[0],
    .prev_reg_settings_size = ARRAY_SIZE(
        ov5647_sunny_preview),
    .snap_reg_settings		= &ov5647_sunny_snapshot[0],
    .snap_reg_settings_size = ARRAY_SIZE(
        ov5647_sunny_snapshot),
};


/*AF  parameters*/
#define OV5647_SUNNY_AF_I2C_ADDR				0x18
#define OV5647_SUNNY_STEPS_NEAR_TO_CLOSEST_INF	40
#define OV5647_SUNNY_TOTAL_STEPS_NEAR_TO_FAR	40
#define OV5647_SUNNY_SW_DAMPING_STEP			10
#define  OV5647_SUNNY_MAX_FPS               30


static uint16_t ov5647_sunny_pos_tbl [OV5647_SUNNY_TOTAL_STEPS_NEAR_TO_FAR];
static uint8_t  ov5647_sunny_mode_mask = 0; 
uint16_t ov5647_sunny_damping_threshold = 3;
uint16_t ov5647_sunny_sw_damping_step = 10;
uint16_t ov5647_sunny_sw_damping_time_wait = 1;
uint16_t ov5647_sunny_debug_total_step = 40;
uint16_t ov5647_sunny_debug_focus_method = 0;

/*=============================================================*/

static int ov5647_sunny_i2c_rxdata(unsigned short saddr, unsigned char *rxdata,
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

    if (i2c_transfer(ov5647_sunny_client->adapter, msgs, 2) < 0)
    {
        CDBG("ov5647_sunny_i2c_rxdata failed!\n");
        return -EIO;
    }

    return 0;
}

static int32_t ov5647_sunny_i2c_read_w(unsigned short saddr, unsigned short raddr,
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

    rc = ov5647_sunny_i2c_rxdata(saddr, buf, 2);
    if (rc < 0)
    {
        return rc;
    }

    *rdata = buf[0] << 8 | buf[1];

    if (rc < 0)
    {
        CDBG("ov5647_sunny_i2c_read failed!\n");
    }

    return rc;
}

static int32_t ov5647_sunny_i2c_txdata(unsigned short saddr, unsigned char *txdata,
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

    if (i2c_transfer(ov5647_sunny_client->adapter, msg, 1) < 0)
    {
        CDBG("ov5647_sunny_i2c_txdata failed\n");
        return -EIO;
    }

    return 0;
}

static int32_t ov5647_sunny_i2c_write(unsigned short saddr, unsigned short waddr,
                                      unsigned short wdata)
{
    int32_t rc = -EIO;
    unsigned char buf[3];

    memset(buf, 0, sizeof(buf));
    buf[0] = (waddr & 0xFF00) >> 8;
    buf[1] = (waddr & 0x00FF);
    buf[2] = wdata;

    rc = ov5647_sunny_i2c_txdata(saddr, buf, 3);

    if (rc < 0)
    {
        CDBG("i2c_write_w failed, addr = 0x%x, val = 0x%x!\n",
             waddr, wdata);
    }

    return rc;
}

static int32_t ov5647_sunny_i2c_write_table(struct register_address_value_pair const
                                            *reg_conf_tbl, int num)
{
    int i;
    int32_t rc = 0;

    for (i = 0; i < num; i++)
    {
        rc = ov5647_sunny_i2c_write(ov5647_sunny_client->addr,
                                    reg_conf_tbl->register_address,
                                    reg_conf_tbl->register_value);
        if (rc < 0)
        {
            break;
        }

        reg_conf_tbl++;
    }

    return rc;
}

static int32_t ov5647_sunny_lens_shading_enable(uint8_t is_enable)
{
    int32_t rc = 0;

    return rc;
}

static void ov5647_sunny_setup_af_tbl(void)
{
	int i;
	uint16_t ov5647_sunny_nl_region_boundary1 = 6;
	uint16_t ov5647_sunny_nl_region_boundary2 = 20;
	uint16_t ov5647_sunny_nl_region_boundary3 = 30;
	uint16_t ov5647_sunny_nl_region_boundary4 = 35;

	uint16_t ov5647_sunny_nl_region_code_per_step1 = 10;
	uint16_t ov5647_sunny_nl_region_code_per_step2= 5;
	uint16_t ov5647_sunny_nl_region_code_per_step3= 10;
	uint16_t ov5647_sunny_nl_region_code_per_step4= 20;
	uint16_t ov5647_sunny_nl_region_code_per_step5 = 30;


	ov5647_sunny_pos_tbl[0] = 0;
	ov5647_sunny_pos_tbl[1] =170;
  for(i=2; i < OV5647_SUNNY_TOTAL_STEPS_NEAR_TO_FAR; i++) 
  {
     if ( i < ov5647_sunny_nl_region_boundary1)
     {
       ov5647_sunny_pos_tbl[i] = ov5647_sunny_pos_tbl[i-1] + ov5647_sunny_nl_region_code_per_step1;
     }
     else if ( i <= ov5647_sunny_nl_region_boundary2)
     {
       ov5647_sunny_pos_tbl[i] = ov5647_sunny_pos_tbl[i-1] + ov5647_sunny_nl_region_code_per_step2;
     }
     else if ( i <= ov5647_sunny_nl_region_boundary3)
     {
       ov5647_sunny_pos_tbl[i] = ov5647_sunny_pos_tbl[i-1] + ov5647_sunny_nl_region_code_per_step3;
     }
     else if ( i <= ov5647_sunny_nl_region_boundary4)
     {
       ov5647_sunny_pos_tbl[i] = ov5647_sunny_pos_tbl[i-1] + ov5647_sunny_nl_region_code_per_step4;
     }
     else
     {
       ov5647_sunny_pos_tbl[i] = ov5647_sunny_pos_tbl[i-1] + ov5647_sunny_nl_region_code_per_step5;
     }

  }


  
}


static void ov5647_sunny_get_pict_fps(uint16_t fps, uint16_t *pfps)
{
    uint16_t preview_frame_length_lines, snapshot_frame_length_lines;
    uint16_t preview_line_length_pck, snapshot_line_length_pck;
    uint16_t snapshot_fps;
    uint16_t divider;

    CDBG("ov5647_sunny_get_pict_fps - preview_fps  = %d", fps );

    /* Total frame_length_lines and line_length_pck for preview */
    preview_frame_length_lines = OV5647_SUNNY_QTR_SIZE_HEIGHT + OV5647_SUNNY_VER_QTR_BLK_LINES;
    preview_line_length_pck = OV5647_SUNNY_QTR_SIZE_WIDTH + OV5647_SUNNY_HRZ_QTR_BLK_PIXELS;

    /* Total frame_length_lines and line_length_pck for snapshot */
    snapshot_frame_length_lines = OV5647_SUNNY_FULL_SIZE_HEIGHT + OV5647_SUNNY_VER_FULL_BLK_LINES;
    snapshot_line_length_pck = OV5647_SUNNY_FULL_SIZE_WIDTH + OV5647_SUNNY_HRZ_FULL_BLK_PIXELS;

    divider = (preview_frame_length_lines * preview_line_length_pck * Q10)
              / (snapshot_frame_length_lines * snapshot_line_length_pck);

    //Verify PCLK settings and frame sizes.
    snapshot_fps = (uint16_t) (fps * divider / Q10);

    *pfps = snapshot_fps;
}

static uint16_t ov5647_sunny_get_prev_lines_pf(void)
{
    uint16_t preview_frame_length_lines;

    preview_frame_length_lines = OV5647_SUNNY_QTR_SIZE_HEIGHT + OV5647_SUNNY_VER_QTR_BLK_LINES;

    return preview_frame_length_lines;
}

static uint16_t ov5647_sunny_get_prev_pixels_pl(void)
{
    uint16_t preview_line_length_pck;

    preview_line_length_pck = OV5647_SUNNY_QTR_SIZE_WIDTH + OV5647_SUNNY_HRZ_QTR_BLK_PIXELS;

    return preview_line_length_pck;
}

static uint16_t ov5647_sunny_get_pict_lines_pf(void)
{
    return (OV5647_SUNNY_FULL_SIZE_HEIGHT + OV5647_SUNNY_VER_FULL_BLK_LINES);
}

static uint16_t ov5647_sunny_get_pict_pixels_pl(void)
{
    return (OV5647_SUNNY_FULL_SIZE_WIDTH + OV5647_SUNNY_HRZ_FULL_BLK_PIXELS);
}

static uint32_t ov5647_sunny_get_pict_max_exp_lc(void)
{
/*increase max exposure line counts for decrease frame rate to decrease noise*/
    return (OV5647_SUNNY_FULL_SIZE_HEIGHT + OV5647_SUNNY_VER_FULL_BLK_LINES) * 2;
}

static int32_t ov5647_sunny_set_fps(struct fps_cfg *fps)
{
    /* input is new fps in Q10 format */
    int32_t rc = 0;

    return rc;
}

static int32_t ov5647_sunny_write_exp_gain(uint16_t gain, uint32_t line)
{
	int32_t max_lines_per_frame,min_lines_per_frame ;
	int32_t frame_boundary ;
	uint8_t gain_msb , gain_lsb;
	uint8_t line0,line1,line2;
	uint8_t boundary_msb , boundary_lsb;

	if( RES_PREVIEW== ov5647_sunny_ctrl->curr_res)
	{
		min_lines_per_frame = OV5647_SUNNY_QTR_SIZE_HEIGHT + OV5647_SUNNY_VER_QTR_BLK_LINES ;
		max_lines_per_frame = min_lines_per_frame * 4;
	}
	else
	{	
		min_lines_per_frame = OV5647_SUNNY_FULL_SIZE_HEIGHT + OV5647_SUNNY_OFFSET ;
		max_lines_per_frame = min_lines_per_frame * 4;
	}

	if ( (max_lines_per_frame -OV5647_SUNNY_OFFSET ) < line)
	{
		frame_boundary = max_lines_per_frame;
		line = frame_boundary -OV5647_SUNNY_OFFSET;
	}
	else if ( line < min_lines_per_frame ) 
	{
		frame_boundary = min_lines_per_frame;
	}
	else 
	{
		frame_boundary = line + OV5647_SUNNY_OFFSET ;
	}

	if (RES_PREVIEW== ov5647_sunny_ctrl->curr_res) {
		ov5647_sunny_ctrl->my_reg_gain = gain;
		ov5647_sunny_ctrl->my_reg_line_count =  line;
	}	
	CDBG("ov5647_sunny_write_exp_gain start gain=%d  line = %d fram_boundary =%d \n", gain,line,frame_boundary);
	gain_msb = (uint8_t) ((gain & 0xFF00) >>8);
	gain_lsb = (uint8_t) (gain & 0xFF);
	if (!!ov5647_sunny_i2c_write(ov5647_sunny_client->addr,0x350a, gain_msb))
		return CAMERA_FAILED;	
	if (!!ov5647_sunny_i2c_write(ov5647_sunny_client->addr,0x350b, gain_lsb))
		return CAMERA_FAILED;
	
	/* update line count registers */
	boundary_msb = (uint8_t)  ((frame_boundary & 0xFF00)>>8);
	boundary_lsb = (uint8_t) (frame_boundary & 0xFF);
	if (!!ov5647_sunny_i2c_write(ov5647_sunny_client->addr,0x380e, boundary_msb))
		return CAMERA_FAILED;	
	if (!!ov5647_sunny_i2c_write(ov5647_sunny_client->addr,0x380f, boundary_lsb))
		return CAMERA_FAILED;

	line=line<<4;  
	line0 = (uint8_t) ((line & 0xFF0000) >> 16);
	line1 = (uint8_t) ((line & 0xFF00) >> 8);
	line2 = (uint8_t) ((line & 0xFF));
	if(!!ov5647_sunny_i2c_write(ov5647_sunny_client->addr,0x3500, line0))	
		return CAMERA_FAILED; 
	if(!!ov5647_sunny_i2c_write(ov5647_sunny_client->addr,0x3501, line1))	 
		return CAMERA_FAILED; 
	if(!!ov5647_sunny_i2c_write(ov5647_sunny_client->addr,0x3502, line2))	 
		return CAMERA_FAILED;

    return 0;
}

static int32_t ov5647_sunny_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
    int32_t rc = 0;

    CDBG("Line:%d ov5647_sunny_set_pict_exp_gain \n", __LINE__);

    rc = ov5647_sunny_write_exp_gain(gain, line);
    if (rc < 0)
    {
        CDBG("Line:%d ov5647_sunny_set_pict_exp_gain failed... \n",
             __LINE__);
        return rc;
    }

	if(!!ov5647_sunny_i2c_write(ov5647_sunny_client->addr,0x0100, 0x01))	 
		return CAMERA_FAILED;
    mdelay(50);

    return rc;
}

static int32_t ov5647_sunny_setting(enum ov5647_sunny_reg_update rupdate,
                                    enum ov5647_sunny_setting    rt)
{
    int32_t rc = 0;

    CDBG("ov5647_sunny_setting rupdate = %d , rt=%d\n", rupdate, rt);
    switch (rupdate)
    {
        case UPDATE_PERIODIC:
            if (rt == RES_PREVIEW)
            {
                rc = ov5647_sunny_i2c_write_table(ov5647_sunny_regs.prev_reg_settings,
                                                  ov5647_sunny_regs.prev_reg_settings_size);
                mdelay(10);
            }
            else
            {
                rc = ov5647_sunny_i2c_write_table(ov5647_sunny_regs.snap_reg_settings,
                                                  ov5647_sunny_regs.snap_reg_settings_size);
                mdelay(10);
            }

            break;

        case REG_INIT:
            if (rt == RES_PREVIEW)
            {
                rc = ov5647_sunny_i2c_write_table(ov5647_sunny_regs.prev_reg_settings,
                                                  ov5647_sunny_regs.prev_reg_settings_size);
                mdelay(10);
            }
            else
            {
		  		rc = ov5647_sunny_i2c_write_table(ov5647_sunny_regs.prev_reg_settings,
                                                  ov5647_sunny_regs.prev_reg_settings_size);
                rc = ov5647_sunny_i2c_write_table(ov5647_sunny_regs.snap_reg_settings,
                                                  ov5647_sunny_regs.snap_reg_settings_size);
                mdelay(10);
            }

            break;

        default:
            rc = -EINVAL;
            break;
    }

    return rc;
}

static int32_t ov5647_sunny_video_config(int mode, int res)
{
    int32_t rc;

    switch (res)
    {
        case QTR_SIZE:
            rc = ov5647_sunny_setting(UPDATE_PERIODIC, RES_PREVIEW);
            if (rc < 0)
            {
                return rc;
            }

            CDBG("ov5647_sunny sensor configuration done!\n");
            break;

        case FULL_SIZE:
            rc = ov5647_sunny_setting(UPDATE_PERIODIC, RES_CAPTURE);
            if (rc < 0)
            {
                return rc;
            }

            break;

        default:
            return 0;
    }           /* switch */

    ov5647_sunny_ctrl->prev_res   = res;
    ov5647_sunny_ctrl->curr_res   = res;
    ov5647_sunny_ctrl->sensormode = mode;

    rc = ov5647_sunny_write_exp_gain(ov5647_sunny_ctrl->my_reg_gain,
                                     ov5647_sunny_ctrl->my_reg_line_count);
    CDBG("ov5647_sunny_write_exp_gain rc=%d!\n", rc);

    return rc;
}

static int32_t ov5647_sunny_snapshot_config(int mode)
{
    int32_t rc = 0;

    rc = ov5647_sunny_setting(UPDATE_PERIODIC, RES_CAPTURE);
    if (rc < 0)
    {
        return rc;
    }

    ov5647_sunny_ctrl->curr_res = ov5647_sunny_ctrl->pict_res;

    ov5647_sunny_ctrl->sensormode = mode;

    return rc;
}

static int32_t ov5647_sunny_raw_snapshot_config(int mode)
{
    int32_t rc = 0;

    rc = ov5647_sunny_setting(UPDATE_PERIODIC, RES_CAPTURE);
    if (rc < 0)
    {
        return rc;
    }

    ov5647_sunny_ctrl->curr_res = ov5647_sunny_ctrl->pict_res;

    ov5647_sunny_ctrl->sensormode = mode;

    return rc;
}

static int32_t ov5647_sunny_power_down(void)
{
    int32_t rc = 0;
    if(ov5647_sunny_ctrl->sensordata->vreg_disable_func)
    {
        ov5647_sunny_ctrl->sensordata->vreg_disable_func(0);
    }
    return rc;
}

static int32_t ov5647_sunny_af_i2c_write(uint16_t data)
{

	uint8_t code_val_msb, code_val_lsb;
	int32_t rc = 0;
	unsigned char buf[2];
	code_val_msb = data >> 4;
	code_val_lsb = (data & 0x000F) << 4;
	code_val_lsb |= ov5647_sunny_mode_mask;
	buf[0] = code_val_msb;
	buf[1] = code_val_lsb;
	rc = ov5647_sunny_i2c_txdata(OV5647_SUNNY_AF_I2C_ADDR >> 1, buf, 2);
	if (rc < 0) {
		CDBG("i2c_write failed, saddr = 0x%x addr = 0x%x, val =0x%x!\n",OV5647_SUNNY_AF_I2C_ADDR >> 1,code_val_msb, code_val_msb);
	}
	return rc;
}

static int32_t ov5647_sunny_move_focus(int direction, int32_t num_steps)
{
		int16_t step_direction, dest_lens_position, dest_step_position;
		int16_t target_dist, small_step, next_lens_position;
		int32_t rc = 0;

		CDBG("%s:%d  ,direction = %d, num_steps= %d \n",__func__,__LINE__,direction,num_steps);
		if (direction == MOVE_NEAR) {
			step_direction = 1;  
		}
		else if (direction == MOVE_FAR) {
			step_direction = -1;  
		CDBG("%s:%d\n",__func__,__LINE__);
		}
		else
		{
		CDBG("%s:%d\n",__func__,__LINE__);
		return -EINVAL;
		}
		dest_step_position = ov5647_sunny_ctrl->curr_step_pos + (step_direction * num_steps);
		if (dest_step_position < 0)
		{
		CDBG("%s:%d\n",__func__,__LINE__);
		dest_step_position = 0;
		}
		else if (dest_step_position > OV5647_SUNNY_STEPS_NEAR_TO_CLOSEST_INF)
		{
			CDBG("%s:%d\n",__func__,__LINE__);
			dest_step_position = OV5647_SUNNY_STEPS_NEAR_TO_CLOSEST_INF;
		}
		if(dest_step_position == ov5647_sunny_ctrl->curr_step_pos)
		{
			CDBG("%s:%d\n",__func__,__LINE__);
			return rc;
		}
		dest_lens_position = ov5647_sunny_pos_tbl[dest_step_position];
		target_dist = step_direction * (dest_lens_position - ov5647_sunny_ctrl->curr_lens_pos);


		ov5647_sunny_mode_mask = 0;
		/* SW damping */ 
		if(step_direction < 0 && (target_dist >= 150)) {
			small_step = (uint16_t)(target_dist/10);
		if (small_step ==0)
			small_step = 1;
		ov5647_sunny_sw_damping_time_wait = 1;
		}
		else {
		small_step = (uint16_t)(target_dist/4);
		if (small_step ==0)
			small_step = 1;
			ov5647_sunny_sw_damping_time_wait = 4;
		}
		for (next_lens_position = ov5647_sunny_ctrl->curr_lens_pos + (step_direction * small_step);
			(step_direction * next_lens_position) <= (step_direction * dest_lens_position);
		next_lens_position += (step_direction * small_step)) {
		if(ov5647_sunny_af_i2c_write(next_lens_position) < 0)
		return rc; 
		ov5647_sunny_ctrl->curr_lens_pos = next_lens_position;
		mdelay(ov5647_sunny_sw_damping_time_wait);
		}
		if(ov5647_sunny_ctrl->curr_lens_pos != dest_lens_position) {
			if(ov5647_sunny_af_i2c_write(dest_lens_position) < 0)
				return rc;
		mdelay(ov5647_sunny_sw_damping_time_wait);
		}
	
		ov5647_sunny_ctrl->curr_lens_pos = dest_lens_position;
		ov5647_sunny_ctrl->curr_step_pos = dest_step_position;
		return rc;

}

static int32_t ov5647_sunny_set_default_focus(void)
{

	int32_t rc = 0;
	if (ov5647_sunny_ctrl->curr_step_pos != 0) {
		rc = ov5647_sunny_move_focus(MOVE_FAR, ov5647_sunny_ctrl->curr_step_pos);
		if (rc < 0) {
			CDBG("ov5647_sunny_set_default_focus Failed!!!\n");
			return rc;
		}
	} else {
	CDBG("%s:%d\n",__func__,__LINE__);
		rc = ov5647_sunny_af_i2c_write(0);
		if (rc < 0) {
			CDBG("ov5647_sunny_go_to_position Failed!!!\n");
			return rc;
		}
	}
	ov5647_sunny_ctrl->curr_lens_pos = 0;
	ov5647_sunny_ctrl->curr_step_pos = 0;

	return rc;
}
static int ov5647_sunny_probe_init_done(const struct msm_camera_sensor_info *data)
{
    CDBG("ov5647_sunny_probe_init_done start\n");

    gpio_direction_output ( data->sensor_pwd, 1);
    gpio_free ( data->sensor_pwd );
    gpio_free ( data->sensor_reset );
    gpio_free ( data->vcm_pwd);
/*probe finish ,power down camera*/
    if (data->vreg_disable_func)
    {
        data->vreg_disable_func(0);
    }
    return 0;
}

static int ov5647_sunny_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
    int32_t rc;
    uint16_t chipid;

    CDBG("ov5647_sunny_probe_init_sensor\n");

    rc = gpio_request(data->sensor_reset, "ov5647_sunny");
    if (!rc)
    {
        gpio_direction_output(data->sensor_reset, 1);
    }
    else
    {
        gpio_free(data->sensor_reset);
        gpio_request(data->sensor_reset, "ov5647_sunny");
        rc = gpio_direction_output(data->sensor_reset, 1);
        CDBG("gpio_direction_output(data->sensor_reset, 0) rc=%d \n", rc);
    }
	
    mdelay(1);

    rc = gpio_request(data->sensor_pwd, "ov5647_sunny");
    if (!rc)
    {
        gpio_direction_output(data->sensor_pwd, 1);
    }
    else
    {
        gpio_free(data->sensor_pwd);
        gpio_request(data->sensor_pwd, "ov5647_sunny");
        rc = gpio_direction_output(data->sensor_pwd, 1);
        CDBG("gpio_direction_output(data->sensor_pwd, 1) rc=%d \n", rc);
    }
	
    mdelay(1);

    rc = gpio_request(data->vcm_pwd, "ov5647_sunny");
    if (!rc)
    {
        gpio_direction_output(data->vcm_pwd, 1);
    }
    else
    {
        gpio_free(data->vcm_pwd);
        gpio_request(data->vcm_pwd, "ov5647_sunny");
        rc = gpio_direction_output(data->vcm_pwd, 1);
        CDBG("gpio_direction_output(data->vcm_pwd, 1) rc=%d \n", rc);
    }
	
    mdelay(1);

    if (data->vreg_enable_func)
    {
        data->vreg_enable_func(1);
    }
    mdelay( 6 );

    gpio_direction_output(data->sensor_pwd, 0);

    mdelay(22);

    /*  Read sensor Model ID: */
    rc = ov5647_sunny_i2c_read_w(ov5647_sunny_client->addr,
                                 OV5647_SUNNY_REG_MODEL_ID, &chipid);
    if (rc < 0)
    {
        goto init_probe_fail;
    }

    CDBG("ov5647_sunny  model_id = 0x%x\n", chipid);

    /*  Compare sensor ID to OV5647_SUNNY ID: */
    if (chipid != OV5647_SUNNY_MODEL_ID)
    {
        CDBG("ov5647_sunny wrong model_id = 0x%x\n", chipid);
        rc = -ENODEV;
        goto init_probe_fail;
    }

    goto init_probe_done;

    CDBG("init_probe_done start\n");
init_probe_fail:
    ov5647_sunny_probe_init_done(data);
init_probe_done:
    return rc;
}

static int ov5647_sunny_sensor_open_init(const struct msm_camera_sensor_info *data)
{
    int32_t rc;

    ov5647_sunny_ctrl = kzalloc(sizeof(struct ov5647_sunny_ctrl), GFP_KERNEL);
    if (!ov5647_sunny_ctrl)
    {
        CDBG("ov5647_sunny_init failed!\n");
        rc = -ENOMEM;
        goto init_done;
    }

    ov5647_sunny_ctrl->fps_divider = 1 * 0x00000400;
    ov5647_sunny_ctrl->pict_fps_divider = 1 * 0x00000400;
    ov5647_sunny_ctrl->set_test = TEST_OFF;
    ov5647_sunny_ctrl->prev_res = QTR_SIZE;
    ov5647_sunny_ctrl->pict_res = FULL_SIZE;

    if (data)
    {
        ov5647_sunny_ctrl->sensordata = data;
    }

    msm_camio_camif_pad_reg_reset();
    mdelay( 20 );

    rc = ov5647_sunny_probe_init_sensor(data);
    CDBG("ov5647_sunny_probe_init_sensor rc=%d!\n", rc);
    if (rc < 0)
    {
        goto init_fail1;
    }

    if (ov5647_sunny_ctrl->prev_res == QTR_SIZE)
    {
        rc = ov5647_sunny_setting(REG_INIT, RES_PREVIEW);
    }
    else
    {
        rc = ov5647_sunny_setting(REG_INIT, RES_CAPTURE);
    }

    if (rc < 0)
    {
        CDBG("ov5647_sunny_setting failed. rc = %d\n", rc);
        goto init_fail1;
    }

	ov5647_sunny_setup_af_tbl();

    goto init_done;
init_fail1:
    ov5647_sunny_probe_init_done(data);
    kfree(ov5647_sunny_ctrl);
init_done:
    return rc;
}

static int ov5647_sunny_init_client(struct i2c_client *client)
{
    /* Initialize the MSM_CAMI2C Chip */
    init_waitqueue_head(&ov5647_sunny_wait_queue);
    return 0;
}

static int32_t ov5647_sunny_set_sensor_mode(int mode, int res)
{
    int32_t rc = 0;

    switch (mode)
    {
        case SENSOR_PREVIEW_MODE:
            rc = ov5647_sunny_video_config(mode, res);
            break;

        case SENSOR_SNAPSHOT_MODE:
            rc = ov5647_sunny_snapshot_config(mode);
            break;

        case SENSOR_RAW_SNAPSHOT_MODE:
            rc = ov5647_sunny_raw_snapshot_config(mode);
            break;

        default:
            rc = -EINVAL;
            break;
    }

    return rc;
}

int ov5647_sunny_sensor_config(void __user *argp)
{
    struct sensor_cfg_data cdata;
    int rc = 0;

    if (copy_from_user(&cdata,
                       (void *)argp, sizeof(struct sensor_cfg_data)))
    {
        return -EFAULT;
    }

    mutex_lock(&ov5647_sunny_mut);

    CDBG("%s: cfgtype = %d\n", __func__, cdata.cfgtype);
    switch (cdata.cfgtype)
    {
        case CFG_GET_PICT_FPS:
            ov5647_sunny_get_pict_fps(cdata.cfg.gfps.prevfps,
                                      &(cdata.cfg.gfps.pictfps));

            if (copy_to_user((void *)argp, &cdata,
                             sizeof(struct sensor_cfg_data)))
            {
                rc = -EFAULT;
            }

            break;

        case CFG_GET_PREV_L_PF:
            cdata.cfg.prevl_pf = ov5647_sunny_get_prev_lines_pf();

            if (copy_to_user((void *)argp,
                             &cdata, sizeof(struct sensor_cfg_data)))
            {
                rc = -EFAULT;
            }

            break;

        case CFG_GET_PREV_P_PL:
            cdata.cfg.prevp_pl = ov5647_sunny_get_prev_pixels_pl();

            if (copy_to_user((void *)argp,
                             &cdata, sizeof(struct sensor_cfg_data)))
            {
                rc = -EFAULT;
            }

            break;

        case CFG_GET_PICT_L_PF:
            cdata.cfg.pictl_pf = ov5647_sunny_get_pict_lines_pf();

            if (copy_to_user((void *)argp,
                             &cdata, sizeof(struct sensor_cfg_data)))
            {
                rc = -EFAULT;
            }

            break;

        case CFG_GET_PICT_P_PL:
            cdata.cfg.pictp_pl = ov5647_sunny_get_pict_pixels_pl();

            if (copy_to_user((void *)argp,
                             &cdata, sizeof(struct sensor_cfg_data)))
            {
                rc = -EFAULT;
            }

            break;

        case CFG_GET_PICT_MAX_EXP_LC:
            cdata.cfg.pict_max_exp_lc = ov5647_sunny_get_pict_max_exp_lc();

            if (copy_to_user((void *)argp,
                             &cdata, sizeof(struct sensor_cfg_data)))
            {
                rc = -EFAULT;
            }

            break;

        case CFG_SET_FPS:
        case CFG_SET_PICT_FPS:
            rc = ov5647_sunny_set_fps(&(cdata.cfg.fps));
            break;

        case CFG_SET_EXP_GAIN:
            rc = ov5647_sunny_write_exp_gain(cdata.cfg.exp_gain.gain,
                                             cdata.cfg.exp_gain.line);
            break;

        case CFG_SET_PICT_EXP_GAIN:
            CDBG("Line:%d CFG_SET_PICT_EXP_GAIN \n", __LINE__);
            rc = ov5647_sunny_set_pict_exp_gain(cdata.cfg.exp_gain.gain,
                                                cdata.cfg.exp_gain.line);
            break;

        case CFG_SET_MODE:
            rc = ov5647_sunny_set_sensor_mode(cdata.mode, cdata.rs);
            break;

        case CFG_PWR_DOWN:
            rc = ov5647_sunny_power_down();
            break;

        case CFG_MOVE_FOCUS:
            CDBG("ov5647_sunny_ioctl: CFG_MOVE_FOCUS: cdata.cfg.focus.dir=%d \
				cdata.cfg.focus.steps=%d\n"                                                                   ,
                 cdata.cfg.focus.dir, cdata.cfg.focus.steps);
            rc = ov5647_sunny_move_focus(cdata.cfg.focus.dir,
                                         cdata.cfg.focus.steps);
            break;

        case CFG_SET_DEFAULT_FOCUS:
            rc = ov5647_sunny_set_default_focus();
            break;

        case CFG_SET_LENS_SHADING:
            CDBG("%s: CFG_SET_LENS_SHADING\n", __func__);
            rc = ov5647_sunny_lens_shading_enable(cdata.cfg.lens_shading);
            break;

        case CFG_GET_AF_MAX_STEPS:
            cdata.max_steps = OV5647_SUNNY_STEPS_NEAR_TO_CLOSEST_INF;
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

    mutex_unlock(&ov5647_sunny_mut);

    return rc;
}

int ov5647_sunny_sensor_release(void)
{
    int rc = -EBADF;

    mutex_lock(&ov5647_sunny_mut);

    gpio_direction_output(ov5647_sunny_ctrl->sensordata->sensor_pwd, 1);
    gpio_free(ov5647_sunny_ctrl->sensordata->sensor_reset);

    gpio_free(ov5647_sunny_ctrl->sensordata->sensor_pwd);

    gpio_free(ov5647_sunny_ctrl->sensordata->vcm_pwd);
/*quit camera application ,power down camera*/
    rc = ov5647_sunny_power_down();
    kfree(ov5647_sunny_ctrl);
    ov5647_sunny_ctrl = NULL;

    CDBG("ov5647_sunny_release completed\n");

    mutex_unlock(&ov5647_sunny_mut);
    return rc;
}

static int ov5647_sunny_i2c_probe(struct i2c_client *         client,
                                  const struct i2c_device_id *id)
{
    int rc = 0;

    CDBG("ov5647_sunny_probe called!\n");

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        CDBG("i2c_check_functionality failed\n");
        goto probe_failure;
    }

    ov5647_sunny_sensorw = kzalloc(sizeof(struct ov5647_sunny_work), GFP_KERNEL);
    if (!ov5647_sunny_sensorw)
    {
        CDBG("kzalloc failed.\n");
        rc = -ENOMEM;
        goto probe_failure;
    }

    i2c_set_clientdata(client, ov5647_sunny_sensorw);
    ov5647_sunny_init_client(client);
    ov5647_sunny_client = client;

    mdelay(50);

    CDBG("ov5647_sunny_probe successed! rc = %d\n", rc);
    return 0;

probe_failure:
    CDBG("ov5647_sunny_probe failed! rc = %d\n", rc);
    return rc;
}

static const struct i2c_device_id ov5647_sunny_i2c_id[] =
{
    {"ov5647_sunny", 0},
    {}
};

static struct i2c_driver ov5647_sunny_i2c_driver =
{
    .id_table = ov5647_sunny_i2c_id,
    .probe    = ov5647_sunny_i2c_probe,
    .remove   = __exit_p(ov5647_sunny_i2c_remove),
    .driver   = {
        .name = "ov5647_sunny",
    },
};

static int ov5647_sunny_sensor_probe(const struct msm_camera_sensor_info *info,
                                     struct msm_sensor_ctrl *             s)
{
    int rc = i2c_add_driver(&ov5647_sunny_i2c_driver);

    CDBG("ov5647_sunny_sensor_probe \n" );
    if ((rc < 0) || (ov5647_sunny_client == NULL))
    {
        rc = -ENOTSUPP;
        CDBG("ov5647:   i2c_add_driver  failed \n" );
        goto probe_done;
    }

    rc = ov5647_sunny_probe_init_sensor(info);
    if (rc < 0)
    {
        CDBG("ov5647_sunny_probe_init_sensor failed!!\n");
        goto probe_done;
    }

    #ifdef CONFIG_HUAWEI_HW_DEV_DCT
    /* detect current device successful, set the flag as present */
    set_hw_dev_flag(DEV_I2C_CAMERA_MAIN);
    #endif
    s->s_init = ov5647_sunny_sensor_open_init;
    s->s_release = ov5647_sunny_sensor_release;
    s->s_config = ov5647_sunny_sensor_config;
	s->s_camera_type = BACK_CAMERA_2D;
	s->s_mount_angle = 0;
    ov5647_sunny_probe_init_done(info);

probe_done:
    CDBG("%s %s:%d\n", __FILE__, __func__, __LINE__);
    return rc;
}

static int __ov5647_sunny_probe(struct platform_device *pdev)
{
    return msm_camera_drv_start(pdev, ov5647_sunny_sensor_probe);
}

static struct platform_driver msm_camera_driver =
{
    .probe     = __ov5647_sunny_probe,
    .driver    = {
        .name  = "msm_camera_ov5647_sunny",
        .owner = THIS_MODULE,
    },
};

static int __init ov5647_sunny_init(void)
{
    return platform_driver_register(&msm_camera_driver);
}

module_init(ov5647_sunny_init);
