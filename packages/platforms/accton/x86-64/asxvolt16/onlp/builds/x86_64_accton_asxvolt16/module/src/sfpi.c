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
#include <onlp/platformi/sfpi.h>
#include <onlplib/file.h>
#include "platform_lib.h"

#include "x86_64_accton_asxvolt16_log.h"

#define NUM_OF_PORT 	20
#define MAX_SFP_PATH 	64
static char sfp_node_path[MAX_SFP_PATH] = {0};

static const int sfp_bus_index[] = {
47, 48, 37, 38, 35, 36, 33, 34,
39, 40, 41, 42, 43, 44, 45, 46,
49, 50, 51, 52
};

#define FRONT_PORT_BUS_INDEX(port) (sfp_bus_index[port])

static char*
sfp_get_port_path_addr(int port, int addr, char *node_name)
{
    sprintf(sfp_node_path, "/sys/bus/i2c/devices/%d-00%d/%s",
                           FRONT_PORT_BUS_INDEX(port), addr, node_name);
    return sfp_node_path;
}

static char*
sfp_get_port_path(int port, char *node_name)
{
    return sfp_get_port_path_addr(port, 50, node_name);
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
    
    for(p = 0; p < NUM_OF_PORT; p++) {
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
    char *path = sfp_get_port_path(port, "sfp_is_present");

    if (onlp_file_read_int(&present, path) < 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    
    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t bytes[3];
    char* path;
    FILE* fp;

    AIM_BITMAP_CLR_ALL(dst);

    path = sfp_get_port_path(0, "sfp_is_present_all");
    fp = fopen(path, "r");

    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the sfp_is_present_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }
    int count = fscanf(fp, "%x %x %x", bytes+0, bytes+1, bytes+2);
    fclose(fp);
    if(count != AIM_ARRAYSIZE(bytes)) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields from the sfp_is_present_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Mask out non-existant QSFP ports */
    bytes[2] &= 0xF;

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t presence_all = 0 ;
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
	AIM_BITMAP_CLR_ALL(dst);
	return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    char* path = sfp_get_port_path(port, "sfp_eeprom");

    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    memset(data, 0, 256);
    
    if (onlp_file_read_binary(path, (char*)data, 256, 256) != 0) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv;

    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                char* path = sfp_get_port_path(port, "sfp_tx_disable");

                if (onlp_file_write_integer(path, value) != 0) {
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

    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_FAULT:
            {
                path = sfp_get_port_path(port, "sfp_tx_fault");

                if (onlp_file_read_int(value, path) < 0) {
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
                path = sfp_get_port_path(port, "sfp_tx_disable");

                if (onlp_file_read_int(value, path) < 0) {
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

