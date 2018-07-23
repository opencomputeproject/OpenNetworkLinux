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
 ************************************************************/
#include <onlp/platformi/base.h>
#include "platform_lib.h"

#define SYS_I2C_DEVICES             "/sys/bus/i2c/devices"
#define SYSFS_FAN_DIR               "/sys/bus/i2c/devices/2-0066"

#define PREFIX_PATH_ON_MAIN_BOARD   "/sys/bus/i2c/devices/2-0066/"
#define PSU_SYSFS_DIR               "/sys/bus/i2c/devices/"

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

typedef struct fan_path_S
{
    char present[LEN_FILE_NAME];
    char status[LEN_FILE_NAME];
    char speed[LEN_FILE_NAME];
    char direction[LEN_FILE_NAME];
    char ctrl_speed[LEN_FILE_NAME];
    char r_speed[LEN_FILE_NAME];

}fan_path_T;

#define _MAKE_FAN_PATH_ON_MAIN_BOARD(prj,id)                            \
    {                                                                   \
        .present = #prj"fan"#id"_present",                              \
            .status = #prj"fan"#id"_fault",                             \
            .speed = #prj"fan"#id"_front_speed_rpm",                    \
            .direction = #prj"fan"#id"_direction",                      \
            .ctrl_speed = #prj"fan_duty_cycle_percentage",              \
            .r_speed = #prj"fan"#id"_rear_speed_rpm",                   \
            }

#define MAKE_FAN_PATH_ON_MAIN_BOARD(prj,id) _MAKE_FAN_PATH_ON_MAIN_BOARD(prj,id)

#define MAKE_FAN_PATH_ON_PSU(folder) \
    {"", #folder"/psu_fan1_fault",  #folder"/psu_fan1_speed_rpm", \
     "", #folder"/psu_fan1_duty_cycle_percentage", ""  }

static fan_path_T fan_path[] =  /* must map with onlp_fan_id */
{
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_RESERVED),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_1_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_2_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_3_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_4_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_5_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_6_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_PSU(11-005b),
    MAKE_FAN_PATH_ON_PSU(10-0058)
};





#define MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(_id)                           \
    {                                                                   \
        .hdr = {                                                        \
            .id = ONLP_FAN_ID_CREATE(FAN_##_id##_ON_MAIN_BOARD),        \
            .description = "Chassis Fan "#_id,                          \
            .poid = ONLP_OID_CHASSIS,                                   \
        },                                                              \
        .dir = ONLP_FAN_DIR_UNKNOWN,                                    \
        .caps = (ONLP_FAN_CAPS_GET_DIR | ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
    }

#define MAKE_FAN_INFO_NODE_ON_PSU(psu_id, fan_id)                       \
    {                                                                   \
        .hdr = {                                                        \
            .id = ONLP_FAN_ID_CREATE(FAN_##fan_id##_ON_PSU##psu_id),    \
            .description = "Chassis PSU-"#psu_id " Fan "#fan_id,        \
            .poid = ONLP_PSU_ID_CREATE(PSU##psu_id##_ID)                         \
        },                                                              \
        .dir = ONLP_FAN_DIR_UNKNOWN,                                    \
        .caps = (ONLP_FAN_CAPS_GET_DIR | ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
    }

/* Static fan information */
onlp_fan_info_t fan_info[] = {
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

static int
chassis_fani_hdr_get__(int id, onlp_oid_hdr_t* hdr)
{
    int v;

    *hdr = fan_info[id].hdr;
    ONLP_TRY(onlp_file_read_int(&v, "%s/fan%d_present",
                                SYSFS_FAN_DIR, id));
    if(v) {
        /* Fan is present */
        ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
        ONLP_TRY(onlp_file_read_int(&v, "%s/fan%d_fault",
                                    SYSFS_FAN_DIR, id));
        if(v) {
            /* Fan has failed. */
            ONLP_OID_STATUS_FLAG_SET(hdr, FAILED);
        }
    }
    return 0;
}

static int
chassis_fani_info_get__(int id, onlp_fan_info_t* info)
{
    *info = fan_info[id];
    ONLP_TRY(chassis_fani_hdr_get__(id, &info->hdr));

    /*
     * Get fan/fanr direction (both : the same)
     */
    int v;
    ONLP_TRY(onlp_file_read_int(&v, "%s/fan%d_direction",
                                SYSFS_FAN_DIR, id));
    info->dir = (v) ? ONLP_FAN_DIR_F2B : ONLP_FAN_DIR_B2F;

    /*
     * Get fan speed (take the min from two speeds)
     */
    ONLP_TRY(onlp_file_read_int(&info->rpm, "%s/fan%d_front_speed_rpm",
                                SYSFS_FAN_DIR, id));
    ONLP_TRY(onlp_file_read_int(&v, "%s/fan%d_rear_speed_rpm",
                                SYSFS_FAN_DIR, id));
    if(v < info->rpm) {
        info->rpm = v;
    }

    /* get speed percentage from rpm */
    info->percentage = (info->rpm * 100)/MAX_FAN_SPEED;

    return ONLP_STATUS_OK;
}

static int
psu_fani_hdr_get__(int id, const char* device, onlp_oid_hdr_t* hdr)
{
    int v;
    int rv;
    *hdr = fan_info[id].hdr;
    if(ONLP_SUCCESS(rv = onlp_file_read_int(&v,
                                            "%s/%s/psu_fan1_fault",
                                            SYS_I2C_DEVICES,
                                            device))) {
        ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
        if(v) {
            ONLP_OID_STATUS_FLAG_SET(hdr, FAILED);
        }
    }
    return rv;
}

static int
psu_fani_info_get__(int id, const char* device, onlp_fan_info_t* info)
{
    *info = fan_info[id];
    ONLP_TRY(psu_fani_hdr_get__(id, device, &info->hdr));

    if(ONLP_OID_STATUS_FLAG_NOT_SET(info, PRESENT) ||
       ONLP_OID_STATUS_FLAG_IS_SET(info, FAILED)) {
        return 0;
    }

    /*
     * Direction.
     */
    char* dir = NULL;
    ONLP_TRY(onlp_file_read_str(&dir,
                                "%s/%s/psu_fan_dir",
                                SYS_I2C_DEVICES,
                                device));
    if(dir) {
        if(!strcmp(dir, "F2B\n")) {
            info->dir = ONLP_FAN_DIR_F2B;
        }
        else if(!strcmp(dir, "B2F\n")) {
            info->dir = ONLP_FAN_DIR_B2F;
        }
        else {
            info->dir = ONLP_FAN_DIR_UNKNOWN;
        }
        aim_free(dir);
    }

    ONLP_TRY(onlp_file_read_int(&info->rpm, "%s/%s/psu_fan1_speed_rpm",
                                SYS_I2C_DEVICES, device));

    info->percentage = (info->rpm * 100) / MAX_PSU_FAN_SPEED;
    return ONLP_STATUS_OK;
}

int
onlp_fani_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    int id = ONLP_OID_ID_GET(oid);
    switch(id)
        {
        case FAN_1_ON_PSU1:
            return psu_fani_hdr_get__(id, "11-005b", hdr);
        case FAN_1_ON_PSU2:
            return psu_fani_hdr_get__(id, "10-0058", hdr);
        case FAN_1_ON_MAIN_BOARD:
        case FAN_2_ON_MAIN_BOARD:
        case FAN_3_ON_MAIN_BOARD:
        case FAN_4_ON_MAIN_BOARD:
        case FAN_5_ON_MAIN_BOARD:
        case FAN_6_ON_MAIN_BOARD:
            return chassis_fani_hdr_get__(id, hdr);
        default:
            return ONLP_STATUS_E_PARAM;
        }
}


int
onlp_fani_info_get(onlp_oid_t oid, onlp_fan_info_t* info)
{
    int id = ONLP_OID_ID_GET(oid);
    switch(id)
        {
        case FAN_1_ON_PSU1:
            return psu_fani_info_get__(id, "11-005b", info);
        case FAN_1_ON_PSU2:
            return psu_fani_info_get__(id, "10-0058", info);
        case FAN_1_ON_MAIN_BOARD:
        case FAN_2_ON_MAIN_BOARD:
        case FAN_3_ON_MAIN_BOARD:
        case FAN_4_ON_MAIN_BOARD:
        case FAN_5_ON_MAIN_BOARD:
        case FAN_6_ON_MAIN_BOARD:
            return chassis_fani_info_get__(id, info);
        default:
            return ONLP_STATUS_E_PARAM;
        }
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
            sprintf(fullpath, "%s%s", SYS_I2C_DEVICES, fan_path[local_id].ctrl_speed);
            break;
        case FAN_1_ON_MAIN_BOARD:
        case FAN_2_ON_MAIN_BOARD:
        case FAN_3_ON_MAIN_BOARD:
        case FAN_4_ON_MAIN_BOARD:
        case FAN_5_ON_MAIN_BOARD:
        case FAN_6_ON_MAIN_BOARD:
            sprintf(fullpath, "%s%s", PREFIX_PATH_ON_MAIN_BOARD, fan_path[local_id].ctrl_speed);
            break;
        default:
            return ONLP_STATUS_E_INVALID;
    }
    sprintf(data, "%d", p);

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
