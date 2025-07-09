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
#include "x86_64_accton_dcg8510_32d_int.h"
#include "x86_64_accton_dcg8510_32d_log.h"

//#define IDPROM_PATH "/sys/bus/i2c/devices/i2c-0/0-0057/eeprom"
#define IDPROM_PATH "/sys/switch/syseeprom/eeprom"
#define SYSTEM_CPLD_REV_PATH "/sys/switch/cpld/cpld1/firmware_version"
#define FAN_CPLD_REV_PATH "/sys/switch/cpld/cpld2/firmware_version"
#define PORT_CPLD1_REV_PATH "/sys/switch/cpld/cpld3/firmware_version"
#define PORT_CPLD2_REV_PATH "/sys/switch/cpld/cpld4/firmware_version"

typedef struct cpld_version
{
    char *attr_name;
    char  version[16];
    char *description;
} cpld_version_t;

const char*
onlp_sysi_platform_get(void)
{
/*     if (fpga_pltfm_init(0) != 0)
    {
        return NULL;
    } */

    return "x86-64-accton-dcg8510-32d-r0";
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
    
    /* 20 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 4 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 6 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    //bmc_tty_init();

    return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int i;
    char  path[64] = {0};
    cpld_version_t cplds[] = { { "sys_cpld_ver", "", "SYSTEM CPLD"},
                               { "fan_cpld_ver", "", "FAN CPLD"},
                               { "port_cpld1_ver", "", "PORT CPLD1"},
                               { "port_cpld2_ver", "", "PORT CPLD2"},
                            };
    uint8_t version_strings[16]={0};
    int readi;
    /* Read CPLD version */
   for (i = 0; i < AIM_ARRAYSIZE(cplds); i++)
   {
        if(strcmp("sys_cpld_ver", cplds[i].attr_name) ==0)
        {
            strcpy(path, SYSTEM_CPLD_REV_PATH);
        }

        if(strcmp("fan_cpld_ver", cplds[i].attr_name) ==0)
        {
            strcpy(path, FAN_CPLD_REV_PATH);
        }

        if(strcmp("port_cpld1_ver", cplds[i].attr_name) ==0)
        {
            strcpy(path, PORT_CPLD1_REV_PATH);
        }        

        if(strcmp("port_cpld2_ver", cplds[i].attr_name) ==0)
        {
            strcpy(path, PORT_CPLD2_REV_PATH);
        }

        if(0 > onlp_file_read(version_strings, 16, &readi, path))
            return ONLP_STATUS_E_INTERNAL;

 
        strcpy(cplds[i].version, (char*)version_strings);  
    }

    pi->cpld_versions = aim_fstrdup("%s:%s, %s:%s,%s:%s", 
                                    cplds[0].description, cplds[0].version,
                                    cplds[1].description, cplds[1].version,
                                    cplds[2].description, cplds[2].version,
                                    cplds[3].description, cplds[3].version);
    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

int
onlp_sysi_platform_manage_fans(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
