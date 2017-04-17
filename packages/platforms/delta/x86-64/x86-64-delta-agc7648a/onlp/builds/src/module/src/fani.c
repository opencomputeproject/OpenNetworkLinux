/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017 Delta Networks, Inc.
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
#include <onlplib/i2c.h>

typedef struct fan_path_S
{
    char *status;
    char *speed;
    char *ctrl_speed;
}fan_path_T;

static fan_path_T fan_path[] =  /* must map with onlp_fan_id in platform_lib.h */
{
    { NULL, NULL, NULL },
    { "3-002c/fan1_fault", "3-002c/fan1_input", "3-002c/fan1_input" },
    { "3-002c/fan2_fault", "3-002c/fan2_input", "3-002c/fan2_input" },
    { "3-002c/fan3_fault", "3-002c/fan3_input", "3-002c/fan3_input" },
    { "3-002c/fan4_fault", "3-002c/fan4_input", "3-002c/fan4_input" },
    { "3-002d/fan1_fault", "3-002d/fan1_input", "3-002d/fan1_input" },
    { "3-002d/fan2_fault", "3-002d/fan2_input", "3-002d/fan2_input" },
    { "3-002d/fan3_fault", "3-002d/fan3_input", "3-002d/fan3_input" },
    { "3-002d/fan4_fault", "3-002d/fan4_input", "3-002d/fan4_input" },
    { "4-0058/psu_fan1_fault", "4-0058/psu_fan1_speed_rpm", "4-0058/psu_fan1_duty_cycle_percentage" },
    { "4-0058/psu_fan1_fault", "4-0058/psu_fan1_speed_rpm", "4-0058/psu_fan1_duty_cycle_percentage" }
};

#define MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##id##_ON_MAIN_BOARD), "Chassis Fan "#id, 0 }, \
        0x0, \
        (ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM), \
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
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(7),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(8),
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
dni_fani_info_get_fan(int local_id, onlp_fan_info_t* info)
{
    int rpm = 0;
    char fullpath[100] = {0};

    /* Update fan rpm */
    sprintf(fullpath, "%s/%s", SYS_DEV_PATH, fan_path[local_id].speed);
    rpm = dni_i2c_lock_read_attribute(NULL, fullpath);

    /* If rpm equal FAN_ZERO_TACH meaning 0 */
    if(rpm == FAN_ZERO_TACH)
    {
        rpm = 0;
    }
    info->rpm = rpm;

    /* Update fan speed percentage based from rpm */
    switch(local_id)
    {
    case FAN_1_ON_MAIN_BOARD:
    case FAN_2_ON_MAIN_BOARD:
    case FAN_3_ON_MAIN_BOARD:
    case FAN_4_ON_MAIN_BOARD:
        info->percentage = (info->rpm * 100)/MAX_FRONT_FAN_SPEED;
        break;
    case FAN_5_ON_MAIN_BOARD:
    case FAN_6_ON_MAIN_BOARD:
    case FAN_7_ON_MAIN_BOARD:
    case FAN_8_ON_MAIN_BOARD:
        info->percentage = (info->rpm * 100)/MAX_REAR_FAN_SPEED;
        break;
    default:
        AIM_LOG_ERROR("Error local_id");
        break;
    }

    /* Update present bit of fan status */ 
    mux_info_t mux_info;
    mux_info.bus = I2C_BUS_5;
    mux_info.addr = SWPLD;
    mux_info.offset = FAN_MUX_REG;
    mux_info.channel = 0x07;

    dev_info_t dev_info;
    dev_info.bus = I2C_BUS_3;
    dev_info.addr = FAN_IO_CTRL;
    dev_info.offset = 0x00;
    dev_info.size = 1;
    dev_info.flags = 0;

    int fantray_present_val = dni_i2c_lock_read(&mux_info, &dev_info);

    switch(local_id)
    {
        case FAN_1_ON_MAIN_BOARD:
        case FAN_5_ON_MAIN_BOARD:
            if((fantray_present_val & 0x01) == 0x00)
                info->status |= ONLP_FAN_STATUS_PRESENT;
            break;
        case FAN_2_ON_MAIN_BOARD:
        case FAN_6_ON_MAIN_BOARD:
            if((fantray_present_val & 0x02) == 0x00)
                info->status |= ONLP_FAN_STATUS_PRESENT;
            break;
        case FAN_3_ON_MAIN_BOARD:
        case FAN_7_ON_MAIN_BOARD:
            if((fantray_present_val & 0x04) == 0x00)
                info->status |= ONLP_FAN_STATUS_PRESENT;
            break;
        case FAN_4_ON_MAIN_BOARD:
        case FAN_8_ON_MAIN_BOARD:
            if((fantray_present_val & 0x08) == 0x00)
                info->status |= ONLP_FAN_STATUS_PRESENT;
            break;
    }

    return ONLP_STATUS_OK;
}


static int
dni_fani_info_get_fan_on_psu(int local_id, onlp_fan_info_t* info)
{
    int r_data = 0, psu_present_val = 0;
    char fullpath[80] = {0}, psu_num[2];

    /* Read cpld psu present bit for updating psu present status */
    psu_present_val = dni_lock_cpld_read_attribute(SWPLD_PATH, PSU_PRESENT_REG);
    if(local_id == FAN_1_ON_PSU1 &&
       ((psu_present_val & 0x01) == 0x00))
    {
        info->status |= ONLP_FAN_STATUS_PRESENT;
    }
    else if(local_id == FAN_1_ON_PSU2 &&
       ((psu_present_val & 0x02) == 0x00))
    {
        info->status |= ONLP_FAN_STATUS_PRESENT;
    }


    /* Configure MUX settings */
    if(local_id == FAN_1_ON_PSU1)
    {
        sprintf(psu_num, "%d", 1);
    }
    else if(local_id == FAN_1_ON_PSU2)
    {
        sprintf(psu_num, "%d", 2);
    }
    
    /* Set MUX */
    dni_i2c_lock_write_attribute(NULL, psu_num, PSU_SEL_PATH);

    /* Get psu fan fault status */
    sprintf(fullpath, "%s/%s", SYS_DEV_PATH, fan_path[local_id].status);
    r_data = dni_i2c_lock_read_attribute(NULL, fullpath);
    if (r_data > 0)
        info->status |= ONLP_FAN_STATUS_FAILED;

    /* Get psu fan rpm */
    sprintf(fullpath, "%s/%s", SYS_DEV_PATH, fan_path[local_id].speed);
    r_data = dni_i2c_lock_read_attribute(NULL, fullpath);
    info->rpm = r_data;

    /* Calculate psu fan duty cycle based on rpm */
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
    *info = linfo[local_id];


    switch (local_id)
    {
	    case FAN_1_ON_PSU1:
        case FAN_1_ON_PSU2:
            rc = dni_fani_info_get_fan_on_psu(local_id, info);
            break;
        case FAN_1_ON_MAIN_BOARD:
        case FAN_2_ON_MAIN_BOARD:
        case FAN_3_ON_MAIN_BOARD:
        case FAN_4_ON_MAIN_BOARD:
        case FAN_5_ON_MAIN_BOARD:
        case FAN_6_ON_MAIN_BOARD:
        case FAN_7_ON_MAIN_BOARD:
        case FAN_8_ON_MAIN_BOARD:
            rc = dni_fani_info_get_fan(local_id, info);
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
    int  local_id;
    char data[10] = {0};
    char fullpath[256] = {0};
    VALIDATE(id);
    local_id = ONLP_OID_ID_GET(id);

    /* get fullpath */
    switch (local_id)
    {
        case FAN_1_ON_MAIN_BOARD:
        case FAN_2_ON_MAIN_BOARD:
        case FAN_3_ON_MAIN_BOARD:
        case FAN_4_ON_MAIN_BOARD:
        case FAN_5_ON_MAIN_BOARD:
        case FAN_6_ON_MAIN_BOARD:
        case FAN_7_ON_MAIN_BOARD:
        case FAN_8_ON_MAIN_BOARD:
            sprintf(fullpath, "%s/%s", SYS_DEV_PATH, fan_path[local_id].ctrl_speed);
            break;
        default:
            return ONLP_STATUS_E_INVALID;
    }
    sprintf(data, "%d", rpm);

    dni_i2c_lock_write_attribute(NULL, data, fullpath);

    return ONLP_STATUS_OK;
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
    int local_id;
    char data[10], fullpath[256] = {0}, psu_num[2];
    VALIDATE(id);
    local_id = ONLP_OID_ID_GET(id);

    /* Configure MUX settings and fan percentage attribute path */
    switch(local_id)
    {
        case FAN_1_ON_PSU1:
            sprintf(psu_num, "%d", 1);
            sprintf(fullpath, "%s/%s", SYS_DEV_PATH, fan_path[local_id].ctrl_speed);
            break;
        case FAN_1_ON_PSU2:
            sprintf(psu_num, "%d", 2);
            sprintf(fullpath, "%s/%s", SYS_DEV_PATH, fan_path[local_id].ctrl_speed);
            break;
        default:
            return ONLP_STATUS_E_INVALID;
    }

    /* Set MUX */
    dni_i2c_lock_write_attribute(NULL, psu_num, PSU_SEL_PATH);

    /* Set fan percentage */
    sprintf(data, "%d", p);
    dni_i2c_lock_write_attribute(NULL, data, fullpath);
    
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

