/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 (C) Delta Networks, Inc.
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
#include "platform_lib.h"
#include "eeprom_info.h"

#define AG8032_FAN_TRAY_DEF_CAP \
	(ONLP_FAN_CAPS_F2B)

static int _fan_tray_present (void *e);
static int _psu_fan_present (void *e);
static int _fan_event_cb (void *e, int ev);

static cpld_reg_t _fan_tray_present_reg[] = {
	[PLAT_LED_ID_5] = CPLD_REG(CPLD_CPUPLD, 0x11, 0, 1),
	[PLAT_LED_ID_6] = CPLD_REG(CPLD_CPUPLD, 0x11, 1, 1),
	[PLAT_LED_ID_7] = CPLD_REG(CPLD_CPUPLD, 0x11, 2, 1),
};


static plat_fan_t plat_fans[] = {
	[PLAT_FAN_ID_1] = {
		.name = "FanTray1-1",
		.present = _fan_tray_present,
		.present_data = &_fan_tray_present_reg[PLAT_LED_ID_5],
		.rpm_get_path = "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-0029/fan1_input",
		.rpm_set_path = "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-0029/fan1_target",
		.def_rpm = 7000,
		.per_get_path = NULL,
		.per_set_path = NULL,
		.caps = AG8032_FAN_TRAY_DEF_CAP,

		.eeprom_path = "/sys/devices/platform/*/*/2-0051/eeprom",

		.event_callback = _fan_event_cb,
		PLAT_FAN_INTERNAL_DEF,
	},
	[PLAT_FAN_ID_2] = {
		.name = "FanTray1-2",
		.present = _fan_tray_present,
		.present_data = &_fan_tray_present_reg[PLAT_LED_ID_5],
		.rpm_get_path = "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-0029/fan2_input",
		.rpm_set_path = "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-0029/fan2_target",
		.def_rpm = 7000,
		.per_get_path = NULL,
		.per_set_path = NULL,
		.caps = AG8032_FAN_TRAY_DEF_CAP,

		.eeprom_path = "/sys/devices/platform/*/*/2-0051/eeprom",

		.event_callback = _fan_event_cb,
		PLAT_FAN_INTERNAL_DEF,
	},
	[PLAT_FAN_ID_3] = {
		.name = "FanTray2-1",
		.present = _fan_tray_present,
		.present_data = &_fan_tray_present_reg[PLAT_LED_ID_6],
		.rpm_get_path = "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-0029/fan3_input",
		.rpm_set_path = "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-0029/fan3_target",
		.def_rpm = 7000,
		.per_get_path = NULL,
		.per_set_path = NULL,
		.caps = AG8032_FAN_TRAY_DEF_CAP,

		.eeprom_path = "/sys/devices/platform/*/*/2-0052/eeprom",

		.event_callback = _fan_event_cb,
		PLAT_FAN_INTERNAL_DEF,
	},
	[PLAT_FAN_ID_4] = {
		.name = "FanTray2-2",
		.present = _fan_tray_present,
		.present_data = &_fan_tray_present_reg[PLAT_LED_ID_6],
		.rpm_get_path = "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-0029/fan4_input",
		.rpm_set_path = "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-0029/fan4_target",
		.def_rpm = 7000,
		.per_get_path = NULL,
		.per_set_path = NULL,
		.caps = AG8032_FAN_TRAY_DEF_CAP,

		.eeprom_path = "/sys/devices/platform/*/*/2-0052/eeprom",

		.event_callback = _fan_event_cb,
		PLAT_FAN_INTERNAL_DEF,
	},
	[PLAT_FAN_ID_5] = {
		.name = "FanTray3-1",
		.present = _fan_tray_present,
		.present_data = &_fan_tray_present_reg[PLAT_LED_ID_7],
		.rpm_get_path = "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-002a/fan1_input",
		.rpm_set_path = "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-002a/fan1_target",
		.def_rpm = 7000,
		.per_get_path = NULL,
		.per_set_path = NULL,
		.caps = AG8032_FAN_TRAY_DEF_CAP,

		.eeprom_path = "/sys/devices/platform/*/*/2-0053/eeprom",

		.event_callback = _fan_event_cb,
		PLAT_FAN_INTERNAL_DEF,
	},
	[PLAT_FAN_ID_6] = {
		.name = "FanTray3-2",
		.present = _fan_tray_present,
		.present_data = &_fan_tray_present_reg[PLAT_LED_ID_7],
		.rpm_get_path = "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-002a/fan2_input",
		.rpm_set_path = "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-2/2-002a/fan2_target",
		.def_rpm = 7000,
		.per_get_path = NULL,
		.per_set_path = NULL,
		.caps = AG8032_FAN_TRAY_DEF_CAP,

		.eeprom_path = "/sys/devices/platform/*/*/2-0053/eeprom",

		.event_callback = _fan_event_cb,
		PLAT_FAN_INTERNAL_DEF,
	},
	[PLAT_FAN_ID_7] = {
		.name = "PSU1 Fan",
		.present = _psu_fan_present,
		.rpm_get_path = "/sys/bus/i2c/devices/i2c-4/4-0058/hwmon/*/fan1_input",
		.rpm_set_path = NULL,
		.per_get_path = NULL,
		.per_set_path = NULL,
		.caps = AG8032_FAN_TRAY_DEF_CAP,

		PLAT_FAN_INTERNAL_DEF,
	},
	[PLAT_FAN_ID_8] = {
		.name = "PSU2 Fan",
		.present = _psu_fan_present,
		.rpm_get_path = "/sys/bus/i2c/devices/i2c-4/4-0059/hwmon/*/fan1_input",
		.rpm_set_path = NULL,
		.per_get_path = NULL,
		.per_set_path = NULL,
		.caps = AG8032_FAN_TRAY_DEF_CAP,

		PLAT_FAN_INTERNAL_DEF,
	},
};

static int _psu_fan_present (void *e)
{
	plat_fan_t *fan = e;
	return plat_os_file_is_existed (fan->rpm_get_path) ? 1 : 0;
}


static int _fan_event_cb (void *e, int ev)
{
	int len;
	plat_fan_t *fan = e;

	switch (ev) {
	case PLAT_FAN_EVENT_PLUGIN:
		// reflush fan setting
		if (fan->rpm_set_value > 0 && fan->rpm_set_path)
			plat_os_file_write_int (fan->rpm_set_value, fan->rpm_set_path, NULL);

		if (fan->per_set_value > 0 && fan->per_set_path)
			plat_os_file_write_int (fan->per_set_value, fan->per_set_path, NULL);

		// read eeprom info
		if (fan->eeprom_path && (plat_os_file_read (fan->eeprom, sizeof(fan->eeprom), &len,
				fan->eeprom_path, NULL) == ONLP_STATUS_OK)) {
			break;
		}

		memset (fan->eeprom, 0xff, sizeof(fan->eeprom));
		break;
	case PLAT_FAN_EVENT_UNPLUG:

		// clear eeprom info
		memset (fan->eeprom, 0xff, sizeof(fan->eeprom));
		break;
	default:
		break;
	}

	return 0;
}

static int _fan_tray_present (void *e)
{
	plat_fan_t *fan = e;
	return cpld_reg_get (fan->present_data) == 0 ? 1 : 0;
}

static int plat_fan_is_valid (int id)
{
	if (id > PLAT_FAN_ID_INVALID && id < PLAT_FAN_ID_MAX) {
		if (plat_fans[id].name)
			return 1;
	}
	return 0;
}


int onlp_fani_init(void)
{
#if 0
	plat_fan_t *fan;
	int i;

	for (i = PLAT_FAN_ID_1 ; i < PLAT_FAN_ID_MAX ; i ++) {

		if (!plat_fan_is_valid(i))
			continue;
		fan = &plat_fans[i];

		if (fan->def_rpm && fan->rpm_set_path)
			onlp_fani_rpm_set (ONLP_FAN_ID_CREATE(i), fan->def_rpm);

		if (fan->def_per && fan->per_set_path)
			onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), fan->def_per);
	}
#endif

    return ONLP_STATUS_OK;
}

int onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
	int error;
	plat_fan_t *fan;
	int fid;
	int present = 1;

    if (!ONLP_OID_IS_FAN(id))
		return ONLP_STATUS_E_INVALID;

    fid = ONLP_OID_ID_GET(id);

	if (!plat_fan_is_valid(fid))
		return ONLP_STATUS_E_INVALID;

	fan = &plat_fans[fid];

	plat_fan_state_update (fan);

	present = fan->state == PLAT_FAN_STATE_PRESENT ? 1 : 0;

	memset (info, 0, sizeof(*info));

	info->hdr.id = id;
	if (fan->name)
		snprintf (info->hdr.description, sizeof(info->hdr.description), "%s", fan->name);

	info->caps = fan->caps;
	if (fan->rpm_get_path) info->caps |= ONLP_FAN_CAPS_GET_RPM;
	if (fan->rpm_set_path) info->caps |= ONLP_FAN_CAPS_SET_RPM;
	if (fan->per_get_path) info->caps |= ONLP_FAN_CAPS_GET_PERCENTAGE;
	if (fan->per_set_path) info->caps |= ONLP_FAN_CAPS_SET_PERCENTAGE;

	error = 0;
	if (present) {
		info->status |= ONLP_FAN_STATUS_PRESENT;
		if (info->caps & ONLP_FAN_CAPS_GET_RPM) {
			if (plat_os_file_read_int(&info->rpm, fan->rpm_get_path, NULL) < 0) {
				error ++;
			} else {
				if (fan->rpm_set_value > 0) {
					if (info->rpm < ((fan->rpm_set_value * 80) / 100)) {
						info->status |= ONLP_FAN_STATUS_FAILED;
					}
				}
			}
		}
		if (info->caps & ONLP_FAN_CAPS_GET_PERCENTAGE) {
			if (plat_os_file_read_int(&info->percentage, fan->per_get_path, NULL) < 0) {
				error ++;
			} else {
				if (fan->per_set_value > 0) {
					if (info->percentage < ((fan->per_set_value * 80) / 100)) {
						info->status |= ONLP_FAN_STATUS_FAILED;
					}
				}
			}
		}

		// get fan info
		eeprom_info_get (fan->eeprom, sizeof(fan->eeprom), "fan_model", info->model);
		eeprom_info_get (fan->eeprom, sizeof(fan->eeprom),  "fan_series", info->serial);
	}

	if ((info->caps & (ONLP_FAN_CAPS_B2F | ONLP_FAN_CAPS_B2F)) == (ONLP_FAN_CAPS_B2F | ONLP_FAN_CAPS_B2F)) {
		// should do check it auto
		// TODO
	} else if (info->caps & ONLP_FAN_CAPS_B2F) {
		info->status |= ONLP_FAN_STATUS_B2F;
	} else if (info->caps & ONLP_FAN_CAPS_F2B) {
		info->status |= ONLP_FAN_STATUS_F2B;
	}
	return error ? ONLP_STATUS_E_INTERNAL : ONLP_STATUS_OK;
}

/*
 * This function sets the speed of the given fan in RPM.
 *
 * This function will only be called if the fan supprots the RPM_SET
 * capability.
 *
 * It is optional if you have no fans at all with this feature.
 */
int
onlp_fani_rpm_set(onlp_oid_t id, int rpm)
{
	int error;
	plat_fan_t *fan;
	int fid;

    if (!ONLP_OID_IS_FAN(id))
		return ONLP_STATUS_E_INVALID;

    fid = ONLP_OID_ID_GET(id);

	if (!plat_fan_is_valid(fid))
		return ONLP_STATUS_E_INVALID;

	fan = &plat_fans[fid];

	if (fan->rpm_set_path) {
		if (fan->rpm_set_value != rpm) {
			error = plat_os_file_write_int (rpm, fan->rpm_set_path, NULL);
		}
	} else
		return ONLP_STATUS_E_UNSUPPORTED;

	if (error < 0) {
        return ONLP_STATUS_E_PARAM;
    }

	fan->rpm_set_value = rpm;
    
    return ONLP_STATUS_OK;
}

int onlp_fani_percentage_set(onlp_oid_t id, int p)
{
	int error;
	plat_fan_t *fan;
	int fid;

    if (!ONLP_OID_IS_FAN(id))
		return ONLP_STATUS_E_INVALID;

    fid = ONLP_OID_ID_GET(id);

	if (!plat_fan_is_valid(fid))
		return ONLP_STATUS_E_INVALID;

	fan = &plat_fans[fid];

	if (fan->per_set_path) {
		if (fan->per_set_value != p) {
			error = plat_os_file_write_int (p, fan->per_set_path, NULL);
		}
	} else
		return ONLP_STATUS_E_UNSUPPORTED;

	if (error < 0) {
        return ONLP_STATUS_E_PARAM;
    }

	fan->per_set_value = p;
    
    return ONLP_STATUS_OK;

}

/*
 * This function sets the fan speed of the given OID as per
 * the predefined ONLP fan speed modes: off, slow, normal, fast, max.
 *
 * Interpretation of these modes is up to the platform.
 *
 */
int
onlp_fani_mode_set(onlp_oid_t id, onlp_fan_mode_t mode)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * This function sets the fan direction of the given OID.
 *
 * This function is only relevant if the fan OID supports both direction
 * capabilities.
 *
 * This function is optional unless the functionality is available.
 */
int
onlp_fani_dir_set(onlp_oid_t id, onlp_fan_dir_t dir)
{
	plat_fan_t *fan;
	int fid;

    if (!ONLP_OID_IS_FAN(id))
		return ONLP_STATUS_E_INVALID;

    fid = ONLP_OID_ID_GET(id);

	if (!plat_fan_is_valid(fid))
		return ONLP_STATUS_E_INVALID;

	fan = &plat_fans[fid];

	if ((fan->caps & (ONLP_FAN_CAPS_B2F | ONLP_FAN_CAPS_B2F)) == (ONLP_FAN_CAPS_B2F | ONLP_FAN_CAPS_B2F)) {
		// TODO
	}

    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * Generic fan ioctl. Optional.
 */
int
onlp_fani_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

