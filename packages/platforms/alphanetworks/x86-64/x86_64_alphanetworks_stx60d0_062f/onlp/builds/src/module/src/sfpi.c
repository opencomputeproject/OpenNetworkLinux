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

#if 0
#define DEBUG                           1
#else
#define DEBUG                           0
#endif

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

#define PAC9539_NUM1_I2C_BUS_ID           13

#define PAC9539_NUM1_I2C_ADDR              0x76  /* PAC9539#1 Physical Address in the I2C */

#define PAC9539_IO_INPUT 1
#define PAC9539_IO_OUTPUT 0

#define PAC9539_NUM1_IO0_INPUT_OFFSET   0x00 /* used to read SFP+ P1 and SFP+ P2 data */
#define PAC9539_NUM1_IO1_INPUT_OFFSET   0x01 /* used to read SFP+ uplink P1 and SFP+ uplink P2 data */

#define PAC9539_NUM1_IO0_OUTPUT_OFFSET   0x02 /* used to write SFP+ P1 and SFP+ P2 TXDIS bit (IO0_0 and IO0_4)*/
#define PAC9539_NUM1_IO1_OUTPUT_OFFSET   0x03 /* used to write SFP+ uplink P1 and SFP+ uplink P2 TXDIS bit (IO1_0 and IO1_4)*/

/* configures the directions of the I/O pin. 1:input, 0:output */
#define PAC9539_NUM1_IO0_DIRECTION_OFFSET   0x06 
#define PAC9539_NUM1_IO1_DIRECTION_OFFSET   0x07


#define PAC9539_NUM1_P1_TXDIS_BIT    0
#define PAC9539_NUM1_P1_TXFAULT_BIT    1
#define PAC9539_NUM1_P1_PRESENT_BIT    2
#define PAC9539_NUM1_P1_RXLOS_BIT    3
#define PAC9539_NUM1_P2_TXDIS_BIT    4
#define PAC9539_NUM1_P2_TXFAULT_BIT    5
#define PAC9539_NUM1_P2_PRESENT_BIT    6
#define PAC9539_NUM1_P2_RXLOS_BIT    7

#define SFP_I2C_BUS_ID_BASE             9

typedef enum
{
	SFP_PLUS_P1 =4,
	SFP_PLUS_P2 =5,
	SFP_PLUS_UPLINK_P1 =6,
	SFP_PLUS_UPLINK_P2 =7
}SFP_PLUS_PORT_T;


static int
port_to_busid(int port)
{
    int index = 0;
    index = (port-SFP_START_INDEX) + SFP_I2C_BUS_ID_BASE;

    DIAG_PRINT("%s, port:%d, busid:%d ", __FUNCTION__, port, index);
    return index;
}

static int
port_to_pca9539_sfp_offset(int port, int direction)
{
    int ret = 0;

    if (direction == PAC9539_IO_INPUT)
    {
        switch (port)
        {
            case SFP_PLUS_P1:
            case SFP_PLUS_P2:
                ret = PAC9539_NUM1_IO0_INPUT_OFFSET;
                break;

            case SFP_PLUS_UPLINK_P1:
            case SFP_PLUS_UPLINK_P2:
                ret = PAC9539_NUM1_IO1_INPUT_OFFSET;
                break;
               
            default:
                break;        
        }
    }
    else if (direction == PAC9539_IO_OUTPUT)
    {
        switch (port)
        {
            case SFP_PLUS_P1:
            case SFP_PLUS_P2:
                ret = PAC9539_NUM1_IO0_OUTPUT_OFFSET;
                break;

            case SFP_PLUS_UPLINK_P1:
            case SFP_PLUS_UPLINK_P2:
                ret = PAC9539_NUM1_IO1_OUTPUT_OFFSET;
                break;
               
            default:
                break;        
        }
    }

    DIAG_PRINT("%s, port:%d, direction %d, offset:0x%X", __FUNCTION__, port, direction, ret);
    return ret;
}


static int
port_to_pca9539_txdis_bit(int port)
{
    int index = 0;

    switch (port)
    {
        case SFP_PLUS_P1:
        case SFP_PLUS_UPLINK_P1:
            index = PAC9539_NUM1_P1_TXDIS_BIT;
            break;

        case SFP_PLUS_P2:
        case SFP_PLUS_UPLINK_P2:
            index = PAC9539_NUM1_P2_TXDIS_BIT;
            break;
           
        default:
            break;        
    }
    
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, index);
    return index;

}

static int
port_to_pca9539_txfault_bit(int port)
{
    int index = 0;

    switch (port)
    {
        case SFP_PLUS_P1:
        case SFP_PLUS_UPLINK_P1:
            index = PAC9539_NUM1_P1_TXFAULT_BIT;
            break;

        case SFP_PLUS_P2:
        case SFP_PLUS_UPLINK_P2:
            index = PAC9539_NUM1_P2_TXFAULT_BIT;
            break;
           
        default:
            break;        
    }
    
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, index);
    return index;

}

static int
port_to_pca9539_present_bit(int port)
{
    int index = 0;

    switch (port)
    {
        case SFP_PLUS_P1:
        case SFP_PLUS_UPLINK_P1:
            index = PAC9539_NUM1_P1_PRESENT_BIT;
            break;

        case SFP_PLUS_P2:
        case SFP_PLUS_UPLINK_P2:
            index = PAC9539_NUM1_P2_PRESENT_BIT;
            break;
           
        default:
            break;        
    }
    
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, index);
    return index;

}

static int
port_to_pca9539_rxlos_bit(int port)
{
    int index = 0;

    switch (port)
    {
        case SFP_PLUS_P1:
        case SFP_PLUS_UPLINK_P1:
            index = PAC9539_NUM1_P1_RXLOS_BIT;
            break;

        case SFP_PLUS_P2:
        case SFP_PLUS_UPLINK_P2:
            index = PAC9539_NUM1_P2_RXLOS_BIT;
            break;
           
        default:
            break;        
    }
    
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, index);
    return index;

}


static int
stx60d0_sfp_present(int port)
{
    int ret = 0;
    char data = 0;

    DIAG_PRINT("%s, port:%d", __FUNCTION__, port);

    switch (port)
    {
        case SFP_PLUS_P1:
        case SFP_PLUS_P2:
            ret = i2c_read_byte(PAC9539_NUM1_I2C_BUS_ID, PAC9539_NUM1_I2C_ADDR, PAC9539_NUM1_IO0_INPUT_OFFSET, &data);
            break;

        case SFP_PLUS_UPLINK_P1:
        case SFP_PLUS_UPLINK_P2:
            ret = i2c_read_byte(PAC9539_NUM1_I2C_BUS_ID, PAC9539_NUM1_I2C_ADDR, PAC9539_NUM1_IO1_INPUT_OFFSET, &data);
            break;
           
        default:
            break;        
    }

    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

//apple test
//printf("%s:%d port[%d],busid[%d],reg[%02x],read_byte[%02x]\n", __FUNCTION__, __LINE__,
//             port, PAC9539_NUM1_I2C_BUS_ID, PAC9539_NUM1_IO0_INPUT_OFFSET, (unsigned char)data);

    if (DEBUG)
        AIM_LOG_INFO("%s:%d port[%d],busid[%d],reg[%02x],read_byte[%02x]\n", __FUNCTION__, __LINE__,
                     port, PAC9539_NUM1_I2C_BUS_ID, PAC9539_NUM1_IO0_INPUT_OFFSET, (unsigned char)data);

    /* SFP+_P1_PRESENT_L. "0" indicates Module Present. */
    if (!(data & (1 << port_to_pca9539_present_bit(port))))
    {
        /*
         * Return 1 if present.
         * Return 0 if not present.
         * Return < 0 if error.
         */
        return 1;
    }

    return 0;
}

static int
stx60d0_sfp_pca9539_direction_set(int IO_port)
{
    int offset = 0;
    int pca9539_bus = PAC9539_NUM1_I2C_BUS_ID;
    int addr = PAC9539_NUM1_I2C_ADDR;
    char data = 0;
    int ret = 0;

    if (IO_port == 0)
        offset = PAC9539_NUM1_IO0_DIRECTION_OFFSET;
    else
        offset = PAC9539_NUM1_IO1_DIRECTION_OFFSET;
    
    data = onlp_i2c_readb(pca9539_bus, addr, offset, ONLP_I2C_F_FORCE);

    /* configuration direction to output, input:1, output:0.  */
    data &= ~(1 << PAC9539_NUM1_P1_TXDIS_BIT);
    data &= ~(1 << PAC9539_NUM1_P2_TXDIS_BIT);

    ret = onlp_i2c_writeb(pca9539_bus, addr, offset, data, ONLP_I2C_F_FORCE);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    return 0;      
}

static int
stx60d0_sfp_init(void)
{
    DIAG_PRINT("%s", __FUNCTION__);

#if 0 //apple test, why return 0x00 after i2cget -y 13 0x76 0x06 at the monent???
    int ret = 0;

    /* config pca9539#1 IO port0 direction */
    ret = stx60d0_sfp_pca9539_direction_set(0);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    /* config pca9539#1 IO port1 direction */
    ret = stx60d0_sfp_pca9539_direction_set(1);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }
#endif
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
    stx60d0_sfp_init();

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

    for (p = SFP_START_INDEX; p < SFP_START_INDEX+NUM_OF_SFP_PORT; p++)
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

    present = stx60d0_sfp_present(port);

    DIAG_PRINT("%s, port=%d, present:%d", __FUNCTION__, port, present);
    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t *dst)
{
    DIAG_PRINT("%s", __FUNCTION__);

    int i = 0;
    AIM_BITMAP_CLR_ALL(dst);
    for (i = SFP_START_INDEX; i < SFP_START_INDEX+NUM_OF_SFP_PORT; i++)
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

    if (i2c_read(busid, SFP_PLUS_EEPROM_I2C_ADDR, 0x0, 256, (char *)data) != 0)
    {
        AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
    
}

int onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    int busid = port_to_busid(port);
    memset(data, 0x0, 256);
    DIAG_PRINT("%s, port:%d, busid:%d", __FUNCTION__, port, busid);

    if (i2c_read(busid, SFP_DOM_EEPROM_I2C_ADDR, 0x0, 256, (char *)data) != 0)
    {
        AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
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
            *supported = 0;
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
    int pca9539_bus = PAC9539_NUM1_I2C_BUS_ID;
    int addr = PAC9539_NUM1_I2C_ADDR;
    int offset = 0;
    int port_bit = port_to_pca9539_txdis_bit(port);
    int supported = 0;
    char optval = 0;

    if ((onlp_sfpi_control_supported(port, control, &supported) == ONLP_STATUS_OK) && (supported == 0))
        return ONLP_STATUS_E_UNSUPPORTED;

    DIAG_PRINT("%s, port:%d, control:%d(%s), value:0x%X", __FUNCTION__, port, control, sfp_control_to_str(control), value);

    int ret = 0;

    /* config pca9539#1 IO port0 direction */
    ret = stx60d0_sfp_pca9539_direction_set(0);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    /* config pca9539#1 IO port1 direction */
    ret = stx60d0_sfp_pca9539_direction_set(1);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

//    if (port < QSFP_START_INDEX) //SFP
    {
        switch (control)
        {
            /* TXDIS is Output direction!!!! */
            case ONLP_SFP_CONTROL_TX_DISABLE:
                offset = port_to_pca9539_sfp_offset(port, PAC9539_IO_OUTPUT);
                break;
            default:
                return ONLP_STATUS_E_UNSUPPORTED;
                break;
        }

        optval = onlp_i2c_readb(pca9539_bus, addr, offset, ONLP_I2C_F_FORCE);

        if (value != 0)
        {
            optval |= (1 << port_bit);
        }
        else
        {
            optval &= ~(1 << port_bit);
        }

        rv = onlp_i2c_writeb(pca9539_bus, addr, offset, optval, ONLP_I2C_F_FORCE);
        if (rv < 0)
        {
            AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, rv);
            return rv;
        }

    }
    
    DIAG_PRINT("%s, port_bit:%d, bus:%d, pca9539_bus:%d, addr:0x%X, offset:0x%X", __FUNCTION__, port_bit, bus, pca9539_bus, addr, offset);
    return rv;
  
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int *value)
{
    int rv = ONLP_STATUS_OK;
    int pca9539_bus = PAC9539_NUM1_I2C_BUS_ID;
    int addr = PAC9539_NUM1_I2C_ADDR;
    int offset = 0;
    int port_bit;
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

    switch (control)
    {
        case ONLP_SFP_CONTROL_RX_LOS:
            offset = port_to_pca9539_sfp_offset(port, PAC9539_IO_INPUT);               
            port_bit = port_to_pca9539_rxlos_bit(port);
            break;
            
        case ONLP_SFP_CONTROL_TX_FAULT:
            offset = port_to_pca9539_sfp_offset(port, PAC9539_IO_INPUT);
            port_bit = port_to_pca9539_txfault_bit(port);
            break;
            
        case ONLP_SFP_CONTROL_TX_DISABLE:
            offset = port_to_pca9539_sfp_offset(port, PAC9539_IO_INPUT);
            port_bit = port_to_pca9539_txdis_bit(port);
            break;
            
        default:
            return ONLP_STATUS_E_UNSUPPORTED;
            break;
    }
    optval = onlp_i2c_readb(pca9539_bus, addr, offset, ONLP_I2C_F_FORCE);
    if ((optval & (1 << port_bit)) != 0) //1
    {
        *value = 1;
    }
    else
    {
        *value = 0;
    }
    
    DIAG_PRINT("%s, port_bit:%d, cpld_bus:%d, addr:0x%X, offset:0x%X", __FUNCTION__, port_bit, pca9539_bus, addr, offset);
    DIAG_PRINT("%s, port:%d, control:%d(%s), value:0x%X", __FUNCTION__, port, control, sfp_control_to_str(control), *value);

    return rv;

}


int
onlp_sfpi_denit(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
    return ONLP_STATUS_OK;
}

