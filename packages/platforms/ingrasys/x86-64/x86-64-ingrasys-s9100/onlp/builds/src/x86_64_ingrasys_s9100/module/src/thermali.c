/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
#include <onlplib/file.h>
#include "x86_64_ingrasys_s9100_log.h"
#include "platform_lib.h"
      
static onlp_thermal_info_t thermal_info[] = {
    { }, /* Not used */
    { { THERMAL_OID_FRONT_MAC, "Chassis Thermal (Front)", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_REAR_MAC, "Chassis Thermal (Rear)", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU1, "CPU Thermal 1", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU2, "CPU Thermal 2", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU3, "CPU Thermal 3", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU4, "CPU Thermal 4", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_PSU1_1, "PSU-1 Thermal 1", PSU_OID_PSU1},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0
    },
    { { THERMAL_OID_PSU1_2, "PSU-1 Thermal 2", PSU_OID_PSU1},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0
    },
    { { THERMAL_OID_PSU2_1, "PSU-2 Thermal 1", PSU_OID_PSU2},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0
    },
    { { THERMAL_OID_PSU2_2, "PSU-2 Thermal 2", PSU_OID_PSU2},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0
    }       
};

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{    
    return ONLP_STATUS_OK;
}

static int
sys_thermal_info_get(onlp_thermal_info_t* info, int id)
{
    int rv;

    rv = onlp_file_read_int(&info->mcelsius,
                            SYS_FAN_PREFIX "/temp%d_input", id);

    if(rv == ONLP_STATUS_E_INTERNAL) {
        return rv;
    }

    if(rv == ONLP_STATUS_E_MISSING) {
        info->status &= ~1;
        return 0;
    }
    
    return ONLP_STATUS_OK;
}

static int
cpu_thermal_info_get(onlp_thermal_info_t* info, int id)
{
    int rv;
    int offset;
    offset = 1;
    id = id - offset;
    rv = onlp_file_read_int(&info->mcelsius,
                            SYS_CORE_TEMP_PREFIX "/temp%d_input", id);

    if(rv == ONLP_STATUS_E_INTERNAL) {
        return rv;
    }

    if(rv == ONLP_STATUS_E_MISSING) {
        info->status &= ~1;
        return 0;
    }
    
    return ONLP_STATUS_OK;
}


int 
psu_thermal_info_get(onlp_thermal_info_t* info, int id)
{
    int rv;
    
    rv = psu_thermal_get(info, id);
    if(rv == ONLP_STATUS_E_INTERNAL) {
        return rv;
    }
    
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
    int sensor_id, rc;
    sensor_id = ONLP_OID_ID_GET(id);
    
    *info = thermal_info[sensor_id];
    info->caps |= ONLP_THERMAL_CAPS_GET_TEMPERATURE;

    switch (sensor_id) {
        case THERMAL_ID_FRONT_MAC:            
        case THERMAL_ID_REAR_MAC:
            rc = sys_thermal_info_get(info, sensor_id);
            break;
        case THERMAL_ID_CPU1:
        case THERMAL_ID_CPU2:
        case THERMAL_ID_CPU3:
        case THERMAL_ID_CPU4:
            rc = cpu_thermal_info_get(info, sensor_id);
            break;
        case THERMAL_ID_PSU1_1:
        case THERMAL_ID_PSU1_2:
        case THERMAL_ID_PSU2_1:
        case THERMAL_ID_PSU2_2:
            rc = psu_thermal_info_get(info, sensor_id);
            break;
        default:            
            return ONLP_STATUS_E_INTERNAL;
            break;
    }

    return rc;
}
