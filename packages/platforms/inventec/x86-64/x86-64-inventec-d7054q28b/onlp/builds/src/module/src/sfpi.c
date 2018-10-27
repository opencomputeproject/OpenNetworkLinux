/************************************************************
 * sfpi.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 ***********************************************************/
#include <onlp/platformi/sfpi.h>

#include <fcntl.h> /* For O_RDWR && open */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include "platform_lib.h"

static char sfp_node_path[ONLP_NODE_MAX_PATH_LEN] = {0};

#define MUX_START_INDEX	(18)
#define NUM_OF_SFP_PORT	(CHASSIS_SFP_COUNT)
static const int sfp_mux_index[NUM_OF_SFP_PORT] = {
 4,  5,  6,  7,  9,  8, 11, 10,
 0,  1,  2,  3, 12, 13, 14, 15,
16, 17, 18, 19, 28, 29, 30, 31,
20, 21, 22, 23, 24, 25, 26, 27,
36, 37, 38, 39, 41, 40, 43, 42,
28, 29, 30, 31, 42, 45, 46, 47,
48, 49, 50, 51, 52, 53
};

#define FRONT_PORT_TO_MUX_INDEX(port) (sfp_mux_index[port]+MUX_START_INDEX)

static int
sfp_node_read_int(char *node_path, int *value, int data_len)
{
    int ret = 0;
    *value = 0;

    ret = onlp_file_read_int(value, node_path);

    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", node_path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ret;
}

static char*
sfp_get_port_path(int port, char *node_name)
{
    sprintf(sfp_node_path, "/sys/class/swps/port%d/%s", port, node_name);

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
     * Ports {0, 32}
     */
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = 0; p < NUM_OF_SFP_PORT; p++) {
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
    char* path = sfp_get_port_path(port, "present");
    if (sfp_node_read_int(path, &present, 0) != 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    if (present == 0) {
	present = 1;
    }
    else
    if (present == 1) {
	present = 0;
    }
    else {
        AIM_LOG_ERROR("Unvalid present status %d from port(%d)\r\n",present,port);
        return ONLP_STATUS_E_INTERNAL;
    }
    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t presence_all = 0 ;
    int port, ret;

    for (port = 0; port < NUM_OF_SFP_PORT; port++) {
	ret = onlp_sfpi_is_present(port);
	if (ret == 1) {
	    presence_all |= (1<<port);
	}
	else
	if (ret == 0) {
	    presence_all &= ~(1<<port);
	}
	else {
            AIM_LOG_ERROR("Unable to read present status of port(%d).", port);
	}
    }

    /* Populate bitmap */
    for(port = 0; presence_all; port++) {
        AIM_BITMAP_MOD(dst, port, (presence_all & 1));
        presence_all >>= 1;
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    char* path;
    int len = 0;
    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    memset(data, 0, 256);
    path = sfp_get_port_path(port, "eeprom");
    if (onlp_file_read(&data[0], 128, &len, path) < 0) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    path = sfp_get_port_path(port, "uppage");
    if (onlp_file_read(&data[128], 128, &len, path) < 0) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = FRONT_PORT_TO_MUX_INDEX(port);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int bus = FRONT_PORT_TO_MUX_INDEX(port);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = FRONT_PORT_TO_MUX_INDEX(port);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int bus = FRONT_PORT_TO_MUX_INDEX(port);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

