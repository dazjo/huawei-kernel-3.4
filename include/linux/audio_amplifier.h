/*
 * include/linux/tpa2028d1_i2c.h - platform data structure for tpa2028d1
 *
 * Copyright (C) 2008 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef _LINUX_AUDIO_AMPLIFIER_H
#define _LINUX_AUDIO_AMPLIFIER_H


struct amplifier_platform_data {
	void (*amplifier_on)(void);	
    void (*amplifier_off)(void);	
    #ifdef CONFIG_HUAWEI_KERNEL
    void (*amplifier_4music_on)(void);
    #endif
};

#endif /* _LINUX_AUDIO_AMPLIFIER_H */
