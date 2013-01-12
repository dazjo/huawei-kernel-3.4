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

#ifndef OV7690_H
#define OV7690_H

#include <linux/types.h>
#include <mach/camera.h>


enum ov7690_width
{
    WORD_LEN,
    BYTE_LEN
};

struct ov7690_i2c_reg_conf
{
    unsigned short    waddr;
    unsigned short    wdata;
    enum ov7690_width width;
    unsigned short    mdelay_time;
};

struct ov7690_reg
{
    const struct register_address_value_pair *prev_snap_reg_settings;
    uint16_t                                  prev_snap_reg_settings_size;
};

long ov7690_reg_init(void);

#endif /* OV7690_H */
