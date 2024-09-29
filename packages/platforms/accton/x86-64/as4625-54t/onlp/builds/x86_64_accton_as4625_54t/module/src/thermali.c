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
//#include <unistd.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include "platform_lib.h"

#define CPU_CORE_ID_PATH_FORMAT  "/sys/devices/system/cpu/cpu%d/topology/core_id"
#define CPU_CORETEMP_PATH_FORMAT "/sys/devices/platform/coretemp.0*temp%d_input"
#define THERMAL_PATH_FORMAT     "/sys/bus/i2c/devices/%s/*temp1_input"
#define PSU_THERMAL_PATH_FORMAT "/sys/bus/i2c/devices/%s/*psu_temp1_input"

#define VALIDATE(_id)                           \
	do {                                        \
		if(!ONLP_OID_IS_THERMAL(_id)) {         \
			return ONLP_STATUS_E_INVALID;       \
		}                                       \
	} while(0)

enum onlp_thermal_id {
	THERMAL_RESERVED = 0,
	THERMAL_CPU_CORE,
	THERMAL_1_ON_MAIN_BROAD,
	THERMAL_2_ON_MAIN_BROAD,
	THERMAL_3_ON_MAIN_BROAD,
	THERMAL_4_ON_MAIN_BROAD,
	THERMAL_5_ON_MAIN_BROAD,
	THERMAL_1_ON_PSU1,
	THERMAL_2_ON_PSU1,
	THERMAL_1_ON_PSU2,
	THERMAL_2_ON_PSU2,
	THERMAL_COUNT,
};

static char* devfiles__[] = { /* must map with onlp_thermal_id */
	NULL,
	NULL, /* CPU_CORE files */
	"/sys/bus/i2c/devices/3-004a*temp1_input",
	"/sys/bus/i2c/devices/3-004b*temp1_input",
	"/sys/bus/i2c/devices/3-004d*temp1_input",
	"/sys/bus/i2c/devices/3-004e*temp1_input",
	"/sys/bus/i2c/devices/3-004f*temp1_input",
	"/sys/bus/i2c/devices/8-0058*psu_temp2_input",
	"/sys/bus/i2c/devices/8-0058*psu_temp3_input",
	"/sys/bus/i2c/devices/9-0059*psu_temp2_input",
	"/sys/bus/i2c/devices/9-0059*psu_temp3_input"
};

/* Static values */
static onlp_thermal_info_t linfo[] = {
	{ }, /* Not used */
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0, {0} },
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "MB_RightRear_temp(0x4A)", 0, {0} },
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "MB_RightCenter_temp(0x4B)", 0, {0} },
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "MB_FrontCenter_temp(0x4D)", 0, {0} },
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "MB_LeftCenter_temp(0x4E)", 0, {0} },
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "MB_RightFront_temp(0x4F)", 0, {0} },
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID)},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU1), "PSU-1 Thermal Sensor 2", ONLP_PSU_ID_CREATE(PSU1_ID)},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID)},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU2), "PSU-2 Thermal Sensor 2", ONLP_PSU_ID_CREATE(PSU2_ID)},
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
	[FAN_DIR_F2B][THERMAL_CPU_CORE].warning  = 70000,
	[FAN_DIR_F2B][THERMAL_CPU_CORE].error    = 75000,
	[FAN_DIR_F2B][THERMAL_CPU_CORE].shutdown = 95000,
	[FAN_DIR_F2B][THERMAL_1_ON_MAIN_BROAD].warning  = 60000,
	[FAN_DIR_F2B][THERMAL_1_ON_MAIN_BROAD].error    = 65000,
	[FAN_DIR_F2B][THERMAL_1_ON_MAIN_BROAD].shutdown = 70000,
	[FAN_DIR_F2B][THERMAL_2_ON_MAIN_BROAD].warning  = 60000,
	[FAN_DIR_F2B][THERMAL_2_ON_MAIN_BROAD].error    = 65000,
	[FAN_DIR_F2B][THERMAL_2_ON_MAIN_BROAD].shutdown = 70000,
	[FAN_DIR_F2B][THERMAL_3_ON_MAIN_BROAD].warning  = 57000,
	[FAN_DIR_F2B][THERMAL_3_ON_MAIN_BROAD].error    = 62000,
	[FAN_DIR_F2B][THERMAL_3_ON_MAIN_BROAD].shutdown = 67000,
	[FAN_DIR_F2B][THERMAL_4_ON_MAIN_BROAD].warning  = 65000,
	[FAN_DIR_F2B][THERMAL_4_ON_MAIN_BROAD].error    = 70000,
	[FAN_DIR_F2B][THERMAL_4_ON_MAIN_BROAD].shutdown = 75000,
	[FAN_DIR_F2B][THERMAL_5_ON_MAIN_BROAD].warning  = 60000,
	[FAN_DIR_F2B][THERMAL_5_ON_MAIN_BROAD].error    = 65000,
	[FAN_DIR_F2B][THERMAL_5_ON_MAIN_BROAD].shutdown = 70000,
	[FAN_DIR_F2B][THERMAL_1_ON_PSU1].warning  = 60000,
	[FAN_DIR_F2B][THERMAL_1_ON_PSU1].error    = 65000,
	[FAN_DIR_F2B][THERMAL_1_ON_PSU1].shutdown = 83000,
	[FAN_DIR_F2B][THERMAL_2_ON_PSU1].warning  = 60000,
	[FAN_DIR_F2B][THERMAL_2_ON_PSU1].error    = 65000,
	[FAN_DIR_F2B][THERMAL_2_ON_PSU1].shutdown = 77000,
	[FAN_DIR_F2B][THERMAL_1_ON_PSU2].warning  = 60000,
	[FAN_DIR_F2B][THERMAL_1_ON_PSU2].error    = 65000,
	[FAN_DIR_F2B][THERMAL_1_ON_PSU2].shutdown = 83000,
	[FAN_DIR_F2B][THERMAL_2_ON_PSU2].warning  = 60000,
	[FAN_DIR_F2B][THERMAL_2_ON_PSU2].error    = 65000,
	[FAN_DIR_F2B][THERMAL_2_ON_PSU2].shutdown = 77000,

	[FAN_DIR_B2F][THERMAL_CPU_CORE].warning  = 70000,
	[FAN_DIR_B2F][THERMAL_CPU_CORE].error    = 75000,
	[FAN_DIR_B2F][THERMAL_CPU_CORE].shutdown = 95000,
	[FAN_DIR_B2F][THERMAL_1_ON_MAIN_BROAD].warning  = 60000,
	[FAN_DIR_B2F][THERMAL_1_ON_MAIN_BROAD].error    = 65000,
	[FAN_DIR_B2F][THERMAL_1_ON_MAIN_BROAD].shutdown = 70000,
	[FAN_DIR_B2F][THERMAL_2_ON_MAIN_BROAD].warning  = 62000,
	[FAN_DIR_B2F][THERMAL_2_ON_MAIN_BROAD].error    = 67000,
	[FAN_DIR_B2F][THERMAL_2_ON_MAIN_BROAD].shutdown = 72000,
	[FAN_DIR_B2F][THERMAL_3_ON_MAIN_BROAD].warning  = 61000,
	[FAN_DIR_B2F][THERMAL_3_ON_MAIN_BROAD].error    = 66000,
	[FAN_DIR_B2F][THERMAL_3_ON_MAIN_BROAD].shutdown = 71000,
	[FAN_DIR_B2F][THERMAL_4_ON_MAIN_BROAD].warning  = 68000,
	[FAN_DIR_B2F][THERMAL_4_ON_MAIN_BROAD].error    = 73000,
	[FAN_DIR_B2F][THERMAL_4_ON_MAIN_BROAD].shutdown = 78000,
	[FAN_DIR_B2F][THERMAL_5_ON_MAIN_BROAD].warning  = 61000,
	[FAN_DIR_B2F][THERMAL_5_ON_MAIN_BROAD].error    = 66000,
	[FAN_DIR_B2F][THERMAL_5_ON_MAIN_BROAD].shutdown = 71000,
	[FAN_DIR_B2F][THERMAL_1_ON_PSU1].warning  = 60000,
	[FAN_DIR_B2F][THERMAL_1_ON_PSU1].error    = 65000,
	[FAN_DIR_B2F][THERMAL_1_ON_PSU1].shutdown = 83000,
	[FAN_DIR_B2F][THERMAL_2_ON_PSU1].warning  = 60000,
	[FAN_DIR_B2F][THERMAL_2_ON_PSU1].error    = 65000,
	[FAN_DIR_B2F][THERMAL_2_ON_PSU1].shutdown = 77000,
	[FAN_DIR_B2F][THERMAL_1_ON_PSU2].warning  = 60000,
	[FAN_DIR_B2F][THERMAL_1_ON_PSU2].error    = 65000,
	[FAN_DIR_B2F][THERMAL_1_ON_PSU2].shutdown = 83000,
	[FAN_DIR_B2F][THERMAL_2_ON_PSU2].warning  = 60000,
	[FAN_DIR_B2F][THERMAL_2_ON_PSU2].error    = 65000,
	[FAN_DIR_B2F][THERMAL_2_ON_PSU2].shutdown = 77000,
};

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
	return ONLP_STATUS_OK;
}

int get_max_cpu_coretemp(int *max_cpu_coretemp)
{
	int core_id = 0;
	int cpu_coretemp = 0;
	int i = 0;

	for(i = 0; i < NUM_OF_CPU_CORES; i++){
		if(onlp_file_read_int(&core_id, CPU_CORE_ID_PATH_FORMAT, i) < 0) {
			AIM_LOG_ERROR("Unable to read cpu core id from "CPU_CORE_ID_PATH_FORMAT"\r\n",
				i);
			*max_cpu_coretemp = 0;
			return ONLP_STATUS_E_INTERNAL;
		}

		if(onlp_file_read_int(&cpu_coretemp, CPU_CORETEMP_PATH_FORMAT, core_id+2) < 0) {
			AIM_LOG_ERROR("Unable to read cpu coretemp from "CPU_CORETEMP_PATH_FORMAT"\r\n",
				core_id+2);
			*max_cpu_coretemp = 0;
			return ONLP_STATUS_E_INTERNAL;
		}

		if(cpu_coretemp > *max_cpu_coretemp)
			*max_cpu_coretemp = cpu_coretemp;
	}

	// read package temp, which is always in temp1_input
	if(onlp_file_read_int(&cpu_coretemp, CPU_CORETEMP_PATH_FORMAT, 1) < 0) {
			AIM_LOG_ERROR("Unable to read cpu package temp from "CPU_CORETEMP_PATH_FORMAT"\r\n",
				1);
			*max_cpu_coretemp = 0;
			return ONLP_STATUS_E_INTERNAL;
	}

	*max_cpu_coretemp = (*max_cpu_coretemp > cpu_coretemp) ? 
							*max_cpu_coretemp : cpu_coretemp; 

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

	if (tid == THERMAL_CPU_CORE){
		return get_max_cpu_coretemp(&info->mcelsius);
	}

	return onlp_file_read_int(&info->mcelsius, devfiles__[tid]);
}
