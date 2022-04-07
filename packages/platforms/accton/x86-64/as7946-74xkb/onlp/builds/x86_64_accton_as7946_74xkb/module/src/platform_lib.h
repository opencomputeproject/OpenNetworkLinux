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

#include <onlplib/file.h>
#include "x86_64_accton_as7946_74xkb_log.h"

#define CHASSIS_FAN_COUNT     5
#define CHASSIS_THERMAL_COUNT 10
#define CHASSIS_PSU_THERMAL_COUNT 3
#define CHASSIS_LED_COUNT     5
#define CHASSIS_PSU_COUNT     2

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_SYSFS_PATH "/sys/devices/platform/as7946_74xkb_psu/"
#define FAN_BOARD_PATH "/sys/devices/platform/as7946_74xkb_fan/"
#define IDPROM_PATH    "/sys/devices/platform/as7946_74xkb_sys/eeprom"

enum onlp_led_id {
    LED_LOC = 1,
    LED_DIAG,
    LED_PSU1,
    LED_PSU2,
    LED_FAN,
};

enum onlp_thermal_id {
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_CPU_MODULE, /* LM75_4B Temp */
    THERMAL_1_ON_MAINBOARD,  /* TMP432_4C Temp */
    THERMAL_2_ON_MAINBOARD,  /* TMP432_4C Temp */
    THERMAL_3_ON_MAINBOARD,  /* LM75_4A Temp */
    THERMAL_4_ON_MAINBOARD,  /* LM75_4D Temp */
    THERMAL_5_ON_MAINBOARD,  /* LM75_4E Temp */
    THERMAL_6_ON_MAINBOARD,  /* LM75_4F Temp */
    THERMAL_1_ON_FANCPLD,    /* FAN_4D Temp */
    THERMAL_2_ON_FANCPLD,    /* FAN_4E Temp */
    THERMAL_1_ON_PSU1,
    THERMAL_2_ON_PSU1,
    THERMAL_3_ON_PSU1,
    THERMAL_1_ON_PSU2,
    THERMAL_2_ON_PSU2,
    THERMAL_3_ON_PSU2,
};

#endif  /* __PLATFORM_LIB_H__ */
