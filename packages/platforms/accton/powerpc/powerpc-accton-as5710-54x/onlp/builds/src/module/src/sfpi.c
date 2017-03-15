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

#include <fcntl.h> /* For O_RDWR && open */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <onlplib/i2c.h>
#include "platform_lib.h"

static int
as5710_54x_sfp_node_read_int(char *node_path, int *value, int data_len)
{
    int ret = 0;
    char buf[8] = {0};
    *value = 0;

    ret = deviceNodeReadString(node_path, buf, sizeof(buf), data_len);

    if (ret == 0) {
        *value = atoi(buf);
    }

    return ret;
}

static int set_active_port(int port)
{
    return deviceNodeWriteInt(SFP_HWMON_NODE(sfp_active_port), port, 0);
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
     * The QSFP port numbering on the powerpc-as5710-54x-r0b platform
     * differs from the numbering on the production
     * powerpc-accton-as5710-54x-r0 platform.
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

    if(platform_id == PLATFORM_ID_POWERPC_ACCTON_AS5710_54X_R0B) {
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
    }
    else {
        *rport = port;
    }
    return ONLP_STATUS_OK;
}

/**
 * The CPLD registers on the AS5710 use the original R0B QSFP (horizontal)
 * portmapping.
 *
 * While the SFP driver handles the remap correctly for sfp_active_port()
 * for both the R0 and R0B. When we have to interpret the CPLD register
 * values directly, however, we need to apply the correct mapping from R0B -> R0.
 */
static void
port_qsfp_cpld_map__(int port, int* rport)
{
    if(platform_id == PLATFORM_ID_POWERPC_ACCTON_AS5710_54X_R0) {
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
    else {
        *rport = port;
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

    if (set_active_port(port+1) != 0) {
        AIM_LOG_ERROR("Unable to set active port to port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (as5710_54x_sfp_node_read_int(SFP_HWMON_NODE(sfp_is_present), &present, 1) != 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        set_active_port(0);
        return ONLP_STATUS_E_INTERNAL;
    }

    set_active_port(0);

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t bytes[7];
    FILE* fp = fopen(SFP_HWMON_NODE(sfp_is_present_all), "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the sfp_is_present_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }
    int count = fscanf(fp, "%x %x %x %x %x %x %x",
                       bytes+0,
                       bytes+1,
                       bytes+2,
                       bytes+3,
                       bytes+4,
                       bytes+5,
                       bytes+6
                       );
    fclose(fp);
    if(count != 7) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields from the sfp_is_present_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Mask out non-existant QSFP ports */
    bytes[6] &= 0x3F;

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t presence_all = 0 ;
    for(i = 6; i >= 0; i--) {
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
    uint32_t bytes[7];
    FILE* fp = fopen(SFP_HWMON_NODE(sfp_rx_los_all), "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the sfp_rx_los_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }
    int count = fscanf(fp, "%x %x %x %x %x %x",
                       bytes+0,
                       bytes+1,
                       bytes+2,
                       bytes+3,
                       bytes+4,
                       bytes+5
                       );
    fclose(fp);
    if(count != 6) {
        AIM_LOG_ERROR("Unable to read all fields from the sfp_rx_los_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Convert to 64 bit integer in port order */
    int i = 0;
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
onlp_sfpi_eeprom_read(int port, int dev_addr, uint8_t data[256])
{
    char *eeprom_path;

    if (dev_addr == SFP_IDPROM_ADDR) {
        eeprom_path = SFP_HWMON_NODE(sfp_eeprom);
    } else if (dev_addr == SFP_DOM_ADDR) {
        eeprom_path = SFP_HWMON_DOM_NODE(eeprom);
    } else
        return ONLP_STATUS_E_PARAM;

    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    memset(data, 0, 256);

    if (set_active_port(port+1) != 0) {
        AIM_LOG_ERROR("Unable to set active port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (deviceNodeReadBinary(eeprom_path, (char*)data, 256, 256) != 0) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        set_active_port(0);
        return ONLP_STATUS_E_INTERNAL;
    }

    set_active_port(0);

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int rc;

    if (set_active_port(port+1) != 0) {
        AIM_LOG_ERROR("Unable to set active port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    rc= onlp_i2c_readb(SFP_BUS, devaddr, addr, ONLP_I2C_F_FORCE);

    set_active_port(0);

    return rc;
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int rc;

    if (set_active_port(port+1) != 0) {
        AIM_LOG_ERROR("Unable to set active port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    rc = onlp_i2c_writeb(SFP_BUS, devaddr, addr, value, ONLP_I2C_F_FORCE);

    set_active_port(0);

    return rc;
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int rc;

    if (set_active_port(port+1) != 0) {
        AIM_LOG_ERROR("Unable to set active port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    rc= onlp_i2c_readw(SFP_BUS, devaddr, addr, ONLP_I2C_F_FORCE);

    set_active_port(0);

    return rc;
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int rc;

    if (set_active_port(port+1) != 0) {
        AIM_LOG_ERROR("Unable to set active port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    rc = onlp_i2c_writew(SFP_BUS, devaddr, addr, value, ONLP_I2C_F_FORCE);

    set_active_port(0);

    return rc;

}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv;


    if (port < 0 || port >= 48) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    /* Set SFP active port */
    if (set_active_port(port+1) != 0) {
        AIM_LOG_ERROR("Unable to set active port to port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                if (deviceNodeWriteInt(SFP_HWMON_NODE(sfp_tx_disable), value, 0) != 0) {
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

    set_active_port(0);
    return rv;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int rv;

    if (port < 0 || port >= 48) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    /* Set SFP active port */
    if (set_active_port(port+1) != 0) {
        AIM_LOG_ERROR("Unable to set active port to port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    switch(control)
        {
        case ONLP_SFP_CONTROL_RX_LOS:
            {
                if (as5710_54x_sfp_node_read_int(SFP_HWMON_NODE(sfp_rx_loss), value, 1) != 0) {
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
                if (as5710_54x_sfp_node_read_int(SFP_HWMON_NODE(sfp_tx_fault), value, 1) != 0) {
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
                if (as5710_54x_sfp_node_read_int(SFP_HWMON_NODE(sfp_tx_disable), value, 0) != 0) {
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

    set_active_port(0);
    return rv;
}


int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

