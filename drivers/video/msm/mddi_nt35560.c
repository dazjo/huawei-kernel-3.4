/* drivers\video\msm\mddi_nt35560.c
 * NT35560 LCD driver for 7x30 platform
 *
 * Copyright (C) 2010 HUAWEI Technology Co., ltd.
 * 
 * Date: 2011/03/08
 * By jiaoshuangwei
 * 
 */

#include "msm_fb.h"
#include "mddihost.h"
#include "mddihosti.h"
#include <linux/mfd/pmic8058.h>
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/hardware_self_adapt.h>
#include <linux/pwm.h>
#include <mach/pmic.h>
#include "hw_backlight.h"
#include "hw_lcd_common.h"
#include "lcd_hw_debug.h"
#include <asm/mach-types.h> 
struct sequence* nt35560_fwvga_init_table = NULL;
static lcd_panel_type lcd_panel_fwvga = LCD_NONE;

#ifdef CONFIG_FB_AUTO_CABC
/* NT35560 CABC registers default value */
#define DEFAULT_VAL_ABC_CTRL2           0x0080
#define DEFAULT_VAL_ABC_CTRL6           0x0021
#define DEFAULT_VAL_ABC_CTRL7           0x0040
#define DEFAULT_VAL_ABC_CTRL14          0x0000
#define DEFAULT_VAL_MOV_CTRL1           0x0000

/* Value for NT35560 CABC registers parameters */
#define VAL_BIT_DMCT                    (0x01 << 1)
#define VAL_BIT_DD_C                    (0x01 << 7)
#define VAL_DIM_STEP_STILL(x)           ((x & 0x07) << 0)
#define VAL_DMST_C(x)                   ((x & 0x0F) << 0)
#define VAL_MOVDET(x)                   ((x & 0x7F) << 0)

/* Bit mask for parameter */
#define MASK_DIM_STEP_STILL             (0x07 << 0)
#define MASK_DMST_C                     (0x0F << 0)
#define MASK_MOVDET                     (0x7F << 0)

/* CABC state macro */
#define STATE_OFF                       0
#define STATE_ON                        1

static struct sequence nt35560_fwvga_write_cabc_mode_table[] = 
{
    /* Write CABC mode */
    {0x5500,0x00,0}
};

static struct sequence nt35560_fwvga_abc_ctrl_2_table[] = 
{
    /* ABC CTRL 2 */
    {0x19C0,0x80,0}
};

static struct sequence nt35560_fwvga_abc_ctrl_6_table[] = 
{
    /* ABC CTRL 6 */
    {0x1DC0,0x21,0}
};

static struct sequence nt35560_fwvga_abc_ctrl_7_table[] = 
{
    /* ABC CTRL 7 */
    {0x1EC0,0x40,0}
};

static struct sequence nt35560_fwvga_abc_ctrl_14_table[] = 
{
    /* ABC CTRL 14 */
    {0x25C0,0x00,0}
};

static struct sequence nt35560_fwvga_automatic_moving_selection_table[] = 
{
    /* MOV CTRL 1 */
    {0x72C0,0x00,0}
};
#endif // CONFIG_FB_AUTO_CABC

static struct sequence nt35560_fwvga_write_cabc_brightness_table[] = 
{
    /* Write CABC brightness */
    {0x5100,0x00,0}
};


/* reload CABC frequency register ,because sleep out ,it recover default value 
 * U8860 and C8860 use 300Hz
 */
static const struct sequence nt35560_fwvga_standby_exit_table[] = 
{
	{0x1100,0x00,120},
	{0x22C0,0xFF,0},
};
/* reload CABC frequency register ,because sleep out ,it recover default value 
 * low power board is 22kHz
 */
static const struct sequence nt35560_fwvga_standby_exit_tablelp[] = 
{
	{0x1100,0x00,120},
	{0x22C0,0x04,0},
};

static const struct sequence nt35560_fwvga_standby_enter_table[]= 
{
	{0x1000,0x00,120}
};
/* add the code for dynamic gamma function  */
#ifdef CONFIG_FB_DYNAMIC_GAMMA
//gamma 2.2
/* Revise some spelling mistake */
static const struct sequence nt35560_fwvga_dynamic_gamma22_table[] = 
{          
    {0XC980,0X01,0},
    {0x5500,0x00,0},
    {0X0180,0X14,0},
    {0X0280,0X00,0},
    {0X0380,0X33,0},
    {0X0480,0X48,0},
    {0X0780,0X00,0},
    {0X0880,0X44,0},
    {0X0980,0X54,0},
    {0X0A80,0X12,0},
    {0X1280,0X00,0},
    {0X1380,0X10,0},
    {0X1480,0X0d,0},
    {0X1580,0XA0,0},
    {0X1A80,0X67,0},
    {0X1F80,0X00,0},
    {0X2080,0X01,0},
    {0X2180,0X63,0},
    {0X2480,0X09,0},
    {0X2580,0X1E,0},
    {0X2680,0X4B,0},
    {0X2780,0X68,0},
    {0X2880,0X1F,0},
    {0X2980,0X37,0},
    {0X2A80,0X64,0},
    {0X2B80,0X84,0},
    {0X2D80,0X20,0},
    {0X2F80,0X2B,0},
    {0X3080,0XBD,0},
    {0X3180,0X1B,0},
    {0X3280,0X3A,0},
    {0X3380,0X4E,0},
    {0X3480,0X9B,0},
    {0X3580,0XBF,0},
    {0X3680,0XD9,0},
    {0X3780,0X76,0},
    {0X3880,0X09,0},
    {0X3980,0X26,0},
    {0X3A80,0X40,0},
    {0X3B80,0X64,0},
    {0X3D80,0X31,0},
    {0X3F80,0X45,0},
    {0X4080,0X64,0},
    {0X4180,0X42,0},
    {0X4280,0X14,0},
    {0X4380,0X1F,0},
    {0X4480,0X7B,0},
    {0X4580,0X1B,0},
    {0X4680,0X48,0},
    {0X4780,0X60,0},
    {0X4880,0X97,0},
    {0X4980,0XB4,0},
    {0X4A80,0XE1,0},
    {0X4B80,0X76,0},
    {0X4C80,0X09,0},
    {0X4D80,0X1E,0},
    {0X4E80,0X4B,0},
    {0X4F80,0X68,0},
    {0X5080,0X1F,0},
    {0X5180,0X37,0},
    {0X5280,0X64,0},
    {0X5380,0X8D,0},
    {0X5480,0X20,0},
    {0X5580,0X2B,0},
    {0X5680,0XC2,0},
    {0X5780,0X1E,0},
    {0X5880,0X40,0},
    {0X5980,0X54,0},
    {0X5A80,0X98,0},
    {0X5B80,0XB0,0},
    {0X5C80,0XD0,0},
    {0X5D80,0X76,0},
    {0X5E80,0X09,0},
    {0X5F80,0X2F,0},
    {0X6080,0X4F,0},
    {0X6180,0X67,0},
    {0X6280,0X2B,0},
    {0X6380,0X3F,0},
    {0X6480,0X61,0},
    {0X6580,0X3D,0},
    {0X6680,0X14,0},
    {0X6780,0X1F,0},
    {0X6880,0X72,0},
    {0X6980,0X1B,0},
    {0X6A80,0X48,0},
    {0X6B80,0X60,0},
    {0X6C80,0X97,0},
    {0X6D80,0XB4,0},
    {0X6E80,0XE1,0},
    {0X6F80,0X76,0},
    {0X7080,0X19,0},
    {0X7180,0X1E,0},
    {0X7280,0X4B,0},
    {0X7380,0X90,0},
    {0X7480,0X35,0},
    {0X7580,0X46,0},
    {0X7680,0X69,0},
    {0X7780,0XA4,0},
    {0X7880,0X20,0},
    {0X7980,0X2B,0},
    {0X7A80,0XCC,0},
    {0X7B80,0X19,0},
    {0X7C80,0X3C,0},
    {0X7D80,0X51,0},
    {0X7E80,0XA5,0},
    {0X7F80,0XD0,0},
    {0X8080,0XD0,0},
    {0X8180,0X76,0},
    {0X8280,0X09,0},
    {0X8380,0X2F,0},
    {0X8480,0X2F,0},
    {0X8580,0X5A,0},
    {0X8680,0X2E,0},
    {0X8780,0X43,0},
    {0X8880,0X66,0},
    {0X8980,0X33,0},
    {0X8A80,0X14,0},
    {0X8B80,0X1F,0},
    {0X8C80,0X5B,0},
    {0X8D80,0X16,0},
    {0X8E80,0X39,0},
    {0X8F80,0X4A,0},
    {0X9080,0X6F,0},
    {0X9180,0XB4,0},
    {0X9280,0XE1,0},
    {0X9380,0X66,0},
    {0X9480,0XBF,0},
    {0X9580,0X00,0},
    {0X9680,0X00,0},
    {0X9780,0XB4,0},
    {0X9880,0X0D,0},
    {0X9980,0X2C,0},
    {0X9A80,0X0A,0},
    {0X9B80,0X01,0},
    {0X9C80,0X01,0},
    {0X9D80,0X00,0},
    {0X9E80,0X00,0},
    {0X9F80,0X00,0},
    {0XA080,0X0A,0},
    {0XA280,0X06,0},
    {0XA380,0X2E,0},
    {0XA480,0X0E,0},
    {0XA580,0XC0,0},
    {0XA680,0X01,0},
    {0XA780,0X00,0},
    {0XA980,0X00,0},
    {0XAA80,0X00,0},
    {0XE780,0X00,0},
    {0XED80,0X00,0},
    {0XF380,0XCC,0},
    {0XFB80,0X00,0},
    {0X3500,0X00,0},
/* delete two lines */
};
//gamma1.9
/* Revise some spelling mistake */
static const struct sequence nt35560_fwvga_dynamic_gamma19_table[] = 
{
	{0x1100,0x00,120}
};
//gamma2.5
/* Revise some spelling mistake */
static const struct sequence nt35560_fwvga_dynamic_gamma25_table[] = 
{
/*there is 2.5 GAMA initialization sequence */
	{0XC980,0X01,0},
	{0x5500,0x00,0},
	{0X0180,0X14,0},
	{0X0280,0X00,0},
	{0X0380,0X33,0},
	{0X0480,0X48,0},
	{0X0780,0X00,0},
	{0X0880,0X44,0},
	{0X0980,0X54,0},
	{0X0A80,0X12,0},
	{0X1280,0X00,0},
	{0X1380,0X10,0},
	{0X1480,0X0d,0},
	{0X1580,0XA0,0},
	{0X1A80,0X67,0},
	{0X1F80,0X00,0},
	{0X2080,0X01,0},
	{0X2180,0X63,0},
	{0X2480,0X09,0},
	{0X2580,0X1E,0},
	{0X2680,0X54,0},
	{0X2780,0X73,0},
	{0X2880,0X1F,0},
	{0X2980,0X36,0},
	{0X2A80,0X64,0},
	{0X2B80,0X8B,0},
	{0X2D80,0X20,0},
	{0X2F80,0X29,0},
	{0X3080,0XC5,0},
	{0X3180,0X16,0},
	{0X3280,0X38,0},
	{0X3380,0X4D,0},
	{0X3480,0XAB,0},
	{0X3580,0XCB,0},
	{0X3680,0XD9,0},
	{0X3780,0X76,0},
	{0X3880,0X09,0},
	{0X3980,0X26,0},
	{0X3A80,0X34,0},
	{0X3B80,0X54,0},
	{0X3D80,0X32,0},
	{0X3F80,0X47,0},
	{0X4080,0X69,0},
	{0X4180,0X3A,0},
	{0X4280,0X16,0},
	{0X4380,0X1F,0},
	{0X4480,0X74,0},
	{0X4580,0X1B,0},
	{0X4680,0X49,0},
	{0X4780,0X60,0},
	{0X4880,0X8C,0},
	{0X4980,0XAB,0},
	{0X4A80,0XE1,0},
	{0X4B80,0X76,0},
	{0X4C80,0X09,0},
	{0X4D80,0X1E,0},
	{0X4E80,0X54,0},
	{0X4F80,0X73,0},
	{0X5080,0X1F,0},
	{0X5180,0X36,0},
	{0X5280,0X64,0},
	{0X5380,0X94,0},
	{0X5480,0X20,0},
	{0X5580,0X29,0},
	{0X5680,0XCA,0},
	{0X5780,0X19,0},
	{0X5880,0X3E,0},
	{0X5980,0X53,0},
	{0X5A80,0XA8,0},
	{0X5B80,0XBC,0},
	{0X5C80,0XD0,0},
	{0X5D80,0X76,0},
	{0X5E80,0X09,0},
	{0X5F80,0X2F,0},
	{0X6080,0X43,0},
	{0X6180,0X57,0},
	{0X6280,0X2C,0},
	{0X6380,0X41,0},
	{0X6480,0X66,0},
	{0X6580,0X35,0},
	{0X6680,0X16,0},
	{0X6780,0X1F,0},
	{0X6880,0X6B,0},
	{0X6980,0X1B,0},
	{0X6A80,0X49,0},
	{0X6B80,0X60,0},
	{0X6C80,0X8C,0},
	{0X6D80,0XAB,0},
	{0X6E80,0XE1,0},
	{0X6F80,0X76,0},
	{0X7080,0X19,0},
	{0X7180,0X1E,0},
	{0X7280,0X54,0},
	{0X7380,0X9B,0},
	{0X7480,0X35,0},
	{0X7580,0X45,0},
	{0X7680,0X69,0},
	{0X7780,0XAB,0},
	{0X7880,0X20,0},
	{0X7980,0X29,0},
	{0X7A80,0XD4,0},
	{0X7B80,0X14,0},
	{0X7C80,0X3A,0},
	{0X7D80,0X50,0},
	{0X7E80,0XB5,0},
	{0X7F80,0XDC,0},
	{0X8080,0XD0,0},
	{0X8180,0X76,0},
	{0X8280,0X09,0},
	{0X8380,0X2F,0},
	{0X8480,0X23,0},
	{0X8580,0X4A,0},
	{0X8680,0X2F,0},
	{0X8780,0X45,0},
	{0X8880,0X6B,0},
	{0X8980,0X2B,0},
	{0X8A80,0X16,0},
	{0X8B80,0X1F,0},
	{0X8C80,0X54,0},
	{0X8D80,0X16,0},
	{0X8E80,0X3A,0},
	{0X8F80,0X4A,0},
	{0X9080,0X64,0},
	{0X9180,0XAB,0},
	{0X9280,0XE1,0},
	{0X9380,0X66,0},
	{0X9480,0XBF,0},
	{0X9580,0X00,0},
	{0X9680,0X00,0},
	{0X9780,0XB4,0},
	{0X9880,0X0D,0},
	{0X9980,0X2C,0},
	{0X9A80,0X0A,0},
	{0X9B80,0X01,0},
	{0X9C80,0X01,0},
	{0X9D80,0X00,0},
	{0X9E80,0X00,0},
	{0X9F80,0X00,0},
	{0XA080,0X0A,0},
	{0XA280,0X06,0},
	{0XA380,0X2E,0},
	{0XA480,0X0E,0},
	{0XA580,0XC0,0},
	{0XA680,0X01,0},
	{0XA780,0X00,0},
	{0XA980,0X00,0},
	{0XAA80,0X00,0},
	{0XE780,0X00,0},
	{0XED80,0X00,0},
	{0XF380,0XCC,0},
	{0XFB80,0X00,0},
    {0X3500,0X00,0},
};
/* add the function  to set different gama by different mode */
/* Revise some spelling mistakes */
int nt35560_set_dynamic_gamma(enum danymic_gamma_mode  gamma_mode)
{
    int ret = 0;
    if (LOW_LIGHT == gamma_mode)
    {
        printk(KERN_ERR "the dynamic_gamma_setting is wrong\n");
    }
    switch(gamma_mode)
    {
        case GAMMA25:
            ret = process_mddi_table((struct sequence*)&nt35560_fwvga_dynamic_gamma25_table,
                        ARRAY_SIZE(nt35560_fwvga_dynamic_gamma25_table), lcd_panel_fwvga);
            break ;
        case GAMMA22:
            ret = process_mddi_table((struct sequence*)&nt35560_fwvga_dynamic_gamma22_table,
                        ARRAY_SIZE(nt35560_fwvga_dynamic_gamma22_table), lcd_panel_fwvga);
            break;
        case HIGH_LIGHT:
            ret = process_mddi_table((struct sequence*)&nt35560_fwvga_dynamic_gamma19_table,
                        ARRAY_SIZE(nt35560_fwvga_dynamic_gamma19_table), lcd_panel_fwvga);
            break;
        default:
            ret= -1;
            break;
    }
    LCD_DEBUG("%s: change gamma mode to %d\n",__func__,gamma_mode);
    return ret;
}
#endif

#ifdef CONFIG_FB_AUTO_CABC
/***************************************************************
Function: nt35560_set_cabc_moving_detect
Description: Set CABC moving detect function on or off
Parameters:
    uint32 state: 0 for off, 1 for on
Return:
    0: success
***************************************************************/
static int nt35560_set_cabc_moving_detect(uint32 state)
{
    int ret = 0;

    if (state == STATE_OFF)
    {
        /* Turn off automatic moving mode selection */
        nt35560_fwvga_automatic_moving_selection_table[0].value = DEFAULT_VAL_MOV_CTRL1;
        ret = process_mddi_table((struct sequence*)&nt35560_fwvga_automatic_moving_selection_table,
                    ARRAY_SIZE(nt35560_fwvga_automatic_moving_selection_table), lcd_panel_fwvga);
    }
    else
    {
        /* Automatic moving mode selection
         * If host's frame RAM update rate is 20 frames per second,
         * the CABC mode will be changed from still mode to moving mode.
         * This function is only available in normal display mode with CABC mode is set still mode.
         */
        nt35560_fwvga_automatic_moving_selection_table[0].value = (DEFAULT_VAL_MOV_CTRL1 & (~MASK_MOVDET)) | VAL_MOVDET(0x13);
        ret = process_mddi_table((struct sequence*)&nt35560_fwvga_automatic_moving_selection_table,
                    ARRAY_SIZE(nt35560_fwvga_automatic_moving_selection_table), lcd_panel_fwvga);
    }
    LCD_DEBUG("%s: set cabc moving detect: %d\n", __func__, state);

    return ret;
}

/***************************************************************
Function: nt35560_set_cabc_dimming
Description: Set CABC dimming function on or off
Parameters:
    uint32 state: 0 for off, 1 for on
Return:
    0: success
***************************************************************/
static int nt35560_set_cabc_dimming(uint32 state)
{
    int ret = 0;

    /* Set DMCT bit to 1, then the CABC dimming function is controlled by DD_C */
    nt35560_fwvga_abc_ctrl_14_table[0].value = VAL_BIT_DMCT | DEFAULT_VAL_ABC_CTRL14;
    ret = process_mddi_table((struct sequence*)&nt35560_fwvga_abc_ctrl_14_table,
                    ARRAY_SIZE(nt35560_fwvga_abc_ctrl_14_table), lcd_panel_fwvga);

    if (state == STATE_OFF)
    {
        /* Turn off the CABC dimming function */
        nt35560_fwvga_abc_ctrl_2_table[0].value = (~VAL_BIT_DD_C) & DEFAULT_VAL_ABC_CTRL2;
        ret = process_mddi_table((struct sequence*)&nt35560_fwvga_abc_ctrl_2_table,
                    ARRAY_SIZE(nt35560_fwvga_abc_ctrl_2_table), lcd_panel_fwvga);
    }
    else
    {
        /* Turn on the CABC dimming function */
        nt35560_fwvga_abc_ctrl_2_table[0].value = VAL_BIT_DD_C | DEFAULT_VAL_ABC_CTRL2;
        ret = process_mddi_table((struct sequence*)&nt35560_fwvga_abc_ctrl_2_table,
                    ARRAY_SIZE(nt35560_fwvga_abc_ctrl_2_table), lcd_panel_fwvga);

        /* DIM_STEP_STILL, 8 steps */
        nt35560_fwvga_abc_ctrl_6_table[0].value = (DEFAULT_VAL_ABC_CTRL6 & (~MASK_DIM_STEP_STILL)) | VAL_DIM_STEP_STILL(0x02);
        ret = process_mddi_table((struct sequence*)&nt35560_fwvga_abc_ctrl_6_table,
                    ARRAY_SIZE(nt35560_fwvga_abc_ctrl_6_table), lcd_panel_fwvga);

        /* DMST_C, 4 frames per step */
        nt35560_fwvga_abc_ctrl_7_table[0].value = (DEFAULT_VAL_ABC_CTRL7 & (~MASK_DMST_C)) | VAL_DMST_C(0x3);
        ret = process_mddi_table((struct sequence*)&nt35560_fwvga_abc_ctrl_7_table,
                    ARRAY_SIZE(nt35560_fwvga_abc_ctrl_7_table), lcd_panel_fwvga);
    }
    LCD_DEBUG("%s: set cabc dimming: %d\n", __func__, state);

    return ret;
}

/***************************************************************
Function: nt35560_set_cabc_mode
Description: Set CABC mode
Parameters:
    uint32 mode: 0 for off, 1 for UI mode, 2 for still mode, 3 for moving mode
Return:
    0: success
***************************************************************/
static int nt35560_set_cabc_mode(uint32 mode)
{
    int ret = 0;

    switch (mode)
    {
        case CABC_MODE_OFF:
        case CABC_MODE_UI:
        case CABC_MODE_STILL:
        case CABC_MODE_MOVING:
            /* Set CABC mode, 0 for off, 1 for UI mode, 2 for still mode, 3 for moving mode */
            nt35560_fwvga_write_cabc_mode_table[0].value = mode;
            ret = process_mddi_table((struct sequence*)&nt35560_fwvga_write_cabc_mode_table,
                        ARRAY_SIZE(nt35560_fwvga_write_cabc_mode_table), lcd_panel_fwvga);
            LCD_DEBUG("%s: set cabc mode to %d\n", __func__, mode);
            break;
        default:
            LCD_DEBUG("%s: invalid cabc mode: %d\n", __func__, mode);
            ret = -EINVAL;
            break;
    }

    return ret;
}

/***************************************************************
Function: nt35560_config_cabc
Description: Set CABC configuration
Parameters:
    struct msmfb_cabc_config cabc_cfg: CABC configuration struct
Return:
    0: success
***************************************************************/
static int nt35560_config_cabc(struct msmfb_cabc_config cabc_cfg)
{
    int ret = 0;

    /* Enable/Disable CABC dimming function */
    nt35560_set_cabc_dimming(cabc_cfg.dimming_on);

    /* Enable/Disable CABC moving detect function */
    nt35560_set_cabc_moving_detect(cabc_cfg.mov_det_on);

    /* Set CABC mode */
    nt35560_set_cabc_mode(cabc_cfg.mode);

    return ret;
}
#endif // CONFIG_FB_AUTO_CABC

/***************************************************************
Function: nt35560_set_cabc_brightness
Description: Set CABC brightness
Parameters:
    uint32 brightness: backlight brightness value
Return:
    0: success
***************************************************************/
static void nt35560_set_cabc_brightness(struct msm_fb_data_type *mfd,uint32 bl_level)
{
    nt35560_fwvga_write_cabc_brightness_table[0].value = bl_level;
    process_mddi_table((struct sequence*)&nt35560_fwvga_write_cabc_brightness_table,
                    ARRAY_SIZE(nt35560_fwvga_write_cabc_brightness_table), lcd_panel_fwvga);
}

static int nt35560_lcd_on(struct platform_device *pdev)
{
    int ret = 0;
   	boolean para_debug_flag = FALSE;
    uint32 para_num = 0;
/* open debug file and read the para */

	switch(lcd_panel_fwvga)
	{
		case LCD_NT35560_TOSHIBA_FWVGA:
			para_debug_flag = lcd_debug_malloc_get_para( "nt35560_toshiba_fwvga_init_table", 
	    		(void**)&nt35560_fwvga_init_table,&para_num);
			break;
		default:
			break;
	}
	/* If exist the init file ,then init lcd with it for debug */
    if( (TRUE == para_debug_flag)&&(NULL != nt35560_fwvga_init_table))
    {
		ret = process_mddi_table(nt35560_fwvga_init_table, para_num, lcd_panel_fwvga);
    }
    else
    {
		if(machine_is_msm8255_u8860lp()
        || machine_is_msm8255_u8860_r()
	    || machine_is_msm8255_u8860_51())
        {
			/* Exit Standby Mode */
			ret = process_mddi_table((struct sequence*)&nt35560_fwvga_standby_exit_tablelp, 
				ARRAY_SIZE(nt35560_fwvga_standby_exit_tablelp), lcd_panel_fwvga);
	
	    }
		else
		{ 
			/* Exit Standby Mode */
			ret = process_mddi_table((struct sequence*)&nt35560_fwvga_standby_exit_table, 
				ARRAY_SIZE(nt35560_fwvga_standby_exit_table), lcd_panel_fwvga);
       	}
	}
       
	/* Must malloc before,then you can call free */
	if((TRUE == para_debug_flag)&&(NULL != nt35560_fwvga_init_table))
	{
		lcd_debug_free_para((void *)nt35560_fwvga_init_table);
	}
	
    LCD_DEBUG("%s: nt35560_lcd exit sleep mode ,on_ret=%d\n",__func__,ret);
	
	return ret;
}
static int nt35560_lcd_off(struct platform_device *pdev)
{
    int ret = 0;
    /*enter sleep mode*/
    ret = process_mddi_table((struct sequence*)&nt35560_fwvga_standby_enter_table, 
    	      		ARRAY_SIZE(nt35560_fwvga_standby_enter_table), lcd_panel_fwvga);
    LCD_DEBUG("%s: nt35560_lcd enter sleep mode ,off_ret=%d\n",__func__,ret);
	return ret;
}

static int __devinit nt35560_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);
 	return 0;
}

static struct platform_driver this_driver = {
	.probe  = nt35560_probe,
	.driver = {
		.name   = "mddi_nt35560_fwvga",
	},
};

static struct msm_fb_panel_data nt35560_panel_data = {
	.on = nt35560_lcd_on,
	.off = nt35560_lcd_off,
	.set_backlight = pwm_set_backlight,
#ifdef CONFIG_FB_DYNAMIC_GAMMA
    .set_dynamic_gamma = nt35560_set_dynamic_gamma,
#endif
#ifdef CONFIG_FB_AUTO_CABC
    .config_cabc = nt35560_config_cabc,
#endif
    .set_cabc_brightness = nt35560_set_cabc_brightness,
};

static struct platform_device this_device = {
	.name   = "mddi_nt35560_fwvga",
	.id	= 0,
	.dev	= {
		.platform_data = &nt35560_panel_data,
	}
};
static int __init nt35560_init(void)
{
	int ret = 0;
	struct msm_panel_info *pinfo = NULL;
	bpp_type bpp = MDDI_OUT_16BPP;
	hw_lcd_interface_type mddi_port_type = get_hw_lcd_interface_type();

	lcd_panel_fwvga=get_lcd_panel_type();
	
	if(LCD_NT35560_TOSHIBA_FWVGA != lcd_panel_fwvga)
	{
		return 0;
	}
	LCD_DEBUG("%s:------nt35560_init------\n",__func__);
	/* Select which bpp accroding MDDI port type */
	if(LCD_IS_MDDI_TYPE1 == mddi_port_type)
	{
		bpp = MDDI_OUT_16BPP;
	}
	else if(LCD_IS_MDDI_TYPE2 == mddi_port_type)
	{
		bpp = MDDI_OUT_24BPP;
	}
	else
	{
		bpp = MDDI_OUT_16BPP;
	}
	
	ret = platform_driver_register(&this_driver);
	if (!ret) 
	{
		pinfo = &nt35560_panel_data.panel_info;
		pinfo->xres = 480;
		pinfo->yres = 854;
		pinfo->type = MDDI_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->mddi.vdopkt = MDDI_DEFAULT_PRIM_PIX_ATTR;
		pinfo->wait_cycle = 0;
		pinfo->bpp = (uint32)bpp;
		pinfo->fb_num = 2;
        pinfo->clk_rate = 192000000;
	    pinfo->clk_min = 192000000;
	    pinfo->clk_max = 192000000;
        pinfo->lcd.vsync_enable = TRUE;
        pinfo->lcd.refx100 = 6000;
		pinfo->lcd.v_back_porch = 0;
		pinfo->lcd.v_front_porch = 0;
		pinfo->lcd.v_pulse_width = 22;
		pinfo->lcd.hw_vsync_mode = TRUE;
		pinfo->lcd.vsync_notifier_period = 0;
		pinfo->bl_max = 255;

		ret = platform_device_register(&this_device);
		if (ret)
		{
			platform_driver_unregister(&this_driver);
		}
	}

	return ret;
}
module_init(nt35560_init);

