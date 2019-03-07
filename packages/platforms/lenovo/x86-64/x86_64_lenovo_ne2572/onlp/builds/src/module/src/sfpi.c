/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2018 Alpha Networks Incorporation.
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <onlplib/i2c.h>
#include "platform_lib.h"

#define DEBUG                           0

#define SYSTEM_CPLD_I2C_BUS_ID          8
#define SYSTEM_CPLD_I2C_ADDR            0x5F  /* System CPLD Physical Address in the I2C */
#define SYSTEM_CPLD_RESET1_ADDR_OFFSET  0x02

#define PORT_CPLD0_I2C_BUS_ID           13
#define PORT_CPLD1_I2C_BUS_ID           15
#define PORT_CPLD2_I2C_BUS_ID           17

//#define PORT_CPLD_I2C_ADDR              0x5F  /* Port CPLD Physical Address in the I2C */
#define PORT_CPLD0_I2C_ADDR              0x5B
#define PORT_CPLD1_I2C_ADDR              0x5C
#define PORT_CPLD2_I2C_ADDR              0x5D

#define PORT_CPLD2_MODULE_SELECT_ADDR_OFFSET  0x0B


#define PORT_CPLD_PRESENT_ADDR_OFFSET   0x05
#define PORT_CPLD_RESET_ADDR_OFFSET     0x07
#define PORT_CPLD_LOWPWR_ADDR_OFFSET    0x09
#define PORT_CPLD_MOD_SEL_ADDR_OFFSET   0x0B  /* Module Select of QSFP ports */

#define PORT_CPLD_SFP_PRESENT_ADDR_OFFSET    0x16
#define PORT_CPLD_SFP_RX_LOS_ADDR_OFFSET     0x13
#define PORT_CPLD_SFP_TX_FAULT_ADDR_OFFSET   0x10
#define PORT_CPLD_SFP_TX_DISABLE_ADDR_OFFSET 0x19

#define SFP_I2C_BUS_ID_BASE             21


static int
port_to_busid(int port)
{
    int index = 0;
    index = port + SFP_I2C_BUS_ID_BASE;

    DIAG_PRINT("%s, port:%d, busid:%d ", __FUNCTION__, port, index);
    return index;
}


static int
port_to_cpld_busid(int port)
{
    int index = 0;
    int ret = 0;
    index = port / 24;
    ret = (index * 2) + PORT_CPLD0_I2C_BUS_ID;
    DIAG_PRINT("%s, port:%d, cpld_busid:%d ", __FUNCTION__, port, ret);
    return ret;
}

static int
port_to_cpld_addr(int port)
{
    int index = 0;
    int ret = 0;
    index = port / 24;
    ret = index + PORT_CPLD0_I2C_ADDR;
    DIAG_PRINT("%s, port:%d, cpld_busaddr:%d ", __FUNCTION__, port, ret);
    return ret;
}

static int
port_to_cpld_sfp_offset(int port, int start_index)
{
    int index = 0;
    int ret = 0;

    if (port < QSFP_START_INDEX)
    {

        index = (port / 8) % 3;
        ret = index + start_index;
    }
    else
    {
        AIM_LOG_INFO("%s:%d invaild sfp port index(%d), start_index:%d\n", __FUNCTION__, __LINE__, port, start_index);
    }

    DIAG_PRINT("%s, port:%d, present_offset:0x%X, start_index:%d, index:%d", __FUNCTION__, port, ret, start_index, index);
    return ret;
}

static int
port_to_cpld_present_offset(int port)
{
    int ret = 0;

    if (port < QSFP_START_INDEX)
    {
        ret = port_to_cpld_sfp_offset(port, PORT_CPLD_SFP_PRESENT_ADDR_OFFSET);
    }
    else
    {
        ret = PORT_CPLD_PRESENT_ADDR_OFFSET; // QSFP28 Transceiver Present Register
    }

    DIAG_PRINT("%s, port:%d, present_offset:%d ", __FUNCTION__, port, ret);
    return ret;
}

static int
port_to_cpld_port_bit(int port)
{
    int index = 0;
    index = (port % 8);
    DIAG_PRINT("%s, port:%d, present_bit:%d ", __FUNCTION__, port, index);
    return index;
}

static int
_sfp_present(int port)
{
    int ret = 0;
    char data = 0;

    DIAG_PRINT("%s, port:%d", __FUNCTION__, port);

    int busid = port_to_cpld_busid(port);
    int addr = port_to_cpld_addr(port);
    int offset = port_to_cpld_present_offset(port);

    ret = i2c_read_byte(busid, addr, offset, &data);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    if (DEBUG)
        AIM_LOG_INFO("%s:%d port[%d],busid[%d],addr[0x%2X],offset[0x%02X],read_byte[%02x]\n", __FUNCTION__, __LINE__,
                     port, busid, addr, offset, (unsigned char)data);

    if (data & (1 << port_to_cpld_port_bit(port)))
    {
        return 1;
    }

    return 0;
}

/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
int
onlp_sfpi_init(void)
{
    DIAG_PRINT("%s", __FUNCTION__);

    int ret = 0;
    char data = 0;

    /* pull low for Reset Register 1 of Port CPLD */
    ret = i2c_write_byte(SYSTEM_CPLD_I2C_BUS_ID, SYSTEM_CPLD_I2C_ADDR, SYSTEM_CPLD_RESET1_ADDR_OFFSET, data);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    //Module select
    ret = i2c_write_byte(PORT_CPLD2_I2C_BUS_ID, PORT_CPLD2_I2C_ADDR, PORT_CPLD2_MODULE_SELECT_ADDR_OFFSET, 0x3F); //bit 0~5
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t *bmap)
{
    int p = 0;
    AIM_BITMAP_CLR_ALL(bmap);

    for (p = 0; p < NUM_OF_SFP_PORT; p++)
    {
        AIM_BITMAP_SET(bmap, p);
    }
    DIAG_PRINT("%s", __FUNCTION__);

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
    int present = 0;

    present = _sfp_present(port);

    DIAG_PRINT("%s, port=%d, present:%d", __FUNCTION__, port, present);
    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t *dst)
{
    DIAG_PRINT("%s", __FUNCTION__);

    int i = 0;
    AIM_BITMAP_CLR_ALL(dst);
    for (i = 0; i < NUM_OF_SFP_PORT; i++)
    {
        if (onlp_sfpi_is_present(i))
        {
            AIM_BITMAP_SET(dst, i);
        }
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{

    int busid = port_to_busid(port);

    DIAG_PRINT("%s, port:%d, busid:%d", __FUNCTION__, port, busid);

    /*
     * Read the SFP eeprom into data[]
     */
    memset(data, 0x0, 256);

    if (i2c_read(busid, QSFP28_EEPROM_I2C_ADDR, 0x0, 256, (char *)data) != 0)
    {
        AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    int busid = port_to_busid(port);
    int ret = 0;
    memset(data, 0x0, 256);
    DIAG_PRINT("%s, port:%d, busid:%d", __FUNCTION__, port, busid);

    if (port < QSFP_START_INDEX)
    {
        if (i2c_read(busid, SFP_DOM_EEPROM_I2C_ADDR, 0x0, 256, (char *)data) != 0)
        {
            AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    else
    {
        //Set page select to pahe 00h.
        ret = onlp_sfpi_dev_writeb(port, QSFP28_EEPROM_I2C_ADDR, QSFP28_EEPROM_PAGE_SELECT_OFFSET, 0);
        if (ret < 0)
        {
            AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
            return ret;
        }
        if (i2c_read(busid, QSFP28_EEPROM_I2C_ADDR, 0x0, 256, (char *)data) != 0)
        {
            AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{

    int ret = 0;
    int bus = port_to_busid(port);

    ret = onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
    DIAG_PRINT("%s, port:%d, devaddr:%d, addr:%d, ret:%d(0x%02X)", __FUNCTION__, port, devaddr, addr, ret, ret);

    return ret;
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{

    int ret = 0;
    int bus = port_to_busid(port);

    ret = onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
    DIAG_PRINT("%s, port:%d, devaddr:%d, addr:%d, value:%d(0x%02X), ret:%d", __FUNCTION__, port, devaddr, addr, value, value, ret);

    return ret;
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int ret = 0;
    int bus = port_to_busid(port);

    ret = onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
    DIAG_PRINT("%s, port:%d, devaddr:%d, addr:%d, ret:%d(0x%04X)", __FUNCTION__, port, devaddr, addr, ret, ret);

    return ret;
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int ret = 0;
    int bus = port_to_busid(port);

    ret = onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
    DIAG_PRINT("%s, port:%d, devaddr:%d, addr:%d, value:%d(0x%04X), ret:%d", __FUNCTION__, port, devaddr, addr, value, value, ret);

    return ret;
}

/* 
  Reset and LP mode can control by CPLD so the setting will be keep in CPLD.
  For other options, control is get/set to QSFP28.
  Control options set to QSFP28 will be lost when the QSFP28 is removed.
  Upper layer software system should keep the configuration and set it again when detect a new sfp module insert. 
    [QSFP]
     function                            R/W  CPLD           EEPROM
    ------------------------------------ ---  -------------  -----------------
    ONLP_SFP_CONTROL_RESET                W   0x7
    ONLP_SFP_CONTROL_RESET_STATE         R/W  0x7  
    ONLP_SFP_CONTROL_RX_LOS               R   none           byte 4
    ONLP_SFP_CONTROL_TX_FAULT             R   none           byte 3
    ONLP_SFP_CONTROL_TX_DISABLE          R/W  none           byte 86
    ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL  R/W  none           byte 86
    ONLP_SFP_CONTROL_LP_MODE             R/W  0x9          
    ONLP_SFP_CONTROL_POWER_OVERRIDE      R/W  none           byte 93
    [SFP]
     function                            R/W  CPLD       
    ------------------------------------ ---  ---------------  
    ONLP_SFP_CONTROL_RESET               Not Support(There is no RESET pin in SFP module)
    ONLP_SFP_CONTROL_RESET_STATE         Not Support(There is no RESET pin in SFP module)
    ONLP_SFP_CONTROL_RX_LOS               R   0x13/0x14/0x15
    ONLP_SFP_CONTROL_TX_FAULT             R   0x10/0x11/0x12
    ONLP_SFP_CONTROL_TX_DISABLE          R/W  0x19/0x1A/0x1B
    ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL  Not Support(It is for QSFP)
    ONLP_SFP_CONTROL_LP_MODE             Not Support(It is for QSFP)
    ONLP_SFP_CONTROL_POWER_OVERRIDE      Not Support(It is for QSFP)

*/

int onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int *supported)
{
    if (supported == NULL)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_PARAM);
        return ONLP_STATUS_E_PARAM;
    }

    *supported = 0;



    switch (control)
    {
        case ONLP_SFP_CONTROL_RX_LOS:
        case ONLP_SFP_CONTROL_TX_FAULT:
        case ONLP_SFP_CONTROL_TX_DISABLE:
            *supported = 1;
            break;

        case ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL:
        case ONLP_SFP_CONTROL_RESET:
        case ONLP_SFP_CONTROL_RESET_STATE:
        case ONLP_SFP_CONTROL_LP_MODE:
        case ONLP_SFP_CONTROL_POWER_OVERRIDE:
            if (port < QSFP_START_INDEX) //SFP port, those options are designed for QSFP.
                *supported = 0;
            else
                *supported = 1;
            break;
        default:
            *supported = 0;
            break;
    }

    DIAG_PRINT("%s, port:%d, control:%d(%s), supported:%d", __FUNCTION__, port, control, sfp_control_to_str(control), *supported);
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv = ONLP_STATUS_OK;
    int bus = port_to_busid(port);
    int cpld_bus = port_to_cpld_busid(port);
    int addr = port_to_cpld_addr(port);
    int offset = 0;
    int port_bit = port_to_cpld_port_bit(port);
    int supported = 0;
    char optval = 0;

    if ((onlp_sfpi_control_supported(port, control, &supported) == ONLP_STATUS_OK) && (supported == 0))
        return ONLP_STATUS_E_UNSUPPORTED;

    DIAG_PRINT("%s, port:%d, control:%d(%s), value:0x%X", __FUNCTION__, port, control, sfp_control_to_str(control), value);

    if (port < QSFP_START_INDEX) //SFP
    {
        switch (control)
        {
            case ONLP_SFP_CONTROL_TX_DISABLE:
                offset = port_to_cpld_sfp_offset(port, PORT_CPLD_SFP_TX_DISABLE_ADDR_OFFSET);
                break;
            default:
                return ONLP_STATUS_E_UNSUPPORTED;
                break;
        }
        optval = onlp_i2c_readb(cpld_bus, addr, offset, ONLP_I2C_F_FORCE);
        if (value != 0)
        {
            optval |= (1 << port_bit);
        }
        else
        {
            optval &= !(1 << port_bit);
        }
        rv = onlp_i2c_writeb(cpld_bus, addr, offset, optval, ONLP_I2C_F_FORCE);
        if (rv < 0)
        {
            AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, rv);
            return rv;
        }

    }
    else
    {

        switch (control)
        {
            case ONLP_SFP_CONTROL_RESET:
                if (value == 0) //set ONLP_SFP_CONTROL_RESET_STATE to 0
                {
                    rv = onlp_sfpi_control_set(port, ONLP_SFP_CONTROL_RESET_STATE, 0);
                    break;
                }

                rv = onlp_sfpi_control_set(port, ONLP_SFP_CONTROL_RESET_STATE, 1);
                if (rv < 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, rv);
                    return rv;
                }
                rv = onlp_sfpi_control_set(port, ONLP_SFP_CONTROL_RESET_STATE, 0);
                if (rv < 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, rv);
                    return rv;
                }

                break;

            case ONLP_SFP_CONTROL_RESET_STATE:
                offset = PORT_CPLD_RESET_ADDR_OFFSET;
                optval = onlp_i2c_readb(cpld_bus, addr, offset, ONLP_I2C_F_FORCE);
                if (value != 0)
                {
                    optval |= (1 << port_bit);
                }
                else
                {
                    optval &= !(1 << port_bit);
                }
                
                rv = onlp_i2c_writeb(cpld_bus, addr, offset, optval, ONLP_I2C_F_FORCE);
                if (rv < 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, rv);
                    return rv;
                }
                break;

            case ONLP_SFP_CONTROL_TX_DISABLE:
                if (onlp_sfpi_is_present(port) == 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_MISSING);
                    return ONLP_STATUS_E_MISSING;
                }
                offset = QSFP28_EEPROM_TX_DISABLE_OFFSET;
                addr = QSFP28_EEPROM_I2C_ADDR;
                optval = onlp_i2c_readb(bus, addr, offset, ONLP_I2C_F_FORCE);
                if (value != 0)
                {
                    optval |= 0x0f; //bit 0~3
                }
                else
                {
                    optval &= !(0x0f);
                }
                rv = onlp_i2c_writeb(bus, addr, offset, optval, ONLP_I2C_F_FORCE);
                if (rv < 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, rv);
                    return rv;
                }
                break;

            case ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL:
                if (onlp_sfpi_is_present(port) == 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_MISSING);
                    return ONLP_STATUS_E_MISSING;
                }
                if (value < 0 || value > 0x0f)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_PARAM);
                    return ONLP_STATUS_E_PARAM;
                }
                offset = QSFP28_EEPROM_TX_DISABLE_OFFSET;
                addr = QSFP28_EEPROM_I2C_ADDR;
                optval = value;
                rv = onlp_i2c_writeb(bus, addr, offset, optval, ONLP_I2C_F_FORCE);
                if (rv < 0)
                    return rv;
                break;

            case ONLP_SFP_CONTROL_POWER_OVERRIDE:
                if (onlp_sfpi_is_present(port) == 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_MISSING);
                    return ONLP_STATUS_E_MISSING;
                }
                offset = QSFP28_EEPROM_POWERSET_OFFSET;
                addr = QSFP28_EEPROM_I2C_ADDR;
                optval = onlp_i2c_readb(bus, addr, offset, ONLP_I2C_F_FORCE);
                if (value != 0)
                {
                    optval |= 0x01;
                }
                else
                {
                    optval &= !(0x01);
                }
                rv = onlp_i2c_writeb(bus, addr, offset, optval, ONLP_I2C_F_FORCE);
                if (rv < 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, rv);
                    return rv;
                }
                break;

            case ONLP_SFP_CONTROL_LP_MODE:
                offset = PORT_CPLD_LOWPWR_ADDR_OFFSET;
                optval = onlp_i2c_readb(cpld_bus, addr, offset, ONLP_I2C_F_FORCE);
                if (value != 0)
                {
                    optval |= (1 << port_bit);
                }
                else
                {
                    optval &= !(1 << port_bit);
                }
                rv = onlp_i2c_writeb(cpld_bus, addr, offset, optval, ONLP_I2C_F_FORCE);
                if (rv < 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, rv);
                    return rv;
                }
                break;

                //Read Only
            case ONLP_SFP_CONTROL_RX_LOS:
            case ONLP_SFP_CONTROL_TX_FAULT:
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_INVALID);
                return ONLP_STATUS_E_INVALID;
                break;
            default:
                break;
        }
    }
    DIAG_PRINT("%s, port_bit:%d, bus:%d, cpld_bus:%d, addr:0x%X, offset:0x%X", __FUNCTION__, port_bit, bus, cpld_bus, addr, offset);
    return rv;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int *value)
{
    int rv = ONLP_STATUS_OK;
    int bus = port_to_busid(port);
    int cpld_bus = port_to_cpld_busid(port);
    int addr = port_to_cpld_addr(port);
    int offset = 0;
    int port_bit = port_to_cpld_port_bit(port);
    int supported = 0;
    char optval = 0;

    if (value == NULL)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_PARAM);
        return ONLP_STATUS_E_PARAM;
    }

    if ((onlp_sfpi_control_supported(port, control, &supported) == ONLP_STATUS_OK) && (supported == 0))
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_UNSUPPORTED);
        return ONLP_STATUS_E_UNSUPPORTED;
    }
    *value = 0;


    if (port < QSFP_START_INDEX) //SFP
    {
        switch (control)
        {
            case ONLP_SFP_CONTROL_RX_LOS:
                offset = port_to_cpld_sfp_offset(port, PORT_CPLD_SFP_RX_LOS_ADDR_OFFSET);
                break;
            case ONLP_SFP_CONTROL_TX_FAULT:
                offset = port_to_cpld_sfp_offset(port, PORT_CPLD_SFP_TX_FAULT_ADDR_OFFSET);
                break;
            case ONLP_SFP_CONTROL_TX_DISABLE:
                offset = port_to_cpld_sfp_offset(port, PORT_CPLD_SFP_TX_DISABLE_ADDR_OFFSET);
                break;
            default:
                return ONLP_STATUS_E_UNSUPPORTED;
                break;
        }
        optval = onlp_i2c_readb(cpld_bus, addr, offset, ONLP_I2C_F_FORCE);
        if ((optval & (1 << port_bit)) != 0) //1
        {
            *value = 1;
        }
        else
        {
            *value = 0;
        }
    }
    else //QSFP
    {
        switch (control)
        {
            case ONLP_SFP_CONTROL_RESET_STATE:
                offset = PORT_CPLD_RESET_ADDR_OFFSET;
                optval = onlp_i2c_readb(cpld_bus, addr, offset, ONLP_I2C_F_FORCE);
                if ((optval & (1 << port_bit)) != 0) //1
                {
                    *value = 1;
                }
                else
                {
                    *value = 0;
                }
                break;

            case ONLP_SFP_CONTROL_RX_LOS:
                if (onlp_sfpi_is_present(port) == 0)
                {
#if 1 //for display RX_LOS Bitmap in onlpdump
                    *value = 0;
                    break;
#else
                    return ONLP_STATUS_E_MISSING;

#endif
                }
                offset = QSFP28_EEPROM_TXRX_LOS_OFFSET;
                addr = QSFP28_EEPROM_I2C_ADDR;
                optval = onlp_i2c_readb(bus, addr, offset, ONLP_I2C_F_FORCE);
                if ((optval & (0x0f)) != 0) //bit 0~3
                {
                    *value = 1;
                }
                else
                {
                    *value = 0;
                }
                break;

            case ONLP_SFP_CONTROL_TX_FAULT:
                if (onlp_sfpi_is_present(port) == 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_MISSING);
                    return ONLP_STATUS_E_MISSING;
                }
                offset = QSFP28_EEPROM_TX_FAULT_OFFSET;
                addr = QSFP28_EEPROM_I2C_ADDR;
                optval = onlp_i2c_readb(bus, addr, offset, ONLP_I2C_F_FORCE);
                if ((optval & (0x0f)) != 0) //bit 0~3
                {
                    *value = 1;
                }
                else
                {
                    *value = 0;
                }
                break;

            case ONLP_SFP_CONTROL_TX_DISABLE:
                if (onlp_sfpi_is_present(port) == 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_MISSING);
                    return ONLP_STATUS_E_MISSING;
                }
                offset = QSFP28_EEPROM_TX_DISABLE_OFFSET;
                addr = QSFP28_EEPROM_I2C_ADDR;
                optval = onlp_i2c_readb(bus, addr, offset, ONLP_I2C_F_FORCE);
                if ((optval & (0x0f)) != 0) //bit 0~3
                {
                    *value = 1;
                }
                else
                {
                    *value = 0;
                }
                break;

            case ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL:
                if (onlp_sfpi_is_present(port) == 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_MISSING);
                    return ONLP_STATUS_E_MISSING;
                }
                offset = QSFP28_EEPROM_TX_DISABLE_OFFSET;
                addr = QSFP28_EEPROM_I2C_ADDR;
                optval = onlp_i2c_readb(bus, addr, offset, ONLP_I2C_F_FORCE);
                *value = optval & (0x0f); //bit 0~3
                break;

            case ONLP_SFP_CONTROL_LP_MODE:
                offset = PORT_CPLD_LOWPWR_ADDR_OFFSET;
                optval = onlp_i2c_readb(cpld_bus, addr, offset, ONLP_I2C_F_FORCE);
                if ((optval & (1 << port_bit)) != 0)
                {
                    *value = 1;
                }
                else
                {
                    *value = 0;
                }
                break;

            case ONLP_SFP_CONTROL_POWER_OVERRIDE:
                if (onlp_sfpi_is_present(port) == 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_MISSING);
                    return ONLP_STATUS_E_MISSING;
                }
                offset = QSFP28_EEPROM_POWERSET_OFFSET;
                addr = QSFP28_EEPROM_I2C_ADDR;
                optval = onlp_i2c_readb(bus, addr, offset, ONLP_I2C_F_FORCE);
                if ((optval & (0x01)) != 0) //bit 0
                {
                    *value = 1;
                }
                else
                {
                    *value = 0;
                }
                break;

                //Set Only
            case ONLP_SFP_CONTROL_RESET:
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_INVALID);
                return ONLP_STATUS_E_INVALID;
            }
            default:
                return ONLP_STATUS_E_UNSUPPORTED;
                break;
        }
    }
    DIAG_PRINT("%s, port_bit:%d, bus:%d, cpld_bus:%d, addr:0x%X, offset:0x%X", __FUNCTION__, port_bit, bus, cpld_bus, addr, offset);
    DIAG_PRINT("%s, port:%d, control:%d(%s), value:0x%X", __FUNCTION__, port, control, sfp_control_to_str(control), *value);

    return rv;
}


int
onlp_sfpi_denit(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
    return ONLP_STATUS_OK;
}

