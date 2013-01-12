/* include/linux/input/gs_kxtik1004.h*/
/*
 * Copyright (C) 2012 HUAWEI, Inc.
 * Author: zhangmin/195861 
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

#ifndef __KXTIK_H__
#define __KXTIK_H__

#define ECS_IOCTL_READ_ACCEL_XYZ            _IOR(0xA1, 0x06, char[3])
#define ECS_IOCTL_APP_SET_DELAY		        _IOW(0xA1, 0x18, short)
#define ECS_IOCTL_APP_GET_DELAY             _IOR(0xA1, 0x30, short)
#define ECS_IOCTL_APP_SET_AFLAG		        _IOW(0xA1, 0x13, short)
#define ECS_IOCTL_APP_GET_AFLAG		        _IOR(0xA1, 0x14, short)
#define ECS_IOCTL_READ_DEVICEID				_IOR(0xA1, 0x31, char[20])

/* CTRL_REG1: set resolution, g-range, data ready enable 
        Output resolution: 8-bit valid or 12-bit valid */
#define RES_8BIT		0x00
#define RES_12BIT		0x40
u8 res_12bit;
/* Output g-range: +/-2g, 4g, or 8g */
#define KXTIK_G_2G		0x00
#define KXTIK_G_4G		0x08
#define KXTIK_G_8G		0x10
u8 g_range;

#define	GPIO_INT1		19

/* DATA_CTRL_REG: controls the output data rate of the part */
#define ODR12_5F		0x00
#define ODR25F			0x01
#define ODR50F			0x02
#define ODR100F			0x03
#define ODR200F			0x04
#define ODR400F			0x05
#define ODR800F			0x06

#define G_MAX			8000
/* OUTPUT REGISTERS */
#define XOUT_L_REG		0x06
#define XOUT_H_REG		0x07
#define YOUT_L_REG		0x08
#define YOUT_H_REG		0x09
#define ZOUT_L_REG		0x0A
#define ZOUT_H_REG		0x0B

#define GS_KX_TIMRER		(1000)		/*1000ms*/
#define WHO_AM_I		0x0F
/* CONTROL REGISTERS */
#define INT_REL			0x1A
#define CTRL_REG1		0x1B
#define INT_CTRL1		0x1E
#define DATA_CTRL		0x21
/* CONTROL REGISTER 1 BITS */
#define PC1_OFF			0x7F
#define PC1_ON			0x80
/* INPUT_ABS CONSTANTS */
#define FUZZ			3
#define FLAT			3
/* RESUME STATE INDICES */
#define RES_DATA_CTRL		0
#define RES_CTRL_REG1		1
#define RES_INT_CTRL1		2
#define RESUME_ENTRIES		3

#endif  /* __KXTIK_H__ */
