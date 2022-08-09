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
#include <onlp/platformi/fani.h>
#include <onlplib/file.h>
#include "x86_64_quanta_ix8_rglbmc_int.h"
#include "x86_64_quanta_ix8_rglbmc_log.h"

int
onlp_fani_init(void)
{
    return ONLP_STATUS_OK;
}

static int
sys_fan_info_get__(onlp_fan_info_t* info, int id)
{
	int err = 0, strlen = 0;
	int fan_present = 0, direction = 0;
	int tid = ONLP_OID_ID_GET(id);

	char *tempstr = NULL;

	if (tid == 0) return ONLP_FAN_STATUS_FAILED;

	/* get present */
	if (tid < QUANTA_IX8_PSU1_FAN_ID)
	{
		err = onlp_file_read_int(&fan_present, SYS_HWMON_PREFIX "/fan%d_present", tid);
		if (fan_present == 0)
		{
			info->status = ONLP_FAN_STATUS_FAILED;
			return ONLP_STATUS_OK;
		}
	}

	/* get id label */
	strlen = onlp_file_read_str(&tempstr, SYS_HWMON_PREFIX "/fan%d_label", tid);
	if (strlen <= ONLP_STATUS_OK)
	{
		info->status = ONLP_FAN_STATUS_FAILED;
		printf("Error[%d] : read [%d] label fail\n", err, tid);
		return err;
	}
	else
	{
		memcpy(info->hdr.description, tempstr, strlen);
		aim_free(tempstr);
		info->hdr.id = id;
		info->status = ONLP_FAN_STATUS_PRESENT;
	}

	/* get id input */
	err = onlp_file_read_int(&info->rpm, SYS_HWMON_PREFIX "/fan%d_input", tid);
	if (err != ONLP_STATUS_OK)
	{
		info->status = ONLP_FAN_STATUS_FAILED;
		printf("Error[%d] : read [%d] input failn", err, tid);
		return err;
	}
	info->caps |= ONLP_FAN_CAPS_GET_RPM;

	/* get pwm */
	err = onlp_file_read_int(&info->percentage, SYS_HWMON_PREFIX "/fan%d_pwm", tid);
	if (err == ONLP_STATUS_OK) info->caps |= ONLP_FAN_CAPS_GET_PERCENTAGE;

	/* get direction */
	err = onlp_file_read_int(&direction, SYS_HWMON_PREFIX "/fan%d_direction", tid);
	if (err == ONLP_STATUS_OK)
	{
		if(direction == 1) info->caps |= ONLP_FAN_CAPS_F2B;
		else if (direction == 2) info->caps |= ONLP_FAN_CAPS_B2F;
	}

	return ONLP_STATUS_OK;
}

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* rv)
{
	onlp_fan_info_t fan__ =
	{
		{ 0, "", 0,{ 0, } },
		0,0,0,0,0,
		"",""
	};

	*rv = fan__;

	return sys_fan_info_get__(rv, id);
}
