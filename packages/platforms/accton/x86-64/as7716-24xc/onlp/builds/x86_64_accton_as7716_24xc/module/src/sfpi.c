/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 Accton Technology Corporation.
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
#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include "platform_lib.h"

#define MAX_PORT_PATH 		64
#define NUM_OF_PORT 		32
static const int qsfp_mux_index[] = {
29, 30, 31, 32, 34, 33, 36, 35,
25, 26, 27, 28, 37, 38, 39, 40,
42, 41, 44, 43, 46, 45, 48, 47, 
50, 49, 52, 51, 54, 53, 56, 55
};

#define QSFP_BUS_INDEX(port) (qsfp_mux_index[port])
#define QSFP_PORT_FORMAT	 "/sys/bus/i2c/devices/%d-0050/%s"

#define BIT_INDEX(i)			(1ULL << (i))

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
    AIM_BITMAP_CLR_ALL(bmap);

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
    int present = 0;
	port_type_t type = get_port_type(port);

	//printf("%s(%d):Port Type = (%d)\r\n", __FUNCTION__, __LINE__, type);
	if (port >= 24 && type != PORT_TYPE_Q28) {
		return 0;
	}

    if (onlp_file_read_int(&present, QSFP_PORT_FORMAT, QSFP_BUS_INDEX(port), "sfp_is_present") < 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t bytes[4];
    char  path[MAX_PORT_PATH] = {0};
    FILE* fp;

    sprintf(path, QSFP_PORT_FORMAT, QSFP_BUS_INDEX(0), "sfp_is_present_all");
    fp = fopen(path, "r");

    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the sfp_is_present_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }
    int count = fscanf(fp, "%x %x %x %x", bytes+0, bytes+1, bytes+2, bytes+3);
    fclose(fp);
    if(count != AIM_ARRAYSIZE(bytes)) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields from the sfp_is_present_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint32_t presence_all = 0 ;
    for(i = AIM_ARRAYSIZE(bytes)-1; i >= 0; i--) {
        presence_all <<= 8;
        presence_all |= bytes[i];
    }

	for (i = 0; i < NUM_OF_PORT; i++) {
		port_type_t type = get_port_type(i);

		if (i >= 24 && type != PORT_TYPE_Q28) {
			presence_all &= ~BIT_INDEX(i);
		}
	}

    /* Populate bitmap */
    for(i = 0; presence_all; i++) {
        AIM_BITMAP_MOD(dst, i, (presence_all & 1));
        presence_all >>= 1;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
	int ret = ONLP_STATUS_OK;
	port_type_t port_type = get_port_type(port);

	if (PORT_TYPE_UNKNOWN == port_type) {
		return ONLP_STATUS_E_INTERNAL;
	}

	if (PORT_TYPE_Q28 == port_type) {
		int size = 0;

	    if(onlp_file_read(data, 256, &size, QSFP_PORT_FORMAT, QSFP_BUS_INDEX(port), "sfp_eeprom") == ONLP_STATUS_OK) {
			ret = (size == 256) ? ONLP_STATUS_OK : ONLP_STATUS_E_INTERNAL;
	    }
		else {
			ret = ONLP_STATUS_E_INTERNAL;
		}
	}
	else {
		return cfp_eeprom_read(port, data);
	}

	return ret;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = QSFP_BUS_INDEX(port);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int bus = QSFP_BUS_INDEX(port);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = QSFP_BUS_INDEX(port);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int bus = QSFP_BUS_INDEX(port);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

