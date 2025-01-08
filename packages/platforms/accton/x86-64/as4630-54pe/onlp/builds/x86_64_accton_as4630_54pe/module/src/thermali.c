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
#include <dirent.h>
#include <regex.h>

#define THERMAL_PATH_FORMAT 	"/sys/bus/i2c/devices/%s/*temp1_input"
#define PSU_THERMAL_PATH_FORMAT "/sys/bus/i2c/devices/%s/*psu_temp1_input"
#define CPU_CORETEMP_DIR_PATH "/sys/devices/platform/coretemp.0/hwmon"
#define MAX_ENTRIES 128  /* Maximum number of temp*_input files */
#define MAX_NAME_LEN 256 /* Maximum file name length */

typedef struct {
    char name[MAX_NAME_LEN];
    int index;
} TempEntry;

TempEntry temp_entries[MAX_ENTRIES];
/* Global regex variables */
regex_t dir_regex, file_regex;
int sysfs_count = 0;
/* Function to safely concatenate paths and handle overflow */
int safe_snprintf(char *buffer, size_t buffer_size, const char *path1, const char *path2) {
    size_t needed_size = strlen(path1) + strlen(path2) + 2; /* 1 for '/', 1 for '\0' */
    if (needed_size > buffer_size) {
        AIM_LOG_ERROR("Path too long: %s/%s\n", path1, path2);
        return -1;
    }
    snprintf(buffer, buffer_size, "%s/%s", path1, path2);
    return 0;
}

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,   
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
};

static char* directory[] =  /* must map with onlp_thermal_id */
{
    NULL,
    NULL,                  /* CPU_CORE files */
    "24-004b",
    "25-004a",
    "14-0048",
    "10-0058",
    "11-0059",
};

/* Static values */
static onlp_thermal_info_t linfo[] = {
	{ }, /* Not used */
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },	
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "LM75-1-4B", 0}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "LM75-2-4A", 0}, 
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "LM75-3-48", 0}, 
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

int initialize_regex() {
    if (regcomp(&dir_regex, "^hwmon[0-9]+$", REG_EXTENDED) != 0) {
        AIM_LOG_ERROR("Failed to compile directory regex\n");
        return -1;
    }
    if (regcomp(&file_regex, "^temp([0-9]+)_input$", REG_EXTENDED) != 0) {
        AIM_LOG_ERROR("Failed to compile file regex\n");
        regfree(&dir_regex);
        return -1;
    }
    return 0;
}

void cleanup_regex() {
    regfree(&dir_regex);
    regfree(&file_regex);
}

/* Function to scan hwmon directories for temp*_input files */
void scan_hwmon_files(const char *parent_path) {
    DIR *parent_dir = opendir(parent_path);
    if (!parent_dir) {
        AIM_LOG_ERROR("Failed to open parent directory");
        return;
    }

    size_t count = 0;

    struct dirent *entry;
    while ((entry = readdir(parent_dir)) != NULL) {
        if (regexec(&dir_regex, entry->d_name, 0, NULL, 0) != 0) {
            continue;
        }

        char subdir_path[MAX_NAME_LEN];
        if (safe_snprintf(subdir_path, sizeof(subdir_path), parent_path, entry->d_name) < 0) {
            continue;
        }

        DIR *subdir = opendir(subdir_path);
        if (!subdir) {
            AIM_LOG_ERROR("Failed to open subdirectory");
            continue;
        }

        struct dirent *sub_entry;
        while ((sub_entry = readdir(subdir)) != NULL) {
            regmatch_t pmatch[2];
            if (regexec(&file_regex, sub_entry->d_name, 2, pmatch, 0) == 0) {
                if (count >= MAX_ENTRIES) {
                    AIM_LOG_ERROR("Maximum entries reached. Ignoring additional files.\n");
                    break;
                }

                char full_path[MAX_NAME_LEN];
                if (safe_snprintf(full_path, sizeof(full_path), subdir_path, sub_entry->d_name) < 0) {
                    continue;
                }

                int index = atoi(&sub_entry->d_name[pmatch[1].rm_so]);
                strncpy(temp_entries[count].name, full_path, MAX_NAME_LEN - 1);
                temp_entries[count].name[MAX_NAME_LEN - 1] = '\0';
                temp_entries[count].index = index;
                count++;
            }
        }
        closedir(subdir);
    }

    closedir(parent_dir);

    sysfs_count = count;
}

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
    /* Initialize regex patterns */
    if (initialize_regex() != 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    scan_hwmon_files(CPU_CORETEMP_DIR_PATH);
    /* Clean up regex patterns */
    cleanup_regex();

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
    int   tid;
	char *format   = NULL;
	char  path[64] = {0};
    VALIDATE(id);
	
    tid = ONLP_OID_ID_GET(id);
    int coretemp_max = 0, coretemp_temp = 0;
	
    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[tid];
    if(tid == THERMAL_CPU_CORE) {
        for (size_t i = 0; i < sysfs_count; i++) {
            if (onlp_file_read_int(&coretemp_temp, temp_entries[i].name) < 0) {
                AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", temp_entries[i].name);
                return ONLP_STATUS_E_INTERNAL;
            }
            if (coretemp_temp > coretemp_max)
                coretemp_max = coretemp_temp;
        }

        info->mcelsius = coretemp_max;

        return ONLP_STATUS_OK;
    }

	switch (tid) {
    	case THERMAL_1_ON_MAIN_BROAD:
    	case THERMAL_2_ON_MAIN_BROAD:
    	case THERMAL_3_ON_MAIN_BROAD:
			format = THERMAL_PATH_FORMAT;
			break;
    	case THERMAL_1_ON_PSU1:
    	case THERMAL_1_ON_PSU2:
			format = PSU_THERMAL_PATH_FORMAT;
			break;
		default:
			return ONLP_STATUS_E_INVALID;
	};
	
    /* get path */
    sprintf(path, format, directory[tid], tid);
    if (onlp_file_read_int(&info->mcelsius, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

