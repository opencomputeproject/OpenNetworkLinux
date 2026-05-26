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
#include <onlp/platformi/sfpi.h>
#include "platform_lib.h"

#include "x86_64_accton_as7927_50x_int.h"
#include "x86_64_accton_as7927_50x_log.h"

#define NUM_OF_CPLD_VER          7
#define BMC_FILE_RETRY_COUNT     3    // Retry count for file read/write operations
#define BMC_FILE_RETRY_DELAY_S   1    // Delay between retries (in seconds, 1s)

#define MODULE_EFUSE_FORMAT        "/sys/bus/platform/devices/as7927_50x_fpga/module_efuse_%d"

int xcvr_over_temp_protector(int port);

enum temp_sensors {
    TEMP_SENSOR_XCVR,
    TEMP_SENSOR_COUNT
};

typedef int (*temp_getter_t)(temp_reader_data_t *temp);
typedef int (*ot_protector_t)(int port);

typedef struct temp_handler {
    temp_getter_t    temp_readers[TEMP_SENSOR_COUNT];
} temp_handler_t;

/* over temp protection */
typedef struct otp_handler {
    ot_protector_t  otp_writer;
} otp_handler_t;

struct thermal_policy_manager {
    temp_handler_t  temp_hdlr;
    otp_handler_t   otp_hdlr; /* over temp protector */
};

struct thermal_policy_manager tp_mgr = {
    .temp_hdlr = {
        .temp_readers = {
            [TEMP_SENSOR_XCVR] = get_xcvr_temp
        }
    },
    .otp_hdlr = {
        .otp_writer = xcvr_over_temp_protector
    }
};

static char* cpld_ver_path[NUM_OF_CPLD_VER] = {
    "/sys/bus/platform/devices/as7927_50x_sys/come_e_cpld_ver", /* CPU CPLD */
    "/sys/bus/platform/devices/as7927_50x_sys/sys_cpld_ver", /* System CPLD */
    "/sys/bus/platform/devices/as7927_50x_fpga/cpld1_version", /* Port CPLD1 */
    "/sys/bus/platform/devices/as7927_50x_fpga/cpld2_version", /* Port CPLD2 */
    "/sys/bus/platform/devices/as7927_50x_sys/dcscm_cpld_ver", /* DC-SCM CPLD */
    "/sys/bus/platform/devices/as7927_50x_sys/fan_cpld_ver", /* Fan CPLD */
    "/sys/bus/platform/devices/as7927_50x_sys/fpga_cpld_ver", /* FPGA */
};

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as7927-50x-r0";
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

    /* 5 LEDs on the chassis */
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
    char *bmc_buf = NULL;
    char *aux_buf = NULL;
    int bmc_major = 0, bmc_minor = 0;
    unsigned int bmc_aux[4] = {0};
    char bmc_ver[16] = ""; 
    onlp_onie_info_t onie;
    char *bios_ver = NULL;

    for (i = 0; i < AIM_ARRAYSIZE(cpld_ver_path); i++) {

        len = onlp_file_read_str(&v[i], cpld_ver_path[i]);

        if (v[i] == NULL || len <= 0)
            return ONLP_STATUS_E_INTERNAL;
    }

    onlp_file_read_str(&bios_ver, BIOS_VER_PATH);
    onlp_onie_decode_file(&onie, IDPROM_PATH);

    if ((onlp_file_read_str(&bmc_buf, BMC_VER1_PATH) >= 0) &&
        (onlp_file_read_str(&aux_buf, BMC_VER2_PATH) >= 0))
    {
        bmc_buf[strcspn(bmc_buf, "\n")] = '\0';
        aux_buf[strcspn(aux_buf, "\n")] = '\0';

        /*
         * NOTE: The value in /sys/devices/platform/ipmi_bmc.0/firmware_revision is formatted
         * using "%u.%x" in the kernel driver (see ipmi_msghandler.c::firmware_revision_show).
         * The second field (after the dot) is output in hexadecimal format and must be parsed
         * using "%x" from user-space.
         */
        if (sscanf(bmc_buf, "%u.%x", &bmc_major, &bmc_minor) == 2 &&
            sscanf(aux_buf, "0x%x 0x%x 0x%x 0x%x", &bmc_aux[0], &bmc_aux[1], &bmc_aux[2], &bmc_aux[3]) == 4)
        {
            snprintf(bmc_ver, sizeof(bmc_ver), "%02X.%02X.%02X",
                     bmc_major, bmc_minor, bmc_aux[3]);
        }
    }

    pi->cpld_versions = aim_fstrdup("\r\n\t   CPU(0x21):%s"
                                    "\r\n\t   Main(0x61):%s"
                                    "\r\n\t   Main(0x62):%s"
                                    "\r\n\t   Main(0x63):%s"
                                    "\r\n\t   Carrier(0x60):%s"
                                    "\r\n\t   Fan(0x33):%s"
                                    , v[0], v[1], v[2], v[3], v[4], v[5]);

    pi->other_versions = aim_fstrdup("\r\n\t   FPGA(0x60):%s"
                                     "\r\n\t   BIOS: %s"
                                     "\r\n\t   ONIE: %s"
                                     "\r\n\t   BMC: %s",
                                     v[6], bios_ver, onie.onie_version, bmc_ver);

    for (i = 0; i < AIM_ARRAYSIZE(v); i++) {
        AIM_FREE_IF_PTR(v[i]);
    }

    AIM_FREE_IF_PTR(bmc_buf);
    AIM_FREE_IF_PTR(aux_buf);
    AIM_FREE_IF_PTR(bios_ver);
    onlp_onie_info_free(&onie);

    return ret;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
    aim_free(pi->other_versions);
}

int 
xcvr_over_temp_protector(int port)
{
    int ret = ONLP_STATUS_E_INTERNAL;
    AIM_SYSLOG_CRIT("Temperature critical", "OTP Action",
                    "Critical temperature detected on port %d; performing OTP protect action!", port);

    // SFP
    if (port > 0 && port <= LAST_OF_SFP_PORT){
        ret = onlp_file_write_int(0, MODULE_EFUSE_FORMAT, port);
        if (ret != ONLP_STATUS_OK){
            AIM_LOG_ERROR("Unable to write e-fuse status from port(%d)\r\n", port);
        }
    }
    // QSFP
    else if (port > LAST_OF_SFP_PORT && port <= PORT_NUM){
        ret = onlp_sfpi_control_set(port, ONLP_SFP_CONTROL_RESET_STATE, 1);
        if (ret != ONLP_STATUS_OK){
            AIM_LOG_ERROR("Unable to write reset status to port(%d)\r\n", port);
        }
    }

    return ret;
}

/*
 * Send thermal data (MAC temp, XCVR temp, port number) to BMC.
 *
 * This writes to the BMC thermal policy interface, equivalent to:
 *     ipmitool raw 0x34 0x13 <mac_temp> <xcvr_temp> <xcvr_num>
 *
 * Temperatures are in millidegree Celsius and converted to degrees Celsius before sending.
 *
 * @param mac_temp   MAC sensor temperature in milli-degrees Celsius
 * @param xcvr_temp  Transceiver temperature in milli-degrees Celsius
 * @param xcvr_num   Transceiver port number
 *
 * @return ONLP_STATUS_OK         on success
 *         ONLP_STATUS_E_INTERNAL if formatting fails
 *         ONLP_STATUS_E_MISSING  if writing to BMC fails
 */
int 
send_thermal_data_to_bmc(int mac_temp, int xcvr_temp, int xcvr_num)
{
    char data[64];
    int ret = ONLP_STATUS_E_INTERNAL;

    if (xcvr_temp == 0) {
        xcvr_num = 0;
    }

    ret = snprintf(data, sizeof(data), "%d %d %d",
                   (mac_temp / 1000), (xcvr_temp / 1000), xcvr_num);
    if (ret < 0 || ret >= (int)sizeof(data)) {
        AIM_LOG_WARN("snprintf failed or truncated: mac=%d xcvr=%d port=%d (ret=%d)\n",
                     mac_temp, xcvr_temp, xcvr_num, ret);
        return ONLP_STATUS_E_INTERNAL;
    }

    for (int i = 0; i < BMC_FILE_RETRY_COUNT; i++) {
        ret = onlp_file_write_str(data, BMC_THERMAL_DATA_PATH);
        if (ret == ONLP_STATUS_OK) {
            return ONLP_STATUS_OK;
        }
        sleep(BMC_FILE_RETRY_DELAY_S);
    }

    AIM_LOG_ERROR("Failed to write '%s' to %s", data , BMC_THERMAL_DATA_PATH);
    return ONLP_STATUS_E_MISSING;
}

/*
 * Control BMC thermal policy by collecting and sending temperature data.
 *
 *This function performs the following:
 * . Reads transceiver temperatures via registered readers.
 * . The MAC temperature will Sends 0 because it is not being used.
 * . Sends the collected data to the BMC for thermal management.
 *
 * @return ONLP_STATUS_OK on success,
 *         ONLP_STATUS_E_MISSING if:
 *             - sending thermal data fails.
 */
int 
control_thermal_policy_via_bmc(void)
{
    int i, p;
    int max_temp = ONLP_STATUS_E_MISSING;
    int max_port = ONLP_STATUS_E_MISSING;
    temp_reader_data_t temp[TEMP_SENSOR_COUNT] = {0};
    static bool port_otp_triggered[PORT_NUM + 1] = {false};

    for (i = 0; i < AIM_ARRAYSIZE(temp); i++) {
        tp_mgr.temp_hdlr.temp_readers[i](&temp[i]);
    }

    for (p = 1; p <= PORT_NUM; p++) {
        int present = temp[TEMP_SENSOR_XCVR].ports[p].present;
        int current_temp = temp[TEMP_SENSOR_XCVR].ports[p].temp;
        int high_alarm   = temp[TEMP_SENSOR_XCVR].ports[p].high_alarm;

        if (!present || current_temp == ONLP_STATUS_E_MISSING) {
            port_otp_triggered[p] = false;
            continue;
        }

        if (max_temp == ONLP_STATUS_E_MISSING || current_temp > max_temp) {
            max_temp = current_temp;
            max_port = p;
        }

        if (high_alarm != ONLP_STATUS_E_MISSING) {
            if (current_temp >= high_alarm) {
                if (!port_otp_triggered[p]) {
                    AIM_LOG_WARN("Port %d temperature (%d mC) exceeded high alarm (%d mC)! Triggering OTP.\n", 
                                    p, current_temp, high_alarm);
                    tp_mgr.otp_hdlr.otp_writer(p);
                    port_otp_triggered[p] = true;
                }
            }
            else {
                port_otp_triggered[p] = false;
            }
        } 
        else {
            port_otp_triggered[p] = false;
        }
    }

    if (max_temp == ONLP_STATUS_E_MISSING) {
        max_temp = 0;
        max_port = 0;
    }
    int mac_temp = 0; // MAC temperature not required

    return send_thermal_data_to_bmc(mac_temp, max_temp, max_port);
}

static pthread_mutex_t thermal_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t thermal_cond = PTHREAD_COND_INITIALIZER;
static bool thermal_thread_started = false;
static bool thermal_thread_waiting = false;
static pthread_t thermal_thread;

/*
 * Thermal policy thread loop.
 *
 * This background thread waits for a condition signal and runs the thermal policy.
 * It uses a condition variable to sleep until triggered, and only one instance runs at a time.
 *
 * Note: Signals are ignored if the thread is busy. No event queueing.
 */
void 
*thermal_policy_thread_loop(void *arg)
{
    while (1) {
        pthread_mutex_lock(&thermal_lock);
        thermal_thread_waiting = true;
        pthread_cond_wait(&thermal_cond, &thermal_lock);
        thermal_thread_waiting = false;
        pthread_mutex_unlock(&thermal_lock);

        control_thermal_policy_via_bmc();
    }

    return NULL;
}

void 
start_thermal_policy_thread_once(void)
{
    pthread_mutex_lock(&thermal_lock);
    if (!thermal_thread_started) {
        thermal_thread_started = true;
        if (pthread_create(&thermal_thread, NULL, thermal_policy_thread_loop, NULL) != 0) {
            AIM_LOG_ERROR("Failed to start thermal policy thread.");
            thermal_thread_started = false;
        } else {
            pthread_detach(thermal_thread);
            thermal_thread_waiting = true;
            AIM_LOG_INFO("Thermal policy thread started.");
        }
    }
    pthread_mutex_unlock(&thermal_lock);
}

int
onlp_sysi_platform_manage_fans(void)
{
    start_thermal_policy_thread_once();

    pthread_mutex_lock(&thermal_lock);
    if (thermal_thread_waiting) {
        pthread_cond_signal(&thermal_cond);
    } else {
        AIM_LOG_INFO("Thermal policy thread is busy; skipping this trigger.");
    }
    pthread_mutex_unlock(&thermal_lock);

    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}