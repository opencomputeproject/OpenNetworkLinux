/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2017 Delta Networks, Inc
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

static int _psu_thermal_present (void *e);

////////////////////////////////////////////////////////////////
// THERMALS PLAT CONFIG
static plat_thermal_t plat_thermals[] = {

	[PLAT_THERMAL_ID_1] = {
		.desc = "Thermal Sensor %d - close to cpu",
		.temp_get_path     = "/sys/bus/i2c/devices/0-0048/hwmon/*/temp1_input",
		.warnning_set_path = "/sys/bus/i2c/devices/0-0048/hwmon/*/temp1_max_hyst",
		.critical_set_path = NULL,
		.shutdown_set_path = "/sys/bus/i2c/devices/0-0048/hwmon/*/temp1_max",

		.def_warnning = ONLP_THERMAL_THRESHOLD_WARNING_DEFAULT,
		.def_critical = ONLP_THERMAL_THRESHOLD_ERROR_DEFAULT,
		.def_shutdown = ONLP_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT,
	},
	[PLAT_THERMAL_ID_2] = {
		.desc = "Thermal Sensor %d - Main Board (U54)",
		.temp_get_path     = "/sys/bus/i2c/devices/2-004a/hwmon/*/temp1_input",
		.warnning_set_path = "/sys/bus/i2c/devices/2-004a/hwmon/*/temp1_max_hyst",
		.critical_set_path = NULL,
		.shutdown_set_path = "/sys/bus/i2c/devices/2-004a/hwmon/*/temp1_max",

		.def_warnning = ONLP_THERMAL_THRESHOLD_WARNING_DEFAULT,
		.def_critical = ONLP_THERMAL_THRESHOLD_ERROR_DEFAULT,
		.def_shutdown = ONLP_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT,
	},
	[PLAT_THERMAL_ID_3] = {
		.desc = "Thermal Sensor %d - BCM chip Bottom (U70)",
		.temp_get_path     = "/sys/bus/i2c/devices/2-004c/hwmon/*/temp1_input",
		.warnning_set_path = "/sys/bus/i2c/devices/2-004c/hwmon/*/temp1_max_hyst",
		.critical_set_path = NULL,
		.shutdown_set_path = "/sys/bus/i2c/devices/2-004c/hwmon/*/temp1_max",

		.def_warnning = ONLP_THERMAL_THRESHOLD_WARNING_DEFAULT,
		.def_critical = ONLP_THERMAL_THRESHOLD_ERROR_DEFAULT,
		.def_shutdown = ONLP_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT,
	},
	[PLAT_THERMAL_ID_4] = {
		.desc = "Thermal Sensor %d - BCM chip Top (U71)",
		.temp_get_path     = "/sys/bus/i2c/devices/2-004d/hwmon/*/temp1_input",
		.warnning_set_path = "/sys/bus/i2c/devices/2-004d/hwmon/*/temp1_max_hyst",
		.critical_set_path = NULL,
		.shutdown_set_path = "/sys/bus/i2c/devices/2-004d/hwmon/*/temp1_max",

		.def_warnning = ONLP_THERMAL_THRESHOLD_WARNING_DEFAULT,
		.def_critical = ONLP_THERMAL_THRESHOLD_ERROR_DEFAULT,
		.def_shutdown = ONLP_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT,
	},
	[PLAT_THERMAL_ID_5] = {
		.desc = "Thermal Sensor %d - Cpu Core",
		.temp_get_path     = "/sys/devices/platform/coretemp.0/hwmon/*/temp2_input",
		.warnning_set_path = "/sys/devices/platform/coretemp.0/hwmon/*/temp2_crit",
		.critical_set_path = NULL,
		.shutdown_set_path = "/sys/devices/platform/coretemp.0/hwmon/*/temp2_max",

		.def_warnning = 0,
		.def_critical = 0,
		.def_shutdown = 0,
	},
	[PLAT_THERMAL_ID_6] = {
		.desc = "Thermal Sensor %d - PSU1",
		.present = _psu_thermal_present,
		.temp_get_path     = "/sys/bus/i2c/devices/i2c-4/4-0058/hwmon/*/temp1_input",
		.shutdown_set_path = "/sys/bus/i2c/devices/i2c-4/4-0058/hwmon/*/temp1_max",

		.def_warnning = 0,
		.def_critical = 0,
		.def_shutdown = 0,
	},
	[PLAT_THERMAL_ID_7] = {
		.desc = "Thermal Sensor %d - PSU2",
		.present = _psu_thermal_present,
		.temp_get_path     = "/sys/bus/i2c/devices/i2c-4/4-0059/hwmon/*/temp1_input",
		.shutdown_set_path = "/sys/bus/i2c/devices/i2c-4/4-0059/hwmon/*/temp1_max",

		.def_warnning = 0,
		.def_critical = 0,
		.def_shutdown = 0,
	},
};

#define plat_thermals_size	(sizeof(plat_thermals)/sizeof(plat_thermals[0]))

static int _psu_thermal_present (void *e)
{
	plat_thermal_t *thermal = e;
	return plat_os_file_is_existed (thermal->temp_get_path) ? 1 : 0;
}

static int plat_thermal_is_valid (int id)
{
	plat_thermal_t *thermal;

	if (id < 0 && id >= plat_thermals_size)
		return 0;

	thermal = &plat_thermals[id];
	if (thermal->temp_get_path || thermal->desc)
			return 1;

	return 0;
}

int onlp_thermali_init(void)
{ 
	int i;
	plat_thermal_t *thermal;

	for (i = 0 ; i < plat_thermals_size ; i ++) {
		if (!plat_thermal_is_valid (i))
			continue;
		thermal = &plat_thermals[i];

		if (thermal->warnning_set_path && thermal->def_warnning)
			plat_os_file_write_int (thermal->def_warnning, thermal->warnning_set_path, NULL);
		if (thermal->critical_set_path && thermal->def_critical)
			plat_os_file_write_int (thermal->def_critical, thermal->critical_set_path, NULL);
		if (thermal->shutdown_set_path && thermal->def_shutdown)
			plat_os_file_write_int (thermal->def_shutdown, thermal->shutdown_set_path, NULL);
	}
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
    int tid;
	int present = 1;
	plat_thermal_t *thermal;
	int value;
	int error;

    if (!ONLP_OID_IS_THERMAL(id))
		return ONLP_STATUS_E_INVALID;

    tid = ONLP_OID_ID_GET(id);

	if (!plat_thermal_is_valid(tid))
		return ONLP_STATUS_E_INVALID;

	thermal = &plat_thermals[tid];

	if (thermal->present) {
		present = thermal->present(thermal) ? 1 : 0;
	}

	memset (info, 0, sizeof(*info));

	// fix onlp_thermal_info_t
	info->hdr.id = id;
	if (thermal->desc)
		snprintf (info->hdr.description, sizeof(info->hdr.description), thermal->desc, tid);

	if (thermal->temp_get_path)
		info->caps |= ONLP_THERMAL_CAPS_GET_TEMPERATURE;
	if (thermal->warnning_set_path || thermal->def_warnning)
		info->caps |= ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD;
	if (thermal->critical_set_path || thermal->def_critical)
		info->caps |= ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD;
	if (thermal->shutdown_set_path || thermal->def_shutdown)
		info->caps |= ONLP_THERMAL_CAPS_GET_SHUTDOWN_THRESHOLD;

	// Get value
	error = 0;
	if (info->caps & ONLP_THERMAL_CAPS_GET_TEMPERATURE) {
		if (plat_os_file_read_int(&value, thermal->temp_get_path, NULL) < 0)
			error ++;
		else
			info->mcelsius = value;
	}
	if (info->caps & ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD) {
		if (thermal->warnning_set_path) {
			if (plat_os_file_read_int(&value, thermal->warnning_set_path, NULL) < 0)
				error ++;
			else
				info->thresholds.warning = value;
		} else {
				info->thresholds.warning = thermal->def_warnning;
		}
	}
	if (info->caps & ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD) {
		if (thermal->critical_set_path) {
			if (plat_os_file_read_int(&value, thermal->critical_set_path, NULL) < 0)
				error ++;
			else
				info->thresholds.error = value;
		} else {
				info->thresholds.error = thermal->def_critical;
		}
	}
	if (info->caps & ONLP_THERMAL_CAPS_GET_SHUTDOWN_THRESHOLD) {
		if (thermal->shutdown_set_path) {
			if (plat_os_file_read_int(&value, thermal->shutdown_set_path, NULL) < 0)
				error ++;
			else
				info->thresholds.shutdown = value;
		} else {
				info->thresholds.shutdown = thermal->def_shutdown;
		}
	}



	if (present)
		info->status |= ONLP_THERMAL_STATUS_PRESENT;

	// check threshold
	if (info->caps & ONLP_THERMAL_CAPS_GET_TEMPERATURE) {
		if (info->caps & ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD) {
			if (info->mcelsius >= info->thresholds.error) {
				info->status |= ONLP_THERMAL_STATUS_FAILED;
			}
		}
		if (info->caps & ONLP_THERMAL_CAPS_GET_SHUTDOWN_THRESHOLD) {
			if (info->mcelsius >= info->thresholds.shutdown) {
				info->status |= ONLP_THERMAL_STATUS_FAILED;
			}
		}
	}

	
	return error ? ONLP_STATUS_E_INTERNAL : ONLP_STATUS_OK;
}


