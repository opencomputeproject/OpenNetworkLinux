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
#include "x86_64_accton_as4630_54te_log.h"

#define CHASSIS_FAN_COUNT     3
#define CHASSIS_THERMAL_COUNT 4
#define CHASSIS_PSU_COUNT     2
#define CHASSIS_LED_COUNT     8

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_NODE_MAX_INT_LEN  8
#define PSU_NODE_MAX_PATH_LEN 64

#define PSU1_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/10-0058/"
#define PSU2_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/11-0059/"

#define PSU1_AC_PMBUS_NODE(node) PSU1_AC_PMBUS_PREFIX#node
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node

#define PSU1_AC_EEPROM_PREFIX "/sys/bus/i2c/devices/10-0050/"
#define PSU2_AC_EEPROM_PREFIX "/sys/bus/i2c/devices/11-0051/"

#define CPLD_NODE_PATH    "/sys/bus/i2c/devices/i2c-3/3-0060/"
#define FAN_NODE(node)    CPLD_NODE_PATH#node

#define IDPROM_PATH "/sys/bus/i2c/devices/1-0057/eeprom"

int psu_pmbus_info_get(int id, char *node, int *value);

typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_YM1151D_F2B,
    PSU_TYPE_YM1151D_B2F,
    PSU_TYPE_YM1151F_F2B
} psu_type_t;

psu_type_t get_psu_type(int id, char* modelname, int modelname_len);
int get_psu_eeprom_str(int id, char *data_buf, int data_len, char *data_name);

#define AIM_FREE_IF_PTR(p) \
    do \
    { \
        if (p) { \
            aim_free(p); \
            p = NULL; \
        } \
    } while (0)

#endif  /* __PLATFORM_LIB_H__ */

