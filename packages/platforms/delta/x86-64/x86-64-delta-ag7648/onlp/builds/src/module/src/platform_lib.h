/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2015 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc.
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

#include "x86_64_delta_ag7648_log.h"

#define PSU1_ID 1
#define PSU2_ID 2

#define CHASSIS_FAN_COUNT     6
#define CHASSIS_THERMAL_COUNT 4
#define CHASSIS_PSU_COUNT     2


typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_AC_F2B,
    PSU_TYPE_AC_B2F,
    PSU_TYPE_DC_48V_F2B,
    PSU_TYPE_DC_48V_B2F
} psu_type_t;


psu_type_t get_psu_type(int id, char* modelname, int modelname_len);

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(format, ...)  \
    {\
        printf("[%s:%d] ", __FUNCTION__, __LINE__);\
        printf(format, __VA_ARGS__); \
    }
#else
    #define DEBUG_PRINT(format, ...)
#endif

enum onlp_fan_duty_cycle_percentage
{
    FAN_IDLE_RPM    = 7500,
	FAN_LEVEL1_RPM  = 10000,
	FAN_LEVEL2_RPM  = 13000,
	FAN_LEVEL3_RPM  = 16000,
	FAN_LEVEL4_RPM  = 19000,
};

enum ag7648_product_id {
	PID_AG7648= 2,
	PID_UNKNOWN
};
/* LED related data */
enum sys_led_light_mode {
	SYS_LED_MODE_GREEN_BLINKING = 0,
	SYS_LED_MODE_GREEN,
	SYS_LED_MODE_YELLOW,
	SYS_LED_MODE_YELLOW_BLINKING,
	SYS_LED_MODE_UNKNOWN
};

enum fan_led_light_mode {
	FAN_LED_MODE_OFF = 0,
	FAN_LED_MODE_YELLOW,
	FAN_LED_MODE_GREEN,
	FAN_LED_MODE_YELLOW_BLINKING,
	FAN_LED_MODE_UNKNOWN
};

enum psu_led_light_mode {
	PSU_LED_MODE_OFF = 0,
	PSU_LED_MODE_GREEN,
	PSU_LED_MODE_YELLOW,
	PSU_LED_MODE_YELLOW_BLINKING,
	PSU_LED_MODE_RESERVERD,
	PSU_LED_MODE_UNKNOWN
};

enum locator_led_light_mode {
    LOCATOR_LED_MODE_OFF = 0,
	LOCATOR_LED_MODE_GREEN_BLINKING,
	LOCATOR_LED_MODE_GREEN,
	LOCATOR_LED_MODE_RESERVERD,
	LOCATOR_LED_MODE_UNKNOWN
};

enum power_led_light_mode {
	POWER_LED_MODE_OFF = 0,
	POWER_LED_MODE_YELLOW,
	POWER_LED_MODE_GREEN,
	POWER_LED_MODE_YELLOW_BLINKING,
	POWER_LED_MODE_UNKNOWN
};

enum fan_tray_led_light_mode {
	FAN_TRAY_LED_MODE_OFF = 0,
	FAN_TRAY_LED_MODE_GREEN,
	FAN_TRAY_LED_MODE_YELLOW,
	FAN_TRAY_LED_MODE_RESERVERD,
	FAN_TRAY_LED_MODE_UNKNOWN
};

typedef enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_SYS,
	LED_FAN,
    LED_LOCATOR,
	LED_POWER,
    LED_FAN_TRAY0,
    LED_FAN_TRAY1,
	LED_FAN_TRAY2,
} onlp_led_id_t;

enum ag7648_product_id get_product_id(void);
int chassis_fan_count(void);
int chassis_led_count(void);

typedef enum platform_id_e {
    PLATFORM_ID_UNKNOWN,
    PLATFORM_ID_DELTA_AG7648_R0,
} platform_id_t;

extern platform_id_t platform_id;
#endif  /* __PLATFORM_LIB_H__ */
