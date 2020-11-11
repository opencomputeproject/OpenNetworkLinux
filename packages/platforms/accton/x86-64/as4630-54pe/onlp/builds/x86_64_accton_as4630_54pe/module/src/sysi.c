/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"
#include "x86_64_accton_as4630_54pe_int.h"
#include "x86_64_accton_as4630_54pe_log.h"


#define PREFIX_PATH_ON_CPLD_DEV          "/sys/bus/i2c/devices/3-0060/"

#define NUM_OF_CPLD                      3
#define FAN_DUTY_CYCLE_MAX         (100)
/* Note, all chassis fans share 1 single duty setting. 
 * Here use fan 1 to represent global fan duty value.*/

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as4630-54pe-r0";
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);
	
    if(onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK) {
        if(*size == 256) {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }

    aim_free(rdata);
    *size = 0;
    return ONLP_STATUS_E_INTERNAL;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));
    
    /* 6 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 5 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 6 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int ver=0;
	
    if(onlp_file_read_int(&ver, "%s/version", PREFIX_PATH_ON_CPLD_DEV) < 0)
        return ONLP_STATUS_E_INTERNAL;
		
    pi->cpld_versions = aim_fstrdup("%d", ver);
	
    return 0;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

/* Temperature Policy
 * If any fan fail , please set fan speed register to 16
 * The max value of fan speed register is 14
 * LM77(48)+LM75(4B)+LM75(4A)  >  140, Set 10
 * LM77(48)+LM75(4B)+LM75(4A)  >  150, Set 12
 * LM77(48)+LM75(4B)+LM75(4A)  >  160, Set 14
 * LM77(48)+LM75(4B)+LM75(4A)  <  140, Set 8
 * LM77(48)+LM75(4B)+LM75(4A)  <  150, Set 10
 * LM77(48)+LM75(4B)+LM75(4A)  <  160, Set 12
 * Reset DUT:LM77(48)>=70C
 */

typedef struct fan_ctrl_policy {
   int duty_cycle; 
   int pwm;
   int temp_down; /* The boundary temperature to down adjust fan speed */
   int temp_up;   /* The boundary temperature to up adjust fan speed */
   int state;
} fan_ctrl_policy_t;
   
enum
{
   LEVEL_FAN_MIN=0,
   LEVEL_FAN_NORMAL,   
   LEVEL_FAN_MID,
   LEVEL_FAN_HIGH,
   LEVEL_TEMP_CRITICAL
};
    
fan_ctrl_policy_t  fan_thermal_policy[] = {
{50,  8,  0,      140000, LEVEL_FAN_MIN},
{62,  10, 140000, 150000, LEVEL_FAN_NORMAL},
{75,  12, 150000, 160000, LEVEL_FAN_MID},
{88,  14, 160000, 240000, LEVEL_FAN_HIGH},
{100, 16, 240000, 300000, LEVEL_TEMP_CRITICAL },
};

#define FAN_SPEED_CTRL_PATH "/sys/bus/i2c/devices/3-0060/fan_duty_cycle_percentage"
#define SHUTDOWN_DUT_CMD "i2cset -y -f 3 0x60 0x4 0xE4"
    
static int fan_state=LEVEL_FAN_MIN;
static int fan_fail = 0;

int onlp_sysi_platform_manage_fans(void)
    {
    int i=0, ori_state=LEVEL_FAN_MIN, current_state=LEVEL_FAN_MIN;
    int  fd, len;
    int cur_duty_cycle, new_duty_cycle, temp=0;
    onlp_thermal_info_t thermali[3];
    char  buf[10] = {0};

    /* Get current temperature
     */
    for (i=0; i<3; i++)
    {
        if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i+2), &thermali[i]) != ONLP_STATUS_OK  )
        {   
            AIM_LOG_ERROR("Unable to read thermal status, set fans to 87 %% speed");
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), fan_thermal_policy[LEVEL_FAN_HIGH].duty_cycle);
            return ONLP_STATUS_E_INTERNAL;
        }
        temp+=thermali[i].mcelsius;
    }

    
    /* Get current fan pwm percent
     */
    fd = open(FAN_SPEED_CTRL_PATH, O_RDONLY);
    if (fd == -1){
        AIM_LOG_ERROR("Unable to open fan speed control node (%s)", FAN_SPEED_CTRL_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }
    len = read(fd, buf, sizeof(buf));
    close(fd);
    if (len <= 0) {
        AIM_LOG_ERROR("Unable to read fan speed from (%s)", FAN_SPEED_CTRL_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }
    cur_duty_cycle = atoi(buf);
    ori_state=fan_state;
    /* Inpunt temp to get theraml_polyc state and new pwm percent. */
    for(i=0; i < sizeof(fan_thermal_policy)/sizeof(fan_ctrl_policy_t); i++)
    {
        if (temp > fan_thermal_policy[i].temp_down)
        {
            if (temp <= fan_thermal_policy[i].temp_up)
            {
                current_state =i;
            }
        }
    }
    if(current_state > LEVEL_TEMP_CRITICAL || current_state < LEVEL_FAN_MIN)
    {
        AIM_LOG_ERROR("onlp_sysi_platform_manage_fans get error  current_state\n");
        return 0;
    }
    /* Decision 3: Decide new fan pwm percent.
     */
    if (fan_fail==0 &&cur_duty_cycle!=fan_thermal_policy[current_state].duty_cycle)
    {
        new_duty_cycle = fan_thermal_policy[current_state].duty_cycle;
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), new_duty_cycle);
    }
    /* Get each fan status
     */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
    {
        onlp_fan_info_t fan_info;
        if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to get fan(%d) status, try to set the other fans as full speed\r\n", i);
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MAX);
            fan_fail=1;
            break;
        }
        else
        {
            fan_fail = 0;
	}
        /* Decision 1: Set fan as full speed if any fan is failed.
         */
        if (fan_info.status & ONLP_FAN_STATUS_FAILED || !(fan_info.status & ONLP_FAN_STATUS_PRESENT))
        {
            AIM_LOG_ERROR("Fan(%d) is not working, set the other fans as full speed\r\n", i);
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MAX);
            fan_fail=1;
            break;
        }
        else 
        {
            fan_fail = 0;
        }
    }
    if(thermali[2].mcelsius >= 70000) /*LM75-48*/
    {
        /*critical case*/
        AIM_SYSLOG_CRIT("Temperature critical", "Temperature critical", "Alarm for temperature critical is detected, reset DUT");
        sleep(2);
        system(SHUTDOWN_DUT_CMD);
    }
    if(current_state!=ori_state)
    {
        fan_state=current_state;
    }

    return 0;
}
int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

