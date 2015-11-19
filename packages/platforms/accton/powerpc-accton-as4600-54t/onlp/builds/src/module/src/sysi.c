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
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>

#include "powerpc_accton_as4600_54t_int.h"
#include "powerpc_accton_as4600_54t_log.h"

const char*
onlp_sysi_platform_get(void)
{
    return "powerpc-accton-as4600-54t-rX";
}

int
onlp_sysi_platform_set(const char* platform)
{
    /*
     * Support all revisions of the 4600-54t
     */
    if(strstr(platform, "powerpc-accton-as4600-54t-r")) {
        return ONLP_STATUS_OK;
    }

    if(strstr(platform, "powerpc-as4600-54t")) {
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

    /* 4 Thermal sensors on the chassis */
    *e++ = ONLP_THERMAL_ID_CREATE(1);
    *e++ = ONLP_THERMAL_ID_CREATE(2);
    *e++ = ONLP_THERMAL_ID_CREATE(3);

    /* 2 PSUs */
    *e++ = ONLP_PSU_ID_CREATE(1);
    *e++ = ONLP_PSU_ID_CREATE(2);

    /* 2 Fans */
    *e++ = ONLP_FAN_ID_CREATE(1);
    *e++ = ONLP_FAN_ID_CREATE(2);

    /* 7 LEDs */
    *e++ = ONLP_LED_ID_CREATE(1);
    *e++ = ONLP_LED_ID_CREATE(2);
    *e++ = ONLP_LED_ID_CREATE(3);
    *e++ = ONLP_LED_ID_CREATE(4);
    *e++ = ONLP_LED_ID_CREATE(5);
    *e++ = ONLP_LED_ID_CREATE(6);
    *e++ = ONLP_LED_ID_CREATE(7);

    return 0;
}



#include <onlp/platformi/fani.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/psui.h>

int
onlp_sysi_platform_manage_fans(void)
{
    /* This comes from the fan initialization */
    static int previous_percentage = 100;
    int current_condition = 0;

    /*
     * If any fan is missing or failed set the other to maximum.
     */
    onlp_fan_info_t fi;
    if( (onlp_fani_info_get(ONLP_FAN_ID_CREATE(1), &fi) < 0) ||
        (fi.status & ONLP_FAN_STATUS_FAILED) ||
        ((fi.status & 0x1) == 0) ) {
        AIM_LOG_ERROR("Fan 1 missing or failed.");
        current_condition = 3;
        goto control;
    }

    if( (onlp_fani_info_get(ONLP_FAN_ID_CREATE(2), &fi) < 0) ||
        (fi.status & ONLP_FAN_STATUS_FAILED) ||
        ((fi.status & 0x1) == 0) ) {
        AIM_LOG_ERROR("Fan 2 missing or failed.");
        current_condition = 3;
        goto control;
    }

    /*
     * Temperature Condition
     */
    onlp_thermal_info_t ti;
    int percentage = 50;
    int t1, t2;


    if( (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(1), &ti) < 0) ||
        (ti.status & ONLP_THERMAL_STATUS_FAILED) ||
        ((ti.status & 0x1) == 0) ) {
        current_condition = 3;
        goto control;
    }
    else {
        t1 = ti.mcelsius;
    }

    if( (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(2), &ti) < 0) ||
        (ti.status & ONLP_THERMAL_STATUS_FAILED) ||
        ((ti.status & 0x1) == 0) ) {
        current_condition = 3;
        goto control;
    }
    else {
        t2 = ti.mcelsius;
    }

    /*
     * Determine current temperature condition if we aren't in
     * an exception state:
     */
    if(current_condition != 3) {
        if(t1 >= 55000 || t2 >= 65000) {
            current_condition = 1;
        }
        if(t1 < 45000 && t2 < 55000) {
            current_condition = 2;
        }
    }

 control:

    switch(current_condition)
        {
        case 3:
            percentage = 100;
            break;
        case 2:
            switch(previous_percentage)
                {
                case 100:
                    percentage = 75;
                    break;
                case 75:
                    percentage = 50;
                    break;
                }
            break;
        case 1:
            switch(previous_percentage)
                {
                case 50:
                    percentage = 75;
                    break;
                case 75:
                    percentage = 100;
                    break;
                }
            break;
        }

    AIM_LOG_MSG("Fans are now at %d%%", percentage);
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), percentage);
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(2), percentage);
    previous_percentage = percentage;
    return 0;
}


int
onlp_sysi_platform_manage_leds(void)
{

    /*
     * FAN Indicators
     *
     *     Green - Good
     *     Amber - Present but Failed
     *     Off   - Missing
     *
     */

    /*
     * FAN1
     */
    onlp_fan_info_t fi;
    onlp_led_mode_t mode = ONLP_LED_MODE_GREEN;
    if(onlp_fani_info_get(ONLP_FAN_ID_CREATE(1), &fi) < 0) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    else if( (fi.status & 0x1) == 0) {
        /* Not present -- Off */
        mode = ONLP_LED_MODE_OFF;
    }
    else if(fi.status & ONLP_FAN_STATUS_FAILED) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    onlp_ledi_mode_set(LED_FAN_1, mode);

    /*
     * FAN2
     */
    mode = ONLP_LED_MODE_GREEN;
    if(onlp_fani_info_get(ONLP_FAN_ID_CREATE(2), &fi) < 0) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    else if( (fi.status & 0x1) == 0) {
        /* Not present -- Off */
        mode = ONLP_LED_MODE_OFF;
    }
    else if(fi.status & ONLP_FAN_STATUS_FAILED) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    onlp_ledi_mode_set(LED_FAN_2, mode);


    /*
     * PSU Indicators
     *
     *     Green - Good
     *     Amber - Present but failed/unplugged
     *     Off   - Missing
     *
     */
    onlp_psu_info_t pi;

    /* PSU1 */
    mode = ONLP_LED_MODE_GREEN;
    if(onlp_psui_info_get(ONLP_PSU_ID_CREATE(1), &pi) < 0) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    else if( (pi.status & 0x1) == 0) {
        /* Not present */
        mode = ONLP_LED_MODE_OFF;
    }
    else if(pi.status & ONLP_PSU_STATUS_FAILED) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    else if(pi.status & ONLP_PSU_STATUS_UNPLUGGED) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    onlp_ledi_mode_set(LED_PSU_1, mode);

    /* PSU2 */
    mode = ONLP_LED_MODE_GREEN;
    if(onlp_psui_info_get(ONLP_PSU_ID_CREATE(2), &pi) < 0) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    else if( (pi.status & 0x1) == 0) {
        /* Not present */
        mode = ONLP_LED_MODE_OFF;
    }
    else if(pi.status & ONLP_PSU_STATUS_FAILED) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    else if(pi.status & ONLP_PSU_STATUS_UNPLUGGED) {
        mode = ONLP_LED_MODE_ORANGE;
    }
    onlp_ledi_mode_set(LED_PSU_2, mode);

    return ONLP_STATUS_OK;
}
