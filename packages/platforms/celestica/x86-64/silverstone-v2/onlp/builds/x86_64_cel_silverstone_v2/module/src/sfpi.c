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
#include <x86_64_cel_silverstone_v2/x86_64_cel_silverstone_v2_config.h>
#include "x86_64_cel_silverstone_v2_log.h"
#include "platform_common.h"

static char node_path[PREFIX_PATH_LEN] = {0};
char command[256];
char buf[256];
FILE *fp;
//The user port,bus number and physical port are irregular.This array make user port map to bus number.
int qsfp_bus_map[QSFP_COUNT]={27,28,25,26,31,30,48,29,45,44,47,43,42,46,32,41,34,40,37,33,38,35,22,39,36,23,17,24,19,21,20,18};

static int cel_silverstone_v2_qsfp_sfp_node_read_int(char *path, int *value, int data_len)
{
    int ret = 0;
    char tmp[8];
    *value = 0;

    ret = read_device_node_string(path, tmp, sizeof(tmp), data_len);
    if (ret == 0) {
        int is_not_present = atoi(tmp);
        if (!is_not_present)
        {
            *value = !is_not_present;
        }
    }
    return ret;
}

static char * cel_silverstone_v2_sfp_qsfp_get_port_path(int port, char *node_name)
{
    if (port <= QSFP_COUNT + SFP_COUNT)
    {
        if (port <= QSFP_COUNT)
        {
            sprintf(node_path, "%s/QSFP%d/qsfp_modprs", PLATFORM_QSFP_PATH, port);
        }
        else
        {
            sprintf(node_path, "%s/SFP%d/sfp_modabs", PLATFORM_SFP_PATH, port - QSFP_COUNT);
        }
    }
    else
    {
        AIM_LOG_ERROR("Number of port config is mismatch port(%d)\r\n", port);
        return "";
    }

    return node_path;
}

static char * cel_silverstone_v2_sfp_qsfp_get_eeprom_path(int port, char *node_name)
{
    if (port <= QSFP_COUNT + SFP_COUNT)
    {
        if (port <= QSFP_COUNT)
        {
            sprintf(node_path, "%s%d-0050/eeprom", I2C_DEVICE_PATH, qsfp_bus_map[port-1]); //QSFP 17 - 48
        }
        else
        {
            sprintf(node_path, "%s%d-0050/eeprom", I2C_DEVICE_PATH, port -  QSFP_COUNT + 10); //SFP 11 - 12
        }
    }
    else
    {
        AIM_LOG_ERROR("Number of port config is mismatch port(%d)\r\n", port);
        return "";
    }

    return node_path;
}

static uint64_t cel_silverstone_v2_sfp_qsfp_get_all_ports_present(void)
{
    int i, ret;
    uint64_t present = 0;
    char *path;

    for (i = 0; i < (QSFP_COUNT + SFP_COUNT); i++)
    {
        path = cel_silverstone_v2_sfp_qsfp_get_port_path(i + 1, "present");
        if (cel_silverstone_v2_qsfp_sfp_node_read_int(path, &ret, 0) != 0)
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
    char *path = cel_silverstone_v2_sfp_qsfp_get_port_path(port + 1, "present");
    if (cel_silverstone_v2_qsfp_sfp_node_read_int(path, &present, 0) != 0)
    {
        if (port <= QSFP_COUNT)
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

    presence_all = cel_silverstone_v2_sfp_qsfp_get_all_ports_present();

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
    char *path;

    path = cel_silverstone_v2_sfp_qsfp_get_eeprom_path(port + 1, "eeprom");

    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    memset(data, 0, 256);
    if (read_device_node_binary(path, (char*)data, 256, 256) != 0) {
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
