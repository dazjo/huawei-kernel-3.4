/* modify for platform 8255 and 7x25a/27a */
/*
 * Copyright (C) 2012 The HUAWEI ,inc
 * All rights reserved.
 *
 */

#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <asm-arm/huawei/smem_vendor_huawei.h>

smem_huawei_vender usb_para_data;

/*
 * import_kernel_nv: parse parameters in cmdline.
 * And then initialize the corresponding globle variables.
 * For usb.pid.index, usb.serial, androidboot.localproppath,
 * the globle variable is smem_huawei_vender usb_para_data.
 * If you want to use other parameters in cmdline,
 * define new globle variables and initialize them in this function.
 * Return value: void
 * Side effect : none
 */
static void import_kernel_nv(char *name)
{
	unsigned long pid_index;
    /* init the variable */
	char local_property_info[65] = {0};
	char *country_name;
	
    if (*name != '\0') 
    {
        char *value = strchr(name, '=');
        if (value != NULL) 
        {
            *value++ = 0;
            
            /* Parse usb_pid_index in cmdline */
            if (!strcmp(name,"usb.pid.index")) 
            {
                if(!strict_strtoul(value, 10, &pid_index))
                {
                    usb_para_data.usb_para.usb_pid_index = (unsigned int)pid_index;
                }
            }
            /* Parse usb serial number in cmdline */
            else if (!strcmp(name,"usb.serial")) 
            {
                strlcpy(usb_para_data.usb_para.usb_serial, value, sizeof(usb_para_data.usb_para.usb_serial));
            }
            /* Parse vender_name and country_name, format in cmdline is "vender_name/country_name" */
            else if (!strcmp(name,"androidboot.localproppath")) 
            {
                strlcpy(local_property_info, value, sizeof(local_property_info));
                /* solve the problem that phone can't start up due to command line error */
                if(0 == *local_property_info || '/' == *local_property_info || !strchr(local_property_info, '/')) 
                {
                    memcpy(local_property_info, "hw/default", strlen("hw/default"));
                }
                
                country_name= strchr(local_property_info, '/');                
                *country_name++ = 0;

                strlcpy(usb_para_data.vender_para.vender_name, local_property_info, sizeof(usb_para_data.vender_para.vender_name));          
                strlcpy(usb_para_data.vender_para.country_name, country_name, sizeof(usb_para_data.vender_para.country_name));
            } 
        }
    }
}

/*
 * import_kernel_cmdline: get parameters from cmdline and initialize
 * corresponding loble variables. 
 * If you want use parameters in cmdline, you needn't change this funiton,
 * you should check wether import_kernel_nv() has parsed the parameter, if not,
 * add the parsing in import_kernel_nv(). 
 * Return value: void
 * Side effect : none
 */
void import_kernel_cmdline(void)
{
    /* init the variable */
    char cmdline[512] = {0};
    char *ptr;
    
	printk(KERN_INFO "%s\n", __func__);

    memcpy(cmdline, saved_command_line, strlen(saved_command_line));

    ptr = cmdline;
    while (ptr && *ptr) {
        char *x = strchr(ptr, ' ');
        if (x != 0) *x++ = 0;
        import_kernel_nv(ptr);
        ptr = x;
    }
        
    printk(KERN_INFO "cmdline sb_serial=%s,usb_pid_index=%d\n", usb_para_data.usb_para.usb_serial,
            usb_para_data.usb_para.usb_pid_index);
    
    printk(KERN_INFO "cmdline vendor=%s,country=%s\n", usb_para_data.vender_para.vender_name,
            usb_para_data.vender_para.country_name);
}


