/*
 * A hwmon driver for the Accton as7816-64x fan
 *
 * Copyright (C) 2014 Accton Technology Corporation.
 * Brandon Chuang <brandon_chuang@accton.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/dmi.h>

#define DRVNAME "fan"

#define VAL_TO_RPM_FACTOR 150

#define FAN_LED_OFF	  3
#define FAN_LED_AMBER	  2 
#define FAN_LED_GREEN	  1 
#define FAN_WDT_CTRL      0x30

static struct fan_data *fan_update_device(struct device *dev);                    
static ssize_t fan_show_value(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count);

static ssize_t set_led_ctrl(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count);

/* fan related data, the index should match sysfs_fan_attributes
 */
static const u8 fan_reg[] = {
    0x32,  /* FAN1 PWM Reg */     
    0x36,  /* FAN2 PWM Reg */   
    0x3a,  /* FAN3 PWM Reg */      
    0x31,  /* FAN1 Speed Reg */       
    0x35,  /* FAN2 Speed Reg */   
    0x39,  /* FAN3 Speed Reg */     
    0x88,  /* FAN1 Direction Reg */     
    0x88,  /* FAN2 Direction Reg */   
    0x88,  /* FAN3 Direction Reg */   
    0x31,  /* FAN1 Present Reg */ 
    0x35,  /* FAN2 Present Reg */ 
    0x39,  /* FAN3 Present Reg */
    0x31,  /* FAN1 Fault Reg */
    0x35,  /* FAN2 Fault Reg */
    0x39,  /* FAN3 Fault Reg */
    0x33,  /* FAN1 Led Control Reg */
    0x37,  /* FAN2 Led Control Reg */
    0x3B,  /* FAN3 Led Control Reg */
};

/* Each client has this additional data */
struct fan_data {
    struct device   *hwmon_dev;
    struct mutex     update_lock;
    char             valid;           /* != 0 if registers are valid */
    unsigned long    last_updated;    /* In jiffies */
    u8               reg_val[ARRAY_SIZE(fan_reg)]; /* Register value */
};

enum fan_id {
    FAN1_ID,
    FAN2_ID,
    FAN3_ID
};

enum sysfs_fan_attributes {
    FAN1_DUTY_CYCLE_PERCENTAGE, 
    FAN2_DUTY_CYCLE_PERCENTAGE, 
    FAN3_DUTY_CYCLE_PERCENTAGE, 
    FAN1_SPEED_RPM,
    FAN2_SPEED_RPM,
    FAN3_SPEED_RPM,
    FAN1_DIRECTION,
    FAN2_DIRECTION,
    FAN3_DIRECTION,
    FAN1_PRESENT,
    FAN2_PRESENT,
    FAN3_PRESENT,
    FAN1_FAULT,
    FAN2_FAULT,
    FAN3_FAULT,
    FAN1_LED,
    FAN2_LED,
    FAN3_LED,
};

/* Define attributes
 */
#define DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_fault, S_IRUGO, fan_show_value, NULL, FAN##index##_FAULT);

#define DECLARE_FAN_FAULT_ATTR(index)      &sensor_dev_attr_fan##index##_fault.dev_attr.attr 

#define DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_direction, S_IRUGO, fan_show_value, NULL, FAN##index##_DIRECTION); 

#define DECLARE_FAN_DIRECTION_ATTR(index)  &sensor_dev_attr_fan##index##_direction.dev_attr.attr

/*#define DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_duty_cycle_percentage, S_IWUSR | S_IRUGO, fan_show_value, set_duty_cycle, FAN##index##_DUTY_CYCLE_PERCENTAGE);*/

#define DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(pwm##index, S_IWUSR | S_IRUGO, fan_show_value, set_duty_cycle, FAN##index##_DUTY_CYCLE_PERCENTAGE);

/*#define DECLARE_FAN_DUTY_CYCLE_ATTR(index) &sensor_dev_attr_fan##index##_duty_cycle_percentage.dev_attr.attr*/

#define DECLARE_FAN_DUTY_CYCLE_ATTR(index) &sensor_dev_attr_pwm##index.dev_attr.attr

#define DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_present, S_IRUGO, fan_show_value, NULL, FAN##index##_PRESENT); 

#define DECLARE_FAN_PRESENT_ATTR(index)      &sensor_dev_attr_fan##index##_present.dev_attr.attr

/*#define DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_speed_rpm, S_IRUGO, fan_show_value, NULL, FAN##index##_SPEED_RPM);*/

#define DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_input, S_IRUGO, fan_show_value, NULL, FAN##index##_SPEED_RPM);

/*#define DECLARE_FAN_SPEED_RPM_ATTR(index)  &sensor_dev_attr_fan##index##_speed_rpm.dev_attr.attr*/

#define DECLARE_FAN_SPEED_RPM_ATTR(index)  &sensor_dev_attr_fan##index##_input.dev_attr.attr

#define DECLARE_FAN_LED_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan##index##_led, S_IRUGO | S_IWUSR, fan_show_value, set_led_ctrl, FAN##index##_LED);

#define DECLARE_FAN_LED_ATTR(index) &sensor_dev_attr_fan##index##_led.dev_attr.attr

/* 3 fan fault attributes in this platform */
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(1);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(2);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(3);
/* 3 fan speed(rpm) attributes in this platform */
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(1);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(2);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(3);
/* 6 fan present attributes in this platform */
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(1);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(2);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(3);
/* 6 fan direction attribute in this platform */
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(1);
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(2);
DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(3);
/* 1 fan duty cycle attribute in this platform */
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(1);
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(2);
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(3);

DECLARE_FAN_LED_SENSOR_DEV_ATTR(1);
DECLARE_FAN_LED_SENSOR_DEV_ATTR(2);
DECLARE_FAN_LED_SENSOR_DEV_ATTR(3);

static struct attribute *fan_attributes[] = {
    /* fan related attributes */
    DECLARE_FAN_DUTY_CYCLE_ATTR(1),
    DECLARE_FAN_DUTY_CYCLE_ATTR(2),
    DECLARE_FAN_DUTY_CYCLE_ATTR(3),
    DECLARE_FAN_SPEED_RPM_ATTR(1),
    DECLARE_FAN_SPEED_RPM_ATTR(2),
    DECLARE_FAN_SPEED_RPM_ATTR(3),
    DECLARE_FAN_DIRECTION_ATTR(1),
    DECLARE_FAN_DIRECTION_ATTR(2),
    DECLARE_FAN_DIRECTION_ATTR(3),
    DECLARE_FAN_PRESENT_ATTR(1),
    DECLARE_FAN_PRESENT_ATTR(2),
    DECLARE_FAN_PRESENT_ATTR(3),

    DECLARE_FAN_FAULT_ATTR(1),
    DECLARE_FAN_FAULT_ATTR(2),
    DECLARE_FAN_FAULT_ATTR(3),

    DECLARE_FAN_LED_ATTR(1),
    DECLARE_FAN_LED_ATTR(2),
    DECLARE_FAN_LED_ATTR(3),

    NULL
};

#define FAN_DUTY_CYCLE_REG_MASK         0xFF
#define FAN_MAX_DUTY_CYCLE              100

static int fan_read_value(struct i2c_client *client, u8 reg)
{
    return i2c_smbus_read_byte_data(client, reg);
}

static int fan_write_value(struct i2c_client *client, u8 reg, u8 value)
{
    return i2c_smbus_write_byte_data(client, reg, value);
}

/* fan utility functions
 */
static u8 reg_val_to_duty_cycle(u8 reg_val)
{
    u8 res = 0; 

    reg_val &= FAN_DUTY_CYCLE_REG_MASK;

    if (!reg_val) {
	return 0;
    }

    res = (u8)(reg_val); 
    return res;
}

static u8 duty_cycle_to_reg_val(u8 duty_cycle) 
{
	if (duty_cycle >= FAN_DUTY_CYCLE_REG_MASK) {
		return FAN_DUTY_CYCLE_REG_MASK;
	}

    return duty_cycle;
}

static u32 reg_val_to_speed_rpm(u8 reg_val)
{
    if (reg_val == 0 || reg_val == 255) {
        return 0;
    } else {
        u64 f;
        f = VAL_TO_RPM_FACTOR*reg_val; 
        return (u32)f;
    }
}

static u8 reg_val_to_direction(enum fan_id index, u8 reg_val)
{
    u8 tmp = reg_val >> index;
    tmp &= 0x1; 
    return tmp;
}

static u8 reg_val_to_color(u8 reg_val, char **reg_color)
{
    switch(reg_val)
    {
        case FAN_LED_OFF:
	    return sprintf(*reg_color, "%s\n", "off");	
	case FAN_LED_AMBER:
	    return sprintf(*reg_color, "%s\n", "amber"); 
	case FAN_LED_GREEN:
	    return sprintf(*reg_color, "%s\n", "green");
	default:
	    return sprintf(*reg_color, "invalid:%d\n", reg_val); 
    }
}

static u8 reg_val_to_is_present(u8 reg_val)
{
    if (reg_val == 0 || reg_val == 255) {
        return 1;
    } else {
        return 0;
    }

}

static u8 is_fan_fault(u8 reg_val)
{
    u8 ret = 1;

    /* Check if the speed of front or rear fan is ZERO,  
     */
    if (reg_val_to_speed_rpm(reg_val))
    {
        ret = 0;
    }

    return ret;
}

static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count) 
{
    int error, value;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

    error = kstrtoint(buf, 10, &value);
    if (error)
        return error;
        
    if (value < 0 || value > FAN_DUTY_CYCLE_REG_MASK)
        return -EINVAL;

    if (attr->index > FAN3_DUTY_CYCLE_PERCENTAGE || attr->index < FAN1_DUTY_CYCLE_PERCENTAGE)
	return -EINVAL;

    fan_write_value(client, fan_reg[attr->index], duty_cycle_to_reg_val(value));
    return count;
}

static ssize_t set_led_ctrl(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count)
{
    int value;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

    if (!buf)
        return -EINVAL;

    if(sysfs_streq(buf, "amber")){
	value = FAN_LED_AMBER;
    }else if(sysfs_streq(buf, "green")){
   	value = FAN_LED_GREEN; 
    }else if(sysfs_streq(buf, "off")){
        value = FAN_LED_OFF;
    }else{
   	return -EINVAL; 
    }

    fan_write_value(client, fan_reg[attr->index], value);
    return count;
}

static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct fan_data *data = fan_update_device(dev);
    ssize_t ret = 0;
    u8 fan_index = 0;   
    u8 duty_cycle = 0;
 
    if (data->valid) {
        switch (attr->index) {
            case FAN1_DUTY_CYCLE_PERCENTAGE:
            case FAN2_DUTY_CYCLE_PERCENTAGE:
            case FAN3_DUTY_CYCLE_PERCENTAGE:
            {
                duty_cycle = reg_val_to_duty_cycle(data->reg_val[attr->index]);
                ret = sprintf(buf, "%u\n", duty_cycle);
                break;
            }
            case FAN1_SPEED_RPM:
            case FAN2_SPEED_RPM:
            case FAN3_SPEED_RPM:
			{
                ret = sprintf(buf, "%u\n", reg_val_to_speed_rpm(data->reg_val[attr->index]));
                break;
			}
	    /* 0: present 1: absent */
            case FAN1_PRESENT:
            case FAN2_PRESENT:
            case FAN3_PRESENT:
                ret = sprintf(buf, "%d\n",
                              reg_val_to_is_present(data->reg_val[attr->index]));
                break;
	    /*0: normal 1: fault */
            case FAN1_FAULT:
            case FAN2_FAULT:
            case FAN3_FAULT:
                ret = sprintf(buf, "%d\n", is_fan_fault(data->reg_val[attr->index]));
                break;
	    /* 1: b2f  0: f2b */
            case FAN1_DIRECTION:
		fan_index = FAN1_ID;	
		ret = sprintf(buf, "%d\n",
                              reg_val_to_direction(fan_index, data->reg_val[attr->index]));
		break;
            case FAN2_DIRECTION:
		fan_index = FAN2_ID;
		ret = sprintf(buf, "%d\n",
                              reg_val_to_direction(fan_index, data->reg_val[attr->index]));
		break;
            case FAN3_DIRECTION:
		fan_index = FAN3_ID;
                ret = sprintf(buf, "%d\n",
                              reg_val_to_direction(fan_index, data->reg_val[attr->index]));
                break;
	    case FAN1_LED:
	    case FAN2_LED:
	    case FAN3_LED:
		ret = reg_val_to_color(data->reg_val[attr->index], &buf);
		break;

	    default:
                break;
        }        
    }
    
    return ret;
}

static const struct attribute_group fan_group = {
    .attrs = fan_attributes,
};

static struct fan_data *fan_update_device(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct fan_data *data = i2c_get_clientdata(client);

    mutex_lock(&data->update_lock);

    if (time_after(jiffies, data->last_updated + HZ + HZ / 2) || 
        !data->valid) {
        int i;

        dev_dbg(&client->dev, "Starting belgitefan update\n");
        data->valid = 0;
        
        /* Update fan data
         */
        for (i = 0; i < ARRAY_SIZE(data->reg_val); i++) {
            int status = fan_read_value(client, fan_reg[i]);
            
            if (status < 0) {
                data->valid = 0;
                mutex_unlock(&data->update_lock);
                dev_dbg(&client->dev, "reg %d, err %d\n", fan_reg[i], status);
                return data;
            }
            else {
                data->reg_val[i] = status;
            }
        }
        
        data->last_updated = jiffies;
        data->valid = 1;
    }
    
    mutex_unlock(&data->update_lock);

    return data;
}

static int fan_probe(struct i2c_client *client,
            const struct i2c_device_id *dev_id)
{
    struct fan_data *data;
    int status;

    dev_err(&client->dev, "check i2c functionality\n");

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        status = -EIO;
        goto exit;
    }

    dev_err(&client->dev, "kzalloc the memory.\n");

    data = kzalloc(sizeof(struct fan_data), GFP_KERNEL);
    if (!data) {
        status = -ENOMEM;
        goto exit;
    }

    i2c_set_clientdata(client, data);
    data->valid = 0;
    mutex_init(&data->update_lock);

    dev_err(&client->dev, "chip found\n");

    /* Register sysfs hooks */
    status = sysfs_create_group(&client->dev.kobj, &fan_group);
    if (status) {
        goto exit_free;
    }

    data->hwmon_dev = hwmon_device_register(&client->dev);
    if (IS_ERR(data->hwmon_dev)) {
        status = PTR_ERR(data->hwmon_dev);
        goto exit_remove;
    }

    dev_err(&client->dev, "%s: fan '%s'\n",
         dev_name(data->hwmon_dev), client->name);
    
    return 0;

exit_remove:
    sysfs_remove_group(&client->dev.kobj, &fan_group);
exit_free:
    kfree(data);
exit:
    
    return status;
}

static int fan_remove(struct i2c_client *client)
{
    struct fan_data *data = i2c_get_clientdata(client);
    hwmon_device_unregister(data->hwmon_dev);
    sysfs_remove_group(&client->dev.kobj, &fan_group);
    
    return 0;
}

/* Addresses to scan */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

static const struct i2c_device_id fan_id[] = {
    { "fan", 0 },
    {}
};
MODULE_DEVICE_TABLE(i2c, fan_id);

static struct i2c_driver fan_driver = {
    .class        = I2C_CLASS_HWMON,
    .driver = {
        .name     = DRVNAME,
    },
    .probe        = fan_probe,
    .remove       = fan_remove,
    .id_table     = fan_id,
    .address_list = normal_i2c,
};

static int __init fan_init(void)
{
	return i2c_add_driver(&fan_driver);
}

static void __exit fan_exit(void)
{
    i2c_del_driver(&fan_driver);
}

module_init(fan_init);
module_exit(fan_exit);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("fan driver");
MODULE_LICENSE("GPL");

