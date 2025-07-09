/*
 * Copyright (C)  Brandon Chuang
 *
 * This module supports the A15 cpld that hold the channel select
 * mechanism for other i2c slave devices, such as SFP.
 * This includes the:
 *  switch_port CPLD1/CPLD2/CPLD3/CPLD4
 *  switch_fan CPLD1/CPLD2
 *
 * Based on:
 *  pca954x.c from Kumar Gala <galak@kernel.crashing.org>
 * Copyright (C) 2006
 *
 * Based on:
 *  pca954x.c from Ken Harrenstien
 * Copyright (C) 2004 Google, Inc. (Ken Harrenstien)
 *
 * Based on:
 *  i2c-virtual_cb.c from Brian Kuschak <bkuschak@yahoo.com>
 * and
 *  pca9540.c from Jean Delvare <khali@linux-fr.org>.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/version.h>
#include <linux/stat.h>
#include <linux/hwmon-sysfs.h>
#include <linux/delay.h>
#ifdef C11_ANNEX_K
#include "libboundscheck/include/securec.h"
#endif
#include "switch_cpld_driver.h"


#define I2C_RW_RETRY_COUNT              10
#define I2C_RW_RETRY_INTERVAL           60 /* ms */

#define BOARD_VERSION_REG               0x00
#define MOJOR_VERSION_REG               0x01
#define MONOR_VERSION_REG               0x02

#define MAX_I2C_CPLD_NAME_LEN          20

static LIST_HEAD(cpld_client_list);
static struct mutex     list_lock;

struct cpld_client_node {
    struct i2c_client *client;
    struct list_head   list;
};


struct switch_i2c_cpld_data {
    char cpld_name[MAX_I2C_CPLD_NAME_LEN];
    u8 index;
    struct mutex     update_lock;
    struct attribute_group attr_group;
};

/* sysfs attributes for hwmon */
static int switch_i2c_cpld_read_internal(struct i2c_client *client, u8 reg);
static int switch_i2c_cpld_write_internal(struct i2c_client *client, u8 reg, u8 value);
static ssize_t i2c_cpld_show_byte(struct device *dev, struct device_attribute *devattr, char *buf);
//static ssize_t i2c_cpld_set_byte(struct device *dev, struct device_attribute *devattr, const char *buf, size_t count);


static SENSOR_DEVICE_ATTR(board_ver, S_IRUGO, i2c_cpld_show_byte, NULL, BOARD_VERSION_REG);
static SENSOR_DEVICE_ATTR(major_ver, S_IRUGO, i2c_cpld_show_byte, NULL, MOJOR_VERSION_REG);
static SENSOR_DEVICE_ATTR(minor_ver, S_IRUGO, i2c_cpld_show_byte, NULL, MONOR_VERSION_REG);


static struct attribute *switch_i2c_cpld_attributes[] = {
    &sensor_dev_attr_board_ver.dev_attr.attr,
    &sensor_dev_attr_major_ver.dev_attr.attr,
    &sensor_dev_attr_minor_ver.dev_attr.attr,
    NULL
};

static const struct attribute_group switch_i2c_cpld_group = {
    .attrs = switch_i2c_cpld_attributes,
};

static ssize_t i2c_cpld_show_byte(struct device *dev,
                         struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct switch_i2c_cpld_data *data = i2c_get_clientdata(client);
    unsigned char reg = attr->index;
    int status = 0;

    mutex_lock(&data->update_lock);
    status = switch_i2c_cpld_read_internal(client, reg);
    mutex_unlock(&data->update_lock);

    if (unlikely(status < 0)) {
        return status;
    }

    status = status & 0xff;
    return snprintf(buf, PAGE_SIZE, "0x%x\n", status);
}
#if 0
static ssize_t i2c_cpld_set_byte(struct device *dev, struct device_attribute *devattr,
                        const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct switch_i2c_cpld_data *data = i2c_get_clientdata(client);
    unsigned char reg = attr->index;
    unsigned char value;
    int status;

    status = kstrtou8(buf, 10, &value);
    if (status) {
        return status;
    }

    mutex_lock(&data->update_lock);
    status = switch_i2c_cpld_write_internal(client, reg, value);
    mutex_unlock(&data->update_lock);
    if (unlikely(status < 0)) {
        return status;;
    }

    return count;
}
#endif

static int switch_i2c_cpld_read_internal(struct i2c_client *client, u8 reg)
{
    int status = 0, retry = I2C_RW_RETRY_COUNT;

    while (retry) {
        status = i2c_smbus_read_byte_data(client, reg);
        if (unlikely(status < 0)) {
            msleep(I2C_RW_RETRY_INTERVAL);
            retry--;
            continue;
        }

        break;
    }

    return status;
}

static int switch_i2c_cpld_write_internal(struct i2c_client *client, u8 reg, u8 value)
{
    int status = 0, retry = I2C_RW_RETRY_COUNT;

    while (retry) {
        status = i2c_smbus_write_byte_data(client, reg, value);
        if (unlikely(status < 0)) {
            msleep(I2C_RW_RETRY_INTERVAL);
            retry--;
            continue;
        }

        break;
    }

    return status;
}

static void switch_i2c_cpld_add_client(struct i2c_client *client)
{
    struct cpld_client_node *node = kzalloc(sizeof(struct cpld_client_node), GFP_KERNEL);

    if (!node) {
        dev_dbg(&client->dev, "Can't allocate cpld_client_node (0x%x)\n", client->addr);
        return;
    }

    node->client = client;

    mutex_lock(&list_lock);
    list_add(&node->list, &cpld_client_list);
    mutex_unlock(&list_lock);
}

static void switch_i2c_cpld_remove_client(struct i2c_client *client)
{
    struct list_head *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int found = 0;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client == client) {
            found = 1;
            break;
        }
    }

    if (found) {
        list_del(list_node);
        kfree(cpld_node);
    }

    mutex_unlock(&list_lock);
}

/*
 * I2C init/probing/exit functions
 */
static int switch_i2c_cpld_probe(struct i2c_client *client,
             const struct i2c_device_id *id)
{
    struct i2c_adapter *adap = to_i2c_adapter(client->dev.parent);
    struct device *dev = &client->dev;
    struct switch_i2c_cpld_data *data;
    int ret = -ENODEV;

    if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE)) {
        dev_dbg(dev, "i2c_check_functionality failed (0x%x)\n", client->addr);
        return -EIO;
    }

    data = devm_kzalloc(dev, sizeof(struct switch_i2c_cpld_data), GFP_KERNEL);
    if (!data) {
        return -ENOMEM;
    }

    i2c_set_clientdata(client, data);
    mutex_init(&data->update_lock);

    if (client->adapter->nr == SYS_CPLD_BUS_NR) {
        strcpy(data->cpld_name, "sys_cpld");
        data->index = SYS_CPLD;
    }
    else if (client->adapter->nr == FAN_CPLD_BUS_NR) {
        strcpy(data->cpld_name, "fan_cpld");
        data->index = FAN_CPLD;
    }
    else if (client->adapter->nr == PORT_CPLD1_BUS_NR) {
        strcpy(data->cpld_name, "port_cpld1");
        data->index = PORT_CPLD1;
    }
    else if (client->adapter->nr == PORT_CPLD2_BUS_NR) {
        strcpy(data->cpld_name, "port_cpld2");
        data->index = PORT_CPLD2;
    }
    else {
        mutex_destroy(&data->update_lock);
        return -EINVAL;
    }

    /* Register sysfs hooks */
    data->attr_group = switch_i2c_cpld_group;
    if (&data->attr_group) {
        ret = sysfs_create_group(&client->dev.kobj, &data->attr_group);
        if (ret) {
            mutex_destroy(&data->update_lock);
            return ret;
        }
    }

    switch_i2c_cpld_add_client(client);
    return 0;
}

static int switch_i2c_cpld_remove(struct i2c_client *client)
{
    struct switch_i2c_cpld_data *data = i2c_get_clientdata(client);

    switch_i2c_cpld_remove_client(client);
    sysfs_remove_group(&client->dev.kobj, &data->attr_group);

    mutex_destroy(&data->update_lock);

    return 0;
}

int switch_i2c_cpld_read(u8 index, u8 reg, u8 *value)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    struct switch_i2c_cpld_data *data = NULL;
    int ret = -EPERM;

    mutex_lock(&list_lock);
    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        data = i2c_get_clientdata(cpld_node->client);
        if (data->index == index) {
            mutex_lock(&data->update_lock);
            ret = switch_i2c_cpld_read_internal(cpld_node->client, reg);
            mutex_unlock(&data->update_lock);
            break;
        }
    }
    mutex_unlock(&list_lock);

    if(ret < 0)
    {
        return -1;
    }
    *value = ret;
    return 0;
}
EXPORT_SYMBOL(switch_i2c_cpld_read);

int switch_i2c_cpld_write(u8 index, u8 reg, u8 value)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    struct switch_i2c_cpld_data *data = NULL;
    int ret = -EIO;
    
    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        data = i2c_get_clientdata(cpld_node->client);
        if (data->index == index) {
            mutex_lock(&data->update_lock);
            ret = switch_i2c_cpld_write_internal(cpld_node->client, reg, value);
            mutex_unlock(&data->update_lock);
            break;
        }
    }

    mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(switch_i2c_cpld_write);


static const struct i2c_device_id switch_i2c_cpld_id[] = {
    { "sys_cpld",   sys_cpld },
    { "fan_cpld",   fan_cpld },
    { "port_cpld1", port_cpld1 },
    { "port_cpld2", port_cpld2 },
    { }
};
MODULE_DEVICE_TABLE(i2c, switch_i2c_cpld_id);

static struct i2c_driver switch_i2c_cpld_driver = {
    .driver     = {
        .name   = "switch_i2c_cpld",
        .owner  = THIS_MODULE,
    },
    .probe      = switch_i2c_cpld_probe,
    .remove     = switch_i2c_cpld_remove,
    .id_table   = switch_i2c_cpld_id,
};

static int __init switch_i2c_cpld_init(void)
{
    mutex_init(&list_lock);
    return i2c_add_driver(&switch_i2c_cpld_driver);
}

static void __exit switch_i2c_cpld_exit(void)
{
    i2c_del_driver(&switch_i2c_cpld_driver);
    mutex_destroy(&list_lock);
}

module_init(switch_i2c_cpld_init);
module_exit(switch_i2c_cpld_exit);

MODULE_AUTHOR("Gil Wei");
MODULE_DESCRIPTION("Switch I2C CPLD Driver");
MODULE_LICENSE("GPL");
