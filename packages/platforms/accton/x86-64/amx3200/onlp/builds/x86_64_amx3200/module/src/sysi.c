/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <unistd.h>
#include <fcntl.h>

#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#include "x86_64_amx3200_int.h"
#include "x86_64_amx3200_log.h"

#define NUM_OF_CPLD_VER                   2

static char* cpld_ver_path[NUM_OF_CPLD_VER] = {
    "/sys/bus/platform/devices/amx3200_sys/cpld_sys_ver",
    "/sys/devices/platform/amx3200_fan/hwmon/hwmon2/version"
};

char* send_sled_thermal_path = "/sys/devices/platform/amx3200_thermal*temp_sled_input";

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-amx3200-r0";
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);
    if (onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK) {
        if(*size == 256) {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }

    aim_free(rdata);
    *size = 0;
    return ONLP_STATUS_E_INTERNAL;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 7 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 4 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 10 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}


int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int i, len, ret = ONLP_STATUS_OK;
    char *v[NUM_OF_CPLD_VER] = {NULL};

    for (i = 0; i < AIM_ARRAYSIZE(cpld_ver_path); i++) {
        len = onlp_file_read_str(&v[i], cpld_ver_path[i]);
        if (v[i] == NULL || len <= 0) {
            ret = ONLP_STATUS_E_INTERNAL;
            break;
        }
    }

    if (ret == ONLP_STATUS_OK) {
        pi->cpld_versions = aim_fstrdup("\r\nMAIN CPLD:%s"
                                        "\r\nFan CPLD:%s"
                                        , v[0], v[1]);
    }

    for (i = 0; i < AIM_ARRAYSIZE(v); i++) {
        AIM_FREE_IF_PTR(v[i]);
    }

    return ret;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

int
onlp_sysi_platform_manage_fans(void)
{
    int i, rv;
    onlp_thermal_info_t ti[2];
    int sled_remote_thermal[2];
    int sled_thermal_id[2] = {THERMAL_6_ON_SLED1_REMOTE_BROAD, THERMAL_8_ON_SLED2_REMOTE_BROAD};
    char set_sled_termal[64] = {0};

    /* Get sled1 and sled2 remote thermal*/
    for (i = 0; i < 2 ; i++)
    {
        if (!onlp_sled_board_is_ready(i + 1))
        {
            sled_remote_thermal[i] =  0;
            continue;
        }

        rv = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(sled_thermal_id[i]), &ti[i]);
        if (rv != ONLP_STATUS_OK) {
            return ONLP_STATUS_E_INTERNAL;
        }

        sled_remote_thermal[i] =  (ti[i].mcelsius/1000);
    }

    sprintf(set_sled_termal, "%d %d", sled_remote_thermal[0], sled_remote_thermal[1]);

    onlp_file_write_str(set_sled_termal, send_sled_thermal_path);

    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

