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
#define TP_ID0  127
#define TP_ID1  128


struct config_tp{
	uint32_t version; 
	u16 tp_model ;
	u16 ic_number;
	uint8_t *config;
};

/* Y300 and G510 Defined separately */
enum {
   COB_Y300_OFILM = 0,
   COB_Y300_ECW = 2,
   COB_Y300_TRULY = 8,
   COB_Y300_NO_MOUDLE = 10,
};

enum {
   COB_G510_OFILM = 0,
   COB_G510_TRULY = 8,
   COB_G510_JDC = 10,
};

uint32_t get_tp_version_config(int moudle_id, u16 ic_number, uint8_t ** temp_version);
uint8_t* get_tp_lockdown_config(void);
/*COB tp module id gpio init*/
int moudle_gpio_init(void);
/*get COB tp module id*/
int get_tp_id(void);
