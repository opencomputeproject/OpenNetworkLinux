 /*
  * A hwmon driver for the SWITCH SYSTEM CPLD
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
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/i2c.h>

#include "switch_cpld_driver.h"
//simon: try to support 4.x kernel
#include <linux/version.h>

#if 1
#define SYS_CPLD_DEBUG(...)
#else
#define SYS_CPLD_DEBUG(...) printk(KERN_DEBUG __VA_ARGS__)
#endif

#define LPC_DEVICE_ADDRESS1          0xFC800000
#define LPC_IO_MEMORY_SIZE           1 << 12

#define BOARD_VERSION_REG            0x02
#define BOARD_VERSION_MASK           0x10

#define MAJOR_VERSION_REG            0x00
#define MAJOR_VERSION_MASK           0xFF

#define MINOR_VERSION_REG            0x01
#define MINOR_VERSION_MASK           0xFF

#define I2C_MAX_CHANNEL              18
#define SYSTEM_CPLD_SLEEP(X)         usleep_range((X*8/10), X)
#define I2C_PRESCALE_HIGH_REG        0x80  // PRESCALE registers, PRERhi
#define I2C_PRESCALE_LOW_REG         0x81  // PRESCALE registers, PRERlo
#define I2C_CONTROLLER_REG           0x82  // Control Register, CTR
#define I2C_TRANSMIT_REG             0x83  // Transmit Register, TXR
#define I2C_COMMAND_REG              0x84  // Command Register, CR
#define I2C_RECEIVE_REG              0x86  // Receive Register, RXR
#define I2C_STATUS_REG               0x87  // Status Registesr, SR


#define PRER_HI_DEFAULT              0x00
#define PRER_LO_DEFAULT              0x63
#define CONTROL_DISABLE              0x00
#define CONTROL_ENABLE               0x80
#define COMMAND_START_WRITE          0x90
#define COMMAND_ENABLE_WRITE         0x10
#define COMMAND_READ                 0x20
#define COMMAND_ACK_STOP_READ        0x68
#define COMMAND_STOP                 0x40
#define COMMAND_STOP_WRITE           0x50
#define BUSY_BIT                     0x40
#define TIP_BIT                      0x02
#define ACK_BIT                      0x80
#define SET_TXR_WRITE(x)             (x &= ~(1UL))
#define SET_TXR_READ(x)              (x |= (1UL))

#define MAX_BUFF_SIZE                256

#define DRV_NAME                   "switch_system_cpld"
#define SYSTEM_CPLD_SMBUS_NAME     "Accton_System_CPLD_SMBus_"

#define SYSTEM_CPLD_DRIVER_VERSION "0.0.0.1"


/* System CPLD SMBus control parameters */
static unsigned int max_retry_time = 20;
static unsigned int delay_ack = 50;
static unsigned int delay_tip = 26;
static unsigned int delay_busy = 10;

static struct platform_device *switch_system_cpld_device;

struct switch_system_cpld_data {
    struct device *dev;
    void __iomem  *base_addr;

    struct mutex  update_lock;
    struct semaphore    sem[I2C_MAX_CHANNEL];
    struct i2c_adapter  adapter[I2C_MAX_CHANNEL];
};

#define SYSTEM_CPLD_ACCESS(cmd) \
    do { \
        mutex_lock(&cpld_data->update_lock); \
        cmd \
        mutex_unlock(&cpld_data->update_lock); \
    }while(0);


static const unsigned char crc8_table[256] = {
    0x00,0x07,0x0E,0x09,0x1C,0x1B,0x12,0x15,0x38,0x3F,0x36,0x31,0x24,0x23,0x2A,0x2D,
    0x70,0x77,0x7E,0x79,0x6C,0x6B,0x62,0x65,0x48,0x4F,0x46,0x41,0x54,0x53,0x5A,0x5D,
    0xE0,0xE7,0xEE,0xE9,0xFC,0xFB,0xF2,0xF5,0xD8,0xDF,0xD6,0xD1,0xC4,0xC3,0xCA,0xCD,
    0x90,0x97,0x9E,0x99,0x8C,0x8B,0x82,0x85,0xA8,0xAF,0xA6,0xA1,0xB4,0xB3,0xBA,0xBD,
    0xC7,0xC0,0xC9,0xCE,0xDB,0xDC,0xD5,0xD2,0xFF,0xF8,0xF1,0xF6,0xE3,0xE4,0xED,0xEA,
    0xB7,0xB0,0xB9,0xBE,0xAB,0xAC,0xA5,0xA2,0x8F,0x88,0x81,0x86,0x93,0x94,0x9D,0x9A,
    0x27,0x20,0x29,0x2E,0x3B,0x3C,0x35,0x32,0x1F,0x18,0x11,0x16,0x03,0x04,0x0D,0x0A,
    0x57,0x50,0x59,0x5E,0x4B,0x4C,0x45,0x42,0x6F,0x68,0x61,0x66,0x73,0x74,0x7D,0x7A,
    0x89,0x8E,0x87,0x80,0x95,0x92,0x9B,0x9C,0xB1,0xB6,0xBF,0xB8,0xAD,0xAA,0xA3,0xA4,
    0xF9,0xFE,0xF7,0xF0,0xE5,0xE2,0xEB,0xEC,0xC1,0xC6,0xCF,0xC8,0xDD,0xDA,0xD3,0xD4,
    0x69,0x6E,0x67,0x60,0x75,0x72,0x7B,0x7C,0x51,0x56,0x5F,0x58,0x4D,0x4A,0x43,0x44,
    0x19,0x1E,0x17,0x10,0x05,0x02,0x0B,0x0C,0x21,0x26,0x2F,0x28,0x3D,0x3A,0x33,0x34,
    0x4E,0x49,0x40,0x47,0x52,0x55,0x5C,0x5B,0x76,0x71,0x78,0x7F,0x6A,0x6D,0x64,0x63,
    0x3E,0x39,0x30,0x37,0x22,0x25,0x2C,0x2B,0x06,0x01,0x08,0x0F,0x1A,0x1D,0x14,0x13,
    0xAE,0xA9,0xA0,0xA7,0xB2,0xB5,0xBC,0xBB,0x96,0x91,0x98,0x9F,0x8A,0x8D,0x84,0x83,
    0xDE,0xD9,0xD0,0xD7,0xC2,0xC5,0xCC,0xCB,0xE6,0xE1,0xE8,0xEF,0xFA,0xFD,0xF4,0xF3,
};

static unsigned char pmbus_pec_calc(unsigned char crc,  unsigned char *buffer, size_t count)
{
    unsigned char r = crc;
    const unsigned char *p = (const unsigned char*)buffer;

    while (count--)
        r = crc8_table[r ^ *p++];
    return r;
}
#if 0
static ssize_t system_cpld_show_byte(struct device *dev,
                         struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute_2 *attr2 = to_sensor_dev_attr_2(devattr);
    struct switch_system_cpld_data *data = dev_get_drvdata(dev);
    void __iomem  *base_addr = data->base_addr;
    unsigned char reg = attr2->index;
    unsigned char mask = attr2->nr;
    int val = 0;

    mutex_lock(&data->update_lock);
    val = (int)readb(base_addr + reg);
    mutex_unlock(&data->update_lock);

    if (unlikely(val < 0)) {
        return val;
    }

    val = val & mask;
    return snprintf(buf, PAGE_SIZE, "0x%x\n", val);
}

static ssize_t system_cpld_set_byte(struct device *dev, struct device_attribute *devattr,
                         const char *buf, size_t count)
{
    struct sensor_device_attribute_2 *attr2 = to_sensor_dev_attr_2(devattr);
    struct switch_system_cpld_data *data = dev_get_drvdata(dev);
    void __iomem  *base_addr = data->base_addr;
    unsigned char reg = attr2->index;
    unsigned char mask = attr2->nr;
    long value;
    int status;

    status = kstrtol(buf, 10, &value);
    if (status) {
        return status;
    }

    value = value & mask;
    mutex_lock(&data->update_lock);
    writeb(value, base_addr + reg);
    mutex_unlock(&data->update_lock);

    return count;
}
#endif
static int system_cpld_smbus_check_busy(struct switch_system_cpld_data *cpld_data, unsigned int i2c_stat_addr, unsigned int *i2c_stat)
{
    unsigned int retry_times = 0;
    unsigned int transaction_delay = delay_busy;

    for(retry_times = 0; retry_times < max_retry_time; retry_times++)
    {
        SYSTEM_CPLD_SLEEP(transaction_delay);
        SYSTEM_CPLD_ACCESS(*i2c_stat = readb(cpld_data->base_addr + i2c_stat_addr);)
        SYS_CPLD_DEBUG("[check_busy] i2c_stat:%x\n", *i2c_stat);
        if((*i2c_stat & BUSY_BIT) == 0)
        {
            return 0;
        }
    }

    return -1;
}

static int system_cpld_smbus_check_tip(struct switch_system_cpld_data *cpld_data, unsigned int i2c_stat_addr, unsigned int *i2c_stat)
{
    unsigned int retry_times = 0;
    unsigned int transaction_delay = delay_tip;

    for(retry_times = 0; retry_times < max_retry_time; retry_times++)
    {
        SYSTEM_CPLD_SLEEP(transaction_delay);
        SYSTEM_CPLD_ACCESS(*i2c_stat = readb(cpld_data->base_addr + i2c_stat_addr);)
        SYS_CPLD_DEBUG("[check_tip] i2c_stat:%x\n", *i2c_stat);
        if((*i2c_stat & TIP_BIT) == 0)
        {
            return 0;
        }
    }

    return -1;
}

static int system_cpld_smbus_check_ack(struct switch_system_cpld_data *cpld_data, unsigned int i2c_stat_addr, unsigned int *i2c_stat)
{
    unsigned int retry_times = 0;
    unsigned int ack_transaction_delay = delay_ack;

    for(retry_times = 0; retry_times < max_retry_time; retry_times++)
    {
        SYS_CPLD_DEBUG("[check_ack] i2c_stat:%x\n", *i2c_stat);
        if((*i2c_stat & ACK_BIT) == 0)
        {
            return 0;
        }
        SYSTEM_CPLD_SLEEP(ack_transaction_delay);
        SYSTEM_CPLD_ACCESS(*i2c_stat = readb(cpld_data->base_addr + i2c_stat_addr);)
    }

    if(*i2c_stat & ACK_BIT)
    {
        return -1;
    }

    return 0;
}

int system_cpld_smbus_read(struct switch_system_cpld_data *cpld_data,
    unsigned char channel, unsigned char addr, unsigned char offset, unsigned char length,
    unsigned char *data, int size)
{
    int retval = 0;
    int stop_step = 0;
    unsigned int i2c_txr_addr = 0, i2c_txr_data = 0;
    unsigned int i2c_rxr_addr = 0, i2c_rxr_data = 0;
    unsigned int i2c_cmd_addr = 0, i2c_cmd_data = 0;
    unsigned int i2c_stat_addr = 0, i2c_stat_data = 0;
    unsigned char byte_to_rw = 0, temp_offset = 0;
    bool is_current_read = false;
    bool len_change_flag = false;

    SYS_CPLD_DEBUG("[%s] READ\n", __func__);

    if(addr > 0x7F || channel >= I2C_MAX_CHANNEL)
    {
        return(-EINVAL);
    }

    retval = down_interruptible(&(cpld_data->sem[channel]));
    if(retval)
    {
        return(-ERESTARTSYS);
    }

    i2c_txr_addr = I2C_TRANSMIT_REG + channel*0x08;
    i2c_rxr_addr = I2C_RECEIVE_REG + channel*0x08;
    i2c_cmd_addr = I2C_COMMAND_REG + channel*0x08;
    i2c_stat_addr = I2C_STATUS_REG + channel*0x08;

    /*
    1. Set the Transmit Register TXR with a value of Slave address + Write bit.
    2. Set the Command Register CR to 8'h90 to enable the START and WRITE. This starts the transmission on the I2C bus.
    3. Check the Transfer In Progress (TIP) bit of the Status Register, SR, to make sure the command is done.
       Check the Received Acknowledge (RxACK) bit of the Status Register, SR, to make sure I2C slave has ACK.
    4. Set TRX with the slave memory address, where the data is to be read from.
    5. Set CR with 8'h10 to enable a WRITE to send to the slave memory address.
    6. Check the TIP bit of SR, to make sure the command is done.
       Check the Received Acknowledge (RxACK) bit of the Status Register, SR, to make sure I2C slave has ACK.
    7. Set TRX with a value of Slave address + READ bit.
    8. Set CR with the 8'h90 to enable the START (repeated START in this case) and WRITE the value in TXR to the slave device.
    9. Check the TIP bit of SR, to make sure the command is done.
       Check the Received Acknowledge (RxACK) bit of the Status Register, SR, to make sure I2C slave has ACK.
    10. Set CR with 8'h20 to issue a READ command and then an ACK command. This enables the reading of data from the slave device.
    11. Check the TIP bit of SR, to make sure the command is done.
    12. Repeat steps 10 and 11 to continue to read data from the slave device.
    13. When the Master is ready to stop reading from the Slave, set CR to 8'h28. This will read the last byte of data and then issue a NACK.
        Check the TIP bit of SR, to make sure the command is done.
    */

    if(size == I2C_SMBUS_BYTE)
        is_current_read = true;

    if((size == I2C_SMBUS_BLOCK_DATA) ||(size == I2C_SMBUS_I2C_BLOCK_DATA))
    {
        len_change_flag = true;
    }

    if(is_current_read == false)
    {
        /* 1. Set the Transmit Register TXR with a value of Slave address + Write bit.*/
        i2c_txr_data = (addr) << 1;
        SET_TXR_WRITE(i2c_txr_data);
        SYSTEM_CPLD_ACCESS(writeb(i2c_txr_data, cpld_data->base_addr + i2c_txr_addr);)
        SYS_CPLD_DEBUG("[1] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);

        /*2. Set the Command Register CR to 8'h90 to enable the START and WRITE. This starts the transmission on the I2C bus.*/
        i2c_cmd_data = COMMAND_START_WRITE;
        SYSTEM_CPLD_ACCESS(writeb(i2c_cmd_data, cpld_data->base_addr + i2c_cmd_addr);)
        SYS_CPLD_DEBUG("[2] i2c_cmd_addr:%x i2c_cmd_data:%x\n", i2c_cmd_addr, i2c_cmd_data);

        /*3. Check the Transfer In Progress (TIP) bit of the Status Register, SR, to make sure the command is done.*/
        if(system_cpld_smbus_check_tip(cpld_data, i2c_stat_addr, &i2c_stat_data))
        {
            stop_step = 3;
            goto device_busy;
        }

        /*Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
        if(system_cpld_smbus_check_ack(cpld_data, i2c_stat_addr, &i2c_stat_data))
        {
            stop_step = 3;
            goto no_ack_response;
        }

        /*4. Set TRX with the slave memory address, where the data is to be read from.*/
        i2c_txr_data = offset;
        SYSTEM_CPLD_ACCESS(writeb(i2c_txr_data, cpld_data->base_addr + i2c_txr_addr);)
        SYS_CPLD_DEBUG("[4] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);

        /*5. Set CR with 8'h10 to enable a WRITE to send to the slave memory address.*/
        i2c_cmd_data = COMMAND_ENABLE_WRITE;
        SYSTEM_CPLD_ACCESS(writeb(i2c_cmd_data, cpld_data->base_addr + i2c_cmd_addr);)
        SYS_CPLD_DEBUG("[5] i2c_cmd_ddr:%x i2c_cmd_data:%x\n", i2c_cmd_addr, i2c_cmd_data);

        /*6. Check the TIP bit of SR, to make sure the command is done.*/
        if(system_cpld_smbus_check_tip(cpld_data, i2c_stat_addr, &i2c_stat_data))
        {
            stop_step = 6;
            goto device_busy;
        }

        /*Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
        if(system_cpld_smbus_check_ack(cpld_data, i2c_stat_addr, &i2c_stat_data))
        {
            stop_step = 6;
            goto no_ack_response;
        }
    }

    /*7. Set TRX with a value of Slave address + READ bit.*/
    i2c_txr_data = (addr) << 1;
    SET_TXR_READ(i2c_txr_data);
    SYSTEM_CPLD_ACCESS(writeb(i2c_txr_data, cpld_data->base_addr + i2c_txr_addr);)
    SYS_CPLD_DEBUG("[7] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);

    /*8. Set CR with the 8'h90 to enable the START (repeated START in this case) and WRITE the value in TXR to the slave device.*/
    i2c_cmd_data = COMMAND_START_WRITE;
    SYSTEM_CPLD_ACCESS(writeb(i2c_cmd_data, cpld_data->base_addr + i2c_cmd_addr);)
    SYS_CPLD_DEBUG("[8] i2c_cmd_addr:%x i2c_cmd_data:%x\n", i2c_cmd_addr, i2c_cmd_data);

    /*9. Check the TIP bit of SR, to make sure the command is done.*/
    if(system_cpld_smbus_check_tip(cpld_data, i2c_stat_addr, &i2c_stat_data))
    {
        stop_step = 9;
        goto device_busy;
    }

    /*Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
    if(system_cpld_smbus_check_ack(cpld_data, i2c_stat_addr, &i2c_stat_data))
    {
        stop_step = 9;
        goto no_ack_response;
    }

    /*12. Repeat steps 10 and 11 to continue to read data from the slave device.*/
    byte_to_rw = length;
    temp_offset = 0;
    while(byte_to_rw > 1)
    {
        /*10. Set CR with 8'h20 to issue a READ command and then an ACK command. This enables the reading of data from the slave device.*/
        i2c_cmd_data = COMMAND_READ;
        SYSTEM_CPLD_ACCESS(writeb(i2c_cmd_data, cpld_data->base_addr + i2c_cmd_addr);)
        SYS_CPLD_DEBUG("[10] i2c_cmd_addr:%x i2c_cmd_data:%x\n", i2c_cmd_addr, i2c_cmd_data);

        /*11. Check the TIP bit of SR, to make sure the command is done.*/
        if(system_cpld_smbus_check_tip(cpld_data, i2c_stat_addr, &i2c_stat_data))
        {
            stop_step = 11;
            goto device_busy;
        }

        /*Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
        if(system_cpld_smbus_check_ack(cpld_data, i2c_stat_addr, &i2c_stat_data))
        {
            stop_step = 11;
            goto no_ack_response;
        }

        /*Get RXR with 8-bit data from the slave device.*/
        SYSTEM_CPLD_ACCESS(i2c_rxr_data = readb(cpld_data->base_addr + i2c_rxr_addr);)
        data[temp_offset] = i2c_rxr_data;
        SYS_CPLD_DEBUG("[11-2] i2c_rxr_addr:%x i2c_rxr_data:%x\n", i2c_rxr_addr, i2c_rxr_data);

        if(len_change_flag == true)
        {
            byte_to_rw = i2c_rxr_data + 1;
            len_change_flag = false;
        }

        byte_to_rw--;
        temp_offset++;
    }

    /*13.When the Master is ready to stop reading from the Slave, set CR to 8'h28. This will read the last byte of data and then issue a NACK.*/
    i2c_cmd_data = COMMAND_ACK_STOP_READ;
    SYSTEM_CPLD_ACCESS(writeb(i2c_cmd_data, cpld_data->base_addr + i2c_cmd_addr);)
    SYS_CPLD_DEBUG("[13-1] i2c_cmd_addr:%x i2c_cmd_data:%x\n", i2c_cmd_addr, i2c_cmd_data);

    /*Check the BUSY bit of SR, to make sure the command is done.*/
    if(system_cpld_smbus_check_busy(cpld_data, i2c_stat_addr, &i2c_stat_data))
    {
        stop_step = 13;
        goto device_busy;
    }

    if(byte_to_rw)
    {
        /*Get the last byte from RXR with 8-bit data from the slave device.*/
        SYSTEM_CPLD_ACCESS(i2c_rxr_data = readb(cpld_data->base_addr + i2c_rxr_addr);)
        data[temp_offset] = i2c_rxr_data;
        SYS_CPLD_DEBUG("[13-3] i2c_rxr_addr:%x i2c_rxr_data:%x\n", i2c_rxr_addr, i2c_rxr_data);
    }

    up(&(cpld_data->sem[channel]));
    return(0);

device_busy:
    SYS_CPLD_DEBUG("device_busy, read failed, channel:%d addr:0x%x, length:0x%x, offset:0x%x, stop_step:%d, i2c_stat:0x%x.\n", channel, addr, length, offset, stop_step, i2c_stat_data);

    /*Re-initailize BUS.*/
    /*0. Disable the core by writing 8'h00 to the Control Register, CTR.*/
    SYSTEM_CPLD_ACCESS(writeb(CONTROL_DISABLE, cpld_data->base_addr + I2C_CONTROLLER_REG + channel*0x08);)
    /*1. Program the clock PRESCALE registers, PRERlo and PRERhi, with the desired value.
    This value is determined by the clock frequency and the speed of the I2C bus.*/
    SYSTEM_CPLD_ACCESS(writeb(PRER_HI_DEFAULT, cpld_data->base_addr + I2C_PRESCALE_HIGH_REG + channel*0x08);)
    SYSTEM_CPLD_ACCESS(writeb(PRER_LO_DEFAULT, cpld_data->base_addr + I2C_PRESCALE_LOW_REG + channel*0x08);)

    /*2. Enable the core by writing 8'h80 to the Control Register, CTR.*/
    SYSTEM_CPLD_ACCESS(writeb(CONTROL_ENABLE, cpld_data->base_addr + I2C_CONTROLLER_REG + channel*0x08);)
    SYSTEM_CPLD_SLEEP(500);

    /*Set CR to 8'h40 to issue a STOP command to avoid error.*/
    i2c_cmd_data = COMMAND_STOP;
    SYSTEM_CPLD_ACCESS(writeb(i2c_cmd_data, cpld_data->base_addr + i2c_cmd_addr);)
    /*Check the BUSY bit of SR, to make sure STOP is sent.*/
    if(system_cpld_smbus_check_busy(cpld_data, i2c_stat_addr, &i2c_stat_data))
    {
        SYS_CPLD_DEBUG("device_busy, fail to issue STOP, i2c_stat:0x%x.\n", i2c_stat_data);
    }

    SYS_CPLD_DEBUG("device_busy, read failed, channel:%d addr:0x%x, length:0x%x, offset:0x%x, stop_step:%d, i2c_stat:0x%x.\n", channel, addr, length, offset, stop_step, i2c_stat_data);
    up(&(cpld_data->sem[channel]));
    return (-EBUSY);

no_ack_response:
    /*Set CR to 8'h40 to issue a STOP command to avoid error.*/
    i2c_cmd_data = COMMAND_STOP;
    SYSTEM_CPLD_ACCESS(writeb(i2c_cmd_data, cpld_data->base_addr + i2c_cmd_addr);)
    SYS_CPLD_DEBUG("no_ack_response, read failed, channel:%d addr:0x%x, length:0x%x, offset:0x%x, stop_step:%d, i2c_stat:0x%x.\n", channel, addr, length, offset, stop_step, i2c_stat_data);

    /*Check the BUSY bit of SR, to make sure STOP is sent.*/
    if(system_cpld_smbus_check_busy(cpld_data, i2c_stat_addr, &i2c_stat_data))
    {
        stop_step = 14;
        goto device_busy;
    }

    up(&(cpld_data->sem[channel]));
    return (-ETIMEDOUT);
}

int system_cpld_smbus_write( struct switch_system_cpld_data *cpld_data,
    unsigned char channel, unsigned char addr, unsigned char offset, unsigned char length,
    unsigned char *data)
{
    int retval = 0;
    int stop_step = 0;
    unsigned int i2c_txr_addr = 0, i2c_txr_data = 0;
    unsigned int i2c_cmd_addr = 0, i2c_cmd_data = 0;
    unsigned int i2c_stat_addr = 0, i2c_stat_data = 0;
    unsigned char byte_to_rw = 0, temp_offset = 0;

    SYS_CPLD_DEBUG("[%s] WRITE\n", __func__);

    if(addr > 0x7F || channel >= I2C_MAX_CHANNEL)
    {
        return(-EINVAL);
    }

    retval = down_interruptible(&(cpld_data->sem[channel]));
    if(retval)
    {
        return(-ERESTARTSYS);
    }

    i2c_txr_addr = I2C_TRANSMIT_REG + channel*0x08;
    i2c_cmd_addr = I2C_COMMAND_REG + channel*0x08;
    i2c_stat_addr = I2C_STATUS_REG + channel*0x08;

    /*
    1. Set the Transmit Register TXR with a value of Slave address + Write bit.
    2. Set the Command Register CR to 8'h90 to enable the START and WRITE. This starts the transmission on the I2C bus.
    3. Check the Transfer In Progress (TIP) bit of the Status Register, SR, to make sure the command is done.
       Check the Received Acknowledge (RxACK) bit of the Status Register, SR, to make sure I2C slave has ACK.
    4. Set TXR with a slave memory address for the data to be written to.
    5. Set CR with 8'h10 to enable a WRITE to send to the slave memory address.
    6. Check the TIP bit of SR, to make sure the command is done.
       Check the Received Acknowledge (RxACK) bit of the Status Register, SR, to make sure I2C slave has ACK.
    7. Set TXR with 8-bit data for the slave device.
    8. Set CR to 8'h10 to enable a WRITE to send data.
    9. Check the TIP bit of SR, to make sure the command is done.
       Check the Received Acknowledge (RxACK) bit of the Status Register, SR, to make sure I2C slave has ACK.
    10. Repeat steps 7 to 9 to continue to send data to the slave device.
    11. Set CR to 8'h40 to then issue a STOP command.
        Check the TIP bit of SR, to make sure the command is done.
    */

    /* 1. Set the Transmit Register TXR with a value of Slave address + Write bit.*/
    i2c_txr_data = (addr) << 1;
    SET_TXR_WRITE(i2c_txr_data);
    SYSTEM_CPLD_ACCESS(writeb(i2c_txr_data, cpld_data->base_addr + i2c_txr_addr);)
    SYS_CPLD_DEBUG("[1] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);

    /* 2. Set the Command Register CR to 8'h90 to enable the START and WRITE. This starts the transmission on the I2C bus.*/
    i2c_cmd_data = COMMAND_START_WRITE;
    SYSTEM_CPLD_ACCESS(writeb(i2c_cmd_data, cpld_data->base_addr + i2c_cmd_addr);)
    SYS_CPLD_DEBUG("[2] i2c_cmd_addr:%x i2c_cmdt_data:%x\n", i2c_cmd_addr, i2c_cmd_data);

    /* 3. Check the Transfer In Progress (TIP) bit of the Status Registesr, SR, to make sure the command is done.*/
    if(system_cpld_smbus_check_tip(cpld_data, i2c_stat_addr, &i2c_stat_data))
    {
        stop_step = 3;
        goto device_busy;
    }

    /* Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
    if(system_cpld_smbus_check_ack(cpld_data, i2c_stat_addr, &i2c_stat_data))
    {
        stop_step = 3;
        goto no_ack_response;
    }

    /* 4. Set TXR with a slave memory address for the data to be written to.*/
    i2c_txr_data = offset;
    SYSTEM_CPLD_ACCESS(writeb(i2c_txr_data, cpld_data->base_addr + i2c_txr_addr);)
    SYS_CPLD_DEBUG("[4] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);

    /* 5. Set CR with 8'h10 to enable a WRITE to send to the slave memory address.*/
    i2c_cmd_data = COMMAND_ENABLE_WRITE;
    SYSTEM_CPLD_ACCESS(writeb(i2c_cmd_data, cpld_data->base_addr + i2c_cmd_addr);)
    SYS_CPLD_DEBUG("[5] i2c_cmd_addr:%x i2c_cmd_data:%x\n", i2c_cmd_addr, i2c_cmd_data);

    /* 6. Check the TIP bit of SR, to make sure the command is done.*/
    if(system_cpld_smbus_check_tip(cpld_data, i2c_stat_addr, &i2c_stat_data))
    {
        stop_step = 6;
        goto device_busy;
    }

    /* Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
    if(system_cpld_smbus_check_ack(cpld_data, i2c_stat_addr, &i2c_stat_data))
    {
        stop_step = 6;
        goto no_ack_response;
    }

    /* 10. Repeat steps 7 to 9 to continue to send data to the slave device.*/
    byte_to_rw = length;
    temp_offset = 0;
    while(byte_to_rw > 0)
    {
        /* 7. Set TXR with 8-bit data for the slave device.*/
        i2c_txr_data = data[temp_offset];
        SYSTEM_CPLD_ACCESS(writeb(i2c_txr_data, cpld_data->base_addr + i2c_txr_addr);)
        SYS_CPLD_DEBUG("[7] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);

        /* 8. Set CR to 8'h10 to enable a WRITE to send data.*/
        i2c_cmd_data = COMMAND_ENABLE_WRITE;
        SYSTEM_CPLD_ACCESS(writeb(i2c_cmd_data, cpld_data->base_addr + i2c_cmd_addr);)
        SYS_CPLD_DEBUG("[8] i2c_cmd_addr:%x i2c_cmd_data:%x\n", i2c_cmd_addr, i2c_cmd_data);

        /* 9. Check the TIP bit of SR, to make sure the command is done.*/
        if(system_cpld_smbus_check_tip(cpld_data, i2c_stat_addr, &i2c_stat_data))
        {
            stop_step = 9;
            goto device_busy;
        }

        /* Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
        if(system_cpld_smbus_check_ack(cpld_data, i2c_stat_addr, &i2c_stat_data))
        {
            stop_step = 9;
            goto no_ack_response;
        }

        byte_to_rw--;
        temp_offset++;
    }

    /* 11. Set CR to 8'h40 to issue a STOP command.*/
    i2c_cmd_data = COMMAND_STOP;
    SYSTEM_CPLD_ACCESS(writeb(i2c_cmd_data, cpld_data->base_addr + i2c_cmd_addr);)
    SYS_CPLD_DEBUG("[12] i2c_cmd_addr:%x i2c_cmd_data:%x\n", i2c_cmd_addr, i2c_cmd_data);

    /* Check the BUSY bit of SR, to make sure STOP is sent.*/
    if(system_cpld_smbus_check_busy(cpld_data, i2c_stat_addr, &i2c_stat_data))
    {
        stop_step = 12;
        goto device_busy;
    }

    up(&(cpld_data->sem[channel]));
    return(0);

device_busy:
    SYS_CPLD_DEBUG("device_busy, write failed, channel:%d addr:0x%x, length:0x%x, offset:0x%x, stop_step:%d, i2c_stat:0x%x.\n", channel, addr, length, offset, stop_step, i2c_stat_data);

    /* Re-initailize BUS.*/
    /* 0. Disable the core by writing 8'h00 to the Control Register, CTR.*/
    SYSTEM_CPLD_ACCESS(writeb(CONTROL_DISABLE, cpld_data->base_addr + I2C_CONTROLLER_REG + channel*0x08);)
    /* 1. Program the clock PRESCALE registers, PRERlo and PRERhi, with
    the desired value. This value is determined by the clock frequency and the speed of the I2C bus.*/
    SYSTEM_CPLD_ACCESS(writeb(PRER_HI_DEFAULT, cpld_data->base_addr + I2C_PRESCALE_HIGH_REG + channel*0x08);)
    SYSTEM_CPLD_ACCESS(writeb(PRER_LO_DEFAULT, cpld_data->base_addr + I2C_PRESCALE_LOW_REG + channel*0x08);)

    /* 2. Enable the core by writing 8'h80 to the Control Register, CTR.*/
    SYSTEM_CPLD_ACCESS(writeb(CONTROL_ENABLE, cpld_data->base_addr + I2C_CONTROLLER_REG + channel*0x08);)
    SYSTEM_CPLD_SLEEP(500);

    /*Set CR to 8'h40 to issue a STOP command to avoid error.*/
    i2c_cmd_data = COMMAND_STOP;
    SYSTEM_CPLD_ACCESS(writeb(i2c_cmd_data, cpld_data->base_addr + i2c_cmd_addr);)
    /* Check the BUSY bit of SR, to make sure STOP is sent.*/
    if(system_cpld_smbus_check_busy(cpld_data, i2c_stat_addr, &i2c_stat_data))
    {
        SYS_CPLD_DEBUG("device_busy, fail to issue STOP, i2c_stat:0x%x.\n", i2c_stat_data);
    }

    up(&(cpld_data->sem[channel]));
    return (-EBUSY);

no_ack_response:
    /* Set CR to 8'h40 to issue a STOP command to avoid error.*/
    i2c_cmd_data = COMMAND_STOP;
    SYSTEM_CPLD_ACCESS(writeb(i2c_cmd_data, cpld_data->base_addr + i2c_cmd_addr);)

    SYS_CPLD_DEBUG("no_ack_response, write failed, channel:%d addr:0x%x, length:0x%x, offset:0x%x, stop_step:%d, i2c_stat:0x%x.\n", channel, addr, length, offset, stop_step, i2c_stat_data);

    /* Check the BUSY bit of SR, to make sure STOP is sent.*/
    if(system_cpld_smbus_check_busy(cpld_data, i2c_stat_addr, &i2c_stat_data))
    {
        stop_step = 13;
        goto device_busy;
    }

    up(&(cpld_data->sem[channel]));
    return (-ETIMEDOUT);
}

/* Return negative errno on error. */
static s32 system_cpld_smbus_access(struct i2c_adapter *adap, u16 addr,
		       unsigned short flags, char read_write, u8 command,
		       int size, union i2c_smbus_data *data)
{
    struct switch_system_cpld_data *cpld_data;
    unsigned char buff[MAX_BUFF_SIZE] = {0};
    int retval = 0;
    unsigned char length = 0, channel = 0;
    char cha_num_str[3] = {0};
    unsigned char addr_shift = 0;
    unsigned char pec = 0;

    SYS_CPLD_DEBUG("[%s]name:%s addr:0x%x flags:0x%x read_write:0x%x command:0x%x size:0x%x\n",
        __func__, adap->name, addr, flags, read_write, command, size);

    switch(size)
    {
        case I2C_SMBUS_BYTE:
            if(I2C_SMBUS_WRITE == read_write)
            {
                /* It must be 0 to avoid null pointer exception in smbus_write */
                length = 0;
            }
            else
            {
                length = 1;
            }
            break;
        case I2C_SMBUS_BYTE_DATA:
            length = 1;
            if(read_write == I2C_SMBUS_WRITE)
            {
                buff[0] = data->byte;
            }
            break;
        case I2C_SMBUS_WORD_DATA:
            length = 2;
            if(read_write == I2C_SMBUS_WRITE)
            {
                buff[0] = data->word & 0xff;
                buff[1] = data->word >> 8;
            }
            break;
        case I2C_SMBUS_BLOCK_DATA:
            length = I2C_SMBUS_BLOCK_MAX;
            if(read_write == I2C_SMBUS_WRITE)
            {
                length = data->block[0] + 1;
                memcpy(buff, data->block, length);
            }
            break;
        case I2C_SMBUS_I2C_BLOCK_DATA:
            if(data->block[0] > I2C_SMBUS_BLOCK_MAX)
            {
                SYS_CPLD_DEBUG("Error:Invalid block %s size %d\n",
                    read_write == I2C_SMBUS_READ ? "read" : "write",
                    data->block[0]);
                return -EINVAL;
            }
            length = data->block[0];
            if(read_write == I2C_SMBUS_WRITE)
            {
                memcpy(buff, data->block + 1, data->block[0]);
            }
            break;
        case I2C_SMBUS_QUICK:
            length = 0;
            break;
        default:
            SYS_CPLD_DEBUG("Error:Unsupported transaction %d\n", size);
            return -EOPNOTSUPP;
    }

    /* some device support PEC, so check PEC function */
    if((flags & I2C_CLIENT_PEC) && size != I2C_SMBUS_QUICK
                          && size != I2C_SMBUS_I2C_BLOCK_DATA)
    {
        /*  we will ignore the PEC when read*/
        if(I2C_SMBUS_WRITE == read_write)
        {
            addr_shift = (unsigned char)(addr << 1);
            pec = pmbus_pec_calc(pec, &addr_shift, 1);
            pec = pmbus_pec_calc(pec, &command, 1);
            pec = pmbus_pec_calc(pec, (unsigned char *)buff, length);
            *((unsigned char *)buff + length) = pec;
            length++;
        }
    }

    memcpy(cha_num_str, (adap->name+strlen(SYSTEM_CPLD_SMBUS_NAME)), 2);
    retval = kstrtou8(cha_num_str, 10, &channel);
    if(retval == 0)
    {
        SYS_CPLD_DEBUG("channel:%d \n", channel);
    }
    else
    {
        SYS_CPLD_DEBUG("Error:%d, channel:%s \n", retval, cha_num_str);
    }

    cpld_data = i2c_get_adapdata(adap);
    if ((cpld_data == NULL)||(length > I2C_SMBUS_BLOCK_MAX + 2))
    {
        return(-EINVAL);
    }

    if(I2C_SMBUS_READ == read_write)
    {
        retval = system_cpld_smbus_read(cpld_data, channel, addr, command, length, buff, size);
    }
    else if(I2C_SMBUS_WRITE == read_write)
    {
        retval = system_cpld_smbus_write(cpld_data, channel, addr, command, length, buff);
    }

    if (read_write == I2C_SMBUS_READ)
    {
        switch (size)
        {
            case I2C_SMBUS_BYTE:
                data->byte = buff[0];
                break;
            case I2C_SMBUS_BYTE_DATA:
                data->byte = buff[0];
                break;
            case I2C_SMBUS_WORD_DATA:
                data->word = buff[0] | (buff[1] << 8);
                break;
            case I2C_SMBUS_I2C_BLOCK_DATA:
                memcpy(data->block + 1, buff, data->block[0]);
                break;
            case I2C_SMBUS_BLOCK_DATA:
                if (buff[0] > I2C_SMBUS_BLOCK_MAX)
                {
                    SYS_CPLD_DEBUG("Error:Invalid block size returned: %d\n", buff[0]);
                    return(-EPROTO);
                }
                memcpy(data->block, buff, buff[0] + 1);
                break;
        }
    }

    return retval;
}

static u32 system_cpld_smbus_func(struct i2c_adapter *adapter)
{
    return I2C_FUNC_SMBUS_QUICK | I2C_FUNC_SMBUS_BYTE |
           I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA |
           I2C_FUNC_SMBUS_BLOCK_DATA | I2C_FUNC_SMBUS_I2C_BLOCK |
           I2C_FUNC_SMBUS_PEC;
}

static const struct i2c_algorithm smbus_algorithm = {
    .smbus_xfer	= system_cpld_smbus_access,
    .functionality	= system_cpld_smbus_func,
};


#if 1 // For user space API uages
/* file_operations: ioctl */
struct switch_cpld_access {
    /* intput output buff */
    unsigned int    offset;
    unsigned char   buf;
};

#define SWITCH_CPLD_MAGIC  0xA7
#define SWITCH_CPLD_IO     0x11

#define SWITCH_CPLD_READ   _IOR(SWITCH_CPLD_MAGIC, SWITCH_CPLD_IO,   struct switch_cpld_access)
#define SWITCH_CPLD_WRITE  _IOW(SWITCH_CPLD_MAGIC, SWITCH_CPLD_IO,   struct switch_cpld_access)

#endif

struct class *switch_cpld_driver_class;
struct cdev switch_cpld_cdev;
dev_t switch_cpld_dev_num;
#define SWITCH_CPLD_DEV_NAME   "switch_cpld_dev"

struct switch_system_cpld_data *global_switch_cpld_dev = NULL;

long switch_cpld_ioctl(struct file *filp, unsigned int cmd, unsigned long user_addr)
{
    struct switch_cpld_access acc = {0};
    struct switch_system_cpld_data *data = global_switch_cpld_dev;
    int retval = 0;
    void __iomem  *base_addr = data->base_addr;

    if (cmd == SWITCH_CPLD_READ)
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
        if (!access_ok(VERIFY＿READ,user_addr, _IOC_SIZE(cmd)))
#else
        if (!access_ok(user_addr, _IOC_SIZE(cmd)))
#endif   
        {
            return(-EINVAL);
        }

        retval = copy_from_user((void *) &acc, (const void *) user_addr, sizeof(struct switch_cpld_access));

        mutex_lock(&data->update_lock);
        acc.buf = readb(base_addr + acc.offset);
        mutex_unlock(&data->update_lock);
    
        retval = copy_to_user((void *) user_addr, (const void *) &acc, sizeof(struct switch_cpld_access));
        return(retval);
    }
    else if(cmd == SWITCH_CPLD_WRITE)
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
        if (!access_ok(VERIFY＿WRITE,user_addr, _IOC_SIZE(cmd)))
#else
        if (!access_ok(user_addr, _IOC_SIZE(cmd)))
#endif           
        {
            return(-EINVAL);
        }

        retval = copy_from_user((void *) &acc, (const void *) user_addr, sizeof(struct switch_cpld_access));

        mutex_lock(&data->update_lock);
        writeb(acc.buf, base_addr + acc.offset);
        mutex_unlock(&data->update_lock);

        retval = copy_to_user((void *) user_addr, (const void *) &acc, sizeof(struct switch_cpld_access));
        return(retval);
    }
    else
    {
        SYS_CPLD_DEBUG( "[%s] Unknown command\n", __func__);
        return(-EINVAL);
    }
}
EXPORT_SYMBOL_GPL(switch_cpld_ioctl);

static struct file_operations switch_cpld_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = switch_cpld_ioctl,
};


void system_cpld_smbus_init(struct switch_system_cpld_data *cpld_data)
{
    unsigned int i2c_ctrl_addr = 0;
    unsigned int i2c_ctrl_data = 0;
    unsigned int i2c_pre_low_addr = 0, i2c_pre_high_addr = 0;
    unsigned int i2c_pre_low_data = 0, i2c_pre_high_data = 0;
    int temp_channel = 0;

    for(temp_channel = 0; temp_channel < I2C_MAX_CHANNEL; temp_channel++)
    {
        /*
        1. Program the clock PRESCALE registers, PRERlo and PRERhi, with the desired value.
        This value is determined by the clock frequency and the speed of the I2C bus.
        2. Enable the core by writing 8'h80 to the Control Register, CTR.
        */
        i2c_pre_low_addr = I2C_PRESCALE_LOW_REG + temp_channel*0x08;
        i2c_pre_high_addr = I2C_PRESCALE_HIGH_REG + temp_channel*0x08;
        i2c_ctrl_addr = I2C_CONTROLLER_REG + temp_channel*0x08;

        /* 1. Program the clock PRESCALE registers, PRERlo and PRERhi,
        with the desired value. This value is determined by the clock frequency and the speed of the I2C bus.*/
        i2c_pre_low_data = PRER_LO_DEFAULT;
        SYSTEM_CPLD_ACCESS(writeb(i2c_pre_low_data, cpld_data->base_addr + i2c_pre_low_addr);)
        SYS_CPLD_DEBUG("[pre1-1] i2c_pre_low_addr:%x %x\n", i2c_pre_low_addr, i2c_pre_low_data);

        i2c_pre_high_data = PRER_HI_DEFAULT;
        SYSTEM_CPLD_ACCESS(writeb(i2c_pre_high_data, cpld_data->base_addr + i2c_pre_high_addr);)
        SYS_CPLD_DEBUG("[pre1-2] i2c_pre_high_addr:%x %x\n", i2c_pre_high_addr, i2c_pre_high_data);

        /* 2. Enable the core by writing 8'h80 to the Control Register, CTR.*/
        i2c_ctrl_data= CONTROL_ENABLE;
        SYSTEM_CPLD_ACCESS(writeb(i2c_ctrl_data, cpld_data->base_addr + i2c_ctrl_addr);)
        SYS_CPLD_DEBUG("[pre1] i2c_ctrl_addr:%x %x\n", i2c_ctrl_addr, i2c_ctrl_data);
    }

    return;
}

static int switch_system_cpld_probe(struct platform_device *pdev)
{
    struct switch_system_cpld_data *data = NULL;
    int smbus_temp_num = 0;
    int smbus_adapter_num = 0;
    int retval = 0;
    dev_t curr_dev;

    data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
    if (!data) 
        return -ENOMEM;

    if (!request_mem_region(LPC_DEVICE_ADDRESS1, LPC_IO_MEMORY_SIZE, "cpld-lpc")) {
        dev_err(&pdev->dev, "Couldn't allocate cpld-lpc memory region\n");
        return -ENODEV;
    }

    if ((data->base_addr = ioremap(LPC_DEVICE_ADDRESS1, LPC_IO_MEMORY_SIZE)) == NULL) {
        release_mem_region(LPC_DEVICE_ADDRESS1, LPC_IO_MEMORY_SIZE);
        dev_err(&pdev->dev, "Remap cpld-lpc memory failed\n");
        return -1;
    }

    dev_info(&pdev->dev, "[%s] lpc_base0: 0x%p\n", __func__, data->base_addr);

    mutex_init(&data->update_lock);
    data->dev = &pdev->dev;
    platform_set_drvdata(pdev, data);

    /* Request the kernel for N_MINOR devices */
    alloc_chrdev_region(&switch_cpld_dev_num, 0, 1, "switch_cpld_driver");

    /* Create a class : appears at /sys/class */
    switch_cpld_driver_class = class_create(THIS_MODULE, "switch_cpld_driver_class");

    /* Initialize and create each of the device(cdev) */
    {
        /* Associate the cdev with a set of file_operations */
        cdev_init(&switch_cpld_cdev, &switch_cpld_fops);

        /* Build up the current device number. To be used further */
        curr_dev = MKDEV(MAJOR(switch_cpld_dev_num), MINOR(switch_cpld_dev_num));

        /* Create a device node for this device. Look, the class is
         * being used here. The same class is associated with N_MINOR
         * devices. Once the function returns, device nodes will be
         * created as /dev/my_dev0, /dev/my_dev1,... You can also view
         * the devices under /sys/class/my_driver_class.
         */
        device_create(switch_cpld_driver_class, NULL, curr_dev, NULL, SWITCH_CPLD_DEV_NAME);

        /* Now make the device live for the users to access */
        cdev_add(&switch_cpld_cdev, curr_dev, 1);
    }

    for (smbus_temp_num = 0; smbus_temp_num < I2C_MAX_CHANNEL; smbus_temp_num++)
    {
        sema_init(&(data->sem[smbus_temp_num]), 1);
    }

    for (smbus_temp_num = 0; smbus_temp_num < I2C_MAX_CHANNEL; smbus_temp_num++)
    {
        i2c_set_adapdata(&data->adapter[smbus_temp_num], data);
        data->adapter[smbus_temp_num].owner = THIS_MODULE;
        data->adapter[smbus_temp_num].class = I2C_CLASS_HWMON | I2C_CLASS_SPD;
        data->adapter[smbus_temp_num].algo = &smbus_algorithm;
        data->adapter[smbus_temp_num].dev.parent = &pdev->dev;
        data->adapter[smbus_temp_num].retries = 3;

        /* Default timeout in interrupt mode: 200 ms */
        data->adapter[smbus_temp_num].timeout = HZ / 5;

        snprintf(data->adapter[smbus_temp_num].name, sizeof(data->adapter[smbus_temp_num].name),
            "%s%02d adapter", SYSTEM_CPLD_SMBUS_NAME, smbus_temp_num);

        data->adapter[smbus_temp_num].nr = 101 + smbus_temp_num;
        retval = i2c_add_numbered_adapter(&data->adapter[smbus_temp_num]);

        if (retval) {
            dev_err(&pdev->dev, "Add i2c adapter failed\n");
            goto fail;
        }
    }

    system_cpld_smbus_init(data);
    global_switch_cpld_dev = data;
    return 0;

fail:
    for (smbus_adapter_num = 0; smbus_adapter_num < smbus_temp_num; smbus_adapter_num++)
    {
        i2c_del_adapter(&data->adapter[smbus_adapter_num]);
    }
    release_mem_region(LPC_DEVICE_ADDRESS1, LPC_IO_MEMORY_SIZE);
    mutex_destroy(&data->update_lock);
    return retval;
}

static int switch_system_cpld_remove(struct platform_device *pdev)
{
    struct switch_system_cpld_data *data = platform_get_drvdata(pdev);
    void __iomem  *base_addr = data->base_addr;
    int smbus_temp_num = 0;

    if(base_addr != NULL)
        iounmap(base_addr);
    release_mem_region(LPC_DEVICE_ADDRESS1, LPC_IO_MEMORY_SIZE);

    data->base_addr = NULL;
    mutex_destroy(&data->update_lock);
    for(smbus_temp_num = 0; smbus_temp_num < I2C_MAX_CHANNEL; smbus_temp_num++)
    {
        i2c_del_adapter(&data->adapter[smbus_temp_num]);
    }

    device_destroy(switch_cpld_driver_class, switch_cpld_dev_num);
    class_destroy(switch_cpld_driver_class);
    cdev_del(&switch_cpld_cdev);
    unregister_chrdev_region(switch_cpld_dev_num, 1);

    return 0;
}

int switch_system_cpld_read(u8 reg, u8 *value)
{
    struct switch_system_cpld_data *data = NULL;
    void __iomem *lpc_base = NULL;
    u8 val = 0;

    if(switch_system_cpld_device == NULL)
    {
        return -1;
    }

    data = platform_get_drvdata(switch_system_cpld_device);
    lpc_base = data->base_addr;
    if(lpc_base == NULL)
    {
        return -1;
    }

    mutex_lock(&data->update_lock);
    val = readb(lpc_base + reg);
    mutex_unlock(&data->update_lock);
    SYS_CPLD_DEBUG( "[%s] base: 0x%p, reg:0x%x, val:0x%x\n", __func__, lpc_base, reg, val);

    *value = val;
    return 0;
}
EXPORT_SYMBOL(switch_system_cpld_read);

int switch_system_cpld_write(u8 reg, u8 value)
{
    struct switch_system_cpld_data *data = NULL;
    void __iomem *lpc_base = NULL;

    if(switch_system_cpld_device == NULL)
    {
        return -1;
    }

    data = platform_get_drvdata(switch_system_cpld_device);
    lpc_base = data->base_addr;
    if(lpc_base == NULL)
    {
        return -1;
    }

    SYS_CPLD_DEBUG( "[%s] base: 0x%x, reg:0x%x, val:0x%x\n", __func__, lpc_base, reg, value);
    mutex_lock(&data->update_lock);
    writeb(value, lpc_base + reg);
    mutex_unlock(&data->update_lock);

    return 0;
}
EXPORT_SYMBOL(switch_system_cpld_write);

static struct platform_driver switch_system_cpld_driver = {
     .driver = {
         .name  = DRV_NAME,
         .owner = THIS_MODULE,
     },
     .probe     = switch_system_cpld_probe,
     .remove    = switch_system_cpld_remove,
};
 
static int __init switch_system_cpld_init(void)
{
    int err=0;

    switch_system_cpld_device = platform_device_alloc(DRV_NAME, -1);
    if(!switch_system_cpld_device)
    {
        SYS_CPLD_DEBUG("%s(#%d): platform_device_alloc fail\n", __func__, __LINE__);
        return -ENOMEM;
    }

    err = platform_device_add(switch_system_cpld_device);
    if(err)
    {
        SYS_CPLD_DEBUG("%s(#%d): platform_device_add failed\n", __func__, __LINE__);
        platform_device_put(switch_system_cpld_device);
        return -ENODEV;
    }

    err = platform_driver_register(&switch_system_cpld_driver);
    if(err)
    {
        SYS_CPLD_DEBUG("%s(#%d): platform_driver_register failed(%d)\n", __func__, __LINE__, err);
        platform_device_unregister(switch_system_cpld_device);
    }

    return err;
}

static void __exit switch_system_cpld_exit(void)
{
    platform_device_unregister(switch_system_cpld_device);
    platform_driver_unregister(&switch_system_cpld_driver);
}


module_init(switch_system_cpld_init);
module_exit(switch_system_cpld_exit);

MODULE_DESCRIPTION("Switch System CPLD Driver");
MODULE_VERSION(SYSTEM_CPLD_DRIVER_VERSION);
MODULE_LICENSE("GPL");

