/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
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
#include "powerpc_accton_as5610_52x_log.h"

const char*
onlp_sysi_platform_get(void)
{
    return "powerpc-accton-as5610-52x-rX";
}

int
onlp_sysi_platform_set(const char* platform)
{
    /** Support all revisions of the 5610-52x */
    if(strstr(platform, "powerpc-accton-as5610-52x-r")) {
        return ONLP_STATUS_OK;
    }
    if(strstr(platform, "powerpc-as5610-52x")) {
        return ONLP_STATUS_OK;
    }

    AIM_LOG_ERROR("No support for platform '%s'", platform);
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}


int
onlp_sysi_onie_data_phys_addr_get(void** physaddr)
{
    *physaddr = (void*)(0xeff70000);
    return ONLP_STATUS_OK;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 2 PSUs */
    *e++ = ONLP_PSU_ID_CREATE(1);
    *e++ = ONLP_PSU_ID_CREATE(2);

    /* 1 Fan */
    *e++ = ONLP_FAN_ID_CREATE(1);

    /* 9 Thermal sensors */
    *e++ = ONLP_THERMAL_ID_CREATE(1);
    *e++ = ONLP_THERMAL_ID_CREATE(2);
    *e++ = ONLP_THERMAL_ID_CREATE(3);
    *e++ = ONLP_THERMAL_ID_CREATE(4);
    *e++ = ONLP_THERMAL_ID_CREATE(5);
    *e++ = ONLP_THERMAL_ID_CREATE(6);
    *e++ = ONLP_THERMAL_ID_CREATE(7);
    *e++ = ONLP_THERMAL_ID_CREATE(8);
    *e++ = ONLP_THERMAL_ID_CREATE(11);

    /* 5 LEDs */
    *e++ = ONLP_LED_ID_CREATE(1);
    *e++ = ONLP_LED_ID_CREATE(2);
    *e++ = ONLP_LED_ID_CREATE(3);
    *e++ = ONLP_LED_ID_CREATE(4);
    *e++ = ONLP_LED_ID_CREATE(5);

    return 0;
}

int
onlp_sysi_platform_manage_fans(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include <onlp/platformi/ledi.h>

int
onlp_sysi_platform_manage_leds(void)
{
    int rv;
    onlp_fan_info_t fi;
    onlp_led_mode_t mode = ONLP_LED_MODE_GREEN;

    rv = onlp_fani_info_get(ONLP_FAN_ID_CREATE(1), &fi);
    if(rv < 0) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    else if((fi.status & 1) == 0) {
        mode = ONLP_LED_MODE_OFF;
    }
    else if(fi.status & ONLP_FAN_STATUS_FAILED) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(2), mode);

    onlp_psu_info_t pi;

    mode = ONLP_LED_MODE_GREEN;
    rv = onlp_psu_info_get(ONLP_PSU_ID_CREATE(1), &pi);
    if(rv < 0) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    else if((pi.status & 1) == 0) {
        mode = ONLP_LED_MODE_OFF;
    }
    else if(pi.status & ONLP_PSU_STATUS_FAILED) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(4), mode);


    mode = ONLP_LED_MODE_GREEN;
    rv = onlp_psu_info_get(ONLP_PSU_ID_CREATE(2), &pi);
    if(rv < 0) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    else if((pi.status & 1) == 0) {
        mode = ONLP_LED_MODE_OFF;
    }
    else if(pi.status & ONLP_PSU_STATUS_FAILED) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(5), mode);

    return 0;
}




