/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017 Delta Networks, Inc.
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

static char* last_path[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    NULL, /* CPU Core */
    "2-004d/hwmon/hwmon3/temp1_input",
    "3-004f/hwmon/hwmon4/temp1_input",
    "6-004c/hwmon/hwmon6/temp1_input",
    "6-004c/hwmon/hwmon6/temp2_input",
    "6-004e/hwmon/hwmon5/temp1_input",
    "6-004f/hwmon/hwmon7/temp1_input",
    "7-004c/hwmon/hwmon8/temp1_input",
    "4-0058/psu_temp1_input",
    "4-0058/psu_temp1_input",
};

static char* cpu_coretemp_files[] =
{
    "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp2_input",
    "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp3_input",
    "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp4_input",
    "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp5_input",
    NULL,
};

/* Static values */
static onlp_thermal_info_t linfo[] = {
	{ }, /* Not used */
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core",   0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_CPU_BROAD), "CPU below side thermal sensor (U57, Below of CPU)", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_FAN_BROAD), "FAN board thermal sensor (U334)", 0},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "MAC up side thermal sensor, local (U3, up side of MAC)", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "MAC up side thermal sensor, remote (U3, up side of MAC)", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "MAC down side thermal sensor (U24, down side of MAC)", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BROAD), "MAC up side thermal sensor (U25, up side of MAC)", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_7_ON_MAIN_BROAD), "Thermal sensor on main board (U17)", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID)},
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
    int local_id, r_data;
    char fullpath[256], psu_num[2];
    mux_info_t mux_info;

    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[local_id];

    if(local_id == THERMAL_CPU_CORE) {
        int rv = onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
        return rv;
    }

    /* Configure attribute path of thermal */
    sprintf(fullpath, "%s/%s", SYS_DEV_PATH, last_path[local_id]);

    /* Configure MUX settings */
    switch (local_id) {
    case THERMAL_2_ON_FAN_BROAD:
        mux_info.addr = SWPLD;
        mux_info.flags = 0x00;
        mux_info.bus = I2C_BUS_5;
        mux_info.offset = FAN_MUX_REG;
        mux_info.channel = 0x06;
        break;
    case THERMAL_1_ON_PSU1:
        sprintf(psu_num, "%d", 1);
        dni_i2c_lock_write_attribute(NULL, psu_num, PSU_SEL_PATH);
        break;
    case THERMAL_1_ON_PSU2:
        sprintf(psu_num, "%d", 2);
        dni_i2c_lock_write_attribute(NULL, psu_num, PSU_SEL_PATH);
        break;
    }

    /* Read temperature into r_data */
    if(local_id == THERMAL_2_ON_FAN_BROAD){
        r_data = dni_i2c_lock_read_attribute(&mux_info, fullpath);
    }else{
        r_data = dni_i2c_lock_read_attribute(NULL, fullpath);
    }
    
    info->mcelsius = r_data;

    return ONLP_STATUS_OK;
}

