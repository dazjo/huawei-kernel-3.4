# bcm4330
DHDCFLAGS = -Wall -Wstrict-prototypes -DLINUX -Idrivers/net/wireless/bcm4330 -Idrivers/net/wireless/bcm4330/include

DHDCFLAGS += -DBCMDRIVER -DDHDTHREAD -DBDC -DTOE

DHDCFLAGS += -DOOB_INTR_ONLY
DHDCFLAGS += -DHW_OOB
DHDCFLAGS += -DMMC_SDIO_ABORT
DHDCFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI
DHDCFLAGS += -DKEEP_ALIVE -DPNO_SUPPORT

DHDCFLAGS += -DCUSTOMER_HW2 -DCONFIG_WIFI_CONTROL_FUNC
DHDCFLAGS += -DDHD_USE_STATIC_BUF

# Huawei magic numbers
DHDCFLAGS += -DCUSTOM_SDCC_WIFI_SLOT=2
DHDCFLAGS += -DCUSTOM_OOB_GPIO_NUM=48

DHDCFLAGS += -DEMBEDDED_PLATFORM -DARP_OFFLOAD_SUPPORT -DPKT_FILTER_SUPPORT

DHDCFLAGS += -DBCMLXSDMMC -DBCMPLATFORM_BUS
DHDCFLAGS += -DWLP2P
DHDCFLAGS += -DWIFI_ACT_FRAME
DHDCFLAGS += -DNEW_COMPAT_WIRELESS
DHDCFLAGS += -DENABLE_INSMOD_NO_FW_LOAD

DHDOFILES = aiutils.o bcmsdh_sdmmc_linux.o dhd_linux.o siutils.o bcmutils.o \
	dhd_linux_sched.o bcmwifi.o dhd_sdio.o bcmevent.o dhd_bta.o hndpmu.o    \
	bcmsdh.o dhd_cdc.o bcmsdh_linux.o dhd_common.o linux_osl.o              \
	bcmsdh_sdmmc.o dhd_custom_gpio.o sbutils.o wldev_common.o wl_android.o

obj-$(CONFIG_BCM4330) += dhd.o
dhd-objs += $(DHDOFILES)
ifneq ($(CONFIG_CFG80211),)
dhd-objs += wl_cfg80211.o dhd_cfg80211.o wl_cfgp2p.o wl_linux_mon.o
DHDCFLAGS += -DWL_CFG80211
endif
EXTRA_CFLAGS = $(DHDCFLAGS)
ifeq ($(CONFIG_BCM4330),m)
EXTRA_LDFLAGS += --strip-debug
endif
