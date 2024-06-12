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

#include "x86_64_amx3200_log.h"

#define CHASSIS_FAN_COUNT      10
#define CHASSIS_THERMAL_COUNT  9
#define SLED1_THERMAL_COUNT    1
#define SLED2_THERMAL_COUNT    1
#define CHASSIS_LED_COUNT      4
#define CHASSIS_PSU_COUNT      2
#if 1
#define NUM_OF_THERMAL_PER_PSU 3
#endif

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_SYSFS_FORMAT   "/sys/devices/platform/amx3200_psu.%d*psu%d_%s"
#define PSU_SYSFS_FORMAT_1 "/sys/devices/platform/amx3200_psu.%d/hwmon/hwmon%d/%s"
#define FAN_SYSFS_FORMAT   "/sys/devices/platform/amx3200_fan*"
#define FAN_SYSFS_FORMAT_1 "/sys/devices/platform/amx3200_fan/hwmon/hwmon%d/%s"
#define IDPROM_PATH        "/sys/bus/i2c/devices/1-0052/eeprom"
#define SLED_PRESENT_PATH  "/sys/bus/platform/devices/amx3200_sled/sled_%d_present"
#define SLED_POWER_GOOD_PATH "/sys/bus/platform/devices/amx3200_sled/sled_%d_all_power_good"

enum onlp_thermal_id {
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_MAIN_BROAD,
    THERMAL_5_ON_SLED1_LOCAL_BROAD,
    THERMAL_6_ON_SLED1_REMOTE_BROAD,
    THERMAL_7_ON_SLED2_LOCAL_BROAD,
    THERMAL_8_ON_SLED2_REMOTE_BROAD,
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
    LED_PSU,
    LED_FAN,
};

enum onlp_fan_dir {
    FAN_DIR_F2B,
    FAN_DIR_B2F,
    FAN_DIR_COUNT
};

typedef enum amx3200_platform_id {
    amx3200,
    PID_UNKNOWN
} amx3200_platform_id_t;

enum onlp_fan_dir onlp_get_fan_dir(int fid);
int onlp_get_psu_hwmon_idx(int pid);
int onlp_get_fan_hwmon_idx(void);
int onlp_sled_board_is_ready(int index);

#define AIM_FREE_IF_PTR(p) \
    do \
    { \
        if (p) { \
            aim_free(p); \
            p = NULL; \
        } \
    } while (0)

#endif  /* __PLATFORM_LIB_H__ */
