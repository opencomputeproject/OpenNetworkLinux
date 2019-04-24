/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
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
#include <onlplib/file.h>
#include "x86_64_accton_asgvolt64_int.h"
#include "x86_64_accton_asgvolt64_log.h"

#define PORT_EEPROM_FORMAT              "/sys/bus/i2c/devices/%d-0050/eeprom"
#define MODULE_PRESENT_FORMAT		    "/sys/bus/i2c/devices/%d-00%d/module_present_%d"
#define MODULE_RXLOS_FORMAT             "/sys/bus/i2c/devices/%d-00%d/module_rx_los_%d"
#define MODULE_TXFAULT_FORMAT           "/sys/bus/i2c/devices/%d-00%d/module_tx_fault_%d"
#define MODULE_TXDISABLE_FORMAT         "/sys/bus/i2c/devices/%d-00%d/module_tx_disable_%d"

int sfp_map_bus[] ={
    41,42,43,44,45,46,47,48,
    49,50,51,52,53,54,55,56,
    57,58,59,60,61,62,63,64,
    65,66,67,68,69,70,71,72,
    73,74,75,76,77,78,79,80,
    81,82,83,84,85,86,87,88,
    89,90,91,92,93,94,95,96,
    97,98,99,100,101,102,103,104,
    20, 21,
    25, 26, 27, 28, 29, 30, 31, 32,};

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
onlp_sfpi_map_bus_index(int port)
{
    if(port < 0 || port >=74)
        return ONLP_STATUS_E_INTERNAL;
    return sfp_map_bus[port];
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    /*
     * Ports {0, 73}
     */
    int p;

    for(p = 0; p < 74; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}
int onlp_sfpi_get_cpld_addr(int port, int * addr, int *bus)
{
    
    if(port <0 || port >= 74)
        return ONLP_STATUS_E_INTERNAL;
   
    /*gpon*/
    *addr=60;
    *bus=9;  
    if(port >=0 && port <=25)
    {
        if(port ==22 || port ==23 || port==18 || port==19)
        {
            *addr = 61;
            *bus  = 10;
        }
        else
        { 
            *addr = 60;
            *bus  = 9;
        }
    }
    if(port >=26 && port <=47)
    {
        if(port ==34 || port==35 || port==38 || port==39 || port==42)
        {
            *addr = 62;
            *bus  = 11;
        }
        else
        {
            *addr = 61;
            *bus  = 10;
        }
    }
    if(port >=48 && port <=63)
    {
         *addr = 62;
         *bus  = 11;
    }
    /*QSFP*/
    if(port >=64 && port <=65)
    {
         *addr = 60;
         *bus  = 9;
    }
    /*SFP*/
    if(port >=66 && port <=73)
    {
         *addr = 61;
         *bus  = 10;
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
    int present;
    int bus=9, addr=0x60;
    
    if(port <0 || port >= 74)
        return ONLP_STATUS_E_INTERNAL;
    
    if(onlp_sfpi_get_cpld_addr(port, &addr, &bus)!=ONLP_STATUS_OK)
        return ONLP_STATUS_E_INTERNAL;
    
    
	if (onlp_file_read_int(&present, MODULE_PRESENT_FORMAT, bus, addr, (port+1)) < 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    
    
    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int i=0, val=0;
   
    /* Populate bitmap */
    for(i = 0; i<74; i++) {
        val=0;
        if(i >=66 && i <=73)
        {
            if (onlp_file_read_int(&val, MODULE_RXLOS_FORMAT, 10, 61, i+1) < 0)
            {
                AIM_LOG_ERROR("Unable to read rx_loss status from port(%d)\r\n", i);
            }
    
            if(val)
                AIM_BITMAP_MOD(dst, i, 1);
            else
                AIM_BITMAP_MOD(dst, i, 0);
        }
        else
            AIM_BITMAP_MOD(dst, i, 0);
        
    }
    
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    int size = 0;
    if(port <0 || port >= 74)
        return ONLP_STATUS_E_INTERNAL;
    
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

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    FILE* fp;
    char file[64] = {0};
    
    if(port < 0 || port >=74)
        return ONLP_STATUS_E_INTERNAL;
    
    sprintf(file, PORT_EEPROM_FORMAT, onlp_sfpi_map_bus_index(port));
    fp = fopen(file, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the eeprom device file of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (fseek(fp, 256, SEEK_CUR) != 0) {
        fclose(fp);
        AIM_LOG_ERROR("Unable to set the file position indicator of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    int ret = fread(data, 1, 256, fp);
    fclose(fp);
    if (ret != 256) {
        AIM_LOG_ERROR("Unable to read the module_eeprom device file of port(%d)", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = onlp_sfpi_map_bus_index(port);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int bus = onlp_sfpi_map_bus_index(port);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = onlp_sfpi_map_bus_index(port);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int bus = onlp_sfpi_map_bus_index(port);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv;
    int addr=0x60;
    int bus=9;
    
    if(port <0 || port >= 74)
        return ONLP_STATUS_E_INTERNAL;
   
    if(onlp_sfpi_get_cpld_addr(port, &addr, &bus)!=ONLP_STATUS_OK)
        return ONLP_STATUS_E_INTERNAL;

    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                if(port==64  || port == 65)
                {
                    return ONLP_STATUS_OK;
                }
               
                if (onlp_file_write_int(0, MODULE_TXDISABLE_FORMAT, bus, addr, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to set tx_disable status to port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
                }
                
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
    int rv;
    int addr=0x60;
    int bus=9;
    
    if(port <0 || port >= 74)
        return ONLP_STATUS_E_INTERNAL;
   
    if(onlp_sfpi_get_cpld_addr(port, &addr, &bus)!=ONLP_STATUS_OK)
        return ONLP_STATUS_E_INTERNAL;
    
    switch(control)
        {
        case ONLP_SFP_CONTROL_RX_LOS:
            {
                if(port<=65)
                {
                    rv = ONLP_STATUS_E_UNSUPPORTED;
                }
                else
                {
            	    if (onlp_file_read_int(value, MODULE_RXLOS_FORMAT, bus, addr, (port+1)) < 0) {
                        AIM_LOG_ERROR("Unable to read rx_loss status from port(%d)\r\n", port);
                        rv = ONLP_STATUS_E_INTERNAL;
                    }
                    else
                    {
                        rv = ONLP_STATUS_OK;
                    }
                }
                break;
            }

        case ONLP_SFP_CONTROL_TX_FAULT:
            {
                if(port==64 || port==65)
                {
                    rv = ONLP_STATUS_E_UNSUPPORTED;
                }
                else
                {
            	    if (onlp_file_read_int(value, MODULE_TXFAULT_FORMAT, bus, addr, (port+1)) < 0) {
                        AIM_LOG_ERROR("Unable to read tx_fault status from port(%d)\r\n", port);
                        rv = ONLP_STATUS_E_INTERNAL;
                    }
                    else
                    {
                        rv = ONLP_STATUS_OK;
                    }
                }
                break;
            }

        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                if(port==64 || port==65)
                {
                    rv = ONLP_STATUS_E_UNSUPPORTED;
                }
            	else
            	{
            	    if (onlp_file_read_int(value, MODULE_TXDISABLE_FORMAT, bus, addr, (port+1)) < 0)
            	    {
                        AIM_LOG_ERROR("Unable to read tx_disabled status from port(%d)\r\n", port);
                        rv = ONLP_STATUS_E_INTERNAL;
                    }
                    else
                    {
                        rv = ONLP_STATUS_OK;
                    }
                }
                break;
            }

        default:
            rv = ONLP_STATUS_E_UNSUPPORTED;
        }

    return rv;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
