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
#include "x86_64_accton_as9926_24db_log.h"

#define IDPROM_PATH "/sys/devices/platform/as9926_24db_sys/eeprom"
#define CPLD_VERSION_FORMAT "/sys/devices/platform/as9926_24db_sys/%s"

typedef struct cpld_version {
    char *attr_name;
    int   version;
    char *description;
} cpld_version_t;

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
    int i, rv = ONLP_STATUS_OK;
    cpld_version_t cplds[] = { { "mb_cpld2_ver", 0, "Mainboard-CPLD#2"},
                   { "mb_cpld3_ver", 0, "Mainboard-CPLD#3"},
                   { "cpu_cpld_ver", 0, "CPU-CPLD"},
                   { "fan_cpld_ver", 0, "FAN-CPLD"},
                   { "fpga_cpld_ver", 0, "FPGA-CPLD"} };

    if (oid != ONLP_OID_CHASSIS) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if (rp == NULL) {
        return 1;
    }

    rp->oid = oid;
    rp->manufacturer = aim_strdup("Accton");

    for (i = 0; i < AIM_ARRAYSIZE(cplds); i++) {
        rv = onlp_file_read_int(&cplds[i].version,
                     CPLD_VERSION_FORMAT,
                     cplds[i].attr_name);
        if (ONLP_FAILURE(rv)) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    rp->firmware_revision = aim_fstrdup("%s:%d, %s:%d, %s:%d, %s:%d, %s:%d",
                cplds[0].description,
                cplds[0].version,
                cplds[1].description,
                cplds[1].version,
                cplds[2].description,
                cplds[2].version,
                cplds[3].description,
                cplds[3].version,
                cplds[4].description,
                cplds[4].version);

    return rv;
}
