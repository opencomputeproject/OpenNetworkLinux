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

#include "x86_64_accton_as7315_30x_log.h"

#define CHASSIS_FAN_COUNT      5
#define CHASSIS_THERMAL_COUNT  4
#define CHASSIS_LED_COUNT      6
#define CHASSIS_PSU_COUNT      2
#define NUM_OF_THERMAL_PER_PSU 2

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU1_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/45-005b/"
#define PSU2_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/44-0058/"
#define PSU1_AC_EEPROM_PREFIX "/sys/bus/i2c/devices/45-0053/"
#define PSU2_AC_EEPROM_PREFIX "/sys/bus/i2c/devices/44-0050/"

#define FAN_BOARD_PATH "/sys/bus/i2c/devices/41-0066/"

#define IDPROM_PATH "/sys/class/i2c-adapter/i2c-1/1-0056/eeprom"

enum onlp_thermal_id {
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_2_ON_PSU1,
    THERMAL_1_ON_PSU2,
    THERMAL_2_ON_PSU2,
};

enum onlp_led_id {
    LED_LOC = 1,
    LED_DIAG,
    LED_PSU1,
    LED_PSU2,
    LED_FAN,
    LED_ALARM
};

#endif  /* __PLATFORM_LIB_H__ */
