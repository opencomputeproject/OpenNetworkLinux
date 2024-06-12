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
 *
 *
 ***********************************************************/
#include <onlplib/i2c.h>
#include <unistd.h>
#include <onlplib/file.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"
#include <curl/curl.h>

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

#define MODEL_LEN 21
#define SERIAL_LEN 18
#define STRING_STRAT_LOC 2
static uint32_t ps[curl_data_psu_num];
static char ps_model[PS_DESC_LEN];
static char ps_serial[PS_DESC_LEN];
static char ps_rev[PS_DESC_LEN];
static bool ps_presence;
/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
{
    { }, /* Not used */
    {
        { ONLP_PSU_ID_CREATE(PSU1_ID), "PSU-1", 0 },
    },
    {
        { ONLP_PSU_ID_CREATE(PSU2_ID), "PSU-2", 0 },
    }
};

static void ps_call_back(void *p)
{
    int i = 0;
    int j = 0;
    int k = 0;
    char *data_loc_start;
    char *data_loc_end;
    char str[32];
    char *ret;
    ps_presence = PSU_ABSCENT;
    const char *ptr = (const char *)p;

    for (i = 0; i < curl_data_psu_num; i++)
        ps[i] = 0;

    if (ptr == NULL)
    {
        AIM_LOG_ERROR("NULL POINTER PASSED to call back function\n");
        return;
    }
    /* init ps var value */
    memset(ps_model, 0, sizeof(ps_model));
    memset(ps_serial, 0, sizeof(ps_serial));
    memset(ps_rev, 0, sizeof(ps_rev));

    ret = strstr(ptr, "absent");
    if (ret)
    {
        ps_presence = PSU_ABSCENT;
        return;
    }
    else
    {
        ps_presence = PSU_PRESENT;
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
        if (j >= PS_DESC_LEN) 
        { /* sanity check */
            AIM_LOG_ERROR("bad string len from BMC i %d j %d k %d 0x%02x\n", i, j, k, ptr[i]);
            return;
        }

        str[j] = '\0';
        if (k < curl_data_psu_num)
        {
            /* There are three string -  model name/serial/model ver */
            if ((k < curl_data_loc_psu_model_name) || (k > curl_data_loc_psu_model_ver))
            {
                ps[k] = atoi(str);
            }
            else
            {
                switch (k)
                {
                    case curl_data_loc_psu_model_name:
                        strncpy(ps_model, str, sizeof(ps_model));
                        break;
                    case curl_data_loc_psu_mode_serial:
                        strncpy(ps_serial, str, sizeof(ps_serial));
                        break;
                    case curl_data_loc_psu_model_ver:
                        strncpy(ps_rev, str, sizeof(ps_rev));
                        break;
                }
            }

            k++;
        }

        if (ptr[i] == ']') 
            break;

        i++;
    }

    return;
}

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int pid;
    char url[256] = {0};
    int curlid = 0;
    int still_running = 1;
    bool curl_response_status = 1;
    CURLMcode mc;

    VALIDATE(id);

    pid  = ONLP_OID_ID_GET(id);
    *info = pinfo[pid]; /* Set the onlp_oid_hdr_t */

    /* Get psu status by curl */
    snprintf(url, sizeof(url),"%s""ps/%d", BMC_CURL_PREFIX, pid);

    curlid = CURL_PSU_STATUS_1 + pid - 1;

    curl_easy_setopt(curl[curlid], CURLOPT_URL, url);
    curl_easy_setopt(curl[curlid], CURLOPT_WRITEFUNCTION, ps_call_back);
    curl_multi_add_handle(multi_curl, curl[curlid]);

    while(still_running) {
        mc = curl_multi_perform(multi_curl, &still_running);
        if(mc != CURLM_OK)
        {
            AIM_LOG_ERROR("multi_curl failed, code %d.\n", mc);
        }
    }

    if (PSU_PRESENT != ps_presence)
    {
        info->status &= ~ONLP_PSU_STATUS_PRESENT;

        return ONLP_STATUS_OK;
    }
    info->status |= ONLP_PSU_STATUS_PRESENT;
    /* only support AC type */
    info->caps = ONLP_PSU_CAPS_AC;
    /* There are no power good status. We use curl status to replace power good with Openbmc v1.x */
    curl_response_status = ps[curl_data_loc_psu_status];
    if (curl_data_normal != curl_response_status) 
    {
        info->status |= ONLP_PSU_STATUS_FAILED;
    }
    else
    {
        char model[MODEL_LEN+1];
        char serial[SERIAL_LEN+1];

        memset(model, 0, sizeof(model));
        memset(serial, 0, sizeof(serial));
        /* Parse the model name */
        memcpy(model, ps_model + STRING_STRAT_LOC, MODEL_LEN);
        model[MODEL_LEN+1] = '\0';
        /* Parse the serial number */
        memcpy(serial, ps_serial + STRING_STRAT_LOC, SERIAL_LEN);
        serial[SERIAL_LEN+1] = '\0';
        /* Get model name */
        strncpy(info->model, model, sizeof(info->model));
        /* Get serial number */
        strncpy(info->serial, serial, sizeof(info->serial));
        /* Read vin */
        info->mvin = ps[curl_data_loc_psu_vin] * 1000;
        info->caps |= ONLP_PSU_CAPS_VIN;
        /* Read iin */
        info->miin = ps[curl_data_loc_psu_iin];
        info->caps |= ONLP_PSU_CAPS_IIN;
        /* Get pin */
        info->mpin = ps[curl_data_loc_psu_pin];
        info->caps |= ONLP_PSU_CAPS_PIN;
        /* Get vout */
        info->mvout = ps[curl_data_loc_psu_vout] * 1000;
        info->caps |= ONLP_PSU_CAPS_VOUT;

        info->hdr.coids[0] = ONLP_FAN_ID_CREATE(pid + CHASSIS_FAN_COUNT);
    }

    return ONLP_STATUS_OK;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
