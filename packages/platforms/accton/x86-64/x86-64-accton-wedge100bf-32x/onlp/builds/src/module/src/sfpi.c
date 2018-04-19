/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
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
#include <onlplib/i2c.h>
#include <onlp/platformi/sfpi.h>
#include <onlplib/file.h>
#include "platform_lib.h"

#include "x86_64_accton_wedge100bf_32x_log.h"

#define BIT(i)          (1 << (i))
#define NUM_OF_SFP_PORT 32
static const int sfp_bus_index[] = {
   3,  2,  5,  4,  7,  6,  9, 8,
 11,  10, 13, 12, 15, 14, 17, 16,
 19, 18, 21, 20, 23, 22, 25, 24,
 27, 26, 29, 28, 31, 30, 33, 32
};

/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
int
onlp_sfpi_init(void)
{
    /* Called at initialization time */    
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    /*
     * Ports {0, 32}
     */
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = 0; p < NUM_OF_SFP_PORT; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

static uint8_t
onlp_sfpi_reg_val_to_port_sequence(uint8_t value, int revert)
{
    int i;
    uint8_t ret = 0;

    for (i = 0; i < 8; i++) {
        if (i % 2) {
            ret |= (value & BIT(i)) >> 1;
        }
        else {
            ret |= (value & BIT(i)) << 1;
        }
    }

    return revert ? ~ret : ret;
}

int
onlp_sfpi_is_present(int port)
{
    /*
     * Return 1 if present.
     * Return 0 if not present.
     * Return < 0 if error.
     */
    int present;
    int bus = (port < 16) ? 36 : 37;
    int addr = (port < 16) ? 0x22 : 0x23; /* pca9535 slave address */
    int offset;

    if (port < 8 || (port >= 16 && port <= 23)) {
        offset = 0;
    }
    else {
        offset = 1;
    }

    present = onlp_i2c_readb(bus, addr, offset, ONLP_I2C_F_FORCE);
    if (present < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    present = onlp_sfpi_reg_val_to_port_sequence(present, 0);
    return !(present & BIT(port % 8));
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int i;
    uint8_t bytes[4] = {0};

    for (i = 0; i < AIM_ARRAYSIZE(bytes); i++) {
        int bus = (i < 2) ? 36 : 37;
        int addr = (i < 2) ? 0x22 : 0x23; /* pca9535 slave address */
        int offset = (i % 2);

        bytes[i] = onlp_i2c_readb(bus, addr, offset, ONLP_I2C_F_FORCE);
        if (bytes[i] < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }

        bytes[i] = onlp_sfpi_reg_val_to_port_sequence(bytes[i], 1);
    }

    /* Convert to 32 bit integer in port order */
    i = 0;
    uint32_t presence_all = 0 ;
    for(i = AIM_ARRAYSIZE(bytes)-1; i >= 0; i--) {
        presence_all <<= 8;
        presence_all |= bytes[i];
    }

    /* Populate bitmap */
    for(i = 0; presence_all; i++) {
        AIM_BITMAP_MOD(dst, i, (presence_all & 1));
        presence_all >>= 1;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    return ONLP_STATUS_OK;
}

static int
sfpi_eeprom_read(int port, uint8_t devaddr, uint8_t data[256])
{
    int i;

    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    memset(data, 0, 256);

    for (i = 0; i < 128; i++) {
        int bus = sfp_bus_index[port];
        int val = onlp_i2c_readw(bus, devaddr, i*2, ONLP_I2C_F_FORCE);

        if (val < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }

        data[i]   = val & 0xff;
        data[i+1] = (val >> 8) & 0xff;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    return sfpi_eeprom_read(port, 0x50, data);
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    return sfpi_eeprom_read(port, 0x51, data);
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

