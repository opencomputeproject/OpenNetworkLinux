#ifndef _FANCONTROL_H_
#define _FANCONTROL_H_

#include "platform_comm.h"

#define INTERVAL         3
#define TEMP_CONV_FACTOR 1000
#define PWM_START        128   // 255 * 50%
#define PWM_MIN          128
#define PWM_MAX          255
#define RPM_FAULT_DEF    1000 // less than 1000rpm, regarded as fault
#define TEMP_DIFF_FAULT  15   //15 C
#define UNDEF            -100
#define OFF_H            0    // not add compensate

typedef struct linear
{
    uint8_t min_temp;
    uint8_t max_temp;
    uint8_t min_pwm;
    uint8_t max_pwm;
    uint8_t hysteresis;
    char    lasttemp;
    // effect by altitude, offset = (H-950)/175);
    char    temp_offset;
    uint8_t lastpwm;
    uint8_t lastpwmpercent;
} linear_t;

typedef struct pid
{
    char lasttemp;
    char lastlasttemp;
    uint32_t setpoint;
    float P;
    float I;
    float D;
    float lastpwmpercent;
} pid_algo_t;

int update_fan(void);

#endif
