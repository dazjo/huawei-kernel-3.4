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

#ifndef MT9T113_H
#define MT9T113_H

#include <mach/board.h>
#include <linux/types.h>
extern struct mt9t113_reg mt9t113_regs;

struct mt9t113_i2c_reg_conf
{
    unsigned short waddr;
    unsigned short wdata;
};

struct mt9t113_reg
{
    const struct mt9t113_i2c_reg_conf * mt9t113_init_reg_config_sunny;
    const unsigned short               mt9t113_init_reg_config_sunny_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_init_reg_config_sunny_2;
    const unsigned short               mt9t113_init_reg_config_sunny_2_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_init_reg_config_sunny_3;
    const unsigned short               mt9t113_init_reg_config_sunny_3_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_init_reg_sensor_start;
    const unsigned short               mt9t113_init_reg_sensor_start_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_effect_off_reg_config;
    const unsigned short               mt9t113_effect_off_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_effect_mono_reg_config;
    const unsigned short               mt9t113_effect_mono_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_effect_negative_reg_config;
    const unsigned short               mt9t113_effect_negative_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_effect_sepia_reg_config;
    const unsigned short               mt9t113_effect_sepia_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_effect_aqua_reg_config;
    const unsigned short               mt9t113_effect_aqua_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_effect_solarize_reg_config;
    const unsigned short               mt9t113_effect_solarize_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_effect_posterize_reg_config;
    const unsigned short               mt9t113_effect_posterize_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_preview_reg_config;
    const unsigned short               mt9t113_preview_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_snapshot_reg_config;
    const unsigned short               mt9t113_snapshot_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_wb_auto_reg_config;
    const unsigned short               mt9t113_wb_auto_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_wb_a_reg_config;
    const unsigned short               mt9t113_wb_a_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_wb_tl84_reg_config;
    const unsigned short               mt9t113_wb_tl84_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_wb_f_reg_config;
    const unsigned short               mt9t113_wb_f_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_wb_d65_reg_config;
    const unsigned short               mt9t113_wb_d65_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_wb_d50_reg_config;
    const unsigned short               mt9t113_wb_d50_reg_config_size;
    const struct mt9t113_i2c_reg_conf * mt9t113_mirror_mode_reg_config;
    const unsigned short               mt9t113_mirror_mode_reg_config_size;
    /*delete antibanding array pointer*/
};

#endif /* MT9T113_H */
