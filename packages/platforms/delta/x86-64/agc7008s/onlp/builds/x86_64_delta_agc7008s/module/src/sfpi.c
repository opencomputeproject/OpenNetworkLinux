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
    /* Ports {5, 16} */
    int p;

    AIM_BITMAP_CLR_ALL(bmap);
    for (p = 5; p <= NUM_OF_ALL_PORT + 4; p++)
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
    int reg_t = 0x00;
    int bit_t = 0x00;
    int mux_bit = 0x01;

    VALIDATE_PORT(port);

    if (dni_bmc_data_set(BMC_BUS, MUX_ADDR, reg_t, (uint8_t)mux_bit) != ONLP_STATUS_OK)
        return ONLP_STATUS_E_INTERNAL;

    if (port >= 5 && port <= 8) {        /* SFP Port 5-8 */
        reg_t = SFP_PRESENT_REG;
    } else if (port >= 9 && port <= 16) { /* SFP+ Port 9-16 */
        reg_t = SFPP_PRESENT_REG;
    } else
        present_bit = 1; /* return 1, module is not present */

    if (port >= 5 && port <= 16) {
        if (dni_bmc_data_get(BMC_BUS, PORT_CPLD0_ADDR, reg_t, &present_bit) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;

        if(port >= 5 && port <= 8)
            bit_t = 1 << (8 - port);
        else if (port >= 9 && port <= 16)
            bit_t = 1 << (16 - port);

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
    for (port = 5; port <= NUM_OF_ALL_PORT + 4; port++) {
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
    uint8_t bytes[2] = {0};
    int reg_t = 0x00;
    int mux_bit = 0x01;
    int rxlos_data = 0x00;
    if (dni_bmc_data_set(BMC_BUS, MUX_ADDR, reg_t, (uint8_t)mux_bit) != ONLP_STATUS_OK)
        return ONLP_STATUS_E_INTERNAL;
   
    /* SFP Part 5-8*/
    if (dni_bmc_data_get(BMC_BUS, PORT_CPLD0_ADDR, SFP_RX_LOSS_REG, &rxlos_data) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
    bytes[0] = rxlos_data;

    /* SFP+ Part 9-16*/
    if (dni_bmc_data_get(BMC_BUS, PORT_CPLD0_ADDR, SFPP_RX_LOSS_REG, &rxlos_data) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
    bytes[1] = rxlos_data;

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t rxlos_all = 0;

    rxlos_all |= bytes[0];
    rxlos_all <<= 8;
    rxlos_all |= bytes[1];

    for(i = 16; i >= 5; i--)
    {
        AIM_BITMAP_MOD(dst, i, (rxlos_all & 1));
        rxlos_all >>= 1;
    }

     return ONLP_STATUS_OK;
}

int onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    int size = 0;

    VALIDATE_PORT(port);

    memset(data, 0, 256);
    if (onlp_file_read(data, 256, &size, PORT_EEPROM_FORMAT, FRONT_PORT_TO_BUS_INDEX(port))
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
    int reg_t = 0x00;
    int bit_t = 0x00;
    int rdata_bit = 0x00;
    int value_t;
    int mux_bit = 0x01;

    VALIDATE_PORT(port);

    if (dni_bmc_data_set(BMC_BUS, MUX_ADDR, reg_t, (uint8_t)mux_bit) != ONLP_STATUS_OK)
        return ONLP_STATUS_E_INTERNAL;

    switch (control) {
        case ONLP_SFP_CONTROL_RESET_STATE:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
        case ONLP_SFP_CONTROL_RX_LOS:
            if (port >= 5 && port <= 8) {        /* SFP Port 5-8 */
                reg_t = SFP_RX_LOSS_REG;
            } else if (port >= 9 && port <= 16) { /* SFP+ Port 9-16 */
                reg_t = SFPP_RX_LOSS_REG;
            } else
                rdata_bit = 1; /* return 1, module is not in RX LOSS */

            if (port >= 5 && port <= 16) {
                if (dni_bmc_data_get(BMC_BUS, PORT_CPLD0_ADDR, reg_t, &rdata_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                
                if(port >= 5 && port <= 8)
                    bit_t = 1 << (8 - port);
                else if (port >= 9 && port <= 16)
                    bit_t = 1 << (16 - port);

                rdata_bit = rdata_bit & bit_t;
                rdata_bit = rdata_bit / bit_t;
            }

            /* "1" = The module is not in RX LOSS
             * "0" = The module is in RX LOSS */
            *value = rdata_bit;
            value_t = ONLP_STATUS_OK;
            break;
        case ONLP_SFP_CONTROL_TX_DISABLE:
            if (port >= 5 && port <= 8) {        /* SFP Port 5-8 */
                reg_t = SFP_TX_DISABLE_REG;
            } else if (port >= 9 && port <= 16) { /* SFP+ Port 9-16 */
                reg_t = SFPP_TX_DISABLE_REG;
            } else
                rdata_bit = 0; /* return 0, module is not in TX DISABLE */

            if (port >= 5 && port <= 16) {
                if (dni_bmc_data_get(BMC_BUS, PORT_CPLD0_ADDR, reg_t, &rdata_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;

                if(port >= 5 && port <= 8)
                    bit_t = 1 << (8 - port);
                else if (port >= 9 && port <= 16)
                    bit_t = 1 << (16 - port);

                rdata_bit = rdata_bit & bit_t;
                rdata_bit = rdata_bit / bit_t;
            }

            /* "0" = The module is not in TX DISABLE
             * "1" = The module is in TX DISABLE */
            *value = rdata_bit;
            value_t = ONLP_STATUS_OK;
            break;
        case ONLP_SFP_CONTROL_TX_FAULT:
            if (port >= 5 && port <= 8) {        /* SFP Port 5-8 */
                reg_t = SFP_TX_FAULT_REG;
            } else if (port >= 9 && port <= 16) { /* SFP+ Port 9-16 */
                reg_t = SFPP_TX_FAULT_REG;
            } else
                rdata_bit = 1; /* return 1, module is not in TX FAULT */

            if (port >= 5 && port <= 16) {
                if (dni_bmc_data_get(BMC_BUS, PORT_CPLD0_ADDR, reg_t, &rdata_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;

                if(port >= 5 && port <= 8)
                    bit_t = 1 << (8 - port);
                else if (port >= 9 && port <= 16)
                    bit_t = 1 << (16 - port);

                rdata_bit = rdata_bit & bit_t;
                rdata_bit = rdata_bit / bit_t;
            }

            /* "1" = The module is not in TX FAULT
             * "0" = The module is in TX FAULT */
            *value = rdata_bit;
            value_t = ONLP_STATUS_OK;

            break;
        case ONLP_SFP_CONTROL_LP_MODE:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
        default:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }
    return value_t;
}

int onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int reg_t = 0x00;
    int bit_t = 0x00;
    int data_bit = 0x00;
    int mux_bit = 0x01;
    int value_t;

    VALIDATE_PORT(port);

    if (dni_bmc_data_set(BMC_BUS, MUX_ADDR, reg_t, (uint8_t)mux_bit) != ONLP_STATUS_OK)
        return ONLP_STATUS_E_INTERNAL;

    switch (control) {
        case ONLP_SFP_CONTROL_RESET_STATE:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
        case ONLP_SFP_CONTROL_RX_LOS:
            if (port >= 5 && port <= 8) {        /* SFP Port 5-8 */
                reg_t = SFP_RX_LOSS_REG;
            } else if (port >= 9 && port <= 16) { /* SFP+ Port 9-16 */
                reg_t = SFPP_RX_LOSS_REG;
            } else

            if (port >= 5 && port <= 16) {
                if (dni_bmc_data_get(BMC_BUS, PORT_CPLD0_ADDR, reg_t, &data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                /* Indicate the module is in RX LOSS or not
                 * 1 = Disable
                 * 0 = Enable */
                if (value == 1) {
                    if(port >= 5 && port <= 8)
                        bit_t = ~(1 << (8 - port));
                    else if (port >= 9 && port <= 16)
                        bit_t = ~(1 << (16 - port));

                    data_bit = data_bit & bit_t;
                }
                else if (value == 0) {
                    if(port >= 5 && port <= 8)
                        bit_t = 1 << (8 - port);
                    else if (port >= 9 && port <= 16)
                        bit_t = 1 << (16 - port);

                    data_bit = data_bit | bit_t;
                }
                if (dni_bmc_data_set(BMC_BUS, PORT_CPLD0_ADDR, reg_t, (uint8_t)data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
            }
            else
                return ONLP_STATUS_E_UNSUPPORTED;

            value_t = ONLP_STATUS_OK;
            break;
        case ONLP_SFP_CONTROL_TX_DISABLE:
            if (port >= 5 && port <= 8) {        /* SFP Port 5-8 */
                reg_t = SFP_TX_DISABLE_REG;
            } else if (port >= 9 && port <= 16) { /* SFP+ Port 9-16 */
                reg_t = SFPP_TX_DISABLE_REG;
            } else

            if (port >= 5 && port <= 16) {
                if (dni_bmc_data_get(BMC_BUS, PORT_CPLD0_ADDR, reg_t, &data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                /* Indicate the module is in TX DISABLE or not
                 * 0 = Disable
                 * 1 = Enable */
                if (value == 0) {
                    if(port >= 5 && port <= 8)
                        bit_t = ~(1 << (8 - port));
                    else if (port >= 9 && port <= 16)
                        bit_t = ~(1 << (16 - port));

                    data_bit = data_bit & bit_t;
                }
                else if (value == 1) {
                    if(port >= 5 && port <= 8)
                        bit_t = 1 << (8 - port);
                    else if (port >= 9 && port <= 16)
                        bit_t = 1 << (16 - port);

                    data_bit = data_bit | bit_t;
                }
                if (dni_bmc_data_set(BMC_BUS, PORT_CPLD0_ADDR, reg_t, (uint8_t)data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
            }
            else
                return ONLP_STATUS_E_UNSUPPORTED;

            value_t = ONLP_STATUS_OK;

            break;
        case ONLP_SFP_CONTROL_TX_FAULT:
            if (port >= 5 && port <= 8) {        /* SFP Port 5-8 */
                reg_t = SFP_TX_FAULT_REG;
            } else if (port >= 9 && port <= 16) { /* SFP+ Port 9-16 */
                reg_t = SFPP_TX_FAULT_REG;
            } else

            if (port >= 5 && port <= 16) {
                if (dni_bmc_data_get(BMC_BUS, PORT_CPLD0_ADDR, reg_t, &data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                /* Indicate the module is in TX FAULT or not
                 * 1 = Disable
                 * 0 = Enable */
                if (value == 1) {
                    if(port >= 5 && port <= 8)
                        bit_t = ~(1 << (8 - port));
                    else if (port >= 9 && port <= 16)
                        bit_t = ~(1 << (16 - port));

                    data_bit = data_bit & bit_t;
                }
                else if (value == 0) {
                    if(port >= 5 && port <= 8)
                        bit_t = 1 << (8 - port);
                    else if (port >= 9 && port <= 16)
                        bit_t = 1 << (16 - port);

                    data_bit = data_bit | bit_t;
                }
                if (dni_bmc_data_set(BMC_BUS, PORT_CPLD0_ADDR, reg_t, (uint8_t)data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
            }
            else
                return ONLP_STATUS_E_UNSUPPORTED;

            value_t = ONLP_STATUS_OK;

            break;
        case ONLP_SFP_CONTROL_LP_MODE:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
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
            *rv = 0;
            break;
        case ONLP_SFP_CONTROL_RX_LOS:
            if (port >= 5 && port <= 16)
                *rv = 1;
            else
                *rv = 0;
            break;
        case ONLP_SFP_CONTROL_TX_DISABLE:
            if (port >= 5 && port <= 16)
                *rv = 1;
            else
                *rv = 0;
            break;
        case ONLP_SFP_CONTROL_TX_FAULT:
            if (port >= 5 && port <= 16)
                *rv = 1;
            else
                *rv = 0;
            break;
        case ONLP_SFP_CONTROL_LP_MODE:
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
    int bus = FRONT_PORT_TO_BUS_INDEX(port);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    VALIDATE_PORT(port);
    int bus = FRONT_PORT_TO_BUS_INDEX(port);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    VALIDATE_PORT(port);
    int bus = FRONT_PORT_TO_BUS_INDEX(port);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    VALIDATE_PORT(port);
    int bus = FRONT_PORT_TO_BUS_INDEX(port);
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

