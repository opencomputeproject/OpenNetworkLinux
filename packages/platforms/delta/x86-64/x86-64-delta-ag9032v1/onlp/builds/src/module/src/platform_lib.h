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

#include "x86_64_delta_ag9032v1_log.h"

/* CPLD numbrt & peripherals */
#define NUM_OF_THERMAL_ON_BOARDS  6
#define NUM_OF_FAN_ON_FAN_BOARD   10
#define NUM_OF_PSU_ON_PSU_BOARD   2
#define NUM_OF_LED_ON_BOARDS      9
#define NUM_OF_CPLD               2
#define CHASSIS_FAN_COUNT         10
#define CHASSIS_THERMAL_COUNT     6


#define PREFIX_PATH   "/sys/bus/i2c/devices/"
#define PSU1_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/4-0058/"
#define PSU2_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/4-0058/"
#define SWPLD_ADDR_PATH "/sys/bus/i2c/devices/6-0031/addr"
#define SWPLD_DATA_PATH "/sys/bus/i2c/devices/6-0031/data"
#define FAN_INPUT_PATH "/sys/bus/i2c/devices/3-002c/"
#define FAN1_FRONT     "/sys/bus/i2c/devices/3-002c/fan1_input"
#define FAN1_REAR      "/sys/bus/i2c/devices/3-002d/fan1_input"
#define FAN2_FRONT     "/sys/bus/i2c/devices/3-002c/fan2_input"
#define FAN2_REAR      "/sys/bus/i2c/devices/3-002d/fan2_input"
#define FAN3_FRONT     "/sys/bus/i2c/devices/3-002c/fan3_input"
#define FAN3_REAR      "/sys/bus/i2c/devices/3-002d/fan3_input"
#define FAN4_FRONT     "/sys/bus/i2c/devices/3-002c/fan4_input"
#define FAN4_REAR      "/sys/bus/i2c/devices/3-002d/fan4_input"
#define FAN5_FRONT     "/sys/bus/i2c/devices/3-002c/fan5_input"
#define FAN5_REAR      "/sys/bus/i2c/devices/3-002d/fan5_input"
#define IDPROM_PATH    "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-0053/eeprom"
#define PSU_SELECT_MEMBER_PATH  "/sys/bus/i2c/devices/4-0058/psu_select_member"
#define SFP_SELECT_PORT_PATH    "/sys/bus/i2c/devices/5-0050/sfp_select_port"
#define SFP_IS_PRESENT_PATH     "/sys/bus/i2c/devices/5-0050/sfp_is_present"
#define SFP_IS_PRESENT_ALL_PATH "/sys/bus/i2c/devices/5-0050/sfp_is_present_all"
#define SFP_EEPROM_PATH         "/sys/bus/i2c/devices/5-0050/sfp_eeprom"
#define SFP_RESET_PATH          "/sys/bus/i2c/devices/5-0050/sfp_reset"
#define SFP_LP_MODE_PATH        "/sys/bus/i2c/devices/5-0050/sfp_lp_mode"

#define PSU1_AC_PMBUS_NODE(node) PSU1_AC_PMBUS_PREFIX#node
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node

/* BUS define */
#define I2C_BUS_0               (0)
#define I2C_BUS_1               (1)
#define I2C_BUS_2               (2)
#define I2C_BUS_3               (3)
#define I2C_BUS_4               (4)
#define I2C_BUS_5               (5)
#define I2C_BUS_6               (6)
#define I2C_BUS_7               (7)
#define I2C_BUS_8               (8)
#define I2C_BUS_9               (9)
#define PSU1_ID   (1)
#define PSU2_ID   (2)
#define TURN_OFF  (0)
#define TURN_ON   (1)
#define ALL_FAN_TRAY_EXIST (5)
#define PSU_STATUS_PRESENT    (1)
#define PSU_NODE_MAX_PATH_LEN (64)
#define FAN_SPEED_NORMALLY (5)
#define SPEED_25_PERCENTAGE (25)
#define SPEED_50_PERCENTAGE (50)
#define SPEED_75_PERCENTAGE (75)
#define SPEED_100_PERCENTAGE (100)
#define FAN_ZERO_TACH (960)

/* REG define*/
#define DEFAULT_FLAG (0x00)
#define SWPLD_QSFP28_I2C_MUX_REG  (0x20)
#define SWPLD_PSU_FAN_I2C_MUX_REG (0x21)
#define CPUCPLD             (0x31)
#define CPUPLD_VERSION_ADDR (0x01)
#define SWPLD	           (0x31)
#define SWPLD_VERSION_ADDR (0x01)
#define LED_REG (0x1C)
#define CTL_REG (0x0A)
#define PSU_EEPROM (0x50)
#define CLOSE_RESPOND (0xFF)
#define EMC2305_FRONT_FAN     (0x2C)
#define EMC2305_REAR_FAN      (0x2D)
#define PSU_FAN1     (0x00)
#define PSU_FAN2     (0x20)
#define FAN_DATA_HALF_SPEED (0x0032)
#define FAN_DATA_FULL_SPEED (0x0064)
#define FAN_DATA_STOP_D10_D3    (0xFF)
#define FAN_DATA_STOP_D2_D0     (0xE0)
#define FAN_IO_CTL (0x27)
#define FAN_TRAY_1 (0x51)
#define FAN_TRAY_2 (0x52)
#define FAN_TRAY_3 (0x53)
#define FAN_TRAY_4 (0x54)
#define FAN_TRAY_5 (0x55)
#define FAN_TRAY_LED_REG   (0x1D)
#define FAN_TRAY_LED_REG_2 (0x1E)
#define PSU_I2C_SEL_PSU1_EEPROM (0x00)
#define PSU_I2C_SEL_PSU2_EEPROM (0x20)
#define FAN_I2C_SEL_FAN_THERMAL (0x06)
#define SFP_I2C_MUX_REG (0x20)
#define SFP_RESPOND_1   (0x30)
#define SFP_RESPOND_2   (0x31)
#define SFP_RESPOND_3   (0x32)
#define SFP_RESPOND_4   (0x33)
#define SFP_LP_MODE_1   (0x34)
#define SFP_LP_MODE_2   (0x35)
#define SFP_LP_MODE_3   (0x36)
#define SFP_LP_MODE_4   (0x37)
#define SFP_RESET_1     (0x3C)
#define SFP_RESET_2     (0x3D)
#define SFP_RESET_3     (0x3E)
#define SFP_RESET_4     (0x3F)

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
int dni_lock_swpld_read_attribute(int addr);
int dni_lock_swpld_write_attribute(int addr, int addr1);

typedef enum
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_CPU_BOARD,
    THERMAL_2_ON_FAN_BOARD,
    THERMAL_3_ON_SW_BOARD,
    THERMAL_4_ON_SW_BOARD,
    THERMAL_5_ON_SW_BOARD,
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
    FAN_9_ON_FAN_BOARD,
    FAN_10_ON_FAN_BOARD,
    FAN_1_ON_PSU1,
    FAN_1_ON_PSU2
} onlp_fan_id;

typedef enum
{
    LED_RESERVED = 0,
    LED_FRONT_FAN,
    LED_FRONT_SYS,
    LED_FRONT_PWR1,
    LED_FRONT_PWR2,
    LED_REAR_FAN_TRAY_1,
    LED_REAR_FAN_TRAY_2,
    LED_REAR_FAN_TRAY_3,
    LED_REAR_FAN_TRAY_4,
    LED_REAR_FAN_TRAY_5
}onlp_led_id;

#endif  /* __PLATFORM_LIB_H__ */

