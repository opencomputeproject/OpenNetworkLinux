/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *        Copyright 2017 Delta Networks, Inc.
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
#include <onlp/platformi/thermali.h>
#include "platform_lib.h"


#define VALIDATE(_id)                           \
    do                                          \
    {                                           \
        if(!ONLP_OID_IS_THERMAL(_id))           \
        {                                       \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/* Static values */
static onlp_thermal_info_t thermal_info[] = {
    { }, /* Not used */
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BOARD_TEMP), "TMP75-4F", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BOARD_TEMP), "TMP75-4A", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BOARD_TEMP), "TMP75-49", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { 
        { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BOARD_TEMP), "TMP75-4B", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {
        { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BOARD_TEMP), "TMP75-48", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {
        { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BOARD_TEMP), "Temp411-4C", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {
        { ONLP_THERMAL_ID_CREATE(THERMAL_7_ON_PSU1), "PSU1 internal temp 1", ONLP_PSU_ID_CREATE(PSU1_ID)},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {
        { ONLP_THERMAL_ID_CREATE(THERMAL_8_ON_PSU1), "PSU1 internal temp 2", ONLP_PSU_ID_CREATE(PSU1_ID)},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {
        { ONLP_THERMAL_ID_CREATE(THERMAL_9_ON_PSU2), "PSU2 internal temp 1", ONLP_PSU_ID_CREATE(PSU2_ID)},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {
        { ONLP_THERMAL_ID_CREATE(THERMAL_10_ON_PSU2), "PSU2 internal temp 2", ONLP_PSU_ID_CREATE(PSU2_ID)},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }
};

int onlp_thermali_init(void)
{
    lockinit();
    return ONLP_STATUS_OK;
}

int onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    uint8_t local_id = 0;
    int rv = ONLP_STATUS_OK;
    UINT4 multiplier = 1000;
    UINT4 u4Data = 0;
    VALIDATE(id);
    local_id = ONLP_OID_ID_GET(id);
    *info = thermal_info[local_id];
    if (thermal_dev_list[local_id].dev_name != NULL &&
             local_id <= NUM_OF_THERMAL_ON_MAIN_BROAD + NUM_OF_PSU_ON_MAIN_BROAD + 2)
    {
        rv = dni_bmc_sensor_read(thermal_dev_list[local_id].dev_name,
                                 &u4Data,
                                 multiplier,
                                 thermal_dev_list[local_id].dev_type);
        if (u4Data == 0 || rv == ONLP_STATUS_E_GENERIC)
            rv = ONLP_STATUS_E_INTERNAL;
        else
            info->mcelsius = u4Data;
    }
    else
    {
        AIM_LOG_ERROR("Invalid Thermal ID!\n");
        rv = ONLP_STATUS_E_PARAM;
    }

    return rv;
}

int onlp_thermali_ioctl(int id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
