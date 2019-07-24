/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *        Copyright 2017 Delta Networks, Inc.
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
 * HardwareVersion: 02
 *
 ***********************************************************/
 
#include <onlp/platformi/sysi.h>
#include <onlplib/crc32.h>
#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "x86_64_delta_ag9064_log.h"
#include "platform_lib.h"

const char* onlp_sysi_platform_get(void)
{
    return "x86-64-delta-ag9064-r0";
}

int onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}

int onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);
    
    if(onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK) 
    {
        if(*size == 256) 
        {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }
    
    aim_free(rdata);
    *size = 0;
    
    return ONLP_STATUS_E_INTERNAL;
}

int onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int rv = ONLP_STATUS_OK;
    uint32_t u4Data = 0;
    
    rv = ifnOS_LINUX_BmcI2CGet(I2C_BMC_BUS_5, SWPLD_1_ADDR, CPLD_VERSION_REGISTER, &u4Data, 1);
    
    if(rv == ONLP_STATUS_OK)
    {
        u4Data = u4Data >> CPLD_VERSION_OFFSET;
        
        pi->cpld_versions = aim_fstrdup("%d", u4Data);
    }
    
    return rv;
}

void onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

int onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i = 0;
    onlp_oid_t* e = table;
    
    memset(table, 0, max*sizeof(onlp_oid_t));
    
    for (i = 1; i <= NUM_OF_THERMAL_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }
    
    for (i = 1; i <= NUM_OF_LED_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }
    
    for (i = 1; i <= NUM_OF_PSU_ON_MAIN_BROAD; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }
    
    for (i = 1; i <= NUM_OF_FAN; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }
    
    return 0;
}

int onlp_sysi_platform_manage_leds(void)
{ 
    int i = 0, rc = ONLP_STATUS_OK;
    onlp_fan_info_t fan_info;
    onlp_led_mode_t fan_new_mode;
    onlp_led_mode_t fan_tray_new_mode[NUM_OF_FAN_ON_MAIN_BROAD];
    onlp_psu_info_t psu_info[NUM_OF_PSU_ON_MAIN_BROAD];
    onlp_led_mode_t psu_new_mode[NUM_OF_PSU_ON_MAIN_BROAD];
    onlp_led_mode_t sys_new_mode;

    /* FAN LED */
    for(i = 0; i < NUM_OF_FAN_ON_MAIN_BROAD; i++)
    {
        rc = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i+1), &fan_info);
        
        if ( (rc != ONLP_STATUS_OK) || !(fan_info.status & ONLP_FAN_STATUS_PRESENT) )
        {
            fan_tray_new_mode[i] = ONLP_LED_MODE_ORANGE;
            continue;
        }
        else
        {
            if(fan_info.status & ONLP_FAN_STATUS_FAILED)
            {
                fan_tray_new_mode[i] = ONLP_LED_MODE_ORANGE;
                continue;
            }
        }
        
        fan_tray_new_mode[i] = ONLP_LED_MODE_GREEN;
    }
    
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN_TRAY_1), fan_tray_new_mode[0]);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN_TRAY_2), fan_tray_new_mode[1]);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN_TRAY_3), fan_tray_new_mode[2]);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN_TRAY_4), fan_tray_new_mode[3]);
    
    if((fan_tray_new_mode[0] == ONLP_LED_MODE_GREEN) && (fan_tray_new_mode[1] == ONLP_LED_MODE_GREEN) && 
       (fan_tray_new_mode[2] == ONLP_LED_MODE_GREEN) && (fan_tray_new_mode[3] == ONLP_LED_MODE_GREEN))
    {
        fan_new_mode = ONLP_LED_MODE_GREEN;
    }
    else
    {
        fan_new_mode = ONLP_LED_MODE_ORANGE;
    }
    
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN), fan_new_mode);

    /* PSU1 and PSU2 LED */
    for( i = 0; i < NUM_OF_PSU_ON_MAIN_BROAD; i++)
    {
        rc = onlp_psui_info_get(ONLP_PSU_ID_CREATE(i+1), &psu_info[i]);
        
        if (rc != ONLP_STATUS_OK) 
        {
           psu_new_mode[i] = ONLP_LED_MODE_OFF;
           continue;
        }
    
        if(psu_info[i].status & ONLP_PSU_STATUS_PRESENT)
        {
            if(psu_info[i].status & ONLP_PSU_STATUS_FAILED)
            {
                psu_new_mode[i] = ONLP_LED_MODE_RED;
            }
            else
            {
                psu_new_mode[i] = ONLP_LED_MODE_GREEN;
            }
        }
        else
        {
            psu_new_mode[i] = ONLP_LED_MODE_OFF;
        }
    }

    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PSU1), psu_new_mode[0]);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PSU2), psu_new_mode[1]);

    /* SYS LED */
    if( ((psu_new_mode[0] == ONLP_LED_MODE_GREEN) || (psu_new_mode[1] == ONLP_LED_MODE_GREEN)) &&
        (fan_new_mode == ONLP_LED_MODE_GREEN))
    {
        sys_new_mode = ONLP_LED_MODE_GREEN;
    }
    else
    {
        sys_new_mode = ONLP_LED_MODE_RED;
    }
        
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_SYS), sys_new_mode);
        
    return ONLP_STATUS_OK;
}
