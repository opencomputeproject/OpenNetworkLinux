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
#include "x86_64_accton_as4630_54npe_int.h"
#include "x86_64_accton_as4630_54npe_log.h"


#define PREFIX_PATH_ON_CPLD_DEV          "/sys/bus/i2c/devices/3-0060/"

#define NUM_OF_CPLD                      3
#define FAN_DUTY_CYCLE_MAX         (100)
/* Note, all chassis fans share 1 single duty setting. 
 * Here use fan 1 to represent global fan duty value.*/

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as4630-54npe-r0";
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

/*
    If any fan fails, please set fan speed register to 16
    Default system fan speed set 12
    The max value of fan speed register is 16
    LM77(48)+LM75(4B)+LM75(4A) > 145 => Set 16
    LM77(48)+LM75(4B)+LM75(4A) < 105 => Set 12
    12 mean 75%
    16 mean 100%
    Thermal shuntdown:LM77(48)>=75C
*/

#define FAN_SPEED_CTRL_PATH "/sys/bus/i2c/devices/3-0060/fan_duty_cycle_percentage"
#define SHUTDOWN_DUT_CMD "i2cset -y -f 3 0x60 0x4 0xE4"
    
static int fan_fail = 0;

int onlp_sysi_platform_manage_fans(void)
 {
    int i=0;
    int temp=0;
    onlp_thermal_info_t thermali[3];

    /* Get current temperature
     */
    for (i=0; i<3; i++)
    {
        if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i+2), &thermali[i]) != ONLP_STATUS_OK  )
        {   
            AIM_LOG_ERROR("Unable to read thermal status, set fans to 100 %% speed");
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MAX);
            return ONLP_STATUS_E_INTERNAL;
        }
        temp+=thermali[i].mcelsius;
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
    /* if fan status is no fail, check the thermal condition */
    if (fan_fail == 0)
    {
        if (temp > 145000)
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MAX);

        if (temp < 105000)
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1),75);
    }
    /*Thermal shuntdown:LM77(48)>=75C */
    if(thermali[3].mcelsius >= 75000)
    {
        /*critical case*/
        AIM_SYSLOG_CRIT("Temperature critical", "Temperature critical", "Alarm for temperature critical is detected, shutdown DUT");
        sleep(2);
        system(SHUTDOWN_DUT_CMD);
    }

    return 0;
}
int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

