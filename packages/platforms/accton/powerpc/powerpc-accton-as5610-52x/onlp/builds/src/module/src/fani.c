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
#include <sys/mman.h>
#include <onlplib/mmap.h>
#include <stdio.h>
#include <string.h>

#include "platform_lib.h"
//#include "onlpie_int.h"

/* PSU/FAN tatus register in CPLD
 */
#define CPLD_BASE_ADDRESS    		 0xEA000000

#define CPLD_REG_PSU1_STATUS         0x02
#define CPLD_REG_PSU2_STATUS         0x01
#define CPLD_REG_SYS_STATUS          0x03
#define CPLD_PSU_PRESENT_MASK        0x01
#define CPLD_PSU_POWER_GOOD_MASK     0x02
#define CPLD_PSU_FAN_FAILURE_MASK    0x04
#define CPLD_FAN_PRESENT_MASK        0x04
#define CPLD_FAN_FAILURE_MASK        0x08
#define CPLD_FAN_DIRECTION_MASK      0x10
#define CPLD_FAN_SPEED_CTL_REG       0x0D
#define CPLD_FAN_SPEED_VALUE_MAX     0x1F
#define CPLD_FAN_SPEED_VALUE_MID     0x15
#define CPLD_FAN_SPEED_VALUE_MIN     0x0C

#define I2C_PSU_BUS_ID                  0
#define I2C_PSU1_SLAVE_ADDR_CFG      0x3E
#define I2C_PSU1_SLAVE_ADDR_EEPROM   0x3A
#define I2C_PSU2_SLAVE_ADDR_CFG      0x3D
#define I2C_PSU2_SLAVE_ADDR_EEPROM   0x39
#define I2C_PSU_FAN_SPEED_CTL_REG    0x3B
#define I2C_PSU_FAN_STATUS_REG       0x81
#define I2C_PSU_FAN_FAILURE_MASK     0x80

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

typedef enum onlp_fan_id
{
    FAN_ID_CHASSIS = 1,
    FAN_ID_PSU1,
    FAN_ID_PSU2
} onlp_fan_id_t;

/*
 * Private functions for fan controller
 */
static int
chassis_fan_cpld_val_to_duty_cycle(unsigned char reg_val)
{
    reg_val &= 0x1F;

    if (reg_val == CPLD_FAN_SPEED_VALUE_MAX) {
        return FAN_PERCENTAGE_MAX;
    }
    else if (reg_val == CPLD_FAN_SPEED_VALUE_MID) {
        return FAN_PERCENTAGE_MID;
    }
    else if (reg_val == CPLD_FAN_SPEED_VALUE_MIN) {
        return FAN_PERCENTAGE_MIN;
    }

    return (reg_val * 3.25);
}

static int
chassis_fan_duty_cycle_to_cpld_val(int duty_cycle)
{
    if (duty_cycle == FAN_PERCENTAGE_MAX) {
        return CPLD_FAN_SPEED_VALUE_MAX;
    }
    else if (duty_cycle == FAN_PERCENTAGE_MID) {
        return CPLD_FAN_SPEED_VALUE_MID;
    }
    else if (duty_cycle == FAN_PERCENTAGE_MIN) {
        return CPLD_FAN_SPEED_VALUE_MIN;
    }

    return (duty_cycle / 3.25);
}

static volatile uint8_t* cpld_base__ = NULL;

/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
    /*
     * Map the CPLD address
     */
    cpld_base__ = onlp_mmap(CPLD_BASE_ADDRESS, getpagesize(), __FILE__);

    if(cpld_base__ == NULL || cpld_base__ == MAP_FAILED) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /*
     * Bring both fans to max.
     * These will be reduced after the first platform management sequence.
     */
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_PERCENTAGE_MAX);

    return ONLP_STATUS_OK;
}

/*
 * Get the fan information.
 */

/* Static fan information */
onlp_fan_info_t finfo[] = {
    { }, /* Not used */
    {
        { ONLP_FAN_ID_CREATE(FAN_ID_CHASSIS), "Chassis Fan 1", 0 },
    },
    {
        { ONLP_FAN_ID_CREATE(FAN_ID_PSU1), "PSU-1 Fan 1", ONLP_PSU_ID_CREATE(1) },
    },
    {
        { ONLP_FAN_ID_CREATE(FAN_ID_PSU2), "PSU-2 Fan 1", ONLP_PSU_ID_CREATE(2) },
    }
};

static int
chassis_fan_get_info(onlp_fan_info_t* info)
{
    unsigned char data;
    info->status = 0;

    data = cpld_base__[CPLD_REG_SYS_STATUS];

    /* Get the present bit */
    if ((~data) & CPLD_FAN_PRESENT_MASK) {
        info->status |= ONLP_FAN_STATUS_PRESENT;
    }

    /* Get the failure bit */
    if (data & CPLD_FAN_FAILURE_MASK) {
        info->status |= ONLP_FAN_STATUS_FAILED;
    }

    /* Get the airflow direction */
    info->status |= (data & CPLD_FAN_DIRECTION_MASK) ? ONLP_FAN_STATUS_F2B :
                                                       ONLP_FAN_STATUS_B2F ;

    /* Get the capabilities
     */
    info->caps |= ONLP_FAN_CAPS_SET_PERCENTAGE;
    info->caps |= ONLP_FAN_CAPS_GET_PERCENTAGE;
    info->caps |= (data & CPLD_FAN_DIRECTION_MASK) ? ONLP_FAN_CAPS_F2B : ONLP_FAN_CAPS_B2F;

    /* Get the percentage
     */
    data = cpld_base__[CPLD_FAN_SPEED_CTL_REG];
    info->percentage = chassis_fan_cpld_val_to_duty_cycle(data);

    return ONLP_STATUS_OK;
}

static int
psu_cpr_4011_fan_info_get(onlp_fan_info_t* info)
{
    int idx = ONLP_OID_ID_GET(info->hdr.id) - 2;
    unsigned char speed[2]      = {0};
    unsigned char status        = 0;
    unsigned char mux_ch[]      = {0x2, 0x4};
    unsigned char cfg_addr[]    = {I2C_PSU1_SLAVE_ADDR_CFG, I2C_PSU2_SLAVE_ADDR_CFG};

    info->mode   = ONLP_FAN_MODE_INVALID; /* Set current mode */
    info->rpm    = 0; /* Set rpm as 0 since the fan controller does not support rpm reading */

    /* Get operating status
     */
    if (as5610_52x_i2c0_pca9548_channel_set(mux_ch[idx]) != 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (i2c_nRead(I2C_PSU_BUS_ID, cfg_addr[idx], I2C_PSU_FAN_STATUS_REG, sizeof(status), &status) != 0) {
        as5610_52x_i2c0_pca9548_channel_set(0);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (status & I2C_PSU_FAN_FAILURE_MASK) {
        info->status |= ONLP_FAN_STATUS_FAILED;
    }

    /* Get the percentage
     */
    if (i2c_nRead(I2C_PSU_BUS_ID, cfg_addr[idx], I2C_PSU_FAN_SPEED_CTL_REG, sizeof(speed), speed) != 0) {
        as5610_52x_i2c0_pca9548_channel_set(0);
        return ONLP_STATUS_E_INTERNAL;
    }
    info->percentage = speed[0];

    /* Close psu channel
     */
    as5610_52x_i2c0_pca9548_channel_set(0);

    return ONLP_STATUS_OK;
}

static int
psu_cpr_4011_f2b_fan_info_get(onlp_fan_info_t* info)
{
    info->status = ONLP_FAN_STATUS_PRESENT      | ONLP_FAN_STATUS_F2B;
    info->caps   = ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_F2B;

    return psu_cpr_4011_fan_info_get(info);
}

static int
psu_cpr_4011_b2f_fan_info_get(onlp_fan_info_t* info)
{
    info->status = ONLP_FAN_STATUS_PRESENT      | ONLP_FAN_STATUS_B2F;
    info->caps   = ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_B2F;

    return psu_cpr_4011_fan_info_get(info);
}

static int
psu_um400d_fan_info_get(onlp_fan_info_t* info)
{
    unsigned char cpld_offset, data;

    info->mode       = ONLP_FAN_MODE_INVALID; /* Set current mode */
    info->rpm        = 0; /* Set rpm as 0 since the fan controller does not support rpm reading */
    info->percentage = 0;

    /* Get operating status
     */
    if (FAN_ID_PSU1 == ONLP_OID_ID_GET(info->hdr.id)) {
        cpld_offset = CPLD_REG_PSU1_STATUS;
    }
    else { /* FAN_ID_PSU2 */
        cpld_offset = CPLD_REG_PSU2_STATUS;
    }

    data = cpld_base__[cpld_offset];

    if (!(data & CPLD_PSU_FAN_FAILURE_MASK)) {
        info->status |= ONLP_FAN_STATUS_FAILED;
    }

    return ONLP_STATUS_OK;
}

static int
psu_um400d_f2b_fan_info_get(onlp_fan_info_t* info)
{
    info->status = ONLP_FAN_STATUS_PRESENT | ONLP_FAN_STATUS_F2B;
    info->caps   = ONLP_FAN_CAPS_F2B;

    return psu_um400d_fan_info_get(info);
}

static int
psu_um400d_b2f_fan_info_get(onlp_fan_info_t* info)
{
    info->status = ONLP_FAN_STATUS_PRESENT | ONLP_FAN_STATUS_B2F;
    info->caps   = ONLP_FAN_CAPS_B2F;

    return psu_um400d_fan_info_get(info);
}

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    VALIDATE(id);

    *info = finfo[ONLP_OID_ID_GET(id)]; /* Set the onlp_oid_hdr_t */
    ONLP_OID_TABLE_CLEAR(info->hdr.coids);

    if (ONLP_OID_ID_GET(id) == FAN_ID_CHASSIS) {
        return chassis_fan_get_info(info);
    }
    else {
        int psu_id = ONLP_OID_ID_GET(id)-1;
        as5610_52x_psu_type_t psu_type = as5610_52x_get_psu_type(psu_id, NULL, 0);

        switch (psu_type) {
            case PSU_TYPE_AC_F2B:
                return psu_cpr_4011_f2b_fan_info_get(info);
            case PSU_TYPE_AC_B2F:
                return psu_cpr_4011_b2f_fan_info_get(info);
            case PSU_TYPE_DC_48V_F2B:
                return psu_um400d_f2b_fan_info_get(info);
            case PSU_TYPE_DC_48V_B2F:
                return psu_um400d_b2f_fan_info_get(info);
            default:
                break;
        }
    }

    return ONLP_STATUS_E_UNSUPPORTED;
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

static int
onlp_chassis_fan_percentage_set(int p)
{
    cpld_base__[CPLD_FAN_SPEED_CTL_REG] = chassis_fan_duty_cycle_to_cpld_val(p);

    return ONLP_STATUS_OK;
}

static int
onlp_psu_cpr_4011_fan_percentage_set(onlp_oid_t id, int p)
{
    int i = ONLP_OID_ID_GET(id) - 2;
    unsigned char mux_ch[]      = {0x2, 0x4};
    unsigned char cfg_addr[]    = {I2C_PSU1_SLAVE_ADDR_CFG, I2C_PSU2_SLAVE_ADDR_CFG};
    unsigned char speed[2] = {(unsigned char)p, 0};
    VALIDATE(id);

    if (as5610_52x_i2c0_pca9548_channel_set(mux_ch[i]) != 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (i2c_nWrite(I2C_PSU_BUS_ID, cfg_addr[i], I2C_PSU_FAN_SPEED_CTL_REG, sizeof(speed), speed) != 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

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
    VALIDATE(id);

    if (ONLP_OID_ID_GET(id) == FAN_ID_CHASSIS) {
        return onlp_chassis_fan_percentage_set(p);
    }
    else if ((ONLP_OID_ID_GET(id) == FAN_ID_PSU1) || (ONLP_OID_ID_GET(id) == FAN_ID_PSU2)) {
        as5610_52x_psu_type_t psu_type = as5610_52x_get_psu_type(ONLP_OID_ID_GET(id), NULL, 0);

        if (PSU_TYPE_AC_F2B == psu_type || PSU_TYPE_AC_B2F == psu_type) {
            return onlp_psu_cpr_4011_fan_percentage_set(id, p);
        }
    }

    return ONLP_STATUS_E_UNSUPPORTED;
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

