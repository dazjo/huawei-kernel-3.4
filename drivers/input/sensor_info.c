/*
 * Copyright (C) huawei company
 *
 * This	program	is free	software; you can redistribute it and/or modify
 * it under	the	terms of the GNU General Public	License	version	2 as
 * published by	the	Free Software Foundation.
 */
/*==============================================================================
History

Problem NO.         Name        Time         Reason

==============================================================================*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/sensors.h>
#include <linux/module.h>

static char *sensor_binder_input[SENSOR_MAX] = {NULL};

int set_sensor_input(enum input_name name, const char *input_num)
{
	if (name < ACC || name >= SENSOR_MAX || input_num == NULL) {
		pr_err("set_sensor_input name =%d input_num = %s\n",
				name, input_num);
		return -EINVAL;
	}
	sensor_binder_input[name] = (char *)input_num;

	return 0;
}
EXPORT_SYMBOL(set_sensor_input);

static struct platform_device sensor_input_info = {
	.name = "huawei_sensor",
	.id = -1,
};

static ssize_t sensor_show_akm_input(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	if (dev == NULL) {
		pr_err("sensor_show_akm_input dev is null\n");
		return -EINVAL;
	}
	return sprintf(buf, "%s\n", sensor_binder_input[AKM]);

}
static DEVICE_ATTR(akm_input, S_IRUGO,
				   sensor_show_akm_input, NULL);



static ssize_t sensor_show_acc_input(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	if (dev == NULL) {
		pr_err("sensor_show_acc_input dev is null\n");
		return -EINVAL;
	}
	return sprintf(buf, "%s\n", sensor_binder_input[ACC]);
}
static DEVICE_ATTR(acc_input, S_IRUGO,
				   sensor_show_acc_input, NULL);

static ssize_t sensor_show_gyro_input(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	if (dev == NULL) {
		pr_err("sensor_show_gyro_input dev is null\n");
		return -EINVAL;
	}
	return sprintf(buf, "%s\n", sensor_binder_input[GYRO]);
}
static DEVICE_ATTR(gyro_input, S_IRUGO,
				   sensor_show_gyro_input, NULL);

static ssize_t sensor_show_ps_input(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	if (dev == NULL) {
		pr_err("sensor_show_ps_input dev is null\n");
		return -EINVAL;
	}
	return sprintf(buf, "%s\n", sensor_binder_input[PS]);
}
static DEVICE_ATTR(ps_input, S_IRUGO,
				   sensor_show_ps_input, NULL);

static ssize_t sensor_show_als_input(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	if (dev == NULL) {
		pr_err("sensor_show_als_input dev is null\n");
		return -EINVAL;
	}
	return sprintf(buf, "%s\n", sensor_binder_input[ALS]);
}
static DEVICE_ATTR(als_input, S_IRUGO,
				   sensor_show_als_input, NULL);

static struct attribute *apds990x_input_attributes[] = {
	&dev_attr_ps_input.attr,
	&dev_attr_als_input.attr,
	&dev_attr_acc_input.attr,
	&dev_attr_akm_input.attr,
	&dev_attr_gyro_input.attr,
	NULL
};
static const struct attribute_group sensor_input = {
	.attrs = apds990x_input_attributes,
};
static int __init sensor_input_info_init(void)
{
	int ret = 0;
	printk(KERN_INFO"[%s] ++", __func__);

	ret = platform_device_register(&sensor_input_info);
	if (ret) {
		pr_err("%s: platform_device_register failed, ret:%d.\n",
				__func__, ret);
		goto REGISTER_ERR;
	}

	ret = sysfs_create_group(&sensor_input_info.dev.kobj, &sensor_input);
	if (ret) {
		pr_err("sensor_input_info_init sysfs_create_group error ret =%d", ret);
		goto SYSFS_CREATE_CGOUP_ERR;
	}
	printk(KERN_INFO"[%s] --", __func__);

	return 0;
SYSFS_CREATE_CGOUP_ERR:
	platform_device_unregister(&sensor_input_info);
REGISTER_ERR:
	return ret;

}

device_initcall(sensor_input_info_init);
MODULE_DESCRIPTION("sensor input info");
MODULE_AUTHOR("huawei driver group of k3v2");
MODULE_LICENSE("GPL");
