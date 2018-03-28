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
#include "x86_64_accton_as7326_56x_int.h"
#include "x86_64_accton_as7326_56x_log.h"

#define PORT_BUS_INDEX(port) sfp_map[port]

#define PORT_EEPROM_FORMAT              "/sys/bus/i2c/devices/%d-0050/eeprom"
#define MODULE_PRESENT_FORMAT		    "/sys/bus/i2c/devices/%d-00%d/module_present_%d"
#define MODULE_RXLOS_FORMAT             "/sys/bus/i2c/devices/%d-00%d/module_rx_los_%d"
#define MODULE_TXFAULT_FORMAT           "/sys/bus/i2c/devices/%d-00%d/module_tx_fault_%d"
#define MODULE_TXDISABLE_FORMAT         "/sys/bus/i2c/devices/%d-00%d/module_tx_disable_%d"
#define MODULE_PRESENT_ALL_ATTR	        "/sys/bus/i2c/devices/%d-00%d/module_present_all"
#define MODULE_RXLOS_ALL_ATTR_CPLD1	    "/sys/bus/i2c/devices/18-0060/module_rx_los_all"
#define MODULE_RXLOS_ALL_ATTR_CPLD2	    "/sys/bus/i2c/devices/12-0062/module_rx_los_all"

const int sfp_map[] =  {
        42,41,44,43,47,45,46,50,
        48,49,52,51,53,56,55,54,
        58,57,60,59,61,63,62,64,
        66,68,65,67,69,71,72,70,
        74,73,76,75,77,79,78,80,
        81,82,84,85,83,87,88,86,    /*port 41~48*/
        25,26,27,28,29,30,31,32,    /*port 49~56 QSFP*/
        22,23};                      /*port 57~58 SFP+ from CPU NIF.*/


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
     * Ports {0, 58}
     */
    int p;

    for(p = 0; p < 58; p++) {
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
    int present;
    int bus, addr;

    addr = (port < 30) ? 62 : 60;
    bus  = (addr == 62) ? 12 : 18;
    
	if (onlp_file_read_int(&present, MODULE_PRESENT_FORMAT, bus, addr, (port+1)) < 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint8_t bytes[8];
    uint32_t *words[2];
    FILE* fp;
    int addr, i;
    uint8_t *ptr = bytes;
    unsigned long long presence_all = 0, per_cpld;

    for (addr = 62; addr >= 60; addr-=2) {
        /* Read present status of port 0~53 */
        char file[64] = {0};
        int bus = (addr == 62) ? 12 : 18;
        
        sprintf(file, MODULE_PRESENT_ALL_ATTR, bus, addr);
        fp = fopen(file, "r");
        if(fp == NULL) {
            AIM_LOG_ERROR("Unable to open the module_present_all device file of CPLD3.");
            return ONLP_STATUS_E_INTERNAL;
        }

        int count = fscanf(fp, "%hhx %hhx %hhx %hhx", ptr+0, ptr+1, ptr+2, ptr+3);
        fclose(fp);
        if(count != 4) {
            /* Likely a CPLD read timeout. */
            AIM_LOG_ERROR("Unable to read all fields the module_present_all device file of CPLD3.");
            return ONLP_STATUS_E_INTERNAL;
        }

        ptr += count;
    }

    /*Presumed here are little-endian.*/
    words[0] = (uint32_t*)&bytes[0];
    words[1] = (uint32_t*)&bytes[4];

    /* Convert to 64 bit integer in port order */
    presence_all |= *words[0] & ((1<<30)-1);    /*30 port at cpld 2*/
    per_cpld = (unsigned long long)(*words[1] & ((1<<28)-1)) << 30; /*28 port at cpld 1*/
    presence_all |= per_cpld;

    /* Populate bitmap */
    for(i = 0; presence_all; i++) {
        AIM_BITMAP_MOD(dst, i, (presence_all & 1));
        presence_all >>= 1;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint8_t bytes[8];
    uint32_t *words[2];
    uint8_t *ptr = bytes;
    FILE* fp;
    unsigned long long all = 0, per_cpld;

    /* Read present status of port 0~23 */
    int addr, i = 0;

    for (addr = 62; addr >= 60; addr-=2) {
        if (addr == 62) {
            fp = fopen(MODULE_RXLOS_ALL_ATTR_CPLD2, "r");
        }
        else {
            fp = fopen(MODULE_RXLOS_ALL_ATTR_CPLD1, "r");
        }

        if(fp == NULL) {
            AIM_LOG_ERROR("Unable to open the module_rx_los_all device file of CPLD(0x%d)", addr);
            return ONLP_STATUS_E_INTERNAL;
        }

        int count = fscanf(fp, "%hhx %hhx %hhx %hhx", ptr+0, ptr+1, ptr+2, ptr+3);
        fclose(fp);
        if(count != 4) {
            /* Likely a CPLD read timeout. */
            AIM_LOG_ERROR("Unable to read all fields from the module_rx_los_all device file of CPLD(0x%d)", addr);
            return ONLP_STATUS_E_INTERNAL;
        }

        ptr += count;
    }
    /*Presumed here are little-endian.*/
    words[0] = (uint32_t*)&bytes[0];
    words[1] = (uint32_t*)&bytes[4];

    /* Convert to 64 bit integer in port order */
    all |= *words[0] & ((1<<30)-1);    /*30 port at cpld 2*/
    per_cpld = (unsigned long long)(*words[1] & ((1<<28)-1)) << 30; /*28 port at cpld 1*/
    all |= per_cpld;

    /* Populate bitmap */
    for(i = 0; all; i++) {
        AIM_BITMAP_MOD(dst, i, (all & 1));
        all >>= 1;
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
    memset(data, 0, 256);

	if(onlp_file_read(data, 256, &size, PORT_EEPROM_FORMAT, PORT_BUS_INDEX(port)) != ONLP_STATUS_OK) {
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
    
    sprintf(file, PORT_EEPROM_FORMAT, PORT_BUS_INDEX(port));
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
    int bus = PORT_BUS_INDEX(port);
    return onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int bus = PORT_BUS_INDEX(port);
    return onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = PORT_BUS_INDEX(port);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int bus = PORT_BUS_INDEX(port);
    return onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv;

    if (port < 0 || port >= 48) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    int addr = (port < 30) ? 62 : 60;
    int bus  = (addr == 62) ? 12 : 18;

    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                if (onlp_file_write_int(value, MODULE_TXDISABLE_FORMAT, bus, addr, (port+1)) < 0) {
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

    if (port < 0 || port >= 48) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    int addr = (port < 30) ? 62 : 60;
    int bus  = (addr == 62) ? 12 : 18;

    switch(control)
        {
        case ONLP_SFP_CONTROL_RX_LOS:
            {
            	if (onlp_file_read_int(value, MODULE_RXLOS_FORMAT, bus, addr, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to read rx_loss status from port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
                }
                break;
            }

        case ONLP_SFP_CONTROL_TX_FAULT:
            {
            	if (onlp_file_read_int(value, MODULE_TXFAULT_FORMAT, bus, addr, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to read tx_fault status from port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
                }
                break;
            }

        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
            	if (onlp_file_read_int(value, MODULE_TXDISABLE_FORMAT, bus, addr, (port+1)) < 0) {
                    AIM_LOG_ERROR("Unable to read tx_disabled status from port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
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

