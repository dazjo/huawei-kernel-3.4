#
# Makefile for nfc devices
#

obj-$(CONFIG_PN544_NFC)		+= pn544.o
obj-$(CONFIG_NFC_PN533)		+= pn533.o
obj-$(CONFIG_NFC_WILINK)	+= nfcwilink.o

obj-$(CONFIG_HUAWEI_NFC_PN544) += nfc_pn544.o
nfc_pn544-$(CONFIG_HUAWEI_NFC_PN544) :=pn544.o hcibase.o pn544mmi.o scriptarrays.o commonutility.o
ccflags-$(CONFIG_NFC_DEBUG) := -DDEBUG
