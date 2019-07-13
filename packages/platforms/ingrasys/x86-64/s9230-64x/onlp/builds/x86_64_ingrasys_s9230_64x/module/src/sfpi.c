/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
 ***********************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <onlplib/sfp.h>
#include <onlplib/file.h>
#include <onlp/platformi/sfpi.h>
#include <x86_64_ingrasys_s9230_64x/x86_64_ingrasys_s9230_64x_config.h>
#include "x86_64_ingrasys_s9230_64x_log.h"
#include "platform_lib.h"

static int _fp2phy_port_mapping[64] = {
    1, 2, 5, 6, 9, 10, 13, 14, 17, 18, 21, 22, 25, 26, 29, 30, 
    33, 34, 37, 38, 41, 42, 45, 46, 49, 50, 53, 54, 57, 58, 61, 62,
    3, 4, 7, 8, 11, 12, 15, 16, 19, 20, 23, 24, 27, 28, 31, 32, 
    35, 36, 39, 40, 43, 44, 47, 48, 51, 52, 55, 56, 59, 60, 63, 64};

static void
qsfp_to_cpld_index(int phy_port, int *cpld_id, int *cpld_port_index)
{
    phy_port = phy_port  - 1;
    if (phy_port < CPLD1_PORTS) { 
        *cpld_id = 0;
        *cpld_port_index = phy_port + 1;
    } else {
        *cpld_id = 1 + (phy_port - CPLD1_PORTS) / CPLDx_PORTS;
        *cpld_port_index = ((phy_port - CPLD1_PORTS) % CPLDx_PORTS) + 1;
    }
    return;
}


int
onlp_sfpi_init(void)
{  
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;
    for(p = 1; p <= PORT_NUM; p++) {
        AIM_BITMAP_SET(bmap, p);
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    int status, phy_port;
    int i2c_id, cpld_id, cpld_port_index;
    char reg_path[128];
    int value, mask;
    uint8_t data[8];
    int data_len;

    if (port >= 1 && port <=64) {
        phy_port = _fp2phy_port_mapping[port-1];
        qsfp_to_cpld_index(phy_port, &cpld_id, &cpld_port_index);

        i2c_id = CPLD_OFFSET + cpld_id;
        mask = 1 << CPLD_QSFP_PRES_BIT;

        snprintf(reg_path, 128, CPLD_QSFP_REG_PATH, i2c_id, CPLD_REG, CPLD_QSFP_PORT_STATUS_KEY, cpld_port_index);
    } else if (port>= 65 && port <= 66) {
        cpld_port_index = 0;
        mask = 1 << CPLD_SFP_PRES_BIT;

        if (port == 65) {
            i2c_id = CPLD_OFFSET;
        } else {
            i2c_id = CPLD_OFFSET + 1;
        }

        snprintf(reg_path, 128, CPLD_SFP_REG_PATH, i2c_id, CPLD_REG, CPLD_SFP_PORT_STATUS_KEY);
    } else {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (onlp_file_read(data, sizeof(data), &data_len, reg_path) == ONLP_STATUS_OK) {
        //convert hex string to integer
        value = (int) strtol ((char *) data, NULL, 16);

        if ( (value & mask) == 0) {
            status = 1;
        } else {
            status = 0;
        }
    } else {
        return ONLP_STATUS_E_INTERNAL;
    }

    return status;
}


int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int p = 1;
    int rc = 0;

    for (p = 1; p <= PORT_NUM; p++) {
        rc = onlp_sfpi_is_present(p);
        AIM_BITMAP_MOD(dst, p, (1 == rc) ? 1 : 0);
    }

    return ONLP_STATUS_OK;
}

/*
 * This function reads the SFPs idrom and returns in
 * in the data buffer provided.
 */
int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{   
    int eeprombus=0, eeprombusbase=0, phy_port=0, port_group=0, eeprombusshift=0;
    char eeprom_path[512], eeprom_addr[32];
    memset(eeprom_path, 0, sizeof(eeprom_path));    
    memset(eeprom_addr, 0, sizeof(eeprom_addr));    
    strncpy(eeprom_addr, "0050", sizeof(eeprom_addr));    
    
    memset(data, 0, 256);
    
    if (port >=1 && port <= 64) {
        phy_port = _fp2phy_port_mapping[port-1];
        port_group = (phy_port-1)/8;
        eeprombusbase = 33 + (port_group * 8);
        eeprombusshift = (phy_port-1)%8;
        eeprombus = eeprombusbase + eeprombusshift;
    } else if (port == 65 ){
        eeprombus = 21;
    } else if (port == 66 ){
        eeprombus = 22;
    } else {
        return ONLP_STATUS_E_INTERNAL;
    }
    
    snprintf(eeprom_path, sizeof(eeprom_path), 
             "/sys/bus/i2c/devices/%d-%s/eeprom", eeprombus, eeprom_addr);

    if (onlplib_sfp_eeprom_read_file(eeprom_path, data) != 0) {
        return ONLP_STATUS_E_INTERNAL;
    } 

    return ONLP_STATUS_OK;
}

/*
 * De-initialize the SFPI subsystem.
 */
int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
