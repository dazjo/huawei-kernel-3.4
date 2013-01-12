/* Copyright (c) 2010-2011, Code Aurora Forum. All rights reserved.
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
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/mfd/msm-adie-codec.h>
#include <linux/uaccess.h>
#include <asm/mach-types.h>
#include <mach/qdsp5v2/aux_pcm.h>
#include <mach/qdsp5v2/snddev_ecodec.h>
#include <mach/board.h>
#include <mach/qdsp5v2/snddev_icodec.h>
#include <mach/qdsp5v2/snddev_mi2s.h>
#include <mach/qdsp5v2/mi2s.h>
#include <mach/qdsp5v2/audio_acdb_def.h>
#include <mach/qdsp5v2/snddev_virtual.h>
#include "timpani_profile_7x30.h"
#include <mach/qdsp5v2/audio_dev_ctl.h>
#include <asm-arm/huawei/smem_vendor_huawei.h>
#include "../smd_private.h"
/* define the value for BT_SCO */
#define BT_SCO_PCM_CTL_VAL (PCM_CTL__RPCM_WIDTH__LINEAR_V |\
		PCM_CTL__TPCM_WIDTH__LINEAR_V)
#define BT_SCO_DATA_FORMAT_PADDING (DATA_FORMAT_PADDING_INFO__RPCM_FORMAT_V |\
		DATA_FORMAT_PADDING_INFO__TPCM_FORMAT_V)
#define BT_SCO_AUX_CODEC_INTF   AUX_CODEC_INTF_CTL__PCMINTF_DATA_EN_V

extern struct platform_device *snd_devices_c8860[];
extern int arraysize_c8860;

#ifdef CONFIG_DEBUG_FS
static struct dentry *debugfs_hsed_config;
static void snddev_hsed_config_modify_setting(int type);
static void snddev_hsed_config_restore_setting(void);
#endif

static struct adie_codec_action_unit iearpiece_ffa_48KHz_osr256_actions[] =
	EAR_PRI_MONO_8000_OSR_256; /* 8000 profile also works for 48k */

static struct adie_codec_hwsetting_entry iearpiece_ffa_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iearpiece_ffa_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(iearpiece_ffa_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile iearpiece_ffa_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = iearpiece_ffa_settings,
	.setting_sz = ARRAY_SIZE(iearpiece_ffa_settings),
};

static struct snddev_icodec_data snddev_iearpiece_ffa_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_SPKR,
	.profile = &iearpiece_ffa_profile,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.property = SIDE_TONE_MASK,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -1400,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2900,
};

static struct platform_device msm_iearpiece_ffa_device = {
	.name = "snddev_icodec",
	.id = 19,
	.dev = { .platform_data = &snddev_iearpiece_ffa_data },
};

static struct adie_codec_action_unit imic_ffa_48KHz_osr256_actions[] =
	AMIC_PRI_MONO_8000_OSR_256; /* 8000 profile also works for 48k */

static struct adie_codec_hwsetting_entry imic_ffa_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = imic_ffa_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(imic_ffa_48KHz_osr256_actions),
	}
};

static enum hsed_controller imic_pmctl_id[] = {PM_HSED_CONTROLLER_0};

static struct adie_codec_dev_profile imic_ffa_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = imic_ffa_settings,
	.setting_sz = ARRAY_SIZE(imic_ffa_settings),
};

static struct snddev_icodec_data snddev_imic_ffa_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC,
	.profile = &imic_ffa_profile,
	.channel_mode = 1,
	.pmctl_id = imic_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(imic_pmctl_id),
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_imic_ffa_device = {
	.name = "snddev_icodec",
	.id = 20,
	.dev = { .platform_data = &snddev_imic_ffa_data },
};

static struct adie_codec_action_unit ispkr_stereo_48KHz_osr256_actions[] =
	SPEAKER_PRI_STEREO_48000_OSR_256;

static struct adie_codec_hwsetting_entry ispkr_stereo_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ispkr_stereo_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispkr_stereo_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ispkr_stereo_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ispkr_stereo_settings,
	.setting_sz = ARRAY_SIZE(ispkr_stereo_settings),
};

static struct snddev_icodec_data snddev_ispkr_stereo_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_stereo_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_STEREO,
	.profile = &ispkr_stereo_profile,
	.channel_mode = 2,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_poweramp_on,
	.pamp_off = msm_snddev_poweramp_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_NB_INDEX] = -500,
	.max_voice_rx_vol[VOC_WB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_WB_INDEX] = -500
};

static struct platform_device msm_ispkr_stereo_device = {
	.name = "snddev_icodec",
	.id = 8,
	.dev = { .platform_data = &snddev_ispkr_stereo_data },
};

static struct adie_codec_action_unit iheadset_mic_tx_osr256_actions[] =
	AMIC1_HEADSET_TX_MONO_PRIMARY_OSR256;

static struct adie_codec_hwsetting_entry iheadset_mic_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iheadset_mic_tx_osr256_actions,
		.action_sz = ARRAY_SIZE(iheadset_mic_tx_osr256_actions),
	}
};

static struct adie_codec_dev_profile iheadset_mic_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = iheadset_mic_tx_settings,
	.setting_sz = ARRAY_SIZE(iheadset_mic_tx_settings),
};

static struct snddev_icodec_data snddev_headset_mic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "headset_mono_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_MIC,
	.profile = &iheadset_mic_profile,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device msm_headset_mic_device = {
	.name = "snddev_icodec",
	.id = 6,
	.dev = { .platform_data = &snddev_headset_mic_data },
};

static struct snddev_mi2s_data snddev_mi2s_fm_tx_data = {
	.capability = SNDDEV_CAP_TX ,
	.name = "fmradio_stereo_tx",
	.copp_id = 2,
	.acdb_id = ACDB_ID_FM_TX,
	.channel_mode = 2,
	.sd_lines = MI2S_SD_3,
	.route = NULL,
	.deroute = NULL,
	.default_sample_rate = 48000,
};

static struct platform_device  msm_snddev_mi2s_fm_tx_device = {
	.name = "snddev_mi2s",
	.id = 1,
	.dev = { .platform_data = &snddev_mi2s_fm_tx_data},
};

static struct snddev_mi2s_data snddev_mi2s_fm_rx_data = {
	.capability = SNDDEV_CAP_RX ,
	.name = "fmradio_stereo_rx",
	.copp_id = 3,
	.acdb_id = ACDB_ID_FM_RX,
	.channel_mode = 2,
	.sd_lines = MI2S_SD_3,
	.route = NULL,
	.deroute = NULL,
	.default_sample_rate = 48000,
};

static struct platform_device  msm_snddev_mi2s_fm_rx_device = {
	.name = "snddev_mi2s",
	.id = 2,
	.dev = { .platform_data = &snddev_mi2s_fm_rx_data},
};

static struct snddev_ecodec_data snddev_bt_sco_earpiece_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "bt_sco_rx",
	.copp_id = 1,
	.acdb_id = ACDB_ID_BT_SCO_SPKR,
	.channel_mode = 1,
	.conf_pcm_ctl_val = BT_SCO_PCM_CTL_VAL,
	.conf_aux_codec_intf = BT_SCO_AUX_CODEC_INTF,
	.conf_data_format_padding_val = BT_SCO_DATA_FORMAT_PADDING,
	.max_voice_rx_vol[VOC_NB_INDEX] = 400,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1100,
	.max_voice_rx_vol[VOC_WB_INDEX] = 400,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1100,
};

static struct snddev_ecodec_data snddev_bt_sco_mic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "bt_sco_tx",
	.copp_id = 1,
	.acdb_id = ACDB_ID_BT_SCO_MIC,
	.channel_mode = 1,
	.conf_pcm_ctl_val = BT_SCO_PCM_CTL_VAL,
	.conf_aux_codec_intf = BT_SCO_AUX_CODEC_INTF,
	.conf_data_format_padding_val = BT_SCO_DATA_FORMAT_PADDING,
};

static struct platform_device msm_bt_sco_earpiece_device = {
	.name = "msm_snddev_ecodec",
	.id = 0,
	.dev = { .platform_data = &snddev_bt_sco_earpiece_data },
};

static struct platform_device msm_bt_sco_mic_device = {
	.name = "msm_snddev_ecodec",
	.id = 1,
	.dev = { .platform_data = &snddev_bt_sco_mic_data },
};

static struct adie_codec_action_unit headset_ab_cpls_48KHz_osr256_actions[] =
	HEADSET_AB_CPLS_48000_OSR_256;

static struct adie_codec_hwsetting_entry headset_ab_cpls_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = headset_ab_cpls_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(headset_ab_cpls_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile headset_ab_cpls_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = headset_ab_cpls_settings,
	.setting_sz = ARRAY_SIZE(headset_ab_cpls_settings),
};

static struct snddev_icodec_data snddev_ihs_stereo_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_stereo_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_SPKR_STEREO,
	.profile = &headset_ab_cpls_profile,
	.channel_mode = 2,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.property = SIDE_TONE_MASK,
	.voltage_on = msm_snddev_hsed_voltage_on,
	.voltage_off = msm_snddev_hsed_voltage_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -900,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2400,
};

static struct platform_device msm_headset_stereo_device = {
	.name = "snddev_icodec",
	.id = 2,
	.dev = { .platform_data = &snddev_ihs_stereo_rx_data },
};

/*debug FS interface is exposed to test Class D and class AB mode
 * amplifers for headset device folloowing options are supported
 * 0 -> settings will be restored
 * 1 -> Cladd D mode is selected
 * 2 -> Class AB mode is selected
*/
#ifdef CONFIG_DEBUG_FS
static struct adie_codec_action_unit
	ihs_stereo_rx_class_d_legacy_48KHz_osr256_actions[] =
	HPH_PRI_D_LEG_STEREO;

static struct adie_codec_hwsetting_entry
	ihs_stereo_rx_class_d_legacy_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions =
		ihs_stereo_rx_class_d_legacy_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE
		(ihs_stereo_rx_class_d_legacy_48KHz_osr256_actions),
	}
};

static struct adie_codec_action_unit
	ihs_stereo_rx_class_ab_legacy_48KHz_osr256_actions[] =
	HPH_PRI_AB_LEG_STEREO;

static struct adie_codec_hwsetting_entry
	ihs_stereo_rx_class_ab_legacy_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions =
		ihs_stereo_rx_class_ab_legacy_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE
		(ihs_stereo_rx_class_ab_legacy_48KHz_osr256_actions),
	}
};

static void snddev_hsed_config_modify_setting(int type)
{
	struct platform_device *device;
	struct snddev_icodec_data *icodec_data;

	device = &msm_headset_stereo_device;
	icodec_data = (struct snddev_icodec_data *)device->dev.platform_data;

	if (icodec_data) {
		if (type == 1) {
			icodec_data->voltage_on = NULL;
			icodec_data->voltage_off = NULL;
			icodec_data->profile->settings =
				ihs_stereo_rx_class_d_legacy_settings;
			icodec_data->profile->setting_sz =
			ARRAY_SIZE(ihs_stereo_rx_class_d_legacy_settings);
		} else if (type == 2) {
			icodec_data->voltage_on = NULL;
			icodec_data->voltage_off = NULL;
			icodec_data->profile->settings =
				ihs_stereo_rx_class_ab_legacy_settings;
			icodec_data->profile->setting_sz =
			ARRAY_SIZE(ihs_stereo_rx_class_ab_legacy_settings);
		}
	}
}

static void snddev_hsed_config_restore_setting(void)
{
	struct platform_device *device;
	struct snddev_icodec_data *icodec_data;

	device = &msm_headset_stereo_device;
	icodec_data = device->dev.platform_data;

	if (icodec_data) {
		icodec_data->voltage_on = msm_snddev_hsed_voltage_on;
		icodec_data->voltage_off = msm_snddev_hsed_voltage_off;
		icodec_data->profile->settings = headset_ab_cpls_settings;
		icodec_data->profile->setting_sz =
			ARRAY_SIZE(headset_ab_cpls_settings);
	}
}

static ssize_t snddev_hsed_config_debug_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	char *lb_str = filp->private_data;
	char cmd;

	if (get_user(cmd, ubuf))
		return -EFAULT;

	if (!strcmp(lb_str, "msm_hsed_config")) {
		switch (cmd) {
		case '0':
			snddev_hsed_config_restore_setting();
			break;

		case '1':
			snddev_hsed_config_modify_setting(1);
			break;

		case '2':
			snddev_hsed_config_modify_setting(2);
			break;

		default:
			break;
		}
	}
	return cnt;
}

static int snddev_hsed_config_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static const struct file_operations snddev_hsed_config_debug_fops = {
	.open = snddev_hsed_config_debug_open,
	.write = snddev_hsed_config_debug_write
};
#endif

static enum hsed_controller ispk_pmctl_id[] = {PM_HSED_CONTROLLER_0};

static struct snddev_icodec_data snddev_ispkr_mic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_mono_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MIC,
	.profile = &imic_ffa_profile,
	.channel_mode = 1,
	.pmctl_id = ispk_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(ispk_pmctl_id),
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device msm_ispkr_mic_device = {
	.name = "snddev_icodec",
	.id = 18,
	.dev = { .platform_data = &snddev_ispkr_mic_data },
};

static struct adie_codec_action_unit idual_mic_endfire_8KHz_osr256_actions[] =
	AMIC_DUAL_8000_OSR_256;

static struct adie_codec_hwsetting_entry idual_mic_endfire_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = idual_mic_endfire_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idual_mic_endfire_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = idual_mic_endfire_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idual_mic_endfire_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 48KHz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = idual_mic_endfire_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idual_mic_endfire_8KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile idual_mic_endfire_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = idual_mic_endfire_settings,
	.setting_sz = ARRAY_SIZE(idual_mic_endfire_settings),
};

static enum hsed_controller idual_mic_endfire_pmctl_id[] = {
	PM_HSED_CONTROLLER_0, PM_HSED_CONTROLLER_2
};

static struct snddev_icodec_data snddev_idual_mic_endfire_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_dual_mic_endfire_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC_ENDFIRE,
	.profile = &idual_mic_endfire_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_endfire_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_endfire_pmctl_id),
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_idual_mic_endfire_device = {
	.name = "snddev_icodec",
	.id = 12,
	.dev = { .platform_data = &snddev_idual_mic_endfire_data },
};

static struct snddev_icodec_data snddev_spk_idual_mic_endfire_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_dual_mic_endfire_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MIC_ENDFIRE,
	.profile = &idual_mic_endfire_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_endfire_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_endfire_pmctl_id),
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_spk_idual_mic_endfire_device = {
	.name = "snddev_icodec",
	.id = 14,
	.dev = { .platform_data = &snddev_spk_idual_mic_endfire_data },
};

static struct adie_codec_action_unit itty_mono_tx_actions[] =
	TTY_HEADSET_MONO_TX_8000_OSR_256;

static struct adie_codec_hwsetting_entry itty_mono_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = itty_mono_tx_actions,
		.action_sz = ARRAY_SIZE(itty_mono_tx_actions),
	},
};

static struct adie_codec_dev_profile itty_mono_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = itty_mono_tx_settings,
	.setting_sz = ARRAY_SIZE(itty_mono_tx_settings),
};

static struct snddev_icodec_data snddev_itty_mono_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY),
	.name = "tty_headset_mono_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_TTY_HEADSET_MIC,
	.profile = &itty_mono_tx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_itty_mono_tx_device = {
	.name = "snddev_icodec",
	.id = 16,
	.dev = { .platform_data = &snddev_itty_mono_tx_data },
};

static struct adie_codec_action_unit itty_mono_rx_actions[] =
	TTY_HEADSET_MONO_RX_8000_OSR_256;

static struct adie_codec_hwsetting_entry itty_mono_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = itty_mono_rx_actions,
		.action_sz = ARRAY_SIZE(itty_mono_rx_actions),
	},
};

static struct adie_codec_dev_profile itty_mono_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = itty_mono_rx_settings,
	.setting_sz = ARRAY_SIZE(itty_mono_rx_settings),
};

static struct snddev_icodec_data snddev_itty_mono_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY),
	.name = "tty_headset_mono_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_TTY_HEADSET_SPKR,
	.profile = &itty_mono_rx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.max_voice_rx_vol[VOC_NB_INDEX] = 0,
	.min_voice_rx_vol[VOC_NB_INDEX] = 0,
	.max_voice_rx_vol[VOC_WB_INDEX] = 0,
	.min_voice_rx_vol[VOC_WB_INDEX] = 0,
};

static struct platform_device msm_itty_mono_rx_device = {
	.name = "snddev_icodec",
	.id = 17,
	.dev = { .platform_data = &snddev_itty_mono_rx_data },
};

static struct snddev_virtual_data snddev_a2dp_tx_data = {
	.capability = SNDDEV_CAP_TX,
	.name = "a2dp_tx",
	.copp_id = 5,
	.acdb_id = PSEUDO_ACDB_ID,
};

static struct snddev_virtual_data snddev_a2dp_rx_data = {
	.capability = SNDDEV_CAP_RX,
	.name = "a2dp_rx",
	.copp_id = 2,
	.acdb_id = PSEUDO_ACDB_ID,
};

static struct platform_device msm_a2dp_rx_device = {
	.name = "snddev_virtual",
	.id = 0,
	.dev = { .platform_data = &snddev_a2dp_rx_data },
};

static struct platform_device msm_a2dp_tx_device = {
	.name = "snddev_virtual",
	.id = 1,
	.dev = { .platform_data = &snddev_a2dp_tx_data },
};

static struct snddev_virtual_data snddev_uplink_rx_data = {
	.capability = SNDDEV_CAP_RX,
	.name = "uplink_rx",
	.copp_id = 5,
	.acdb_id = PSEUDO_ACDB_ID,
};

static struct platform_device msm_uplink_rx_device = {
	.name = "snddev_virtual",
	.id = 2,
	.dev = { .platform_data = &snddev_uplink_rx_data },
};

static struct snddev_icodec_data\
		snddev_idual_mic_endfire_real_stereo_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_dual_mic_endfire_tx_real_stereo",
	.copp_id = 0,
	.acdb_id = PSEUDO_ACDB_ID,
	.profile = &idual_mic_endfire_profile,
	.channel_mode = REAL_STEREO_CHANNEL_MODE,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_endfire_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_endfire_pmctl_id),
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_real_stereo_tx_device = {
	.name = "snddev_icodec",
	.id = 26,
	.dev = { .platform_data =
			&snddev_idual_mic_endfire_real_stereo_data },
};

static struct adie_codec_action_unit ihs_ffa_mono_rx_48KHz_osr256_actions[] =
	HEADSET_RX_CAPLESS_48000_OSR_256;

static struct adie_codec_hwsetting_entry ihs_ffa_mono_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ihs_ffa_mono_rx_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ihs_ffa_mono_rx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ihs_ffa_mono_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ihs_ffa_mono_rx_settings,
	.setting_sz = ARRAY_SIZE(ihs_ffa_mono_rx_settings),
};

static struct snddev_icodec_data snddev_ihs_ffa_mono_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_mono_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_SPKR_MONO,
	.profile = &ihs_ffa_mono_rx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_hsed_voltage_on,
	.pamp_off = msm_snddev_hsed_voltage_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -900,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2400,
	.property = SIDE_TONE_MASK,
};

static struct platform_device msm_ihs_ffa_mono_rx_device = {
	.name = "snddev_icodec",
	.id = 5,
	.dev = { .platform_data = &snddev_ihs_ffa_mono_rx_data },
};

static struct adie_codec_action_unit
	ihs_stereo_speaker_stereo_rx_48KHz_osr256_actions[] =
	HEADSET_STEREO_SPEAKER_STEREO_RX_CAPLESS_48000_OSR_256;


static struct adie_codec_hwsetting_entry
	ihs_stereo_speaker_stereo_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ihs_stereo_speaker_stereo_rx_48KHz_osr256_actions,
		.action_sz =
		ARRAY_SIZE(ihs_stereo_speaker_stereo_rx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ihs_stereo_speaker_stereo_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ihs_stereo_speaker_stereo_rx_settings,
	.setting_sz = ARRAY_SIZE(ihs_stereo_speaker_stereo_rx_settings),
};

/* acdb_id from ACDB_ID_HEADSET_STEREO_PLUS_SPKR_STEREO_RX to ACDB_ID_HEADSET_STEREO_PLUS_SPKR_MONO_RX */
/* set msm_snddev_poweramp_4music_on as pamp_on func */
static struct snddev_icodec_data snddev_ihs_stereo_speaker_stereo_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_stereo_speaker_stereo_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_STEREO_PLUS_SPKR_MONO_RX,
	.profile = &ihs_stereo_speaker_stereo_rx_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_poweramp_4music_on, 
	.pamp_off = msm_snddev_poweramp_off,
	.voltage_on = msm_snddev_hsed_voltage_on,
	.voltage_off = msm_snddev_hsed_voltage_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = -500,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2000,
	.max_voice_rx_vol[VOC_WB_INDEX] = -900,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2400,
};

static struct platform_device msm_ihs_stereo_speaker_stereo_rx_device = {
	.name = "snddev_icodec",
	.id = 21,
	.dev = { .platform_data = &snddev_ihs_stereo_speaker_stereo_rx_data },
};

static struct adie_codec_action_unit ispk_dual_mic_bs_8KHz_osr256_actions[] =
	HS_DMIC2_STEREO_8000_OSR_256;

static struct adie_codec_hwsetting_entry ispk_dual_mic_bs_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = ispk_dual_mic_bs_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispk_dual_mic_bs_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 16Khz */
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = ispk_dual_mic_bs_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispk_dual_mic_bs_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 48KHz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ispk_dual_mic_bs_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispk_dual_mic_bs_8KHz_osr256_actions),
	},
};

static enum hsed_controller idual_mic_broadside_pmctl_id[] = {
	PM_HSED_CONTROLLER_0, PM_HSED_CONTROLLER_2
};

static struct adie_codec_dev_profile ispk_dual_mic_bs_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = ispk_dual_mic_bs_settings,
	.setting_sz = ARRAY_SIZE(ispk_dual_mic_bs_settings),
};
static struct snddev_icodec_data snddev_spk_idual_mic_broadside_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_dual_mic_broadside_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MIC_BROADSIDE,
	.profile = &ispk_dual_mic_bs_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_broadside_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_broadside_pmctl_id),
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_spk_idual_mic_broadside_device = {
	.name = "snddev_icodec",
	.id = 15,
	.dev = { .platform_data = &snddev_spk_idual_mic_broadside_data },
};

static struct adie_codec_action_unit idual_mic_bs_8KHz_osr256_actions[] =
	HS_DMIC2_STEREO_8000_OSR_256;

static struct adie_codec_hwsetting_entry idual_mic_broadside_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile idual_mic_broadside_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = idual_mic_broadside_settings,
	.setting_sz = ARRAY_SIZE(idual_mic_broadside_settings),
};

static struct snddev_icodec_data snddev_idual_mic_broadside_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_dual_mic_broadside_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC_BROADSIDE,
	.profile = &idual_mic_broadside_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_broadside_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_broadside_pmctl_id),
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_idual_mic_broadside_device = {
	.name = "snddev_icodec",
	.id = 13,
	.dev = { .platform_data = &snddev_idual_mic_broadside_data },
};

static struct snddev_mi2s_data snddev_mi2s_stereo_rx_data = {
	.capability = SNDDEV_CAP_RX ,
	.name = "hdmi_stereo_rx",
	.copp_id = 3,
	.acdb_id = ACDB_ID_HDMI,
	.channel_mode = 2,
	.sd_lines = MI2S_SD_0,
	.route = msm_snddev_tx_route_config,
	.deroute = msm_snddev_tx_route_deconfig,
	.default_sample_rate = 48000,
};

static struct platform_device msm_snddev_mi2s_stereo_rx_device = {
	.name = "snddev_mi2s",
	.id = 0,
	.dev = { .platform_data = &snddev_mi2s_stereo_rx_data },
};

static struct adie_codec_action_unit auxpga_lb_lo_actions[] =
	LB_AUXPGA_LO_STEREO;

static struct adie_codec_hwsetting_entry auxpga_lb_lo_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = auxpga_lb_lo_actions,
		.action_sz = ARRAY_SIZE(auxpga_lb_lo_actions),
	},
};

static struct adie_codec_dev_profile auxpga_lb_lo_profile = {
	.path_type = ADIE_CODEC_LB,
	.settings = auxpga_lb_lo_settings,
	.setting_sz = ARRAY_SIZE(auxpga_lb_lo_settings),
};

static struct snddev_icodec_data snddev_auxpga_lb_lo_data = {
	.capability = SNDDEV_CAP_LB,
	.name = "auxpga_loopback_lo",
	.copp_id = 0,
	.acdb_id = PSEUDO_ACDB_ID,
	.profile = &auxpga_lb_lo_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_poweramp_on,
	.pamp_off = msm_snddev_poweramp_off,
	.dev_vol_type = SNDDEV_DEV_VOL_ANALOG,
};

static struct platform_device msm_auxpga_lb_lo_device = {
	.name = "snddev_icodec",
	.id = 27,
	.dev = { .platform_data = &snddev_auxpga_lb_lo_data },
};

static struct adie_codec_action_unit auxpga_lb_hs_actions[] =
	LB_AUXPGA_HPH_AB_CPLS_STEREO;

static struct adie_codec_hwsetting_entry auxpga_lb_hs_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = auxpga_lb_hs_actions,
		.action_sz = ARRAY_SIZE(auxpga_lb_hs_actions),
	},
};

static struct adie_codec_dev_profile auxpga_lb_hs_profile = {
	.path_type = ADIE_CODEC_LB,
	.settings = auxpga_lb_hs_settings,
	.setting_sz = ARRAY_SIZE(auxpga_lb_hs_settings),
};

static struct snddev_icodec_data snddev_auxpga_lb_hs_data = {
	.capability = SNDDEV_CAP_LB,
	.name = "auxpga_loopback_hs",
	.copp_id = 0,
	.acdb_id = PSEUDO_ACDB_ID,
	.profile = &auxpga_lb_hs_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.voltage_on = msm_snddev_hsed_voltage_on,
	.voltage_off = msm_snddev_hsed_voltage_off,
	.dev_vol_type = SNDDEV_DEV_VOL_ANALOG,
};

static struct platform_device msm_auxpga_lb_hs_device = {
	.name = "snddev_icodec",
	.id = 25,
	.dev = { .platform_data = &snddev_auxpga_lb_hs_data },
};

/****************************************************************/
/**            HUAWEI ADDED DEVICE                             **/
/****************************************************************/
#ifdef CONFIG_HUAWEI_KERNEL
/* RX EAR */
static struct adie_codec_action_unit iearpiece_48KHz_osr256_actions_u8860[] =
	EAR_PRI_MONO_8000_OSR_256_U8860;     /* 8000 profile also works for 48k */

static struct adie_codec_hwsetting_entry iearpiece_settings_u8860[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iearpiece_48KHz_osr256_actions_u8860,
		.action_sz = ARRAY_SIZE(iearpiece_48KHz_osr256_actions_u8860),
	}
};

static struct adie_codec_dev_profile iearpiece_profile_u8860 = {
	.path_type = ADIE_CODEC_RX,
	.settings = iearpiece_settings_u8860,
	.setting_sz = ARRAY_SIZE(iearpiece_settings_u8860),
};

/* rx volume from 6dB to 0dB */
/* rx volume from -7dB to 6dB */
static struct snddev_icodec_data snddev_iearpiece_data_u8860 = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_SPKR,
	.profile = &iearpiece_profile_u8860,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.property = SIDE_TONE_MASK,
	.max_voice_rx_vol[VOC_NB_INDEX] = 0,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1500,
	.max_voice_rx_vol[VOC_WB_INDEX] = -700,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2200,
};

static struct platform_device msm_iearpiece_device_u8860 = {
	.name = "snddev_icodec",
	.id = 19,
	.dev = { .platform_data = &snddev_iearpiece_data_u8860 },
};

/* AMIC Primary mono, common for SPEAKER MONO TX and HANDSET MONO TX */
static struct adie_codec_action_unit imic_48KHz_osr256_actions_u8860[] =
	AMIC_PRI_MONO_8000_OSR_256_U8860;   /* 8000 profile also works for 48k */

static struct adie_codec_hwsetting_entry imic_settings_u8860[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = imic_48KHz_osr256_actions_u8860,
		.action_sz = ARRAY_SIZE(imic_48KHz_osr256_actions_u8860),
	}
};

static enum hsed_controller imic_pmctl_id_u8860[] = {PM_HSED_CONTROLLER_0};

static struct adie_codec_dev_profile imic_profile_u8860 = {
	.path_type = ADIE_CODEC_TX,
	.settings = imic_settings_u8860,
	.setting_sz = ARRAY_SIZE(imic_settings_u8860),
};

static struct snddev_icodec_data snddev_imic_data_u8860 = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC,
	.profile = &imic_profile_u8860,
	.channel_mode = 1,
	.pmctl_id = imic_pmctl_id_u8860,
	.pmctl_id_sz = ARRAY_SIZE(imic_pmctl_id_u8860),
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_imic_device_u8860 = {
	.name = "snddev_icodec",
	.id = 20,
	.dev = { .platform_data = &snddev_imic_data_u8860 },
};

/* SPEAKER STEREO RX */
static struct adie_codec_action_unit ispkr_stereo_48KHz_osr256_actions_u8860[] =
	SPEAKER_PRI_STEREO_48000_OSR_256_U8860;

static struct adie_codec_hwsetting_entry ispkr_stereo_settings_u8860[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ispkr_stereo_48KHz_osr256_actions_u8860,
		.action_sz = ARRAY_SIZE(ispkr_stereo_48KHz_osr256_actions_u8860),
	}
};

static struct adie_codec_dev_profile ispkr_stereo_profile_u8860 = {
	.path_type = ADIE_CODEC_RX,
	.settings = ispkr_stereo_settings_u8860,
	.setting_sz = ARRAY_SIZE(ispkr_stereo_settings_u8860),
};

/* rx volume from -8dB to 3dB */
/* rx volume from -2dB to -8dB */
/* set to mono device */
static struct snddev_icodec_data snddev_ispkr_stereo_data_u8860 = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_stereo_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MONO,
	.profile = &ispkr_stereo_profile_u8860,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_poweramp_on,
	.pamp_off = msm_snddev_poweramp_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = 300,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1200,
	.max_voice_rx_vol[VOC_WB_INDEX] = 300,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1200
};

static struct platform_device msm_ispkr_stereo_device_u8860 = {
	.name = "snddev_icodec",
	.id = 8,
	.dev = { .platform_data = &snddev_ispkr_stereo_data_u8860 },
};

/* HEADSET MONO TX */
static struct adie_codec_action_unit iheadset_mic_tx_osr256_actions_u8860[] =
	AMIC1_HEADSET_TX_MONO_PRIMARY_OSR256_U8860;

static struct adie_codec_hwsetting_entry iheadset_mic_tx_settings_u8860[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iheadset_mic_tx_osr256_actions_u8860,
		.action_sz = ARRAY_SIZE(iheadset_mic_tx_osr256_actions_u8860),
	}
};

static struct adie_codec_dev_profile iheadset_mic_profile_u8860 = {
	.path_type = ADIE_CODEC_TX,
	.settings = iheadset_mic_tx_settings_u8860,
	.setting_sz = ARRAY_SIZE(iheadset_mic_tx_settings_u8860),
};

static struct snddev_icodec_data snddev_headset_mic_data_u8860 = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "headset_mono_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_MIC,
	.profile = &iheadset_mic_profile_u8860,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device msm_headset_mic_device_u8860 = {
	.name = "snddev_icodec",
	.id = 6,
	.dev = { .platform_data = &snddev_headset_mic_data_u8860 },
};

/* SPEAKER MONO TX */
static enum hsed_controller ispk_pmctl_id_u8860[] = {PM_HSED_CONTROLLER_0};

static struct snddev_icodec_data snddev_ispkr_mic_data_u8860 = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_mono_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MIC,
	.profile = &imic_profile_u8860,
	.channel_mode = 1,
	.pmctl_id = ispk_pmctl_id_u8860,
	.pmctl_id_sz = ARRAY_SIZE(ispk_pmctl_id_u8860),
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device msm_ispkr_mic_device_u8860 = {
	.name = "snddev_icodec",
	.id = 18,
	.dev = { .platform_data = &snddev_ispkr_mic_data_u8860 },
};

/* HEADSET STEREO */
static struct adie_codec_action_unit headset_ab_cpls_48KHz_osr256_actions_u8860[] =
	HEADSET_AB_CPLS_48000_OSR_256_U8860;

static struct adie_codec_hwsetting_entry headset_ab_cpls_settings_u8860[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = headset_ab_cpls_48KHz_osr256_actions_u8860,
		.action_sz = ARRAY_SIZE(headset_ab_cpls_48KHz_osr256_actions_u8860),
	}
};

static struct adie_codec_dev_profile headset_ab_cpls_profile_u8860 = {
	.path_type = ADIE_CODEC_RX,
	.settings = headset_ab_cpls_settings_u8860,
	.setting_sz = ARRAY_SIZE(headset_ab_cpls_settings_u8860),
};

/* rxvolume from -4dB to 2dB */
/* rxvolume from -7dB to -4dB */
static struct snddev_icodec_data snddev_ihs_stereo_rx_data_u8860 = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_stereo_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_SPKR_STEREO,
	.profile = &headset_ab_cpls_profile_u8860,
	.channel_mode = 2,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.property = SIDE_TONE_MASK,
	.voltage_on = msm_snddev_hsed_voltage_on,
	.voltage_off = msm_snddev_hsed_voltage_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = 200, 
	.min_voice_rx_vol[VOC_NB_INDEX] = -1300,
	.max_voice_rx_vol[VOC_WB_INDEX] = -500, 
	.min_voice_rx_vol[VOC_WB_INDEX] = -2000,
};

static struct platform_device msm_headset_stereo_device_u8860 = {
	.name = "snddev_icodec",
	.id = 2,
	.dev = { .platform_data = &snddev_ihs_stereo_rx_data_u8860 },
};

/* HANDSET DUAL MIC BROADSIDE */
static struct adie_codec_action_unit idual_mic_bs_8KHz_osr256_actions_u8860[] =
	AMIC_BROADSIDE_DUAL_8000_OSR_256_U8860; 

static struct adie_codec_hwsetting_entry idual_mic_broadside_settings_u8860[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions_u8860,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions_u8860),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions_u8860,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions_u8860),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions_u8860,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions_u8860),
	}
};

static struct adie_codec_dev_profile idual_mic_broadside_profile_u8860 = {
	.path_type = ADIE_CODEC_TX,
	.settings = idual_mic_broadside_settings_u8860,
	.setting_sz = ARRAY_SIZE(idual_mic_broadside_settings_u8860),
};

static struct snddev_icodec_data snddev_idual_mic_broadside_data_u8860 = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_dual_mic_broadside_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC_BROADSIDE,
	.profile = &idual_mic_broadside_profile_u8860,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_broadside_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_broadside_pmctl_id),
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_idual_mic_broadside_device_u8860 = {
	.name = "snddev_icodec",
	.id = 13,
	.dev = { .platform_data = &snddev_idual_mic_broadside_data_u8860 },
};
/* ES HANDSET DUAL MIC BROADSIDE */
static struct adie_codec_action_unit idual_mic_bs_8KHz_osr256_actions_u8860_es[] =
	AMIC_BROADSIDE_DUAL_8000_OSR_256_U8860_es; 

static struct adie_codec_hwsetting_entry idual_mic_broadside_settings_u8860_es[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions_u8860_es,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions_u8860_es),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions_u8860_es,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions_u8860_es),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions_u8860_es,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions_u8860_es),
	}
};

static struct adie_codec_dev_profile idual_mic_broadside_profile_u8860_es = {
	.path_type = ADIE_CODEC_TX,
	.settings = idual_mic_broadside_settings_u8860_es,
	.setting_sz = ARRAY_SIZE(idual_mic_broadside_settings_u8860_es),
};

static struct snddev_icodec_data snddev_idual_mic_broadside_data_u8860_es = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_dual_mic_broadside_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC_BROADSIDE,
	.profile = &idual_mic_broadside_profile_u8860_es,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_broadside_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_broadside_pmctl_id),
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_idual_mic_broadside_device_u8860_es = {
	.name = "snddev_icodec",
	.id = 13,
	.dev = { .platform_data = &snddev_idual_mic_broadside_data_u8860_es },
};

/* SPEAKER MUSIC STEREO DEVICE */
static struct adie_codec_action_unit ispkr_music_stereo_48KHz_osr256_actions_u8860[] =
	SPEAKER_PRI_MUSIC_STEREO_48000_OSR_256_U8860;

static struct adie_codec_hwsetting_entry ispkr_music_stereo_settings_u8860[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ispkr_music_stereo_48KHz_osr256_actions_u8860,
		.action_sz = ARRAY_SIZE(ispkr_music_stereo_48KHz_osr256_actions_u8860),
	}
};

static struct adie_codec_dev_profile ispkr_music_stereo_profile_u8860 = {
	.path_type = ADIE_CODEC_RX,
	.settings = ispkr_music_stereo_settings_u8860,
	.setting_sz = ARRAY_SIZE(ispkr_music_stereo_settings_u8860),
};

/* set ACDB_ID_SPKR_PHONE_MUSIC_MONO as acdb_id */
/* set to mono device, use music tpa parameter */
static struct snddev_icodec_data snddev_ispkr_music_stereo_data_u8860 = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_music_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MUSIC_MONO, 
	.profile = &ispkr_music_stereo_profile_u8860,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_poweramp_4music_on,
	.pamp_off = msm_snddev_poweramp_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_NB_INDEX] = -500,
	.max_voice_rx_vol[VOC_WB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_WB_INDEX] = -500
};

static struct platform_device msm_ispkr_music_stereo_device_u8860 = {
	.name = "snddev_icodec",
	.id = 252,
	.dev = { .platform_data = &snddev_ispkr_music_stereo_data_u8860 },
};

/* HEADSET MUSIC STEREO DEVICE */
static struct adie_codec_action_unit headset_ab_cpls_music_48KHz_osr256_actions_u8860[] =
	HEADSET_AB_CPLS_MUSIC_48000_OSR_256_U8860;

static struct adie_codec_hwsetting_entry headset_ab_cpls_music_settings_u8860[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = headset_ab_cpls_music_48KHz_osr256_actions_u8860,
		.action_sz = ARRAY_SIZE(headset_ab_cpls_music_48KHz_osr256_actions_u8860),
	}
};

static struct adie_codec_dev_profile headset_ab_cpls_music_profile_u8860 = {
	.path_type = ADIE_CODEC_RX,
	.settings = headset_ab_cpls_music_settings_u8860,
	.setting_sz = ARRAY_SIZE(headset_ab_cpls_music_settings_u8860),
};

/* set ACDB_ID_HEADSET_SPKR_MUSIC_STEREO as acdb_id */
static struct snddev_icodec_data snddev_ihs_music_stereo_rx_data_u8860 = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_music_stereo_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_SPKR_MUSIC_STEREO, 
	.profile = &headset_ab_cpls_music_profile_u8860,
	.channel_mode = 2,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.property = SIDE_TONE_MASK,
	.voltage_on = msm_snddev_hsed_voltage_on,
	.voltage_off = msm_snddev_hsed_voltage_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -900,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2400,
};

static struct platform_device msm_headset_music_stereo_device_u8860 = {
	.name = "snddev_icodec",
	.id = 253,
	.dev = { .platform_data = &snddev_ihs_music_stereo_rx_data_u8860 },
};

/* EARPIECE HAC DEVICE */
static struct adie_codec_action_unit iearpiece_hac_48KHz_osr256_actions_u8860[] =
	EAR_HAC_PRI_MONO_8000_OSR_256_U8860; /* 8000 profile also works for 48k */

static struct adie_codec_hwsetting_entry iearpiece_hac_settings_u8860[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iearpiece_hac_48KHz_osr256_actions_u8860,
		.action_sz = ARRAY_SIZE(iearpiece_hac_48KHz_osr256_actions_u8860),
	}
};

static struct adie_codec_dev_profile iearpiece_hac_profile_u8860 = {
	.path_type = ADIE_CODEC_RX,
	.settings = iearpiece_hac_settings_u8860,
	.setting_sz = ARRAY_SIZE(iearpiece_hac_settings_u8860),
};

/* set ACDB_ID_HANDSET_HAC_SPKR as acdb_id */
static struct snddev_icodec_data snddev_iearpiece_hac_data_u8860 = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_hac_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_HAC_SPKR,
	.profile = &iearpiece_hac_profile_u8860,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
    /* U8860 add hac gpio ctl func */
	.pamp_on = msm_snddev_hac_on,  // when earpiece power on, pull up GPIO for hac
	.pamp_off = msm_snddev_hac_off, // when earpiece power off, pull down GPIO for hac
	.property = SIDE_TONE_MASK,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -1400,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2900,
};

static struct platform_device msm_iearpiece_hac_device_u8860 = {
	.name = "snddev_icodec",
	.id = 251,
	.dev = { .platform_data = &snddev_iearpiece_hac_data_u8860 },
};

/* HANDSET SECONDARY MIC DEVICE */
static struct adie_codec_action_unit handset_secondary_mic_tx_48KHz_osr256_actions[] =
	AMIC_SEC_MONO_8000_OSR_256; /* 8000 profile also works for 48k */

static struct adie_codec_hwsetting_entry handset_secondary_mic_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = handset_secondary_mic_tx_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(handset_secondary_mic_tx_48KHz_osr256_actions),
	}
};

static enum hsed_controller handset_secondary_mic_tx_pmctl_id[] = {PM_HSED_CONTROLLER_0};

static struct adie_codec_dev_profile handset_secondary_mic_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = handset_secondary_mic_tx_settings,
	.setting_sz = ARRAY_SIZE(handset_secondary_mic_tx_settings),
};

static struct snddev_icodec_data snddev_handset_secondary_mic_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_secondary_mic_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC,
	.profile = &handset_secondary_mic_tx_profile,
	.channel_mode = 1,
	.pmctl_id = handset_secondary_mic_tx_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(handset_secondary_mic_tx_pmctl_id),
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_handset_secondary_mic_tx_device = {
	.name = "snddev_icodec",
	.id = 250,
	.dev = { .platform_data = &snddev_handset_secondary_mic_tx_data },
};

/* ANALOG FM SPEAKER DEVICE */
static struct adie_codec_action_unit ifm_analog_speaker_48KHz_osr256_actions[] =
	FM_ANALOG_SPEAKER_48000_OSR_256; 

static struct adie_codec_hwsetting_entry ifm_analog_speaker_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ifm_analog_speaker_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ifm_analog_speaker_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ifm_analog_speaker_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ifm_analog_speaker_settings,
	.setting_sz = ARRAY_SIZE(ifm_analog_speaker_settings),
};

/* set msm_snddev_poweramp_4music_on as pamp_on for FM Speaker */
static struct snddev_icodec_data snddev_analog_fm_speaker_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "fmradio_analog_speaker",
	.copp_id = 0,
	.acdb_id = PSEUDO_ACDB_ID,
	.profile = &ifm_analog_speaker_profile,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = &msm_snddev_poweramp_4music_on,
	.pamp_off = &msm_snddev_poweramp_off,
};

static struct platform_device  msm_snddev_analog_fm_speaker_device = {
	.name = "snddev_icodec",
	.id = 249,
	.dev = { .platform_data = &snddev_analog_fm_speaker_data},
};

/* ANALOG FM HEADSET DEVICE */
static struct adie_codec_action_unit ifm_analog_hs_48KHz_osr256_actions[] =
	FM_ANALOG_HEADSET_48000_OSR_256;

static struct adie_codec_hwsetting_entry ifm_analog_hs_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ifm_analog_hs_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ifm_analog_hs_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ifm_analog_hs_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ifm_analog_hs_settings,
	.setting_sz = ARRAY_SIZE(ifm_analog_hs_settings),
};

static struct snddev_icodec_data snddev_analog_fm_hs_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "fmradio_analog_headset",
	.copp_id = 0,
	.acdb_id = PSEUDO_ACDB_ID,
	.profile = &ifm_analog_hs_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_hsed_voltage_on,
	.pamp_off = msm_snddev_hsed_voltage_off,
};

static struct platform_device msm_snddev_analog_fm_hs_device = {
	.name = "snddev_icodec",
	.id = 248,
	.dev = { .platform_data = &snddev_analog_fm_hs_data },
};

#endif
/****************************************************************/
/**            HUAWEI ADDED DEVICE                             **/
/****************************************************************/
/* RX EAR */
static struct snddev_icodec_data snddev_iearpiece_data_u8730 = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_SPKR,
	.profile = &iearpiece_profile_u8860,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.property = SIDE_TONE_MASK,
	.max_voice_rx_vol[VOC_NB_INDEX] = 800,
	.min_voice_rx_vol[VOC_NB_INDEX] = -700,
	.max_voice_rx_vol[VOC_WB_INDEX] = -700,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2200,
};


static struct platform_device msm_iearpiece_device_u8730 = {
	.name = "snddev_icodec",
	.id = 19,
	.dev = { .platform_data = &snddev_iearpiece_data_u8730 },
};

/* SPEAKER STEREO RX */

static struct snddev_icodec_data snddev_ispkr_stereo_data_u8730 = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_stereo_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MONO,
	.profile = &ispkr_stereo_profile_u8860,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_poweramp_on,
	.pamp_off = msm_snddev_poweramp_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = 1200,
	.min_voice_rx_vol[VOC_NB_INDEX] = -300,
	.max_voice_rx_vol[VOC_WB_INDEX] = 300,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1200
};

static struct platform_device msm_ispkr_stereo_device_u8730 = {
	.name = "snddev_icodec",
	.id = 8,
	.dev = { .platform_data = &snddev_ispkr_stereo_data_u8730 },
};
/* AMIC Primary mono, common for SPEAKER MONO TX and HANDSET MONO TX */
static struct snddev_icodec_data snddev_imic_data_u8730 = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC,
	.profile = &imic_profile_u8860,
	.channel_mode = 1,
	.pmctl_id = imic_pmctl_id_u8860,
	.pmctl_id_sz = ARRAY_SIZE(imic_pmctl_id_u8860),
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device msm_imic_device_u8730 = {
	.name = "snddev_icodec",
	.id = 20,
	.dev = { .platform_data = &snddev_imic_data_u8730 },
};


/* SPEAKER MONO TX */
static struct snddev_icodec_data snddev_ispkr_mic_data_u8730 = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_mono_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MIC,
	.profile = &imic_profile_u8860,
	.channel_mode = 1,
	.pmctl_id = ispk_pmctl_id_u8860,
	.pmctl_id_sz = ARRAY_SIZE(ispk_pmctl_id_u8860),
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device msm_ispkr_mic_device_u8730 = {
	.name = "snddev_icodec",
	.id = 18,
	.dev = { .platform_data = &snddev_ispkr_mic_data_u8730 },
};
/* HANDSET DUAL MIC BROADSIDE */
static struct adie_codec_action_unit idual_mic_bs_8KHz_osr256_actions_u8730[] =
	AMIC_BROADSIDE_DUAL_8000_OSR_256_U8730; 

static struct adie_codec_hwsetting_entry idual_mic_broadside_settings_u8730[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions_u8730,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions_u8730),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions_u8730,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions_u8730),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions_u8730,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions_u8730),
	}
};

static struct adie_codec_dev_profile idual_mic_broadside_profile_u8730 = {
	.path_type = ADIE_CODEC_TX,
	.settings = idual_mic_broadside_settings_u8730,
	.setting_sz = ARRAY_SIZE(idual_mic_broadside_settings_u8730),
};

static struct snddev_icodec_data snddev_idual_mic_broadside_data_u8730 = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_dual_mic_broadside_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC_BROADSIDE,
	.profile = &idual_mic_broadside_profile_u8730,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_broadside_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_broadside_pmctl_id),
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};
static struct platform_device msm_idual_mic_broadside_device_u8730 = {
	.name = "snddev_icodec",
	.id = 13,
	.dev = { .platform_data = &snddev_idual_mic_broadside_data_u8730 },
};

/* HANDSET SECONDARY MIC DEVICE */

static struct snddev_icodec_data snddev_handset_secondary_mic_tx_data_u8730 = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_secondary_mic_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC,
	.profile = &handset_secondary_mic_tx_profile,
	.channel_mode = 1,
	.pmctl_id = handset_secondary_mic_tx_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(handset_secondary_mic_tx_pmctl_id),
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device msm_handset_secondary_mic_tx_device_u8730 = {
	.name = "snddev_icodec",
	.id = 250,
	.dev = { .platform_data = &snddev_handset_secondary_mic_tx_data_u8730 },
};

/****************************************************************/
/**            HUAWEI ADDED DEVICE U8680                        **/
/****************************************************************/
/* HANDSET DUAL MIC BROADSIDE */
static struct adie_codec_action_unit idual_mic_bs_8KHz_osr256_actions_u8680[] =
	AMIC_BROADSIDE_DUAL_8000_OSR_256_U8680; 

static struct adie_codec_hwsetting_entry idual_mic_broadside_settings_u8680[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions_u8680,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions_u8680),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions_u8680,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions_u8680),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions_u8680,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions_u8680),
	}
};

static struct adie_codec_dev_profile idual_mic_broadside_profile_u8680 = {
	.path_type = ADIE_CODEC_TX,
	.settings = idual_mic_broadside_settings_u8680,
	.setting_sz = ARRAY_SIZE(idual_mic_broadside_settings_u8680),
};

static struct snddev_icodec_data snddev_idual_mic_broadside_data_u8680 = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_dual_mic_broadside_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC_BROADSIDE,
	.profile = &idual_mic_broadside_profile_u8680,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_broadside_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_broadside_pmctl_id),
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_idual_mic_broadside_device_u8680 = {
	.name = "snddev_icodec",
	.id = 13,
	.dev = { .platform_data = &snddev_idual_mic_broadside_data_u8680 },
};
static struct snddev_icodec_data snddev_ihs_stereo_rx_data_u8680 = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_stereo_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_SPKR_STEREO,
	.profile = &headset_ab_cpls_profile_u8860,
	.channel_mode = 2,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.property = SIDE_TONE_MASK,
	.voltage_on = msm_snddev_hsed_voltage_on,
	.voltage_off = msm_snddev_hsed_voltage_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = 600, 
	.min_voice_rx_vol[VOC_NB_INDEX] = -900,
	.max_voice_rx_vol[VOC_WB_INDEX] = -500, 
	.min_voice_rx_vol[VOC_WB_INDEX] = -2000,
};

static struct platform_device msm_headset_stereo_device_u8680 = {
	.name = "snddev_icodec",
	.id = 2,
	.dev = { .platform_data = &snddev_ihs_stereo_rx_data_u8680 },
};

static struct snddev_icodec_data snddev_iearpiece_data_u8680 = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_SPKR,
	.profile = &iearpiece_profile_u8860,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.property = SIDE_TONE_MASK,
	.max_voice_rx_vol[VOC_NB_INDEX] =1200,
	.min_voice_rx_vol[VOC_NB_INDEX] = -300,
	.max_voice_rx_vol[VOC_WB_INDEX] = -700,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2200,
};

static struct platform_device msm_iearpiece_device_u8680 = {
	.name = "snddev_icodec",
	.id = 19,
	.dev = { .platform_data = &snddev_iearpiece_data_u8680 },
};

static struct platform_device *snd_devices_ffa[] __initdata = {
	&msm_iearpiece_ffa_device,
	&msm_imic_ffa_device,
	&msm_ispkr_stereo_device,
	&msm_headset_mic_device,
	&msm_ihs_ffa_mono_rx_device,
	&msm_snddev_mi2s_fm_rx_device,
	&msm_snddev_mi2s_fm_tx_device,
	&msm_bt_sco_earpiece_device,
	&msm_bt_sco_mic_device,
	&msm_ispkr_mic_device,
	&msm_headset_stereo_device,
	&msm_idual_mic_endfire_device,
	&msm_spk_idual_mic_endfire_device,
	&msm_itty_mono_tx_device,
	&msm_itty_mono_rx_device,
	&msm_a2dp_rx_device,
	&msm_a2dp_tx_device,
	&msm_uplink_rx_device,
	&msm_real_stereo_tx_device,
	&msm_ihs_stereo_speaker_stereo_rx_device,
	&msm_spk_idual_mic_broadside_device,
	&msm_idual_mic_broadside_device,
	&msm_snddev_mi2s_stereo_rx_device,
	&msm_auxpga_lb_hs_device,
	&msm_auxpga_lb_lo_device,
};

#ifdef CONFIG_HUAWEI_KERNEL
static struct platform_device *snd_devices_u8860[] __initdata = {
	&msm_iearpiece_device_u8860,
	&msm_imic_device_u8860,
	&msm_ispkr_stereo_device_u8860,
	&msm_headset_mic_device_u8860,
	&msm_snddev_mi2s_fm_tx_device,
	&msm_bt_sco_earpiece_device,
	&msm_bt_sco_mic_device,
	&msm_ispkr_mic_device_u8860,
	&msm_headset_stereo_device_u8860,
	&msm_itty_mono_tx_device,
	&msm_itty_mono_rx_device,
	&msm_ihs_stereo_speaker_stereo_rx_device,
	&msm_spk_idual_mic_broadside_device,
	&msm_idual_mic_broadside_device_u8860,
	&msm_ispkr_music_stereo_device_u8860,
	&msm_headset_music_stereo_device_u8860,
	&msm_iearpiece_hac_device_u8860,
	&msm_handset_secondary_mic_tx_device,
	&msm_snddev_analog_fm_speaker_device,
	&msm_snddev_analog_fm_hs_device,
};
static struct platform_device *snd_devices_u8860_es[] __initdata = {
	&msm_iearpiece_device_u8860,
	&msm_imic_device_u8860,
	&msm_ispkr_stereo_device_u8860,
	&msm_headset_mic_device_u8860,
	&msm_snddev_mi2s_fm_tx_device,
	&msm_bt_sco_earpiece_device,
	&msm_bt_sco_mic_device,
	&msm_ispkr_mic_device_u8860,
	&msm_headset_stereo_device_u8860,
	&msm_itty_mono_tx_device,
	&msm_itty_mono_rx_device,
	&msm_ihs_stereo_speaker_stereo_rx_device,
	&msm_spk_idual_mic_broadside_device,
	&msm_idual_mic_broadside_device_u8860_es,
	&msm_ispkr_music_stereo_device_u8860,
	&msm_headset_music_stereo_device_u8860,
	&msm_iearpiece_hac_device_u8860,
	&msm_handset_secondary_mic_tx_device,
	&msm_snddev_analog_fm_speaker_device,
	&msm_snddev_analog_fm_hs_device,
};
static struct platform_device *snd_devices_u8680[] __initdata = {
	&msm_iearpiece_device_u8680,
	&msm_imic_device_u8860,
	&msm_ispkr_stereo_device_u8860,
	&msm_headset_mic_device_u8860,
	&msm_snddev_mi2s_fm_tx_device,
	&msm_bt_sco_earpiece_device,
	&msm_bt_sco_mic_device,
	&msm_ispkr_mic_device_u8860,
	&msm_headset_stereo_device_u8680,
	&msm_itty_mono_tx_device,
	&msm_itty_mono_rx_device,
	&msm_ihs_stereo_speaker_stereo_rx_device,
	&msm_spk_idual_mic_broadside_device,
	&msm_idual_mic_broadside_device_u8680,
	&msm_ispkr_music_stereo_device_u8860,
	&msm_headset_music_stereo_device_u8860,
	&msm_iearpiece_hac_device_u8860,
	&msm_handset_secondary_mic_tx_device,
	&msm_snddev_analog_fm_speaker_device,
	&msm_snddev_analog_fm_hs_device,
};
#endif
#ifdef CONFIG_HUAWEI_KERNEL
static struct platform_device *snd_devices_u8730[] __initdata = {
	&msm_iearpiece_device_u8730,
	&msm_imic_device_u8730,
	&msm_ispkr_stereo_device_u8730,
	&msm_headset_mic_device_u8860,
	&msm_snddev_mi2s_fm_tx_device,
	&msm_bt_sco_earpiece_device,
	&msm_bt_sco_mic_device,
	&msm_ispkr_mic_device_u8730,
	&msm_headset_stereo_device_u8860,
	&msm_itty_mono_tx_device,
	&msm_itty_mono_rx_device,
	&msm_ihs_stereo_speaker_stereo_rx_device,
	&msm_spk_idual_mic_broadside_device,
	&msm_idual_mic_broadside_device_u8730,
	&msm_ispkr_music_stereo_device_u8860,
	&msm_headset_music_stereo_device_u8860,
	&msm_iearpiece_hac_device_u8860,
	&msm_handset_secondary_mic_tx_device_u8730,
	&msm_snddev_analog_fm_speaker_device,
	&msm_snddev_analog_fm_hs_device,
};
#endif

void __ref msm_snddev_init_timpani(void)
{
#ifndef CONFIG_HUAWEI_KERNEL
	platform_add_devices(snd_devices_ffa,
			ARRAY_SIZE(snd_devices_ffa));
#ifdef CONFIG_DEBUG_FS
	debugfs_hsed_config = debugfs_create_file("msm_hsed_config",
				S_IFREG | S_IWUGO, NULL,
		(void *) "msm_hsed_config", &snddev_hsed_config_debug_fops);
	if (!debugfs_hsed_config)
		pr_err("failed to create msm_head_config debug fs entry\n");
#endif
#else  //#ifndef CONFIG_HUAWEI_KERNEL
    smem_huawei_vender *vender_para_ptr;
    const char *country_name = "es";
    if (machine_is_msm7x30_ffa() || machine_is_msm8x55_ffa() ||
    		machine_is_msm8x55_svlte_ffa()) {
    		platform_add_devices(snd_devices_ffa,
		ARRAY_SIZE(snd_devices_ffa));
#ifdef CONFIG_DEBUG_FS
        debugfs_hsed_config = debugfs_create_file("msm_hsed_config",
        			S_IFREG | S_IWUGO, NULL,
    	(void *) "msm_hsed_config", &snddev_hsed_config_debug_fops);
        if (!debugfs_hsed_config)
        	pr_err("failed to create msm_head_config debug fs entry\n");	
#endif        
    }else if( machine_is_msm8255_u8860() 
	        || machine_is_msm8255_u8860lp() 
            || machine_is_msm8255_u8860_r()
            || machine_is_msm8255_u8860_92()
            || machine_is_msm8255_u8860_51())
    {
        vender_para_ptr = (smem_huawei_vender*)smem_alloc(SMEM_ID_VENDOR0, sizeof(smem_huawei_vender));
        if (!vender_para_ptr)
        {
          pr_info("%s: Can't find vender parameter\n", __func__);
          return;
        }
        pr_info("vendor:%s,country:%s\n", vender_para_ptr->vender_para.vender_name, vender_para_ptr->vender_para.country_name);

        /* choose audio parameter table according to the vender name */
        if(!memcmp(vender_para_ptr->vender_para.country_name, country_name, strlen(country_name)))
        {
          platform_add_devices(snd_devices_u8860_es, ARRAY_SIZE(snd_devices_u8860_es));
        }
        else
        {
          platform_add_devices(snd_devices_u8860, ARRAY_SIZE(snd_devices_u8860));
        }
		
    }else if ( machine_is_msm8255_u8680())
    {
       platform_add_devices(snd_devices_u8680, ARRAY_SIZE(snd_devices_u8680));  
    }else if (machine_is_msm8255_u8730()
	        || machine_is_msm8255_u8667())
    {
        platform_add_devices(snd_devices_u8730,  ARRAY_SIZE(snd_devices_u8730));
    }else if (machine_is_msm8255_c8860())
    {
        platform_add_devices(snd_devices_c8860, arraysize_c8860);
    }
    else
    {
        platform_add_devices(snd_devices_u8860, ARRAY_SIZE(snd_devices_u8860));
    }
#endif  //#ifndef CONFIG_HUAWEI_KERNEL

}
