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

#define PCA9539_NUM1_I2C_BUS_ID           3
#define PCA9539_NUM3_I2C_BUS_ID           3
#define TCA6424_NUM0_I2C_BUS_ID           7

#define PCA9539_NUM1_I2C_ADDR              0x76  /* PCA9539#1 Physical Address in the I2C */
#define PCA9539_NUM3_I2C_ADDR              0x77  /* PCA9539#3 Physical Address in the I2C */
#define TCA6424_NUM0_I2C_ADDR              0x22  /* TCA6424#0 Physical Address in the I2C */

/******************* PCA9539 *******************/
#define NUM_OF_PCA9539_IO_PORT 2 /*IO0 bit 0-7, and IO1 bit 0-7*/

#define PCA9539_IO_INPUT 1
#define PCA9539_IO_OUTPUT 0

#define PCA9539_NUM1_IO0_INPUT_OFFSET   0x00 /* used to read SFP P1 and SFP P2 data */
#define PCA9539_NUM1_IO1_INPUT_OFFSET   0x01 /* used to read SFP P3 and SFP P4 data */

#define PCA9539_NUM1_IO0_OUTPUT_OFFSET   0x02 /* used to write SFP P1 and SFP P2 TXDIS bit (IO0_0 and IO0_4)*/
#define PCA9539_NUM1_IO1_OUTPUT_OFFSET   0x03 /* used to write SFP P3 and SFP P4 TXDIS bit (IO1_0 and IO1_4)*/

/* configures the directions of the I/O pin. 1:input, 0:output */
#define PCA9539_NUM1_IO0_DIRECTION_OFFSET   0x06 
#define PCA9539_NUM1_IO1_DIRECTION_OFFSET   0x07

/* general define for all PCA9539 IO, include IO0 and IO1 */
#define PCA9539_NUM1_IO_SFP_P2_TXDIS_BIT    0
#define PCA9539_NUM1_IO_SFP_P2_TXFAULT_BIT    1
#define PCA9539_NUM1_IO_SFP_P2_PRESENT_BIT    2
#define PCA9539_NUM1_IO_SFP_P2_RXLOS_BIT    3
#define PCA9539_NUM1_IO_SFP_P1_TXDIS_BIT    4
#define PCA9539_NUM1_IO_SFP_P1_TXFAULT_BIT    5
#define PCA9539_NUM1_IO_SFP_P1_PRESENT_BIT    6
#define PCA9539_NUM1_IO_SFP_P1_RXLOS_BIT   7


/******************* TCA6424 *******************/
#define NUM_OF_TCA6424_IO_PORT 3 /*IO0 bit 0-7, IO1 bit 0-7, and IO2 bit 0-7*/

#define TCA6424_IO_INPUT 1
#define TCA6424_IO_OUTPUT 0

#define TCA6424_NUM0_IO0_INPUT_OFFSET   0x00 /* used to read SFP+ P6 and SFP+ P5 data */
#define TCA6424_NUM0_IO1_INPUT_OFFSET   0x01 /* used to read SFP+ P4 and SFP+ P3 data */
#define TCA6424_NUM0_IO2_INPUT_OFFSET   0x02 /* used to read SFP+ P2 and SFP+ P1 data */

#define TCA6424_NUM0_IO0_OUTPUT_OFFSET   0x04 /* used to write SFP+ P6 and SFP+ P5 TXDIS bit (IO0_0 and IO0_4)*/
#define TCA6424_NUM0_IO1_OUTPUT_OFFSET   0x05 /* used to write SFP+ P4 and SFP+ P3 TXDIS bit (IO1_0 and IO1_4)*/
#define TCA6424_NUM0_IO2_OUTPUT_OFFSET   0x06 /* used to write SFP+ P2 and SFP+ P1 TXDIS bit (IO1_0 and IO1_4)*/

/* configures the directions of the I/O pin. 1:input, 0:output */
#define TCA6424_NUM0_IO0_DIRECTION_OFFSET   0x0c 
#define TCA6424_NUM0_IO1_DIRECTION_OFFSET   0x0d
#define TCA6424_NUM0_IO2_DIRECTION_OFFSET   0x0e

/* general define for all TCA6424 IO, include IO0,IO1, and IO2 */
#define TCA6424_NUM0_IO_SFP_P2_TXDIS_BIT    0
#define TCA6424_NUM0_IO_SFP_P2_TXFAULT_BIT    1
#define TCA6424_NUM0_IO_SFP_P2_PRESENT_BIT    2
#define TCA6424_NUM0_IO_SFP_P2_RXLOS_BIT    3
#define TCA6424_NUM0_IO_SFP_P1_TXDIS_BIT    4
#define TCA6424_NUM0_IO_SFP_P1_TXFAULT_BIT    5
#define TCA6424_NUM0_IO_SFP_P1_PRESENT_BIT    6
#define TCA6424_NUM0_IO_SFP_P1_RXLOS_BIT   7

/* special define for TCA6424 IO2, because SFP+_P2_TXFAULT and SFP+_P2_TXDIS are swapped. */
/* reference to STX-60D0-126F_HW_Spec_V01_20200320B.docx Note 3 table. */
#define TCA6424_NUM0_IO2_SFP_P2_TXDIS_BIT    1
#define TCA6424_NUM0_IO2_SFP_P2_TXFAULT_BIT    0


#define SFP_I2C_BUS_ID_BASE             9

typedef enum sfp_port_e
{
//	1G_RJ45_P1 =0,
//	1G_RJ45_P2 =1,
//	1G_RJ45_P3 =2,
//	1G_RJ45_P4 =3,
	SFP_PORT_E_1G_SFP_P1 =4,
	SFP_PORT_E_1G_SFP_P2 =5,
	SFP_PORT_E_1G_SFP_P3 =6,
	SFP_PORT_E_1G_SFP_P4 =7,
	SFP_PORT_E_1G_SFP_P5 =8,
	SFP_PORT_E_1G_SFP_P6 =9,
	SFP_PORT_E_1G_SFP_P7 =10,
	SFP_PORT_E_1G_SFP_P8 =11,		
	SFP_PORT_E_10G_SFP_PLUS_P1 =12,
	SFP_PORT_E_10G_SFP_PLUS_P2 =13,
	SFP_PORT_E_10G_SFP_PLUS_P3 =14,
	SFP_PORT_E_10G_SFP_PLUS_P4 =15,
	SFP_PORT_E_10G_SFP_PLUS_P5 =16,
	SFP_PORT_E_10G_SFP_PLUS_P6 =17,
}sfp_port_t;

static int
port_to_pca9539_busid(int port)
{
    int ret = 0;

    if ((port < SFP_PORT_E_1G_SFP_P1) || (port > SFP_PORT_E_1G_SFP_P8))
        return ret;

    if (port >= SFP_PORT_E_1G_SFP_P5)
        ret = PCA9539_NUM3_I2C_BUS_ID;
    else    
        ret = PCA9539_NUM1_I2C_BUS_ID;
    
    DIAG_PRINT("%s, port:%d, cpld_busid:%d ", __FUNCTION__, port, ret);
    return ret;
}

static int
port_to_pca9539_addr(int port)
{
    int ret = 0;

    if ((port < SFP_PORT_E_1G_SFP_P1) || (port > SFP_PORT_E_1G_SFP_P8))
        return ret;
    
    if (port >= SFP_PORT_E_1G_SFP_P5)
        ret = PCA9539_NUM3_I2C_ADDR;
    else    
        ret = PCA9539_NUM1_I2C_ADDR;
	
    DIAG_PRINT("%s, port:%d, pca9539_addr:%d ", __FUNCTION__, port, ret);
    return ret;
}

static int
port_to_pca9539_sfp_offset(int port, int direction)
{
    int index = 0;
    int ret = 0;

    if (direction == PCA9539_IO_INPUT)
    {
        index = port % 4;
        
        if (index==0 ||index==1) // 1G_SFP_P1, 1G_SFP_P2, 1G_SFP_P5, 1G_SFP_P6,
            ret = PCA9539_NUM1_IO0_INPUT_OFFSET;
        else // 1G_SFP_P3, 1G_SFP_P4, 1G_SFP_P7, 1G_SFP_P8,
            ret = PCA9539_NUM1_IO1_INPUT_OFFSET;            
    }
    else if (direction == PCA9539_IO_OUTPUT)
    {
        index = port % 4;
        
        if (index==0 ||index==1) // 1G_SFP_P1, 1G_SFP_P2, 1G_SFP_P5, 1G_SFP_P6,
            ret = PCA9539_NUM1_IO0_OUTPUT_OFFSET;
        else // 1G_SFP_P3, 1G_SFP_P4, 1G_SFP_P7, 1G_SFP_P8,
            ret = PCA9539_NUM1_IO1_OUTPUT_OFFSET;    
    }

    DIAG_PRINT("%s, port:%d, direction %d, offset:0x%X", __FUNCTION__, port, direction, ret);
    return ret;
}

static int
port_to_tca6424_sfp_offset(int port, int direction)
{
    int ret = 0;

    if (direction == TCA6424_IO_INPUT)
    {
        switch (port)
        {
            case SFP_PORT_E_10G_SFP_PLUS_P1:
            case SFP_PORT_E_10G_SFP_PLUS_P2:
                ret = TCA6424_NUM0_IO2_INPUT_OFFSET;
                break;

            case SFP_PORT_E_10G_SFP_PLUS_P3:
            case SFP_PORT_E_10G_SFP_PLUS_P4:
                ret = TCA6424_NUM0_IO1_INPUT_OFFSET;
                break;

            case SFP_PORT_E_10G_SFP_PLUS_P5:
            case SFP_PORT_E_10G_SFP_PLUS_P6:
                ret = TCA6424_NUM0_IO0_INPUT_OFFSET;
                break;
               
            default:
                break;        
        }
    }
    else if (direction == TCA6424_IO_OUTPUT)
    {
        switch (port)
        {
            case SFP_PORT_E_10G_SFP_PLUS_P1:
            case SFP_PORT_E_10G_SFP_PLUS_P2:
                ret = TCA6424_NUM0_IO2_OUTPUT_OFFSET;
                break;

            case SFP_PORT_E_10G_SFP_PLUS_P3:
            case SFP_PORT_E_10G_SFP_PLUS_P4:
                ret = TCA6424_NUM0_IO1_OUTPUT_OFFSET;
                break;

            case SFP_PORT_E_10G_SFP_PLUS_P5:
            case SFP_PORT_E_10G_SFP_PLUS_P6:
                ret = TCA6424_NUM0_IO0_OUTPUT_OFFSET;
                break;
               
            default:
                break;        
        }
    }

    DIAG_PRINT("%s, port:%d, direction %d, offset:0x%X", __FUNCTION__, port, direction, ret);
    return ret;
}


static int
port_to_pca9539_present_bit(int port)
{
    int index = 0;
    int ret = 0;

    index = port % 2;

    if (index == 0)
        ret = PCA9539_NUM1_IO_SFP_P1_PRESENT_BIT;
    else
        ret = PCA9539_NUM1_IO_SFP_P2_PRESENT_BIT;
       
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, ret);
    return ret;

}

static int
port_to_tca6424_present_bit(int port)
{
    int index = 0;
    int ret = 0;

    index = port % 2;

    if (index == 0)
        ret = TCA6424_NUM0_IO_SFP_P1_PRESENT_BIT;
    else
        ret = TCA6424_NUM0_IO_SFP_P2_PRESENT_BIT;
       
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, ret);
    return ret;

}

static int
port_to_pca9539_txdis_bit(int port)
{
    int index = 0;
    int ret = 0;

    index = port % 2;

    if (index == 0)
        ret = PCA9539_NUM1_IO_SFP_P1_TXDIS_BIT;
    else
        ret = PCA9539_NUM1_IO_SFP_P2_TXDIS_BIT;
   
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, index);
    return ret;
}

static int
port_to_tca6424_txdis_bit(int port)
{
    int index = 0;
    int ret = 0;

    /* SFP+_P2_TXFAULT and SFP+_P2_TXDIS are swapped. */
    /* reference to STX-60D0-126F_HW_Spec_V01_20200320B.docx Note 3 table. */
    if ( port == SFP_PORT_E_10G_SFP_PLUS_P2)
    {
        ret = TCA6424_NUM0_IO2_SFP_P2_TXDIS_BIT;
    }
    else
    {
        index = port % 2;

        if (index == 0)
            ret = TCA6424_NUM0_IO_SFP_P1_TXDIS_BIT;
        else
            ret = TCA6424_NUM0_IO_SFP_P2_TXDIS_BIT;
    }
   
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, index);
    return ret;
}

static int
port_to_pca9539_rxlos_bit(int port)
{
    int index = 0;
    int ret = 0;

    index = port % 2;

    if (index == 0)
        ret = PCA9539_NUM1_IO_SFP_P1_RXLOS_BIT;
    else
        ret = PCA9539_NUM1_IO_SFP_P2_RXLOS_BIT;
   
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, index);
    return ret;
}

static int
port_to_tca6424_rxlos_bit(int port)
{
    int index = 0;
    int ret = 0;

    index = port % 2;

    if (index == 0)
        ret = TCA6424_NUM0_IO_SFP_P1_RXLOS_BIT;
    else
        ret = TCA6424_NUM0_IO_SFP_P2_RXLOS_BIT;
   
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, index);
    return ret;
}

static int
port_to_pca9539_txfault_bit(int port)
{
    int index = 0;
    int ret = 0;

    index = port % 2;

    if (index == 0)
        ret = PCA9539_NUM1_IO_SFP_P1_TXFAULT_BIT;
    else
        ret = PCA9539_NUM1_IO_SFP_P2_TXFAULT_BIT;
   
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, index);
    return ret;
}

static int
port_to_tca6424_txfault_bit(int port)
{
    int index = 0;
    int ret = 0;

    /* SFP+_P2_TXFAULT and SFP+_P2_TXDIS are swapped. */
    /* reference to STX-60D0-126F_HW_Spec_V01_20200320B.docx Note 3 table. */
    if ( port == SFP_PORT_E_10G_SFP_PLUS_P2)
    {
        ret = TCA6424_NUM0_IO2_SFP_P2_TXFAULT_BIT;
    }
    else
    {
        index = port % 2;

        if (index == 0)
            ret = TCA6424_NUM0_IO_SFP_P1_TXFAULT_BIT;
        else
            ret = TCA6424_NUM0_IO_SFP_P2_TXFAULT_BIT;
    }
    
    DIAG_PRINT("%s, port:%d, index:%d ", __FUNCTION__, port, index);
    return ret;
}

static int
stx60d0_10g_sfp_plus_port_to_busid(int port)
{
    int busid = 0;
    busid = (port-SFP_START_INDEX) + SFP_I2C_BUS_ID_BASE;

    DIAG_PRINT("%s, port:%d, busid:%d ", __FUNCTION__, port, busid);
    return busid;

}

static int
stx60d0_1g_sfp_port_to_busid(int port)
{
    int busid = 0;
    int index = 0;
	
    busid = (port-SFP_START_INDEX) + SFP_I2C_BUS_ID_BASE;

    /* I2C Channels in Main Board for 1G SFP ports upside down */
    /* reference to STX-60D0-126F_HW_Spec_V01_20200320B.docx Figure 25 */
    index = port % 2;

    if (index == 0)
        busid = busid + 1;
    else
        busid = busid - 1;	

    DIAG_PRINT("%s, port:%d, busid:%d ", __FUNCTION__, port, busid);
    return busid;

}

static int
port_to_busid(int port)
{
    int busid = 0;

    if (port >= SFP_PORT_E_10G_SFP_PLUS_P1)
        busid = stx60d0_10g_sfp_plus_port_to_busid(port);
    else         
        busid = stx60d0_1g_sfp_port_to_busid(port);

    return busid;
}

static int
stx60d0_1g_sfp_present(int port)
{
    int ret = 0;
    char data = 0;
    
    DIAG_PRINT("%s, port:%d", __FUNCTION__, port);

    int busid = port_to_pca9539_busid(port);
    int addr = port_to_pca9539_addr(port);
    int offset = port_to_pca9539_sfp_offset(port, PCA9539_IO_INPUT);

    ret = i2c_read_byte(busid, addr, offset, &data);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

//debug
//printf("%s:%d port[%d],busid[%d],offset[%02x],read_byte[%02x]\n", __FUNCTION__, __LINE__,
//             port, busid, offset, (unsigned char)data);

    if (DEBUG)
        AIM_LOG_INFO("%s:%d port[%d],busid[%d],reg[%02x],read_byte[%02x]\n", __FUNCTION__, __LINE__,
                     port, PCA9539_NUM1_I2C_BUS_ID, PCA9539_NUM1_IO0_INPUT_OFFSET, (unsigned char)data);

    /* SFP+_P1_PRESENT_L. "0" indicates Module Present. */
    if (!(data & (1 << port_to_pca9539_present_bit(port))))
    {
//debug
//printf("%s:%d port[%d], port_to_pca9539_present_bit(port)[%d]\n", __FUNCTION__, __LINE__,
//             port, port_to_pca9539_present_bit(port));

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
stx60d0_10g_sfp_plus_present(int port)
{
    int ret = 0;
    char data = 0;

    DIAG_PRINT("%s, port:%d", __FUNCTION__, port);

    int busid = TCA6424_NUM0_I2C_BUS_ID;
    int addr = TCA6424_NUM0_I2C_ADDR;
    int offset = port_to_tca6424_sfp_offset(port, PCA9539_IO_INPUT);

    ret = i2c_read_byte(busid, addr, offset, &data);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

//debug
//printf("%s:%d port[%d],busid[%d],reg[%02x],read_byte[%02x]\n", __FUNCTION__, __LINE__,
//             port, PCA9539_NUM1_I2C_BUS_ID, PCA9539_NUM1_IO0_INPUT_OFFSET, (unsigned char)data);

    if (DEBUG)
        AIM_LOG_INFO("%s:%d port[%d],busid[%d],reg[%02x],read_byte[%02x]\n", __FUNCTION__, __LINE__,
                     port, PCA9539_NUM1_I2C_BUS_ID, PCA9539_NUM1_IO0_INPUT_OFFSET, (unsigned char)data);

    /* SFP+_P1_PRESENT_L. "0" indicates Module Present. */
    if (!(data & (1 << port_to_tca6424_present_bit(port))))
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
stx60d0_sfp_pca9539_direction_set(int port)
{
    int offset = 0;
    int pca9539_bus = port_to_pca9539_busid(port);
    int addr = port_to_pca9539_addr(port);
    char data = 0;
    int ret = 0;
    int IO_port = 0;

    for (IO_port = 0; IO_port < NUM_OF_PCA9539_IO_PORT; IO_port++)
    {
        if (IO_port == 0)
            offset = PCA9539_NUM1_IO0_DIRECTION_OFFSET;
        else
            offset = PCA9539_NUM1_IO1_DIRECTION_OFFSET;
        
        data = onlp_i2c_readb(pca9539_bus, addr, offset, ONLP_I2C_F_FORCE);

        /* configuration direction to output, input:1, output:0.  */
        data &= ~(1 << PCA9539_NUM1_IO_SFP_P1_TXDIS_BIT);
        data &= ~(1 << PCA9539_NUM1_IO_SFP_P2_TXDIS_BIT);

        ret = onlp_i2c_writeb(pca9539_bus, addr, offset, data, ONLP_I2C_F_FORCE);
        if (ret < 0)
        {
            AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
            return ret;
        }
    }

    return 0;      
}


static int
stx60d0_sfp_tca6424_direction_set(int port)
{
    int offset = 0;
    int bus = TCA6424_NUM0_I2C_BUS_ID;
    int addr = TCA6424_NUM0_I2C_ADDR;
    char data = 0;
    int ret = 0;
    int IO_port = 0;

    for (IO_port = 0; IO_port < NUM_OF_TCA6424_IO_PORT; IO_port++)
    {
        switch (IO_port)
        {
            case 0:
                offset = TCA6424_NUM0_IO0_DIRECTION_OFFSET;
                break;

            case 1:
                offset = TCA6424_NUM0_IO1_DIRECTION_OFFSET;
                break;

            case 2:
                offset = TCA6424_NUM0_IO2_DIRECTION_OFFSET;
                break;
               
            default:
                break;        
        }
        
        data = onlp_i2c_readb(bus, addr, offset, ONLP_I2C_F_FORCE);

    /* SFP+_P2_TXFAULT and SFP+_P2_TXDIS are swapped. */
    /* reference to STX-60D0-126F_HW_Spec_V01_20200320B.docx Note 3 table. */
#if 1 
        switch (IO_port)
        {
            case 0:
            case 1:
	        /* configure direction to output, input:1, output:0.  */
	        data &= ~(1 << TCA6424_NUM0_IO_SFP_P1_TXDIS_BIT);
	        data &= ~(1 << TCA6424_NUM0_IO_SFP_P2_TXDIS_BIT);
                break;

            case 2:
	        data &= ~(1 << TCA6424_NUM0_IO_SFP_P1_TXDIS_BIT);
	        data &= ~(1 << TCA6424_NUM0_IO2_SFP_P2_TXDIS_BIT);
                break;
               
            default:
                break;        
        }	
#else
        /* configuration direction to output, input:1, output:0.  */
        data &= ~(1 << TCA6424_NUM0_IO_SFP_P1_TXDIS_BIT);
        data &= ~(1 << TCA6424_NUM0_IO_SFP_P2_TXDIS_BIT);
#endif

        ret = onlp_i2c_writeb(bus, addr, offset, data, ONLP_I2C_F_FORCE);
        if (ret < 0)
        {
            AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
            return ret;
        }
    }

    return 0;      
}


int
stx60d0_1g_sfp_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv = ONLP_STATUS_OK;
    int pca9539_bus = port_to_pca9539_busid(port);
    int addr = port_to_pca9539_addr(port);
    int offset = 0;
    int supported = 0;
    char optval = 0;
    int port_bit = port_to_pca9539_txdis_bit(port);

    if ((onlp_sfpi_control_supported(port, control, &supported) == ONLP_STATUS_OK) && (supported == 0))
        return ONLP_STATUS_E_UNSUPPORTED;

    DIAG_PRINT("%s, port:%d, control:%d(%s), value:0x%X", __FUNCTION__, port, control, sfp_control_to_str(control), value);

    int ret = 0;

    /* config pca9539#1 IO port direction */
    ret = stx60d0_sfp_pca9539_direction_set(port);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    switch (control)
    {
        /* TXDIS is Output direction!!!! */
        case ONLP_SFP_CONTROL_TX_DISABLE:
            offset = port_to_pca9539_sfp_offset(port, PCA9539_IO_OUTPUT);
            break;
        default:
            return ONLP_STATUS_E_UNSUPPORTED;
            break;
    }

    optval = onlp_i2c_readb(pca9539_bus, addr, offset, ONLP_I2C_F_FORCE);
//debug
//printf("%s:%d port[%d],busid[%d],reg[%02x],read_byte[%02x]\n", __FUNCTION__, __LINE__,
//             port, pca9539_bus, offset, (unsigned char)data);
    
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

    DIAG_PRINT("%s, port_bit:%d, pca9539_bus:%d, addr:0x%X, offset:0x%X", __FUNCTION__, port_bit, pca9539_bus, addr, offset);
    return rv;
  
}

int
stx60d0_10g_sfp_plus_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv = ONLP_STATUS_OK;
    int offset = 0;
    int bus = TCA6424_NUM0_I2C_BUS_ID;
    int addr = TCA6424_NUM0_I2C_ADDR;    
    int supported = 0;
    char optval = 0;
    int port_bit = port_to_tca6424_txdis_bit(port);    

    if ((onlp_sfpi_control_supported(port, control, &supported) == ONLP_STATUS_OK) && (supported == 0))
        return ONLP_STATUS_E_UNSUPPORTED;

    DIAG_PRINT("%s, port:%d, control:%d(%s), value:0x%X", __FUNCTION__, port, control, sfp_control_to_str(control), value);

    int ret = 0;

    /* config pca9539#1 IO port direction */
    ret = stx60d0_sfp_tca6424_direction_set(port);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    switch (control)
    {
        /* TXDIS is Output direction!!!! */
        case ONLP_SFP_CONTROL_TX_DISABLE:
            offset = port_to_tca6424_sfp_offset(port, PCA9539_IO_OUTPUT);
            break;
        default:
            return ONLP_STATUS_E_UNSUPPORTED;
            break;
    }

    optval = onlp_i2c_readb(bus, addr, offset, ONLP_I2C_F_FORCE);
//debug
//printf("%s:%d port[%d],tca6424_bus[%d],reg[%02x],read_byte[%02x]\n", __FUNCTION__, __LINE__,
//             port, bus, offset, (unsigned char)optval);
    
    if (value != 0)
    {
        optval |= (1 << port_bit);
    }
    else
    {
        optval &= ~(1 << port_bit);
    }

    rv = onlp_i2c_writeb(bus, addr, offset, optval, ONLP_I2C_F_FORCE);
    if (rv < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, rv);
        return rv;
    }

    DIAG_PRINT("%s, port_bit:%d, tca6424_bus:%d, addr:0x%X, offset:0x%X, optval:0x%X", __FUNCTION__, port_bit, bus, addr, offset, (unsigned char)optval);

    return rv;
  
}

int
stx60d0_1g_sfp_control_get(int port, onlp_sfp_control_t control, int *value)
{
    int rv = ONLP_STATUS_OK;
    int pca9539_bus = port_to_pca9539_busid(port);
    int addr = port_to_pca9539_addr(port);
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
            offset = port_to_pca9539_sfp_offset(port, PCA9539_IO_INPUT);               
            port_bit = port_to_pca9539_rxlos_bit(port);
            break;
            
        case ONLP_SFP_CONTROL_TX_FAULT:
            offset = port_to_pca9539_sfp_offset(port, PCA9539_IO_INPUT);
            port_bit = port_to_pca9539_txfault_bit(port);
            break;
            
        case ONLP_SFP_CONTROL_TX_DISABLE:
            offset = port_to_pca9539_sfp_offset(port, PCA9539_IO_INPUT);
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
stx60d0_10g_sfp_plus_control_get(int port, onlp_sfp_control_t control, int *value)
{
    int rv = ONLP_STATUS_OK;
    int offset = 0;
    int bus = TCA6424_NUM0_I2C_BUS_ID;
    int addr = TCA6424_NUM0_I2C_ADDR;    
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
            offset = port_to_tca6424_sfp_offset(port, PCA9539_IO_INPUT);               
            port_bit = port_to_tca6424_rxlos_bit(port);
            break;
            
        case ONLP_SFP_CONTROL_TX_FAULT:
            offset = port_to_tca6424_sfp_offset(port, PCA9539_IO_INPUT);
            port_bit = port_to_tca6424_txfault_bit(port);
            break;
            
        case ONLP_SFP_CONTROL_TX_DISABLE:
            offset = port_to_tca6424_sfp_offset(port, PCA9539_IO_INPUT);
            port_bit = port_to_tca6424_txdis_bit(port);
            break;
            
        default:
            return ONLP_STATUS_E_UNSUPPORTED;
            break;
    }
    
    optval = onlp_i2c_readb(bus, addr, offset, ONLP_I2C_F_FORCE);
    if ((optval & (1 << port_bit)) != 0) //1
    {
        *value = 1;
    }
    else
    {
        *value = 0;
    }
    
    DIAG_PRINT("%s, port_bit:%d, tca6424_busid:%d, addr:0x%X, offset:0x%X", __FUNCTION__, port_bit, bus, addr, offset);
    DIAG_PRINT("%s, port:%d, control:%d(%s), value:0x%X", __FUNCTION__, port, control, sfp_control_to_str(control), *value);

    return rv;

}

static int
stx60d0_sfp_init(void)
{
    DIAG_PRINT("%s", __FUNCTION__);

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

    if (port >= SFP_PORT_E_10G_SFP_PLUS_P1)
        present = stx60d0_10g_sfp_plus_present(port);
    else         
        present = stx60d0_1g_sfp_present(port);

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

    if (port >= SFP_PORT_E_10G_SFP_PLUS_P1)
        rv = stx60d0_10g_sfp_plus_control_set(port, control, value);
    else         
        rv = stx60d0_1g_sfp_control_set(port, control, value);

    return rv;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int *value)
{
    int rv = ONLP_STATUS_OK;

    if (port >= SFP_PORT_E_10G_SFP_PLUS_P1)
        rv = stx60d0_10g_sfp_plus_control_get(port, control, value);
    else         
        rv = stx60d0_1g_sfp_control_get(port, control, value);

    return rv;
}

int
onlp_sfpi_denit(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
    return ONLP_STATUS_OK;
}

