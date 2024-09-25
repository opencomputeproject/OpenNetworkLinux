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
#include <onlp/platformi/psui.h>
#include <onlplib/mmap.h>
//#include <stdio.h>
#include <string.h>
#include "platform_lib.h"

#define PSU_STATUS_PRESENT    1
#define PSU_STATUS_POWER_GOOD 1



#define VALIDATE(_id)                           \
do {                                        	\
	if(!ONLP_OID_IS_PSU(_id)) {             \
		return ONLP_STATUS_E_INVALID;   \
	}                                       \
} while(0)

static int psu_status_info_get(int id, char *node, int *value)
{
	int ret = 0;
	char path[PSU_NODE_MAX_PATH_LEN] = {0};

	*value = 0;

	if (PSU1_ID == id)
		sprintf(path, "%s%s", PSU1_AC_HWMON_PREFIX, node);
	else if (PSU2_ID == id)
		sprintf(path, "%s%s", PSU2_AC_HWMON_PREFIX, node);

	if (onlp_file_read_int(value, path) < 0) {
		AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", path);
		return ONLP_STATUS_E_INTERNAL;
	}

	return ret;
}

int onlp_psui_init(void)
{
	return ONLP_STATUS_OK;
}

static int psu_data_info_get(onlp_psu_info_t* info)
{
	int val   = 0;
	int index = ONLP_OID_ID_GET(info->hdr.id);

	if (info->status & ONLP_PSU_STATUS_FAILED)
		return ONLP_STATUS_OK;

	/* Set the associated oid_table */
	info->hdr.coids[0] = ONLP_FAN_ID_CREATE(index + CHASSIS_FAN_COUNT);
	info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(index 
				+ CHASSIS_THERMAL_COUNT);

	/* Read voltage, current and power */
	if (psu_pmbus_info_get(index, "psu_v_out", &val) == 0) {
		info->mvout = val;
		info->caps |= ONLP_PSU_CAPS_VOUT;
	}

	if (psu_pmbus_info_get(index, "psu_v_in", &val) == 0) {
		info->mvin = val;
		info->caps |= ONLP_PSU_CAPS_VIN;
	}

	if (psu_pmbus_info_get(index, "psu_i_out", &val) == 0) {
		info->miout = val;
		info->caps |= ONLP_PSU_CAPS_IOUT;
	}

	if (psu_pmbus_info_get(index, "psu_i_in", &val) == 0) {
		info->miin = val;
		info->caps |= ONLP_PSU_CAPS_IIN;
	}

	if (psu_pmbus_info_get(index, "psu_p_out", &val) == 0) {
		info->mpout = val;
		info->caps |= ONLP_PSU_CAPS_POUT;
	}

	if (psu_pmbus_info_get(index, "psu_p_in", &val) == 0) {
		info->mpin = val;
		info->caps |= ONLP_PSU_CAPS_PIN;
	}

	if(info->status & ONLP_PSU_STATUS_UNPLUGGED) {
		info->mvout = 0;
		info->mvin = 0;
		info->miout = 0;
		info->miin = 0;
		info->mpout = 0;
		info->mpin = 0;
	}

	psu_serial_number_get(index, info->serial, sizeof(info->serial));
	
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

int onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
	int val   = 0;
	int ret   = ONLP_STATUS_OK;
	int index = ONLP_OID_ID_GET(id);
	psu_type_t psu_type;

	VALIDATE(id);

	memset(info, 0, sizeof(onlp_psu_info_t));
	*info = pinfo[index]; /* Set the onlp_oid_hdr_t */

	/* Get the present state */
	if (psu_status_info_get(index, "psu_present", &val) != 0)
		AIM_LOG_ERROR("Unable to read PSU(%d) node(psu_present)\r\n",
				index);

	if (val != PSU_STATUS_PRESENT) {
		info->status &= ~ONLP_PSU_STATUS_PRESENT;
		return ONLP_STATUS_OK;
	}

	info->status |= ONLP_PSU_STATUS_PRESENT;

	/* Get power good status */
	if (psu_status_info_get(index, "psu_power_good", &val) != 0)
		AIM_LOG_ERROR("Unable to read PSU(%d) node(psu_power_good)\r\n",
				index);

	if (val != PSU_STATUS_POWER_GOOD)
		info->status |=  ONLP_PSU_STATUS_UNPLUGGED;

	/* Get PSU type
	 */
	psu_type = get_psu_type(index, info->model, sizeof(info->model));

	switch (psu_type) {
	case PSU_TYPE_AC_ACBEL_F2B:
	case PSU_TYPE_AC_ACBEL_B2F:
	case PSU_TYPE_AC_BELPOWER_F2B:
	case PSU_TYPE_AC_BELPOWER_B2F:
		info->caps = ONLP_PSU_CAPS_AC;
		ret = psu_data_info_get(info);
		break;
	case PSU_TYPE_DC_48V_ACBEL_F2B:
	case PSU_TYPE_DC_48V_ACBEL_B2F:
		info->caps = ONLP_PSU_CAPS_DC48;
		ret = psu_data_info_get(info);
		break;
	case PSU_TYPE_UNKNOWN:  /* User insert a unknown PSU or unplugged.*/
		info->status |= ONLP_PSU_STATUS_UNPLUGGED;
		info->status &= ~ONLP_PSU_STATUS_FAILED;
		ret = ONLP_STATUS_OK;
		break;
	default:
		ret = ONLP_STATUS_E_UNSUPPORTED;
		break;
	}

	return ret;
}

int onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
	return ONLP_STATUS_E_UNSUPPORTED;
}
