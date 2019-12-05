/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2019 WNC Computing Technology Corporation.
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
#include <onlp/platformi/thermali.h>
#include <fcntl.h>
#include "platform_lib.h"
#include <onlplib/file.h>

#define PSU_THERMAL_PATH_MAX_STR_LEN 128
#define THERMAL_PATH_FORMAT "/sys/bus/i2c/devices/%s/hwmon/hwmon%s/temp1_input"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static char* i2c_directory[] =  /* must map with onlp_thermal_id */
{
    NULL,
    NULL,        /* CPU_CORE files */
    "6-004a",    /* on MAIN board */
    "6-004b",    /* on MAIN board */
    "6-004c",    /* on MAIN board */
    "0-004f",    /* on CPU board */
    "78-004e",   /* on FAN board */
    "18-0058",   /* on PUS-1 */
    "19-0058",   /* on PUS-2 */
};

static char* hwmon_directory[] =  /* must map with onlp_thermal_id */
{
    NULL,
    NULL,   /* CPU_CORE files */
    "11",   /* on MAIN board */
    "12",   /* on MAIN board */
    "13",   /* on MAIN board */
    "5",    /* on CPU board */
    "16",   /* on FAN board */
    "14",   /* on PUS-1 */
    "15",   /* on PUS-2 */
};

static char* psu_1_temp_files[] =
{
    "/sys/bus/i2c/devices/18-0058/hwmon/hwmon14/temp1_input",
    "/sys/bus/i2c/devices/18-0058/hwmon/hwmon14/temp2_input",
    "/sys/bus/i2c/devices/18-0058/hwmon/hwmon14/temp3_input",
    NULL,
};

static char* psu_2_temp_files[] =
{
    "/sys/bus/i2c/devices/19-0058/hwmon/hwmon15/temp1_input",
    "/sys/bus/i2c/devices/19-0058/hwmon/hwmon15/temp2_input",
    "/sys/bus/i2c/devices/19-0058/hwmon/hwmon15/temp3_input",
    NULL,
};

/* Static values */
static onlp_thermal_info_t linfo[] = {
    { }, /* Not used */
    { { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0},
      ONLP_THERMAL_STATUS_PRESENT,
      ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "LM75-1-4A", 0},
      ONLP_THERMAL_STATUS_PRESENT,
      ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "LM75-2-4B", 0},
      ONLP_THERMAL_STATUS_PRESENT,
      ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "LM75-3-4C", 0},
      ONLP_THERMAL_STATUS_PRESENT,
      ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_CPU_BROAD), "LM75-4-4F", 0},
      ONLP_THERMAL_STATUS_PRESENT,
      ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_FAN_BROAD), "LM75-5-4E", 0},
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
 * Get CPU temperature for system nodes and return the maximum temprature of them.
*/
int static get_cpu_core_temp_max(void)
{
    FILE *fp;
    int tmp, max_temp = 0;

    /* Get system node of cpu core temprature */
    if ((fp = popen("ls /sys/devices/platform/coretemp.0/hwmon/hwmon1/temp*_input | xargs cat", "r")) == NULL){
        AIM_LOG_ERROR("Unable to get numbers of cpu core temprature system node \r\n");
    }

    while (fscanf(fp, "%d", &tmp) != EOF)
    {
        if (tmp > max_temp)
            max_temp = tmp;
    }

    pclose(fp);

    return max_temp;
}

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
    int   tid;
    char  path[64] = {0};

    VALIDATE(id);

    tid = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[tid];

    switch (tid) 
    {
        case THERMAL_CPU_CORE:
            return get_cpu_core_temp_max();
        case THERMAL_1_ON_MAIN_BROAD:
        case THERMAL_2_ON_MAIN_BROAD:
        case THERMAL_3_ON_MAIN_BROAD:
        case THERMAL_4_ON_CPU_BROAD:
        case THERMAL_5_ON_FAN_BROAD:
            /* get path */
            sprintf(path, THERMAL_PATH_FORMAT, i2c_directory[tid], hwmon_directory[tid]);

            if (onlp_file_read_int(&info->mcelsius, path) < 0) {
                AIM_LOG_ERROR("Unable to read thermal from file (%s)\r\n", path);
                return ONLP_STATUS_E_INTERNAL;
            }
            break;
        case THERMAL_1_ON_PSU1:
            return onlp_file_read_int_max(&info->mcelsius, psu_1_temp_files);
        case THERMAL_1_ON_PSU2:
            return onlp_file_read_int_max(&info->mcelsius, psu_2_temp_files);
        default:
            return ONLP_STATUS_E_INVALID;
    };

    return ONLP_STATUS_OK;
}