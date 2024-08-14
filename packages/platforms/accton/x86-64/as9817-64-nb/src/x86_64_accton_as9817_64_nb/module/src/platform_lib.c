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
    int hwmon_idx, ret, value;

    hwmon_idx = onlp_get_fan_hwmon_idx();
    if (hwmon_idx < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Read attribute */
    ret = onlp_file_read_int(&value, FAN_SYSFS_FORMAT"fan%d_direction", (fid-1)%4 + 1);
    return ((ret < 0) || (ret >= FAN_DIR_COUNT)) ? ONLP_STATUS_E_INTERNAL : ret;
}

char* psu_get_pmbus_dir(int id)
{
    char *path[] = { PSU1_PMBUS_SYSFS_FORMAT, PSU2_PMBUS_SYSFS_FORMAT };
    return path[id-1];
}

int onlp_get_psu_hwmon_idx(int pid)
{
    /* find hwmon index */
    char* file = NULL;
    char* dir = NULL;
    char path[64];
    int ret, hwmon_idx, max_hwmon_idx = 20;

    dir = psu_get_pmbus_dir(pid);
    if (dir == NULL)
        return ONLP_STATUS_E_INTERNAL;

    for (hwmon_idx = 0; hwmon_idx <= max_hwmon_idx; hwmon_idx++) {
        snprintf(path, sizeof(path), "%s/hwmon/hwmon%d/", dir, hwmon_idx);

        ret = onlp_file_find(path, "name", &file);
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
        snprintf(path, sizeof(path), "/sys/bus/i2c/devices/76-0033/hwmon/hwmon%d/", hwmon_idx);

        ret = onlp_file_find(path, "name", &file);
        AIM_FREE_IF_PTR(file);

        if (ONLP_STATUS_OK == ret)
            return hwmon_idx;
    }

    return -1;
}

as9817_64_platform_id_t get_platform_id(void)
{
    int ret, pid = 0;

    ret = onlp_file_read_int(&pid, "/sys/devices/platform/as9817_64_fpga/platform_id");
    if (ONLP_STATUS_OK != ret) {
        return PID_UNKNOWN;
    }

    return (pid == 0) ? AS9817_64O : AS9817_64D;
}

int psu_pmbus_info_get(int id, char *node, int *value)
{
    char *path;
    *value = 0;

    path = psu_get_pmbus_dir(id);
    if (path == NULL)
        return ONLP_STATUS_E_INTERNAL;

    return onlp_file_read_int(value, "%s*%s", path, node);
}

int psu_pmbus_str_get(int id, char *data_buf, int data_len, char *data_name)
{
    char *path;
    int   len    = 0;
    char *str = NULL;
    int hwmon_idx;

    path = psu_get_pmbus_dir(id);
    if (path == NULL)
        return ONLP_STATUS_E_INTERNAL;

    hwmon_idx = onlp_get_psu_hwmon_idx(id);

    /* Read attribute */
    len = onlp_file_read_str(&str, "%s/hwmon/hwmon%d/%s", path, hwmon_idx, data_name);
    if (!str || len <= 0) {
        AIM_FREE_IF_PTR(str);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (len > data_len) {
        AIM_FREE_IF_PTR(str);
        return ONLP_STATUS_E_INVALID;
    }

    aim_strlcpy(data_buf, str, len+1);
    AIM_FREE_IF_PTR(str);
    return ONLP_STATUS_OK;
}

int psu_pmbus_info_set(int id, char *node, int value)
{
    char *path;

    path = psu_get_pmbus_dir(id);
    if (path == NULL)
        return ONLP_STATUS_E_INTERNAL;

    return onlp_file_write_int(value, "%s*%s", path, node);
}
