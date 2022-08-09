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
#include <onlp/platformi/thermali.h>
#include <onlplib/file.h>
#include "x86_64_quanta_ix8_rglbmc_int.h"
#include "x86_64_quanta_ix8_rglbmc_log.h"

int
onlp_thermali_init(void)
{
    return ONLP_STATUS_OK;
}

static int
sys_thermal_info_get__(onlp_thermal_info_t* info, int id)
{
    int err = 0, strlen = 0;
	int tid = ONLP_OID_ID_GET(id);

	char *tempstr = NULL;

	/* get id label */
	strlen = onlp_file_read_str(&tempstr, SYS_HWMON_PREFIX "/temp%d_label", tid);
	if (strlen <= ONLP_STATUS_OK)
	{
		info->status = ONLP_THERMAL_STATUS_FAILED;
		printf("Error[%d] : read [%d] label fail\n", err, tid);
		return err;
	}
	else
	{
		memcpy(info->hdr.description, tempstr, strlen);
		aim_free(tempstr);
		info->hdr.id = id;
		info->status = ONLP_THERMAL_STATUS_PRESENT;
	}

	/* get id input */
	err = onlp_file_read_int(&info->mcelsius, SYS_HWMON_PREFIX "/temp%d_input", tid);
	if (err != ONLP_STATUS_OK)
	{
		info->status = info->status & ~0x01;
		printf("Error[%d] : read [%d] input failn", err, tid);
		return err;
	}
	info->caps |= ONLP_THERMAL_CAPS_GET_TEMPERATURE;

	/* get id cap and thresholds */
	err = onlp_file_read_int(&info->thresholds.warning, SYS_HWMON_PREFIX "/temp%d_ncrit", tid);
	if (err == ONLP_STATUS_OK) info->caps |= ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD;

	err = onlp_file_read_int(&info->thresholds.error, SYS_HWMON_PREFIX "/temp%d_crit", tid);
	if (err == ONLP_STATUS_OK) info->caps |= ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD;

	err = onlp_file_read_int(&info->thresholds.shutdown, SYS_HWMON_PREFIX "/temp%d_max", tid);
	if (err == ONLP_STATUS_OK) info->caps |= ONLP_THERMAL_CAPS_GET_SHUTDOWN_THRESHOLD;

	return ONLP_STATUS_OK;
}

int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* rv)
{
	onlp_thermal_info_t thermal__ =
	{
		{ 0, "", 0,{ 0, } },
		0,0,0,
		{ 0, 0, 0}
	};

	*rv = thermal__;

    return sys_thermal_info_get__(rv, id);
}
