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
 * PSU Platform Implementation Defaults.
 *
 ***********************************************************/
#include <stdio.h>
#include <string.h>
#include <onlplib/file.h>
#include <onlplib/mmap.h>
#include <onlp/platformi/psui.h>
#include <mlnx_common/mlnx_common.h>
#include "mlnx_common_log.h"

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

int
psu_module_info_get(int id, char *node, int *value)
{
    int ret = 0;

    *value = 0;

    ret = onlp_file_read_int(value, PSU_MODULE_PREFIX, id, node);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ret;
}

int
psu_power_info_get(int id, char *node, int *value)
{
    int ret = 0;

    *value = 0;

    ret = onlp_file_read_int(value, PSU_POWER_PREFIX, id, node);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ret;
}

int
onlp_psui_init(void)
{
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

static int
_psu_info_get_type1(onlp_psu_info_t* info)
{
    int val   = 0;
    int index = ONLP_OID_ID_GET(info->hdr.id);
    const char psu_model[]=PSU_MODEL;

    strncpy(info->model, psu_model, sizeof(info->model));

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

static int
_psu_info_get_type2(onlp_psu_info_t* info)
{
    int val   = 0;
    int index = ONLP_OID_ID_GET(info->hdr.id);

    mlnx_platform_info_t* mlnx_platform_info = get_platform_info();
    /* Set capability
     */
    info->caps = ONLP_PSU_CAPS_AC;

    if (info->status & ONLP_PSU_STATUS_FAILED) {
        return ONLP_STATUS_OK;
    }

    /* Set the associated oid_table */
    info->hdr.coids[0] = ONLP_FAN_ID_CREATE(index + mlnx_platform_info->fan_num);
    info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(index + mlnx_platform_info->thermal_num);

    /* Read voltage, current and power */
    if (psu_power_info_get(index, "volt_in", &val) == 0 &&
            0 != val) {
        info->mvin = val;
        info->caps |= ONLP_PSU_CAPS_VIN;

        if (psu_power_info_get(index, "volt", &val) == 0) {
            info->mvout = val;
            info->caps |= ONLP_PSU_CAPS_VOUT;
        }

        if (psu_power_info_get(index, "curr_in", &val) == 0) {
            info->miin = val;
            info->caps |= ONLP_PSU_CAPS_IIN;
        }

        if (psu_power_info_get(index, "curr", &val) == 0) {
            info->miout = val;
            info->caps |= ONLP_PSU_CAPS_IOUT;
        }

        if (psu_power_info_get(index, "power_in", &val) == 0) {
            info->mpin = val;
            info->caps |= ONLP_PSU_CAPS_PIN;
        }

        if (psu_power_info_get(index, "power", &val) == 0) {
            info->mpout = val;
            info->caps |= ONLP_PSU_CAPS_POUT;
        }
    } else {
        info->status |= ONLP_PSU_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }

    return psu_read_eeprom(index, info, NULL);
}

int _psu_info_get(onlp_psu_info_t* info)
{
    int res;
    mlnx_platform_info_t* mlnx_platform_info = get_platform_info();
    switch(mlnx_platform_info->psu_type) {
      case PSU_TYPE_1:
	  res=_psu_info_get_type1(info);
	  break;
      case PSU_TYPE_2:
	  res=_psu_info_get_type2(info);
	  break;
    }
    return res;
}

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int val   = 0;
    int ret   = ONLP_STATUS_OK;
    int index = ONLP_OID_ID_GET(id);

    VALIDATE(id);

    memset(info, 0, sizeof(onlp_psu_info_t));
    *info = pinfo[index]; /* Set the onlp_oid_hdr_t */

    mlnx_platform_info_t* mlnx_platform_info = get_platform_info();
       if(mlnx_platform_info->psu_fixed == false) {
	  /* Get the present state */
	  if (psu_module_info_get(index, "status", &val) != 0) {
	      AIM_LOG_ERROR("Unable to read PSU(%d) node(psu_present)\r\n", index);
	  }

	  if (val != PSU_STATUS_PRESENT) {
	      info->status &= ~ONLP_PSU_STATUS_PRESENT;
	      info->status |= ONLP_PSU_STATUS_UNPLUGGED;
	      return ONLP_STATUS_OK;
	  }
	  else
	      info->status |= ONLP_PSU_STATUS_PRESENT;
	 }
       else {
	 /* Fixed system, PSU is always present */
	     info->status |= ONLP_PSU_STATUS_PRESENT;
       }

    /* Get the cable preset state */
    if (psu_module_info_get(index, "pwr_status", &val) != 0) {
        AIM_LOG_ERROR("Unable to read PSU(%d) node(cable_present)\r\n", index);
    }

    if (val != PSU_CABLE_PRESENT) {
        info->status |= ONLP_PSU_STATUS_UNPLUGGED;
        return ONLP_STATUS_OK;
    }
    if(mlnx_platform_info->psu_fixed == false) {
	info->status |= ONLP_PSU_STATUS_PRESENT;
    }
    ret = _psu_info_get(info);

    return ret;
}
