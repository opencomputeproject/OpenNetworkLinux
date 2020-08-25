/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 Accton Technology Corporation.
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
#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include "platform_lib.h"

#define EEPROM_I2C_ADDR     0x50
#define EEPROM_START_OFFSET 0x0
#define NUM_OF_SFP_PORT 	30
static const int port_bus_index[NUM_OF_SFP_PORT] = {
    33, 34, 35, 36, 37, 38,
    41, 42, 43, 44, 45, 46,
    49, 50, 51, 52, 53, 54, 55, 56,
    57, 58, 59, 60, 61, 62, 63, 64,
    31, 30
};

#define PORT_BUS_INDEX(port) (port_bus_index[port])
#define PORT_FORMAT	 "/sys/bus/i2c/devices/%d-0050/%s"
#define MODULE_PRESENT_CPLD2_FORMAT "/sys/bus/i2c/devices/12-0061/module_present_%d"
#define MODULE_PRESENT_CPLD3_FORMAT "/sys/bus/i2c/devices/13-0062/module_present_%d"
#define MODULE_RESET_CPLD2_FORMAT   "/sys/bus/i2c/devices/12-0061/module_reset_%d"
#define MODULE_RESET_CPLD3_FORMAT   "/sys/bus/i2c/devices/13-0062/module_reset_%d"
#define MODULE_RXLOS_FORMAT             "/sys/bus/i2c/devices/12-0061/module_rx_los_%d"
#define MODULE_TXDISABLE_FORMAT         "/sys/bus/i2c/devices/12-0061/module_tx_disable_%d"


#define IS_SFP_PORT(_port) (_port >= 28 && _port <= 29)
#define IS_QSFP_PORT(_port) (_port >= 0 && _port <= 27)

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
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = 0; p < NUM_OF_SFP_PORT; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

static int sfp_remap(int port) {
    if (IS_SFP_PORT(port)) { /*Reverse 10G OOBF ports*/
        return (port == 28)? 29: 28;
    } else {
        return port;
    }
}

int onlp_sfpi_is_present(int port)
{
    /*
     * Return 1 if present.
     * Return 0 if not present.
     * Return < 0 if error.
     */
    int present;
    if ((port >= 0 && port <=15) || (IS_SFP_PORT(port))) {
        if (onlp_file_read_int(&present, MODULE_PRESENT_CPLD2_FORMAT,
                               sfp_remap(port)+1) < 0) {
            AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    else if (port >= 16 && port <=27) {
        if (onlp_file_read_int(&present, MODULE_PRESENT_CPLD3_FORMAT, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    } else {
        return ONLP_STATUS_E_INVALID;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int i=0, val=0;
    /* Populate bitmap */
    for(i = 0; i < NUM_OF_SFP_PORT; i++)
    {
        if(IS_SFP_PORT(i))
        {
            if (onlp_sfpi_is_present(i) == 1) {
                val=0;
                if (onlp_file_read_int(&val, MODULE_RXLOS_FORMAT, sfp_remap(i)+1) < 0)
                {
                    AIM_LOG_ERROR("Unable to read rx_loss status from port(%d)\r\n", i);
                }
                if(val)
                    AIM_BITMAP_MOD(dst, i, 1);
                else
                    AIM_BITMAP_MOD(dst, i, 0);
            } else {
                AIM_BITMAP_MOD(dst, i, 0);
            }
        }
        else
            AIM_BITMAP_MOD(dst, i, 0);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    int size = 0;
    if(port < 0 || port >= NUM_OF_SFP_PORT)
        return ONLP_STATUS_E_INTERNAL;

    if(onlp_file_read(data, 256, &size, PORT_FORMAT, PORT_BUS_INDEX(port), "eeprom") != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if(size != 256) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = PORT_BUS_INDEX(port);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int bus = PORT_BUS_INDEX(port);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = PORT_BUS_INDEX(port);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int bus = PORT_BUS_INDEX(port);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv;

    switch(control)
    {
    case ONLP_SFP_CONTROL_RESET:
    {
        if(IS_QSFP_PORT(port)) {
            char *reset_node = MODULE_RESET_CPLD2_FORMAT;
            if (port > 15) {
                reset_node = MODULE_RESET_CPLD3_FORMAT;
            }

            if (onlp_file_write_int(!!value, reset_node, (port+1)) < 0) {
                AIM_LOG_ERROR("Unable to set tx_disable status to port(%d)\r\n", port);
                rv = ONLP_STATUS_E_INTERNAL;
            }
            else {
                rv = ONLP_STATUS_OK;
            }
        }
        else {
            rv = ONLP_STATUS_E_UNSUPPORTED;
        }
        break;
    }
    case ONLP_SFP_CONTROL_TX_DISABLE:
    {
        if(IS_SFP_PORT(port)) {
            if (onlp_file_write_int(!!value, MODULE_TXDISABLE_FORMAT,
                                    sfp_remap(port)+1) < 0) {
                AIM_LOG_ERROR("Unable to set tx_disable status to port(%d)\r\n", port);
                rv = ONLP_STATUS_E_INTERNAL;
            }
            else {
                rv = ONLP_STATUS_OK;
            }
        }
        else {
            rv = ONLP_STATUS_E_UNSUPPORTED;
        }
        break;
    }

    default:
        rv = ONLP_STATUS_E_UNSUPPORTED;
        break;
    }

    return rv;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int rv;

    switch(control)
    {
    case ONLP_SFP_CONTROL_RESET:
    {
        if(IS_QSFP_PORT(port)) {
            char *reset_node = MODULE_RESET_CPLD2_FORMAT;
            if (port > 15) {
                reset_node = MODULE_RESET_CPLD3_FORMAT;
            }

            if (onlp_file_read_int(value, reset_node, (port+1)) < 0) {
                AIM_LOG_ERROR("Unable to set tx_disable status to port(%d)\r\n", port);
                rv = ONLP_STATUS_E_INTERNAL;
            }
            else {
                rv = ONLP_STATUS_OK;
            }
        }
        else {
            rv = ONLP_STATUS_E_UNSUPPORTED;
        }
        break;
    }

    case ONLP_SFP_CONTROL_RX_LOS:
    {
        if(IS_SFP_PORT(port))
        {
            if (onlp_file_read_int(value, MODULE_RXLOS_FORMAT,
                                   sfp_remap(port)+1) < 0)
            {
                AIM_LOG_ERROR("Unable to read rx_loss status from port(%d)\r\n", port);
                rv = ONLP_STATUS_E_INTERNAL;
            }
            else
            {
                rv = ONLP_STATUS_OK;
            }

        }
        else
        {
            rv = ONLP_STATUS_E_UNSUPPORTED;
        }
        break;
    }
    case ONLP_SFP_CONTROL_TX_DISABLE:
    {
        if (IS_SFP_PORT(port)) {
            if (onlp_file_read_int(value, MODULE_TXDISABLE_FORMAT,
                                   sfp_remap(port)+1) < 0) {
                AIM_LOG_ERROR("Unable to read tx_disabled status from port(%d)\r\n", port);
                rv = ONLP_STATUS_E_INTERNAL;
            }
            else {
                rv = ONLP_STATUS_OK;
            }
        }
        else {
            rv = ONLP_STATUS_E_UNSUPPORTED;
        }
        break;
    }

    default:
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }

    return rv;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

