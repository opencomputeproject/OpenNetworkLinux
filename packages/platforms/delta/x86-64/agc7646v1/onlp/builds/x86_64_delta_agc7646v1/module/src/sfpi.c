/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017  Delta Networks, Inc. 
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
#include <math.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include "platform_lib.h"

/******************* Utility Function *****************************************/
int sfp_map_bus[] = {
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60, /* SFP */
    61, 62, 63, 64, 65, 66, 67, 68, 69, 70, /* SFP */
    71, 72, 73, 74, 75, 76, 77, 78, 79, 80, /* SFP */
    81, 82, 83, 84, 85, 86, 87, 88, 89, 90, /* SFP */
    91, 92, 93, 94, 95, 96, /* SFP */
    41, 42, 43, 44, 45, 46, /* QSFP */
};

/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
int onlp_sfpi_init(void)
{
    /* Called at initialization time */
    lockinit();
    return ONLP_STATUS_OK;
}

int onlp_sfpi_map_bus_index(int port)
{
    if (port < 0 || port > 52)
        return ONLP_STATUS_E_INTERNAL;
    return sfp_map_bus[port-1];
}

int onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    /* Ports {1, 52} */
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = 1; p <= NUM_OF_PORT; p++) {
        AIM_BITMAP_SET(bmap, p);
    }
    return ONLP_STATUS_OK;
}
  
int onlp_sfpi_is_present(int port)
{
    int present, present_bit = 0x00;
    uint8_t reg_t = 0x00;
    int bit_t = 0x00;

    if (port > 0 && port < 9) {          /* SFP Port 1-8 */
        reg_t = SFP_PRESENCE_1;
    } else if (port > 8 && port < 17) {  /* SFP Port 9-16 */
        reg_t = SFP_PRESENCE_2;
    } else if (port > 16 && port < 25) { /* SFP Port 17-24 */
        reg_t = SFP_PRESENCE_3;
    } else if (port > 24 && port < 33) { /* SFP Port 25-32 */
        reg_t = SFP_PRESENCE_4;
    } else if (port > 32 && port < 41) { /* SFP Port 33-40 */
        reg_t = SFP_PRESENCE_5;
    } else if (port > 40 && port < 47) { /* SFP Port 41-46 */
        reg_t = SFP_PRESENCE_6;
    } else if (port > 46 && port < 53) { /* QSFP Port 1-6 */
        reg_t = QSFP_PRESENCE;
    } else {
        present_bit = 1; /* return 1, module is not present */
    }

    if (port > 46 && port < 53) { /* QSFP */
        if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_1_ADDR, reg_t, &present_bit) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
        bit_t = 1 << (port % 47);
        present_bit = present_bit & bit_t;
        present_bit = present_bit / bit_t; 
    }
    else { /* SFP */
        if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, reg_t, &present_bit) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
        port = port - 1;
        bit_t = 1 << (port % 8);
        present_bit = present_bit & bit_t;
        present_bit = present_bit / bit_t;
    }

    /* From sfp_is_present value,
     * return 0 = The module is preset
     * return 1 = The module is NOT present */
    if (present_bit == 0) {
        present = 1;
    } else if (present_bit == 1) {
        present = 0;
        AIM_LOG_ERROR("Unble to present status from port(%d)\r\n", port);
    } else {
        /* Port range over 1-52, return -1 */
        AIM_LOG_ERROR("Error to present status from port(%d)\r\n", port);
        present = -1;
    }
    return present;
}

int onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int count = 0;
    uint8_t bytes[7] = {0};
    uint8_t reg_t = 0x00;
    int present_data = 0x00;
    uint8_t r_array[7] = {0};

    /* Read presence bitmap from SWPLD2 SFP+ and SWPLD1 QSFP28 Presence Register
     * if only port 0 is present,    return F FF FF FF FF FF FE
     * if only port 0 and 1 present, return F FF FF FF FF FF FC */

    for (count = 0; count < 7; count++) {
        if (count < 6) { /* SFP Port 1-46 */
            reg_t = SFP_PRESENCE_1 + count;
            if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, reg_t, &present_data) != ONLP_STATUS_OK)
                return ONLP_STATUS_E_INTERNAL;
            r_array[count] = present_data;
        }
        else { /* QSFP Port 47-52 */
            reg_t = QSFP_PRESENCE;
            if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_1_ADDR, reg_t, &present_data) != ONLP_STATUS_OK)
                 return ONLP_STATUS_E_INTERNAL;
            r_array[count] = present_data;
        }
    }

    /* Invert r_array[] and reverse elements, saved into bytes[] */
    for (count = 0; count < 7; count++) {
        bytes[count] = ~r_array[6 - count];
    }
    /* Mask out non-existant SFP/QSFP ports */
    uint8_t save_bytes = 0x00;

    bytes[1] &= 0x3F;
    bytes[0] &= 0x3F;
    save_bytes = bytes[0] & 0x03;
    save_bytes = save_bytes << 6;
    bytes[1] |= save_bytes;
    bytes[0] &= 0x3c;
    bytes[0] = bytes[0] >> 2;

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t presence_all = 0;

    for (i = 0; i < AIM_ARRAYSIZE(bytes); i++) {
        presence_all <<= 8;
        presence_all |= bytes[i];
    }
    /* Populate bitmap & remap */
    for (i = 0; presence_all; i++)
    {
        AIM_BITMAP_MOD(dst, i+1, (presence_all & 1));
        presence_all >>= 1;
    }
    return ONLP_STATUS_OK;
}

int onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    int size = 0;

    memset(data, 0, 256);

    if (onlp_file_read(data, 256, &size, PORT_EEPROM_FORMAT, onlp_sfpi_map_bus_index(port)) != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (size != 256) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d), size is different!\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}

int onlp_sfpi_port_map(int port, int* rport)
{
    *rport = port;
    return ONLP_STATUS_OK;
}

int onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int value_t;
    uint8_t reg_t = 0x00;
    int rdata_bit = 0x00;
    int bit_t = 0x00;

    switch (control) {
        case ONLP_SFP_CONTROL_RESET_STATE:
            /* From sfp_reset value,
             * return 0 = The module is in Reset
             * return 1 = The module is not in Reset */

            if (port > 46 && port < 53) { /* QSFP Port 47-52 */
                reg_t = QSFP_RESET;
                if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_1_ADDR, reg_t, &rdata_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                bit_t = 1 << (port % 47);
                rdata_bit = rdata_bit & bit_t;
                rdata_bit = rdata_bit / bit_t;
            } else { /* In agc7646v1 only QSFP support control RESET MODE */
                rdata_bit = 1; /* return 1, module not in reset mode */
            }

            if (rdata_bit == 0)
                *value = 1;
            else if (rdata_bit == 1)
                *value = 0;

            value_t = ONLP_STATUS_OK;
            break;
        case ONLP_SFP_CONTROL_RX_LOS:
            /* From sfp_rx_los value,
             * return 0 = The module is Normal Operation
             * return 1 = The module is Error */

            if (port > 0 && port < 47) { /* SFP */
                if (port > 0 && port < 9) {          /* SFP Port 1-8 */
                    reg_t = SFP_RXLOS_1;
                } else if (port > 8 && port < 17) {  /* SFP Port 9-16 */
                    reg_t = SFP_RXLOS_2;
                } else if (port > 16 && port < 25) { /* SFP Port 17-24 */
                    reg_t = SFP_RXLOS_3;
                } else if (port > 24 && port < 33) { /* SFP Port 25-32 */
                    reg_t = SFP_RXLOS_4;
                } else if (port > 32 && port < 41) { /* SFP Port 33-40 */
                    reg_t = SFP_RXLOS_5;
                } else if (port > 40 && port < 47) { /* SFP Port 41-46 */
                    reg_t = SFP_RXLOS_6;
                }
                if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, reg_t, &rdata_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                                port = port - 1;
                                bit_t = 1 << (port % 8);
                                rdata_bit = rdata_bit & bit_t;
                                rdata_bit = rdata_bit / bit_t;
            }
            else { /* In agc7646v1 only SFP support control RX_LOS MODE */
                rdata_bit = 1; /* return 1, module Error */
            }
            *value = rdata_bit;

            value_t = ONLP_STATUS_OK;
            break;
        case ONLP_SFP_CONTROL_TX_DISABLE:
            /* From sfp_tx_disable value,
             * return 0 = The module is Enable Transmitter on
             * return 1 = The module is Transmitter Disabled */

            if (port > 0 && port < 47) { /* SFP */
                if (port > 0 && port < 9) {          /* SFP Port 1-8 */
                    reg_t = SFP_TXDIS_1;
                } else if (port > 8 && port < 17) {  /* SFP Port 9-16 */
                    reg_t = SFP_TXDIS_2;
                } else if (port > 16 && port < 25) { /* SFP Port 17-24 */
                    reg_t = SFP_TXDIS_3;
                } else if (port > 24 && port < 33) { /* SFP Port 25-32 */
                    reg_t = SFP_TXDIS_4;
                } else if (port > 32 && port < 41) { /* SFP Port 33-40 */
                    reg_t = SFP_TXDIS_5;
                } else if (port > 40 && port < 47) { /* SFP Port 41-46 */
                    reg_t = SFP_TXDIS_6;
                }
                if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, reg_t, &rdata_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                                port = port - 1;
                                bit_t = 1 << (port % 8);
                                rdata_bit = rdata_bit & bit_t;
                                rdata_bit = rdata_bit / bit_t;
            }
            else { /* In agc7646v1 only SFP support control TX_DISABLE MODE */
                rdata_bit = 1; /* return 1, module Transmitter Disabled */
            }
            *value = rdata_bit;

            value_t = ONLP_STATUS_OK;
            break;
        case ONLP_SFP_CONTROL_TX_FAULT:
            /* From sfp_tx_fault value,
             * return 0 = The module is Normal
             * return 1 = The module is Fault */

            if (port > 0 && port < 47) { /* SFP */
                if (port > 0 && port < 9) {          /* SFP Port 1-8 */
                    reg_t = SFP_TXFAULT_1;
                } else if (port > 8 && port < 17) {  /* SFP Port 9-16 */
                    reg_t = SFP_TXFAULT_2;
                } else if (port > 16 && port < 25) { /* SFP Port 17-24 */
                    reg_t = SFP_TXFAULT_3;
                } else if (port > 24 && port < 33) { /* SFP Port 25-32 */
                    reg_t = SFP_TXFAULT_4;
                } else if (port > 32 && port < 41) { /* SFP Port 33-40 */
                    reg_t = SFP_TXFAULT_5;
                } else if (port > 40 && port < 47) { /* SFP Port 41-46 */
                    reg_t = SFP_TXFAULT_6;
                }
                if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, reg_t, &rdata_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                                port = port - 1;
                                bit_t = 1 << (port % 8);
                                rdata_bit = rdata_bit & bit_t;
                                rdata_bit = rdata_bit / bit_t;
            }
            else { /* In agc7646v1 only SFP support control TX_FAULT MODE */
                rdata_bit = 1; /* return 1, module is Fault */
            }
            *value = rdata_bit;

            value_t = ONLP_STATUS_OK;
            break;
        case ONLP_SFP_CONTROL_LP_MODE:
            /* From sfp_lp_mode value,
             * return 0 = The module is not in LP mode
             * return 1 = The module is in LP mode */

            if (port > 46 && port < 53) { /* QSFP Port 47-52 */
                reg_t = QSFP_LPMODE;
                if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_1_ADDR, reg_t, &rdata_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                bit_t = 1 << (port % 47);
                rdata_bit = rdata_bit & bit_t;
                rdata_bit = rdata_bit / bit_t;
            } else { /* In agc7646v1 only QSFP support control LP MODE */
                rdata_bit = 0; /* return 0, module is not in LP mode */
            }
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
    int value_t;
    uint8_t reg_t = 0x00;
    int data_bit = 0x00;
    int bit_t = 0x00;

    switch (control) {
        case ONLP_SFP_CONTROL_RESET_STATE:
            if (port > 46 && port < 53) { /* QSFP Port 47-52 */
                reg_t = QSFP_RESET;
                if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_1_ADDR, reg_t, &data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                /* Indicate the module is in reset mode or not
                 * 0 = Reset
                 * 1 = Normal */
                if (value == 0) {
                    bit_t = ~(1 << (port % 47));
                    data_bit = data_bit & bit_t;
                }
                else if (value == 1) {
                    bit_t = (1 << (port % 47));
                    data_bit = data_bit | bit_t;
                }
                if (dni_bmc_data_set(BMC_SWPLD_BUS, SWPLD_1_ADDR, reg_t, (uint8_t)data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
            } else {
                return ONLP_STATUS_E_UNSUPPORTED;
            }

            value_t = ONLP_STATUS_OK;
            break;
        case ONLP_SFP_CONTROL_RX_LOS:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
        case ONLP_SFP_CONTROL_TX_DISABLE:
            if (port > 0 && port < 47) { /* SFP */
                if (port > 0 && port < 9) {          /* SFP Port 1-8 */
                    reg_t = SFP_TXDIS_1;
                } else if (port > 8 && port < 17) {  /* SFP Port 9-16 */
                    reg_t = SFP_TXDIS_2;
                } else if (port > 16 && port < 25) { /* SFP Port 17-24 */
                    reg_t = SFP_TXDIS_3;
                } else if (port > 24 && port < 33) { /* SFP Port 25-32 */
                    reg_t = SFP_TXDIS_4;
                } else if (port > 32 && port < 41) { /* SFP Port 33-40 */
                    reg_t = SFP_TXDIS_5;
                } else if (port > 40 && port < 47) { /* SFP Port 41-46 */
                    reg_t = SFP_TXDIS_6;
                }

                if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, reg_t, &data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                /* Indicate the module is Enable Transmitter on or not
                 * 0 = Enable
                 * 1 = Disable */
                port = port - 1;
                if (value == 0) {
                    bit_t = ~(1 << (port % 8));
                    data_bit = data_bit & bit_t;
                }
                else if (value == 1) {
                    bit_t = (1 << (port % 8));
                    data_bit = data_bit | bit_t;
                }
                if (dni_bmc_data_set(BMC_SWPLD_BUS, SWPLD_2_ADDR, reg_t, (uint8_t)data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
            } else {
                return ONLP_STATUS_E_UNSUPPORTED;
            }

            value_t = ONLP_STATUS_OK;
            break;
        case ONLP_SFP_CONTROL_TX_FAULT:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
        case ONLP_SFP_CONTROL_LP_MODE:
            if (port > 46 && port < 53) { /* QSFP Port 47-52 */
                reg_t = QSFP_LPMODE;
                if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_1_ADDR, reg_t, &data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
                /* Indicate the module is in LP mode or not
                 * 0 = Disable
                 * 1 = Enable */
                if (value == 0) {
                    bit_t = ~(1 << (port % 47));
                    data_bit = data_bit & bit_t;
                }
                else if (value == 1) {
                    bit_t = (1 << (port % 47));
                    data_bit = data_bit | bit_t;
                }
                if (dni_bmc_data_set(BMC_SWPLD_BUS, SWPLD_1_ADDR, reg_t, (uint8_t)data_bit) != ONLP_STATUS_OK)
                    return ONLP_STATUS_E_INTERNAL;
            } else {
                return ONLP_STATUS_E_UNSUPPORTED;
            }

            value_t = ONLP_STATUS_OK;
            break;
        default:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }
    return value_t;
}

int onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int bus;
 
    bus = onlp_sfpi_map_bus_index(port);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int bus;

    bus = onlp_sfpi_map_bus_index(port);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
    
}

int onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int bus;

    bus = onlp_sfpi_map_bus_index(port);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int bus;
 
    bus = onlp_sfpi_map_bus_index(port);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv)
{
    switch (control) {
        case ONLP_SFP_CONTROL_RESET_STATE:
            if (port > 46 && port < 53) /* QSFP */
                *rv = 1;
            else
                *rv = 0;
            break;
        case ONLP_SFP_CONTROL_RX_LOS:
            if (port > 0 && port < 47)  /* SFP */
                *rv = 1;
            else
                *rv = 0;
            break;
        case ONLP_SFP_CONTROL_TX_DISABLE:
            if (port > 0 && port < 47)  /* SFP */
                *rv = 1;
            else
                *rv = 0;
            break;
        case ONLP_SFP_CONTROL_TX_FAULT:
            if (port > 0 && port < 47)  /* SFP */
                *rv = 1;
            else
                *rv = 0;
            break;
        case ONLP_SFP_CONTROL_LP_MODE:
            if (port > 46 && port < 53) /* QSFP */
                *rv = 1;
            else
                *rv = 0;
            break;
        default:
            break;
    }
    return ONLP_STATUS_OK;
}

int onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

int onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int count = 0;
    uint8_t bytes[6] = {0};
    uint8_t reg_t = 0x00;
    int rxlos_data = 0x00;
    uint8_t r_array[6] = {0};

    /* Read rx_los bitmap from SWPLD2 SFP+ LOSS Register
     * if only port 0 is normal operation,    return FF FF FF FF FF FE
     * if only port 0 and 1 normal operation, return FF FF FF FF FF FC */

    for (count = 0; count < 6; count++) { /* SFP Port 1-46 */
        reg_t = SFP_RXLOS_1 + count;
        if (dni_bmc_data_get(BMC_SWPLD_BUS, SWPLD_2_ADDR, reg_t, &rxlos_data) != ONLP_STATUS_OK)
            return ONLP_STATUS_E_INTERNAL;
        r_array[count] = rxlos_data;
    }

    /* Invert r_array[] and saved into bytes[] */
    for (count = 0; count < 6; count++) {
        bytes[count] = r_array[5 - count];
    }

    bytes[0] &= 0x3F;

    /* Convert to 64 bit integer in port order */
    int i = 6;
    uint64_t rxlos_all = 0;

    for (i = 0; i < AIM_ARRAYSIZE(bytes); i++) {
        rxlos_all <<= 8;
        rxlos_all |= bytes[i];
    }

    for (i = 0; i < 46; i++)
    {
        AIM_BITMAP_MOD(dst, i+1, (rxlos_all & 1));
        rxlos_all >>= 1;
    }

    /* Mask out non-existant QSFP ports */
    for (i = 46; i < 52; i++)
        AIM_BITMAP_MOD(dst, i+1, 0);

    return ONLP_STATUS_OK;

}

int onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    return ONLP_STATUS_OK;
}

int onlp_sfpi_post_insert(int port, sff_info_t* info)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

void onlp_sfpi_debug(int port, aim_pvs_t* pvs)
{
}

int onlp_sfpi_ioctl(int port, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

