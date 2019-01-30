/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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
#include <fcntl.h>
#include <onlplib/file.h>
#include <onlplib/mmap.h>
#include <onlp/platformi/fani.h>
#include "platform_lib.h"
#include "mlnx_common/mlnx_common.h"

#define FAN_RESERVED        0
#define FAN_1_ON_MAIN_BOARD 1
#define FAN_2_ON_MAIN_BOARD 2
#define FAN_3_ON_MAIN_BOARD 3
#define FAN_4_ON_MAIN_BOARD 4
#define FAN_5_ON_MAIN_BOARD 5
#define FAN_6_ON_MAIN_BOARD 6
#define FAN_7_ON_MAIN_BOARD 7
#define FAN_8_ON_MAIN_BOARD 8
#define FAN_1_ON_PSU1       9
#define FAN_1_ON_PSU2       10

#define FIRST_PSU_FAN_ID 9

static int min_fan_speed[CHASSIS_FAN_COUNT+1] = {0};
static int max_fan_speed[CHASSIS_FAN_COUNT+1] = {0};

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
    MAKE_FAN_PATH_ON_PSU(1 ,1),
    MAKE_FAN_PATH_ON_PSU(2, 1)
};

/* Static fan information */
onlp_fan_info_t finfo[] = {
    { }, /* Not used */
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(1),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(2),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(3),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(4),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(5),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(6),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(7),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(8),
    MAKE_FAN_INFO_NODE_ON_PSU(1,1),
    MAKE_FAN_INFO_NODE_ON_PSU(2,1)
};

/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
    mlnx_platform_info_t* mlnx_platform_info = get_platform_info();
    mlnx_platform_info->min_fan_speed=min_fan_speed;
    mlnx_platform_info->max_fan_speed=max_fan_speed;
    mlnx_platform_info->finfo = finfo;
    mlnx_platform_info->fan_fnames = fan_path;
    mlnx_platform_info->fan_type = FAN_TYPE_EEPROM;
    mlnx_platform_info->fan_per_module = 2;
    mlnx_platform_info->first_psu_fan_id = FIRST_PSU_FAN_ID;
    return ONLP_STATUS_OK;
}
