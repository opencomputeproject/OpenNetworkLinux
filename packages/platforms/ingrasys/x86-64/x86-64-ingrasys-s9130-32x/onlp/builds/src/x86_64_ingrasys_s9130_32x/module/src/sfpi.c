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
#include <x86_64_ingrasys_s9130_32x/x86_64_ingrasys_s9130_32x_config.h>
#include "x86_64_ingrasys_s9130_32x_log.h"
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
    for(p = 1; p <= QSFP_NUM; p++) {
        AIM_BITMAP_SET(bmap, p);
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    int status, presentchan, rc, pres_val;    
           
    if ((rc = qsfp_present_get(port, &pres_val)) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }
    
    presentchan = (( (port - 1) % 8 ));    

    /* status: 0 -> Down, 1 -> Up */
    status=(( ((pres_val & ( 1 << presentchan)))  != 0 ? 0 : 1 ));
   
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
    int eeprombusidx, eeprombus, eeprombusbase;
    char eeprom_path[512], eeprom_addr[32];
    memset(eeprom_path, 0, sizeof(eeprom_path));    
    memset(eeprom_addr, 0, sizeof(eeprom_addr));    
    strncpy(eeprom_addr, "0050", sizeof(eeprom_addr));    
    
    memset(data, 0, 256);
    
    if (port >= 1 && port <= 8) {
        eeprombusbase=21;
    } else if (port >= 9 && port <= 16) {
        eeprombusbase=29;
    } else if (port >= 17 && port <= 24) {
        eeprombusbase=37;
    } else if (port >= 25 && port <= 32) {
        eeprombusbase=45;        
    } else {
        return 0;
    }
    
    eeprombusidx = port % 8;
    eeprombusidx = (eeprombusidx == 0) ? 8 : eeprombusidx;
    eeprombus = eeprombusbase + eeprombusidx - 1;
    
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
