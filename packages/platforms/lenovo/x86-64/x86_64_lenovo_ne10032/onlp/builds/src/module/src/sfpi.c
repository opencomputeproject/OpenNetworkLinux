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
#define SYSTEM_CPLD_RESET1_ADDR_OFFSET  0x02  /* Need to reset Port CPLD0~3 */

#define PORT_CPLD0_I2C_BUS_ID           13
#define PORT_CPLD1_I2C_BUS_ID           14
#define PORT_CPLD2_I2C_BUS_ID           15
#define PORT_CPLD3_I2C_BUS_ID           16
#define PORT_CPLD_I2C_ADDR              0x5F  /* Port CPLD Physical Address in the I2C */


#define PORT_CPLD_PRESENT_ADDR_OFFSET   0x05
#define PORT_CPLD_RESET_ADDR_OFFSET     0x07
#define PORT_CPLD_LOWPWR_ADDR_OFFSET    0x09
#define PORT_CPLD_MOD_SEL_ADDR_OFFSET   0x0B  /* Module Select of QSFP ports */


static int
port_to_busid(int port)
{
    int ret = 0;
    int index = 0;
    index = (port / 8);
    ret = index + PORT_CPLD0_I2C_BUS_ID;
    DIAG_PRINT("%s, port:%d, busid:%d ", __FUNCTION__, port, ret);

    return ret;
}

static int
port_to_cpld_present_bit(int port)
{
    int index = 0;
    index = (port % 8);
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, index);
    return index;
}

static int
port_to_cpld_mod_sel_bit(int port)
{
    int index = 0;
    index = (port % 8);
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, index);
    return index;
}

static int
ne10032_sfp_present(int port)
{
    int ret = 0;
    char data = 0;

    DIAG_PRINT("%s, port:%d", __FUNCTION__, port);

    ret = i2c_read_byte(port_to_busid(port), PORT_CPLD_I2C_ADDR, PORT_CPLD_PRESENT_ADDR_OFFSET, &data);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    if (DEBUG)
        AIM_LOG_INFO("%s:%d port[%d],busid[%d],reg[%02x],read_byte[%02x]\n", __FUNCTION__, __LINE__,
                     port, port_to_busid(port), PORT_CPLD_PRESENT_ADDR_OFFSET, (unsigned char)data);

    if (data & (1 << port_to_cpld_present_bit(port)))
    {
        return 1;
    }

    return 0;
}

static int
ne10032_sfp_init(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
    int ret = 0;
    char data = 0;

    /* pull low for Reset Register 1 of Port CPLD0~3 */
    ret = i2c_write_byte(SYSTEM_CPLD_I2C_BUS_ID, SYSTEM_CPLD_I2C_ADDR, SYSTEM_CPLD_RESET1_ADDR_OFFSET, data);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
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
    /* Called at initialization time */
    ne10032_sfp_init();

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t *bmap)
{

    /*
     * Ports {0, 32}
     */
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

    present = ne10032_sfp_present(port);

    DIAG_PRINT("%s, port=%d, present:%d", __FUNCTION__, port, present);
    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t *dst)
{
    DIAG_PRINT("%s", __FUNCTION__);
    uint8_t bytes[4] = { 0 };
    char data = 0;
    int i = 0, ret = 0;

    for (i = 0; i < 4; i++)
    {
        data = 0;
        ret = i2c_read_byte(PORT_CPLD0_I2C_BUS_ID + i, PORT_CPLD_I2C_ADDR, PORT_CPLD_PRESENT_ADDR_OFFSET, &data);

        if (ret < 0)
        {
            AIM_LOG_INFO("%s:%d [%d], fail[%d]\n", __FUNCTION__, __LINE__, PORT_CPLD0_I2C_BUS_ID + i, ret);
            return ONLP_STATUS_E_INTERNAL;
        }

        bytes[i] = data;
    }

    //debug
    //printf("[DEBUG]onlp_sfpi_presence_bitmap_get {0x%x 0x%x 0x%x 0x%x}\n",bytes[0],bytes[1],bytes[2],bytes[3]);

    if (DEBUG)
        AIM_LOG_INFO("onlp_sfpi_presence_bitmap_get 0x%x %x %x %x %x %x %x\r\n", bytes[3], bytes[2], bytes[1], bytes[0]);

    /* Convert to 64 bit integer in port order */
    uint32_t presence_all = 0;
    for (i = AIM_ARRAYSIZE(bytes) - 1; i >= 0; i--)
    {
        presence_all <<= 8;
        presence_all |= bytes[i];
    }

    /* Populate bitmap */
    for (i = 0; presence_all; i++)
    {
        AIM_BITMAP_MOD(dst, i, (presence_all & 1));
        presence_all >>= 1;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    DIAG_PRINT("%s, port=%d", __FUNCTION__, port);
    char mod_sel = 0;
    int ret = 0;

    /* Module select for the port */
    mod_sel = (1 << port_to_cpld_mod_sel_bit(port));

    ret = i2c_write_byte(port_to_busid(port), PORT_CPLD_I2C_ADDR, PORT_CPLD_MOD_SEL_ADDR_OFFSET, mod_sel);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    /*
     * Read the SFP eeprom into data[]
     */
    memset(data, 0x0, 256);

    if (i2c_read(port_to_busid(port), QSFP28_EEPROM_I2C_ADDR, 0x0, 256, (char *)data) != 0)
    {
        AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    DIAG_PRINT("%s, port=%d", __FUNCTION__, port);
    char mod_sel = 0;
    int ret = 0;

    //Set page select to pahe 00h.
    ret = onlp_sfpi_dev_writeb(port, QSFP28_EEPROM_I2C_ADDR, QSFP28_EEPROM_PAGE_SELECT_OFFSET, 0);
    //ret = i2c_write_byte(port_to_busid(port), QSFP28_EEPROM_I2C_ADDR, QSFP28_EEPROM_PAGE_SELECT_OFFSET, 0);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    /* Module select for the port */
    mod_sel = (1 << port_to_cpld_mod_sel_bit(port));

    ret = i2c_write_byte(port_to_busid(port), PORT_CPLD_I2C_ADDR, PORT_CPLD_MOD_SEL_ADDR_OFFSET, mod_sel);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    /*
     * Read the SFP DOM eeprom into data[]
     */
    memset(data, 0x0, 256);

    if (i2c_read(port_to_busid(port), QSFP28_EEPROM_I2C_ADDR, 0x0, 256, (char *)data) != 0)
    {
        AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int ret;
    int bus = port_to_busid(port);
    char mod_sel = 0;

    /* Module select for the port */
    mod_sel = (1 << port_to_cpld_mod_sel_bit(port));

    ret = i2c_write_byte(bus, PORT_CPLD_I2C_ADDR, PORT_CPLD_MOD_SEL_ADDR_OFFSET, mod_sel);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
    }

    ret = onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
    DIAG_PRINT("%s, port:%d, devaddr:%d, addr:%d, ret:%d(0x%02X)", __FUNCTION__, port, devaddr, addr, ret, ret);
    return ret;
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{

    int ret;
    int bus = port_to_busid(port);
    char mod_sel = 0;

    /* Module select for the port */
    mod_sel = (1 << port_to_cpld_mod_sel_bit(port));
    ret = i2c_write_byte(bus, PORT_CPLD_I2C_ADDR, PORT_CPLD_MOD_SEL_ADDR_OFFSET, mod_sel);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    ret = onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
    DIAG_PRINT("%s, port:%d, devaddr:%d, addr:%d, value:%d(0x%02X), ret:%d", __FUNCTION__, port, devaddr, addr, value, value, ret);

    return ret;
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int ret;
    int bus = port_to_busid(port);
    char mod_sel = 0;

    /* Module select for the port */
    mod_sel = (1 << port_to_cpld_mod_sel_bit(port));
    ret = i2c_write_byte(bus, PORT_CPLD_I2C_ADDR, PORT_CPLD_MOD_SEL_ADDR_OFFSET, mod_sel);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
    }

    ret = onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
    DIAG_PRINT("%s, port:%d, devaddr:%d, addr:%d, ret:%d(0x%04X)", __FUNCTION__, port, devaddr, addr, ret, ret);
    return ret;
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{

    int ret;
    int bus = port_to_busid(port);
    char mod_sel = 0;

    /* Module select for the port */
    mod_sel = (1 << port_to_cpld_mod_sel_bit(port));
    ret = i2c_write_byte(bus, PORT_CPLD_I2C_ADDR, PORT_CPLD_MOD_SEL_ADDR_OFFSET, mod_sel);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    ret = onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
    DIAG_PRINT("%s, port:%d, devaddr:%d, addr:%d, value:%d(0x%04X), ret:%d", __FUNCTION__, port, devaddr, addr, value, value, ret);
    return ret;
}

/* 
  Reset and LP mode can control by CPLD so the setting will be keep in CPLD.
  For other options, control is get/set to QSFP28.
  Control options set to QSFP28 will be lost when the QSFP28 is removed.
  Upper layer software system should keep the configuration and set it again when detect a new sfp module insert. 
 
     function                            R/W  CPLD    QSFP28 EEPROM
    ------------------------------------ ---  ------  -----------------
    ONLP_SFP_CONTROL_RESET                W   0x7
    ONLP_SFP_CONTROL_RESET_STATE         R/W  0x7  
    ONLP_SFP_CONTROL_RX_LOS               R   none    byte 4
    ONLP_SFP_CONTROL_TX_FAULT             R   none    byte 3
    ONLP_SFP_CONTROL_TX_DISABLE          R/W  none    byte 86
    ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL  R/W  none    byte 86
    ONLP_SFP_CONTROL_LP_MODE             R/W  0x9
    ONLP_SFP_CONTROL_POWER_OVERRIDE      R/W  none    byte 93
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
        case ONLP_SFP_CONTROL_RESET:
        case ONLP_SFP_CONTROL_RESET_STATE:
        case ONLP_SFP_CONTROL_LP_MODE:
        case ONLP_SFP_CONTROL_POWER_OVERRIDE:
        case ONLP_SFP_CONTROL_RX_LOS:
        case ONLP_SFP_CONTROL_TX_FAULT:
        case ONLP_SFP_CONTROL_TX_DISABLE:
        case ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL:
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
    int mod_sel_bit = port_to_cpld_mod_sel_bit(port);
    int supported = 0;
    char optval = 0;

    if ((onlp_sfpi_control_supported(port, control, &supported) == ONLP_STATUS_OK) && (supported == 0))
        return ONLP_STATUS_E_UNSUPPORTED;

    DIAG_PRINT("%s, port:%d, control:%d(%s), value:%d", __FUNCTION__, port, control, sfp_control_to_str(control), value);

    /* Module select for the port */
    optval = (1 << mod_sel_bit);
    rv = i2c_write_byte(bus, PORT_CPLD_I2C_ADDR, PORT_CPLD_MOD_SEL_ADDR_OFFSET, optval);
    if (rv < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, rv);
    }

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
            optval = onlp_i2c_readb(bus, PORT_CPLD_I2C_ADDR, PORT_CPLD_RESET_ADDR_OFFSET, ONLP_I2C_F_FORCE);
            if (value != 0)
            {
                optval |= (1 << mod_sel_bit);
            }
            else
            {
                optval &= !(1 << mod_sel_bit);
            }
            rv = onlp_i2c_writeb(bus, PORT_CPLD_I2C_ADDR, PORT_CPLD_RESET_ADDR_OFFSET, optval, ONLP_I2C_F_FORCE);
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
            optval = onlp_i2c_readb(bus, QSFP28_EEPROM_I2C_ADDR, QSFP28_EEPROM_TX_DISABLE_OFFSET, ONLP_I2C_F_FORCE);
            if (value != 0)
            {
                optval |= 0x0f; //bit 0~3
            }
            else
            {
                optval &= !(0x0f);
            }
            rv = onlp_i2c_writeb(bus, QSFP28_EEPROM_I2C_ADDR, QSFP28_EEPROM_TX_DISABLE_OFFSET, optval, ONLP_I2C_F_FORCE);
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
            optval = value;
            rv = onlp_i2c_writeb(bus, QSFP28_EEPROM_I2C_ADDR, QSFP28_EEPROM_TX_DISABLE_OFFSET, optval, ONLP_I2C_F_FORCE);
            if (rv < 0)
                return rv;
            break;

        case ONLP_SFP_CONTROL_POWER_OVERRIDE:
            if (onlp_sfpi_is_present(port) == 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ONLP_STATUS_E_MISSING);
                return ONLP_STATUS_E_MISSING;
            }
            optval = onlp_i2c_readb(bus, QSFP28_EEPROM_I2C_ADDR, QSFP28_EEPROM_POWERSET_OFFSET, ONLP_I2C_F_FORCE);
            if (value != 0)
            {
                optval |= 0x01;
            }
            else
            {
                optval &= !(0x01);
            }
            rv = onlp_i2c_writeb(bus, QSFP28_EEPROM_I2C_ADDR, QSFP28_EEPROM_POWERSET_OFFSET, optval, ONLP_I2C_F_FORCE);
            if (rv < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, rv);
                return rv;
            }
            break;

        case ONLP_SFP_CONTROL_LP_MODE:
            optval = onlp_i2c_readb(bus, PORT_CPLD_I2C_ADDR, PORT_CPLD_LOWPWR_ADDR_OFFSET, ONLP_I2C_F_FORCE);
            if (value != 0)
            {
                optval |= (1 << mod_sel_bit);
            }
            else
            {
                optval &= !(1 << mod_sel_bit);
            }
            rv = onlp_i2c_writeb(bus, PORT_CPLD_I2C_ADDR, PORT_CPLD_LOWPWR_ADDR_OFFSET, optval, ONLP_I2C_F_FORCE);
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
    return rv;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int *value)
{
    int rv = ONLP_STATUS_OK;
    int bus = port_to_busid(port);
    int mod_sel_bit = port_to_cpld_mod_sel_bit(port);
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

    /* Module select for the port */
    optval = (1 << mod_sel_bit);
    rv = i2c_write_byte(bus, PORT_CPLD_I2C_ADDR, PORT_CPLD_MOD_SEL_ADDR_OFFSET, optval);
    if (rv < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, rv);
    }

    *value = 0;
    switch (control)
    {
        case ONLP_SFP_CONTROL_RESET_STATE:
            optval = onlp_i2c_readb(bus, PORT_CPLD_I2C_ADDR, PORT_CPLD_RESET_ADDR_OFFSET, ONLP_I2C_F_FORCE);
            if ((optval & (1 << mod_sel_bit)) != 0) //1
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
                *value = 1;
                break;
#else
                return ONLP_STATUS_E_MISSING;

#endif
            }
            optval = onlp_i2c_readb(bus, QSFP28_EEPROM_I2C_ADDR, QSFP28_EEPROM_TXRX_LOS_OFFSET, ONLP_I2C_F_FORCE);
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
            optval = onlp_i2c_readb(bus, QSFP28_EEPROM_I2C_ADDR, QSFP28_EEPROM_TX_FAULT_OFFSET, ONLP_I2C_F_FORCE);
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
            optval = onlp_i2c_readb(bus, QSFP28_EEPROM_I2C_ADDR, QSFP28_EEPROM_TX_DISABLE_OFFSET, ONLP_I2C_F_FORCE);
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
            optval = onlp_i2c_readb(bus, QSFP28_EEPROM_I2C_ADDR, QSFP28_EEPROM_TX_DISABLE_OFFSET, ONLP_I2C_F_FORCE);
            *value = optval & (0x0f); //bit 0~3
            break;

        case ONLP_SFP_CONTROL_LP_MODE:
            optval = onlp_i2c_readb(bus, PORT_CPLD_I2C_ADDR, PORT_CPLD_LOWPWR_ADDR_OFFSET, ONLP_I2C_F_FORCE);
            if ((optval & (1 << mod_sel_bit)) != 0)
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
            optval = onlp_i2c_readb(bus, QSFP28_EEPROM_I2C_ADDR, QSFP28_EEPROM_POWERSET_OFFSET, ONLP_I2C_F_FORCE);
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
            break;
    }

    DIAG_PRINT("%s, port:%d, control:%d(%s), value:%d", __FUNCTION__, port, control, sfp_control_to_str(control), *value);

    return rv;
}


int
onlp_sfpi_denit(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
    return ONLP_STATUS_OK;
}

