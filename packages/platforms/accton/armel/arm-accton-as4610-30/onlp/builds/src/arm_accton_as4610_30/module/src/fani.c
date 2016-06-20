/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
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
#include <fcntl.h>
#include "platform_lib.h"
#include "arm_accton_as4610_30_int.h"

#define PREFIX_PATH_ON_MAIN_BOARD   "/sys/devices/platform/as4610_fan/"
#define PREFIX_PATH_ON_PSU          "/sys/bus/i2c/devices/"

#define MAX_FAN_SPEED       13000
#define MAX_PSU_FAN_SPEED   25500

#define FILE_NAME_LEN       80

enum onlp_fan_id
{
    FAN_RESERVED = 0,
    FAN_1_ON_MAIN_BOARD,
    FAN_1_ON_PSU1,
    FAN_1_ON_PSU2
};

typedef struct fan_path_S
{
    char status[FILE_NAME_LEN];
    char speed[FILE_NAME_LEN];
    char ctrl_speed[FILE_NAME_LEN];
}fan_path_T;

#define _MAKE_FAN_PATH_ON_MAIN_BOARD(id) \
    { "fan"#id"_fault", "fan"#id"_speed_rpm", "fan_duty_cycle_percentage"}

#define MAKE_FAN_PATH_ON_MAIN_BOARD(id) _MAKE_FAN_PATH_ON_MAIN_BOARD(id)

#define MAKE_FAN_PATH_ON_PSU(folder) \
    {#folder"/psu_fan1_fault",  #folder"/psu_fan1_speed_rpm", \
     #folder"/psu_fan1_duty_cycle_percentage",}

static fan_path_T fan_path[] =  /* must map with onlp_fan_id */
{
    { }, /* Not used */
    MAKE_FAN_PATH_ON_MAIN_BOARD(2),
    MAKE_FAN_PATH_ON_PSU(8-0058),
    MAKE_FAN_PATH_ON_PSU(8-0059)
};

#define MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##id##_ON_MAIN_BOARD), "Chassis Fan "#id, 0 }, \
        ONLP_FAN_STATUS_F2B | ONLP_FAN_STATUS_PRESENT, \
        (ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }

#define MAKE_FAN_INFO_NODE_ON_PSU(psu_id, fan_id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fan_id##_ON_PSU##psu_id), "Chassis PSU-"#psu_id " Fan "#fan_id, 0 }, \
        ONLP_FAN_STATUS_F2B | ONLP_FAN_STATUS_PRESENT, \
        (ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }

/* Static fan information */
onlp_fan_info_t linfo[] = {
    { }, /* Not used */
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(1),
    MAKE_FAN_INFO_NODE_ON_PSU(1,1),
    MAKE_FAN_INFO_NODE_ON_PSU(2,1),
};

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

#define OPEN_READ_FILE(fd,fullpath,data,nbytes,len) \
    DEBUG_PRINT("[Debug][%s][%d][openfile: %s]\n", __FUNCTION__, __LINE__, fullpath); \
    if ((fd = open(fullpath, O_RDONLY)) == -1)  \
       return ONLP_STATUS_E_INTERNAL;           \
    if ((len = read(fd, r_data, nbytes)) <= 0){ \
        close(fd);                              \
        return ONLP_STATUS_E_INTERNAL;}         \
    DEBUG_PRINT("[Debug][%s][%d][read data: %s]\n", __FUNCTION__, __LINE__, r_data); \
    if (close(fd) == -1)                        \
        return ONLP_STATUS_E_INTERNAL

static int
_onlp_fani_info_get_fan(int local_id, onlp_fan_info_t* info)
{
    int   fd, len, nbytes = 10;
    char  r_data[10]   = {0};
    char  fullpath[65] = {0};

    /* get fan fault status (turn on when any one fails)
     */
    sprintf(fullpath, "%s%s", PREFIX_PATH_ON_MAIN_BOARD, fan_path[local_id].status);
    OPEN_READ_FILE(fd, fullpath, r_data, nbytes, len);
    if (atoi(r_data) > 0) {
        info->status |= ONLP_FAN_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }  

    /* get fan speed
     */
    sprintf(fullpath, "%s%s", PREFIX_PATH_ON_MAIN_BOARD, fan_path[local_id].speed);
    OPEN_READ_FILE(fd, fullpath, r_data, nbytes, len);
    info->rpm = atoi(r_data);

    /* get speed percentage from rpm */
    info->percentage = (info->rpm * 100) / MAX_FAN_SPEED;

    return ONLP_STATUS_OK;
}

static int
_onlp_fani_info_get_fan_on_psu(int local_id, onlp_fan_info_t* info)
{
    int   psu_id;
    int   fd, len, nbytes = 10;
    char  r_data[10]   = {0};
    char  fullpath[80] = {0};
    psu_type_t psu_type;

    /* get fan fault status
     */
    psu_id   = (local_id - FAN_1_ON_PSU1) + 1;
    DEBUG_PRINT("[Debug][%s][%d][psu_id: %d]\n", __FUNCTION__, __LINE__, psu_id);

    psu_type = get_psu_type(psu_id, NULL, 0); /* psu_id = 1 , present PSU1. pus_id =2 , present PSU2 */
    DEBUG_PRINT("[Debug][%s][%d][psu_type: %d]\n", __FUNCTION__, __LINE__, psu_type);

    switch (psu_type) {
        case PSU_TYPE_AC_F2B:
            info->status |= (ONLP_FAN_STATUS_PRESENT | ONLP_FAN_STATUS_F2B);
            break;
        case PSU_TYPE_AC_B2F:
            info->status |= (ONLP_FAN_STATUS_PRESENT | ONLP_FAN_STATUS_B2F);
            break;
        default:
            return ONLP_STATUS_E_UNSUPPORTED;
    }

    /* get fan fault status
     */
    sprintf(fullpath, "%s%s", PREFIX_PATH_ON_PSU, fan_path[local_id].status);
    OPEN_READ_FILE(fd, fullpath, r_data, nbytes, len);
    if (atoi(r_data) > 0) {
        info->status |= ONLP_FAN_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }

    /* get fan speed
     */
    sprintf(fullpath, "%s%s", PREFIX_PATH_ON_PSU, fan_path[local_id].speed);
    OPEN_READ_FILE(fd, fullpath, r_data, nbytes, len);
    info->rpm = atoi(r_data);

    /* get speed percentage from rpm */
    info->percentage = (info->rpm * 100) / MAX_PSU_FAN_SPEED;

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
    if (chassis_fan_count() == 0) {
        local_id += 1;
    }
    
    *info = linfo[local_id];

    switch (local_id)
    {
        case FAN_1_ON_PSU1:
        case FAN_1_ON_PSU2:
            rc = _onlp_fani_info_get_fan_on_psu(local_id, info);
            break;
        case FAN_1_ON_MAIN_BOARD:
            rc =_onlp_fani_info_get_fan(local_id, info);
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
    int  fd, len, nbytes=10, local_id;
    char data[10] = {0};
    char fullpath[70] = {0};

    VALIDATE(id);

    if (chassis_fan_count() == 0) {
        return ONLP_STATUS_E_INVALID;
    }

    local_id = ONLP_OID_ID_GET(id);

    /* reject p=0 (p=0, stop fan) */
    if (p == 0){
        return ONLP_STATUS_E_INVALID;
    }

    /* get fullpath */
    switch (local_id)
	{
        case FAN_1_ON_PSU1:
        case FAN_1_ON_PSU2:
            sprintf(fullpath, "%s%s", PREFIX_PATH_ON_PSU, fan_path[local_id].ctrl_speed);
            break;
        case FAN_1_ON_MAIN_BOARD:
            sprintf(fullpath, "%s%s", PREFIX_PATH_ON_MAIN_BOARD, fan_path[local_id].ctrl_speed);
            break;
        default:
            return ONLP_STATUS_E_INVALID;
    }
    sprintf(data, "%d", p);
    DEBUG_PRINT("[Debug][%s][%d][openfile: %s][data=%s]\n", __FUNCTION__, __LINE__, fullpath, data);

    /* Create output file descriptor */
    fd = open(fullpath, O_WRONLY,  0644);
    if (fd == -1){
        return ONLP_STATUS_E_INTERNAL;
    }

    len = write (fd, data, (ssize_t) nbytes);
    if (len != nbytes) {
        close(fd);
        return ONLP_STATUS_E_INTERNAL;
    }

    close(fd);
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

