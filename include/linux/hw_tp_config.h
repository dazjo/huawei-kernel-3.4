/*
 * Copyright (c) 2012 Huawei Device Company
 *
 * This file include Touch fw and config for solving touch performance.
 * 
 * fw be named as followed:
 *
 * config be named as followed:
 *
 */
#include <linux/types.h>
#include <linux/list.h>


#define SYN_CONFIG_SIZE 32 * 16
#define CURRENT_PR_VERSION  1191601
/*FW for 4.0 inches touch with independent button*/
#define CURRENT_PR_VERSION_BTN 1315366

#define TP_ID0  127
#define TP_ID1  128

#define IC_TYPE_2202 2202
#define IC_TYPE_3200 3200

struct syn_version_config
{
	uint32_t syn_firmware_version;
	uint32_t syn_moudel_version;
	u16 syn_ic_name;
};

/* Y300 and G510 Defined separately */

typedef enum 
{
   TP_COB_ID0 = 0x00, //ID0 low   , ID1  low
   TP_COB_ID2 = 0x02, //ID0 float , ID1  low
   TP_COB_ID8 = 0x08, //ID1 float , ID0  low
   TP_COB_IDA = 0x0A, //ID0 float , ID1  float
}hw_tp_id_index;



uint8_t *get_tp_version_config(int module_id,u16 ic_type);

uint8_t* get_tp_lockdown_config(void);

/*get COB tp module id*/
int get_tp_id(void);

