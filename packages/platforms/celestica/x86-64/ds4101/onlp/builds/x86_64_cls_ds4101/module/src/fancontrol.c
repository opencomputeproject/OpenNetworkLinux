#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <math.h>
#include "fancontrol.h"
#include "platform_wobmc.h"

extern uint8_t BMC_Status;
extern struct fan Fan[FAN_COUNT + 1];
extern struct temp Temp_B2F[THERMAL_COUNT + 1];
extern struct temp Temp_F2B[THERMAL_COUNT + 1];
extern uint8_t Sys_Airflow;

linear_t U15LINEAR_F2B = {
    39, 52, 128, 255, 3, UNDEF, 0, 0, 0
};

linear_t U16LINEAR_F2B = {
    39, 52, 128, 255, 3, UNDEF, 0, 0, 0
};

/* temperature unit is mC, so setpoint multiple 1000 to be the same level */
linear_t CPULINEAR_F2B = {
    67, 85, 66, 255, 3, UNDEF, 0, 0, 0
};

pid_algo_t CPU_PID = {
     UNDEF, 0, 96, 2.5, 0.15, 0.2, 0
};

pid_algo_t SW_PID = {
    UNDEF, 0, 94, 3, 0.2, 0.2, 0
};


static short get_temp(const char *path)
{
    short temp = 0;
    long t  = 0;
    int ret = 0;

    ret = get_sysnode_value(path, (void *)&t);
    if (ret)
    {
        return ERR;
    }
    temp = t / TEMP_CONV_FACTOR;

    return temp;
}

static int set_fan(const char *path, uint8_t pwm)
{
    int ret = 0;

    ret = set_sysnode_value(path, pwm);

    return ret;
}

static int set_all_fans(uint8_t pwm)
{
    int ret = 0, i = 0;

    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        ret = set_fan(Fan[i].pwm_path, pwm);
        if (ret)
        {
            printf("Can't set fan%d pwm\n", i);
            return ERR;
        }
    }

    return 0;
}


static short get_valid_temp(char *path, uint8_t sec)
{
    uint8_t i = 0, cnt = 0;
    int temp_sum = 0;
    short temp = 0;
    static short last_temp = 0;

    for (i = 0; i < 5; i++) {
        temp = get_temp(path);
        if (temp != -1) {
            if (abs(last_temp - temp) > TEMP_DIFF_FAULT) {
                temp_sum += temp;
                cnt++;
            }
            else {
                break;
            }
        }
        sleep(sec);
    }

    /* can not get temperture or reach maximum temperture for 5 times */
    if (0 == cnt && 5 == i) {
        return ERR;
    /* get valid temperture */
    } else {
        if (5 == i) { // every temperture is larger than the maximum difference
            temp = temp_sum / cnt;
        }
        last_temp = temp;
    }

    return temp;
}

static short upper_bound(char temp, uint8_t min_temp, uint8_t max_temp,
                                uint8_t min_pwm, uint8_t max_pwm,
                                uint8_t hysteresis, char temp_offset)
{
    short pwm = 0;

    pwm = (temp - min_temp + temp_offset) * (max_pwm - min_pwm) /
                                (max_temp - min_temp - hysteresis) + min_pwm;
    DEBUG_PRINT("[linear] temp %d, min_temp %d, max_temp %d, min_pwm %d, "
                  "max_pwm %d, hysteresis %d, pwm %d, temp_offset %d\n",
                  temp, min_temp, max_temp, min_pwm, max_pwm, hysteresis, pwm, temp_offset);

    return pwm > 0 ? pwm : 0;
}

static short lower_bound(char temp, uint8_t min_temp, uint8_t max_temp,
                                uint8_t min_pwm, uint8_t max_pwm,
                                uint8_t hysteresis, char temp_offset)
{
    short pwm = 0;

    pwm = (temp - min_temp + temp_offset - hysteresis) * (max_pwm - min_pwm) /
                                (max_temp - min_temp - hysteresis) + min_pwm;
    DEBUG_PRINT("[linear] temp %d, min_temp %d, max_temp %d, min_pwm %d, "
                  "max_pwm %d, hysteresis %d, pwm %d, temp_offset %d\n",
                  temp, min_temp, max_temp, min_pwm, max_pwm, hysteresis, pwm, temp_offset);

    return pwm > 0 ? pwm : 0;
}

static uint8_t linear_cal(uint8_t temp, linear_t *l, uint8_t altitude_effect)
{
    uint8_t pwm = 0;
    short ub = 0, lb = 0;
    int pressure_raw = 0;
    float pressure_scale  = 0.0;
    double pressure = 0.0;
    int altitude = 0;
    int ret = 0;
    char tmp[15];

    if (altitude_effect) {
        ret = read_device_node_string(PRESSURE_RAW_PATH, tmp, sizeof(tmp), 0);
        if (ret) {
            return ERR;
        }
        pressure_raw = atoi(tmp);
        ret = read_device_node_string(PRESSURE_SCALE_PATH, tmp, sizeof(tmp), 0);
        if (ret) {
            return ERR;
        }
        pressure_scale = atof(tmp);
        pressure = pressure_raw * pressure_scale * 1000;
        DEBUG_PRINT("[linear] Pressure: %lf Pa\n", pressure);
        // when height is less than default height 900 M(90970 Pa), set temp_offset as 0
        if (pressure < 90970 || DEBUG_MODE) {
            altitude = 44330.77 * (1 - pow(pressure/101326, 0.1902632)) + OFF_H;
            l->temp_offset = (altitude - 950) / 175;
            DEBUG_PRINT("[linear]  Altitude: %d M, temp offset: %d\n", altitude, l->temp_offset);
        } else
            l->temp_offset = 0;
    }

    ub = upper_bound(temp, l->min_temp, l->max_temp,
                           l->min_pwm, l->max_pwm,
                           l->hysteresis, l->temp_offset);
    lb = lower_bound(temp, l->min_temp, l->max_temp,
                           l->min_pwm, l->max_pwm,
                           l->hysteresis, l->temp_offset);

    if (UNDEF == l->lasttemp) {
        l->lasttemp = temp;
        l->lastpwm = lb;
        DEBUG_PRINT("[linear] start lb pwm %d,  preset initial pwm %d\n", lb, PWM_START);
        return lb > PWM_START ? lb : PWM_START;
    }
    if (temp >= l->max_temp - l->temp_offset) {
        l->lasttemp = temp;
        l->lastpwm = PWM_MAX;
        DEBUG_PRINT("[linear] set max pwm %d\n", PWM_MAX);
        return PWM_MAX;
    } else if (temp < l->min_temp - l->temp_offset) {
        l->lasttemp = temp;
        l->lastpwm = PWM_MIN;
        DEBUG_PRINT("[linear] set min pwm %d\n", PWM_MIN);
        return PWM_MIN;
    }

    DEBUG_PRINT("[linear] temp %d, lasttemp %d\n", temp, l->lasttemp);
    if (temp > l->lasttemp)
    {
        DEBUG_PRINT("[linear] temperature rises from %d to %d\n", l->lasttemp, temp);
        if (lb < l->lastpwm)
        {
            pwm = l->lastpwm;
        }
        else
        {
            pwm = lb;
        }
    }
    else if (temp < l->lasttemp)
    {
        DEBUG_PRINT("[linear] temperature declines from %d to %d\n", l->lasttemp, temp);
        if (ub > l->lastpwm)
        {
            pwm = l->lastpwm;
        }
        else
        {
            pwm = ub;
        }
    }
    else
    {
        DEBUG_PRINT("[linear] temperature keeps to %d\n", temp);
        pwm = l->lastpwm;
    }
    if (pwm > l->max_pwm)
    {
        pwm = l->max_pwm;
    }
    else if (pwm < l->min_pwm)
    {
        pwm = l->min_pwm;
    }
    DEBUG_PRINT("[linear] last pwm: %d, new pwm: %d\n", l->lastpwm, pwm);
    l->lasttemp = temp;
    l->lastpwm = pwm;

    return pwm;
}

static uint8_t pid_cal(uint8_t temp, pid_algo_t *pid)
{
    int pweight = 0;
    int iweight = 0;
    int dweight = 0;
    uint8_t pwmpercent = 0;
    uint8_t pwm = 0;

    if (UNDEF == pid->lasttemp) {
        pid->lasttemp = temp;
        pid->lastlasttemp = temp;
        pid->lastpwmpercent = PWM_MIN * 100 / PWM_MAX;
        return PWM_MIN;
    }

    pweight = temp - pid->lasttemp;
    iweight = temp - pid->setpoint;
    dweight = temp - 2 * (pid->lasttemp) + pid->lastlasttemp;
    DEBUG_PRINT("[pid] temp %d, lasttemp %d, lastlasttemp %d\n",
                               temp, pid->lasttemp, pid->lastlasttemp);
    DEBUG_PRINT("[pid] setpoint %d, P %f, I %f, D %f, pweight %d, iweight %d, dweight %d\n",
                 pid->setpoint, pid->P, pid->I, pid->D, pweight, iweight, dweight);

    /* Add 0.5 pwm to support rounding */
    pwmpercent = (int)(pid->lastpwmpercent +
                  pid->P * pweight + pid->I * iweight + pid->D * dweight + 0.5);
    DEBUG_PRINT("[pid] percent before cal %d\n", pwmpercent);
    pwmpercent = pwmpercent < (PWM_MIN * 100 / PWM_MAX) ? (PWM_MIN * 100 / PWM_MAX) : pwmpercent;
    pwmpercent = pwmpercent > 100 ? 100 : pwmpercent;
    DEBUG_PRINT("[pid] percent after cal %d\n", pwmpercent);
    pwm = pwmpercent * PWM_MAX / 100;
    pid->lastlasttemp = pid->lasttemp;
    pid->lasttemp = temp;
    pid->lastpwmpercent = pwmpercent;
    DEBUG_PRINT("[pid] set pwm %d\n", pwm);

    return pwm;
}

int update_fan(void)
{
    int ret = 0, i = 0;
    uint8_t pwm = 0, fan_full_speed_fault_num = 0;
    static uint8_t fan_status = 0;
    struct temp *temp_array;

    // pwm
    uint8_t cpu_pwm = 0, sw_pwm = 0;
    uint8_t u15_pwm = 0, u16_pwm = 0;

    // Configuration
    uint8_t fan_out_full_speed_enable = 0;
    uint8_t fan_in_duty_speed_enable = 0;
    uint8_t psu_absent_full_speed_enable = 0;
    uint8_t temp_fail_full_speed_enable = 0;
    uint8_t fan_fault_full_speed_enable = 0;
    uint8_t over_temp_full_speed_warn_enable = 0;
    uint8_t fan_in_duty_speed_percent = 70;
    uint8_t fan_in_duty_speed_duration = 20; // seconds

    fan_out_full_speed_enable = 1;
    fan_in_duty_speed_enable = 1;
    psu_absent_full_speed_enable = 1;
    temp_fail_full_speed_enable = 1;
    fan_fault_full_speed_enable = 1;
    over_temp_full_speed_warn_enable = 1;
    fan_full_speed_fault_num = 2;
    fan_in_duty_speed_percent = 70;
    fan_in_duty_speed_duration = 20; // seconds

    if (FAN_F2B == Sys_Airflow) {
        temp_array = Temp_F2B;
    } else {
        temp_array = Temp_B2F;
    }

    for (i = 1; i <= THERMAL_COUNT; i++) {
        if (!strcmp(temp_array[i].descr, "TEMP_BMC") &&  ABSENT == BMC_Status) {
            continue;
        }
        if (!strcmp(temp_array[i].descr, "TEMP_CPU") || !strcmp(temp_array[i].descr, "TEMP_SW_Internal")) {
            /* Eliminate CPU/SW internal temperature jitter */
            temp_array[i].temp = get_valid_temp(temp_array[i].path, 0);
        } else {
            temp_array[i].temp = get_temp(temp_array[i].path);
        }
        /*
         * Any accessing temperature failure execpt DIMM will cause full speed speed
         * because DIMM number is configurable
         */
        if (temp_fail_full_speed_enable && ERR == temp_array[i].temp &&
			NULL == strstr(temp_array[i].descr, "DIMM")) {
            DEBUG_PRINT("fail to get %s, set full pwm: %d\n\n", temp_array[i].descr, PWM_MAX);
            set_all_fans(PWM_MAX);
            return 0;
        }

        DEBUG_PRINT("%s: %d\n", temp_array[i].descr, temp_array[i].temp);
        if (NOT_DEFINE != temp_array[i].hi_shutdown &&
                  temp_array[i].temp > temp_array[i].hi_shutdown / TEMP_CONV_FACTOR) {

            /* soft shutdown */
            /*Record high temperature alarm */
            syslog(LOG_WARNING, "%s temperature %d is over than %d, machine should be shutdown !!!\n",
                           temp_array[i].descr, temp_array[i].temp, temp_array[i].hi_shutdown / TEMP_CONV_FACTOR);
            closelog();
            system("sync");

            /* switch board power off */
            ret = syscpld_setting(LPC_SWITCH_PWCTRL_REG, SWITCH_OFF);
            if (ret)
            {
                perror("Fail to power off switch board !!!");
            }

            temp_array[i].is_ovshutdown_fault = 1;
            /* wait for ledcontrol service to set alarm LED */
            sleep(5);

            system("reboot");
        }

        if (NOT_DEFINE != temp_array[i].hi_err &&
                  temp_array[i].temp >= temp_array[i].hi_err / TEMP_CONV_FACTOR) {
            /* warning if sensor value is higher than major alarm */
            if (0 == temp_array[i].is_overr_fault && over_temp_full_speed_warn_enable)
                syslog(LOG_WARNING, "%s temperature %d is over than %d !!!\n",
                                temp_array[i].descr, temp_array[i].temp, temp_array[i].hi_err / TEMP_CONV_FACTOR);

            temp_array[i].is_overr_fault = 1;
            temp_array[i].is_ovwarn_fault = 0;
        } else if (NOT_DEFINE != temp_array[i].hi_warn &&
                         temp_array[i].temp >= temp_array[i].hi_warn / TEMP_CONV_FACTOR) {
            if (0 == temp_array[i].is_ovwarn_fault && over_temp_full_speed_warn_enable)
                syslog(LOG_WARNING, "%s temperature %d is over than %d !!!\n",
                                temp_array[i].descr, temp_array[i].temp, temp_array[i].hi_warn / TEMP_CONV_FACTOR);

            temp_array[i].is_overr_fault = 0;
            temp_array[i].is_ovwarn_fault = 1;
        } else {
            temp_array[i].is_overr_fault = 0;
            temp_array[i].is_ovwarn_fault = 0;
        }
    }

    ret = check_psu_absent();
    if (0 != ret && psu_absent_full_speed_enable) {
        DEBUG_PRINT("psu absent, set full pwm: %d\n\n", PWM_MAX);
        set_all_fans(PWM_MAX);
        return 0;
    }

    ret = check_fan_absent();
    if (0 != ret && fan_out_full_speed_enable) {
        DEBUG_PRINT("fan absent, set full pwm: %d\n\n", PWM_MAX);
        set_all_fans(PWM_MAX);
        fan_status = ABSENT;
        return 0;
    } else if (0 == ret && ABSENT == fan_status && fan_in_duty_speed_enable) {
        DEBUG_PRINT("fan plug in, set duty pwm: %d\n\n", fan_in_duty_speed_percent * 255 / 100);
        set_all_fans(fan_in_duty_speed_percent * 255 / 100);
        fan_status = PRESENT;
        sleep(fan_in_duty_speed_duration);
    }

    ret = check_fan_fault();
    if (0 != ret && fan_full_speed_fault_num == ret && fan_fault_full_speed_enable) {
        DEBUG_PRINT("fan fault, set full pwm: %d\n\n", PWM_MAX);
        set_all_fans(PWM_MAX);
        return 0;
    } else {
        /* calculate all PWMs */
        if (FAN_F2B == Sys_Airflow) {
            DEBUG_PRINT("\n[pid] CPU temperature\n");
            cpu_pwm = pid_cal(temp_array[TEMP_CPU_ID].temp, &CPU_PID);
            DEBUG_PRINT("\n[pid] SW temperature\n");
            sw_pwm  = pid_cal(temp_array[TEMP_SW_Internal_ID].temp, &SW_PID);
            DEBUG_PRINT("\n[linear] U15 temperature\n");
            u15_pwm = linear_cal(temp_array[TEMP_SW_U15_ID].temp, &U15LINEAR_F2B, 1);
            DEBUG_PRINT("\n[linear] U16 temperature\n");
            u16_pwm = linear_cal(temp_array[TEMP_SW_U15_ID].temp, &U16LINEAR_F2B, 1);
        }
        /* get max PWM */
        DEBUG_PRINT("\n[final] cpu_pwm %d sw_pwm %d u15_pwm %d u16_pwm %d\n",
                                           cpu_pwm, sw_pwm, u15_pwm, u16_pwm);
        pwm = cpu_pwm > sw_pwm ? cpu_pwm : sw_pwm;
        pwm = pwm > u15_pwm ? pwm : u15_pwm;
        pwm = pwm > u16_pwm ? pwm : u16_pwm;
        pwm = pwm > PWM_MAX ? PWM_MAX : pwm;
        /* set max PWM to all fans */
        DEBUG_PRINT("[final] set normal pwm: %d\n\n", pwm);
        set_all_fans(pwm);
    }

    return 0;
}
