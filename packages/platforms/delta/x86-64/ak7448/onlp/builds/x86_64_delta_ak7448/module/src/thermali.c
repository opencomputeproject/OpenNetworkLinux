/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017  Delta Networks, Inc. 
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
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <fcntl.h>
#include "platform_lib.h"
#include <onlplib/i2c.h>

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

static char* last_path[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    NULL, /* CPU Core */
    "/2-004d/hwmon/hwmon4/temp1_input",
    "/6-004c/hwmon/hwmon5/temp1_input",
    "/6-004d/hwmon/hwmon6/temp1_input",
    "/6-004e/hwmon/hwmon7/temp1_input",
    "/7-004f/hwmon/hwmon8/temp1_input",
    "/4-0058/psu_temp1_input",
};

static char* cpu_coretemp_files[] =
{
    "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp2_input",
    "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp3_input",
    NULL,
};

/* Static values */
static onlp_thermal_info_t linfo[] = {
    { }, /* Not used */
    { { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_CPU_BOARD), "Thermal sensor near CPU (U57)", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BOARD), "Thermal sensor near MAC (U10)", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BOARD), "Thermal sensor near middle SFP cage (U11)", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, dni_onlp_thermal_threshold(65000,75000,80000)
        },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BOARD), "Thermal sensor near QSFP cage (U13)", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_FAN_BOARD), "Thermal sensor near FAN (U334)", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
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
    int temp_base = 1;
    int local_id, r_data;
    char  fullpath[256] = {0};
    uint8_t channel, offset;
    mux_info_t mux_info;

    VALIDATE(id);
    local_id = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[local_id];

    if(local_id == THERMAL_CPU_CORE) {
        int rv = onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
        return rv;
    }

    switch (local_id) {
        case THERMAL_1_ON_CPU_BOARD:
        case THERMAL_2_ON_MAIN_BOARD:
        case THERMAL_3_ON_MAIN_BOARD:
        case THERMAL_4_ON_MAIN_BOARD:
            channel = 0x00; /* DEFAULT */
            break;
        case THERMAL_5_ON_FAN_BOARD:
            offset = FAN_I2C_MUX_SEL_REG;
            channel = FAN_I2C_SEL_THERMAL;
            break;
        case THERMAL_1_ON_PSU1:
            offset = PSU_I2C_MUX_SEL_REG;
            channel = PSU_I2C_SEL_PSU_EEPROM;
            break;
        default:
            return ONLP_STATUS_E_INVALID;
    }

    mux_info.offset = offset;
    mux_info.channel = channel;
    
    sprintf(fullpath, "%s%s", PREFIX_PATH, last_path[local_id]);
   
    r_data = dni_i2c_lock_read_attribute(&mux_info, fullpath); 
    if(r_data == -1){
        return ONLP_STATUS_E_INTERNAL;
    }
    
    /* Current temperature in milli-celsius */
    info->mcelsius = r_data / temp_base;

    return ONLP_STATUS_OK;
}
