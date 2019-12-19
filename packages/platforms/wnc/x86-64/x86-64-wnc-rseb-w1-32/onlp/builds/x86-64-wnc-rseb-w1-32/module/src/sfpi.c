/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2019 WNC Computing Technology Corporation.
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

#define NUM_OF_SFP_PORT 32
#define MODULE_PRESENT_FORMAT        "/sys/class/gpio/gpio%d/value"
#define MODULE_LPMODE_FORMAT         "/sys/class/gpio/gpio%d/value"
#define MODULE_RESET_FORMAT          "/sys/class/gpio/gpio%d/value"
#define PORT_EEPROM_FORMAT           "/sys/bus/i2c/devices/%d-0050/eeprom"
#define PORT_GPIO_PRESENT_OFFSET  0
#define PORT_GPIO_LPMODE_OFFSET   1
#define PORT_GPIO_RESET_OFFSET    2

int port_mapping_i2c_busl_table[NUM_OF_SFP_PORT] =
{
    24, 25, 26, 27, 28, 29,   /* Port 1 ~ 6   */
    32, 33, 34, 35, 36, 37,   /* Port 7 ~ 12  */
    40, 41, 42, 43, 44, 45,   /* Port 13 ~ 18 */
    48, 49, 50, 51, 52, 53,   /* Port 19 ~ 24 */
    56, 57, 58, 59, 60, 61,   /* Port 25 ~ 30 */
    64, 65                    /* Port 31, 32  */
};

int port_mapping_gpio_base_table[NUM_OF_SFP_PORT] =
{
    489, 493, 497, 501, 505, 509,   /* Port 1 ~ 6   */
    465, 469, 473, 477, 481, 485,   /* Port 7 ~ 12  */
    441, 445, 449, 453, 457, 461,   /* Port 13 ~ 18 */
    417, 421, 425, 429, 433, 437,   /* Port 19 ~ 24 */
    393, 397, 401, 405, 409, 413,   /* Port 25 ~ 30 */
    369, 373                        /* Port 31, 32  */
};

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
     * Ports {0, 31}
     */
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for (p = 0; p < NUM_OF_SFP_PORT; p++) {
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

    if (onlp_file_read_int(&present, MODULE_PRESENT_FORMAT, 
                            port_mapping_gpio_base_table[port]+PORT_GPIO_PRESENT_OFFSET) < 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* If module is present, GPIO is 0. */
    if (present == 0)
        return 1;
    else if (present == 1)
        return 0;
    else
        return -1;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int port;
    int byte_offset=0, bit_shift=0;
    int present;
    uint32_t bytes[4]={0};
    int i = 0;
    uint32_t presence_all = 0;

    for (port=0; port < NUM_OF_SFP_PORT; port++)
    {
        byte_offset = port / 8;
        bit_shift = port % 8;
        if (onlp_file_read_int(&present, MODULE_PRESENT_FORMAT, 
                                port_mapping_gpio_base_table[port]+PORT_GPIO_PRESENT_OFFSET) < 0) {
            AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }

        if (present == 0)
            bytes[byte_offset] = bytes[byte_offset] | (1 << bit_shift);
    }

    /* Convert to 64 bit integer in port order */
    for (i = AIM_ARRAYSIZE(bytes)-1; i >= 0; i--) {
        presence_all <<= 8;
        presence_all |= bytes[i];
    }

    /* Populate bitmap */
    for (i = 0; presence_all; i++) {
        AIM_BITMAP_MOD(dst, i, (presence_all & 1));
        presence_all >>= 1;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    int size = 0;
    memset(data, 0, 256);

    if (port < 0 || port >= NUM_OF_SFP_PORT) {
        AIM_LOG_ERROR("Illegal port (%d) to read eeprom \r\n", port);
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if (onlp_file_read(data, 256, &size, PORT_EEPROM_FORMAT, port_mapping_i2c_busl_table[port]) != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (size != 256) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d), size is different!\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    FILE* fp;
    char file[64] = {0};

    if (port < 0 || port >= NUM_OF_SFP_PORT) {
        AIM_LOG_ERROR("Illegal port (%d) to read DOM of eeprom \r\n", port);
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    sprintf(file, PORT_EEPROM_FORMAT, port_mapping_i2c_busl_table[port]);
    fp = fopen(file, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the eeprom device file of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (fseek(fp, 256, SEEK_CUR) != 0) {
        fclose(fp);
        AIM_LOG_ERROR("Unable to set the file position indicator of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    int ret = fread(data, 1, 256, fp);
    fclose(fp);
    if (ret != 256) {
        AIM_LOG_ERROR("Unable to read the module_eeprom device file of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    if (port < 0 || port >= NUM_OF_SFP_PORT) {
        AIM_LOG_ERROR("Illegal port (%d) to read byte \r\n", port);
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    int bus = port_mapping_i2c_busl_table[port];
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    if (port < 0 || port >= NUM_OF_SFP_PORT) {
        AIM_LOG_ERROR("Illegal port (%d) to write byte \r\n", port);
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    int bus = port_mapping_i2c_busl_table[port];
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    if (port < 0 || port >= NUM_OF_SFP_PORT) {
        AIM_LOG_ERROR("Illegal port (%d) to read word \r\n", port);
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    int bus = port_mapping_i2c_busl_table[port];
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    if (port < 0 || port >= NUM_OF_SFP_PORT) {
        AIM_LOG_ERROR("Illegal port (%d) to write word \r\n", port);
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    int bus = port_mapping_i2c_busl_table[port];
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv;

    if (port < 0 || port >= NUM_OF_SFP_PORT) {
        AIM_LOG_ERROR("Illegal port (%d) to control set \r\n", port);
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch(control)
    {
        case ONLP_SFP_CONTROL_LP_MODE:
            if (onlp_file_write_int(value, MODULE_LPMODE_FORMAT, 
                                    port_mapping_gpio_base_table[port]+PORT_GPIO_LPMODE_OFFSET) < 0) {
                AIM_LOG_ERROR("Unable to set lp_mode (value:%d) to port(%d)\r\n", value, port);
                rv = ONLP_STATUS_E_INTERNAL;
            } else {
                rv = ONLP_STATUS_OK;
            }
            break;
        case ONLP_SFP_CONTROL_RESET:
            if (onlp_file_write_int(value, MODULE_RESET_FORMAT, 
                                    port_mapping_gpio_base_table[port]+PORT_GPIO_RESET_OFFSET) < 0) {
                AIM_LOG_ERROR("Unable to set reset (value:%d) to port(%d)\r\n", value, port);
                rv = ONLP_STATUS_E_INTERNAL;
            } else {
                rv = ONLP_STATUS_OK;
            }
            break;
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

    if (port < 0 || port >= NUM_OF_SFP_PORT) {
        AIM_LOG_ERROR("Illegal port (%d) to control get \r\n", port);
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch(control)
    {
        case ONLP_SFP_CONTROL_LP_MODE:
            if (onlp_file_read_int(value, MODULE_LPMODE_FORMAT, 
                                   port_mapping_gpio_base_table[port]+PORT_GPIO_LPMODE_OFFSET) < 0) {
                AIM_LOG_ERROR("Unable to read lp_mode status from port(%d)\r\n", port);
                rv = ONLP_STATUS_E_INTERNAL;
            } else {
                rv = ONLP_STATUS_OK;
            }
            break;
        default:
            if (onlp_file_read_int(value, MODULE_RESET_FORMAT, 
                                   port_mapping_gpio_base_table[port]+PORT_GPIO_RESET_OFFSET) < 0) {
                AIM_LOG_ERROR("Unable to read reset status from port(%d)\r\n", port);
                rv = ONLP_STATUS_E_INTERNAL;
            } else {
                rv = ONLP_STATUS_OK;
            }
            rv = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }

    return rv;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
