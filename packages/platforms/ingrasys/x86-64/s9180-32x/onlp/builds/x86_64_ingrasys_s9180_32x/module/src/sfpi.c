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
#include <x86_64_ingrasys_s9180_32x/x86_64_ingrasys_s9180_32x_config.h>
#include "x86_64_ingrasys_s9180_32x_log.h"
#include "platform_lib.h"

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
    int status;

    if (qsfp_present_get(port, &status) != ONLP_STATUS_OK) {
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
    int eeprombusidx, eeprombus, eeprombusbase;
    char eeprom_path[512], eeprom_addr[32];
    memset(eeprom_path, 0, sizeof(eeprom_path));    
    memset(eeprom_addr, 0, sizeof(eeprom_addr));    
    aim_strlcpy(eeprom_addr, "0050", sizeof(eeprom_addr));    
    
    memset(data, 0, 256);
    
    if (port >= 1 && port <= 8) {
        eeprombusbase = 9;
    } else if (port >= 9 && port <= 16) {
        eeprombusbase = 17;
    } else if (port >= 17 && port <= 24) {
        eeprombusbase = 25;
    } else if (port >= 25 && port <= 32) {
        eeprombusbase = 33;        
    } else if (port == 33) {
        eeprombus = 45;        
    } else if (port == 34) {
        eeprombus = 46;        
    } else {
        return 0;
    }
   
    /* port 33 and 34 doesn't need to swap */
    if (port >=1 && port <= 32) {
        eeprombusidx = port % 8;
        switch (eeprombusidx) {
            case 1:
                eeprombus = eeprombusbase + 1;
                break;
            case 2:
                eeprombus = eeprombusbase + 0;
                break;
            case 3:
                eeprombus = eeprombusbase + 3;
                break;
            case 4:
                eeprombus = eeprombusbase + 2;
                break;
            case 5:
                eeprombus = eeprombusbase + 5;
                break;
            case 6:
                eeprombus = eeprombusbase + 4;
                break;
            case 7:
                eeprombus = eeprombusbase + 7;
                break;
            case 0:
                eeprombus = eeprombusbase + 6;
                break;
            default:
                return ONLP_STATUS_E_INTERNAL;
                break;
        }
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
