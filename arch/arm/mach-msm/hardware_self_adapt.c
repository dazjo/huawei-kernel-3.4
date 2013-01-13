/*
 * Copyright (C) 2010 Huawei, Inc.
 * Copyright (c) 2008-2010, Huawei. All rights reserved.
 */
 
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/errno.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/setup.h>

#include <asm/mach-types.h>
#include "linux/hardware_self_adapt.h"

static unsigned int lcd_id = 0;
static unsigned int sub_board_id = 0;
#ifdef CONFIG_HUAWEI_POWER_DOWN_CHARGE
static unsigned int charge_flag = 0;
#endif

static unsigned int lcd_y_res = 480;

/* framebuffer size self adapter */
#ifdef CONFIG_FRAMEBUF_SELF_ADAPT
static __u32	frame_buffer_size = 0;
static __u32	frame_buffer_start = 0;	/* physical start address */
#endif

/* cust size self adapter */
static __u32	cust_buffer_size = 0;
static __u32	cust_buffer_start = 0;	/* physical start address */

static int sensors_list = 0;    /* record sensors which probe success */
static unsigned int recovery_boot_mode = 0;
 
#define ATAG_LCD_ID 0x4d534D73
int __init parse_tag_lcd_id(const struct tag *tags)
{
	 struct tag *t = (struct tag *)tags;
 
	 lcd_id = t->u.revision.rev;
	 printk(KERN_DEBUG "%s: lcd_id = 0x%x\n", __func__, lcd_id);
	 
	 return lcd_id;
}
 __tagtable(ATAG_LCD_ID, parse_tag_lcd_id);
 
#define ATAG_SUB_BOARD_ID 0x4d534D76
int __init parse_tag_sub_board_id(const struct tag *tags)
{
	 struct tag *t = (struct tag *)tags;

	 sub_board_id = t->u.revision.rev;
	 printk(KERN_DEBUG "%s: sub_board_id = 0x%x\n", __func__, sub_board_id);
	 
	 return sub_board_id;
}
 __tagtable(ATAG_SUB_BOARD_ID, parse_tag_sub_board_id);

#ifdef CONFIG_USB_AUTO_INSTALL
#define ATAG_BOOT_MODE_ID   0x4d534d77
 int __init parse_tag_boot_mode_id(const struct tag *tags)
 {
	 struct tag *t = (struct tag *)tags;
 
	 recovery_boot_mode = t->u.revision.rev;
	 printk(KERN_DEBUG "%s: usb_mode_id = 0x%x\n", __func__, recovery_boot_mode);
	 return recovery_boot_mode;
 }
 __tagtable(ATAG_BOOT_MODE_ID, parse_tag_boot_mode_id);
#endif  /* CONFIG_USB_AUTO_INSTALL */
 
#ifdef CONFIG_HUAWEI_POWER_DOWN_CHARGE
#define ATAG_CHARGE_FLAG  0x4d534D78
int __init parse_tag_charge_flag(const struct tag *tags)
{
    struct tag *t = (struct tag *)tags;

    charge_flag = t->u.revision.rev;
    printk(KERN_DEBUG "%s: charge_flag = 0x%x\n", __func__, charge_flag);

    return charge_flag;  
}
__tagtable(ATAG_CHARGE_FLAG, parse_tag_charge_flag);
#endif
 
/*get framebuffer address and size from atag, passed by bootloader*/
#ifdef CONFIG_FRAMEBUF_SELF_ADAPT
#define ATAG_FRAME_BUFFER_ID 0x4d534D79
int __init parse_tag_frame_buffer(const struct tag *tags)
{
	frame_buffer_size = tags->u.mem.size;
	frame_buffer_start = tags->u.mem.start;
	
    printk(KERN_DEBUG "%s: fb addr= 0x%x, size=0x%0x\n", __func__, frame_buffer_start, frame_buffer_size);
    return 0;
}
__tagtable(ATAG_FRAME_BUFFER_ID, parse_tag_frame_buffer);

#define ATAG_LCD_Y_RES_FLAG 0x4d534D7A
int __init parse_tag_lcd_y_res_flag(const struct tag *tags)
{
    struct tag *t = (struct tag *)tags;

    lcd_y_res= t->u.revision.rev;
    printk(KERN_DEBUG "%s: lcd_y_res = %d\n", __func__, lcd_y_res);

    return lcd_y_res;  
}
__tagtable(ATAG_LCD_Y_RES_FLAG, parse_tag_lcd_y_res_flag);

/*used in board-msm7x27a.c*/
void get_frame_buffer_mem_region(__u32 *start_addr, __u32 *size)
{
	*start_addr = frame_buffer_start;
	*size = frame_buffer_size;
}
#endif


/*get cust address and size from atag, passed by bootloader*/
#define ATAG_CUST_BUFFER_ID 0x4d534D7B
int __init parse_tag_cust_buffer(const struct tag * tags)
{
	cust_buffer_size = tags->u.mem.size;
	cust_buffer_start = tags->u.mem.start;
	
    printk(KERN_DEBUG "%s: cust addr= 0x%x, size=0x%0x\n", __func__, cust_buffer_start, cust_buffer_size);
    return 0;
}
__tagtable(ATAG_CUST_BUFFER_ID, parse_tag_cust_buffer);

/*used in board-msm7x27a.c*/
void get_cust_buffer_mem_region(__u32 *start_addr, __u32 *size)
{
	*start_addr = cust_buffer_start;
	*size = cust_buffer_size;
}

char *get_wifi_device_name(void)
{                                                                                                        
  hw_wifi_device_model wifi_device_model = WIFI_UNKNOW;  
  char *wifi_device_id = NULL;                       
                                                             
  wifi_device_model = get_hw_wifi_device_model();  
  //printk("wifi_device_id = %d\n",wifi_device_model);  
  if(WIFI_BROADCOM_4329 == wifi_device_model)                 
  {                                                  
	wifi_device_id = "1.1";
  }
  
  else if(WIFI_BROADCOM_4330 == wifi_device_model)
  {                                                  
	wifi_device_id = "1.2";
  }
                                                 
  else if(WIFI_QUALCOMM_6005 == wifi_device_model)            
  { 
    wifi_device_id = "2.1"; 
  }                                                  
  else                                               
  {                                                  
    wifi_device_id = "UNKNOWN WIFI DEVICE";          
  }                                                  
  return wifi_device_id;                             
} 

/* store bt device model and bt device name in bt_device_array[] */
struct bt_device bt_device_array[] = 
{
    { BT_BCM4329, "1.1" },
	{ BT_BCM4330, "1.2" },
	{ BT_WCN2243, "2.1" },
	{ BT_UNKNOWN, "Unknown" }
};

/* get bt device model by board id */
hw_bt_device_model get_hw_bt_device_model(void)
{
    if(machine_is_msm8x25_U8950D()
	  || machine_is_msm8x25_U8950()
      || machine_is_msm7x27a_H867G()
      || machine_is_msm8x25_H881C()
      || machine_is_msm7x27a_H868C()	
      || machine_is_msm8x25_C8950D())
    {
        return BT_BCM4330;
    }
    else if(machine_is_msm8x25_U8825()
      || machine_is_msm8x25_U8825D()
      || machine_is_msm8x25_C8825D()
      || machine_is_msm8x25_C8833D()
      || machine_is_msm8x25_U8951D()
      || machine_is_msm8x25_U8951()
      || machine_is_msm8x25_U8833D()
      || machine_is_msm8x25_U8833()
      || machine_is_msm8x25_C8813()
      || machine_is_msm8x25_C8812P())
    {
        return BT_WCN2243;
    }
    else
    {
        return BT_UNKNOWN;
    }
}

/* get bt device name */
char *get_bt_device_name(void)
{                   
    hw_bt_device_model bt_device_model = BT_UNKNOWN;
    int i = 0;

    bt_device_model = get_hw_bt_device_model();

    /* lookup bt_device_model in bt_device_array[] */
    for(i = 0; i < BT_UNKNOWN; i++)
    {
        if(bt_device_model == bt_device_array[i].dev_model)
        {
            break; 
        }
    }
	
	return bt_device_array[i].dev_name;
} 

/* modify spk mic function name */
/* Add DTS property */
void get_audio_property(char *audio_property)
{
  unsigned int property = AUDIO_PROPERTY_INVALID;
  audio_property_type mic_type = MIC_NONE;
  audio_property_type fir_enable = FIR_DISABLE;
  audio_property_type fm_type = FM_BROADCOM;
  audio_property_type spk_type = MONO_SPEAKER;
  audio_property_type spkmic_type = SPK_MAIN_MIC;
  audio_property_type dts_enable = DTS_DISABLE;
  
  mic_type = get_audio_mic_type();
  fir_enable = get_audio_fir_enabled();
  fm_type =  get_audio_fm_type();
  spk_type = get_audio_speaker_type();
  spkmic_type = get_audio_spkmic_type();
  dts_enable = get_audio_dts_enable();
  
  property = spkmic_type | spk_type | fm_type | fir_enable | mic_type | dts_enable;

  sprintf(audio_property, "%8x", property);
}

unsigned int get_hw_lcd_id(void)
{
	return lcd_id;
}

hw_ver_sub_type get_hw_sub_board_id(void)
{
	return (hw_ver_sub_type)(sub_board_id&HW_VER_SUB_MASK);
}

#ifdef CONFIG_HUAWEI_POWER_DOWN_CHARGE
unsigned int get_charge_flag(void)
{
    return charge_flag;
}
#endif

lcd_type atag_get_lcd_y_res(void)
{
   return (lcd_type)lcd_y_res;
}

/* the function interface to check boot mode in kernel */
unsigned char bootimage_is_recovery(void)
{
  return recovery_boot_mode;
}
/*  FUNCTION  get_hw_lcd_interface_type
 *  DEPENDENCIES 
 *      get lcd interface type
 *      affect nfc.
 *  RETURN VALUE
 *      lcd interface type:LCD_IS_MIPI or LCD_IS_RGB
 */
hw_lcd_interface_type get_hw_lcd_interface_type(void)
{
	hw_lcd_interface_type lcd_interface_type;
	lcd_panel_type  hw_lcd_panel = LCD_NONE;
	
	hw_lcd_panel = get_lcd_panel_type();	

	if (machine_is_msm7x30_u8800())
	{
		lcd_interface_type = LCD_IS_MDDI_TYPE1;
	}	
	/* U8820 board version A is MMDI type1, so config it type1 
	 * Version B and other is MDDI type2, so config it according to LCD
	 */
	else if(machine_is_msm7x30_u8820())
	{
		if(HW_VER_SUB_VA == get_hw_sub_board_id())
		{
			lcd_interface_type = LCD_IS_MDDI_TYPE1;
		}
		else
		{
			switch(hw_lcd_panel)
			{
				case LCD_NT35582_BYD_WVGA:
				case LCD_NT35582_TRULY_WVGA:
					lcd_interface_type = LCD_IS_MDDI_TYPE1;
					break;
				case LCD_NT35510_ALPHA_SI_WVGA:
					lcd_interface_type = LCD_IS_MDDI_TYPE1;
					break;
				case LCD_NT35510_ALPHA_SI_WVGA_TYPE2:
					lcd_interface_type = LCD_IS_MDDI_TYPE2;
					break;
				default:
					lcd_interface_type = LCD_IS_MDDI_TYPE1;
					break;
			}
		}
	}
	else if (machine_is_msm7x30_u8800_51() 
		|| machine_is_msm8255_u8800_pro())
	{
		switch(hw_lcd_panel)
		{
			case LCD_NT35582_BYD_WVGA:
			case LCD_NT35582_TRULY_WVGA:
				lcd_interface_type = LCD_IS_MDDI_TYPE1;
				break;
			case LCD_NT35510_ALPHA_SI_WVGA:
				lcd_interface_type = LCD_IS_MDDI_TYPE1;
				break;
			case LCD_NT35510_ALPHA_SI_WVGA_TYPE2:
				lcd_interface_type = LCD_IS_MDDI_TYPE2;
				break;
			default:
				lcd_interface_type = LCD_IS_MDDI_TYPE1;
				break;
		}
	}
    else if (machine_is_msm8255_u8860() 
		|| machine_is_msm8255_c8860() 
		|| machine_is_msm8255_u8860lp()
        || machine_is_msm8255_u8860_r()
		|| machine_is_msm8255_u8860_92()
		|| machine_is_msm8255_u8680()
		|| machine_is_msm8255_u8860_51()
		|| machine_is_msm8255_u8730())
	{
		lcd_interface_type = LCD_IS_MDDI_TYPE2;
	}

	else if (machine_is_msm8255_u8667())
	{
		lcd_interface_type = LCD_IS_MDDI_TYPE1;
	}
	else if(machine_is_msm7x27a_U8185()
		||machine_is_msm7x27a_M660())
	{
		lcd_interface_type = LCD_IS_RGB;
	}
	else
	{
		/*add mipi video interface*/
		switch(hw_lcd_panel)
		{
			case MIPI_VIDEO_NT35512_BOE_WVGA:
			case MIPI_VIDEO_HX8369B_TIANMA_WVGA:
			case MIPI_VIDEO_OTM8018B_CHIMEI_WVGA:
			case MIPI_VIDEO_NT35512_BYD_WVGA:
				lcd_interface_type = LCD_IS_MIPI_VIDEO;
				break;
			default:
				lcd_interface_type = LCD_IS_MIPI_CMD;
				break;
		}
	}
	return lcd_interface_type;
}
/* C8820VC uses PM pwm. */
hw_lcd_ctrl_bl_type get_hw_lcd_ctrl_bl_type(void)
{
    hw_lcd_ctrl_bl_type ctrl_bl_type = CTRL_BL_BY_UNKNOW;
	/*control backlight by MSM pwm*/
	/* C8668D uses PM pwm. */
	if (machine_is_msm7x27a_U8815() 
		|| machine_is_msm7x27a_U8655_EMMC()
		|| machine_is_msm7x27a_U8185() 
		|| machine_is_msm7x27a_U8655()
		|| machine_is_msm7x27a_M660()
		|| machine_is_msm7x27a_H867G()
		|| machine_is_msm7x27a_H868C()
		|| machine_is_msm7x30_u8800()
		|| machine_is_msm7x30_u8820() 
		|| machine_is_msm7x30_u8800_51() 
		|| machine_is_msm8255_u8800_pro()
		|| machine_is_msm8255_u8860() 
		|| machine_is_msm8255_c8860()
		|| machine_is_msm8255_u8860_92()
		|| (machine_is_msm7x27a_C8820() && (HW_VER_SUB_VC <= get_hw_sub_board_id()))
		||machine_is_msm7x27a_U8661()
		|| machine_is_msm7x27a_C8668D() 
		)
	{
		ctrl_bl_type = CTRL_BL_BY_MSM;
	}
	/*control backlight by LCD output pwm*/
	else
	{
		ctrl_bl_type = CTRL_BL_BY_LCD;
	}
    return ctrl_bl_type;
}
/*
 *brief: get lcd panel resolution
 */
lcd_type get_hw_lcd_resolution_type(void)
{
    lcd_type lcd_resolution = LCD_IS_HVGA;

/* add 8x55 paltform products */
    /*delete some lines for changing the lcd resolution of C8950D/U8950*/
    /*remove two products to adjust new LCD type*/
    if ( machine_is_msm7x27a_U8815() 
        || machine_is_msm8x25_U8825()
        || machine_is_msm8x25_U8825D()
        || machine_is_msm7x27a_C8820()
        || machine_is_msm8x25_C8825D()
        || machine_is_msm7x30_u8800()
        || machine_is_msm7x30_u8820() 
        || machine_is_msm7x30_u8800_51() 
        || machine_is_msm8255_u8800_pro()
        || machine_is_msm8255_u8680()
        || machine_is_msm8255_u8730()
        || machine_is_msm8x25_C8833D()
        || machine_is_msm8x25_U8833D()
        || machine_is_msm8x25_U8833()        
        || machine_is_msm8x25_H881C()
        || machine_is_msm8x25_C8812P())
	{
		lcd_resolution = LCD_IS_WVGA;
	}
	else if (machine_is_msm8x25_C8950D()
		|| machine_is_msm8x25_U8950()
		|| machine_is_msm8x25_U8950D())
	{
		lcd_resolution = LCD_IS_QHD;
	}
	else if (machine_is_msm8255_u8860() 
		|| machine_is_msm8255_c8860() 
		|| machine_is_msm8255_u8860lp()
        || machine_is_msm8255_u8860_r()
		|| machine_is_msm8255_u8860_92()
        || machine_is_msm8x25_C8813()
		|| machine_is_msm8255_u8860_51()
		|| machine_is_msm8x25_U8951D()
		|| machine_is_msm8x25_U8951())
	{
		lcd_resolution = LCD_IS_FWVGA;
	}
	/* C8668D uses HVGA. */
	else if ( machine_is_msm7x27a_M660() 
		|| machine_is_msm7x27a_U8655()	
		|| machine_is_msm7x27a_U8655_EMMC()
		|| machine_is_msm7x27a_C8655_NAND()
		|| machine_is_msm8255_u8667()
		|| machine_is_msm7x27a_U8661()
		|| machine_is_msm7x27a_H867G()
		|| machine_is_msm7x27a_H868C()
		|| machine_is_msm7x27a_C8668D())
	{
		lcd_resolution = LCD_IS_HVGA;
	}
	else if (machine_is_msm7x27a_U8185())
	{
		lcd_resolution = LCD_IS_QVGA;
	}
	else
	{
		lcd_resolution = LCD_IS_HVGA;
	}

    return lcd_resolution;
}

/*modify lcd name*/
lcd_panel_type get_lcd_panel_type(void)
{
	lcd_panel_type hw_lcd_panel = LCD_NONE;
	/*remove two products to adjust new LCD type*/
	/* separate Y300 from 8825 serials and make sure it can run BOE LCD well */
	/* Add Oem LCD driver */
	if ( machine_is_msm8x25_C8833D() 
		|| machine_is_msm8x25_U8833D()	
		|| machine_is_msm8x25_U8833()
	)
	{
		switch (lcd_id)
		{
			case LCD_HW_ID0:
				hw_lcd_panel = MIPI_CMD_OTM8009A_CHIMEI_WVGA;
				break;
			case LCD_HW_ID1:
				hw_lcd_panel = MIPI_CMD_HX8369A_TIANMA_WVGA;
				break;
			case LCD_HW_ID2:
				hw_lcd_panel = MIPI_VIDEO_HX8369B_TIANMA_WVGA;
				break;
			case LCD_HW_ID5:
				hw_lcd_panel = MIPI_CMD_NT35510_BOE_WVGA;
				break;
			/*Add nt35512 video mode for byd*/
			case LCD_HW_ID6:
				hw_lcd_panel = MIPI_VIDEO_NT35512_BYD_WVGA;
				break;
			/*Add otm8018b for video mode*/
			case LCD_HW_ID8:
				hw_lcd_panel = MIPI_VIDEO_OTM8018B_CHIMEI_WVGA;
				break;
			/*Add nt35512 for video mode*/
			case LCD_HW_ID9:
				hw_lcd_panel = MIPI_VIDEO_NT35512_BOE_WVGA;
				break;
            case LCD_HW_IDA:
                hw_lcd_panel = MIPI_CMD_NT35510_CHIMEI_WVGA;
				break;
			default:
				hw_lcd_panel = MIPI_CMD_NT35510_BOE_WVGA;
				break;
		}
	}
	else if (machine_is_msm8x25_H881C())
	{
		switch (lcd_id)
		{
			case LCD_HW_ID0:
				hw_lcd_panel = MIPI_CMD_NT35510_CHIMEI_WVGA;
				break;
			case LCD_HW_ID1:
				hw_lcd_panel = MIPI_CMD_HX8369A_TIANMA_WVGA;
				break;
			case LCD_HW_ID5:
				hw_lcd_panel = MIPI_CMD_NT35510_BOE_WVGA;
				break;
			default:
				hw_lcd_panel = MIPI_CMD_NT35510_BOE_WVGA;
				break;
		}
	}
	else if(  machine_is_msm8x25_U8825()
		|| machine_is_msm8x25_U8825D()	
		|| machine_is_msm8x25_C8825D()
		|| machine_is_msm8x25_C8812P()
	)
	{
		switch (lcd_id)
		{
			case LCD_HW_ID0:
				hw_lcd_panel = MIPI_CMD_RSP61408_CHIMEI_WVGA;
				break;
			case LCD_HW_ID1:
				hw_lcd_panel = MIPI_CMD_HX8369A_TIANMA_WVGA;
				break;
			case LCD_HW_ID4:
				hw_lcd_panel = MIPI_CMD_RSP61408_BYD_WVGA;
				break;
			case LCD_HW_ID5:
				hw_lcd_panel = MIPI_CMD_RSP61408_TRULY_WVGA;
				break;
			case LCD_HW_IDA:
				hw_lcd_panel = MIPI_CMD_NT35510_BOE_WVGA;
				break;
			default:
				hw_lcd_panel = MIPI_CMD_NT35510_BOE_WVGA;
				break;
		}
	}
	else if (machine_is_msm8x25_C8950D()
		|| machine_is_msm8x25_U8950()
		|| machine_is_msm8x25_U8950D())  
	{
		switch (lcd_id)
		{
			case LCD_HW_ID0:
				hw_lcd_panel = MIPI_CMD_NT35516_CHIMEI_QHD;
				break;
			case LCD_HW_ID5:
				hw_lcd_panel = MIPI_CMD_NT35516_TIANMA_QHD;
				break;
			default: 
				hw_lcd_panel = MIPI_CMD_NT35516_TIANMA_QHD;
				break;
		}
	}
	else if ( machine_is_msm8x25_U8951D()
        || machine_is_msm8x25_C8813()
		|| machine_is_msm8x25_U8951())
	{
		switch (lcd_id)
		{
			case LCD_HW_ID0:
				hw_lcd_panel = MIPI_CMD_NT35510_BOE_FWVGA;
				break;
			case LCD_HW_ID1:
				hw_lcd_panel = MIPI_CMD_HX8369A_TIANMA_FWVGA;
				break;
			case LCD_HW_ID4:
				hw_lcd_panel = MIPI_CMD_OTM8009A_CHIMEI_FWVGA;
				break;
			default:
				hw_lcd_panel = MIPI_CMD_HX8369A_TIANMA_FWVGA;
				break;
		}
	}
	else if( machine_is_msm7x27a_U8815() 
		|| machine_is_msm7x27a_C8820())

	{
		switch (lcd_id)
		{
			case LCD_HW_ID0:
				hw_lcd_panel = MIPI_CMD_RSP61408_CHIMEI_WVGA;
				break;
			case LCD_HW_ID1:
				hw_lcd_panel = MIPI_CMD_HX8369A_TIANMA_WVGA;
				break;
			case LCD_HW_ID4:
				hw_lcd_panel = MIPI_CMD_RSP61408_BYD_WVGA;
				break;
			case LCD_HW_ID5:
				hw_lcd_panel = MIPI_CMD_RSP61408_TRULY_WVGA;
				break;
			default:
				/*no mipi LCD lead to block, so default lcd RGB */
				hw_lcd_panel = LCD_HX8357B_TIANMA_HVGA;
				break;
		}
	}
	else if( machine_is_msm7x27a_U8655_EMMC())
	{
		switch (lcd_id)
		{
			case LCD_HW_ID0:
				hw_lcd_panel = MIPI_CMD_HX8357C_CHIMEI_HVGA;
				break;
			case LCD_HW_ID1:
			    hw_lcd_panel = MIPI_CMD_HX8357C_TIANMA_IPS_HVGA;
				break;
			case LCD_HW_ID4:
				hw_lcd_panel = MIPI_CMD_HX8357C_CHIMEI_IPS_HVGA;
				break;
			case LCD_HW_ID5:
				hw_lcd_panel = MIPI_CMD_HX8357C_TIANMA_HVGA;
				break;
			default: 
				/*no mipi LCD lead to block, so default lcd RGB */
				hw_lcd_panel = MIPI_CMD_HX8357C_CHIMEI_IPS_HVGA;
				break;
		}
	}
	else if(machine_is_msm7x27a_H867G())
	{
		/* U8686 is H867G ver.E,use tianma IPS and chimei IPS LCD */
		if(HW_VER_SUB_VE == get_hw_sub_board_id())
		{
			switch (lcd_id)
			{
				case LCD_HW_ID0:
					hw_lcd_panel = MIPI_CMD_NT35310_BYD_HVGA;
					break;
				case LCD_HW_ID1:
					hw_lcd_panel = MIPI_CMD_HX8357C_TIANMA_IPS_HVGA;
					break;
				case LCD_HW_ID4:
					hw_lcd_panel = MIPI_CMD_HX8357C_CHIMEI_IPS_HVGA;
					break;
				case LCD_HW_ID5:
					hw_lcd_panel = MIPI_CMD_NT35310_BOE_HVGA;
					break;
				default:
					hw_lcd_panel = MIPI_CMD_NT35310_BOE_HVGA;
					break;
			}
		}
		else
		{
			switch (lcd_id)
			{
				case LCD_HW_ID0:
					hw_lcd_panel = MIPI_CMD_NT35310_BYD_HVGA;
					break;
				case LCD_HW_ID1:
					hw_lcd_panel = MIPI_CMD_NT35310_TIANMA_HVGA;
					break;
				case LCD_HW_ID5:
					hw_lcd_panel = MIPI_CMD_NT35310_BOE_HVGA;
					break;
				default:
					hw_lcd_panel = MIPI_CMD_NT35310_BOE_HVGA;
					break;
			}
		}
	}
	else if(machine_is_msm7x27a_H868C())
	{
		switch (lcd_id)
		{
			case LCD_HW_ID0:
				hw_lcd_panel = MIPI_CMD_NT35310_BYD_HVGA;
				break;
			case LCD_HW_ID1:
				hw_lcd_panel = MIPI_CMD_NT35310_TIANMA_HVGA;
				break;
			case LCD_HW_ID5:
				hw_lcd_panel = MIPI_CMD_NT35310_BOE_HVGA;
				break;
			default:
				hw_lcd_panel = MIPI_CMD_NT35310_BOE_HVGA;
				break;
		}
	}
	else
	{
		hw_lcd_panel = LCD_HX8357B_TIANMA_HVGA;
	}
	return hw_lcd_panel;
}
/*modify the size of famebuffer*/
/*Add 4 framebuffer and delete the mem adapter strategy*/	
unsigned int get_framebuffer_size(void)
{
	unsigned int fb_size = 0;
	lcd_type lcd_resolution = LCD_IS_HVGA;
	lcd_resolution = get_hw_lcd_resolution_type();
	switch(lcd_resolution)
	{
		case LCD_IS_QVGA:
			fb_size = 0x100000;
			break;
		case LCD_IS_HVGA:
			fb_size = 0x200000;
			break;
		case LCD_IS_WVGA:
		case LCD_IS_FWVGA:
			fb_size = 0x500000;
			break;
		case LCD_IS_QHD:
			fb_size = 0x600000;
			break;
		default:
			fb_size = 0x600000;
	}
	return fb_size;
	
}
/*===========================================================================


FUNCTION     get_compass_gs_position

DESCRIPTION
  This function probe which G-sensor & Campass status type should be used

DEPENDENCIES
  
RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
compass_gs_position_type  get_compass_gs_position(void)
{
	compass_gs_position_type compass_gs_position=COMPASS_TOP_GS_TOP;
	/* modify compass and gs position by board id */
    //move C8820\25D define from TOP to BOTTOM
    if (machine_is_msm7x27a_surf() 
        || machine_is_msm7x27a_ffa() 
        || machine_is_msm7x27a_U8815()
        || machine_is_msm8x25_C8950D()
        || (machine_is_msm8x25_U8950() && (HW_VER_SUB_VB <= get_hw_sub_board_id()))
        || machine_is_msm8x25_H881C()
        || machine_is_msm8x25_U8950D()
        )
	{
		compass_gs_position=COMPASS_TOP_GS_TOP;
	}
	/*version A and version B has compass, since version C don't have compass*/
	else if(machine_is_msm7x27a_C8820() && (HW_VER_SUB_VC <= get_hw_sub_board_id()))
	{
		compass_gs_position=COMPASS_NONE_GS_BOTTOM;
	}
	/* add U8655_EMMC, use the u8655 configuration */
	else if (machine_is_msm7x27a_U8655() 
		  || machine_is_msm7x27a_U8655_EMMC()  
		  || machine_is_msm7x27a_C8655_NAND()  
		  || machine_is_msm7x27a_M660()  
		  || machine_is_msm7x27a_U8661()    
		  || machine_is_msm7x27a_C8820()   
		  || machine_is_msm7x27a_H867G()
		  || machine_is_msm7x27a_H868C()
		  || machine_is_msm8x25_C8825D()
		  || machine_is_msm7x30_u8800()
		  || machine_is_msm7x30_u8820() 
		  || machine_is_msm7x30_u8800_51()
		  || machine_is_msm8255_u8800_pro()
		  || machine_is_msm8255_u8860() 
		  || machine_is_msm8255_c8860()
		  || machine_is_msm8x25_U8951()
		  || machine_is_msm8x25_U8825()
          || machine_is_msm8x25_U8825D()
		  || (machine_is_msm8x25_U8950() && (HW_VER_SUB_VA == get_hw_sub_board_id()))
		  || machine_is_msm8255_u8860lp()
          || machine_is_msm8255_u8860_r()
		  || machine_is_msm8255_u8860_92()            
		  || machine_is_msm8255_u8860_51())
	{
		compass_gs_position=COMPASS_BOTTOM_GS_BOTTOM;
	}
	else if (machine_is_msm7x27a_U8185())	
	{
		compass_gs_position=COMPASS_NONE_GS_TOP;
	}
	else if(machine_is_msm8255_u8680() 
	     || machine_is_msm8255_u8730())
	{
	    compass_gs_position=COMPASS_TOP_GS_TOP;
	}
    else if (machine_is_msm7x27a_C8668D() 
		|| machine_is_msm8x25_C8833D()
        || machine_is_msm8x25_U8833D()
        || machine_is_msm8x25_U8833()
		)	
	{
		compass_gs_position = COMPASS_BOTTOM_GS_TOP;
	}
    else if (machine_is_msm8x25_C8812P()
         || machine_is_msm8x25_C8813()
	     || machine_is_msm8x25_U8951D() )
	{
		compass_gs_position=COMPASS_NONE_GS_BOTTOM;
	}
	return compass_gs_position;
}

/*===========================================================================


FUNCTION     get_lcd_align_type

DESCRIPTION
  This function probe which LCD align type should be used

DEPENDENCIES
  
RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
lcd_align_type get_lcd_align_type (void)
{
    lcd_panel_type  hw_lcd_panel = LCD_NONE;
    lcd_align_type  lcd_align    = LCD_PANEL_ALIGN_LSB;
     
	hw_lcd_panel = get_lcd_panel_type();

	if ((hw_lcd_panel == LCD_ILI9481DS_TIANMA_HVGA) 
		||(hw_lcd_panel == LCD_ILI9481D_INNOLUX_HVGA))
	{
		lcd_align = LCD_PANEL_ALIGN_MSB;
	}
	else
	{
		lcd_align = LCD_PANEL_ALIGN_LSB;
	}

    return lcd_align;
}
/*modify lcd name*/
char *get_lcd_panel_name(void)
{
	lcd_panel_type hw_lcd_panel = LCD_NONE;
	char *pname = NULL;	 

	hw_lcd_panel = get_lcd_panel_type();

	switch (hw_lcd_panel)
	{
		case LCD_S6D74A0_SAMSUNG_HVGA:
			pname = "SAMSUNG S6D74A0";
			break;

		case LCD_ILI9325_INNOLUX_QVGA:
			pname = "INNOLUX ILI9325";
			break;

		case LCD_ILI9325_BYD_QVGA:
			pname = "BYD ILI9325";
			break;

		case LCD_ILI9325_WINTEK_QVGA:
			pname = "WINTEK ILI9325";
			break;

		case LCD_SPFD5408B_KGM_QVGA:
			pname = "KGM SPFD5408B";
			break;

		case LCD_HX8357A_BYD_QVGA:
			pname = "BYD HX8357A";
			break;

		case LCD_HX8368A_SEIKO_QVGA:
			pname = "SEIKO HX8368A";
			break;

		case LCD_HX8347D_TRULY_QVGA:
			pname = "TRULY HX8347D";
			break;

		case LCD_ILI9325C_WINTEK_QVGA:
			pname = "WINTEK ILI9325C";
			break;

		case LCD_NT35582_BYD_WVGA:
			pname = "BYD NT35582";
			break;

		case LCD_NT35582_TRULY_WVGA:
			pname = "TRULY NT35582";
			break;

		case LCD_NT35560_TOSHIBA_FWVGA:
			pname = "TOSHIBA NT35560";
			break;

		case LCD_NT35510_ALPHA_SI_WVGA:
			pname = "TRULY NT35510";
			break;

		case LCD_NT35510_ALPHA_SI_WVGA_TYPE2:
			pname = "SUCCESS NT35510";
			break;
		case MDDI_RSP61408_CHIMEI_WVGA:
			pname = "CHIMEI RSP61408";
			break;
		case MDDI_RSP61408_BYD_WVGA:
			pname = "BYD RSP61408";
			break;
		case MDDI_HX8369A_TIANMA_WVGA:
			pname = "TIANMA HX8369A";
			break;
		case MDDI_HX8357C_TIANMA_HVGA:
			pname = "TIANMA HX8357C";
			break;
			
		case MDDI_HX8357C_CHIMEI_HVGA:
			pname = "CHIMEI HX8357C";
			break;
		case MDDI_HX8357C_CHIMEI_IPS_HVGA:
			pname = "CHIMEI IPS HX8357C";
			break;

		case LCD_HX8368A_TRULY_QVGA:
			pname = "TRULY HX8368A";
			break;

		case LCD_HX8347D_CHIMEI_QVGA:
			pname = "CHIMEI HX8347D";
			break;

		case LCD_HX8347G_TIANMA_QVGA:
			pname = "TIANMA HX8347G";
			break;

		case LCD_HX8357B_TIANMA_HVGA:
			pname = "TIANMA HX8357B";
			break;  

		case LCD_HX8357C_TIANMA_HVGA:
			pname = "TIANMA HX8357C";
			break;

		case LCD_R61529_TRULY_HVGA:
			pname = "TRULY R61529";
			break; 
			
		case LCD_ILI9481DS_TIANMA_HVGA:
			pname = "TIANMA ILI9481";
			break;

		case LCD_ILI9481D_INNOLUX_HVGA:
			pname = "INNOLUX ILI9481";
			break;

		case LCD_NT35410_CHIMEI_HVGA:
			pname = "CHIMEI NT35410";
			break;
			
		case MIPI_CMD_RSP61408_CHIMEI_WVGA:
			pname = "CHIMEI RSP61408";
			break;
			
		case MIPI_CMD_RSP61408_BYD_WVGA:
			pname = "BYD RSP61408";
			break;

		case MIPI_CMD_RSP61408_TRULY_WVGA: 
			pname = "TRULY RSP61408";
			break;

		case MIPI_CMD_HX8357C_TIANMA_IPS_HVGA:
		    pname = "TIANMA IPS HX8357C";
		    break;
		case MIPI_CMD_NT35510_BOE_WVGA:
			pname = "BOE NT35510";
			break;
		case MIPI_CMD_HX8357C_CHIMEI_HVGA:
			pname = "CHIMEI HX8357C";
			break;
			
		case MIPI_CMD_HX8357C_TIANMA_HVGA:
			pname = "TIANMA HX8357C";
			break;
			
		case MIPI_CMD_HX8369A_TIANMA_WVGA:
			pname = "TIANMA HX8369A";
			break;
		case MIPI_VIDEO_HX8369B_TIANMA_WVGA:
			pname = "TIANMA HX8369B";
			break;
		case MIPI_CMD_HX8357C_CHIMEI_IPS_HVGA:
			pname = "CHIMEI IPS HX8357C";
			break;

		case MIPI_CMD_NT35516_TIANMA_QHD:
			pname = "TIANMA NT35516";
			break;
		case MIPI_CMD_NT35516_CHIMEI_QHD:
			pname = "CHIMEI NT35516";
			break;
		case MIPI_CMD_HX8369A_TIANMA_FWVGA:
			pname = "TIANMA HX8369A";
			break;
		case MIPI_CMD_OTM8009A_CHIMEI_FWVGA:
		case MIPI_CMD_OTM8009A_CHIMEI_WVGA:
			pname = "CHIMEI OTM8009A";
			break;
		/*Add otm8018b for video mode*/
		case MIPI_VIDEO_OTM8018B_CHIMEI_WVGA:
			pname = "CHIMEI OTM8018B";
			break;
		/*Add nt35512 for video mode*/
		case MIPI_VIDEO_NT35512_BOE_WVGA:
			pname = "BOE NT35512";
			break;
		/*Add nt35512 video mode for byd*/
		case MIPI_VIDEO_NT35512_BYD_WVGA:
			pname = "BYD NT35512";
			break;
		case MIPI_CMD_NT35510_BOE_FWVGA:
			pname = "BOE NT35510";
			break;
		case MIPI_CMD_NT35310_TIANMA_HVGA:
			pname = "TIANMA NT35310";
			break;
		case MIPI_CMD_NT35310_BYD_HVGA:
			pname = "BYD NT35310";
			break;
		case MIPI_CMD_NT35310_BOE_HVGA:
			pname = "BOE NT35310";
			break;
		case MIPI_CMD_NT35510_CHIMEI_WVGA:
			pname = "CHIMEI NT35510";
			break;
		default:
			pname = "UNKNOWN LCD";
			break;
	}

	return pname;
}

int board_surport_fingers(bool * is_surport_fingers)
{
	 int result = 0;

	 if (is_surport_fingers == NULL)
	 {
		  return -ENOMEM;
	 }
	 
	 *is_surport_fingers = false;

	 return result;
}

int board_use_tssc_touch(bool * use_touch_key)
{
	 int result = 0;

	 *use_touch_key = false;
	 return result;
}

int board_support_ofn(bool * ofn_support)
{
	 int ret = 0;

	 if(NULL == ofn_support)
	 {
		 return -EPERM;
	 }

	 *ofn_support = false;
	 return ret;
}

static bool camera_i2c_state = false;
bool camera_is_supported(void)
{
	 return camera_i2c_state;
}

void set_camera_support(bool status)
{
	 camera_i2c_state = status;
}

bool board_support_flash(void)
{
	 /*product list that have flash*/
    if( machine_is_msm8x25_U8825()
        || machine_is_msm8x25_U8825D()
        || machine_is_msm8x25_U8833()        
        || machine_is_msm8x25_C8825D()
        || machine_is_msm8x25_C8950D()
        || machine_is_msm8x25_U8950D()
        || machine_is_msm8x25_U8951()
        || machine_is_msm8x25_C8813()
        || machine_is_msm8x25_H881C()	
        || machine_is_msm8x25_U8950())
	 {
		 return true;
	 }
	 return false;
}
static bool st303_gs_state = false;
bool st303_gs_is_supported(void)
{
	 return st303_gs_state;
}

void set_st303_gs_support(bool status)
{
	 st303_gs_state = status;
}

/*
*  return: 0 ----not support RGB LED driver
* 		 1 ----support RGB LED driver
*/
bool rgb_led_is_supported(void)
{
	bool ret = false;

	return ret;
}

bool qwerty_is_supported(void)
{
	bool ret = false;
	ret=(machine_is_msm7x27a_M660());
	return ret;
}
/* set sensors_list, called by sensor driver */
void set_sensors_list(int sensor)
{
	sensors_list |= sensor;
}

static int get_sensors_list(void)
{
	return sensors_list;
}

char *get_sensors_list_name(void)
{
	int sensors_list = G_SENSOR + L_SENSOR + P_SENSOR + M_SENSOR;
	char *list_name=NULL;

	sensors_list = get_sensors_list();

	switch(sensors_list)
	{
		case G_SENSOR + L_SENSOR + P_SENSOR + M_SENSOR + GY_SENSOR:
			 list_name = "G_L_P_M_GY_SENSORS";
			 break;
			 
		case G_SENSOR + L_SENSOR + P_SENSOR + M_SENSOR:
			 list_name = "G_L_P_M_SENSORS";
			 break;
			 
		case G_SENSOR + L_SENSOR + P_SENSOR:
			 list_name = "G_L_P_SENSORS";
			 break;
			 
		case G_SENSOR:
			 list_name = "G_SENSORS";
			 break;
			 
		case G_SENSOR + M_SENSOR + GY_SENSOR:
			 list_name = "G_M_GY_SENSORS";
			 break;
			 
		case G_SENSOR + M_SENSOR:
			 list_name = "G_M_SENSORS";
			 break;
			 
		case NONE_SENSOR:
			 list_name = "NONE_SENSORS";
			 break;
			 
		default:
			 list_name = "G_L_P_M_SENSORS";
			 break;
	}

	return list_name;
	
}

/*return the string by compass position*/
char *get_compass_gs_position_name(void)
{
	compass_gs_position_type compass_gs_position=COMPASS_TOP_GS_TOP;
	char *position_name=NULL;

	compass_gs_position = get_compass_gs_position();

	switch(compass_gs_position)
	{
		case COMPASS_TOP_GS_TOP:
			 position_name = "COMPASS_TOP_GS_TOP";
			 break;
			 
		case COMPASS_TOP_GS_BOTTOM:
			 position_name = "COMPASS_TOP_GS_BOTTOM";
			 break;

		case COMPASS_BOTTOM_GS_TOP:
			 position_name = "COMPASS_BOTTOM_GS_TOP";
			 break;

		case COMPASS_BOTTOM_GS_BOTTOM:
			 position_name = "COMPASS_BOTTOM_GS_BOTTOM";
			 break;
			 
		case COMPASS_NONE_GS_BOTTOM:
			 position_name = "COMPASS_NONE_GS_BOTTOM";
			 break;
		/*add gs position of COMPASS_NONE_GS_TOP*/			 
		case COMPASS_NONE_GS_TOP:
			 position_name = "COMPASS_NONE_GS_TOP";
			 break;

		default:
			 position_name = "COMPASS_TOP_GS_TOP";
			 break;
	}

	return position_name;
	
}

/*  FUNCTION  get_hw_wifi_device_type
 *  DEPENDENCIES 
 *      get wifi device type.
 *      affect wifi and camer.
 *  RETURN VALUE
 *      wifi device type:WIFI_QUALCOMM or WIFI_BROADCOM
 */
hw_wifi_device_type get_hw_wifi_device_type(void)
{
  if (machine_is_msm7x27a_U8185()
    || machine_is_msm7x27a_U8661() 
    || machine_is_msm8x25_C8825D() 
    || machine_is_msm8x25_U8825()
    || machine_is_msm8x25_U8825D()
    || machine_is_msm7x27a_C8668D()
    || machine_is_msm8x25_C8833D()
    || machine_is_msm8x25_U8951D()
    || machine_is_msm8x25_U8951()
    || machine_is_msm8x25_U8833D()
    || machine_is_msm8x25_U8833()    
    || machine_is_msm8x25_C8813()
    || machine_is_msm8x25_C8812P())
  {
      return WIFI_QUALCOMM;
  }
  else
  {
      return WIFI_BROADCOM;
  }
}

/*  FUNCTION  get_touch_type
 *  DEPENDENCIES 
 *      get tp project type.
 *  RETURN VALUE
 *      TP_COB: use cob code auto upgreade FW.
 *      TP_COF: Not use cob code, use before code.
 */
tp_type get_touch_type(void)
{
	if(machine_is_msm8x25_C8833D()
	    || machine_is_msm8x25_U8951D()
        || machine_is_msm8x25_U8951()
        || machine_is_msm8x25_U8833D()
        || machine_is_msm8x25_U8833()        
        || machine_is_msm8x25_C8813()
        || machine_is_msm8x25_H881C()		
       )
	{
		return TP_COB;
	}
	else
	{
		return TP_COF;
	}
}

/* Configuration upgrade mode */
 /* default need to update fw */
tp_update_type is_need_update_fw(void)
{
	return NEED_UPDATE_FW;
}

/*  FUNCTION  get_hw_wifi_device_model
 *  DEPENDENCIES 
 *      get wifi device model.
 *      affect app_info.
 *  RETURN VALUE
 *      wifi device model:WIFI_QUALCOMM_6005 or WIFI_BROADCOM_4329 ro WIFI_BROADCOM_4330
 */
hw_wifi_device_model get_hw_wifi_device_model(void)
{
  if(machine_is_msm7x27a_U8185()|| machine_is_msm7x27a_U8661()|| machine_is_msm7x27a_C8668D()
        || machine_is_msm8x25_C8825D()
        || machine_is_msm8x25_U8825D() 
        || machine_is_msm8x25_U8825()
        || machine_is_msm8x25_C8833D()
        || machine_is_msm8x25_U8951D()
        || machine_is_msm8x25_U8951()
        || machine_is_msm8x25_U8833D()
        || machine_is_msm8x25_U8833()        
        || machine_is_msm8x25_C8813()
        || machine_is_msm8x25_C8812P())
  {
      return WIFI_QUALCOMM_6005;
  }
  else if(machine_is_msm8255_u8860() 
		|| machine_is_msm8255_c8860() 
		|| machine_is_msm8255_u8860lp()
        || machine_is_msm8255_u8860_r()
		|| machine_is_msm8255_u8860_92()
		|| machine_is_msm8255_u8860_51())
  {
      return WIFI_BROADCOM_4329;
  }
  else
  {
      return WIFI_BROADCOM_4330;
  }
}

/*  FUNCTION  get_hw_ds_type
 *  DEPENDENCIES 
 *      get single sim card or double sim card,
 *      affect led.
 *  RETURN VALUE
 *      single sim card:sim card type HW_NODS 
 *      double sim card:sim card type HW_DS
 */
hw_ds_type get_hw_ds_type(void)
{
    hw_ds_type ret = HW_NONES;
    if( machine_is_msm8x25_C8950D()
      || machine_is_msm7x27a_U8815()
      || machine_is_msm7x27a_U8655_EMMC()
      || machine_is_msm7x27a_H867G()
      || machine_is_msm8x25_H881C()
      || machine_is_msm7x27a_H868C()
	  || (machine_is_msm8x25_U8950() && (HW_VER_SUB_VA == get_hw_sub_board_id())) )
    {
        ret = HW_NODS;
    }
    else
    {
        ret = HW_DS;
    }
  return ret;
}
/*  FUNCTION  get_hw_sd_trigger_type
 *  DEPENDENCIES 
 *      get sd interrupt trigger type
 *      affect sd detect.
 *  RETURN VALUE
 *      raise edge trigger : return RAISE_TRIGGER
 *      fall edge trigger : return FALL_TRIGGER
 */
hw_sd_trigger_type get_hw_sd_trigger_type(void)
{
  if(machine_is_msm7x27a_U8185())
  {
      return RAISE_TRIGGER;
  }
  else
  {
      return FALL_TRIGGER;
  }
}

/*  FUNCTION  get_hw_sd_trigger_type
 *  DESCRIPTION 
 *      get the bt wakeup gpio type
 *
 *  RETURN VALUE
 *       the gpio number
 */
hw_bt_wakeup_gpio_type get_hw_bt_wakeup_gpio_type(void)
{
    hw_bt_wakeup_gpio_type bt_wakeup_gpio_type = HW_BT_WAKEUP_GPIO_IS_NONES;
    hw_ver_sub_type ver_sub_type = HW_VER_SUB_MAX;
    ver_sub_type = get_hw_sub_board_id();
	/* U8825 use qcom bt, does not need BT_WAKEUP_GPIO */
    if (machine_is_msm7x27a_U8815()
        || machine_is_msm7x27a_U8655()
        || machine_is_msm7x27a_U8655_EMMC()
        || machine_is_msm7x27a_C8668D())
    {
        bt_wakeup_gpio_type = HW_BT_WAKEUP_GPIO_IS_83;
    }
    else if (machine_is_msm8x25_C8950D()
        || machine_is_msm8x25_U8950()
        || machine_is_msm7x27a_H867G()
        || machine_is_msm7x27a_H868C()
        || machine_is_msm8x25_H881C()
        || machine_is_msm8x25_U8950D())
    {
        bt_wakeup_gpio_type = HW_BT_WAKEUP_GPIO_IS_27;
    }
    else if (machine_is_msm7x27a_C8655_NAND())
    {
        if (ver_sub_type > HW_VER_SUB_VB)
		{
            bt_wakeup_gpio_type = HW_BT_WAKEUP_GPIO_IS_27;
		}
		else
		{
            bt_wakeup_gpio_type = HW_BT_WAKEUP_GPIO_IS_83;
		}
    }
    else if (machine_is_msm7x27a_M660())
    {
        if (ver_sub_type > HW_VER_SUB_VA)
		{
            bt_wakeup_gpio_type = HW_BT_WAKEUP_GPIO_IS_27;
		}
		else
		{
            bt_wakeup_gpio_type = HW_BT_WAKEUP_GPIO_IS_83;
		}
    }
    else if (machine_is_msm7x27a_C8820())
    {
        if (ver_sub_type > HW_VER_SUB_VB)
		{
            bt_wakeup_gpio_type = HW_BT_WAKEUP_GPIO_IS_27;
		}
		else
		{
            bt_wakeup_gpio_type = HW_BT_WAKEUP_GPIO_IS_83;
		}
    }
    /* C8825D use qcom bt, does not need BT_WAKEUP_GPIO, and move C8668D above */
    	
    printk(KERN_INFO "the bt_wakeup_gpio_type is %d\n", bt_wakeup_gpio_type);
    return bt_wakeup_gpio_type;
}

audio_property_type get_audio_speaker_type(void)
{
      if ( machine_is_msm8x25_C8950D()
        || machine_is_msm8x25_U8950()
        || machine_is_msm8x25_U8950D() )
      {
          return STEREO_SPEAKER;
      }
      else
      {
          return MONO_SPEAKER;
      }
}

/*===========================================================================


FUNCTION     audio_property_type get_audio_spkmic_type

DESCRIPTION
           This function descripe which mic type speaker used.

DEPENDENCIES
  
RETURN VALUE
  SPK_MAIN_MIC or SPK_SUB_MIC

SIDE EFFECTS
  None
===========================================================================*/
audio_property_type get_audio_spkmic_type(void)
{
    if(machine_is_msm8x25_U8950D()
      || machine_is_msm8x25_U8950()
      || machine_is_msm7x27a_H867G()
      || machine_is_msm7x27a_H868C()
      || machine_is_msm8x25_C8950D())
    {
        return SPK_SUB_MIC;
    }
    else
    {
        return SPK_MAIN_MIC;
    }
}

/*===========================================================================


FUNCTION     audio_property_type get_audio_dts_enable

DESCRIPTION
           This function descripe if dts audio effect is enabled.

DEPENDENCIES
  
RETURN VALUE
  DTS_ENABLE or DTS_DISABLE

SIDE EFFECTS
  None
===========================================================================*/
audio_property_type get_audio_dts_enable(void)
{
    if( machine_is_msm8x25_U8825()
      || machine_is_msm8x25_U8825D()
      || machine_is_msm8x25_C8950D()
      || machine_is_msm8x25_U8950()
      || machine_is_msm8x25_U8950D()
      || machine_is_msm8x25_U8951D()
      || machine_is_msm8x25_U8951())
    {
        return DTS_ENABLE;
    }
    else
    {
        return DTS_DISABLE;
    }
}

/* add C8820VC for SINGLE_MIC */
/* add C8825D for single mic */
/*===========================================================================


FUNCTION     audio_property_type get_audio_mic_type

DESCRIPTION
       This function descripe the fluence of dual mic arithmetic is enable or disable

DEPENDENCIES
  
RETURN VALUE
  SINGLE_MIC  or DUAL_MIC

SIDE EFFECTS
  None
===========================================================================*/
audio_property_type get_audio_mic_type(void)
{
  if(machine_is_msm7x27a_U8185()
     || (machine_is_msm7x27a_C8820() && (HW_VER_SUB_VC <= get_hw_sub_board_id()))
     || machine_is_msm8x25_U8825()
     || machine_is_msm8x25_U8825D()
     || machine_is_msm8x25_C8950D()
     || machine_is_msm8x25_U8950()
     || machine_is_msm8x25_U8950D()
     || machine_is_msm8x25_C8825D()
     || machine_is_msm8x25_C8833D()
     || machine_is_msm8x25_U8951D()
     || machine_is_msm8x25_U8951()
     || machine_is_msm8x25_U8833D()
     || machine_is_msm8x25_U8833()     
     || machine_is_msm8x25_C8813()
     || machine_is_msm8x25_C8812P()
    )
  {
      return SINGLE_MIC;
  }
  else
  {
      return DUAL_MIC;
  }  
}

/* if you want to enable fir function, please return FIR_ENABLE for adapted project */
/* enable C8825D and U8825 huawei fir */
audio_property_type get_audio_fir_enabled(void)
{
    /* add the fir enabl for C8833D U8833D U8833 */
    /* add the fir for c8812e */
    /* add fir enable for G510U */
    if(machine_is_msm8x25_C8825D()
       || machine_is_msm8x25_U8825D()
       || machine_is_msm8x25_U8825()
       || machine_is_msm8x25_C8813()
       || machine_is_msm8x25_U8951()
       || machine_is_msm8x25_U8951D()
       || machine_is_msm8x25_H881C()
       || machine_is_msm8x25_C8812P()
       || machine_is_msm7x27a_C8820()
       || machine_is_msm7x27a_H867G()
       || machine_is_msm7x27a_H868C()
       || machine_is_msm8x25_C8833D()
       || machine_is_msm8x25_U8833D()
       || machine_is_msm8x25_U8833()
       )
    {
        return FIR_ENABLE;
    }
    else
    {
        return FIR_DISABLE;
    }
}
audio_property_type get_audio_fm_type(void)
{
   if (machine_is_msm7x27a_U8185()
      || machine_is_msm8x25_U8825()
      || machine_is_msm8x25_U8825D()
      || machine_is_msm8x25_U8951D()
      || machine_is_msm8x25_U8951()
      || machine_is_msm8x25_C8825D()
      || machine_is_msm7x27a_U8661() 
      || machine_is_msm7x27a_C8668D()
      || machine_is_msm8x25_C8833D()
      || machine_is_msm8x25_U8833D()
      || machine_is_msm8x25_U8833()      
      || machine_is_msm8x25_C8813()
      || machine_is_msm8x25_C8812P())
   {
       return FM_QUALCOMM;
   }
   else
   {
       return FM_BROADCOM;
   }
}
hw_camera_type get_hw_camera_mirror_type(void)
{
    hw_camera_type ret = HW_NOT_MIRROR_OR_FLIP;
    if( machine_is_msm7x27a_C8820() || machine_is_msm7x27a_U8661() 
    || machine_is_msm7x27a_U8655_EMMC()
    || machine_is_msm7x27a_H867G()
    || machine_is_msm7x27a_H868C())
    {
        /*back camera should mirror and flip*/
        ret |= HW_MIRROR_AND_FLIP;
    }
    if(machine_is_msm8x25_U8951()
    ||machine_is_msm8x25_U8951D())
    {
        /*front camera should mirror and flip*/
        ret |= HW_MIRROR_AND_FLIP << 1;
    }
  return ret;
}
/* get touch info */
char *get_touch_info(void)
{
	char *touch_info = NULL;

	touch_info = get_synaptics_touch_info();
	if (touch_info != NULL)
		return touch_info;

	touch_info = get_melfas_touch_info();
	if (touch_info != NULL)
		return touch_info;

	return NULL;
}
/*4pin battery voltage id*/
char* get_battery_manufacturer_info()
{
	hw_battery_id_mv batt_id;
	char *pmanufacturer_name = "Unknown battery";
	batt_id = get_battery_resistance_id();
	switch (batt_id)
	{
	case BATTERY_RESISTANCE_MV_10:
		pmanufacturer_name = "BYD";
		break;	
	case BATTERY_RESISTANCE_MV_22:
		pmanufacturer_name = "GY";
		break;
	case BATTERY_RESISTANCE_MV_40:
		pmanufacturer_name = "LS";
		break;
	case BATTERY_RESISTANCE_MV_110_1:
		pmanufacturer_name = "MAX";
		break;
	case BATTERY_RESISTANCE_MV_470_1:
		pmanufacturer_name = "SAN";
		break;
	default:
		break;
	}
	return pmanufacturer_name;
}
hw_camera_flash_number get_hw_camera_flash_number(void)
{
    hw_camera_flash_number ret = CAMERA_FLASH_LED_SINGLE;
    if (machine_is_msm8x25_C8950D()
    || machine_is_msm8x25_U8950D()
    || machine_is_msm8x25_U8950())
    {
        ret = CAMERA_FLASH_LED_DOUBLE;
    }
    else
    {
        ret = CAMERA_FLASH_LED_SINGLE;
    }
    
    return ret;
}
