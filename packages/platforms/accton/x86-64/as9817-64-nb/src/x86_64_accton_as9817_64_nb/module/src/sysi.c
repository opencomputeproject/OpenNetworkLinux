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

#include "x86_64_accton_as9817_64_nb_int.h"
#include "x86_64_accton_as9817_64_nb_log.h"

#define NUM_OF_CPLD_VER 4
#define NUM_OF_QSFP_PORT 64

int onlp_sysi_get_cpu_temp(int *temp);
int onlp_sysi_get_mac_temp(int *temp);
int onlp_sysi_get_xcvr_temp(int *temp);
int onlp_sysi_over_temp_protector(void);
int onlp_sysi_set_fan_duty_all(int duty);
int onlp_sysi_get_fan_status(void);

enum fan_duty_level {
    FAN_DUTY_MIN = 30,
    FAN_DUTY_MID = 60,
    FAN_DUTY_MAX = 100
};

enum temp_sensors {
    TEMP_SENSOR_CPU = 0,
    TEMP_SENSOR_MAC,
    TEMP_SENSOR_XCVR,
    TEMP_SENSOR_COUNT
};

typedef struct temp_threshold {
    int idle;
    int up_adjust;
    int down_adjust;
    int otp;
} temp_threshold_t;

typedef int (*temp_getter_t)(int *temp);
typedef int (*fan_pwm_setter_t)(int pwm);
typedef int (*fan_status_getter_t)(void);
typedef int (*ot_protector_t)(void);

typedef struct temp_handler {
    temp_getter_t    temp_readers[TEMP_SENSOR_COUNT];
    temp_threshold_t thresholds[TEMP_SENSOR_COUNT];
} temp_handler_t;

typedef struct fan_handler {
    fan_pwm_setter_t    pwm_writer;
    fan_status_getter_t status_reader;
} fan_handler_t;

/* over temp protection */
typedef struct otp_handler {
    ot_protector_t  otp_writer;
} otp_handler_t;

struct thermal_policy_manager {
    temp_handler_t  temp_hdlr;
    fan_handler_t   fan_hdlr;
    otp_handler_t   otp_hdlr; /* over temp protector */
};

struct thermal_policy_manager tp_mgr = {
    .temp_hdlr = {
        .thresholds = {
            [TEMP_SENSOR_CPU]  = { .idle = 60000, .up_adjust = 85000, .down_adjust = 75000, .otp = 100000 },
            [TEMP_SENSOR_MAC]  = { .idle = 60000, .up_adjust = 90000, .down_adjust = 80000, .otp = 105000 },
            [TEMP_SENSOR_XCVR] = { .idle = ONLP_STATUS_E_MISSING, .up_adjust = 75000, .down_adjust = 65000 }
        },
        .temp_readers = {
            [TEMP_SENSOR_CPU] = onlp_sysi_get_cpu_temp,
            [TEMP_SENSOR_MAC] = onlp_sysi_get_mac_temp,
            [TEMP_SENSOR_XCVR] = onlp_sysi_get_xcvr_temp
        }
    },
    .fan_hdlr = {
        .pwm_writer = onlp_sysi_set_fan_duty_all,
        .status_reader = onlp_sysi_get_fan_status
    },
    .otp_hdlr = {
        .otp_writer = onlp_sysi_over_temp_protector
    }
};

static char* cpld_ver_path[NUM_OF_CPLD_VER] = {
    "/sys/bus/i2c/devices/0-0060/version",           /* FPGA */
    "/sys/devices/platform/as9817_64_fpga/cpld1_version", /* CPLD-1 */
    "/sys/devices/platform/as9817_64_fpga/cpld2_version", /* CPLD-2 */
    "/sys/bus/i2c/devices/76-0033/hwmon/hwmon*version"    /* Fan CPLD */
};

const char*
onlp_sysi_platform_get(void)
{
    as9817_64_platform_id_t pid = get_platform_id();

    switch (pid) {
        case AS9817_64O: return "x86-64-accton-as9817-64o-nb-r0";
        case AS9817_64D: return "x86-64-accton-as9817-64d-nb-r0";
        default: break;
    }

    return "Unknown Platform";
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

    /* 9 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 6 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }


    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 8 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int i, len, ret = ONLP_STATUS_OK;
    char *v[NUM_OF_CPLD_VER] = {NULL};

    for (i = 0; i < AIM_ARRAYSIZE(cpld_ver_path); i++) {
        if (i == 3) {
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
        pi->cpld_versions = aim_fstrdup("\r\nFPGA:%s\r\nCPLD-1:%s"
                                        "\r\nCPLD-2:%s\r\nFan CPLD:%s",
                                        v[0], v[1], v[2], v[3]);
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

int onlp_sysi_get_cpu_temp(int *temp)
{
    int ret;
    onlp_thermal_info_t ti;

    ret = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), &ti);
    if (ret != ONLP_STATUS_OK) {
        return ret;
    }

    *temp = ti.mcelsius;
    return ONLP_STATUS_OK;
}

int onlp_sysi_get_mac_temp(int *temp)
{
    int ret;
    char* file = NULL;

    ret = onlp_file_find("/run/mac/", "temp1_input", &file);
    AIM_FREE_IF_PTR(file);

    if (ONLP_STATUS_OK != ret) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_OK;
    }

    ret = onlp_file_read_int(temp, "/run/mac/temp1_input");
    if (ONLP_STATUS_OK != ret) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_OK;
    }

    return ONLP_STATUS_OK;
}

int onlp_sysi_get_xcvr_presence(void)
{
    onlp_sfp_bitmap_t bitmap;
    onlp_sfp_bitmap_t_init(&bitmap);
    onlp_sfp_presence_bitmap_get(&bitmap);

    /* Ignore SFP */
    AIM_BITMAP_CLR(&bitmap, 65);
    AIM_BITMAP_CLR(&bitmap, 66);
    return !(AIM_BITMAP_COUNT(&bitmap) == 0);
}

int onlp_sysi_get_sff8436_temp(int port, int *temp)
{
    int value;
    int16_t port_temp;

    /* Read memory model */
    value = onlp_sfpi_dev_readb(port, 0x50, 0x2);
    if (value & 0x04) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_OK;
    }

    value = onlp_sfpi_dev_readb(port, 0x50, 22);
    if (value < 0) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_OK;
    }
    port_temp = (int16_t)((value & 0xFF) << 8);

    value = onlp_sfpi_dev_readb(port, 0x50, 23);
    if (value < 0) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_OK;
    }
    port_temp = (port_temp | (int16_t)(value & 0xFF));

    *temp = (int)port_temp * 1000 / 256;
    return ONLP_STATUS_OK;
}

int onlp_sysi_get_cmis_temp(int port, int *temp)
{
    int value;
    int16_t port_temp;

    /* Read memory model */
    value = onlp_sfpi_dev_readb(port, 0x50, 0x2);
    if (value & 0x80) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_OK;
    }

    value = onlp_sfpi_dev_readb(port, 0x50, 14);
    if (value < 0) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_OK;
    }
    port_temp = (int16_t)((value & 0xFF) << 8);

    value = onlp_sfpi_dev_readb(port, 0x50, 15);
    if (value < 0) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_OK;
    }
    port_temp = (port_temp | (int16_t)(value & 0xFF));

    *temp = (int)port_temp * 1000 / 256;
    return ONLP_STATUS_OK;
}

int onlp_sysi_get_xcvr_temp(int *temp)
{
    int ret = ONLP_STATUS_OK;
    int value, port;
    int port_temp = ONLP_STATUS_E_MISSING, max_temp = ONLP_STATUS_E_MISSING;

    *temp = ONLP_STATUS_E_MISSING;

    if (!onlp_sysi_get_xcvr_presence()) {
        return ONLP_STATUS_OK;
    }

    for (port = 1; port <= NUM_OF_QSFP_PORT; port++) {
        if (!onlp_sfpi_is_present(port)) {
            continue;
        }

        value = onlp_sfpi_dev_readb(port, 0x50, 0);
        if (value < 0) {
            AIM_LOG_ERROR("Unable to get read port(%d) eeprom\r\n", port);
            continue;
        }

        if (value == 0x18 || value == 0x19 || value == 0x1E) {
            ret = onlp_sysi_get_cmis_temp(port, &port_temp);
            if (ret != ONLP_STATUS_OK) {
                continue;
            }
        }
        else if (value == 0x0C || value == 0x0D || value == 0x11 || value ==  0xE1) {
            ret = onlp_sysi_get_sff8436_temp(port, &port_temp);
            if (ret != ONLP_STATUS_OK) {
                continue;
            }
        }
        else {
            continue;
        }

        if (port_temp > max_temp) {
            max_temp = port_temp;
        }
    }

    *temp = max_temp;
    return ONLP_STATUS_OK;
}

int onlp_sysi_reset_front_port(void)
{
    int port, ret;

    for (port = 1; port <= NUM_OF_QSFP_PORT; port++) {
        if (ONLP_STATUS_OK != onlp_sfpi_control_set(port, ONLP_SFP_CONTROL_RESET_STATE, 1)) {
            ret = ONLP_STATUS_E_INTERNAL;
        }
    }

    return ret;
}

int onlp_sysi_over_temp_protector(void)
{
    int pid;
    AIM_SYSLOG_CRIT("Temperature critical", "Temperature critical",
                    "Alarm for temperature critical is detected; performing OTP protect action!");
    system("sync;sync;sync");

    for (pid = 1; pid <= CHASSIS_PSU_COUNT; pid++) {
        psu_pmbus_info_set(pid, "psu_power_cycle", 1);
    }

    system("reboot");
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
    int i, ret;
    int temp[TEMP_SENSOR_COUNT] = {0};
    static int fan_duty = 60;

    /* Get fan status
     * Bring fan speed to FAN_DUTY_MAX if any fan is not present or operational
     */
    if (tp_mgr.fan_hdlr.status_reader() != ONLP_STATUS_OK) {
        fan_duty = FAN_DUTY_MAX;
        tp_mgr.fan_hdlr.pwm_writer(fan_duty);
        return ONLP_STATUS_E_INTERNAL;
    }

    for (i = 0; i < AIM_ARRAYSIZE(temp); i++) {
        ret = tp_mgr.temp_hdlr.temp_readers[i](&temp[i]);
        if (ret != ONLP_STATUS_OK) {
            fan_duty = FAN_DUTY_MAX;
            tp_mgr.fan_hdlr.pwm_writer(fan_duty);
            return ret;
        }
    }

    /* Adjust fan pwm based on current temperature status */
    if (!onlp_sysi_get_xcvr_presence() &&
        temp[TEMP_SENSOR_CPU] < tp_mgr.temp_hdlr.thresholds[TEMP_SENSOR_CPU].idle &&
        temp[TEMP_SENSOR_MAC] < tp_mgr.temp_hdlr.thresholds[TEMP_SENSOR_MAC].idle) {
        fan_duty = FAN_DUTY_MIN;
    }
    else if (temp[TEMP_SENSOR_CPU] > tp_mgr.temp_hdlr.thresholds[TEMP_SENSOR_CPU].up_adjust ||
             temp[TEMP_SENSOR_MAC] > tp_mgr.temp_hdlr.thresholds[TEMP_SENSOR_MAC].up_adjust ||
             temp[TEMP_SENSOR_XCVR] > tp_mgr.temp_hdlr.thresholds[TEMP_SENSOR_XCVR].up_adjust) {
        fan_duty = FAN_DUTY_MAX;
    }
    else if (temp[TEMP_SENSOR_CPU] < tp_mgr.temp_hdlr.thresholds[TEMP_SENSOR_CPU].down_adjust &&
             temp[TEMP_SENSOR_MAC] < tp_mgr.temp_hdlr.thresholds[TEMP_SENSOR_MAC].down_adjust &&
             temp[TEMP_SENSOR_XCVR] < tp_mgr.temp_hdlr.thresholds[TEMP_SENSOR_XCVR].down_adjust) {
        fan_duty = FAN_DUTY_MID;
    }

    tp_mgr.fan_hdlr.pwm_writer(fan_duty);

    /* Handle over temp condition */
    if (temp[TEMP_SENSOR_CPU] > tp_mgr.temp_hdlr.thresholds[TEMP_SENSOR_CPU].otp ||
        temp[TEMP_SENSOR_MAC] > tp_mgr.temp_hdlr.thresholds[TEMP_SENSOR_MAC].otp) {
        tp_mgr.otp_hdlr.otp_writer();
    }

    return ONLP_STATUS_OK;
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
            fan_led = ONLP_LED_MODE_RED;
            break;
        }

        if (!(fan_info.status & ONLP_FAN_STATUS_PRESENT)) {
            AIM_LOG_ERROR("Fan(%d) is not present\r\n", i);
            fan_led = ONLP_LED_MODE_RED;
            break;
        }

        if (fan_info.status & ONLP_FAN_STATUS_FAILED) {
            AIM_LOG_ERROR("Fan(%d) is not working\r\n", i);
            fan_led = ONLP_LED_MODE_RED;
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
            psu_led[i] = ONLP_LED_MODE_RED;
            continue;
        }

        if (!(psu_info.status & ONLP_PSU_STATUS_PRESENT)) {
            AIM_LOG_ERROR("Psu(%d) is not present\r\n", i);
            psu_led[i] = ONLP_LED_MODE_OFF;
            continue;
        }

        if (psu_info.status & ONLP_PSU_STATUS_FAILED) {
            AIM_LOG_ERROR("Psu(%d) is not working\r\n", i);
            psu_led[i] = ONLP_LED_MODE_RED;
            continue;
        }
    }

    for (i = 0; i < CHASSIS_PSU_COUNT; i++) {
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PSU1 + i), psu_led[i]);
    }

    return ONLP_STATUS_OK; 
}
