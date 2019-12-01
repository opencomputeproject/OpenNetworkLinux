/************************************************************
 * <bsn.cl fy=2017 v=onl>
 *
 *        Copyright 2017 Delta Networks, Inc.
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

#include "x86_64_delta_agc7008s_log.h"
#include <onlp/onlp.h>
#include <onlplib/shlocks.h>

typedef unsigned int UINT4;

/* Vendor device list */
#define THERMAL_LIST_SIZE         11
#define FAN_LIST_SIZE              7
#define PSU_LIST_SIZE             13

#define VENDOR_MAX_NAME_SIZE      20
#define VENDOR_MAX_DATA_SIZE      20
#define VENDOR_MAX_PATH_SIZE      50
#define VENDOR_MAX_CMD_SIZE      120
#define VENDOR_MAX_BUF_SIZE       10
#define FIRST_DEV_INDEX            1

#define THERMAL_TIME_THRESHOLD    10
#define FAN_TIME_THRESHOLD         5
#define PSU_TIME_THRESHOLD         5
#define SWPLD_NUM                  2
#define SWPLD_DATA_TIME_THRESHOLD  5
#define PSU_EEPROM_TIME_THRESHOLD 10
#define PSU_EEPROM_NUM             2
#define PSU_EEPROM_TABLE_NUM       2

/* BMC BUS define */
#define BMC_BUS 3

/* CPLD number & peripherals */
#define NUM_OF_SFP_PORT              12
#define NUM_OF_QSFP_PORT              0
#define NUM_OF_ALL_PORT (NUM_OF_QSFP_PORT + NUM_OF_SFP_PORT)
#define NUM_OF_THERMAL_ON_MAIN_BROAD  6
#define NUM_OF_LED_ON_MAIN_BROAD      3
#define NUM_OF_FAN_ON_MAIN_BROAD      4
#define NUM_OF_PSU_ON_MAIN_BROAD      2
#define CHASSIS_FAN_COUNT             4
#define CHASSIS_THERMAL_COUNT         6
#define PSU_NUM_LENGTH               15

#define FAN_ZERO_RPM                960
#define MAX_FRONT_FAN_SPEED       23000
#define MAX_PSU_FAN_SPEED         18380
#define MAX_REAR_FAN_SPEED        20500

#define CPU_CPLD_VERSION "/sys/devices/platform/delta-agc7008s-cpld.0/cpu_cpld_ver"
#define IDPROM_PATH "/sys/class/i2c-adapter/i2c-0/0-0056/eeprom"
#define PORT_EEPROM_FORMAT "/sys/bus/i2c/devices/%d-0050/eeprom"
#define CHECK_TIME_FILE "/tmp/check_time_file"
#define BMC_INFO_TABLE "/tmp/bmc_info"
#define PREFIX_PATH   "/sys/bus/i2c/devices/"

#define CPLD_VERSION_OFFSET             4
#define ATTRIBUTE_BASE_DEC             10
#define ATTRIBUTE_BASE_HEX             16

/* REG define */
#define CPU_CPLD_ADDR                0x3d
#define SYSTEM_CPLD_ADDR             0x31
#define PORT_CPLD0_ADDR              0x32
#define EEPROM_ADDR                  0x56
#define MUX_ADDR                     0x71

#define LED_REGISTER                 0x28
#define PSU_PRESENT_REGISTER         0x21

#define LED_FRONT_FAN_MASK           0x30
#define LED_FRONT_FAN_MODE_GREEN     0x10
#define LED_FRONT_FAN_MODE_RED       0x20
#define LED_FRONT_FAN_MODE_RED_BLK   0x30

#define LED_FRONT_SYS_MASK           0x0C
#define LED_FRONT_SYS_MODE_GREEN     0x04
#define LED_FRONT_SYS_MODE_RED       0x08
#define LED_FRONT_SYS_MODE_GREEN_BLK 0x0C

#define LED_FRONT_PWR_MASK           0x03
#define LED_FRONT_PWR_MODE_GREEN     0x01
#define LED_FRONT_PWR_MODE_RED       0x02

/* REG on CPLD0 */
#define SFPP_PRESENT_REG             0x0B
#define SFP_PRESENT_REG              0x12
#define SFPP_RX_LOSS_REG             0x0D
#define SFP_RX_LOSS_REG              0x14
#define SFPP_TX_FAULT_REG            0x08
#define SFP_TX_FAULT_REG             0x0F
#define SFPP_TX_DISABLE_REG          0x0A
#define SFP_TX_DISABLE_REG           0x11

#define I2C_ACCESS    7

/* QSFP port map */
#define BUS_START_INDEX 20
#define FRONT_PORT_TO_BUS_INDEX(port) (port + BUS_START_INDEX - 5)
#define VALIDATE_PORT(p) {                \
    if ((p < 5) || (p > NUM_OF_ALL_PORT + 5)) \
        return ONLP_STATUS_E_PARAM;       \
}

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

typedef struct vendor_dev_s
{
    char dev_name[VENDOR_MAX_NAME_SIZE];
    unsigned int dev_type;
    int id;
    float data;
}vendor_dev_t;

typedef struct check_time_s
{
    long time;
}check_time_t;

typedef struct swpld_info_s
{
    char name[VENDOR_MAX_NAME_SIZE];
    uint8_t addr;
    long time;
}swpld_info_t;

typedef struct platform_info_s
{
    uint8_t data;
    long time;
}platform_info_t;

typedef struct eeprom_info_s
{
    char name[VENDOR_MAX_NAME_SIZE];
    char data[VENDOR_MAX_DATA_SIZE];
}eeprom_info_t;

typedef struct vendor_psu_dev_s
{
    eeprom_info_t psu_eeprom_table[2];
}vendor_psu_dev_t;

extern vendor_dev_t thermal_dev_list[];
extern vendor_dev_t fan_dev_list[];
extern vendor_dev_t psu_dev_list[];
extern vendor_psu_dev_t psu_eeprom_info_table[];

int dni_fan_present(int id);
int dni_bmc_sensor_read(char *device_name, UINT4 *num, UINT4 multiplier, int sensor_type);
int dni_bmc_psueeprom_info_get(char *r_data, char *device_name, int number);
int dni_bmc_fanpresent_info_get(uint8_t *fan_present_bit);
int dni_i2c_lock_read_attribute(char *fullpath, int base);
int dni_i2c_lock_read( mux_info_t * mux_info, dev_info_t * dev_info);
int dni_i2c_lock_write( mux_info_t * mux_info, dev_info_t * dev_info);
int dni_bmc_data_get(int bus, int addr, int reg, int *r_data);
int dni_bmc_data_set(int bus, int addr, int reg, uint8_t w_data);
void lockinit();

typedef enum
{
    THERMAL_RESERVED = 0,
    THERMAL_1_ON_MAIN_BOARD_TEMP,
    THERMAL_2_ON_MAIN_BOARD_TEMP,
    THERMAL_3_ON_MAIN_BOARD_TEMP,
    THERMAL_4_ON_MAIN_BOARD_TEMP,
    THERMAL_5_ON_MAIN_BOARD_TEMP,
    THERMAL_6_ON_MAIN_BOARD_TEMP,
    THERMAL_7_ON_PSU1,
    THERMAL_8_ON_PSU1,
    THERMAL_9_ON_PSU2,
    THERMAL_10_ON_PSU2,
}onlp_thermal_id;

typedef enum
{
    FAN_RESERVED = 0,
    FAN_1_ON_FAN_BOARD,
    FAN_2_ON_FAN_BOARD,
    FAN_3_ON_FAN_BOARD,
    FAN_4_ON_FAN_BOARD,
    FAN_1_ON_PSU1,
    FAN_1_ON_PSU2
}onlp_fan_id;

typedef enum
{
    LED_RESERVED = 0,
    LED_FRONT_FAN,
    LED_FRONT_SYS,
    LED_FRONT_PWR,
}onlp_led_id;

typedef enum
{
    PSU_RESERVED = 0,
    PSU1_ID,
    PSU2_ID
}onlp_psu_id;

typedef enum
{
    SENSOR_RESERVED = 0,
    THERMAL_SENSOR,
    FAN_SENSOR,
    PSU_SENSOR,
    LED_SENSOR
}onlp_dev_type;

#endif  /* __PLATFORM_LIB_H__ */

