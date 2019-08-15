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

#include "x86_64_delta_ag9032v2_log.h"

typedef unsigned int    UINT4;

/* CPLD numbrt & peripherals */
#define NUM_OF_THERMAL_ON_MAIN_BROAD (6)
#define NUM_OF_LED_ON_MAIN_BROAD     (9)
#define NUM_OF_FAN_ON_MAIN_BROAD     (5)
#define NUM_OF_PSU_ON_MAIN_BROAD     (2)
#define NUM_OF_SENSORS               (47)
#define CHASSIS_FAN_COUNT            (5)
#define CHASSIS_THERMAL_COUNT        (6)
#define I2C_BUS_1         (1)
#define PSU1_ID           (1)
#define PSU2_ID           (2)
#define NUM_OF_SFP_PORT   (2)
#define NUM_OF_QSFP_PORT  (32)
#define NUM_OF_ALL_PORT   (34)
#define PSU_NUM_LENGTH    (15)
#define UPDATE_THRESHOLD  (2) //second
#define MAX_FAN_SPEED     (23000)
#define IDPROM_PATH "/sys/class/i2c-adapter/i2c-1/1-0053/eeprom"

/* REG define*/
#define SWPLD_1_ADDR         (0x6A)
#define SWPLD_2_ADDR         (0x73)
#define SWPLD_3_ADDR         (0x75)
#define DEFAULT_FLAG         (0x00)
#define CPUCPLD              (0x31)
#define CPUPLD_VERSION_ADDR  (0x01)
#define SWPLD                (0x31)
#define SWPLD_VERSION_ADDR   (0x01)
#define DEFAULT_FLAG         (0x00)
#define POWER_STAT_REGISTER  (0x03)
#define SYS_LED1_REGISTER    (0x21)
#define SYS_FANLED2_REGISTER (0x20)
#define SYS_FANLED1_REGISTER (0x23)
#define PSU_I2C_SEL_PSU1_EEPROM (0x00)
#define PSU_I2C_SEL_PSU2_EEPROM (0x20)
#define PSU_FAN_MUX_REG         (0x1E)
#define PSU_MODEL_REG           (0x9A)
#define PSU_SERIAL_REG          (0x9E)

/*SFP REG define*/
#define SFP_SIGNAL_REG    (0x02)
#define SFP_MODULE_EEPROM (0x50)
#define SFP_I2C_MUX_REG   (0x1F)
#define SFP_RESET_1       (0x16)
#define SFP_RESET_2       (0x17)
#define SFP_RESET_3       (0x18)
#define SFP_RESET_4       (0x19)
#define SFP_LP_MODE_1     (0x0E)
#define SFP_LP_MODE_2     (0x0F)
#define SFP_LP_MODE_3     (0x10)
#define SFP_LP_MODE_4     (0x11)
#define SFP_RESPOND_1     (0x0A)
#define SFP_RESPOND_2     (0x0B)
#define SFP_RESPOND_3     (0x0C)
#define SFP_RESPOND_4     (0x0D)
#define SFP_PRESENT_1     (0x12)
#define SFP_PRESENT_2     (0x13)
#define SFP_PRESENT_3     (0x14)
#define SFP_PRESENT_4     (0x15)

int dni_get_bmc_data(char *device_name, UINT4 *num, UINT4 multiplier);
int dni_fanpresent_info_get(int *r_data);
int dni_psui_eeprom_info_get(char *r_data, int psu_id, int psu_reg);

char dev_name[50][32];
float dev_sensor[50];

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

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_CPU_BOARD,
    THERMAL_2_ON_FAN_BOARD,
    THERMAL_3_ON_MAIN_BOARD,
    THERMAL_4_ON_MAIN_BOARD,
    THERMAL_5_ON_MAIN_BOARD,
    THERMAL_6_ON_PSU1,
    THERMAL_7_ON_PSU2
};

typedef enum
{
    FAN_RESERVED = 0,
    FAN_1_ON_FAN_BOARD,
    FAN_2_ON_FAN_BOARD,
    FAN_3_ON_FAN_BOARD,
    FAN_4_ON_FAN_BOARD,
    FAN_5_ON_FAN_BOARD,
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

