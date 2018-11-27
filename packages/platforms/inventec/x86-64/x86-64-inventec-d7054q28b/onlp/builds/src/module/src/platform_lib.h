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

#include "x86_64_inventec_d7054q28b_log.h"

#define SWPS_CYPRESS_GA2	(1)

#define ONLP_NODE_MAX_INT_LEN	(8)
#define ONLP_NODE_MAX_PATH_LEN	(64)

#define CHASSIS_PSU_COUNT	(2)
#define CHASSIS_LED_COUNT	(5)
#define CHASSIS_FAN_COUNT	(10)
#define CHASSIS_SFP_COUNT	(54)
#define CHASSIS_THERMAL_COUNT	(14)

#define INV_CPLD_COUNT		(1)
#define INV_CPLD_PREFIX		"/sys/bus/i2c/devices/0-0055/"
#define INV_PSOC_PREFIX		"/sys/bus/i2c/devices/0-0066/"
#define INV_EPRM_PREFIX		"/sys/bus/i2c/devices/0-0053/"
#define INV_CTMP_PREFIX		"/sys/devices/platform/coretemp.0/hwmon/hwmon0/"

/*
 * Definitions of Chassis EEPROM
 */
#define EEPROM_NODE(node)	INV_EPRM_PREFIX#node


/*
 * Definitions of PSU device
 */
enum onlp_psu_id
{
    PSU_RESERVED = 0,
    PSU1_ID,
    PSU2_ID
};

#define PSU1_AC_PMBUS_PREFIX	INV_PSOC_PREFIX
#define PSU2_AC_PMBUS_PREFIX	INV_PSOC_PREFIX

#define PSU1_AC_PMBUS_NODE(node) PSU1_AC_PMBUS_PREFIX#node
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node

#define PSU1_AC_HWMON_PREFIX	INV_CPLD_PREFIX
#define PSU2_AC_HWMON_PREFIX	INV_CPLD_PREFIX

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

#define PSU1_AC_HWMON_NODE(node) PSU1_AC_HWMON_PREFIX#node
#define PSU2_AC_HWMON_NODE(node) PSU2_AC_HWMON_PREFIX#node

/*
 * Definitions of FAN device
 */
#define FAN_BOARD_PATH	INV_PSOC_PREFIX
#define FAN_NODE(node)	FAN_BOARD_PATH#node

/*
 * Prototypes
 */
int onlp_file_read_binary(char *filename, char *buffer, int buf_size, int data_len);
int onlp_file_read_string(char *filename, char *buffer, int buf_size, int data_len);

int psu_pmbus_info_get(int id, char *node, int *value);
int psu_pmbus_info_set(int id, char *node, int value);

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(format, ...)   printf(format, __VA_ARGS__)
#else
    #define DEBUG_PRINT(format, ...)
#endif

#endif  /* __PLATFORM_LIB_H__ */

