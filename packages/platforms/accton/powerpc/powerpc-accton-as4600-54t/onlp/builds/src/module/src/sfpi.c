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
#include <onlp/platformi/sfpi.h>
#include <onlplib/mmap.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include "powerpc_accton_as4600_54t_log.h"

#include "platform_lib.h"

#define CPLD_BASE_ADDRESS    0xEA000000
#define CPLD_REG_SFP_PRESENT 0x0A
#define CPLD_REG_SFP_RX_LOSS    0x0B
#define CPLD_REG_SFP_TX_FAIL    0x0C
#define CPLD_REG_SFP_TX_DISABLE 0x0D
#define CPLD_SFP_1_BIT_MASK  0x80
#define CPLD_SFP_2_BIT_MASK  0x40
#define CPLD_SFP_3_BIT_MASK  0x20
#define CPLD_SFP_4_BIT_MASK  0x10

#define I2C_SLAVE_ADDR_PCA9548  0x70

#define PCA9548_PORT_0_BIT_MASK 0x01
#define PCA9548_PORT_1_BIT_MASK 0x02
#define PCA9548_PORT_2_BIT_MASK 0x04
#define PCA9548_PORT_3_BIT_MASK 0x08

#define MAX_I2C_BUSSES     2
#define I2C_BUFFER_MAXSIZE 16

static volatile uint8_t* cpld_base__ = NULL;

int
onlp_sfpi_init(void)
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
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    /*
     * Report all SFP-capable ports.
     * BA: SFP 49, 50, 51, 52
     */
    AIM_BITMAP_SET(bmap, 48);
    AIM_BITMAP_SET(bmap, 49);
    AIM_BITMAP_SET(bmap, 50);
    AIM_BITMAP_SET(bmap, 51);

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    unsigned char val;
    val = cpld_base__[CPLD_REG_SFP_PRESENT];
    
    AIM_BITMAP_MOD(dst, 48, (val & CPLD_SFP_1_BIT_MASK) ? 0 : 1);
    AIM_BITMAP_MOD(dst, 49, (val & CPLD_SFP_2_BIT_MASK) ? 0 : 1);
    AIM_BITMAP_MOD(dst, 50, (val & CPLD_SFP_3_BIT_MASK) ? 0 : 1);
    AIM_BITMAP_MOD(dst, 51, (val & CPLD_SFP_4_BIT_MASK) ? 0 : 1);

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    unsigned char val;
    val = cpld_base__[CPLD_REG_SFP_RX_LOSS];
    
    AIM_BITMAP_MOD(dst, 48, (val & CPLD_SFP_1_BIT_MASK) ? 1 : 0);
    AIM_BITMAP_MOD(dst, 49, (val & CPLD_SFP_2_BIT_MASK) ? 1 : 0);
    AIM_BITMAP_MOD(dst, 50, (val & CPLD_SFP_3_BIT_MASK) ? 1 : 0);
    AIM_BITMAP_MOD(dst, 51, (val & CPLD_SFP_4_BIT_MASK) ? 1 : 0);

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    unsigned int regOffset;
    unsigned char val, mask;
    int rc = 0;

    regOffset = CPLD_REG_SFP_PRESENT;

    switch(port)
        {
        case 48:
            mask = CPLD_SFP_1_BIT_MASK;
            break;
        case 49:
            mask = CPLD_SFP_2_BIT_MASK;
            break;
        case 50:
            mask = CPLD_SFP_3_BIT_MASK;
            break;
        case 51:
            mask = CPLD_SFP_4_BIT_MASK;
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
        }

    /*
     * Return 1 if present.
     * Return 0 if not present.
     * Return < 0 if error.
     */
    val = cpld_base__[regOffset];
    rc = (val & mask) ? 0 : 1;

    return rc;
}

static int
onlp_sfpi_read_addr__(int port, int addr, unsigned char *data)
{
    int i, rc = 0;
    unsigned int bus_id = 0;
    unsigned char mask = 0;
    unsigned int i2c_addr;

    /*
     * switch channel on i2c multiplexer
     */
    switch(port)
        {
        case 48:
            mask = PCA9548_PORT_0_BIT_MASK;
            break;
        case 49:
            mask = PCA9548_PORT_1_BIT_MASK;
            break;
        case 50:
            mask = PCA9548_PORT_2_BIT_MASK;
            break;
        case 51:
            mask = PCA9548_PORT_3_BIT_MASK;
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
        }
    i2c_addr = I2C_SLAVE_ADDR_PCA9548;
    rc = I2C_nWrite(bus_id, i2c_addr, 0, 1, &mask);
    if(0 != rc)
        return ONLP_STATUS_E_INTERNAL;

    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    i2c_addr = addr;
    for(i=0; i<16; i++) {
        rc = I2C_nRead(bus_id, i2c_addr, i*I2C_BUFFER_MAXSIZE, I2C_BUFFER_MAXSIZE, data + i*I2C_BUFFER_MAXSIZE);
        if(0 != rc)
            return ONLP_STATUS_E_INTERNAL;
    }

    return 0;
}

int
onlp_sfpi_eeprom_read(int port, int dev_addr, unsigned char *data)
{
    if ((dev_addr == SFP_IDPROM_ADDR) || (dev_addr == SFP_DOM_ADDR)) {
        return onlp_sfpi_read_addr__(port, dev_addr, data);
    }
    return ONLP_STATUS_E_PARAM;
}

/*
 * Manually enable or disable the given SFP.
 *
 */
static int
tx_enable_set__(int port, int enable)
{
    /* If enable is 1, enable the SFP */
    /* If enable is 0, disable the SFP */
    unsigned int regOffset;
    unsigned char val, mask;

    regOffset = CPLD_REG_SFP_TX_DISABLE;

    switch(port)
    {
    case 48:
        mask = CPLD_SFP_1_BIT_MASK;
        break;
    case 49:
        mask = CPLD_SFP_2_BIT_MASK;
        break;
    case 50:
        mask = CPLD_SFP_3_BIT_MASK;
        break;
    case 51:
        mask = CPLD_SFP_4_BIT_MASK;
        break;
    default:
        return ONLP_STATUS_E_INVALID;
    }

    /* read the current status.
     */
    val = cpld_base__[regOffset];

    /* CPLD value:
     * 0: transmit Enable
     * 1: transmit Disable
     */
    if ( enable == 1)
    {
        val &= ~mask;
    }
    else
    {
        val |= mask;
    }

    cpld_base__[regOffset] = val;

    return ONLP_STATUS_OK;
}

/*
 * Returns whether the SFP is currently enabled or disabled.
 */
static int
tx_enable_get__(int port, int* enable)
{
    unsigned int regOffset;
    unsigned char val, mask;

    regOffset = CPLD_REG_SFP_TX_DISABLE;

    switch(port)
    {
    case 48:
        mask = CPLD_SFP_1_BIT_MASK;
        break;
    case 49:
        mask = CPLD_SFP_2_BIT_MASK;
        break;
    case 50:
        mask = CPLD_SFP_3_BIT_MASK;
        break;
    case 51:
        mask = CPLD_SFP_4_BIT_MASK;
        break;
    default:
        return ONLP_STATUS_E_INTERNAL;
    }

    val = cpld_base__[regOffset];

    /* Return whether the SFP is currently enabled
     * =0, if disable
     * =1, if enalbe
     */
    *enable = (val & mask) ? 0 : 1;

    return ONLP_STATUS_OK;
}


static int
control_flags_get__(int port, uint32_t* status)
{
    unsigned int regOffset1, regOffset2;
    unsigned char val1, val2, mask;

    regOffset1 = CPLD_REG_SFP_RX_LOSS;
    regOffset2 = CPLD_REG_SFP_TX_FAIL;

    *status = 0;

    switch(port)
    {
    case 48:
        mask = CPLD_SFP_1_BIT_MASK;
        break;
    case 49:
        mask = CPLD_SFP_2_BIT_MASK;
        break;
    case 50:
        mask = CPLD_SFP_3_BIT_MASK;
        break;
    case 51:
        mask = CPLD_SFP_4_BIT_MASK;
        break;
    default:
        return ONLP_STATUS_E_INTERNAL;
    }

    val1 = cpld_base__[regOffset1];
    val2 = cpld_base__[regOffset2];

    /* Report any current status flags for the SFP */
    if ((val1 & mask) == mask)
    {
        *status |= ONLP_SFP_CONTROL_FLAG_RX_LOS;
    }

    if ((val2 & mask) == mask)
    {
        *status |= ONLP_SFP_CONTROL_FLAG_TX_FAULT;
    }

    return ONLP_STATUS_OK;
}


int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            return tx_enable_set__(port, !value);
        default:
            return ONLP_STATUS_E_UNSUPPORTED;
        }
}
int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                int rv;
                ONLP_IF_ERROR_RETURN(rv = tx_enable_get__(port, value));
                *value = ! *value;
                return ONLP_STATUS_OK;
            }

        case ONLP_SFP_CONTROL_RX_LOS:
            {
                uint32_t flags = 0;
                ONLP_IF_ERROR_RETURN(control_flags_get__(port, &flags));
                *value = (flags & ONLP_SFP_CONTROL_FLAG_RX_LOS) ? 1 : 0;
                return ONLP_STATUS_OK;
            }

        case ONLP_SFP_CONTROL_TX_FAULT:
            {
                uint32_t flags = 0;
                ONLP_IF_ERROR_RETURN(control_flags_get__(port, &flags));
                *value = (flags & ONLP_SFP_CONTROL_FLAG_TX_FAULT) ? 1 : 0;
                return ONLP_STATUS_OK;
            }

        default:
            return ONLP_STATUS_E_UNSUPPORTED;
        }
}

int
onlp_sfpi_denit(void)
{
    munmap((void*)cpld_base__, getpagesize());
    return 0;
}


