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
#include <onlp/platformi/thermali.h>
#include <onlplib/file.h>
#include "x86_64_netberg_aurora_420_rangeley_int.h"
#include "x86_64_netberg_aurora_420_rangeley_log.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static int
sys_thermal_info_get__(onlp_thermal_info_t* info, int id)
{
    int rv;

    if (id == THERMAL_ID_THERMAL3)
    {
        rv = onlp_file_read_int(&info->mcelsius, SYS_HWMON1_PREFIX "/mac_temp");
        info->mcelsius *= 1000;
    }
    else
    {
        uint8_t buffer[64];
        double dvalue;
        int len;

        memset(buffer, 0, sizeof(buffer));
        rv = onlp_file_read(buffer, sizeof(buffer), &len, SYS_HWMON1_PREFIX "/remote_temp%d", id);
        if (rv == ONLP_STATUS_OK)
        {
            dvalue = atof((const char *)buffer);
            info->mcelsius = (int)(dvalue * 1000);
        }
    }

    if(rv == ONLP_STATUS_E_INTERNAL)
        return rv;

    if(rv == ONLP_STATUS_E_MISSING)
    {
        info->status &= ~(ONLP_THERMAL_STATUS_PRESENT);
        return ONLP_STATUS_OK;
    }

    return ONLP_STATUS_OK;
}

static int
psu1_thermal_info_get__(onlp_thermal_info_t* info, int id)
{
    int rv;
    uint8_t buffer[64];
    double dvalue;
    int len;

    memset(buffer, 0, sizeof(buffer));
    rv = onlp_file_read(buffer, sizeof(buffer), &len, SYS_HWMON2_PREFIX "/psu1_temp_%d", id);
    if (rv == ONLP_STATUS_OK)
    {
        dvalue = atof((const char *)buffer);
        info->mcelsius = (int)(dvalue * 1000);
    }
    return rv;
}

static int
psu2_thermal_info_get__(onlp_thermal_info_t* info, int id)
{
    int rv;
    uint8_t buffer[64];
    double dvalue;
    int len;

    memset(buffer, 0, sizeof(buffer));
    rv = onlp_file_read(buffer, sizeof(buffer), &len, SYS_HWMON2_PREFIX "/psu2_temp_%d", id);
    if (rv == ONLP_STATUS_OK)
    {
        dvalue = atof((const char *)buffer);
        info->mcelsius = (int)(dvalue * 1000);
    }
    return rv;
}

static onlp_thermal_info_t temps__[] =
{
    { }, /* Not used */
    { { THERMAL_OID_THERMAL1,  "Chassis Thermal 1 (Front of MAC)",  0}, ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0},
    { { THERMAL_OID_THERMAL2,  "Chassis Thermal 2 (Rear of MAC)",  0}, ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0},
    { { THERMAL_OID_THERMAL3,  "Chassis Thermal 3 (MAC)",  0}, ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0},

    { { THERMAL_OID_THERMAL4, "PSU-1 Thermal 1", PSU_OID_PSU1 }, ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0},
    { { THERMAL_OID_THERMAL5, "PSU-1 Thermal 2", PSU_OID_PSU1 }, ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0},

    { { THERMAL_OID_THERMAL6, "PSU-2 Thermal 1", PSU_OID_PSU2 }, ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0},
    { { THERMAL_OID_THERMAL7, "PSU-2 Thermal 2", PSU_OID_PSU2 }, ONLP_THERMAL_STATUS_PRESENT, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0},
};


/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
    return ONLP_STATUS_OK;
}

/*
 * Retrieve the information structure for the given thermal OID.
 *
 * If the OID is invalid, return ONLP_E_STATUS_INVALID.
 * If an unexpected error occurs, return ONLP_E_STATUS_INTERNAL.
 * Otherwise, return ONLP_STATUS_OK with the OID's information.
 *
 * Note -- it is expected that you fill out the information
 * structure even if the sensor described by the OID is not present.
 */
int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    int tid;

    VALIDATE(id);

    memset(info, 0, sizeof(onlp_thermal_info_t));
    tid = ONLP_OID_ID_GET(id);
    *info = temps__[tid];

    switch(tid)
    {
        case THERMAL_ID_THERMAL1:
        case THERMAL_ID_THERMAL2:
        case THERMAL_ID_THERMAL3:
            return sys_thermal_info_get__(info, tid);

        case THERMAL_ID_THERMAL4:
        case THERMAL_ID_THERMAL5:
            return psu1_thermal_info_get__(info, (tid - THERMAL_ID_THERMAL4 + 1));

        case THERMAL_ID_THERMAL6:
        case THERMAL_ID_THERMAL7:
            return psu2_thermal_info_get__(info, (tid - THERMAL_ID_THERMAL6 + 1));
    }

    return ONLP_STATUS_E_INVALID;
}

