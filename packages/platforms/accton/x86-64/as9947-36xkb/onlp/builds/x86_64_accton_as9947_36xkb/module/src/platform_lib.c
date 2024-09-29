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
        len = onlp_file_read_str(&str, FAN_SYSFS_FORMAT_1, hwmon_idx, file);

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
        snprintf(path, sizeof(path), "/sys/devices/platform/as9947_36xkb_psu.%d/hwmon/hwmon%d/", pid-1, hwmon_idx);

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
        snprintf(path, sizeof(path), "/sys/devices/platform/as9947_36xkb_fan/hwmon/hwmon%d/", hwmon_idx);

        ret = onlp_file_find(path, "name", &file);
        AIM_FREE_IF_PTR(file);

        if (ONLP_STATUS_OK == ret)
            return hwmon_idx;
    }

    return -1;
}
