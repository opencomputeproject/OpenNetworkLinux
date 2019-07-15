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

#define EEPROM_I2C_ADDR 0x50
#define EEPROM_START_OFFSET 0x0
#define NUM_OF_SFP_PORT 		80
static const int port_bus_index[NUM_OF_SFP_PORT] = {
33, 34, 35, 36, 37, 38, 39, 40,
41, 42, 43, 44, 45, 46, 47, 48,
49, 50, 51, 52, 53, 54, 55, 56,
57, 58, 59, 60, 61, 62, 63, 64,
65, 66, 67, 68, 69, 70, 71, 72,
97, 98, 99, 100, 101, 102, 103,
104, 105, 106, 107, 108, 109,
110, 111, 112, 113, 114, 115,
116, 117, 118, 119, 120, 121,
122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 
134, 135, 136,
};

#define PORT_BUS_INDEX(port) (port_bus_index[port])
#define PORT_FORMAT	 "/sys/bus/i2c/devices/%d-0050/%s"
#define MODULE_PRESENT_BOTTOM_BOARD_CPLD1_FORMAT "/sys/bus/i2c/devices/12-0062/module_present_%d"
#define MODULE_PRESENT_BOTTOM_BOARD_CPLD2_FORMAT "/sys/bus/i2c/devices/13-0063/module_present_%d"
#define MODULE_PRESENT_TOP_BOARD_CPLD3_FORMAT "/sys/bus/i2c/devices/76-0062/module_present_%d"
#define MODULE_PRESENT_TOP_BOARD_CPLD4_FORMAT "/sys/bus/i2c/devices/77-0063/module_present_%d"

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
     * Ports {0, 80}
     */
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = 0; p < NUM_OF_SFP_PORT; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
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

    if (port >= 0 && port <=19) {
        if (onlp_file_read_int(&present, MODULE_PRESENT_BOTTOM_BOARD_CPLD1_FORMAT, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    else if (port >= 20 && port <=39){
        if (onlp_file_read_int(&present, MODULE_PRESENT_BOTTOM_BOARD_CPLD2_FORMAT, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    else if (port >= 40 && port <=59){
        if (onlp_file_read_int(&present, MODULE_PRESENT_TOP_BOARD_CPLD3_FORMAT, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    else if (port >= 60 && port <=79){
        if (onlp_file_read_int(&present, MODULE_PRESENT_TOP_BOARD_CPLD4_FORMAT, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
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
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

