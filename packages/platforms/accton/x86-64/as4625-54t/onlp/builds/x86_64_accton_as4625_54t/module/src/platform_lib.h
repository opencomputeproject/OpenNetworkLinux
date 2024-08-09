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
#include "x86_64_accton_as4625_54t_log.h"

#define CHASSIS_FAN_COUNT     3
#define CHASSIS_THERMAL_COUNT 6
#define CHASSIS_PSU_COUNT     2
#define CHASSIS_LED_COUNT     6

#define NUM_OF_THERMAL_PER_PSU 2

#define NUM_OF_CPU_CORES 4

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_NODE_MAX_INT_LEN  8
#define PSU_NODE_MAX_PATH_LEN 64

#define PSU1_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/8-0058/"
#define PSU2_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/9-0059/"

#define PSU1_AC_PMBUS_NODE(node) PSU1_AC_PMBUS_PREFIX#node
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node

#define PSU1_AC_EEPROM_PREFIX "/sys/bus/i2c/devices/8-0050/"
#define PSU2_AC_EEPROM_PREFIX "/sys/bus/i2c/devices/9-0051/"

#define CPLD_NODE_PATH    "/sys/bus/i2c/devices/i2c-0/0-0064/"

#define FAN_NODE_PATH     "/sys/devices/platform/as4625_fan/hwmon*"
#define FAN_NODE(node)    FAN_NODE_PATH#node

#define IDPROM_PATH "/sys/bus/i2c/devices/7-0051/eeprom"

int psu_pmbus_info_get(int id, char *node, int *value);

enum onlp_fan_dir {
	FAN_DIR_F2B,
	FAN_DIR_B2F,
	FAN_DIR_COUNT,
};

typedef enum psu_type {
	PSU_TYPE_UNKNOWN,
	PSU_TYPE_UP1K21R_1085G_F2B,
	PSU_TYPE_UPD1501SA_1179G_F2B,
	PSU_TYPE_UPD1501SA_1279G_B2F
} psu_type_t;

psu_type_t get_psu_type(int id, char* modelname, int modelname_len);
int get_psu_eeprom_str(int id, char *data_buf, int data_len, char *data_name);

enum onlp_fan_dir onlp_get_fan_dir(void);

#define AIM_FREE_IF_PTR(p) \
	do \
	{ \
		if (p) { \
			aim_free(p); \
			p = NULL; \
		} \
	} while (0)

#endif  /* __PLATFORM_LIB_H__ */
