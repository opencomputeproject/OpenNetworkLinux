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
#ifndef __VENDOR_DRIVER_POOL_H__
#define __VENDOR_DRIVER_POOL_H__

#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/sfpi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/thermali.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <AIM/aim.h>
#include "x86_64_delta_agc032_log.h"


#define VENDOR_MAX_NAME_SIZE    20
#define CPLD_DEV 1
#define BMC_DEV 2


typedef struct
{
	char *dev_name;
    char *bus_drv_name;
    char *dev_drv_name;
	int bus;
	uint8_t addr;
    int id;
}vendor_dev_t;

typedef struct
{
    int type; /* 2 -> CPLD; 1 -> MUX; 0 -> invalid */
    int bus;
	uint8_t addr;
    uint8_t offset; /* FOR CPLD & MUX */
    uint8_t value;  /* FOR CPLD & MUX */
    uint8_t mask;   /* FOR CPLD */
    uint8_t match;  /* FOR CPLD */
}vendor_dev_oc_t;

typedef struct
{
    int type; /* 2 -> BMC; 1 -> CPLD; 0 -> invalid */
    int bus;
	uint8_t addr; /* DEVICE_TYPE + UNIT */
    uint8_t offset;
    uint8_t mask;
    uint8_t match;
}vendor_dev_io_pin_t;

typedef struct
{
    int bus;
	uint8_t addr;
    uint8_t offset;
    uint8_t mask;
    uint8_t match;
    onlp_led_mode_t mode;
}vendor_dev_led_pin_t;

typedef enum
{
    VENDOR_TEMPERATURE = 0,
    VENDOR_FAN = 1,
    VENDOR_PSU = 3,
    VENDOR_FAN_PRESENT = VENDOR_FAN,
    VENDOR_PSU_PRESENT = VENDOR_PSU,
}vendor_bmc_device_type_t;
typedef struct
{
    char name[VENDOR_MAX_NAME_SIZE];
    void *dev_driver;
}vendor_driver_t;

typedef struct
{
    void *node;
    vendor_driver_t *driver_hdl;
}vendor_driver_node_t;

typedef struct
{
    int (*readb)        (int bus, uint8_t addr, uint16_t offset);
    int (*writeb)       (int bus, uint8_t addr, uint16_t offset, uint8_t byte);
    int (*readw)        (int bus, uint8_t addr, uint16_t offset);
    int (*writew)       (int bus, uint8_t addr, uint16_t offset, uint16_t word);
    int (*block_read)   (int bus, uint8_t addr, uint16_t offset, int size, uint8_t* rdata);
}i2c_bus_driver_t;

typedef struct
{
    int (*get)(int bus, char *cmd, char *filter, char *data, uint32_t dlen);
    int (*set)(int bus, char *cmd, char *filter);
}ipmi_bus_driver_t;


typedef struct
{
    int (*present_get)(void *busDrvPtr, int bus, uint8_t addr, uint8_t offset, uint8_t *present);
}present_get_driver_t;

/* ONLY I2C */
typedef struct
{
    int (*readb)    (void *busDrvPtr, int bus, uint8_t addr, uint16_t offset, uint16_t len, uint8_t *buf);
    int (*writeb)   (void *busDrvPtr, int bus, uint8_t addr, uint16_t offset, uint16_t len, uint8_t *buf);
    int (*load)     (void *busDrvPtr, int bus, uint8_t addr, uint8_t *buf);
}eeprom_dev_driver_t;

/* ONLY I2C */
typedef struct
{
    int (*readb)    (void *busDrvPtr, int bus, uint8_t addr, uint8_t offset, uint8_t *value);
    int (*writeb)   (void *busDrvPtr, int bus, uint8_t addr, uint8_t offset, uint8_t value);
}cpld_dev_driver_t;

typedef struct
{
    int (*rpm_get)  (void *busDrvPtr, int bus, uint8_t addr, int id, int *rpm);
    int (*rpm_set)  (void *busDrvPtr, int bus, uint8_t addr, int id, int rpm);
}fan_dev_driver_t;

typedef enum
{
    VENDOR_THERMAL_LOW_THRESHOLD,
    VENDOR_THERMAL_HIGH_THRESHOLD,
}vendor_thermal_threshold_e;

typedef struct
{
    int (*temp_get)  (void *busDrvPtr, int bus, uint8_t addr, int id, int *temperature);
    int (*limit_get) (void *busDrvPtr, int bus, uint8_t addr, int id, int type, int *temperature);
    int (*limit_set) (void *busDrvPtr, int bus, uint8_t addr, int id, int type, int temperature);
}thermal_dev_driver_t;

typedef struct
{
    int (*model_get)   (void *busDrvPtr, int bus, uint8_t addr, char *model);
    int (*serial_get)  (void *busDrvPtr, int bus, uint8_t addr, char *serial);
    int (*volt_get)    (void *busDrvPtr, int bus, uint8_t addr, int *volt);
    int (*amp_get)     (void *busDrvPtr, int bus, uint8_t addr, int *amp);
    int (*watt_get)    (void *busDrvPtr, int bus, uint8_t addr, int *watt);
}psu_dev_driver_t;

typedef enum
{
    SFP_CONTROL_RESET,
    SFP_CONTROL_RESET_STATE,
    SFP_CONTROL_RX_LOS,
    SFP_CONTROL_TX_FAULT,
    SFP_CONTROL_TX_DISABLE,
    SFP_CONTROL_TX_DISABLE_CHANNEL,
    SFP_CONTROL_LP_MODE,
    SFP_CONTROL_POWER_OVERRIDE,
    SFP_CONTROL_TEMPERATURE,
    SFP_CONTROL_VOLTAGE,
    SFP_CONTROL_TX_BIAS,
    SFP_CONTROL_RX_POWER,
    SFP_CONTROL_TX_POWER,
    SFP_CONTROL_LAST = SFP_CONTROL_TX_POWER,
    SFP_CONTROL_COUNT,
    SFP_CONTROL_INVALID = -1,
}vendor_sfp_control_t;

/* ONLY I2C */
typedef struct
{
    int (*eeprom_load)          (void *busDrvPtr, int bus, uint8_t addr, uint8_t *data);
    int (*eeprom_readb)         (void *busDrvPtr, int bus, uint8_t addr, uint16_t offset, uint8_t *data);
    int (*eeprom_writeb)        (void *busDrvPtr, int bus, uint8_t addr, uint16_t offset, uint8_t data);
    int (*control_is_support)   (int control, uint8_t *is_support);
    int (*control_get)          (void *busDrvPtr, int bus, uint8_t addr, int control, int *status);
    int (*control_set)          (void *busDrvPtr, int bus, uint8_t addr, int control, int status);
}sfp_dev_driver_t;

int vendor_find_cpld_idx(uint8_t addr);
int vendor_system_call_get(char *cmd, char *data);
int vendor_system_call_set(char *cmd);
int vendor_driver_init();
void *vendor_find_driver_by_name(const char *driver_name);
int vendor_dev_do_oc(vendor_dev_oc_t *dev_oc);
int vendor_get_present_status(vendor_dev_io_pin_t *present_info, int *present);
int vendor_find_copper_by_name(const char *dev_name);

#endif /* __VENDOR_DRIVER_POOL_H__ */