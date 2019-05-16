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


#define MAX_FAN_SPEED     25500
#define MAX_PSU_FAN_SPEED 25500


#define CHASSIS_FAN_INFO(fid)           \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fid##_ON_FAN_BOARD), "Chassis Fan - "#fid, 0 },\
        0x0,\
        ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,\
        0,\
        0,\
        ONLP_FAN_MODE_INVALID,\
    }

#define PSU_FAN_INFO(pid, fid) 		\
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fid##_ON_PSU_##pid), "PSU "#pid" - Fan "#fid, 0 },\
        0x0,\
        ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,\
        0,\
        0,\
        ONLP_FAN_MODE_INVALID,\
    }

enum fan_id {
    FAN_1_ON_FAN_BOARD = 1,
    FAN_2_ON_FAN_BOARD,
    FAN_3_ON_FAN_BOARD,
    FAN_4_ON_FAN_BOARD,
    FAN_5_ON_FAN_BOARD,
    FAN_1_ON_PSU_1,
    FAN_1_ON_PSU_2
};

/* Static fan information */
onlp_fan_info_t finfo[] = {
    { }, /* Not used */
    CHASSIS_FAN_INFO(1),
    CHASSIS_FAN_INFO(2),
    CHASSIS_FAN_INFO(3),
    CHASSIS_FAN_INFO(4),
    CHASSIS_FAN_INFO(5),
    PSU_FAN_INFO(1, 1),
    PSU_FAN_INFO(2, 1)
};

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static int pmbus_cfg[CHASSIS_PSU_COUNT][2] = {{13, 0x5b}, {12, 0x58}};

static int
_onlp_fani_info_get_fan_on_psu(int pid, onlp_fan_info_t* info)
{
    int     bus, offset;
    int     value, ret;
    int zid = pid - 1 ;   /*Turn to 0-base*/

    info->status |= ONLP_FAN_STATUS_PRESENT;

    /* get fan direction
     */
    info->status |= ONLP_FAN_STATUS_F2B;

    /* get fan speed
     */

    bus = pmbus_cfg[zid][0];
    offset = pmbus_cfg[zid][1];
    ret = onlp_file_read_int(&value, PSU_SYSFS_PATH"psu_fan1_speed_rpm", bus, offset);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read status from (%s)\r\n", PSU_SYSFS_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }

    info->rpm = value;
    info->percentage = (info->rpm * 100)/MAX_PSU_FAN_SPEED;

    /* get fan fault status
     */
    if (!info->rpm) {
        info->status |= ONLP_FAN_STATUS_FAILED;
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

static int
_onlp_fani_info_get_fan(int fid, onlp_fan_info_t* info)
{
    int   value;

#define USE_FAKE_FAN 1
#if USE_FAKE_FAN
    value = 11500;
    info->status |= ONLP_FAN_STATUS_PRESENT;
    info->percentage = (info->rpm * 100)/MAX_FAN_SPEED;
    info->status |=  ONLP_FAN_STATUS_F2B;
    info->rpm = value;
    info->percentage = (info->rpm * 100)/MAX_FAN_SPEED;
#else
    /* get fan present status
     */
    if (onlp_file_read_int(&value, "%s""fan%d_present", FAN_BOARD_PATH, fid) < 0) {
        AIM_LOG_ERROR("Unable to read status from (%s)\r\n", FAN_BOARD_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (value == 0) {
        return ONLP_STATUS_OK; /* fan is not present */
    }
    info->status |= ONLP_FAN_STATUS_PRESENT;


    /* get fan fault status (turn on when any one fails)
     */
    if (onlp_file_read_int(&value, "%s""fan%d_fault", FAN_BOARD_PATH, fid) < 0) {
        AIM_LOG_ERROR("Unable to read status from (%s)\r\n", FAN_BOARD_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (value > 0) {
        info->status |= ONLP_FAN_STATUS_FAILED;
    }


    /* get fan direction (both : the same)
     */
    if (onlp_file_read_int(&value, "%s""fan%d_direction", FAN_BOARD_PATH, fid) < 0) {
        AIM_LOG_ERROR("Unable to read status from (%s)\r\n", FAN_BOARD_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }

    info->status |= value ? ONLP_FAN_STATUS_B2F : ONLP_FAN_STATUS_F2B;


    /* get front fan speed
     */
    if (onlp_file_read_int(&value, "%s""fan%d_front_speed_rpm", FAN_BOARD_PATH, fid) < 0) {
        AIM_LOG_ERROR("Unable to read status from (%s)\r\n", FAN_BOARD_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }
    info->rpm = value;

    /* get rear fan speed
     */
    if (onlp_file_read_int(&value, "%s""fan%d_rear_speed_rpm", FAN_BOARD_PATH, fid) < 0) {
        AIM_LOG_ERROR("Unable to read status from (%s)\r\n", FAN_BOARD_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* take the min value from front/rear fan speed
     */
    if (info->rpm > value) {
        info->rpm = value;
    }

    /* get speed percentage from rpm
         */
    info->percentage = (info->rpm * 100)/MAX_FAN_SPEED;
#endif
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
    return ONLP_STATUS_E_INTERNAL;
}

