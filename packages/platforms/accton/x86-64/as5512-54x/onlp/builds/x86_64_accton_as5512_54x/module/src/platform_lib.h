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

#include "x86_64_accton_as5512_54x_log.h"

#define PSU1_ID 1
#define PSU2_ID 2

#define CHASSIS_FAN_COUNT     5
#define CHASSIS_THERMAL_COUNT 4

#define PSU1_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/59-003c/"
#define PSU2_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/60-003f/"

#define PSU1_AC_HWMON_PREFIX "/sys/bus/i2c/devices/59-0038/"
#define PSU1_DC_HWMON_PREFIX "/sys/bus/i2c/devices/59-0050/"
#define PSU2_AC_HWMON_PREFIX "/sys/bus/i2c/devices/60-003b/"
#define PSU2_DC_HWMON_PREFIX "/sys/bus/i2c/devices/60-0053/"

#define PSU1_AC_HWMON_NODE(node) PSU1_AC_HWMON_PREFIX#node
#define PSU1_DC_HWMON_NODE(node) PSU1_DC_HWMON_PREFIX#node
#define PSU2_AC_HWMON_NODE(node) PSU2_AC_HWMON_PREFIX#node
#define PSU2_DC_HWMON_NODE(node) PSU2_DC_HWMON_PREFIX#node

#define IDPROM_PATH "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/1-0057/eeprom"

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

#endif  /* __PLATFORM_LIB_H__ */
