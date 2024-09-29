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
#include <string.h>
#include "platform_lib.h"

#define PSU_STATUS_PRESENT 1
#define PSU_STATUS_POWER_GOOD 1

#define VALIDATE(_id)                           \
	do {                                        \
		if(!ONLP_OID_IS_PSU(_id)) {             \
			return ONLP_STATUS_E_INVALID;       \
		}                                       \
	} while(0)

static int
psu_status_info_get(int id, char *node, int *value)
{
	char *path[] = { PSU1_AC_EEPROM_PREFIX, PSU2_AC_EEPROM_PREFIX };
	*value = 0;

	return onlp_file_read_int(value, "%s%s", path[id-1], node);
}

int
onlp_psui_init(void)
{
	return ONLP_STATUS_OK;
}

static int
psu_ym2651y_info_get(onlp_psu_info_t* info)
{
	int val   = 0;
	int index = ONLP_OID_ID_GET(info->hdr.id);

	/* Set capability
	 */
	info->caps = ONLP_PSU_CAPS_AC;

	if (info->status & ONLP_PSU_STATUS_FAILED)
		return ONLP_STATUS_OK;

	/* Set the associated oid_table */
	info->hdr.coids[0] = ONLP_FAN_ID_CREATE(index + CHASSIS_FAN_COUNT);

	info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE((index-1) * 
						NUM_OF_THERMAL_PER_PSU 
						+ CHASSIS_THERMAL_COUNT
						+ 1);

	info->hdr.coids[2] = ONLP_THERMAL_ID_CREATE((index-1) * 
						NUM_OF_THERMAL_PER_PSU 
						+ CHASSIS_THERMAL_COUNT
						+ 2);

	/* Read voltage, current and power */
	if (psu_pmbus_info_get(index, "psu_v_out", &val) == 0) {
		info->mvout = val;
		info->caps |= ONLP_PSU_CAPS_VOUT;
	}

	if (psu_pmbus_info_get(index, "psu_i_out", &val) == 0) {
		info->miout = val;
		info->caps |= ONLP_PSU_CAPS_IOUT;
	}

	if (psu_pmbus_info_get(index, "psu_p_out", &val) == 0) {
		info->mpout = val;
		info->caps |= ONLP_PSU_CAPS_POUT;
	}

	get_psu_eeprom_str(index, info->serial, sizeof(info->serial), 
				"psu_serial_number");

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
	int ret   = ONLP_STATUS_OK;
	int index = ONLP_OID_ID_GET(id);
	psu_type_t psu_type;

	VALIDATE(id);

	memset(info, 0, sizeof(onlp_psu_info_t));
	*info = pinfo[index]; /* Set the onlp_oid_hdr_t */

	/* Get the present state */
	if (psu_status_info_get(index, "psu_present", &val) != 0) {
		AIM_LOG_ERROR("Unable to read PSU(%d) node(psu_present)\r\n", 
				index);
		return ONLP_STATUS_E_INTERNAL;
	}

	if (val != PSU_STATUS_PRESENT) {
		info->status &= ~ONLP_PSU_STATUS_PRESENT;
		return ONLP_STATUS_OK;
	}
	info->status |= ONLP_PSU_STATUS_PRESENT;


	/* Get power good status */
	if (psu_status_info_get(index, "psu_power_good", &val) != 0) {
		AIM_LOG_ERROR("Unable to read PSU(%d) node(psu_power_good)\r\n", 
				index);
		return ONLP_STATUS_E_INTERNAL;
	}

	/* Get PSU type */
	psu_type = get_psu_type(index, info->model, sizeof(info->model));

	if ((val != PSU_STATUS_POWER_GOOD) && (psu_type == PSU_TYPE_UNKNOWN)) {
		info->status |=  ONLP_PSU_STATUS_UNPLUGGED;
	} else if((val != PSU_STATUS_POWER_GOOD) && (psu_type != PSU_TYPE_UNKNOWN)){
		info->status |=  ONLP_PSU_STATUS_FAILED;
	}

	switch (psu_type) {
		case PSU_TYPE_UP1K21R_1085G_F2B:
		case PSU_TYPE_UPD1501SA_1179G_F2B:
		case PSU_TYPE_UPD1501SA_1279G_B2F:
			ret = psu_ym2651y_info_get(info);
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
