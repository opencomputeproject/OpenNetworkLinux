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
#include <onlp/platformi/base.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BOARD,
    THERMAL_2_ON_MAIN_BOARD,
    THERMAL_3_ON_MAIN_BOARD,
    THERMAL_4_ON_MAIN_BOARD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
};

static char* devfiles__[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    NULL,                  /* CPU_CORE files */
    "/sys/bus/i2c/devices/3-0048*temp1_input",
    "/sys/bus/i2c/devices/3-0049*temp1_input",
    "/sys/bus/i2c/devices/3-004a*temp1_input",
    "/sys/bus/i2c/devices/3-004b*temp1_input",
    "/sys/bus/i2c/devices/11-005b*psu_temp1_input",
    "/sys/bus/i2c/devices/10-0058*psu_temp1_input",
};

static char* cpu_coretemp_files[] =
    {
        "/sys/devices/platform/coretemp.0*temp2_input",
        "/sys/devices/platform/coretemp.0*temp3_input",
        "/sys/devices/platform/coretemp.0*temp4_input",
        "/sys/devices/platform/coretemp.0*temp5_input",
        NULL,
    };

/* Static values */
static onlp_thermal_info_t linfo[] = {
    { }, /* Not used */

    { { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", ONLP_OID_CHASSIS,
        .status = ONLP_OID_STATUS_FLAG_PRESENT },
      ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },

    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BOARD), "Chassis Thermal Sensor 1", ONLP_OID_CHASSIS,
        .status = ONLP_OID_STATUS_FLAG_PRESENT },
      ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },

    { { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BOARD), "Chassis Thermal Sensor 2", ONLP_OID_CHASSIS,
        .status = ONLP_OID_STATUS_FLAG_PRESENT },
      ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },

    { { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BOARD), "Chassis Thermal Sensor 3", ONLP_OID_CHASSIS,
        .status = ONLP_OID_STATUS_FLAG_PRESENT },
      ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },

    { { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BOARD), "Chassis Thermal Sensor 4", ONLP_OID_CHASSIS,
        .status = ONLP_OID_STATUS_FLAG_PRESENT },
      ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },

    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID),
        .status = ONLP_OID_STATUS_FLAG_PRESENT },
      ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },

    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID),
        .status = ONLP_OID_STATUS_FLAG_PRESENT },
      ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }
};

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
    int local_id;
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[local_id];

    if(local_id == THERMAL_CPU_CORE) {
        int rv = onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
        return rv;
    }

    return onlp_file_read_int(&info->mcelsius, devfiles__[local_id]);
}
