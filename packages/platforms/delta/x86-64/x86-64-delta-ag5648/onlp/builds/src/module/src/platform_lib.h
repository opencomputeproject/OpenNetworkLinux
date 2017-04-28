/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017  Delta Networks, Inc. 
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
#ifndef __PLATFORM_LIB_H__
#define __PLATFORM_LIB_H__

#include "x86_64_delta_ag5648_log.h"

/* CPLD numbrt & peripherals */
#define NUM_OF_THERMAL_ON_BOARDS  9
#define NUM_OF_FAN_ON_FAN_BOARD   8
#define NUM_OF_PSU_ON_PSU_BOARD   2
#define NUM_OF_LED_ON_BOARDS      7
#define NUM_OF_CPLD               3
#define CHASSIS_FAN_COUNT         8
#define CHASSIS_THERMAL_COUNT     7

#define MAX_REAR_FAN_SPEED  20500
#define MAX_FRONT_FAN_SPEED 23000
#define MAX_PSU_FAN_SPEED   19000

#define NUM_OF_SFP 48
#define NUM_OF_QSFP 6
#define NUM_OF_PORT NUM_OF_SFP + NUM_OF_QSFP

#define PREFIX_PATH "/sys/bus/i2c/devices"
#define SYS_CPLD_PATH PREFIX_PATH "/2-0031"      
#define MASTER_CPLD_PATH PREFIX_PATH "/2-0035"   
#define SLAVE_CPLD_PATH PREFIX_PATH "/2-0039"    
#define MASTER_CPLD_ADDR_PATH PREFIX_PATH "/2-0035/addr"
#define MASTER_CPLD_DATA_PATH PREFIX_PATH "/2-0035/data"
#define SLAVE_CPLD_ADDR_PATH PREFIX_PATH "/2-0039/addr"
#define SLAVE_CPLD_DATA_PATH PREFIX_PATH "/2-0039/data"

#define PSU1_AC_PMBUS_PREFIX PREFIX_PATH "/6-0059/"
#define PSU2_AC_PMBUS_PREFIX PREFIX_PATH "/6-0058/"
#define PSU1_SELECT_MEMBER_PATH PREFIX_PATH "/6-0059/psu_select_member"
#define PSU2_SELECT_MEMBER_PATH PREFIX_PATH "/6-0058/psu_select_member"
#define PSU1_AC_PMBUS_NODE(node) PSU1_AC_PMBUS_PREFIX#node
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node

#define FAN1_FRONT     PREFIX_PATH "/3-004d/fan1_input"
#define FAN1_REAR      PREFIX_PATH "/5-004d/fan1_input"
#define FAN2_FRONT     PREFIX_PATH "/3-004d/fan2_input"
#define FAN2_REAR      PREFIX_PATH "/5-004d/fan2_input"
#define FAN3_FRONT     PREFIX_PATH "/3-004d/fan3_input"
#define FAN3_REAR      PREFIX_PATH "/5-004d/fan3_input"
#define FAN4_FRONT     PREFIX_PATH "/3-004d/fan4_input"
#define FAN4_REAR      PREFIX_PATH "/5-004d/fan4_input"
#define IDPROM_PATH    "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-0053/eeprom"

#define SFP_SELECT_PORT_PATH    PREFIX_PATH "/4-0050/sfp_select_port"
#define SFP_IS_PRESENT_PATH     PREFIX_PATH "/4-0050/sfp_is_present"
#define SFP_IS_PRESENT_ALL_PATH PREFIX_PATH "/4-0050/sfp_is_present_all"
#define SFP_EEPROM_PATH         PREFIX_PATH "/4-0050/sfp_eeprom"
#define SFP_RESET_PATH          PREFIX_PATH "/4-0050/sfp_reset"
#define SFP_LP_MODE_PATH        PREFIX_PATH "/4-0050/sfp_lp_mode"

/* BUS define */
#define I2C_BUS_0             (0)
#define I2C_BUS_1             (1)
#define I2C_BUS_2             (2)
#define I2C_BUS_3             (3)
#define I2C_BUS_4             (4)
#define I2C_BUS_5             (5)
#define I2C_BUS_6             (6)
#define I2C_BUS_7             (7)
#define I2C_BUS_8             (8)
#define I2C_BUS_9             (9)
#define PSU1_ID               (1)
#define PSU2_ID               (2)
#define TURN_OFF              (0)
#define TURN_ON               (1)
#define ALL_FAN_TRAY_EXIST    (4)
#define PSU_STATUS_PRESENT    (1)
#define PSU_NODE_MAX_PATH_LEN (64)
#define FAN_ZERO_RPM          (960)
#define SPEED_100_PERCENTAGE  (100)


/* REG define*/
#define DEFAULT_FLAG             (0x00)
#define QSFP_RESPOND_REG         (0x10)
#define SYS_CPLD_VERSION_ADDR    (0x08)
#define MASTER_CPLD_VERSION_ADDR (0x17)
#define SLAVE_CPLD_VERSION_ADDR  (0x01)
#define SYS_CPLD                 (0x31)
#define MASTER_CPLD              (0x35)
#define SLAVE_CPLD               (0x39)
#define SYS_VERSION_REG          (0x08)
#define MASTER_VERSION_REG       (0x17)
#define SLAVE_VERSION_REG        (0x01)
#define LED_REG                  (0x04)  
#define PSU1_EEPROM              (0x51)
#define PSU2_EEPROM              (0x50)
#define EMC2305_FRONT_FAN        (0x4D)
#define EMC2305_REAR_FAN         (0x4D)
#define FAN_STATUS_REG           (0x06)
#define FAN_TRAY_1               (0x51)
#define FAN_TRAY_2               (0x52)
#define FAN_TRAY_3               (0x53)
#define FAN_TRAY_4               (0x54)
#define FAN_TRAY_5               (0x55)
#define FAN_TRAY_LED_REG         (0x05) 
#define PSU_I2C_SEL_PSU1_EEPROM  (0xB2)
#define PSU_I2C_SEL_PSU2_EEPROM  (0xB0)
#define SFP_I2C_MUX_REG          (0x18)
#define SFP_LP_MODE              (0x11)
#define SFP_RESET                (0x13)
#define QSFP_MODE_SEL_REG        (0x10)
#define FAN_STAT1_REG            (0x05)  
#define FAN_STAT2_REG            (0x06)  
#define PSU_STAT_REG             (0x03)  
#define ALARM_REG                (0x06)  
#define INTERRUPT_REG            (0x02)
#define PORT_ADDR                (0x50)


int dni_i2c_read_attribute_binary(char *filename, char *buffer, int buf_size, int data_len);
int dni_i2c_read_attribute_string(char *filename, char *buffer, int buf_size, int data_len);

typedef struct dev_info_s
{
    int bus;
    int size;
    uint8_t addr;
    uint8_t data_8;
	uint16_t data_16;
    uint8_t offset;
    uint32_t flags;
}dev_info_t;

typedef struct mux_info_s
{
    int bus;
    uint8_t addr;
    uint8_t offset;
    uint8_t channel;
    char dev_data[10];
    uint32_t flags;
}mux_info_t;

pthread_mutex_t mutex;
pthread_mutex_t mutex1;
int dni_i2c_lock_read(mux_info_t * mux_info, dev_info_t * dev_info);
int dni_i2c_lock_write(mux_info_t * mux_info, dev_info_t * dev_info);
int dni_i2c_lock_read_attribute(mux_info_t * mux_info, char * fullpath);
int dni_i2c_lock_write_attribute(mux_info_t * mux_info, char * data,char * fullpath);
int dni_lock_cpld_write_attribute(char *cpld_path, int addr, int data);
int dni_lock_cpld_read_attribute(char *cpld_path, int addr);

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(format, ...)   printf(format, __VA_ARGS__)
#else
    #define DEBUG_PRINT(format, ...)
#endif

typedef enum
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_CPU_BOARD,
    THERMAL_2_ON_FAN_BOARD,
    THERMAL_3_ON_MAIN_BOARD,
    THERMAL_4_ON_MAIN_BOARD,
    THERMAL_5_ON_MAIN_BOARD,
    THERMAL_6_ON_MAIN_BOARD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
} onlp_thermal_id;

typedef enum
{
    FAN_RESERVED = 0,
    FAN_1_ON_FAN_BOARD,
    FAN_2_ON_FAN_BOARD,
    FAN_3_ON_FAN_BOARD,
    FAN_4_ON_FAN_BOARD,
    FAN_5_ON_FAN_BOARD,
    FAN_6_ON_FAN_BOARD,
    FAN_7_ON_FAN_BOARD,
    FAN_8_ON_FAN_BOARD,
    FAN_1_ON_PSU1,
    FAN_1_ON_PSU2
} onlp_fan_id;

typedef enum
{
    LED_RESERVED = 0,
    LED_FRONT_FAN,
    LED_FRONT_SYS,
    LED_FRONT_PWR,
    LED_REAR_FAN_TRAY_1,
    LED_REAR_FAN_TRAY_2,
    LED_REAR_FAN_TRAY_3,
    LED_REAR_FAN_TRAY_4
}onlp_led_id;

#endif  /* __PLATFORM_LIB_H__ */

