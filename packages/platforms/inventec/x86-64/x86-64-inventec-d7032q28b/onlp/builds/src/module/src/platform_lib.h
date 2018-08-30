/************************************************************
 * platform_lib.h
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 ***********************************************************/
#ifndef __PLATFORM_LIB_H__
#define __PLATFORM_LIB_H__

#include "x86_64_inventec_d7032q28b_log.h"

#define CHASSIS_FAN_COUNT     10
#define CHASSIS_THERMAL_COUNT 5

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_HWMON_PSOC_PREFIX	"/sys/bus/i2c/devices/0-0066/"
#define PSU_HWMON_CPLD_PREFIX	"/sys/bus/i2c/devices/0-0055/"

#define PSU1_AC_PMBUS_PREFIX	PSU_HWMON_PSOC_PREFIX
#define PSU2_AC_PMBUS_PREFIX	PSU_HWMON_PSOC_PREFIX

#define PSU1_AC_PMBUS_NODE(node) PSU1_AC_PMBUS_PREFIX#node
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node

#define PSU1_AC_HWMON_PREFIX	PSU_HWMON_CPLD_PREFIX
#define PSU2_AC_HWMON_PREFIX	PSU_HWMON_CPLD_PREFIX

#define PSU1_AC_HWMON_NODE(node) PSU1_AC_HWMON_PREFIX#node
#define PSU2_AC_HWMON_NODE(node) PSU2_AC_HWMON_PREFIX#node

#define FAN_BOARD_PATH	"/sys/devices/platform/fan/"
#define FAN_NODE(node)	FAN_BOARD_PATH#node

#define IDPROM_PATH	"/sys/class/i2c-adapter/i2c-0/0-0053/eeprom"

int onlp_file_read_binary(char *filename, char *buffer, int buf_size, int data_len);
int onlp_file_read_string(char *filename, char *buffer, int buf_size, int data_len);

int psu_pmbus_info_get(int id, char *node, int *value);
int psu_pmbus_info_set(int id, char *node, int value);

typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_AC_F2B,
    PSU_TYPE_AC_B2F,
    PSU_TYPE_DC_48V_F2B,
    PSU_TYPE_DC_48V_B2F,
    PSU_TYPE_DC_12V_FANLESS,
    PSU_TYPE_DC_12V_F2B,
    PSU_TYPE_DC_12V_B2F
} psu_type_t;

psu_type_t get_psu_type(int id, char* modelname, int modelname_len);

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(format, ...)   printf(format, __VA_ARGS__)
#else
    #define DEBUG_PRINT(format, ...)
#endif

#endif  /* __PLATFORM_LIB_H__ */

