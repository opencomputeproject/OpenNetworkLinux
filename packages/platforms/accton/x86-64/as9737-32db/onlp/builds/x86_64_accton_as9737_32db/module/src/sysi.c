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
#include <onlp/platformi/sfpi.h>
#include "platform_lib.h"

#include "x86_64_accton_as9737_32db_int.h"
#include "x86_64_accton_as9737_32db_log.h"

#define NUM_OF_CPLD_VER 5
#define NUM_OF_QSFP_PORT 32
#define BMC_THERMAL_POLICY_VER_MAJOR 0
#define BMC_THERMAL_POLICY_VER_MINOR 2
#define BMC_THERMAL_POLICY_VER_PATCH 0
#define BMC_FILE_RETRY_COUNT 3             // Retry count for file read/write operations
#define BMC_FILE_RETRY_DELAY_US 1000000    // Delay between retries (in microseconds, 1s)

typedef struct temp_reader_data {
	int data;
	int index;
} temp_reader_data_t;

int onlp_sysi_get_mac_temp(temp_reader_data_t *temp);
int onlp_sysi_get_xcvr_temp(temp_reader_data_t *temp);
int onlp_sysi_fan_dir_checker(enum onlp_fan_dir *major_dir);

enum temp_sensors {
	TEMP_SENSOR_MAC = 0,
	TEMP_SENSOR_XCVR,
	TEMP_SENSOR_COUNT
};

typedef struct temp_threshold {
	int up_adjust1;
	int up_adjust2;
	int up_adjust3;
	int down_adjust1;
	int down_adjust2;
	int down_adjust3;
} temp_threshold_t;

typedef int (*temp_getter_t)(temp_reader_data_t *temp);
typedef int (*fan_dir_checker_t)(enum onlp_fan_dir *major_fan_dir);

typedef struct temp_handler {
	temp_getter_t    temp_readers[TEMP_SENSOR_COUNT];
	temp_threshold_t thresholds[FAN_DIR_COUNT][TEMP_SENSOR_COUNT];
} temp_handler_t;

typedef struct fan_handler {
	fan_dir_checker_t   dir_checker;
} fan_handler_t;

struct thermal_policy_manager {
	temp_handler_t  temp_hdlr;
	fan_handler_t   fan_hdlr;
};

struct thermal_policy_manager tp_mgr = {
	.temp_hdlr = {
		.thresholds = {
			[FAN_DIR_F2B][TEMP_SENSOR_MAC]  = { .up_adjust1 = 70000, .up_adjust2 = 80000, .up_adjust3 = 90000, 
							    .down_adjust1 = 83000, .down_adjust2 = 73000, .down_adjust3 = 63000 },
			[FAN_DIR_F2B][TEMP_SENSOR_XCVR] = { .up_adjust1 = 58000, .up_adjust2 = 63000, .up_adjust3 = 68000,
							    .down_adjust1 = 65000, .down_adjust2 = 60000, .down_adjust3 = 55000 },
			[FAN_DIR_B2F][TEMP_SENSOR_MAC]  = { .up_adjust1 = 70000, .up_adjust2 = 80000, .up_adjust3 = 90000,
							    .down_adjust1 = 85000, .down_adjust2 = 75000, .down_adjust3 = 65000 },
			[FAN_DIR_B2F][TEMP_SENSOR_XCVR] = { .up_adjust1 = 58000, .up_adjust2 = 63000, .up_adjust3 = 68000,
							    .down_adjust1 = 65000, .down_adjust2 = 60000, .down_adjust3 = 55000 }
		},
		.temp_readers = {
			[TEMP_SENSOR_MAC] = onlp_sysi_get_mac_temp,
			[TEMP_SENSOR_XCVR] = onlp_sysi_get_xcvr_temp
		}
	},
	.fan_hdlr = {
		.dir_checker = onlp_sysi_fan_dir_checker
	}
};

const char* onlp_sysi_platform_get(void)
{
	return "x86-64-accton-as9737-32db-r0";
}

int onlp_sysi_onie_data_get(uint8_t** data, int* size)
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

int onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
	int i;
	onlp_oid_t* e = table;
	memset(table, 0, max*sizeof(onlp_oid_t));

	/* 5 Thermal sensors on the chassis */
	for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++)
		*e++ = ONLP_THERMAL_ID_CREATE(i);

	/* 5 LEDs on the chassis */
	for (i = 1; i <= CHASSIS_LED_COUNT; i++)
		*e++ = ONLP_LED_ID_CREATE(i);

	/* 2 PSUs on the chassis */
	for (i = 1; i <= CHASSIS_PSU_COUNT; i++)
		*e++ = ONLP_PSU_ID_CREATE(i);

	/* 6 Fans on the chassis */
	for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
		*e++ = ONLP_FAN_ID_CREATE(i);

	return 0;
}

static char* cpld_ver_path[NUM_OF_CPLD_VER] = {
	"/sys/devices/platform/as9737_32db_sys/cpu_ec_version",
	"/sys/devices/platform/as9737_32db_sys/fpga_version",
	"/sys/bus/i2c/devices/35-0061/version", /* CPLD-2 */
	"/sys/bus/i2c/devices/36-0062/version", /* CPLD-3 */
	"/sys/devices/platform/as9737_32db_fan/hwmon/hwmon*/version" /* Fan CPLD */
};

int onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
	int i, len, ret = ONLP_STATUS_OK;
	char *v[NUM_OF_CPLD_VER] = {NULL};
	onlp_onie_info_t onie;
	char *bios_ver = NULL;
	char *bmc_buf = NULL;
	char *aux_buf = NULL;
	int bmc_major = 0, bmc_minor = 0;
	unsigned int bmc_aux[4] = {0};
	char bmc_ver[16] = "";
	const char *bios = "";
	const char *onie_ver = "";

	for (i = 0; i < AIM_ARRAYSIZE(cpld_ver_path); i++) {
		if (i == 4) {
			int hwmon_idx = onlp_get_fan_hwmon_idx();

			if (hwmon_idx < 0) {
				ret = ONLP_STATUS_E_INTERNAL;
				break;
			}

			len = onlp_file_read_str(&v[i], FAN_SYSFS_FORMAT_1, hwmon_idx, "version");
		} else {
			len = onlp_file_read_str(&v[i], cpld_ver_path[i]);
		}

		if (v[i] == NULL || len <= 0) {
			ret = ONLP_STATUS_E_INTERNAL;
			break;
		}
	}

	if (ret == ONLP_STATUS_OK) {
	pi->cpld_versions = aim_fstrdup("\r\n\t   CPU EC: %s"
					"\r\n\t   FPGA(0x60): %s"
					"\r\n\t   SMB CPLD(0x61): %s"
					"\r\n\t   SMB CPLD(0x62): %s"
					"\r\n\t   Fan CPLD(0x66): %s\r\n",
					v[0], v[1], v[2], v[3], v[4]);
	}

	for (i = 0; i < AIM_ARRAYSIZE(v); i++)
		AIM_FREE_IF_PTR(v[i]);

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

	return ret;
}

void onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
	aim_free(pi->cpld_versions);
	aim_free(pi->other_versions);
}

int onlp_sysi_get_mac_temp(temp_reader_data_t *temp)
{
	int ret;
	char* file = NULL;

	ret = onlp_file_find("/run/mac/", "temp1_input", &file);
	AIM_FREE_IF_PTR(file);

	if (ONLP_STATUS_OK != ret) {
		temp->data = ONLP_STATUS_E_MISSING;
		return ret;
	}

	ret = onlp_file_read_int(&(temp->data), "/run/mac/temp1_input");
	if (ONLP_STATUS_OK != ret) {
		temp->data = ONLP_STATUS_E_MISSING;
		return ret;
	}

	return ONLP_STATUS_OK;
}

int onlp_sysi_get_xcvr_presence(void)
{
	onlp_sfp_bitmap_t bitmap;
	onlp_sfp_bitmap_t_init(&bitmap);
	onlp_sfp_presence_bitmap_get(&bitmap);

	/* Ignore SFP */
	AIM_BITMAP_CLR(&bitmap, 33);
	AIM_BITMAP_CLR(&bitmap, 34);
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

int onlp_sysi_get_xcvr_temp(temp_reader_data_t *temp)
{
	int ret = ONLP_STATUS_OK;
	int value, port;
	int port_temp = ONLP_STATUS_E_MISSING, max_temp = ONLP_STATUS_E_MISSING;
	int max_port = ONLP_STATUS_E_MISSING;

	temp->data = 0;
	temp->index = 0;

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
			max_port = port;
		}
	}

	if (max_temp != ONLP_STATUS_E_MISSING && 
	max_port != ONLP_STATUS_E_MISSING) {
		temp->data = max_temp;
		temp->index = max_port;
	}

	return ONLP_STATUS_OK;
}

int onlp_sysi_fan_dir_checker(enum onlp_fan_dir *major_dir)
{
	int i, f2b_fans = 0, b2f_fans = 0, ret = ONLP_STATUS_OK;

	for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
		if (onlp_get_fan_dir(i) == FAN_DIR_F2B)
			f2b_fans++;
		else
			b2f_fans++;
	}

	if (f2b_fans > 0 && b2f_fans > 0) {
		*major_dir = (f2b_fans >= b2f_fans) ? FAN_DIR_F2B : FAN_DIR_B2F;
		AIM_LOG_ERROR("Fan directions are NOT identical\r\n");
		ret = ONLP_STATUS_E_INTERNAL;
	} 
	else {
		*major_dir = (f2b_fans > 0) ? FAN_DIR_F2B : FAN_DIR_B2F;
		ret = ONLP_STATUS_OK;
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
int send_thermal_data_to_bmc(int mac_temp, int xcvr_temp, int xcvr_num)
{
	char data[32];
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
	int i, ret;
	int bmc_ver[] = {0, 0, 0};
	int target_ver[] = {
		BMC_THERMAL_POLICY_VER_MAJOR,
		BMC_THERMAL_POLICY_VER_MINOR,
		BMC_THERMAL_POLICY_VER_PATCH
	};
	temp_reader_data_t temp[TEMP_SENSOR_COUNT] = {0};
	enum onlp_fan_dir dir = FAN_DIR_F2B;

	ret = get_bmc_version(bmc_ver);
	if (ret != ONLP_STATUS_OK) {
		return ONLP_STATUS_E_MISSING;
	}

	for (i = 0; i < 3; i++) {
		if (bmc_ver[i] < target_ver[i]) {
			return ONLP_STATUS_E_MISSING;
		} else if (bmc_ver[i] > target_ver[i]) {
			break;
		}
	}

	tp_mgr.fan_hdlr.dir_checker(&dir);
	for (i = 0; i < AIM_ARRAYSIZE(temp); i++) {
		ret = tp_mgr.temp_hdlr.temp_readers[i](&temp[i]);
		if (ret != ONLP_STATUS_OK) {
			temp[i].data = tp_mgr.temp_hdlr.thresholds[dir][i].up_adjust3 + 1000;
		}
	}

	return send_thermal_data_to_bmc(temp[TEMP_SENSOR_MAC].data, 
					temp[TEMP_SENSOR_XCVR].data, 
					temp[TEMP_SENSOR_XCVR].index);
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
	int ret;

	while (1) {
		pthread_mutex_lock(&thermal_lock);
		thermal_thread_waiting = true;
		pthread_cond_wait(&thermal_cond, &thermal_lock);
		thermal_thread_waiting = false;
		pthread_mutex_unlock(&thermal_lock);

		ret = control_thermal_policy_via_bmc();
		if (ret != ONLP_STATUS_OK) {
			AIM_LOG_WARN("Failed to send thermal data to BMC !\n");
		}
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
