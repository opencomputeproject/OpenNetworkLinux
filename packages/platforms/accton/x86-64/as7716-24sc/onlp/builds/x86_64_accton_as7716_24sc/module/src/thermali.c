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
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static char* devfiles__[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    NULL,                  /* CPU_CORE files */	
    "/sys/bus/i2c/devices/10-0048*temp1_input",
    "/sys/bus/i2c/devices/10-0049*temp1_input",
    "/sys/bus/i2c/devices/10-004a*temp1_input",
    "/sys/bus/i2c/devices/10-004c*temp1_input",
    "/sys/bus/i2c/devices/10-004d*temp1_input",
    "/sys/bus/i2c/devices/10-004b*temp1_input",
    "/sys/bus/i2c/devices/18-0058/psu_temp1_input",
    "/sys/bus/i2c/devices/17-0059/psu_temp1_input",
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
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },	
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "Chassis Thermal Sensor 1", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "Chassis Thermal Sensor 2", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "Chassis Thermal Sensor 3", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "Chassis Thermal Sensor 4", 0},
	    ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "Chassis Thermal Sensor 5", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BROAD), "Chassis Thermal Sensor 6", 0},
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

typedef struct threshold_t {
    int warning;
    int error;
    int shutdown;
} threshold_t;

threshold_t psu_threshold[NUM_OF_PSU_TYPE] = {
[PSU_TYPE_AC_DPS850_F2B].warning  = ONLP_THERMAL_THRESHOLD_WARNING_DEFAULT,
[PSU_TYPE_AC_DPS850_F2B].error    = ONLP_THERMAL_THRESHOLD_ERROR_DEFAULT,
[PSU_TYPE_AC_DPS850_F2B].shutdown = ONLP_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT,
[PSU_TYPE_AC_DPS850_B2F].warning  = ONLP_THERMAL_THRESHOLD_WARNING_DEFAULT,
[PSU_TYPE_AC_DPS850_B2F].error    = ONLP_THERMAL_THRESHOLD_ERROR_DEFAULT,
[PSU_TYPE_AC_DPS850_B2F].shutdown = ONLP_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT,
[PSU_TYPE_AC_R1CA2122A].warning  = 53000,
[PSU_TYPE_AC_R1CA2122A].error    = 58000,
[PSU_TYPE_AC_R1CA2122A].shutdown = 63000,
[PSU_TYPE_DC_R1CD2122A].warning  = 95000,
[PSU_TYPE_DC_R1CD2122A].error    = 98000,
[PSU_TYPE_DC_R1CD2122A].shutdown = 101000,
};

static int
onlp_thermali_psu_threshold_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    int ret = ONLP_STATUS_OK;
    int thermal_id;
    int psu_id;
    psu_type_t psu_type;
    VALIDATE(id);

    thermal_id = ONLP_OID_ID_GET(id);
    psu_id = (thermal_id - THERMAL_1_ON_PSU1) + 1;

    /* Get PSU type
     */
    psu_type = get_psu_type(psu_id, NULL, 0);

    switch (psu_type) {
    case PSU_TYPE_AC_DPS850_F2B:
    case PSU_TYPE_AC_DPS850_B2F:
    case PSU_TYPE_AC_R1CA2122A:
    case PSU_TYPE_DC_R1CD2122A:
        info->thresholds.warning  = psu_threshold[psu_type].warning;
        info->thresholds.error    = psu_threshold[psu_type].error;
        info->thresholds.shutdown = psu_threshold[psu_type].shutdown;
        break;
    default:
        ret = ONLP_STATUS_E_UNSUPPORTED;
        break;
    }

    return ret;
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
    int   tid;
    VALIDATE(id);
	
    tid = ONLP_OID_ID_GET(id);
	
    /* Set the onlp_oid_hdr_t and capabilities */		
    *info = linfo[tid];

    if ((tid >= THERMAL_1_ON_PSU1) && (tid <= THERMAL_1_ON_PSU2)) {
        onlp_thermali_psu_threshold_get(id, info);
    }

    if(tid == THERMAL_CPU_CORE) {
        int rv = onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
        return rv;
    }

    return onlp_file_read_int(&info->mcelsius, devfiles__[tid]);
}

