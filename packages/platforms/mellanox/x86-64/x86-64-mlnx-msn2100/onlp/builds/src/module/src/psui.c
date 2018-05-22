/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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
#include <stdio.h>
#include <string.h>
#include <onlplib/file.h>
#include <onlplib/mmap.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#define PSU_STATUS_PRESENT   1
#define PSU_CABLE_PRESENT    1

#define PSU_NODE_MAX_INT_LEN  8
#define PSU_NODE_MAX_PATH_LEN 64

#define PSU_MODEL	"POW000167"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static int
psu_module_info_get(int id, char *node, int *value)
{
    int len, ret = 0;
    char buf[PSU_NODE_MAX_INT_LEN + 1] = {0};

    *value = 0;

    ret = onlp_file_read((uint8_t*)buf, sizeof(buf), &len,
    		PSU_MODULE_PREFIX, id, node);
    if (ret == 0) {
        *value = atoi(buf);
    }

    return ret;
}

static int
psu_power_info_get(int id, char *node, int *value)
{
    int len, ret = 0;
    char buf[PSU_NODE_MAX_INT_LEN + 1] = {0};

    *value = 0;

    ret = onlp_file_read((uint8_t*)buf, sizeof(buf), &len,
    		PSU_POWER_PREFIX, id, node);
    if (ret == 0) {
        *value = atoi(buf);
    }

    return ret;
}

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

static int
_psu_info_get(onlp_psu_info_t* info)
{
    int val   = 0;
    int index = ONLP_OID_ID_GET(info->hdr.id);

    /* Set capability */
    info->caps = ONLP_PSU_CAPS_AC;

    if (info->status & ONLP_PSU_STATUS_FAILED) {
        return ONLP_STATUS_OK;
    }

    /* Read voltage, current and power */
	if (psu_power_info_get(index, "volt", &val) == 0) {
		info->mvout = val;
		info->caps |= ONLP_PSU_CAPS_VOUT;
	}

	if (psu_power_info_get(index, "curr", &val) == 0) {
		info->miout = val;
		info->caps |= ONLP_PSU_CAPS_IOUT;
	}

	info->mpout = info->mvout * info->miout;
	info->caps |= ONLP_PSU_CAPS_POUT;

	info->mpin = ((int)(info->mpout / 91)) * 100;
	info->caps |= ONLP_PSU_CAPS_PIN;

    return ONLP_STATUS_OK;
}

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

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int val   = 0;
    int index = ONLP_OID_ID_GET(id);
    const char psu_model[]=PSU_MODEL;

    VALIDATE(id);

    memset(info, 0, sizeof(onlp_psu_info_t));
    *info = pinfo[index]; /* Set the onlp_oid_hdr_t */

    /* Fixed system, PSU is always present */
	info->status |= ONLP_PSU_STATUS_PRESENT;

	strncpy(info->model, psu_model, sizeof(info->model));

    /* Get the cable preset state */
    if (psu_module_info_get(index, "pwr_status", &val) != 0) {
        AIM_LOG_ERROR("Unable to read PSU(%d) node(cable_present)\r\n", index);
    }

    if (val != PSU_CABLE_PRESENT) {
        info->status |= ONLP_PSU_STATUS_UNPLUGGED;
    }
    else {
    	info->status &= ~ONLP_PSU_STATUS_UNPLUGGED;
    }

    _psu_info_get(info);

    return ONLP_STATUS_OK;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

