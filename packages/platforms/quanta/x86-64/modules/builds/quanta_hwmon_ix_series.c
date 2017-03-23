/*
 * <bsn.cl fy=2013 v=gpl>
 *
 *        Copyright 2013, 2014 BigSwitch Networks, Inc.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under  the terms ofthe GNU General Public License as
 * published by the Free Software Foundation;  either version 2 of the  License,
 * or (at your option) any later version.
 *
 *
 * </bsn.cl>
 *
 * A hwmon driver for the Quanta IX1/IX2
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/log2.h>
#include <linux/kthread.h>
#include <linux/slab.h>

static const unsigned short normal_i2c[] = { 0x4E, I2C_CLIENT_END };

#define QUANTA_HWMON_REG_TEMP_INPUT_BASE 0x20
#define QUANTA_HWMON_REG_FAN_MODE 0x33
#define QUANTA_HWMON_REG_FAN_DIR 0x56
#define QUANTA_HWMON_REG_FAN_PWM_BASE 0x3C
#define QUANTA_HWMON_REG_FAN_INPUT_BASE 0x40

#define QUANTA_HWMON_FAN_MANUAL_MODE 1
#define QUANTA_HWMON_FAN_SMART_MODE 3

#define QUANTA_HWMON_NUM_FANS 6

struct quanta_hwmon_ix_series_data {
	struct device		*hwmon_dev;
	struct attribute_group	attrs;
	struct mutex		lock;
};

enum quanta_hwmon_ix_series_s {
	quanta_ix1_hwmon,
	quanta_ix2_hwmon,
};

static int quanta_hwmon_ix_series_probe(struct i2c_client *client,
			 const struct i2c_device_id *id);
static int quanta_hwmon_ix_series_remove(struct i2c_client *client);

static const struct i2c_device_id quanta_hwmon_ix_series_id[] = {
	{ "quanta_ix1_hwmon",	quanta_ix1_hwmon },
	{ "quanta_ix2_hwmon",	quanta_ix2_hwmon },
	{ }
};
MODULE_DEVICE_TABLE(i2c, quanta_hwmon_ix_series_id);

static struct i2c_driver quanta_hwmon_ix_series_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "quanta_hwmon_ix_series",
	},
	.probe		= quanta_hwmon_ix_series_probe,
	.remove		= quanta_hwmon_ix_series_remove,
	.id_table	= quanta_hwmon_ix_series_id,
	.address_list	= normal_i2c,
};

static ssize_t show_temp(struct device *dev, struct device_attribute *devattr,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct quanta_hwmon_ix_series_data *data = i2c_get_clientdata(client);
	int temp;

	mutex_lock(&data->lock);
	temp = i2c_smbus_read_byte_data(client,
					QUANTA_HWMON_REG_TEMP_INPUT_BASE
					+ attr->index);
	mutex_unlock(&data->lock);
	return sprintf(buf, "%d\n", 1000 * temp);
}

static ssize_t show_fan(struct device *dev, struct device_attribute *devattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct quanta_hwmon_ix_series_data *data = i2c_get_clientdata(client);
	int fan;

	mutex_lock(&data->lock);
	fan = i2c_smbus_read_word_swapped(client,
					  QUANTA_HWMON_REG_FAN_INPUT_BASE
					  + 2 * attr->index);
	mutex_unlock(&data->lock);
	return sprintf(buf, "%d\n", fan);
}

static ssize_t show_pwm(struct device *dev, struct device_attribute *devattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct quanta_hwmon_ix_series_data *data = i2c_get_clientdata(client);
	int pwm;

	mutex_lock(&data->lock);
	pwm = i2c_smbus_read_word_swapped(client,
					  QUANTA_HWMON_REG_FAN_PWM_BASE
					  + 2 * attr->index);
	mutex_unlock(&data->lock);
	return sprintf(buf, "%d\n", pwm * 255 / 10000);
}

static ssize_t set_pwm(struct device *dev, struct device_attribute *devattr,
		const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct quanta_hwmon_ix_series_data *data = i2c_get_clientdata(client);
	long val;
	int ret;

	ret = kstrtol(buf, 10, &val);
	if (ret)
		return ret;
	mutex_lock(&data->lock);
	i2c_smbus_write_word_swapped(client,
				     QUANTA_HWMON_REG_FAN_PWM_BASE
				     + 2 * attr->index, val * 10000 / 255);
	mutex_unlock(&data->lock);
	return count;
}

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, show_temp, NULL, 0);
static SENSOR_DEVICE_ATTR(temp2_input, S_IRUGO, show_temp, NULL, 1);
static SENSOR_DEVICE_ATTR(temp3_input, S_IRUGO, show_temp, NULL, 2);
static SENSOR_DEVICE_ATTR(temp5_input, S_IRUGO, show_temp, NULL, 4);
static SENSOR_DEVICE_ATTR(temp6_input, S_IRUGO, show_temp, NULL, 5);
static SENSOR_DEVICE_ATTR(temp7_input, S_IRUGO, show_temp, NULL, 6);//For IX2
static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO, show_fan, NULL, 0);
static SENSOR_DEVICE_ATTR(fan2_input, S_IRUGO, show_fan, NULL, 1);
static SENSOR_DEVICE_ATTR(fan3_input, S_IRUGO, show_fan, NULL, 2);
static SENSOR_DEVICE_ATTR(fan4_input, S_IRUGO, show_fan, NULL, 3);//For IX1, IX2
static SENSOR_DEVICE_ATTR(fan5_input, S_IRUGO, show_fan, NULL, 4);
static SENSOR_DEVICE_ATTR(fan6_input, S_IRUGO, show_fan, NULL, 5);
static SENSOR_DEVICE_ATTR(fan7_input, S_IRUGO, show_fan, NULL, 6);
static SENSOR_DEVICE_ATTR(fan8_input, S_IRUGO, show_fan, NULL, 7);//For IX1, IX2
static SENSOR_DEVICE_ATTR(pwm1, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 0);

static struct attribute *quanta_hwmon_ix_series_attr_5temps_4fans[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_temp5_input.dev_attr.attr,
	&sensor_dev_attr_temp6_input.dev_attr.attr,
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&sensor_dev_attr_fan7_input.dev_attr.attr,
	&sensor_dev_attr_fan8_input.dev_attr.attr,
	&sensor_dev_attr_pwm1.dev_attr.attr,
	NULL
};

static struct attribute *quanta_hwmon_ix_series_attr_6temps_4fans[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_temp5_input.dev_attr.attr,
	&sensor_dev_attr_temp6_input.dev_attr.attr,
	&sensor_dev_attr_temp7_input.dev_attr.attr,
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&sensor_dev_attr_fan7_input.dev_attr.attr,
	&sensor_dev_attr_fan8_input.dev_attr.attr,
	&sensor_dev_attr_pwm1.dev_attr.attr,
	NULL
};

static int quanta_hwmon_ix_series_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct quanta_hwmon_ix_series_data *data;
	int err;

	data = devm_kzalloc(&client->dev, sizeof(struct quanta_hwmon_ix_series_data),
			GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);
	mutex_init(&data->lock);

	dev_info(&client->dev, "%s chip found\n", client->name);
    if(!strcmp(client->name, "quanta_ix1_hwmon")){
	    data->attrs.attrs = quanta_hwmon_ix_series_attr_5temps_4fans;
    }
    else{
	    data->attrs.attrs = quanta_hwmon_ix_series_attr_6temps_4fans;
    }
	err = sysfs_create_group(&client->dev.kobj, &data->attrs);
	if (err)
		return err;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	i2c_smbus_write_byte_data(client,
				QUANTA_HWMON_REG_FAN_MODE,
				QUANTA_HWMON_FAN_SMART_MODE);

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &data->attrs);
	return err;
}

static int quanta_hwmon_ix_series_remove(struct i2c_client *client)
{
	struct quanta_hwmon_ix_series_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &data->attrs);
	return 0;
}

module_i2c_driver(quanta_hwmon_ix_series_driver);

MODULE_AUTHOR("Jonathan Tsai (jonathan.tsai@quantatw.com)");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("Quanta IX1/IX2 hardware monitor driver");
MODULE_LICENSE("GPL");
