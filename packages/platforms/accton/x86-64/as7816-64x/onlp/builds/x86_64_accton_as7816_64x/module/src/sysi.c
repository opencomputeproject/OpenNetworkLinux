/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 Accton Technology Corporation.
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

#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#include "x86_64_accton_as7816_64x_int.h"
#include "x86_64_accton_as7816_64x_log.h"

#define CPLD_VERSION_FORMAT			"/sys/bus/i2c/devices/%s/version"
#define NUM_OF_CPLD         		4

static char* cpld_path[NUM_OF_CPLD] =
{
 "19-0060",
 "20-0062",
 "21-0064",
 "22-0066"
};

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as7816-64x-r0";
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    int ret = ONLP_STATUS_OK;
    int i = 0;
    uint8_t* rdata = aim_zmalloc(256);
    
    for (i = 0; i < 128; i++) {
        ret = onlp_i2c_readw(0, 0x56, i*2, ONLP_I2C_F_FORCE);
        if (ret < 0) {
            aim_free(rdata);
            *size = 0;
            return ret;
        }

        rdata[i*2]   = ret & 0xff;
        rdata[i*2+1] = (ret >> 8) & 0xff;
    }

    *size = 256;
    *data = rdata;

    return ONLP_STATUS_OK;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));
    
    /* 5 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 5 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 1 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 2 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int   i, v[NUM_OF_CPLD] = {0};

    for (i = 0; i < AIM_ARRAYSIZE(cpld_path); i++) {
        v[i] = 0;

        if(onlp_file_read_int(v+i, CPLD_VERSION_FORMAT , cpld_path[i]) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    pi->cpld_versions = aim_fstrdup("%d.%d.%d.%d", v[0], v[1], v[2], v[3]);
    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

int
onlp_sysi_platform_manage_init(void)
{
    return 0;
}

#define FAN_DUTY_MAX  (100)
#define FAN_DUTY_MIN  (40)

enum fan_level {
    FAN_LEVEL5 = 0,
    FAN_LEVEL7,
    FAN_LEVEL9,
    FAN_LEVELB,
    FAN_LEVELD,
    FAN_LEVELF,
    NUM_FAN_LEVEL,
    FAN_LEVEL_INVALID = NUM_FAN_LEVEL
};

static const int fanduty_by_level[] = {
    FAN_DUTY_MIN, 52, 64, 76, 88, FAN_DUTY_MAX
};

static int
sysi_fanctrl_fan_fault_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                              onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                              int *adjusted)
{
	int i;
    *adjusted = 0;

    /* Bring fan speed to FAN_DUTY_MAX if any fan is not operational */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        if (!(fi[i].status & ONLP_FAN_STATUS_FAILED)) {
            continue;
        }

        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
    }

    return ONLP_STATUS_OK;
}

static int 
sysi_fanctrl_fan_absent_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                               onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                               int *adjusted)
{
	int i;
    *adjusted = 0;

    /* Bring fan speed to FAN_DUTY_MAX if fan is not present */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        if (fi[i].status & ONLP_FAN_STATUS_PRESENT) {
            continue;
        }

        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
    }

    return ONLP_STATUS_OK;
}

static unsigned int get_fan_level_by_duty(int fanduty)
{
    switch(fanduty) {
        case FAN_DUTY_MIN: return FAN_LEVEL5;
        case 52: return FAN_LEVEL7;
        case 64: return FAN_LEVEL9;
        case 76: return FAN_LEVELB;
        case 88: return FAN_LEVELD;
        case FAN_DUTY_MAX: return FAN_LEVELF;
        default: return FAN_LEVEL_INVALID;
    }
}

static int
sysi_fanctrl_fan_unknown_speed_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                      onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                      int *adjusted)
{
    int fanduty;
    unsigned int fanlevel;

	*adjusted = 0;

    if (onlp_file_read_int(&fanduty, FAN_NODE(fan_duty_cycle_percentage)) < 0) {
        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
    }

    fanlevel = get_fan_level_by_duty(fanduty);

    if (fanlevel == FAN_LEVEL_INVALID) {
        *adjusted = 1;
        return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
	}

    return ONLP_STATUS_OK;
}

#define THERMAL_COUNT (THERMAL_6_ON_MAIN_BROAD - THERMAL_1_ON_MAIN_BROAD + 1)
typedef struct _threshold{
    int val[THERMAL_COUNT];
    unsigned int next_level;
} threshold_t;

#define NA_L 0
#define NA_H 999999
threshold_t up_th_f2b[NUM_FAN_LEVEL] = {
    {{57000,60000,40000,39000,36000,38000}, FAN_LEVEL7}, //LEVEL5
    {{62000,64000,46000,45000,43000,44000}, FAN_LEVELB}, //LEVEL7
    {{ NA_L, NA_L, NA_L, NA_L, NA_L, NA_L}, FAN_LEVELF}, //LEVEL9, NA, force to change level.
    {{ NA_H, NA_H,51000,49000,48000,49000}, FAN_LEVELD}, //LEVELB
    {{67000,70000,55000,54000,53000,54000}, FAN_LEVELF}, //LEVELD
    {{ NA_H, NA_H, NA_H, NA_H, NA_H, NA_H}, FAN_LEVELF}  //LEVELF, Won't go any higher.
};

threshold_t down_th_f2b[NUM_FAN_LEVEL] = {
    {{ NA_L, NA_L, NA_L, NA_L, NA_L, NA_L}, FAN_LEVEL5}, //LEVEL5, Won't go any lower.
    {{55000,58000,38000,37000,34000,36000}, FAN_LEVEL5}, //LEVEL7
    {{ NA_H, NA_H, NA_H, NA_H, NA_H, NA_H}, FAN_LEVELF}, //LEVEL9, NA, force to change level.
    {{60000,62000,43000,42000,40000,41000}, FAN_LEVEL7}, //LEVELB
    {{65000,68000,49000,47000,46000,47000}, FAN_LEVELB}, //LEVELD
    {{ NA_H, NA_H,53000,52000,51000,52000}, FAN_LEVELD}  //LEVELF
};

threshold_t up_th_b2f[NUM_FAN_LEVEL] = {
    {{52000,41000,34000,27000,26000,26000}, FAN_LEVEL7}, //LEVEL5
    {{ NA_H, NA_H,38000,32000,31000,31000}, FAN_LEVEL9}, //LEVEL7
    {{57000,48000,42000,37000,37000,36000}, FAN_LEVELB}, //LEVEL9,
    {{61000,52000,46000,42000,42000,42000}, FAN_LEVELD}, //LEVELB
    {{66000,57000,51000,47000,47000,47000}, FAN_LEVELF}, //LEVELD
    {{ NA_H, NA_H, NA_H, NA_H, NA_H, NA_H}, FAN_LEVELF}  //LEVELF, Won't go any higher.
};

threshold_t down_th_b2f[NUM_FAN_LEVEL] = {
    {{ NA_L, NA_L, NA_L, NA_L, NA_L, NA_L}, FAN_LEVEL5}, //LEVEL5, Won't go any lower.
    {{50000,39000,32000,25000,24000,24000}, FAN_LEVEL5}, //LEVEL7
    {{55000,45000,36000,30000,29000,29000}, FAN_LEVEL7}, //LEVEL9, NA, force to change level.
    {{ NA_H, NA_H,40000,35000,34000,34000}, FAN_LEVEL9}, //LEVELB
    {{59000,50000,44000,40000,40000,40000}, FAN_LEVELB}, //LEVELD
    {{63000,55000,48000,45000,45000,45000}, FAN_LEVELD}  //LEVELF
};

static int
sysi_fanctrl_overall_thermal_sensor_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                           onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                           int *adjusted)
{
    int i, fanduty, fanlevel, next_up_level, next_down_level;
    bool can_level_up = false; //prioritze up more than down
    bool can_level_down = true;
    int high, low;
    threshold_t* up_th = (fi[0].status & ONLP_FAN_STATUS_F2B)? up_th_f2b : up_th_b2f;
    threshold_t* down_th = (fi[0].status & ONLP_FAN_STATUS_F2B)? down_th_f2b : down_th_b2f;

    *adjusted = 1;

    // decide fanlevel by fanduty
    if(onlp_file_read_int(&fanduty, FAN_NODE(fan_duty_cycle_percentage)) < 0)
        fanduty = FAN_DUTY_MAX;
    fanlevel = get_fan_level_by_duty(fanduty);
    fanlevel = (fanlevel==FAN_LEVEL_INVALID)? FAN_LEVELF : fanlevel; //maximum while invalid.

    // decide target level by thermal sensor input.
    next_up_level = up_th[fanlevel].next_level;
    next_down_level = down_th[fanlevel].next_level;
    for (i=THERMAL_1_ON_MAIN_BROAD ; i<=THERMAL_6_ON_MAIN_BROAD; i++){
        high = up_th[fanlevel].val[i-THERMAL_1_ON_MAIN_BROAD];
        low = down_th[fanlevel].val[i-THERMAL_1_ON_MAIN_BROAD];
        // perform level up if anyone is higher than high_th.
        if(ti[i-1].mcelsius > high) {
            can_level_up = true;
            break;
        }
        // cancel level down if anyone is higher than low_th.
        if(ti[i-1].mcelsius > low)
            can_level_down = false;
    }
    fanlevel = (can_level_up)? next_up_level : ((can_level_down)? next_down_level : fanlevel);
    return onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1),
                                    fanduty_by_level[fanlevel]);
}

typedef int (*fan_control_policy)(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                  onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                  int *adjusted);

fan_control_policy fan_control_policies[] = {
sysi_fanctrl_fan_fault_policy,
sysi_fanctrl_fan_absent_policy,
sysi_fanctrl_fan_unknown_speed_policy,
sysi_fanctrl_overall_thermal_sensor_policy,
};

int
onlp_sysi_platform_manage_fans(void)
{
    int i, rc;
    onlp_fan_info_t fi[CHASSIS_FAN_COUNT];
    onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT];

    memset(fi, 0, sizeof(fi));
    memset(ti, 0, sizeof(ti));

    /* Get fan status
     */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        rc = onlp_fani_info_get(ONLP_FAN_ID_CREATE(i+1), &fi[i]);

        if (rc != ONLP_STATUS_OK) {
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
			AIM_LOG_ERROR("Unable to get fan(%d) status\r\n", i+1);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    /* Get thermal sensor status
     */
    for (i = 0; i < CHASSIS_THERMAL_COUNT; i++) {
        rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i+1), &ti[i]);
        
        if (rc != ONLP_STATUS_OK) {
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_MAX);
			AIM_LOG_ERROR("Unable to get thermal(%d) status\r\n", i+1);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    /* Apply thermal policy according the policy list,
     * If fan duty is adjusted by one of the policies, skip the others
     */
    for (i = 0; i < AIM_ARRAYSIZE(fan_control_policies); i++) {
        int adjusted = 0;

        rc = fan_control_policies[i](fi, ti, &adjusted);
        if (!adjusted) {
            continue;
        }

        return rc;
    }

    return ONLP_STATUS_OK;
}
int
onlp_sysi_platform_manage_leds(void)
{
	int i = 0, fan_fault = 0;

    /* Get each fan status
     */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
    {
        onlp_fan_info_t fan_info;

        if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to get fan(%d) status\r\n", i);
            return ONLP_STATUS_E_INTERNAL;
        }

		if (!(fan_info.status & ONLP_FAN_STATUS_PRESENT)) {
            AIM_LOG_ERROR("Fan(%d) is not present, set the fan system led as orange\r\n", i);
            fan_fault = 1;
			break;
		}

        if (fan_info.status & ONLP_FAN_STATUS_FAILED) {
            AIM_LOG_ERROR("Fan(%d) is not working, set the fan system led as orange\r\n", i);
            fan_fault = 1;
			break;
        }
    }

	return fan_fault ? onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN), ONLP_LED_MODE_ORANGE) :
					   onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN), ONLP_LED_MODE_GREEN);
}

