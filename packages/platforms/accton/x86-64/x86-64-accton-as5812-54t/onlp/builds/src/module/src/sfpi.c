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
#define MUX_START_INDEX 2

static int front_port_to_cpld_mux_index(int port)
{
    int rport = 0;

    switch (port)
    {
        case 48:
            rport = 2;
            break;
        case 49:
            rport = 4;
            break;
        case 50:
            rport = 1;
            break;
        case 51:
            rport = 3;
            break;
        case 52:
            rport = 5;
            break;
        case 53:
            rport = 0;
            break;
        default:
            break;
    }

    return (rport + MUX_START_INDEX);
}

static int
as5812_54t_sfp_node_read_int(char *node_path, int *value, int data_len)
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
as5812_54t_sfp_get_port_path_addr(int port, int addr, char *node_name)
{
    sprintf(sfp_node_path, "/sys/bus/i2c/devices/%d-00%d/%s",
                           front_port_to_cpld_mux_index(port), addr,
                           node_name);
    return sfp_node_path;
}

static char*
as5812_54t_sfp_get_port_path(int port, char *node_name)
{
    return as5812_54t_sfp_get_port_path_addr(port, 50, node_name);
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
     * Ports {49, 54}
     */
    int p;

    for(p = 48; p < 54; p++) {
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
    char* path = as5812_54t_sfp_get_port_path(port, "sfp_is_present");

    if (as5812_54t_sfp_node_read_int(path, &present, 1) != 0) {
        AIM_LOG_INFO("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t bytes[1];
    char* path;
    FILE* fp;

    path = as5812_54t_sfp_get_port_path(0, "sfp_is_present_all");
    fp = fopen(path, "r");

    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the sfp_is_present_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }
    int count = fscanf(fp, "%x", bytes+0);
    fclose(fp);
    if(count != AIM_ARRAYSIZE(bytes)) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields from the sfp_is_present_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Mask out non-existant QSFP ports */
    bytes[0] &= 0x3F;

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t presence_all = 0 ;
    presence_all |= (uint64_t)bytes[i] << 48;

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

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    char* path = as5812_54t_sfp_get_port_path(port, "sfp_eeprom");

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
    char* path = as5812_54t_sfp_get_port_path_addr(port, 51, "sfp_eeprom");
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

