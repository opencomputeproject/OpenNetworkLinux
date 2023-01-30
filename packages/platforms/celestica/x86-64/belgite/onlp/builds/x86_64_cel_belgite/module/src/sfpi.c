/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
 ********************************************************** */
#include <onlp/platformi/sfpi.h>
#include <x86_64_cel_belgite/x86_64_cel_belgite_config.h>
#include "x86_64_cel_belgite_log.h"
#include "platform.h"

// static int QSFP_COUNT = 0;
// static int SFP_COUNT = 8;
// static int SFP_BUS_START = 1;
static char node_path[PREFIX_PATH_LEN] = {0};
char command[256];
char buf[256];
FILE *fp;

static int cel_belgite_qsfp_sfp_node_read_int(char *path, int *value, int data_len)
{
    int ret = 0;
    char buf[8];
    *value = 0;

    ret = read_device_node_string(path, buf, sizeof(buf), data_len);
    if (ret == 0) {
        int is_not_present = atoi(buf);
        if (!is_not_present)
        {
            *value = !is_not_present;
        }
    }
    return ret;
}

static int cel_belgite_sfp_qsfp_get_port_path(int port, char *node_path)
{
    if (port <= QSFP_COUNT + SFP_COUNT)
    {
        if (port > SFP_COUNT)
        {
            sprintf(node_path, "%s/qsfp%d_modprs", PLATFORM_PATH, port - SFP_COUNT);
        }
        else
        {
            sprintf(node_path, "%s/sfp%d_modabs", PLATFORM_PATH, port);
        }
    }
    else
    {
        AIM_LOG_ERROR("Number of port config is mismatch port(%d)\r\n", port);
        return -1;
    }
    return 0;
}

static int cel_belgite_sfp_qsfp_get_eeprom_path(int port, char *node_path)
{
    if (port <= QSFP_COUNT + SFP_COUNT)
    {
        sprintf(node_path, "%s/%d-0050/eeprom", I2C_DEVICE_PATH, port + SFP_BUS_START);
    }
    else
    {
        AIM_LOG_ERROR("Number of port config is mismatch port(%d)\r\n", port);
        return -1;
    }
    return 0;
}

static uint64_t cel_belgite_sfp_qsfp_get_all_ports_present(void)
{
    int i, ret;
    uint64_t present = 0;

    for (i = 0; i < (QSFP_COUNT + SFP_COUNT); i++)
    {
        cel_belgite_sfp_qsfp_get_port_path(i + 1, node_path);
        if (cel_belgite_qsfp_sfp_node_read_int(node_path, &ret, 0) != 0)
        {
            ret = 0;
        }
        present |= ((uint64_t)ret << i);
    }

    return present;
}

int onlp_sfpi_init(void)
{
    return ONLP_STATUS_OK;
}

int onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t *bmap)
{
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for (p = 0; p < (QSFP_COUNT + SFP_COUNT); p++)
    {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

/*
* Return 1 if present.
* Return 0 if not present.
* Return < 0 if error.
*/
int onlp_sfpi_is_present(int port)
{
    int present;

    cel_belgite_sfp_qsfp_get_port_path(port + 1, node_path);
    if (cel_belgite_qsfp_sfp_node_read_int(node_path, &present, 0) != 0)
    {
        if (port < QSFP_COUNT)
        {
            AIM_LOG_ERROR("Unable to read present status from qsfp port(%d)\r\n", port);
        }
        else
        {
            AIM_LOG_ERROR("Unable to read present status from sfp port(%d)\r\n", port - QSFP_COUNT);
        }

        return ONLP_STATUS_E_INTERNAL;
    }
    return present;
}

int onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t *dst)
{
    int i = 0;
    uint64_t presence_all = 0;

    presence_all = cel_belgite_sfp_qsfp_get_all_ports_present();

    /* Populate bitmap */
    for (i = 0; presence_all; i++)
    {
        AIM_BITMAP_MOD(dst, i, (presence_all & 1));
        presence_all >>= 1;
    }
    return ONLP_STATUS_OK;
}

/*
 * This function reads the SFPs idrom and returns in
 * in the data buffer provided.
 */
int onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    cel_belgite_sfp_qsfp_get_eeprom_path(port + 1, node_path);

    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    memset(data, 0, 256);
    if (read_device_node_binary(node_path, (char*)data, 256, 256) != 0) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t *dst)
{
    return ONLP_STATUS_OK;
}

/*
 * De-initialize the SFPI subsystem.
 */
int onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
