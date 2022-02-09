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
#include "x86_64_accton_as7726_32x_int.h"
#include "x86_64_accton_as7726_32x_log.h"

#define NUM_OF_FAN_ON_MAIN_BROAD      6
#define PREFIX_PATH_ON_CPLD_DEV          "/sys/bus/i2c/devices/"
#define NUM_OF_CPLD                      3
#define FAN_DUTY_CYCLE_MAX         (100)
#define FAN_DUTY_CYCLE_DEFAULT     (38)


static char arr_cplddev_name[NUM_OF_CPLD][10] =
{
 "11-0060",
 "12-0062",
 "13-0064"
};

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as7726-32x-r0";
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
    int   i, v[NUM_OF_CPLD]={0};

    for (i = 0; i < NUM_OF_CPLD; i++) {
        v[i] = 0;

        if(onlp_file_read_int(v+i, "%s%s/version", PREFIX_PATH_ON_CPLD_DEV, arr_cplddev_name[i]) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    pi->cpld_versions = aim_fstrdup("%d.%d.%d", v[0], v[1], v[2]);

    return 0;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

/*
 * Air Flow Front to Back :
 * (Thermal sensor_LM75_4A + Thermal sensor_LM75_CPU) /2 <=38C : Keep 37.5%(0x04) Fan speed
 * (Thermal sensor_LM75_4A + Thermal sensor_LM75_CPU) /2 > 38C : Change Fan speed from 37.5%(0x04) to 62.5%(0x08)
 * (Thermal sensor_LM75_4A + Thermal sensor_LM75_CPU) /2 > 46C : Change Fan speed from 62.5%(0x08) to 100%(0x0E)
 * (Thermal sensor_LM75_4A + Thermal sensor_LM75_CPU) /2 > 58C : Send alarm message
 * (Thermal sensor_LM75_4A + Thermal sensor_LM75_CPU) /2 > 66C : Shut down system
 * One Fan fail : Change Fan speed to 100%(0x0E)

 * Air Flow Back to Front :
 * (Thermal sensor_LM75_4A + Thermal sensor_LM75_CPU) /2 <=34C : Keep 37.5%(0x04) Fan speed
 * (Thermal sensor_LM75_4A + Thermal sensor_LM75_CPU) /2 > 34C : Change Fan speed from 37.5%(0x04) to 62.5%(0x08)
 * (Thermal sensor_LM75_4A + Thermal sensor_LM75_CPU) /2 > 44C : Change Fan speed from 62.5%(0x08) to 100%(0x0E)
 * (Thermal sensor_LM75_4A + Thermal sensor_LM75_CPU) /2 > 59C : Send alarm message
 * (Thermal sensor_LM75_4A + Thermal sensor_LM75_CPU) /2 > 67C : Shut down system
 * One Fan fail:  Change Fan speed to 100%(0x0E)
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
   LEVEL_FAN_DEF=0,
   LEVEL_FAN_MID,
   LEVEL_FAN_MAX,
   LEVEL_TEMP_HIGH,
   LEVEL_TEMP_CRITICAL
};

fan_ctrl_policy_t  fan_thermal_policy_f2b[] = {
{38,  0x4, 0,     38000,   LEVEL_FAN_DEF},
{63,  0x8, 38000, 46000,   LEVEL_FAN_MID},
{100, 0xE, 46000, 58000,   LEVEL_FAN_MAX},
{100, 0xE, 58000, 66000,   LEVEL_TEMP_HIGH},
{100, 0xE, 66000, 200000,  LEVEL_TEMP_CRITICAL}
};

fan_ctrl_policy_t  fan_thermal_policy_b2f[] = {
{38,  0x4, 0,     34000,   LEVEL_FAN_DEF},
{63,  0x8, 34000, 44000,   LEVEL_FAN_MID},
{100, 0xE, 44000, 59000,   LEVEL_FAN_MAX},
{100, 0xE, 59000, 67000,   LEVEL_TEMP_HIGH},
{100, 0xE, 67000, 200000,  LEVEL_TEMP_CRITICAL}
};

#define FAN_SPEED_CTRL_PATH "/sys/bus/i2c/devices/54-0066/fan_duty_cycle_percentage"
#define FAN_DIRECTION_PATH "/sys/bus/i2c/devices/54-0066/fan1_direction"

static int fan_state=LEVEL_FAN_DEF;
static int alarm_state = 0; /* 0->default or clear, 1-->alarm detect */
static int fan_fail = 0;

int onlp_sysi_platform_manage_fans(void)
{
    int i=0, ori_state=LEVEL_FAN_DEF, current_state=LEVEL_FAN_DEF;
    int  fd, len, value=1;
    int cur_duty_cycle, new_duty_cycle, temp=0;
    onlp_thermal_info_t thermal_4, thermal_5;
    char  buf[10] = {0};
    fan_ctrl_policy_t *fan_thermal_policy;

    /* Get fan direction
     */
    if (onlp_file_read_int(&value, FAN_DIRECTION_PATH) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", FAN_DIRECTION_PATH);
    }

    if(value==1)
        fan_thermal_policy=fan_thermal_policy_f2b;
    else
        fan_thermal_policy=fan_thermal_policy_b2f;

    /* Get current temperature
     */
    if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(4), &thermal_4) != ONLP_STATUS_OK  )
    {
        AIM_LOG_ERROR("Unable to read thermal status, set fans to 75 %% speed");
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), fan_thermal_policy[LEVEL_FAN_MID].duty_cycle);
        return ONLP_STATUS_E_INTERNAL;
    }
    if(onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(5), &thermal_5) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("Unable to read thermal status, set fans to 75 %% speed");
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), fan_thermal_policy[LEVEL_FAN_MID].duty_cycle);
        return ONLP_STATUS_E_INTERNAL;
    }

    temp = (thermal_4.mcelsius + thermal_5.mcelsius)/2;
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
    for(i=0; i < sizeof(fan_thermal_policy_f2b)/sizeof(fan_ctrl_policy_t); i++)
    {
        if (temp > fan_thermal_policy[i].temp_down)
        {
            if (temp <= fan_thermal_policy[i].temp_up)
            {
                current_state =i;
            }
        }
    }
    if(current_state > LEVEL_TEMP_CRITICAL || current_state < LEVEL_FAN_DEF)
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

    for (i = 1; i <= NUM_OF_FAN_ON_MAIN_BROAD; i++)
    {
        onlp_fan_info_t fan_info;

        if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to get fan(%d) status, try to set the other fans as full speed\r\n", i);
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MAX);
            fan_fail=1;
            break;
        } else {
	    fan_fail = 0;
	}
        /* Decision 1: Set fan as full speed if any fan is failed.
         */
        if (fan_info.status & ONLP_FAN_STATUS_FAILED || !(fan_info.status & ONLP_FAN_STATUS_PRESENT)) {
            AIM_LOG_ERROR("Fan(%d) is not working, set the other fans as full speed\r\n", i);
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MAX);
            fan_fail=1;
            break;
        } else {
            fan_fail = 0;
	}
    }

    if(current_state!=ori_state)
    {
         fan_state=current_state;

         switch (ori_state)
         {
             case LEVEL_FAN_DEF:
                 if(current_state==LEVEL_TEMP_HIGH)
                 {
                     if(alarm_state==0)
                     {
                        AIM_SYSLOG_WARN("Temperature high", "Temperature high","Alarm for temperature high is detected");
                        alarm_state=1;
                     }
                 }
                 if(current_state==LEVEL_TEMP_CRITICAL)
                 {
                     AIM_SYSLOG_CRIT("Temperature critical", "Temperature critical", "Alarm for temperature critical is detected, reboot DUT");
                     system("sync;sync;sync");
                     system("reboot");
                 }
                 break;
             case LEVEL_FAN_MID:
                 if(current_state==LEVEL_TEMP_HIGH)
                 {
                     if(alarm_state==0)
                     {
                        AIM_SYSLOG_WARN("Temperature high", "Temperature high","Alarm for temperature high is detected");
                        alarm_state=1;
                     }
                 }
                 if(current_state==LEVEL_TEMP_CRITICAL)
                 {
                     AIM_SYSLOG_CRIT("Temperature critical", "Temperature critical", "Alarm for temperature critical is detected, reboot DUT");
                     system("sync;sync;sync");
                     system("reboot");
                 }
                 break;
             case LEVEL_FAN_MAX:
                 if(current_state==LEVEL_TEMP_HIGH)
                 {
                     if(alarm_state==0)
                     {
                        AIM_SYSLOG_WARN("Temperature high", "Temperature high","Alarm for temperature high is detected");
                        alarm_state=1;
                     }
                 }
                 if(current_state==LEVEL_TEMP_CRITICAL)
                 {
                     AIM_SYSLOG_CRIT("Temperature critical", "Temperature critical ", "Alarm for temperature critical is detected, reboot DUT");
                     system("sync;sync;sync");
                     system("reboot");
                 }
                 break;
             case LEVEL_TEMP_HIGH:
                 if(current_state==LEVEL_TEMP_CRITICAL)
                 {
                     AIM_SYSLOG_CRIT("Temperature critical", "Temperature critical ", "Alarm for temperature critical is detected, reboot DUT");
                     system("sync;sync;sync");
                     system("reboot");
                 }
                 break;
             case LEVEL_TEMP_CRITICAL:
                 break;
             default:
                AIM_SYSLOG_WARN("onlp_sysi_platform_manage_fans abnormal state", "onlp_sysi_platform_manage_fans  abnormal state", "onlp_sysi_platform_manage_fans at abnormal state\n");
                 break;
         }

    }
    if(alarm_state==1 && current_state < LEVEL_TEMP_HIGH)
    {
       if (temp < (fan_thermal_policy[LEVEL_TEMP_HIGH].temp_down - 5000)) /*below 58 C, clear alarm*/
       {
           AIM_SYSLOG_INFO("Temperature high is clean", "Temperature high is clear", "Alarm for temperature high is cleared");
           alarm_state=0;
       }
    }

    return 0;
}

int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
