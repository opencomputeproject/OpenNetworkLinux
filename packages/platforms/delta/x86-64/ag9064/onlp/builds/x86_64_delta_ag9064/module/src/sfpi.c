/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *        Copyright 2017 Delta Networks, Inc.
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
 ***********************************************************/
#include <onlp/platformi/sfpi.h>
#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include "platform_lib.h"


int onlp_sfpi_init(void)
{
    lockinit();
    return ONLP_STATUS_OK;
}

int onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t *bmap)
{
    /* Ports {1, 64} */
    int p;

    AIM_BITMAP_CLR_ALL(bmap);
    for (p = 1; p <= NUM_OF_ALL_PORT; p++)
        AIM_BITMAP_SET(bmap, p);

    return ONLP_STATUS_OK;
}

int onlp_sfpi_port_map(int port, int *rport)
{
    VALIDATE_PORT(port);
    *rport = port;
    return ONLP_STATUS_OK;
}

int onlp_sfpi_is_present(int port)
{
    int present, present_bit = 0x00;
    int port_t = -1;
    int swpld_addr = 0x00;
    int reg_t = 0x00;
    int bit_t = 0x00;

    VALIDATE_PORT(port);

    if (port >= 0 * QSFP_PORT_OFFSET + 1 && port <= 1 * QSFP_PORT_OFFSET) {        /* QSFP Port 1-8 */
        swpld_addr = SWPLD_1_ADDR;
        reg_t = QSFP_1_TO_8_PRESENT_REG;
    } else if (port >= 1 * QSFP_PORT_OFFSET + 1 && port <= 2 * QSFP_PORT_OFFSET) { /* QSFP Port 9-16 */
        swpld_addr = SWPLD_2_ADDR;
        reg_t = QSFP_9_TO_16_PRESENT_REG;
    } else if (port >= 2 * QSFP_PORT_OFFSET + 1 && port <= 3 * QSFP_PORT_OFFSET) { /* QSFP Port 17-24 */
        swpld_addr = SWPLD_4_ADDR;
        reg_t = QSFP_17_TO_24_PRESENT_REG;
    } else if (port >= 3 * QSFP_PORT_OFFSET + 1 && port <= 4 * QSFP_PORT_OFFSET) { /* QSFP Port 25-32 */
        swpld_addr = SWPLD_3_ADDR;
        reg_t = QSFP_25_TO_32_PRESENT_REG;
    } else if (port >= 4 * QSFP_PORT_OFFSET + 1 && port <= 5 * QSFP_PORT_OFFSET) { /* QSFP Port 33-40 */
        swpld_addr = SWPLD_1_ADDR;
        reg_t = QSFP_33_TO_40_PRESENT_REG;
    } else if (port >= 5 * QSFP_PORT_OFFSET + 1 && port <= 6 * QSFP_PORT_OFFSET) { /* QSFP Port 41-48 */
        swpld_addr = SWPLD_2_ADDR;
        reg_t = QSFP_41_TO_48_PRESENT_REG;
    } else if (port >= 6 * QSFP_PORT_OFFSET + 1 && port <= 7 * QSFP_PORT_OFFSET) { /* QSFP Port 49-56 */
        swpld_addr = SWPLD_4_ADDR;
        reg_t = QSFP_49_TO_56_PRESENT_REG;
    } else if (port >= 7 * QSFP_PORT_OFFSET + 1 && port <= 8 * QSFP_PORT_OFFSET) { /* QSFP Port 57-64 */
        swpld_addr = SWPLD_3_ADDR;
        reg_t = QSFP_57_TO_64_PRESENT_REG;
    } else
        present_bit = 1; /* return 1, module is not present */

    if (port >= 0 * QSFP_PORT_OFFSET + 1 && port <= 8 * QSFP_PORT_OFFSET) {
        if (dni_bmc_data_get(BMC_SWPLD_BUS, swpld_addr, reg_t, &present_bit) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
        port_t = port - 1;
        bit_t = 1 << (port_t % 8);
        present_bit = present_bit & bit_t;
        present_bit = present_bit / bit_t;
    }

    /* "0" = The module is preset
     * "1" = The module is NOT present */
    if (present_bit == 0)
        present = 1;
    else if (present_bit == 1)
        present = 0;
    else {
        AIM_LOG_ERROR("Error to present status from port(%d)\r\n", port);
        present = -1;
    }

    return present;
}

int onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int port;

    AIM_BITMAP_CLR_ALL(dst);
    for (port = 0; port <= NUM_OF_ALL_PORT; port++) {
        if (onlp_sfpi_is_present(port) == 1)
            AIM_BITMAP_MOD(dst, port, 1);
        else if (onlp_sfpi_is_present(port) == 0)
            AIM_BITMAP_MOD(dst, port, 0);
        else
            return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}

int onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    int size = 0;

    VALIDATE_PORT(port);

    memset(data, 0, 256);
    if (onlp_file_read(data, 256, &size, PORT_EEPROM_FORMAT, FRONT_PORT_TO_BUS_INDEX(port-1))
        != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (size != 256) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d), size is different!\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int port_t = -1;
    int swpld_addr = 0x00;
    int reg_t = 0x00;
    int bit_t = 0x00;
    int rdata_bit = 0x00;
	int value_t;

    VALIDATE_PORT(port);

    switch (control) {
        case ONLP_SFP_CONTROL_RESET_STATE:
            if (port >= 0 * QSFP_PORT_OFFSET + 1 && port <= 1 * QSFP_PORT_OFFSET) { 	   /* QSFP Port 1-8 */
                swpld_addr = SWPLD_1_ADDR;
                reg_t = QSFP_1_TO_8_RESET_REG;
            } else if (port >= 1 * QSFP_PORT_OFFSET + 1 && port <= 2 * QSFP_PORT_OFFSET) { /* QSFP Port 9-16 */
                swpld_addr = SWPLD_2_ADDR;
                reg_t = QSFP_9_TO_16_RESET_REG;
            } else if (port >= 2 * QSFP_PORT_OFFSET + 1 && port <= 3 * QSFP_PORT_OFFSET) { /* QSFP Port 17-24 */
                swpld_addr = SWPLD_4_ADDR;
                reg_t = QSFP_17_TO_24_RESET_REG;
            } else if (port >= 3 * QSFP_PORT_OFFSET + 1 && port <= 4 * QSFP_PORT_OFFSET) { /* QSFP Port 25-32 */
                swpld_addr = SWPLD_3_ADDR;
                reg_t = QSFP_25_TO_32_RESET_REG;
            } else if (port >= 4 * QSFP_PORT_OFFSET + 1 && port <= 5 * QSFP_PORT_OFFSET) { /* QSFP Port 33-40 */
                swpld_addr = SWPLD_1_ADDR;
                reg_t = QSFP_33_TO_40_RESET_REG;
            } else if (port >= 5 * QSFP_PORT_OFFSET + 1 && port <= 6 * QSFP_PORT_OFFSET) { /* QSFP Port 41-48 */
                swpld_addr = SWPLD_2_ADDR;
                reg_t = QSFP_41_TO_48_RESET_REG;
            } else if (port >= 6 * QSFP_PORT_OFFSET + 1 && port <= 7 * QSFP_PORT_OFFSET) { /* QSFP Port 49-56 */
                swpld_addr = SWPLD_4_ADDR;
                reg_t = QSFP_49_TO_56_RESET_REG;
            } else if (port >= 7 * QSFP_PORT_OFFSET + 1 && port <= 8 * QSFP_PORT_OFFSET) { /* QSFP Port 57-64 */
                swpld_addr = SWPLD_3_ADDR;
                reg_t = QSFP_57_TO_64_RESET_REG;
            } else
                rdata_bit = 1; /* return 1, module not in reset mode */

            if (port >= 0 * QSFP_PORT_OFFSET + 1 && port <= 8 * QSFP_PORT_OFFSET) {
                if (dni_bmc_data_get(BMC_SWPLD_BUS, swpld_addr, reg_t, &rdata_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                port_t = port - 1;
                bit_t = 1 << (port_t % 8);
                rdata_bit = rdata_bit & bit_t;
                rdata_bit = rdata_bit / bit_t;
			}

            /* "0" = The module is in Reset
             * "1" = The module is not in Reset */
            if (rdata_bit == 0)
                *value = 1;
            else if (rdata_bit == 1)
                *value = 0;
            value_t = ONLP_STATUS_OK;
            break;
        case ONLP_SFP_CONTROL_RX_LOS:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
        case ONLP_SFP_CONTROL_TX_DISABLE:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
        case ONLP_SFP_CONTROL_TX_FAULT:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
        case ONLP_SFP_CONTROL_LP_MODE:
            if (port >= 0 * QSFP_PORT_OFFSET + 1 && port <= 1 * QSFP_PORT_OFFSET) { 	   /* QSFP Port 1-8 */
                swpld_addr = SWPLD_1_ADDR;
                reg_t = QSFP_1_TO_8_LPMODE_REG;
            } else if (port >= 1 * QSFP_PORT_OFFSET + 1 && port <= 2 * QSFP_PORT_OFFSET) { /* QSFP Port 9-16 */
                swpld_addr = SWPLD_2_ADDR;
                reg_t = QSFP_9_TO_16_LPMODE_REG;
            } else if (port >= 2 * QSFP_PORT_OFFSET + 1 && port <= 3 * QSFP_PORT_OFFSET) { /* QSFP Port 17-24 */
                swpld_addr = SWPLD_4_ADDR;
                reg_t = QSFP_17_TO_24_LPMODE_REG;
            } else if (port >= 3 * QSFP_PORT_OFFSET + 1 && port <= 4 * QSFP_PORT_OFFSET) { /* QSFP Port 25-32 */
                swpld_addr = SWPLD_3_ADDR;
                reg_t = QSFP_25_TO_32_LPMODE_REG;
            } else if (port >= 4 * QSFP_PORT_OFFSET + 1 && port <= 5 * QSFP_PORT_OFFSET) { /* QSFP Port 33-40 */
                swpld_addr = SWPLD_1_ADDR;
                reg_t = QSFP_33_TO_40_LPMODE_REG;
            } else if (port >= 5 * QSFP_PORT_OFFSET + 1 && port <= 6 * QSFP_PORT_OFFSET) { /* QSFP Port 41-48 */
                swpld_addr = SWPLD_2_ADDR;
                reg_t = QSFP_41_TO_48_LPMODE_REG;
            } else if (port >= 6 * QSFP_PORT_OFFSET + 1 && port <= 7 * QSFP_PORT_OFFSET) { /* QSFP Port 49-56 */
                swpld_addr = SWPLD_4_ADDR;
                reg_t = QSFP_49_TO_56_LPMODE_REG;
            } else if (port >= 7 * QSFP_PORT_OFFSET + 1 && port <= 8 * QSFP_PORT_OFFSET) { /* QSFP Port 57-64 */
                swpld_addr = SWPLD_3_ADDR;
                reg_t = QSFP_57_TO_64_LPMODE_REG;
            } else
                rdata_bit = 0; /* return 0, module is not in LP mode */

            if (port >= 0 * QSFP_PORT_OFFSET + 1 && port <= 8 * QSFP_PORT_OFFSET) {
                if (dni_bmc_data_get(BMC_SWPLD_BUS, swpld_addr, reg_t, &rdata_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                port_t = port - 1;
                bit_t = 1 << (port_t % 8);
                rdata_bit = rdata_bit & bit_t;
                rdata_bit = rdata_bit / bit_t;
            }

            /* "0" = The module is not in LP mode
             * "1" = The module is in LP mode */
            *value = rdata_bit;
            value_t = ONLP_STATUS_OK;
            break;
        default:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }
    return value_t;
}

int onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int port_t = -1;
    int swpld_addr = 0x00;
    int reg_t = 0x00;
    int bit_t = 0x00;
    int data_bit = 0x00;
    int value_t;

    VALIDATE_PORT(port);

    switch (control) {
        case ONLP_SFP_CONTROL_RESET_STATE:
            if (port >= 0 * QSFP_PORT_OFFSET + 1 && port <= 1 * QSFP_PORT_OFFSET) { 	   /* QSFP Port 1-8 */
                swpld_addr = SWPLD_1_ADDR;
                reg_t = QSFP_1_TO_8_RESET_REG;
            } else if (port >= 1 * QSFP_PORT_OFFSET + 1 && port <= 2 * QSFP_PORT_OFFSET) { /* QSFP Port 9-16 */
                swpld_addr = SWPLD_2_ADDR;
                reg_t = QSFP_9_TO_16_RESET_REG;
            } else if (port >= 2 * QSFP_PORT_OFFSET + 1 && port <= 3 * QSFP_PORT_OFFSET) { /* QSFP Port 17-24 */
                swpld_addr = SWPLD_4_ADDR;
                reg_t = QSFP_17_TO_24_RESET_REG;
            } else if (port >= 3 * QSFP_PORT_OFFSET + 1 && port <= 4 * QSFP_PORT_OFFSET) { /* QSFP Port 25-32 */
                swpld_addr = SWPLD_3_ADDR;
                reg_t = QSFP_25_TO_32_RESET_REG;
            } else if (port >= 4 * QSFP_PORT_OFFSET + 1 && port <= 5 * QSFP_PORT_OFFSET) { /* QSFP Port 33-40 */
                swpld_addr = SWPLD_1_ADDR;
                reg_t = QSFP_33_TO_40_RESET_REG;
            } else if (port >= 5 * QSFP_PORT_OFFSET + 1 && port <= 6 * QSFP_PORT_OFFSET) { /* QSFP Port 41-48 */
                swpld_addr = SWPLD_2_ADDR;
                reg_t = QSFP_41_TO_48_RESET_REG;
            } else if (port >= 6 * QSFP_PORT_OFFSET + 1 && port <= 7 * QSFP_PORT_OFFSET) { /* QSFP Port 49-56 */
                swpld_addr = SWPLD_4_ADDR;
                reg_t = QSFP_49_TO_56_RESET_REG;
            } else if (port >= 7 * QSFP_PORT_OFFSET + 1 && port <= 8 * QSFP_PORT_OFFSET) { /* QSFP Port 57-64 */
                swpld_addr = SWPLD_3_ADDR;
                reg_t = QSFP_57_TO_64_RESET_REG;
            }

            if (port >= 0 * QSFP_PORT_OFFSET + 1 && port <= 8 * QSFP_PORT_OFFSET) {
                if (dni_bmc_data_get(BMC_SWPLD_BUS, swpld_addr, reg_t, &data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                /* Indicate the module is in reset mode or not
                 * 0 = Reset
                 * 1 = Normal */
                port_t = port - 1;
                if (value == 0) {
                    bit_t = ~(1 << (port_t % 8));
                    data_bit = data_bit & bit_t;
                }
                else if (value == 1) {
                    bit_t = (1 << (port_t % 8));
                    data_bit = data_bit | bit_t;
                }
                if (dni_bmc_data_set(BMC_SWPLD_BUS, swpld_addr, reg_t, (uint8_t)data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
            } else
                return ONLP_STATUS_E_UNSUPPORTED;

            value_t = ONLP_STATUS_OK;
            break;
        case ONLP_SFP_CONTROL_RX_LOS:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
        case ONLP_SFP_CONTROL_TX_DISABLE:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
        case ONLP_SFP_CONTROL_TX_FAULT:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
        case ONLP_SFP_CONTROL_LP_MODE:
            if (port >= 0 * QSFP_PORT_OFFSET + 1 && port <= 1 * QSFP_PORT_OFFSET) { 	   /* QSFP Port 1-8 */
                swpld_addr = SWPLD_1_ADDR;
                reg_t = QSFP_1_TO_8_LPMODE_REG;
            } else if (port >= 1 * QSFP_PORT_OFFSET + 1 && port <= 2 * QSFP_PORT_OFFSET) { /* QSFP Port 9-16 */
                swpld_addr = SWPLD_2_ADDR;
                reg_t = QSFP_9_TO_16_LPMODE_REG;
            } else if (port >= 2 * QSFP_PORT_OFFSET + 1 && port <= 3 * QSFP_PORT_OFFSET) { /* QSFP Port 17-24 */
                swpld_addr = SWPLD_4_ADDR;
                reg_t = QSFP_17_TO_24_LPMODE_REG;
            } else if (port >= 3 * QSFP_PORT_OFFSET + 1 && port <= 4 * QSFP_PORT_OFFSET) { /* QSFP Port 25-32 */
                swpld_addr = SWPLD_3_ADDR;
                reg_t = QSFP_25_TO_32_LPMODE_REG;
            } else if (port >= 4 * QSFP_PORT_OFFSET + 1 && port <= 5 * QSFP_PORT_OFFSET) { /* QSFP Port 33-40 */
                swpld_addr = SWPLD_1_ADDR;
                reg_t = QSFP_33_TO_40_LPMODE_REG;
            } else if (port >= 5 * QSFP_PORT_OFFSET + 1 && port <= 6 * QSFP_PORT_OFFSET) { /* QSFP Port 41-48 */
                swpld_addr = SWPLD_2_ADDR;
                reg_t = QSFP_41_TO_48_LPMODE_REG;
            } else if (port >= 6 * QSFP_PORT_OFFSET + 1 && port <= 7 * QSFP_PORT_OFFSET) { /* QSFP Port 49-56 */
                swpld_addr = SWPLD_4_ADDR;
                reg_t = QSFP_49_TO_56_LPMODE_REG;
            } else if (port >= 7 * QSFP_PORT_OFFSET + 1 && port <= 8 * QSFP_PORT_OFFSET) { /* QSFP Port 57-64 */
                swpld_addr = SWPLD_3_ADDR;
                reg_t = QSFP_57_TO_64_LPMODE_REG;
            }

            if (port >= 0 * QSFP_PORT_OFFSET + 1 && port <= 8 * QSFP_PORT_OFFSET) {
                if (dni_bmc_data_get(BMC_SWPLD_BUS, swpld_addr, reg_t, &data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                /* Indicate the module is in LP mode or not
                 * 0 = Disable
                 * 1 = Enable */
                port_t = port - 1;
                if (value == 0) {
                    bit_t = ~(1 << (port_t % 8));
                    data_bit = data_bit & bit_t;
                }
                else if (value == 1) {
                    bit_t = (1 << (port_t % 8));
                    data_bit = data_bit | bit_t;
                }
                if (dni_bmc_data_set(BMC_SWPLD_BUS, swpld_addr, reg_t, (uint8_t)data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
            }
            else
                return ONLP_STATUS_E_UNSUPPORTED;

            value_t = ONLP_STATUS_OK;
            break;
        default:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }
    return value_t;
}

int onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv)
{
    switch (control) {
        case ONLP_SFP_CONTROL_RESET_STATE:
            if (port >= 0 * QSFP_PORT_OFFSET + 1 && port <= 8 * QSFP_PORT_OFFSET)
                *rv = 1;
            else
                *rv = 0;
            break;
        case ONLP_SFP_CONTROL_RX_LOS:
            *rv = 0;
            break;
        case ONLP_SFP_CONTROL_TX_DISABLE:
            *rv = 0;
            break;
        case ONLP_SFP_CONTROL_TX_FAULT:
            *rv = 0;
            break;
        case ONLP_SFP_CONTROL_LP_MODE:
            if (port >= 0 * QSFP_PORT_OFFSET + 1 && port <= 8 * QSFP_PORT_OFFSET)
                *rv = 1;
            else
                *rv = 0;
            break;
        default:
            break;
    }
    return ONLP_STATUS_OK;
}

int onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    VALIDATE_PORT(port);
    int bus = FRONT_PORT_TO_BUS_INDEX(port-1);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    VALIDATE_PORT(port);
    int bus = FRONT_PORT_TO_BUS_INDEX(port-1);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    VALIDATE_PORT(port);
    int bus = FRONT_PORT_TO_BUS_INDEX(port-1);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    VALIDATE_PORT(port);
    int bus = FRONT_PORT_TO_BUS_INDEX(port-1);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    return ONLP_STATUS_OK;
}

int onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

int onlp_sfpi_ioctl(int port, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

