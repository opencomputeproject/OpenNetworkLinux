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
#include "x86_64_delta_agc7646slv1b_log.h"


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
#ifdef I2C
static char* path[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    "26-004f/hwmon/hwmon6/temp1_input",
    "8-004b/hwmon/hwmon1/temp1_input",
    "8-004c/hwmon/hwmon2/temp1_input",
    "8-004c/hwmon/hwmon2/temp2_input",
    "8-004d/hwmon/hwmon3/temp1_input",
    "8-004d/hwmon/hwmon3/temp2_input",
    "8-004d/hwmon/hwmon3/temp3_input",
    "8-004e/hwmon/hwmon4/temp1_input",
    "8-004f/hwmon/hwmon5/temp1_input",
    "31-0058/psu_temp1_input",
    "32-0058/psu_temp1_input"
};
#endif

/* Static values */
static onlp_thermal_info_t linfo[] = {
	{ }, /* Not used */
    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_FAN_BOARD), "Board sensor on Fan_BD", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_CPU_BOARD), "Board sensor near CPU", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BOARD_TEMP_1), "Board sensor near MAC temp-1", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BOARD_TEMP_2), "Board sensor near MAC temp-2", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BOARD_TEMP_1), "Board sensor near SyncE temp-1", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BOARD_TEMP_2), "Board sensor near SyncE temp-2", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_7_ON_MAIN_BOARD_TEMP_3), "Board sensor near SyncE temp-3", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_8_ON_MAIN_BOARD), "Board sensor near MAC", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_9_ON_MAIN_BOARD), "Board sensor near MAC", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_10_ON_PSU1), "PSU-1 internal sensor", ONLP_PSU_ID_CREATE(PSU1_ID)},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_11_ON_PSU2), "PSU-2 internal Sensor", ONLP_PSU_ID_CREATE(PSU2_ID)},
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
    lockinit();
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
    uint8_t local_id = 0;
    int rv = ONLP_STATUS_OK;

    VALIDATE(id);
    local_id = ONLP_OID_ID_GET(id);
    *info = linfo[local_id];

#ifdef BMC
    UINT4 multiplier = 1000;
    UINT4 u4Data = 0;
    char device_buf[20] = {0};

    switch(local_id) {
        case THERMAL_1_ON_FAN_BOARD:
            sprintf(device_buf, "Fan_Temp");
            rv = dni_bmc_sensor_read(device_buf, &u4Data, multiplier, THERMAL_SENSOR);
            break;
        case THERMAL_2_ON_CPU_BOARD:
        case THERMAL_3_ON_MAIN_BOARD_TEMP_1:
        case THERMAL_4_ON_MAIN_BOARD_TEMP_2:
        case THERMAL_5_ON_MAIN_BOARD_TEMP_1:
        case THERMAL_6_ON_MAIN_BOARD_TEMP_2:
        case THERMAL_7_ON_MAIN_BOARD_TEMP_3:
        case THERMAL_8_ON_MAIN_BOARD:
        case THERMAL_9_ON_MAIN_BOARD:
            local_id--;
            sprintf(device_buf, "Temp_Sensor_%d", local_id);
            rv = dni_bmc_sensor_read(device_buf, &u4Data, multiplier, THERMAL_SENSOR);
            break;
        case THERMAL_10_ON_PSU1:
            sprintf(device_buf, "PSU1_Temp_1");
            rv = dni_bmc_sensor_read(device_buf, &u4Data, multiplier, THERMAL_SENSOR);
            break;
        case THERMAL_11_ON_PSU2:
            sprintf(device_buf, "PSU2_Temp_1");
            rv = dni_bmc_sensor_read(device_buf, &u4Data, multiplier, THERMAL_SENSOR);
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
#elif defined I2C
    int temp_base = 1;
    char fullpath[50] = {0};
    int r_data = 0;

    switch (local_id) {
        case THERMAL_1_ON_FAN_BOARD:
            sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
            break;
        case THERMAL_2_ON_CPU_BOARD:
            sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
            break;
        case THERMAL_3_ON_MAIN_BOARD_TEMP_1:
            sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
            break;
        case THERMAL_4_ON_MAIN_BOARD_TEMP_2:
            sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
            break;
        case THERMAL_5_ON_MAIN_BOARD_TEMP_1:
            sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
            break;
        case THERMAL_6_ON_MAIN_BOARD_TEMP_2:
            sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
            break;
        case THERMAL_7_ON_MAIN_BOARD_TEMP_3:
            sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
            break;
        case THERMAL_8_ON_MAIN_BOARD:
            sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
            break;
        case THERMAL_9_ON_MAIN_BOARD:
            sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
            break;
        case THERMAL_10_ON_PSU1:
            sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
            break;
        case THERMAL_11_ON_PSU2:
            sprintf(fullpath,"%s%s", PREFIX_PATH, path[local_id]);
            break;
    }  
    r_data = dni_i2c_lock_read_attribute(NULL, fullpath);
    info->mcelsius = r_data / temp_base;
#endif
    return rv;
}
