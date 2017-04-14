/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017 Delta Networks, Inc.
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

#include "x86_64_delta_agc7648a_log.h"

#define CHASSIS_FAN_COUNT     8
#define CHASSIS_THERMAL_COUNT 8

#define PSU1_ID 1
#define PSU2_ID 2

#define SYS_DEV_PATH "/sys/bus/i2c/devices"
#define CPU_CPLD_PATH SYS_DEV_PATH "/2-0031"
#define SWPLD_PATH SYS_DEV_PATH "/5-0030"
#define SWPLD1_PATH SYS_DEV_PATH "/5-0031"
#define SWPLD2_PATH SYS_DEV_PATH "/5-0032"
#define PSU1_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/4-0058/"
#define PSU2_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/4-0058/"

#define PSU1_AC_PMBUS_NODE(node) PSU1_AC_PMBUS_PREFIX#node
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node

#define PSU1_AC_HWMON_PREFIX "/sys/bus/i2c/devices/35-0038/"
#define PSU2_AC_HWMON_PREFIX "/sys/bus/i2c/devices/36-003b/"

#define PSU1_AC_HWMON_NODE(node) PSU1_AC_HWMON_PREFIX#node
#define PSU2_AC_HWMON_NODE(node) PSU2_AC_HWMON_PREFIX#node


#define FAN1_FRONT "/sys/bus/i2c/devices/3-002c/fan1_input"
#define FAN1_REAR "/sys/bus/i2c/devices/3-002d/fan1_input"
#define FAN2_FRONT "/sys/bus/i2c/devices/3-002c/fan2_input"
#define FAN2_REAR "/sys/bus/i2c/devices/3-002d/fan2_input"
#define FAN3_FRONT "/sys/bus/i2c/devices/3-002c/fan3_input"
#define FAN3_REAR "/sys/bus/i2c/devices/3-002d/fan3_input"
#define FAN4_FRONT "/sys/bus/i2c/devices/3-002c/fan4_input"
#define FAN4_REAR "/sys/bus/i2c/devices/3-002d/fan4_input"

#define SFP_SELECT_PORT_PATH "/sys/bus/i2c/devices/8-0050/sfp_select_port"
#define SFP_IS_PRESENT_PATH "/sys/bus/i2c/devices/8-0050/sfp_is_present"
#define SFP_IS_PRESENT_ALL_PATH "/sys/bus/i2c/devices/8-0050/sfp_is_present_all"
#define SFP_EEPROM_PATH "/sys/bus/i2c/devices/8-0050/sfp_eeprom"
#define SFP_RESET_PATH "/sys/bus/i2c/devices/8-0050/sfp_reset"
#define SFP_LP_MODE_PATH "/sys/bus/i2c/devices/8-0050/sfp_lp_mode"

#define PSU_SEL_PATH "/sys/bus/i2c/devices/4-0058/psu_select_member"

#define IDPROM_PATH "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-0053/eeprom"

/* I2C bus */
#define I2C_BUS_0               (0)
#define I2C_BUS_1               (1)
#define I2C_BUS_2               (2)
#define I2C_BUS_3               (3)
#define I2C_BUS_4               (4)
#define I2C_BUS_5               (5)
#define I2C_BUS_6               (6)
#define I2C_BUS_7               (7)
#define I2C_BUS_8               (8)

/* Device address */
#define SWPLD	 (0x30)
#define SWPLD1   (0x31)
#define SWPLD2   (0x32)
#define SWPLD2   (0x32)
#define FAN_IO_CTRL (0x27)
#define PSU_EEPROM (0x50)
#define EMC2305_FRONT_FAN     (0x2C)
#define EMC2305_REAR_FAN     (0x2D)
#define FAN_TRAY_LED_REG (0x65)
#define FAN_TRAY_1 (0x52)
#define FAN_TRAY_2 (0x53)
#define FAN_TRAY_3 (0x54)
#define FAN_TRAY_4 (0x55)
#define PORT_ADDR (0x50)

/* CPU CPLD Register */
#define SWPLD_VERSION_ADDR (0x01)

/* SWPLD(U21) Register */
#define QSFP_RESPOND_REG (0x64)
#define FAN_MUX_REG (0x67)
#define PSU_PRESENT_REG (0x0D)
#define PSU_PWR_REG (0x0B)
#define LED_REG (0x1C)


/* Not Yet classified */
#define PSU1_MUX_MASK (0x00)
#define PSU2_MUX_MASK (0x02)
#define CLOSE_RESPOND (0xFF)
#define PSU_FAN1     (0x00)
#define PSU_FAN2     (0x20)
#define FAN_DATA_HALF_SPEED (0x0032)
#define FAN_DATA_FULL_SPEED (0x0064)
#define FAN_DATA_STOP_D10_D3    (0xFF)
#define FAN_DATA_STOP_D2_D0     (0xE0)
#define TURN_OFF (0)
#define TURN_ON (1)
#define FAN_ZERO_TACH 960

#define MAX_REAR_FAN_SPEED  20500
#define MAX_FRONT_FAN_SPEED 23000
#define MAX_PSU_FAN_SPEED   19000

#define NUM_OF_THERMAL 8
#define NUM_OF_FAN_ON_MAIN_BROAD      8
#define NUM_OF_PSU_ON_MAIN_BROAD      2
#define NUM_OF_LED_ON_MAIN_BROAD      7
#define NUM_OF_CPLD                   4

#define DEFAULT_FLAG (0x00)

#define NUM_OF_SFP 48
#define NUM_OF_QSFP 6
#define NUM_OF_PORT NUM_OF_SFP + NUM_OF_QSFP

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
    char data[10];
    char path[80];
    uint32_t flags;
}mux_info_t;

pthread_mutex_t mutex;
pthread_mutex_t mutex1;
int dni_i2c_lock_read(mux_info_t * mux_info, dev_info_t * dev_info);
int dni_i2c_lock_write(mux_info_t * mux_info, dev_info_t * dev_info);
int dni_i2c_lock_read_attribute(mux_info_t * mux_info, char * fullpath);
int dni_i2c_lock_write_attribute(mux_info_t * mux_info, char *data, char * fullpath);
int dni_lock_cpld_read_attribute(char *cpld_path, int addr);
int dni_lock_cpld_write_attribute(char *cpld_path, int addr, int data);
int dni_psu_present_get(int index);

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
    THERMAL_1_ON_CPU_BROAD,
    THERMAL_2_ON_FAN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_MAIN_BROAD,
    THERMAL_5_ON_MAIN_BROAD,
    THERMAL_6_ON_MAIN_BROAD,
    THERMAL_7_ON_MAIN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
} onlp_thermal_id;

typedef enum
{
    FAN_RESERVED = 0,
    FAN_1_ON_MAIN_BOARD,
    FAN_2_ON_MAIN_BOARD,
    FAN_3_ON_MAIN_BOARD,
    FAN_4_ON_MAIN_BOARD,
    FAN_5_ON_MAIN_BOARD,
    FAN_6_ON_MAIN_BOARD,
    FAN_7_ON_MAIN_BOARD,
    FAN_8_ON_MAIN_BOARD,
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

