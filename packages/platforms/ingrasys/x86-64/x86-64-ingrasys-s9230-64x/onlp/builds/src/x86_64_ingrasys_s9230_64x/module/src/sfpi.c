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
#include <onlp/platformi/sfpi.h>
#include <x86_64_ingrasys_s9230_64x/x86_64_ingrasys_s9230_64x_config.h>
#include "x86_64_ingrasys_s9230_64x_log.h"
#include "platform_lib.h"

int port_mapping[] = {
    0,  1,  4,  5,  8,  9, 12, 13, 16, 17,
    20,21, 24, 25, 28, 29, 32, 33, 36, 37,
    40,41, 44, 45, 48, 49, 52, 53, 56, 57,
    60,61,  2,  3,  6,  7, 10, 11, 14, 15,
    18,19, 22, 23, 26, 27, 30, 31, 34, 35,
    38,39, 42, 43, 46, 47, 50, 51, 54, 55,
    58,59, 62, 63};

int
onlp_sfpi_init(void)
{  
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;
    for(p = 1; p <= QSFP_NUM; p++) {
        AIM_BITMAP_SET(bmap, p);
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    int status, rc;    
           
    if ((rc = qsfp_present_get(port_mapping[port-1] + 1, &status)) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }
    
    return status;
}


int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int p = 1;
    int rc = 0;
    
    for (p = 1; p <= QSFP_NUM; p++) {        
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
    int eeprombus;
    char eeprom_path[512], eeprom_addr[64];
    
    memset(eeprom_path, 0, sizeof(eeprom_path));    
    memset(eeprom_addr, 0, sizeof(eeprom_addr));    
    strncpy(eeprom_addr, "0050", sizeof(eeprom_addr));    
    
    memset(data, 0, 256);
    
    eeprombus = port_mapping[port-1] + 33;
    
    snprintf(eeprom_path, sizeof(eeprom_path), 
             "/sys/bus/i2c/devices/%d-%s/eeprom", eeprombus, eeprom_addr);

    if (onlplib_sfp_eeprom_read_file(eeprom_path, data) != 0) {
        AIM_LOG_INFO("Unable to read eeprom from port(%d)\r\n", port);
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
