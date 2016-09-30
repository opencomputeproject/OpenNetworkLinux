/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
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

#include "powerpc_accton_as5710_54x_log.h"
#include "powerpc_accton_as5710_54x_int.h"

#define PSU1_ID 1
#define PSU2_ID 2

#define CHASSIS_FAN_COUNT     5
#define CHASSIS_THERMAL_COUNT 5

#define PSU1_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/5-003c/"
#define PSU2_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/6-003f/"

#define PSU1_AC_HWMON_PREFIX "/sys/bus/i2c/devices/5-0038/"
#define PSU1_DC_HWMON_PREFIX "/sys/bus/i2c/devices/5-0050/"
#define PSU2_AC_HWMON_PREFIX "/sys/bus/i2c/devices/6-003b/"
#define PSU2_DC_HWMON_PREFIX "/sys/bus/i2c/devices/6-0053/"

#define PSU1_AC_HWMON_NODE(node) PSU1_AC_HWMON_PREFIX#node
#define PSU1_DC_HWMON_NODE(node) PSU1_DC_HWMON_PREFIX#node
#define PSU2_AC_HWMON_NODE(node) PSU2_AC_HWMON_PREFIX#node
#define PSU2_DC_HWMON_NODE(node) PSU2_DC_HWMON_PREFIX#node

#define SFP_HWMON_PREFIX "/sys/bus/i2c/devices/3-0050/"
#define SFP_HWMON_NODE(node)     SFP_HWMON_PREFIX#node
#define SFP_HWMON_DOM_PREFIX "/sys/bus/i2c/devices/3-0051/"
#define SFP_HWMON_DOM_NODE(node)     SFP_HWMON_DOM_PREFIX#node

int deviceNodeWriteInt(char *filename, int value, int data_len);
int deviceNodeReadBinary(char *filename, char *buffer, int buf_size, int data_len);
int deviceNodeReadString(char *filename, char *buffer, int buf_size, int data_len);

typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_AC_F2B,
    PSU_TYPE_AC_B2F,
    PSU_TYPE_DC_48V_F2B,
    PSU_TYPE_DC_48V_B2F
} psu_type_t;

psu_type_t get_psu_type(int id, char* modelname, int modelname_len);

extern platform_id_t platform_id;

#endif  /* __PLATFORM_LIB_H__ */
