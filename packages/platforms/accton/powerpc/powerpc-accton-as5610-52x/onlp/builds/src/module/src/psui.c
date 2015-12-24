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
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "powerpc_accton_as5610_52x_log.h"
#include "platform_lib.h"

#define CPLD_BASE_ADDRESS    		0xEA000000

/* PSU status register in CPLD
 */
#define CPLD_REG_PSU1_STATUS       0x2
#define CPLD_REG_PSU2_STATUS       0x1
#define CPLD_REG_SYS_STATUS        0x3
#define CPLD_PSU_PRESENT_MASK      0x1
#define CPLD_PSU_POWER_GOOD_MASK   0x2

/* i2c device info */
#define I2C_PSU_BUS_ID                  0
#define I2C_AC_PSU1_SLAVE_ADDR_CFG      0x3E
#define I2C_AC_PSU2_SLAVE_ADDR_CFG      0x3D

#define PSU_PMBUS_REG_READ_VIN     0x88
#define PSU_PMBUS_REG_READ_VOUT    0x8B
#define PSU_PMBUS_REG_READ_IIN     0x89
#define PSU_PMBUS_REG_READ_IOUT    0x8C
#define PSU_PMBUS_REG_READ_PIN     0x97
#define PSU_PMBUS_REG_READ_POUT    0x96
#define PSU_PMBUS_REG_VOUT_MODE    0x20

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static volatile uint8_t* cpld_base__ = NULL;

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

int
psu_cpr_4011_info_get(onlp_psu_info_t* info)
{
    int index = ONLP_OID_ID_GET(info->hdr.id);
    unsigned char mux_ch[]   = {0x2, 0x4};
    unsigned char cfg_addr[] = {I2C_AC_PSU1_SLAVE_ADDR_CFG, I2C_AC_PSU2_SLAVE_ADDR_CFG};

    /* Set capability
     */
    info->caps = ONLP_PSU_CAPS_AC;

	if (info->status & ONLP_PSU_STATUS_FAILED) {
	    return ONLP_STATUS_OK;
	}

    /* Set the associated oid_table */
    info->hdr.coids[0] = ONLP_FAN_ID_CREATE(index + 1);
    info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(index + 8);

    /* Open channel for PSU
     */
    if (as5610_52x_i2c0_pca9548_channel_set(mux_ch[index-1]) != 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Read voltage, current and power */
    if (pmbus_read_vout_data(I2C_PSU_BUS_ID, cfg_addr[index-1], &info->mvout) == 0) {
        info->caps |= ONLP_PSU_CAPS_VOUT;
    }

    if (pmbus_read_literal_data(I2C_PSU_BUS_ID, cfg_addr[index-1], PSU_PMBUS_REG_READ_VIN,  &info->mvin) == 0) {
        info->caps |= ONLP_PSU_CAPS_VIN;
    }

    if (pmbus_read_literal_data(I2C_PSU_BUS_ID, cfg_addr[index-1], PSU_PMBUS_REG_READ_IOUT, &info->miout) == 0) {
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }

    if (pmbus_read_literal_data(I2C_PSU_BUS_ID, cfg_addr[index-1], PSU_PMBUS_REG_READ_IIN,  &info->miin) == 0) {
        info->caps |= ONLP_PSU_CAPS_IIN;
    }

    if (pmbus_read_literal_data(I2C_PSU_BUS_ID, cfg_addr[index-1], PSU_PMBUS_REG_READ_POUT, &info->mpout) == 0) {
        info->caps |= ONLP_PSU_CAPS_POUT;
    }

    if (pmbus_read_literal_data(I2C_PSU_BUS_ID, cfg_addr[index-1], PSU_PMBUS_REG_READ_PIN,  &info->mpin) == 0) {
        info->caps |= ONLP_PSU_CAPS_PIN;
    }

    /* Close psu channel
     */
    if (as5610_52x_i2c0_pca9548_channel_set(0) != 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
psu_um400d_info_get(onlp_psu_info_t* info)
{
    int index = ONLP_OID_ID_GET(info->hdr.id);

    /* Set capability
     */
    info->caps = ONLP_PSU_CAPS_DC48;

    if (info->status & ONLP_PSU_STATUS_FAILED) {
        return ONLP_STATUS_OK;
    }

    /* Set the associated oid_table */
    info->hdr.coids[0] = ONLP_FAN_ID_CREATE(index + 1);

    return ONLP_STATUS_OK;
}

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
{
    { }, /* Not used */
    {
        { ONLP_PSU_ID_CREATE(1), "PSU-1", 0 },
    },
    {
        { ONLP_PSU_ID_CREATE(2), "PSU-2", 0 },
    }
};

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int   ret = ONLP_STATUS_OK;
    int index = ONLP_OID_ID_GET(id);
    unsigned char cpld_offset, data;
    as5610_52x_psu_type_t psu_type;

    VALIDATE(id);

    *info = pinfo[index]; /* Set the onlp_oid_hdr_t */
    info->caps = 0;
    info->mvout = 0;
    info->mvin  = 0;
    info->miout = 0;
    info->miin  = 0;
    info->mpout = 0;
    info->mpin  = 0;
    ONLP_OID_TABLE_CLEAR(info->hdr.coids);

    /* Get the present state */
    cpld_offset = (index == 1) ? CPLD_REG_PSU1_STATUS: CPLD_REG_PSU2_STATUS;
    data = cpld_base__[cpld_offset];

    if (data & CPLD_PSU_PRESENT_MASK) {
        info->status &= ~ONLP_PSU_STATUS_PRESENT;
        return ONLP_STATUS_OK;
    }
    info->status |= ONLP_PSU_STATUS_PRESENT;

    /* Get power good status */
    if (!(data & CPLD_PSU_POWER_GOOD_MASK)) {
        info->status |=  ONLP_PSU_STATUS_FAILED;
    }

    /* Get PSU type
     */
    psu_type = as5610_52x_get_psu_type(index, info->model, sizeof(info->model));

    switch (psu_type) {
        case PSU_TYPE_AC_F2B:
        case PSU_TYPE_AC_B2F:
            ret = psu_cpr_4011_info_get(info);
            break;
        case PSU_TYPE_DC_48V_F2B:
        case PSU_TYPE_DC_48V_B2F:
            ret = psu_um400d_info_get(info);
            break;
        default:
            ret = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }

    return ret;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

