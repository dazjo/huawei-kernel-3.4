/* drivers/input/misc/aps-12d.h
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

#ifndef _LINUX_APS_12D_H
#define _LINUX_APS_12D_H
/*add 4 register's address for intersil*/
enum aps_12d_reg{
  APS_12D_REG_CMD1     =0x00,
  APS_12D_REG_CMD2     =0x01,
  APS_12D_DATA_LSB     =0x02,
  APS_12D_DATA_MSB     =0x03,
  APS_INT_LT_LSB       =0x04,
  APS_INT_LT_MSB       =0x05,
  APS_INT_HT_LSB       =0x06,
  APS_INT_HT_MSB       =0x07,
  APS_TEST             =0x08,
};
enum aps_12d_op_mode{
  APS_12D_POWER_DOWN           =0x00,
  APS_12D_ALS_ONCE              =0x20,
  APS_12D_IR_ONCE                =0x40,
  APS_12D_PROXIMITY_ONCE         =0x60,
  APS_12D_RESERVED                =0x80,
  APS_12D_ALS_CONTINUOUS          =0xA0,
  APS_12D_IR_CONTINUOUS            =0xC0,
  APS_12D_PROXIMITY_CONTINUOUS     =0xE0,
};
/*add intersil's current value*/
enum aps_12d_irdr_sel_intersil{
  APS_12D_IRDR_SEL_INTERSIL_12P5MA        =0x00,
  APS_12D_IRDR_SEL_INTERSIL_25MA          =0x01,
  APS_12D_IRDR_SEL_INTERSIL_50MA          =0x02,
  APS_12D_IRDR_SEL_INTERSIL_100MA         =0x03,
};
typedef enum everlight_intersil_flag{  
    INTERSIL       =0,
    EVERLIGHT      =1,
}EVE_INTER_F;
enum aps_12d_irdr_sel{
  APS_12D_IRDR_SEL_50MA       =0x00,
  APS_12D_IRDR_SEL_25MA       =0x01,
  APS_12D_IRDR_SEL_12P5MA     =0x02,
  APS_12D_IRDR_SEL_6P25MA     =0x03,
};

enum aps_12d_modulation_freq_sel{
  APS_12D_FREQ_SEL_DC         =0x00,
  APS_12D_FREQ_SEL_NA0        =0x01,
  APS_12D_FREQ_SEL_NA1        =0x02,
  APS_12D_FREQ_SEL_40P96KHZ   =0x03,
};
/*add intersil's freq value*/
enum aps_modulation_freq_intersil{
  APS_FREQ_INTERSIL_DC            =0x00,
  APS_FREQ_INTERSIL_360KHZ        =0x01,
};
/*add intersil's adc value*/
enum aps_resolution_intersil{
    APS_ADC_16          =0x00,
    APS_ADC_12          =0x01,
    APS_ADC_8           =0x02,
    APS_ADC_4           =0x03,
};
enum aps_intersil_scheme{
    APS_INTERSIL_SCHEME_OFF=0x00,
    APS_INTERSIL_SCHEME_ON =0x80,
};
enum aps_12d_resolution_sel{
  APS_12D_RES_SEL_12          =0x00,
};

enum aps_12d_range_sel{
  APS_12D_RANGE_SEL_ALS_1000    =0x00,
  APS_12D_RANGE_SEL_ALS_4000    =0x01,
  APS_12D_RANGE_SEL_ALS_16000   =0x02,
  APS_12D_RANGE_SEL_ALS_64000   =0x03,
};

#define APS_12D_TIMRER        (1000)           /*1s*/

#define ECS_IOCTL_APP_SET_DELAY	    _IOW(0xA1, 0x18, short)
#define ECS_IOCTL_APP_GET_DELAY       _IOR(0xA1, 0x30, short)
#define ECS_IOCTL_APP_SET_LFLAG		_IOW(0xA1, 0x1C, short)
#define ECS_IOCTL_APP_GET_LFLAG		_IOR(0xA1, 0x1B, short)
#define ECS_IOCTL_APP_SET_PFLAG		_IOW(0xA1, 0x1E, short)
#define ECS_IOCTL_APP_GET_PFLAG		_IOR(0xA1, 0x1D, short)
#define ECS_IOCTL_APP_GET_PDATA_VALVE	_IOR(0xA1, 0x32, short)
#define ECS_IOCTL_APP_GET_LDATA_VALVE	_IOR(0xA1, 0x33, short)
#define RANG_VALUE	64
#define VREG_GP4_NAME	"gp4"
#define VREG_GP4_VOLTAGE_VALUE	2700


#endif /* _LINUX_APS_12D_H */

