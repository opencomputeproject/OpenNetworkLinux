/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 (C) Delta Networks, Inc.
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
#include "platform_lib.h"

static inline int ag9032v2_sfp_get_lp_mode_reg(int port) {
    uint8_t reg_offset = 0x00;
    if (port < 8)			            /* port 0-7 */
        reg_offset = SFP_LP_MODE_1;	
    else if (port > 7 && port < 16)	    /* port 8-15 */
        reg_offset = SFP_LP_MODE_2;
    else if (port > 15 && port < 24)	/* port 16-23 */
        reg_offset = SFP_LP_MODE_3;
    else if (port > 23 && port < 32)	/* port 24-31 */
        reg_offset = SFP_LP_MODE_4;

    return reg_offset;
}

static inline int ag9032v2_sfp_get_reset_reg(int port) {
    uint8_t reg_offset = 0x00;
    if (port < 8)			            /* port 0-7 */
        reg_offset = SFP_RESET_1;	
    else if (port > 7 && port < 16)	    /* port 8-15 */
        reg_offset = SFP_RESET_2;
    else if (port > 15 && port < 24)	/* port 16-23 */
        reg_offset = SFP_RESET_3;
    else if (port > 23 && port < 32)	/* port 24-31 */
        reg_offset = SFP_RESET_4;

    return reg_offset;
}

static inline int ag9032v2_sfp_get_present_reg(int port) {
    uint8_t reg_offset = 0x00;
    if (port < 8)			            /* port 0-7 */
        reg_offset = SFP_PRESENT_1;	
    else if (port > 7 && port < 16)	    /* port 8-15 */
        reg_offset = SFP_PRESENT_2;
    else if (port > 15 && port < 24)	/* port 16-23 */
        reg_offset = SFP_PRESENT_3;
    else if (port > 23 && port < 32)	/* port 24-31 */
        reg_offset = SFP_PRESENT_4;

    return reg_offset;
}

static inline int ag9032v2_sfp_get_respond_reg(int port) {
    uint8_t reg_offset = 0x00;
    if (port < 8)                       /* port 0-7 */
        reg_offset = SFP_RESPOND_1; 
    else if (port > 7 && port < 16)     /* port 8-15 */
        reg_offset = SFP_RESPOND_2;
    else if (port > 15 && port < 24)    /* port 16-23 */
        reg_offset = SFP_RESPOND_3;
    else if (port > 23 && port < 32)    /* port 24-31 */
        reg_offset = SFP_RESPOND_4;

    return reg_offset;
}

static inline int ag9032v2_sfp_get_mux_reg(int port) {
    uint8_t sel_channel = 0x00;
    if (port >= 0 && port < NUM_OF_QSFP_PORT)   /* port 0-31 ,reg : 0x01 - 0x32 */
        sel_channel = port;                                     
    else if (port == NUM_OF_QSFP_PORT){         /* port 32 */
        sel_channel = 0x20;
    } 
    else if (port == NUM_OF_QSFP_PORT + 1){     /* port 333 */
        sel_channel = 0x21;
    }
    else
        AIM_LOG_ERROR("qsfp port range is 0-33");

    return sel_channel;
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
    int port;
    AIM_BITMAP_CLR_ALL(bmap);

    for(port = 0; port < NUM_OF_ALL_PORT; port++) {
        AIM_BITMAP_SET(bmap, port);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    uint8_t present = 0;
    uint8_t present_bit = 0;
    UINT4 byte_get;

    /* Read QSFP MODULE is present or not */
    if(port < NUM_OF_QSFP_PORT){    //port: QSFP(0-31)
        byte_get = onlp_i2c_readb(I2C_BUS_1, SWPLD_1_ADDR, ag9032v2_sfp_get_present_reg(port), ONLP_I2C_F_TENBIT);
        if(byte_get < 0){
            AIM_LOG_ERROR("Error to present status from port(%d)\r\n", port);
            present = ONLP_STATUS_E_GENERIC;
            return present;
        }
        present_bit = byte_get & (1 << (7 - (port % 8)));
        present_bit = present_bit >> (7 - (port % 8));
    }
    else if((port > (NUM_OF_QSFP_PORT-1)) && (port < NUM_OF_ALL_PORT)){ //port: SFP(32-33)
        byte_get = onlp_i2c_readb(I2C_BUS_1, SWPLD_2_ADDR, SFP_SIGNAL_REG, ONLP_I2C_F_TENBIT);
        if(byte_get < 0){
            AIM_LOG_ERROR("Error to present status from port(%d)\r\n", port);
            present = ONLP_STATUS_E_GENERIC;
            return present;
        }
        if(port == NUM_OF_ALL_PORT-2){
            byte_get=byte_get >> 4;
            present_bit=((byte_get & 0x08) >> 3) & 1;
        }
        else if(port == NUM_OF_ALL_PORT-1){
            present_bit = (byte_get & 0x08) >> 3;
            present_bit = present_bit & 1;
        }
    }

    /* Read from sfp presence register value,
     * return 0 = The module is preset
     * return 1 = The module is NOT present
     */
    if(present_bit == 0) {
        present = 1;
    } else if (present_bit == 1) {
        present = 0;
        AIM_LOG_ERROR("Unble to present status from port(%d)\r\n", port);
    } else {
	/* Port range over 0-31 and 32-33, return -1 */
        AIM_LOG_ERROR("Error to present status from port(%d)\r\n", port);
        present = ONLP_STATUS_E_GENERIC;
    }
    return present;
}


int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    char present_all_data[12] = {'\0'};
    char *r_byte;
    char *r_array[4];
    int count = 0;
    int i     = 0;
    int j     = NUM_OF_QSFP_PORT - 1;        
    uint8_t bytes[4];
    uint8_t byte_get;
    uint8_t sfp1;
    uint8_t sfp2;
    uint32_t presence_all = 0 ;
     
    /* Read presence bitmap from SWPLD QSFP28 Presence Register 
     * if only port 0 is present, return 7F FF FF FF
     * if only port 0 and 1 present, return 3F FF FF FF
     */
    byte_get = onlp_i2c_readb(I2C_BUS_1, SWPLD_1_ADDR, SFP_PRESENT_1, ONLP_I2C_F_TENBIT);
    if(byte_get < 0)return ONLP_STATUS_E_GENERIC;
    bytes[0] = (~byte_get) & 0xFF;
    sprintf(present_all_data, "%x%c", byte_get, ' ');

    byte_get = onlp_i2c_readb(I2C_BUS_1, SWPLD_1_ADDR, SFP_PRESENT_2, ONLP_I2C_F_TENBIT);
    if(byte_get < 0)return ONLP_STATUS_E_GENERIC;
    bytes[1] = (~byte_get) & 0xFF;
    sprintf(present_all_data + 3, "%x%c", byte_get, ' ');

    byte_get = onlp_i2c_readb(I2C_BUS_1, SWPLD_1_ADDR, SFP_PRESENT_3, ONLP_I2C_F_TENBIT);
    if(byte_get < 0)return ONLP_STATUS_E_GENERIC;
    bytes[2] = (~byte_get) & 0xFF;
    sprintf(present_all_data + 6, "%x%c", byte_get, ' ');

    byte_get = onlp_i2c_readb(I2C_BUS_1, SWPLD_1_ADDR, SFP_PRESENT_4, ONLP_I2C_F_TENBIT);
    if(byte_get < 0)return ONLP_STATUS_E_GENERIC;
    bytes[3] = (~byte_get) & 0xFF;
    sprintf(present_all_data + 9, "%x%c", byte_get, '\0');

    /* String split */
    r_byte = strtok(present_all_data, " ");
    count = 0;
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
    for(i = AIM_ARRAYSIZE(bytes)-1; i >= 0; i--) {
        presence_all <<= 8;
        presence_all |= bytes[i];
    }
    /* Populate QSFP bitmap & remap*/
    for(i = 0; presence_all; i++)
    {
        if(23 < j)
            AIM_BITMAP_MOD(dst, j - 24,(presence_all & 1));
        else if(15 < j && j < 24)
            AIM_BITMAP_MOD(dst, j - 8,(presence_all & 1));
        else if(7 < j && j < 16)
            AIM_BITMAP_MOD(dst, j + 8,(presence_all & 1));
        else
            AIM_BITMAP_MOD(dst, j + 24,(presence_all & 1));
        presence_all >>= 1;
        j--;
    }

    /* Populate SFP bitmap */  
    byte_get = onlp_i2c_readb(I2C_BUS_1, SWPLD_2_ADDR, SFP_SIGNAL_REG, ONLP_I2C_F_TENBIT);
    if(byte_get < 0)return ONLP_STATUS_E_GENERIC;
    sfp2 = (byte_get & 0x08) >> 3;  //get sfp2 present bit
    byte_get = byte_get >> 4;
    sfp1 = (byte_get & 0x08) >> 3;  //get sfp1 present bit

    AIM_BITMAP_MOD(dst, 32, !(sfp1 & 1));    
    AIM_BITMAP_MOD(dst, 33, !(sfp2 & 1));

    return ONLP_STATUS_OK;
}


int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    uint8_t sfp_response_reg = 0x00;
    uint8_t sfp_mux_reg = 0x00;
    uint8_t backup_response_data = 0x00;
    uint8_t response_data = 0x00;

    /* Get port respond register offset */
    sfp_response_reg = ag9032v2_sfp_get_respond_reg(port);
    sfp_mux_reg = ag9032v2_sfp_get_mux_reg(port);

    /* Select qsfp port to response mode */
    backup_response_data = onlp_i2c_readb(I2C_BUS_1, SWPLD_1_ADDR, sfp_response_reg, ONLP_I2C_F_TENBIT);
    if(backup_response_data < 0)return ONLP_STATUS_E_GENERIC;
    response_data = ~(1 << (7 - (port % 8)));    
    onlp_i2c_write(I2C_BUS_1, SWPLD_1_ADDR, sfp_response_reg, 1, &response_data, ONLP_I2C_F_TENBIT);
    
    /* Select QSFP port */
    onlp_i2c_write(I2C_BUS_1, SWPLD_1_ADDR, SFP_I2C_MUX_REG, 1, &sfp_mux_reg, ONLP_I2C_F_TENBIT);
    memset(data, 0, 256);

    /* Read qsfp eeprom information into data[] */
    if (onlp_i2c_read(I2C_BUS_1, SFP_MODULE_EEPROM, 0, 256, data, ONLP_I2C_F_DISABLE_READ_RETRIES)) {
        AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    onlp_i2c_write(I2C_BUS_1, SWPLD_1_ADDR, sfp_response_reg, 1, &backup_response_data, ONLP_I2C_F_TENBIT);
    return ONLP_STATUS_OK;
}

int onlp_sfpi_port_map(int port, int* rport)
{
    *rport = port;
    return ONLP_STATUS_OK;
}


int
onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv)
{
    uint8_t sfp_mux_reg = 0x00;
    sfp_mux_reg = ag9032v2_sfp_get_mux_reg(port);
    /* Select QSFP port */

    onlp_i2c_write(I2C_BUS_1, SWPLD_1_ADDR, SFP_I2C_MUX_REG, 1, &sfp_mux_reg, ONLP_I2C_F_TENBIT);

    if(port < NUM_OF_QSFP_PORT){    //port: QSFP(0-31)
        switch (control) {
            case ONLP_SFP_CONTROL_RESET_STATE:
            case ONLP_SFP_CONTROL_LP_MODE:
                *rv = 1; 
                break;        
            case ONLP_SFP_CONTROL_RX_LOS:
            case ONLP_SFP_CONTROL_TX_DISABLE:
                *rv = 0; 
                break;
            default:
                break;
        }
    }
    else if(port >= (NUM_OF_ALL_PORT - NUM_OF_SFP_PORT) && port < NUM_OF_ALL_PORT ){    //port: SFP(32,33)
        switch (control) {
            case ONLP_SFP_CONTROL_RESET_STATE:
            case ONLP_SFP_CONTROL_LP_MODE:
                *rv = 0; 
                break;        
            case ONLP_SFP_CONTROL_RX_LOS:
            case ONLP_SFP_CONTROL_TX_DISABLE:
                *rv = 1; 
                break;
            default:
                break;
        }
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    uint8_t value_t      = 0;
    uint8_t value_backup = 0;
    uint8_t value_u8     =(uint8_t)value;
    uint8_t sfp_mux_reg  = 0x00;

    sfp_mux_reg = ag9032v2_sfp_get_mux_reg(port);

    /* Select QSFP port */
    onlp_i2c_write(I2C_BUS_1, SWPLD_1_ADDR, SFP_I2C_MUX_REG, 1, &sfp_mux_reg, ONLP_I2C_F_TENBIT);
    
    if(port < NUM_OF_QSFP_PORT){    //port: QSFP(0-31)
        switch (control) {
            case ONLP_SFP_CONTROL_RESET_STATE:
                onlp_i2c_write(I2C_BUS_1, SWPLD_1_ADDR, ag9032v2_sfp_get_reset_reg(port), 1, &value_u8, ONLP_I2C_F_TENBIT);
                value_t = ONLP_STATUS_OK;
                break;
            case ONLP_SFP_CONTROL_LP_MODE:
                onlp_i2c_write(I2C_BUS_1, SWPLD_1_ADDR, ag9032v2_sfp_get_lp_mode_reg(port), 1, &value_u8, ONLP_I2C_F_TENBIT);
                value_t = ONLP_STATUS_OK;
                break;
            case ONLP_SFP_CONTROL_RX_LOS:
            case ONLP_SFP_CONTROL_TX_DISABLE:
                value_t = ONLP_STATUS_E_INTERNAL;
                break;
            default:
                value_t = ONLP_STATUS_E_UNSUPPORTED;
                break;
        }
    }
    else if(port >= (NUM_OF_ALL_PORT - NUM_OF_SFP_PORT) && port < NUM_OF_ALL_PORT ){    //port: SFP(32,33)
        switch (control) {
            case ONLP_SFP_CONTROL_RESET_STATE:
            case ONLP_SFP_CONTROL_LP_MODE:
                value_t = ONLP_STATUS_E_INTERNAL;
                break;                
            case ONLP_SFP_CONTROL_RX_LOS:
                value_backup = onlp_i2c_readb(I2C_BUS_1, SWPLD_2_ADDR, SFP_SIGNAL_REG, ONLP_I2C_F_TENBIT);
                if(value_backup < 0)return ONLP_STATUS_E_GENERIC;
                if(port == (NUM_OF_ALL_PORT - NUM_OF_SFP_PORT) && value_backup >= 0){
                    value_u8 = (value_backup | (1 << 6)) & (value << 6);
                }else if(port == ( NUM_OF_ALL_PORT - 1) && value_backup >= 0){
                    value_u8 = (value_backup | (1 << 2)) & (value << 2);
                }
                else{
                    value_t = ONLP_STATUS_E_UNSUPPORTED;
                    break;
                }
                onlp_i2c_write(I2C_BUS_1, SWPLD_2_ADDR, SFP_SIGNAL_REG, 1, &value_u8, ONLP_I2C_F_TENBIT);
                value_t = ONLP_STATUS_OK;
                break;
            case ONLP_SFP_CONTROL_TX_DISABLE:
                value_backup = onlp_i2c_readb(I2C_BUS_1, SWPLD_2_ADDR, SFP_SIGNAL_REG, ONLP_I2C_F_TENBIT);
                if(value_backup < 0)return ONLP_STATUS_E_GENERIC;
                if(port == (NUM_OF_ALL_PORT - NUM_OF_SFP_PORT) && value_backup >= 0){
                    value_u8 = (value_backup | (1 << 5)) & (value << 5);
                }else if(port == (NUM_OF_ALL_PORT - 1) && value_backup >= 0){
                    value_u8 = (value_backup | (1 << 1)) & (value << 1);
                }
                else{
                    value_t = ONLP_STATUS_E_UNSUPPORTED;
                    break;
                }
                onlp_i2c_write(I2C_BUS_1, SWPLD_2_ADDR, SFP_SIGNAL_REG, 1, &value_u8, ONLP_I2C_F_TENBIT);
                value_t = ONLP_STATUS_OK;
                break;
            default:
                value_t = ONLP_STATUS_E_UNSUPPORTED;
                break;
        }
    }
    else{
        value_t = ONLP_STATUS_E_UNSUPPORTED;
    }
    return value_t;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    uint8_t value_t = 0;
    uint8_t sfp_mux_reg = 0x00;
    sfp_mux_reg = ag9032v2_sfp_get_mux_reg(port);
    /* Select QSFP port */
    onlp_i2c_write(I2C_BUS_1, SWPLD_1_ADDR, SFP_I2C_MUX_REG, 1, &sfp_mux_reg, ONLP_I2C_F_TENBIT);

    if(port < NUM_OF_QSFP_PORT){    //port: QSFP(0-31)
        switch (control) {
            case ONLP_SFP_CONTROL_RESET_STATE:
                *value = onlp_i2c_readb(I2C_BUS_1, SWPLD_1_ADDR, ag9032v2_sfp_get_reset_reg(port), ONLP_I2C_F_TENBIT);
                if(*value < 0){
                    value_t = ONLP_STATUS_E_UNSUPPORTED;
                    break;
                }
                *value = (1 << (7 - (*value % 8))) & 1 ;
                /* From sfp_reset value,
                 * return 0 = The module is in Reset
                 * return 1 = The module is NOT in Reset
                 */
                if (*value == 0)
                    *value = 1;         
                else if (*value == 1)
                    *value = 0;     
                    value_t = ONLP_STATUS_OK;
                break;
            case ONLP_SFP_CONTROL_LP_MODE:
                /* From sfp_lp_mode value,
                 * return 0 = The module is NOT in LP mode
                 * return 1 = The moduel is in LP mode   */
                *value = onlp_i2c_readb(I2C_BUS_1, SWPLD_1_ADDR, ag9032v2_sfp_get_lp_mode_reg(port), ONLP_I2C_F_TENBIT);
                if(*value < 0){
                    value_t = ONLP_STATUS_E_UNSUPPORTED;
                    break;
                }
                *value = (1 << (7 - (*value % 8))) & 1 ;
                value_t = ONLP_STATUS_OK;
                break;            
        case ONLP_SFP_CONTROL_RX_LOS:
        case ONLP_SFP_CONTROL_TX_DISABLE:
            *value = 0; 
            value_t = ONLP_STATUS_OK;
            break;  
        default:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
        }
    }
    else if(port >= (NUM_OF_ALL_PORT - NUM_OF_SFP_PORT) && port < NUM_OF_ALL_PORT ){    //port: SFP(32,33)
        switch (control) {
        case ONLP_SFP_CONTROL_RESET_STATE:
        case ONLP_SFP_CONTROL_LP_MODE:
            value_t = ONLP_STATUS_OK;
            break;           
        case ONLP_SFP_CONTROL_RX_LOS:
            value_t = onlp_i2c_readb(I2C_BUS_1, SWPLD_2_ADDR, SFP_SIGNAL_REG, ONLP_I2C_F_TENBIT);
            if(port == (NUM_OF_ALL_PORT - NUM_OF_SFP_PORT) && value_t >= 0){
                *value = (value_t >> 6) & 0x01;    
            }
            else if(port == (NUM_OF_ALL_PORT - 1) && value_t >= 0){
                *value = (value_t >> 2) & 0x01;
            }
            else{
                value_t = ONLP_STATUS_E_UNSUPPORTED;
                break;
            }
            value_t = ONLP_STATUS_OK;
            break;
        case ONLP_SFP_CONTROL_TX_DISABLE:
            value_t = onlp_i2c_readb(I2C_BUS_1, SWPLD_2_ADDR, SFP_SIGNAL_REG, ONLP_I2C_F_TENBIT);
            if(port == (NUM_OF_ALL_PORT - NUM_OF_SFP_PORT) && value_t >= 0){
                *value = (value_t >> 5) & 0x01;
            }
            else if(port == (NUM_OF_ALL_PORT - 1) && value_t >= 0){
                *value = (value_t >> 1) & 0x01;
            }
            else{
                value_t = ONLP_STATUS_E_UNSUPPORTED;
                break;
            }
            value_t = ONLP_STATUS_OK;
            break;
        default:
            value_t = ONLP_STATUS_E_UNSUPPORTED;
            break;
        }
    }
    else{
        value_t = ONLP_STATUS_E_UNSUPPORTED;
    }
    return value_t;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
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
