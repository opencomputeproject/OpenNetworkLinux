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

#include "x86_64_inventec_d5254_log.h"

/* This is definitions for x86-64-inventec-d5254*/
/* OID map*/
/*
 *  SYS---------ONLP_THERMAL_CPU_PHY
 *         |----ONLP_THERMAL_CPU_CORE0
 *         |----ONLP_THERMAL_CPU_CORE1
 *         |----ONLP_THERMAL_CPU_CORE2
 *         |----ONLP_THERMAL_CPU_CORE3
 *         |----ONLP_THERMAL_1_ON_MAIN_BROAD
 *         |----ONLP_THERMAL_2_ON_MAIN_BROAD
 *         |----ONLP_THERMAL_3_ON_MAIN_BROAD
 *         |----ONLP_THERMAL_4_ON_MAIN_BROAD
 *         |----ONLP_THERMAL_5_ON_MAIN_BROAD
 *         |----ONLP_FAN_1--------ONLP_FAN_1_WEAK
 *         |                   |--ONLP_LED_FAN1_GREEN
 *         |                   |--ONLP_LED_FAN1_RED
 *         |
 *         |----ONLP_FAN_2--------ONLP_FAN_2_WEAK
 *         |                   |--ONLP_LED_FAN2_GREEN
 *         |                   |--ONLP_LED_FAN2_RED
 *         |
 *         |----ONLP_FAN_3--------ONLP_FAN_3_WEAK
 *         |                   |--ONLP_LED_FAN3_GREEN
 *         |                   |--ONLP_LED_FAN3_RED
 *         |
 *         |----ONLP_FAN_4--------ONLP_FAN_4_WEAK
 *         |                   |--ONLP_LED_FAN4_GREEN
 *         |                   |--ONLP_LED_FAN4_RED
 *         |
 *         |----ONLP_FAN_5--------ONLP_FAN_5_WEAK
 *         |                   |--ONLP_LED_FAN5_GREEN
 *         |                   |--ONLP_LED_FAN5_RED
 *         |
 *         |----ONLP_PSU_1--------ONLP_THERMAL_1_ON_PSU1
 *         |                   |--ONLP_THERMAL_2_ON_PSU1
 *         |                   |--ONLP_FAN_PSU_1
 *         |
 *         |----ONLP_PSU_2--------ONLP_THERMAL_1_ON_PSU2
 *         |                   |--ONLP_THERMAL_2_ON_PSU2
 *         |                   |--ONLP_FAN_PSU_2
 *         |
 *         |----ONLP_LED_MGMT_GREEN
 *         |----ONLP_LED_MGMT_RED
 */

/* Thermal definitions*/
enum onlp_thermal_id {
    ONLP_THERMAL_RESERVED = 0,
    ONLP_THERMAL_CPU_PHY,
    ONLP_THERMAL_CPU_CORE0,
    ONLP_THERMAL_CPU_CORE1,
    ONLP_THERMAL_CPU_CORE2,
    ONLP_THERMAL_CPU_CORE3,
    ONLP_THERMAL_1_ON_MAIN_BROAD,
    ONLP_THERMAL_2_ON_MAIN_BROAD,
    ONLP_THERMAL_3_ON_MAIN_BROAD,
    ONLP_THERMAL_4_ON_MAIN_BROAD,
    ONLP_THERMAL_5_ON_MAIN_BROAD,
    ONLP_THERMAL_1_ON_PSU1,
    ONLP_THERMAL_2_ON_PSU1,
    ONLP_THERMAL_1_ON_PSU2,
    ONLP_THERMAL_2_ON_PSU2,
    ONLP_THERMAL_MAX
};

#define ONLP_THERMAL_COUNT 15 /*include "reserved"*/

/* Fan definitions*/
enum onlp_fan_id {
    ONLP_FAN_RESERVED = 0,
    ONLP_FAN_1,
    ONLP_FAN_2,
    ONLP_FAN_3,
    ONLP_FAN_4,
    ONLP_FAN_5,
    ONLP_FAN_1_WEAK,
    ONLP_FAN_2_WEAK,
    ONLP_FAN_3_WEAK,
    ONLP_FAN_4_WEAK,
    ONLP_FAN_5_WEAK,
    ONLP_FAN_PSU_1,
    ONLP_FAN_PSU_2,
    ONLP_FAN_MAX
};

#define ONLP_FAN_COUNT 13 /*include "reserved"*/

/* PSU definitions*/
enum onlp_psu_id {
    ONLP_PSU_RESERVED,
    ONLP_PSU_1,
    ONLP_PSU_2,
    ONLP_PSU_MAX
};

#define ONLP_PSU_COUNT 3 /*include "reserved"*/

/* LED definitions*/
enum onlp_led_id {
    ONLP_LED_RESERVED = 0,
    ONLP_LED_MGMT_GREEN,
    ONLP_LED_MGMT_RED,
    ONLP_LED_FAN1_GREEN,
    ONLP_LED_FAN1_RED,
    ONLP_LED_FAN2_GREEN,
    ONLP_LED_FAN2_RED,
    ONLP_LED_FAN3_GREEN,
    ONLP_LED_FAN3_RED,
    ONLP_LED_FAN4_GREEN,
    ONLP_LED_FAN4_RED,
    ONLP_LED_FAN5_GREEN,
    ONLP_LED_FAN5_RED,
    ONLP_LED_MAX
};

#define ONLP_LED_COUNT 13 /*include "reserved"*/


/* platform functions*/
#define PLATFORM_PSOC_DIAG_PATH "/sys/class/hwmon/hwmon1/device/diag"
#define PLATFORM_PSOC_DIAG_LOCK platform_psoc_diag_enable_write(0)
#define PLATFORM_PSOC_DIAG_UNLOCK platform_psoc_diag_enable_write(1)
int platform_psoc_diag_enable_read(int *enable);
int platform_psoc_diag_enable_write(int enable);




#endif  /* __PLATFORM_LIB_H__ */

