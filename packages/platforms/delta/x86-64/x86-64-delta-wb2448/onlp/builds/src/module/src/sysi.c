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
 *
 *
 ***********************************************************/
#include <onlp/platformi/sysi.h>
#include <onlplib/crc32.h>
#include <onlplib/file.h>

#include "x86_64_delta_wb2448_log.h"
#include "platform_lib.h"

#define IDPROM_PATH                     "/sys/class/i2c-adapter/i2c-0/0-0053/eeprom"

#define NUM_OF_THERMAL_ON_MAIN_BROAD    5
#define NUM_OF_LED_ON_MAIN_BROAD        3
#define NUM_OF_PSU_ON_MAIN_BROAD        1
#define NUM_OF_FAN_ON_MAIN_BROAD        2

#define I2C_BUS                 0x04
#define CPLD_ADDR               0x28
#define CPLD_VERSION_REGISTER   0x08
#define CPLD_VERSION_ADDR_LEN   0x01
#define CPLD_VERSION_DATA_LEN   0x01
#define CPLD_VERSION_OFFSET     5

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-delta-wb2448-r0";
}

int
onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);
    
    if(onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK) 
    {
        if(*size == 256) 
        {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }
    
    aim_free(rdata);
    *size = 0;
    
    return ONLP_STATUS_E_INTERNAL;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    INT4 rv      = ONLP_STATUS_OK;
    UINT4 u4Data = 0;
    
    rv = ifnOS_LINUX_BmcI2CGet(I2C_BUS, CPLD_ADDR, CPLD_VERSION_REGISTER, CPLD_VERSION_ADDR_LEN, &u4Data, CPLD_VERSION_DATA_LEN);
    
    if(rv == ONLP_STATUS_OK)
    {
        u4Data = u4Data >> CPLD_VERSION_OFFSET;
        
        pi->cpld_versions = aim_fstrdup("%d", u4Data);
    }
        
    return rv;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i = 0;
    onlp_oid_t* e = table;
    
    memset(table, 0, max*sizeof(onlp_oid_t));
    
    /* 5 Thermal sensors on the chassis */
    for (i = 1; i <= NUM_OF_THERMAL_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }
    
    /* 3 LEDs on the chassis */
    for (i = 1; i <= NUM_OF_LED_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }
    
    /* 1 PSUs on the chassis */
    for (i = 1; i <= NUM_OF_PSU_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }
    
    /* 2 Fans on the chassis */
    for (i = 1; i <= NUM_OF_FAN_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }
    
    return 0;
}

int 
onlp_sysi_ioctl(int code, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}


