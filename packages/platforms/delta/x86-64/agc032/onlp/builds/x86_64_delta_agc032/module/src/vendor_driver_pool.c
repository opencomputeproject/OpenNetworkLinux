/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2019 Delta Electronics, Inc.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <AIM/aim.h>
#include "vendor_driver_pool.h"
#include "vendor_i2c_device_list.h"

static int vendor_driver_add(vendor_driver_t *driver);

/*
    Two I2C bus driver here:
        SMBUS: using onlp i2c driver
        IPMB: using raw command

    I2C BUS DRIVER START:
*/

uint32_t i2c_flag = ONLP_I2C_F_FORCE;

static int smbus_readb(int bus, uint8_t addr, uint16_t offset)
{
    //AIM_LOG_ERROR("Function: %s, bus: %d,addr: %d  \n", __FUNCTION__, bus,addr);

    int rv = 0;

    rv = onlp_i2c_readb(bus, addr, offset, i2c_flag);

    if(rv < 0)
    {
        AIM_LOG_ERROR("I2C readb failed bus: %d, addr: %d, offset: %d.",
            bus, addr, offset);
    }
    /*else
    {
        AIM_LOG_ERROR("I2C readb: bus: %02x, addr: %02x, offset: %02x, value: %02x",
                bus, addr, offset, rv);
    }*/
    return rv;

}

static int smbus_writeb(int bus, uint8_t addr, uint16_t offset, uint8_t byte)
{
    int rv = 0;
    rv = onlp_i2c_writeb(bus, addr, offset, byte, i2c_flag);
    if(rv < 0)
    {
        AIM_LOG_ERROR("I2C writeb failed bus: %d, addr: %d, offset: %d.",
            bus, addr, offset);
    }
    /*else
    {
        AIM_LOG_ERROR("I2C writeb: bus: %02x, addr: %02x, offset: %02x, value: %02x",
                bus, addr, offset, byte);
    }*/
    return rv;
}

static int smbus_readw(int bus, uint8_t addr, uint16_t offset)
{
    int rv = 0;
    rv = onlp_i2c_readw(bus, addr, offset, i2c_flag);
    if(rv < 0)
    {
        AIM_LOG_ERROR("I2C readw failed bus: %d, addr: %d, offset: %d.",
            bus, addr, offset);
    }
    return rv;
}

static int smbus_writew(int bus, uint8_t addr, uint16_t offset, uint16_t word)
{
    int rv = 0;
    rv = onlp_i2c_writew(bus, addr, offset, word, i2c_flag);
    if(rv < 0)
    {
        AIM_LOG_ERROR("I2C writew failed bus: %d, addr: %d, offset: %d.",
            bus, addr, offset);
    }
    return rv;
}

static int smbus_block_read(int bus, uint8_t addr, uint16_t offset, int size, uint8_t* rdata)
{
    int rv = 0;
    rv = onlp_i2c_block_read(bus, addr, offset, size, rdata, i2c_flag);
    if(rv < 0)
    {
        AIM_LOG_ERROR("I2C block read failed bus: %d, addr: %d, offset: %d.",
            bus, addr, offset);
    }
    /*else
    {
        AIM_LOG_ERROR("I2C block_read: bus: %02x, addr: %02x, offset: %02x, value: %02x",
                bus, addr, offset, rv);
    }*/
    return rv;
}

static i2c_bus_driver_t smbus_functions =
{
    smbus_readb,
    smbus_writeb,
    smbus_readw,
    smbus_writew,
    smbus_block_read
};

static int smbus_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "I2C", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &smbus_functions;

    return vendor_driver_add(driver);
}

static int ipmb_readb(int bus, uint8_t addr, uint16_t offset)
{
    int rv = 0, idx = 0;
    char ipmi_cmd[80], rv_char[256], *delim = " ", *tmp;
    uint8_t rv_data[32] = {0};

    sprintf(ipmi_cmd, "ipmitool raw 0x3c 0x01 %d %d %d %d", bus, addr, offset, 1);
    if (vendor_system_call_get(ipmi_cmd, rv_char) != 0)
    {
        AIM_LOG_ERROR("IPMITOOL command: \"%s\": Get Data Failed (ret: %d)", ipmi_cmd);
        return ONLP_STATUS_E_INTERNAL;
    }

    tmp = strtok(rv_char, delim);

    while (tmp != NULL)
    {
        rv_data[idx] = strtol(tmp,NULL,16);
        tmp = strtok (NULL, delim);
        idx++;
    }

    rv = rv_data[0] & 0xff;

    /*
    AIM_LOG_ERROR("IPMITOOL readb: bus: %02x, addr: %02x, offset: %02x, value: %x",
                bus, addr, offset, rv);
    */
    return rv;
}

static int ipmb_writeb(int bus, uint8_t addr, uint16_t offset, uint8_t byte)
{
    int rv = 0;
    char ipmi_cmd[80];

    sprintf(ipmi_cmd, "ipmitool raw 0x3c 0x02 %d %d %d %d %d > /dev/null", bus, addr, offset, 1, byte);
    if (vendor_system_call_set(ipmi_cmd) != 0)
    {
        AIM_LOG_ERROR("IPMITOOL command: \"%s\": Set Data Failed.", ipmi_cmd);
        return ONLP_STATUS_E_INTERNAL;
    }
    /*else
    {
        AIM_LOG_ERROR("IPMITOOL writeb: bus: %02x, addr: %02x, offset: %02x, data: %02x",
                bus, addr, offset, byte);
    }*/

    return rv;
}

static int ipmb_readw(int bus, uint8_t addr, uint16_t offset)
{
    int rv = 0, idx = 0;
    char ipmi_cmd[80], rv_char[256], *delim = " ", *tmp;
    uint16_t rv_data[32] = {0};

    sprintf(ipmi_cmd, "ipmitool raw 0x3c 0x01 %d %d %d %d", bus, addr, offset, 2);
    if (vendor_system_call_get(ipmi_cmd, rv_char) != 0)
    {
        AIM_LOG_ERROR("IPMITOOL command: \"%s\": Get Data Failed.", ipmi_cmd);
        return ONLP_STATUS_E_INTERNAL;
    }

    tmp = strtok(rv_char, delim);

    while (tmp != NULL)
    {
        rv_data[idx] = strtol(tmp,NULL,16);
        tmp = strtok (NULL, delim);
        idx++;
    }

    rv = (rv_data[0] & 0xff) + ((rv_data[1]&0xff) << 8);

    /*
    AIM_LOG_ERROR("IPMITOOL readw: bus: %02x, addr: %02x, offset: %02x, value: %x",
                bus, addr, offset, rv);
    */

    return rv;
}

static int ipmb_writew(int bus, uint8_t addr, uint16_t offset, uint16_t word)
{
    int rv = 0;
    char ipmi_cmd[80];

    sprintf(ipmi_cmd, "ipmitool raw 0x3c 0x02 %d %d %d %d %d > /dev/null", bus, addr, offset, 2, word);
    if (vendor_system_call_set(ipmi_cmd) != 0)
    {
        AIM_LOG_ERROR("IPMITOOL command: \"%s\": Set Data Failed.", ipmi_cmd);
        return ONLP_STATUS_E_INTERNAL;
    }

    /*
    AIM_LOG_ERROR("IPMITOOL writew: bus: %02x, addr: %02x, offset: %02x, value: %x",
                bus, addr, offset, word);
    */

    return rv;
}

static int ipmb_block_read(int bus, uint8_t addr, uint16_t offset, int size, uint8_t* rdata)
{
    int rv = 0, idx = 0;
    char ipmi_cmd[80], rv_char[256], *delim = " ", *tmp;
    uint8_t rv_data[32] = {0};

    if (size > 64)
    {
        AIM_LOG_ERROR("The limitation of size is 64.");
        return ONLP_STATUS_E_INTERNAL;
    }

    sprintf(ipmi_cmd, "ipmitool raw 0x3c 0x01 %d %d %d %d", bus, addr, offset, size+1);
    if (vendor_system_call_get(ipmi_cmd, rv_char) != 0)
    {
        AIM_LOG_ERROR("IPMITOOL command: \"%s\": Block read Failed.", ipmi_cmd);
        return ONLP_STATUS_E_INTERNAL;
    }

    tmp = strtok(rv_char, delim);

    while (tmp != NULL)
    {
        rv_data[idx] = strtol(tmp,NULL,16);
        tmp = strtok (NULL, delim);
        idx++;
    }

    for(idx = 0; idx < size; idx++)
        rdata[idx] = rv_data[idx+1] & 0xff;

    return rv;
}

static i2c_bus_driver_t ipmb_functions =
{
    ipmb_readb,
    ipmb_writeb,
    ipmb_readw,
    ipmb_writew,
    ipmb_block_read
};

static int ipmb_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "IPMB", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &ipmb_functions;

    return vendor_driver_add(driver);
}

/*
    I2C BUS DRIVER END:
*/

/*
    IPMI BUS DRIVER START:
*/
static int ipmi_get(int bus, char *cmd, char *filter, char *data, uint32_t dlen)
{
    char sys_cmd[128];
    char rv_char[256];

    if (bus) {}

    if (!cmd || !filter)
        return 0;

    sprintf(sys_cmd, "ipmitool %s %s", cmd, filter);
    if (vendor_system_call_get(sys_cmd, rv_char) != 0)
        return 0;

    memcpy(data, rv_char, (dlen-1));
    data[dlen-1] = '\0';

    return 0;

}

static int ipmi_set(int bus, char *cmd, char *filter)
{
    char sys_cmd[128];

    if (bus) {}

    if (!cmd || !filter)
        return 0;

    sprintf(sys_cmd, "ipmitool %s %s > /dev/null", cmd, filter);

    return vendor_system_call_set(sys_cmd);

}

ipmi_bus_driver_t ipmi_functions =
{
    ipmi_get,
    ipmi_set
};

static int ipmi_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "IPMI", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &ipmi_functions;

    return vendor_driver_add(driver);
}

/*
    IPMI BUS DRIVER END:
*/

/*
    DEVICE DRIVER START:
*/

/* CPLD DEVICE START*/
static int cpld_read(void *busDrvPtr, int bus, uint8_t addr, uint8_t offset, uint8_t *value)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;

    rv = i2c->readb(bus, addr, offset);
    if(rv >= 0)
    {
        *value = rv;
    }

    return rv;
}

static int cpld_write(void *busDrvPtr, int bus, uint8_t addr, uint8_t offset, uint8_t value)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;

    rv = i2c->writeb(bus, addr, offset, value);

    return rv;
}

static cpld_dev_driver_t cpld_functions =
{
    cpld_read,
    cpld_write
};

static int cpld_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "CPLD", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &cpld_functions;

    return vendor_driver_add(driver);
}
/* CPLD DEVICE END*/

/* EEPROM DEVICE START*/
static int eeprom_readb(void *busDrvPtr, int bus, uint8_t addr, uint16_t offset, uint16_t len, uint8_t *buf)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0, index = 0;

    for(index = 0; index < len; index++)
    {
        rv = i2c->readb(bus, addr, offset+index);
        if(rv < 0)
        {
            return ONLP_STATUS_E_INTERNAL;
        }
        *(buf+index) = (uint8_t) (rv & 0xff);
    }

    return 0;
}

static int eeprom_writeb(void *busDrvPtr, int bus, uint8_t addr, uint16_t offset, uint16_t len, uint8_t *buf)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0, index = 0;

    for(index = 0; index<len; index++)
    {
        rv = i2c->writeb(bus, addr, offset+index, *(buf+index));
        usleep(5000);
        if(rv < 0)
        {
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    return 0;
}

static int eeprom_load(void *busDrvPtr, int bus, uint8_t addr, uint8_t *buf)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;

    rv = i2c->block_read(bus, addr, 0, 256, buf);

    return rv;
}

static eeprom_dev_driver_t eeprom_functions =
{
    eeprom_readb,
    eeprom_writeb,
    eeprom_load,
};

static int eeprom_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "EEPROM", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &eeprom_functions;

    return vendor_driver_add(driver);
}
/* EEPROM DEVICE END*/

/* FAN DEVICE EMC2305 START*/
static const uint8_t fan_config1_reg[] =
{
    0x32, 0x42, 0x52, 0x62, 0x72
};

static const uint8_t fan_tach_reg[][2] =
{
    {0x3e, 0x3f}, {0x4e, 0x4f}, {0x5e, 0x5f}, {0x6e, 0x6f}, {0x7e, 0x7f},
};

static const uint8_t fan_tach_target_reg[][2] =
{
    {0x3d, 0x3c}, {0x4d, 0x4c}, {0x5d, 0x5c}, {0x6d, 0x6c}, {0x7d, 0x7c},
};

static int emc2305_rpm_get(void *busDrvPtr, int bus, uint8_t addr, int id, int *rpm)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    uint16_t tach_count = 0, htach = 0, ltach = 0;

    rv = i2c->readb(bus, addr, fan_tach_reg[id][0]);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;
    htach = rv;

    rv = i2c->readb(bus, addr, fan_tach_reg[id][1]);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;
    ltach = rv;

    tach_count = (((htach << 8) | ltach) >> 0x3);

    if(tach_count == 0)
    {
        *rpm = 0;
        return 0;
    }

    *rpm = 7864320 / tach_count;

    return 0;
}

static int emc2305_rpm_set(void *busDrvPtr, int bus, uint8_t addr, int id, int rpm)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    uint16_t tach_count = 0, htach = 0, ltach = 0;

    tach_count = ((7864320 / rpm) << 0x3);
    htach =  (tach_count & 0xff00) >> 8;
    ltach =  tach_count & 0xff;

    rv = i2c->writeb(bus, addr, fan_tach_target_reg[id][0], (uint8_t) htach);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;

    rv = i2c->writeb(bus, addr, fan_tach_target_reg[id][1], (uint8_t) ltach);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;

    /* Set to RPM Mode */
    rv = i2c->writeb(bus, addr, fan_config1_reg[id], 0xab);
    return 0;
}

static fan_dev_driver_t emc2305_functions =
{
    emc2305_rpm_get,
    emc2305_rpm_set
};

static int emc2305_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "EMC2305", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &emc2305_functions;

    return vendor_driver_add(driver);
}
/* FAN DEVICE EMC2305 END*/

/*THERMAL DEVICE TMP75 START*/

#define TMP75_TEMP_REG        0x00
#define TMP75_CONFIG_REG      0x01
#define TMP75_TLOW_REG        0x02
#define TMP75_THIGH_REG       0x03

static int tmp75_temp_get(void *busDrvPtr, int bus, uint8_t addr, int id, int *temperature)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0, is_negative = 0;

    rv = i2c->readb(bus, addr, TMP75_TEMP_REG);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;

    if (rv & 0x800)
    {
        is_negative = 1;
        rv = ~(rv) + 1; /* 2's complement */
        rv &= 0x7ff;
	}

    rv = (((rv << 8) | (rv >> 8)) >> 4);
    *temperature = rv * 62.5;
    if(is_negative) *temperature *= -1;

    return 0;
}

static int tmp75_limit_get(void *busDrvPtr, int bus, uint8_t addr, int id, int type, int *temperature)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    uint8_t limitAddr = 0;

    switch (type)
    {
        case VENDOR_THERMAL_LOW_THRESHOLD:
            limitAddr = TMP75_TLOW_REG;
            break;
        case VENDOR_THERMAL_HIGH_THRESHOLD:
            limitAddr = TMP75_THIGH_REG;
            break;
        default:
            AIM_LOG_ERROR("VENDOR: TMP75 unknow limit type!");
            break;
    }

    rv = i2c->readb(bus, addr, limitAddr);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;

    *temperature = rv*1000;

    return 0;
}

static int tmp75_limit_set(void *busDrvPtr, int bus, uint8_t addr, int id, int type, int temperature)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    uint8_t limitAddr = 0;

    switch (type)
    {
        case VENDOR_THERMAL_LOW_THRESHOLD:
            limitAddr = TMP75_TLOW_REG;
            break;
        case VENDOR_THERMAL_HIGH_THRESHOLD:
            limitAddr = TMP75_THIGH_REG;
            break;
        default:
            AIM_LOG_ERROR("VENDOR: TMP75 unknow limit type!");
            break;
    }

    rv = i2c->writeb(bus, addr, limitAddr, (uint16_t) temperature);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;

    return 0;
}

static thermal_dev_driver_t tmp75_functions =
{
    tmp75_temp_get,
    tmp75_limit_get,
    tmp75_limit_set
};

static int tmp75_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "TMP75", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &tmp75_functions;

    return vendor_driver_add(driver);
}
/*THERMAL DEVICE TMP75 END*/

/*THERMAL DEVICE TMP461 START*/

#define TMP461_HIGH_ADDRESS    0
#define TMP461_LOW_ADDRESS     1

static const uint8_t tmp461_thermal_reg[2] = { 0x1, 0x10 };
static const uint8_t tmp461_limit_reg_read_high[2] = { 0x7, 0x8 };
static const uint8_t tmp461_limit_reg_read_low[2] = { 0x13, 0x14 };
static const uint8_t tmp461_limit_reg_write_high[2] = { 0xd, 0xe };
static const uint8_t tmp461_limit_reg_write_low[2] = { 0x13, 0x14 };

static int tmp461_temp_get(void *busDrvPtr, int bus, uint8_t addr, int id, int *temperature)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    uint16_t temp_high = 0, temp_low = 0, is_negative = 0;
    rv = i2c->readb(bus, addr, tmp461_thermal_reg[TMP461_HIGH_ADDRESS]);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;
    temp_high = (uint16_t) rv;

    rv = i2c->readb(bus, addr, tmp461_thermal_reg[TMP461_LOW_ADDRESS]);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;
    temp_low = (uint16_t) rv;

    if (temp_high & 0x80)
    {
        is_negative = 1;
        temp_high = ~temp_high + 1; /* 2's complement */
        temp_high &= 0xff;
    }

    *temperature = temp_high*1000 + (temp_low >> 4) * 62.5;
    if (is_negative) *temperature *= -1;

    return 0;
}

static int tmp461_limit_get(void *busDrvPtr, int bus, uint8_t addr, int id, int type, int *temperature)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    uint8_t limitAddr_high = 0, limitAddr_low = 0;
    uint16_t temp_high = 0, temp_low = 0, is_negative = 0;

    switch (type)
    {
        case VENDOR_THERMAL_LOW_THRESHOLD:
            limitAddr_high = tmp461_limit_reg_read_high[TMP461_LOW_ADDRESS];
            limitAddr_low = tmp461_limit_reg_read_low[TMP461_LOW_ADDRESS];
            break;
        case VENDOR_THERMAL_HIGH_THRESHOLD:
            limitAddr_high = tmp461_limit_reg_read_high[TMP461_HIGH_ADDRESS];
            limitAddr_low = tmp461_limit_reg_read_low[TMP461_HIGH_ADDRESS];
            break;
        default:
            AIM_LOG_ERROR("VENDOR: TMP461 unknow limit type!");
            break;
    }

    rv = i2c->readb(bus, addr, limitAddr_high);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;
    temp_high = (uint16_t) rv;

    rv = i2c->readb(bus, addr, limitAddr_low);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;
    temp_low = (uint16_t) rv;

    if (temp_high & 0x80)
    {
        is_negative = 1;
        temp_high = ~temp_high + 1; /* 2's complement */
        temp_high &= 0xff;
    }

    *temperature = temp_high*1000 + (temp_low >> 4) * 62.5;
    if (is_negative) *temperature *= -1;

    return 0;
}

static int tmp461_limit_set(void *busDrvPtr, int bus, uint8_t addr, int id, int type, int temperature)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    uint8_t limitAddr_high = 0, limitAddr_low = 0;

    switch (type)
    {
        case VENDOR_THERMAL_LOW_THRESHOLD:
            limitAddr_high = tmp461_limit_reg_write_high[TMP461_LOW_ADDRESS];
            limitAddr_low = tmp461_limit_reg_write_low[TMP461_LOW_ADDRESS];
            break;
        case VENDOR_THERMAL_HIGH_THRESHOLD:
            limitAddr_high = tmp461_limit_reg_write_high[TMP461_HIGH_ADDRESS];
            limitAddr_low = tmp461_limit_reg_write_low[TMP461_HIGH_ADDRESS];
            break;
        default:
            AIM_LOG_ERROR("VENDOR: TMP461 unknow limit type!");
            break;
    }

    rv = i2c->writeb(bus, addr, limitAddr_high, (uint8_t) temperature);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;

    rv = i2c->writeb(bus, addr, limitAddr_low, 0);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;

    return 0;
}

static thermal_dev_driver_t tmp461_functions =
{
    tmp461_temp_get,
    tmp461_limit_get,
    tmp461_limit_set
};

static int tmp461_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "TMP461", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &tmp461_functions;

    return vendor_driver_add(driver);
}
/*THERMAL DEVICE TMP461 END*/

/*PSU DEVICE PMBUS START*/

typedef enum
{
    PMBUS_VOUT_MODE         = 0x20,
    PMBUS_FAN_CONFIG_12     = 0x3A,
    PMBUS_FAN_CMD_1         = 0x3B,
    PMBUS_FAN_CMD_2         = 0x3C,
    PMBUS_FAN_CONFIG_34     = 0x3D,
    PMBUS_FAN_CMD_3         = 0x3E,
    PMBUS_FAN_CMD_4         = 0x3F,
    PMBUS_READ_FAN_SPEED_1  = 0x90,
    PMBUS_READ_FAN_SPEED_2  = 0x91,
    PMBUS_READ_FAN_SPEED_3  = 0x92,
    PMBUS_READ_FAN_SPEED_4  = 0x93,
    PMBUS_AC_VOLTAGE_IN     = 0x88,
    PMBUS_AC_AMPERE_IN      = 0x89,
    PMBUS_DC_VOLTAGE_OUT    = 0x8B,
    PMBUS_DC_AMPERE_OUT     = 0x8C,
    PMBUS_READ_TEMP_1       = 0x8D,
    PMBUS_READ_TEMP_2       = 0x8E,
    PMBUS_READ_TEMP_3       = 0x8F,
    PMBUS_DC_WATT_OUT       = 0x96,
    PMBUS_AC_WATT_IN        = 0x97,
    PMBUS_MODEL_NAME        = 0x9A,
    PMBUS_VERSION           = 0x9B,
    PMBUS_SERIAL_NUMBER     = 0x9E,
}pmbus_command_t;

typedef enum
{
    LINEAR_DATA_FORMAT,
    DIRECT_DATA_FORMAT,
    FAN_SPEED_DATA_FORMAT
}pmbus_algorithm_t;

typedef struct
{
    pmbus_command_t command;
    pmbus_algorithm_t algorithm;
    uint16_t dataLen;
}pmbus_command_info_t;

static pmbus_command_info_t pmbus_command_info[] =
{
    {PMBUS_VOUT_MODE,           DIRECT_DATA_FORMAT,     1},
    {PMBUS_FAN_CONFIG_12,       LINEAR_DATA_FORMAT,     1},
    {PMBUS_FAN_CMD_1,           LINEAR_DATA_FORMAT,     2},
    {PMBUS_FAN_CMD_2,           LINEAR_DATA_FORMAT,     2},
    {PMBUS_FAN_CONFIG_34,       LINEAR_DATA_FORMAT,     1},
    {PMBUS_FAN_CMD_3,           LINEAR_DATA_FORMAT,     2},
    {PMBUS_FAN_CMD_4,           LINEAR_DATA_FORMAT,     2},
    {PMBUS_READ_FAN_SPEED_1,    FAN_SPEED_DATA_FORMAT,  2},
    {PMBUS_READ_FAN_SPEED_2,    FAN_SPEED_DATA_FORMAT,  2},
    {PMBUS_READ_FAN_SPEED_3,    FAN_SPEED_DATA_FORMAT,  2},
    {PMBUS_READ_FAN_SPEED_4,    FAN_SPEED_DATA_FORMAT,  2},
    {PMBUS_AC_VOLTAGE_IN,       LINEAR_DATA_FORMAT,     2},
    {PMBUS_AC_AMPERE_IN,        LINEAR_DATA_FORMAT,     2},
    {PMBUS_DC_VOLTAGE_OUT,      DIRECT_DATA_FORMAT,     2},
    {PMBUS_DC_AMPERE_OUT,       LINEAR_DATA_FORMAT,     2},
    {PMBUS_READ_TEMP_1,         LINEAR_DATA_FORMAT,     2},
    {PMBUS_READ_TEMP_2,         LINEAR_DATA_FORMAT,     2},
    {PMBUS_READ_TEMP_3,         LINEAR_DATA_FORMAT,     2},
    {PMBUS_DC_WATT_OUT,         LINEAR_DATA_FORMAT,     2},
    {PMBUS_AC_WATT_IN,          LINEAR_DATA_FORMAT,     2},
    {PMBUS_MODEL_NAME,          LINEAR_DATA_FORMAT,     3},
    {PMBUS_VERSION,             LINEAR_DATA_FORMAT,     3},
    {PMBUS_SERIAL_NUMBER,       LINEAR_DATA_FORMAT,     3}
};

static int get_pmbus_algorithm_result(pmbus_algorithm_t algorithm, uint16_t *data)
{
    switch (algorithm)
    {
        case FAN_SPEED_DATA_FORMAT:
            return (*data & 0x7ff) * pow(2, ( (*data >> 11) ) & 0x1f);

        case LINEAR_DATA_FORMAT:
            return (*data & 0x7ff) * 1000 / pow(2, (~(*data >> 11) + 1) & 0x1f);

        case DIRECT_DATA_FORMAT:
            return data[0] * 1000 / pow(2, (~data[1] + 1) & 0x1f);

        default:
            AIM_LOG_ERROR("VENDOR: Cannot find algorithm!");
            return -1;
    }

    return 0;
}

static pmbus_command_info_t *get_pmbus_command_info(pmbus_command_t command)
{
    int i, cnt = sizeof(pmbus_command_info) / sizeof(pmbus_command_info_t);

    for (i = 0; i < cnt; i++)
    {
        if (pmbus_command_info[i].command == command) {
            return &pmbus_command_info[i];
        }
    }

    return NULL;
}

static int pmbus_command_data_get(
    i2c_bus_driver_t *i2c,
    int bus,
    uint8_t addr,
    pmbus_command_info_t *info,
    uint16_t *pmbusData)
{
    int rv = 0;

    if(info == NULL) return ONLP_STATUS_E_INTERNAL;
    if(info->dataLen == 1)
    {
        rv = i2c->readb(bus, addr, info->command);
        if(rv < 0) return ONLP_STATUS_E_INTERNAL;
        *pmbusData = (uint8_t) rv;
        return 0;
    }
    else if(info->dataLen == 2)
    {
        rv = i2c->readw(bus, addr, info->command);
        if(rv < 0) return ONLP_STATUS_E_INTERNAL;
        *pmbusData = (uint16_t) rv;
        return 0;
    }

    return -1;
}

static int pmbus_command_string_get(
    i2c_bus_driver_t *i2c,
    int bus,
    uint8_t addr,
    pmbus_command_info_t *info,
    char *string)
{
    int rv = 0;
    uint16_t len;

    if(info == NULL) return ONLP_STATUS_E_INTERNAL;

    rv = i2c->readb(bus, addr, info->command);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;
    len = (uint16_t) rv;

    rv = i2c->block_read(bus, addr, info->command, len, (uint8_t *) string);

    return rv;
}

static int pmbus_model_get(void *busDrvPtr, int bus, uint8_t addr, char *model)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    pmbus_command_info_t *info = get_pmbus_command_info(PMBUS_MODEL_NAME);
    rv = pmbus_command_string_get(i2c, bus, addr, info, model);
    return rv;
}

static int pmbus_serial_get(void *busDrvPtr, int bus, uint8_t addr, char *serial)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    pmbus_command_info_t *info = get_pmbus_command_info(PMBUS_SERIAL_NUMBER);
    rv = pmbus_command_string_get(i2c, bus, addr, info, serial);
    return rv;
}

static int pmbus_volt_get(void *busDrvPtr, int bus, uint8_t addr, int *volt)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;

    uint16_t pmbus_data[2] = {0};
    pmbus_command_info_t *info = NULL;
    info = get_pmbus_command_info(PMBUS_DC_VOLTAGE_OUT);
    rv = pmbus_command_data_get(i2c, bus, addr, info, &pmbus_data[0]);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;

    info = get_pmbus_command_info(PMBUS_VOUT_MODE);
    rv = pmbus_command_data_get(i2c, bus, addr, info, &pmbus_data[1]);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;

    *volt = get_pmbus_algorithm_result(info->algorithm, pmbus_data);

    return rv;
}

static int pmbus_amp_get(void *busDrvPtr, int bus, uint8_t addr, int *amp)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;

    uint16_t pmbus_data = 0;
    pmbus_command_info_t *info = get_pmbus_command_info(PMBUS_DC_AMPERE_OUT);
    rv = pmbus_command_data_get(i2c, bus, addr, info, &pmbus_data);
    if(rv != 0) return ONLP_STATUS_E_INTERNAL;

    *amp = get_pmbus_algorithm_result(info->algorithm, &pmbus_data);

    return rv;
}

static int pmbus_watt_get(void *busDrvPtr, int bus, uint8_t addr, int *watt)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;

    uint16_t pmbus_data = 0;
    pmbus_command_info_t *info = get_pmbus_command_info(PMBUS_DC_WATT_OUT);
    rv = pmbus_command_data_get(i2c, bus, addr, info, &pmbus_data);
    if(rv != 0) return ONLP_STATUS_E_INTERNAL;

    *watt = get_pmbus_algorithm_result(info->algorithm, &pmbus_data);

    return rv;
}

static int pmbus_fan_rpm_get(void *busDrvPtr, int bus, uint8_t addr, int id, int *rpm)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;

    uint16_t pmbus_data = 0;
    pmbus_command_info_t *info = get_pmbus_command_info(PMBUS_READ_FAN_SPEED_1);
    rv = pmbus_command_data_get(i2c, bus, addr, info, &pmbus_data);
    if(rv != 0) return ONLP_STATUS_E_INTERNAL;

    *rpm = get_pmbus_algorithm_result(info->algorithm, &pmbus_data);

    return rv;
}

static int pmbus_fan_rpm_set(void *busDrvPtr, int bus, uint8_t addr, int id, int rpm)
{
    AIM_LOG_ERROR("VENDOR: It is not supported in PMBUS.");
    return ONLP_STATUS_E_INTERNAL;
}

static psu_dev_driver_t pmbus_psu_functions =
{
    pmbus_model_get,
    pmbus_serial_get,
    pmbus_volt_get,
    pmbus_amp_get,
    pmbus_watt_get
};

static fan_dev_driver_t pmbus_fan_functions =
{
    pmbus_fan_rpm_get,
    pmbus_fan_rpm_set,
};

static int pmbus_psu_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "PMBUS_PSU", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &pmbus_psu_functions;

    return vendor_driver_add(driver);
}

static int pmbus_fan_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "PMBUS_FAN", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &pmbus_fan_functions;

    return vendor_driver_add(driver);
}

/*PSU DEVICE PMBUS END*/

/*SFP and QSFP CONTROL DEFINE*/

typedef enum
{
    /* Shared QSFP and SFP fields: */
    SFF_FIELD_MIN,
    IDENTIFIER, /* Type of Transceiver */
    STATUS,     /* Support flags for upper pages */
    CHANNEL_LOS_INDICATOR, /* TX and RX LOS */
    CHANNEL_TX_FAULT,
    TEMPERATURE_ALARMS,
    VCC_ALARMS, /* Voltage */
    CHANNEL_RX_PWR_ALARMS,
    CHANNEL_TX_BIAS_ALARMS,
    TEMPERATURE,
    VCC, /* Voltage */
    CHANNEL_RX_PWR,
    CHANNEL_TX_PWR,
    CHANNEL_TX_BIAS,
    CHANNEL_TX_DISABLE,
    POWER_CONTROL,
    PAGE_SELECT_BYTE, /*SFF8436/SFF8636*/

    DIAGNOSTIC_MONITORING_TYPE,   /* SFF8472 Diagnostic monitoring implemented */
    EXTERNAL_CALIBRATION,    /* SFF8472 diagnostic calibration constants */
    SFF_FIELD_MAX /* keep this the last */

}sff_field_t;

/*SFP DEVICE SFF8636 START*/

#define SFF8636_PAGE_CONTROL 127
#define SFF8636_UPPER_PAGE_START_OFFSET 128
#define SFP_EEPROM_SIZE 256
#define SFP_EEPROM_PAGE_SIZE 128
#define CONTROL_IS_SUPPORT 1
#define CONTROL_IS_NOT_SUPPORT 0

typedef enum
{
    SFF8636_EEPROM_BASE_PAGE,
    SFF8636_EEPROM_PAGE0 = 0,
}sff8636_page_t;

typedef struct
{
    sff_field_t field;
    uint8_t page;
    uint16_t offset;
    uint16_t length;
}sff8636_field_map_t;

static uint8_t get_sfp_flat_mem(i2c_bus_driver_t *i2c, int bus, uint8_t addr);
static uint8_t filter_sff8636_control_value_for_get(int control, uint8_t *eeprom_data, int *combined_data);
static uint8_t filter_sff8636_control_value_for_set(int control, uint8_t *eeprom_data, int *write_status);
static sff_field_t get_sff8636_control_field(int control);
static int eeprom_read_lowermap(i2c_bus_driver_t *i2c, int bus, uint8_t addr, uint8_t *data);
static int eeprom_uppermap_change_page(i2c_bus_driver_t *i2c, int bus, uint8_t addr, uint8_t page);
static int eeprom_read_uppermap_by_page(i2c_bus_driver_t *i2c, int bus, uint8_t addr, uint8_t page, uint8_t *data);
static int get_sff8636_field(sff_field_t field, sff8636_field_map_t **info);
static int get_sff8636_value_from_eeprom(i2c_bus_driver_t *i2c, int bus, uint8_t addr, sff8636_field_map_t *info, uint8_t *value);
static int set_sff8636_value_to_eeprom(i2c_bus_driver_t *i2c, int bus, uint8_t addr, sff8636_field_map_t *info, uint8_t *value);

static sff8636_field_map_t sff8636_field_map[] = {
    {IDENTIFIER, SFF8636_EEPROM_BASE_PAGE, 0, 1},
    {STATUS, SFF8636_EEPROM_BASE_PAGE, 1, 2},
    {CHANNEL_LOS_INDICATOR, SFF8636_EEPROM_BASE_PAGE, 3, 1},
    {CHANNEL_TX_FAULT, SFF8636_EEPROM_BASE_PAGE, 4, 1},
    {VCC_ALARMS, SFF8636_EEPROM_BASE_PAGE, 7, 1},
    {CHANNEL_RX_PWR_ALARMS, SFF8636_EEPROM_BASE_PAGE, 9, 2},
    {CHANNEL_TX_BIAS_ALARMS, SFF8636_EEPROM_BASE_PAGE, 11, 2},
    {TEMPERATURE, SFF8636_EEPROM_BASE_PAGE, 22, 2},
    {VCC, SFF8636_EEPROM_BASE_PAGE, 26, 2},
    {CHANNEL_RX_PWR, SFF8636_EEPROM_BASE_PAGE, 34, 8},
    {CHANNEL_TX_BIAS, SFF8636_EEPROM_BASE_PAGE, 42, 8},
    {CHANNEL_TX_PWR, SFF8636_EEPROM_BASE_PAGE, 50, 8},
    {CHANNEL_TX_DISABLE, SFF8636_EEPROM_BASE_PAGE, 86, 1},
    {POWER_CONTROL, SFF8636_EEPROM_BASE_PAGE, 93, 1},
    {PAGE_SELECT_BYTE, SFF8636_EEPROM_BASE_PAGE, 127, 1},
};

static int sff8636_eeprom_load(void *busDrvPtr, int bus, uint8_t addr, uint8_t *data)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;

    rv = eeprom_read_lowermap(i2c, bus, addr, data);
    rv = eeprom_read_uppermap_by_page(i2c, bus, addr, SFF8636_EEPROM_PAGE0, data+SFP_EEPROM_PAGE_SIZE);

    return rv;
}

static int sff8636_eeprom_readb(void *busDrvPtr, int bus, uint8_t addr, uint16_t offset, uint8_t *data)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;

    rv = eeprom_readb(i2c, bus, addr, offset, 1, data);

    return rv;
}

static int sff8636_eeprom_writeb(void *busDrvPtr, int bus, uint8_t addr, uint16_t offset, uint8_t data)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;

    rv = eeprom_writeb(i2c, bus, addr, offset, 1, &data);

    return rv;
}

static int sff8636_control_is_support(int control, uint8_t *is_support)
{
    switch (control)
    {
        //case SFP_CONTROL_RESET:
        case SFP_CONTROL_RESET_STATE:
        case SFP_CONTROL_LP_MODE:
        case SFP_CONTROL_RX_LOS:
        case SFP_CONTROL_TX_FAULT:
        case SFP_CONTROL_TX_DISABLE:
        //case SFP_CONTROL_TX_DISABLE_CHANNEL:
        //case SFP_CONTROL_POWER_OVERRIDE:
            *is_support = CONTROL_IS_SUPPORT;
            break;
        default:
            *is_support = CONTROL_IS_NOT_SUPPORT;
            break;
    }

    return 0;
}

static int sff8636_control_get(
    void *busDrvPtr, int bus, uint8_t addr,
    int control,
    int *status)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    uint8_t *eeprom_data;
    sff_field_t field = get_sff8636_control_field(control);
    if(field == SFF_FIELD_MIN)
    {
        AIM_LOG_ERROR("VENDOR: The control type is not supported in SFF8636");
        return -1;
    }
    sff8636_field_map_t *info = (sff8636_field_map_t *)calloc(1, sizeof(sff8636_field_map_t));

    /* get the address of the control */
    rv = get_sff8636_field(field, &info);
    if(rv != 0) return rv;
    eeprom_data = (uint8_t *)calloc(1, sizeof(uint8_t) * info->length);

    /* get the control value from eeprom */
    rv = get_sff8636_value_from_eeprom(i2c, bus, addr, info, eeprom_data);
    if(rv != 0) return rv;
    /* filter the right date which user need */
    if(filter_sff8636_control_value_for_get(control, eeprom_data, status) != CONTROL_IS_SUPPORT)
    {
        AIM_LOG_ERROR("VENDOR: The control type get status error in SFF8636");
        return -1;
    }
    free(eeprom_data);
    eeprom_data = NULL;
    return rv;
}

static int sff8636_control_set(
    void *busDrvPtr, int bus, uint8_t addr,
    int control,
    int status)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    uint8_t *eeprom_data;
    uint8_t value;

    if(control == SFP_CONTROL_RX_LOS || control == SFP_CONTROL_TX_FAULT)
    {
        AIM_LOG_ERROR("VENDOR: This control type is not writable in SFF8636");
        return -1;
    }

    sff_field_t field = get_sff8636_control_field(control);
    if(field == SFF_FIELD_MIN)
    {
        AIM_LOG_ERROR("VENDOR: This control type is not supported in SFF8636");
        return -1;
    }

    sff8636_field_map_t *info = (sff8636_field_map_t *)calloc(1, sizeof(sff8636_field_map_t));

    /* get the address of the control */
    rv = get_sff8636_field(field, &info);
    if(rv != 0) return rv;

    eeprom_data = (uint8_t *)calloc(1, sizeof(uint8_t) * info->length);
    /* get the control value from eeprom */
    rv = get_sff8636_value_from_eeprom(i2c, bus, addr, info, eeprom_data);

    /* value check */
    rv = filter_sff8636_control_value_for_set(control, eeprom_data, &status);
    if(rv != 0) return rv;

    /* set the control value to eeprom */
    value = (uint8_t)status;
    rv = set_sff8636_value_to_eeprom(i2c, bus, addr, info, &value);

    free(eeprom_data);
    eeprom_data = NULL;

    return rv;
}

/*================ internal function ================*/

static int get_sff8636_field(sff_field_t field, sff8636_field_map_t **info) {

    int i, cnt = sizeof(sff8636_field_map) / sizeof(sff8636_field_map_t);

    if (field >= SFF_FIELD_MAX) {
        return -1;
    }

    for (i = 0; i < cnt; i++) {
        if (sff8636_field_map[i].field == field) {
            *info = &(sff8636_field_map[i]);
        }
    }

    if (!info)
    {
        AIM_LOG_ERROR("VENDOR: Cannot find sff8636 field!");
        return -1;
    }

    return 0;
}

static uint8_t get_sfp_flat_mem(i2c_bus_driver_t *i2c, int bus, uint8_t addr)
{
    int rv = 0;
    uint8_t status[2];
    uint8_t flat_mem = 0;
    sff8636_field_map_t *info = (sff8636_field_map_t *)calloc(1, sizeof(sff8636_field_map_t));

    rv = get_sff8636_field(STATUS, &info);
    if(rv != 0) return rv;
    rv = eeprom_readb(i2c, bus, addr, info->offset, info->length, status);
    if(rv != 0) return rv;

    flat_mem = status[1] & (1 << 2);

    return flat_mem;
}

static int eeprom_read_lowermap(i2c_bus_driver_t *i2c, int bus, uint8_t addr, uint8_t *data)
{
    int rv = 0;
    rv = smbus_block_read(bus, addr, 0, SFP_EEPROM_PAGE_SIZE, data);

    return rv;
}

static int eeprom_uppermap_change_page(i2c_bus_driver_t *i2c, int bus, uint8_t addr, uint8_t page)
{
    int rv = 0;
    if(!get_sfp_flat_mem(i2c, bus, addr)){
        rv = eeprom_writeb(i2c, bus, addr, SFF8636_PAGE_CONTROL, 1, &page);
    }

    return rv;
}

static int eeprom_read_uppermap_by_page(i2c_bus_driver_t *i2c, int bus, uint8_t addr, uint8_t page, uint8_t *data)
{
    int rv = 0;

    rv = eeprom_uppermap_change_page(i2c, bus, addr, page);
    rv = smbus_block_read(bus, addr, SFF8636_UPPER_PAGE_START_OFFSET, SFP_EEPROM_PAGE_SIZE, data);

    return rv;
}

static int get_sff8636_value_from_eeprom(i2c_bus_driver_t *i2c, int bus, uint8_t addr, sff8636_field_map_t *info, uint8_t *value)
{
    int rv = 0;

    rv = eeprom_uppermap_change_page(i2c, bus, addr, info->page);
    if(rv != 0) return rv;

    rv = eeprom_readb(i2c, bus, addr, info->offset, info->length, value);
    if(rv != 0) return rv;

    return rv;
}

static int set_sff8636_value_to_eeprom(i2c_bus_driver_t *i2c, int bus, uint8_t addr, sff8636_field_map_t *info, uint8_t *value)
{
    int rv = 0;

    rv = eeprom_uppermap_change_page(i2c, bus, addr, info->page);
    if(rv != 0) return rv;

    rv = eeprom_writeb(i2c, bus, addr, info->offset, info->length, value);
    if(rv != 0) return rv;

    return rv;
}

static sff_field_t get_sff8636_control_field(int control)
{
    switch (control)
    {
        case SFP_CONTROL_RX_LOS:
            return CHANNEL_LOS_INDICATOR;

        case SFP_CONTROL_TX_FAULT:
            return CHANNEL_TX_FAULT;

        case SFP_CONTROL_TX_DISABLE:
        case SFP_CONTROL_TX_DISABLE_CHANNEL:
            return CHANNEL_TX_DISABLE;

        case SFP_CONTROL_POWER_OVERRIDE:
            return POWER_CONTROL;

        case SFP_CONTROL_TEMPERATURE:
            return TEMPERATURE;

        case SFP_CONTROL_VOLTAGE:
            return VCC;

        case SFP_CONTROL_TX_BIAS:
            return CHANNEL_TX_BIAS;

        case SFP_CONTROL_RX_POWER:
            return CHANNEL_RX_PWR;

        case SFP_CONTROL_TX_POWER:
            return CHANNEL_TX_PWR;

        default:
            return SFF_FIELD_MIN;
    }
}

/* SFP_CONTROL_RESET */                 /* SUPPORT_BY_CPLD */
/* SFP_CONTROL_RESET_STATE */           /* SUPPORT_BY_CPLD */
/* SFP_CONTROL_RX_LOS */                /* R 3.0 - 3.3 */
/* SFP_CONTROL_TX_FAULT */              /* R 4.0 - 4.3 */
/* SFP_CONTROL_TX_DISABLE */            /* RW 86.0 - 86.3 */
/* SFP_CONTROL_TX_DISABLE_CHANNEL */    /* RW 86.0 - 86.3 */
/* SFP_CONTROL_LP_MODE */               /* SUPPORT_BY_CPLD */
/* SFP_CONTROL_POWER_OVERRIDE */        /* RW 93.0 POWER_OVERRIDE
                                         * RW 93.1 POWER_SET
                                         */

static uint8_t filter_sff8636_control_value_for_get(
    int control,
    uint8_t *eeprom_data, int *combined_data)
{
    int i = 0, j = 0;
    uint8_t channel = 4;

    switch (control)
    {
        case SFP_CONTROL_TX_DISABLE:
        case SFP_CONTROL_TX_DISABLE_CHANNEL:
            *combined_data = *eeprom_data & 0xf;
            break;

        case SFP_CONTROL_TX_FAULT:
        case SFP_CONTROL_RX_LOS:
            for(i = 0; i < channel; i++)
            {
                *(combined_data + i) = (*eeprom_data & (0x01 << i)) >> i ;
            }
            break;

        case SFP_CONTROL_POWER_OVERRIDE:
            *combined_data = *eeprom_data & 0x3;
            break;

        case SFP_CONTROL_TEMPERATURE:/* Unit: Celsius */
            *combined_data = (eeprom_data[0] << 8 | eeprom_data[1]) / 256;
            break;

        case SFP_CONTROL_VOLTAGE:/* Unit: mV */
            *combined_data = (eeprom_data[0] << 8 | eeprom_data[1])  / 10;
            break;

        case SFP_CONTROL_TX_BIAS:/* Unit: uA */
            for(i = 0, j = 0 ; i < channel; i++, j=j+2)
            {
                *(combined_data + i) = (eeprom_data[j] << 8 | eeprom_data[j+1]) *2 ;
            }
            break;

        case SFP_CONTROL_RX_POWER:/* Unit: uW */
            for(i = 0, j = 0; i < channel; i++, j=j+2)
            {
                *(combined_data + i) = (eeprom_data[j] << 8 | eeprom_data[j+1]) / 10;
            }
            break;

        case SFP_CONTROL_TX_POWER:/* Unit: uW */
            for(i = 0, j = 0; i < channel; i++, j=j+2)
            {
                *(combined_data + i) = (eeprom_data[j] << 8 | eeprom_data[j+1]) / 10;
            }
            break;

        default:
            return CONTROL_IS_NOT_SUPPORT;
    }

    return CONTROL_IS_SUPPORT;
}

#define SFP_CONTROL_POWER_OVERRIDE_MASK       0x3
#define SFP_CONTROL_TX_DISABLE_CHANNEL_MASK   0xF

static uint8_t filter_sff8636_control_value_for_set(
    int control,
    uint8_t *eeprom_data,
    int *write_status)
{
    int data;
    int status=*write_status;
    uint8_t curr_status = *eeprom_data;
    switch (control)
    {
        case SFP_CONTROL_TX_DISABLE:

            if(status == 0 )
            {
                /*  clean mask bit value of current status */
                data = curr_status & (~SFP_CONTROL_TX_DISABLE_CHANNEL_MASK);
                *write_status = data;
                return 0;
            }
            else if(status == 1 )
            {
                /*  clean mask bit of current status */
                data = curr_status & (~SFP_CONTROL_TX_DISABLE_CHANNEL_MASK);
                /*  calculate write data  */
                data |= SFP_CONTROL_TX_DISABLE_CHANNEL_MASK;
                *write_status = data;
                return 0;
            }
            else
            {
                AIM_LOG_ERROR("VENDOR:\t[SFF8636] Status of TX disable should be 0 or 1 ");
                return -1;
            }

        case SFP_CONTROL_TX_DISABLE_CHANNEL:
            if(status >= 0 && status <= SFP_CONTROL_TX_DISABLE_CHANNEL_MASK)
            {
                /*  clean mask bit of current status */
                data = curr_status & (~SFP_CONTROL_TX_DISABLE_CHANNEL_MASK);
                /*  calculate write data  */
                data |= (status & SFP_CONTROL_TX_DISABLE_CHANNEL_MASK);
                *write_status = data;
                return 0;
            }
            else
            {
                AIM_LOG_ERROR("VENDOR:\t[SFF8636] Status of TX disable channel should be 0 to 0xF ");
                return -1;
            }

        case SFP_CONTROL_POWER_OVERRIDE:
            if(status >= 0 && status <= SFP_CONTROL_POWER_OVERRIDE_MASK)
            {
                /*  clean mask bit of current status */
                data = curr_status & (~SFP_CONTROL_POWER_OVERRIDE_MASK);
                /*  calculate write data  */
                data |= (status & SFP_CONTROL_POWER_OVERRIDE_MASK);
                *write_status = data;
                return 0;
            }
            else
            {
                AIM_LOG_ERROR("VENDOR:\t[SFF8636] Status of TX disable channel should be 0 to 0x3 ");
                return -1;
            }
        default:
            return CONTROL_IS_NOT_SUPPORT;
    }
}

static sfp_dev_driver_t sff8636_functions =
{
    sff8636_eeprom_load,
    sff8636_eeprom_readb,
    sff8636_eeprom_writeb,
    sff8636_control_is_support,
    sff8636_control_get,
    sff8636_control_set
};

static int sff8636_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "SFF8636", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &sff8636_functions;
    vendor_driver_add(driver);

    driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));
    strncpy(driver->name, "SFF8436", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &sff8636_functions;
    vendor_driver_add(driver);

    return 0;
}
/*SFP DEVICE SFF8636 END*/

/*SFP DEVICE SFF8472 START*/

typedef enum
{
    SFF8472_EEPROM_ADDRESS_A0H = 0x50,
    SFF8472_EEPROM_ADDRESS_A2H = 0x51,

}sff8472_address_t;

typedef struct sff8472_field_map
{
    sff_field_t field;
    uint8_t address;
    uint16_t offset;
    uint16_t length;
}sff8472_field_map_t;

static uint8_t filter_sff8472_control_value_for_get(int control, uint8_t *eeprom_data, int *combined_data);
static uint8_t filter_sff8472_control_value_for_set(int control, uint8_t *eeprom_data, int *write_status);
static sff_field_t get_sff8472_control_field(int control);
static int get_sff8472_field(sff_field_t field, sff8472_field_map_t **info);
static int get_sff8472_value_from_eeprom(i2c_bus_driver_t *i2c, int bus, uint8_t addr, sff8472_field_map_t *info, uint8_t *value);
static int set_sff8472_value_to_eeprom(i2c_bus_driver_t *i2c, int bus, uint8_t addr, sff8472_field_map_t *info, uint8_t *value);
static int check_sff8472_diagnostic_monitoring_type(void *busDrvPtr, int bus, uint8_t addr, uint8_t *type);

static sff8472_field_map_t sff8472_field_map[] = {
    {IDENTIFIER, SFF8472_EEPROM_ADDRESS_A0H, 0, 1},
    {DIAGNOSTIC_MONITORING_TYPE, SFF8472_EEPROM_ADDRESS_A0H, 92, 1},
    {EXTERNAL_CALIBRATION, SFF8472_EEPROM_ADDRESS_A2H, 56, 36},
    {TEMPERATURE, SFF8472_EEPROM_ADDRESS_A2H, 96, 2},
    {VCC, SFF8472_EEPROM_ADDRESS_A2H, 98, 2},
    {CHANNEL_TX_BIAS, SFF8472_EEPROM_ADDRESS_A2H, 100, 2},
    {CHANNEL_TX_PWR, SFF8472_EEPROM_ADDRESS_A2H, 102, 2},
    {CHANNEL_RX_PWR, SFF8472_EEPROM_ADDRESS_A2H, 104, 2},
    {CHANNEL_LOS_INDICATOR, SFF8472_EEPROM_ADDRESS_A2H, 110, 1},
    {CHANNEL_TX_FAULT, SFF8472_EEPROM_ADDRESS_A2H, 110, 1},
    {CHANNEL_TX_DISABLE, SFF8472_EEPROM_ADDRESS_A2H, 110, 1},
};

static int sff8472_eeprom_load(void *busDrvPtr, int bus, uint8_t addr, uint8_t *data)
{
    int rv = 0;

    rv = smbus_block_read(bus, SFF8472_EEPROM_ADDRESS_A0H, 0, SFP_EEPROM_SIZE, data);

    return rv;
}

/*static int sff8472_dom_eeprom_load(i2c_bus_driver_t *i2c, int bus, uint8_t addr, uint8_t *data)
{
    int rv = 0;

    rv = smbus_block_read(bus, SFF8472_EEPROM_ADDRESS_A2H, 0, SFP_EEPROM_SIZE, data);

    return rv;
}
*/
static int sff8472_eeprom_readb(void *busDrvPtr, int bus, uint8_t addr, uint16_t offset, uint8_t *data)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;

    rv = eeprom_readb(i2c, bus, addr, offset, 1, data);

    return rv;
}

static int sff8472_eeprom_writeb(void *busDrvPtr, int bus, uint8_t addr, uint16_t offset, uint8_t data)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;

    rv = eeprom_writeb(i2c, bus, addr, offset, 1, &data);

    return rv;
}

static int sff8472_control_is_support(int control, uint8_t *is_support)
{
    switch (control)
    {
        case SFP_CONTROL_RX_LOS:
        case SFP_CONTROL_TX_FAULT:
        case SFP_CONTROL_TX_DISABLE:
        case SFP_CONTROL_TEMPERATURE:
        case SFP_CONTROL_VOLTAGE:
        case SFP_CONTROL_TX_BIAS:
        case SFP_CONTROL_RX_POWER:
        case SFP_CONTROL_TX_POWER:
            *is_support = CONTROL_IS_SUPPORT;
            break;
        case SFP_CONTROL_RESET:
        case SFP_CONTROL_RESET_STATE:
        case SFP_CONTROL_LP_MODE:
        case SFP_CONTROL_TX_DISABLE_CHANNEL:
        case SFP_CONTROL_POWER_OVERRIDE:
            *is_support = CONTROL_IS_NOT_SUPPORT;
            break;
        default:
            *is_support = CONTROL_IS_NOT_SUPPORT;
            break;
    }

    return 0;
}

static int check_sff8472_diagnostic_monitoring_type(void *busDrvPtr, int bus, uint8_t addr, uint8_t *type)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    uint8_t data;

    sff8472_field_map_t *info = (sff8472_field_map_t *)calloc(1, sizeof(sff8472_field_map_t));

    rv = get_sff8472_field(DIAGNOSTIC_MONITORING_TYPE, &info);
    if(rv != 0) return rv;

    rv = eeprom_readb(i2c, bus, addr, info->offset, info->length, &data);
    if(rv != 0) return rv;

    *type = (data >> 5) & 0x01;

    return rv;
}

static int sff8472_control_get(
    void *busDrvPtr, int bus, uint8_t addr,
    int control,
    int *status)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    uint8_t *eeprom_data;
    uint8_t type;
    sff_field_t field = get_sff8472_control_field(control);
    if(field == SFF_FIELD_MIN)
    {
        AIM_LOG_ERROR("VENDOR: The control type is not supported in SFF8472");
        return -1;
    }

    sff8472_field_map_t *info = (sff8472_field_map_t *)calloc(1, sizeof(sff8472_field_map_t));

    rv = get_sff8472_field(field, &info);
    if(rv != 0) return rv;

    eeprom_data = (uint8_t *)calloc(1, sizeof(uint8_t) * info->length);


    /* get the control value from eeprom */
    rv = get_sff8472_value_from_eeprom(i2c, bus, addr, info, eeprom_data);
    if(rv != 0) return rv;

    if(check_sff8472_diagnostic_monitoring_type(i2c, bus, addr, &type) == 0)
    {

        if(!type)
        {
            /*TODO*/
            AIM_LOG_ERROR("VENDOR: External calibration not yet supported.");
            return -1;

        }
        else
        {
            /* filter the right date which user need */
            if(filter_sff8472_control_value_for_get(control, eeprom_data, status) != CONTROL_IS_SUPPORT)
            {
                AIM_LOG_ERROR("VENDOR: The control type get status error in SFF8472");
                return -1;
            }
        }
    }
    else
    {
        AIM_LOG_ERROR("VENDOR: The control type get diagnostic monitoring type error in SFF8472");
        return -1;

    }

    free(eeprom_data);
    eeprom_data = NULL;

    return rv;
}

static int sff8472_control_set(
    void *busDrvPtr, int bus, uint8_t addr,
    int control,
    int status)
{
    i2c_bus_driver_t *i2c = (i2c_bus_driver_t *)busDrvPtr;
    int rv = 0;
    uint8_t *eeprom_data;
    uint8_t value;

    if(control != SFP_CONTROL_TX_DISABLE)
    {
        AIM_LOG_ERROR("VENDOR: This control type is not writable in SFF8472");
        return -1;
    }

    sff_field_t field = get_sff8472_control_field(control);
    if(field == SFF_FIELD_MIN)
    {
        AIM_LOG_ERROR("VENDOR: This control type is not supported in SFF8472");
        return -1;
    }

    sff8472_field_map_t *info = (sff8472_field_map_t *)calloc(1, sizeof(sff8472_field_map_t));

    rv = get_sff8472_field(field, &info);
    if(rv != 0) return rv;

    eeprom_data = (uint8_t *)calloc(1, sizeof(uint8_t) * info->length);
    /* get the control value from eeprom */
    rv = get_sff8472_value_from_eeprom(i2c, bus, addr, info, eeprom_data);

    /* value check */
    rv = filter_sff8472_control_value_for_set(control, eeprom_data, &status);
    if(rv != 0) return rv;

    /* set the control value to eeprom */
    value = (uint8_t)status;
    rv = set_sff8472_value_to_eeprom(i2c, bus, addr, info, &value);

    free(eeprom_data);
    eeprom_data = NULL;

    return rv;
}

/*================ internal function ================*/

static int get_sff8472_field(sff_field_t field, sff8472_field_map_t **info) {

    int i, cnt = sizeof(sff8472_field_map) / sizeof(sff8472_field_map_t);

    if (field >= SFF_FIELD_MAX) {
        return -1;
    }

    for (i = 0; i < cnt; i++) {
        if (sff8472_field_map[i].field == field) {
            *info = &(sff8472_field_map[i]);
        }
    }

    if (!info){
        AIM_LOG_ERROR("VENDOR: Cannot find sff8472 field!");
        return -1;
    }

    return 0;
}

static int get_sff8472_value_from_eeprom(i2c_bus_driver_t *i2c, int bus, uint8_t addr, sff8472_field_map_t *info, uint8_t *value)
{
    int rv = 0;

    rv = eeprom_readb(i2c, bus, info->address, info->offset, info->length, value);

    return rv;
}

static int set_sff8472_value_to_eeprom(i2c_bus_driver_t *i2c, int bus, uint8_t addr, sff8472_field_map_t *info, uint8_t *value)
{
    int rv = 0;

    rv = eeprom_writeb(i2c, bus, info->address, info->offset, info->length, value);

    return rv;
}

static sff_field_t get_sff8472_control_field(int control)
{
    switch (control)
    {
        case SFP_CONTROL_RX_LOS:
            return CHANNEL_LOS_INDICATOR;

        case SFP_CONTROL_TX_FAULT:
            return CHANNEL_TX_FAULT;

        case SFP_CONTROL_TX_DISABLE:
            return CHANNEL_TX_DISABLE;

        case SFP_CONTROL_TEMPERATURE:
            return TEMPERATURE;

        case SFP_CONTROL_VOLTAGE:
            return VCC;

        case SFP_CONTROL_TX_BIAS:
            return CHANNEL_TX_BIAS;

        case SFP_CONTROL_RX_POWER:
            return CHANNEL_RX_PWR;

        case SFP_CONTROL_TX_POWER:
            return CHANNEL_TX_PWR;

        default:
            return SFF_FIELD_MIN;
    }
}

/* SFP_CONTROL_RESET */ /* SUPPORT_BY_CPLD */
/* SFP_CONTROL_RESET_STATE */ /* SUPPORT_BY_CPLD */
/* SFP_CONTROL_RX_LOS */ /*R 3.0 - 3.3 */
/* SFP_CONTROL_TX_FAULT */ /*R 4.0 - 4.3 */
/* SFP_CONTROL_TX_DISABLE */ /*RW 86.0 - 86.3 */
/* SFP_CONTROL_TX_DISABLE_CHANNEL */
/* SFP_CONTROL_LP_MODE */ /* SUPPORT_BY_CPLD */
/* SFP_CONTROL_POWER_OVERRIDE, */ /*RW 93.0 */

static uint8_t filter_sff8472_control_value_for_get(
    int control,
    uint8_t *eeprom_data, int *combined_data)
{
    switch (control)
    {
        case SFP_CONTROL_TX_DISABLE:
            *combined_data = *eeprom_data & 0x40 ? 0x1 : 0x0;
            break;

        case SFP_CONTROL_RX_LOS:
            *combined_data = *eeprom_data & 0x02 ? 0x1 : 0x0;
            break;

        case SFP_CONTROL_TX_FAULT:
            *combined_data = *eeprom_data & 0x4 ? 0x1 : 0x0;
            break;

        case SFP_CONTROL_TEMPERATURE:/* Unit: Celsius */
            *combined_data = (eeprom_data[0] << 8 | eeprom_data[1]) / 256;
            break;

        case SFP_CONTROL_VOLTAGE:/* Unit: mV */
            *combined_data = (eeprom_data[0] << 8 | eeprom_data[1]) / 10;
            break;

        case SFP_CONTROL_TX_BIAS:/* Unit: uA */
            *combined_data = (eeprom_data[0] << 8 | eeprom_data[1]) * 2 ;
            break;

        case SFP_CONTROL_RX_POWER:/* Unit: uW */
            *combined_data = (eeprom_data[0] << 8 | eeprom_data[1]) / 10;
            break;

        case SFP_CONTROL_TX_POWER:/* Unit: uW */
                *combined_data = (eeprom_data[0] << 8 | eeprom_data[1]) / 10;
            break;

        default:
            return CONTROL_IS_NOT_SUPPORT;
    }

    return CONTROL_IS_SUPPORT;

}

static uint8_t filter_sff8472_control_value_for_set(
    int control,
    uint8_t *eeprom_data,
    int *write_status)
{
    int status=*write_status;
    uint8_t curr_status = *eeprom_data;
    switch (control)
    {
        case SFP_CONTROL_TX_DISABLE:
        if(status == 0 )
        {
            *write_status = curr_status & ~0x40;
             return 0;
        }
        else if(status == 1 )
        {
            *write_status = curr_status & ~0x40;
            *write_status |= 0x40;
             return 0;
        }
        else
        {
            AIM_LOG_ERROR("VENDOR:\t[SFF8472] Status of TX disable should be 0 or 1 ");
            return -1;
        }

        default:
            return CONTROL_IS_NOT_SUPPORT;
    }
}

static sfp_dev_driver_t sff8472_functions =
{
    sff8472_eeprom_load,
    sff8472_eeprom_readb,
    sff8472_eeprom_writeb,
    sff8472_control_is_support,
    sff8472_control_get,
    sff8472_control_set
};

static int sff8472_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "SFF8472", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &sff8472_functions;
    vendor_driver_add(driver);

    return 0;
}
/*SFP DEVICE SFF8472 END*/

/*BMC DEVICE START*/

typedef enum bmc_command
{
    BMC_CMD_PSU_GET_PRESENT = 0,
    BMC_CMD_PSU_GET_POWER_GOOD,
    BMC_CMD_PSU_GET_MODEL_NAME,
    BMC_CMD_PSU_GET_SERIAL_NUM,
    BMC_CMD_PSU_GET_VOLTAGE,
    BMC_CMD_PSU_GET_CURRENT,
    BMC_CMD_FAN_GET_PRESENT,
    BMC_CMD_FAN_GET_RPM,
    BMC_CMD_THERMAL_GET_TEMP,
    BMC_CMD_MAX
} bmc_command_t;

typedef enum bmc_command_type
{
    BMC_CMD_TYPE_NONE = 0,
    BMC_CMD_TYPE_STANDARD,
    BMC_CMD_TYPE_RAW
} bmc_command_type_t;

typedef struct bmc_ipmi_command_info_s
{
    bmc_command_t       command;
    bmc_command_type_t  command_type;
    char               *cmd;
    char               *filter;
} bmc_ipmi_command_info_t;

/*
 *  The command array MUST 1-to-1 map to bmc_command_t
 */
static bmc_ipmi_command_info_t bmc_ipmi_command_info[] =
{
    {
        BMC_CMD_PSU_GET_PRESENT,
        BMC_CMD_TYPE_STANDARD,
        "fru print %s 2>/dev/null",
        "|grep \"Product Name\" |cut -d : -f 2 |cut -d ' ' -f 2-"
    },
    {
        BMC_CMD_PSU_GET_POWER_GOOD,
        BMC_CMD_TYPE_NONE,
        NULL,
        NULL
    },
    {
        BMC_CMD_PSU_GET_MODEL_NAME,
        BMC_CMD_TYPE_STANDARD,
        "fru print %s",
        "|grep \"Product Name\" |cut -d : -f 2 |cut -d ' ' -f 2-"
    },
    {
        BMC_CMD_PSU_GET_SERIAL_NUM,
        BMC_CMD_TYPE_STANDARD,
        "fru print %s",
        "|grep \"Product Serial\" |cut -d : -f 2 |cut -d ' ' -f 2-"
    },
    {
        BMC_CMD_PSU_GET_VOLTAGE,
        BMC_CMD_TYPE_RAW,
        "raw 0x3C 0xE0 0x11 %s 0x8B",
        ""
    },
    {
        BMC_CMD_PSU_GET_CURRENT,
        BMC_CMD_TYPE_RAW,
        "raw 0x3C 0xE0 0x11 %s 0x8C",
        ""
    },

    {
        BMC_CMD_FAN_GET_PRESENT,
        BMC_CMD_TYPE_RAW,
        "raw 0x3C 0xE0 0x05 %s 0x00",
        ""
    },
    {
        BMC_CMD_FAN_GET_RPM,
        BMC_CMD_TYPE_STANDARD,
        "sensor get %s",
        "|grep \"Sensor Reading\" |cut -d : -f 2 |cut -d ' ' -f 2"
    },

    {
        BMC_CMD_THERMAL_GET_TEMP,
        BMC_CMD_TYPE_STANDARD,
        "sensor get %s",
        "|grep \"Sensor Reading\" |cut -d : -f 2 |cut -d ' ' -f 2"
    }
};

static char *ipmi_psu_std_unit_id[] =
{
    "1",
    "2"
};

static char *ipmi_psu_raw_unit_id[] =
{
    "0",
    "1"
};

static char *ipmi_fan_std_unit_id[] =
{
    "FanPWM_0",
    "FanPWM_1",
    "FanPWM_2",
    "FanPWM_3"

};

static char *ipmi_fan_raw_unit_id[] =
{
    "0",
    "1",
    "2",
    "3"
};

static char *ipmi_thrml_unit_id[] =
{
    "Temp_L0",
    "Temp_L1",
    "Temp_L2",
    "Temp_L3",
    "Temp_CPUB"
};

static int bmc_addr_get_device_type(uint8_t addr)
{
    return addr >> 6;
}

static int bmc_addr_get_id(uint8_t addr)
{
    return addr & 0x3f;
}

static bmc_ipmi_command_info_t *get_bmc_command_info(bmc_command_t command)
{
    int i;

    for (i = 0; i < BMC_CMD_MAX; i++) {
        if (bmc_ipmi_command_info[i].command == command)
            return &bmc_ipmi_command_info[i];
    }

    return NULL;
}

//OK
static char *get_bmc_command_unit_id(int deviceType, int unit, bmc_ipmi_command_info_t *info)
{
    char *unit_id_str = NULL;

    if (deviceType == VENDOR_PSU)
    {
        if (info->command_type == BMC_CMD_TYPE_STANDARD)
            unit_id_str = ipmi_psu_std_unit_id[unit];
        else if (info->command_type == BMC_CMD_TYPE_RAW)
            unit_id_str = ipmi_psu_raw_unit_id[unit];
    }
    else if (deviceType == VENDOR_FAN)
    {
        if (info->command_type == BMC_CMD_TYPE_STANDARD)
            unit_id_str = ipmi_fan_std_unit_id[unit];
        else if (info->command_type == BMC_CMD_TYPE_RAW)
            unit_id_str = ipmi_fan_raw_unit_id[unit];
    }
    else if (deviceType == VENDOR_TEMPERATURE)
    {
        unit_id_str = ipmi_thrml_unit_id[unit];
    }
    else if (deviceType == VENDOR_PSU_PRESENT)
    {
        if (info->command_type == BMC_CMD_TYPE_STANDARD)
            unit_id_str = ipmi_psu_std_unit_id[unit];
        else if (info->command_type == BMC_CMD_TYPE_RAW)
            unit_id_str = ipmi_psu_raw_unit_id[unit];
    }
    else if (deviceType == VENDOR_FAN_PRESENT)
    {
        if (info->command_type == BMC_CMD_TYPE_STANDARD)
            unit_id_str = ipmi_fan_std_unit_id[unit];
        else if (info->command_type == BMC_CMD_TYPE_RAW)
            unit_id_str = ipmi_fan_raw_unit_id[unit];
    }

    return unit_id_str;
}

//OK
static int bmc_raw_data_process(char *bmcData)
{
    uint32_t result, value[4];

    if ((sscanf(bmcData, " %02X %02X %02X %02X %02X", &result,
                    &value[0], &value[1], &value[2], &value[3]) != 5) ||
        (result != 0))
    {
        return 0;
    }
    else
    {
        result = (value[0] & 0xff);
        result |= ((value[1] & 0xff) << 8);
        result |= ((value[2] & 0xff) << 16);
        result |= ((value[3] & 0xff) << 24);
        sprintf(bmcData, "%u", result);

        return 0;
    }
}

static int _bmc_data_get(
    ipmi_bus_driver_t *ipmi,
    int deviceType,
    int id,
    int bus,
    void *command_info,
    char *bmcData,
    int dataLen)
{
    int rv = 0;

    if (!command_info || !bmcData) return 0;

    bmc_ipmi_command_info_t *info = NULL;
    char *unit_id_str = NULL;
    char  cmd[32] = {0};

    info = (bmc_ipmi_command_info_t *)command_info;
    if (!info->cmd || !info->filter) return 0;

    if ((unit_id_str = get_bmc_command_unit_id(deviceType, id, info)) == NULL)
    {
        AIM_LOG_ERROR("Can't get unit id for deviceType %d!", deviceType);
        return 0;
    }
    sprintf(cmd, info->cmd, unit_id_str);

    rv = ipmi->get(bus,
                cmd,
                info->filter,
                bmcData,
                dataLen);
    //AIM_LOG_ERROR("Issue ipmitool %s %s => %s", cmd, info->filter, bmcData);

    if ((rv == 0) && (info->command_type == BMC_CMD_TYPE_RAW))
        rv = bmc_raw_data_process(bmcData);

    return rv;
}

int bmc_data_get(
    ipmi_bus_driver_t *ipmi,
    int deviceType,
    int id,
    int bus,
    bmc_command_t command,
    char *bmcData,
    int dataLen)
{
    void   *info = NULL;

    info = get_bmc_command_info(command);
    return _bmc_data_get(ipmi, deviceType, id, bus, info, bmcData, dataLen);
}

static int bmc_psu_model_get(
    void *busDrvPtr,
    int bus,
    uint8_t addr,
    char *model)
{
    ipmi_bus_driver_t *ipmi = (ipmi_bus_driver_t *)busDrvPtr;
    int rv = 0;
    char buf[256] = {0};
    int len = 0;
    int deviceType = bmc_addr_get_device_type(addr);
    int id = bmc_addr_get_id(addr);

    if (!model) return 0;

    if ((rv = bmc_data_get(ipmi, deviceType, id,  bus, BMC_CMD_PSU_GET_MODEL_NAME, buf, sizeof(buf))) == 0)
    {
        len = strlen(buf);
        if (buf[len-1] == '\n')
        {
            buf[len-1] = '\0';
            len--;
        }
        strcpy(model, buf);
    }

    return rv;
}

static int bmc_psu_serial_get(
    void *busDrvPtr,
    int bus,
    uint8_t addr,
    char *serial)
{
    ipmi_bus_driver_t *ipmi = (ipmi_bus_driver_t *)busDrvPtr;
    int rv = 0;
    char buf[256] = {0};
    int len = 0;
    int deviceType = bmc_addr_get_device_type(addr);
    int id = bmc_addr_get_id(addr);

    if (!serial) return 0;

    if ((rv = bmc_data_get(ipmi, deviceType, id,  bus, BMC_CMD_PSU_GET_SERIAL_NUM, buf, sizeof(buf))) == 0)
    {
        len = strlen(buf);
        if (buf[len-1] == '\n')
        {
            buf[len-1] = '\0';
            len--;
        }
        strcpy(serial, buf);
    }

    return rv;
}

static int bmc_psu_volt_get(
    void *busDrvPtr,
    int bus,
    uint8_t addr,
    int *volt)
{
    ipmi_bus_driver_t *ipmi = (ipmi_bus_driver_t *)busDrvPtr;
    int rv = 0;
    char buf[256] = {0};
    int deviceType = bmc_addr_get_device_type(addr);
    int id = bmc_addr_get_id(addr);

    if (!volt) return 0;

    if ((rv = bmc_data_get(ipmi, deviceType, id,  bus, BMC_CMD_PSU_GET_VOLTAGE, buf, sizeof(buf))) == 0)
    {
        *volt = atoi(buf) ;
    }

    return rv;
}

static int bmc_psu_amp_get(
    void *busDrvPtr,
    int bus,
    uint8_t addr,
    int *amp)
{
    ipmi_bus_driver_t *ipmi = (ipmi_bus_driver_t *)busDrvPtr;
    int rv = 0;
    char buf[256] = {0};
    int deviceType = bmc_addr_get_device_type(addr);
    int id = bmc_addr_get_id(addr);

    if (!amp) return 0;

    if ((rv = bmc_data_get(ipmi, deviceType, id,  bus, BMC_CMD_PSU_GET_CURRENT, buf, sizeof(buf))) == 0)
    {
        *amp = atoi(buf) ;
    }

    return rv;
}

static int bmc_psu_watt_get(
    void *busDrvPtr,
    int bus,
    uint8_t addr,
    int *watt)
{
    ipmi_bus_driver_t *ipmi = (ipmi_bus_driver_t *)busDrvPtr;
    int rv = 0;
    char buf[256] = {0};
    int deviceType = bmc_addr_get_device_type(addr);
    int id = bmc_addr_get_id(addr);

    if (!watt) return 0;

    if ((rv = bmc_data_get(ipmi, deviceType, id,  bus, BMC_CMD_PSU_GET_VOLTAGE, buf, sizeof(buf))) == 0)
    {
        *watt = atoi(buf) ;
    }

    if ((rv = bmc_data_get(ipmi, deviceType, id,  bus, BMC_CMD_PSU_GET_CURRENT, buf, sizeof(buf))) == 0)
    {
        *watt *= atoi(buf);
        *watt /= 1000;
    }

    return rv;
}

static int bmc_fan_rpm_get(
    void *busDrvPtr,
    int bus,
    uint8_t addr,
    int noused,
    int *rpm)
{
    ipmi_bus_driver_t *ipmi = (ipmi_bus_driver_t *)busDrvPtr;
    int rv = 0;
    char buf[256] = {0};
    int deviceType = bmc_addr_get_device_type(addr);
    int id = bmc_addr_get_id(addr);

    if ((rv = bmc_data_get(ipmi, deviceType, id,  bus, BMC_CMD_FAN_GET_RPM, buf, sizeof(buf))) == 0)
    {
        *rpm = (uint16_t)atoi(buf);
    }

    return rv;
}

static int bmc_fan_rpm_set(
    void *busDrvPtr,
    int bus,
    uint8_t addr,
    int noused,
    int rpm)
{
    int rv = 0;
    AIM_LOG_ERROR("It is not supported in BMC.");
    return rv;
}

static int bmc_thrml_temp_get(
    void *busDrvPtr,
    int bus,
    uint8_t addr,
    int noused,
    int *temperature)
{
    ipmi_bus_driver_t *ipmi = (ipmi_bus_driver_t *)busDrvPtr;
    int rv = 0;
    char buf[256] = {0};
    int deviceType = bmc_addr_get_device_type(addr);
    int id = bmc_addr_get_id(addr);

    if (!temperature) return 0;

    if ((rv = bmc_data_get(ipmi, deviceType, id,  bus, BMC_CMD_THERMAL_GET_TEMP, buf, sizeof(buf))) == 0)
    {
        *temperature = atoi(buf) * 1000;
    }

    return rv;
}

static int bmc_thrml_limit_get(
    void *busDrvPtr,
    int bus,
    uint8_t addr,
    int noused,
    int type,
    int *temperature)
{
    return 0;
}

static int bmc_thrml_limit_set(
    void *busDrvPtr,
    int bus,
    uint8_t addr,
    int noused,
    int type,
    int temperature)
{
    return 0;
}
static int bmc_present_get(
    void *busDrvPtr,
    int bus,
    uint8_t addr,
    uint8_t offset,
    uint8_t *present)
{
    ipmi_bus_driver_t *ipmi = (ipmi_bus_driver_t *)busDrvPtr;
    bmc_command_t       command;
    int rv = 0;
    char buf[256] = {0};
    int deviceType = bmc_addr_get_device_type(addr);
    int id = bmc_addr_get_id(addr);

    if (!present) return 0;

    if (deviceType == VENDOR_PSU_PRESENT)
        command = BMC_CMD_PSU_GET_PRESENT;
    else if (deviceType == VENDOR_FAN_PRESENT)
        command = BMC_CMD_FAN_GET_PRESENT;
    else
    {
        AIM_LOG_ERROR("Unknown BMC deviceType: %d!", deviceType);
        return 0;
    }

    if ((rv = bmc_data_get(ipmi, deviceType, id,  bus, command, buf, sizeof(buf))) == 0)
    {
        if (buf[0] == '\0')
            *present = 0;
        else
        {
            if ((command == BMC_CMD_FAN_GET_PRESENT) && !atoi(buf))
                *present = 0;
            else
                *present = 1;
        }
    }

    return rv;
}

static psu_dev_driver_t bmc_psu_functions =
{
    bmc_psu_model_get,
    bmc_psu_serial_get,
    bmc_psu_volt_get,
    bmc_psu_amp_get,
    bmc_psu_watt_get
};

static fan_dev_driver_t bmc_fan_functions =
{
    bmc_fan_rpm_get,
    bmc_fan_rpm_set,
};

static thermal_dev_driver_t bmc_thrml_functions =
{
    bmc_thrml_temp_get,
    bmc_thrml_limit_get,
    bmc_thrml_limit_set
};

static present_get_driver_t bmc_stat_functions =
{
    bmc_present_get
};

int bmc_psu_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "BMC_PSU", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &bmc_psu_functions;

    return vendor_driver_add(driver);
}

int bmc_fan_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "BMC_FAN", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &bmc_fan_functions;

    return vendor_driver_add(driver);
}

int bmc_thrml_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "BMC_TMP", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &bmc_thrml_functions;

    return vendor_driver_add(driver);
}

int bmc_present_get_driver_init()
{
    vendor_driver_t *driver = (vendor_driver_t *)calloc(1, sizeof(vendor_driver_t));

    strncpy(driver->name, "BMC_STAT", VENDOR_MAX_NAME_SIZE);
    driver->dev_driver = &bmc_stat_functions;

    return vendor_driver_add(driver);
}
/*BMC DEVICE END*/

/*
    DEVICE DRIVER END:
*/

/*
    VENDOR FUNCTION START
*/

static vendor_driver_node_t *driver_list_head, *driver_list_curr;

static int vendor_driver_add(vendor_driver_t *driver)
{
    vendor_driver_node_t *newnode =
        (vendor_driver_node_t *) calloc(1, sizeof(vendor_driver_node_t));

    newnode->driver_hdl = driver;

    if(!driver_list_head)
    {
        driver_list_head = driver_list_curr = newnode;
    }
    else
    {
        driver_list_curr->node = newnode;
        driver_list_curr = newnode;
    }

    return 0;
}

void *vendor_find_driver_by_name(const char *driver_name)
{
    vendor_driver_node_t *driver_node = driver_list_head;

    while(driver_node)
    {
        if (strncmp(
            ((vendor_driver_t *)((vendor_driver_node_t *)driver_node->driver_hdl))->name,
            driver_name,
            VENDOR_MAX_NAME_SIZE) == 0)
        {
            return (void *) (driver_node->driver_hdl->dev_driver);
        }
        driver_node = driver_node->node;
    }

    AIM_LOG_ERROR("Function: %s, Cannot find driver %s.", __FUNCTION__, driver_name);

    return NULL;
}

int vendor_find_copper_by_name(const char *dev_name)
{

    if (strncmp(dev_name, "copper", VENDOR_MAX_NAME_SIZE) == 0)
        return 1;
    else
        return 0;
}

int vendor_driver_init()
{
    smbus_driver_init();
    ipmi_driver_init();
    cpld_driver_init();
    eeprom_driver_init();
    emc2305_driver_init();
    tmp75_driver_init();
    tmp461_driver_init();
    pmbus_psu_driver_init();
    pmbus_fan_driver_init();
    sff8636_driver_init();
    sff8472_driver_init();
    ipmb_driver_init();
    bmc_psu_driver_init();
    bmc_fan_driver_init();
    bmc_thrml_driver_init();
    bmc_present_get_driver_init();

    return 0;
}

int vendor_dev_do_oc(vendor_dev_oc_t *dev_oc)
{
    //AIM_LOG_ERROR("Function: %s", __FUNCTION__);
    int rv = 0;

    if(dev_oc == NULL)
    {
        /*NO NEED */
        return rv;
    }

    while (dev_oc->type != 0)
    {
        if(dev_oc->type == 1)
        {
            rv = smbus_writeb(dev_oc->bus, dev_oc->addr, dev_oc->offset, dev_oc->value);
        }
        else if (dev_oc->type == 2)
        {
            rv = smbus_readb(dev_oc->bus, dev_oc->addr, dev_oc->offset);
            if(rv < 0) return rv;

            rv &= ~dev_oc->mask;
            rv |= (dev_oc->match & dev_oc->mask);

            rv = smbus_writeb(dev_oc->bus, dev_oc->addr, dev_oc->offset, rv);
        }
        else
        {
            AIM_LOG_ERROR("Function: %s, unknown oc type. \n", __FUNCTION__);
            rv = -1;
        }

        if(rv < 0) return rv;

        dev_oc++;
    }

    return rv;
}

int vendor_find_cpld_idx(uint8_t addr)
{
    int idx = 0;

    while(idx < cpld_list_size)
    {
        if(cpld_dev_list[idx].addr == addr) return idx;
        idx++;
    }

    AIM_LOG_ERROR("Function: %s, Cannot find CPLD index.", __FUNCTION__);
    return -1;
}

int vendor_get_present_status(vendor_dev_io_pin_t *present_info, int *present)
{
    int rv = 0, cpld_idx = 0;
    void *busDrv = NULL;
    present_get_driver_t *pg = NULL;


    if(present_info->type == CPLD_DEV)
    {
        if(present_info->addr == 0)
        {
            *present = 1;
            return 0;
        }

        busDrv = (void *)vendor_find_driver_by_name("I2C");
        pg = (present_get_driver_t *) vendor_find_driver_by_name("CPLD");

        cpld_idx = vendor_find_cpld_idx(present_info->addr);
        if(cpld_idx < 0) return ONLP_STATUS_E_INTERNAL;

        vendor_dev_do_oc(cpld_o_list[cpld_idx]);
    }
    else if(present_info->type == BMC_DEV)
    {
        busDrv = (void *)vendor_find_driver_by_name("IPMI");
        pg = (present_get_driver_t *) vendor_find_driver_by_name("BMC_STAT");
    }
    else
    {
        AIM_LOG_ERROR("Unknow PRESENT TYPE.");
        return -1;
    }

    rv = pg->present_get(
            busDrv,
            present_info->bus,
            present_info->addr,
            present_info->offset,
            (uint8_t *) present);

    if(present_info->type == CPLD_DEV)
    {
        vendor_dev_do_oc(cpld_c_list[cpld_idx]);
        *present = ((rv & present_info->mask) == present_info->match) ? 1 : 0;
    }

    return rv;
}

int vendor_system_call_get(char *cmd, char *data)
{
    FILE *fp;
    char buf[256] = {0};
    int c;

    fp = popen(cmd,"r");
    if(!fp)
        return -1;

    while(fread(buf, sizeof(buf), 1, fp))
        while ((c = getchar()) != EOF)
            printf("output = %s", buf);

    memcpy(data, buf, sizeof(buf));

    return WEXITSTATUS(pclose(fp));
}

int vendor_system_call_set(char *cmd)
{
    return system(cmd);
}

/*
    VENDOR DRIVER FUNCTION END
*/