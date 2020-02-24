/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *        Copyright 2017 Delta Networks, Inc.
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
 * HardwareVersion: 02
 *
 ***********************************************************/
#include <onlp/platformi/sysi.h>
#include <onlplib/file.h>
#include "platform_lib.h"

const char* onlp_sysi_platform_get(void)
{
    return "x86-64-delta-ag9064-r0";
}

int onlp_sysi_init(void)
{
    lockinit();
    return ONLP_STATUS_OK;
}

int onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);

    if (onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK)
    {
        if (*size == 256)
        {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }
    aim_free(rdata);
    *size = 0;

    return ONLP_STATUS_E_INTERNAL;
}

int onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    uint32_t u4Data = 0;

    u4Data = dni_i2c_lock_read_attribute(CPU_CPLD_VERSION, ATTRIBUTE_BASE_HEX);
    u4Data = u4Data >> CPLD_VERSION_OFFSET;
    pi->cpld_versions = aim_fstrdup("%d", u4Data);

    return ONLP_STATUS_OK;
}

void onlp_sysi_onie_data_free(uint8_t* data)
{
    aim_free(data);
}

void onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

int onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i = 0;
    onlp_oid_t* e = table;
    
    memset(table, 0, max*sizeof(onlp_oid_t));
    
    for (i = 1; i <= NUM_OF_THERMAL_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    for (i = 1; i <= NUM_OF_LED_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    for (i = 1; i <= NUM_OF_PSU_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    for (i = 1; i <= NUM_OF_FAN_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_manage_fans(void)
{
    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_OK;
}
