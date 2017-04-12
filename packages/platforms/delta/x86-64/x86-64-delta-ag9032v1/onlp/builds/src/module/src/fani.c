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

#define MAX_FAN_SPEED     23000
#define MAX_PSU_FAN_SPEED 19328

typedef struct fan_path_S
{
    char *status;
    char *speed;
    char *ctrl_speed;
}fan_path_T;

static fan_path_T fan_path[] =  /* must map with onlp_fan_id */
{
    { NULL, NULL, NULL },
    { "3-002c/fan1_fault", "3-002c/fan1_input", "3-002c/fan1_input_percentage" },
    { "3-002c/fan2_fault", "3-002c/fan2_input", "3-002c/fan2_input_percentage" },
    { "3-002c/fan3_fault", "3-002c/fan3_input", "3-002c/fan3_input_percentage" },
    { "3-002c/fan4_fault", "3-002c/fan4_input", "3-002c/fan4_input_percentage" },
    { "3-002c/fan5_fault", "3-002c/fan5_input", "3-002c/fan5_input_percentage" },
    { "3-002d/fan1_fault", "3-002d/fan1_input", "3-002d/fan1_input_percentage" },
    { "3-002d/fan2_fault", "3-002d/fan2_input", "3-002d/fan2_input_percentage" },
    { "3-002d/fan3_fault", "3-002d/fan3_input", "3-002d/fan3_input_percentage" },
    { "3-002d/fan4_fault", "3-002d/fan4_input", "3-002d/fan4_input_percentage" },
    { "3-002d/fan5_fault", "3-002d/fan5_input", "3-002d/fan5_input_percentage" },
    { "4-0058/psu_fan1_fault", "4-0058/psu_fan1_speed_rpm", "4-0058/psu_fan1_duty_cycle_percentage" },
    { "4-0058/psu_fan1_fault", "4-0058/psu_fan1_speed_rpm", "4-0058/psu_fan1_duty_cycle_percentage" }
};

#define MAKE_FAN_INFO_NODE_ON_FAN_BOARD(id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##id##_ON_FAN_BOARD), "Chassis Fan "#id, 0 }, \
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
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(1),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(2),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(3),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(4),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(5),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(6),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(7),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(8),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(9),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(10),
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
    uint8_t present_bit=0x00, bit=0x00;

    mux_info_t mux_info;
    mux_info.offset = SWPLD_PSU_FAN_I2C_MUX_REG;
    mux_info.channel = 0x05;
    mux_info.flags = DEFAULT_FLAG;

    dev_info_t dev_info;
    dev_info.bus = I2C_BUS_3;
    dev_info.addr = FAN_IO_CTL;
    dev_info.offset = 0x00;
    dev_info.flags = DEFAULT_FLAG;
    
    sprintf(fullpath, "%s%s", PREFIX_PATH, fan_path[local_id].speed);
    rpm = dni_i2c_lock_read_attribute(&mux_info, fullpath);
    info->rpm = rpm;

    /* If rpm is FAN_ZERO_TACH, then the rpm value is zero. */
    if(info->rpm == FAN_ZERO_TACH)
        info->rpm = 0;

    /* get speed percentage from rpm */
    info->percentage = (info->rpm * 100)/MAX_FAN_SPEED;
    
    mux_info.channel = 0x07;
    present_bit = dni_i2c_lock_read(&mux_info, &dev_info);
    switch(local_id)
    {
        case FAN_1_ON_FAN_BOARD:
        case FAN_6_ON_FAN_BOARD:
            if((present_bit & (bit+1)) == 0)
                info->status |= ONLP_FAN_STATUS_PRESENT;
            else
                info->status |= ONLP_FAN_STATUS_FAILED;
            break;
        case FAN_2_ON_FAN_BOARD:
        case FAN_7_ON_FAN_BOARD:
            if((present_bit & ((bit+1)<<1)) == 0)
                info->status |= ONLP_FAN_STATUS_PRESENT;
            else
                info->status |= ONLP_FAN_STATUS_FAILED;
            break;
        case FAN_3_ON_FAN_BOARD:
        case FAN_8_ON_FAN_BOARD:
            if((present_bit & ((bit+1)<<2)) == 0)
                info->status |= ONLP_FAN_STATUS_PRESENT;
            else
                info->status |= ONLP_FAN_STATUS_FAILED;
            break;
        case FAN_4_ON_FAN_BOARD:
        case FAN_9_ON_FAN_BOARD:
            if((present_bit & ((bit+1)<<3)) == 0)
                info->status |= ONLP_FAN_STATUS_PRESENT;
            else
                info->status |= ONLP_FAN_STATUS_FAILED;
            break;
        case FAN_5_ON_FAN_BOARD:
        case FAN_10_ON_FAN_BOARD:
            if((present_bit & ((bit+1)<<4)) == 0)
                info->status |= ONLP_FAN_STATUS_PRESENT;
            else
                info->status |= ONLP_FAN_STATUS_FAILED;
            break;
    }
    return ONLP_STATUS_OK;
}


static int
dni_fani_info_get_fan_on_psu(int local_id, onlp_fan_info_t* info)
{
    int r_data = 0;
    uint8_t channel = 0x00;
    char fullpath[80] = {0};
    char channel_data[2] = {'\0'};
    mux_info_t mux_info;
    dev_info_t dev_info;

    /* Select PSU member */
    switch (local_id) {
    case FAN_1_ON_PSU1:
	channel = PSU_I2C_SEL_PSU1_EEPROM;
	sprintf(channel_data, "%x", PSU_I2C_SEL_PSU1_EEPROM);
    	dni_i2c_lock_write_attribute(NULL, channel_data, PSU_SELECT_MEMBER_PATH);
	break;
    case FAN_1_ON_PSU2:
	channel = PSU_I2C_SEL_PSU2_EEPROM;
	sprintf(channel_data, "%x", PSU_I2C_SEL_PSU2_EEPROM);
    	dni_i2c_lock_write_attribute(NULL, channel_data, PSU_SELECT_MEMBER_PATH);
	break;
    default:
	channel = 0x00; /* DEFAULT */
	break;
    }

    mux_info.offset = SWPLD_PSU_FAN_I2C_MUX_REG;
    mux_info.channel = channel;
    mux_info.flags = DEFAULT_FLAG;

    dev_info.bus = I2C_BUS_4;
    dev_info.addr = PSU_EEPROM;
    dev_info.offset = 0x00;     /* In EEPROM address 0x00 */
    dev_info.flags = DEFAULT_FLAG;

    /* Check PSU is PRESENT or not
     * Read PSU EEPROM 1 byte from adress 0x00
     * if not present, return Negative value.
     */
    if(dni_i2c_lock_read(&mux_info, &dev_info) >= 0) {
        info->status |= ONLP_FAN_STATUS_PRESENT | ONLP_FAN_STATUS_B2F;
    }

    /* Check PSU FAN is fault or not
     * Read PSU FAN Fault from psu_fan1_fault
     * Return 1 is PSU fan fault
     */
    sprintf(fullpath, "%s%s", PREFIX_PATH, fan_path[local_id].status);
    r_data = dni_i2c_lock_read_attribute(&mux_info, fullpath);

    if (r_data == 1) {
        info->status |= ONLP_FAN_STATUS_FAILED;
    }

    /* Read PSU FAN speed from psu_fan1_speed_rpm */
    sprintf(fullpath, "%s%s", PREFIX_PATH, fan_path[local_id].speed);
    r_data = dni_i2c_lock_read_attribute(&mux_info, fullpath);
    info->rpm = r_data;

    /* get speed percentage from rpm */
    info->percentage = ((info->rpm) * 100) / MAX_PSU_FAN_SPEED;
    
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
        case FAN_1_ON_FAN_BOARD:
        case FAN_2_ON_FAN_BOARD:
        case FAN_3_ON_FAN_BOARD:
        case FAN_4_ON_FAN_BOARD:
        case FAN_5_ON_FAN_BOARD:
        case FAN_6_ON_FAN_BOARD:
        case FAN_7_ON_FAN_BOARD:
        case FAN_8_ON_FAN_BOARD:
        case FAN_9_ON_FAN_BOARD:
        case FAN_10_ON_FAN_BOARD:
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
    char fullpath[70] = {0};
    VALIDATE(id);
    local_id = ONLP_OID_ID_GET(id);

    /* get fullpath */
    switch (local_id)
    {
        case FAN_1_ON_FAN_BOARD:
        case FAN_2_ON_FAN_BOARD:
        case FAN_3_ON_FAN_BOARD:
        case FAN_4_ON_FAN_BOARD:
        case FAN_5_ON_FAN_BOARD:
        case FAN_6_ON_FAN_BOARD:
        case FAN_7_ON_FAN_BOARD:
        case FAN_8_ON_FAN_BOARD:
        case FAN_9_ON_FAN_BOARD:
        case FAN_10_ON_FAN_BOARD:
            sprintf(fullpath, "%s%s", PREFIX_PATH, fan_path[local_id].ctrl_speed);
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
    char data[10] = {0};
    char fullpath[70] = {0};
    char channel_data[2] = {'\0'};

    VALIDATE(id);
    local_id = ONLP_OID_ID_GET(id);

    /* Select PSU member */
    switch (local_id) {
    case FAN_1_ON_PSU1:
        sprintf(channel_data, "%x", PSU_I2C_SEL_PSU1_EEPROM);
	dni_i2c_lock_write_attribute(NULL, channel_data,
                             "/sys/bus/i2c/devices/4-0058/psu_select_member");
        break;
    case FAN_1_ON_PSU2:
        sprintf(channel_data, "%x", PSU_I2C_SEL_PSU2_EEPROM);
	dni_i2c_lock_write_attribute(NULL, channel_data,
                             "/sys/bus/i2c/devices/4-0058/psu_select_member");
        break;
    case FAN_1_ON_FAN_BOARD:
    case FAN_2_ON_FAN_BOARD:
    case FAN_3_ON_FAN_BOARD:
    case FAN_4_ON_FAN_BOARD:
    case FAN_5_ON_FAN_BOARD:
    case FAN_6_ON_FAN_BOARD:
    case FAN_7_ON_FAN_BOARD:
    case FAN_8_ON_FAN_BOARD:
    case FAN_9_ON_FAN_BOARD:
    case FAN_10_ON_FAN_BOARD:
        break;
    default:
        return ONLP_STATUS_E_INVALID;
    }

    sprintf(fullpath, "%s%s", PREFIX_PATH, fan_path[local_id].ctrl_speed);
 
    /* Write percentage to psu_fan1_duty_cycle_percentage */
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

