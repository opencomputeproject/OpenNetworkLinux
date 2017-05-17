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
#include <x86_64_quanta_ly9_rangeley/x86_64_quanta_ly9_rangeley_config.h>
#include <x86_64_quanta_ly9_rangeley/x86_64_quanta_ly9_rangeley_gpio_table.h>
#include <onlp/platformi/fani.h>

#include "x86_64_quanta_ly9_rangeley_int.h"
#include "x86_64_quanta_ly9_rangeley_log.h"

#include <onlplib/file.h>
#include <onlplib/gpio.h>

int
onlp_fani_init(void)
{
    return ONLP_STATUS_OK;
}

struct fan_gpio_s {
	int present;
	int fan_dir_detect;
};

static struct fan_gpio_s fan_gpio[] = {
	{}, /* Not used */
	{ .present = QUANTA_LY9_FAN_PRSNT_N_1, .fan_dir_detect = QUANTA_LY9_FAN_BF_DET1 },
	{ .present = QUANTA_LY9_FAN_PRSNT_N_2, .fan_dir_detect = QUANTA_LY9_FAN_BF_DET2 },
	{ .present = QUANTA_LY9_FAN_PRSNT_N_3, .fan_dir_detect = QUANTA_LY9_FAN_BF_DET3 },
	{}, /* Not used */
	{ .present = QUANTA_LY9_FAN_PRSNT_N_1, .fan_dir_detect = QUANTA_LY9_FAN_BF_DET1 },
	{ .present = QUANTA_LY9_FAN_PRSNT_N_2, .fan_dir_detect = QUANTA_LY9_FAN_BF_DET2 },
	{ .present = QUANTA_LY9_FAN_PRSNT_N_3, .fan_dir_detect = QUANTA_LY9_FAN_BF_DET3 },
	{}, /* Not used */
};

static int
sys_fan_info_get__(onlp_fan_info_t* info, int id)
{
    int value = 0;
    int rv;

	if(onlp_gpio_get(fan_gpio[id].present, &value) == ONLP_STATUS_OK
			&& value == 0) {
		info->status = ONLP_FAN_STATUS_PRESENT;
		if(onlp_gpio_get(fan_gpio[id].fan_dir_detect, &value) == ONLP_STATUS_OK
				&& value == 0) {
			info->status |= ONLP_FAN_STATUS_F2B;
			info->caps |= ONLP_FAN_CAPS_F2B;
		}
		else {
			info->status |= ONLP_FAN_STATUS_B2F;
			info->caps |= ONLP_FAN_CAPS_B2F;
		}
	}
	else {
		info->status = ONLP_FAN_STATUS_FAILED;
	}

    rv = onlp_file_read_int(&info->rpm,
                            SYS_HWMON_PREFIX "/fan%d_input", id);

    if(rv == ONLP_STATUS_E_INTERNAL) {
        return rv;
    }

    if(rv == ONLP_STATUS_E_MISSING) {
        info->status &= ~1;
        return 0;
    }

    if(info->rpm <= X86_64_QUANTA_LY9_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD) {
        info->status |= ONLP_FAN_STATUS_FAILED;
    }

    /*
     * Calculate percentage based on current speed and the maximum.
     */
    info->caps |= ONLP_FAN_CAPS_GET_PERCENTAGE;
    if(info->status & ONLP_FAN_STATUS_F2B) {
        info->percentage = (int) ((double) info->rpm * (double)100 / (double)X86_64_QUANTA_LY9_RANGELEY_CONFIG_SYSFAN_F2B_RPM_MAX);
    }
    if(info->status & ONLP_FAN_STATUS_B2F) {
        info->percentage = (int) ((double) info->rpm * (double)100 / (double)X86_64_QUANTA_LY9_RANGELEY_CONFIG_SYSFAN_B2F_RPM_MAX);
    }

    return 0;
}

static int
psu_fan_info_get__(onlp_fan_info_t* info, int id)
{
    extern struct psu_info_s psu_info[];
    char* dir = psu_info[id].path;

    return onlp_file_read_int(&info->rpm, "%s*fan1_input", dir);
}


/* Onboard Fans */
static onlp_fan_info_t fans__[] = {
    { }, /* Not used */
    { { FAN_OID_FAN1,  "Left  (Module/Fan 1/1)", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN2,  "Center(Module/Fan 2/1)", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN3,  "Right (Module/Fan 3/1)", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN4,  "Reserved (Module/Fan 4/1)", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN5,  "Left  (Module/Fan 1/2)", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN6,  "Center(Module/Fan 2/2)", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN7,  "Right (Module/Fan 3/2)", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN8,  "Reserved (Module/Fan 4/2)", 0}, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN9,  "PSU-1 Fan", 0 }, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN10, "PSU-2 Fan", 0 }, ONLP_FAN_STATUS_PRESENT },

};

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* rv)
{
    int fid = ONLP_OID_ID_GET(id);

    *rv = fans__[ONLP_OID_ID_GET(id)];
    rv->caps |= ONLP_FAN_CAPS_GET_RPM;

    switch(fid) {
		case FAN_ID_FAN1:
		case FAN_ID_FAN2:
		case FAN_ID_FAN3:
		case FAN_ID_FAN4:
		case FAN_ID_FAN5:
		case FAN_ID_FAN6:
		case FAN_ID_FAN7:
		case FAN_ID_FAN8:
			return sys_fan_info_get__(rv, fid);
			break;

		case FAN_ID_FAN9:
		case FAN_ID_FAN10:
			return psu_fan_info_get__(rv, fid - FAN_ID_FAN9 + 1);
			break;

		default:
			return ONLP_STATUS_E_INVALID;
			break;
	}

	return ONLP_STATUS_E_INVALID;
}
