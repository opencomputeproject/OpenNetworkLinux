/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 (C) Delta Networks, Inc.
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

#include "platform_lib.h"
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include "x86_64_delta_ag9032v2a_log.h"


#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

#define dni_onlp_thermal_threshold(WARNING_DEFAULT, ERROR_DEFAULT, SHUTDOWN_DEFAULT){ \
    WARNING_DEFAULT,                                                                  \
    ERROR_DEFAULT,                                                                    \
    SHUTDOWN_DEFAULT,                                                                 \
}
static char* path[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    NULL, /* CPU Core */
    "27-004f/hwmon/hwmon7/temp1_input",
    "8-004c/hwmon/hwmon1/temp1_input",
    "8-004d/hwmon/hwmon2/temp1_input",
    "8-004e/hwmon/hwmon3/temp1_input",
    "8-004f/hwmon/hwmon4/temp1_input",
    "4-0058/psu_temp1_input",
    "5-0058/psu_temp1_input",
};

static char* cpu_coretemp_files[] =
    {
        "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp1_input",
        "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp2_input",
        "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp3_input",
        "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp4_input",
        "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp5_input",
        NULL,
    };

/* Static values */
static onlp_thermal_info_t linfo[] = {
	{ }, /* Not used */
    { { ONLP_THERMAL_ID_CREATE(THERMAL_WIND_ON_ADAPTER_BOARD), "Wind thermal sensor",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BOARD), "MAC up side thermal sensor", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BOARD), "MAC down side thermal sensor", 0},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BOARD), "Surroundings thermal sensor", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_CPU_BOARD), "ON CPU BOARD", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID)},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID)},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
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
    uint8_t local_id    = 0;
    UINT4 multiplier    = 1000;
    UINT4 u4Data        = 0;
    char device_buf[20] = {0};
    int temp_base = 1;

    int rv;
    char  fullpath[50] = {0};
    int r_data = 0;
    VALIDATE(id);
    local_id    = ONLP_OID_ID_GET(id);
    *info       = linfo[local_id]; 
    if(local_id == THERMAL_4_ON_CPU_BOARD) {
        rv = onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
        return rv;
    }
    if(dni_bmc_check()==BMC_ON)
    {
        switch(local_id)
        {
            case THERMAL_WIND_ON_ADAPTER_BOARD:
            case THERMAL_1_ON_MAIN_BOARD:
            case THERMAL_2_ON_MAIN_BOARD:
            case THERMAL_3_ON_MAIN_BOARD:
            case THERMAL_4_ON_CPU_BOARD:
                sprintf(device_buf, "Sensor_Temp_%d", local_id);
                rv = dni_get_bmc_data(device_buf, &u4Data, multiplier);
                break;
            case THERMAL_5_ON_PSU1:
                sprintf(device_buf, "PSU1_Temp_1");
                rv = dni_get_bmc_data(device_buf, &u4Data, multiplier);
                break;
            case THERMAL_6_ON_PSU2:
                sprintf(device_buf, "PSU2_Temp_1");
                rv = dni_get_bmc_data(device_buf, &u4Data, multiplier);
                break;
            default:
                AIM_LOG_ERROR("Invalid Thermal ID!!\n");
                return ONLP_STATUS_E_PARAM;
         }
    
         if (u4Data == 0 || rv == ONLP_STATUS_E_GENERIC){
            return ONLP_STATUS_E_INTERNAL;
        }
        else{
            info->mcelsius = u4Data;
            return 0;
        }
    }
    else
    {
        switch (local_id) 
        {
            case THERMAL_WIND_ON_ADAPTER_BOARD:
                sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
                break;
            case THERMAL_1_ON_MAIN_BOARD:
                sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
                break;
            case THERMAL_2_ON_MAIN_BOARD:
                sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
                break;
            case THERMAL_3_ON_MAIN_BOARD:
                sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
                break;
            case THERMAL_4_ON_CPU_BOARD:
                sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
                break;
            case THERMAL_5_ON_PSU1:
                sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
                break;
            case THERMAL_6_ON_PSU2:
                sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
                break;
        }  
        r_data = dni_i2c_lock_read_attribute(NULL, fullpath);
        info->mcelsius = r_data / temp_base;
    }
    return ONLP_STATUS_OK;
}
