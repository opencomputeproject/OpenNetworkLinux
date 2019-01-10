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
#include <onlplib/i2c.h>
#include <unistd.h>
#include <onlplib/file.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum {
    PSU1_ID = 1,    /*At the left-upper of the front view.*/
    PSU2_ID,        /*At the left-lower of the front view.*/
    PSU3_ID,        /*At the right-upper of the front view.*/
    PSU4_ID,        /*At the right-lower of the front view.*/
};

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
{
    { }, /* Not used */
    { {ONLP_PSU_ID_CREATE(PSU1_ID), "PSU-1", 0}, },
    { {ONLP_PSU_ID_CREATE(PSU2_ID), "PSU-2", 0}, },
    { {ONLP_PSU_ID_CREATE(PSU3_ID), "PSU-3", 0}, },
    { {ONLP_PSU_ID_CREATE(PSU4_ID), "PSU-4", 0}, },
};

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

#define PMBUS_PATH_FORMAT "/sys/bus/platform/devices/minipack_psensor/%s%d_input"

static int
onlp_psui_rm_special_char(char* in, char* out, int max)
{
    int i, j;
    char c;

    DEBUG_PRINT("Before strip: [%s]\n", in);
    j = 0;
    for (i = 0; in[i]; i++) {
        c = in[i];
        if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'z')) {
            out[j]=in[i];
            j++;
        }
        if (j == max)
            break;
    }
    out[j] = '\0';
    DEBUG_PRINT("After strip: [%s]\n", out);
    return j;
}


static int
onlp_psui_get_BMC_info(int pid, onlp_psu_info_t* info)
{
    char cmd[128] = {0};
    char resp[128] = {0};

    int i2c_addr[][2] = {{49, 0x59},{48, 0x58},{57, 0x59},{56, 0x58}};
    int model  = 0x9a;
    int serial = 0x9e;
    int bus, addr, offset;
    char *bcmd = "i2cdump -y -f %d 0x%x s 0x%x|tail -n +2|cut -c56-";

    bus = i2c_addr[pid-1][0];
    addr = i2c_addr[pid-1][1];

    /* PSU is present if its model name can be retrieved.*/
    offset = model;
    sprintf(cmd, bcmd, bus, addr, offset);
    memset(info->model, 0, sizeof(info->model));
    if (bmc_reply_pure(cmd, 200000, resp, sizeof(resp)) < 0) {
        AIM_LOG_ERROR("Unable to send command to bmc(%s)\r\n", cmd);
        info->status &= ~ONLP_PSU_STATUS_PRESENT;
        return ONLP_STATUS_OK;
    }

    /*i2cdump return "failed" when slave is not present.*/
    if (strstr(resp, "failed") != NULL) {
        info->status &= ~ONLP_PSU_STATUS_PRESENT;
        return ONLP_STATUS_OK;
    }

    info->status |= ONLP_PSU_STATUS_PRESENT;
    info->caps = ONLP_PSU_CAPS_AC;
    onlp_psui_rm_special_char(resp, info->model, sizeof(info->model)-1);

    offset = serial;
    sprintf(cmd, bcmd, bus, addr, offset);
    memset(info->serial, 0, sizeof(info->serial));
    if (bmc_reply_pure(cmd, 200000, resp, sizeof(resp)) < 0) {
        AIM_LOG_ERROR("Unable to send command to bmc(%s)\r\n", cmd);
        info->status &= ~ONLP_PSU_STATUS_PRESENT;
        return ONLP_STATUS_OK;
    }
    onlp_psui_rm_special_char(resp, info->serial, sizeof(info->serial)-1);
    return ONLP_STATUS_OK;
}

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int pid, value, pid_in, pid_out;
    char  path[64] = {0};

    VALIDATE(id);
    pid  = ONLP_OID_ID_GET(id);
    *info = pinfo[pid]; /* Set the onlp_oid_hdr_t */

    /* Get the present status
     */

    if (onlp_psui_get_BMC_info(pid, info) != ONLP_STATUS_OK)
    {
        return ONLP_STATUS_E_INTERNAL;
    }
    if (!(info->status & ONLP_PSU_STATUS_PRESENT)) {
        return ONLP_STATUS_OK;
    }

    pid_in  = (pid * 2) - 1;
    pid_out = pid * 2;

    DEBUG_PRINT("in%d_input: for pid:%d\n",pid_in, pid);
    /* Read vin */
    sprintf(path, PMBUS_PATH_FORMAT, "in", pid_in);
    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (value >= 1000) {
        info->mvin = value;
        info->caps |= ONLP_PSU_CAPS_VIN;
    }

    /* Read iin */
    sprintf(path, PMBUS_PATH_FORMAT, "curr", pid_in);
    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }
    if (value >= 0) {
        info->miin = value;
        info->caps |= ONLP_PSU_CAPS_IIN;
    }

    /* Get pin */
    sprintf(path, PMBUS_PATH_FORMAT, "power", pid_in);
    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }
    if (value >= 0) {
        info->mpin = value / 1000;  /*power is in unit of microWatts.*/
        info->caps |= ONLP_PSU_CAPS_PIN;
    }
    /* Get vout */
    sprintf(path, PMBUS_PATH_FORMAT, "in", pid_out);
    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }
    if (value >= 0) {
        info->mvout = value;
        info->caps |= ONLP_PSU_CAPS_VOUT;
    }

    /* Read iout */
    sprintf(path, PMBUS_PATH_FORMAT, "curr", pid_out);
    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }
    if (value >= 0) {
        info->miout = value;
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }

    /* Read pout */
    sprintf(path, PMBUS_PATH_FORMAT, "power", pid_out);
    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }
    if (value >= 0) {
        info->mpout = value/1000;  /*power is in unit of microWatts.*/;
        info->caps |= ONLP_PSU_CAPS_POUT;
    }
    return 0;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}


