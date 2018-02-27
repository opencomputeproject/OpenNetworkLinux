/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc
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
#include <stdio.h>
#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "x86_64_delta_ag7648c_int.h"
#include "x86_64_delta_ag7648c_log.h"

#include "platform_lib.h"
#include "x86_64_delta_i2c.h"
platform_id_t platform_id = PLATFORM_ID_UNKNOWN;

#define ONIE_PLATFORM_NAME "x86-64-delta-ag7648c-r0"
const char*
onlp_sysi_platform_get(void)
{ 
	enum ag7648c_product_id pid = get_product_id();
	
	if (pid == PID_AG7648C)
		return "x86-64-delta-ag7648c";
	else 
		return "unknow";
}

int
onlp_sysi_platform_set(const char* platform)
{ 
    if(strstr(platform,"x86-64-delta-ag7648c-r0")) {
        platform_id = PLATFORM_ID_DELTA_AG7648C_R0;
        return ONLP_STATUS_OK;
    }
	AIM_LOG_ERROR("No support for platform '%s'", platform);
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{ 
    int   v;
    
	v = i2c_devname_read_byte("SYSCPLD", 0X0);
	
    pi->cpld_versions = aim_fstrdup("%d", v & 0xf);

    return 0;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
	int i,re_cnt;
    uint8_t* rdata = aim_zmalloc(256);
    if(!rdata){
		AIM_LOG_ERROR("Unable to malloc memory \r\n");
		return ONLP_STATUS_E_INTERNAL;
	}
	for(i=0;i<8;i++){
		re_cnt=3;
		while(re_cnt){
			if (i2c_devname_read_block("ID_EEPROM", i * 32, (rdata + i * 32), 32) < 0)
			{
                re_cnt--;
				continue;
			}
			break;
		}
		if(re_cnt==0){
			AIM_LOG_ERROR("Unable to read the %d reg \r\n",i);
			break;
		}
			
	}
   
    *data = rdata;

    return ONLP_STATUS_OK;

	
}

void
onlp_sysi_onie_data_free(uint8_t* data)
{ 
    aim_free(data);
}



int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{ 
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 1 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* LEDs on the chassis */
    for (i = 1; i <= chassis_led_count(); i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 1 Fans on the chassis */
    for (i = 1; i <= chassis_fan_count(); i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    return 0;
}
int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{   
    if(onie){
        onie->platform_name = aim_strdup(ONIE_PLATFORM_NAME);
    }
    return ONLP_STATUS_OK;
}



int
onlp_sysi_platform_manage_fans(void)
{ 

    int rc;
    onlp_thermal_info_t ti2, ti3, ti4;
    int mtemp=0;
    int new_rpm=0;
    
    if (chassis_fan_count() == 0) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
  
    /* Get temperature */
    /*rc = onlp_thermal_info_get(ONLP_THERMAL_ID_CREATE(1), &ti1);
	
    if (rc != ONLP_STATUS_OK) {
        return rc;
    }*/
        
    rc = onlp_thermal_info_get(ONLP_THERMAL_ID_CREATE(2), &ti2);
	
    if (rc != ONLP_STATUS_OK) {
        return rc;
    }
    rc = onlp_thermal_info_get(ONLP_THERMAL_ID_CREATE(3), &ti3);
	
    if (rc != ONLP_STATUS_OK) {
        return rc;
    }
        
    rc = onlp_thermal_info_get(ONLP_THERMAL_ID_CREATE(4), &ti4);
	
    if (rc != ONLP_STATUS_OK) {
        return rc;
    }
             
    mtemp=(ti2.mcelsius+ti3.mcelsius + ti4.mcelsius) / 3;

    DEBUG_PRINT("mtemp %d\n", mtemp);

    /* Bring fan speed according the temp
     */
  
    if(mtemp<25000)
        new_rpm=FAN_IDLE_RPM;
    else if((mtemp>=30000)&&(mtemp<40000))
        new_rpm=FAN_LEVEL1_RPM;
    else if((mtemp>=45000)&&(mtemp<55000))
        new_rpm=FAN_LEVEL2_RPM;
    else if((mtemp>=60000)&&(mtemp<75000))
        new_rpm=FAN_LEVEL3_RPM;
    else if(mtemp>=80000)
        new_rpm=FAN_LEVEL4_RPM;
    else{
        return ONLP_STATUS_OK;
   }
  
    onlp_fan_rpm_set(ONLP_FAN_ID_CREATE(1),new_rpm);
    onlp_fan_rpm_set(ONLP_FAN_ID_CREATE(2),new_rpm); 
    onlp_fan_rpm_set(ONLP_FAN_ID_CREATE(3),new_rpm); 
    onlp_fan_rpm_set(ONLP_FAN_ID_CREATE(4),new_rpm); 
    onlp_fan_rpm_set(ONLP_FAN_ID_CREATE(5),new_rpm); 
    onlp_fan_rpm_set(ONLP_FAN_ID_CREATE(6),new_rpm);
     
    return ONLP_STATUS_OK;
}


int
onlp_sysi_platform_manage_leds(void)
{ 
		int i,tray_i,rc;
		onlp_fan_info_t info;
		onlp_led_mode_t fan_new_mode;
        onlp_led_mode_t fan_tray_new_mode[3];
		onlp_psu_info_t psu;
		onlp_led_mode_t psu_new_mode;
		onlp_led_mode_t sys_new_mode;
        onlp_led_mode_t locator_new_mode;
		/*fan led */
		/*fan led */
        for(tray_i=0;tray_i<3;tray_i++){
            for(i=CHASSIS_FAN_COUNT-2*tray_i;i>=CHASSIS_FAN_COUNT-2*tray_i-1;i--){
                rc=onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &info);
                if ((rc != ONLP_STATUS_OK) ||((info.status&0x1)!=1)){
                    fan_tray_new_mode[tray_i]=ONLP_LED_MODE_OFF;
                    goto tray_next;
                }
                else{
                    if((info.status&0x2)==1){
                        fan_tray_new_mode[tray_i]=ONLP_LED_MODE_YELLOW;
                        goto tray_next;
                    }
                }
            }
            fan_tray_new_mode[tray_i]=ONLP_LED_MODE_GREEN;
tray_next:  continue;
        }
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN_TRAY0),fan_tray_new_mode[0]);
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN_TRAY1),fan_tray_new_mode[1]);
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN_TRAY2),fan_tray_new_mode[2]);
        
        if((fan_tray_new_mode[0]==ONLP_LED_MODE_GREEN)&&(fan_tray_new_mode[1]==ONLP_LED_MODE_GREEN)&& 
            (fan_tray_new_mode[2]==ONLP_LED_MODE_GREEN))
            fan_new_mode=ONLP_LED_MODE_GREEN;
        else if((fan_tray_new_mode[0]==ONLP_LED_MODE_OFF)||(fan_tray_new_mode[1]==ONLP_LED_MODE_OFF)|| 
            (fan_tray_new_mode[2]==ONLP_LED_MODE_OFF))
             fan_new_mode=ONLP_LED_MODE_YELLOW;
        else
            fan_new_mode=ONLP_LED_MODE_YELLOW_BLINKING;
    
		onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN),fan_new_mode);
		/*psu1 and psu2 led */
        for(i=1;i<=CHASSIS_PSU_COUNT;i++){
            rc=onlp_psui_info_get(ONLP_PSU_ID_CREATE(i),&psu);
            
            if (rc != ONLP_STATUS_OK) {
               continue;
            }
            if((psu.status&0x1)&&!(psu.status&0x2)){
                psu_new_mode=ONLP_LED_MODE_GREEN;
                goto sys_led;
            }
        }
		psu_new_mode=ONLP_LED_MODE_YELLOW_BLINKING;
     
sys_led	:		
		onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_POWER),psu_new_mode);
		//sys led 	----------------
		if((fan_new_mode!=ONLP_LED_MODE_GREEN)||(psu_new_mode!=ONLP_LED_MODE_GREEN))
			sys_new_mode=ONLP_LED_MODE_YELLOW_BLINKING;
		else
			sys_new_mode=ONLP_LED_MODE_GREEN;
		
		onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_SYS),sys_new_mode);
        
        locator_new_mode=ONLP_LED_MODE_BLUE;
        onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_LOCATOR),locator_new_mode);
		return ONLP_STATUS_OK;
}

