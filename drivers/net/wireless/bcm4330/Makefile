# bcm4330
DHDCFLAGS = -Wall -Wstrict-prototypes -Dlinux -DBCMDRIVER           \
	-DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 -DWLBTAMP           \
	-DDHDTHREAD -DDHD_GPL -DDHD_SCHED -DDHD_DEBUG -DSDTEST          \
	-DDHD_BCMEVENTS -DSHOW_EVENTS -DDONGLEOVERLAYS -DBCMDBG         \
	-DCUSTOMER_HW2 -DOOB_INTR_ONLY -DHW_OOB -DWLP2P -DBCMFILEIMAGE  \
	-DMMC_SDIO_ABORT -DBCMSDIO -DBCMLXSDMMC -DBCMPLATFORM_BUS       \
	-DNEW_COMPAT_WIRELESS -DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT    \
	-DKEEP_ALIVE -DCSCAN -DPKT_FILTER_SUPPORT -DBDC -DTOE           \
	-DEMBEDDED_PLATFORM -DENABLE_INSMOD_NO_FW_LOAD -DPNO_SUPPORT    \
	-Idrivers/net/wireless/bcm4330 -Idrivers/net/wireless/bcm4330/include

DHDCFLAGS += -DCUSTOM_SDCC_WIFI_SLOT=2
DHDCFLAGS += -DCUSTOM_OOB_GPIO_NUM=2

DHDOFILES = aiutils.o bcmsdh_sdmmc_linux.o dhd_linux.o siutils.o bcmutils.o   \
	dhd_linux_sched.o bcmwifi.o dhd_sdio.o bcmevent.o dhd_bta.o hndpmu.o  \
	bcmsdh.o dhd_cdc.o bcmsdh_linux.o dhd_common.o linux_osl.o            \
	bcmsdh_sdmmc.o dhd_custom_gpio.o sbutils.o wldev_common.o wl_android.o

obj-$(CONFIG_BCM4330) += bcm4330.o
bcm4330-objs += $(DHDOFILES)
ifneq ($(CONFIG_WIRELESS_EXT),)
bcm4330-objs += wl_iw.o
DHDCFLAGS += -DSOFTAP
endif
ifneq ($(CONFIG_CFG80211),)
bcm4330-objs += wl_cfg80211.o dhd_cfg80211.o wl_cfgp2p.o wl_linux_mon.o
DHDCFLAGS += -DWL_CFG80211
endif
EXTRA_CFLAGS = $(DHDCFLAGS)
ifeq ($(CONFIG_BCM4330),m)
EXTRA_LDFLAGS += --strip-debug
endif
