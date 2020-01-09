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

#define INV_EPRM_PREFIX		"/sys/bus/i2c/devices/0-0053/"
#define INV_SYSLED_PREFIX	"/sys/bus/i2c/devices/0-0055/"
#define INV_HWMON_PREFIX	"/sys/bus/i2c/devices/0-0066/"
#define GET_CTMP_PREFIX		get_core_thermal_path()
#define INV_SWPS_PREFIX		"/sys/class/swps/"

#define EEPROM_NODE(node)	INV_EPRM_PREFIX#node
#define SYSLED_NODE(node)	INV_SYSLED_PREFIX#node
#define HWMON_NODE(node)		INV_HWMON_PREFIX#node
#define LOCAL_ID_TO_INFO_IDX(id)  (id-1)

#define IDPROM_PATH		EEPROM_NODE(eeprom)

/*
 * Definitions of D7032Q28B device
 */

/* THERMAL */
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
#define CHASSIS_THERMAL_COUNT		(9)
#define NUM_OF_THERMAL_ON_MAIN_BOARD	(5)

/* FAN */
enum onlp_fan_id {
    FAN_RESERVED = 0,
    FAN_1_ON_MAIN_BOARD,
    FAN_2_ON_MAIN_BOARD,
    FAN_3_ON_MAIN_BOARD,
    FAN_4_ON_MAIN_BOARD,
    FAN_1_ON_PSU1,
    FAN_1_ON_PSU2,
    FAN_MAX
};
#define CHASSIS_FAN_COUNT		(4)
#define NUM_OF_FAN_ON_MAIN_BOARD	(4)

/* LED */
enum onlp_led_id {
    LED_RESERVED = 0,
    LED_SYS,
    LED_FAN1,
    LED_FAN2,
    LED_FAN3,
    LED_FAN4,
    LED_MAX
};
#define CHASSIS_LED_COUNT		(1)
#define NUM_OF_LED_ON_MAIN_BOARD	(5)

/* PSU */
enum onlp_psu_id {
    PSU_RESERVED = 0,
    PSU1_ID,
    PSU2_ID,
    PSU_MAX
};
#define CHASSIS_PSU_COUNT		(2)
#define NUM_OF_PSU_ON_MAIN_BOARD	(2)

#define CHASSIS_SFP_COUNT		(0)
#define CHASSIS_QSFP_COUNT		(32)
#define CHASSIS_PORT_COUNT		(CHASSIS_SFP_COUNT+CHASSIS_QSFP_COUNT)

#define PSU_STATUS_NODE(node)		SYSLED_NODE(node)


/* MISC */
#define INV_NODE_MAX_PATH_LEN		(ONLP_CONFIG_INFO_STR_MAX)
#define INV_DEV_NOT_SUPPORT		(1)
#define LOCAL_ID_TO_INFO_IDX(id)	(id-1)
#define ONLP_NODE_MAX_PATH_LEN		(64)

char* get_core_thermal_path( );

#endif  /* __PLATFORM_LIB_H__ */
