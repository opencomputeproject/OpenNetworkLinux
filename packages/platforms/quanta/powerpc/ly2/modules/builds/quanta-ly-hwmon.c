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
 * A hwmon driver for the Quanta LYx
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

static const unsigned short normal_i2c[] = { 0x2C, 0x2E, 0x2F, I2C_CLIENT_END };

#define QUANTA_LY_HWMON_REG_TEMP_INPUT_BASE 0x30
#define QUANTA_LY_HWMON_REG_FAN_MODE 0x55
#define QUANTA_LY_HWMON_REG_FAN_DIR 0x56
#define QUANTA_LY_HWMON_REG_FAN_PWM_BASE 0x60
#define QUANTA_LY_HWMON_REG_FAN_INPUT_BASE 0x80

#define QUANTA_LY_HWMON_FAN_MANUAL_MODE 1
#define QUANTA_LY_HWMON_FAN_AUTO_MODE 2

#define QUANTA_LY_HWMON_NUM_FANS 8

struct quanta_ly_hwmon_data {
	struct device		*hwmon_dev;
	struct attribute_group	attrs;
	struct mutex		lock;
};

static int quanta_ly_hwmon_probe(struct i2c_client *client,
			 const struct i2c_device_id *id);
static int quanta_ly_hwmon_remove(struct i2c_client *client);

static const struct i2c_device_id quanta_ly_hwmon_id[] = {
	{ "quanta_ly_hwmon", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, quanta_ly_hwmon_id);

static struct i2c_driver quanta_ly_hwmon_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "quanta_ly_hwmon",
	},
	.probe		= quanta_ly_hwmon_probe,
	.remove		= quanta_ly_hwmon_remove,
	.id_table	= quanta_ly_hwmon_id,
	.address_list	= normal_i2c,
};

static ssize_t show_temp(struct device *dev, struct device_attribute *devattr,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct quanta_ly_hwmon_data *data = i2c_get_clientdata(client);
	int temp;

	mutex_lock(&data->lock);
	temp = i2c_smbus_read_byte_data(client,
					QUANTA_LY_HWMON_REG_TEMP_INPUT_BASE
					+ attr->index);
	mutex_unlock(&data->lock);
	return sprintf(buf, "%d\n", 1000 * temp);
}

static ssize_t show_fan(struct device *dev, struct device_attribute *devattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct quanta_ly_hwmon_data *data = i2c_get_clientdata(client);
	int fan;

	mutex_lock(&data->lock);
	fan = i2c_smbus_read_word_swapped(client,
					  QUANTA_LY_HWMON_REG_FAN_INPUT_BASE
					  + 2 * attr->index);
	mutex_unlock(&data->lock);
	return sprintf(buf, "%d\n", fan);
}

static ssize_t show_pwm(struct device *dev, struct device_attribute *devattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct quanta_ly_hwmon_data *data = i2c_get_clientdata(client);
	int pwm;

	mutex_lock(&data->lock);
	pwm = i2c_smbus_read_word_swapped(client,
					  QUANTA_LY_HWMON_REG_FAN_PWM_BASE
					  + 2 * attr->index);
	mutex_unlock(&data->lock);
        return sprintf(buf, "%d\n", pwm * 255 / 10000);
}

static ssize_t set_pwm(struct device *dev, struct device_attribute *devattr,
		const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct quanta_ly_hwmon_data *data = i2c_get_clientdata(client);
	long val;
	int ret;

	ret = kstrtol(buf, 10, &val);
	if (ret)
		return ret;
	mutex_lock(&data->lock);
	i2c_smbus_write_word_swapped(client,
				     QUANTA_LY_HWMON_REG_FAN_PWM_BASE
                                     + 2 * attr->index, val * 10000 / 255);
	mutex_unlock(&data->lock);
	return count;
}

static ssize_t show_fan_dir(struct device *dev,
			    struct device_attribute *devattr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct quanta_ly_hwmon_data *data = i2c_get_clientdata(client);

        int f2b = 0;
        int b2f = 0;
        int i;

	mutex_lock(&data->lock);
        for(i = 0; i < 4; i++) {
            f2b += i2c_smbus_read_word_swapped(client,
                                               QUANTA_LY_HWMON_REG_FAN_INPUT_BASE
                                               + 2 * i);
        }
        for(i = 4; i < 8; i++) {
            b2f += i2c_smbus_read_word_swapped(client,
                                               QUANTA_LY_HWMON_REG_FAN_INPUT_BASE
                                               + 2 * i);
        }

	mutex_unlock(&data->lock);
        if(f2b) {
            return sprintf(buf, "front-to-back");
        }
        if(b2f) {
            return sprintf(buf, "back-to-front");
        }
        return sprintf(buf, "unknown");
}

static ssize_t set_fan_dir(struct device *dev,
			   struct device_attribute *devattr, const char *buf,
			   size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct quanta_ly_hwmon_data *data = i2c_get_clientdata(client);
	int dir;

	if (!strncmp(buf, "normal", 6))
		dir = 0;
	else if (!strncmp(buf, "reverse", 7))
		dir = 1;
	else
		return -EINVAL;

	mutex_lock(&data->lock);
	i2c_smbus_write_byte_data(client,
				  QUANTA_LY_HWMON_REG_FAN_DIR, dir);
	mutex_unlock(&data->lock);
	return count;
}

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, show_temp, NULL, 0);
static SENSOR_DEVICE_ATTR(temp2_input, S_IRUGO, show_temp, NULL, 1);
static SENSOR_DEVICE_ATTR(temp3_input, S_IRUGO, show_temp, NULL, 2);
static SENSOR_DEVICE_ATTR(temp4_input, S_IRUGO, show_temp, NULL, 3);
static SENSOR_DEVICE_ATTR(temp5_input, S_IRUGO, show_temp, NULL, 4);
static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO, show_fan, NULL, 0);
static SENSOR_DEVICE_ATTR(fan2_input, S_IRUGO, show_fan, NULL, 1);
static SENSOR_DEVICE_ATTR(fan3_input, S_IRUGO, show_fan, NULL, 2);
static SENSOR_DEVICE_ATTR(fan4_input, S_IRUGO, show_fan, NULL, 3);
static SENSOR_DEVICE_ATTR(fan5_input, S_IRUGO, show_fan, NULL, 4);
static SENSOR_DEVICE_ATTR(fan6_input, S_IRUGO, show_fan, NULL, 5);
static SENSOR_DEVICE_ATTR(fan7_input, S_IRUGO, show_fan, NULL, 6);
static SENSOR_DEVICE_ATTR(fan8_input, S_IRUGO, show_fan, NULL, 7);
static SENSOR_DEVICE_ATTR(pwm1, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 0);
static SENSOR_DEVICE_ATTR(pwm2, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 1);
static SENSOR_DEVICE_ATTR(pwm3, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 2);
static SENSOR_DEVICE_ATTR(pwm4, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 3);
static SENSOR_DEVICE_ATTR(pwm5, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 4);
static SENSOR_DEVICE_ATTR(pwm6, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 5);
static SENSOR_DEVICE_ATTR(pwm7, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 6);
static SENSOR_DEVICE_ATTR(pwm8, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 7);
static SENSOR_DEVICE_ATTR(fan_dir, S_IWUSR | S_IRUGO, show_fan_dir,
			  set_fan_dir, 0);

static struct attribute *quanta_ly_hwmon_attr[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_temp4_input.dev_attr.attr,
	&sensor_dev_attr_temp5_input.dev_attr.attr,
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&sensor_dev_attr_fan7_input.dev_attr.attr,
	&sensor_dev_attr_fan8_input.dev_attr.attr,
	&sensor_dev_attr_pwm1.dev_attr.attr,
	&sensor_dev_attr_pwm2.dev_attr.attr,
	&sensor_dev_attr_pwm3.dev_attr.attr,
	&sensor_dev_attr_pwm4.dev_attr.attr,
	&sensor_dev_attr_pwm5.dev_attr.attr,
	&sensor_dev_attr_pwm6.dev_attr.attr,
	&sensor_dev_attr_pwm7.dev_attr.attr,
	&sensor_dev_attr_pwm8.dev_attr.attr,
	&sensor_dev_attr_fan_dir.dev_attr.attr,
	NULL
};

static int quanta_ly_hwmon_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct quanta_ly_hwmon_data *data;
	int err;
	int i;

	data = devm_kzalloc(&client->dev, sizeof(struct quanta_ly_hwmon_data),
			GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);
	mutex_init(&data->lock);

	dev_info(&client->dev, "%s chip found\n", client->name);

	data->attrs.attrs = quanta_ly_hwmon_attr;
	err = sysfs_create_group(&client->dev.kobj, &data->attrs);
	if (err)
		return err;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	i2c_smbus_write_byte_data(client,
				QUANTA_LY_HWMON_REG_FAN_MODE,
				QUANTA_LY_HWMON_FAN_MANUAL_MODE);
	for (i = 0; i < QUANTA_LY_HWMON_NUM_FANS; i++) {
		u8 cmd = QUANTA_LY_HWMON_REG_FAN_PWM_BASE + i * 2;
		i2c_smbus_write_word_swapped(client, cmd, 10000);
	}

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &data->attrs);
	return err;
}

static int quanta_ly_hwmon_remove(struct i2c_client *client)
{
	struct quanta_ly_hwmon_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &data->attrs);
	return 0;
}

module_i2c_driver(quanta_ly_hwmon_driver);

MODULE_AUTHOR("Big Switch Networks <support@bigswitch.com>");
MODULE_DESCRIPTION("Quanta LYx hardware monitor driver");
MODULE_LICENSE("GPL");
