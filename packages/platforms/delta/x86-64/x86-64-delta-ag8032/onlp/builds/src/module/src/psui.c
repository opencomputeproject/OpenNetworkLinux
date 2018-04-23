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
 *
 *
 ***********************************************************/
#include <onlplib/file.h>
#include <onlp/platformi/psui.h>
#include "eeprom_drv.h"
#include "eeprom_info.h"
#include "platform_lib.h"

static int _psu_present (void *e);
static int _psu_event (void *e, int ev);

static plat_psu_t plat_psus[] = {
	[PLAT_PSU_ID_1] = {
		.name = "PSU1",
		.present = _psu_present,
		.present_cpld_reg = CPLD_REG (CPLD_CPUPLD, 0x1a, 6, 1),

		.vin_path = "/sys/bus/i2c/devices/4-0058/hwmon/*/in1_input",
		.iin_path = "/sys/bus/i2c/devices/4-0058/hwmon/*/curr1_input",
		.pin_path = "/sys/bus/i2c/devices/4-0058/hwmon/*/power1_input",

		.vout_path = "/sys/bus/i2c/devices/4-0058/hwmon/*/in2_input",
		.iout_path = "/sys/bus/i2c/devices/4-0058/hwmon/*/curr2_input",
		.pout_path = "/sys/bus/i2c/devices/4-0058/hwmon/*/power2_input",

		.vin_max_path = "/sys/bus/i2c/devices/4-0058/hwmon/*/in1_max",
		.vin_min_path = "/sys/bus/i2c/devices/4-0058/hwmon/*/in1_min",

		.eeprom_bus = 4,
		.eeprom_addr= 0x50,
		.event_callback = _psu_event,

		.state = PLAT_PSU_STATE_UNPRESENT,
		.pmbus_insert_cmd = "echo pmbus 0x58 > /sys/bus/i2c/devices/i2c-4/new_device",
		.pmbus_remove_cmd = "echo 0x58 > /sys/bus/i2c/devices/i2c-4/delete_device",
		.pmbus_ready_path = "/sys/bus/i2c/devices/4-0058/hwmon",
		.pmbus_bus = 4,
		.pmbus_addr = 0x58,
	},
	[PLAT_PSU_ID_2] = {
		.name = "PSU2",
		.present = _psu_present,
		.present_cpld_reg = CPLD_REG (CPLD_CPUPLD, 0x1a, 7, 1),

		.vin_path = "/sys/bus/i2c/devices/4-0059/hwmon/*/in1_input",
		.iin_path = "/sys/bus/i2c/devices/4-0059/hwmon/*/curr1_input",
		.pin_path = "/sys/bus/i2c/devices/4-0059/hwmon/*/power1_input",

		.vout_path = "/sys/bus/i2c/devices/4-0059/hwmon/*/in2_input",
		.iout_path = "/sys/bus/i2c/devices/4-0059/hwmon/*/curr2_input",
		.pout_path = "/sys/bus/i2c/devices/4-0059/hwmon/*/power2_input",

		.vin_max_path = "/sys/bus/i2c/devices/4-0059/hwmon/*/in1_max",
		.vin_min_path = "/sys/bus/i2c/devices/4-0059/hwmon/*/in1_min",

		.eeprom_bus = 4,
		.eeprom_addr= 0x51,
		.event_callback = _psu_event,

		.state = PLAT_PSU_STATE_UNPRESENT,
		.pmbus_insert_cmd = "echo pmbus 0x59 > /sys/bus/i2c/devices/i2c-4/new_device",
		.pmbus_remove_cmd = "echo 0x59 > /sys/bus/i2c/devices/i2c-4/delete_device",
		.pmbus_ready_path = "/sys/bus/i2c/devices/4-0059/hwmon",
		.pmbus_bus = 4,
		.pmbus_addr = 0x59,
	},
};

#define plat_psus_size	(sizeof(plat_psus)/sizeof(plat_psus[0]))

static int plat_psu_is_valid (int id)
{
	plat_psu_t *psu;

	if (id < 0 && id >= plat_psus_size)
		return 0;

	psu = &plat_psus[id];
	if (psu->name)
			return 1;

	return 0;
}

static int _psu_present (void *e)
{
	plat_psu_t *psu = e;
	return cpld_reg_get (&psu->present_cpld_reg) == 0 ? 1 : 0;
}

static int _psu_event (void *e, int ev)
{
	plat_psu_t *psu = e;

	switch (ev) {
	case PLAT_PSU_PMBUS_CONNECT:
		if (eeprom_read (psu->eeprom_bus, psu->eeprom_addr, 0, psu->eeprom, sizeof(psu->eeprom)))
			memset (psu->eeprom, 0xff, sizeof(psu->eeprom));
		break;
	case PLAT_PSU_PMBUS_DISCONNECT:
		memset (psu->eeprom, 0xff, sizeof(psu->eeprom));
		break;
	case PLAT_PSU_EVENT_UNPLUG:
	case PLAT_PSU_EVENT_PLUGIN:
	default:
		break;
	}

	return 0;
}

static uint32_t _psu_vin_type_guess (plat_psu_t *psu)
{
	uint32_t ret;
	int vmax = -1;
	int vmin = -1;

	if ((psu->vin_max_path) && 
		(plat_os_file_read_int (&vmax, psu->vin_max_path, NULL) < 0))
		vmax = -1;
	if ((psu->vin_min_path) && 
		(plat_os_file_read_int (&vmin, psu->vin_min_path, NULL) < 0))
		vmin = -1;
	
	ret = 0;
	if (12000 > vmin && 12000 < vmax)
		ret |= ONLP_PSU_CAPS_DC12;
	if (48000 > vmin && 48000 < vmax)
		ret |= ONLP_PSU_CAPS_DC48;
	if (110000 > vmin && 110000 < vmax)
		ret |= ONLP_PSU_CAPS_AC;

	if (!ret)
		ret |= ONLP_PSU_CAPS_AC;

	return ret;
}

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
	int error ;
	plat_psu_t *psu;
	int present;
    int pid= ONLP_OID_ID_GET(id);

    if (!ONLP_OID_IS_PSU(id))
		return ONLP_STATUS_E_INVALID;

	if (!plat_psu_is_valid(pid))
		return ONLP_STATUS_E_INVALID;

	psu = &plat_psus[pid];

    memset(info, 0, sizeof(onlp_psu_info_t));

	info->hdr.id = id;
	if (psu->name)
		snprintf (info->hdr.description, sizeof(info->hdr.description), "%s", psu->name);


	plat_psu_state_update (psu);

	// check present;
	present = psu->state != PLAT_PSU_STATE_UNPRESENT ? 1 : 0;

	if (present) {
		info->status |= ONLP_PSU_STATUS_PRESENT;
		info->status &= ~ONLP_PSU_STATUS_UNPLUGGED;
	} else {
		info->status |= ONLP_PSU_STATUS_UNPLUGGED;
		info->status &= ~ONLP_PSU_STATUS_PRESENT;
	}

	// unpresent will return directly
	if (info->status & ONLP_PSU_STATUS_UNPLUGGED) {
		return ONLP_STATUS_OK;
	}

	///////////////////////////////////////////////////////////////
	// get caps
	if (psu->vin_path && plat_os_file_is_existed(psu->vin_path))  info->caps |= ONLP_PSU_CAPS_VIN;
	if (psu->iin_path && plat_os_file_is_existed(psu->iin_path))  info->caps |= ONLP_PSU_CAPS_IIN;
	if (psu->pin_path && plat_os_file_is_existed(psu->pin_path))  info->caps |= ONLP_PSU_CAPS_PIN;
	if (psu->vout_path && plat_os_file_is_existed(psu->vout_path)) info->caps |= ONLP_PSU_CAPS_VOUT;
	if (psu->iout_path && plat_os_file_is_existed(psu->iout_path)) info->caps |= ONLP_PSU_CAPS_IOUT;
	if (psu->pout_path && plat_os_file_is_existed(psu->pout_path)) info->caps |= ONLP_PSU_CAPS_POUT;

	//// TODO : auto detect AC / DC type
	// we do a guess
	info->caps |= _psu_vin_type_guess (psu);

	// get psu info
	eeprom_info_get (psu->eeprom, sizeof(psu->eeprom), "psu_model", info->model);
	eeprom_info_get (psu->eeprom, sizeof(psu->eeprom), "psu_series", info->serial);

	///////////////////////////////////////////////////////////////
	// get and check value
	error = 0;
	if (info->caps & ONLP_PSU_CAPS_VIN) {

		if (psu->state != PLAT_PSU_STATE_PMBUS_READY) {
				info->status |= ONLP_PSU_STATUS_FAILED;
		} else {
			
			if (plat_os_file_read_int(&info->mvin, psu->vin_path, NULL) < 0) {
				error ++;
			} else {
				if (info->mvin < 2)
					info->status |= ONLP_PSU_STATUS_FAILED;
			}
		}
	}

	//// If VIN is not ok, skip other 
	if ((info->status & ONLP_PSU_STATUS_FAILED) == 0) {

	if (info->caps & ONLP_PSU_CAPS_IIN) {
		if (plat_os_file_read_int(&info->miin, psu->iin_path, NULL) < 0)
			error ++;
	}
	if (info->caps & ONLP_PSU_CAPS_PIN) {
		if (plat_os_file_read_int(&info->mpin, psu->pin_path, NULL) < 0)
			error ++;
		else
			info->mpin = info->mpin / 1000;
	}
	if (info->caps & ONLP_PSU_CAPS_VOUT) {
		if (plat_os_file_read_int(&info->mvout, psu->vout_path, NULL) < 0) {
			error ++;
		} else {
			if (info->mvout < 2)
				info->status |= ONLP_PSU_STATUS_FAILED;
		}
	}
	if (info->caps & ONLP_PSU_CAPS_IOUT) {
		if (plat_os_file_read_int(&info->miout, psu->iout_path, NULL) < 0)
			error ++;
	}
	if (info->caps & ONLP_PSU_CAPS_POUT) {
		if (plat_os_file_read_int(&info->mpout, psu->pout_path, NULL) < 0)
			error ++;
		else
			info->mpout = info->mpout / 1000;
	}
	} // if ((info->status & ONLP_PSU_STATUS_FAILED) == 0)

    return error ? ONLP_STATUS_E_INTERNAL : ONLP_STATUS_OK;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
