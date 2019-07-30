/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 MiTAC Computing Technology Corporation.
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
#include <linux/i2c-dev.h>
#include <errno.h>
#include <stdlib.h>
#include <onlplib/i2c.h>
#include "platform_lib.h"

#define MAX_SFP_PATH 64
static char sfp_node_path[MAX_SFP_PATH] = {0};

#define NUM_OF_SFP_PORT 32

#define XCVR_BUS_START_INDEX 9
#define FRONT_PORT_TO_BUS_INDEX(port) (port+XCVR_BUS_START_INDEX)

/* This is based on CPLD spec */
#define PRESENT_EN 0
#define MASTER_CPLD_START_PORT 1
#define SLAVE_CPLD_START_PORT 17

static int
ly1200_b32h0_c3_xcvr_open(uint8_t bus,
                     int *i2c_fd)
{
    int err = 0;
    char dev_name[20];

    sprintf(dev_name, "/dev/i2c-%u", bus);
    if ((*i2c_fd = open(dev_name, O_RDWR)) < 0) {
        err = errno;
        return -err;
    }

    return err;
}

static int
ly1200_b32h0_c3_xcvr_read(uint8_t bus,
                     uint8_t addr,
                     uint16_t offset,
                     uint8_t *val)
{
    int err = 0, value;
    int i2c_fd = 0;

    if ((err = ly1200_b32h0_c3_xcvr_open(bus, &i2c_fd)) < 0) {goto quit;}
    if ((ioctl(i2c_fd, I2C_SLAVE_FORCE, addr)) < 0) {
        err = errno;
        return -err;
    }
    value = i2c_smbus_read_byte_data(i2c_fd, offset);
    if (value < 0) {
        err = errno;
        close(i2c_fd);
        return -err;
    }
    *val = value;

quit:
    close(i2c_fd);
    return err;
}

static int
ly1200_b32h0_c3_sfp_node_read_file(char *filename,
                              uint32_t *value,
                              uint32_t mask)
{
    char buf[4];
    int fd, ret_code = 0, buf_size = sizeof(buf);
    ssize_t count;

    if ((fd = open(filename, O_RDONLY)) < 0) {
        ret_code = errno;
        return -ret_code;
    }

    if ((count = read(fd, buf, buf_size)) != (-1)) {
        if (PRESENT_EN == 0)
            *value = mask & ~strtol(buf, NULL, 16);
        else
            *value = mask & strtol(buf, NULL, 16);
    }
    else {
        ret_code = (count >= 0) ? EIO : errno;
        close(fd);
        return -ret_code;
    }

    close(fd);
    return ret_code;
}

static char*
ly1200_b32h0_c3_sfp_get_port_path(int port, char *node_name)
{
    if (port == -1) {
        sprintf(sfp_node_path, "/sys/bus/i2c/devices/%s",
                               node_name);
    } else if (port >= MASTER_CPLD_START_PORT && port < SLAVE_CPLD_START_PORT) {
        sprintf(sfp_node_path, "/sys/bus/i2c/devices/1-0032/port%d/port%d_%s",
                               port, port,
                               node_name);
    } else if (port >= SLAVE_CPLD_START_PORT && port <= NUM_OF_SFP_PORT) {
        sprintf(sfp_node_path, "/sys/bus/i2c/devices/1-0033/port%d/port%d_%s",
                               port, port,
                               node_name);
    }

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
    /*
     * Ports {1, 32}
     */
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = 1; p <= NUM_OF_SFP_PORT; p++) {
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
    uint32_t value;
    char* path = ly1200_b32h0_c3_sfp_get_port_path(port, "present");

    if (ly1200_b32h0_c3_sfp_node_read_file(path, &value, 0x1) != 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    present = value;

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t bytes[4], value;
    char* path;

    path = ly1200_b32h0_c3_sfp_get_port_path(-1, "1-0032/port_1_8_present");
    if (ly1200_b32h0_c3_sfp_node_read_file(path, &value, 0xFF) != 0) {
        AIM_LOG_ERROR("Unable to read present status from port 1 to 8\r\n");
        return ONLP_STATUS_E_INTERNAL;
    }
    bytes[0] = value;

    path = ly1200_b32h0_c3_sfp_get_port_path(-1, "1-0032/port_9_16_present");
    if (ly1200_b32h0_c3_sfp_node_read_file(path, &value, 0xFF) != 0) {
        AIM_LOG_ERROR("Unable to read present status from port 8 to 16\r\n");
        return ONLP_STATUS_E_INTERNAL;
    }
    bytes[1] = value;

    path = ly1200_b32h0_c3_sfp_get_port_path(-1, "1-0033/port_17_24_present");
    if (ly1200_b32h0_c3_sfp_node_read_file(path, &value, 0xFF) != 0) {
        AIM_LOG_ERROR("Unable to read present status from port 17 to 24\r\n");
        return ONLP_STATUS_E_INTERNAL;
    }
    bytes[2] = value;

    path = ly1200_b32h0_c3_sfp_get_port_path(-1, "1-0033/port_25_32_present");
    if (ly1200_b32h0_c3_sfp_node_read_file(path, &value, 0xFF) != 0) {
        AIM_LOG_ERROR("Unable to read present status from port 25 to 32\r\n");
        return ONLP_STATUS_E_INTERNAL;
    }
    bytes[3] = value;

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint32_t presence_all = 0 ;
    for(i = AIM_ARRAYSIZE(bytes)-1; i >= 0; i--) {
        presence_all <<= 8;
        presence_all |= bytes[i];
    }

    /* Populate bitmap */
    for(i = 1; presence_all; i++) {
        AIM_BITMAP_MOD(dst, i, (presence_all & 1));
        presence_all >>= 1;
    }

    return ONLP_STATUS_OK;
}

int
//onlp_sfpi_eeprom_read(int port, int dev_addr, uint8_t data[256])
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    uint16_t offset = 0;
    uint8_t val = 0, addr = 0x50;//dev_addr;
    uint8_t bus = FRONT_PORT_TO_BUS_INDEX(port);
    if (data == NULL) {return ONLP_STATUS_E_INTERNAL;}

    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    memset(data, 0, 256);

    for(offset = 0; offset < 256; offset++) {
        if (ly1200_b32h0_c3_xcvr_read( bus, addr, offset, &val) < 0) {
            AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
        data[offset] = val & 0xFF;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
