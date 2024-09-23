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

#define WARM_RESET_FORMAT "/sys/devices/platform/as7926_40xfb_sys/reset_%s"

/**
 * @brief warm reset for mac, mux, op2, gb and jr2
 * @param unit_id The warm reset device unit id, should be 0
 * @param reset_dev The warm reset device id, should be 1 ~ (WARM_RESET_MAX-1)
 * @param ret return value.
 */
int onlp_data_path_reset(uint8_t unit_id, uint8_t reset_dev)
{
    int len = 0;
    int ret = ONLP_STATUS_OK;
    char *magic_num = NULL;
    char *device_id[] = { NULL, "mac", NULL, "mux", "op2", "gb", "jr2" };

    if (unit_id != 0 || reset_dev >= WARM_RESET_MAX)
        return ONLP_STATUS_E_PARAM;

    if (reset_dev == 0 || reset_dev == WARM_RESET_PHY)
        return ONLP_STATUS_E_UNSUPPORTED;

    /* Reset device */
    len = onlp_file_read_str(&magic_num, WARM_RESET_FORMAT, device_id[reset_dev]);
    if (magic_num && len) {
        ret = onlp_file_write_str(magic_num, WARM_RESET_FORMAT, device_id[reset_dev]);
        if (ret < 0) {
            AIM_LOG_ERROR("Reset device-%d:(%s) failed.", reset_dev, device_id[reset_dev]);
        }
    }
    else {
        ret = ONLP_STATUS_E_INTERNAL;
    }

    AIM_FREE_IF_PTR(magic_num);
    return ret;
}
