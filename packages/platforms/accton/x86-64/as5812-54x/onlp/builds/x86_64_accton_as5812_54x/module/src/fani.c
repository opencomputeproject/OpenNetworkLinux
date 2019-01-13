/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2015 Accton Technology Corporation.
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
#include <unistd.h>
#include <onlplib/file.h>
#include <fcntl.h>
#include "platform_lib.h"
#include "x86_64_accton_as5812_54x/x86_64_accton_as5812_54x_config.h"

#define PREFIX_PATH_ON_MAIN_BOARD  "/sys/devices/platform/as5812_54x_fan/"
#define PREFIX_PATH_ON_PSU  "/sys/bus/i2c/devices/"
#define LOCAL_DEBUG 0


#define FAN_RESERVED        0
#define FAN_1_ON_MAIN_BOARD 1
#define FAN_2_ON_MAIN_BOARD	2
#define FAN_3_ON_MAIN_BOARD	3
#define FAN_4_ON_MAIN_BOARD	4
#define FAN_5_ON_MAIN_BOARD	5
#define FAN_1_ON_PSU1       6
#define FAN_1_ON_PSU2       7

#define PROJECT_NAME

#define LEN_FILE_NAME 50

typedef struct last_path_S
{
    char status[LEN_FILE_NAME];
    char speed[LEN_FILE_NAME];
    char direction[LEN_FILE_NAME];
    char ctrl_speed[LEN_FILE_NAME];
    char r_status[LEN_FILE_NAME];
    char r_speed[LEN_FILE_NAME];
}last_path_T;

#define _MAKE_FAN_LAST_PATH_ON_MAIN_BOARD(prj,id) \
    { #prj"fan"#id"_fault", #prj"fan"#id"_speed_rpm", #prj"fan"#id"_direction",\
      #prj"fan"#id"_duty_cycle_percentage", #prj"fanr"#id"_fault", #prj"fanr"#id"_speed_rpm" }

#define MAKE_FAN_LAST_PATH_ON_MAIN_BOARD(prj,id) _MAKE_FAN_LAST_PATH_ON_MAIN_BOARD(prj,id)


#define MAKE_FAN_LAST_PATH_ON_PSU(folder) \
    {#folder"/psu_fan1_fault",  #folder"/psu_fan1_speed_rpm", "",\
     #folder"/psu_fan1_duty_cycle_percentage", "", ""  }

static last_path_T last_path[] =  /* must map with onlp_fan_id */
{
    MAKE_FAN_LAST_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_RESERVED),
    MAKE_FAN_LAST_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_1_ON_MAIN_BOARD),
    MAKE_FAN_LAST_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_2_ON_MAIN_BOARD),
    MAKE_FAN_LAST_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_3_ON_MAIN_BOARD),
    MAKE_FAN_LAST_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_4_ON_MAIN_BOARD),
    MAKE_FAN_LAST_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_5_ON_MAIN_BOARD),

    MAKE_FAN_LAST_PATH_ON_PSU(57-003c),
    MAKE_FAN_LAST_PATH_ON_PSU(58-003f),
};

#define MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##id##_ON_MAIN_BOARD), "Chassis Fan "#id, ONLP_OID_CHASSIS }, \
        0x0, \
        ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE, \
        0, \
        0, \
     }

#define MAKE_FAN_INFO_NODE_ON_PSU(psu_id, fan_id) \
    { \
    { ONLP_FAN_ID_CREATE(FAN_##fan_id##_ON_PSU##psu_id), "Chassis PSU-"#psu_id " Fan "#fan_id, ONLP_PSU_ID_CREATE(psu_id) }, \
        0x0, \
        0, \
        0, \
        0, \
    }

/* Static fan information */
static onlp_fan_info_t finfo[] = {
    { }, /* Not used */
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(1),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(2),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(3),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(4),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(5),
    MAKE_FAN_INFO_NODE_ON_PSU(1,1),
    MAKE_FAN_INFO_NODE_ON_PSU(2,1),
};


/* PSU relative marco */
#define SET_PSU_TYPE_AC_F2B_FAN(info)                                   \
    do {                                                                \
        ONLP_OID_STATUS_FLAG_SET(info, PRESENT);                        \
        info->dir = ONLP_FAN_DIR_F2B;                                   \
        info->caps  |=  ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE; \
    } while(0)

#define SET_PSU_TYPE_AC_B2F_FAN(info)                                   \
    do {                                                                \
        ONLP_OID_STATUS_FLAG_SET(info, PRESENT);                        \
        info->dir = ONLP_FAN_DIR_B2F;                                   \
        info->caps |=  ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE; \
    } while(0)

#define SET_PSU_TYPE_UM400D_F2B_FAN(info)               \
    do {                                                \
        ONLP_OID_STATUS_FLAG_SET(info, PRESENT);        \
        info->dir = ONLP_FAN_DIR_F2B;                   \
    } while(0)

#define SET_PSU_TYPE_UM400D_B2F_FAN(info) \
    do { \
        ONLP_OID_STATUS_FLAG_SET(info, PRESENT); \
        info->dir = ONLP_FAN_DIR_B2F; \
    } while(0)


int
onlp_fani_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, 1, AIM_ARRAYSIZE(finfo) - 1);
}

static int
_onlp_fani_info_get_fan(int local_id, onlp_fan_info_t* info)
{
    int v;

    /* get fan/fanr fault status (turn on when any one fails)
     */
    ONLP_TRY(onlp_file_read_int(&v, "%s%s", PREFIX_PATH_ON_MAIN_BOARD, last_path[local_id].status));
    if (v > 0) {
        ONLP_OID_STATUS_FLAG_SET(info, FAILED);
    }

    ONLP_TRY(onlp_file_read_int(&v, "%s%s", PREFIX_PATH_ON_MAIN_BOARD, last_path[local_id].r_status));
    if(v > 0) {
        ONLP_OID_STATUS_FLAG_SET(info, FAILED);
    }

    /* get fan/fanr direction (both : the same)
     */
    ONLP_TRY(onlp_file_read_int(&v, "%s%s", PREFIX_PATH_ON_MAIN_BOARD, last_path[local_id].direction));
    if (v == 0) {
        /*F2B*/
        info->dir = ONLP_FAN_DIR_F2B;
    }
    else {
        info->dir = ONLP_FAN_DIR_B2F;
    }

    /* get fan/fanr speed (take the average of two speeds)
     */
    ONLP_TRY(onlp_file_read_int(&info->rpm, "%s%s", PREFIX_PATH_ON_MAIN_BOARD, last_path[local_id].speed));
    ONLP_TRY(onlp_file_read_int(&v, "%s%s", PREFIX_PATH_ON_MAIN_BOARD, last_path[local_id].r_speed));
    info->rpm = (info->rpm + v)/2;

    /* get speed percentage from rpm */
    info->percentage = (info->rpm * 100)/x86_64_accton_as5812_54x_CONFIG_SYS_FAN_FRONT_RPM_MAX;

    /* check present */
    if (info->rpm > 0) {
        ONLP_OID_STATUS_FLAG_SET(info, PRESENT);
    }

    return ONLP_STATUS_OK;
}

static int
_onlp_fani_info_get_fan_on_psu_ym2401(int pid, onlp_fan_info_t* info)
{
    int val = 0;

    /* get fan status
     */
    if (psu_ym2401_pmbus_info_get(pid, "psu_fan1_speed_rpm", &val) == ONLP_STATUS_OK) {
        ONLP_OID_STATUS_FLAG_SET_VALUE(info, FAILED, val <= 0);
        info->rpm = val;
        info->percentage = (info->rpm * 100) / 21600;
    }

    return ONLP_STATUS_OK;
}

static int
_onlp_fani_info_get_fan_on_psu(int local_id, onlp_fan_info_t* info)
{
    int v;
    int   psu_id;
    psu_type_t psu_type;

    /* get fan other cap status according to psu type
     */
    psu_id    = (local_id-FAN_1_ON_PSU1) + 1;

    psu_type  = get_psu_type(psu_id, NULL, 0); /* psu_id = 1 , present PSU1. pus_id =2 , present PSU2 */

    switch (psu_type) {
        case PSU_TYPE_AC_COMPUWARE_F2B:
        case PSU_TYPE_AC_3YPOWER_F2B:
            SET_PSU_TYPE_AC_F2B_FAN(info);
            break;
        case PSU_TYPE_AC_COMPUWARE_B2F:
        case PSU_TYPE_AC_3YPOWER_B2F:
            SET_PSU_TYPE_AC_B2F_FAN(info);
            break;
        case PSU_TYPE_DC_48V_F2B:
            SET_PSU_TYPE_UM400D_F2B_FAN(info);
            break;
        case PSU_TYPE_DC_48V_B2F:
            SET_PSU_TYPE_UM400D_B2F_FAN(info);
            break;
        default:
            if (LOCAL_DEBUG)
                printf("[Debug][%s][%d][psu_type=%d]\n", __FUNCTION__, __LINE__, psu_type);

            break;
    }

    if (psu_type == PSU_TYPE_AC_COMPUWARE_F2B ||
        psu_type == PSU_TYPE_AC_COMPUWARE_B2F  )
    {
        /* get fan fault status
         */
        ONLP_TRY(onlp_file_read_int(&v, "%s%s", PREFIX_PATH_ON_PSU, last_path[local_id].status));
        if (v > 0) {
            ONLP_OID_STATUS_FLAG_SET(info, FAILED);
        }

        /* get fan speed
         */
        ONLP_TRY(onlp_file_read_int(&info->rpm, "%s%s", PREFIX_PATH_ON_PSU, last_path[local_id].speed));
        /* get speed percentage from rpm */
        info->percentage = (info->rpm * 100)/19328;
    }
    else if (psu_type == PSU_TYPE_AC_3YPOWER_F2B ||
            psu_type == PSU_TYPE_AC_3YPOWER_B2F  )
    {
        return _onlp_fani_info_get_fan_on_psu_ym2401(psu_id, info);
    }

    return ONLP_STATUS_OK;
}


int
onlp_fani_info_get(onlp_oid_id_t id, onlp_fan_info_t* info)
{
    int rc = 0;

    *info = finfo[id];

    switch (id)
        {
        case FAN_1_ON_PSU1:
        case FAN_1_ON_PSU2:
            rc = _onlp_fani_info_get_fan_on_psu(id, info);
            break;

        default:
            rc =_onlp_fani_info_get_fan(id, info);
            break;
    }

    return rc;
}

int
onlp_fani_percentage_set(onlp_oid_id_t id, int p)
{
    int  fd, len, nbytes=10;
    char data[10] = {0};
    char fullpath[70] = {0};

    /* reject p=0 (p=0, stop fan) */
    if (p == 0){
        return ONLP_STATUS_E_INVALID;
    }

    /* get fullpath */
    switch (id)
	{
        case FAN_1_ON_PSU1:
        case FAN_1_ON_PSU2:
        {
            int psu_id;
            psu_type_t psu_type;

            psu_id   = id - FAN_1_ON_PSU1 + 1;
            psu_type = get_psu_type(psu_id, NULL, 0);

            if (psu_type == PSU_TYPE_AC_3YPOWER_F2B ||
                psu_type == PSU_TYPE_AC_3YPOWER_B2F  ) {
                return psu_ym2401_pmbus_info_set(psu_id, "psu_fan1_duty_cycle_percentage", p);
            }

            sprintf(fullpath, "%s%s", PREFIX_PATH_ON_PSU, last_path[id].ctrl_speed);
            break;
        }

        default:
            sprintf(fullpath, "%s%s", PREFIX_PATH_ON_MAIN_BOARD, last_path[id].ctrl_speed);
            break;
    }

    sprintf(data, "%d", p);

    if (LOCAL_DEBUG)
        printf("[Debug][%s][%d][openfile: %s][data=%s]\n", __FUNCTION__, __LINE__, fullpath, data);

    /* Create output file descriptor */
    fd = open(fullpath, O_WRONLY, 0644);
    if(fd == -1){
        return ONLP_STATUS_E_INTERNAL;
    }

    len = write (fd, data, (ssize_t) nbytes);
    if(len != nbytes){
        close(fd);
        return ONLP_STATUS_E_INTERNAL;
    }

    close(fd);
    return ONLP_STATUS_OK;
}
