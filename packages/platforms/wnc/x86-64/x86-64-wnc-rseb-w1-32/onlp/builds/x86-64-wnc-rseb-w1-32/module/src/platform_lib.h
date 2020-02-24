/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2019 WNC Computing Technology Corporation.
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

#include <onlplib/file.h>
#include "x86_64_wnc_rseb_w1_32_log.h"

#define CHASSIS_FAN_COUNT       6 /* Fan 1~6 */
#define CHASSIS_THERMAL_COUNT   6 /* CPU Core, LM75-1, LM75-2, LM75-3, LM75-4, LM75-5 */
#define CHASSIS_PSU_COUNT       2
#define CHASSIS_LED_COUNT       8 /* Status, ID, Fan1 ~ Fan6 */

#define PSU1_ID 1
#define PSU2_ID 2

#define SYSFS_NODE_PATH_MAX_LEN  128

#define ONL_VER_NUM_MAX_STR_LEN  64
#define CPLD_VER_MAX_STR_LEN     20

#define GPIO_VALUE_MAX_STR_LEN 64

#define FAN_MIN_PERCENTAGE 50

#define PSU1_PMBUS_PREFIX "/sys/bus/i2c/devices/18-0058/hwmon/hwmon14/"
#define PSU2_PMBUS_PREFIX "/sys/bus/i2c/devices/19-0058/hwmon/hwmon15/"

#define PSU1_PMBUS_NODE(node) PSU1_PMBUS_PREFIX#node
#define PSU2_PMBUS_NODE(node) PSU2_PMBUS_PREFIX#node

#define PSU1_STATUS_PREFIX "/sys/bus/i2c/devices/7-0066/"
#define PSU2_STATUS_PREFIX "/sys/bus/i2c/devices/7-0066/"

/* IDPROM => ONIE EEPROM */
#define IDPROM_PATH "/sys/bus/i2c/devices/3-0054/eeprom"

/* Fan controlled by CPLD */
#define FAN_BOARD_PATH "/sys/bus/i2c/devices/7-0066/"
#define FAN_NODE(node) FAN_BOARD_PATH#node

typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_AC_F2B,
    PSU_TYPE_AC_B2F,
    PSU_TYPE_DC_F2B,
    PSU_TYPE_DC_B2F
} psu_type_t;

enum fan_id {
    FAN_1_ON_FAN_BOARD = 1,
    FAN_2_ON_FAN_BOARD,
    FAN_3_ON_FAN_BOARD,
    FAN_4_ON_FAN_BOARD,
    FAN_5_ON_FAN_BOARD,
    FAN_6_ON_FAN_BOARD,
    FAN_1_ON_PSU_1,
    FAN_1_ON_PSU_2,
};

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_CPU_BROAD,
    THERMAL_5_ON_FAN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
};

enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_STATUS,
    LED_ID,
    LED_FAN_1,
    LED_FAN_2,
    LED_FAN_3,
    LED_FAN_4,
    LED_FAN_5,
    LED_FAN_6,
};

int psu_pmbus_info_get(int id, char *node, int *value);
int psu_pmbus_info_set(int id, char *node, int value);
psu_type_t get_psu_type(int id, char* modelname, int modelname_len);
int onlp_file_read_int_hex(int* value, const char* node_path);

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(format, ...)   printf(format, __VA_ARGS__)
#else
    #define DEBUG_PRINT(format, ...)
#endif

#endif  /* __PLATFORM_LIB_H__ */

