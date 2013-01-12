GCC_VERSION      := $(shell $(CONFIG_SHELL) $(PWD)/scripts/gcc-version.sh $(CROSS_COMPILE)gcc)
EXTRA_CFLAGS += -Idrivers/media/video/msm
EXTRA_CFLAGS += -Idrivers/media/video/msm/io
EXTRA_CFLAGS += -Idrivers/media/video/msm/eeprom
EXTRA_CFLAGS += -Idrivers/media/video/msm/csi
obj-$(CONFIG_MSM_CAMERA_SENSOR) += msm_sensor.o
#/* Because of hardware,s5k5ca should be probed first.
obj-$(CONFIG_HUAWEI_CAMERA_SENSOR_S5K5CA) += s5k5ca_v4l2.o 
obj-$(CONFIG_OV5647) += ov5647_v4l2.o
obj-$(CONFIG_OV8825) += ov8825_v4l2.o
obj-$(CONFIG_IMX074) += imx074_v4l2.o
obj-$(CONFIG_S5K3L1YX) += s5k3l1yx.o
obj-$(CONFIG_IMX091) += imx091.o
obj-$(CONFIG_OV2720) += ov2720.o
obj-$(CONFIG_MT9M114) += mt9m114_v4l2.o
obj-$(CONFIG_HUAWEI_SENSOR_S5K4E1) += s5k4e1_v4l2.o
obj-$(CONFIG_HUAWEI_SENSOR_MT9E013) += mt9e013_v4l2.o
obj-$(CONFIG_WEBCAM_OV9726) += ov9726_v4l2.o
obj-$(CONFIG_OV7692) += ov7692_v4l2.o
obj-$(CONFIG_VX6953) += vx6953.o
obj-$(CONFIG_HUAWEI_SENSOR_MT9P017) += mt9p017_v4l2.o
#/* the front camera must be probe last */
obj-$(CONFIG_HUAWEI_CAMERA_SENSOR_S5K3H2) += s5k3h2_v4l2.o 
obj-$(CONFIG_HUAWEI_CAMERA_SENSOR_IMX105) += imx105_v4l2.o 
#/* delete one line */
obj-$(CONFIG_HUAWEI_CAMERA_SENSOR_MT9T113) += mt9t113_v4l2.o 
obj-$(CONFIG_HUAWEI_CAMERA_SENSOR_HI542) += hi542_v4l2.o 
obj-$(CONFIG_HUAWEI_CAMERA_SENSOR_MT9V113) += mt9v113_v4l2.o 
obj-$(CONFIG_HUAWEI_CAMERA_SENSOR_GC0313) += gc0313_v4l2.o 
obj-$(CONFIG_HUAWEI_CAMERA_SENSOR_BF3905) += bf3905_v4l2.o 

