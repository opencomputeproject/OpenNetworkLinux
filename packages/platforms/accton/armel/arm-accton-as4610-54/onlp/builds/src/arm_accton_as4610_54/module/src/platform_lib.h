/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2015 Accton Technology Corporation.
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

#include "arm_accton_as4610_54_log.h"

#define CHASSIS_THERMAL_COUNT 1 
#define CHASSIS_PSU_COUNT     2 

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU1_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/8-0058/"
#define PSU2_AC_PMBUS_PREFIX "/sys/bus/i2c/devices/8-0059/"

#define PSU1_AC_PMBUS_NODE(node) PSU1_AC_PMBUS_PREFIX#node
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node

#define PSU1_AC_EEPROM_PREFIX "/sys/bus/i2c/devices/8-0050/"
#define PSU2_AC_EEPROM_PREFIX "/sys/bus/i2c/devices/8-0051/"

#define PSU1_AC_EEPROM_NODE(node) PSU1_AC_EEPROM_PREFIX#node
#define PSU2_AC_EEPROM_NODE(node) PSU2_AC_EEPROM_PREFIX#node

#define IDPROM_PATH "/sys/devices/1803b000.i2c/i2c-1/i2c-9/9-0050/eeprom"

int deviceNodeWriteInt(char *filename, int value, int data_len);
int deviceNodeReadBinary(char *filename, char *buffer, int buf_size, int data_len);
int deviceNodeReadString(char *filename, char *buffer, int buf_size, int data_len);

typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_AC_F2B,
    PSU_TYPE_AC_B2F
} psu_type_t;

psu_type_t get_psu_type(int id, char* modelname, int modelname_len);

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(format, ...)   printf(format, __VA_ARGS__)
#else
    #define DEBUG_PRINT(format, ...)
#endif

enum onlp_fan_duty_cycle_percentage
{
    FAN_PERCENTAGE_LOW  = 13,
    FAN_PERCENTAGE_HIGH = 75
};

enum as4610_product_id {
	PID_AS4610_30T = 0,
	PID_AS4610_30P,
	PID_AS4610_54T,
	PID_AS4610_54P,
	PID_UNKNOWN
};

enum as4610_product_id get_product_id(void);
int chassis_fan_count(void);
int chassis_led_count(void);

#endif  /* __PLATFORM_LIB_H__ */

