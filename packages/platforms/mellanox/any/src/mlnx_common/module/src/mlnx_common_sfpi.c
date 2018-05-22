/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlplib/sfp.h>
#include <sys/ioctl.h>
#include "mlnx_common_log.h"
#include "mlnx_common_int.h"

#define MAX_SFP_PATH           64
#define SFP_SYSFS_VALUE_LEN    20
static char sfp_node_path[MAX_SFP_PATH] = {0};

int get_sfp_port_num(void);

static int
mc_sfp_node_read_int(char *node_path, int *value)
{
    int data_len = 0, ret = 0;
    char buf[SFP_SYSFS_VALUE_LEN] = {0};
    *value = -1;
    char sfp_present_status[16];
    char sfp_not_present_status[16];

    if (mc_get_kernel_ver() >= KERNEL_VERSION(4,9,30)) {
        strcpy(sfp_present_status, "1");
        strcpy(sfp_not_present_status, "0");
    } else {
        strcpy(sfp_present_status, "good");
        strcpy(sfp_not_present_status, "not_connected");
    }

    ret = onlp_file_read((uint8_t*)buf, sizeof(buf), &data_len, node_path);

    if (ret == 0) {
        if (!strncmp(buf, sfp_present_status, strlen(sfp_present_status))) {
            *value = 1;
        } else if (!strncmp(buf, sfp_not_present_status, strlen(sfp_not_present_status))) {
            *value = 0;
        }
    }

    return ret;
}

static char*
mc_sfp_get_port_path(int port, char *node_name)
{
    if (node_name)
        sprintf(sfp_node_path, "/bsp/qsfp/qsfp%d%s", port, node_name);
    else
        sprintf(sfp_node_path, "/bsp/qsfp/qsfp%d", port);
    return sfp_node_path;
}

static char*
mc_sfp_convert_i2c_path(int port, int devaddr)
{
    sprintf(sfp_node_path, "/bsp/qsfp/qsfp%d", port);
    return sfp_node_path;
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
    int p = 1;
    mlnx_platform_info_t* platform_info = get_platform_info();

    AIM_BITMAP_CLR_ALL(bmap);

    for (; p <= platform_info->sfp_num; p++) {
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
    int present = -1;
    char* path = mc_sfp_get_port_path(port, "_status");

    if (mc_sfp_node_read_int(path, &present) != 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int ii = 1;
    int rc = 0;
    mlnx_platform_info_t* platform_info = get_platform_info();

    for (;ii <= platform_info->sfp_num; ii++) {
        rc = onlp_sfpi_is_present(ii);
        AIM_BITMAP_MOD(dst, ii, (1 == rc) ? 1 : 0);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    char* path = mc_sfp_get_port_path(port, NULL);

    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    memset(data, 0, 256);

    if (onlplib_sfp_eeprom_read_file(path, data) != 0) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    char* path = mc_sfp_convert_i2c_path(port, devaddr);
    uint8_t data;
    int fd;
    int nrd;

    if (!path)
		return ONLP_STATUS_E_MISSING;

    fd = open(path, O_RDONLY);
    if (fd < 0)
		return ONLP_STATUS_E_MISSING;

    lseek(fd, addr, SEEK_SET);
    nrd = read(fd, &data, 1);
    close(fd);

    if (nrd != 1) {
		AIM_LOG_INTERNAL("Failed to read EEPROM file '%s'", path);
		return ONLP_STATUS_E_INTERNAL;
    }
    return data;
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    char* path = mc_sfp_convert_i2c_path(port, devaddr);
    uint16_t data;
    int fd;
    int nrd;

    if (!path){
		return ONLP_STATUS_E_MISSING;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
		return ONLP_STATUS_E_MISSING;
    }

    lseek(fd, addr, SEEK_SET);
    nrd = read(fd, &data, 2);
    close(fd);

    if (nrd != 2) {
		AIM_LOG_INTERNAL("Failed to read EEPROM file '%s'", path);
		return ONLP_STATUS_E_INTERNAL;
    }
    return data;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
