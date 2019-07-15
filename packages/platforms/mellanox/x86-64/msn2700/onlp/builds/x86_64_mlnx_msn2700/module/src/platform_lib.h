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

#include "x86_64_mlnx_msn2700_log.h"

#define CHASSIS_PSU_COUNT           2
#define CHASSIS_TOTAL_FAN_COUNT     10
#define CHASSIS_TOTAL_THERMAL_COUNT 8
#define CHASSIS_FAN_COUNT     (CHASSIS_TOTAL_FAN_COUNT - CHASSIS_PSU_COUNT)
#define CHASSIS_THERMAL_COUNT (CHASSIS_TOTAL_THERMAL_COUNT - CHASSIS_PSU_COUNT)
#define CPLD_COUNT                 3
#define SFP_PORT_COUNT            32
#define CHASSIS_LED_COUNT       6

#endif  /* __PLATFORM_LIB_H__ */
