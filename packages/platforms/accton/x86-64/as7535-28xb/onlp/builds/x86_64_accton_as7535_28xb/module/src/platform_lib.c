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

#define WARM_RESET_FORMAT "/sys/devices/platform/as7535_28xb_sys/reset_%s"

enum onlp_fan_dir onlp_get_fan_dir(int fid)
{
    int len = 0;
    int i = 0;
    char *str = NULL;
    char *dirs[FAN_DIR_COUNT] = { "F2B", "B2F" };
    enum onlp_fan_dir dir = FAN_DIR_F2B;

    /* Read attribute */
    len = onlp_file_read_str(&str, "%s""fan%d_dir", FAN_BOARD_PATH, fid);
    if (!str || len <= 0) {
        AIM_FREE_IF_PTR(str);
        return dir;
    }

    /* Verify Fan dir string length */
    if (len < 3) {
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
    return dir;
}

int get_pcb_id()
{
    FILE *pf;
    char command[32];
    char data[8];
    int pcb_id = 0;

    sprintf(command, "ipmitool raw 0x34 0x22 0x60 0");
    pf = popen(command,"r");
    fgets(data, 8 , pf);

    if (pclose(pf) != 0)
    {
        fprintf(stderr," Error: Failed to close command stream \n");
        return ONLP_STATUS_E_INTERNAL;
    }
    /* get the pcb id to check the thermal number */
    pcb_id = (atoi(data) >> 2) & 0xff;

    return pcb_id;
}

/**
 * @brief warm reset for mac, mux
 * @param unit_id The warm reset device unit id, should be 0
 * @param reset_dev The warm reset device id, should be 1 ~ (WARM_RESET_MAX-1)
 * @param ret return value.
 */
int onlp_data_path_reset(uint8_t unit_id, uint8_t reset_dev)
{
    int len = 0;
    int ret = ONLP_STATUS_OK;
    char *magic_num = NULL;
    char *device_id[] = { NULL, "mac", NULL, "mux" };

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
