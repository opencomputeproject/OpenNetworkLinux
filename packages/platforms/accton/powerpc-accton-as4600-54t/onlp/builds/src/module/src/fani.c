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
//#include "onlpie_int.h"
#include "platform_lib.h"

/*-- cpld --*/
#define CPLD_BASE_ADDRESS                  0xEA000000
#define CPLD_REG_FAN_STATUS_OFFSET         0x0F
#define CPLD_REG_PSU_2_FAN_STATUS_OFFSET   0x13
#define CPLD_REG_PSU_1_FAN_STATUS_OFFSET   0x14

/* FAN status mask
 */
#define CPLD_FAN_1_PRESENT_BIT_MASK     0x80
#define CPLD_FAN_2_PRESENT_BIT_MASK     0x40
#define CPLD_FAN_1_DIRECTION_BIT_MASK   0x20
#define CPLD_FAN_2_DIRECTION_BIT_MASK   0x10
#define CPLD_PSU_FAN_STATUS_BIT_MASK    0x8
#define CPLD_PSU_POWER_GOOD_MASK        0x10
#define CPLD_PSU_PRESENT_BIT_MASK       0x80

/*-- i2c --*/
#define I2C_SLAVE_ADDR_ADT7473   0x2E
#define I2C_SLAVE_ADDR_PSU_1_PMBUS   0x1A
#define I2C_SLAVE_ADDR_PSU_2_PMBUS   0x19

/* FAN status register in the ADT7473
 */
#define I2C_FAN_1_STATUS_REG_TACH_1_LOW    0x28
#define I2C_FAN_1_STATUS_REG_TACH_1_HIGH   0x29
#define I2C_FAN_2_STATUS_REG_TACH_3_LOW    0x2C
#define I2C_FAN_2_STATUS_REG_TACH_3_HIGH   0x2D

#define I2C_FAN_1_DUTY_CYCLE_REG_PWM_1     0x30
#define I2C_FAN_2_DUTY_CYCLE_REG_PWM_3     0x32

#define I2C_AC_PSU_CHICONY_REG_FAN_SPEED     0x90
#define I2C_AC_PSU_CHICONY_REG_FAN_COMMAND   0x3B

#define FAN_DUTY_CYCLE_MAX   0xFF
#define PSU_FAN_RPM_MAX      15000

typedef struct onlp_fan_cpld
{
    unsigned char  cpld_offset;
    unsigned char  cpld_mask;
} onlp_fan_cpld_t;

const onlp_fan_cpld_t fan_data[] = {
{},
{CPLD_REG_FAN_STATUS_OFFSET, CPLD_FAN_1_PRESENT_BIT_MASK},
{CPLD_REG_FAN_STATUS_OFFSET, CPLD_FAN_2_PRESENT_BIT_MASK},
{CPLD_REG_PSU_1_FAN_STATUS_OFFSET, CPLD_PSU_FAN_STATUS_BIT_MASK},
{CPLD_REG_PSU_2_FAN_STATUS_OFFSET, CPLD_PSU_FAN_STATUS_BIT_MASK},
};

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
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), 100);
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(2), 100);
    return ONLP_STATUS_OK;
}

/*
 * Get the fan information.
 */

/* Static fan information */
onlp_fan_info_t finfo[] = {
    { }, /* Not used */
    {
        { ONLP_FAN_ID_CREATE(1), "Chassis Fan 1", 0 },
        0x0,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_SET_PERCENTAGE,
        0,
        0,
        ONLP_FAN_MODE_INVALID,
    },
    {
        { ONLP_FAN_ID_CREATE(2), "Chassis Fan 2", 0 },
        0x0,
        ONLP_FAN_CAPS_F2B|ONLP_FAN_CAPS_SET_PERCENTAGE,
        0,
        0,
        ONLP_FAN_MODE_INVALID,
    },
    {
        { ONLP_FAN_ID_CREATE(3), "PSU-1 Fan 1", 0 },
        0x0,
        0,
        0,
        0,
        ONLP_FAN_MODE_INVALID,
    },
    {
        { ONLP_FAN_ID_CREATE(4), "PSU-2 Fan 1", 0 },
        0x0,
        0,
        0,
        0,
        ONLP_FAN_MODE_INVALID,
    },
};

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    unsigned int  bus_id = 1, fid = ONLP_OID_ID_GET(id);
    unsigned char val = 0, data[2]={0},i2c_addr;
    unsigned short psu_fan_rpm =0;
    int rc = 0, psu_model = PSU_MODULE_TYPE_UNKNOWN;

    *info = finfo[fid];

    val = cpld_base__[fan_data[fid].cpld_offset];

    switch (fid)
    {
        case 1:  /* FAN 1 */
            /* update the present status
             */
            if ((val & fan_data[fid].cpld_mask) == fan_data[fid].cpld_mask)
            {
                info->status &= ~ONLP_FAN_STATUS_PRESENT;
            }
            else
            {
                info->status |= ONLP_FAN_STATUS_PRESENT;
            }

            /* Update the Direction
             * 0: Back to front. 1: Front to back.
             */
            if((val & CPLD_FAN_1_DIRECTION_BIT_MASK) == CPLD_FAN_1_DIRECTION_BIT_MASK)
            {
                info->status |= ONLP_FAN_STATUS_F2B;
                info->status &= ~ONLP_FAN_STATUS_B2F;
            }
            else
            {
                info->status |= ONLP_FAN_STATUS_B2F;
                info->status &= ~ONLP_FAN_STATUS_F2B;
            }

#if 0  /* Hardware issue, the rpm value is unstable.*/
            /* Get the information from I2C
             */
            i2c_addr = I2C_SLAVE_ADDR_ADT7473;

            /* Get the status
             */
            rc = I2C_nRead(bus_id, i2c_addr, I2C_FAN_1_STATUS_REG_TACH_1_LOW, 1, &data[0]);
            if(0 != rc)
            {
                return ONLP_STATUS_E_INTERNAL;
            }
            rc = I2C_nRead(bus_id, i2c_addr, I2C_FAN_1_STATUS_REG_TACH_1_HIGH, 1, &data[1]);
            if(0 != rc)
            {
                return ONLP_STATUS_E_INTERNAL;
            }

            if (data[0] == 0xFF && data[1] == 0xFF)
            {
                /* FAN fail
                 */
                info->status |= ONLP_FAN_STATUS_FAILED;
            }
            else
            {
                info->status &= ~ONLP_FAN_STATUS_FAILED;
            }

            /* Update the rpm, Fan Speed (RPM) = (90,000 * 60)/Fan TACH Reading
             */
            fan_tach = data[1];
            fan_tach = (fan_tach << 8) + data[0];

            if (fan_tach != 0)
            {
                info->rpm = (90000 * 60) / fan_tach;
            }
#endif
            break;

        case 2: /* FAN 2 */
            if ((val & fan_data[fid].cpld_mask) == fan_data[fid].cpld_mask)
            {
                info->status &= ~ONLP_FAN_STATUS_PRESENT;
            }
            else
            {
                info->status |= ONLP_FAN_STATUS_PRESENT;
            }

            if((val & CPLD_FAN_2_DIRECTION_BIT_MASK) == CPLD_FAN_2_DIRECTION_BIT_MASK)
            {
                info->status |= ONLP_FAN_STATUS_F2B;
                info->status &= ~ONLP_FAN_STATUS_B2F;
            }
            else
            {
                info->status |= ONLP_FAN_STATUS_B2F;
                info->status &= ~ONLP_FAN_STATUS_F2B;
            }

#if 0  /* Hardware issue, the rpm value is unstable.*/
            /* Get information from I2C
             */
            i2c_addr = I2C_SLAVE_ADDR_ADT7473;

            /* Get the status
             */
            rc = I2C_nRead(bus_id, i2c_addr, I2C_FAN_2_STATUS_REG_TACH_3_LOW, 1, &data[0]);
            if(0 != rc)
            {
                return ONLP_STATUS_E_INTERNAL;
            }
            rc = I2C_nRead(bus_id, i2c_addr, I2C_FAN_2_STATUS_REG_TACH_3_HIGH, 1, &data[1]);
            if(0 != rc)
            {
                return ONLP_STATUS_E_INTERNAL;
            }

            if (data[0] == 0xFF && data[1] == 0xFF)
            {
                info->status |= ONLP_FAN_STATUS_FAILED;
            }
            else
            {
                info->status &= ~ONLP_FAN_STATUS_FAILED;
            }

            /* Get the rpm, Fan Speed (RPM) = (90,000 * 60)/Fan TACH Reading
             */
            fan_tach = data[1];
            fan_tach = (fan_tach << 8) + data[0];

            if (fan_tach != 0)
            {
                info->rpm = (90000 * 60) / fan_tach;
            }
#endif
            break;

        case 3: /* FAN in the PSU 1*/
        case 4: /* FAN in the PSU 2*/
            /* Get the PSU FAN status from CPLD
             * 1: FAN is OK. 0: FAN is fail
             */
            if ((val & fan_data[fid].cpld_mask) == fan_data[fid].cpld_mask)
            {
                info->status &= ~ONLP_FAN_STATUS_FAILED;
            }
            else
            {
                info->status |= ONLP_FAN_STATUS_FAILED;
            }
#if 0  /*#if 0 for test only, need this code block*/
            /* PSU needs to verify status of power_good_12v
             * if the power_good_12v is fail, we cant get the infomation from pmbus via i2c.
             */
            if ((val & CPLD_PSU_POWER_GOOD_MASK) == CPLD_PSU_POWER_GOOD_MASK)
            {
                break;
            }
#endif
            psu_model = as4600_54t_get_psu_type(fid-2, NULL,0); /* fid 3: inside PSU1. fid 4: inside PSU2*/

            switch (psu_model)
            {
                case PSU_MODULE_TYPE_AC_CHICONY_F2B:
                    info->status |= ONLP_FAN_STATUS_PRESENT;  /* always present. */

                    /* Update the Direction.
                     * 0: Back to front. 1: Front to back.
                     */
                    info->status |= ONLP_FAN_STATUS_F2B;

                    /* update caps */
                    info->caps |= ONLP_FAN_CAPS_F2B;
                    info->caps |= ONLP_FAN_CAPS_SET_PERCENTAGE;

                    break;

                case PSU_MODULE_TYPE_AC_CHICONY_B2F:
                    info->status |= ONLP_FAN_STATUS_PRESENT;  /* always present. */

                    /* Update the Direction.
                     * 0: Back to front. 1: Front to back.
                     */
                    info->status |= ONLP_FAN_STATUS_B2F;

                    /* update caps */
                    info->caps |= ONLP_FAN_CAPS_B2F;
                    info->caps |= ONLP_FAN_CAPS_SET_PERCENTAGE;

                    break;

                case PSU_MODULE_TYPE_DC_UMEC_48V_F2B:
                    info->status |= ONLP_FAN_STATUS_PRESENT;  /* always present. */
                    info->status |= ONLP_FAN_STATUS_F2B;

                    /* update caps */
                    info->caps |= ONLP_FAN_CAPS_F2B;

                    /* DC power supply does not has PMBus, return directly.
                     */
                    return ONLP_STATUS_OK;

                case PSU_MODULE_TYPE_DC_UMEC_48V_B2F:
                    info->status |= ONLP_FAN_STATUS_PRESENT;  /* always present. */
                    info->status |= ONLP_FAN_STATUS_B2F;

                    /* update caps */
                    info->caps |= ONLP_FAN_CAPS_B2F;

                    return ONLP_STATUS_OK;

                default :
                    return ONLP_STATUS_E_UNSUPPORTED;
            }

            /* Get the information from I2C
             */
            if (fid == 3)
            {
                i2c_addr = I2C_SLAVE_ADDR_PSU_1_PMBUS;
            }
            else
            {
                i2c_addr = I2C_SLAVE_ADDR_PSU_2_PMBUS;
            }

            rc = I2C_nRead(bus_id, i2c_addr, I2C_AC_PSU_CHICONY_REG_FAN_SPEED, 2, data);
            if(0 != rc)
            {
                return ONLP_STATUS_E_INTERNAL;
            }
            psu_fan_rpm = (data[1] << 8) + data[0];
            info->rpm = parse_literal_format(psu_fan_rpm, 1);

            break;

        default:
            return ONLP_STATUS_E_UNSUPPORTED;
    }

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
    /* precentage: range 0~100. (0: trun off)
     */
    unsigned int bus_id = 1, fan_prec_to_rpm = 0, fid = ONLP_OID_ID_GET(id);
    unsigned char i2c_addr=0, adt_offset=0, duty_cycle=0, fanspeed[3] = {0};
    unsigned short pmbus_value =0;
    int rc=0;

    if ( fid == 1 || fid == 2)
    {
        i2c_addr = I2C_SLAVE_ADDR_ADT7473;
        duty_cycle = (unsigned char)( (p * FAN_DUTY_CYCLE_MAX) / 100);

        if (duty_cycle > FAN_DUTY_CYCLE_MAX)
        {
            duty_cycle = FAN_DUTY_CYCLE_MAX;
        }

        if (fid == 1) /* FAN 1 */
        {
            adt_offset = I2C_FAN_1_DUTY_CYCLE_REG_PWM_1;
        }
        else if (fid == 2)  /* FAN 2 */
        {
            adt_offset = I2C_FAN_2_DUTY_CYCLE_REG_PWM_3;
        }

        /* write the status
         */
        rc = I2C_nWrite(bus_id, i2c_addr, adt_offset, 1, &duty_cycle);
        if(0 != rc)
        {
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    else if (fid == 3 || fid == 4)
    {
        int crc_rem=0;

        if ( fid == 3)
        {
            i2c_addr = I2C_SLAVE_ADDR_PSU_1_PMBUS;
        }
        else
        {
            i2c_addr = I2C_SLAVE_ADDR_PSU_2_PMBUS;
        }

        fan_prec_to_rpm = ( p * PSU_FAN_RPM_MAX ) / 100;
        pmbus_value = int_to_pmbus_linear(fan_prec_to_rpm);

        fanspeed[0] = pmbus_value & 0xFF;
        fanspeed[1] = (pmbus_value >> 8)& 0xFF;

        /* Create the PEC byte
         */
        crc_calc((i2c_addr<<1), &crc_rem);
        crc_calc(I2C_AC_PSU_CHICONY_REG_FAN_COMMAND, &crc_rem);
        crc_calc(fanspeed[0], &crc_rem);
        crc_calc(fanspeed[1], &crc_rem);
        fanspeed[2] = crc_rem;

        rc = I2C_nWrite(bus_id, i2c_addr, I2C_AC_PSU_CHICONY_REG_FAN_COMMAND, 3, fanspeed);
        if(0 != rc)
        {
            return ONLP_STATUS_E_INTERNAL;
        }

    }
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


