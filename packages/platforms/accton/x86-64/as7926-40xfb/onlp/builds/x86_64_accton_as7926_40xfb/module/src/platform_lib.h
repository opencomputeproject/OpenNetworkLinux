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
#include "x86_64_accton_as7926_40xfb_log.h"

#define CHASSIS_FAN_COUNT     10
#define CHASSIS_THERMAL_COUNT 11
#define CHASSIS_LED_COUNT     6
#define CHASSIS_PSU_COUNT     2
#define CHASSIS_SFP_COUNT     55

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_SYSFS_PATH "/sys/devices/platform/as7926_40xfb_psu/"
#define FAN_SYSFS_PATH "/sys/devices/platform/as7926_40xfb_fan/"

#define PSU_STATUS_PRESENT     1
#define PSU_STATUS_POWER_GOOD  1

#endif  /* __PLATFORM_LIB_H__ */
