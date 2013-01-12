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
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include "ov7690.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <linux/hw_dev_dec.h>
#endif

#define SENSOR_DEBUG 1
#undef CDBG
#define CDBG(fmt, args...) printk(KERN_ERR "ov7690:" fmt, ## args)

#define OV7690_REG_MODEL_ID 0x0a
#define OV7690_MODEL_ID 0x76
#define OV7690_REG_RESET_REGISTER 0x12
#define OV7690_RESET_DELAY_MSECS 66

struct ov7690_work
{
    struct work_struct work;
};

static struct  ov7690_work *ov7690_sensorw;
static struct  i2c_client *ov7690_client;

struct ov7690_ctrl
{
    const struct msm_camera_sensor_info *sensordata;
};

static struct ov7690_ctrl *ov7690_ctrl;

static DECLARE_WAIT_QUEUE_HEAD(ov7690_wait_queue);
DEFINE_SEMAPHORE(ov7690_sem);

struct register_address_value_pair const
reg_settings_array[] =
{

    {0x0c, 0x56},
    {0x48, 0x42},
    {0x41, 0x43},
    {0x4c, 0x73},

    {0x81, 0xef},
    {0x21, 0x44},
    {0x16, 0x03},
    {0x39, 0x80},
    {0x1e, 0xb1},

    {0x12, 0x00},
    {0x82, 0x03},
    {0xd0, 0x48},
    {0x80, 0x7e},
    {0x3e, 0x30},
    {0x22, 0x00},

    {0x17, 0x69},
    {0x18, 0xa4},
    {0x19, 0x0c},
    {0x1a, 0xf6},

    {0xc8, 0x02},
    {0xc9, 0x80}, 
    {0xca, 0x01},
    {0xcb, 0xe0}, 

    {0xcc, 0x02},
    {0xcd, 0x80}, 
    {0xce, 0x01},
    {0xcf, 0xe0}, 

    {0x85, 0x90},
    {0x86, 0x18},

    {0x87, 0x00},
    {0x88, 0x10},

    {0x89, 0x18},
    {0x8a, 0x10},
    {0x8b, 0x14},

    {0xBB, 0x20},
    {0xBC, 0x40},
    {0xBD, 0x60},
    {0xBE, 0x58},
    {0xBF, 0x48},
    {0xC0, 0x10},
    {0xC1, 0x33},
    {0xc2, 0x02},

    {0xb7, 0x02},
    {0xb8, 0x0b},
    {0xb9, 0x00},
    {0xba, 0x18},

    {0x5A, 0x4A},
    {0x5B, 0x9F},
    {0x5C, 0x48},
    {0x5d, 0x32},

    {0x24, 0x88},
    {0x25, 0x78},
    {0x26, 0xb3},

    {0xa3, 0x0a},
    {0xa4, 0x13},
    {0xa5, 0x28},
    {0xa6, 0x50},
    {0xa7, 0x60},
    {0xa8, 0x72},
    {0xa9, 0x7e},
    {0xaa, 0x8a},
    {0xab, 0x94},
    {0xac, 0x9c},
    {0xad, 0xa8},
    {0xae, 0xb4},
    {0xaf, 0xc6},
    {0xb0, 0xd7},
    {0xb1, 0xe8},
    {0xb2, 0x20},

    {0x8e, 0x92}, 
    {0x96, 0xff},
    {0x97, 0x00}, 

    {0x50, 0x4d}, 
    {0x51, 0x3f},
    {0x21, 0x57}, 
    {0x20, 0x00},

    {0x14, 0x39},
    {0x13, 0xf7},
    {0x11, 0x01},
    {0x68, 0xb0},
};

struct ov7690_reg ov7690_regs =
{
    .prev_snap_reg_settings      = &reg_settings_array[0],
    .prev_snap_reg_settings_size = ARRAY_SIZE(
        reg_settings_array),
};


/*=============================================================
    EXTERNAL DECLARATIONS
==============================================================*/

/*=============================================================*/

static int32_t ov7690_i2c_txdata(unsigned short saddr,
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

#if SENSOR_DEBUG
    if (length == 2)
    {
        CDBG("msm_io_i2c_w: 0x%04x 0x%04x\n",
             *(u16 *) txdata, *(u16 *) (txdata + 2));
    }
    else if (length == 4)
    {
        CDBG("msm_io_i2c_w: 0x%04x\n", *(u16 *) txdata);
    }
    else
    {
        CDBG("msm_io_i2c_w: length = %d\n", length);
    }
#endif

    if (i2c_transfer(ov7690_client->adapter, msg, 1) < 0)
    {
        CDBG("ov7690_i2c_txdata failed\n");
        return -EIO;
    }

    return 0;
}

static int32_t ov7690_i2c_write(unsigned short saddr,
                                unsigned short waddr, unsigned short wdata, enum ov7690_width width)
{
    int32_t rc = -EIO;
    unsigned char buf[4];

    memset(buf, 0, sizeof(buf));
    switch (width)
    {
        case WORD_LEN:
        {
            buf[0] = (waddr & 0xFF00) >> 8;
            buf[1] = (waddr & 0x00FF);
            buf[2] = (wdata & 0xFF00) >> 8;
            buf[3] = (wdata & 0x00FF);

            rc = ov7690_i2c_txdata(saddr, buf, 4);
        }
            break;

        case BYTE_LEN:
        {
            buf[0] = waddr;
            buf[1] = wdata;
            rc = ov7690_i2c_txdata(saddr, buf, 2);
        }
            break;

        default:
            break;
    }

    if (rc < 0)
    {
        CDBG(
            "i2c_write failed, addr = 0x%x, val = 0x%x!\n",
            waddr, wdata);
    }

    return rc;
}

static int ov7690_i2c_read(unsigned short saddr,
                           unsigned char reg, unsigned char *value)
{
    unsigned char buf;
    struct i2c_msg msgs[] =
    {
        {
            .addr  = saddr,
            .flags = 0,
            .len = 1,
            .buf = &buf,
        },
        {
            .addr  = saddr,
            .flags = I2C_M_RD,
            .len = 1,
            .buf = &buf,
        },
    };

    buf = reg;

    if (i2c_transfer(ov7690_client->adapter, msgs, 2) < 0)
    {
        CDBG("ov7690_i2c_read failed!\n");
        return -EIO;
    }

    *value = buf;

    return 0;
}

long ov7690_reg_init(void)
{
    int32_t array_length;
    int32_t i;
    long rc;

    array_length = ov7690_regs.prev_snap_reg_settings_size;

    /* Configure sensor for Preview mode and Snapshot mode */
    for (i = 0; i < array_length; i++)
    {
        rc = ov7690_i2c_write(ov7690_client->addr,
                              ov7690_regs.prev_snap_reg_settings[i].register_address,
                              ov7690_regs.prev_snap_reg_settings[i].register_value,
                              BYTE_LEN);

        if (rc < 0)
        {
            return rc;
        }
    }

    return 0;
}

static long ov7690_set_effect(int mode, int effect)
{
    return 0;
}

static long ov7690_set_sensor_mode(int mode)
{
    switch (mode)
    {
        case SENSOR_PREVIEW_MODE:
            break;

        case SENSOR_SNAPSHOT_MODE:
            break;

        case SENSOR_RAW_SNAPSHOT_MODE:
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

static int ov7690_sensor_init_done(const struct msm_camera_sensor_info *data)
{
    int rc = 0;

    rc = gpio_direction_output(data->sensor_reset, 1);
    if (rc < 0)
    {
        CDBG("ov7690_sensor_init_done:gpio_direction_output failed\n");
    }

    gpio_free(data->sensor_reset);

     if (data->vreg_disable_func)
    {
        data->vreg_disable_func(0);
    }

    return rc;
}

static int ov7690_sensor_init_probe(const struct msm_camera_sensor_info *data)
{
    int rc;
    unsigned char chipid;

    CDBG("ov7690_sensor_init_probe\n");

    rc = gpio_request(data->sensor_reset, "ov7690");
    if (!rc)
    {
        gpio_direction_output(data->sensor_reset, 0);
    }
    else
    {
        goto init_probe_fail;
    }

    mdelay(OV7690_RESET_DELAY_MSECS);

    if (data->vreg_enable_func)
    {
        data->vreg_enable_func(1);
    }

    mdelay(OV7690_RESET_DELAY_MSECS);

    /* RESET the sensor image part via I2C command */
    rc = ov7690_i2c_write(ov7690_client->addr,
                          OV7690_REG_RESET_REGISTER, 0x80, BYTE_LEN);
    if (rc < 0)
    {
        goto init_probe_fail;
    }

    mdelay(5);

    /* 3. Read sensor Model ID: */
    rc = ov7690_i2c_read(ov7690_client->addr,
                         OV7690_REG_MODEL_ID, &chipid);

    if (rc < 0)
    {
        goto init_probe_fail;
    }

    CDBG("ov7690 model_id = 0x%x\n", chipid);

    /* 4. Compare sensor ID to OV7690 ID: */
    if (chipid != OV7690_MODEL_ID)
    {
        rc = -ENODEV;
        goto init_probe_fail;
    }

    ov7690_reg_init();

    goto init_probe_done;

init_probe_fail:
    ov7690_sensor_init_done(data);
init_probe_done:
    return rc;
}

int ov7690_sensor_init(const struct msm_camera_sensor_info *data)
{
    int rc = 0;

    ov7690_ctrl = kzalloc(sizeof(struct ov7690_ctrl), GFP_KERNEL);
    if (!ov7690_ctrl)
    {
        CDBG("ov7690_init failed!\n");
        rc = -ENOMEM;
        goto init_done;
    }

    if (data)
    {
        ov7690_ctrl->sensordata = data;
    }

    /* Input MCLK = 24MHz */
    msm_camio_clk_rate_set(24000000);
    mdelay(5);

    msm_camio_camif_pad_reg_reset();

    rc = ov7690_sensor_init_probe(data);
    if (rc < 0)
    {
        CDBG("ov7690_sensor_init failed!\n");
        goto init_fail;
    }

    rc = ov7690_reg_init();
    if (rc < 0)
    {
        goto init_fail;
    }

init_done:
    return rc;

init_fail:
    kfree(ov7690_ctrl);
    return rc;
}

static int ov7690_init_client(struct i2c_client *client)
{
    /* Initialize the MSM_CAMI2C Chip */
    init_waitqueue_head(&ov7690_wait_queue);
    return 0;
}

int ov7690_sensor_config(void __user *argp)
{
    struct sensor_cfg_data cfg_data;
    long rc = 0;

    if (copy_from_user(&cfg_data,
                       (void *)argp,
                       sizeof(struct sensor_cfg_data)))
    {
        return -EFAULT;
    }

    /* down(&ov7690_sem); */

    CDBG("ov7690_ioctl, cfgtype = %d, mode = %d\n",
         cfg_data.cfgtype, cfg_data.mode);

    switch (cfg_data.cfgtype)
    {
        case CFG_SET_MODE:
            rc = ov7690_set_sensor_mode(
                cfg_data.mode);
            break;

        case CFG_SET_EFFECT:
            rc = ov7690_set_effect(cfg_data.mode,
                                   cfg_data.cfg.effect);
            break;

        case CFG_GET_AF_MAX_STEPS:
        default:
            rc = -EINVAL;
            break;
    }

    /* up(&ov7690_sem); */

    return rc;
}

int ov7690_sensor_release(void)
{
    int rc = 0;

    /* down(&ov7690_sem); */

    down(&ov7690_sem);
    ov7690_sensor_init_done(ov7690_ctrl->sensordata);

    kfree(ov7690_ctrl);

    up(&ov7690_sem);

    //CDBG("ov7690_release completed!\n");
    return rc;

    /* up(&ov7690_sem); */

    return rc;
}

static int ov7690_i2c_probe(struct i2c_client *         client,
                            const struct i2c_device_id *id)
{
    int rc = 0;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        rc = -ENOTSUPP;
        goto probe_failure;
    }

    ov7690_sensorw =
        kzalloc(sizeof(struct ov7690_work), GFP_KERNEL);

    if (!ov7690_sensorw)
    {
        rc = -ENOMEM;
        goto probe_failure;
    }

    i2c_set_clientdata(client, ov7690_sensorw);
    ov7690_init_client(client);
    ov7690_client = client;

    CDBG("ov7690_probe succeeded!\n");

    return 0;

probe_failure:
    kfree(ov7690_sensorw);
    ov7690_sensorw = NULL;
    CDBG("ov7690_probe failed!\n");
    return rc;
}

static const struct i2c_device_id ov7690_i2c_id[] =
{
    { "ov7690", 0},
    { },
};

static struct i2c_driver ov7690_i2c_driver =
{
    .id_table = ov7690_i2c_id,
    .probe    = ov7690_i2c_probe,
    .remove   = __exit_p(ov7690_i2c_remove),
    .driver   = {
        .name = "ov7690",
    },
};

int ov7690_sensor_probe(const struct msm_camera_sensor_info *info,
                        struct msm_sensor_ctrl *             s)
{
    int rc = i2c_add_driver(&ov7690_i2c_driver);

    if ((rc < 0) || (ov7690_client == NULL))
    {
        rc = -ENOTSUPP;
        goto probe_done;
    }

    /* Input MCLK = 24MHz */

    //msm_camio_clk_rate_set(24000000);
    mdelay(5);

    CDBG("ov7690_sensor_probe!\n");

    rc = ov7690_sensor_init_probe(info);
    if (rc < 0)
    {
        goto probe_done;
    }

    CDBG("ov7690_sensor_init_probe rc=%d! \n", rc);
    #ifdef CONFIG_HUAWEI_HW_DEV_DCT
    /* detect current device successful, set the flag as present */
    set_hw_dev_flag(DEV_I2C_CAMERA_SLAVE);
    #endif
    s->s_init = ov7690_sensor_init;
    s->s_release = ov7690_sensor_release;
    s->s_config = ov7690_sensor_config;
	s->s_camera_type = FRONT_CAMERA_2D;
	s->s_mount_angle = 0;

    ov7690_sensor_init_done(info);

probe_done:
    CDBG("%s %s:%d\n", __FILE__, __func__, __LINE__);
    return rc;
}

static int __ov7690_probe(struct platform_device *pdev)
{
    CDBG("------__ov7690_probe!-------------\n");
    return msm_camera_drv_start(pdev, ov7690_sensor_probe);
}

static struct platform_driver msm_camera_driver =
{
    .probe     = __ov7690_probe,
    .driver    = {
        .name  = "msm_camera_ov7690",
        .owner = THIS_MODULE,
    },
};

static int __init ov7690_init(void)
{
    CDBG("------ov7690_init!-------------\n");
    return platform_driver_register(&msm_camera_driver);
}

module_init(ov7690_init);
