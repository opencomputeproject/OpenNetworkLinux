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
    "/sys/bus/i2c/devices/18-0048*temp1_input",
    "/sys/bus/i2c/devices/18-0049*temp1_input",
    "/sys/bus/i2c/devices/18-004a*temp1_input",
    "/sys/bus/i2c/devices/18-004b*temp1_input",
    "/sys/bus/i2c/devices/17-004d*temp1_input",
    "/sys/bus/i2c/devices/17-004e*temp1_input",
    "/sys/bus/i2c/devices/10-005b*psu_temp1_input",
    "/sys/bus/i2c/devices/9-0058*psu_temp1_input",
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
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "LM75-1-48", 0},
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
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "LM75-4-4B", 0},
	    ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "LM75-5-4D", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BROAD), "LM75-6-4E", 0},
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

typedef struct threshold_t {
	int warning;
	int error;
	int shutdown;
} threshold_t;

threshold_t threshold[FAN_DIR_COUNT][THERMAL_COUNT] = {
	[FAN_DIR_F2B][THERMAL_CPU_CORE].warning  = 85000,
	[FAN_DIR_F2B][THERMAL_CPU_CORE].error    = 94000,
	[FAN_DIR_F2B][THERMAL_CPU_CORE].shutdown = 100000,
	[FAN_DIR_F2B][THERMAL_1_ON_MAIN_BROAD].warning  = 67000,
	[FAN_DIR_F2B][THERMAL_1_ON_MAIN_BROAD].error    = 70000,
	[FAN_DIR_F2B][THERMAL_1_ON_MAIN_BROAD].shutdown = 73000,
	[FAN_DIR_F2B][THERMAL_2_ON_MAIN_BROAD].warning  = 70000,
	[FAN_DIR_F2B][THERMAL_2_ON_MAIN_BROAD].error    = 73000,
	[FAN_DIR_F2B][THERMAL_2_ON_MAIN_BROAD].shutdown = 76000,
	[FAN_DIR_F2B][THERMAL_3_ON_MAIN_BROAD].warning  = 55000,
	[FAN_DIR_F2B][THERMAL_3_ON_MAIN_BROAD].error    = 58000,
	[FAN_DIR_F2B][THERMAL_3_ON_MAIN_BROAD].shutdown = 61000,
	[FAN_DIR_F2B][THERMAL_4_ON_MAIN_BROAD].warning  = 54000,
	[FAN_DIR_F2B][THERMAL_4_ON_MAIN_BROAD].error    = 57000,
	[FAN_DIR_F2B][THERMAL_4_ON_MAIN_BROAD].shutdown = 60000,
	[FAN_DIR_F2B][THERMAL_5_ON_MAIN_BROAD].warning  = 53000,
	[FAN_DIR_F2B][THERMAL_5_ON_MAIN_BROAD].error    = 56000,
	[FAN_DIR_F2B][THERMAL_5_ON_MAIN_BROAD].shutdown = 59000,
	[FAN_DIR_F2B][THERMAL_6_ON_MAIN_BROAD].warning  = 54000,
	[FAN_DIR_F2B][THERMAL_6_ON_MAIN_BROAD].error    = 57000,
	[FAN_DIR_F2B][THERMAL_6_ON_MAIN_BROAD].shutdown = 60000,
	[FAN_DIR_F2B][THERMAL_1_ON_PSU1].warning  = 85000,
	[FAN_DIR_F2B][THERMAL_1_ON_PSU1].error    = 90000,
	[FAN_DIR_F2B][THERMAL_1_ON_PSU1].shutdown = 95000,
	[FAN_DIR_F2B][THERMAL_1_ON_PSU2].warning  = 85000,
	[FAN_DIR_F2B][THERMAL_1_ON_PSU2].error    = 90000,
	[FAN_DIR_F2B][THERMAL_1_ON_PSU2].shutdown = 95000,

	[FAN_DIR_B2F][THERMAL_CPU_CORE].warning  = 85000,
	[FAN_DIR_B2F][THERMAL_CPU_CORE].error    = 94000,
	[FAN_DIR_B2F][THERMAL_CPU_CORE].shutdown = 100000,
	[FAN_DIR_B2F][THERMAL_1_ON_MAIN_BROAD].warning  = 66000,
	[FAN_DIR_B2F][THERMAL_1_ON_MAIN_BROAD].error    = 69000,
	[FAN_DIR_B2F][THERMAL_1_ON_MAIN_BROAD].shutdown = 72000,
	[FAN_DIR_B2F][THERMAL_2_ON_MAIN_BROAD].warning  = 57000,
	[FAN_DIR_B2F][THERMAL_2_ON_MAIN_BROAD].error    = 60000,
	[FAN_DIR_B2F][THERMAL_2_ON_MAIN_BROAD].shutdown = 63000,
	[FAN_DIR_B2F][THERMAL_3_ON_MAIN_BROAD].warning  = 51000,
	[FAN_DIR_B2F][THERMAL_3_ON_MAIN_BROAD].error    = 54000,
	[FAN_DIR_B2F][THERMAL_3_ON_MAIN_BROAD].shutdown = 57000,
	[FAN_DIR_B2F][THERMAL_4_ON_MAIN_BROAD].warning  = 47000,
	[FAN_DIR_B2F][THERMAL_4_ON_MAIN_BROAD].error    = 50000,
	[FAN_DIR_B2F][THERMAL_4_ON_MAIN_BROAD].shutdown = 53000,
	[FAN_DIR_B2F][THERMAL_5_ON_MAIN_BROAD].warning  = 47000,
	[FAN_DIR_B2F][THERMAL_5_ON_MAIN_BROAD].error    = 50000,
	[FAN_DIR_B2F][THERMAL_5_ON_MAIN_BROAD].shutdown = 53000,
	[FAN_DIR_B2F][THERMAL_6_ON_MAIN_BROAD].warning  = 47000,
	[FAN_DIR_B2F][THERMAL_6_ON_MAIN_BROAD].error    = 50000,
	[FAN_DIR_B2F][THERMAL_6_ON_MAIN_BROAD].shutdown = 53000,
	[FAN_DIR_B2F][THERMAL_1_ON_PSU1].warning  = 85000,
	[FAN_DIR_B2F][THERMAL_1_ON_PSU1].error    = 90000,
	[FAN_DIR_B2F][THERMAL_1_ON_PSU1].shutdown = 95000,
	[FAN_DIR_B2F][THERMAL_1_ON_PSU2].warning  = 85000,
	[FAN_DIR_B2F][THERMAL_1_ON_PSU2].error    = 90000,
	[FAN_DIR_B2F][THERMAL_1_ON_PSU2].shutdown = 95000,
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
	enum onlp_fan_dir dir;
	VALIDATE(id);

	tid = ONLP_OID_ID_GET(id);
	dir = onlp_get_fan_dir();

	/* Set the onlp_oid_hdr_t and capabilities */
	*info = linfo[tid];
	info->thresholds.warning  = threshold[dir][tid].warning;
	info->thresholds.error    = threshold[dir][tid].error;
	info->thresholds.shutdown = threshold[dir][tid].shutdown;

	if(tid == THERMAL_CPU_CORE){
		return onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
	} else if(tid == THERMAL_1_ON_PSU1 || tid == THERMAL_1_ON_PSU2){
		int pid = tid - THERMAL_1_ON_PSU1 + 1;
		return onlp_file_read_int(&info->mcelsius, "%s%s", psu_pmbus_path(pid), "psu_temp1_input");
	}

	return onlp_file_read_int(&info->mcelsius, devfiles__[tid]);
}
