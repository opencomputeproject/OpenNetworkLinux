/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 MiTAC Computing Technology Corporation.
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

#include "x86_64_mitac_ly1200_b32h0_c3_int.h"
#include "x86_64_mitac_ly1200_b32h0_c3_log.h"

#include "platform_lib.h"

#define NUM_OF_THERMAL_ON_DEVICE    5
#define NUM_OF_FAN_ON_DEVICE        6
#define NUM_OF_PSU_ON_DEVICE        2
#define NUM_OF_LED_ON_DEVICE        4

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-mitac-ly1200-b32h0-c3-r0";
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);
    if(onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK) {
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

    /* Thermal sensors */
    for (i = 1; i <= NUM_OF_THERMAL_ON_DEVICE; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* Fans */
    for (i = 1; i <= NUM_OF_FAN_ON_DEVICE; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* PSUs */
    for (i = 1; i <= NUM_OF_PSU_ON_DEVICE; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* LEDs */
    for (i = 1; i <= NUM_OF_LED_ON_DEVICE; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    return ONLP_STATUS_OK;
}
