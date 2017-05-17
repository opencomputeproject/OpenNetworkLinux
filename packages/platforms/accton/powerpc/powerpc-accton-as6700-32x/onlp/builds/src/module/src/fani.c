/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
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
#include <onlplib/mmap.h>
#include <fcntl.h>
#include "platform_lib.h"

#define PREFIX_PATH_ON_MAIN_BROAD  "/sys/class/hwmon/hwmon8/"
#define PREFIX_PATH_ON_PSU  "/sys/bus/i2c/devices/"

#define MAX_FAN_SPEED 15000
#define MAX_PSU_FAN_SPEED 18000

#define LOCAL_DEBUG 0


#define FAN_RESERVED         0
#define FAN_1_ON_MAIN_BROAD  1
#define FAN_2_ON_MAIN_BROAD  2
#define FAN_3_ON_MAIN_BROAD  3
#define FAN_4_ON_MAIN_BROAD  4
#define FAN_5_ON_MAIN_BROAD  5
#define FAN_1_ON_PSU1       6
#define FAN_1_ON_PSU2       7

#define PROJECT_NAME accton_as6700_32x_

#define LEN_FILE_NAME 40

typedef struct last_path_S
{
    char speed_rpm[LEN_FILE_NAME];
	char a_speed_rpm[LEN_FILE_NAME];
    char direction[LEN_FILE_NAME];
	char fault[LEN_FILE_NAME];
	char duty_cycle_level[LEN_FILE_NAME];
	char duty_cycle_percentage[LEN_FILE_NAME];
}last_path_T;

#define _MAKE_FAN_LAST_PATH_ON_MAIN_BROAD(prj,id) \
    { #prj"fan"#id"_speed_rpm", #prj"fan"#id"a_speed_rpm", #prj"fan"#id"_direction", "", #prj"fan_duty_cycle_level", ""}

#define MAKE_FAN_LAST_PATH_ON_MAIN_BROAD(prj,id) _MAKE_FAN_LAST_PATH_ON_MAIN_BROAD(prj,id)

#define MAKE_FAN_LAST_PATH_ON_PSU(folder) \
    { #folder"/psu_fan1_speed_rpm",  "", "", #folder"/psu_fan1_fault", "", #folder"/psu_fan1_duty_cycle_percentage"}

static last_path_T last_path[] =  /* must map with onlp_fan_id */
{
    MAKE_FAN_LAST_PATH_ON_MAIN_BROAD(PROJECT_NAME, FAN_RESERVED),
    MAKE_FAN_LAST_PATH_ON_MAIN_BROAD(PROJECT_NAME, FAN_1_ON_MAIN_BROAD),
    MAKE_FAN_LAST_PATH_ON_MAIN_BROAD(PROJECT_NAME, FAN_2_ON_MAIN_BROAD),
    MAKE_FAN_LAST_PATH_ON_MAIN_BROAD(PROJECT_NAME, FAN_3_ON_MAIN_BROAD),
    MAKE_FAN_LAST_PATH_ON_MAIN_BROAD(PROJECT_NAME, FAN_4_ON_MAIN_BROAD),
    MAKE_FAN_LAST_PATH_ON_MAIN_BROAD(PROJECT_NAME, FAN_5_ON_MAIN_BROAD),
    MAKE_FAN_LAST_PATH_ON_PSU(6-003e),
    MAKE_FAN_LAST_PATH_ON_PSU(6-003d),
};

#define MAKE_FAN_INFO_NODE_ON_MAIN_BROAD(id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##id##_ON_MAIN_BROAD), "Chassis Fan "#id, 0 }, \
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
        0, \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }


/* Static fan information */
onlp_fan_info_t linfo[] = {
    { }, /* Not used */
    MAKE_FAN_INFO_NODE_ON_MAIN_BROAD(1),
    MAKE_FAN_INFO_NODE_ON_MAIN_BROAD(2),
    MAKE_FAN_INFO_NODE_ON_MAIN_BROAD(3),
    MAKE_FAN_INFO_NODE_ON_MAIN_BROAD(4),
    MAKE_FAN_INFO_NODE_ON_MAIN_BROAD(5),
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
    if (LOCAL_DEBUG)                            \
        printf("[Debug][%s][%d][openfile: %s]\n", __FUNCTION__, __LINE__, fullpath); \
    if ((fd = open(fullpath, O_RDONLY)) == -1)  \
       return ONLP_STATUS_E_INTERNAL;           \
    if ((len = read(fd, r_data, nbytes)) <= 0){ \
        close(fd);                              \
        return ONLP_STATUS_E_INTERNAL;}         \
    if (LOCAL_DEBUG)                            \
        printf("[Debug][%s][%d][read data: %s]\n", __FUNCTION__, __LINE__, r_data); \
    if (close(fd) == -1)                        \
        return ONLP_STATUS_E_INTERNAL


/* PSU relative marco */
#define SET_PSU_TYPE_CPR_4011_F2B_FAN(info) \
    info->status = (ONLP_FAN_STATUS_PRESENT | ONLP_FAN_STATUS_F2B); \
    info->caps   = (ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE)

#define SET_PSU_TYPE_CPR_4011_B2F_FAN(info) \
    info->status = (ONLP_FAN_STATUS_PRESENT | ONLP_FAN_STATUS_B2F); \
    info->caps   = (ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE)

#define SET_PSU_TYPE_UM400D_F2B_FAN(info) \
    info->status = (ONLP_FAN_STATUS_PRESENT | ONLP_FAN_STATUS_F2B)

#define SET_PSU_TYPE_UM400D_B2F_FAN(info) \
    info->status = (ONLP_FAN_STATUS_PRESENT | ONLP_FAN_STATUS_B2F)


static int
_onlp_fani_info_get_fan(int local_id, onlp_fan_info_t* info)
{
    int   fd, len, nbytes = 10;
    char  r_data[10]   = {0};
    int t_ra_data = 0, t_r_data = 0;
    char  fullpath[65] = {0};

    /* get fan/fanr fault status (turn on when any one fails)
     */
    sprintf(fullpath, "%s%s", PREFIX_PATH_ON_MAIN_BROAD, last_path[local_id].speed_rpm);
    OPEN_READ_FILE(fd, fullpath, r_data, nbytes, len);

    t_ra_data = atoi(r_data);

    sprintf(fullpath, "%s%s", PREFIX_PATH_ON_MAIN_BROAD, last_path[local_id].a_speed_rpm);
    OPEN_READ_FILE(fd, fullpath, r_data, nbytes, len);

    t_r_data = atoi(r_data);

    if ( t_ra_data == 0 || t_r_data == 0 )
        info->status |= ONLP_FAN_STATUS_FAILED;

    if ( t_ra_data > 0 ||  t_r_data > 0 )
    {
        info->status |= ONLP_FAN_STATUS_PRESENT;
    }

    if ( t_ra_data >= t_r_data )
    {
        info->rpm = t_r_data;
    }
    else
    {
        info->rpm = t_ra_data;
    }

    sprintf(fullpath, "%s%s", PREFIX_PATH_ON_MAIN_BROAD, last_path[local_id].direction);
    OPEN_READ_FILE(fd, fullpath, r_data,nbytes, len);

    if (atoi(r_data) == 0) /*B2F*/
        info->status |= ONLP_FAN_STATUS_F2B;
    else
        info->status |= ONLP_FAN_STATUS_B2F;

	info->percentage = info->rpm * 100 / MAX_FAN_SPEED;

    return ONLP_STATUS_OK;
}


static int
_onlp_fani_info_get_fan_on_psu(int local_id, onlp_fan_info_t* info)
{
    int   psu_id, is_ac=0;
    int   fd, len, nbytes = 10;
    char  r_data[10]   = {0};
    char  fullpath[50] = {0};
    psu_type_t psu_type;

    /* get fan other cap status according to psu type
     */
    psu_id    = (local_id-FAN_1_ON_PSU1) + 1;

    if (LOCAL_DEBUG)
        printf("[Debug][%s][%d][psu_id: %d]\n", __FUNCTION__, __LINE__, psu_id);

    psu_type  = get_psu_type(psu_id, NULL, 0); /* psu_id = 1 , present PSU1. pus_id =2 , present PSU2 */

    if (LOCAL_DEBUG)
        printf("[Debug][%s][%d][psu_type: %d]\n", __FUNCTION__, __LINE__, psu_type);


    switch (psu_type) {
        case PSU_TYPE_AC_F2B:
            SET_PSU_TYPE_CPR_4011_F2B_FAN(info);
            is_ac = 1;
            break;
        case PSU_TYPE_AC_B2F:
            SET_PSU_TYPE_CPR_4011_B2F_FAN(info);
            is_ac = 1;
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

    if (1 == is_ac)
    {
        /* get fan fault status
         */
        sprintf(fullpath, "%s%s", PREFIX_PATH_ON_PSU, last_path[local_id].fault);
        OPEN_READ_FILE(fd ,fullpath, r_data, nbytes, len);
        if (atoi(r_data) > 0)
            info->status |= ONLP_FAN_STATUS_FAILED;

		/* get fan rpm
         */
        sprintf(fullpath, "%s%s", PREFIX_PATH_ON_PSU, last_path[local_id].speed_rpm);
        OPEN_READ_FILE(fd, fullpath, r_data, nbytes, len);
        info->rpm = atoi(r_data);
		info->percentage = info->rpm * 100 / MAX_PSU_FAN_SPEED;
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

    if (LOCAL_DEBUG)
        printf("\n[Debug][%s][%d][local_id: %d]", __FUNCTION__, __LINE__, local_id);

    switch (local_id)
    {
        case FAN_1_ON_PSU1:
        case FAN_1_ON_PSU2:
            rc = _onlp_fani_info_get_fan_on_psu(local_id, info);
            break;

        default:
            rc =_onlp_fani_info_get_fan(local_id, info);
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

    local_id = ONLP_OID_ID_GET(id);

	if (p == 0)  /* reject p=0 */
	{
		return ONLP_STATUS_E_INVALID;
	}

    /* get fullpath */
    switch (local_id)
    {
        case FAN_1_ON_PSU1:
        case FAN_1_ON_PSU2:
            sprintf(fullpath, "%s%s", PREFIX_PATH_ON_PSU, last_path[local_id].duty_cycle_percentage);
            break;
        default:
            sprintf(fullpath, "%s%s", PREFIX_PATH_ON_MAIN_BROAD, last_path[local_id].duty_cycle_level);

            /* normal fan power level and percentage.
             * set fan power level by fuzzy control method
             * {{0x00, 0}, {0x03, 37.5}, {0x04, 50}, {0x05, 62.5}, {0x06, 75}, {0x07, 100}};
             */
            if ( (0 < p) && (p < 38) )
            {
                p = 3;
            }
            else if ( (38 <= p) && (p < 50) )
            {
                p = 3;
            }
            else if ( (50 <= p) && (p < 63) )
            {
                p = 4;
            }
            else if ( (63 <= p) && (p < 75) )
            {
                p = 5;
            }
            else if ( (75 <= p) && (p < 100) )
            {
                p = 6;
            }
            else if ( p ==100 )
            {
                p = 7;
            }

            break;
    }

    sprintf(data, "%d", p);

    if (LOCAL_DEBUG)
        printf("[Debug][%s][%d][openfile: %s][data=%s]\n", __FUNCTION__, __LINE__, fullpath, data);

    /* Create output file descriptor */
    fd = open(fullpath, O_WRONLY,  0644);
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
