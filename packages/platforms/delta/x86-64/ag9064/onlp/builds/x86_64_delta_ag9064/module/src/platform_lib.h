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

#include "x86_64_delta_ag9064_log.h"
#include <onlp/onlp.h>
#include <onlplib/shlocks.h>

typedef unsigned int UINT4;

/* Vendor device list */
#define THERMAL_LIST_SIZE          9
#define FAN_LIST_SIZE             11
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
#define SWPLD_NUM                  4
#define SWPLD_DATA_TIME_THRESHOLD  5
#define PSU_EEPROM_TIME_THRESHOLD 10
#define PSU_EEPROM_NUM             2
#define PSU_EEPROM_TABLE_NUM       2

/* BMC BUS define */
#define BMC_SWPLD_BUS 4

/* CPLD number & peripherals */
#define NUM_OF_SFP_PORT               0
#define NUM_OF_QSFP_PORT             64
#define NUM_OF_ALL_PORT (NUM_OF_QSFP_PORT + NUM_OF_SFP_PORT)
#define NUM_OF_THERMAL_ON_MAIN_BROAD  6
#define NUM_OF_LED_ON_MAIN_BROAD      8
#define NUM_OF_FAN_ON_MAIN_BROAD      8
#define NUM_OF_PSU_ON_MAIN_BROAD      2
#define CHASSIS_FAN_COUNT             8
#define CHASSIS_THERMAL_COUNT         6
#define PSU_NUM_LENGTH               15

#define FAN_ZERO_RPM                960
#define MAX_FRONT_FAN_SPEED       23000
#define MAX_PSU_FAN_SPEED         18380
#define MAX_REAR_FAN_SPEED        20500

#define CPU_CPLD_VERSION "/sys/devices/platform/delta-ag9064-cpld.0/cpld_ver"
#define IDPROM_PATH "/sys/class/i2c-adapter/i2c-0/0-0056/eeprom"
#define PORT_EEPROM_FORMAT "/sys/bus/i2c/devices/%d-0050/eeprom"
#define CHECK_TIME_FILE "/tmp/check_time_file"
#define BMC_INFO_TABLE "/tmp/bmc_info"
#define PREFIX_PATH   "/sys/bus/i2c/devices/"

#define CPLD_VERSION_OFFSET             4
#define ATTRIBUTE_BASE_DEC             10
#define ATTRIBUTE_BASE_HEX             16

/* REG define */
#define SWPLD_1_ADDR                 0x35
#define SWPLD_2_ADDR                 0x34
#define SWPLD_3_ADDR                 0x33
#define SWPLD_4_ADDR                 0x32

#define LED_FRONT_FAN_SYS_REGISTER   0x02
#define PSU_STATUS_REGISTER          0x02
#define LED_FAN_TRAY_REGISTER        0x1B

#define LED_FRONT_FAN_MASK           0xC0
#define LED_FRONT_FAN_MODE_GREEN     0x40
#define LED_FRONT_FAN_MODE_RED       0x80

#define LED_FRONT_SYS_MASK           0x30
#define LED_FRONT_SYS_MODE_GREEN     0x10
#define LED_FRONT_SYS_MODE_GREEN_BLK 0x20
#define LED_FRONT_SYS_MODE_RED       0x30

#define LED_FRONT_PSU1_PRESENT_MASK  0x80
#define LED_FRONT_PSU1_PWR_OK_MASK   0x40
#define LED_FRONT_PSU2_PRESENT_MASK  0x08
#define LED_FRONT_PSU2_PWR_OK_MASK   0x04
#define LED_FRONT_PSU_PRESENT        0x00
#define LED_FRONT_PSU_PWR_GOOD       0x00

#define LED_REAR_FAN1_MASK           0x80
#define LED_REAR_FAN1_MODE_GREEN     0x80
#define LED_REAR_FAN2_MASK           0x40
#define LED_REAR_FAN2_MODE_GREEN     0x40
#define LED_REAR_FAN3_MASK           0x20
#define LED_REAR_FAN3_MODE_GREEN     0x20
#define LED_REAR_FAN4_MASK           0x10
#define LED_REAR_FAN4_MODE_GREEN     0x10
#define LED_REAR_FAN_MODE_ORANGE     0x00

/* REG on SWPLD1 */
#define QSFP_1_TO_8_PRESENT_REG   0x03
#define QSFP_33_TO_40_PRESENT_REG 0x04
#define QSFP_1_TO_8_RESET_REG     0x06
#define QSFP_33_TO_40_RESET_REG   0x07
#define QSFP_1_TO_8_RESPOND_REG   0x09
#define QSFP_33_TO_40_RESPOND_REG 0x0A
#define QSFP_1_TO_8_LPMODE_REG    0x0C
#define QSFP_33_TO_40_LPMODE_REG  0x0D

/* REG on SWPLD2 */
#define QSFP_9_TO_16_PRESENT_REG  0x03
#define QSFP_41_TO_48_PRESENT_REG 0x04
#define QSFP_9_TO_16_RESET_REG    0x06
#define QSFP_41_TO_48_RESET_REG   0x07
#define QSFP_9_TO_16_RESPOND_REG  0x09
#define QSFP_41_TO_48_RESPOND_REG 0x0A
#define QSFP_9_TO_16_LPMODE_REG   0x0C
#define QSFP_41_TO_48_LPMODE_REG  0x0D

/* REG on SWPLD3 */
#define QSFP_25_TO_32_PRESENT_REG 0x24
#define QSFP_57_TO_64_PRESENT_REG 0x25
#define QSFP_25_TO_32_RESET_REG   0x26
#define QSFP_57_TO_64_RESET_REG   0x27
#define QSFP_25_TO_32_RESPOND_REG 0x28
#define QSFP_57_TO_64_RESPOND_REG 0x29
#define QSFP_25_TO_32_LPMODE_REG  0x2A
#define QSFP_57_TO_64_LPMODE_REG  0x2B

/* REG on SWPLD4 */
#define QSFP_17_TO_24_PRESENT_REG 0x24
#define QSFP_49_TO_56_PRESENT_REG 0x25
#define QSFP_17_TO_24_RESET_REG   0x26
#define QSFP_49_TO_56_RESET_REG   0x27
#define QSFP_17_TO_24_RESPOND_REG 0x28
#define QSFP_49_TO_56_RESPOND_REG 0x29
#define QSFP_17_TO_24_LPMODE_REG  0x2A
#define QSFP_49_TO_56_LPMODE_REG  0x2B

/* QSFP port map */
#define BUS_START_INDEX 20
#define QSFP_PORT_OFFSET 8
#define FRONT_PORT_TO_BUS_INDEX(port) (port + BUS_START_INDEX)
#define VALIDATE_PORT(p) {                \
    if ((p < 0) || (p > NUM_OF_ALL_PORT)) \
        return ONLP_STATUS_E_PARAM;       \
}

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
int dni_bmc_data_get(int bus, int addr, int reg, int *r_data);
int dni_bmc_data_set(int bus, int addr, int reg, uint8_t w_data);
void lockinit();

typedef enum
{
    THERMAL_RESERVED = 0,
    THERMAL_1_ON_CPU_BOARD,
    THERMAL_2_ON_FAN_BOARD,
    THERMAL_3_ON_MAIN_BOARD_TEMP_1,
    THERMAL_4_ON_MAIN_BOARD_TEMP_2,
    THERMAL_5_ON_MAIN_BOARD_TEMP_3,
    THERMAL_6_ON_MAIN_BOARD_TEMP_4,
    THERMAL_7_ON_PSU1,
    THERMAL_8_ON_PSU2
}onlp_thermal_id;

typedef enum
{
    FAN_RESERVED = 0,
    FAN_1_ON_FAN_BOARD,
    FAN_2_ON_FAN_BOARD,
    FAN_3_ON_FAN_BOARD,
    FAN_4_ON_FAN_BOARD,
    FAN_6_ON_FAN_BOARD,
    FAN_7_ON_FAN_BOARD,
    FAN_8_ON_FAN_BOARD,
    FAN_9_ON_FAN_BOARD,
    FAN_1_ON_PSU1,
    FAN_1_ON_PSU2
}onlp_fan_id;

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
    LED_REAR_FAN_TRAY_4
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

