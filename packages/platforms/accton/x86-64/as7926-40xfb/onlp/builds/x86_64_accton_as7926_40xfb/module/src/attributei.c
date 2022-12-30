/************************************************************
 * <bsn.cl fy=2017 v=onl>
 *
 *        Copyright 2017 Big Switch Networks, Inc.
 *        Copyright 2017 Accton Technology Corporation.
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
 ********************************************************//**
 *
 *
 *
 ***********************************************************/
#include <onlp/platformi/attributei.h>
#include <onlplib/file.h>
#include "x86_64_accton_as7926_40xfb_log.h"

#define NUM_OF_CPLD 3

#define IDPROM_PATH "/sys/devices/platform/as7926_40xfb_sys/eeprom"

static char* cpld_path[NUM_OF_CPLD] = {
    "/sys/bus/i2c/devices/12-0062/version",
    "/sys/bus/i2c/devices/13-0063/version",
    "/sys/bus/i2c/devices/20-0064/version"
};

int
onlp_attributei_sw_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_attributei_sw_denit(void)
{
	return ONLP_STATUS_OK;
}

int
onlp_attributei_onie_info_get(onlp_oid_t oid, onlp_onie_info_t* rp)
{
    if (oid != ONLP_OID_CHASSIS) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if (rp == NULL) {
        return 1;
    }

    return onlp_onie_decode_file(rp, IDPROM_PATH);
}

int
onlp_attributei_asset_info_get(onlp_oid_t oid, onlp_asset_info_t* rp)
{
    int rv = ONLP_STATUS_OK;
    int i, v[NUM_OF_CPLD] = {0};

    if (oid != ONLP_OID_CHASSIS) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if (rp == NULL) {
        return 1;
    }

    rp->oid = oid;
    rp->manufacturer = aim_strdup("Accton");

    for (i = 0; i < AIM_ARRAYSIZE(cpld_path); i++) {
        rv = onlp_file_read_int(v+i, cpld_path[i]);

        if (ONLP_FAILURE(rv)) {
            break;
        }

        if(onlp_file_read_int(v+i, cpld_path[i]) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    if (ONLP_SUCCESS(rv)) {
        rp->firmware_revision = aim_fstrdup("%d.%d.%d", v[0], v[1], v[2]);
    }

    return rv;
}
