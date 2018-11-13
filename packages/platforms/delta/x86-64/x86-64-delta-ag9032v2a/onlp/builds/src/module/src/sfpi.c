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

int sfp_map_bus[] ={31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                    41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
                    51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
                    61, 62, 63};

int 
ag9032v2a_get_respond_val(int port){   
    int respond_default = 0xff;
    int value = 0x00;
    int port_select;
    if(port > 0 && port <= 32){
        port_select = (port % 8);

        if (port_select==0){
            value = respond_default & (~(1 << 0));
        }
        else{
            value = respond_default & (~(1 << (8 - port_select)));
        }
        return value;
    }
    else{
        return respond_default;
    }
}

int 
ag9032v2a_get_respond_reg(int port){
    uint8_t reg_offset = 0x00;
    if (port < 9)
        reg_offset = SFP_RESPOND_1;
    else if (port > 8 && port < 17)
        reg_offset = SFP_RESPOND_2;
    else if (port > 16 && port < 25)
        reg_offset = SFP_RESPOND_3;
    else if (port > 24 && port < 33)
        reg_offset = SFP_RESPOND_4;
    else 
        reg_offset = 0x00;

    return reg_offset;
}

/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/

int  
onlp_sfpi_init(void){
    /* Called at initialization time */
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_map_bus_index(int port)
{
    if(port < 0 || port > 33)
        return ONLP_STATUS_E_INTERNAL;
    return sfp_map_bus[port-1];
}

int  
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap){
    /*Ports {1, 33}*/
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = 1; p <= NUM_OF_PORT; p++) {
        AIM_BITMAP_SET(bmap, p);
    }
    return ONLP_STATUS_OK;
}
  
int  
onlp_sfpi_is_present(int port){
    char port_data[2];
    int present, present_bit;

    if(port > 0 && port < 34)
    {
        /* Select QSFP/SFP port */
        sprintf(port_data, "%d", port );
        if(dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH) < 0){
            AIM_LOG_ERROR("Unable to select port(%d)\r\n", port);
        }

        /* Read QSFP/SFP MODULE is present or not */
        present_bit = dni_i2c_lock_read_attribute(NULL, SFP_IS_PRESENT_PATH);
        if(present_bit < 0){
            AIM_LOG_ERROR("Unable to read present or not from port(%d)\r\n", port);
        }
    }

    /* From sfp_is_present value,
     * return 0 = The module is preset
     * return 1 = The module is NOT present
     */
    if(present_bit == 0) {
        present = 1;
    } else if (present_bit == 1) {
        present = 0;
        AIM_LOG_ERROR("Unble to present status from port(%d)\r\n", port);
    } else {
        /* Port range over 1-33, return -1 */
        AIM_LOG_ERROR("Error to present status from port(%d)\r\n", port);
        present = -1;
    }
    return present;
}

int  
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    char present_all_data[12] = {'\0'};
    char *r_byte;
    char *r_array[4];
    uint8_t bytes[4];
    char port_data[2];
    int count = 0;
    uint8_t sfp_presence;

    /* Read presence bitmap from SWPLD QSFP28 Presence Register
     * if only port 0 is present, return 7F FF FF FF
     * if only port 0 and 1 present, return 3F FF FF FF
     */
    if(dni_i2c_read_attribute_string(SFP_IS_PRESENT_ALL_PATH, present_all_data,
                                         sizeof(present_all_data), 0) < 0) {
        return -1;
    }

    /* String split */
    r_byte = strtok(present_all_data, " ");
    while (r_byte != NULL) {
        r_array[count++] = r_byte;
        r_byte = strtok(NULL, " ");
    }

    /* Convert a string to long integer
     * and saved into bytes[]
     */
    for (count = 0; count < 4; count++) {
        bytes[count] = ~strtol(r_array[count], NULL, 16);
    }

    /* Convert to 64 bit integer in port order */
    int i = 0;
    int j = 31;
    uint32_t presence_all = 0 ;
    for(i = AIM_ARRAYSIZE(bytes)-1; i >= 0; i--) {
        presence_all <<= 8;
        presence_all |= bytes[i];
    }

    /* Populate bitmap & remap*/
    for(i = 0; presence_all; i++)
    {
        if(23 < j)
            AIM_BITMAP_MOD(dst, (j - 24)+1,(presence_all & 1));
        else if(15 < j && j < 24)
            AIM_BITMAP_MOD(dst, (j - 8)+1,(presence_all & 1));
        else if(7 < j && j < 16)
            AIM_BITMAP_MOD(dst, (j + 8)+1,(presence_all & 1));
        else
            AIM_BITMAP_MOD(dst, (j + 24)+1,(presence_all & 1));
        presence_all >>= 1;
        j--;
    }

    /* Populate SFP bitmap */
    int port = 33;
    sprintf(port_data, "%d", port );
    if(dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH) < 0){
        AIM_LOG_ERROR("Unable to select port(%d)\r\n", port);
    }
    sfp_presence = dni_i2c_lock_read_attribute(NULL, SFP_IS_PRESENT_PATH);;

    AIM_BITMAP_MOD(dst, 33, !(sfp_presence & 1));


    return ONLP_STATUS_OK;
}

int  
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    int sfp_respond_reg;
    int sfp_respond_val;
    int size = 0;

    /* Get respond register if port have it */
    sfp_respond_reg = ag9032v2a_get_respond_reg(port);

    /* Set respond val */
    sfp_respond_val = ag9032v2a_get_respond_val(port);
    dni_lock_cpld_write_attribute(SWPLD1_PATH, sfp_respond_reg, sfp_respond_val);

    memset(data, 0, 256);

    if(onlp_file_read(data, 256, &size, PORT_EEPROM_FORMAT, onlp_sfpi_map_bus_index(port)) != ONLP_STATUS_OK) {
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

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value) 
{
    int value_t;
    char port_data[2];

    if(port > 0 && port < 34)
    {
        /* Select QSFP port */
        sprintf(port_data, "%d", port );
        if(dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH) < 0){
            AIM_LOG_INFO("Unable to select port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    switch (control) {   
        case ONLP_SFP_CONTROL_RESET_STATE:
           *value = dni_i2c_lock_read_attribute(NULL, QSFP_RESET_PATH);
            /* From sfp_reset value,
             * return 0 = The module is in Reset
             * return 1 = The module is NOT in Reset
             */
            if (*value == 0)
            {
                *value = 1;      
            }
            else if (*value == 1) 
            {
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
            *value = dni_i2c_lock_read_attribute(NULL, QSFP_LP_MODE_PATH);
            value_t = ONLP_STATUS_OK;
            break;
        default:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }
    return value_t;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int value_t;
    char port_data[2];

    if(port > 0 && port < 33)
    {
        /* Select QSFP port */
        sprintf(port_data, "%d", port );
        if(dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH) < 0){
            AIM_LOG_INFO("Unable to select port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    switch (control) {
        case ONLP_SFP_CONTROL_RESET_STATE:
           sprintf(port_data, "%d", value);
           if(dni_i2c_lock_write_attribute(NULL, port_data, QSFP_RESET_PATH) < 0){
               AIM_LOG_INFO("Unable to control reset state from port(%d)\r\n", port);
               value_t = ONLP_STATUS_E_INTERNAL;
           }
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
           if(dni_i2c_lock_write_attribute(NULL, port_data, QSFP_LP_MODE_PATH) < 0){
               AIM_LOG_INFO("Unable to control LP mode from port(%d)\r\n", port);
               value_t = ONLP_STATUS_E_INTERNAL;
           }
           value_t = ONLP_STATUS_OK;
           break;
        default:
           value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }
    return value_t;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int bus;
 
    bus = onlp_sfpi_map_bus_index(port);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int bus;

    bus = onlp_sfpi_map_bus_index(port);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
    
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int bus;

    bus = onlp_sfpi_map_bus_index(port);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int bus;
 
    bus = onlp_sfpi_map_bus_index(port);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv)
{
    char port_data[2] ;

    if(port > 0 && port < 33)
    {
        /* Select QSFP port */
        sprintf(port_data, "%d", port );
        if(dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH) < 0){
            AIM_LOG_INFO("Unable to select port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    switch (control) {
        case ONLP_SFP_CONTROL_RESET_STATE:
            if(port > 0 && port < 33){
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
            if(port > 0 && port < 33){
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
    return ONLP_STATUS_OK;
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

