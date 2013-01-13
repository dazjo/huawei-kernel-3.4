
#ifndef __USB_SWITCH_HUAWEI_H__
#define __USB_SWITCH_HUAWEI_H__


#include <asm-arm/huawei/smem_vendor_huawei.h>

#define USB_DEFAULT_SN "0123456789AB"
/* support 3 luns at most, 1 lun for cdrom and 2 luns for udisk */
#define USB_MAX_LUNS   3

/* enable printing information about USB switching */
#define USB_AUTO_DEBUG

/* which value should be written to file:
       sys/devices/platform/msm_hsusb_periphera/fixusb
   when the user want to keep the USB composition stable.
   echo 21>fixusb       ->  normal usb switch
   echo 22>fixusb       ->  usb is kept in multiport and not switch
*/
#define ORI_INDEX                       0
#define CDROM_INDEX                     21
#define NORM_INDEX                      22
#define GOOGLE_INDEX                    25
#define GOOGLE_WLAN_INDEX               26

/* 
  This mode is specially used to automation slate test for SPINT,
  it same to CDROM_INDEX exception the serial number is valid.
*/
#define SLATE_TEST_INDEX                98
#define AUTH_INDEX                      99


#define SC_REWIND                       0x01
#define SC_REWIND_11                    0x11


#ifdef USB_AUTO_DEBUG
#define USB_PR(fmt, ...) \
        printk(KERN_INFO pr_fmt("usb_autorun: "fmt), ##__VA_ARGS__)
#else
#define USB_PR(fmt, ...) 
#endif

/* READ_TOC command structure */
typedef  struct _usbsdms_read_toc_cmd_type
{
   u8  op_code;  
   u8  msf;             /* bit1 is MSF, 0: address format is LBA form
                                        1: address format is MSF form */
   u8  format;          /* bit3~bit0,   MSF Field   Track/Session Number
                           0000b:       Valid       Valid as a Track Number
                           0001b:       Valid       Ignored by Drive
                           0010b:       Ignored     Valid as a Session Number
                           0011b~0101b: Ignored     Ignored by Drive
                           0110b~1111b: Reserved
                        */
   u8  reserved1;  
   u8  reserved2;  
   u8  reserved3;  
   u8  session_num;     /* a specific session or a track */
   u8  allocation_length_msb;
   u8  allocation_length_lsb;
   u8  control;
} usbsdms_read_toc_cmd_type;

/* vendor and country string */
#define COUNTRY_JAPAN   "jp"
#define VENDOR_EMOBILE  "emobile"
#define COUNTRY_US   "us"
#define VENDOR_TRACFONE  "tracfone"

extern void usb_port_switch_request(int usb_pid_index);

void android_disable_send_uevent(bool disable);
void android_usb_force_reset(void);

#endif  /* __USB_SWITCH_HUAWEI_H__ */

