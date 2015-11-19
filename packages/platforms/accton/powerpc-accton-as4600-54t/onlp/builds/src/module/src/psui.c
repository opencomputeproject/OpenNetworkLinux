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
 *
 *
 ***********************************************************/
#include <onlp/platformi/psui.h>
#include <onlplib/mmap.h>
//#include "onlpie_int.h"
#include "platform_lib.h"

/*-- i2c --*/
#define I2C_SLAVE_ADDR_PSU_1_PMBUS   0x1A
#define I2C_SLAVE_ADDR_PSU_2_PMBUS   0x19

#define I2C_AC_PSU_CHICONY_REG_OUT_VOLT      0x8B
#define I2C_AC_PSU_CHICONY_REG_OUT_CURRENT   0x8C
#define I2C_AC_PSU_CHICONY_REG_OUT_POWER     0x96
#define I2C_AC_PSU_CHICONY_REG_STATUS        0x79

#define I2C_AC_PSU_CHICONY_POWER_GOOD_NETAGED_MASK  0x8

/*-- cpld --*/
#define CPLD_BASE_ADDRESS                0xEA000000
#define CPLD_REG_PSU_1_STATUS_OFFSET     0x14
#define CPLD_REG_PSU_2_STATUS_OFFSET     0x13
#define CPLD_REG_PSU_PRESENT_MASK        0x80
#define CPLD_REG_PSU_POWER_GOOD_MASK     0x10

#define PSU_PMBUS_DATA_MULTIPLIER        1000

typedef enum onlp_psu_id
{
    PSU_1 = 0,
    PSU_2
} onlp_psu_id_t;

typedef struct onlp_psu_i2c
{
    onlp_psu_id_t  pid;
    unsigned char  i2c_pmbus_addr;
} onlp_psu_i2c_t;

typedef struct onlp_psu_cpld
{
    onlp_psu_id_t  pid;
    unsigned char  cpld_addr;
    unsigned char  present_mask;     /* Presnet: BA device needs to verify persent & power_good_12v*/
    unsigned char  power_good_mask;
} onlp_psu_cpld_t;

const onlp_psu_i2c_t psu_i2c_data[] = {
{},
{PSU_1, I2C_SLAVE_ADDR_PSU_1_PMBUS},
{PSU_2, I2C_SLAVE_ADDR_PSU_2_PMBUS}
};

const onlp_psu_cpld_t psu_cpld_data[] = {
{},
{PSU_1, CPLD_REG_PSU_1_STATUS_OFFSET, CPLD_REG_PSU_PRESENT_MASK, CPLD_REG_PSU_POWER_GOOD_MASK},
{PSU_2, CPLD_REG_PSU_2_STATUS_OFFSET, CPLD_REG_PSU_PRESENT_MASK, CPLD_REG_PSU_POWER_GOOD_MASK}
};

static volatile uint8_t* cpld_base__ = NULL;
/*
 * This function will be called prior to any other onlp_psui functions.
 */
int
onlp_psui_init(void)
{
    /*
     * Map the CPLD address
     */
    cpld_base__ = onlp_mmap(CPLD_BASE_ADDRESS, getpagesize(), __FILE__);
    if(cpld_base__ == NULL || cpld_base__ == MAP_FAILED) {
        return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
    {
        { }, /* Not used */
        {
            { ONLP_PSU_ID_CREATE(1), "PSU-1", 0, { 0 } },
            { 0 }, { 0 },
            0x0,
            0x0,
            0,
            0,
            0,
            0,
            0,
            0,
        },
        {
            { ONLP_PSU_ID_CREATE(2), "PSU-2", 0, { 0 } },
            { 0 }, { 0 },
            0x0,
            0x0,
            0,
            0,
            0,
            0,
            0,
            0,
        }
    };


int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    unsigned int bus_id = 1, pid = ONLP_OID_ID_GET(id);
    unsigned short volt_out=0, cur_out=0, pow_out=0;
    unsigned char i2c_addr, val=0, data[2]={0};
    int rc=0, psu_model = PSU_MODULE_TYPE_UNKNOWN;

    *info = pinfo[ONLP_OID_ID_GET(id)];

    val = cpld_base__[psu_cpld_data[pid].cpld_addr];

    if ((val & psu_cpld_data[pid].present_mask) == 0 )
    {
        info->status |= ONLP_PSU_STATUS_PRESENT;
    }
    else
    {
        info->status &= ~ONLP_PSU_STATUS_PRESENT;
        return ONLP_STATUS_OK;
    }

    psu_model = as4600_54t_get_psu_type(pid, info->model, sizeof(info->model));

    switch (psu_model)
    {
        case PSU_MODULE_TYPE_AC_CHICONY_B2F:
        case PSU_MODULE_TYPE_AC_CHICONY_F2B:
            /* This PSU has 1 fan and 1 thermal sensor. Create the child oid.
             */
            info->hdr.coids[0] = ONLP_FAN_ID_CREATE(pid+2);
            info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(pid+3);
            info->caps |= ONLP_PSU_CAPS_AC;

            i2c_addr = psu_i2c_data[pid].i2c_pmbus_addr;
            rc = I2C_nRead(bus_id, i2c_addr, I2C_AC_PSU_CHICONY_REG_STATUS, 2, data);
            if(0 != rc)
            {
                return ONLP_STATUS_E_INTERNAL;
            }

            /* update the power good status
             */
            if ( (data[1] & I2C_AC_PSU_CHICONY_POWER_GOOD_NETAGED_MASK) == I2C_AC_PSU_CHICONY_POWER_GOOD_NETAGED_MASK)
            {
                info->status |= ONLP_PSU_STATUS_FAILED;
            }
            else
            {
                info->status &= ~ONLP_PSU_STATUS_FAILED;
            }

            /* update output voltage
             */
            rc = I2C_nRead(bus_id, i2c_addr, I2C_AC_PSU_CHICONY_REG_OUT_VOLT, 2, data);
            if(0 != rc)
            {
                return ONLP_STATUS_E_INTERNAL;
            }
            volt_out = (data[1] << 8) + data[0];
            info->mvout = parse_literal_format(volt_out, PSU_PMBUS_DATA_MULTIPLIER);
            info->caps |= ONLP_PSU_CAPS_VOUT;

            /* update output current
             */
            rc = I2C_nRead(bus_id, i2c_addr, I2C_AC_PSU_CHICONY_REG_OUT_CURRENT, 2, data);
            if(0 != rc)
            {
                return ONLP_STATUS_E_INTERNAL;
            }
            cur_out = (data[1] << 8) + data[0];
            info->miout = parse_literal_format(cur_out, PSU_PMBUS_DATA_MULTIPLIER);
            info->caps |= ONLP_PSU_CAPS_IOUT;

            /* CHICONY: Not support
             */
            info->mvin = 0;

            /* update output power
             */
            rc = I2C_nRead(bus_id, i2c_addr, I2C_AC_PSU_CHICONY_REG_OUT_POWER, 2, data);
            if(0 != rc)
            {
                return ONLP_STATUS_E_INTERNAL;
            }
            pow_out = (data[1] << 8) + data[0];
            info->mpout = parse_literal_format(pow_out, PSU_PMBUS_DATA_MULTIPLIER);
            info->caps |= ONLP_PSU_CAPS_POUT;

            rc = ONLP_STATUS_OK;
            break;

        case PSU_MODULE_TYPE_DC_UMEC_48V_F2B:
        case PSU_MODULE_TYPE_DC_UMEC_48V_B2F:
            /* This PSU has 1 fan. Create the child oid.
             */
            info->hdr.coids[0] = ONLP_FAN_ID_CREATE(pid+2);
            info->caps |= ONLP_PSU_CAPS_DC48;

            /* update the power good status
             */
            if ((val & psu_cpld_data[pid].power_good_mask) == psu_cpld_data[pid].power_good_mask)
            {
                info->status |= ONLP_PSU_STATUS_FAILED;
            }
            else
            {
                info->status &= ~ONLP_PSU_STATUS_FAILED;
            }

            rc = ONLP_STATUS_OK;
            break;

        case PSU_MODULE_TYPE_UNKNOWN: /* User insert a nuknown PSU or unplugged.*/
            info->status |= ONLP_PSU_STATUS_UNPLUGGED;
            rc = ONLP_STATUS_OK;
            break;

        default:
            rc = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }

    return rc;

}

/*
 * This is an optional generic ioctl() interface.
 * Its purpose is to allow future expansion and
 * custom functionality that is not otherwise exposed
 * in the standard interface.
 *
 * The semantics of this function are platform specific.
 * This function is completely optional.
 */
int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
