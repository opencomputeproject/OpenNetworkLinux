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
#include <onlplib/mmap.h>
#include <fcntl.h>
#include "platform_lib.h"

#define PREFIX_PATH_ON_MAIN_BOARD   "/sys/bus/i2c/devices/2-0066/"
#define PREFIX_PATH_ON_PSU          "/sys/bus/i2c/devices/"

#define MAX_FAN_SPEED     18000
#define MAX_PSU_FAN_SPEED 25500

#define PROJECT_NAME
#define LEN_FILE_NAME 80

#define FAN_RESERVED        0
#define FAN_1_ON_MAIN_BOARD 1
#define FAN_2_ON_MAIN_BOARD	2
#define FAN_3_ON_MAIN_BOARD	3
#define FAN_4_ON_MAIN_BOARD	4
#define FAN_5_ON_MAIN_BOARD	5
#define FAN_6_ON_MAIN_BOARD	6
#define FAN_1_ON_PSU1       7
#define FAN_1_ON_PSU2       8

enum fan_id {
        FAN_1_ON_FAN_BOARD = 1,
        FAN_2_ON_FAN_BOARD,
        FAN_3_ON_FAN_BOARD,
        FAN_4_ON_FAN_BOARD,
        FAN_5_ON_FAN_BOARD,
        FAN_1_ON_PSU_1,
        FAN_1_ON_PSU_2,
};

#define _MAKE_FAN_PATH_ON_MAIN_BOARD(prj,id) \
    { #prj"fan"#id"_present", #prj"fan"#id"_fault", #prj"fan"#id"_front_speed_rpm", \
      #prj"fan"#id"_direction", #prj"fan_duty_cycle_percentage", #prj"fan"#id"_rear_speed_rpm" }

#define MAKE_FAN_PATH_ON_MAIN_BOARD(prj,id) _MAKE_FAN_PATH_ON_MAIN_BOARD(prj,id)

#define MAKE_FAN_PATH_ON_PSU(folder) \
    {"", #folder"/psu_fan1_fault",  #folder"/psu_fan1_speed_rpm", \
     "", #folder"/psu_fan1_duty_cycle_percentage", ""  }

#define MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##id##_ON_MAIN_BOARD), "Chassis Fan "#id, 0 }, \
        0x0, \
        (ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }

#define MAKE_FAN_INFO_NODE_ON_PSU(psu_id, fan_id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fan_id##_ON_PSU##psu_id), "Chassis PSU-"#psu_id " Fan "#fan_id, 0 }, \
        0x0, \
        (ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }

/* Static fan information */
onlp_fan_info_t linfo[] = {
    { }, /* Not used */
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(1),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(2),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(3),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(4),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(5),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(6),
    MAKE_FAN_INFO_NODE_ON_PSU(1,1),
    MAKE_FAN_INFO_NODE_ON_PSU(2,1),
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
        int   value, ret;

        /* get fan present status
         */
        ret = onlp_file_read_int(&value, "%s""fan%d_present", FAN_BOARD_PATH, fid);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

        if (value == 0) {
                return ONLP_STATUS_OK;
        }
        info->status |= ONLP_FAN_STATUS_PRESENT;


    /* get fan fault status (turn on when any one fails)
     */
        ret = onlp_file_read_int(&value, "%s""fan%d_fault", FAN_BOARD_PATH, fid);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (value > 0) {
        info->status |= ONLP_FAN_STATUS_FAILED;
    }


    /* get fan direction (both : the same)
     */
        ret = onlp_file_read_int(&value, "%s""fan%d_direction", FAN_BOARD_PATH, fid);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

        info->status |= value ? ONLP_FAN_STATUS_B2F : ONLP_FAN_STATUS_F2B;


    /* get front fan speed
     */
        ret = onlp_file_read_int(&value, "%s""fan%d_front_speed_rpm", FAN_BOARD_PATH, fid);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
        info->rpm = value;

        /* get rear fan speed
         */
        ret = onlp_file_read_int(&value, "%s""fan%d_rear_speed_rpm", FAN_BOARD_PATH, fid);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

        /* take the min value from front/rear fan speed
         */
        if (info->rpm > value) {
        info->rpm = value;
    }

    /* get speed percentage from rpm
         */
        ret = onlp_file_read_int(&value, "%s""fan_max_speed_rpm", FAN_BOARD_PATH);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    info->percentage = (info->rpm * 100)/value;

        return ONLP_STATUS_OK;
}


static uint32_t
_onlp_get_fan_direction_on_psu(void)
{
    /* Try to read direction from PSU1.
     * If PSU1 is not valid, read from PSU2
     */
    int i = 0;

    for (i = PSU1_ID; i <= PSU2_ID; i++) {
        psu_type_t psu_type;
        psu_type = get_psu_type(i, NULL, 0);

        if (psu_type == PSU_TYPE_UNKNOWN) {
            continue;
        }

        if (PSU_TYPE_AC_F2B == psu_type) {
            return ONLP_FAN_STATUS_F2B;
        }
        else {
            return ONLP_FAN_STATUS_B2F;
        }
    }

    return 0;
}


static int
_onlp_fani_info_get_fan_on_psu(int pid, onlp_fan_info_t* info)
{
        int val = 0;

        info->status |= ONLP_FAN_STATUS_PRESENT;

    /* get fan direction
     */
    info->status |= _onlp_get_fan_direction_on_psu();

    /* get fan fault status
     */
    if (psu_pmbus_info_get(pid, "psu_fan1_fault", &val) == ONLP_STATUS_OK) {
        info->status |= (val > 0) ? ONLP_FAN_STATUS_FAILED : 0;
    }

    /* get fan speed
     */
    if (psu_pmbus_info_get(pid, "psu_fan1_speed_rpm", &val) == ONLP_STATUS_OK) {
        info->rpm = val;
            info->percentage = (info->rpm * 100) / MAX_PSU_FAN_SPEED;
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
    int local_id;
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);
    *info = linfo[local_id];

    switch (local_id)
    {
	    case FAN_1_ON_PSU1:
        case FAN_1_ON_PSU2:
            rc = _onlp_fani_info_get_fan_on_psu(local_id, info);
            break;
        case FAN_1_ON_MAIN_BOARD:
        case FAN_2_ON_MAIN_BOARD:
        case FAN_3_ON_MAIN_BOARD:
        case FAN_4_ON_MAIN_BOARD:
        case FAN_5_ON_MAIN_BOARD:
        case FAN_6_ON_MAIN_BOARD:
            rc =_onlp_fani_info_get_fan(local_id, info);
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
    char *path = NULL;

    VALIDATE(id);

    fid = ONLP_OID_ID_GET(id);

    /* reject p=0 (p=0, stop fan) */
    if (p == 0){
        return ONLP_STATUS_E_INVALID;
    }

    switch (fid)
        {
        case FAN_1_ON_PSU_1:
                        return psu_pmbus_info_set(PSU1_ID, "psu_fan1_duty_cycle_percentage", p);
        case FAN_1_ON_PSU_2:
                        return psu_pmbus_info_set(PSU2_ID, "psu_fan1_duty_cycle_percentage", p);
        case FAN_1_ON_FAN_BOARD:
        case FAN_2_ON_FAN_BOARD:
        case FAN_3_ON_FAN_BOARD:
        case FAN_4_ON_FAN_BOARD:
        case FAN_5_ON_FAN_BOARD:
                        path = FAN_NODE(fan_duty_cycle_percentage);
                        break;
        default:
            return ONLP_STATUS_E_INVALID;
    }

    if (onlp_file_write_int(p, path, NULL) != 0) {
        AIM_LOG_ERROR("Unable to write data to file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

        return ONLP_STATUS_OK;
}
