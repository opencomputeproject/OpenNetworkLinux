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
#include <limits.h>

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)


/*Thermal 1 can be at 0x48 for R0A board, or 0x4C otherwise.*/
static int thermal1_addrs[] = {0x4c, 0x48};
static int thermal1_addr = -1;

static char* devfiles__[] =  /* must map with onlp_thermal_id */
{
    NULL,
    NULL,                  /* CPU_CORE files */
    "/sys/bus/i2c/devices/10-00%2x*temp1_input",
    "/sys/bus/i2c/devices/10-0049*temp1_input",
    "/sys/bus/i2c/devices/10-004a*temp1_input",
    "/sys/bus/i2c/devices/10-004b*temp1_input",
    "/sys/bus/i2c/devices/18-005b*psu_temp1_input",
    "/sys/bus/i2c/devices/17-0058*psu_temp1_input",	
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
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "LM75-1-%2X", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "LM75-2-49", 0}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "LM75-3-4A", 0}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "LM75-3-4B", 0}, 
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

/*check which addr of thermal1_addrs[] can be read.*/
static int
_get_valid_t1_addr(char *path, int *addr)
{
    char fname[PATH_MAX];
    int i, rv, tmp;
    if (thermal1_addr > 0){
        *addr = thermal1_addr;
        return ONLP_STATUS_OK;
    }

    for (i=0; i<AIM_ARRAYSIZE(thermal1_addrs); i++){
        snprintf(fname, sizeof(fname), path, thermal1_addrs[i]);
        rv = onlp_file_read_int(&tmp, fname);
        if (rv == ONLP_STATUS_OK){
            thermal1_addr = thermal1_addrs[i];
            *addr = thermal1_addr;
            return ONLP_STATUS_OK;
        }
    }
    return ONLP_STATUS_E_INVALID;
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
    char *devfile;
    char fname[PATH_MAX];

    VALIDATE(id);
	
    tid = ONLP_OID_ID_GET(id);
    /* Set the onlp_oid_hdr_t and capabilities */		
    *info = linfo[tid];
    if(tid == THERMAL_CPU_CORE) {
        int rv = onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
        return rv;
    }

    if (tid >= AIM_ARRAYSIZE(devfiles__)){
        return ONLP_STATUS_E_PARAM;
    }

	devfile = devfiles__[tid];
    if (tid == THERMAL_1_ON_MAIN_BROAD)
    {
        onlp_oid_desc_t *des = &info->hdr.description;
        char tmp[PATH_MAX];
        int t1_addr = thermal1_addrs[0];
        int rv;

        rv = _get_valid_t1_addr(devfiles__[tid], &t1_addr);
        if(rv != ONLP_STATUS_OK)
            return rv;

        /*Get real path of THERMAL_1 dev file*/
        snprintf(fname, sizeof(fname), devfiles__[tid], t1_addr);
        devfile = fname;

        /*Replace description*/
        strncpy(tmp, *des, sizeof(tmp));
        snprintf(*des, sizeof(*des), tmp, t1_addr);
    }

    return onlp_file_read_int(&info->mcelsius, devfile);
}

