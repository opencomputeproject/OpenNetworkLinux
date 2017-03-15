/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
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
#include "platform_lib.h"

#include <arm_accton_as4610/arm_accton_as4610_config.h>
#include "arm_accton_as4610_log.h"

#define MAX_SFP_PATH 64
static char sfp_node_path[MAX_SFP_PATH] = {0};
#define FRONT_PORT_MUX_INDEX(port) (port-46)

static int
sfp_node_read_int(char *node_path, int *value, int data_len)
{
    int ret = 0;
    char buf[8];
    *value = 0;

    ret = deviceNodeReadString(node_path, buf, sizeof(buf), data_len);

    if (ret == 0) {
        *value = atoi(buf);
    }

    return ret;
}

static char*
sfp_get_port_path_addr(int port, int addr, char *node_name)
{
    int front_port_mux_id;

    if(platform_id == PLATFORM_ID_POWERPC_ACCTON_AS4610_30_R0)
        front_port_mux_id = port - 22;
    else /*PLATFORM_ID_POWERPC_ACCTON_AS4610_54_R0*/
        front_port_mux_id = port - 46;

    sprintf(sfp_node_path, "/sys/bus/i2c/devices/%d-00%d/%s",
                           front_port_mux_id, addr, node_name);
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
    int p;
    int start_port, end_port;

    if(platform_id == PLATFORM_ID_POWERPC_ACCTON_AS4610_30_R0)
    {
        start_port = 24;
        end_port   = 30;
    }
    else /*PLATFORM_ID_POWERPC_ACCTON_AS4610_54_R0*/
    {
        start_port = 48;
        end_port   = 54;
    }

    for(p = start_port; p < end_port; p++) {
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
    char* path = sfp_get_port_path(port, "sfp_is_present");

    if (sfp_node_read_int(path, &present, 0) != 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t byte;
    char* path;
    FILE* fp;
    int   port;

    AIM_BITMAP_CLR_ALL(dst);

    if(platform_id == PLATFORM_ID_POWERPC_ACCTON_AS4610_30_R0)
        port = 24;
    else /*PLATFORM_ID_POWERPC_ACCTON_AS4610_54_R0*/
        port = 48;

    path = sfp_get_port_path(port, "sfp_is_present_all");
    fp = fopen(path, "r");

    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the sfp_is_present_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }
    int count = fscanf(fp, "%x", &byte);
    fclose(fp);
    if(count != 1) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields from the sfp_is_present_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t presence_all = byte;
    presence_all <<= port;

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
    uint32_t byte;
    char* path;
    FILE* fp;
    int   port;

    if(platform_id == PLATFORM_ID_POWERPC_ACCTON_AS4610_30_R0)
        port = 24;
    else /*PLATFORM_ID_POWERPC_ACCTON_AS4610_54_R0*/
        port = 48;

    path = sfp_get_port_path(port, "sfp_rx_los_all");
    fp = fopen(path, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the sfp_rx_los_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }
    int count = fscanf(fp, "%x", &byte);
    fclose(fp);
    if(count != 1) {
        AIM_LOG_ERROR("Unable to read all fields from the sfp_rx_los_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t rx_los_all = byte;
    rx_los_all <<= port;

    /* Populate bitmap */
    for(i = 0; rx_los_all; i++) {
        AIM_BITMAP_MOD(dst, i, (rx_los_all & 1));
        rx_los_all >>= 1;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, int dev_addr, uint8_t data[256])
{
    char* path;

    if (dev_addr == SFP_IDPROM_ADDR) {
        path = sfp_get_port_path(port, "sfp_eeprom");
    } else if (dev_addr == SFP_DOM_ADDR) {
        path = sfp_get_port_path_addr(port, 51, "sfp_eeprom");
    } else
        return ONLP_STATUS_E_PARAM;

    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    memset(data, 0, 256);

    if (deviceNodeReadBinary(path, (char*)data, 256, 256) != 0) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
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

    switch(control)
        {
        case ONLP_SFP_CONTROL_RX_LOS:
            {
                path = sfp_get_port_path(port, "sfp_rx_los");

                if (sfp_node_read_int(path, value, 0) != 0) {
                    AIM_LOG_ERROR("Unable to read rx_los status from port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
                }
                break;
            }

        case ONLP_SFP_CONTROL_TX_FAULT:
            {
                path = sfp_get_port_path(port, "sfp_tx_fault");

                if (sfp_node_read_int(path, value, 0) != 0) {
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

                if (sfp_node_read_int(path, value, 0) != 0) {
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
