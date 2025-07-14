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

#include "x86_64_accton_as7515_24x_log.h"

#define CHASSIS_FAN_COUNT      5
#define CHASSIS_THERMAL_COUNT  7
#define CHASSIS_LED_COUNT      5
#define CHASSIS_PSU_COUNT      2
#define NUM_OF_THERMAL_PER_PSU 2

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_STATUS_PRESENT 1
#define PSU_STATUS_POWER_GOOD 1

#define PSU1_EEPROM_SYSFS_FORMAT "/sys/bus/i2c/devices/6-0052"
#define PSU2_EEPROM_SYSFS_FORMAT "/sys/bus/i2c/devices/7-0050"
#define PSU1_PMBUS_SYSFS_FORMAT "/sys/bus/i2c/devices/6-005a"
#define PSU2_PMBUS_SYSFS_FORMAT "/sys/bus/i2c/devices/7-0058"

#define FAN_SYSFS_FORMAT   "/sys/bus/i2c/devices/8-0066*fan%d_%s"
#define FAN_SYSFS_FORMAT_1 "/sys/bus/i2c/devices/8-0066/hwmon/hwmon%d/%s"
#define SYS_LED_FORMAT   "/sys/devices/platform/as7515_24x_led/led_%s"
#define IDPROM_PATH "/sys/bus/i2c/devices/4-0056/eeprom"

enum onlp_thermal_id {
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_MAIN_BROAD,
    THERMAL_5_ON_MAIN_BROAD,
    THERMAL_6_ON_MAIN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_2_ON_PSU1,
    THERMAL_1_ON_PSU2,
    THERMAL_2_ON_PSU2,
    THERMAL_COUNT
};

enum onlp_led_id {
    LED_LOC = 1,
    LED_DIAG,
    LED_PSU1,
    LED_PSU2,
    LED_FAN
};

typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_SPAACTN_03,
    PSU_TYPE_CRXT_T0T12
} psu_type_t;

int psu_cpld_status_get(int pid, char *node, int *value);
int psu_pmbus_info_get(int id, char *node, int *value);
int psu_eeprom_str_get(int pid, char *data_buf, int data_len, char *data_name);
int onlp_get_psu_hwmon_idx(int pid);
int onlp_get_fan_hwmon_idx(void);
int fan_info_get(int fid, char *node, int *value);
psu_type_t get_psu_type(int id, char* modelname, int modelname_len);

#define AIM_FREE_IF_PTR(p) \
    do \
    { \
        if (p) { \
            aim_free(p); \
            p = NULL; \
        } \
    } while (0)

#endif  /* __PLATFORM_LIB_H__ */
