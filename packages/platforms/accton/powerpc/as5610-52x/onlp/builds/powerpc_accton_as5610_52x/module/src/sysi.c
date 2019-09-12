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
#include "platform_lib.h"

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

    /* 11 Thermal sensors */
    *e++ = ONLP_THERMAL_ID_CREATE(NE1617A_LOCAL_SENSOR);
    *e++ = ONLP_THERMAL_ID_CREATE(NE1617A_REMOTE_SENSOR);
    *e++ = ONLP_THERMAL_ID_CREATE(MAX6581_LOCAL_SENSOR);
    *e++ = ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_1);
    *e++ = ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_2);
    *e++ = ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_3);
    *e++ = ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_4);
    *e++ = ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_5);
    *e++ = ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_6);
    *e++ = ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_7);
    *e++ = ONLP_THERMAL_ID_CREATE(BCM56846_LOCAL_SENSOR);

    /* 5 LEDs */
    *e++ = ONLP_LED_ID_CREATE(1);
    *e++ = ONLP_LED_ID_CREATE(2);
    *e++ = ONLP_LED_ID_CREATE(3);
    *e++ = ONLP_LED_ID_CREATE(4);
    *e++ = ONLP_LED_ID_CREATE(5);

    return 0;
}

/* The tA/tB/tC/tD/tCritical is defined in the thermal policy spec
 */
typedef struct temp_sensor_threshold {
    int  tA;
    int  tB;
    int  tC;
    int  tD;
    int  tCitical;
} temp_sensor_threshold_t;

/* The thermal plan table is for F2B(Front to back) airflow direction
 */
static const temp_sensor_threshold_t temp_sensor_threshold_f2b[NUM_OF_CHASSIS_THERMAL_SENSOR] = {
{53, 46, 62, 67, 75}, {71, 63, 82, 86, 95}, {46, 38, 53, 58, 66}, {49, 42, 57, 62, 69},
{59, 52, 70, 73, 81}, {57, 50, 65, 70, 77}, {49, 42, 57, 62, 70}, {47, 39, 55, 60, 69},
{53, 46, 59, 65, 73}, {56, 48, 65, 69, 78}, {75, 71, 89, 94, 111}
};

/* The thermal plan table is for B2F(Back to front) airflow direction
 */
static const temp_sensor_threshold_t temp_sensor_threshold_b2f[NUM_OF_CHASSIS_THERMAL_SENSOR] = {
{58, 52, 73, 73, 80}, {63, 51, 67, 70, 84}, {43, 33, 47, 51, 63}, {49, 38, 56, 59, 69},
{53, 47, 65, 68, 75}, {63, 57, 80, 78, 84}, {46, 37, 52, 55, 67}, {44, 34, 48, 52, 65},
{55, 47, 65, 67, 76}, {47, 38, 53, 56, 68}, {81, 78, 99, 98, 104}
};

#include <onlp/platformi/fani.h>
#include <onlp/platformi/thermali.h>

int
onlp_sysi_platform_manage_init(void)
{
    /*
     * Bring the fan to max.
     * These will be reduced after the first platform management sequence.
     */
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_PERCENTAGE_MAX);
    return 0;
}

int
onlp_sysi_platform_manage_fans(void)
{
    int i = 0, rc;
    onlp_fan_info_t fi;
    const temp_sensor_threshold_t *pThreshold = NULL;
    onlp_thermal_info_t ti[NUM_OF_CHASSIS_THERMAL_SENSOR];
    int aboveTa = 0, aboveTb = 0, aboveTc = 0, aboveTd = 0, newPercentage = 0;

    /* Get current fan status
     */
    rc = onlp_fani_info_get(ONLP_FAN_ID_CREATE(1), &fi);
    if (rc < 0) {
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_PERCENTAGE_MAX);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (fi.status & ONLP_FAN_STATUS_FAILED) {
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_PERCENTAGE_MAX);
        return ONLP_STATUS_OK;
    }

    /* Bring fan speed to max if current speed is not expected
     */
    if (fi.percentage != FAN_PERCENTAGE_MIN && 
        fi.percentage != FAN_PERCENTAGE_MID && 
        fi.percentage != FAN_PERCENTAGE_MAX  ) {
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_PERCENTAGE_MAX);
        return ONLP_STATUS_OK;
    }    

    /* Apply thermal plan by air flow direction 
     */
    pThreshold = (fi.status & ONLP_FAN_STATUS_F2B) ? temp_sensor_threshold_f2b :
                                                     temp_sensor_threshold_b2f ;

    /* Get temperature from each thermal sensor 
     */
    for (i = 0; i < NUM_OF_CHASSIS_THERMAL_SENSOR; i++) {
        rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i+1), &ti[i]);
        
        if (rc != ONLP_STATUS_OK) {
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_PERCENTAGE_MAX);
            return rc;
        }

        aboveTa += (ti[i].mcelsius > pThreshold[i].tA* TEMPERATURE_MULTIPLIER) ? 1 : 0;
        aboveTb += (ti[i].mcelsius > pThreshold[i].tB* TEMPERATURE_MULTIPLIER) ? 1 : 0;
        aboveTc += (ti[i].mcelsius > pThreshold[i].tC* TEMPERATURE_MULTIPLIER) ? 1 : 0;
        aboveTd += (ti[i].mcelsius > pThreshold[i].tD* TEMPERATURE_MULTIPLIER) ? 1 : 0;
    }

    /* Adjust fan speed based on current temperature if fan speed changed
     */
    if (fi.percentage == FAN_PERCENTAGE_MIN && aboveTc) {
        newPercentage = FAN_PERCENTAGE_MID;
    }
    else if (fi.percentage == FAN_PERCENTAGE_MID) {
        if (aboveTd) {
            newPercentage = FAN_PERCENTAGE_MAX;
        }          
        else if (!aboveTb) {
            newPercentage = FAN_PERCENTAGE_MIN;
        }
    }
    else if (fi.percentage == FAN_PERCENTAGE_MAX && !aboveTa) {
        newPercentage = FAN_PERCENTAGE_MID;
    }

    if (newPercentage != 0) {
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), newPercentage);
    }
    
    return 0;
}

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

