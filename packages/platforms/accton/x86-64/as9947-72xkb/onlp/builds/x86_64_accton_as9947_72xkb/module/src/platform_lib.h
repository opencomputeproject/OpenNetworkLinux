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

#include "x86_64_accton_as9947_72xkb_log.h"

#define CHASSIS_FAN_COUNT      16
#define CHASSIS_THERMAL_COUNT  11
#define CHASSIS_LED_COUNT      6
#define CHASSIS_PSU_COUNT      2
#define NUM_OF_THERMAL_PER_PSU 3

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_STATUS_PRESENT 1
#define PSU_STATUS_POWER_GOOD 1

#define LED_FORMAT "/sys/devices/platform/as9947_72xkb_led/led_%s"
#define PSU_SYSFS_FORMAT   "/sys/devices/platform/as9947_72xkb_psu*psu%d_%s"
#define PSU_SYSFS_FORMAT_1 "/sys/devices/platform/as9947_72xkb_psu/hwmon/hwmon%d/%s"
#define FAN_SYSFS_FORMAT   "/sys/devices/platform/as9947_72xkb_fan*"
#define FAN_SYSFS_FORMAT_1 "/sys/devices/platform/as9947_72xkb_fan/hwmon/hwmon%d/%s"
#define SYS_LED_PATH   "/sys/devices/platform/as9947_72xkb_led/"
#define IDPROM_PATH "/sys/bus/platform/devices/as9947_72xkb_sys/eeprom"

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
    THERMAL_1_ON_FAN_BROAD,
    THERMAL_2_ON_FAN_BROAD,
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
    LED_ALARM,
    LED_DIAG,
    LED_PSU1,
    LED_PSU2,
    LED_FAN
};

enum onlp_fan_dir {
    FAN_DIR_F2B,
    FAN_DIR_B2F,
    FAN_DIR_COUNT
};

typedef enum as9947_72xkb_platform_id {
    as9947_72xkb,
    PID_UNKNOWN
} as9947_72xkb_platform_id_t;

enum onlp_fan_dir onlp_get_fan_dir(int fid);
int onlp_get_psu_hwmon_idx(int pid);
int onlp_get_fan_hwmon_idx(void);
int psu_status_info_get(int pid, char *node, int *value);
int psu_status_string_get(int pid, char *node, char **string);

#define AIM_FREE_IF_PTR(p) \
    do \
    { \
        if (p) { \
            aim_free(p); \
            p = NULL; \
        } \
    } while (0)

#endif  /* __PLATFORM_LIB_H__ */
