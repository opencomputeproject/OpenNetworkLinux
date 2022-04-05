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
#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#include "x86_64_accton_as7315_30x_int.h"
#include "x86_64_accton_as7315_30x_log.h"

#define NUM_OF_CPLD_VER 6

static char* cpld_ver_path[] = {
    "/sys/bus/i2c/devices/39-0064/pcb_version", /* Main FPGA */
    "/sys/bus/i2c/devices/39-0064/version",     /* Main FPGA */
    "/sys/bus/i2c/devices/40-0063/version",     /* Main CPLD */
    "/sys/bus/i2c/devices/40-0063/sub_version", /* Main CPLD */
    "/sys/bus/i2c/devices/41-0066/version",     /* Fan  CPLD */
    "/sys/bus/i2c/devices/41-0066/sub_version", /* Fan  CPLD */
};

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as7315-30x-r0";
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

    /* 8 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 5 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 6 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int   i, v[NUM_OF_CPLD_VER] = {0};

    for (i = 0; i < AIM_ARRAYSIZE(cpld_ver_path); i++) {
        v[i] = 0;

        if(onlp_file_read_int(v+i, cpld_ver_path[i]) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    pi->cpld_versions = aim_fstrdup("\r\nPCB:%d\r\nMain FPGA:%d\r\nMain CPLD:%d.%d\r\nFan CPLD:%d.%d",
                                     v[0], v[1], v[2], v[3], v[4], v[5]);

    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

int onlp_sysi_platform_manage_leds(void)
{
    int i, rc;
    onlp_fan_info_t fi[CHASSIS_FAN_COUNT];

    memset(fi, 0, sizeof(fi));

    /* Get fan status
     */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        rc = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i+1), &fi[i]);

        if (rc != ONLP_STATUS_OK) {
            return ONLP_STATUS_E_INTERNAL;
        }

        if (!(fi[i].status & ONLP_FAN_STATUS_PRESENT)) {
            /* Set System-Fan LED as orange if any fan is not present
            */
           return onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN), ONLP_LED_MODE_ORANGE);
        }

        if (fi[i].status & ONLP_FAN_STATUS_FAILED) {
            /* Set System-Fan LED as orange if any fan failed
            */
            return onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN), ONLP_LED_MODE_ORANGE);
        }
    }

    return onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN), ONLP_LED_MODE_GREEN);
}

enum fan_duty {
    FAN_DUTY_MAX = 100,    /* high speed */
    FAN_DUTY_MID = 60,     /* middle speed */
    FAN_DUTY_NOR = 30,     /* normal speed */
    FAN_DUTY_LOW_1_3_5 = 0, /* low speed for fan1/fan3/fan5 */
    FAN_DUTY_LOW_2_4 = 40   /* low speed for fan2 and fan4 */
};

struct thermal_threshold_t {
    int Tc;
    int Tm;
    int Tn;
    int Tb;
    int Ta;
    int T0;
    int Te;
};

struct thermal_threshold_t thresholds[FAN_DIR_COUNT] = {
    [FAN_DIR_L2R].Tc = 76000,
    [FAN_DIR_L2R].Tm = 58000,
    [FAN_DIR_L2R].Tn = 52000,
    [FAN_DIR_L2R].Tb = 46000,
    [FAN_DIR_L2R].Ta = 38000,
    [FAN_DIR_L2R].T0 = 17000,
    [FAN_DIR_L2R].Te = 6000,

    [FAN_DIR_R2L].Tc = 74000,
    [FAN_DIR_R2L].Tm = 56000,
    [FAN_DIR_R2L].Tn = 51000,
    [FAN_DIR_R2L].Tb = 43000,
    [FAN_DIR_R2L].Ta = 36000,
    [FAN_DIR_R2L].T0 = 11000,
    [FAN_DIR_R2L].Te = 5000
};

static int
sysi_fanctrl_fan_set_duty(int p)
{
    int i;
    int status = 0;
    int duties_nor[CHASSIS_FAN_COUNT] = { [0 ... CHASSIS_FAN_COUNT-1] = p };
    int duties_low[CHASSIS_FAN_COUNT] = { FAN_DUTY_LOW_1_3_5, FAN_DUTY_LOW_2_4,
                                          FAN_DUTY_LOW_1_3_5, FAN_DUTY_LOW_2_4,
                                          FAN_DUTY_LOW_1_3_5 };
    int *duties = (p == FAN_DUTY_LOW_1_3_5) ? duties_low : duties_nor;

    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        int ret = 0;

        ret = onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), duties[i-1]);
        if (ret < 0) {
            status = ret;
        }
    }

    return status;
}

static int
sysi_fanctrl_fan_status_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
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
        return sysi_fanctrl_fan_set_duty(FAN_DUTY_MAX);
    }

    /* Bring fan speed to FAN_DUTY_MAX if fan is not present */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        if (fi[i].status & ONLP_FAN_STATUS_PRESENT) {
            continue;
        }

        *adjusted = 1;
        return sysi_fanctrl_fan_set_duty(FAN_DUTY_MAX);
    }

    return ONLP_STATUS_OK;
}

static int
sysi_fanctrl_thermal_status_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                   onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                   int *adjusted)
{
    int rc;
    int fanduty;
    int Tsensor = ti[THERMAL_2_ON_MAIN_BROAD-1].mcelsius;
    enum onlp_fan_dir dir;

    *adjusted = 0;

    rc = onlp_file_read_int(&fanduty, "%s%s", FAN_BOARD_PATH, "fan1_pwm");
    if (rc < 0) {
        *adjusted = 1;
        AIM_LOG_ERROR("Unable to get fan pwm, ret:%d\r\n", rc);
        return sysi_fanctrl_fan_set_duty(FAN_DUTY_MAX);
    }

    dir = onlp_get_fan_dir();

    switch (fanduty) {
    case FAN_DUTY_MAX:
    {
        if (Tsensor >= thresholds[dir].Tc) { /* reboot threshold */
            AIM_SYSLOG_CRIT("Temperature critical", "Temperature critical", "Alarm for temperature critical is detected, reboot DUT");
            system("sync;sync;sync");
            system("reboot");
        } else if (Tsensor < thresholds[dir].Tn) { /* down adjust threshold */
            *adjusted = 1;
            return sysi_fanctrl_fan_set_duty(FAN_DUTY_MID);
        }

        break;
    }
    case FAN_DUTY_MID:
    {
        if (Tsensor > thresholds[dir].Tm) { /* up adjust threshold */
            *adjusted = 1;
            return sysi_fanctrl_fan_set_duty(FAN_DUTY_MAX);
        } else if (Tsensor < thresholds[dir].Ta) { /* down adjust threshold */
            *adjusted = 1;
            return sysi_fanctrl_fan_set_duty(FAN_DUTY_NOR);
        }

        break;
    }
    case FAN_DUTY_NOR:
    {
        if (Tsensor > thresholds[dir].Tb) { /* up adjust threshold */
            *adjusted = 1;
            return sysi_fanctrl_fan_set_duty(FAN_DUTY_MID);
        } else if (Tsensor < thresholds[dir].Te) { /* down adjust threshold */
            *adjusted = 1;
            return sysi_fanctrl_fan_set_duty(FAN_DUTY_LOW_1_3_5);
        }

        break;
    }
    case FAN_DUTY_LOW_1_3_5:
    {
        if (Tsensor > thresholds[dir].T0) { /* up adjust threshold */
            *adjusted = 1;
            return sysi_fanctrl_fan_set_duty(FAN_DUTY_NOR);
        }

        break;
    }
    default:
        *adjusted = 1;
        return sysi_fanctrl_fan_set_duty(FAN_DUTY_NOR);
    }

    return sysi_fanctrl_fan_set_duty(fanduty);
}

typedef int (*fan_control_policy)(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                  onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                  int *adjusted);

fan_control_policy fan_control_policies[] = {
    sysi_fanctrl_fan_status_policy,
    sysi_fanctrl_thermal_status_policy
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
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        rc = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i+1), &fi[i]);

        if (rc != ONLP_STATUS_OK) {
            sysi_fanctrl_fan_set_duty(FAN_DUTY_MAX);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    /* Get thermal sensor status
     */
    for (i = 0; i < CHASSIS_THERMAL_COUNT; i++) {
        rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i+1), &ti[i]);

        if (rc != ONLP_STATUS_OK) {
            sysi_fanctrl_fan_set_duty(FAN_DUTY_MAX);
            return ONLP_STATUS_E_INTERNAL;
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
