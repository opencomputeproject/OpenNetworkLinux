/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017 Delta Networks, Inc.
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
#include <onlplib/i2c.h>

#include "platform_lib.h"

/******************* Utility Function *****************************************/
int 
delta_agc7648a_get_respond_reg(int port)
{
    if(port >= NUM_OF_SFP && port < (NUM_OF_SFP + NUM_OF_QSFP)){
        return QSFP_RESPOND_REG;
    }
    else{
        return -1;
    }
}

int 
delta_agc7648a_get_respond_val(int port)
{
    if(port >= NUM_OF_SFP && port < (NUM_OF_SFP + NUM_OF_QSFP)){
        return (port%8);
    }
    else{
        return -1;
    }
}
/******************************************************************************/


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
    char port_data[2];
    int present_val, present;

    /* Select port */
    sprintf(port_data, "%d", port + 1);
    dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH);

    /* Read sfp_is_present attribute of sfp module is presented or not */
    present_val = dni_i2c_lock_read_attribute(NULL, SFP_IS_PRESENT_PATH);

    /* From sfp_is_present value,
     * return 0 = The module is preset
     * return 1 = The module is NOT present
     */
    if(present_val == 0) {
        present = 1;
    } else if (present_val == 1) {
        present = 0;
        AIM_LOG_ERROR("Unble to present status from port(%d)\r\n", port);
    } else {
        AIM_LOG_ERROR("Error to present status from port(%d)\r\n", port);
        present = -1;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    char present_all_data[21] = {0};
    uint32_t bytes[7];

    /* Read presence bitmap from SWPLD QSFP28 Presence Register
     * if only port 0 is present, return 7F FF FF FF
     * if only port 0 and 1 present, return 3F FF FF FF
     */
    if(dni_i2c_read_attribute_string(SFP_IS_PRESENT_ALL_PATH, present_all_data,
                     sizeof(present_all_data), 0) < 0) {
        return -1;
    }


    int count = sscanf(present_all_data, "%x %x %x %x %x %x %x",
                       bytes+0,
                       bytes+1,
                       bytes+2,
                       bytes+3,
                       bytes+4,
                       bytes+5,
                       bytes+6
                       );

    if(count != AIM_ARRAYSIZE(bytes)) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields from the sfp_is_present_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Mask out non-existant QSFP ports */
    bytes[6] &= 0x3F;

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t presence_all = 0 ;
    for(i = AIM_ARRAYSIZE(bytes)-1; i >= 0; i--) {
        presence_all <<= 8;
        presence_all |= bytes[i];
    }

    /* Populate bitmap */
    for(i = 0; i < NUM_OF_PORT; i++) {
        AIM_BITMAP_MOD(dst, i, !(presence_all & 1));
        presence_all >>= 1;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    char port_data[2];
    int sfp_respond_reg, sfp_respond_val;

    /* Get respond register if port have it */
    sfp_respond_reg = delta_agc7648a_get_respond_reg(port);
    if(sfp_respond_reg >= 0)
    {
        /* Set respond val */
        sfp_respond_val = delta_agc7648a_get_respond_val(port);
        dni_lock_cpld_write_attribute(SWPLD_PATH, sfp_respond_reg, sfp_respond_val);
    }

    /* Select port */
    sprintf(port_data, "%d", port + 1);
    dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH);

    /* Read eeprom information into data[] */
    if (dni_i2c_read_attribute_binary(SFP_EEPROM_PATH, (char *)data, 256, 256)
        != 0)
    {
        AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    char port_data[2];
    int sfp_respond_reg, sfp_respond_val;
    dev_info_t dev_info;

    /* Get respond register if port have it */
    sfp_respond_reg = delta_agc7648a_get_respond_reg(port);
    if(sfp_respond_reg >= 0)
    {
        /* Set respond val */
        sfp_respond_val = delta_agc7648a_get_respond_val(port);
        dni_lock_cpld_write_attribute(SWPLD_PATH, sfp_respond_reg, sfp_respond_val);
    }

    /* Select port */
    sprintf(port_data, "%d", port + 1);
    dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH);

    dev_info.bus = I2C_BUS_8;
    dev_info.addr = PORT_ADDR;
    dev_info.offset = addr;
    dev_info.flags = ONLP_I2C_F_FORCE;
    dev_info.size = 1;  /* Read 1 byte */

    return dni_i2c_lock_read(NULL, &dev_info);
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    char port_data[2];
    int sfp_respond_reg, sfp_respond_val;
    dev_info_t dev_info;

    /* Get respond register if port have it */
    sfp_respond_reg = delta_agc7648a_get_respond_reg(port);
    if(sfp_respond_reg >= 0)
    {
        /* Set respond val */
        sfp_respond_val = delta_agc7648a_get_respond_val(port);
        dni_lock_cpld_write_attribute(SWPLD_PATH, sfp_respond_reg, sfp_respond_val);
    }

    /* Select port */
    sprintf(port_data, "%d", port + 1);
    dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH);

    dev_info.bus = I2C_BUS_8;
    dev_info.addr = PORT_ADDR;
    dev_info.offset = addr;
    dev_info.flags = ONLP_I2C_F_FORCE;
    dev_info.size = 1;  /* Write 1 byte */
    dev_info.data_8 = value;

    return dni_i2c_lock_write(NULL, &dev_info);
}

int 
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    char port_data[2];
    int sfp_respond_reg, sfp_respond_val;
    dev_info_t dev_info;

    /* Get respond register if port have it */
    sfp_respond_reg = delta_agc7648a_get_respond_reg(port);
    if(sfp_respond_reg >= 0)
    {
        /* Set respond val */
        sfp_respond_val = delta_agc7648a_get_respond_val(port);
        dni_lock_cpld_write_attribute(SWPLD_PATH, sfp_respond_reg, sfp_respond_val);
    }

    /* Select port */
    sprintf(port_data, "%d", port + 1);
    dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH);

    dev_info.bus = I2C_BUS_8;
    dev_info.addr = PORT_ADDR;
    dev_info.offset = addr;
    dev_info.flags = ONLP_I2C_F_FORCE;
    dev_info.size = 2;  /* Read 1 byte */

    return dni_i2c_lock_read(NULL, &dev_info);
}

int 
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    char port_data[2];
    int sfp_respond_reg, sfp_respond_val;
    dev_info_t dev_info;

    /* Get respond register if port have it */
    sfp_respond_reg = delta_agc7648a_get_respond_reg(port);
    if(sfp_respond_reg >= 0)
    {
        /* Set respond val */
        sfp_respond_val = delta_agc7648a_get_respond_val(port);
        dni_lock_cpld_write_attribute(SWPLD_PATH, sfp_respond_reg, sfp_respond_val);
    }

    /* Select port */
    sprintf(port_data, "%d", port + 1);
    dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH);

    dev_info.bus = I2C_BUS_8;
    dev_info.addr = PORT_ADDR;
    dev_info.offset = addr;
    dev_info.flags = ONLP_I2C_F_FORCE;
    dev_info.size = 2;  /* Write 2 byte */
    dev_info.data_16 = value;

    return dni_i2c_lock_write(NULL, &dev_info);
}

int
onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv)
{
    char port_data[2];

    /* Select QSFP port */
    sprintf(port_data, "%d", port + 1);
    dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH);

    switch (control) {
    case ONLP_SFP_CONTROL_RESET_STATE:
        if(port >= NUM_OF_SFP && port < (NUM_OF_SFP + NUM_OF_QSFP)){
            *rv = 1;
        }
        else{
            *rv = 0;
        }
        break;
    case ONLP_SFP_CONTROL_RX_LOS:
        *rv = 0;
        break;
    case ONLP_SFP_CONTROL_TX_DISABLE:
        *rv = 0;
        break;
    case ONLP_SFP_CONTROL_LP_MODE:
       if(port >= NUM_OF_SFP && port < (NUM_OF_SFP + NUM_OF_QSFP)){
            *rv = 1;
        }
        else{
            *rv = 0;
        }
        break;
    default:
        break;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    uint8_t value_t;
    char port_data[2];

    /* Select QSFP port */
    sprintf(port_data, "%d", port + 1);
    dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH);

    switch (control) {
    case ONLP_SFP_CONTROL_RESET_STATE:
        sprintf(port_data, "%d", value);
        dni_i2c_lock_write_attribute(NULL, port_data, SFP_RESET_PATH);
        value_t = ONLP_STATUS_OK;
        break;
    case ONLP_SFP_CONTROL_RX_LOS:
        value_t = ONLP_STATUS_E_UNSUPPORTED;
        break;
    case ONLP_SFP_CONTROL_TX_DISABLE:
        value_t = ONLP_STATUS_E_UNSUPPORTED;
        break;
    case ONLP_SFP_CONTROL_LP_MODE:
        sprintf(port_data, "%d", value);
        dni_i2c_lock_write_attribute(NULL, port_data, SFP_LP_MODE_PATH);
        value_t = ONLP_STATUS_OK;
        break;
    default:
        value_t = ONLP_STATUS_E_UNSUPPORTED;
        break;
    }

    return value_t;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    uint8_t value_t;
    char port_data[2];

    /* Select QSFP port */
    sprintf(port_data, "%d", port + 1);
    dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH);

    switch (control) {
    case ONLP_SFP_CONTROL_RESET_STATE:
        *value = dni_i2c_lock_read_attribute(NULL, SFP_RESET_PATH);
        /* From sfp_reset value,
         * return 0 = The module is in Reset
         * return 1 = The module is NOT in Reset
         */
        if (*value == 0){
            *value = 1;
        }
        else if(*value == 1){
            *value = 0;
        }
        value_t = ONLP_STATUS_OK;
        break;
    case ONLP_SFP_CONTROL_RX_LOS:
        *value = 0;
        value_t = ONLP_STATUS_E_UNSUPPORTED;
        break;
    case ONLP_SFP_CONTROL_TX_DISABLE:
        *value = 0;
        value_t = ONLP_STATUS_E_UNSUPPORTED;
        break;
    case ONLP_SFP_CONTROL_LP_MODE:
        /* From sfp_lp_mode value,
         * return 0 = The module is NOT in LP mode
         * return 1 = The moduel is in LP mode
         */
        *value = dni_i2c_lock_read_attribute(NULL, SFP_LP_MODE_PATH);
        value_t = ONLP_STATUS_OK;
        break;
    default:
        value_t = ONLP_STATUS_E_UNSUPPORTED;
        break;
    }

    return value_t;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_post_insert(int port, sff_info_t* info)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

void
onlp_sfpi_debug(int port, aim_pvs_t* pvs)
{

}

int
onlp_sfpi_ioctl(int port, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
