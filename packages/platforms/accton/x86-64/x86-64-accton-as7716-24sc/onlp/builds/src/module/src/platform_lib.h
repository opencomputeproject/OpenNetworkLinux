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

#include "x86_64_accton_as7716_24sc_log.h"

#define CHASSIS_FAN_COUNT     4
#define CHASSIS_THERMAL_COUNT 7
#define CHASSIS_LED_COUNT     5
#define CHASSIS_PSU_COUNT     2

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU1_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/18-0058/"
#define PSU2_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/17-0059/"

#define PSU1_AC_PMBUS_NODE(node) PSU1_AC_PMBUS_PREFIX#node
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node

#define PSU1_AC_EEPROM_PREFIX "/sys/bus/i2c/devices/18-0050/"
#define PSU2_AC_EEPROM_PREFIX "/sys/bus/i2c/devices/17-0051/"

#define PSU1_AC_EEPROM_NODE(node) PSU1_AC_EEPROM_PREFIX#node
#define PSU2_AC_EEPROM_NODE(node) PSU2_AC_EEPROM_PREFIX#node

#define FAN_BOARD_PATH	"/sys/bus/i2c/devices/9-0066/"
#define FAN_NODE(node)	FAN_BOARD_PATH#node

#define IDPROM_PATH "/sys/bus/i2c/devices/0-0056/eeprom"

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
	THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_MAIN_BROAD,
    THERMAL_5_ON_MAIN_BROAD,
    THERMAL_6_ON_MAIN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
};

typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_AC_DPS850_F2B,
    PSU_TYPE_AC_DPS850_B2F
} psu_type_t;

typedef enum port_type {
	PORT_TYPE_Q28,
	PORT_TYPE_ACO_200G,
	PORT_TYPE_DCO_200G,
	PORT_TYPE_UNKNOWN
} port_type_t;

//int get_port_number(void);
port_type_t get_port_type(int port);
psu_type_t get_psu_type(int id, char* modelname, int modelname_len);
int get_psu_serial_number(int id, char *serial, int serial_len);
int psu_dps850_pmbus_info_get(int id, char *node, int *value);

struct dsp_ctrl_s
{
	char 		*type;
	unsigned int value1;
	unsigned int value2;
	unsigned int value3;
	unsigned int value4;
};

int dsp_initialize(int dsp_bus);
int dsp_exec_read(int bus, struct dsp_ctrl_s* dsp_ctrl, int *ret_val);
int cfp_eeprom_read(int port, uint8_t data[256]);

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
	#define DEBUG_PRINT(fmt, args...)                                        \
		printf("%s:%s[%d]: " fmt "\r\n", __FILE__, __FUNCTION__, __LINE__, ##args)
#else
	#define DEBUG_PRINT(fmt, args...)
#endif

#endif  /* __PLATFORM_LIB_H__ */

