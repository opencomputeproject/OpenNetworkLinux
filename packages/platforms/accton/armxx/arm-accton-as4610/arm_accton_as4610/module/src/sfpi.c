/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
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
#include <onlp/platformi/sfpi.h>
#include <onlplib/file.h>
#include "platform_lib.h"

#include <arm_accton_as4610/arm_accton_as4610_config.h>
#include "arm_accton_as4610_log.h"

#define PORT_EEPROM_FORMAT              "/sys/bus/i2c/devices/%d-0050/eeprom"
#define MODULE_PRESENT_FORMAT		    "/sys/bus/i2c/devices/0-0030/module_present_%d"
#define MODULE_RXLOS_FORMAT             "/sys/bus/i2c/devices/0-0030/module_rx_los_%d"
#define MODULE_TXFAULT_FORMAT           "/sys/bus/i2c/devices/0-0030/module_tx_fault_%d"
#define MODULE_PRESENT_ALL_ATTR_CPLD	"/sys/bus/i2c/devices/0-0030/module_present_all"
#define MODULE_RXLOS_ALL_ATTR_CPLD	    "/sys/bus/i2c/devices/0-0030/module_rx_los_all"

static int front_port_bus_index(int port)
{
    return (platform_id == PLATFORM_ID_POWERPC_ACCTON_AS4610_30_R0) ? 
           (port - 22) : /* PLATFORM_ID_POWERPC_ACCTON_AS4610_30_R0 */
           (port - 46) ; /* PLATFORM_ID_POWERPC_ACCTON_AS4610_54_R0 */
}

static int front_port_to_cpld_port(int port)
{
    return (platform_id == PLATFORM_ID_POWERPC_ACCTON_AS4610_30_R0) ? 
           (port - 23) : /* PLATFORM_ID_POWERPC_ACCTON_AS4610_30_R0 */
           (port - 47) ; /* PLATFORM_ID_POWERPC_ACCTON_AS4610_54_R0 */
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
    int p;
    int start_port, end_port;

    if(platform_id == PLATFORM_ID_POWERPC_ACCTON_AS4610_30_R0)
    {
        start_port = 24;
        end_port   = 30;
    }
    else /*PLATFORM_ID_POWERPC_ACCTON_AS4610_54_R0*/
    {
        start_port = 48;
        end_port   = 54;
    }

    for(p = start_port; p < end_port; p++) {
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
    int cpld_port = front_port_to_cpld_port(port);
    
	if (onlp_file_read_int(&present, MODULE_PRESENT_FORMAT, cpld_port) < 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t byte;
    FILE* fp;
    int   port;

    AIM_BITMAP_CLR_ALL(dst);

    if(platform_id == PLATFORM_ID_POWERPC_ACCTON_AS4610_30_R0)
        port = 24;
    else /*PLATFORM_ID_POWERPC_ACCTON_AS4610_54_R0*/
        port = 48;

    /* Read present status of each port */
    fp = fopen(MODULE_PRESENT_ALL_ATTR_CPLD, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the module_present_all device file of CPLD.");
        return ONLP_STATUS_E_INTERNAL;
    }

    int count = fscanf(fp, "%x", &byte);
    fclose(fp);
    if(count != 1) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields the module_present_all device file of CPLD.");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t presence_all = byte;
    presence_all <<= port;

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
    uint32_t byte;
    FILE* fp;
    int   port;

    if(platform_id == PLATFORM_ID_POWERPC_ACCTON_AS4610_30_R0)
        port = 24;
    else /*PLATFORM_ID_POWERPC_ACCTON_AS4610_54_R0*/
        port = 48;

    /* Read present status of each port */
    fp = fopen(MODULE_RXLOS_ALL_ATTR_CPLD, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the module_rx_los_all device file of CPLD.");
        return ONLP_STATUS_E_INTERNAL;
    }

    int count = fscanf(fp, "%x", &byte);
    fclose(fp);
    if(count != 1) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields the module_rx_los_all device file of CPLD.");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint64_t rx_los_all = byte;
    rx_los_all <<= port;

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

	if(onlp_file_read(data, 256, &size, PORT_EEPROM_FORMAT, front_port_bus_index(port)) != ONLP_STATUS_OK) {
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
    
    sprintf(file, PORT_EEPROM_FORMAT, front_port_bus_index(port));
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
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int rv;
    int cpld_port = front_port_to_cpld_port(port);

    if (cpld_port > 4) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch(control)
        {
        case ONLP_SFP_CONTROL_RX_LOS:
            {
            	if (onlp_file_read_int(value, MODULE_RXLOS_FORMAT, cpld_port) < 0) {
                    AIM_LOG_ERROR("Unable to read rx_los status from port(%d)\r\n", port);
                    rv = ONLP_STATUS_E_INTERNAL;
                }
                else {
                    rv = ONLP_STATUS_OK;
                }
                break;
            }

        case ONLP_SFP_CONTROL_TX_FAULT:
            {
            	if (onlp_file_read_int(value, MODULE_TXFAULT_FORMAT, cpld_port) < 0) {
                    AIM_LOG_ERROR("Unable to read tx_fault status from port(%d)\r\n", port);
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

