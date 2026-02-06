/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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
 * Platform Library
 *
 ***********************************************************/
#include <unistd.h>
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include "platform_lib.h"

int psu_status_info_get(int id, char *node, int *value)
{
    int ret = 0;
    int len = 0;
    char path[PSU_NODE_MAX_PATH_LEN] = {0};
    char *fandir = NULL;

    *value = 0;

    sprintf(path, "%spsu%d_%s", PSU_SYSFS_PATH, id, node);

    if (strncmp(node, "fan_dir", strlen("fan_dir")) == 0) {
        len = onlp_file_read_str(&fandir, "%s", path);

        if (fandir && len) {
            if (strncmp(fandir, "B2F", strlen("B2F")) == 0)
                *value = PSU_FAN_B2F;
            else if (strncmp(fandir, "F2B", strlen("F2B")) == 0)
                *value = PSU_FAN_F2B;

        } else {
            AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", path);
            return ONLP_STATUS_E_INTERNAL;
        }
        AIM_FREE_IF_PTR(fandir);
    } else {
        if (onlp_file_read_int(value, path) < 0) {
            AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", path);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    return ret;
}
