/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/psui.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlplib/file.h>
#include <onlplib/crc32.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "platform_lib.h"

bool bmc_enable = false;

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-ufispace-s9500-22xst-r0";
}

int
onlp_sysi_platform_set(const char* platform)
{
    AIM_LOG_INFO("Set ONL platform interface to '%s'\n", platform);
    AIM_LOG_INFO("Real HW Platform: '%s'\n", onlp_sysi_platform_get());
    return ONLP_STATUS_OK;
}

int
onlp_sysi_init(void)
{    
    /* check if the platform is bmc enabled */
    if ( onlp_sysi_bmc_en_get() ) {
        bmc_enable = true;
    } else {
        bmc_enable = false;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(SYS_EEPROM_SIZE);
    if(onlp_file_read(rdata, SYS_EEPROM_SIZE, size, SYS_EEPROM_PATH) == ONLP_STATUS_OK) {
        if(*size == SYS_EEPROM_SIZE) {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }
    
    AIM_LOG_INFO("Unable to get data from eeprom \n");    
    aim_free(rdata);
    *size = 0;
    return ONLP_STATUS_E_INTERNAL;
}

void
onlp_sysi_onie_data_free(uint8_t* data)
{
    if (data) {
        aim_free(data);
    }
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));
    int i;

    if ( !bmc_enable ) {
        /* 2 PSUs */
        *e++ = ONLP_PSU_ID_CREATE(1);
        *e++ = ONLP_PSU_ID_CREATE(2);

        /* LEDs Item */
        for (i=1; i<=LED_NUM; i++) {
            *e++ = ONLP_LED_ID_CREATE(i);
        }

        /* Fans Item */
        for (i=1; i<=FAN_NUM; i++) {
            *e++ = ONLP_FAN_ID_CREATE(i);
        }

        for (i=1; i<=THERMAL_NUM; i++) {
            *e++ = ONLP_THERMAL_ID_CREATE(i);
        }
    } else {
        *e++ = THERMAL_OID_CPU;
        *e++ = THERMAL_OID_MAC;
        *e++ = THERMAL_OID_BMC;
        *e++ = THERMAL_OID_100G_CAGE;
        *e++ = THERMAL_OID_DDR4;
        *e++ = THERMAL_OID_FANCARD1;
        *e++ = THERMAL_OID_FANCARD2;
        *e++ = THERMAL_OID_PSU0;
        *e++ = THERMAL_OID_PSU1;
        *e++ = THERMAL_OID_CPU_PKG;
        *e++ = THERMAL_OID_CPU1;
        *e++ = THERMAL_OID_CPU2;
        *e++ = THERMAL_OID_CPU3;
        *e++ = THERMAL_OID_CPU4;
        *e++ = THERMAL_OID_CPU_BOARD;
        *e++ = THERMAL_OID_AMB;
        *e++ = THERMAL_OID_PHY1;
        *e++ = THERMAL_OID_HEATER;

        *e++ = LED_OID_SYSTEM;
        *e++ = LED_OID_SYNC;
        *e++ = LED_OID_GPS;

        *e++ = PSU_OID_PSU0;
        *e++ = PSU_OID_PSU1;

        *e++ = FAN_OID_FAN1;
        *e++ = FAN_OID_FAN2;
        *e++ = FAN_OID_FAN3;
        //*e++ = FAN_OID_PSU0_FAN;
        //*e++ = FAN_OID_PSU1_FAN;
    }
    
    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_fans(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int rc;
    if ((rc = sysi_platform_info_get(pi)) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }
    
    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    if (pi->cpld_versions) {
        aim_free(pi->cpld_versions);
    }

    if (pi->other_versions) {
        aim_free(pi->other_versions);
    }
}

