/* drivers/input/misc/aps-9900.h
 *
 * Copyright (C) 2010 HUAWEI, Inc.
 * Author: Benjamin Gao <gaohuajiang@huawei.com>
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

#ifndef _LINUX_APS_9900_H
#define _LINUX_APS_9900_H

#define ECS_IOCTL_APP_SET_DELAY	    _IOW(0xA1, 0x18, short)
#define ECS_IOCTL_APP_GET_DELAY       _IOR(0xA1, 0x30, short)
#define ECS_IOCTL_APP_SET_LFLAG		_IOW(0xA1, 0x1C, short)
#define ECS_IOCTL_APP_GET_LFLAG		_IOR(0xA1, 0x1B, short)
#define ECS_IOCTL_APP_SET_PFLAG		_IOW(0xA1, 0x1E, short)
#define ECS_IOCTL_APP_GET_PFLAG		_IOR(0xA1, 0x1D, short)
#define ECS_IOCTL_APP_GET_PDATA_VALVE	_IOR(0xA1, 0x32, short)
#define ECS_IOCTL_APP_GET_LDATA_VALVE	_IOR(0xA1, 0x33, short)

#define VREG_GP4_NAME	"gp4"
#define VREG_GP4_VOLTAGE_VALUE	2700

#define CMD_BYTE      0x80
#define CMD_WORD      0xA0
#define CMD_SPECIAL   0xE0

#define APDS9900_ENABLE_REG  0x00
#define APDS9900_ATIME_REG   0x01
#define APDS9900_PTIME_REG   0x02
#define APDS9900_WTIME_REG   0x03
#define APDS9900_AILTL_REG   0x04
#define APDS9900_AILTH_REG   0x05
#define APDS9900_AIHTL_REG   0x06
#define APDS9900_AIHTH_REG   0x07
#define APDS9900_PILTL_REG   0x08
#define APDS9900_PILTH_REG   0x09
#define APDS9900_PIHTL_REG   0x0A
#define APDS9900_PIHTH_REG   0x0B
#define APDS9900_PERS_REG    0x0C
#define APDS9900_CONFIG_REG  0x0D
#define APDS9900_PPCOUNT_REG 0x0E
#define APDS9900_CONTROL_REG 0x0F
#define APDS9900_REV_REG     0x11
#define APDS9900_ID_REG      0x12
#define APDS9900_STATUS_REG  0x13
#define APDS9900_CDATAL_REG  0x14
#define APDS9900_CDATAH_REG  0x15
#define APDS9900_IRDATAL_REG 0x16
#define APDS9900_IRDATAH_REG 0x17
#define APDS9900_PDATAL_REG  0x18
#define APDS9900_PDATAH_REG  0x19

#define DETECTION_THRESHOLD	500

#define PPCOUNT_REG_INDEX 4
#define APDS9900_POWER_ON 1     /* set the APDS9900_ENABLE_REG's PON=1,Writing a 1 activates the APDS9900 */
#define APDS9900_POWER_OFF 0    /* set the APDS9900_ENABLE_REG's PON=1,Writing a 0 disables the APDS9900 */
/*reconfig reg after resume*/
#define APDS9900_ENABLE 0x3F    /* set the APDS9900_ENABLE_REG's*/
#define APDS9900_POWER_MASK (1<<0)
#define APDS9900_STATUS_PROXIMITY_BIT (1<<5)
#define APDS9900_STATUS_ALS_BIT (1<<4)
#define APDS9900_PEN_BIT_SHIFT 2
#define APDS9900_AEN_BIT_SHIFT 1

#define APDS_9901_ID  0x20 /* APDS-9901 */
#define APDS_9900_ID  0x29 /* APDS-9900 */
#define APDS_9900_REV_ID 0x01
#define APDS9900_REG0_POWER_OFF	0xfe
#define APDS9900_REG0_AEN_OFF	0xfd
#define APDS9900_REG0_PEN_OFF	0xfb

/* add lsensor level*/
#define LSENSOR_MAX_LEVEL 7
/* The max value of the  variable open_count */
#define OPEN_COUNT_MAX (10)
#define MAX_ADC_PROX_VALUE 1022

#define U8655_WAVE  35
#define U8655_WINDOW  115
#define U8815_WAVE  150
#define U8815_WINDOW 150
#define C8655_WAVE  40
#define C8655_WINDOW 225

#define M660_WAVE  25
#define M660_WINDOW 90
/* the values below are all experience value */
#define CU8825_WAVE 144
#define CU8825_WINDOW 303
#define C8812E_WAVE 84
#define C8812E_WINDOW 200
/* the values below are all experience value */
#define CU8950_WAVE 113
#define CU8950_WINDOW 193
/* the values below are all experience value */
#define Y300_WAVE 127
#define Y300_WINDOW 300
#define G510_WAVE 176
#define G510_WINDOW 310
/* the values below are all experience value */
#define G510C_WAVE 137
#define G510C_WINDOW 290
#define H867G_WAVE 74
#define H867G_WINDOW 221
/* delete this part */

static uint16_t lsensor_adc_table_u8655[LSENSOR_MAX_LEVEL] = {
	30, 48, 72, 134, 245, 360, 500
};
static uint16_t lsensor_adc_table_u8815[LSENSOR_MAX_LEVEL] = {
	50, 125, 200, 134, 503, 650, 838
};
static uint16_t lsensor_adc_table_c8655[LSENSOR_MAX_LEVEL] = {
	36, 120, 200, 356, 430, 490, 593
};
static uint16_t lsensor_adc_table_m660[LSENSOR_MAX_LEVEL] = {
	33, 100, 150, 332, 400, 480, 553
};
/* the values below are all experience value */
static uint16_t lsensor_adc_table_c8812[LSENSOR_MAX_LEVEL] = {
	39, 52, 170, 389, 475, 561, 648
};
static uint16_t lsensor_adc_table_u8680[LSENSOR_MAX_LEVEL] = {
	10, 26, 210, 560, 860, 1200, 1500
};
/*u8730's value,provided by wenjuan*/
static uint16_t lsensor_adc_table_u8730[LSENSOR_MAX_LEVEL] = {
	25, 55, 210, 600, 860, 1000, 1300
};
static uint16_t lsensor_adc_table_u8667[LSENSOR_MAX_LEVEL] = {
	25, 55, 210, 600, 860, 1000, 1300
};
/* the values below are all experience value */
static uint16_t lsensor_adc_table_cu8825[LSENSOR_MAX_LEVEL] = {
	37, 64, 300, 794, 1200, 2079, 3000
};
static uint16_t lsensor_adc_table_c8812e[LSENSOR_MAX_LEVEL] = {
	37, 64, 300, 794, 1200, 2079, 3000
};
/* the values below are all experience value */
static uint16_t lsensor_adc_table_cu8950[LSENSOR_MAX_LEVEL] = {
	29, 56, 260, 625, 795, 965, 1300
};
/* the values below are all experience value */
static uint16_t lsensor_adc_table_g510[LSENSOR_MAX_LEVEL] = {
	12,	30,	210, 560, 860, 1200, 1500
};
/* the values below are all experience value */
static uint16_t lsensor_adc_table_g510c[LSENSOR_MAX_LEVEL] = {
	12,	30,	210, 560, 860, 1200, 1500
};
static uint16_t lsensor_adc_table_y300[LSENSOR_MAX_LEVEL] = {
	14,	32,	218, 576, 880, 1250, 1600
};
static uint16_t lsensor_adc_table_H867G[LSENSOR_MAX_LEVEL] = {
	14, 40, 400, 740, 950, 1355, 3000
};
#endif /* _LINUX_APS_9900_H */
