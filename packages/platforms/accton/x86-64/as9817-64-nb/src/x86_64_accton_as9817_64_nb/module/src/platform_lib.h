/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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

#include "x86_64_accton_as9817_64_nb_log.h"

#define CHASSIS_FAN_COUNT      8
#define CHASSIS_THERMAL_COUNT  9
#define CHASSIS_LED_COUNT      6
#define CHASSIS_PSU_COUNT      2
#define NUM_OF_THERMAL_PER_PSU 3

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_STATUS_SYSFS_FORMAT "/sys/bus/i2c/devices/0-0060/psu%d_%s"
#define PSU1_PMBUS_SYSFS_FORMAT "/sys/bus/i2c/devices/77-0058"
#define PSU2_PMBUS_SYSFS_FORMAT "/sys/bus/i2c/devices/77-0059"

#define FAN_SYSFS_FORMAT   "/sys/bus/i2c/devices/76-0033*"
#define FAN_SYSFS_FORMAT_1 "/sys/bus/i2c/devices/76-0033/hwmon/hwmon%d/%s"
#define SYS_LED_PATH   "/sys/devices/platform/as9817_64_led/"
#define IDPROM_PATH "/sys/bus/i2c/devices/68-0056/eeprom"


enum onlp_thermal_id {
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_MAIN_BROAD,
    THERMAL_5_ON_MAIN_BROAD,
    THERMAL_6_ON_MAIN_BROAD,
    THERMAL_7_ON_MAIN_BROAD,
    THERMAL_8_ON_MAIN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_2_ON_PSU1,
    THERMAL_3_ON_PSU1,
    THERMAL_1_ON_PSU2,
    THERMAL_2_ON_PSU2,
    THERMAL_3_ON_PSU2,
    THERMAL_COUNT
};

enum onlp_led_id {
    LED_LOC = 1,
    LED_DIAG,
    LED_PSU1,
    LED_PSU2,
    LED_FAN,
    LED_ALARM
};

enum onlp_fan_dir {
    FAN_DIR_F2B,
    FAN_DIR_B2F,
    FAN_DIR_COUNT
};

typedef enum as9817_64_platform_id {
    AS9817_64O,
    AS9817_64D,
    PID_UNKNOWN
} as9817_64_platform_id_t;

int psu_pmbus_info_get(int id, char *node, int *value);
int psu_pmbus_info_set(int id, char *node, int value);
int psu_pmbus_str_get(int id, char *data_buf, int data_len, char *data_name);
int onlp_get_psu_hwmon_idx(int pid);
int onlp_get_fan_hwmon_idx(void);
enum onlp_fan_dir onlp_get_fan_dir(int fid);
as9817_64_platform_id_t get_platform_id(void);

#define AIM_FREE_IF_PTR(p) \
    do \
    { \
        if (p) { \
            aim_free(p); \
            p = NULL; \
        } \
    } while (0)

#endif  /* __PLATFORM_LIB_H__ */
