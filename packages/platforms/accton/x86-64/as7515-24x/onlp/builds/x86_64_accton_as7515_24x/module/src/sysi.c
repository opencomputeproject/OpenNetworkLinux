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
#include <onlp/platformi/sfpi.h>
#include "platform_lib.h"

#include "x86_64_accton_as7515_24x_int.h"
#include "x86_64_accton_as7515_24x_log.h"

#define NUM_OF_CPLD_VER 3
#define THERMAL_POLICY_SENSOR_COUNT 5

int onlp_sysi_set_fan_duty_all(int duty);
int onlp_sysi_get_fan_status(void);
int onlp_sysi_get_thermal_temp(int tid, int *temp);

enum fan_duty_level {
    FAN_DUTY_MIN = 40,
    FAN_DUTY_MID = 70,
    FAN_DUTY_MAX = 100
};

typedef int (*temp_getter_t)(int tid, int *temp);
typedef int (*fan_pwm_setter_t)(int pwm);
typedef int (*fan_status_getter_t)(void);

typedef struct {
    int id;
    int tlow_1; /* In mini-Celsius */
    int tup_1;  /* In mini-Celsius */
    int tlow_2; /* In mini-Celsius */
    int tup_2;  /* In mini-Celsius */
} thermal_policy_t;

typedef struct temp_handler {
    temp_getter_t temp_reader;
} temp_handler_t;

typedef struct fan_handler {
    fan_pwm_setter_t    pwm_writer;
    fan_status_getter_t status_reader;
} fan_handler_t;

struct thermal_policy_manager {
    temp_getter_t    temp_reader;
    fan_handler_t    fan_hdlr;
    thermal_policy_t policy[THERMAL_POLICY_SENSOR_COUNT];
    const int duty_default;
    const int duty_on_fail;
};

static struct thermal_policy_manager tp_mgr = {
    .temp_reader = onlp_sysi_get_thermal_temp,
    .fan_hdlr = {
        .pwm_writer = onlp_sysi_set_fan_duty_all,
        .status_reader = onlp_sysi_get_fan_status
    },
    .duty_on_fail = FAN_DUTY_MAX,
    .duty_default = FAN_DUTY_MIN,
    .policy = {
        [0] = { THERMAL_1_ON_MAIN_BROAD, 47000, 52000, 56000, 61000 }, /* MB_FrontCenter_temp(0x49) */
        [1] = { THERMAL_2_ON_MAIN_BROAD, 45000, 50000, 54000, 59000 }, /* MB_FrontLeft_temp(0x4A) */
        [2] = { THERMAL_3_ON_MAIN_BROAD, 53000, 58000, 61000, 66000 }, /* MB_FrontCenter_temp(0x4C) */
        [3] = { THERMAL_4_ON_MAIN_BROAD, 43000, 48000, 53000, 58000 }, /* MB_FrontRight_temp(0x4D) */
        [4] = { THERMAL_6_ON_MAIN_BROAD, 70000, 75000, 80000, 85000 }  /* MB_RearLeft_temp(0x4C) */
    }
};

static char* cpld_ver_path[NUM_OF_CPLD_VER] = {
    "/sys/devices/platform/as7515_24x_fpga/version", /* FPGA */
    "/sys/bus/i2c/devices/2-0061/version", /* CPLD */
    "/sys/bus/i2c/devices/8-0066/hwmon/hwmon*version" /* Fan CPLD */
};

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as7515-24x-r0";
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);
    if (onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK) {
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

    /* 7 Thermal sensors on the chassis */
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

    /* 5 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int i, len, ret = ONLP_STATUS_OK;
    char *v[NUM_OF_CPLD_VER] = { NULL };

    for (i = 0; i < AIM_ARRAYSIZE(cpld_ver_path); i++) {
        if (i == 2) {
            int hwmon_idx = onlp_get_fan_hwmon_idx();

            if (hwmon_idx < 0) {
                ret = ONLP_STATUS_E_INTERNAL;
                break;
            }

            len = onlp_file_read_str(&v[i], FAN_SYSFS_FORMAT_1, hwmon_idx, "version");
        }
        else {
            len = onlp_file_read_str(&v[i], cpld_ver_path[i]);
        }

        if (v[i] == NULL || len <= 0) {
            ret = ONLP_STATUS_E_INTERNAL;
            break;
        }
    }

    if (ret == ONLP_STATUS_OK) {
        pi->cpld_versions = aim_fstrdup("\r\nFPGA:%s\r\nCPLD:%s"
                                        "\r\nFan CPLD:%s", v[0], v[1], v[2]);
    }

    for (i = 0; i < AIM_ARRAYSIZE(v); i++) {
        AIM_FREE_IF_PTR(v[i]);
    }

    return ret;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

void
onlp_sysi_onie_data_free(uint8_t* data)
{
    aim_free(data);
}

int
onlp_sysi_platform_manage_leds(void)
{
    int i, ret = ONLP_STATUS_OK;
    int fan_led = ONLP_LED_MODE_GREEN;
    int psu_led[CHASSIS_PSU_COUNT] = { ONLP_LED_MODE_GREEN, ONLP_LED_MODE_GREEN };

    /* Get each fan status
     */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
    {
        onlp_fan_info_t fan_info;

        ret = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info);
        if (ret != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to get fan(%d) status\r\n", i);
            fan_led = ONLP_LED_MODE_ORANGE;
            break;
        }

        if (!(fan_info.status & ONLP_FAN_STATUS_PRESENT)) {
            fan_led = ONLP_LED_MODE_OFF;
            break;
        }

        if (fan_info.status & ONLP_FAN_STATUS_FAILED) {
            fan_led = ONLP_LED_MODE_ORANGE;
            break;
        }
    }

    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN), fan_led);

    /* Get each psu status
     */
    for (i = 0; i < CHASSIS_PSU_COUNT; i++) {
        onlp_psu_info_t psu_info;

        if (onlp_psui_info_get(ONLP_PSU_ID_CREATE(PSU1_ID + i), &psu_info) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to get psu(%d) status\r\n", i);
            psu_led[i] = ONLP_LED_MODE_ORANGE;
            continue;
        }

        if (!(psu_info.status & ONLP_PSU_STATUS_PRESENT)) {
            psu_led[i] = ONLP_LED_MODE_OFF;
            continue;
        }

        if (psu_info.status & ONLP_PSU_STATUS_FAILED) {
            psu_led[i] = ONLP_LED_MODE_ORANGE;
            continue;
        }
    }

    for (i = 0; i < CHASSIS_PSU_COUNT; i++) {
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PSU1 + i), psu_led[i]);
    }

    return ONLP_STATUS_OK; 
}

int onlp_sysi_get_thermal_temp(int tid, int *temp)
{
    int ret;
    onlp_thermal_info_t ti;

    ret = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(tid), &ti);
    if (ret != ONLP_STATUS_OK)
        return ret;

    *temp = ti.mcelsius;
    return ONLP_STATUS_OK;
}

int onlp_sysi_set_fan_duty_all(int duty)
{
    int fid, ret = ONLP_STATUS_OK;

    for (fid = 1; fid <= CHASSIS_FAN_COUNT; fid++) {
        if (ONLP_STATUS_OK != onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(fid), duty)) {
            ret = ONLP_STATUS_E_INTERNAL;
        }
    }

    return ret;
}

int onlp_sysi_get_fan_status(void)
{
    int i, ret;
    onlp_fan_info_t fi[CHASSIS_FAN_COUNT];
    memset(fi, 0, sizeof(fi));

    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        ret = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i+1), &fi[i]);
        if (ret != ONLP_STATUS_OK) {
			AIM_LOG_ERROR("Unable to get fan(%d) status\r\n", i+1);
            return ONLP_STATUS_E_INTERNAL;
        }

        if (!(fi[i].status & ONLP_FAN_STATUS_PRESENT)) {
            AIM_LOG_ERROR("Fan(%d) is NOT present\r\n", i+1);
            return ONLP_STATUS_E_INTERNAL;
        }

        if (fi[i].status & ONLP_FAN_STATUS_FAILED) {
            AIM_LOG_ERROR("Fan(%d) is NOT operational\r\n", i+1);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_manage_fans(void)
{
    int i, ret, duty_found = 0;
    int all_below_tlow_1 = 1;
    int all_below_tlow_2 = 1;
    int any_above_tup_1 = 0;
    int any_above_tup_2 = 0;
    static int fan_duty = FAN_DUTY_MIN;
    int temperature[THERMAL_POLICY_SENSOR_COUNT] = { 0 };

    /* Get fan status
     * Bring fan speed to FAN_DUTY_MAX if any fan is not present or operational
     */
    if (tp_mgr.fan_hdlr.status_reader() != ONLP_STATUS_OK) {
        fan_duty = tp_mgr.duty_on_fail;
        tp_mgr.fan_hdlr.pwm_writer(fan_duty);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Get thermal status
     * Bring fan speed to FAN_DUTY_MAX if read any thermal failed
     */
    for (i = 0; i < AIM_ARRAYSIZE(tp_mgr.policy); i++) {
        ret = tp_mgr.temp_reader(tp_mgr.policy[i].id, &temperature[i]);
        if (ret != ONLP_STATUS_OK) {
            fan_duty = tp_mgr.duty_on_fail;
            tp_mgr.fan_hdlr.pwm_writer(fan_duty);
            return ret;
        }
    }

    /* Apply thermal policy */
    for (i = 0; i < AIM_ARRAYSIZE(tp_mgr.policy); i++) {
        any_above_tup_1 = (temperature[i] > tp_mgr.policy[i].tup_1) ? 1 : any_above_tup_1;
        any_above_tup_2 = (temperature[i] > tp_mgr.policy[i].tup_2) ? 1 : any_above_tup_2;
        all_below_tlow_1 = (temperature[i] >= tp_mgr.policy[i].tlow_1) ? 0 : all_below_tlow_1;
        all_below_tlow_2 = (temperature[i] >= tp_mgr.policy[i].tlow_2) ? 0 : all_below_tlow_2;
    }

    switch (fan_duty) {
    case FAN_DUTY_MIN:
        duty_found = 1;
        fan_duty = any_above_tup_1 ? FAN_DUTY_MID : FAN_DUTY_MIN;
        break;
    case FAN_DUTY_MID:
        duty_found = 1;
        fan_duty = any_above_tup_2 ? FAN_DUTY_MAX :
                       (all_below_tlow_1 ? FAN_DUTY_MIN : FAN_DUTY_MID);
        break;
    case FAN_DUTY_MAX:
        duty_found = 1;
        fan_duty = all_below_tlow_2 ? FAN_DUTY_MID : FAN_DUTY_MAX;
        break;
    default:
        duty_found = 0;
        break;
    }

    if (!duty_found) {
        fan_duty = tp_mgr.duty_default;
    }

    tp_mgr.fan_hdlr.pwm_writer(fan_duty);
    return ONLP_STATUS_OK;
}
