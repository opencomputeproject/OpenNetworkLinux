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
 * Fan Platform Implementation Defaults.
 *
 ***********************************************************/
#include <onlplib/file.h>
#include <onlp/platformi/fani.h>
#include "platform_lib.h"

enum fan_id {
    FAN_1_ON_FAN_BOARD = 1,
    FAN_2_ON_FAN_BOARD,
    FAN_3_ON_FAN_BOARD,
    FAN_4_ON_FAN_BOARD,
    FAN_5_ON_FAN_BOARD,
    FAN_6_ON_FAN_BOARD,
    FAN_7_ON_FAN_BOARD,
    FAN_8_ON_FAN_BOARD,
    FAN_9_ON_FAN_BOARD,
    FAN_10_ON_FAN_BOARD,
    FAN_1_ON_PSU_1,
    FAN_1_ON_PSU_2
};

#define MAX_PSU_FAN_SPEED 25500

#define CHASSIS_FAN_INFO(fid) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fid##_ON_FAN_BOARD), "Chassis Fan - "#fid, 0, {0} },\
        0x0,\
        ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,\
        0,\
        0,\
        ONLP_FAN_MODE_INVALID,\
    }

#define PSU_FAN_INFO(pid, fid) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fid##_ON_PSU_##pid), "PSU "#pid" - Fan "#fid, 0, {0} },\
        0x0,\
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,\
        0,\
        0,\
        ONLP_FAN_MODE_INVALID,\
    }

/* Static fan information */
onlp_fan_info_t finfo[] = {
    { }, /* Not used */
    CHASSIS_FAN_INFO(1),
    CHASSIS_FAN_INFO(2),
    CHASSIS_FAN_INFO(3),
    CHASSIS_FAN_INFO(4),
    CHASSIS_FAN_INFO(5),
    CHASSIS_FAN_INFO(6),
    CHASSIS_FAN_INFO(7),
    CHASSIS_FAN_INFO(8),
    CHASSIS_FAN_INFO(9),
    CHASSIS_FAN_INFO(10),
    PSU_FAN_INFO(1,1),
    PSU_FAN_INFO(2,1)
};

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static int
_onlp_fani_set_fan_dir_info(int fid, onlp_fan_info_t* info)
{
    enum onlp_fan_dir dir = onlp_get_fan_dir(fid);

    if (FAN_DIR_F2B == dir) {
        info->caps |= ONLP_FAN_CAPS_F2B;
        info->status |= ONLP_FAN_STATUS_F2B;
    }
    else {
        info->caps |= ONLP_FAN_CAPS_B2F;
        info->status |= ONLP_FAN_STATUS_B2F;
    }

    return ONLP_STATUS_OK;
}

static int
_onlp_fani_info_get_fan(int fid, onlp_fan_info_t* info)
{
    int value;
    int ret, pwm;

    /* get fan present status
     */
    ret = onlp_file_read_int(&value, FAN_SYSFS_FORMAT"fan%d_present", fid);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (value == 0) {
        return ONLP_STATUS_OK;
    }
    info->status |= ONLP_FAN_STATUS_PRESENT;

    /* get fan fault status (turn on when any one fails)
     */
    ret = onlp_file_read_int(&value, "%s""fan%d_fault", FAN_SYSFS_FORMAT, fid);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (value > 0) {
        info->status |= ONLP_FAN_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }

    /* get fan speed
     */
    ret = onlp_file_read_int(&value, "%s""fan%d_input", FAN_SYSFS_FORMAT, fid);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    info->rpm = value;

    /* get speed percentage from rpm
     */
    pwm = 0;
    ret = onlp_file_read_int(&pwm, "%s""fan%d_pwm", FAN_SYSFS_FORMAT, fid);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    /* openbmc not support fan target now */
    #if 0
    value = 0;
    ret = onlp_file_read_int(&value, "%s""fan%d_target", FAN_SYSFS_FORMAT, fid);
    if (ret < 0 || value == 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    info->percentage = (info->rpm*pwm)/value;
    if (info->percentage > 100)
        info->percentage = 100;
    #else
    info->percentage = pwm;
    #endif
    _onlp_fani_set_fan_dir_info(fid, info);

    return ONLP_STATUS_OK;
}

static int
_onlp_fani_info_get_fan_on_psu(int pid, onlp_fan_info_t* info)
{
    char *str = NULL;
    int   len = 0;
    int   val = 0;
    int   ret = 0;
    int   hwmon_idx;

    info->status |= ONLP_FAN_STATUS_PRESENT;

    /* get fan direction
     */
    hwmon_idx = onlp_get_psu_hwmon_idx(pid);
    if (hwmon_idx >= 0) {
        char file[32];
        snprintf(file, sizeof(file), "psu%d_fan_dir", pid);

        len = onlp_file_read_str(&str, PSU_SYSFS_FORMAT_1, (pid-1), hwmon_idx, file);
        if (str && len >= 3) {
            if (strncmp(str, "B2F", strlen("B2F")) == 0) {
                info->status |= ONLP_FAN_STATUS_B2F;
            }
            else {
                info->status |= ONLP_FAN_STATUS_F2B;
            }
        }
        AIM_FREE_IF_PTR(str);
    }

    /* get fan speed
     */
    ret = onlp_file_read_int(&val, PSU_SYSFS_FORMAT, (pid-1), pid, "fan1_input");
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read status from PSU(%d)\r\n", pid);
        return ONLP_STATUS_E_INTERNAL;
    }
    info->rpm = val;
    info->percentage = (info->rpm * 100)/MAX_PSU_FAN_SPEED;

    return ONLP_STATUS_OK;
}
/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    int rc = 0;
    int fid;
    VALIDATE(id);

    fid = ONLP_OID_ID_GET(id);
    *info = finfo[fid];

    switch (fid) {
        case FAN_1_ON_FAN_BOARD:
        case FAN_2_ON_FAN_BOARD:
        case FAN_3_ON_FAN_BOARD:
        case FAN_4_ON_FAN_BOARD:
        case FAN_5_ON_FAN_BOARD:
        case FAN_6_ON_FAN_BOARD:
        case FAN_7_ON_FAN_BOARD:
        case FAN_8_ON_FAN_BOARD:
        case FAN_9_ON_FAN_BOARD:
        case FAN_10_ON_FAN_BOARD:
            rc = _onlp_fani_info_get_fan(fid, info);
            break;
        case FAN_1_ON_PSU_1:
        case FAN_1_ON_PSU_2:
            rc = _onlp_fani_info_get_fan_on_psu(fid-FAN_10_ON_FAN_BOARD, info);
            break;
        default:
            rc = ONLP_STATUS_E_INVALID;
            break;
    }

    return rc;
}

/*
 * This function sets the fan speed of the given OID as a percentage.
 *
 * This will only be called if the OID has the PERCENTAGE_SET
 * capability.
 *
 * It is optional if you have no fans at all with this feature.
 */
int
onlp_fani_percentage_set(onlp_oid_t id, int p)
{
    int  fid;

    VALIDATE(id);

    fid = ONLP_OID_ID_GET(id);

    if (fid < FAN_1_ON_FAN_BOARD || fid > FAN_10_ON_FAN_BOARD) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if (onlp_file_write_int(p, "%s""fan%d_pwm", FAN_SYSFS_FORMAT, fid) != 0) {
        AIM_LOG_ERROR("Unable to change duty cycle of fan (%d)\r\n", fid);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}
