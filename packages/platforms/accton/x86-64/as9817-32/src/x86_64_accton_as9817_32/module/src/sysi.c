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

#include "x86_64_accton_as9817_32_int.h"
#include "x86_64_accton_as9817_32_log.h"

#define NUM_OF_QSFP_PORT 32
#define TEMPERATURE_COMPENSATION (5.0f)
#define MAC_MAX_TEMP 90000
#define MAC_XCVR_TEMP 70000
#define BMC_FILE_RETRY_COUNT 3             // Retry count for file read/write operations
#define BMC_FILE_RETRY_DELAY_US 1000000    // Delay between retries (in microseconds, 1s)

int onlp_sysi_get_max_xcvr_temp(int *temp, int *port_num);

typedef struct {
    const char* name;
    const char* path;
    int is_fan;
} cpld_version_entry_t;

enum {
    CPLD_IDX_FPGA = 0,
    CPLD_IDX_CPLD1,
    CPLD_IDX_CPLD2,
    CPLD_IDX_DSCM,
    CPLD_IDX_FAN,
    NUM_OF_CPLD_VER
};

static const cpld_version_entry_t cpld_versions[NUM_OF_CPLD_VER] = {
    { "FPGA",        "/sys/devices/platform/as9817_32_sys/fpga_version",          0 },
    { "Port CPLD1",  "/sys/devices/platform/as9817_32_fpga/cpld1_version",        0 },
    { "Port CPLD2",  "/sys/devices/platform/as9817_32_fpga/cpld2_version",        0 },
    { "DSCM CPLD",   "/sys/devices/platform/as9817_32_fpga/cpld3_version",        0 },
    { "Fan CPLD",    "/sys/devices/platform/as9817_32_fan/hwmon/hwmon*/version",  1 }
};

const char*
onlp_sysi_platform_get(void)
{
    as9817_32_platform_id_t pid = get_platform_id();

    switch (pid) {
        case AS9817_32O: return "x86-64-accton-as9817-32o-r0";
        case AS9817_32D: return "x86-64-accton-as9817-32d-r0";
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

void
onlp_sysi_onie_data_free(uint8_t* data)
{
    AIM_FREE_IF_PTR(data);
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

    /* 14 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}


void convert_version_format(const char *input, char *output, size_t output_size) {
    int major, minor;

    if (sscanf(input, "%d.%d", &major, &minor) == 2) {
        snprintf(output, output_size, "%02X.%02X", major, minor);
    }
    else if (sscanf(input, "%d", &major) == 1) {
        snprintf(output, output_size, "%02X", major);
    }
    else {
        snprintf(output, output_size, "ERR");
    }
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int i, len;
    char *v[NUM_OF_CPLD_VER] = {NULL};
    char n[NUM_OF_CPLD_VER][8] = {0};
    onlp_onie_info_t onie;
    char *bios_ver = NULL;
    char *bmc_buf = NULL;
    char *aux_buf = NULL;
    int bmc_major = 0, bmc_minor = 0;
    unsigned int bmc_aux[4] = {0};
    char bmc_ver[16] = "";
    const char *bios = "";
    const char *onie_ver = "";

    for (i = 0; i < AIM_ARRAYSIZE(cpld_versions); i++) {
        if (cpld_versions[i].is_fan) {
            int hwmon_idx = onlp_get_fan_hwmon_idx();

            if (hwmon_idx < 0) {
                continue;
            }

            len = onlp_file_read_str(&v[i], FAN_SYSFS_FORMAT_1, hwmon_idx, "version");
        }
        else {
            len = onlp_file_read_str(&v[i], cpld_versions[i].path);
        }

        if (v[i] == NULL || len <= 0) {
            continue;
        }
        
        convert_version_format(v[i], n[i], sizeof(n[i]));
    }

    pi->cpld_versions = aim_fstrdup("\r\n\t   %s: %s"
                                    "\r\n\t   %s: %s"
                                    "\r\n\t   %s: %s"
                                    "\r\n\t   %s: %s"
                                    "\r\n\t   %s: %s",
                                    cpld_versions[CPLD_IDX_FPGA].name, n[CPLD_IDX_FPGA], 
                                    cpld_versions[CPLD_IDX_CPLD1].name, n[CPLD_IDX_CPLD1],
                                    cpld_versions[CPLD_IDX_CPLD2].name, n[CPLD_IDX_CPLD2],
                                    cpld_versions[CPLD_IDX_DSCM].name, n[CPLD_IDX_DSCM],
                                    cpld_versions[CPLD_IDX_FAN].name, n[CPLD_IDX_FAN]);

    for (i = 0; i < AIM_ARRAYSIZE(v); i++) {
        AIM_FREE_IF_PTR(v[i]);
    }

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

    if (onlp_file_read_str(&bios_ver, BIOS_VER_PATH) > 0) {
        bios = bios_ver;
    }
    if (onlp_onie_decode_file(&onie, IDPROM_PATH) >= 0) {
        onie_ver = onie.onie_version;
    }

    pi->other_versions = aim_fstrdup("\r\n\t   BIOS: %s\r\n\t   ONIE: %s\r\n\t   BMC: %s",
                                     bios, onie_ver, bmc_ver);

    AIM_FREE_IF_PTR(bmc_buf);
    AIM_FREE_IF_PTR(aux_buf);
    AIM_FREE_IF_PTR(bios_ver);
    onlp_onie_info_free(&onie);

    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    AIM_FREE_IF_PTR(pi->cpld_versions);
    AIM_FREE_IF_PTR(pi->other_versions);
}

/**
 * Reads a 32-bit frequency value from sysfs file (formatted as 4 space-separated hex bytes),
 * and converts it into temperature in Celsius using a datasheet-defined formula.
 *
 * Expected input file content format: "A0 A1 A2 A3"
 *
 * Datasheet Formula:
 *   Period = (Data + 1) x 80 ns
 *   Period = 1 / freq
 *   -> Data = (1 / freq / 80e-9) - 1
 *   -> Temp = -0.317704 x Data + 476.359
 *
 * @param file_path       Path to the file containing the 32-bit frequency as 4 hex values.
 * @param temp            Pointer to store the resulting temperature in Celsius
 * @return                ONLP_STATUS_OK if successful;
 *                        ONLP_STATUS_E_MISSING if I2C read fails, frequency is 0,
 *                        or calculated Data is out of expected range.
 */
int get_mac_temperature_from_fpga(const char *file_path, float *temp)
{
    uint8_t bytes[4] = {0, 0, 0, 0};
    uint32_t freq = 0;
    int ret;
    double period, data;
    char *tmp = NULL;

    if (file_path == NULL || temp == NULL) {
        AIM_LOG_ERROR("Null pointer passed for file_path or temperature result\n");
        return ONLP_STATUS_E_MISSING;
    }

    ret = onlp_file_read_str(&tmp, file_path);
    if (ret <= 0) {
        AIM_LOG_ERROR("Failed to read 4-byte frequency from %s\n", file_path);
        return ONLP_STATUS_E_MISSING;
    }
    ret = sscanf(tmp, "%x %x %x %x",
                 (unsigned int *)&bytes[0],
                 (unsigned int *)&bytes[1],
                 (unsigned int *)&bytes[2],
                 (unsigned int *)&bytes[3]);
    if (ret != 4) {
        AIM_FREE_IF_PTR(tmp);
        AIM_LOG_ERROR("Expected 4 hex values from %s, got %d\n", file_path, ret);
        return ONLP_STATUS_E_MISSING;
    }
    AIM_FREE_IF_PTR(tmp);

    freq = ((uint32_t)bytes[0]) |
           ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[3] << 24);

    if (freq == 0) {
	AIM_SYSLOG_WARN("Temperature warning", "Temperature warning",
                        "Invalid MAC temperature detected! "
                        "Please check the FPGA firmware version.");
        return ONLP_STATUS_E_MISSING;
    }

    period = 1.0 / (double)freq;
    data = (period / 80e-9) - 1.0;

    if (data < 0.0 || data > 2047.0) {
        AIM_LOG_ERROR("ADC data %.2f out of range\n", data);
        return ONLP_STATUS_E_MISSING;
    }

    *temp = (float)(-0.317704 * data + 476.359);
    return ONLP_STATUS_OK;
}

int onlp_sysi_get_mac_temp(int *temp)
{
    int ret;
    float min_temp, max_temp;

    ret = get_mac_temperature_from_fpga(FGPA_MAC_MIN_TEMP_PATH, &min_temp);
    if (ret != ONLP_STATUS_OK) {
        return ret;
    }
    ret = get_mac_temperature_from_fpga(FGPA_MAC_MAX_TEMP_PATH, &max_temp);
    if (ret != ONLP_STATUS_OK) {
        return ret;
    }

    *temp = (int)(((min_temp + max_temp) / 2.0f) + TEMPERATURE_COMPENSATION);
    *temp *= 1000;

    return ONLP_STATUS_OK;
}

int onlp_sysi_get_xcvr_presence(void)
{
    onlp_sfp_bitmap_t bitmap;
    onlp_sfp_bitmap_t_init(&bitmap);
    onlp_sfp_presence_bitmap_get(&bitmap);

    /* Ignore SFP */
    AIM_BITMAP_CLR(&bitmap, 32);
    AIM_BITMAP_CLR(&bitmap, 33);
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

int onlp_sysi_get_max_xcvr_temp(int *temp, int *port_num)
{
    int ret = ONLP_STATUS_OK;
    int value, port;
    int port_temp = MAC_XCVR_TEMP;
    int max_temp_port = ONLP_STATUS_E_MISSING, max_temp = ONLP_STATUS_E_MISSING;

    *temp = ONLP_STATUS_E_MISSING;
    *port_num = ONLP_STATUS_E_MISSING;

    if (!onlp_sysi_get_xcvr_presence()) {
        return ONLP_STATUS_OK;
    }

    for (port = 0; port < NUM_OF_QSFP_PORT; port++) {
        if (!onlp_sfpi_is_present(port)) {
            continue;
        }

        value = onlp_sfpi_dev_readb(port, 0x50, 0);
        if (value < 0) {
            AIM_LOG_ERROR("Unable to get read port(%d) eeprom\r\n", port);
            continue;
        }

        if (value == 0x18 || value == 0x19 || value == 0x1E || value == 0x1B) {
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
            max_temp_port = port + 1;
        }
    }

    *temp = max_temp;
    *port_num = max_temp_port;
    return ONLP_STATUS_OK;
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
int send_thermal_data_to_bmc(int mac_temp, int xcvr_temp, int xcvr_num)
{
    char data[32];
    int ret = ONLP_STATUS_E_INTERNAL;

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
        usleep(BMC_FILE_RETRY_DELAY_US);
    }

    AIM_LOG_ERROR("Failed to write '%s' to %s", data , BMC_THERMAL_DATA_PATH);
    return ONLP_STATUS_E_MISSING;
}

/*
 * Control BMC thermal policy by collecting and sending temperature data.
 *
 *This function performs the following:
 * . Checks if the current BMC firmware version supports thermal policy.
 * . Enables the BMC thermal policy if it is not already enabled.
 * . Reads MAC and transceiver temperatures via registered readers.
 * . Sends the collected data to the BMC for thermal management.
 *
 * @return ONLP_STATUS_OK on success,
 *         ONLP_STATUS_E_MISSING if:
 *             - BMC version is below the minimum required,
 *             - enabling the policy fails,
 *             - or sending thermal data fails.
 */
int control_thermal_policy_via_bmc(void)
{
    int ret;
    int mac_temp;
    int port_temp, port_number;

    ret = onlp_sysi_get_mac_temp(&mac_temp);
    if (ret != ONLP_STATUS_OK) {
        mac_temp = MAC_MAX_TEMP;
    }

    ret = onlp_sysi_get_max_xcvr_temp(&port_temp, &port_number);
    if (ret != ONLP_STATUS_OK ||
        /* No transceiver detected. */
        port_temp == ONLP_STATUS_E_MISSING ||
        port_number == ONLP_STATUS_E_MISSING) {
        /*
         * If port_number is 0, the BMC will ignore the transceiver temperature.
         */
        port_temp = 0;
        port_number = 0;
    }

    return send_thermal_data_to_bmc(mac_temp, port_temp, port_number);
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
void *thermal_policy_thread_loop(void *arg)
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

/*
 * Launch the thermal policy thread once.
 */
void start_thermal_policy_thread_once(void)
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

/*
 * Called periodically to trigger thermal policy evaluation.
 */
int onlp_sysi_platform_manage_fans(void)
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
