/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 Accton Technology Corporation.
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

#include "x86_64_accton_as7936_22xke_int.h"
#include "x86_64_accton_as7936_22xke_log.h"





const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as7936-22xke-r0";
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

    /* 11 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 4 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 3 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 12 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}



int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    #define MAX_STRLEN   8
    #define CPLD_VERSION_FORMAT			"/sys/bus/i2c/devices/%s/version"
    static char* cpld_path[] =
    {
        "11-0060",
        "12-0061",
        "13-0062",
        "22-0068"
    };

    static char cpld_vers[128]={0};
    int   i, v[AIM_ARRAYSIZE(cpld_path)] = {0};
    char  vstr[AIM_ARRAYSIZE(cpld_path)][MAX_STRLEN+1] = {{0}};

    pi->cpld_versions = cpld_vers;
    if (AIM_STRLEN(pi->cpld_versions) > 0) {
        return ONLP_STATUS_OK;
    }
    for (i = 0; i < AIM_ARRAYSIZE(cpld_path); i++) {
        v[i] = 0;
        if(onlp_file_read_int(&v[i], CPLD_VERSION_FORMAT , cpld_path[i]) < 0) {
            memset(cpld_vers, 0, sizeof(cpld_vers));
            return ONLP_STATUS_E_INTERNAL;
        }
        ONLPLIB_SNPRINTF(vstr[i], MAX_STRLEN, "%02x.", v[i]);
        AIM_STRCAT(cpld_vers, vstr[i]);
    }
    /*strip off last char, a dot*/
    cpld_vers[AIM_STRLEN(cpld_vers)-1] = '\0';
    return ONLP_STATUS_OK;

#undef  MAX_STRLEN
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
}

int
onlp_sysi_platform_manage_init(void)
{
    /* Set cs4227 to normal state . need to do before network interface start.
     * So we set net to down, csp4227, do net to up.
     */
    char cmd_str[64];

    memset(cmd_str, 0x0, strlen(cmd_str));
    snprintf(cmd_str, 63, "ifconfig eth1 down");
    system(cmd_str);
    snprintf(cmd_str, 63, "ifconfig eth2 down");
    system(cmd_str);

    snprintf(cmd_str, 63, "i2cset -y -f 11 0x60 0x9 0xf");
    system(cmd_str);

    snprintf(cmd_str, 63, "ifconfig eth1 up");
    system(cmd_str);
    snprintf(cmd_str, 63, "ifconfig eth2 up");
    system(cmd_str);
    return 0;
}

#define FAN_DUTY_MAX  (100)
#define FAN_DUTY_DEF  (50)
#define FAN_DUTY_MIN  (33)

static int
sysi_fanctrl_fan_fault_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                              onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                              int *adjusted)
{
    int i;
    *adjusted = 0;

    /* Bring fan speed to FAN_DUTY_MAX if any fan is not operational */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        if (!(fi[i].status & ONLP_FAN_STATUS_FAILED)) {
            continue;
        }

        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
    }

    return ONLP_STATUS_OK;
}

static int
sysi_fanctrl_fan_absent_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                               onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                               int *adjusted)
{
    int i;
    *adjusted = 0;

    /* Bring fan speed to FAN_DUTY_MAX if fan is not present */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        if (fi[i].status & ONLP_FAN_STATUS_PRESENT) {
            continue;
        }

        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
    }

    return ONLP_STATUS_OK;
}

static int
sysi_fanctrl_fan_unknown_speed_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                      onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                      int *adjusted)
{
    *adjusted = 1;
    return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_DEF);
}

#if 0
static int
sysi_fanctrl_overall_thermal_sensor_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
        onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
        int *adjusted)
{
    int i, num_of_sensor = 0, temp_avg = 0;

    for (i = (THERMAL_1_ON_MAINBOARD); i <= (THERMAL_6_ON_MAINBOARD); i++) {
        num_of_sensor++;
        temp_avg += ti[i-1].mcelsius;
    }

    temp_avg /= num_of_sensor;
    *adjusted = 1;

    if (temp_avg > 57000) {
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
    }
    else if (temp_avg > 52000) {
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), 88);
    }
    else if (temp_avg > 46000) {
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), 76);
    }
    else if (temp_avg > 43000) {
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), 64);
    }

    return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MIN);
}
#endif

typedef int (*fan_control_policy)(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                  onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                  int *adjusted);

fan_control_policy fan_control_policies[] = {
    sysi_fanctrl_fan_fault_policy,
    sysi_fanctrl_fan_absent_policy,
    sysi_fanctrl_fan_unknown_speed_policy,
    //sysi_fanctrl_overall_thermal_sensor_policy,
};

int
onlp_sysi_platform_manage_fans(void)
{
    int i, rc;
    onlp_fan_info_t fi[CHASSIS_FAN_COUNT];
    onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT];

    memset(fi, 0, sizeof(fi));
    memset(ti, 0, sizeof(ti));


    /* Get fan status
     */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++)
    {
        rc = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i+1), &fi[i]);
        if (rc != ONLP_STATUS_OK)
        {
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
            AIM_LOG_ERROR("Unable to get fan(%d) status\r\n", i+1);
            return ONLP_STATUS_E_INTERNAL;
        }
        if (fi[i].status & ONLP_FAN_STATUS_FAILED || !(fi[i].status & ONLP_FAN_STATUS_PRESENT))
        {
            printf("FAN-%d present fail, set to MAX\n", i+1);
            AIM_LOG_ERROR("Fan(%d) is not working, set the other fans as full speed\r\n", i);
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
            break;
        }
    }

    /* Apply thermal policy according the policy list,
     * If fan duty is adjusted by one of the policies, skip the others
     */
    for (i = 0; i < AIM_ARRAYSIZE(fan_control_policies); i++) {
        int adjusted = 0;

        rc = fan_control_policies[i](fi, ti, &adjusted);
        if (!adjusted) {
            continue;
        }
        return rc;
    }
    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    int i = 0, fan_fault = 0;

    /* Get each fan status
     */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
    {
        onlp_fan_info_t fan_info;

        if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to get fan(%d) status\r\n", i);
            return ONLP_STATUS_E_INTERNAL;
        }

        if ((!(fan_info.status & ONLP_FAN_STATUS_PRESENT)) | (fan_info.status & ONLP_FAN_STATUS_FAILED)) {
            AIM_LOG_ERROR("Fan(%d) is not present or not working\r\n", i);
            fan_fault = fan_fault + 1;
        }
    }


    if (fan_fault > 1) {
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN), ONLP_LED_MODE_RED);
    } else if (fan_fault == 1) {
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN), ONLP_LED_MODE_YELLOW);
    } else {
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN), ONLP_LED_MODE_GREEN);
    }

    /* Get each psu status
     */
    int led_psu_idx[] = {LED_PSU1, LED_PSU2};
    for (i = 0; i < AIM_ARRAYSIZE(led_psu_idx); i++)
    {
        onlp_psu_info_t psu_info;
        int idx = led_psu_idx[i];
        if (onlp_psui_info_get(ONLP_PSU_ID_CREATE(idx), &psu_info) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to get psu(%d) status\r\n", i+1);
            return ONLP_STATUS_E_INTERNAL;
        }
        if ((!(psu_info.status & ONLP_PSU_STATUS_PRESENT)) | (psu_info.status & ONLP_PSU_STATUS_FAILED)) {
            AIM_LOG_ERROR("Psu(%d) is not present or not working\r\n", i+1);
            onlp_ledi_mode_set(ONLP_LED_ID_CREATE(idx), ONLP_LED_MODE_RED);
        } else {
            onlp_ledi_mode_set(ONLP_LED_ID_CREATE(idx), ONLP_LED_MODE_GREEN);
        }
    }
    return ONLP_STATUS_OK;
}

