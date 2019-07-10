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

#define ONLP_NODE_MAX_INT_LEN	(8)
#define ONLP_NODE_MAX_PATH_LEN	(64)

#define INV_CPLD_COUNT		(1)
#define INV_CPLD_PREFIX		"/sys/bus/i2c/devices/0-0055/"
#define INV_PSOC_PREFIX		"/sys/bus/i2c/devices/0-0066/"
#define INV_EPRM_PREFIX		"/sys/bus/i2c/devices/0-0053/"
#define INV_CTMP_PREFIX		"/sys/devices/platform/coretemp.0/hwmon/hwmon0/"

#define INV_SFP_EEPROM_UPDATE	"/sys/class/swps/module/eeprom_update"
#define CHASSIS_SFP_COUNT	(32)

/*
 * Definitions of Chassis EEPROM
 */
#define EEPROM_NODE(node)	INV_EPRM_PREFIX#node

/*
 * Definitions of D7032Q28B device
 */
enum onlp_thermal_id {
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE_FIRST,
    THERMAL_CPU_CORE_3,
    THERMAL_CPU_CORE_4,
    THERMAL_CPU_CORE_LAST,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_MAIN_BROAD,
    THERMAL_5_ON_MAIN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
    THERMAL_MAX
};
#define CHASSIS_THERMAL_COUNT	(9)

enum onlp_fan_id {
    FAN_RESERVED = 0,
    FAN_1_ON_MAIN_BOARD,
    FAN_2_ON_MAIN_BOARD,
    FAN_3_ON_MAIN_BOARD,
    FAN_4_ON_MAIN_BOARD,
    FAN_5_ON_MAIN_BOARD,
    FAN_6_ON_MAIN_BOARD,
    FAN_7_ON_MAIN_BOARD,
    FAN_8_ON_MAIN_BOARD,
    FAN_1_ON_PSU1,
    FAN_1_ON_PSU2,
    FAN_MAX
};
#define CHASSIS_FAN_COUNT	(8)

enum onlp_led_id {
    LED_RESERVED = 0,
    LED_SYS,
    LED_FAN1,
    LED_FAN2,
    LED_FAN3,
    LED_FAN4,
    LED_MAX
};
#define CHASSIS_LED_COUNT	(1)

enum onlp_psu_id {
    PSU_RESERVED = 0,
    PSU1_ID,
    PSU2_ID,
    PSU_MAX
};
#define CHASSIS_PSU_COUNT	(2)

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

