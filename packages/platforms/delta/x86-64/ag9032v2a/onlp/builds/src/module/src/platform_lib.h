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

#include "x86_64_delta_ag9032v2a_log.h"
#include <onlp/onlp.h>
#include <onlplib/shlocks.h>
typedef unsigned int    UINT4;

/* CPLD numbrt & peripherals */
#define NUM_OF_SFP (1)
#define NUM_OF_QSFP (32)
#define NUM_OF_PORT NUM_OF_SFP + NUM_OF_QSFP
#define NUM_OF_THERMAL_ON_MAIN_BROAD (5)
#define NUM_OF_LED_ON_MAIN_BROAD     (9)
#define NUM_OF_FAN_ON_MAIN_BROAD     (10)
#define NUM_OF_PSU_ON_MAIN_BROAD     (2)
#define NUM_OF_SENSORS               (47)
#define CHASSIS_FAN_COUNT            (10)
#define CHASSIS_THERMAL_COUNT        (5)
#define NUM_OF_THERMAL               (7)
#define PSU1_ID                      (1)
#define PSU2_ID                      (2)
#define PSU_NUM_LENGTH               (15)
#define MAX_FRONT_FAN_SPEED          (23000)
#define MAX_PSU_FAN_SPEED            (18380)
#define MAX_REAR_FAN_SPEED           (20500)
#define FAN_ZERO_RPM                 (960)
#define FAN_SPEED_NORMALLY           (5)
#define ALL_FAN_TRAY_EXIST           (5)
#define BMC_OFF                      (1)
#define BMC_ON                       (0)
#define PSU_NODE_MAX_PATH_LEN        (64)

#define CPU_CPLD_VERSION "/sys/devices/platform/delta-ag9032v2a-cpld.0/cpld_ver"
#define IDPROM_PATH "/sys/class/i2c-adapter/i2c-2/2-0053/eeprom"
#define SWPLD1_PATH    "/sys/devices/platform/delta-ag9032v2a-swpld1.0"
#define SWPLD2_PATH "/sys/devices/platform/delta-ag9032v2a-swpld2.0"
#define FAN1_FRONT "/sys/bus/i2c/device/i2c-26/26-002c/fan1_input"
#define FAN1_REAR "/sys/bus/i2c/device/i2c-26/26-002d/fan1_input"
#define FAN2_FRONT "/sys/bus/i2c/device/i2c-26/26-002c/fan2_input"
#define FAN2_REAR "/sys/bus/i2c/device/i2c-26/26-002d/fan2_input"
#define FAN3_FRONT "/sys/bus/i2c/device/i2c-26/26-002c/fan3_input"
#define FAN3_REAR "/sys/bus/i2c/device/i2c-26/26-002d/fan3_input"
#define FAN4_FRONT "/sys/bus/i2c/device/i2c-26/26-002c/fan4_input"
#define FAN4_REAR "/sys/bus/i2c/device/i2c-26/26-002d/fan4_input"
#define FAN5_FRONT "/sys/bus/i2c/device/i2c-26/26-002c/fan5_input"
#define FAN5_REAR "/sys/bus/i2c/device/i2c-26/26-002d/fan5_input"
#define PORT_EEPROM_FORMAT      "/sys/bus/i2c/devices/%d-0050/eeprom"
#define SFP_SELECT_PORT_PATH    "/sys/devices/platform/delta-ag9032v2a-swpld1.0/sfp_select_port"
#define SFP_IS_PRESENT_PATH     "/sys/devices/platform/delta-ag9032v2a-swpld1.0/sfp_is_present"
#define SFP_IS_PRESENT_ALL_PATH "/sys/devices/platform/delta-ag9032v2a-swpld1.0/sfp_is_present_all"
#define QSFP_RESET_PATH         "/sys/devices/platform/delta-ag9032v2a-swpld1.0/sfp_reset"
#define QSFP_LP_MODE_PATH       "/sys/devices/platform/delta-ag9032v2a-swpld1.0/sfp_lp_mode"
#define PREFIX_PATH   "/sys/bus/i2c/devices/"
#define PSU1_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/4-0058/"
#define PSU2_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/5-0058/"
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node

/* REG define*/
#define SWPLD_1_ADDR (0x6A)
#define SWPLD_2_ADDR (0x73)
#define SWPLD_3_ADDR (0x75)
#define PSU_EEPROM   (0x50)
#define FAN_TRAY_1   (0x51)
#define FAN_TRAY_2   (0x52)
#define FAN_TRAY_3   (0x53)
#define FAN_TRAY_4   (0x54)
#define FAN_TRAY_5   (0x55)
#define FAN_IO_CTL   (0x27)
#define SYS_LED1_REGISTER (0x21)
#define SYS_LED3_REGISTER (0x23)
#define FAN_LED_REGISTER  (0x20)
#define POWER_STATUS_REGISTER (0x03)
#define DEFAULT_FLAG         (0x00)

/*SFP REG define*/
#define SFP_RESPOND_1     (0x0A)
#define SFP_RESPOND_2     (0x0B)
#define SFP_RESPOND_3     (0x0C)
#define SFP_RESPOND_4     (0x0D)


typedef struct mux_info_s
{
        uint8_t offset;
        uint8_t channel;
        char dev_data[10];
        uint32_t flags;

}mux_info_t;

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
int dni_i2c_read_attribute_binary(char *filename, char *buffer, int buf_size, int data_len);

int dni_fanpresent_info_get(int *r_data);
int dni_lock_cpld_write_attribute(char *cpld_path, int addr, int data);
int dni_lock_cpld_read_attribute(char *cpld_path, int addr);
int dni_fan_present(int id);
int dni_fan_speed_good();
int dni_i2c_read_attribute_string(char *filename, char *buffer, int buf_size, int data_len);
int dni_bmc_sensor_read(char *device_name, UINT4 *num, UINT4 multiplier);
int dni_psui_eeprom_info_get(char *r_data,char *device_name,int number);
int dni_bmc_check();
int dni_bmc_fanpresent_info_get(int *r_data);
int dni_i2c_lock_read( mux_info_t * mux_info, dev_info_t * dev_info);
int dni_i2c_lock_read_attribute(mux_info_t * mux_info, char * fullpath);
int dni_i2c_lock_write_attribute(mux_info_t * mux_info, char * data,char * fullpath);
int dni_psu_present(int *r_data);
int dni_bmc_data_get(int bus, int addr, int reg, int len, int *r_data);
void lockinit();
char dev_name[50][32];
float dev_sensor[50];

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_WIND_ON_ADAPTER_BOARD,
    THERMAL_1_ON_MAIN_BOARD,
    THERMAL_2_ON_MAIN_BOARD,
    THERMAL_3_ON_MAIN_BOARD,
    THERMAL_4_ON_CPU_BOARD,
    THERMAL_5_ON_PSU1,
    THERMAL_6_ON_PSU2
};

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


enum bus
{
     I2C_BUS_0 = 0,
     I2C_BUS_1,
     I2C_BUS_2,
     I2C_BUS_3,
     I2C_BUS_4,
     I2C_BUS_5,
     I2C_BUS_6,
     I2C_BUS_7,
     I2C_BUS_8,
     I2C_BUS_9,
     I2C_BUS_10,
     I2C_BUS_11,
     I2C_BUS_21 = 21,
     I2C_BUS_22,
     I2C_BUS_23,
     I2C_BUS_24,
     I2C_BUS_25,
     I2C_BUS_26,
     I2C_BUS_27,
     I2C_BUS_28,
};
#endif  /* __PLATFORM_LIB_H__ */

