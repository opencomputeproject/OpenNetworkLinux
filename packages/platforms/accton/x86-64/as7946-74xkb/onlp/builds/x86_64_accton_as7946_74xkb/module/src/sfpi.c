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

#define EEPROM_I2C_ADDR     0x50
#define EEPROM_START_OFFSET 0x0
#define NUM_OF_SFP_PORT     74

#define VALIDATE_SFP(_port) \
    do { \
        if (_port < 10 || _port > 73) \
            return ONLP_STATUS_E_UNSUPPORTED; \
    } while(0)

#define VALIDATE_QSFP(_port) \
    do { \
        if (_port < 0 || _port > 9) \
            return ONLP_STATUS_E_UNSUPPORTED; \
    } while(0)

static const int port_bus_index[NUM_OF_SFP_PORT] = {
    33,  34,  35,  36,  37,  38,  39,  40,  41,  42,
    43,  44,  45,  46,  47,  48,  49,  50,  51,  52,
    53,  54,  55,  56,  57,  58,  59,  60,  61,  62,
    63,  64,  65,  66,  67,  68,  69,  70,  71,  72,
    73,  74,  75,  76,  77,  78,  79,  80,  81,  82,
    83,  84,  85,  86,  87,  88,  89,  90,  91,  92,
    93,  94,  95,  96,  97,  98,  99, 100, 101, 102,
    103, 104, 105, 106
};

#define PORT_BUS_INDEX(port) (port_bus_index[port])
#define PORT_FORMAT "/sys/bus/i2c/devices/%d-0050/%s"
#define PORT_EEPROM_FORMAT "/sys/bus/i2c/devices/%d-0050/eeprom"
#define MODULE_RESET_MAIN_BOARD_CPLD1_FORMAT "/sys/bus/i2c/devices/12-0061/module_reset_%d"
#define MODULE_RESET_MAIN_BOARD_CPLD2_FORMAT "/sys/bus/i2c/devices/13-0062/module_reset_%d"
#define MODULE_RESET_MAIN_BOARD_CPLD3_FORMAT "/sys/bus/i2c/devices/16-0063/module_reset_%d"
#define MODULE_LPMODE_MAIN_BOARD_CPLD1_FORMAT "/sys/bus/i2c/devices/12-0061/module_lpmode_%d"
#define MODULE_PRESENT_MAIN_BOARD_CPLD1_FORMAT "/sys/bus/i2c/devices/12-0061/module_present_%d"
#define MODULE_PRESENT_MAIN_BOARD_CPLD2_FORMAT "/sys/bus/i2c/devices/13-0062/module_present_%d"
#define MODULE_PRESENT_MAIN_BOARD_CPLD3_FORMAT "/sys/bus/i2c/devices/16-0063/module_present_%d"
#define MODULE_RXLOS_MAIN_BOARD_CPLD1_FORMAT "/sys/bus/i2c/devices/12-0061/module_rx_los_%d"
#define MODULE_RXLOS_MAIN_BOARD_CPLD2_FORMAT "/sys/bus/i2c/devices/13-0062/module_rx_los_%d"
#define MODULE_RXLOS_MAIN_BOARD_CPLD3_FORMAT "/sys/bus/i2c/devices/16-0063/module_rx_los_%d"
#define MODULE_TXDISABLE_MAIN_BOARD_CPLD1_FORMAT "/sys/bus/i2c/devices/12-0061/module_tx_disable_%d"
#define MODULE_TXDISABLE_MAIN_BOARD_CPLD2_FORMAT "/sys/bus/i2c/devices/13-0062/module_tx_disable_%d"
#define MODULE_TXDISABLE_MAIN_BOARD_CPLD3_FORMAT "/sys/bus/i2c/devices/16-0063/module_tx_disable_%d"

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
     * Ports {0, 73}
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
    char *path = NULL;

    switch (port) {
    case 0 ... 24:
        path = MODULE_PRESENT_MAIN_BOARD_CPLD1_FORMAT;
        break;
    case 25 ... 49:
        path = MODULE_PRESENT_MAIN_BOARD_CPLD2_FORMAT;
        break;
    case 50 ... 73:
        path = MODULE_PRESENT_MAIN_BOARD_CPLD3_FORMAT;
        break;
    default:
        return ONLP_STATUS_E_INVALID;
    }

    if (onlp_file_read_int(&present, path, (port+1)) < 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int i=0, val=0;
    char *path = NULL;
    /* Populate bitmap */
    for (i = 0; i < NUM_OF_SFP_PORT; i++) {
        val = 0;
        switch (i) {
        case 0 ... 24:
            path = MODULE_RXLOS_MAIN_BOARD_CPLD1_FORMAT;
            break;
        case 25 ... 49:
            path = MODULE_RXLOS_MAIN_BOARD_CPLD2_FORMAT;
            break;
        case 50 ... 73:
            path = MODULE_RXLOS_MAIN_BOARD_CPLD3_FORMAT;
            break;
        default:
            break;
        }
        if ((i >= 10) && (i <= 73)) {
            if (onlp_file_read_int(&val, path, i+1) < 0) {
                AIM_LOG_ERROR("Unable to read rx_loss status from port(%d)\r\n", i);
            }

            if (val)
                AIM_BITMAP_MOD(dst, i, 1);
            else
                AIM_BITMAP_MOD(dst, i, 0);
        }
        else {
            AIM_BITMAP_MOD(dst, i, 0);
        }
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
    if (port < 0 || port >= NUM_OF_SFP_PORT) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (onlp_file_read(data, 256, &size, PORT_FORMAT, PORT_BUS_INDEX(port), "eeprom") != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (size != 256) {
        AIM_LOG_ERROR("Invalid file size(%d)\r\n", size);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    FILE* fp;
    char file[64] = {0};

    sprintf(file, PORT_EEPROM_FORMAT, PORT_BUS_INDEX(port));
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
    int bus = PORT_BUS_INDEX(port);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int bus = PORT_BUS_INDEX(port);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = PORT_BUS_INDEX(port);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int bus = PORT_BUS_INDEX(port);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv = ONLP_STATUS_OK;
    char *path = NULL;
    switch(control) {
    case ONLP_SFP_CONTROL_TX_DISABLE:
    {
        VALIDATE_SFP(port);

        switch (port) {
        case 0 ... 24:
            path = MODULE_TXDISABLE_MAIN_BOARD_CPLD1_FORMAT;
            break;
        case 25 ... 49:
            path = MODULE_TXDISABLE_MAIN_BOARD_CPLD2_FORMAT;
            break;
        case 50 ... 73:
            path = MODULE_TXDISABLE_MAIN_BOARD_CPLD3_FORMAT;
            break;
        default:
            break;
        }
        if (onlp_file_write_int(value, path, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to set tx_disable status to port(%d)\r\n", port);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        break;
    }
    case ONLP_SFP_CONTROL_RESET: {
        VALIDATE_QSFP(port);

        switch (port) {
        case 0 ... 24:
            path = MODULE_RESET_MAIN_BOARD_CPLD1_FORMAT;
            break;
        case 25 ... 49:
            path = MODULE_RESET_MAIN_BOARD_CPLD1_FORMAT;
            break;
        case 50 ... 73:
            path = MODULE_RESET_MAIN_BOARD_CPLD1_FORMAT;
            break;
        default:
            break;
        }
        if (onlp_file_write_int(value, path, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to write reset status to port(%d)\r\n", port);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        break;
    }
    case ONLP_SFP_CONTROL_LP_MODE:
    {
        VALIDATE_QSFP(port);

        switch (port) {
        case 0 ... 9:
            path = MODULE_LPMODE_MAIN_BOARD_CPLD1_FORMAT;
            break;
        default:
            break;
        }
        if (onlp_file_write_int(value, path, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to write LP mode status to port(%d)\r\n", port);
            rv = ONLP_STATUS_E_INTERNAL;
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
    int rv = ONLP_STATUS_OK;
    char *path = NULL;
    switch(control) {
    case ONLP_SFP_CONTROL_RX_LOS:
    {
        VALIDATE_SFP(port);

        switch (port) {
        case 0 ... 24:
            path = MODULE_RXLOS_MAIN_BOARD_CPLD1_FORMAT;
            break;
        case 25 ... 49:
            path = MODULE_RXLOS_MAIN_BOARD_CPLD2_FORMAT;
            break;
        case 50 ... 73:
            path = MODULE_RXLOS_MAIN_BOARD_CPLD3_FORMAT;
            break;
        default:
            break;
        }
        if (onlp_file_read_int(value, path, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read rx_loss status from port(%d)\r\n", port);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        break;
    }
    case ONLP_SFP_CONTROL_TX_FAULT:
        rv = ONLP_STATUS_E_UNSUPPORTED;
        break;
    case ONLP_SFP_CONTROL_TX_DISABLE:
    {
        VALIDATE_SFP(port);

        switch (port) {
        case 0 ... 24:
            path = MODULE_TXDISABLE_MAIN_BOARD_CPLD1_FORMAT;
            break;
        case 25 ... 49:
            path = MODULE_TXDISABLE_MAIN_BOARD_CPLD2_FORMAT;
            break;
        case 50 ... 73:
            path = MODULE_TXDISABLE_MAIN_BOARD_CPLD3_FORMAT;
            break;
        default:
            break;
        }
        if (onlp_file_read_int(value, path, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read tx_disabled status from port(%d)\r\n", port);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        break;
    }
    case ONLP_SFP_CONTROL_RESET: {
        VALIDATE_QSFP(port);

        switch (port) {
        case 0 ... 24:
            path = MODULE_RESET_MAIN_BOARD_CPLD1_FORMAT;
            break;
        case 25 ... 49:
            path = MODULE_RESET_MAIN_BOARD_CPLD2_FORMAT;
            break;
        case 50 ... 73:
            path = MODULE_RESET_MAIN_BOARD_CPLD3_FORMAT;
            break;
        default:
            break;
        }
        if (onlp_file_read_int(value, path, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read reset status from port(%d)\r\n", port);
            rv = ONLP_STATUS_E_INTERNAL;
        }
        break;
    }
    case ONLP_SFP_CONTROL_LP_MODE:
    {
        VALIDATE_QSFP(port);

        switch (port) {
        case 0 ... 9:
            path = MODULE_LPMODE_MAIN_BOARD_CPLD1_FORMAT;
            break;
        default:
            break;
        }

        if (onlp_file_read_int(value, path, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read LP mode status from port(%d)\r\n", port);
            rv = ONLP_STATUS_E_INTERNAL;
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
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
