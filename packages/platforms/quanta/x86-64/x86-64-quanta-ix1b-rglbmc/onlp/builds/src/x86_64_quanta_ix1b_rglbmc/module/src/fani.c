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
#include "x86_64_quanta_ix1b_rglbmc_int.h"
#include <onlplib/file.h>

int
onlp_fani_init(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
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
    { { FAN_OID_FAN1,  "PSU-1 Fan", 0 }, ONLP_FAN_STATUS_PRESENT },
    { { FAN_OID_FAN2,  "PSU-2 Fan", 0 }, ONLP_FAN_STATUS_PRESENT },
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
			return psu_fan_info_get__(rv, fid);
			break;

		default:
			return ONLP_STATUS_E_INVALID;
			break;
	}

	return ONLP_STATUS_E_INVALID;
}
