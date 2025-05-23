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
#include <glob.h>
#include <libgen.h>

#define CPU_CORETEMP_PATH "/sys/devices/platform/coretemp.0/hwmon/hwmon*/temp*_input"
#define CPU_TEMP_SCAN_MAX_FILES 64   /* Maximum number of temperature input files to scan */
#define CPU_TEMP_PATH_MAX_LEN 256    /* Maximum temperature file path length */

#define VALIDATE(_id)                           \
	do {                                        \
		if(!ONLP_OID_IS_THERMAL(_id)) {         \
			return ONLP_STATUS_E_INVALID;       \
		}                                       \
	} while(0)

/*
 * Structure to store temperature sensor file information
 */
typedef struct {
    char path[CPU_TEMP_PATH_MAX_LEN]; /* Full path of the temperature file */
    int index;               /* Index number extracted from the filename */
} temp_entry;

size_t get_temp_from_glob_pattern(const char *pattern, temp_entry *result, size_t max_results);

enum onlp_thermal_id {
	THERMAL_RESERVED = 0,
	THERMAL_CPU_CORE,
	THERMAL_1_ON_MAIN_BROAD,
	THERMAL_3_ON_MAIN_BROAD,
	THERMAL_5_ON_MAIN_BROAD,
	THERMAL_1_ON_PSU1,
	THERMAL_2_ON_PSU1,
	THERMAL_3_ON_PSU1,
	THERMAL_1_ON_PSU2,
	THERMAL_2_ON_PSU2,
	THERMAL_3_ON_PSU2,
};

static char* devfiles__[] = { /* must map with onlp_thermal_id */
	NULL,
	NULL, /* CPU_CORE files */
	"/sys/bus/i2c/devices/3-004a*temp1_input",
	"/sys/bus/i2c/devices/3-004d*temp1_input",
	"/sys/bus/i2c/devices/3-004f*temp1_input",
	"/sys/bus/i2c/devices/8-0058*psu_temp1_input",
	"/sys/bus/i2c/devices/8-0058*psu_temp2_input",
	"/sys/bus/i2c/devices/8-0058*psu_temp3_input",
	"/sys/bus/i2c/devices/9-0059*psu_temp1_input",
	"/sys/bus/i2c/devices/9-0059*psu_temp2_input",
	"/sys/bus/i2c/devices/9-0059*psu_temp3_input"
};

/* Static values */
static onlp_thermal_info_t linfo[] = {
	{ }, /* Not used */
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0, {0} },
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, { 66000, 69000, 71000 }
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "MB_RightRear_temp(0x4A)", 0, {0} },
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, { 62000, 64000, 65000 }
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "MB_LeftFront_temp(0x4D)", 0, {0} },
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, { 59000, 61000, 62000 }
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "MB_RightFront_temp(0x4F)", 0, {0} },
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, { 62000, 64000, 65000 }
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID)},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, { 61000, 61000, 72000 }
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU1), "PSU-1 Thermal Sensor 2", ONLP_PSU_ID_CREATE(PSU1_ID)},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, { 65000, 65000, 79000 }
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_PSU1), "PSU-1 Thermal Sensor 3", ONLP_PSU_ID_CREATE(PSU1_ID)},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, { 65000, 65000, 99000 }
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID)},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, { 61000, 61000, 72000 }
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU2), "PSU-2 Thermal Sensor 2", ONLP_PSU_ID_CREATE(PSU2_ID)},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, { 65000, 65000, 79000 }
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_PSU2), "PSU-2 Thermal Sensor 3", ONLP_PSU_ID_CREATE(PSU2_ID)},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, { 65000, 65000, 99000 }
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

int get_max_cpu_coretemp(int *max_cpu_coretemp)
{
	int cpu_coretemp = 0;
	int i = 0;
	size_t count;
	temp_entry temp_entries[CPU_TEMP_SCAN_MAX_FILES] = {0};

	*max_cpu_coretemp = 0;
	count = get_temp_from_glob_pattern(CPU_CORETEMP_PATH, temp_entries, AIM_ARRAYSIZE(temp_entries));
	if (count == 0) {
		AIM_LOG_ERROR("No CPU core temperature sensors found\n");
		return ONLP_STATUS_E_INTERNAL;
	}

	for(i = 0; i < count; i++){
		if(onlp_file_read_int(&cpu_coretemp, temp_entries[i].path) < 0) {
			AIM_LOG_ERROR("Unable to read cpu coretemp from %s\r\n", temp_entries[i].path);
			return ONLP_STATUS_E_INTERNAL;
		}

		if(cpu_coretemp > *max_cpu_coretemp) {
			*max_cpu_coretemp = cpu_coretemp;
		}
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
	int tid;
	int psu_id;
	psu_type_t psu_type;
	VALIDATE(id);

	tid = ONLP_OID_ID_GET(id);

	/* Set the onlp_oid_hdr_t and capabilities */
	*info = linfo[tid];

	if ((tid >= THERMAL_1_ON_PSU1) && (tid <= THERMAL_3_ON_PSU2)) {
		psu_id = ((tid - THERMAL_1_ON_PSU1) / NUM_OF_THERMAL_PER_PSU) + 1;

		/* Get PSU type */
		psu_type = get_psu_type(psu_id, NULL, 0);
		if (psu_type == PSU_TYPE_UNKNOWN) {
			/* For display all PSU information that includes the fan and 
                         * temperature, even access hardware fail.
			 */
			info->status |= ONLP_THERMAL_STATUS_FAILED;
			return ONLP_STATUS_OK;
		}
	}

	if (tid == THERMAL_CPU_CORE) {
		return get_max_cpu_coretemp(&info->mcelsius);
	}

	return onlp_file_read_int(&info->mcelsius, devfiles__[tid]);
}

/**
 * Finds temperature sensor files matching a glob pattern and sorts them by index
 *
 * This function uses glob pattern matching to find temperature sensor files in the
 * file system, extracts the index number from each filename, and sorts the results
 * by this index for consistent processing order.
 *
 * @param pattern The glob pattern to match sensor files (e.g. "/sys/class/hwmon/hwmon[*]/temp[*]_input")
 * @param result Pre-allocated array to store the matched file entries
 * @param max_results Maximum number of results to store in the result array
 * @return Number of temperature files found and stored in the result array, 0 on error
 */
size_t get_temp_from_glob_pattern(const char *pattern, temp_entry *result, size_t max_results) {
	glob_t glob_result;
	size_t i, count = 0;
	int index;

	if (!pattern || !result || max_results == 0) {
		return 0;
	}

	/* Perform glob pattern matching with GLOB_NOSORT for better performance */
	if (glob(pattern, GLOB_NOSORT, NULL, &glob_result) != 0) {
		AIM_LOG_ERROR("Failed to find files matching pattern: %s\n", pattern);
		return 0;
	}

	/* Process each matched file and extract temperature indices */
	for (i = 0; i < glob_result.gl_pathc && count < max_results; i++) {
		char path_buf[CPU_TEMP_PATH_MAX_LEN], *filename;
		const char *full_path = glob_result.gl_pathv[i];

		if (strlen(glob_result.gl_pathv[i]) >= CPU_TEMP_PATH_MAX_LEN) {
			AIM_LOG_ERROR("Path too long (max %d chars): %s\n", 
				      CPU_TEMP_PATH_MAX_LEN - 1, full_path);
			continue;
		}

		snprintf(path_buf, sizeof(path_buf), "%s", full_path);
		filename = basename(path_buf);
		if (filename && sscanf(filename, "temp%d_input", &index) == 1) {
			snprintf(result[count].path, sizeof(result[count].path), "%s", full_path);
			result[count].index = index;
			count++;
		}
	}

	/* Free glob resources */
	globfree(&glob_result);

	return count;
}
