/* arch/arm/mach-msm/rpc_server_handset.c
 *
 * Copyright (c) 2008-2010,2012 Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/switch.h>

#include <asm/mach-types.h>

#include <mach/msm_rpcrouter.h>
#include <mach/board.h>
#include <mach/rpc_server_handset.h>
#ifdef CONFIG_HUAWEI_KERNEL
#include <linux/rtc.h>
#endif
#ifdef CONFIG_HUAWEI_FEATURE_OEMINFO
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/wakelock.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <mach/msm_rpcrouter.h>
#include "smd_private.h"
#endif

/* power key detect solution for ANR */
#include <linux/sysrq.h>

#define DRIVER_NAME	"msm-handset"

#define HS_SERVER_PROG 0x30000062
#define HS_SERVER_VERS 0x00010001

#define HS_RPC_PROG 0x30000091

#define HS_PROCESS_CMD_PROC 0x02
#define HS_SUBSCRIBE_SRVC_PROC 0x03
#define HS_REPORT_EVNT_PROC    0x05
#define OEMINFO_FINISH_PROC   0x06
#define OEMINFO_READY_PROC    0x07
#define HS_EVENT_CB_PROC	1
#define HS_EVENT_DATA_VER	1

#define RPC_KEYPAD_NULL_PROC 0
#define RPC_KEYPAD_PASS_KEY_CODE_PROC 2
#define RPC_KEYPAD_SET_PWR_KEY_STATE_PROC 3

#define HS_PWR_K		0x6F	/* Power key */
#define HS_END_K		0x51	/* End key or Power key */
#define HS_STEREO_HEADSET_K	0x82
#define HS_HEADSET_SWITCH_K	0x84
#define HS_HEADSET_SWITCH_2_K	0xF0
#define HS_HEADSET_SWITCH_3_K	0xF1
#define HS_HEADSET_HEADPHONE_K	0xF6
#define HS_HEADSET_MICROPHONE_K 0xF7

#ifdef CONFIG_HUAWEI_KERNEL
#define HS_STEREO_HEADSET_NO_MIC_K  0xFB
#define HS_PREVIOUS_K   0xFC
#define HS_NEXT_K       0xFD
#endif

#define HS_REL_K		0xFF	/* key release */

#ifdef CONFIG_HUAWEI_FEATURE_OEMINFO
#define HS_OEMINFO_K    0xFE
#endif
#define SW_HEADPHONE_INSERT_W_MIC 1 /* HS with mic */

#define KEY(hs_key, input_key) ((hs_key << 24) | input_key)

enum mschine_type{
    HW_MACHINE_8X55 = 0,
    HW_MACHINE_7X2725A,
};
static int get_current_machine(void);
/* creates /sys/module/rpc_server_handset/parameters/oeminfo_rpc_debug_mask file */
static int oeminfo_rpc_debug_mask = 0;
module_param(oeminfo_rpc_debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);

#define OEMINFO_RPC_DEBUG(x...)		  \
	do {						  \
		if (oeminfo_rpc_debug_mask)	  \
			printk(KERN_ERR x);	  \
	} while (0)
static int get_current_machine()
{
    if( (machine_is_msm8255_u8800_pro())
		|| (machine_is_msm8255_u8860()) 
		|| (machine_is_msm8255_c8860()) 
		|| (machine_is_msm8255_u8860lp())
        || machine_is_msm8255_u8860_r()
		|| (machine_is_msm8255_u8860_92())            
		|| (machine_is_msm8255_u8860_51())
		|| (machine_is_msm8255_u8680()) 
	    || (machine_is_msm8255_u8730()))
    {
        OEMINFO_RPC_DEBUG("8x55 oeminfo. \n");
        return HW_MACHINE_8X55;
    }
    else
    {
        OEMINFO_RPC_DEBUG("27a25a oeminfo. \n");
        return HW_MACHINE_7X2725A;
    }
}

		
enum hs_event {
	HS_EVNT_EXT_PWR = 0,	/* External Power status        */
	HS_EVNT_HSD,		/* Headset Detection            */
	HS_EVNT_HSTD,		/* Headset Type Detection       */
	HS_EVNT_HSSD,		/* Headset Switch Detection     */
	HS_EVNT_KPD,
	HS_EVNT_FLIP,		/* Flip / Clamshell status (open/close) */
	HS_EVNT_CHARGER,	/* Battery is being charged or not */
	HS_EVNT_ENV,		/* Events from runtime environment like DEM */
	HS_EVNT_REM,		/* Events received from HS counterpart on a
				remote processor*/
	HS_EVNT_DIAG,		/* Diag Events  */
	HS_EVNT_LAST,		 /* Should always be the last event type */
	HS_EVNT_MAX		/* Force enum to be an 32-bit number */
};

enum hs_src_state {
	HS_SRC_STATE_UNKWN = 0,
	HS_SRC_STATE_LO,
	HS_SRC_STATE_HI,
};

struct hs_event_data {
	uint32_t	ver;		/* Version number */
	enum hs_event	event_type;     /* Event Type	*/
	enum hs_event	enum_disc;     /* discriminator */
	uint32_t	data_length;	/* length of the next field */
	enum hs_src_state	data;    /* Pointer to data */
	uint32_t	data_size;	/* Elements to be processed in data */
};

enum hs_return_value {
	HS_EKPDLOCKED     = -2,	/* Operation failed because keypad is locked */
	HS_ENOTSUPPORTED  = -1,	/* Functionality not supported */
	HS_FALSE          =  0, /* Inquired condition is not true */
	HS_FAILURE        =  0, /* Requested operation was not successful */
	HS_TRUE           =  1, /* Inquired condition is true */
	HS_SUCCESS        =  1, /* Requested operation was successful */
	HS_MAX_RETURN     =  0x7FFFFFFF/* Force enum to be a 32 bit number */
};

struct hs_key_data {
	uint32_t ver;        /* Version number to track sturcture changes */
	uint32_t code;       /* which key? */
	uint32_t parm;       /* key status. Up/down or pressed/released */
};

enum hs_subs_srvc {
	HS_SUBS_SEND_CMD = 0, /* Subscribe to send commands to HS */
	HS_SUBS_RCV_EVNT,     /* Subscribe to receive Events from HS */
	HS_SUBS_SRVC_MAX
};

enum hs_subs_req {
	HS_SUBS_REGISTER,    /* Subscribe   */
	HS_SUBS_CANCEL,      /* Unsubscribe */
	HS_SUB_STATUS_MAX
};

enum hs_event_class {
	HS_EVNT_CLASS_ALL = 0, /* All HS events */
	HS_EVNT_CLASS_LAST,    /* Should always be the last class type   */
	HS_EVNT_CLASS_MAX
};

enum hs_cmd_class {
	HS_CMD_CLASS_LCD = 0, /* Send LCD related commands              */
	HS_CMD_CLASS_KPD,     /* Send KPD related commands              */
	HS_CMD_CLASS_LAST,    /* Should always be the last class type   */
	HS_CMD_CLASS_MAX
};

/*
 * Receive events or send command
 */
union hs_subs_class {
	enum hs_event_class	evnt;
	enum hs_cmd_class	cmd;
};

struct hs_subs {
	uint32_t                ver;
	enum hs_subs_srvc	srvc;  /* commands or events */
	enum hs_subs_req	req;   /* subscribe or unsubscribe  */
	uint32_t		host_os;
	enum hs_subs_req	disc;  /* discriminator    */
	union hs_subs_class      id;
};

struct hs_event_cb_recv {
	uint32_t cb_id;
	uint32_t hs_key_data_ptr;
	struct hs_key_data key;
};
enum hs_ext_cmd_type {
	HS_EXT_CMD_KPD_SEND_KEY = 0, /* Send Key */
	HS_EXT_CMD_KPD_BKLT_CTRL, /* Keypad backlight intensity	*/
	HS_EXT_CMD_LCD_BKLT_CTRL, /* LCD Backlight intensity */
	HS_EXT_CMD_DIAG_KEYMAP, /* Emulating a Diag key sequence */
	HS_EXT_CMD_DIAG_LOCK, /* Device Lock/Unlock */
	HS_EXT_CMD_GET_EVNT_STATUS, /* Get the status for one of the drivers */
	HS_EXT_CMD_KPD_GET_KEYS_STATUS,/* Get a list of keys status */
	HS_EXT_CMD_KPD_SET_PWR_KEY_RST_THOLD, /* PWR Key HW Reset duration */
	HS_EXT_CMD_KPD_SET_PWR_KEY_THOLD, /* Set pwr key threshold duration */
	HS_EXT_CMD_LAST, /* Should always be the last command type */
	HS_EXT_CMD_MAX = 0x7FFFFFFF /* Force enum to be an 32-bit number */
};

struct hs_cmd_data_type {
	uint32_t hs_cmd_data_type_ptr; /* hs_cmd_data_type ptr length */
	uint32_t ver; /* version */
	enum hs_ext_cmd_type id; /* command id */
	uint32_t handle; /* handle returned from subscribe proc */
	enum hs_ext_cmd_type disc_id1; /* discriminator id */
	uint32_t input_ptr; /* input ptr length */
	uint32_t input_val; /* command specific data */
	uint32_t input_len; /* length of command input */
	enum hs_ext_cmd_type disc_id2; /* discriminator id */
	uint32_t output_len; /* length of output data */
	uint32_t delayed; /* execution context for modem
				true - caller context
				false - hs task context*/
};

static const uint32_t hs_key_map[] = {
	KEY(HS_PWR_K, KEY_POWER),
	KEY(HS_END_K, KEY_END),
	KEY(HS_STEREO_HEADSET_K, SW_HEADPHONE_INSERT_W_MIC),
	KEY(HS_HEADSET_HEADPHONE_K, SW_HEADPHONE_INSERT),
	KEY(HS_HEADSET_MICROPHONE_K, SW_MICROPHONE_INSERT),
	KEY(HS_HEADSET_SWITCH_K, KEY_MEDIA),
	KEY(HS_HEADSET_SWITCH_2_K, KEY_VOLUMEUP),
	KEY(HS_HEADSET_SWITCH_3_K, KEY_VOLUMEDOWN),
#ifdef CONFIG_HUAWEI_KERNEL
    KEY(HS_STEREO_HEADSET_NO_MIC_K, SW_HEADPHONE_INSERT),
    KEY(HS_PREVIOUS_K, KEY_PREVIOUSSONG),
    KEY(HS_NEXT_K,         KEY_NEXTSONG),
#endif
	0
};

enum {
	NO_DEVICE	= 0,
	MSM_HEADSET	= 1,
};
/* Add newer versions at the top of array */
static const unsigned int rpc_vers[] = {
	0x00030001,
	0x00020001,
	0x00010001,
};
/* hs subscription request parameters */
struct hs_subs_rpc_req {
	uint32_t hs_subs_ptr;
	struct hs_subs hs_subs;
	uint32_t hs_cb_id;
	uint32_t hs_handle_ptr;
	uint32_t hs_handle_data;
};

static struct hs_subs_rpc_req *hs_subs_req;

struct msm_handset {
	struct input_dev *ipdev;
	struct switch_dev sdev;
	struct msm_handset_platform_data *hs_pdata;
	bool mic_on, hs_on;
};

static struct msm_rpc_client *rpc_client;
static struct msm_handset *hs;
#ifdef CONFIG_HUAWEI_FEATURE_OEMINFO
static int handle_hs_rpc_call(struct msm_rpc_server *server,
			   struct rpc_request_hdr *req, unsigned len);

static struct msm_rpc_server hs_rpc_server = {
	.prog		= HS_SERVER_PROG,
	.vers		= HS_SERVER_VERS,
	.rpc_call	= handle_hs_rpc_call,
};

#define RMT_OEMINFO_OPEN              0
#define RMT_OEMINFO_WRITE             1
#define RMT_OEMINFO_CLOSE             2
#define RMT_OEMINFO_REGISTER_CB       3

#define RMT_OEMINFO_MAX_IOVEC_XFR_CNT 5
#define MAX_NUM_CLIENTS 10

enum {
	RMT_OEMINFO_NO_ERROR = 0,	/* Success */
	RMT_OEMINFO_ERROR_PARAM,	/* Invalid parameters */
	RMT_OEMINFO_ERROR_PIPE,		/* RPC pipe failure */
	RMT_OEMINFO_ERROR_UNINIT,	/* Server is not initalized */
	RMT_OEMINFO_ERROR_BUSY,		/* Device busy */
	RMT_OEMINFO_ERROR_DEVICE	/* Remote oeminfo device */
} rmt_oeminfo_status;

struct rmt_oeminfo_iovec_desc {
	uint32_t sector_addr;
	uint32_t data_phy_addr;
	uint32_t num_sector;
};

struct rmt_oeminfo_cb {
	uint32_t cb_id;
	uint32_t err_code;
	uint32_t data;
	uint32_t handle;
};

struct rmt_buffer_param {
	uint32_t start;
	uint32_t size;
};

#define OEMINFO_BUFFER_SIZE  (64 * 1024)

struct oeminfo_type
{
  int                            ver;
  int                            func_type;      /* oeminfo_func_type */
  int                            oeminfo_type;   /* oeminfo_info_type_enum_type */
  int                            total_size;
  int                            return_status;
  char                           buffer[OEMINFO_BUFFER_SIZE];
};

#define RMT_OEMINFO_SERVER_IOCTL_MAGIC (0xC2)

#define RMT_OEMINFO_EVENT_FUNC_PTR_TYPE_PROC 1

#define RMT_OEMINFO_WAIT_FOR_REQ 0x5555
#define RMT_OEMINFO_SEND_STATUS  0x6666
/*
#define RMT_OEMINFO_WAIT_FOR_REQ \
	_IOR(RMT_OEMINFO_SERVER_IOCTL_MAGIC, 1, struct oeminfo_type)

#define RMT_OEMINFO_SEND_STATUS \
	_IOW(RMT_OEMINFO_SERVER_IOCTL_MAGIC, 2, struct rmt_oeminfo_cb)
*/

struct rmt_oeminfo_server_info {
	unsigned long cids;
	struct rmt_buffer_param rmt_shrd_mem;
	int open_excl;
	atomic_t total_events;
	wait_queue_head_t event_q;
	struct list_head event_list;
	struct list_head data_list;
	/* Lock to protect event list and client info list */
	spinlock_t lock;
	/* Wakelock to be acquired when processing requests from modem */
	struct wake_lock wlock;
	atomic_t wcount;
};

struct rmt_oeminfo_kevent {
	struct list_head      list;
	struct oeminfo_type   * event;
};

struct rmt_oeminfo_kdata {
	struct list_head list;
    uint32_t handle;
	struct msm_rpc_client_info cinfo;
	struct oeminfo_type  * data;
};

static int rmt_oeminfo_server_probe(struct platform_device *pdev);
static int rmt_oeminfo_open(struct inode *ip, struct file *fp);
static long rmt_oeminfo_ioctl(struct file *fp, unsigned int cmd,
			    unsigned long arg);
static int rmt_oeminfo_release(struct inode *ip, struct file *fp);

static struct platform_driver rmt_oeminfo_driver = {
	.probe	= rmt_oeminfo_server_probe,
	.driver	= {
		.name	= "rmt_oeminfo",
		.owner	= THIS_MODULE,
	},
};

const struct file_operations rmt_oeminfo_fops = {
	.owner = THIS_MODULE,
	.open = rmt_oeminfo_open,
	.unlocked_ioctl	 = rmt_oeminfo_ioctl,
	.release = rmt_oeminfo_release,
};

static struct miscdevice rmt_oeminfo_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "rmt_oeminfo",
	.fops = &rmt_oeminfo_fops,
};

static struct rmt_oeminfo_server_info *_rms;

static int rmt_oeminfo_server_probe(struct platform_device *pdev)
{
	struct rmt_oeminfo_server_info *rms;
	struct resource *res;
	int ret;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pr_err("%s: No resources for rmt_oeminfo server\n", __func__);
		return -ENODEV;
	}

	rms = kzalloc(sizeof(struct rmt_oeminfo_server_info), GFP_KERNEL);
	if (!rms) {
		pr_err("%s: Unable to allocate memory\n", __func__);
		return -ENOMEM;
	}

	rms->rmt_shrd_mem.start = res->start;
	rms->rmt_shrd_mem.size = resource_size(res);
	init_waitqueue_head(&rms->event_q);
	spin_lock_init(&rms->lock);
	atomic_set(&rms->total_events, 0);
	INIT_LIST_HEAD(&rms->event_list);
	INIT_LIST_HEAD(&rms->data_list);
	/* The client expects a non-zero return value for
	 * its open requests. Hence reserve 0 bit.  */
	__set_bit(0, &rms->cids);
	atomic_set(&rms->wcount, 0);
	wake_lock_init(&rms->wlock, WAKE_LOCK_SUSPEND, "rmt_oeminfo");

	ret = misc_register(&rmt_oeminfo_device);
	if (ret) {
		pr_err("%s: Unable to register misc device %d\n", __func__,
				MISC_DYNAMIC_MINOR);
		wake_lock_destroy(&rms->wlock);
		kfree(rms);
		return ret;
	}

	OEMINFO_RPC_DEBUG("%s: Remote oeminfo RPC server initialized\n", __func__);
	_rms = rms;
	return 0;
}

static void oeminfo_put_event(struct rmt_oeminfo_server_info *rms,
			struct rmt_oeminfo_kevent *kevent)
{
	spin_lock(&rms->lock);
	list_add_tail(&kevent->list, &rms->event_list);
	spin_unlock(&rms->lock);
}

static struct rmt_oeminfo_kevent *oeminfo_get_event(struct rmt_oeminfo_server_info *rms)
{
	struct rmt_oeminfo_kevent *kevent = NULL;

	spin_lock(&rms->lock);
	if (!list_empty(&rms->event_list)) {
		kevent = list_first_entry(&rms->event_list,
			struct rmt_oeminfo_kevent, list);
		list_del(&kevent->list);
	}
	spin_unlock(&rms->lock);
	return kevent;
}


static void oeminfo_put_data(struct rmt_oeminfo_server_info *rms,
			struct rmt_oeminfo_kdata *kdata)
{
	spin_lock(&rms->lock);
	list_add_tail(&kdata->list, &rms->data_list);
	spin_unlock(&rms->lock);
}

static struct rmt_oeminfo_kdata *oeminfo_get_data(struct rmt_oeminfo_server_info *rms)
{
	struct rmt_oeminfo_kdata *kdata = NULL;

	spin_lock(&rms->lock);
	if (!list_empty(&rms->data_list)) {
		kdata = list_first_entry(&rms->data_list,
			struct rmt_oeminfo_kdata, list);
		list_del(&kdata->list);
	}
	spin_unlock(&rms->lock);
	return kdata;
}

static void print_oeminfo_data(void * data)
{
  char all_log_string[512 + 4];
  char sub_string[12];
  int * my_int_array = NULL;
  int  ii;
  
  memset(all_log_string,0,sizeof(all_log_string));
  memset(sub_string,0,sizeof(sub_string));
  
  my_int_array = (int *)data;
  
  for(ii = 0 ; ii < 40 ; ii++)
  {
    snprintf(sub_string, sizeof(sub_string), "%08X " , my_int_array[ii]);
  
    strncat(all_log_string,sub_string,sizeof(sub_string));
  }

  OEMINFO_RPC_DEBUG("%s\n",all_log_string);
}


static int rmt_oeminfo_handle_key(uint32_t key_parm)
{
  uint32_t rpc_status = RPC_ACCEPTSTAT_SUCCESS;
  struct rmt_oeminfo_server_info *rms = _rms;
  struct rmt_oeminfo_kevent *kevent;
  struct rmt_oeminfo_kdata  *kdata;
  struct oeminfo_type * share_ptr = NULL;

  OEMINFO_RPC_DEBUG("emmc_oeminfo: %s(),enter. key_parm is 0x%x. \n", 
			 __func__, key_parm);
  
  kevent = kmalloc(sizeof(struct rmt_oeminfo_kevent), GFP_KERNEL);
  if (!kevent) {
  	rpc_status = RPC_ACCEPTSTAT_SYSTEM_ERR;
    pr_err("emmc_oeminfo: %s(). malloc kevent fail. \n",__func__);
  	return 0;
  }
  else
  {
	OEMINFO_RPC_DEBUG("emmc_oeminfo: %s(). malloc kevent OK. \n",__func__);
  }
  
  kdata = kmalloc(sizeof(struct rmt_oeminfo_kdata), GFP_KERNEL);
  if (!kdata)
  {
  	rpc_status = RPC_ACCEPTSTAT_SYSTEM_ERR;
    pr_err("emmc_oeminfo: %s(). malloc kdata fail. \n",__func__);
  	return 0;
  }
  else
  {
	OEMINFO_RPC_DEBUG("emmc_oeminfo: %s(). malloc kdata OK. \n",__func__);
  }

  // share_ptr = (void *)be32_to_cpu(key_parm);
  share_ptr = smem_alloc(SMEM_LCD_CUR_PANEL, sizeof(struct oeminfo_type));
  OEMINFO_RPC_DEBUG("emmc_oeminfo: share_ptr is 0x%x. \n",(int)share_ptr);
  
  print_oeminfo_data(share_ptr);
  
  // memcpy(&kevent->event, share_ptr , sizeof(struct oeminfo_type));
  kevent->event = share_ptr;

  msm_rpc_server_get_requesting_client(&kdata->cinfo);
  kdata->data = share_ptr;

  OEMINFO_RPC_DEBUG("emmc_oeminfo: put event ok!\n");
  
  oeminfo_put_event(rms, kevent);
  oeminfo_put_data(rms, kdata);
  atomic_inc(&rms->total_events);
  wake_up(&rms->event_q);

  OEMINFO_RPC_DEBUG("emmc_oeminfo: %s(), end. \n",__func__);

  return 1;
}


static long rmt_oeminfo_ioctl(struct file *fp, unsigned int cmd,
			    unsigned long arg)
{
	int ret = 0;
	int rc = -1;
	/* notify modem to run tmc_huawei_init only once, on boot */
	static bool firstboot = true;
	struct rmt_oeminfo_server_info *rms = _rms;
	struct rmt_oeminfo_kevent *kevent;
    struct rmt_oeminfo_kdata *kdata;
    struct oeminfo_type * share_ptr = NULL;

	OEMINFO_RPC_DEBUG("emmc_oeminfo: %s: wait for request ioctl\n", __func__);
	
	switch (cmd) {

	case RMT_OEMINFO_WAIT_FOR_REQ:
		if (HW_MACHINE_7X2725A == get_current_machine())
		{
			if (firstboot == true)
			{
				firstboot = false;
				/* use rpc to set sig TMC_KERNEL_READY_SIG */
				rc = msm_rpc_client_req(rpc_client, OEMINFO_READY_PROC,
					NULL, NULL,
					NULL, NULL, -1);
				if (rc)
				{
					firstboot = true;
					pr_err("%s: couldn't send rpc client request OEMINFO_READY_PROC\n", __func__);
				}
			}
		}
		OEMINFO_RPC_DEBUG("emmc_oeminfo: %s: wait for request ioctl\n", __func__);
		if (atomic_read(&rms->total_events) == 0) {
			ret = wait_event_interruptible(rms->event_q,
				atomic_read(&rms->total_events) != 0);
		}
		if (ret < 0)
			break;
		atomic_dec(&rms->total_events);

		kevent = oeminfo_get_event(rms);
		OEMINFO_RPC_DEBUG("emmc_oeminfo: call copy_to_user().\n");
        
		WARN_ON(kevent == NULL);
		if (copy_to_user((void __user *)arg, kevent->event,sizeof(struct oeminfo_type)))
        {
			pr_err("emmc_oeminfo: %s: copy to user failed\n\n", __func__);
			ret = -EFAULT;
		}
		kfree(kevent);
		break;

	case RMT_OEMINFO_SEND_STATUS:
		OEMINFO_RPC_DEBUG("%s: send callback ioctl\n", __func__);
        kdata = oeminfo_get_data(rms);
        share_ptr = smem_alloc(SMEM_LCD_CUR_PANEL, sizeof(struct oeminfo_type));
        
		if (copy_from_user(kdata->data, (void __user *)arg,
				sizeof(struct oeminfo_type))) 
		{
			pr_err("%s: copy from user failed\n\n", __func__);
			ret = -EFAULT;
			if (atomic_dec_return(&rms->wcount) == 0)
				wake_unlock(&rms->wlock);
			break;
		}

		if (HW_MACHINE_7X2725A == get_current_machine())
		{
			rc = msm_rpc_client_req(rpc_client, OEMINFO_FINISH_PROC,
						NULL, NULL,
						NULL, NULL, -1);
            if (rc)
			    pr_err("%s: couldn't send rpc client request\n", __func__);
		}
		OEMINFO_RPC_DEBUG("%s:kernel memory data: \n", __func__);
        print_oeminfo_data(kdata->data);

		OEMINFO_RPC_DEBUG("%s:share memory data: \n", __func__);
        print_oeminfo_data(share_ptr);

#if 0 
		OEMINFO_RPC_DEBUG("%s: call msm_rpc_server_cb_req().\n", __func__);
		ret = msm_rpc_server_cb_req(&hs_rpc_server, &kdata->cinfo,
			RMT_OEMINFO_EVENT_FUNC_PTR_TYPE_PROC, NULL, NULL,NULL, NULL, -1);
        
		if (ret < 0)
			pr_err("%s: send callback failed with ret val = %d\n",
				__func__, ret);
		if (atomic_dec_return(&rms->wcount) == 0)
			wake_unlock(&rms->wlock);
#endif

		kfree(kdata);
		break;

	default:
		ret = -EINVAL;
		break;
	}

	OEMINFO_RPC_DEBUG("emmc_oeminfo: %s(),end.\n",__func__);

	return ret;
}

static int rmt_oeminfo_open(struct inode *ip, struct file *fp)
{
  int ret = 0;
  
  spin_lock(&_rms->lock);
  
  if (!_rms->open_excl)
  	_rms->open_excl = 1;
  else
  	ret = -EBUSY;
  
  spin_unlock(&_rms->lock);
  return ret;
}

static int rmt_oeminfo_release(struct inode *ip, struct file *fp)
{
  spin_lock(&_rms->lock);
  _rms->open_excl = 0;
  spin_unlock(&_rms->lock);
  
  return 0;
}
#endif

#ifdef CONFIG_HUAWEI_KERNEL
static struct wake_lock headset_unplug_wake_lock;
#define HEADSET_WAKE_DURING 2
#endif

static int hs_find_key(uint32_t hscode)
{
	int i, key;

	key = KEY(hscode, 0);

	for (i = 0; hs_key_map[i] != 0; i++) {
		if ((hs_key_map[i] & 0xff000000) == key)
			return hs_key_map[i] & 0x00ffffff;
	}
	return -1;
}

static void update_state(void)
{
	int state;

	if (hs->mic_on && hs->hs_on)
		state = 1 << 0;
	else if (hs->hs_on)
		state = 1 << 1;
	else if (hs->mic_on)
		state = 1 << 2;
	else
		state = 0;

	switch_set_state(&hs->sdev, state);
}

/* power key detect solution for ANR */
#ifdef CONFIG_HUAWEI_FEATURE_POWER_KEY
#define POWER_KEY_TIMEOUT 5
struct timer_list power_key_detect_timer;
static int g_power_key_detect=0;/*disable power key detect solution*/
static int mod_timer_flags=0;/*avoid timer pending*/

static ssize_t
show_power_key_detect(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", g_power_key_detect);
}

static ssize_t
set_power_key_detect(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)
{

	unsigned long val;
	int error;
	
	error = strict_strtoul(buf, 10, &val);
	if (error)
		return error;
	
	g_power_key_detect=val;
	return count;
}

static DEVICE_ATTR(power_key_detect, 0644, show_power_key_detect, set_power_key_detect);


static void exception_power_key_timeout(unsigned long data)
{	
	__handle_sysrq('w', false);
	BUG_ON(1);
}

void del_power_key_timer(void)
{	
	/*if timer add,del timer*/
	if(mod_timer_flags)
	{
		del_timer(&power_key_detect_timer);
		mod_timer_flags=0;
	}
}
EXPORT_SYMBOL(del_power_key_timer);

static void init_power_key_dump(void)
{
	init_timer(&power_key_detect_timer);
	power_key_detect_timer.function = exception_power_key_timeout;
	power_key_detect_timer.data = 0;
}

static int bootmode=0;
int __init get_bootmode(char *s)
{
	if (!strcmp(s, "recovery"))
		bootmode = 1;
	return 0;
}
__setup("androidboot.mode=", get_bootmode);

static void power_key_dump(void)
{
	/*bootmode isn't recovery mode and power detect enable ,mod_timer_flag avoid timer pending*/
   	if(bootmode !=1 && g_power_key_detect==1 && mod_timer_flags == 0)		
   	{
		mod_timer_flags=1;
        mod_timer(&power_key_detect_timer,jiffies + HZ*POWER_KEY_TIMEOUT);
   	}
	else
		return;
}
#else
void del_power_key_timer(void){}
EXPORT_SYMBOL(del_power_key_timer);
static void power_key_dump(void){}
#endif

/*
 * tuple format: (key_code, key_param)
 *
 * old-architecture:
 * key-press = (key_code, 0)
 * key-release = (0xff, key_code)
 *
 * new-architecutre:
 * key-press = (key_code, 0)
 * key-release = (key_code, 0xff)
 */
static void report_hs_key(uint32_t key_code, uint32_t key_parm)
{
	int key, temp_key_code;
/* add it for get UTC */
#ifdef CONFIG_HUAWEI_KERNEL 
    struct timespec ts;  
    struct rtc_time tm;
#endif
	if (key_code == HS_REL_K)
		key = hs_find_key(key_parm);
	else
		key = hs_find_key(key_code);

	temp_key_code = key_code;

	if (key_parm == HS_REL_K)
		key_code = key_parm;

	switch (key) {
	case KEY_POWER:
		/* power key detect solution for ANR */
		del_power_key_timer();

		break;
	case KEY_END:
   /* add log,print key code and UTC */
    #ifdef CONFIG_HUAWEI_KERNEL
        getnstimeofday(&ts);  
        rtc_time_to_tm(ts.tv_sec, &tm);
        printk("%s:Press power key ,key=%d ,now time (%02d:%02d:%02d.%09lu UTC)\n",__func__,key_code,tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);
    #endif
   	#ifdef CONFIG_HUAWEI_KERNEL
		input_report_key(hs->ipdev, KEY_POWER, (key_code != HS_REL_K));
		/* power key detect solution for ANR */
		if(key_code == HS_REL_K)
			power_key_dump();

   	#else
   		input_report_key(hs->ipdev, key, (key_code != HS_REL_K));
   	#endif
		break;
	case KEY_MEDIA:
#ifdef CONFIG_HUAWEI_KERNEL
        /* add 2s wake lock here to fix issue that time of press headset key  
         * is not enough when AP seelp during incall */
        wake_lock_timeout(&headset_unplug_wake_lock, HEADSET_WAKE_DURING*HZ);
#endif
	case KEY_VOLUMEUP:
	case KEY_VOLUMEDOWN:
#ifdef CONFIG_HUAWEI_KERNEL
        printk(KERN_ERR "%s: kernel recieve modem linectl hdset key: KEY_MEDIA\n",__func__);
#endif
		input_report_key(hs->ipdev, key, (key_code != HS_REL_K));
		break;
	case SW_HEADPHONE_INSERT_W_MIC:
#ifdef CONFIG_HUAWEI_KERNEL
        printk(KERN_ERR "%s: SW_HEADPHONE_INSERT: key_code = %d\n",__func__, key_code);

		//delete
            /* add 2s wake lock here to fix issue that time of swtiching audio-output 
             * is not enough when headset unpluging during incall */
            wake_lock_timeout(&headset_unplug_wake_lock, HEADSET_WAKE_DURING*HZ);
		//delete
#endif
		hs->mic_on = hs->hs_on = (key_code != HS_REL_K) ? 1 : 0;
		input_report_switch(hs->ipdev, SW_HEADPHONE_INSERT,
							hs->hs_on);
		input_report_switch(hs->ipdev, SW_MICROPHONE_INSERT,
							hs->mic_on);
		update_state();
		break;
#ifdef CONFIG_HUAWEI_KERNEL
    case KEY_PREVIOUSSONG:
        {
             printk(KERN_ERR "%s: kernel recieve modem linectl hdset key: KEY_PREVIOUSSONG\n",__func__);
            input_report_key(hs->ipdev, key, (key_code != HS_REL_K));
            break;
        }
    case KEY_NEXTSONG:
        {
             printk(KERN_ERR "%s: kernel recieve modem linectl hdset key: KEY_NEXTSONG\n",__func__);
            input_report_key(hs->ipdev, key, (key_code != HS_REL_K));
            break;
        }
#endif

	case SW_HEADPHONE_INSERT:
		hs->hs_on = (key_code != HS_REL_K) ? 1 : 0;
		input_report_switch(hs->ipdev, key, hs->hs_on);
		update_state();
		break;
	case SW_MICROPHONE_INSERT:
		hs->mic_on = (key_code != HS_REL_K) ? 1 : 0;
		input_report_switch(hs->ipdev, key, hs->mic_on);
		update_state();
		break;
	case -1:
		printk(KERN_ERR "%s: No mapping for remote handset event %d\n",
				 __func__, temp_key_code);
		return;
	}
	input_sync(hs->ipdev);
}

static int handle_hs_rpc_call(struct msm_rpc_server *server,
			   struct rpc_request_hdr *req, unsigned len)
{
	struct rpc_keypad_pass_key_code_args {
		uint32_t key_code;
		uint32_t key_parm;
	};

	switch (req->procedure) {
	case RPC_KEYPAD_NULL_PROC:
		return 0;

	case RPC_KEYPAD_PASS_KEY_CODE_PROC: {
		struct rpc_keypad_pass_key_code_args *args;

		args = (struct rpc_keypad_pass_key_code_args *)(req + 1);
		args->key_code = be32_to_cpu(args->key_code);
		args->key_parm = be32_to_cpu(args->key_parm);
        
		#ifdef CONFIG_HUAWEI_FEATURE_OEMINFO
		OEMINFO_RPC_DEBUG("emmc_oeminfo: ori key_code is %d, ori key_parm is %d.\n", 
				args->key_code,args->key_parm);
		OEMINFO_RPC_DEBUG("emmc_oeminfo: key_code is %d, key_parm is %d.\n", 
				args->key_code,args->key_parm);
		#endif

		report_hs_key(args->key_code, args->key_parm);

		return 0;
	}

	case RPC_KEYPAD_SET_PWR_KEY_STATE_PROC:
		/* This RPC function must be available for the ARM9
		 * to function properly.  This function is redundant
		 * when RPC_KEYPAD_PASS_KEY_CODE_PROC is handled. So
		 * input_report_key is not needed.
		 */
		return 0;
	default:
		return -ENODEV;
	}
}

#ifndef CONFIG_HUAWEI_FEATURE_OEMINFO
static struct msm_rpc_server hs_rpc_server = {
	.prog		= HS_SERVER_PROG,
	.vers		= HS_SERVER_VERS,
	.rpc_call	= handle_hs_rpc_call,
};
#endif

static int process_subs_srvc_callback(struct hs_event_cb_recv *recv)
{
	#ifdef CONFIG_HUAWEI_FEATURE_OEMINFO
    int recv_key;

	OEMINFO_RPC_DEBUG("emmc_oeminfo: %s(),enter.\n", __func__);
	#endif
    
	if (!recv)
		return -ENODATA;

	#ifdef CONFIG_HUAWEI_FEATURE_OEMINFO
    recv_key = be32_to_cpu(recv->key.code);
    
    if(HS_OEMINFO_K == recv_key)
    {
	  OEMINFO_RPC_DEBUG("emmc_oeminfo: oeminfo key transfered.\n");
      rmt_oeminfo_handle_key(be32_to_cpu(recv->key.ver));
      return 0;
    }
	#endif
    
	report_hs_key(be32_to_cpu(recv->key.code), be32_to_cpu(recv->key.parm));

	return 0;
}

static void process_hs_rpc_request(uint32_t proc, void *data)
{
	if (proc == HS_EVENT_CB_PROC)
		process_subs_srvc_callback(data);
	else
		pr_err("%s: unknown rpc proc %d\n", __func__, proc);
}

static int hs_rpc_report_event_arg(struct msm_rpc_client *client,
					void *buffer, void *data)
{
	struct hs_event_rpc_req {
		uint32_t hs_event_data_ptr;
		struct hs_event_data data;
	};

	struct hs_event_rpc_req *req = buffer;

	req->hs_event_data_ptr	= cpu_to_be32(0x1);
	req->data.ver		= cpu_to_be32(HS_EVENT_DATA_VER);
	req->data.event_type	= cpu_to_be32(HS_EVNT_HSD);
	req->data.enum_disc	= cpu_to_be32(HS_EVNT_HSD);
	req->data.data_length	= cpu_to_be32(0x1);
	req->data.data		= cpu_to_be32(*(enum hs_src_state *)data);
	req->data.data_size	= cpu_to_be32(sizeof(enum hs_src_state));

	return sizeof(*req);
}

static int hs_rpc_report_event_res(struct msm_rpc_client *client,
					void *buffer, void *data)
{
	enum hs_return_value result;

	result = be32_to_cpu(*(enum hs_return_value *)buffer);
	pr_debug("%s: request completed: 0x%x\n", __func__, result);

	if (result == HS_SUCCESS)
		return 0;

	return 1;
}

void report_headset_status(bool connected)
{
	int rc = -1;
	enum hs_src_state status;

	if (connected == true)
		status = HS_SRC_STATE_HI;
	else
		status = HS_SRC_STATE_LO;

	rc = msm_rpc_client_req(rpc_client, HS_REPORT_EVNT_PROC,
				hs_rpc_report_event_arg, &status,
				hs_rpc_report_event_res, NULL, -1);

	if (rc)
		pr_err("%s: couldn't send rpc client request\n", __func__);
}
EXPORT_SYMBOL(report_headset_status);

static int hs_rpc_pwr_cmd_arg(struct msm_rpc_client *client,
				    void *buffer, void *data)
{
	struct hs_cmd_data_type *hs_pwr_cmd = buffer;

	hs_pwr_cmd->hs_cmd_data_type_ptr = cpu_to_be32(0x01);

	hs_pwr_cmd->ver = cpu_to_be32(0x03);
	hs_pwr_cmd->id = cpu_to_be32(HS_EXT_CMD_KPD_SET_PWR_KEY_THOLD);
	hs_pwr_cmd->handle = cpu_to_be32(hs_subs_req->hs_handle_data);
	hs_pwr_cmd->disc_id1 = cpu_to_be32(HS_EXT_CMD_KPD_SET_PWR_KEY_THOLD);
	hs_pwr_cmd->input_ptr = cpu_to_be32(0x01);
	hs_pwr_cmd->input_val = cpu_to_be32(hs->hs_pdata->pwr_key_delay_ms);
	hs_pwr_cmd->input_len = cpu_to_be32(0x01);
	hs_pwr_cmd->disc_id2 = cpu_to_be32(HS_EXT_CMD_KPD_SET_PWR_KEY_THOLD);
	hs_pwr_cmd->output_len = cpu_to_be32(0x00);
	hs_pwr_cmd->delayed = cpu_to_be32(0x00);

	return sizeof(*hs_pwr_cmd);
}

static int hs_rpc_pwr_cmd_res(struct msm_rpc_client *client,
				    void *buffer, void *data)
{
	uint32_t result;

	result = be32_to_cpu(*((uint32_t *)buffer));
	pr_debug("%s: request completed: 0x%x\n", __func__, result);

	return 0;
}

static int hs_rpc_register_subs_arg(struct msm_rpc_client *client,
				    void *buffer, void *data)
{
	hs_subs_req = buffer;

	hs_subs_req->hs_subs_ptr	= cpu_to_be32(0x1);
	hs_subs_req->hs_subs.ver	= cpu_to_be32(0x1);
	hs_subs_req->hs_subs.srvc	= cpu_to_be32(HS_SUBS_RCV_EVNT);
	hs_subs_req->hs_subs.req	= cpu_to_be32(HS_SUBS_REGISTER);
	hs_subs_req->hs_subs.host_os	= cpu_to_be32(0x4); /* linux */
	hs_subs_req->hs_subs.disc	= cpu_to_be32(HS_SUBS_RCV_EVNT);
	hs_subs_req->hs_subs.id.evnt	= cpu_to_be32(HS_EVNT_CLASS_ALL);

	hs_subs_req->hs_cb_id		= cpu_to_be32(0x1);

	hs_subs_req->hs_handle_ptr	= cpu_to_be32(0x1);
	hs_subs_req->hs_handle_data	= cpu_to_be32(0x0);

	return sizeof(*hs_subs_req);
}

static int hs_rpc_register_subs_res(struct msm_rpc_client *client,
				    void *buffer, void *data)
{
	uint32_t result;

	result = be32_to_cpu(*((uint32_t *)buffer));
	pr_debug("%s: request completed: 0x%x\n", __func__, result);

	return 0;
}

static int hs_cb_func(struct msm_rpc_client *client, void *buffer, int in_size)
{
	int rc = -1;

	struct rpc_request_hdr *hdr = buffer;

	hdr->type = be32_to_cpu(hdr->type);
	hdr->xid = be32_to_cpu(hdr->xid);
	hdr->rpc_vers = be32_to_cpu(hdr->rpc_vers);
	hdr->prog = be32_to_cpu(hdr->prog);
	hdr->vers = be32_to_cpu(hdr->vers);
	hdr->procedure = be32_to_cpu(hdr->procedure);

	process_hs_rpc_request(hdr->procedure,
			    (void *) (hdr + 1));

	msm_rpc_start_accepted_reply(client, hdr->xid,
				     RPC_ACCEPTSTAT_SUCCESS);
	rc = msm_rpc_send_accepted_reply(client, 0);
	if (rc) {
		pr_err("%s: sending reply failed: %d\n", __func__, rc);
		return rc;
	}

	return 0;
}

static int __devinit hs_rpc_cb_init(void)
{
	int rc = 0, i, num_vers;

	num_vers = ARRAY_SIZE(rpc_vers);

	for (i = 0; i < num_vers; i++) {
		rpc_client = msm_rpc_register_client("hs",
			HS_RPC_PROG, rpc_vers[i], 0, hs_cb_func);

		if (IS_ERR(rpc_client))
			pr_debug("%s: RPC Client version %d failed, fallback\n",
				 __func__, rpc_vers[i]);
		else
			break;
	}

	if (IS_ERR(rpc_client)) {
		pr_err("%s: Incompatible RPC version error %ld\n",
			 __func__, PTR_ERR(rpc_client));
		return PTR_ERR(rpc_client);
	}

	rc = msm_rpc_client_req(rpc_client, HS_SUBSCRIBE_SRVC_PROC,
				hs_rpc_register_subs_arg, NULL,
				hs_rpc_register_subs_res, NULL, -1);
	if (rc) {
		pr_err("%s: RPC client request failed for subscribe services\n",
						__func__);
		goto err_client_req;
	}

	rc = msm_rpc_client_req(rpc_client, HS_PROCESS_CMD_PROC,
			hs_rpc_pwr_cmd_arg, NULL,
			hs_rpc_pwr_cmd_res, NULL, -1);
	if (rc)
		pr_err("%s: RPC client request failed for pwr key"
			" delay cmd, using normal mode\n", __func__);
	return 0;
err_client_req:
	msm_rpc_unregister_client(rpc_client);
	return rc;
}

static int __devinit hs_rpc_init(void)
{
	int rc;

#ifdef CONFIG_HUAWEI_KERNEL
    wake_lock_init(&headset_unplug_wake_lock, WAKE_LOCK_SUSPEND, "headset_unplug_wake_lock");
#endif
	rc = hs_rpc_cb_init();
	if (rc) {
		pr_err("%s: failed to initialize rpc client, try server...\n",
						__func__);

		rc = msm_rpc_create_server(&hs_rpc_server);
		if (rc) {
			pr_err("%s: failed to create rpc server\n", __func__);
			return rc;
		}
	}

	return rc;
}

static void __devexit hs_rpc_deinit(void)
{
    #ifdef CONFIG_HUAWEI_KERNEL
    wake_lock_destroy(&headset_unplug_wake_lock);
    #endif
	if (rpc_client)
		msm_rpc_unregister_client(rpc_client);
}

static ssize_t msm_headset_print_name(struct switch_dev *sdev, char *buf)
{
	switch (switch_get_state(&hs->sdev)) {
	case NO_DEVICE:
		return sprintf(buf, "No Device\n");
	case MSM_HEADSET:
		return sprintf(buf, "Headset\n");
	}
	return -EINVAL;
}

static int __devinit hs_probe(struct platform_device *pdev)
{
	int rc = 0;
	struct input_dev *ipdev;

	hs = kzalloc(sizeof(struct msm_handset), GFP_KERNEL);
	if (!hs)
		return -ENOMEM;

	hs->sdev.name	= "h2w";
	hs->sdev.print_name = msm_headset_print_name;

	rc = switch_dev_register(&hs->sdev);
	if (rc)
		goto err_switch_dev_register;

	ipdev = input_allocate_device();
	if (!ipdev) {
		rc = -ENOMEM;
		goto err_alloc_input_dev;
	}
	input_set_drvdata(ipdev, hs);

	hs->ipdev = ipdev;

	if (pdev->dev.platform_data)
		hs->hs_pdata = pdev->dev.platform_data;

	if (hs->hs_pdata->hs_name)
		ipdev->name = hs->hs_pdata->hs_name;
	else
		ipdev->name	= DRIVER_NAME;

	ipdev->id.vendor	= 0x0001;
	ipdev->id.product	= 1;
	ipdev->id.version	= 1;

	input_set_capability(ipdev, EV_KEY, KEY_MEDIA);
	input_set_capability(ipdev, EV_KEY, KEY_VOLUMEUP);
	input_set_capability(ipdev, EV_KEY, KEY_VOLUMEDOWN);
#ifdef CONFIG_HUAWEI_KERNEL
    input_set_capability(ipdev, EV_KEY, KEY_PREVIOUSSONG);
    input_set_capability(ipdev, EV_KEY, KEY_NEXTSONG);
#endif
	input_set_capability(ipdev, EV_SW, SW_HEADPHONE_INSERT);
	input_set_capability(ipdev, EV_SW, SW_MICROPHONE_INSERT);
	input_set_capability(ipdev, EV_KEY, KEY_POWER);
	input_set_capability(ipdev, EV_KEY, KEY_END);

	rc = input_register_device(ipdev);
	if (rc) {
		dev_err(&ipdev->dev,
				"hs_probe: input_register_device rc=%d\n", rc);
		goto err_reg_input_dev;
	}

	platform_set_drvdata(pdev, hs);

	rc = hs_rpc_init();
	if (rc) {
		dev_err(&ipdev->dev, "rpc init failure\n");
		goto err_hs_rpc_init;
	}

/* power key detect solution for ANR */
#ifdef CONFIG_HUAWEI_FEATURE_POWER_KEY
	/*create /sys/devices/platform/msm-handset/power_key_detect*/
	if (device_create_file(&pdev->dev, &dev_attr_power_key_detect))
		printk(KERN_INFO "power_key_detect device file create fail !\n");
	init_power_key_dump();
#endif

	return 0;

err_hs_rpc_init:
	input_unregister_device(ipdev);
	ipdev = NULL;
err_reg_input_dev:
	input_free_device(ipdev);
err_alloc_input_dev:
	switch_dev_unregister(&hs->sdev);
err_switch_dev_register:
	kfree(hs);
	return rc;
}

static int __devexit hs_remove(struct platform_device *pdev)
{
	struct msm_handset *hs = platform_get_drvdata(pdev);

	input_unregister_device(hs->ipdev);
	switch_dev_unregister(&hs->sdev);
	kfree(hs);
	hs_rpc_deinit();
	return 0;
}

static struct platform_driver hs_driver = {
	.probe		= hs_probe,
	.remove		= __devexit_p(hs_remove),
	.driver		= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init hs_init(void)
{
    /* <emmc_oeminfo duangan 2010-4-25 begin */
	#ifdef CONFIG_HUAWEI_FEATURE_OEMINFO
    platform_driver_register(&rmt_oeminfo_driver);
	#endif
    /* emmc_oeminfo duangan 2010-4-25 end> */
    
	return platform_driver_register(&hs_driver);
}
late_initcall(hs_init);

static void __exit hs_exit(void)
{
	platform_driver_unregister(&hs_driver);
}
module_exit(hs_exit);

MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:msm-handset");
