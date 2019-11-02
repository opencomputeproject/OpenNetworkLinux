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
#include "x86_64_accton_as7926_40xke_log.h"

#define CHASSIS_FAN_COUNT     12
#define CHASSIS_THERMAL_COUNT 11
#define CHASSIS_LED_COUNT     4
#define CHASSIS_PSU_COUNT     3

#define PSU1_ID 1
#define PSU2_ID 2
#define PSU3_ID 3

#define PSU1_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/18-005a/"
#define PSU2_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/18-005b/"
#define PSU3_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/18-0059/"

#define PSU1_AC_PMBUS_NODE(node) PSU1_AC_PMBUS_PREFIX#node
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node
#define PSU3_AC_PMBUS_NODE(node) PSU3_AC_PMBUS_PREFIX#node


#define PSU1_AC_HWMON_PREFIX "/sys/bus/i2c/devices/18-0052/"
#define PSU2_AC_HWMON_PREFIX "/sys/bus/i2c/devices/18-0053/"
#define PSU3_AC_HWMON_PREFIX "/sys/bus/i2c/devices/18-0051/"


#define FAN_BOARD_PATH	"/sys/bus/i2c/devices/22-0066/"
#define FAN_NODE(node)	FAN_BOARD_PATH#node

#define IDPROM_PATH "/sys/bus/i2c/devices/0-0057/eeprom"

enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_PSU,
    LED_FAN,
    LED_DIAG,
    LED_LOC
};

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAINBOARD,  /* Main Board Bottom TMP432_1 Temp */
    THERMAL_2_ON_MAINBOARD,  /* Main Board Bottom TMP432_2 Temp */
    THERMAL_3_ON_MAINBOARD,  /* Main Board Bottom TMP432_3 Temp */
    THERMAL_4_ON_MAINBOARD,  /* Main Board Bottom LM75_1 Temp */
    THERMAL_5_ON_MAINBOARD,  /* Main Board Bottom LM75_2 Temp */
    THERMAL_6_ON_MAINBOARD,  /* Main Board Bottom LM75_3 Temp */
    THERMAL_7_ON_MAINBOARD,  /* Main Board Bottom LM75_4 Temp */
    THERMAL_8_ON_MAINBOARD,  /* QSFP-DD Board LM75_1 Temp */
    THERMAL_9_ON_MAINBOARD,  /* QSFP-DD Board LM75_2 Temp */ 
    THERMAL_10_ON_MAINBOARD, /* QSFP-DD Board LM753 Temp */ 
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
    THERMAL_1_ON_PSU3,
};

typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_AC_F2B,
    PSU_TYPE_AC_B2F
} psu_type_t;

psu_type_t psu_type_get(int id, char* modelname, int modelname_len);
int psu_serial_number_get(int id, char *serial, int serial_len);
int psu_ym2651y_pmbus_info_get(int id, char *node, int *value);
int psu_ym2651y_pmbus_info_set(int id, char *node, int value);
int sfpi_i2c_read(int bus, uint8_t addr, uint8_t offset, int size,
              uint8_t* rdata, uint32_t flags);
int sfpi_i2c_readb(int bus, uint8_t addr, uint8_t offset, uint32_t flags);

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
	#define DEBUG_PRINT(fmt, args...)                                        \
		printf("%s:%s[%d]: " fmt "\r\n", __FILE__, __FUNCTION__, __LINE__, ##args)
#else
	#define DEBUG_PRINT(fmt, args...)
#endif

#endif  /* __PLATFORM_LIB_H__ */

