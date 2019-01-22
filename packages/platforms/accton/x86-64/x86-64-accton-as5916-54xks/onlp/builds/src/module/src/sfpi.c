/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 Accton Technology Corporation.
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
#include "x86_64_accton_as5916_54xks_int.h"
#include "x86_64_accton_as5916_54xks_log.h"

#define PORT_EEPROM_FORMAT              "/sys/devices/platform/as5916_54xks_sfp/module_eeprom_%d"
#define MODULE_PRESENT_FORMAT		    "/sys/devices/platform/as5916_54xks_sfp/module_present_%d"
#define MODULE_RXLOS_FORMAT             "/sys/devices/platform/as5916_54xks_sfp/module_rx_los_%d"
#define MODULE_TXFAULT_FORMAT           "/sys/devices/platform/as5916_54xks_sfp/module_tx_fault_%d"
#define MODULE_TXDISABLE_FORMAT         "/sys/devices/platform/as5916_54xks_sfp/module_tx_disable_%d"
#define MODULE_PRESENT_ALL_ATTR	        "/sys/devices/platform/as5916_54xks_sfp/module_present_all"
#define MODULE_RXLOS_ALL_ATTR           "/sys/devices/platform/as5916_54xks_sfp/module_rxlos_all"

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
     * Ports {0, 54}
     */
    int p;

    for(p = 0; p < 54; p++) {
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

	if (onlp_file_read_int(&present, MODULE_PRESENT_FORMAT, (port+1)) < 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t bytes[7];
    FILE* fp;

    /* Read present status of port 0~54 */
    int count = 0;

    fp = fopen(MODULE_PRESENT_ALL_ATTR, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the module_present_all device file from (%s).", MODULE_PRESENT_ALL_ATTR);
        return ONLP_STATUS_E_INTERNAL;
    }

    count = fscanf(fp, "%x %x %x %x %x %x %x", bytes+0, bytes+1, bytes+2, bytes+3,
                                               bytes+4, bytes+5, bytes+6);
    fclose(fp);
    if(count != 7) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields the module_present_all device file from(%s).", MODULE_PRESENT_ALL_ATTR);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Mask out non-existant QSFP ports */
    bytes[6] &= 0x3F;

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t presence_all = 0 ;
    for(i = AIM_ARRAYSIZE(bytes)-1; i >= 0; i--) {
        presence_all <<= 8;
        presence_all |= bytes[i];
    }

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
    uint32_t bytes[6];
    FILE* fp;

    /* Read present status of port 0~25 */
    int count = 0;

    fp = fopen(MODULE_RXLOS_ALL_ATTR, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the module_rxlos_all device file from (%s).", MODULE_RXLOS_ALL_ATTR);
        return ONLP_STATUS_E_INTERNAL;
    }

    count = fscanf(fp, "%x %x %x %x %x %x", bytes+0, bytes+1, bytes+2,
                                            bytes+3, bytes+4, bytes+5);
    fclose(fp);
    if(count != 6) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields the module_rxlos_all device file from(%s).", MODULE_RXLOS_ALL_ATTR);
        return ONLP_STATUS_E_INTERNAL;
    }    

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t rx_los_all = 0 ;
    for(i = AIM_ARRAYSIZE(bytes)-1; i >= 0; i--) {
        rx_los_all <<= 8;
        rx_los_all |= bytes[i];
    }

    /* Populate bitmap */
    for(i = 0; rx_los_all; i++) {
        AIM_BITMAP_MOD(dst, i, (rx_los_all & 1));
        rx_los_all >>= 1;
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

	if(onlp_file_read(data, 256, &size, PORT_EEPROM_FORMAT, (port+1)) != ONLP_STATUS_OK) {
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
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv;

    switch(control)
        {
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                if (onlp_file_write_int(value, MODULE_TXDISABLE_FORMAT, (port+1)) < 0) {
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

    if (port < 0) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch(control)
        {
        case ONLP_SFP_CONTROL_RX_LOS:
            {
                if (port >= 48) {
                    return ONLP_STATUS_E_UNSUPPORTED;
                }
                
            	if (onlp_file_read_int(value, MODULE_RXLOS_FORMAT, (port+1)) < 0) {
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
                if (port >= 48) {
                    return ONLP_STATUS_E_UNSUPPORTED;
                }
                
            	if (onlp_file_read_int(value, MODULE_TXFAULT_FORMAT, (port+1)) < 0) {
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
            	if (onlp_file_read_int(value, MODULE_TXDISABLE_FORMAT, (port+1)) < 0) {
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

