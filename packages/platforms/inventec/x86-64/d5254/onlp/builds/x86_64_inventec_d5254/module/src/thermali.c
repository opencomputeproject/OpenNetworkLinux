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
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <unistd.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <fcntl.h>
#include "platform_lib.h"

typedef struct thermali_info_s {
    char file[ONLP_CONFIG_INFO_STR_MAX];
} thermali_info_t;

static thermali_info_t __info_list[ONLP_THERMAL_COUNT] = {
    {},
    {"/sys/class/hwmon/hwmon0/temp1_input"},
    {"/sys/class/hwmon/hwmon0/temp2_input"},
    {"/sys/class/hwmon/hwmon0/temp3_input"},
    {"/sys/class/hwmon/hwmon0/temp4_input"},
    {"/sys/class/hwmon/hwmon0/temp5_input"},
    {"/sys/class/hwmon/hwmon1/device/temp1_input"},
    {"/sys/class/hwmon/hwmon1/device/temp2_input"},
    {"/sys/class/hwmon/hwmon1/device/temp3_input"},
    {"/sys/class/hwmon/hwmon1/device/temp4_input"},
    {"/sys/class/hwmon/hwmon1/device/temp5_input"},
    {"/sys/class/hwmon/hwmon1/device/thermal_psu1"},
    {"/sys/class/hwmon/hwmon1/device/thermal2_psu1"},
    {"/sys/class/hwmon/hwmon1/device/thermal_psu2"},
    {"/sys/class/hwmon/hwmon1/device/thermal2_psu2"}
};


/* Static values */
static onlp_thermal_info_t __onlp_thermal_info[ONLP_THERMAL_COUNT] = {
    { }, /* Not used */
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_CPU_PHY, "CPU Physical"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_CPU_CORE0, "CPU Core0"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_CPU_CORE1, "CPU Core1"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_CPU_CORE2, "CPU Core2"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_CPU_CORE3, "CPU Core3"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_1_ON_MAIN_BROAD, "Thermal Sensor 1"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_2_ON_MAIN_BROAD, "Thermal Sensor 2"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_3_ON_MAIN_BROAD, "Thermal Sensor 3"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_4_ON_MAIN_BROAD, "Thermal Sensor 4"),
    ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_5_ON_MAIN_BROAD, "Thermal Sensor 5"),
    ONLP_PSU_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_1_ON_PSU1, "PSU-1 Thermal Sensor 1", ONLP_PSU_1),
    ONLP_PSU_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_2_ON_PSU1, "PSU-1 Thermal Sensor 2", ONLP_PSU_1),
    ONLP_PSU_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_1_ON_PSU2, "PSU-2 Thermal Sensor 1", ONLP_PSU_2),
    ONLP_PSU_THERMAL_INFO_ENTRY_INIT(ONLP_THERMAL_2_ON_PSU2, "PSU-2 Thermal Sensor 2", ONLP_PSU_2),
};

int
onlp_thermali_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, 1, ONLP_THERMAL_MAX-1);
}

int
onlp_thermali_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    *hdr = __onlp_thermal_info[id].hdr;
    return ONLP_STATUS_OK;
}

int
onlp_thermali_info_get(onlp_oid_id_t id, onlp_thermal_info_t* info)
{
    *info = __onlp_thermal_info[id];
    return onlp_file_read_int(&info->mcelsius, __info_list[id].file);
}
