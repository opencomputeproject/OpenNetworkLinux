/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2019 WNC Computing Technology Corporation.
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
#include <onlp/platformi/fani.h>
#include "platform_lib.h"

#define FAN_NODE_MAX_NAME_LEN   64

#define MAX_FAN_SPEED     25500
#define MAX_PSU_FAN_SPEED 25500

#define CHASSIS_FAN_INFO(fid) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fid##_ON_FAN_BOARD), "Chassis Fan - "#fid, 0 }, \
        0x0, \
        ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }

#define PSU_FAN_INFO(pid, fid) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fid##_ON_PSU_##pid), "PSU "#pid" - Fan "#fid, 0 }, \
        0x0, \
        ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
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
_onlp_fani_info_get_fan(int fid, onlp_fan_info_t* info)
{
    int   present, direction;
    int   value;
    char  path[FAN_NODE_MAX_NAME_LEN] = {0};

    /* get fan present status */
    sprintf(path, "%s""fan_present_%d", FAN_BOARD_PATH, fid);
    DEBUG_PRINT("Fan (%d), present path = (%s)", fid, path);

    if (onlp_file_read_int_hex(&present, path) < 0) {
        AIM_LOG_ERROR("Unable to read fan present from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* If fan is present, value is 0. */
    if (present == 0)
        info->status |= ONLP_FAN_STATUS_PRESENT;

    /* get fan direction - TBD */
    memset(path, 0, sizeof(path));
    sprintf(path, "%s""fan_direction_%d", FAN_BOARD_PATH, fid);
    DEBUG_PRINT("Fan (%d), direction path = (%s)", fid, path);

    if (onlp_file_read_int_hex(&direction, path) < 0) {
        AIM_LOG_ERROR("Unable to read fan direction from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Fan direciton - 0: AFO (airflow out); F2B (Front-to-Back)
                       1: AFI (airflow in); B2F (Back-to-Front) */
    if (direction == 0)
        info->status |= ONLP_FAN_STATUS_F2B;
    if (direction == 1)
        info->status |= ONLP_FAN_STATUS_B2F;

    /* get front fan speed */
    memset(path, 0, sizeof(path));
    sprintf(path, "%s""fan_tach_%d_1", FAN_BOARD_PATH, fid);
    DEBUG_PRINT("Fan (%d), front speed path = (%s)", fid, path);

    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }
    info->rpm = value;

    /* get rear fan speed */
    memset(path, 0, sizeof(path));
    sprintf(path, "%s""fan_tach_%d_2", FAN_BOARD_PATH, fid);
    DEBUG_PRINT("Fan (%d), rear speed path = (%s)", fid, path);

    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* take the min value from front/rear fan speed */
    if (info->rpm > value) {
        info->rpm = value;
    }

    /* get speed percentage from rpm  */
    info->percentage = ((info->rpm) * 100) / MAX_FAN_SPEED;

    /* get fan fault status */
    if ((info->status & ONLP_FAN_STATUS_PRESENT) &&
        (info->rpm == 0)) {
        info->status |= ONLP_FAN_STATUS_FAILED;
    }

    return ONLP_STATUS_OK;
}

static int
_onlp_fani_info_get_fan_on_psu(int pid, onlp_fan_info_t* info)
{
    int val = 0;
    psu_type_t psu_type;

    /* get fan fault status */
    if (psu_pmbus_info_get(pid, "fan1_fault", &val) == ONLP_STATUS_OK) {
        info->status |= (val > 0) ? ONLP_FAN_STATUS_FAILED : 0;
    } else {
        info->status |= ONLP_FAN_STATUS_FAILED;
    }

    /* get fan direction */
    psu_type = get_psu_type(pid, info->model, sizeof(info->model));

    switch (psu_type)
    {
        case PSU_TYPE_AC_F2B:
            info->status |= ONLP_FAN_STATUS_B2F;
            break;
        case PSU_TYPE_AC_B2F:
            info->status |= ONLP_FAN_STATUS_F2B;
            break;
        case PSU_TYPE_UNKNOWN:  /* User insert a unknown PSU or unplugged.*/
        default:
            break;
    }

    /* get fan speed */
    if (psu_pmbus_info_get(pid, "fan1_input", &val) == ONLP_STATUS_OK) {
        info->rpm = val;
        info->percentage = ((info->rpm) * 100) / MAX_PSU_FAN_SPEED;

        if (info->rpm > 0)
            info->status |= ONLP_FAN_STATUS_PRESENT;
    }

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

    switch (fid)
    {
        case FAN_1_ON_FAN_BOARD:
        case FAN_2_ON_FAN_BOARD:
        case FAN_3_ON_FAN_BOARD:
        case FAN_4_ON_FAN_BOARD:
        case FAN_5_ON_FAN_BOARD:
        case FAN_6_ON_FAN_BOARD:
            rc =_onlp_fani_info_get_fan(fid, info);
            break;
        case FAN_1_ON_PSU_1:
            rc = _onlp_fani_info_get_fan_on_psu(PSU1_ID, info);
            break;
        case FAN_1_ON_PSU_2:
            rc = _onlp_fani_info_get_fan_on_psu(PSU2_ID, info);
            break;
        default:
            rc = ONLP_STATUS_E_INVALID;
            break;
    }

    return rc;
}

/*
 * This function sets the speed of the given fan in RPM.
 *
 * This function will only be called if the fan supprots the RPM_SET
 * capability.
 *
 * It is optional if you have no fans at all with this feature.
 */
int
onlp_fani_rpm_set(onlp_oid_t id, int rpm)
{
    return ONLP_STATUS_E_UNSUPPORTED;
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
    char *path = NULL;
    int  percent_v;
    char node[FAN_NODE_MAX_NAME_LEN] = {0};
    char percent_hex[10] = {0};

    VALIDATE(id);

    fid = ONLP_OID_ID_GET(id);

    /* reject p<50 (protect system) */
    if (p < FAN_MIN_PERCENTAGE){
        return ONLP_STATUS_E_INVALID;
    }

    switch (fid)
    {
        case FAN_1_ON_PSU_1:
            return ONLP_STATUS_E_UNSUPPORTED;
        case FAN_1_ON_PSU_2:
            return ONLP_STATUS_E_UNSUPPORTED;
        case FAN_1_ON_FAN_BOARD:
        case FAN_2_ON_FAN_BOARD:
        case FAN_3_ON_FAN_BOARD:
        case FAN_4_ON_FAN_BOARD:
        case FAN_5_ON_FAN_BOARD:
        case FAN_6_ON_FAN_BOARD:
            sprintf(node, "%s%s%d", FAN_BOARD_PATH, "fan_pwm_", fid);
            path = node;
            percent_v = 0x80 + (int)((p-50)*2.6);
            if (percent_v > 0xFF)
                percent_v = 0xFF;
            sprintf(percent_hex, "%x", percent_v);
            break;
        default:
            return ONLP_STATUS_E_INVALID;
    }

    DEBUG_PRINT("Fan path = (%s), Percent hex = (%s)", path, percent_hex);

    if (onlp_file_write((uint8_t*)percent_hex, strlen(percent_hex), path) < 0) {
        AIM_LOG_ERROR("Unable to write data to file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}


/*
 * This function sets the fan speed of the given OID as per
 * the predefined ONLP fan speed modes: off, slow, normal, fast, max.
 *
 * Interpretation of these modes is up to the platform.
 *
 */
int
onlp_fani_mode_set(onlp_oid_t id, onlp_fan_mode_t mode)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * This function sets the fan direction of the given OID.
 *
 * This function is only relevant if the fan OID supports both direction
 * capabilities.
 *
 * This function is optional unless the functionality is available.
 */
int
onlp_fani_dir_set(onlp_oid_t id, onlp_fan_dir_t dir)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * Generic fan ioctl. Optional.
 */
int
onlp_fani_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}