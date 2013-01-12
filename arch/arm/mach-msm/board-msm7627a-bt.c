/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
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

#if (defined(HUAWEI_BT_BLUEZ_VER30) || (!defined(CONFIG_HUAWEI_KERNEL)))

#include <linux/delay.h>
#include <linux/rfkill.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/mfd/marimba.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <asm/mach-types.h>
#include <mach/rpc_pmapp.h>
#include <mach/socinfo.h>

#include "board-msm7627a.h"
#include "devices-msm7x2xa.h"

#if defined(CONFIG_BT) && defined(CONFIG_MARIMBA_CORE)


static struct bt_vreg_info bt_vregs[] = {
	{"msme1", 2, 1800000, 1800000, 0, NULL},
	/* modify the power from 2.9V to 3.3V */
	{"bt", 21, 3300000, 3300000, 1, NULL}
};

static struct platform_device msm_bt_power_device = {
	.name = "bt_power",
};

static unsigned bt_config_power_on[] = {
	/*RFR*/
	GPIO_CFG(43, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	/*CTS*/
	GPIO_CFG(44, 2, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	/*RX*/
	GPIO_CFG(45, 2, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	/*TX*/
	GPIO_CFG(46, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};
static unsigned bt_config_pcm_on[] = {
	/*PCM_DOUT*/
	GPIO_CFG(68, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	/*PCM_DIN*/
	GPIO_CFG(69, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	/*PCM_SYNC*/
	GPIO_CFG(70, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	/*PCM_CLK*/
	GPIO_CFG(71, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};
static unsigned bt_config_power_off[] = {
	/*RFR*/
	GPIO_CFG(43, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	/*CTS*/
	GPIO_CFG(44, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	/*RX*/
	GPIO_CFG(45, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	/*TX*/
	GPIO_CFG(46, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
};
static unsigned bt_config_pcm_off[] = {
	/*PCM_DOUT*/
	GPIO_CFG(68, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	/*PCM_DIN*/
	GPIO_CFG(69, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	/*PCM_SYNC*/
	GPIO_CFG(70, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	/*PCM_CLK*/
	GPIO_CFG(71, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
};

static unsigned fm_i2s_config_power_on[] = {
	/*FM_I2S_SD*/
	GPIO_CFG(68, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	/*FM_I2S_WS*/
	GPIO_CFG(70, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	/*FM_I2S_SCK*/
	GPIO_CFG(71, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

static unsigned fm_i2s_config_power_off[] = {
	/*FM_I2S_SD*/
	GPIO_CFG(68, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	/*FM_I2S_WS*/
	GPIO_CFG(70, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	/*FM_I2S_SCK*/
	GPIO_CFG(71, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
};

#ifdef CONFIG_HUAWEI_KERNEL
static unsigned bt_config_sys_rest[] = {
        GPIO_CFG(5, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
};
#endif

#ifdef CONFIG_HUAWEI_KERNEL
#define GPIO_BT_SYS_REST 5
#endif

#ifdef CONFIG_HUAWEI_KERNEL
int gpio_bt_sys_rest_en = GPIO_BT_SYS_REST;
#else
int gpio_bt_sys_rest_en = 133;
#endif

#ifndef CONFIG_HUAWEI_KERNEL
static void gpio_bt_config(void)
{
	u32 socinfo = socinfo_get_platform_version();
	if (machine_is_msm7627a_qrd1())
		gpio_bt_sys_rest_en = 114;
	if (machine_is_msm7627a_evb() || machine_is_msm8625_evb()
				|| machine_is_msm8625_evt())
		gpio_bt_sys_rest_en = 16;
	if (machine_is_msm8625_qrd7())
		gpio_bt_sys_rest_en = 88;
	if (machine_is_msm7627a_qrd3()) {
		if (socinfo == 0x70002)
			gpio_bt_sys_rest_en = 88;
		 else
			gpio_bt_sys_rest_en = 85;
	}
}
#endif

static int bt_set_gpio(int on)
{
	int rc = 0;
	struct marimba config = { .mod_id =  SLAVE_ID_BAHAMA};

	pr_debug("%s: Setting SYS_RST_PIN(%d) to %d\n",
			__func__, gpio_bt_sys_rest_en, on);
	if (on) {

		if (machine_is_msm7627a_evb() || machine_is_msm8625_qrd7()) {
			rc = gpio_tlmm_config(GPIO_CFG(gpio_bt_sys_rest_en, 0,
					GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,
					GPIO_CFG_2MA),
					GPIO_CFG_ENABLE);

			gpio_set_value(gpio_bt_sys_rest_en, 1);
		} else {
			rc = gpio_direction_output(gpio_bt_sys_rest_en, 1);
		}
		msleep(100);
	} else {

		if (!marimba_get_fm_status(&config) &&
				!marimba_get_bt_status(&config)) {
			if (machine_is_msm7627a_evb() ||
					 machine_is_msm8625_qrd7()) {
				gpio_set_value(gpio_bt_sys_rest_en, 0);
				rc = gpio_tlmm_config(GPIO_CFG(
					gpio_bt_sys_rest_en, 0,
					GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN,
					GPIO_CFG_2MA),
					GPIO_CFG_ENABLE);
			} else {
				gpio_set_value_cansleep(gpio_bt_sys_rest_en, 0);
				rc = gpio_direction_input(gpio_bt_sys_rest_en);
			}
			msleep(100);
		}
	}
	if (rc)
		pr_err("%s: BT sys_reset_en GPIO : Error", __func__);

	return rc;
}

static struct regulator *fm_regulator;
static int fm_radio_setup(struct marimba_fm_platform_data *pdata)
{
	int rc = 0;
	const char *id = "FMPW";
	uint32_t irqcfg;
	struct marimba config = { .mod_id =  SLAVE_ID_BAHAMA};
	u8 value;

	/* Voting for 1.8V Regulator */
	fm_regulator = regulator_get(NULL , "msme1");
	if (IS_ERR(fm_regulator)) {
		rc = PTR_ERR(fm_regulator);
		pr_err("%s: could not get regulator: %d\n", __func__, rc);
		goto out;
	}

	/* Set the voltage level to 1.8V */
	rc = regulator_set_voltage(fm_regulator, 1800000, 1800000);
	if (rc < 0) {
		pr_err("%s: could not set voltage: %d\n", __func__, rc);
		goto reg_free;
	}

	/* Enabling the 1.8V regulator */
	rc = regulator_enable(fm_regulator);
	if (rc) {
		pr_err("%s: could not enable regulator: %d\n", __func__, rc);
		goto reg_free;
	}

	/* Voting for 19.2MHz clock */
	rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_D1,
			PMAPP_CLOCK_VOTE_ON);
	if (rc < 0) {
		pr_err("%s: clock vote failed with :(%d)\n",
			__func__, rc);
		goto reg_disable;
	}

	rc = bt_set_gpio(1);
	if (rc) {
		pr_err("%s: bt_set_gpio = %d", __func__, rc);
		goto gpio_deconfig;
	}
	/*re-write FM Slave Id, after reset*/
	value = BAHAMA_SLAVE_ID_FM_ADDR;
	rc = marimba_write_bit_mask(&config,
			BAHAMA_SLAVE_ID_FM_REG, &value, 1, 0xFF);
	if (rc < 0) {
		pr_err("%s: FM Slave ID rewrite Failed = %d", __func__, rc);
		goto gpio_deconfig;
	}
	/* Configuring the FM GPIO */
	irqcfg = GPIO_CFG(FM_GPIO, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
			GPIO_CFG_2MA);

	rc = gpio_tlmm_config(irqcfg, GPIO_CFG_ENABLE);
	if (rc) {
		pr_err("%s: gpio_tlmm_config(%#x)=%d\n",
			 __func__, irqcfg, rc);
		goto gpio_deconfig;
	}

	return 0;

gpio_deconfig:
	pmapp_clock_vote(id, PMAPP_CLOCK_ID_D1,
		PMAPP_CLOCK_VOTE_OFF);
	bt_set_gpio(0);
reg_disable:
	regulator_disable(fm_regulator);
reg_free:
	regulator_put(fm_regulator);
	fm_regulator = NULL;
out:
	return rc;
};

static void fm_radio_shutdown(struct marimba_fm_platform_data *pdata)
{
	int rc;
	const char *id = "FMPW";

	/* Releasing the GPIO line used by FM */
	uint32_t irqcfg = GPIO_CFG(FM_GPIO, 0, GPIO_CFG_INPUT,
		GPIO_CFG_PULL_UP, GPIO_CFG_2MA);

	rc = gpio_tlmm_config(irqcfg, GPIO_CFG_ENABLE);
	if (rc)
		pr_err("%s: gpio_tlmm_config(%#x)=%d\n",
			 __func__, irqcfg, rc);

	/* Releasing the 1.8V Regulator */
	if (!IS_ERR_OR_NULL(fm_regulator)) {
		rc = regulator_disable(fm_regulator);
		if (rc)
			pr_err("%s: could not disable regulator: %d\n",
					__func__, rc);
		regulator_put(fm_regulator);
		fm_regulator = NULL;
	}

	/* Voting off the clock */
	rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_D1,
		PMAPP_CLOCK_VOTE_OFF);
	if (rc < 0)
		pr_err("%s: voting off failed with :(%d)\n",
			__func__, rc);
	rc = bt_set_gpio(0);
	if (rc)
		pr_err("%s: bt_set_gpio = %d", __func__, rc);
}
static int switch_pcm_i2s_reg_mode(int mode)
{
	unsigned char reg = 0;
	int rc = -1;
	unsigned char set = I2C_PIN_CTL; /*SET PIN CTL mode*/
	unsigned char unset = I2C_NORMAL; /* UNSET PIN CTL MODE*/
	struct marimba config = { .mod_id =  SLAVE_ID_BAHAMA};

	if (mode == 0) {
		/* as we need to switch path to FM we need to move
		BT AUX PCM lines to PIN CONTROL mode then move
		FM to normal mode.*/
		for (reg = BT_PCM_BCLK_MODE; reg <= BT_PCM_SYNC_MODE; reg++) {
			rc = marimba_write(&config, reg, &set, 1);
			if (rc < 0) {
				pr_err("pcm pinctl failed = %d", rc);
				goto err_all;
			}
		}
		for (reg = FM_I2S_SD_MODE; reg <= FM_I2S_SCK_MODE; reg++) {
			rc = marimba_write(&config, reg, &unset, 1);
			if (rc < 0) {
				pr_err("i2s normal failed = %d", rc);
				goto err_all;
			}
		}
	} else {
		/* as we need to switch path to AUXPCM we need to move
		FM I2S lines to PIN CONTROL mode then move
		BT AUX_PCM to normal mode.*/
		for (reg = FM_I2S_SD_MODE; reg <= FM_I2S_SCK_MODE; reg++) {
			rc = marimba_write(&config, reg, &set, 1);
			if (rc < 0) {
				pr_err("i2s pinctl failed = %d", rc);
				goto err_all;
			}
		}
		for (reg = BT_PCM_BCLK_MODE; reg <= BT_PCM_SYNC_MODE; reg++) {
			rc = marimba_write(&config, reg, &unset, 1);
			if (rc < 0) {
				pr_err("pcm normal failed = %d", rc);
				goto err_all;
			}
		}
	}

	return 0;

err_all:
	return rc;
}


static void config_pcm_i2s_mode(int mode)
{
	void __iomem *cfg_ptr;
	u8 reg2;

	cfg_ptr = ioremap_nocache(FPGA_MSM_CNTRL_REG2, sizeof(char));

	if (!cfg_ptr)
		return;
	if (mode) {
		/*enable the pcm mode in FPGA*/
		reg2 = readb_relaxed(cfg_ptr);
		if (reg2 == 0) {
			reg2 = 1;
			writeb_relaxed(reg2, cfg_ptr);
		}
	} else {
		/*enable i2s mode in FPGA*/
		reg2 = readb_relaxed(cfg_ptr);
		if (reg2 == 1) {
			reg2 = 0;
			writeb_relaxed(reg2, cfg_ptr);
		}
	}
	iounmap(cfg_ptr);
}

static int config_i2s(int mode)
{
	int pin, rc = 0;

	if (mode == FM_I2S_ON) {
#ifndef CONFIG_HUAWEI_KERNEL
		if (machine_is_msm7x27a_surf() || machine_is_msm7625a_surf()
				|| machine_is_msm8625_surf())
#endif
			config_pcm_i2s_mode(0);
		pr_err("%s mode = FM_I2S_ON", __func__);

		rc = switch_pcm_i2s_reg_mode(0);
		if (rc) {
			pr_err("switch mode failed");
			return rc;
		}
		for (pin = 0; pin < ARRAY_SIZE(fm_i2s_config_power_on);
			pin++) {
				rc = gpio_tlmm_config(
					fm_i2s_config_power_on[pin],
					GPIO_CFG_ENABLE
					);
				if (rc < 0)
					return rc;
			}
	} else if (mode == FM_I2S_OFF) {
		pr_err("%s mode = FM_I2S_OFF", __func__);
		rc = switch_pcm_i2s_reg_mode(1);
		if (rc) {
			pr_err("switch mode failed");
			return rc;
		}
		for (pin = 0; pin < ARRAY_SIZE(fm_i2s_config_power_off);
			pin++) {
				rc = gpio_tlmm_config(
					fm_i2s_config_power_off[pin],
					GPIO_CFG_ENABLE
					);
				if (rc < 0)
					return rc;
			}
	}
	return rc;
}

static int config_pcm(int mode)
{
	int pin, rc = 0;

	if (mode == BT_PCM_ON) {
#ifndef CONFIG_HUAWEI_KERNEL
		if (machine_is_msm7x27a_surf() || machine_is_msm7625a_surf()
				|| machine_is_msm8625_surf())
#endif
			config_pcm_i2s_mode(1);
		pr_err("%s mode =BT_PCM_ON", __func__);
		rc = switch_pcm_i2s_reg_mode(1);
		if (rc) {
			pr_err("switch mode failed");
			return rc;
		}
		for (pin = 0; pin < ARRAY_SIZE(bt_config_pcm_on);
			pin++) {
				rc = gpio_tlmm_config(bt_config_pcm_on[pin],
					GPIO_CFG_ENABLE);
				if (rc < 0)
					return rc;
		}
	} else if (mode == BT_PCM_OFF) {
		pr_err("%s mode =BT_PCM_OFF", __func__);
		rc = switch_pcm_i2s_reg_mode(0);
		if (rc) {
			pr_err("switch mode failed");
			return rc;
		}
		for (pin = 0; pin < ARRAY_SIZE(bt_config_pcm_off);
			pin++) {
				rc = gpio_tlmm_config(bt_config_pcm_off[pin],
					GPIO_CFG_ENABLE);
				if (rc < 0)
					return rc;
		}

	}

	return rc;
}

static int msm_bahama_setup_pcm_i2s(int mode)
{
	int fm_state = 0, bt_state = 0;
	int rc = 0;
	struct marimba config = { .mod_id =  SLAVE_ID_BAHAMA};

	fm_state = marimba_get_fm_status(&config);
	bt_state = marimba_get_bt_status(&config);

	switch (mode) {
	case BT_PCM_ON:
	case BT_PCM_OFF:
		if (!fm_state)
			rc = config_pcm(mode);
		break;
	case FM_I2S_ON:
		rc = config_i2s(mode);
		break;
	case FM_I2S_OFF:
		if (bt_state)
			rc = config_pcm(BT_PCM_ON);
		else
			rc = config_i2s(mode);
		break;
	default:
		rc = -EIO;
		pr_err("%s:Unsupported mode", __func__);
	}
	return rc;
}

static int bahama_bt(int on)
{
	int rc = 0;
	int i;

	struct marimba config = { .mod_id =  SLAVE_ID_BAHAMA};

	struct bahama_variant_register {
		const size_t size;
		const struct bahama_config_register *set;
	};

	const struct bahama_config_register *p;

	u8 version;

	const struct bahama_config_register v10_bt_on[] = {
		{ 0xE9, 0x00, 0xFF },
		{ 0xF4, 0x80, 0xFF },
		{ 0xE4, 0x00, 0xFF },
		{ 0xE5, 0x00, 0x0F },
#ifdef CONFIG_WLAN
		{ 0xE6, 0x38, 0x7F },
		{ 0xE7, 0x06, 0xFF },
#endif
		{ 0xE9, 0x21, 0xFF },
		{ 0x01, 0x0C, 0x1F },
		{ 0x01, 0x08, 0x1F },
	};

	const struct bahama_config_register v20_bt_on_fm_off[] = {
		{ 0x11, 0x0C, 0xFF },
		{ 0x13, 0x01, 0xFF },
		{ 0xF4, 0x80, 0xFF },
		{ 0xF0, 0x00, 0xFF },
		{ 0xE9, 0x00, 0xFF },
#ifdef CONFIG_WLAN
		{ 0x81, 0x00, 0x7F },
		{ 0x82, 0x00, 0xFF },
		{ 0xE6, 0x38, 0x7F },
		{ 0xE7, 0x06, 0xFF },
#endif
		{ 0x8E, 0x15, 0xFF },
		{ 0x8F, 0x15, 0xFF },
		{ 0x90, 0x15, 0xFF },

		{ 0xE9, 0x21, 0xFF },
	};

	const struct bahama_config_register v20_bt_on_fm_on[] = {
		{ 0x11, 0x0C, 0xFF },
		{ 0x13, 0x01, 0xFF },
		{ 0xF4, 0x86, 0xFF },
		{ 0xF0, 0x06, 0xFF },
		{ 0xE9, 0x00, 0xFF },
#ifdef CONFIG_WLAN
		{ 0x81, 0x00, 0x7F },
		{ 0x82, 0x00, 0xFF },
		{ 0xE6, 0x38, 0x7F },
		{ 0xE7, 0x06, 0xFF },
#endif
		{ 0xE9, 0x21, 0xFF },
	};

	const struct bahama_config_register v10_bt_off[] = {
		{ 0xE9, 0x00, 0xFF },
	};

	const struct bahama_config_register v20_bt_off_fm_off[] = {
		{ 0xF4, 0x84, 0xFF },
		{ 0xF0, 0x04, 0xFF },
		{ 0xE9, 0x00, 0xFF }
	};

	const struct bahama_config_register v20_bt_off_fm_on[] = {
		{ 0xF4, 0x86, 0xFF },
		{ 0xF0, 0x06, 0xFF },
		{ 0xE9, 0x00, 0xFF }
	};

	const struct bahama_variant_register bt_bahama[2][3] = {
	{
		{ ARRAY_SIZE(v10_bt_off), v10_bt_off },
		{ ARRAY_SIZE(v20_bt_off_fm_off), v20_bt_off_fm_off },
		{ ARRAY_SIZE(v20_bt_off_fm_on), v20_bt_off_fm_on }
	},
	{
		{ ARRAY_SIZE(v10_bt_on), v10_bt_on },
		{ ARRAY_SIZE(v20_bt_on_fm_off), v20_bt_on_fm_off },
		{ ARRAY_SIZE(v20_bt_on_fm_on), v20_bt_on_fm_on }
	}
	};

	u8 offset = 0; /* index into bahama configs */
	on = on ? 1 : 0;
	version = marimba_read_bahama_ver(&config);
	if ((int)version < 0 || version == BAHAMA_VER_UNSUPPORTED) {
		dev_err(&msm_bt_power_device.dev, "%s : Bahama "
			"version read Error, version = %d\n",
			__func__, version);
		return -EIO;
	}

	if (version == BAHAMA_VER_2_0) {
		if (marimba_get_fm_status(&config))
			offset = 0x01;
	}

	p = bt_bahama[on][version + offset].set;

	dev_info(&msm_bt_power_device.dev,
		"%s: found version %d\n", __func__, version);

	for (i = 0; i < bt_bahama[on][version + offset].size; i++) {
		u8 value = (p+i)->value;
		rc = marimba_write_bit_mask(&config,
			(p+i)->reg,
			&value,
			sizeof((p+i)->value),
			(p+i)->mask);
		if (rc < 0) {
			dev_err(&msm_bt_power_device.dev,
				"%s: reg %x write failed: %d\n",
				__func__, (p+i)->reg, rc);
			return rc;
		}
		dev_dbg(&msm_bt_power_device.dev,
			"%s: reg 0x%02x write value 0x%02x mask 0x%02x\n",
				__func__, (p+i)->reg,
				value, (p+i)->mask);
		value = 0;
		rc = marimba_read_bit_mask(&config,
				(p+i)->reg, &value,
				sizeof((p+i)->value), (p+i)->mask);
		if (rc < 0)
			dev_err(&msm_bt_power_device.dev,
				"%s marimba_read_bit_mask- error",
				__func__);
		dev_dbg(&msm_bt_power_device.dev,
			"%s: reg 0x%02x read value 0x%02x mask 0x%02x\n",
				__func__, (p+i)->reg,
				value, (p+i)->mask);
	}
	/* Update BT Status */
	if (on)
		marimba_set_bt_status(&config, true);
	else
		marimba_set_bt_status(&config, false);
	return rc;
}

static int bluetooth_switch_regulators(int on)
{
	int i, rc = 0;
	const char *id = "BTPW";

	for (i = 0; i < ARRAY_SIZE(bt_vregs); i++) {
		if (IS_ERR_OR_NULL(bt_vregs[i].reg)) {
			bt_vregs[i].reg =
				regulator_get(&msm_bt_power_device.dev,
						bt_vregs[i].name);
			if (IS_ERR(bt_vregs[i].reg)) {
				rc = PTR_ERR(bt_vregs[i].reg);
				dev_err(&msm_bt_power_device.dev,
					"%s: could not get regulator %s: %d\n",
					__func__, bt_vregs[i].name, rc);
				goto reg_disable;
			}
		}

		rc = on ? regulator_set_voltage(bt_vregs[i].reg,
					bt_vregs[i].min_level,
						bt_vregs[i].max_level) : 0;
		if (rc) {
			dev_err(&msm_bt_power_device.dev,
				"%s: could not set voltage for %s: %d\n",
					__func__, bt_vregs[i].name, rc);
			goto reg_disable;
		}

		rc = on ? regulator_enable(bt_vregs[i].reg) : 0;
		if (rc) {
			dev_err(&msm_bt_power_device.dev,
				"%s: could not %sable regulator %s: %d\n",
					__func__, "en", bt_vregs[i].name, rc);
			goto reg_disable;
		}

		if (bt_vregs[i].is_pin_controlled) {
			rc = pmapp_vreg_lpm_pincntrl_vote(id,
				bt_vregs[i].pmapp_id,
					PMAPP_CLOCK_ID_D1,
					on ? PMAPP_CLOCK_VOTE_ON :
						PMAPP_CLOCK_VOTE_OFF);
			if (rc) {
				dev_err(&msm_bt_power_device.dev,
					"%s: pin control failed for %s: %d\n",
					__func__, bt_vregs[i].name, rc);
				goto pin_cnt_fail;
			}
		}
		rc = on ? 0 : regulator_disable(bt_vregs[i].reg);

		if (rc) {
			dev_err(&msm_bt_power_device.dev,
				"%s: could not %sable regulator %s: %d\n",
					__func__, "dis", bt_vregs[i].name, rc);
			goto reg_disable;
		}
	}

	return rc;
pin_cnt_fail:
	if (on)
		regulator_disable(bt_vregs[i].reg);
reg_disable:
	while (i) {
		if (on) {
			i--;
			regulator_disable(bt_vregs[i].reg);
			regulator_put(bt_vregs[i].reg);
			bt_vregs[i].reg = NULL;
		}
	}
	return rc;
}

static struct regulator *reg_s3;
static unsigned int msm_bahama_setup_power(void)
{
	int rc = 0;

	reg_s3 = regulator_get(NULL, "msme1");
	if (IS_ERR(reg_s3)) {
		rc = PTR_ERR(reg_s3);
		pr_err("%s: could not get regulator: %d\n", __func__, rc);
		goto out;
	}

	rc = regulator_set_voltage(reg_s3, 1800000, 1800000);
	if (rc < 0) {
		pr_err("%s: could not set voltage: %d\n", __func__, rc);
		goto reg_fail;
	}

	rc = regulator_enable(reg_s3);
	if (rc < 0) {
		pr_err("%s: could not enable regulator: %d\n", __func__, rc);
		goto reg_fail;
	}
#ifdef CONFIG_HUAWEI_KERNEL
    /*config gpio5 before use*/
    gpio_tlmm_config(bt_config_sys_rest[0], GPIO_CFG_ENABLE);
#else
	gpio_tlmm_config(GPIO_CFG(gpio_bt_sys_rest_en, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#endif

	/*setup Bahama_sys_reset_n*/
	rc = gpio_request(gpio_bt_sys_rest_en, "bahama sys_rst_n");
	if (rc < 0) {
		pr_err("%s: gpio_request %d = %d\n", __func__,
			gpio_bt_sys_rest_en, rc);
		goto reg_disable;
	}

	rc = bt_set_gpio(1);
	if (rc < 0) {
		pr_err("%s: bt_set_gpio %d = %d\n", __func__,
			gpio_bt_sys_rest_en, rc);
		goto gpio_fail;
	}

	return rc;

gpio_fail:
	gpio_free(gpio_bt_sys_rest_en);
reg_disable:
	regulator_disable(reg_s3);
reg_fail:
	regulator_put(reg_s3);
out:
	reg_s3 = NULL;
	return rc;
}

static unsigned int msm_bahama_shutdown_power(int value)
{
	int rc = 0;

	if (IS_ERR_OR_NULL(reg_s3)) {
		rc = reg_s3 ? PTR_ERR(reg_s3) : -ENODEV;
		goto out;
	}

	rc = regulator_disable(reg_s3);
	if (rc) {
		pr_err("%s: could not disable regulator: %d\n", __func__, rc);
		goto out;
	}

	if (value == BAHAMA_ID) {
		rc = bt_set_gpio(0);
		if (rc) {
			pr_err("%s: bt_set_gpio = %d\n",
					__func__, rc);
			goto reg_enable;
		}
		gpio_free(gpio_bt_sys_rest_en);
	}

	regulator_put(reg_s3);
	reg_s3 = NULL;

	return 0;

reg_enable:
	regulator_enable(reg_s3);
out:
	return rc;
}

static unsigned int msm_bahama_core_config(int type)
{
	int rc = 0;

	if (type == BAHAMA_ID) {
		int i;
		struct marimba config = { .mod_id =  SLAVE_ID_BAHAMA};
		const struct bahama_config_register v20_init[] = {
			/* reg, value, mask */
			{ 0xF4, 0x84, 0xFF }, /* AREG */
			{ 0xF0, 0x04, 0xFF } /* DREG */
		};
		if (marimba_read_bahama_ver(&config) == BAHAMA_VER_2_0) {
			for (i = 0; i < ARRAY_SIZE(v20_init); i++) {
				u8 value = v20_init[i].value;
				rc = marimba_write_bit_mask(&config,
					v20_init[i].reg,
					&value,
					sizeof(v20_init[i].value),
					v20_init[i].mask);
				if (rc < 0) {
					pr_err("%s: reg %d write failed: %d\n",
						__func__, v20_init[i].reg, rc);
					return rc;
				}
				pr_debug("%s: reg 0x%02x value 0x%02x"
					" mask 0x%02x\n",
					__func__, v20_init[i].reg,
					v20_init[i].value, v20_init[i].mask);
			}
		}
	}
	rc = bt_set_gpio(0);
	if (rc) {
		pr_err("%s: bt_set_gpio = %d\n",
		       __func__, rc);
	}
	pr_debug("core type: %d\n", type);
	return rc;
}

static int bluetooth_power(int on)
{
	int pin, rc = 0;
	const char *id = "BTPW";
	int cid = 0;

	cid = adie_get_detected_connectivity_type();
	if (cid != BAHAMA_ID) {
		pr_err("%s: unexpected adie connectivity type: %d\n",
					__func__, cid);
		return -ENODEV;
	}
	if (on) {
		/*setup power for BT SOC*/
		rc = bt_set_gpio(on);
		if (rc) {
			pr_err("%s: bt_set_gpio = %d\n",
					__func__, rc);
			goto exit;
		}
		rc = bluetooth_switch_regulators(on);
		if (rc < 0) {
			pr_err("%s: bluetooth_switch_regulators rc = %d",
					__func__, rc);
			goto exit;
		}
		/*setup BT GPIO lines*/
		for (pin = 0; pin < ARRAY_SIZE(bt_config_power_on);
			pin++) {
			rc = gpio_tlmm_config(bt_config_power_on[pin],
					GPIO_CFG_ENABLE);
			if (rc < 0) {
				pr_err("%s: gpio_tlmm_config(%#x)=%d\n",
						__func__,
						bt_config_power_on[pin],
						rc);
				goto fail_power;
			}
		}
		/*Setup BT clocks*/
		rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_D1,
			PMAPP_CLOCK_VOTE_ON);
		if (rc < 0) {
			pr_err("Failed to vote for TCXO_D1 ON\n");
			goto fail_clock;
		}
		msleep(20);

		/*I2C config for Bahama*/
		rc = bahama_bt(1);
		if (rc < 0) {
			pr_err("%s: bahama_bt rc = %d", __func__, rc);
			goto fail_i2c;
		}
		msleep(20);

		/*setup BT PCM lines*/
		rc = msm_bahama_setup_pcm_i2s(BT_PCM_ON);
		if (rc < 0) {
			pr_err("%s: msm_bahama_setup_pcm_i2s , rc =%d\n",
				__func__, rc);
				goto fail_power;
			}
		rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_D1,
				  PMAPP_CLOCK_VOTE_PIN_CTRL);
		if (rc < 0)
			pr_err("%s:Pin Control Failed, rc = %d",
					__func__, rc);

	} else {
		rc = bahama_bt(0);
		if (rc < 0)
			pr_err("%s: bahama_bt rc = %d", __func__, rc);

		rc = msm_bahama_setup_pcm_i2s(BT_PCM_OFF);
		if (rc < 0) {
			pr_err("%s: msm_bahama_setup_pcm_i2s, rc =%d\n",
				__func__, rc);
		}
		rc = bt_set_gpio(on);
		if (rc) {
			pr_err("%s: bt_set_gpio = %d\n",
					__func__, rc);
		}
fail_i2c:
		rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_D1,
				  PMAPP_CLOCK_VOTE_OFF);
		if (rc < 0)
			pr_err("%s: Failed to vote Off D1\n", __func__);
fail_clock:
		for (pin = 0; pin < ARRAY_SIZE(bt_config_power_off);
			pin++) {
			rc = gpio_tlmm_config(bt_config_power_off[pin],
				GPIO_CFG_ENABLE);
			if (rc < 0) {
				pr_err("%s:"
					" gpio_tlmm_config(%#x)=%d\n",
					__func__,
					bt_config_power_off[pin], rc);
			}
		}
fail_power:
		rc = bluetooth_switch_regulators(0);
		if (rc < 0) {
			pr_err("%s: switch_regulators : rc = %d",\
				__func__, rc);
			goto exit;
		}
	}
	return rc;
exit:
	pr_err("%s: failed with rc = %d", __func__, rc);
	return rc;
}

static struct marimba_fm_platform_data marimba_fm_pdata = {
	.fm_setup = fm_radio_setup,
	.fm_shutdown = fm_radio_shutdown,
	.irq = MSM_GPIO_TO_INT(FM_GPIO),
	.vreg_s2 = NULL,
	.vreg_xo_out = NULL,
	/* Configuring the FM SoC as I2S Master */
	.is_fm_soc_i2s_master = true,
	.config_i2s_gpio = msm_bahama_setup_pcm_i2s,
};

static struct marimba_platform_data marimba_pdata = {
	.slave_id[SLAVE_ID_BAHAMA_FM]	= BAHAMA_SLAVE_ID_FM_ADDR,
	.slave_id[SLAVE_ID_BAHAMA_QMEMBIST] = BAHAMA_SLAVE_ID_QMEMBIST_ADDR,
	.bahama_setup			= msm_bahama_setup_power,
	.bahama_shutdown		= msm_bahama_shutdown_power,
	.bahama_core_config		= msm_bahama_core_config,
	.fm			        = &marimba_fm_pdata,
};

static struct i2c_board_info bahama_devices[] = {
{
	I2C_BOARD_INFO("marimba", 0xc),
	.platform_data = &marimba_pdata,
},
};

void __init msm7627a_bt_power_init(void)
{
	int i, rc = 0;
	struct device *dev;


#ifndef CONFIG_HUAWEI_KERNEL
    gpio_bt_config();
#endif

	rc = i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
				bahama_devices,
				ARRAY_SIZE(bahama_devices));
	if (rc < 0) {
		pr_err("%s: I2C Register failed\n", __func__);
		return;
	}

	rc = platform_device_register(&msm_bt_power_device);
	if (rc < 0) {
		pr_err("%s: device register failed\n", __func__);
		return;
	}

	dev = &msm_bt_power_device.dev;

	for (i = 0; i < ARRAY_SIZE(bt_vregs); i++) {
		bt_vregs[i].reg = regulator_get(dev, bt_vregs[i].name);
		if (IS_ERR(bt_vregs[i].reg)) {
			rc = PTR_ERR(bt_vregs[i].reg);
			dev_err(dev, "%s: could not get regulator %s: %d\n",
					__func__, bt_vregs[i].name, rc);
			goto reg_get_fail;
		}
	}

	dev->platform_data = &bluetooth_power;

	return;

reg_get_fail:
	while (i--) {
		regulator_put(bt_vregs[i].reg);
		bt_vregs[i].reg = NULL;
	}
	platform_device_unregister(&msm_bt_power_device);
}
#endif

#endif

#if (defined(HUAWEI_BT_BTLA_VER30) && defined(CONFIG_HUAWEI_KERNEL))

#include <linux/delay.h>
#include <linux/rfkill.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/mfd/marimba.h>
#include <linux/io.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <mach/rpc_pmapp.h>
#include <linux/gpio.h>
#include <linux/hardware_self_adapt.h>
#include "board-msm7627a.h"
#include "devices-msm7x2xa.h"


/* BCM BT GPIOs config*/
#define GPIO_BT_UART_RTS   43 
#define GPIO_BT_UART_CTS   44
#define GPIO_BT_RX         45
#define GPIO_BT_TX         46

/*wake signals*/
#define GPIO_BT_WAKE_BT    107
#define GPIO_BT_WAKE_MSM   27 //bt wake msm gpio

/*control signals*/
#define GPIO_BT_SHUTDOWN_N 5
#define GPIO_BT_RESET_N    14

/*pcm signals*/
#define GPIO_BT_PCM_OUT   68 
#define GPIO_BT_PCM_IN   69
#define GPIO_BT_PCM_SYNC         70
#define GPIO_BT_PCM_CLK         71

/*gpio function*/
#define GPIO_BT_FUN_0        0
#define GPIO_BT_FUN_1        1 
#define GPIO_BT_FUN_2        2 
#define GPIO_BT_ON           1
#define GPIO_BT_OFF          0

#define VREG_S3_VOLTAGE_VALUE	1800000

/* Code for BCM4330 */
static struct platform_device msm_bt_power_device = {
    .name = "bt_power",
};


static struct resource bluesleep_resources[] = {
    {
    .name	= "gpio_host_wake",
    .start	= GPIO_BT_WAKE_MSM,
    .end	= GPIO_BT_WAKE_MSM,
    .flags	= IORESOURCE_IO,
    },
    {
    .name	= "gpio_ext_wake",
    .start	= GPIO_BT_WAKE_BT,
    .end	= GPIO_BT_WAKE_BT,
    .flags	= IORESOURCE_IO,
    },
    {
    .name	= "host_wake",
    .start	= MSM_GPIO_TO_INT(GPIO_BT_WAKE_MSM),
    .end	= MSM_GPIO_TO_INT(GPIO_BT_WAKE_MSM),
    .flags	= IORESOURCE_IRQ,
    },
};

static struct platform_device msm_bluesleep_device = {
    .name = "bluesleep",
    .id		= -1,
    .num_resources	= ARRAY_SIZE(bluesleep_resources),
    .resource	= bluesleep_resources,
};


enum {
    BT_WAKE,
    BT_RFR,
    BT_CTS,
    BT_RX,
    BT_TX,
    BT_PCM_DOUT,
    BT_PCM_DIN,
    BT_PCM_SYNC,
    BT_PCM_CLK,
    BT_HOST_WAKE,
};
/* config all msm bt gpio here!*/

static struct msm_gpio bt_config_bcm4330_power_on[] = {
    { GPIO_CFG(GPIO_BT_UART_RTS, GPIO_BT_FUN_2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,   GPIO_CFG_2MA),
        "UART1DM_RFR" },
    { GPIO_CFG(GPIO_BT_UART_CTS, GPIO_BT_FUN_2, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL,   GPIO_CFG_2MA),
        "UART1DM_CTS" },
    { GPIO_CFG(GPIO_BT_RX, GPIO_BT_FUN_2, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL,   GPIO_CFG_2MA),
        "UART1DM_Rx" },
    { GPIO_CFG(GPIO_BT_TX, GPIO_BT_FUN_2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,   GPIO_CFG_2MA),
        "UART1DM_Tx" },
    /*following 2 are the wakeup between 4330 and MSM*/
    { GPIO_CFG(GPIO_BT_WAKE_BT, GPIO_BT_FUN_0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,  GPIO_CFG_2MA ),
        "MSM_WAKE_BT"  },
    { GPIO_CFG(GPIO_BT_WAKE_MSM, GPIO_BT_FUN_0, GPIO_CFG_INPUT , GPIO_CFG_NO_PULL ,  GPIO_CFG_2MA ),	
        "BT_WAKE_MSM"  },
    /*following 4 are the PCM between 4330 and MSM*/
    { GPIO_CFG(GPIO_BT_PCM_OUT, GPIO_BT_FUN_1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	
        "PCM_DOUT" },
    { GPIO_CFG(GPIO_BT_PCM_IN, GPIO_BT_FUN_1, GPIO_CFG_INPUT,  GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	
        "PCM_DIN" },
    { GPIO_CFG(GPIO_BT_PCM_SYNC, GPIO_BT_FUN_1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	
        "PCM_SYNC" },
    { GPIO_CFG(GPIO_BT_PCM_CLK, GPIO_BT_FUN_1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	
        "PCM_CLK " }
};

static struct msm_gpio bt_config_power_control[] = {  
    /*following 2 are bt on/off control*/
    { GPIO_CFG(GPIO_BT_SHUTDOWN_N, GPIO_BT_FUN_0, GPIO_CFG_OUTPUT , GPIO_CFG_NO_PULL ,  GPIO_CFG_2MA ), 
        "BT_REG_ON"  },
    { GPIO_CFG(GPIO_BT_RESET_N, GPIO_BT_FUN_0, GPIO_CFG_OUTPUT , GPIO_CFG_NO_PULL ,  GPIO_CFG_2MA ), 
        "BT_PWR_ON"  }
};

static struct msm_gpio bt_config_bcm4330_power_off[] = {
    { GPIO_CFG(GPIO_BT_UART_RTS, GPIO_BT_FUN_0, GPIO_CFG_INPUT,  GPIO_CFG_PULL_DOWN,   GPIO_CFG_2MA),
        "UART1DM_RFR" },
    { GPIO_CFG(GPIO_BT_UART_CTS, GPIO_BT_FUN_0, GPIO_CFG_INPUT,  GPIO_CFG_PULL_DOWN,   GPIO_CFG_2MA),
        "UART1DM_CTS" },
    { GPIO_CFG(GPIO_BT_RX, GPIO_BT_FUN_0, GPIO_CFG_INPUT,  GPIO_CFG_PULL_DOWN,   GPIO_CFG_2MA),
        "UART1DM_Rx" },
    { GPIO_CFG(GPIO_BT_TX, GPIO_BT_FUN_0, GPIO_CFG_INPUT,  GPIO_CFG_PULL_DOWN,   GPIO_CFG_2MA),
        "UART1DM_Tx" },
    /*following 2 are the wakeup between 4330 and MSM*/
    { GPIO_CFG(GPIO_BT_WAKE_BT, GPIO_BT_FUN_0, GPIO_CFG_INPUT , GPIO_CFG_PULL_DOWN ,  GPIO_CFG_2MA ),
        "MSM_WAKE_BT"  },
    { GPIO_CFG(GPIO_BT_WAKE_MSM, GPIO_BT_FUN_0, GPIO_CFG_INPUT , GPIO_CFG_PULL_DOWN ,  GPIO_CFG_2MA ),	
        "BT_WAKE_MSM"  },
    /*following 4 are the PCM between 4330 and MSM*/
    { GPIO_CFG(GPIO_BT_PCM_OUT, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), 
        "PCM_DOUT" },
    { GPIO_CFG(GPIO_BT_PCM_IN, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
        "PCM_DIN" },
    { GPIO_CFG(GPIO_BT_PCM_SYNC, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
        "PCM_SYNC" },
    { GPIO_CFG(GPIO_BT_PCM_CLK, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
     	"PCM_CLK" }
	};



/* configure all bt power here! */
static const char *vregs_bt_bcm4330_name[] = {
    "msme1"
};
/*static const char *vregs_bt_bcm4330_name[] = {
    "s3"
};*/


static struct regulator *vregs_bt_bcm4330[ARRAY_SIZE(vregs_bt_bcm4330_name)];

/* put power on for bt*/

static int bluetooth_bcm4330_power_regulators(int on)
{
    int i = 0;
    int rc = 0;

    for (i = 0; i < ARRAY_SIZE(vregs_bt_bcm4330_name); i++) {
        rc = on ? regulator_enable(vregs_bt_bcm4330[i]) :
            regulator_disable(vregs_bt_bcm4330[i]);
        if (rc < 0) {
        printk(KERN_ERR "%s: vreg %s %s failed (%d)\n",
            __func__, vregs_bt_bcm4330_name[i],
    			       on ? "enable" : "disable", rc);
        return -EIO;
        }
    }

    /*gpio power for bcm4330*/
    if(on)
    {

        rc = gpio_direction_output(GPIO_BT_SHUTDOWN_N, GPIO_BT_ON);  /*bt_reg_on off on :5 -->1*/
        if (rc) 
        {
            printk(KERN_ERR "%s:  bt power1 on fail (%d)\n",
                   __func__, rc);
            return -EIO;
        }
          
        mdelay(1);
        rc = gpio_direction_output(GPIO_BT_RESET_N, GPIO_BT_ON);  /*bt_pwr_on  on:14 -->1*/
        if (rc) 
        {
            printk(KERN_ERR "%s:  bt power2 off fail (%d)\n",
                   __func__, rc);
            return -EIO;
        }


    }
    else
    {
        rc = gpio_direction_output(GPIO_BT_RESET_N, GPIO_BT_OFF);  /*bt_pwr_on off:14 -->0*/
        if (rc) 
        {
            printk(KERN_ERR "%s:  bt power2 off fail (%d)\n",
                   __func__, rc);
            return -EIO;
        }
        mdelay(1);

        rc = gpio_direction_output(GPIO_BT_SHUTDOWN_N, GPIO_BT_OFF);  /*bt_reg_on :5 -->0*/
        if (rc) 
        {
            printk(KERN_ERR "%s:  bt power1 off fail (%d)\n",
                   __func__, rc);
            return -EIO;
        }
        mdelay(1);

    }		
    return 0;
}
	

static int bluetooth_bcm4330_power(int on)
{
    int rc = 0;

    if (on)
    {
        /* msm: config the msm  bt gpios*/
        rc = msm_gpios_enable(bt_config_bcm4330_power_on,
            ARRAY_SIZE(bt_config_bcm4330_power_on));
        if (rc < 0)
        {
            printk(KERN_ERR "%s: bcm4330_config_gpio on failed (%d)\n",
                __func__, rc);
            return rc;
        }

        rc = bluetooth_bcm4330_power_regulators(on);
        if (rc < 0) 
        {
            printk(KERN_ERR "%s: bcm4330_power_regulators on failed (%d)\n",
                __func__, rc);
            return rc;
        }
    }
    else
    {
        /* msm: config the msm  bt gpios*/
        rc = msm_gpios_enable(bt_config_bcm4330_power_off,
            ARRAY_SIZE(bt_config_bcm4330_power_off));
        if (rc < 0)
        {
            printk(KERN_ERR "%s: bcm4330_config_gpio on failed (%d)\n",
                __func__, rc);
            return rc;
        }

        /* check for initial rfkill block (power off) */
        if (platform_get_drvdata(&msm_bt_power_device) == NULL)
        {
            printk(KERN_DEBUG "bluetooth rfkill block error : \n");
            goto out;
        }
      
        rc = bluetooth_bcm4330_power_regulators(on);
        if (rc < 0) 
        {
            printk(KERN_ERR "%s: bcm4330_power_regulators off failed (%d)\n",
                __func__, rc);
            return rc;
        }
       


    }	
out:
    printk(KERN_DEBUG "Bluetooth BCM4330 power switch: %d\n", on);

    return 0;
}


	
static void bt_bcm4330_power_init(void)
{
    /*here will check the power, */
    int i = 0;
    int rc = -1;

    printk(KERN_ERR "bt_bcm4330_power_init pre\n");
	rc = platform_device_register(&msm_bt_power_device);
	if (rc < 0) {
		pr_err("%s: btla-power device register failed\n", __func__);
		return;
	}

	rc = platform_device_register(&msm_bluesleep_device);
	if (rc < 0) {
		pr_err("%s: btla-bluesleep device register failed\n", __func__);
		return;
	}
		
    for (i = 0; i < ARRAY_SIZE(vregs_bt_bcm4330_name); i++)
    {
        vregs_bt_bcm4330[i] = regulator_get(NULL, vregs_bt_bcm4330_name[i]);
        if (IS_ERR(vregs_bt_bcm4330[i]))
        {
            printk(KERN_ERR "%s: vreg get %s failed (%ld)\n",
                __func__, vregs_bt_bcm4330_name[i],
                PTR_ERR(vregs_bt_bcm4330_name[i]));
        	goto btla_reg_get_fail;
        }
        rc = regulator_set_voltage(vregs_bt_bcm4330[i], VREG_S3_VOLTAGE_VALUE, VREG_S3_VOLTAGE_VALUE);
		if (rc) {
		    printk("%s: regulator_s3  regulator_set_voltage failed\n", __func__);
            return;
	    }
    }
 
    /* handle bt power control: becareful */
    rc = msm_gpios_request_enable(bt_config_power_control,
                            ARRAY_SIZE(bt_config_power_control));
    if (rc < 0) {
            printk(KERN_ERR
                    "%s: bt power control request_enable failed (%d)\n",
                            __func__, rc);
            return;
    }
    
    rc = gpio_direction_output(GPIO_BT_RESET_N, GPIO_BT_OFF);  /*bt_pwr_on off:14 -->0*/
    if (rc) 
    {
        printk(KERN_ERR "%s:  bt power2 off fail (%d)\n",
               __func__, rc);
        return ;
    }
    mdelay(1);

    rc = gpio_direction_output(GPIO_BT_SHUTDOWN_N, GPIO_BT_OFF);  /*bt_reg_on :5 -->0*/
    if (rc) 
    {
        printk(KERN_ERR "%s:  bt power1 off fail (%d)\n",
               __func__, rc);
        return ;
    }
    mdelay(1);


    printk(KERN_ERR "bt_bcm4330_power_init after\n");
    /*config platform_data*/
    msm_bt_power_device.dev.platform_data = &bluetooth_bcm4330_power;

	return;

btla_reg_get_fail:
    /* disable following 3 lines as error is report up
	while (i--) {
		regulator_put(vregs_bt_bcm4330_name[i].reg);
		vregs_bt_bcm4330_name[i].reg = NULL;
	} 
    */

	platform_device_unregister(&msm_bt_power_device);
	platform_device_unregister(&msm_bluesleep_device);
	
}

void __init msm7627a_bt_power_init(void)
{
    /*keep the same interface with QComm base line*/
    bt_bcm4330_power_init();
}
/*move static struct resource bluesleep_resources[] to up*/


/*move static struct platform_device msm_bluesleep_device  to up*/

void bt_wake_msm_config(void)
{
    /*distinguish the bt_wake_msm gpio by get_hw_bt_wakeup_gpio_type*/
    hw_bt_wakeup_gpio_type bt_wake_msm_gpio =  get_hw_bt_wakeup_gpio_type();
    if( bt_wake_msm_gpio == HW_BT_WAKEUP_GPIO_IS_83)
    {
        bluesleep_resources[0].start = bt_wake_msm_gpio;
        bluesleep_resources[0].end = bt_wake_msm_gpio;
        bluesleep_resources[2].start = MSM_GPIO_TO_INT(bt_wake_msm_gpio);
        bluesleep_resources[2].end = MSM_GPIO_TO_INT(bt_wake_msm_gpio);
        bt_config_bcm4330_power_on[5].gpio_cfg = GPIO_CFG(bt_wake_msm_gpio, GPIO_BT_FUN_0, GPIO_CFG_INPUT , GPIO_CFG_NO_PULL ,  GPIO_CFG_2MA );
        bt_config_bcm4330_power_off[5].gpio_cfg = GPIO_CFG(bt_wake_msm_gpio, GPIO_BT_FUN_0, GPIO_CFG_INPUT , GPIO_CFG_PULL_DOWN ,  GPIO_CFG_2MA );
    }
    printk(KERN_DEBUG "bt_wake_msm_gpio = %d\n", bt_wake_msm_gpio); 
}
#endif
