/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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

#include <onlp/fan.h>
#include <onlp/psu.h>
#include "x86_64_mlnx_msn2410_log.h"

#define CHASSIS_PSU_COUNT           2
#define CHASSIS_TOTAL_FAN_COUNT     10
#define CHASSIS_TOTAL_THERMAL_COUNT 8
#define CHASSIS_FAN_COUNT     (CHASSIS_TOTAL_FAN_COUNT - CHASSIS_PSU_COUNT)
#define CHASSIS_THERMAL_COUNT (CHASSIS_TOTAL_THERMAL_COUNT - CHASSIS_PSU_COUNT)

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_MODULE_PREFIX "/bsp/module/psu%d_%s"
#define PSU_POWER_PREFIX "/bsp/power/psu%d_%s"
#define IDPROM_PATH "/bsp/eeprom/%s%d_info"

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

/* LED related data
 */
enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_SYSTEM,
    LED_FAN1,
    LED_FAN2,
    LED_FAN3,
    LED_FAN4,
    LED_PSU,
};

typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_AC_F2B,
    PSU_TYPE_AC_B2F
} psu_type_t;

psu_type_t get_psu_type(int id, char* modelname, int modelname_len);

int psu_read_eeprom(int psu_index, onlp_psu_info_t* psu_info,
                     onlp_fan_info_t* fan_info);

int onlp_fani_get_min_rpm(int id);
int onlp_get_kernel_ver(void);

#endif  /* __PLATFORM_LIB_H__ */
