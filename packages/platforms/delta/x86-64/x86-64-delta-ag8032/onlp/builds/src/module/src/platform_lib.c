
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
#include <stdio.h>
#include <unistd.h>
#include <onlplib/file.h>
#include "platform_lib.h"

////////////////////////////////////////////////////////////////
// PLAT DEV CONFIG
////////////////////////////////////////////////////////////////
static plat_dev_desc_t plat_devs[] = {
	[PLAT_DEV_ID_INVALID] = {
		.name = NULL,
	},

	// valid dev desc start
	[PLAT_DEV_ID_1] = {
		.name = "CPUPLD",
		// i2c dev
		.bus  = 0,
		.addr = 0x2e,
	},
	[PLAT_DEV_ID_2] = {
		.name = "SWPLD",
		// i2c dev
		.bus  = 3,
		.addr = 0x30,
	},
	// valid dev desc end

	// END
	[PLAT_DEV_ID_MAX] = {
		.name = NULL,
	},
};

////////////////////////////////////////////////////////////////
// CPLD CONFIG
////////////////////////////////////////////////////////////////
static plat_cpld_t plat_cplds[] = {
	[PLAT_CPLD_ID_INVALID] = {
		.id = PLAT_DEV_ID_INVALID,
	},


	[PLAT_CPLD_ID_1] = {
		.id = PLAT_DEV_ID_1,
		.cached = {
			[0x11] = 1,
			[0x1a] = 1,
		},
	},
	[PLAT_CPLD_ID_2] = {
		.id = PLAT_DEV_ID_2,
		.cached = {
			[0x0a] = 1,
			[0x0b] = 1,
			[0x0c] = 1,
			[0x0d] = 1,
		},
	},

	// END
	[PLAT_CPLD_ID_MAX] = {
		.id = PLAT_DEV_ID_INVALID,
	},

};

////////////////////////////////////////////////////////////////
// PLAT DEV ROUTINE

int plat_dev_id_is_valid (plat_dev_id_t id)
{
	if (id > PLAT_DEV_ID_INVALID && id < PLAT_DEV_ID_MAX) {
		if (plat_devs[id].name) {
			return 1;
		}
	}
	return 0;
}

int plat_dev_get_byte (plat_dev_id_t id, uint8_t reg)
{
	if (!plat_dev_id_is_valid(id)) {
		return -1;
	}
	return onlp_i2c_readb (plat_devs[id].bus, plat_devs[id].addr, reg, 0);
}

int plat_dev_set_byte (plat_dev_id_t id, uint8_t reg, uint8_t val)
{
	if (!plat_dev_id_is_valid(id))
		return -1;
	return onlp_i2c_writeb(plat_devs[id].bus, plat_devs[id].addr, reg, val, 0);
}

////////////////////////////////////////////////////////////////
// CPLD PLAT ROUTINE
int cpld_id_is_valid (plat_cpld_id_t id)
{
	if (id > PLAT_CPLD_ID_INVALID && id < PLAT_CPLD_ID_MAX) {
		if (plat_dev_id_is_valid(plat_cplds[id].id))
			return 1;
	}
	return 0;
}

int cpld_get (plat_cpld_id_t id, uint8_t reg)
{
	if (!cpld_id_is_valid (id))
		return -1;
	return plat_dev_get_byte (plat_cplds[id].id, reg);
}

int cpld_set (plat_cpld_id_t id, uint8_t reg, uint8_t val)
{
	if (!cpld_id_is_valid (id))
		return -1;
	return plat_dev_set_byte (plat_cplds[id].id, reg, val);
}

int cpld_field_get (plat_cpld_id_t id, uint8_t reg, uint8_t field, uint8_t len)
{
	int val;
	int i;
	uint8_t mask = 0;

	val = cpld_get (id, reg);
	if (val < 0) {
		return val;
	}

	// make mask;
	for (i = 0 ; i < len ; i ++) {
		mask |= (1 << (i));
	}

	val = ((val >> field) & mask);

	return val;
}


int cpld_field_set (plat_cpld_id_t id, uint8_t reg, uint8_t field, uint8_t len, uint8_t val)
{
	int _val;
	int i;
	uint8_t mask = 0;

	_val = cpld_get (id, reg);
	if (_val < 0)
		return val;

	// make mask;
	for (i = 0 ; i < len ; i ++) {
		mask |= (1 << (field + i));
	}
	 val = ((val << field) & mask);
	_val = (_val & ~mask);

	return cpld_set (id, reg, val | (uint8_t)_val);
}

////////////////////////////////////////////////////////////////
// CPLD REG PLAT ROUTINE
int cpld_reg_is_valid (cpld_reg_t *r)
{
	if (!r)
		return 0;
	if (r->valid)
		return 1;
	return 0;
}

int cpld_reg_get (cpld_reg_t *r)
{
	if (!r)
		return -1;

	return cpld_field_get (r->id, r->reg, r->field, r->len);
}

int cpld_reg_set (cpld_reg_t *r, uint8_t val)
{
	if (!r)
		return -1;

	return cpld_field_set (r->id, r->reg, r->field, r->len, val);
}

int present_on_board_always (void *e)
{
	return 1;
}

////////////////////////////////////////////////////////////////
// PSU PLAT ROUTINE
int plat_fan_state_update (plat_fan_t *fan)
{
	int present ;
	plat_fan_state_t old_state;

	do {
		old_state = fan->state;

		present = 1;
		if (fan->present) {
			present = fan->present(fan) ? 1 : 0;
		}

		switch (fan->state) {
		case PLAT_FAN_STATE_UNPRESENT:
			if (present) {
				fan->state = PLAT_FAN_STATE_PRESENT;
				if (fan->event_callback)
					fan->event_callback(fan, PLAT_FAN_EVENT_PLUGIN);
				break;
			}
			break;
		case PLAT_FAN_STATE_PRESENT:
			if (!present) {
				fan->state = PLAT_FAN_STATE_UNPRESENT;
				if (fan->event_callback)
					fan->event_callback(fan, PLAT_FAN_EVENT_UNPLUG);
				break;
			}
			break;
		default:
			break;
		}
	} while (old_state != fan->state);

	return 0;
}


////////////////////////////////////////////////////////////////
// PSU PLAT ROUTINE

int plat_psu_state_update (plat_psu_t *psu)
{
	int present ;
	plat_psu_state_t old_state;

	do {
		old_state = psu->state;
		present = 1;
		if (psu->present) {
			present = psu->present(psu) ? 1 : 0;
		}

		switch (psu->state) {
		case PLAT_PSU_STATE_UNPRESENT:
			if (present) {
				psu->state = PLAT_PSU_STATE_PRESENT;
				if (psu->event_callback)
					psu->event_callback(psu, PLAT_PSU_EVENT_PLUGIN);
			}
			break;
		case PLAT_PSU_STATE_PRESENT:
			if (!present) {
				psu->state = PLAT_PSU_STATE_UNPRESENT;
				if (psu->event_callback)
					psu->event_callback(psu, PLAT_PSU_EVENT_UNPLUG);

				break;
			}

			if (onlp_i2c_readb (psu->pmbus_bus, psu->pmbus_addr, 0x00, 
					ONLP_I2C_F_FORCE | ONLP_I2C_F_DISABLE_READ_RETRIES) >= 0) {

				psu->state = PLAT_PSU_STATE_PMBUS_READY;
				if (!plat_os_file_is_existed(psu->pmbus_ready_path) && psu->pmbus_insert_cmd) {
					system (psu->pmbus_insert_cmd);
				}
				if (psu->event_callback)
					psu->event_callback(psu, PLAT_PSU_PMBUS_CONNECT);
			}
			break;
		case PLAT_PSU_STATE_PMBUS_READY:

			// If unplug, remove kernel module
			if (!present) {
				psu->state = PLAT_PSU_STATE_UNPRESENT;
				if (psu->pmbus_remove_cmd) {
					system (psu->pmbus_remove_cmd);
				}
				if (psu->event_callback)
					psu->event_callback(psu, PLAT_PSU_EVENT_UNPLUG);
				break;
			}
			// If pmbus interface is not ok, remove kernel module
			if (onlp_i2c_readb (psu->pmbus_bus, psu->pmbus_addr, 0x00, 
					ONLP_I2C_F_FORCE | ONLP_I2C_F_DISABLE_READ_RETRIES) < 0) {

				psu->state = PLAT_PSU_STATE_PRESENT;
				if (psu->pmbus_remove_cmd) {
					system (psu->pmbus_remove_cmd);
				}
				if (psu->event_callback)
					psu->event_callback(psu, PLAT_PSU_PMBUS_DISCONNECT);

				break;
			}

			break;
		default:
			break;
		}
	} while (old_state != psu->state);

	return 0;
}

////////////////////////////////////////////////////////////////
// OS HELP ROUTINE
static char *plat_os_path_complete (char *path_pattern, char *buff, int len)
{
	FILE *fp;

	snprintf (buff, len, "realpath -z %s 2>/dev/null", path_pattern);
	fp = popen (buff, "r");
	if (fp) {
		fgets (buff, len, fp);
		pclose (fp);
	} else {
		snprintf (buff, len, "%s", path_pattern);
	}
	return buff;
}

int plat_os_file_is_existed (char *path)
{
	char buff[1024];

	if (path)
		return access (plat_os_path_complete(path, buff, sizeof(buff)), F_OK) == 0 ? 1 : 0;
	return 0;
}

int plat_os_file_read (uint8_t *data, int max, int *len, char *path, ...)
{
	char buff[1024];
	return onlp_file_read (data, max, len, plat_os_path_complete(path, buff, sizeof(buff)), NULL);
}

int plat_os_file_read_int (int *val, char *path, ...)
{
	char buff[1024];
	return onlp_file_read_int (val, plat_os_path_complete(path, buff, sizeof(buff)), NULL);
}

int plat_os_file_write_int(int val, char *path, ...)
{
	char buff[1024];
	return onlp_file_write_int (val, plat_os_path_complete(path, buff, sizeof(buff)), NULL);
}




