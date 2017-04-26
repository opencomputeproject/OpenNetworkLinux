/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc
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

#include "arm_delta_ag6248c_log.h"

#define CHASSIS_THERMAL_COUNT 2 
#define CHASSIS_PSU_COUNT     2 

#define PSU1_ID 1
#define PSU2_ID 2


typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_AC_F2B,
    PSU_TYPE_AC_B2F
} psu_type_t;

psu_type_t get_psu_type(int id);

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(format, ...)   printf(format, __VA_ARGS__)
#else
    #define DEBUG_PRINT(format, ...)
#endif

enum onlp_fan_duty_cycle_percentage
{
    FAN_IDLE_RPM    = 5500,
	FAN_LEVEL1_RPM  = 7000,
	FAN_LEVEL2_RPM  = 9000,
	FAN_LEVEL3_RPM  = 12000,
};

enum ag6248c_product_id {
	PID_AG6248C_48= 2,
	PID_AG6248C_48P=4,
	PID_UNKNOWN
};
/* LED related data */
enum sys_led_light_mode {
	SYS_LED_MODE_GREEN_BLINKING = 0,
	SYS_LED_MODE_GREEN,
	SYS_LED_MODE_RED,
	SYS_LED_MODE_RED_BLINKING,
	SYS_LED_MODE_AUTO,
	SYS_LED_MODE_UNKNOWN
};

enum fan_led_light_mode {
	FAN_LED_MODE_OFF=0,
	FAN_LED_MODE_GREEN,
	FAN_LED_MODE_RED,
	FAN_LED_MODE_RESERVERD,
	FAN_LED_MODE_AUTO,
	FAN_LED_MODE_UNKNOWN
};

enum psu_led_light_mode {
	PSU_LED_MODE_OFF =0,
	PSU_LED_MODE_GREEN,
	PSU_LED_MODE_GREEN_BLINKING,
	PSU_LED_MODE_RESERVERD,
	PSU_LED_MODE_UNKNOWN
};

enum temp_led_light_mode {
	TEMP_LED_MODE_OFF =0,
	TEMP_LED_MODE_GREEN,
	TEMP_LED_MODE_RED,
	TEMP_LED_MODE_RESERVERD,
	TEMP_LED_MODE_UNKNOWN
};

enum master_led_light_mode {
	MASTER_LED_MODE_OFF =0,
	MASTER_LED_MODE_GREEN,
	MASTER_LED_MODE_OFF1,
	MASTER_LED_MODE_RESERVERD,
	MASTER_LED_MODE_UNKNOWN
};

typedef enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_SYS,
	LED_FAN,
    LED_PSU2,
    LED_PSU1,    
    LED_TEMP,
	LED_MASTER
} onlp_led_id_t;

enum ag6248c_product_id get_product_id(void);
int chassis_fan_count(void);
int chassis_led_count(void);

typedef enum platform_id_e {
    PLATFORM_ID_UNKNOWN,
    PLATFORM_ID_POWERPC_DELTA_AG6248C_POE_R0,
    PLATFORM_ID_POWERPC_DELTA_AG6248C_R0,
} platform_id_t;

extern platform_id_t platform_id;

#endif  /* __PLATFORM_LIB_H__ */

