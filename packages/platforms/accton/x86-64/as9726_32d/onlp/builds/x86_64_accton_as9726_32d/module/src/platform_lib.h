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
#include "x86_64_accton_as9726_32d_log.h"

#define CHASSIS_FAN_COUNT		6
#define CHASSIS_THERMAL_COUNT		7
#define CHASSIS_PSU_COUNT		2
#define CHASSIS_LED_COUNT		5

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_NODE_MAX_INT_LEN  8
#define PSU_NODE_MAX_PATH_LEN 64

#define PSU1_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/9-0058/"
#define PSU2_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/9-0059/"

#define PSU1_AC_PMBUS_NODE(node) PSU1_AC_PMBUS_PREFIX#node
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node

#define PSU1_AC_HWMON_PREFIX "/sys/bus/i2c/devices/9-0050/"
#define PSU2_AC_HWMON_PREFIX "/sys/bus/i2c/devices/9-0051/"


#define PSU1_AC_HWMON_NODE(node) PSU1_AC_HWMON_PREFIX#node
#define PSU2_AC_HWMON_NODE(node) PSU2_AC_HWMON_PREFIX#node

#define FAN_BOARD_PATH	"/sys/bus/i2c/devices/14-0066/"
#define FAN_NODE(node)	FAN_BOARD_PATH#node

#define IDPROM_PATH "/sys/bus/i2c/devices/13-0056/eeprom"

int psu_pmbus_info_get(int id, char *node, int *value);
int psu_ym2651y_pmbus_info_set(int id, char *node, int value);

typedef enum psu_type {
        PSU_TYPE_UNKNOWN,
        PSU_TYPE_AC_ACBEL_F2B,
        PSU_TYPE_AC_ACBEL_B2F,
        PSU_TYPE_AC_BELPOWER_F2B,
        PSU_TYPE_AC_BELPOWER_B2F,
        PSU_TYPE_DC_48V_ACBEL_F2B,
        PSU_TYPE_DC_48V_ACBEL_B2F,
} psu_type_t;

enum onlp_fan_dir {
        FAN_DIR_F2B,
        FAN_DIR_B2F,
        FAN_DIR_COUNT,
};

enum onlp_fan_dir onlp_get_fan_dir(void);

psu_type_t get_psu_type(int id, char* modelname, int modelname_len);
int psu_serial_number_get(int id, char *serial, int serial_len);

//#define DEBUG_MODE 1

#if (DEBUG_MODE == 1)
	#define DEBUG_PRINT(format, ...)   printf(format, __VA_ARGS__)
#else
	#define DEBUG_PRINT(format, ...)  
#endif

#endif  /* __PLATFORM_LIB_H__ */
