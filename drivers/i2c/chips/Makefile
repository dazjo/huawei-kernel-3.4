#
# Makefile for miscellaneous I2C chip drivers.
#
# Do not add new drivers to this directory! It is DEPRECATED.
#
# Device drivers are better grouped according to the functionality they
# implement rather than to the bus they are connected to. In particular:
# * Hardware monitoring chip drivers go to drivers/hwmon
# * RTC chip drivers go to drivers/rtc
# * I/O expander drivers go to drivers/gpio
#

#modify for 4125 baseline
#obj-$(CONFIG_SENSORS_MT9T013)	+= mt9t013.o

obj-$(CONFIG_HUAWEI_FEATURE_TPA2028D1_AMPLIFIER)	+= tpa2028d1.o
obj-$(CONFIG_HUAWEI_FEATURE_SENSORS_ST_LSM303DLH) += st303_compass.o
obj-$(CONFIG_HUAWEI_FEATURE_SENSORS_ST_LSM303DLH) += st303_gs.o
obj-$(CONFIG_HUAWEI_FEATURE_SENSORS_AK8975)   += akm8975.o
obj-$(CONFIG_HUAWEI_FEATURE_SENSORS_AK8963)   += akm8963.o
ifeq ($(CONFIG_I2C_DEBUG_CHIP),y)
EXTRA_CFLAGS += -DDEBUG
endif
obj-$(CONFIG_QWERTY_KEYPAD_ADP5587) += adp5587.o
