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
#define MAX_PSU_FAN_SPEED 18380

typedef struct fan_path_S
{
    char *status;
    char *speed;
    char *ctrl_speed;
}fan_path_T;

static fan_path_T fan_path[] =  /* must map with onlp_fan_id */
{
    { NULL, NULL, NULL },
    { "3-002c/fan5_fault", "3-002c/fan5_input", "3-002c/fan5_input_percentage" },
    { "3-002c/fan4_fault", "3-002c/fan4_input", "3-002c/fan4_input_percentage" },
    { "3-002c/fan3_fault", "3-002c/fan3_input", "3-002c/fan3_input_percentage" },
    { "3-002c/fan2_fault", "3-002c/fan2_input", "3-002c/fan2_input_percentage" },
    { "3-002c/fan1_fault", "3-002c/fan1_input", "3-002c/fan1_input_percentage" },
    { "3-002d/fan5_fault", "3-002d/fan5_input", "3-002d/fan5_input_percentage" },
    { "3-002d/fan4_fault", "3-002d/fan4_input", "3-002d/fan4_input_percentage" },
    { "3-002d/fan3_fault", "3-002d/fan3_input", "3-002d/fan3_input_percentage" },
    { "3-002d/fan2_fault", "3-002d/fan2_input", "3-002d/fan2_input_percentage" },
    { "3-002d/fan1_fault", "3-002d/fan1_input", "3-002d/fan1_input_percentage" },
    { "4-0058/psu_fan1_fault", "4-0058/psu_fan1_speed_rpm", "4-0058/psu_fan1_duty_cycle_percentage" },
    { "4-0058/psu_fan1_fault", "4-0058/psu_fan1_speed_rpm", "4-0058/psu_fan1_duty_cycle_percentage" }
};

#define MAKE_FAN_INFO_NODE_ON_FAN_BOARD(_id, _coid)                     \
    {                                                                   \
        .hdr = {                                                        \
            .id = ONLP_FAN_ID_CREATE(FAN_##_id##_ON_FAN_BOARD),         \
            .description = "Chassis Fan "#_id,                          \
            .poid = ONLP_OID_CHASSIS,                                   \
            .coids = {                                                  \
                ONLP_FAN_ID_CREATE(FAN_##_coid##_ON_FAN_BOARD),         \
                ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_##_id)             \
            },                                                          \
        },                                                              \
        .dir = ONLP_FAN_DIR_UNKNOWN,                                    \
        .caps = (ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_PERCENTAGE | ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM), \
    }

#define MAKE_FAN_INFO_NODE_ON_FAN_BOARD_WEAK(_id, _poid)                \
    {                                                                   \
        .hdr = {                                                        \
            .id = ONLP_FAN_ID_CREATE(FAN_##_id##_ON_FAN_BOARD),         \
            .description = "Chassis Fan "#_id,                          \
            .poid = ONLP_FAN_ID_CREATE(FAN_##_poid##_ON_FAN_BOARD)      \
        },                                                              \
        .dir = ONLP_FAN_DIR_UNKNOWN,                                    \
        .caps = (ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_PERCENTAGE | ONLP_FAN_CAPS_SET_RPM | ONLP_FAN_CAPS_GET_RPM), \
    }

#define MAKE_FAN_INFO_NODE_ON_PSU(psu_id, fan_id)                       \
    {                                                                   \
        .hdr = {                                                        \
            .id = ONLP_FAN_ID_CREATE(FAN_##fan_id##_ON_PSU##psu_id),    \
            .description = "Chassis PSU-"#psu_id " Fan "#fan_id,        \
            .poid = ONLP_PSU_ID_CREATE(PSU##psu_id##_ID)                \
        },                                                              \
        .dir = ONLP_FAN_DIR_UNKNOWN,                                    \
        .caps = (ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
    }

/* Static fan information */
onlp_fan_info_t fan_info[] = {
    { }, /* Not used */
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(1,6),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(2,7),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(3,8),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(4,9),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(5,10),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD_WEAK(6,1),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD_WEAK(7,2),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD_WEAK(8,3),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD_WEAK(9,4),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD_WEAK(10,5),
    MAKE_FAN_INFO_NODE_ON_PSU(1,1),
    MAKE_FAN_INFO_NODE_ON_PSU(2,1),
};

static int
chassis_fani_hdr_get(int id, onlp_oid_hdr_t* hdr)
{
    uint8_t present_bit = 0x00, bit = 0x00;
    *hdr = fan_info[id].hdr;

    mux_info_t mux_info;
    mux_info.offset = SWPLD_PSU_FAN_I2C_MUX_REG;
    mux_info.channel = 0x07; /* FAN IO Control */
    mux_info.flags = DEFAULT_FLAG;

    dev_info_t dev_info;
    dev_info.bus = I2C_BUS_3;
    dev_info.addr = FAN_IO_CTL;
    dev_info.offset = 0x00;
    dev_info.flags = DEFAULT_FLAG;

    present_bit = dni_i2c_lock_read(&mux_info, &dev_info);

    switch(id) {
	    case FAN_1_ON_FAN_BOARD:
	    case FAN_6_ON_FAN_BOARD:
            if((present_bit & ((bit+1)<<4)) == 0)
                ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
            else
                ONLP_OID_STATUS_FLAG_SET(hdr, FAILED);
            break;
        case FAN_2_ON_FAN_BOARD:
        case FAN_7_ON_FAN_BOARD:
            if((present_bit & ((bit+1)<<3)) == 0)
                ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
            else
                ONLP_OID_STATUS_FLAG_SET(hdr, FAILED);
            break;
        case FAN_3_ON_FAN_BOARD:
        case FAN_8_ON_FAN_BOARD:
            if((present_bit & ((bit+1)<<2)) == 0)
                ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
            else
                ONLP_OID_STATUS_FLAG_SET(hdr, FAILED);
            break;
        case FAN_4_ON_FAN_BOARD:
        case FAN_9_ON_FAN_BOARD:
            if((present_bit & ((bit+1)<<1)) == 0)
                ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
            else
                ONLP_OID_STATUS_FLAG_SET(hdr, FAILED);
            break;
        case FAN_5_ON_FAN_BOARD:
        case FAN_10_ON_FAN_BOARD:
            if((present_bit & (bit+1)) == 0)
                ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
            else
                ONLP_OID_STATUS_FLAG_SET(hdr, FAILED);
            break;
    }

    return ONLP_STATUS_OK;
}

static int
chassis_fani_info_get(int id, onlp_fan_info_t* info)
{
    int rpm = 0;
    char fullpath[100] = {0};

    *info = fan_info[id];
    ONLP_TRY(chassis_fani_hdr_get(id, &info->hdr));

    mux_info_t mux_info;
    mux_info.offset = SWPLD_PSU_FAN_I2C_MUX_REG;
    mux_info.channel = 0x05; /* FAN Control IC */
    mux_info.flags = DEFAULT_FLAG;

    /* Get Fan speed */
    sprintf(fullpath, "%s%s", PREFIX_PATH, fan_path[id].speed);
    rpm = dni_i2c_lock_read_attribute(&mux_info, fullpath);
    info->rpm = rpm;

    /* If rpm is FAN_ZERO_TACH, then the rpm value is zero. */
    if(info->rpm == FAN_ZERO_TACH)
        info->rpm = 0;

    /* get speed percentage from rpm */
    info->percentage = (info->rpm * 100)/MAX_FAN_SPEED;

    return ONLP_STATUS_OK;
}

static int
psu_fani_hdr_get(int id, onlp_oid_hdr_t* hdr)
{
    int r_data = 0;
    uint8_t channel = 0x00;
    char fullpath[80] = {0};
    char channel_data[2] = {'\0'};
    mux_info_t mux_info;
    dev_info_t dev_info;

    *hdr = fan_info[id].hdr;

    /* Select PSU member */
    switch (id) {
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
    dev_info.offset = 0x00; /* In EEPROM address 0x00 */
    dev_info.flags = DEFAULT_FLAG;

    /* Check PSU is PRESENT or not
     * Read PSU EEPROM 1 byte from adress 0x00
     * if not present, return Negative value.
     */
    if(dni_i2c_lock_read(&mux_info, &dev_info) >= 0) {
        ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
    }

    /* Check PSU FAN is fault or not
     * Read PSU FAN Fault from psu_fan1_fault
     * Return 1 is PSU fan fault
     */
    sprintf(fullpath, "%s%s", PREFIX_PATH, fan_path[id].status);
    r_data = dni_i2c_lock_read_attribute(&mux_info, fullpath);

    if (r_data == 1) {
        ONLP_OID_STATUS_FLAG_SET(hdr, FAILED);
    }

    return ONLP_STATUS_OK;
}

static int
psu_fani_info_get(int id, onlp_fan_info_t* info)
{
    int r_data = 0;
    uint8_t channel = 0x00;
    char fullpath[80] = {0};
    char channel_data[2] = {'\0'};
    mux_info_t mux_info;

    *info = fan_info[id];
    ONLP_TRY(psu_fani_hdr_get(id, &info->hdr));

    if(ONLP_OID_STATUS_FLAG_NOT_SET(info, PRESENT) ||
        ONLP_OID_STATUS_FLAG_IS_SET(info, FAILED)) {
	    return 0;
    }

    /* Direction */
    info->dir = ONLP_FAN_DIR_B2F;

    /* Select PSU member */
    switch (id) {
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

    /* Read PSU FAN speed from psu_fan1_speed_rpm */
    sprintf(fullpath, "%s%s", PREFIX_PATH, fan_path[id].speed);
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
onlp_fani_sw_init(void)
{
    lockinit();
    return ONLP_STATUS_OK;
}

int
onlp_fani_sw_denit(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_fani_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    int local_id;

    local_id = ONLP_OID_ID_GET(id);

    switch(local_id)
    {
        case FAN_1_ON_PSU1:
            return psu_fani_hdr_get(local_id, hdr);
        case FAN_1_ON_PSU2:
            return psu_fani_hdr_get(local_id, hdr);
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
            return chassis_fani_hdr_get(local_id, hdr);
        default:
            return ONLP_STATUS_E_PARAM;
    }
}

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    int local_id;

    local_id = ONLP_OID_ID_GET(id);
    *info = fan_info[local_id];

    switch (local_id)
    {
        case FAN_1_ON_PSU1:
        case FAN_1_ON_PSU2:
            return psu_fani_info_get(local_id, info);
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
            return chassis_fani_info_get(local_id, info);
            break;
        default:
            return ONLP_STATUS_E_PARAM;
            break;
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
    int  local_id;
    char data[10] = {0};
    char fullpath[70] = {0};

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
            sprintf(fullpath, "%s%s", PREFIX_PATH, fan_path[local_id].speed);
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

