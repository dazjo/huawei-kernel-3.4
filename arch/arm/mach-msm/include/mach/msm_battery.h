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
 */


#ifndef __MSM_BATTERY_H__
#define __MSM_BATTERY_H__

#define AC_CHG     0x00000001
#define USB_CHG    0x00000002

struct msm_psy_batt_pdata {
	u32 voltage_max_design;
	u32 voltage_min_design;
	u32 voltage_fail_safe;
	u32 avail_chg_sources;
	u32 batt_technology;
	u32 (*calculate_capacity)(u32 voltage);
};

#ifdef CONFIG_HUAWEI_EVALUATE_POWER_CONSUMPTION 
typedef enum {   
   EVENT_LCD_STATE = 0, /*LCD*/
   EVENT_INS_CAMERA_STATE = 1, /*inside camera*/
   EVENT_OUTS_CAMERA_STATE = 2, /*outside camera*/
   EVENT_CAMERA_STATE = 3,     /*camera*/
   EVENT_WIFI_STATE = 4, /*WIFI status*/  
   EVENT_BT_STATE = 5,  /*blue tooth state*/      
   EVENT_FM_STATE = 6, /* FM */
   EVENT_CODEC_STATE = 7, /* AUDIO Speaker */  
   EVENT_CAMERA_FLASH_STATE = 8, /* Camera flash */  
 
   EVENT_KEYPAD_BACKLIGHT_STATE = 20,  /*KPD backlight*/
   EVENT_VIBRATOR_STATE,  /*vibrator state*/   
   EVENT_GSM850_GSM900_STATE,  /*GSM850\GSM900*/
   EVENT_GSM1800_GSM1900_STATE,  /*GSM1800\GSM1900*/
   EVENT_WCDMA_RF_STATE,  /*WCDMA*/
   EVENT_CDMA1X_RF_STATE,  /*WCDMA*/   
   EVENT_SPEAKER_STATE,  /*SPEAKER*/
   EVENT_CPU_STATE,/*CPU*/  
   EVENT_GPS_STATE,/*GPS*/      
   EVENT_HW_NONE = 0xFF
} device_current_consume_type;

#define DEVICE_POWER_STATE_OFF 0
#define DEVICE_POWER_STATE_ON 1
#define SPEAKER_ON_STATE  1
#define SPEAKER_OFF_STATE 0

/* notify modem sides to calculate consume */
int huawei_rpc_current_consuem_notify(device_current_consume_type device_event, __u32 device_state);
#endif
#endif
