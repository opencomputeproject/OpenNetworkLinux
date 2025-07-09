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

#include <unistd.h>
#include "x86_64_accton_dcg8510_32d_log.h"


#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(fmt, args...)                                        \
        printf("%s:%s[%d]: " fmt "\r\n", __FILE__, __FUNCTION__, __LINE__, ##args)
#else
    #define DEBUG_PRINT(fmt, args...)
#endif

#define CHASSIS_FAN_COUNT     12
#define CHASSIS_THERMAL_COUNT 19
#define CHASSIS_LED_COUNT     4
#define CHASSIS_PSU_COUNT     2

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_MAC_PIPE=0,
    THERMAL_CPU_CORE_0=1,
    THERMAL_CPU_CORE_1,
    THERMAL_CPU_CORE_2,
    THERMAL_CPU_CORE_3,
    THERMAL_CPU_PACKAGE,
    THERMAL_TMP75_48,
    THERMAL_TMP75_49,
    THERMAL_TMP75_4A,
    THERMAL_TMP75_4B,
    THERMAL_1_TMP431_4C,
    THERMAL_2_TMP431_4C,
    THERMAL_TMP75_4E,
    THERMAL_TMP75_4D,
    THERMAL_TMP275_4E,
    THERMAL_VCORE_MAC,
    THERMAL_VDDT0V9_R_MAC,
    THERMAL_VDD1V5_1V8_MAC,
    THERMAL_VCC3V3_QSFP_AB,
    THERMAL_VCC3V3_QSFP_CD,
    THERMAL_PSU1_TEMP_1,
    THERMAL_PSU1_TEMP_2,
    THERMAL_PSU2_TEMP_1,
    THERMAL_PSU2_TEMP_2,
};

enum onlp_fan_id {
    FAN_1_MOTOR_1_ON_FAN_BOARD = 1,
    FAN_2_MOTOR_1_ON_FAN_BOARD,
    FAN_3_MOTOR_1_ON_FAN_BOARD,
    FAN_4_MOTOR_1_ON_FAN_BOARD,
    FAN_5_MOTOR_1_ON_FAN_BOARD,
    FAN_6_MOTOR_1_ON_FAN_BOARD,
    FAN_1_MOTOR_2_ON_FAN_BOARD,
    FAN_2_MOTOR_2_ON_FAN_BOARD,
    FAN_3_MOTOR_2_ON_FAN_BOARD,
    FAN_4_MOTOR_2_ON_FAN_BOARD,
    FAN_5_MOTOR_2_ON_FAN_BOARD,
    FAN_6_MOTOR_2_ON_FAN_BOARD,
    FAN_ON_PSU1,
    FAN_ON_PSU2,
};


int onlp_file_read_hex(int* value, const char* fmt, ...);
int onlp_file_write_hex(int value, const char* fmt, ...);
int onlp_file_readb(int* value,int offset, const char* fmt, ...);
int onlp_file_readw(int* value,int offset, const char* fmt, ...);
int onlp_file_writeb(int value,int offset, const char* fmt, ...);
int onlp_file_writew(int value,int offset, const char* fmt, ...);
#endif  /* __PLATFORM_LIB_H__ */
