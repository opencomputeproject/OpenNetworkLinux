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

char* psu_get_eeprom_dir(int pid)
{
    char *path[] = { PSU1_EEPROM_SYSFS_FORMAT, PSU2_EEPROM_SYSFS_FORMAT };
    return path[pid-1];
}

char* psu_get_pmbus_dir(int pid)
{
    char *path[] = { PSU1_PMBUS_SYSFS_FORMAT, PSU2_PMBUS_SYSFS_FORMAT };
    return path[pid-1];
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
        snprintf(path, sizeof(path), "/sys/bus/i2c/devices/8-0066/hwmon/hwmon%d/", hwmon_idx);

        ret = onlp_file_find(path, "name", &file);
        AIM_FREE_IF_PTR(file);

        if (ONLP_STATUS_OK == ret)
            return hwmon_idx;
    }

    return -1;
}

int psu_cpld_status_get(int pid, char *node, int *value)
{
    char *path;
    *value = 0;

    path = psu_get_eeprom_dir(pid);
    if (path == NULL)
        return ONLP_STATUS_E_INTERNAL;

    return onlp_file_read_int(value, "%s*%s", path, node);
}

int psu_eeprom_str_get(int pid, char *data_buf, int data_len, char *data_name)
{
    char *path;
    int   len    = 0;
    char *str = NULL;

    path = psu_get_eeprom_dir(pid);
    if (path == NULL)
        return ONLP_STATUS_E_INTERNAL;

    /* Read attribute */
    len = onlp_file_read_str(&str, "%s/%s", path, data_name);
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

int psu_pmbus_info_get(int pid, char *node, int *value)
{
    char *path;
    *value = 0;

    path = psu_get_pmbus_dir(pid);
    if (path == NULL)
        return ONLP_STATUS_E_INTERNAL;

    return onlp_file_read_int(value, "%s*%s", path, node);
}

int fan_info_get(int fid, char *node, int *value)
{
    *value = 0;
    return onlp_file_read_int(value, FAN_SYSFS_FORMAT, fid, node);
}

psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
    int   len = 0;
    char  *path;
    char  *str = NULL;

    path = psu_get_pmbus_dir(id);
    if (path == NULL)
        return ONLP_STATUS_E_INTERNAL;

    len = onlp_file_read_str(&str, "%s/%s", path, "psu_mfr_model");

    if (!str || len <= 0) {
        AIM_FREE_IF_PTR(str);
        return PSU_TYPE_UNKNOWN;
    }

    if (!strncmp(str, "SPAACTN-03", strlen("SPAACTN-03")))
    {
        if (modelname)
            aim_strlcpy(modelname, str, strlen("SPAACTN-03")<(modelname_len-1)?(strlen("SPAACTN-03")+1):(modelname_len-1));
            AIM_FREE_IF_PTR(str);
        return PSU_TYPE_SPAACTN_03;
    }

    if (!strncmp(str, "CRXT-T0T12", strlen("CRXT-T0T12")))
    {
        if (modelname)
            aim_strlcpy(modelname, str, strlen("CRXT-T0T12")<(modelname_len-1)?(strlen("CRXT-T0T12")+1):(modelname_len-1));
            AIM_FREE_IF_PTR(str);
        return PSU_TYPE_CRXT_T0T12;
    }

    AIM_FREE_IF_PTR(str);
    return PSU_TYPE_UNKNOWN;
}
