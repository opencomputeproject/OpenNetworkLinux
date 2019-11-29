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
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/sysi.h>
#include "x86_64_delta_agc7646v1_int.h"
#include "x86_64_delta_agc7646v1_log.h"
#include "platform_lib.h"
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

const char* onlp_sysi_platform_get(void)
{
    return "x86-64-delta-agc7646v1-r0";
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

    return ONLP_STATUS_E_UNSUPPORTED;
}

int onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    long cpld_version = 0;
    int swpld1_version = 0x0;
    int swpld2_version = 0x0;
    int swpld3_version = 0x0;
    int reg_t = 0x01;

    cpld_version = dni_i2c_lock_read_attribute(CPU_CPLD_VERSION, ATTRIBUTE_BASE_HEX);
    dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_1_ADDR, reg_t, &swpld1_version);
    dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, reg_t, &swpld2_version);
    dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_3_ADDR, reg_t, &swpld3_version);

    pi->cpld_versions = aim_fstrdup("%ld, SWPLD1_Version: %d, SWPLD2_Version: %d, SWPLD3_Version: %d", cpld_version, swpld1_version, swpld2_version, swpld3_version);

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

int
onlp_sysi_ioctl(int code, va_list vargs)
{
    switch(code)
    {
        case I2C_ACCESS:
        {
            char rw;
            dev_info_t dev_info;

            rw = va_arg(vargs,int);

            dev_info.bus = va_arg(vargs,int);
            dev_info.addr = (uint8_t)va_arg(vargs,int);
            dev_info.offset = (uint8_t)va_arg(vargs,int);
            dev_info.flags = (uint32_t)va_arg(vargs,int);
            dev_info.size = va_arg(vargs,int);

            if (rw == 'r') //read
            {
                uint8_t* data = va_arg(vargs, uint8_t*);
                *data = dni_i2c_lock_read(NULL, &dev_info);
                return ONLP_STATUS_OK;
            }
            else if (rw == 'w') //write
            {
                if (dev_info.size == 1)
                    dev_info.data_8 = (uint8_t)va_arg(vargs,int);
                else
                    dev_info.data_16 = (uint16_t)va_arg(vargs,int);

                return dni_i2c_lock_write(NULL, &dev_info);
            }
            break;
        }
        default:
            break;
    }

    return ONLP_STATUS_OK;
}
