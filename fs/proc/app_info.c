/*
 *  linux/fs/proc/app_info.c
 *
 *
 * Changes:
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/fs.h>
#include <linux/tty.h>
#include <linux/string.h>
#include <linux/mman.h>
#include <linux/quicklist.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/pagemap.h>
#include <linux/interrupt.h>
#include <linux/swap.h>
#include <linux/slab.h>
#include <linux/genhd.h>
#include <linux/smp.h>
#include <linux/signal.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/seq_file.h>
#include <linux/times.h>
#include <linux/profile.h>
#include <linux/utsname.h>
#include <linux/blkdev.h>
#include <linux/hugetlb.h>
#include <linux/jiffies.h>
#include <linux/sysrq.h>
#include <linux/vmalloc.h>
#include <linux/crash_dump.h>
#include <linux/pid_namespace.h>
#include <linux/bootmem.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include <asm/tlb.h>
#include <asm/div64.h>
#include "internal.h"
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <linux/hardware_self_adapt.h>
#include "../../arch/arm/mach-msm/include/mach/socinfo.h"
#include <linux/touch_platform_config.h>
#define PROC_MANUFACTURER_STR_LEN 30
#define MAX_VERSION_CHAR 40
#define BOARD_ID_LEN 32
/* Redefine board sub version id len here */
#define BOARD_ID_SUB_VER_LEN 10
#define LCD_NAME_LEN 20
#define HW_VERSION   20
#define HW_VERSION_SUB_VER  6
/* modify audio property len from 20 to 32 */
#define AUDIO_PROPERTY_LEN 32

static char appsboot_version[MAX_VERSION_CHAR + 1];
static char str_flash_nand_id[PROC_MANUFACTURER_STR_LEN] = {0};
static u32 camera_id;
static u32 ts_id;
#ifdef CONFIG_HUAWEI_POWER_DOWN_CHARGE
static u32 charge_flag;
#endif
typedef struct
{
   int  mach_type; 
   char s_board_id[BOARD_ID_LEN];
   char hw_version_id[HW_VERSION];
}s_board_hw_version_type;

/* this is s_board_id and hw_version_id list,
 * when you want to add s_board_id and hw_version_if for new product,
 * add you s_board_id and hw_version_id this list.
 */
const s_board_hw_version_type s_board_hw_version_table[] =
{  /* machine_arch_type        s_board_id      hw_version_id */
   /*27A platform*/
   {MACH_TYPE_MSM7X27A_U8815, "MSM7227A_U8815", "HD1U861M "},
   {MACH_TYPE_MSM7X27A_U8655, "MSM7225A_U8655", "HD1U8655M "},
   {MACH_TYPE_MSM7X27A_U8655_EMMC, "MSM7225A_U8665", "HD2U8655M "},
   {MACH_TYPE_MSM7X27A_C8655_NAND, "MSM7625A_C8655", "HC1C8655M "},
   {MACH_TYPE_MSM7X27A_U8185, "MSM7225A_U8185", "HD1U8185M "},
   {MACH_TYPE_MSM7X27A_M660, "MSM7625A_M660", "HC1M660M "},
   {MACH_TYPE_MSM7X27A_C8820, "MSM7627A_C8812","HC1C8812M "},
   {MACH_TYPE_MSM7X27A_H867G, "MSM7225A_H867G","HD1H867GM "},
   {MACH_TYPE_MSM7X27A_H868C,"MSM7625A_H868C","HC1H868CM "},
   /*8x25 platform*/
   {MACH_TYPE_MSM8X25_C8833D,	"MSM8X25_C8833D",	"HD1C8833M "},
   {MACH_TYPE_MSM8X25_C8813,	"MSM8X25_C8813",	"HC1C8813M "},
   {MACH_TYPE_MSM8X25_H881C,	"MSM8X25_H881C",	"HC1H881CM "},
   {MACH_TYPE_MSM8X25_C8825D,"MSM8X25_C8825D","HC1C8825M "},
   {MACH_TYPE_MSM8X25_U8825D,"MSM8X25_U8825D","HD1U8825M "},
   {MACH_TYPE_MSM8X25_U8825,"MSM8X25_U8825","HD2U8825M "},
   {MACH_TYPE_MSM8X25_U8833D,"MSM8X25_U8833D","HD1U8833M "},
   {MACH_TYPE_MSM8X25_U8833,"MSM8X25_U8833","HD1U8833M "},
   {MACH_TYPE_MSM8X25_U8950D,	"MSM8X25_U8950D",	"HD1U8950M "},
   {MACH_TYPE_MSM8X25_C8950D,	"MSM8X25_C8950D",	"HC1C8950M "},
   {MACH_TYPE_MSM8X25_U8950,	"MSM8X25_U8950",	"HD1U8950M "},
   {MACH_TYPE_MSM8X25_C8812P,"MSM8X25_C8812E","HC1C8812M "},
   {MACH_TYPE_MSM8X25_U8951D,"MSM8X25_U8951D","HD2U8951M "},
   {MACH_TYPE_MSM8X25_U8951,"MSM8X25_U8951","HD2U8951M "},
   {MACH_TYPE_MSM7X27A_U8661, "MSM7225A_U8661","HD1U8661M "},
   {MACH_TYPE_MSM7X27A_C8668D, "MSM7625A_C8668D","HC1C8668DM"},
   /*30 and 8X55 platform*/
   {MACH_TYPE_MSM7X30_U8800,"MSM7X30_U8800",""},
   {MACH_TYPE_MSM7X30_U8820,"MSM7X30_U8820",""},
   {MACH_TYPE_MSM7X30_U8800_51,"MSM7X30_U8800-51",""},
   {MACH_TYPE_MSM8255_U8800_PRO,"MSM8255_U8800-PRO",""},
   {MACH_TYPE_MSM8255_U8860,"MSM8255_U8860","HD2U886M "},
   {MACH_TYPE_MSM8255_C8860,"MSM8255_C8860","HC1C886M "},
   {MACH_TYPE_MSM8255_U8860LP,"MSM8255_U8860LP","HD2U886M "},
   {MACH_TYPE_MSM8255_U8860_51,"MSM8255_U8860-51","HD3U886M01 "},
   {MACH_TYPE_MSM8255_U8860_92,"MSM8255_U8860-92","HD4U886M "},
   {MACH_TYPE_MSM8255_U8680,"MSM8255_U8680","HD1U868M "},
   {MACH_TYPE_MSM8255_U8730,"MSM8255_U8730","HD1U873M "},
   {MACH_TYPE_MSM8255_U8667,"MSM8255_U8667","HD1U866M "},
   {MACH_TYPE_MSM8255_U8860_R,"MSM8255_U8860-R","HD5U886M "},
};

void set_s_board_hw_version(char *s_board_id,char *hw_version_id)
{  
    unsigned int temp_num = 0;
    unsigned int table_num = 0;

    if ((NULL == s_board_id) || (NULL == hw_version_id))
    {
         printk("app_info : s_board_id or hw_version_type is null!\n");    
         return ;
    }

    table_num = sizeof(s_board_hw_version_table)/sizeof(s_board_hw_version_type);
    for(temp_num = 0;temp_num < table_num;temp_num++)
    {
         if(s_board_hw_version_table[temp_num].mach_type == machine_arch_type )
         {
             memcpy(s_board_id,s_board_hw_version_table[temp_num].s_board_id, BOARD_ID_LEN-1);
             memcpy(hw_version_id,s_board_hw_version_table[temp_num].hw_version_id, HW_VERSION-1);
             break;
         }
    }

    if(table_num == temp_num)
    {
        memcpy(s_board_id,"ERROR", (BOARD_ID_LEN-1));
        memcpy(hw_version_id,"ERROR", HW_VERSION-1);
    }
}
/*===========================================================================


FUNCTION     set_s_board_hw_version_special

DESCRIPTION
  This function deal with special hw_version_id s_board_id and so on
DEPENDENCIES
  
RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
static void set_s_board_hw_version_special(char *hw_version_id,char *hw_version_sub_ver,
                                char *s_board_id,char *sub_ver)
{
                                             
    if ((NULL == s_board_id) || (NULL == sub_ver) || (NULL == hw_version_id) || (NULL == hw_version_sub_ver))
    {
         printk("app_info : parameter pointer is null!\n");    
         return ;
    }

	/* U8815 silk-screen display to VerB */
    if((HW_VER_SUB_VB <= get_hw_sub_board_id()) 
       &&(MACH_TYPE_MSM7X27A_U8815 == machine_arch_type))
    {
        memcpy(hw_version_id,"HD1U8815M ", BOARD_ID_LEN-1);
        sprintf(hw_version_sub_ver, "VER.%c", 'A'+(char)get_hw_sub_board_id());
        strcat(hw_version_id, hw_version_sub_ver);
        hw_version_id[HW_VERSION-1] = '\0';

    }    

    if((MACH_TYPE_MSM7X30_U8820 == machine_arch_type)
       &&(socinfo_get_msm_cpu() == MSM_CPU_8X55))
    {
        memcpy(s_board_id,"MSM8255_U8820", BOARD_ID_LEN-1);
    	sprintf(sub_ver, ".Ver%c", 'A'+(char)get_hw_sub_board_id());
    	strcat(s_board_id, sub_ver);
        s_board_id[BOARD_ID_LEN-1] = '\0';
    }

	if((MACH_TYPE_MSM7X30_U8800_51 == machine_arch_type)
       &&(HW_VER_SUB_VD == get_hw_sub_board_id()))
    {
        memcpy(s_board_id,"MSM7X30_U8800-51", BOARD_ID_LEN-1);
        sprintf(sub_ver, ".Ver%c", 'C');
    	strcat(s_board_id, sub_ver);
        s_board_id[BOARD_ID_LEN-1] = '\0';
    }

    /* change U8185 hw_version to VerB */
    if(MACH_TYPE_MSM7X27A_U8185 == machine_arch_type) 
    {
        if(HW_VER_SUB_VE > get_hw_sub_board_id()) 
        {
            memcpy(hw_version_id,"HD1U8185M ", BOARD_ID_LEN-1);
            sprintf(hw_version_sub_ver, "VER.%c", 'A'+(char)get_hw_sub_board_id() + 1);
            strcat(hw_version_id, hw_version_sub_ver);
            hw_version_id[HW_VERSION-1] = '\0';
        }
        /* add U8186 hw_version 
         * and the U8186 sub ver keep same as U8185 sub ver. 
         */ 
        else if(HW_VER_SUB_VE <= get_hw_sub_board_id())
        {
            memcpy(hw_version_id,"HD1U8186M ", BOARD_ID_LEN-1);
            sprintf(hw_version_sub_ver, "VER.%c", 'A'+(char)get_hw_sub_board_id() - 3);
            strcat(hw_version_id, hw_version_sub_ver);
            hw_version_id[HW_VERSION-1] = '\0';
        }
    }

    /* change sub version to right version */
    if((HW_VER_SUB_VE <= get_hw_sub_board_id()) &&
           (MACH_TYPE_MSM7X27A_U8655_EMMC == machine_arch_type))
    {
       memcpy(hw_version_id,"HD2U8655M ", BOARD_ID_LEN-1);
       sprintf(hw_version_sub_ver, ".Ver%c", 'A'+(char)get_hw_sub_board_id() -3);
       strcat(hw_version_id, hw_version_sub_ver);
       hw_version_id[HW_VERSION-1] = '\0';
    }
    /* change the boardid name in the file of app_info according to the boardid sub version*/
    /* HW_VER_U8951_VC is used as HW_VER_U8951N_1_VA, and is recorded as MSM8X25_U8951-1.VerC in app_info;
     * HW_VER_U8951_VB is used as HW_VER_U8951_51_VA, and is recorded as MSM8X25_U8951-51.VerA in app_info;
     * HW_VER_U8951_VA is used as HW_VER_U8951_1_VA, and is recorded as MSM8X25_U8951-1.VerA in app_info;
     * HW_VER_U8833_VC is used as HW_VER_U8833N_1_VA, and is recorded as MSM8X25_U8833-1.VerC in app_info;
     * HW_VER_U8833_VB is used as HW_VER_U8833_51_VA, and is recorded as MSM8X25_U8833-51.VerA in app_info;
     * HW_VER_U8833_VA is used as HW_VER_U8833_1_VA, and is recorded as MSM8X25_U8833-1.VerA in app_info;
     */
    if(MACH_TYPE_MSM8X25_U8951 == machine_arch_type)
    {
        if(HW_VER_SUB_VA == get_hw_sub_board_id())
        {
            memcpy(s_board_id,"MSM8X25_U8951-1", BOARD_ID_LEN-1);
            sprintf(sub_ver, ".Ver%c", 'A');
            strcat(s_board_id, sub_ver);
            s_board_id[BOARD_ID_LEN-1] = '\0';
        }
        else if(HW_VER_SUB_VB == get_hw_sub_board_id())
        {
            memcpy(s_board_id,"MSM8X25_U8951-51", BOARD_ID_LEN-1);
            sprintf(sub_ver, ".Ver%c", 'A');
            strcat(s_board_id, sub_ver);
            s_board_id[BOARD_ID_LEN-1] = '\0';
        }
        else if(HW_VER_SUB_VC == get_hw_sub_board_id())
        {
            memcpy(s_board_id,"MSM8X25_U8951-1", BOARD_ID_LEN-1);
            sprintf(sub_ver, ".Ver%c", 'C');
            strcat(s_board_id, sub_ver);
            s_board_id[BOARD_ID_LEN-1] = '\0';
        }
    }
    
    if(MACH_TYPE_MSM8X25_U8833 == machine_arch_type)
    {
        if(HW_VER_SUB_VA == get_hw_sub_board_id())
        {
            memcpy(s_board_id,"MSM8X25_U8833-1", BOARD_ID_LEN-1);
            sprintf(sub_ver, ".Ver%c", 'A');
            strcat(s_board_id, sub_ver);
            s_board_id[BOARD_ID_LEN-1] = '\0';
        }
        else if(HW_VER_SUB_VB == get_hw_sub_board_id())
        {
            memcpy(s_board_id,"MSM8X25_U8833-51", BOARD_ID_LEN-1);
            sprintf(sub_ver, ".Ver%c", 'A');
            strcat(s_board_id, sub_ver);
            s_board_id[BOARD_ID_LEN-1] = '\0';
        }
		else if(HW_VER_SUB_VC == get_hw_sub_board_id())
        {
            memcpy(s_board_id,"MSM8X25_U8833-1", BOARD_ID_LEN-1);
            sprintf(sub_ver, ".Ver%c", 'C');
            strcat(s_board_id, sub_ver);
            s_board_id[BOARD_ID_LEN-1] = '\0';
        }
    }
}


/* same as in proc_misc.c */
static int
proc_calc_metrics(char *page, char **start, off_t off, int count, int *eof, int len)
{
	if (len <= off + count)
		*eof = 1;
	*start = page + off;
	len -= off;
	if (len > count)
		len = count;
	if (len < 0)
		len = 0;
	return len;
}

#define ATAG_BOOT_READ_FLASH_ID 0x4d534D72
static int __init parse_tag_boot_flash_id(const struct tag *tag)
{
    char *tag_flash_id=(char*)&tag->u;
    memset(str_flash_nand_id, 0, PROC_MANUFACTURER_STR_LEN);
    memcpy(str_flash_nand_id, tag_flash_id, PROC_MANUFACTURER_STR_LEN);
    
    printk("########proc_misc.c: tag_boot_flash_id= %s\n", tag_flash_id);

    return 0;
}
__tagtable(ATAG_BOOT_READ_FLASH_ID, parse_tag_boot_flash_id);

/*parse atag passed by appsboot, ligang 00133091, 2009-4-13, start*/
#define ATAG_BOOT_VERSION 0x4d534D71 /* ATAG BOOT VERSION */
static int __init parse_tag_boot_version(const struct tag *tag)
{
    char *tag_boot_ver=(char*)&tag->u;
    memset(appsboot_version, 0, MAX_VERSION_CHAR + 1);
    memcpy(appsboot_version, tag_boot_ver, MAX_VERSION_CHAR);
     
    //printk("nand_partitions.c: appsboot_version= %s\n\n", appsboot_version);

    return 0;
}
__tagtable(ATAG_BOOT_VERSION, parse_tag_boot_version);


#define ATAG_CAMERA_ID 0x4d534D74
static int __init parse_tag_camera_id(const struct tag *tag)
{
    char *tag_boot_ver=(char*)&tag->u;
	
    memcpy((void*)&camera_id, tag_boot_ver, sizeof(u32));
     
    return 0;
}
__tagtable(ATAG_CAMERA_ID, parse_tag_camera_id);


#define ATAG_TS_ID 0x4d534D75
static int __init parse_tag_ts_id(const struct tag *tag)
{
    char *tag_boot_ver=(char*)&tag->u;
	
    memcpy((void*)&ts_id, tag_boot_ver, sizeof(u32));
     
    return 0;
}


__tagtable(ATAG_TS_ID, parse_tag_ts_id);


static int app_version_read_proc(char *page, char **start, off_t off,
				 int count, int *eof, void *data)
{
	int len;
	// char *ker_ver = "HUAWEI_KERNEL_VERSION";
	char *ker_ver = HUAWEI_KERNEL_VERSION;
	char *lcd_name = NULL;
	char * touch_info = NULL;
	char * battery_name = NULL;
	char *wifi_device_name = NULL;
	char *bt_device_name = NULL;
	char audio_property[AUDIO_PROPERTY_LEN] = {0};
	/*print sensor info into app_info*/
	/* Array **_**_id must be large enough to hold both id and sub id */
	/* 'cause the following code would call strcat function to connect */
	/* sub id to array **_**_id[] */
	char s_board_id[BOARD_ID_LEN + BOARD_ID_SUB_VER_LEN] = {0};
    char sub_ver[BOARD_ID_SUB_VER_LEN] = {0};
	char hw_version_id[HW_VERSION + HW_VERSION_SUB_VER] = {0};
	char hw_version_sub_ver[HW_VERSION_SUB_VER] = {0};	
	char *compass_gs_name = NULL;
	char *sensors_list_name = NULL;
    set_s_board_hw_version(s_board_id,hw_version_id);
    sprintf(sub_ver, ".Ver%c", 'A'+(char)get_hw_sub_board_id());
   sprintf(hw_version_sub_ver, "VER.%c", 'A'+(char)get_hw_sub_board_id());
    strcat(s_board_id, sub_ver);
    strcat(hw_version_id, hw_version_sub_ver);
    set_s_board_hw_version_special(hw_version_id,hw_version_sub_ver,s_board_id,sub_ver);
	compass_gs_name=get_compass_gs_position_name();
	sensors_list_name = get_sensors_list_name();
	lcd_name = get_lcd_panel_name();
	wifi_device_name = get_wifi_device_name();
	bt_device_name = get_bt_device_name();
	get_audio_property(audio_property);
	touch_info = get_touch_info();
	if (touch_info == NULL)
	{
		touch_info = "Unknow touch";
	}
	battery_name = get_battery_manufacturer_info();
	if (NULL == battery_name)
	{
		battery_name = "Unknown battery";
	}
	
#ifdef CONFIG_HUAWEI_POWER_DOWN_CHARGE
    charge_flag = get_charge_flag();
	len = snprintf(page, PAGE_SIZE, "APPSBOOT:\n"
	"%s\n"
	"KERNEL_VER:\n"
	"%s\n"
	 "FLASH_ID:\n"
	"%s\n"
	"board_id:\n%s\n"
	"lcd_id:\n%s\n"
	"cam_id:\n%d\n"
	"ts_id:\n%d\n"
	"charge_flag:\n%d\n"
	"compass_gs_position:\n%s\n"
	"sensors_list:\n%s\n"
	"hw_version:\n%s\n"
    "wifi_chip:\n%s\n"
    "bt_chip:\n%s\n"
	"audio_property:\n%s\n"
	"touch_info:\n%s\n"
	"battery_id:\n%s\n",
	appsboot_version, ker_ver, str_flash_nand_id, s_board_id, lcd_name, camera_id, ts_id,charge_flag, compass_gs_name,sensors_list_name, hw_version_id,wifi_device_name, bt_device_name, audio_property, touch_info, battery_name);
#else
	len = snprintf(page, PAGE_SIZE, "APPSBOOT:\n"
	"%s\n"
	"KERNEL_VER:\n"
	"%s\n"
	 "FLASH_ID:\n"
	"%s\n"
	"board_id:\n%s\n"
	"lcd_id:\n%s\n"
	"cam_id:\n%d\n"
	"ts_id:\n%d\n"
	"compass_gs_position:\n%s\n"
	"sensors_list:\n%s\n"
	"hw_version:\n%s\n"
	"audio_property:\n%s\n"
	"touch_info:\n%s\n"
	"battery_id:\n%s\n",
	appsboot_version, ker_ver, str_flash_nand_id, s_board_id, lcd_name, camera_id, ts_id, compass_gs_name,sensors_list_name, hw_version_id,audio_property, touch_info, battery_name);
#endif
	
	return proc_calc_metrics(page, start, off, count, eof, len);
}

void __init proc_app_info_init(void)
{
	static struct {
		char *name;
		int (*read_proc)(char*,char**,off_t,int,int*,void*);
	} *p, simple_ones[] = {
		
        {"app_info", app_version_read_proc},
		{NULL,}
	};
	for (p = simple_ones; p->name; p++)
		create_proc_read_entry(p->name, 0, NULL, p->read_proc, NULL);

}


