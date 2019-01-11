/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2015 Accton Technology Corporation.
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
#include "x86_64_accton_as5812_54x_int.h"
#include "x86_64_accton_as5812_54x_log.h"

#define CPLD_MUX_BUS_START_INDEX 2

#define PORT_EEPROM_FORMAT              "/sys/bus/i2c/devices/%d-0050/eeprom"
#define MODULE_PRESENT_FORMAT		    "/sys/bus/i2c/devices/0-00%d/module_present_%d"
#define MODULE_RXLOS_FORMAT             "/sys/bus/i2c/devices/0-00%d/module_rx_los_%d"
#define MODULE_TXFAULT_FORMAT           "/sys/bus/i2c/devices/0-00%d/module_tx_fault_%d"
#define MODULE_TXDISABLE_FORMAT         "/sys/bus/i2c/devices/0-00%d/module_tx_disable_%d"
#define MODULE_PRESENT_ALL_ATTR_CPLD2	"/sys/bus/i2c/devices/0-0061/module_present_all"
#define MODULE_PRESENT_ALL_ATTR_CPLD3	"/sys/bus/i2c/devices/0-0062/module_present_all"
#define MODULE_RXLOS_ALL_ATTR_CPLD2	    "/sys/bus/i2c/devices/0-0061/module_rx_los_all"
#define MODULE_RXLOS_ALL_ATTR_CPLD3	    "/sys/bus/i2c/devices/0-0062/module_rx_los_all"

static int front_port_bus_index(int port)
{
    int rport = 0;

    switch (port)
    {
        case 49:
            rport = 50;
            break;
        case 50:
            rport = 52;
            break;
        case 51:
            rport = 49;
            break;
        case 52:
            rport = 51;
            break;
        default:
            rport = port;
            break;
    }

    return (rport + CPLD_MUX_BUS_START_INDEX);
}

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
     * Ports {0, 54}
     */
    int p;

    for(p = 0; p < 54; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_port_map(int port, int* rport)
{
    /*
     * The QSFP port numbering on the powerpc-as5812-54x-r0b platform
     * differs from the numbering on the production
     * powerpc-accton-as5812-54x-r0 platform.
     *
     * The R0B box numbers the front panel QSFP ports as follows:
     *
     *    49 50
     *    51 52
     *    53 54
     *
     * The production box numbers the front panel QSFP ports as follows:
     *
     *    49 52
     *    50 53
     *    51 54
     *
     * The kernel SFP driver for all 5710 platforms uses the production
     * portmapping. When running on the R0B platform we need to convert
     * the logical SFP port number (from the front panel) to the physical
     * SFP port number used by the kernel driver.
     *
     * SFP port numbers here are 0-based.
     */
    switch(port)
        {
        case 48:
        case 53:
            /* These QSFP ports are numbered the same on both platforms */
            *rport = port; break;
        case 50:
            *rport = 49; break;
        case 52:
            *rport = 50; break;
        case 49:
            *rport = 51; break;
        case 51:
            *rport = 52; break;
        default:
            /* All others are identical */
            *rport = port; break;
        }

    return ONLP_STATUS_OK;
}

/**
 * The CPLD registers on the as5812 use the original R0B QSFP (horizontal)
 * portmapping.
 *
 * While the SFP driver handles the remap correctly for sfp_active_port()
 * for both the R0 and R0B. When we have to interpret the CPLD register
 * values directly, however, we need to apply the correct mapping from R0B -> R0.
 */
static void
port_qsfp_cpld_map__(int port, int* rport)
{
    switch(port)
    {
        case 53:
        case 48: /* The same */ *rport = port; break;
        case 50: *rport = 49; break;
        case 52: *rport = 50; break;
        case 49: *rport = 51; break;
        case 51: *rport = 52; break;
        default: *rport = port; break;
    }
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
    int addr = (port < 24) ? 61 : 62;
    
	if (onlp_file_read_int(&present, MODULE_PRESENT_FORMAT, addr, (port+1)) < 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t bytes[7];
    FILE* fp;

    /* Read present status of port 0~23 */
    fp = fopen(MODULE_PRESENT_ALL_ATTR_CPLD2, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the module_present_all device file of CPLD2.");
        return ONLP_STATUS_E_INTERNAL;
    }

    int count = fscanf(fp, "%x %x %x", bytes+0, bytes+1, bytes+2);
    fclose(fp);
    if(count != 3) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields the module_present_all device file of CPLD2.");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Read present status of port 24~53 */
    fp = fopen(MODULE_PRESENT_ALL_ATTR_CPLD3, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the module_present_all device file of CPLD3.");
        return ONLP_STATUS_E_INTERNAL;
    }

    count = fscanf(fp, "%x %x %x %x", bytes+3, bytes+4, bytes+5, bytes+6);
    fclose(fp);
    if(count != 4) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields the module_present_all device file of CPLD3.");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Mask out non-existant QSFP ports */
    bytes[6] &= 0x3F;

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t presence_all = 0 ;
    for(i = AIM_ARRAYSIZE(bytes)-1; i >= 0; i--) {
        presence_all <<= 8;
        presence_all |= bytes[i];
    }

    /* Populate bitmap */
    for(i = 0; presence_all; i++) {
        int p;
        port_qsfp_cpld_map__(i, &p);
        AIM_BITMAP_MOD(dst, p, (presence_all & 1));
        presence_all >>= 1;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t bytes[6];
    uint32_t *ptr = bytes;
    FILE* fp;

    /* Read present status of port 0~23 */
    int addr, i = 0;

    for (addr = 61; addr <= 62; addr++) {
        if (addr == 61) {
            fp = fopen(MODULE_RXLOS_ALL_ATTR_CPLD2, "r");
        }
        else {
            fp = fopen(MODULE_RXLOS_ALL_ATTR_CPLD3, "r");
        }

        if(fp == NULL) {
            AIM_LOG_ERROR("Unable to open the module_rx_los_all device file of CPLD(0x%d)", addr);
            return ONLP_STATUS_E_INTERNAL;
        }

        int count = fscanf(fp, "%x %x %x", ptr+0, ptr+1, ptr+2);
        fclose(fp);
        if(count != 3) {
            /* Likely a CPLD read timeout. */
            AIM_LOG_ERROR("Unable to read all fields from the module_rx_los_all device file of CPLD(0x%d)", addr);
            return ONLP_STATUS_E_INTERNAL;
        }

        ptr += count;
    }

    /* Convert to 64 bit integer in port order */
    i = 0;
    uint64_t rx_los_all = 0 ;
    for(i = 5; i >= 0; i--) {
        rx_los_all <<= 8;
        rx_los_all |= bytes[i];
    }

    /* Populate bitmap */
    for(i = 0; rx_los_all; i++) {
        int p;
        port_qsfp_cpld_map__(i, &p);
        AIM_BITMAP_MOD(dst, p, (rx_los_all & 1));
        rx_los_all >>= 1;
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
    memset(data, 0, 256);

	if(onlp_file_read(data, 256, &size, PORT_EEPROM_FORMAT, front_port_bus_index(port)) != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (size != 256) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d), size is different!\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    FILE* fp;
    char file[64] = {0};
    
    sprintf(file, PORT_EEPROM_FORMAT, front_port_bus_index(port));
    fp = fopen(file, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the eeprom device file of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (fseek(fp, 256, SEEK_CUR) != 0) {
        fclose(fp);
        AIM_LOG_ERROR("Unable to set the file position indicator of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    int ret = fread(data, 1, 256, fp);
    fclose(fp);
    if (ret != 256) {
        AIM_LOG_ERROR("Unable to read the module_eeprom device file of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = front_port_bus_index(port);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int bus = front_port_bus_index(port);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = front_port_bus_index(port);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int bus = front_port_bus_index(port);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv;

    if (port < 0 || port >= 48) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    int addr = (port < 24) ? 61 : 62;

    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                if (onlp_file_write_int(value, MODULE_TXDISABLE_FORMAT, addr, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to set tx_disable status to port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
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

    if (port < 0 || port >= 48) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    int addr = (port < 24) ? 61 : 62;

    switch(control)
        {
        case ONLP_SFP_CONTROL_RX_LOS:
            {
            	if (onlp_file_read_int(value, MODULE_RXLOS_FORMAT, addr, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to read rx_loss status from port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
                }
                break;
            }

        case ONLP_SFP_CONTROL_TX_FAULT:
            {
            	if (onlp_file_read_int(value, MODULE_TXFAULT_FORMAT, addr, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to read tx_fault status from port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
                }
                break;
            }

        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
            	if (onlp_file_read_int(value, MODULE_TXDISABLE_FORMAT, addr, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to read tx_disabled status from port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
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

