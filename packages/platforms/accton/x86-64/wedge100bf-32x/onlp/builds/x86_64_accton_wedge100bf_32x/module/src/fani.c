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
#include <onlp/platformi/base.h>
#include <onlp/platformi/fani.h>
#include "platform_lib.h"

#define MAX_FAN_SPEED    15400
#define BIT(i) (1 << (i))

enum fan_id {
  FAN_1_ON_FAN_BOARD = 1,
  FAN_2_ON_FAN_BOARD,
  FAN_3_ON_FAN_BOARD,
  FAN_4_ON_FAN_BOARD,
  FAN_5_ON_FAN_BOARD,
};

#define FAN_BOARD_PATH "/sys/bus/i2c/devices/8-0033/"

#define CHASSIS_FAN_INFO(fid)        \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fid##_ON_FAN_BOARD), "Chassis Fan - "#fid, 0 },\
        ONLP_FAN_DIR_F2B,\
        ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE,\
        0,\
        0,\
        "",\
        "",\
    }

/* Static fan information */
onlp_fan_info_t finfo[] = {
    { }, /* Not used */
    CHASSIS_FAN_INFO(1),
    CHASSIS_FAN_INFO(2),
    CHASSIS_FAN_INFO(3),
    CHASSIS_FAN_INFO(4),
    CHASSIS_FAN_INFO(5)
};


/**
 * @brief Software initialization of the Fan module.
 */
int onlp_fani_sw_init(void) {
  return ONLP_STATUS_OK;
}

/**
 * @brief Hardware initialization of the Fan module.
 * @param flags The hardware initialization flags.
 */
int onlp_fani_hw_init(uint32_t flags) {
    return ONLP_STATUS_OK;
}

/**
 * @brief Deinitialize the fan software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_fani_sw_denit(void) {
    return ONLP_STATUS_OK;
}

/**
 * @brief Retrieve the fan's OID hdr.
 * @param id The fan id.
 * @param[out] hdr Receives the OID header.
 */
int onlp_fani_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr) {
    onlp_fan_info_t info;
    onlp_fani_info_get(id, &info);
    *hdr = info.hdr;
    return ONLP_STATUS_OK;
}

/**
 * @brief Get the information structure for the given fan OID.
 * @param id The fan id
 * @param[out] rv Receives the fan information.
 */
int onlp_fani_info_get(onlp_oid_id_t id, onlp_fan_info_t* info) {
    
    int  value = 0, fid;
    char path[64] = {0};

    fid = ONLP_OID_ID_GET(id);
    *info = finfo[fid];

    /* get fan present status
     *
     * 0x0 Present
     * 0x1 Not present
     */
    sprintf(path, "%s""fantray_present", FAN_BOARD_PATH);

    if (bmc_file_read_int(&value, path, 16) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (value & BIT(fid-1)) {
        // Not present bit set for fan N.
        // Return latest info for the fan.
        return ONLP_STATUS_OK;
    }

    info->hdr.status |= ONLP_OID_STATUS_FLAG_PRESENT;

    /* get front fan rpm
     */
    sprintf(path, "%s""fan%d_input", FAN_BOARD_PATH, fid*2 - 1);

    if (bmc_file_read_int(&value, path, 10) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }
    info->rpm = value;

    /* get rear fan rpm
     */
    sprintf(path, "%s""fan%d_input", FAN_BOARD_PATH, fid*2);

    if (bmc_file_read_int(&value, path, 10) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* take the min value from front/rear fan speed
     */
    if (info->rpm > value) {
        info->rpm = value;
    }

    /* set fan status based on rpm
     */
    if (!info->rpm) {
        info->hdr.status |= ONLP_OID_STATUS_FLAG_FAILED;
        return ONLP_STATUS_OK;
    }

    /* get speed percentage from rpm
     */
    info->percentage = (info->rpm * 100)/MAX_FAN_SPEED;

    /* set fan direction
     */
    info->dir = ONLP_FAN_DIR_F2B;

    return ONLP_STATUS_OK;
}

/**
 * @brief Get the fan capabilities.
 * @param id The fan id.
 * @param[out] rv The fan capabilities
 */
int onlp_fani_caps_get(onlp_oid_id_t id, uint32_t* rv) {
    int fid = ONLP_OID_ID_GET(id);
    *rv = finfo[fid].caps;
    return ONLP_STATUS_OK;
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
    char cmd[32] = {0};

    sprintf(cmd, "set_fan_speed.sh %d", p);

    if (bmc_send_command(cmd) < 0) {
        AIM_LOG_ERROR("Unable to send command to bmc(%s)\r\n", cmd);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
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



