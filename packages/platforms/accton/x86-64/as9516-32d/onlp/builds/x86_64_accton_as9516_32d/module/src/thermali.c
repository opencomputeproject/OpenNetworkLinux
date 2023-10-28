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
#include <curl/curl.h>

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static char* cpu_coretemp_files[] =
{
        "/sys/devices/platform/coretemp.0*temp1_input",
        "/sys/devices/platform/coretemp.0*temp2_input",
        "/sys/devices/platform/coretemp.0*temp3_input",
        "/sys/devices/platform/coretemp.0*temp4_input",
        "/sys/devices/platform/coretemp.0*temp5_input",
        NULL,
};

static uint32_t tmp[curl_data_thermal_num] = {0};
static int curl_thermal_data_loc[curl_data_thermal_num] = 
{
    curl_data_loc_psu_status,
    /* 3-0048 tmp75_3_48_temp */
    curl_data_loc_thermal_3_48,
    /* 3-0049 tmp75_3_49_temp */
    curl_data_loc_thermal_3_49,
    /* 3-004a tmp75_3_4a_temp */
    curl_data_loc_thermal_3_4a,
    /* 3-004b tmp75_3_4b_temp */
    curl_data_loc_thermal_3_4b,
    /* 3-004c tmp75_3_4c_temp Max6658 */
    curl_data_loc_thermal_3_4c_max6658,
    /* 3-004d tmp75_3_4d_temp */
    curl_data_loc_thermal_3_4d
};

/* Static values */
static onlp_thermal_info_t linfo[] =
{
    { }, /* Not used */ 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "TMP75 0x48", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "TMP75 0x49", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "TMP75 0x4a", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "TMP75 0x4b", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "MAX6658 0x4c", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BROAD), "TMP75 0x4d", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
};

void tmp_call_back(void *p) 
{
    int i = 0;
    int j = 0;
    int k = 0;
    char *data_loc_start;
    char *data_loc_end;
    char str[40];
    char *ptr = p;

    for (i = 0; i < curl_data_thermal_num; i++)
        tmp[i] = -1;

    if (ptr == NULL)
    {
        AIM_LOG_ERROR("NULL POINTER PASSED to call back function\n");
        return;
    }

    data_loc_start = strchr(ptr, '[');
    data_loc_end = strchr(ptr, ']');
    if((data_loc_start == NULL) || (data_loc_end == NULL))
    {
        AIM_LOG_ERROR("Failed CURL response data.\n");
        return;
    }
    /* Start to parse from the first value, ignore '[' */
    i = data_loc_start - ptr + 1;
    while (ptr[i] && ptr[i] != ']')
    {
        j = 0;
        while (ptr[i] != ',' && ptr[i] != ']')
        {
            str[j] = ptr[i];
            j++;
            i++;
        }

        str[j] = '\0';

        if (k < curl_data_thermal_num)
        {
            tmp[k] = atoi(str);
            k++;
        }

        if (ptr[i] == ']') 
            break;

        i++;
    }

    return;
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
    VALIDATE(id);
    char url[256] = {0};
    CURLMcode mc;
    int still_running = 1;
    
    tid = ONLP_OID_ID_GET(id);
    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[tid];
    /* CPU core thermal */
    if (THERMAL_CPU_CORE == tid) {
        return onlp_file_read_int_max(&info->mcelsius, cpu_coretemp_files);
    }
    else
    {
        /* just need to do curl once 
          tmp[0] response_status [0:success ,!=0:error],
          tmp[1] temp1_input  in 3-0048
          tmp[2] temp1_input  in 3-0049
          tmp[3] temp1_input in 3-004a
          tmp[4] temp1_input in 3-004b 
          tmp[5] temp1_input in 3-004c max6658 thermal sensor ,temperature in local position
          tmp[7] temp1_input in 3-004d 
        */
        snprintf(url, sizeof(url),"%s""tmp/newport", BMC_CURL_PREFIX);

        curl_easy_setopt(curl[CURL_THERMAL], CURLOPT_URL, url);
        curl_easy_setopt(curl[CURL_THERMAL], CURLOPT_WRITEFUNCTION, tmp_call_back);
        curl_multi_add_handle(multi_curl, curl[CURL_THERMAL]);
        while(still_running) {
            mc = curl_multi_perform(multi_curl, &still_running);
            if(mc != CURLM_OK)
            {
                AIM_LOG_ERROR("multi_curl failed, code %d.\n", mc);

                return ONLP_STATUS_E_INTERNAL;
            }
        }

        info->mcelsius = tmp[curl_thermal_data_loc[tid-1]]*100;
    }

    return ONLP_STATUS_OK;
}
