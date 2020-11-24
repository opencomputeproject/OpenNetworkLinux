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
#include <limits.h>

#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#include "x86_64_accton_as7315_27xb_int.h"
#include "x86_64_accton_as7315_27xb_log.h"

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as7315-27xb-r0";
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

    /* 5 Thermal sensors on the chassis */
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
    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}






#define FAN_DUTY_MAX  (100)



extern int fani_enable_fan(int fid, bool enable) ;
extern int fani_is_fan_enabled(int fid, bool *enable);
extern int fani_get_fan_duty(int fid, int *duty);

static int
_set_all_fans_pwm(int duty)
{
    int i, ret;
    int duties_nor[CHASSIS_FAN_COUNT] = {[0 ... CHASSIS_FAN_COUNT-1] = duty};
    /*Low-duty may not drive fan rotating.*/
    int duties_low[CHASSIS_FAN_COUNT] = {0, 41, 0, 41, 0};
    int *duties;

    if (duty <= 20) {
        duties = duties_low;
    } else {
        duties = duties_nor;
    }

    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        ret = onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i+1), duties[i]);
        if (ret != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to onlp_fani_percentage_set(%d), ret:%d\r\n",
                          i+1, ret);
            return ret;
        }
        ret = fani_enable_fan(i+1, !!duties[i]);
        if (ret != ONLP_STATUS_OK) {
            return ret;
        }
    }

    return ONLP_STATUS_OK;
}

static int
sysi_fanctrl_fan_fault_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                              onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                              int *adjusted)
{
    int i, fault;
    bool enable;

    *adjusted = 0;
    /* Bring fan speed to FAN_DUTY_MAX if any fan is not operational */
    for (i = 0; i < CHASSIS_FAN_COUNT; i++) {
        if (!ONLP_FAN_STATUS_PRESENT(fi[i])) {
            *adjusted = 1;
            break;
        }

        /*True fault if fan is enabled.*/
        if( fani_is_fan_enabled(i+1,&enable) != ONLP_STATUS_OK) {
            *adjusted = 1;
            break;
        }
        fault = !!ONLP_FAN_STATUS_FAILED(fi[i]);
        if (fault && enable) {
            *adjusted = 1;
            break;
        }
    }

    if (*adjusted) {
        return _set_all_fans_pwm(FAN_DUTY_MAX);
    }

    return ONLP_STATUS_OK;
}

static int get_ambient_thermal(onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT]) {
    int i = 2;  /*Take LM75 0x4A, supposed to be indexed 2*/
    char *tSenorKey = "4A";

    /*check node desc.*/
    if (NULL == AIM_STRSTR(ti[i].hdr.description, tSenorKey)) {
        _set_all_fans_pwm(FAN_DUTY_MAX);
        AIM_DIE("%s: Cannot find the thermal sensor: %s\n", __func__, tSenorKey);
    }
    return ti[i].mcelsius;
}

static int
sysi_fanctrl_main_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                         onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                         int *adjusted)
{
    int duty, thermal;
    enum {E_DOWN, E_UP, E_THRESHS};
    enum {_LOW, _NORMAL, _MID, _HIGH, _MAX} static stage = _MID;
    const int duties[] = {20, 30, 60, FAN_DUTY_MAX};
    const int thrs[E_THRESHS][_MAX] = {{INT_MIN, 1000, 36000, 54000},
                                               {10000, 47000, 62000, INT_MAX}};

    thermal = get_ambient_thermal(ti);

    switch(stage) {
    case _LOW:
    case _NORMAL:
    case _MID:
    case _HIGH:
        if (thermal < thrs[E_DOWN][stage]) {
            stage--;
        } else if (thermal > thrs[E_UP][stage]) {
            stage++;
        }
        break;
    default:
        stage = _HIGH;
    }
    duty = duties[stage];
    *adjusted = 1;
    return _set_all_fans_pwm(duty);
}

static int
sysi_fanctrl_thermal_shutdown_policy(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                     onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                     int *adjusted)
{
    /*Not implemented yet.*/
    *adjusted = 0;
    return ONLP_STATUS_OK;
}

typedef int (*fan_control_policy)(onlp_fan_info_t fi[CHASSIS_FAN_COUNT],
                                  onlp_thermal_info_t ti[CHASSIS_THERMAL_COUNT],
                                  int *adjusted);

fan_control_policy fan_control_policies[] = {
    sysi_fanctrl_thermal_shutdown_policy,
    sysi_fanctrl_fan_fault_policy,
    sysi_fanctrl_main_policy,
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
            AIM_LOG_ERROR("Unable to get fan(%d) status\r\n", i+1);
            _set_all_fans_pwm(FAN_DUTY_MAX);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    /* Get thermal sensor status
     */
    for (i = 0; i < CHASSIS_THERMAL_COUNT; i++) {
        rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i+1), &ti[i]);

        if (rc != ONLP_STATUS_OK) {
            _set_all_fans_pwm(FAN_DUTY_MAX);
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


