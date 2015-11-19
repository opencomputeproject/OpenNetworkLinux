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

#include "platform_lib.h"

#define MAX_SFP_PATH 64
static char sfp_node_path[MAX_SFP_PATH] = {0};
#define CPLD_MUX_BUS_START_INDEX 2

static int front_port_to_cpld_mux_index(int port)
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

static int
as5712_54x_sfp_node_read_int(char *node_path, int *value, int data_len)
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

static char*
as5712_54x_sfp_get_port_path_addr(int port, int addr, char *node_name)
{
    sprintf(sfp_node_path, "/sys/bus/i2c/devices/%d-00%d/%s",
                           front_port_to_cpld_mux_index(port), addr,
                           node_name);
    return sfp_node_path;
}

static char*
as5712_54x_sfp_get_port_path(int port, char *node_name)
{
    return as5712_54x_sfp_get_port_path_addr(port, 50, node_name);
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
     * The QSFP port numbering on the powerpc-as5712-54x-r0b platform
     * differs from the numbering on the production
     * powerpc-accton-as5712-54x-r0 platform.
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
 * The CPLD registers on the as5712 use the original R0B QSFP (horizontal)
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
    char* path = as5712_54x_sfp_get_port_path(port, "sfp_is_present");

    if (as5712_54x_sfp_node_read_int(path, &present, 1) != 0) {
        AIM_LOG_INFO("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t bytes[7];
    char* path;
    FILE* fp;

    path = as5712_54x_sfp_get_port_path(0, "sfp_is_present_all");
    fp = fopen(path, "r");

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
    if(count != AIM_ARRAYSIZE(bytes)) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields from the sfp_is_present_all device file.");
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
    uint32_t bytes[7];
    char* path;
    FILE* fp;

    path = as5712_54x_sfp_get_port_path(0, "sfp_rx_los_all");
    fp = fopen(path, "r");

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
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    char* path = as5712_54x_sfp_get_port_path(port, "sfp_eeprom");

    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    memset(data, 0, 256);

    if (deviceNodeReadBinary(path, (char*)data, 256, 256) != 0) {
        AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    char* path = as5712_54x_sfp_get_port_path_addr(port, 51, "sfp_eeprom");
    memset(data, 0, 256);

    if (deviceNodeReadBinary(path, (char*)data, 256, 256) != 0) {
        AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv;

    if (port < 0 || port >= 48) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                char* path = as5712_54x_sfp_get_port_path(port, "sfp_tx_disable");

                if (deviceNodeWriteInt(path, value, 0) != 0) {
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
    char* path = NULL;

    if (port < 0 || port >= 48) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch(control)
        {
        case ONLP_SFP_CONTROL_RX_LOS:
            {
                path = as5712_54x_sfp_get_port_path(port, "sfp_rx_loss");

                if (as5712_54x_sfp_node_read_int(path, value, 1) != 0) {
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
                path = as5712_54x_sfp_get_port_path(port, "sfp_tx_fault");

                if (as5712_54x_sfp_node_read_int(path, value, 1) != 0) {
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
                path = as5712_54x_sfp_get_port_path(port, "sfp_tx_disable");

                if (as5712_54x_sfp_node_read_int(path, value, 0) != 0) {
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

