/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
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
#include <unistd.h>
#include <fcntl.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/sfpi.h>
#include <onlplib/file.h>
#include "platform_lib.h"
#include "x86_64_accton_dcg8510_32d_log.h"

#define NUM_OF_QSFP_PORT 32
 
/* Bit value */
#define BIT0            0x0001
#define BIT1            0x0002
#define BIT2            0x0004
#define BIT3            0x0008
#define BIT4            0x0010
#define BIT5            0x0020
#define BIT6            0x0040
#define BIT7            0x0080
#define BIT8            0x0100
#define BIT9            0x0200
#define BIT10           0x0400
#define BIT11           0x0800
#define BIT12           0x1000
#define BIT13           0x2000
#define BIT14           0x4000
#define BIT15           0x8000

#define SFP_PATH    "/sys/switch/transceiver/"
static int port_id_to_eth_id(int port)
{
    return port+1;
}

typedef struct port_info_s
{
    int     port;
    uint8_t address;
    uint8_t chan;
    uint16_t bit;
} port_info_t;

typedef struct bitmap_info_s
{
    uint16_t bit;
    int     port;
} bitmap_info_t;

  
uint64_t g_present_port_val;
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

    for(p = 0; p < NUM_OF_QSFP_PORT; p++)
    {
        AIM_BITMAP_SET(bmap, p);
    }

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

    char path[128] = {0};
    int  value = 0;
    ///sys_switch/transceiver/eth[n]/present
    sprintf(path, "%seth%d/present", SFP_PATH, port_id_to_eth_id(port));
    if (0 > onlp_file_read_int(&value, path ))
        return ONLP_STATUS_E_INTERNAL;
    return value;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{    
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    int size;
    char path[128] = {0};
    if(port <0 || port >= NUM_OF_QSFP_PORT)
        return ONLP_STATUS_E_INTERNAL;

    /* get data of eeprom */
    //sys_switch/transceiver/eth[n]/eeprom
    sprintf(path, "%seth%d/eeprom", SFP_PATH, port_id_to_eth_id(port));

    if(onlp_file_read(data, 256, &size, path) == ONLP_STATUS_OK) {
         if(size == 256) {
            return ONLP_STATUS_OK;
        }
    }
    return ONLP_STATUS_E_INTERNAL;
    
}

 


int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int value;
    onlp_file_readb(&value,addr, "%seth%d/eeprom", SFP_PATH, port_id_to_eth_id(port));
    return value;
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    return onlp_file_writeb(value,addr, "%seth%d/eeprom", SFP_PATH, port_id_to_eth_id(port));
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int value;
    onlp_file_readw(&value,addr, "%seth%d/eeprom", SFP_PATH, port_id_to_eth_id(port));
    return value;
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    return onlp_file_writew(value,addr, "%seth%d/eeprom", SFP_PATH, port_id_to_eth_id(port));
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{    
    int rv;
    char path[128] = {0};
    rv = ONLP_STATUS_OK;   

    switch(control)
    {
        case ONLP_SFP_CONTROL_RESET:
            {
                sprintf(path, "%seth%d/reset", SFP_PATH, port_id_to_eth_id(port));                
                if (0 > onlp_file_write_int(value, path))
                    rv= ONLP_STATUS_E_INTERNAL;                
                break;
            }
        case ONLP_SFP_CONTROL_LP_MODE:
            {
                sprintf(path, "%seth%d/lpmode", SFP_PATH, port_id_to_eth_id(port));
                if (0 > onlp_file_write_int(value, path))
                    rv= ONLP_STATUS_E_INTERNAL;                
                break;
            }                     
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                sprintf(path, "%seth%d/tx_disable", SFP_PATH, port_id_to_eth_id(port));
                if (0 > onlp_file_write_hex(value, path))
                    rv= ONLP_STATUS_E_INTERNAL;
                break;
            }
        default:
            rv = ONLP_STATUS_E_UNSUPPORTED;
            break;
    }

    return rv;
}


int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int rv=ONLP_STATUS_E_UNSUPPORTED;
    char path[128] = {0};
    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                rv=ONLP_STATUS_OK;
                //sys_switch/transceiver/eth[n]/tx_disable
                sprintf(path, "%seth%d/tx_disable", SFP_PATH, port_id_to_eth_id(port));
                if (0 > onlp_file_read_hex(value, path ))
                    rv= ONLP_STATUS_E_INTERNAL;
                break;
            }
        /*
        case ONLP_SFP_CONTROL_TX_FAULT:
            {
                rv=ONLP_STATUS_OK;
                //sys_switch/transceiver/eth[n]/tx_fault
                sprintf(path, "%seth%d/tx_fault", SFP_PATH, port_id_to_eth_id(port));
                if (0 > onlp_file_read_hex(value, path ))
                    rv= ONLP_STATUS_E_INTERNAL;
                break;
            } 
        */             
       /*    
        case ONLP_SFP_CONTROL_RX_LOS:
            {
                rv=ONLP_STATUS_OK;
                //sys_switch/transceiver/eth[n]/rx_los
                sprintf(path, "%seth%d/rx_los", SFP_PATH, port_id_to_eth_id(port));
                if (0 > onlp_file_read_hex(value, path ))
                    rv= ONLP_STATUS_E_INTERNAL;
                break;
            }  
        */  
        case ONLP_SFP_CONTROL_RESET:
            {
                rv=ONLP_STATUS_OK;
                sprintf(path, "%seth%d/reset", SFP_PATH, port_id_to_eth_id(port));
                if (0 > onlp_file_read_int(value, path ))
                    rv= ONLP_STATUS_E_INTERNAL;
                break;
            } 
        case ONLP_SFP_CONTROL_LP_MODE:
            {
                rv=ONLP_STATUS_OK;
                // /sys_switch/transceiver/eth[n]/lpmode
                sprintf(path, "%seth%d/lpmode", SFP_PATH, port_id_to_eth_id(port));
                if (0 > onlp_file_read_int(value, path ))
                    rv= ONLP_STATUS_E_INTERNAL;
                break;
            }
        default:
            rv = ONLP_STATUS_E_UNSUPPORTED;
            break;
        }

    return rv;
}

int
onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv)
{
    //@param rv [out] Receives 1 if supported, 0 if not supported.
    switch(control)
    {
        case ONLP_SFP_CONTROL_RESET:
        case ONLP_SFP_CONTROL_LP_MODE:
        case ONLP_SFP_CONTROL_TX_DISABLE:
            *rv=1;
            break;
        default:
            *rv = 0;
            break;
    }
    return ONLP_STATUS_OK;
}


int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
