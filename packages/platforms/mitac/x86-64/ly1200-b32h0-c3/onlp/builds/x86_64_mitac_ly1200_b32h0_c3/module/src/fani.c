/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 MiTAC Computing Technology Corporation.
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

#define PREFIX_PATH_ON_MAIN_BOARD   "/sys/bus/i2c/devices/9-0066/"

#define MAX_FAN_SPEED     23000

#define PROJECT_NAME
#define LEN_FILE_NAME 80

#define FAN_RESERVED        0
#define FAN_1_ON_MAIN_BOARD	1
#define FAN_2_ON_MAIN_BOARD	2
#define FAN_3_ON_MAIN_BOARD	3
#define FAN_4_ON_MAIN_BOARD	4
#define FAN_5_ON_MAIN_BOARD	5
#define FAN_6_ON_MAIN_BOARD	6
#define FAN_7_ON_MAIN_BOARD	7
#define FAN_8_ON_MAIN_BOARD	8
#define FAN_9_ON_MAIN_BOARD	9
#define FAN_10_ON_MAIN_BOARD	10
#define FAN_11_ON_MAIN_BOARD	11
#define FAN_12_ON_MAIN_BOARD	12

typedef struct fan_path_S
{
    char present[LEN_FILE_NAME];
    char status[LEN_FILE_NAME];
    char speed[LEN_FILE_NAME];
	char direction[LEN_FILE_NAME];
    char ctrl_speed[LEN_FILE_NAME];
    char r_speed[LEN_FILE_NAME];
}fan_path_T;

#define _MAKE_FAN_PATH_ON_MAIN_BOARD(prj,id) \
    { #prj"fan"#id"_present", #prj"fan"#id"_fault", #prj"fan"#id"_front_speed_rpm", \
      #prj"fan"#id"_direction", #prj"fan"#id"_duty_cycle_percentage", #prj"fan"#id"_rear_speed_rpm" }

#define MAKE_FAN_PATH_ON_MAIN_BOARD(prj,id) _MAKE_FAN_PATH_ON_MAIN_BOARD(prj,id)

static fan_path_T fan_path[] =  /* must map with onlp_fan_id */
{
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_RESERVED),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_1_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_2_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_3_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_4_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_5_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_6_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_7_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_8_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_9_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_10_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_11_ON_MAIN_BOARD),
    MAKE_FAN_PATH_ON_MAIN_BOARD(PROJECT_NAME, FAN_12_ON_MAIN_BOARD)
};

#define MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##id##_ON_MAIN_BOARD), "Chassis Fan "#id, 0 }, \
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
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(7),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(8),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(9),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(10),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(11),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(12)
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

/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
    // FIXME: WARNING FREEZE
    if ( 0 ) {
        printf("%d\t%s", linfo[0].status, fan_path[0].speed);
    }
    // END WARNING FREEZE
    return ONLP_STATUS_OK;
}



