/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/mutex.h>
#include "inv_pthread.h"

#define USE_SMBUS		(1)

/* definition */
#define CPLD_INFO_OFFSET	(0x00)
#define CPLD_BIOS_OFFSET	(0x04)
#define CPLD_BIOS_SPI0		(0)
#define CPLD_BIOS_SPI1		(1)
#define CPLD_PSU_OFFSET		(0x08)
#define CPLD_LED_OFFSET		(0x14)
#define CPLD_CTL_OFFSET		(0x0C)
#define CPLD_CTL_SWCH_READY	(0x01)
#define CPLD_CTL_BIOS_READY	(0x10)

/* Each client has this additional data */
struct cpld_data {
	struct device		*hwmon_dev;
	struct mutex		update_lock;
};

static struct device *cpld_led_client_dev = NULL;

/*-----------------------------------------------------------------------*/

static ssize_t cpld_i2c_read(struct i2c_client *client, u8 *buf, u8 offset, size_t count)
{
#if USE_SMBUS    
	int i;
	
    for(i=0; i<count; i++) {
        buf[i] = i2c_smbus_read_byte_data(client, offset+i);
    }	
    return count;
#else
	struct i2c_msg msg[2];
	char msgbuf[2];
	int status;

	memset(msg, 0, sizeof(msg));
	
	msgbuf[0] = offset;
	
	msg[0].addr = client->addr;
	msg[0].buf = msgbuf;
	msg[0].len = 1;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = count;
	
	status = i2c_transfer(client->adapter, msg, 2);
	
	if(status == 2)
	    status = count;
	    
	return status;    
#endif	
}

static ssize_t cpld_i2c_write(struct i2c_client *client, char *buf, unsigned offset, size_t count)
{
#if USE_SMBUS    
	int i;
	
    for(i=0; i<count; i++) {
        i2c_smbus_write_byte_data(client, offset+i, buf[i]);
    }	
    return count;
#else
	struct i2c_msg msg;
	int status;
    u8 writebuf[64];
	
	int i = 0;

	msg.addr = client->addr;
	msg.flags = 0;

	/* msg.buf is u8 and casts will mask the values */
	msg.buf = writebuf;
	
	msg.buf[i++] = offset;
	memcpy(&msg.buf[i], buf, count);
	msg.len = i + count;
	
	status = i2c_transfer(client->adapter, &msg, 1);
	if (status == 1)
		status = count;
	
    return status;	
#endif    
}

/*-----------------------------------------------------------------------*/

/* sysfs attributes for hwmon */

static ssize_t show_info(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u32 status;
	//struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 b[4];
    
    memset(b, 0, 4);
    
	mutex_lock(&data->update_lock);
    status = cpld_i2c_read(client, b, CPLD_INFO_OFFSET, 4);
	mutex_unlock(&data->update_lock);
	
	if(status != 4) return sprintf(buf, "read cpld info fail\n");
	
	status = sprintf (buf,   "The CPLD release date is %02d/%02d/%d.\n", b[2] & 0xf, (b[3] & 0x1f), 2014+(b[2] >> 4));	/* mm/dd/yyyy*/
	status = sprintf (buf, "%sThe PCB  version is %X%X\n", buf,  b[0]>>4, b[0]&0xf);
	status = sprintf (buf, "%sThe CPLD version is %d.%d\n", buf, b[1]>>4, b[1]&0xf);
	
	return strlen(buf);
}


static ssize_t show_ctl(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u32 status;
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte;
    
	mutex_lock(&data->update_lock);
	
    status = cpld_i2c_read(client, &byte, CPLD_CTL_OFFSET, 1);
	
	mutex_unlock(&data->update_lock);
	
	if(status != 1) return sprintf(buf, "read cpld ctl fail\n");
	
	status = sprintf (buf, "0x%X\n", byte);
	    
	return strlen(buf);
}

static ssize_t set_ctl(struct device *dev,
			   struct device_attribute *devattr,
			   const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte = simple_strtoul(buf, NULL, 16);

	//printk(KERN_INFO "%s/%d:  %u (%s)\n",__func__,__LINE__,byte, buf);

        mutex_lock(&data->update_lock);
	cpld_i2c_write(client, &byte, CPLD_CTL_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	return count;
}

ssize_t cpld_set_ctl(const char *buf, size_t count)
{
	struct i2c_client *client = NULL;
	struct cpld_data *data = NULL;
	u8 byte = simple_strtol(buf, NULL, 16);

	if (!cpld_led_client_dev) {
		return 0;
	}

	client = to_i2c_client(cpld_led_client_dev);
	data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);
	cpld_i2c_write(client, &byte, CPLD_CTL_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	return count;
}
EXPORT_SYMBOL(cpld_set_ctl);

static ssize_t show_bios_cs(struct device *dev, struct device_attribute *da,
                         char *buf)
{
        u32 status;
        struct i2c_client *client = to_i2c_client(dev);
        struct cpld_data *data = i2c_get_clientdata(client);
        u8 byte;

        mutex_lock(&data->update_lock);
	status = cpld_i2c_read(client, &byte, CPLD_BIOS_OFFSET, 1);
        mutex_unlock(&data->update_lock);

        if(status != 1) {
	    return sprintf(buf, "read cpld BIOS_CS fail\n");
	}

        status = sprintf (buf, "0x%0X\n", byte);
        return strlen(buf);
}

static ssize_t set_bios_cs(struct device *dev,
                           struct device_attribute *devattr,
                           const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cpld_data *data = i2c_get_clientdata(client);
        u8 byte = buf[0] - '0';

	if (byte != CPLD_BIOS_SPI0 && byte != CPLD_BIOS_SPI1) {
	    printk(KERN_INFO "%s/%d: Invalide Value %s. 0 or 1 Only\n",__func__,__LINE__,buf);
	    return 0;
	}

        mutex_lock(&data->update_lock);
        cpld_i2c_write(client, &byte, CPLD_BIOS_OFFSET, 1);
        mutex_unlock(&data->update_lock);

        return count;
}

static cpld_led_map_t cpld_led_map[] = {
	{ NULL,      0, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ "stk_led", 0, 0x03, 0x00, 0x01, 0x02, 0x03 },
	{ "fan_led", 2, 0x0c, 0x00, 0x04, 0x08, 0x0c },
	{ "pwr_led", 4, 0x30, 0x00, 0x10, 0x20, 0x30 },
	{ "sys_led", 6, 0xc0, 0x00, 0x40, 0x80, 0xc0 }
};

/* System Leds: sys_led/pwr_led/fan_led/stk_led */
ssize_t cpld_show_led(char *buf, size_t count)
{
        u32 status;
	struct i2c_client *client = NULL;
	struct cpld_data *data = NULL;
        u8 byte = simple_strtol(buf, NULL, 10);

	if (!cpld_led_client_dev) {
		return 0;
	}

	client = to_i2c_client(cpld_led_client_dev);
	data = i2c_get_clientdata(client);

        mutex_lock(&data->update_lock);
	status = cpld_i2c_read(client, &byte, CPLD_LED_OFFSET, 1);
        mutex_unlock(&data->update_lock);

        if(status != 1) return sprintf(buf, "read cpld show_led fail\n");

        return sprintf (buf, "0x%02X\n", byte);
}
EXPORT_SYMBOL(cpld_show_led);

static ssize_t show_sys_led(struct device *dev, struct device_attribute *da,
                         char *buf)
{
        u32 status;
        struct i2c_client *client = to_i2c_client(dev);
        struct cpld_data *data = i2c_get_clientdata(client);
        u8 byte;

        mutex_lock(&data->update_lock);
	status = cpld_i2c_read(client, &byte, CPLD_LED_OFFSET, 1);
        mutex_unlock(&data->update_lock);

        if(status != 1) return sprintf(buf, "read cpld sys_led fail\n");

        return sprintf (buf, "%s: 0x%02X\n", cpld_led_map[LED_SYS].name, (byte & cpld_led_map[LED_SYS].bit_mask));
}

static ssize_t set_sys_led(struct device *dev,
                           struct device_attribute *devattr,
                           const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cpld_data *data = i2c_get_clientdata(client);
        u8 temp, byte = simple_strtol(buf, NULL, 10);

	if (byte == 0) {
	    byte = cpld_led_map[LED_SYS].led_off;
	}
	else
	if (byte == 1) {
	    byte = cpld_led_map[LED_SYS].led_on;
	}
	else
	if (byte == 2) {
	    byte = cpld_led_map[LED_SYS].led_blink;
	}
	else
	if (byte == 3) {
	    byte = cpld_led_map[LED_SYS].led_blink_slow;
	}
	else {
	    printk(KERN_INFO "%s/%d: Invalide value: %s\n",__func__,__LINE__,buf);
	    printk(KERN_INFO "0 Blue OFF\n1 Blue ON\n2 Blue Blink 1Hz\nBlue Blink 0.5\n");
	    return 0;
	}

        mutex_lock(&data->update_lock);
        cpld_i2c_read(client, &temp, CPLD_LED_OFFSET, 1);
	temp &= ~cpld_led_map[LED_SYS].bit_mask;
	byte |= temp;
        cpld_i2c_write(client, &byte, CPLD_LED_OFFSET, 1);
        mutex_unlock(&data->update_lock);

        return count;
}

ssize_t cpld_set_sys_led(const char *buf, size_t count)
{
	struct i2c_client *client = NULL;
	struct cpld_data *data = NULL;
        u8 temp, byte = simple_strtol(buf, NULL, 10);

	if (!cpld_led_client_dev) {
		return 0;
	}

	client = to_i2c_client(cpld_led_client_dev);
	data = i2c_get_clientdata(client);

	if (byte == 0) {
	    byte = cpld_led_map[LED_SYS].led_off;
	}
	else
	if (byte == 1) {
	    byte = cpld_led_map[LED_SYS].led_on;
	}
	else
	if (byte == 2) {
	    byte = cpld_led_map[LED_SYS].led_blink;
	}
	else
	if (byte == 3) {
	    byte = cpld_led_map[LED_SYS].led_blink_slow;
	}
	else {
	    printk(KERN_INFO "%s/%d: Invalide value: %s\n",__func__,__LINE__,buf);
	    printk(KERN_INFO "0 Blue OFF\n1 Blue ON\n2 Blue Blink 1Hz\nBlue Blink 0.5\n");
	    return 0;
	}

        mutex_lock(&data->update_lock);
        cpld_i2c_read(client, &temp, CPLD_LED_OFFSET, 1);
	temp &= ~cpld_led_map[LED_SYS].bit_mask;
	byte |= temp;
        cpld_i2c_write(client, &byte, CPLD_LED_OFFSET, 1);
        mutex_unlock(&data->update_lock);

	return count;
}
EXPORT_SYMBOL(cpld_set_sys_led);

static ssize_t show_pwr_led(struct device *dev, struct device_attribute *da,
                         char *buf)
{
        u32 status;
        struct i2c_client *client = to_i2c_client(dev);
        struct cpld_data *data = i2c_get_clientdata(client);
        u8 byte;

        mutex_lock(&data->update_lock);
	status = cpld_i2c_read(client, &byte, CPLD_LED_OFFSET, 1);
        mutex_unlock(&data->update_lock);

        if(status != 1) return sprintf(buf, "read cpld pwr_led fail\n");

        return sprintf (buf, "%s: 0x%02X\n", cpld_led_map[LED_PWR].name, (byte & cpld_led_map[LED_PWR].bit_mask));
}

static ssize_t set_pwr_led(struct device *dev,
                           struct device_attribute *devattr,
                           const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cpld_data *data = i2c_get_clientdata(client);
        u8 temp, byte = simple_strtol(buf, NULL, 10);

	if (byte == 0) {
	    byte = cpld_led_map[LED_PWR].led_off;
	}
	else
	if (byte == 1) {
	    byte = cpld_led_map[LED_PWR].led_on;
	}
	else
	if (byte == 2) {
	    byte = cpld_led_map[LED_PWR].led_blink;
	}
	else
	if (byte == 3) {
	    byte = cpld_led_map[LED_PWR].led_blink_slow;
	}
	else {
	    printk(KERN_INFO "%s/%d: Invalide value: %s\n",__func__,__LINE__,buf);
	    printk(KERN_INFO "0 Blue OFF\n1 Blue ON\n2 Blue Blink 1Hz\nBlue Blink 0.5\n");
	    return 0;
	}

        mutex_lock(&data->update_lock);
        cpld_i2c_read(client, &temp, CPLD_LED_OFFSET, 1);
	temp &= ~cpld_led_map[LED_PWR].bit_mask;
	byte |= temp;
        cpld_i2c_write(client, &byte, CPLD_LED_OFFSET, 1);
        mutex_unlock(&data->update_lock);

        return count;
}

ssize_t cpld_set_pwr_led(const char *buf, size_t count)
{
	struct i2c_client *client = NULL;
	struct cpld_data *data = NULL;
        u8 temp, byte = simple_strtol(buf, NULL, 10);

	if (!cpld_led_client_dev) {
		return 0;
	}

	client = to_i2c_client(cpld_led_client_dev);
	data = i2c_get_clientdata(client);

	if (byte == 0) {
	    byte = cpld_led_map[LED_PWR].led_off;
	}
	else
	if (byte == 1) {
	    byte = cpld_led_map[LED_PWR].led_on;
	}
	else
	if (byte == 2) {
	    byte = cpld_led_map[LED_PWR].led_blink;
	}
	else
	if (byte == 3) {
	    byte = cpld_led_map[LED_PWR].led_blink_slow;
	}
	else {
	    printk(KERN_INFO "%s/%d: Invalide value: %s\n",__func__,__LINE__,buf);
	    printk(KERN_INFO "0 Blue OFF\n1 Blue ON\n2 Blue Blink 1Hz\nBlue Blink 0.5\n");
	    return 0;
	}

        mutex_lock(&data->update_lock);
        cpld_i2c_read(client, &temp, CPLD_LED_OFFSET, 1);
	temp &= ~cpld_led_map[LED_PWR].bit_mask;
	byte |= temp;
        cpld_i2c_write(client, &byte, CPLD_LED_OFFSET, 1);
        mutex_unlock(&data->update_lock);

	return count;
}
EXPORT_SYMBOL(cpld_set_pwr_led);

static ssize_t show_fan_led(struct device *dev, struct device_attribute *da,
                         char *buf)
{
        u32 status;
        struct i2c_client *client = to_i2c_client(dev);
        struct cpld_data *data = i2c_get_clientdata(client);
        u8 byte;

        mutex_lock(&data->update_lock);
	status = cpld_i2c_read(client, &byte, CPLD_LED_OFFSET, 1);
        mutex_unlock(&data->update_lock);

        if(status != 1) return sprintf(buf, "read cpld fan_led fail\n");

        return sprintf (buf, "%s: 0x%02X\n", cpld_led_map[LED_FAN].name, (byte & cpld_led_map[LED_FAN].bit_mask));
}

static ssize_t set_fan_led(struct device *dev,
                           struct device_attribute *devattr,
                           const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cpld_data *data = i2c_get_clientdata(client);
        u8 temp, byte = simple_strtol(buf, NULL, 10);

	if (byte == 0) {
	    byte = cpld_led_map[LED_FAN].led_off;
	}
	else
	if (byte == 1) {
	    byte = cpld_led_map[LED_FAN].led_on;
	}
	else
	if (byte == 2) {
	    byte = cpld_led_map[LED_FAN].led_blink;
	}
	else
	if (byte == 3) {
	    byte = cpld_led_map[LED_FAN].led_blink_slow;
	}
	else {
	    printk(KERN_INFO "%s/%d: Invalide value: %s\n",__func__,__LINE__,buf);
	    printk(KERN_INFO "0 Blue OFF\n1 Blue ON\n2 Blue Blink 1Hz\nBlue Blink 0.5\n");
	    return 0;
	}

        mutex_lock(&data->update_lock);
        cpld_i2c_read(client, &temp, CPLD_LED_OFFSET, 1);
	temp &= ~cpld_led_map[LED_FAN].bit_mask;
	byte |= temp;
        cpld_i2c_write(client, &byte, CPLD_LED_OFFSET, 1);
        mutex_unlock(&data->update_lock);

        return count;
}

ssize_t cpld_set_fan_led(const char *buf, size_t count)
{
	struct i2c_client *client = NULL;
	struct cpld_data *data = NULL;
        u8 temp, byte = simple_strtol(buf, NULL, 10);

	if (!cpld_led_client_dev) {
		return 0;
	}

	client = to_i2c_client(cpld_led_client_dev);
	data = i2c_get_clientdata(client);

	if (byte == 0) {
	    byte = cpld_led_map[LED_FAN].led_off;
	}
	else
	if (byte == 1) {
	    byte = cpld_led_map[LED_FAN].led_on;
	}
	else
	if (byte == 2) {
	    byte = cpld_led_map[LED_FAN].led_blink;
	}
	else
	if (byte == 3) {
	    byte = cpld_led_map[LED_FAN].led_blink_slow;
	}
	else {
	    printk(KERN_INFO "%s/%d: Invalide value: %s\n",__func__,__LINE__,buf);
	    printk(KERN_INFO "0 Blue OFF\n1 Blue ON\n2 Blue Blink 1Hz\nBlue Blink 0.5\n");
	    return 0;
	}

        mutex_lock(&data->update_lock);
        cpld_i2c_read(client, &temp, CPLD_LED_OFFSET, 1);
	temp &= ~cpld_led_map[LED_FAN].bit_mask;
	byte |= temp;
        cpld_i2c_write(client, &byte, CPLD_LED_OFFSET, 1);
        mutex_unlock(&data->update_lock);

	return count;
}
EXPORT_SYMBOL(cpld_set_fan_led);

static ssize_t show_stk_led(struct device *dev, struct device_attribute *da,
                         char *buf)
{
        u32 status;
        struct i2c_client *client = to_i2c_client(dev);
        struct cpld_data *data = i2c_get_clientdata(client);
        u8 byte;

        mutex_lock(&data->update_lock);
	status = cpld_i2c_read(client, &byte, CPLD_LED_OFFSET, 1);
        mutex_unlock(&data->update_lock);

        if(status != 1) return sprintf(buf, "read cpld stk_led fail\n");

        return sprintf (buf, "%s: 0x%02X\n", cpld_led_map[LED_STK].name, (byte & cpld_led_map[LED_STK].bit_mask));
}

static ssize_t set_stk_led(struct device *dev,
                           struct device_attribute *devattr,
                           const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cpld_data *data = i2c_get_clientdata(client);
        u8 temp, byte = simple_strtol(buf, NULL, 10);

	if (byte == 0) {
	    byte = cpld_led_map[LED_STK].led_off;
	}
	else
	if (byte == 1) {
	    byte = cpld_led_map[LED_STK].led_on;
	}
	else
	if (byte == 2) {
	    byte = cpld_led_map[LED_STK].led_blink;
	}
	else
	if (byte == 3) {
	    byte = cpld_led_map[LED_STK].led_blink_slow;
	}
	else {
	    printk(KERN_INFO "%s/%d: Invalide value: %s\n",__func__,__LINE__,buf);
	    printk(KERN_INFO "0 Blue OFF\n1 Blue ON\n2 Blue Blink 1Hz\nBlue Blink 0.5\n");
	    return 0;
	}

        mutex_lock(&data->update_lock);
        cpld_i2c_read(client, &temp, CPLD_LED_OFFSET, 1);
	temp &= ~cpld_led_map[LED_STK].bit_mask;
	byte |= temp;
        cpld_i2c_write(client, &byte, CPLD_LED_OFFSET, 1);
        mutex_unlock(&data->update_lock);

        return count;
}

ssize_t cpld_set_stk_led(const char *buf, size_t count)
{
	struct i2c_client *client = NULL;
	struct cpld_data *data = NULL;
        u8 temp, byte = simple_strtol(buf, NULL, 10);

	if (!cpld_led_client_dev) {
		return 0;
	}

	client = to_i2c_client(cpld_led_client_dev);
	data = i2c_get_clientdata(client);

	if (byte == 0) {
	    byte = cpld_led_map[LED_STK].led_off;
	}
	else
	if (byte == 1) {
	    byte = cpld_led_map[LED_STK].led_on;
	}
	else
	if (byte == 2) {
	    byte = cpld_led_map[LED_STK].led_blink;
	}
	else
	if (byte == 3) {
	    byte = cpld_led_map[LED_STK].led_blink_slow;
	}
	else {
	    printk(KERN_INFO "%s/%d: Invalide value: %s\n",__func__,__LINE__,buf);
	    printk(KERN_INFO "0 Blue OFF\n1 Blue ON\n2 Blue Blink 1Hz\nBlue Blink 0.5\n");
	    return 0;
	}

        mutex_lock(&data->update_lock);
        cpld_i2c_read(client, &temp, CPLD_LED_OFFSET, 1);
	temp &= ~cpld_led_map[LED_STK].bit_mask;
	byte |= temp;
        cpld_i2c_write(client, &byte, CPLD_LED_OFFSET, 1);
        mutex_unlock(&data->update_lock);

	return count;
}
EXPORT_SYMBOL(cpld_set_stk_led);

#if 0
static char* led_str[] = {
    "OFF",     //000
    "0.5 Hz",  //001
    "1 Hz",    //010
    "2 Hz",    //011
    "4 Hz",    //100
    "NA",      //101
    "NA",      //110
    "ON",      //111
};
#endif

/*
CPLD report the PSU0 status
000 = PSU normal operation
100 = PSU fault
010 = PSU unpowered
111 = PSU not installed

7 6 | 5 4 3 |  2 1 0
----------------------
    | psu0  |  psu1
*/
static char* psu_str[] = {
    "normal",           //000
    "NA",               //001
    "unpowered",        //010
    "NA",               //011
    "fault",            //100
    "NA",               //101
    "NA",               //110
    "not installed",    //111
};

static ssize_t show_psu(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u32 status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte;
	int shift = (attr->index == 1)?0:3;
    
	mutex_lock(&data->update_lock);
    status = cpld_i2c_read(client, &byte, CPLD_PSU_OFFSET, 1);
	mutex_unlock(&data->update_lock);
	
    byte = (byte >> shift) & 0x7;
	
	status = sprintf (buf, "%d : %s\n", byte, psu_str[byte]);
	    
	return strlen(buf);
}

ssize_t cpld_show_psu(char *buf)
{ 
	u32 status;
	struct i2c_client *client = NULL;
	struct cpld_data *data = NULL;
	u8 byte;

	if (!cpld_led_client_dev) {
		return 0;
	}

	client = to_i2c_client(cpld_led_client_dev);
	data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);
    status = cpld_i2c_read(client, &byte, CPLD_PSU_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	status = sprintf (buf, "0x%04x\n", byte & 0x3f);

	return strlen(buf);
}
EXPORT_SYMBOL(cpld_show_psu);

static SENSOR_DEVICE_ATTR(info,	S_IRUGO,	        show_info, 0, 0);
static SENSOR_DEVICE_ATTR(ctl,	S_IWUSR|S_IRUGO,	show_ctl, set_ctl, 0);
static SENSOR_DEVICE_ATTR(psu0,  S_IRUGO,	        show_psu, 0, 0);
static SENSOR_DEVICE_ATTR(psu1,  S_IRUGO,	        show_psu, 0, 1);

static SENSOR_DEVICE_ATTR(bios_cs, S_IWUSR|S_IRUGO,	show_bios_cs, set_bios_cs, 0);
static SENSOR_DEVICE_ATTR(sys_led, S_IWUSR|S_IRUGO,	show_sys_led, set_sys_led, 0);
static SENSOR_DEVICE_ATTR(pwr_led, S_IWUSR|S_IRUGO,	show_pwr_led, set_pwr_led, 0);
static SENSOR_DEVICE_ATTR(fan_led, S_IWUSR|S_IRUGO,	show_fan_led, set_fan_led, 0);
static SENSOR_DEVICE_ATTR(stk_led, S_IWUSR|S_IRUGO,	show_stk_led, set_stk_led, 0);
			
static struct attribute *cpld_attributes[] = {
    //info
	&sensor_dev_attr_info.dev_attr.attr,
	&sensor_dev_attr_ctl.dev_attr.attr,
	&sensor_dev_attr_psu0.dev_attr.attr,
	&sensor_dev_attr_psu1.dev_attr.attr,
	
	&sensor_dev_attr_bios_cs.dev_attr.attr,
	&sensor_dev_attr_sys_led.dev_attr.attr,
	&sensor_dev_attr_pwr_led.dev_attr.attr,
	&sensor_dev_attr_fan_led.dev_attr.attr,
	&sensor_dev_attr_stk_led.dev_attr.attr,

	NULL
};

static const struct attribute_group cpld_group = {
	.attrs = cpld_attributes,
};

static struct attribute *cpld2_attributes[] = {
    //info
        &sensor_dev_attr_info.dev_attr.attr,

        NULL
};

static const struct attribute_group cpld2_group = {
        .attrs = cpld2_attributes,
};


/*-----------------------------------------------------------------------*/

/* device probe and removal */

static int
cpld_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct cpld_data *data;
	int status;

	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA))
		return -EIO;

	data = kzalloc(sizeof(struct cpld_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);
	
	/* Register sysfs hooks */
	if(id->driver_data==1)  // CPLD2
		status = sysfs_create_group(&client->dev.kobj, &cpld2_group);
	else                    // default CPLD1
		status = sysfs_create_group(&client->dev.kobj, &cpld_group);

	if (status)
		goto exit_free;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		status = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	dev_info(&client->dev, "%s: sensor '%s'\n",
		 dev_name(data->hwmon_dev), client->name);

	cpld_led_client_dev = &client->dev;
	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &cpld_group);
exit_free:
	i2c_set_clientdata(client, NULL);
	kfree(data);
	cpld_led_client_dev = NULL;
	return status;
}

static int cpld_remove(struct i2c_client *client)
{
	struct cpld_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &cpld_group);
	i2c_set_clientdata(client, NULL);
	kfree(data);
	cpld_led_client_dev = NULL;
	return 0;
}

static const struct i2c_device_id cpld_ids[] = {
	{ "inv_cpld" , 0, },
	{ "inv_cpld2", 1, },
	{ /* LIST END */ }
};
MODULE_DEVICE_TABLE(i2c, cpld_ids);

static struct i2c_driver cpld_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "inv_cpld",
	},
	.probe		= cpld_probe,
	.remove		= cpld_remove,
	.id_table	= cpld_ids,
};

/*-----------------------------------------------------------------------*/

/* module glue */

static int __init inv_cpld_init(void)
{
	return i2c_add_driver(&cpld_driver);
}

static void __exit inv_cpld_exit(void)
{
	i2c_del_driver(&cpld_driver);
}

MODULE_AUTHOR("eddie.lan <eddie.lan@inventec>");
MODULE_DESCRIPTION("inv cpld driver");
MODULE_LICENSE("GPL");

module_init(inv_cpld_init);
module_exit(inv_cpld_exit);
