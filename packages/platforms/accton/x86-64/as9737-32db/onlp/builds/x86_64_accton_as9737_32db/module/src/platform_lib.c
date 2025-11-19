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
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include "platform_lib.h"

enum onlp_fan_dir onlp_get_fan_dir(int fid)
{
	int len = 0;
	int i = 0;
	int hwmon_idx;
	char *str = NULL;
	char *dirs[FAN_DIR_COUNT] = { "F2B", "B2F" };
	char file[32];
	enum onlp_fan_dir dir = FAN_DIR_F2B;

	hwmon_idx = onlp_get_fan_hwmon_idx();
	if (hwmon_idx >= 0) {
		/* Read attribute */
		snprintf(file, sizeof(file), "fan%d_dir", fid);
		len = onlp_file_read_str(&str, FAN_SYSFS_FORMAT_1, hwmon_idx, 
					 file);

		/* Verify Fan dir string length */
		if (!str || len < 3) {
			AIM_FREE_IF_PTR(str);
			return dir;
		}

		for (i = 0; i < AIM_ARRAYSIZE(dirs); i++) {
			if (strncmp(str, dirs[i], strlen(dirs[i])) == 0) {
				dir = (enum onlp_fan_dir)i;
				break;
			}
		}

		AIM_FREE_IF_PTR(str);
	}

	return dir;
}

int onlp_get_psu_hwmon_idx(int pid)
{
	/* find hwmon index */
	char* file = NULL;
	char path[64];
	int ret, hwmon_idx, max_hwmon_idx = 20;

	for (hwmon_idx = 0; hwmon_idx <= max_hwmon_idx; hwmon_idx++) {
		snprintf(path, sizeof(path), "/sys/devices/platform/as9737_32db_psu/hwmon/hwmon%d/", 
				hwmon_idx);

		if (pid == 1)
			ret = onlp_file_find(path, "psu1_present", &file);
		else if (pid == 2)
			ret = onlp_file_find(path, "psu2_present", &file);
		else
			return -1;

		AIM_FREE_IF_PTR(file);

		if (ONLP_STATUS_OK == ret)
			return hwmon_idx;
	}

	return -1;
}

int onlp_get_fan_hwmon_idx(void)
{
	/* find hwmon index */
	char* file = NULL;
	char path[64];
	int ret, hwmon_idx, max_hwmon_idx = 20;

	for (hwmon_idx = 0; hwmon_idx <= max_hwmon_idx; hwmon_idx++) {
		snprintf(path, 
			 sizeof(path), 
			 "/sys/devices/platform/as9737_32db_fan/hwmon/hwmon%d/", 
			 hwmon_idx);

		ret = onlp_file_find(path, "name", &file);
		AIM_FREE_IF_PTR(file);

		if (ONLP_STATUS_OK == ret)
			return hwmon_idx;
	}

	return -1;
}

psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
	int ret = 0;
	int hwmon_idx;
	psu_type_t ptype = PSU_TYPE_UNKNOWN;

	hwmon_idx = onlp_get_psu_hwmon_idx(id);
	if(hwmon_idx >= 0){
		char file[32];
		char *mn = NULL;

		snprintf(file, sizeof(file), "psu%d_model", id);
		ret = onlp_file_read_str(&mn, 
				PSU_SYSFS_FORMAT_1, hwmon_idx, file);

		if (ret <= 0 || ret > PSU_MODEL_NAME_LEN+1 || mn == NULL) {
			AIM_FREE_IF_PTR(mn);
			return PSU_TYPE_UNKNOWN;
		}

		if (modelname)
			strncpy(modelname, mn, PSU_MODEL_NAME_LEN + 1);

		if (strncmp(mn, "FSJ001-610G", PSU_MODEL_NAME_LEN) == 0)
			ptype = PSU_TYPE_AC_FSJ001_610G_F2B;
		else if (strncmp(mn, "FSJ001-612G", PSU_MODEL_NAME_LEN) == 0)
			ptype = PSU_TYPE_AC_FSJ001_612G_F2B;
		else if (strncmp(mn, "FSJ004-612G", PSU_MODEL_NAME_LEN) == 0)
			ptype = PSU_TYPE_AC_FSJ004_612G_B2F;
		else if (strncmp(mn, "FSJ035-610G", PSU_MODEL_NAME_LEN) == 0)
			ptype = PSU_TYPE_DC48_FSJ035_610G_F2B;
		else if (strncmp(mn, "FSJ036-610G", PSU_MODEL_NAME_LEN) == 0)
			ptype = PSU_TYPE_DC48_FSJ036_610G_B2F;

		AIM_FREE_IF_PTR(mn);
		return ptype;
	}

	return ptype;
}

int psu_status_info_get(int id, char *node, int *value)
{
	int ret = 0;
	int len = 0;
	int hwmon_idx = 0;
	char path[PSU_NODE_MAX_PATH_LEN] = {0};
	char *fandir = NULL;

	*value = 0;

	if (strncmp(node, "fan_dir", strlen("fan_dir")) == 0) {
		hwmon_idx = onlp_get_psu_hwmon_idx(id);
		if (hwmon_idx >= 0) {
			snprintf(path, sizeof(path), PSU_SYSFS_FORMAT_1"%d_fan_dir", hwmon_idx, "psu", id);
			len = onlp_file_read_str(&fandir, "%s", path);

			if (fandir && len) {
				if (strncmp(fandir, "B2F", strlen("B2F")) == 0)
					*value = FAN_DIR_B2F;
				else if (strncmp(fandir, "F2B", strlen("F2B")) == 0)
					*value = FAN_DIR_F2B;
			} else {
				AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", path);
				return ONLP_STATUS_E_INTERNAL;
			}
			AIM_FREE_IF_PTR(fandir);
		}
	} else {
		sprintf(path, PSU_SYSFS_FORMAT, id, node);
		if (onlp_file_read_int(value, path) < 0) {
			AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", path);
			return ONLP_STATUS_E_INTERNAL;
		}
	}

	return ret;
}

/**
 * get_bmc_version - Get BMC version as int array
 * @ver: int[3], ver[0]=major, ver[1]=minor, ver[2]=aux last byte
 *
 * Return: ONLP_STATUS_OK on success, or ONLP error code
 */
int get_bmc_version(int *ver)
{
	char *tmp;
	char primary_fw_ver[32] = {0};
	char aux_fw_raw[64] = {0};
	const char *last_aux = NULL;
	int len;

	if (!ver) {
		return ONLP_STATUS_E_INVALID;
	}
	memset(ver, 0, sizeof(int) * 3);

	len = onlp_file_read_str(&tmp, BMC_VER1_PATH);
	if (tmp && len) {
		memcpy(primary_fw_ver, tmp, len);
		primary_fw_ver[len] = '\0';
	} else {
		return ONLP_STATUS_E_INTERNAL;
	}
	AIM_FREE_IF_PTR(tmp);

	len = onlp_file_read_str(&tmp, BMC_VER2_PATH);
	if (tmp && len) {
		memcpy(aux_fw_raw, tmp, len);
		aux_fw_raw[len] = '\0';
	} else {
		return ONLP_STATUS_E_INTERNAL;
	}
	AIM_FREE_IF_PTR(tmp);

	primary_fw_ver[strcspn(primary_fw_ver, "\n")] = '\0';
	aux_fw_raw[strcspn(aux_fw_raw, "\n")] = '\0';

	/* Parse main version: "0.3" => ver[0]=0, ver[1]=3 */
	if (sscanf(primary_fw_ver, "%d.%d", &ver[0], &ver[1]) != 2) {
		return ONLP_STATUS_E_INTERNAL;
	}

	/* Get last AUX byte (e.g., from "0x00 0x00 0x00 0x02") */
	last_aux = strrchr(aux_fw_raw, ' ');
	last_aux = last_aux ? last_aux + 1 : aux_fw_raw;

	if (sscanf(last_aux, "0x%x", &ver[2]) != 1) {
		return ONLP_STATUS_E_INTERNAL;
	}

	return ONLP_STATUS_OK;
}
