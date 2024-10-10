/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
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
#include "x86_64_accton_as7535_28xb_int.h"
#include "x86_64_accton_as7535_28xb_log.h"

#define VALIDATE_SFP(_port) \
    do { \
        if (_port < 4 || _port > 27) \
            return ONLP_STATUS_E_UNSUPPORTED; \
    } while(0)

#define VALIDATE_QSFP(_port) \
    do { \
        if (_port < 0 || _port > 3) \
            return ONLP_STATUS_E_UNSUPPORTED; \
    } while(0)

#define PORT_EEPROM_FORMAT         "/sys/bus/i2c/devices/%d-0050/eeprom"
#define MODULE_PRESENT_FORMAT      "/sys/bus/i2c/devices/12-0061/module_present_%d"
#define MODULE_RXLOS_FORMAT        "/sys/bus/i2c/devices/12-0061/module_rx_los_%d"
#define MODULE_TXFAULT_FORMAT      "/sys/bus/i2c/devices/12-0061/module_tx_fault_%d"
#define MODULE_TXDISABLE_FORMAT    "/sys/bus/i2c/devices/12-0061/module_tx_disable_%d"
#define MODULE_RESET_FORMAT        "/sys/bus/i2c/devices/12-0061/module_reset_%d"
#define MODULE_LPMODE_FORMAT       "/sys/bus/i2c/devices/11-0060/module_lpmode_%d"
#define MODULE_PRESENT_ALL_ATTR    "/sys/bus/i2c/devices/12-0061/module_present_all"
#define MODULE_RXLOS_ALL_ATTR      "/sys/bus/i2c/devices/12-0061/module_rx_los_all"

/* Definitions for Soft_Rate_Select*/
#define PORT_EEPROM_DEVADDR 0X51
#define MULTIRATE_OFFSET 0x6E
#define MBIT_IDENTIFIER_ADDR 0x50
#define MBIT_OFFSET 0x0d
#define RATE_SELECT 0x02

/* Macro for Multirate Bit*/
#define SET_RATE_SELECT_BIT(x,RS_BIT_pos)  (x | (1<<RS_BIT_pos))
#define RESET_RATE_SELECT_BIT(x,RS_BIT_pos)  (x & ~(1<<RS_BIT_pos))

#define NUM_OF_SFP_PORT 28
static const int port_bus_index[NUM_OF_SFP_PORT] = {
    23, 21, 24, 22, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48
};

#define PORT_BUS_INDEX(port) (port_bus_index[port])

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

    if (onlp_file_read_int(&present, MODULE_PRESENT_FORMAT, (port+1)) < 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t bytes[4];
    FILE* fp;

    /* Read present status of port 0 ~ 27 */
    int count  = 0;

    fp = fopen(MODULE_PRESENT_ALL_ATTR, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the module_present_all device file of CPLD");
        return ONLP_STATUS_E_INTERNAL;
    }

    count = fscanf(fp, "%x %x %x %x", bytes, bytes+1, bytes+2, bytes+3);
    fclose(fp);

    if(count != ARRAY_SIZE(bytes)) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields the module_present_all device file of CPLD");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Mask out non-existant QSFP ports */
    bytes[3] &= 0xF;


    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t presence_all = 0 ;
    for(i = 3; i >= 0; i--) {
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
    uint32_t bytes[4];
    FILE* fp;

    /* Read rxlos status of port 0 ~ 27 */
    int count  = 0;

    fp = fopen(MODULE_RXLOS_ALL_ATTR, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the module_rx_los_all device file of CPLD");
        return ONLP_STATUS_E_INTERNAL;
    }

    count = fscanf(fp, "%x %x %x %x", bytes, bytes+1, bytes+2, bytes+3);
    fclose(fp);
    if(count != AIM_ARRAYSIZE(bytes)) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields from the module_rx_los_all device file of CPLD");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Convert to 32 bit integer in port order */
    AIM_BITMAP_CLR_ALL(dst);
    int i = 0;
    uint64_t rx_los_all = 0 ;
    for(i = 3; i >= 0; i--) {
        rx_los_all <<= 8;
        rx_los_all |= bytes[i];
    }

    /* Populate bitmap */
    for(i = 0; rx_los_all; i++) {
        AIM_BITMAP_MOD(dst, i, (rx_los_all & 1));
        rx_los_all >>= 1;
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

    if(onlp_file_read(data, 256, &size, PORT_EEPROM_FORMAT, PORT_BUS_INDEX(port)) != ONLP_STATUS_OK) {
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
    int present = 0;
    int mbit_identifier;
    int mbit_value;
    switch(control) {
    case ONLP_SFP_CONTROL_TX_DISABLE: {
        VALIDATE_SFP(port);

        if (onlp_file_write_int(value, MODULE_TXDISABLE_FORMAT, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to set tx_disable status to port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
        else {
            return ONLP_STATUS_OK;
        }
        break;
    }
    case ONLP_SFP_CONTROL_RESET: {
        VALIDATE_QSFP(port);

        if (onlp_file_write_int(value, MODULE_RESET_FORMAT, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to write reset status to port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
        break;
    }
    case ONLP_SFP_CONTROL_LP_MODE: {
        VALIDATE_QSFP(port);

        if (onlp_file_write_int(value, MODULE_LPMODE_FORMAT, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to write lp mode status to port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
        else {
            return ONLP_STATUS_OK;
        }
        break;
    }
    case ONLP_SFP_CONTROL_SOFT_RATE_SELECT: {
        VALIDATE_SFP(port);

        present = onlp_sfpi_is_present(port);
        if (present == 1) {
            mbit_identifier = onlp_sfpi_dev_readb(port, MBIT_IDENTIFIER_ADDR, MBIT_OFFSET);
            if (mbit_identifier == RATE_SELECT) {
                mbit_value = onlp_sfpi_dev_readb(port, PORT_EEPROM_DEVADDR, MULTIRATE_OFFSET);
                if (value == 0x0) {
                    mbit_value = RESET_RATE_SELECT_BIT(mbit_value,3);
                    if (onlp_sfpi_dev_writeb(port, PORT_EEPROM_DEVADDR, MULTIRATE_OFFSET, mbit_value) < 0){
                        AIM_LOG_ERROR("Unable to write multirate status to port(%d)\r\n", port);
                        return ONLP_STATUS_E_INTERNAL;
                    }
                    return ONLP_STATUS_OK;
                }
                else if(value == 0x1) {
                    mbit_value = SET_RATE_SELECT_BIT(mbit_value,3);
                    if (onlp_sfpi_dev_writeb(port, PORT_EEPROM_DEVADDR, MULTIRATE_OFFSET, mbit_value) < 0){
                        AIM_LOG_ERROR("Unable to write multirate status to port(%d)\r\n", port);
                        return ONLP_STATUS_E_INTERNAL;
                    }
                    return ONLP_STATUS_OK;
                }
                return ONLP_STATUS_E_INTERNAL;
            }
            else {
                AIM_LOG_ERROR("Unable to support multirate for the port(%d)\r\n", port);
                return ONLP_STATUS_E_UNSUPPORTED;
            }
        }
        else {
            AIM_LOG_ERROR("Unable to write multirate status to port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    default:
        break;
    }

    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int present = 0;
    int multirate = 0;
    int mbit_identifier;
    switch(control) {
    case ONLP_SFP_CONTROL_RX_LOS: {
        VALIDATE_SFP(port);

        if (onlp_file_read_int(value, MODULE_RXLOS_FORMAT, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read rx_loss status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }

        return ONLP_STATUS_OK;
    }

    case ONLP_SFP_CONTROL_TX_FAULT: {
        VALIDATE_SFP(port);

        if (onlp_file_read_int(value, MODULE_TXFAULT_FORMAT, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read tx_fault status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }

        return ONLP_STATUS_OK;
    }

    case ONLP_SFP_CONTROL_TX_DISABLE: {
        VALIDATE_SFP(port);

        if (onlp_file_read_int(value, MODULE_TXDISABLE_FORMAT, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read tx_disabled status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }

        return ONLP_STATUS_OK;
    }
    case ONLP_SFP_CONTROL_RESET: {
        VALIDATE_QSFP(port);

        if (onlp_file_read_int(value, MODULE_RESET_FORMAT, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read reset status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }

        return ONLP_STATUS_OK;
    }
    case ONLP_SFP_CONTROL_LP_MODE: {
        VALIDATE_QSFP(port);

        if (onlp_file_read_int(value, MODULE_LPMODE_FORMAT, (port+1)) < 0) {
            AIM_LOG_ERROR("Unable to read lp mode status from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }

        return ONLP_STATUS_OK;
    }
    case ONLP_SFP_CONTROL_SOFT_RATE_SELECT: {
        VALIDATE_SFP(port);

        present = onlp_sfpi_is_present(port);
        if(present == 1) {
            mbit_identifier = onlp_sfpi_dev_readb(port, MBIT_IDENTIFIER_ADDR, MBIT_OFFSET);
            if(mbit_identifier == RATE_SELECT) {
                multirate = onlp_sfpi_dev_readb(port, PORT_EEPROM_DEVADDR, MULTIRATE_OFFSET);
                if (multirate < 0) {
                    return ONLP_STATUS_E_INTERNAL;
                }
                else{
                    *value = multirate;
                    return ONLP_STATUS_OK;
                }
            }
            else {
                AIM_LOG_ERROR("Unable to read multirate status from port(%d)\r\n", port);
                return ONLP_STATUS_E_UNSUPPORTED;
            }
        }
        else {
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    default:
        break;
    }

    return ONLP_STATUS_E_UNSUPPORTED;
}


int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
