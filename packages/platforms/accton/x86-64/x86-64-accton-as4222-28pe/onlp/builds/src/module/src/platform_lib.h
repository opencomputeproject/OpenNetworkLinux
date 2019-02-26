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
#include "x86_64_accton_as4222_28pe_log.h"

#define CHASSIS_FAN_COUNT		3
#define CHASSIS_THERMAL_COUNT	5
#define CHASSIS_PSU_COUNT		1
#define CHASSIS_LED_COUNT		4

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_NODE_MAX_INT_LEN  8
#define PSU_NODE_MAX_PATH_LEN 64


#define PSU1_AC_PMBUS_NODE(node) PSU1_AC_PMBUS_PREFIX#node
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node

#define PSU_PREFIX "/sys/bus/i2c/devices/1-0060/"


#define FAN_BOARD_PATH	"/sys/bus/i2c/devices/1-0060/"
#define FAN_NODE(node)	FAN_BOARD_PATH#node

#define IDPROM_PATH "/sys/bus/i2c/devices/1-0057/eeprom"

int onlp_file_write_integer(char *filename, int value);
int onlp_file_read_binary(char *filename, char *buffer, int buf_size, int data_len);
int onlp_file_read_string(char *filename, char *buffer, int buf_size, int data_len);

typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_ACBEL,
    PSU_TYPE_YM2651Y,
    PSU_TYPE_AC_F2B,
    PSU_TYPE_AC_B2F
} psu_type_t;


//#define DEBUG_MODE 1

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(format, ...)   printf(format, __VA_ARGS__)
#else
    #define DEBUG_PRINT(format, ...)  
#endif

#endif  /* __PLATFORM_LIB_H__ */

