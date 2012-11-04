# bcm4330
DFLAGS = -Wall -Wstrict-prototypes -DLINUX -Dlinux -Idrivers/net/wireless/bcm4330 -Idrivers/net/wireless/bcm4330/include

DFLAGS += -DBCMDRIVER -DDHDTHREAD -DBDC -DTOE
#DFLAGS += -DBCMSDIO -DBCMFILEIMAGE -DBCMDONGLEHOST
#DFLAGS += -DUNRELEASEDCHIP -DBCMDMA32 -DWLBTAMP
#DFLAGS += -DDHD_GPL -DDHD_SCHED -DDHD_DEBUG -DSDTEST
#DFLAGS += -DDHD_BCMEVENTS -DSHOW_EVENTS -DDONGLEOVERLAYS -DBCMDBG

DFLAGS += -DOOB_INTR_ONLY
DFLAGS += -DHW_OOB
DFLAGS += -DMMC_SDIO_ABORT
DFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI
DFLAGS += -DKEEP_ALIVE -DPNO_SUPPORT 

DFLAGS += -DCUSTOMER_HW2 -DCONFIG_WIFI_CONTROL_FUNC
DFLAGS += -DDHD_USE_STATIC_BUF

DFLAGS += -DCUSTOM_SDCC_WIFI_SLOT=2
DFLAGS += -DCUSTOM_OOB_GPIO_NUM=48

DFLAGS += -DEMBEDDED_PLATFORM -DARP_OFFLOAD_SUPPORT -DPKT_FILTER_SUPPORT

#DFLAGS += -DSOFTAP
#DFLAGS += -DCSCAN
#DFLAGS += -DCONFIG_FIRST_SCAN
DFLAGS += -DBCMLXSDMMC -DBCMPLATFORM_BUS 
DFLAGS += -DWLP2P
DFLAGS += -DWIFI_ACT_FRAME
DFLAGS += -DNEW_COMPAT_WIRELESS
DFLAGS += -DENABLE_INSMOD_NO_FW_LOAD

DHDOFILES = aiutils.o bcmsdh_sdmmc_linux.o dhd_linux.o siutils.o bcmutils.o \
	dhd_linux_sched.o bcmwifi.o dhd_sdio.o bcmevent.o dhd_bta.o hndpmu.o    \
	bcmsdh.o dhd_cdc.o bcmsdh_linux.o dhd_common.o linux_osl.o              \
	bcmsdh_sdmmc.o dhd_custom_gpio.o sbutils.o wldev_common.o wl_android.o

obj-$(CONFIG_BCM4330) += dhd.o
dhd-objs += $(DHDOFILES)
ifneq ($(CONFIG_WIRELESS_EXT),)
dhd-objs += wl_iw.o
DFLAGS += -DWL_WIRELESS_EXT
endif
ifneq ($(CONFIG_CFG80211),)
dhd-objs += wl_cfg80211.o dhd_cfg80211.o wl_cfgp2p.o wl_linux_mon.o
DFLAGS += -DWL_CFG80211
endif
EXTRA_CFLAGS = $(DFLAGS)
ifeq ($(CONFIG_BCM4330),m)
EXTRA_LDFLAGS += --strip-debug
endif
