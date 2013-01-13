/*
 * Copyright (C) 2008 The HUAWEI ,inc
 * All rights reserved.
 *
 */
#ifndef _SMEM_VENDOR_HUAWEI_H_
#define _SMEM_VENDOR_HUAWEI_H_


#define APP_USB_SERIAL_LEN   16

#define VENDOR_NAME_LEN      32

typedef struct _app_usb_para_smem
{
  /* Stores usb serial number for apps */
  unsigned char usb_serial[APP_USB_SERIAL_LEN];
  unsigned usb_pid_index;
} app_usb_para_smem;

typedef struct _app_verder_name
{
  unsigned char vender_name[VENDOR_NAME_LEN];
  unsigned char country_name[VENDOR_NAME_LEN];
  /* del the update state */
}app_vender_name;

/* This struct is differnt in 7x30 and 7x25a/27a
 * We don't read the struct form SMEM, but get the struct members from cmdline.
 * If you want to read usb_para and vender_para in 7x30 or 7x25a/27a,
 * use the golable variable smem_huawei_vender usb_para_data dirctly,
 * and the variable has been initialized already.
 * If you want to use other members in 7x25a/27a, define new variables,
 * and initialize it in import_kernel_nv() in smem_vendor_huawei.c
 */
typedef struct
{
  app_usb_para_smem      usb_para;
  app_vender_name   vender_para;
} smem_huawei_vender;

extern smem_huawei_vender usb_para_data;
extern void import_kernel_cmdline(void);

#define COUNTRY_JAPAN   "jp"
#define VENDOR_EMOBILE  "emobile"

#endif //_SMEM_VENDOR_HUAWEI_H_

