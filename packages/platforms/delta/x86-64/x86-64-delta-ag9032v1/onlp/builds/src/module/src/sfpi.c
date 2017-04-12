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

#include <fcntl.h> /* For O_RDWR && open */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <math.h>
#include <onlplib/i2c.h>

#include "platform_lib.h"

static inline int ag9032v1_sfp_get_lp_mode_reg(int port) {
    uint8_t reg_offset = 0x00;
    if (port < 8)			/* port 0-7 */
        reg_offset = SFP_LP_MODE_1;	
    else if (port > 7 && port < 16)	/* port 8-15 */
        reg_offset = SFP_LP_MODE_2;
    else if (port > 15 && port < 24)	/* port 16-23 */
        reg_offset = SFP_LP_MODE_3;
    else if (port > 23 && port < 32)	/* port 24-31 */
        reg_offset = SFP_LP_MODE_4;

    return reg_offset;
}

static inline int ag9032v1_sfp_get_reset_reg(int port) {
    uint8_t reg_offset = 0x00;
    if (port < 8)			/* port 0-7 */
        reg_offset = SFP_RESET_1;	
    else if (port > 7 && port < 16)	/* port 8-15 */
        reg_offset = SFP_RESET_2;
    else if (port > 15 && port < 24)	/* port 16-23 */
        reg_offset = SFP_RESET_3;
    else if (port > 23 && port < 32)	/* port 24-31 */
        reg_offset = SFP_RESET_4;

    return reg_offset;
}

static inline int ag9032v1_sfp_get_respond_reg(int port) {
    uint8_t reg_offset = 0x00;
    if (port < 8)			/* port 0-7 */
        reg_offset = SFP_RESPOND_1;	
    else if (port > 7 && port < 16)	/* port 8-15 */
        reg_offset = SFP_RESPOND_2;
    else if (port > 15 && port < 24)	/* port 16-23 */
        reg_offset = SFP_RESPOND_3;
    else if (port > 23 && port < 32)	/* port 24-31 */
        reg_offset = SFP_RESPOND_4;

    return reg_offset;
}

static inline int ag9032v1_sfp_get_mux_reg(int port) {
    uint8_t sel_channel = 0x00;
    if (port >= 0 && port < 9)
        sel_channel = port + 1;          /* 0x01 - 0x09 */
    else if (port >= 9 && port < 19)
        sel_channel = port + 7;          /* 0x10 - 0x19 */
    else if (port >= 19 && port < 29)
        sel_channel = port + 13;         /* 0x20 - 0x29 */
    else if (port >= 29 && port < 32)
        sel_channel = port + 19;         /* 0x30 - 0x32 */
    else
        AIM_LOG_ERROR("qsfp port range is 0-31");

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
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = 0; p < 32; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    char port_data[2] = {'\0'};
    uint8_t present = 0;
    uint8_t present_bit = 0;

    /* Select QSFP port */
    sprintf(port_data, "%d", port + 1);
    dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH);

    /* Read QSFP MODULE is present or not */
    present_bit = dni_i2c_lock_read_attribute(NULL, SFP_IS_PRESENT_PATH);

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
	/* Port range over 0-31, return -1 */
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
    int count = 0;

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

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    uint8_t sfp_response_reg = 0x00;
    uint8_t backup_response_data = 0x00;
    char port_data[2] = {'\0'};

    /* Get port respond register offset */
    sfp_response_reg = ag9032v1_sfp_get_respond_reg(port);

    /* Select qsfp port to response mode */
    backup_response_data = dni_lock_swpld_read_attribute(sfp_response_reg);
    backup_response_data &= ~(1 << (7 - (port % 8)));
    dni_lock_swpld_write_attribute(sfp_response_reg, backup_response_data);

    /* Select QSFP port */
    sprintf(port_data, "%d", port + 1);
    dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH);

    memset(data, 0, 256);

    /* Read qsfp eeprom information into data[] */
    if (dni_i2c_read_attribute_binary(SFP_EEPROM_PATH, 
						(char *)data, 256, 256) != 0) {
	AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);
	return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int onlp_sfpi_port_map(int port, int* rport)
{
    *rport = port;
    return ONLP_STATUS_OK;
}

int onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    uint8_t sfp_response_reg = 0x00;
    uint8_t port_sel_channel = 0x00;
    uint8_t backup_response_data = 0x00;
    uint8_t channel = 0x00;
    mux_info_t mux_info;
    dev_info_t dev_info;

   /* int port : be used in SWPLD qsfp module respond register offset,
    *		input value port range is 0-31.
    * uint8_t port_sel_channel : be used in SWPLD qsfp i2c mux register offset
    */

    /* Get port respond register offset */
    sfp_response_reg = ag9032v1_sfp_get_respond_reg(port);

    /* Get port mux register channel */
    port_sel_channel = ag9032v1_sfp_get_mux_reg(port);

    /* Select qsfp port to response mode */
    backup_response_data = dni_lock_swpld_read_attribute(sfp_response_reg);
    backup_response_data &= ~(1 << (7 - (port % 8)));
    dni_lock_swpld_write_attribute(sfp_response_reg, backup_response_data);

    channel = port_sel_channel;

    mux_info.offset = SFP_I2C_MUX_REG;
    mux_info.channel = channel;
    mux_info.flags = ONLP_I2C_F_FORCE;

    dev_info.bus = I2C_BUS_5;
    dev_info.addr = devaddr;
    dev_info.offset = addr;
    dev_info.flags = ONLP_I2C_F_FORCE;

    return dni_i2c_lock_read(&mux_info, &dev_info);
}

int onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    uint8_t sfp_response_reg = 0x00;
    uint8_t port_sel_channel = 0x00;
    uint8_t backup_response_data = 0x00;
    uint8_t channel = 0x00;
    mux_info_t mux_info;
    dev_info_t dev_info;

   /* int port : be used in SWPLD qsfp module respond register offset,
    *		input value port range is 0-31.
    * uint8_t port_sel_channel : be used in SWPLD qsfp i2c mux register offset
    */

    /* Get port respond register offset */
    sfp_response_reg = ag9032v1_sfp_get_respond_reg(port);

    /* Get port mux register channel */
    port_sel_channel = ag9032v1_sfp_get_mux_reg(port);

    /* Select qsfp port to response mode */
    backup_response_data = dni_lock_swpld_read_attribute(sfp_response_reg);
    backup_response_data &= ~(1 << (7 - (port % 8)));
    dni_lock_swpld_write_attribute(sfp_response_reg, backup_response_data);

    channel = port_sel_channel;

    mux_info.offset = SFP_I2C_MUX_REG;
    mux_info.channel = channel;
    mux_info.flags = ONLP_I2C_F_FORCE;

    dev_info.bus = I2C_BUS_5;
    dev_info.addr = devaddr;
    dev_info.offset = addr;
    dev_info.flags = ONLP_I2C_F_FORCE;
    dev_info.size = 1;	/* Write 1 byte */
    dev_info.data_8 = value;

    return dni_i2c_lock_write(&mux_info, &dev_info);
}

int onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    uint8_t sfp_response_reg = 0x00;
    uint8_t port_sel_channel = 0x00;
    uint8_t backup_response_data = 0x00;
    uint8_t channel = 0x00;
    mux_info_t mux_info;
    dev_info_t dev_info;

   /* int port : be used in SWPLD qsfp module respond register offset,
    *		input value port range is 0-31.
    * uint8_t port_sel_channel : be used in SWPLD qsfp i2c mux register offset
    */

    /* Get port respond register offset */
    sfp_response_reg = ag9032v1_sfp_get_respond_reg(port);

    /* Get port mux register channel */
    port_sel_channel = ag9032v1_sfp_get_mux_reg(port);

    /* Select qsfp port to response mode */
    backup_response_data = dni_lock_swpld_read_attribute(sfp_response_reg);
    backup_response_data &= ~(1 << (7 - (port % 8)));
    dni_lock_swpld_write_attribute(sfp_response_reg, backup_response_data);

    channel = port_sel_channel;

    mux_info.offset = SFP_I2C_MUX_REG;
    mux_info.channel = channel;
    mux_info.flags = ONLP_I2C_F_FORCE;

    dev_info.bus = I2C_BUS_5;
    dev_info.addr = devaddr;
    dev_info.offset = addr;
    dev_info.flags = ONLP_I2C_F_FORCE;
    dev_info.size = 2;	/* Read two bytes */

    return dni_i2c_lock_read(&mux_info, &dev_info);
}

int onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    uint8_t sfp_response_reg = 0x00;
    uint8_t port_sel_channel = 0x00;
    uint8_t backup_response_data = 0x00;
    uint8_t channel = 0x00;
    mux_info_t mux_info;
    dev_info_t dev_info;

   /* int port : be used in SWPLD qsfp module respond register offset,
    *		input value port range is 0-31.
    * uint8_t port_sel_channel : be used in SWPLD qsfp i2c mux register offset
    */

    /* Get port respond register offset */
    sfp_response_reg = ag9032v1_sfp_get_respond_reg(port);

    /* Get port mux register channel */
    port_sel_channel = ag9032v1_sfp_get_mux_reg(port);

    /* Select qsfp port to response mode */
    backup_response_data = dni_lock_swpld_read_attribute(sfp_response_reg);
    backup_response_data &= ~(1 << (7 - (port % 8)));
    dni_lock_swpld_write_attribute(sfp_response_reg, backup_response_data);

    channel = port_sel_channel;

    mux_info.offset = SFP_I2C_MUX_REG;
    mux_info.channel = channel;
    mux_info.flags = ONLP_I2C_F_FORCE;

    dev_info.bus = I2C_BUS_5;
    dev_info.addr = devaddr;
    dev_info.offset = addr;
    dev_info.flags = ONLP_I2C_F_FORCE;
    dev_info.size = 2;	/* Write two bytes */
    dev_info.data_16 = value;

    return dni_i2c_lock_write(&mux_info, &dev_info);
}

int
onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv)
{
    char port_data[2] = {'\0'};

    /* Select QSFP port */
    sprintf(port_data, "%d", port + 1);
    dni_i2c_lock_write_attribute(NULL, port_data, SFP_SELECT_PORT_PATH);

    switch (control) {
    case ONLP_SFP_CONTROL_RESET_STATE:
	*rv = 1;
	break;
    case ONLP_SFP_CONTROL_RX_LOS:
	*rv = 0; 
	break;
    case ONLP_SFP_CONTROL_TX_DISABLE:
	*rv = 0;
	break;
    case ONLP_SFP_CONTROL_LP_MODE:
	*rv = 1; 
	break;
    default:
	return ONLP_STATUS_OK;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    uint8_t value_t = 0;
    char port_data[2] = {'\0'};

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
	value_t = ONLP_STATUS_OK;
	break;
    case ONLP_SFP_CONTROL_TX_DISABLE:
	value_t = ONLP_STATUS_OK;
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
    uint8_t value_t = 0;
    char port_data[2] = {'\0'};

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
	if (*value == 0)
	    *value = 1;			
	else if (*value == 1)
	    *value = 0;		
	
	value_t = ONLP_STATUS_OK;
	break;
    case ONLP_SFP_CONTROL_RX_LOS:
	*value = 0;
	value_t = ONLP_STATUS_OK;
	break;
    case ONLP_SFP_CONTROL_TX_DISABLE:
	*value = 0;
	value_t = ONLP_STATUS_OK;
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
