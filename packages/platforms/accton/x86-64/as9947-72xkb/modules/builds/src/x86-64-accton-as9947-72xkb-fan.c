/*
 * Copyright (C)  Willy Liu <willy_liu@accton.com>
 *
 *
 * Based on:
 *    pca954x.c from Kumar Gala <galak@kernel.crashing.org>
 * Copyright (C) 2006
 *
 * Based on:
 *    pca954x.c from Ken Harrenstien
 * Copyright (C) 2004 Google, Inc. (Ken Harrenstien)
 *
 * Based on:
 *    i2c-virtual_cb.c from Brian Kuschak <bkuschak@yahoo.com>
 * and
 *    pca9540.c from Jean Delvare <khali@linux-fr.org>.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/stat.h>
#include <linux/sysfs.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/platform_device.h>
#include <linux/string_helpers.h>
#include "accton_ipmi_intf.h"

#define DRVNAME "as9947_72xkb_fan"
#define IPMI_FAN_READ_CMD 0x14
#define IPMI_FAN_WRITE_CMD 0x15
#define IPMI_FAN_READ_MODEL_CMD 0x10
#define IPMI_FAN_READ_SERIAL_CMD 0x11
#define IPMI_FAN_REG_READ_CMD 0x20
#define MAX_FAN_SPEED_RPM 33000
#define IPMI_FAN_MODEL_SIZE 15
#define IPMI_FAN_SERIAL_SIZE 13

static ssize_t set_fan(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count);
static ssize_t show_fan(struct device *dev, struct device_attribute *attr,
            char *buf);
static ssize_t show_string(struct device *dev, struct device_attribute *attr,
            char *buf);
static ssize_t show_version(struct device *dev, struct device_attribute *da,
            char *buf);
static ssize_t show_dir(struct device *dev, struct device_attribute *da,
            char *buf);
static int as9947_72xkb_fan_probe(struct platform_device *pdev);
static int as9947_72xkb_fan_remove(struct platform_device *pdev);

enum fan_id {
    FAN_1,
    FAN_2,
    FAN_3,
    FAN_4,
    FAN_5,
    FAN_6,
    FAN_7,
    FAN_8,
    FAN_9,
    FAN_10,
    FAN_11,
    FAN_12,
    FAN_13,
    FAN_14,
    FAN_15,
    FAN_16,
    NUM_OF_FAN,
    NUM_OF_FAN_MODULE = NUM_OF_FAN
};

enum fan_data_index {
    FAN_PRESENT,
    FAN_PWM,
    FAN_SPEED0,
    FAN_SPEED1,
    FAN_DATA_COUNT
};

struct as9947_72xkb_fan_data {
    struct platform_device *pdev;
    struct device   *hwmon_dev;
    struct mutex update_lock;
    char valid; /* != 0 if registers are valid */
    unsigned long last_updated;    /* In jiffies */
    /* 4 bytes for each fan, the last 2 bytes is fan dir */
    unsigned char ipmi_resp[NUM_OF_FAN * FAN_DATA_COUNT + 2];
    unsigned char ipmi_resp_cpld[2];
    unsigned char ipmi_resp_string[16];
    struct ipmi_data ipmi;
    unsigned char ipmi_tx_data[3];  /* 0: FAN id, 1: 0x02, 2: PWM */
};

struct as9947_72xkb_fan_data *data = NULL;

static struct platform_driver as9947_72xkb_fan_driver = {
    .probe = as9947_72xkb_fan_probe,
    .remove = as9947_72xkb_fan_remove,
    .driver = {
        .name = DRVNAME,
        .owner = THIS_MODULE,
    },
};

#define FAN_PRESENT_ATTR_ID(index) FAN##index##_PRESENT
#define FAN_PWM_ATTR_ID(index) FAN##index##_PWM
#define FAN_RPM_ATTR_ID(index) FAN##index##_INPUT
#define FAN_DIR_ATTR_ID(index) FAN##index##_DIR
#define FAN_MODEL_ATTR_ID(index) FAN##index##_MODEL
#define FAN_SERIAL_ATTR_ID(index) FAN##index##_SERIAL

#define FAN_ATTR(fan_id) \
    FAN_PRESENT_ATTR_ID(fan_id), \
    FAN_PWM_ATTR_ID(fan_id), \
    FAN_RPM_ATTR_ID(fan_id), \
    FAN_DIR_ATTR_ID(fan_id), \
    FAN_MODEL_ATTR_ID(fan_id), \
    FAN_SERIAL_ATTR_ID(fan_id)

enum as9947_72xkb_fan_sysfs_attrs {
    FAN_ATTR(1),
    FAN_ATTR(2),
    FAN_ATTR(3),
    FAN_ATTR(4),
    FAN_ATTR(5),
    FAN_ATTR(6),
    FAN_ATTR(7),
    FAN_ATTR(8),
    FAN_ATTR(9),
    FAN_ATTR(10),
    FAN_ATTR(11),
    FAN_ATTR(12),
    FAN_ATTR(13),
    FAN_ATTR(14),
    FAN_ATTR(15),
    FAN_ATTR(16),
    NUM_OF_FAN_ATTR,
    FAN_VERSION,
    FAN_MAX_RPM,
    NUM_OF_PER_FAN_ATTR = (NUM_OF_FAN_ATTR/NUM_OF_FAN)
};

/* fan attributes */
#define DECLARE_FAN_VER_SENSOR_DEVICE_ATTR() \
    static SENSOR_DEVICE_ATTR(version, S_IRUGO, show_version, NULL, FAN_VERSION)
#define DECLARE_FAN_VER_ATTR() \
    &sensor_dev_attr_version.dev_attr.attr

#define DECLARE_FAN_SENSOR_DEVICE_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_present, S_IRUGO, show_fan, NULL, \
                                FAN##index##_PRESENT); \
    static SENSOR_DEVICE_ATTR(fan##index##_pwm, S_IWUSR | S_IRUGO, show_fan, \
                                set_fan, FAN##index##_PWM); \
    static SENSOR_DEVICE_ATTR(fan##index##_input, S_IRUGO, show_fan, NULL, \
                                FAN##index##_INPUT); \
    static SENSOR_DEVICE_ATTR(fan##index##_dir, S_IRUGO, show_dir, NULL, \
                                FAN##index##_DIR); \
    static SENSOR_DEVICE_ATTR(fan##index##_model, S_IRUGO, show_string,\
                                NULL, FAN##index##_MODEL); \
    static SENSOR_DEVICE_ATTR(fan##index##_serial, S_IRUGO, show_string,\
                                NULL, FAN##index##_SERIAL)

static SENSOR_DEVICE_ATTR(fan_max_speed_rpm, S_IRUGO, show_fan, NULL, \
            FAN_MAX_RPM);
#define DECLARE_FAN_MAX_RPM_ATTR(index) \
            &sensor_dev_attr_fan_max_speed_rpm.dev_attr.attr

#define DECLARE_FAN_ATTR(index) \
    &sensor_dev_attr_fan##index##_present.dev_attr.attr, \
    &sensor_dev_attr_fan##index##_pwm.dev_attr.attr, \
    &sensor_dev_attr_fan##index##_input.dev_attr.attr, \
    &sensor_dev_attr_fan##index##_dir.dev_attr.attr, \
    &sensor_dev_attr_fan##index##_model.dev_attr.attr, \
    &sensor_dev_attr_fan##index##_serial.dev_attr.attr

DECLARE_FAN_SENSOR_DEVICE_ATTR(1);
DECLARE_FAN_SENSOR_DEVICE_ATTR(2);
DECLARE_FAN_SENSOR_DEVICE_ATTR(3);
DECLARE_FAN_SENSOR_DEVICE_ATTR(4);
DECLARE_FAN_SENSOR_DEVICE_ATTR(5);
DECLARE_FAN_SENSOR_DEVICE_ATTR(6);
DECLARE_FAN_SENSOR_DEVICE_ATTR(7);
DECLARE_FAN_SENSOR_DEVICE_ATTR(8);
DECLARE_FAN_SENSOR_DEVICE_ATTR(9);
DECLARE_FAN_SENSOR_DEVICE_ATTR(10);
DECLARE_FAN_SENSOR_DEVICE_ATTR(11);
DECLARE_FAN_SENSOR_DEVICE_ATTR(12);
DECLARE_FAN_SENSOR_DEVICE_ATTR(13);
DECLARE_FAN_SENSOR_DEVICE_ATTR(14);
DECLARE_FAN_SENSOR_DEVICE_ATTR(15);
DECLARE_FAN_SENSOR_DEVICE_ATTR(16);
DECLARE_FAN_VER_SENSOR_DEVICE_ATTR();

static struct attribute *as9947_72xkb_fan_attrs[] = {
    /* fan attributes */
    DECLARE_FAN_ATTR(1),
    DECLARE_FAN_ATTR(2),
    DECLARE_FAN_ATTR(3),
    DECLARE_FAN_ATTR(4),
    DECLARE_FAN_ATTR(5),
    DECLARE_FAN_ATTR(6),
    DECLARE_FAN_ATTR(7),
    DECLARE_FAN_ATTR(8),
    DECLARE_FAN_ATTR(9),
    DECLARE_FAN_ATTR(10),
    DECLARE_FAN_ATTR(11),
    DECLARE_FAN_ATTR(12),
    DECLARE_FAN_ATTR(13),
    DECLARE_FAN_ATTR(14),
    DECLARE_FAN_ATTR(15),
    DECLARE_FAN_ATTR(16),
    DECLARE_FAN_VER_ATTR(),
    DECLARE_FAN_MAX_RPM_ATTR(),
    NULL
};
ATTRIBUTE_GROUPS(as9947_72xkb_fan);

static struct as9947_72xkb_fan_data *as9947_72xkb_fan_update_device(void)
{
    int status = 0;

    if (time_before(jiffies, data->last_updated + HZ * 5) && data->valid)
    {
        return data;
    }

    data->valid = 0;

    status = ipmi_send_message(&data->ipmi, IPMI_FAN_READ_CMD, NULL, 0,
                                data->ipmi_resp, sizeof(data->ipmi_resp));
    if (unlikely(status != 0))
        goto exit;

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->last_updated = jiffies;
    data->valid = 1;

exit:
    return data;
}

static ssize_t show_fan(struct device *dev, struct device_attribute *da,
                            char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char fid = attr->index / NUM_OF_PER_FAN_ATTR;
    int value = 0;
    int index = 0;
    int present = 0;
    int error = 0;

    if (attr->index == FAN_MAX_RPM)
        return sprintf(buf, "%d\n", MAX_FAN_SPEED_RPM);

    mutex_lock(&data->update_lock);
    data = as9947_72xkb_fan_update_device();
    if (!data->valid) {
        error = -EIO;
        goto exit;
    }

    index = fid * FAN_DATA_COUNT; /* base index */
    present = !!data->ipmi_resp[index + FAN_PRESENT];

    switch (attr->index) {
    case FAN1_PRESENT:
    case FAN2_PRESENT:
    case FAN3_PRESENT:
    case FAN4_PRESENT:
    case FAN5_PRESENT:
    case FAN6_PRESENT:
    case FAN7_PRESENT:
    case FAN8_PRESENT:
    case FAN9_PRESENT:
    case FAN10_PRESENT:
    case FAN11_PRESENT:
    case FAN12_PRESENT:
    case FAN13_PRESENT:
    case FAN14_PRESENT:
    case FAN15_PRESENT:
    case FAN16_PRESENT:
        value = present;
        break;
    case FAN1_PWM:
    case FAN2_PWM:
    case FAN3_PWM:
    case FAN4_PWM:
    case FAN5_PWM:
    case FAN6_PWM:
    case FAN7_PWM:
    case FAN8_PWM:
    case FAN9_PWM:
    case FAN10_PWM:
    case FAN11_PWM:
    case FAN12_PWM:
    case FAN13_PWM:
    case FAN14_PWM:
    case FAN15_PWM:
    case FAN16_PWM:
        index = (fid % NUM_OF_FAN_MODULE) * FAN_DATA_COUNT;
        value = data->ipmi_resp[index + FAN_PWM];
        break;
    case FAN1_INPUT:
    case FAN2_INPUT:
    case FAN3_INPUT:
    case FAN4_INPUT:
    case FAN5_INPUT:
    case FAN6_INPUT:
    case FAN7_INPUT:
    case FAN8_INPUT:
    case FAN9_INPUT:
    case FAN10_INPUT:
    case FAN11_INPUT:
    case FAN12_INPUT:
    case FAN13_INPUT:
    case FAN14_INPUT:
    case FAN15_INPUT:
    case FAN16_INPUT:
        value = (int)data->ipmi_resp[index + FAN_SPEED0] |
                (int)data->ipmi_resp[index + FAN_SPEED1] << 8;
        break;
    default:
        error = -EINVAL;
        goto exit;
    }

    mutex_unlock(&data->update_lock);
    return sprintf(buf, "%d\n", value);

exit:
    mutex_unlock(&data->update_lock);
    return error;
}

static ssize_t set_fan(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count)
{
    long pwm;
    int status;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char fid = attr->index / NUM_OF_PER_FAN_ATTR;

    status = kstrtol(buf, 10, &pwm);
    if (status)
        return status;

    mutex_lock(&data->update_lock);
    data->ipmi_tx_data[0] = (fid % (NUM_OF_FAN_MODULE / 2)) + 1;
    data->ipmi_tx_data[1] = 0x02;
    data->ipmi_tx_data[2] = pwm;
    status = ipmi_send_message(&data->ipmi, IPMI_FAN_WRITE_CMD,
                                data->ipmi_tx_data, sizeof(data->ipmi_tx_data),
                                NULL, 0);
    if (unlikely(status != 0))
        goto exit;

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    /* force update */
    data->valid = 0;
    status = count;

exit:
    mutex_unlock(&data->update_lock);
    return status;
}

static struct as9947_72xkb_fan_data *as9947_72xkb_fan_update_cpld_ver(void)
{
    int status = 0;

    data->valid = 0;
    data->ipmi_tx_data[0] = 0x33;
    status = ipmi_send_message(&data->ipmi, IPMI_FAN_REG_READ_CMD,
                                data->ipmi_tx_data, 1,
                                data->ipmi_resp_cpld,
                                sizeof(data->ipmi_resp_cpld));
    if (unlikely(status != 0))
        goto exit;

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }

    data->last_updated = jiffies;
    data->valid = 1;

exit:
    return data;
}

static struct as9947_72xkb_fan_data *as9947_72xkb_fan_update_model_serial(int fan_id, int index)
{
    int status = 0;
    int string_size = 0 ;

    data->valid = 0;

    switch (index) {
    case FAN1_MODEL:
    case FAN2_MODEL:
    case FAN3_MODEL:
    case FAN4_MODEL:
    case FAN5_MODEL:
    case FAN6_MODEL:
    case FAN7_MODEL:
    case FAN8_MODEL:
    case FAN9_MODEL:
    case FAN10_MODEL:
        data->ipmi_tx_data[0] = IPMI_FAN_READ_MODEL_CMD;
        string_size = IPMI_FAN_MODEL_SIZE;
        data->ipmi_resp_string[IPMI_FAN_MODEL_SIZE] = '\0';
        break;
    case FAN1_SERIAL:
    case FAN2_SERIAL:
    case FAN3_SERIAL:
    case FAN4_SERIAL:
    case FAN5_SERIAL:
    case FAN6_SERIAL:
    case FAN7_SERIAL:
    case FAN8_SERIAL:
    case FAN9_SERIAL:
    case FAN10_SERIAL:
        data->ipmi_tx_data[0] = IPMI_FAN_READ_SERIAL_CMD;
        string_size = IPMI_FAN_SERIAL_SIZE;
        data->ipmi_resp_string[IPMI_FAN_SERIAL_SIZE] = '\0';
        break;
    default:
        goto exit;
    }

    if (fan_id > 4)
        data->ipmi_tx_data[1] = fan_id - 5;
    else
        data->ipmi_tx_data[1] = fan_id;
    status = ipmi_send_message(&data->ipmi, IPMI_FAN_READ_CMD,
                                data->ipmi_tx_data, 2,
                                data->ipmi_resp_string,
                                string_size);
    if (unlikely(status != 0))
        goto exit;

    if (unlikely(data->ipmi.rx_result != 0)) {
        status = -EIO;
        goto exit;
    }
    data->valid = 1;
exit:
    return data;
}

static ssize_t show_string(struct device *dev, struct device_attribute *da,
                            char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char fid = attr->index / NUM_OF_PER_FAN_ATTR;
    int present = 0;
    int error = 0;
    int index = 0;
    char *str = NULL;

    mutex_lock(&data->update_lock);
    /* check fan present */
    data = as9947_72xkb_fan_update_device();
    if (!data->valid) {
        error = -EIO;
        goto exit;
    }

    index = fid * FAN_DATA_COUNT; /* base index */
    present = !!data->ipmi_resp[index + FAN_PRESENT];
    mutex_unlock(&data->update_lock);
    if (!present)
        return sprintf(buf, "\n");

    mutex_lock(&data->update_lock);
    data = as9947_72xkb_fan_update_model_serial(fid, attr->index);
    if (!data->valid) {
        error = -EIO;
        goto exit;
    }
    mutex_unlock(&data->update_lock);

    str = data->ipmi_resp_string;
    return sprintf(buf, "%s\n", str);
    exit:
        mutex_unlock(&data->update_lock);
        return error;
}

static ssize_t show_version(struct device *dev, struct device_attribute *da,
                                char *buf)
{
    unsigned char major;
    unsigned char minor;
    int error = 0;

    mutex_lock(&data->update_lock);
    data = as9947_72xkb_fan_update_cpld_ver();
    if (!data->valid) {
        error = -EIO;
        goto exit;
    }

    major = data->ipmi_resp_cpld[0];
    minor = data->ipmi_resp_cpld[1];
    mutex_unlock(&data->update_lock);
    return sprintf(buf, "%d.%d\n", major, minor);

exit:
    mutex_unlock(&data->update_lock);
    return error;
}

static ssize_t show_dir(struct device *dev, struct device_attribute *da,
                            char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    unsigned char fid = (attr->index / NUM_OF_PER_FAN_ATTR);
    int value = 0;
    int index = 0;
    int present = 0;
    int error = 0;

    mutex_lock(&data->update_lock);
    data = as9947_72xkb_fan_update_device();
    if (!data->valid) {
        error = -EIO;
        goto exit;
    }

    index = fid * FAN_DATA_COUNT; /* base index */
    present = !!data->ipmi_resp[index + FAN_PRESENT];

    value = data->ipmi_resp[NUM_OF_FAN * FAN_DATA_COUNT] |
            (data->ipmi_resp[NUM_OF_FAN * FAN_DATA_COUNT + 1] << 8);
    mutex_unlock(&data->update_lock);

    if (!present)
        return sprintf(buf, "0\n");
    else
        return sprintf(buf, "%s\n",
                        (value & BIT(fid % NUM_OF_FAN_MODULE)) ? "B2F" : "F2B");

exit:
    mutex_unlock(&data->update_lock);
    return error;
}

static int as9947_72xkb_fan_probe(struct platform_device *pdev)
{
    int status = 0;
    struct device *hwmon_dev;

    hwmon_dev = hwmon_device_register_with_info(&pdev->dev, DRVNAME,
                    NULL, NULL, as9947_72xkb_fan_groups);
    if (IS_ERR(data->hwmon_dev)) {
        status = PTR_ERR(data->hwmon_dev);
        return status;
    }

    mutex_lock(&data->update_lock);
    data->hwmon_dev = hwmon_dev;
    mutex_unlock(&data->update_lock);

    dev_info(&pdev->dev, "Device Created\n");

    return status;
}

static int as9947_72xkb_fan_remove(struct platform_device *pdev)
{
    mutex_lock(&data->update_lock);
    if (data->hwmon_dev) {
        hwmon_device_unregister(data->hwmon_dev);
        data->hwmon_dev = NULL;
    }
    mutex_unlock(&data->update_lock);

    return 0;
}

static int __init as9947_72xkb_fan_init(void)
{
    int ret;

    data = kzalloc(sizeof(struct as9947_72xkb_fan_data), GFP_KERNEL);
    if (!data) {
        ret = -ENOMEM;
        goto alloc_err;
    }

    mutex_init(&data->update_lock);

    ret = platform_driver_register(&as9947_72xkb_fan_driver);
    if (ret < 0)
        goto dri_reg_err;

    data->pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
    if (IS_ERR(data->pdev)) {
        ret = PTR_ERR(data->pdev);
        goto dev_reg_err;
    }

    /* Set up IPMI interface */
    ret = init_ipmi_data(&data->ipmi, 0, &data->pdev->dev);
    if (ret) {
        goto ipmi_err;
    }

    return 0;

ipmi_err:
    platform_device_unregister(data->pdev);
dev_reg_err:
    platform_driver_unregister(&as9947_72xkb_fan_driver);
dri_reg_err:
    kfree(data);
alloc_err:
    return ret;
}

static void __exit as9947_72xkb_fan_exit(void)
{
    if (data) {
        ipmi_destroy_user(data->ipmi.user);
        platform_device_unregister(data->pdev);
        kfree(data);
    }
    platform_driver_unregister(&as9947_72xkb_fan_driver);
}

MODULE_AUTHOR("Willy Liu <willy_liu@accton.com>");
MODULE_DESCRIPTION("as9947_72xkb_fan driver");
MODULE_LICENSE("GPL");

module_init(as9947_72xkb_fan_init);
module_exit(as9947_72xkb_fan_exit);
