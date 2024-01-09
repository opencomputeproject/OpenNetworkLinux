#ifndef _FANCONTROL_H_
#define _FANCONTROL_H_

#define INTERVAL         3
#define TEMP_CONV_FACTOR 1000
#define PWM_START        128   // 255 * 50%
#define PWM_MIN          77
#define PWM_MAX          255
#define PWM_FAULT_DEF_FRONT  7200
#define PWM_FAULT_DEF_REAR   6600


typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

typedef struct linear
{
    uint8_t min_temp;
    uint8_t max_temp;
    uint8_t min_pwm;
    uint8_t max_pwm;
    uint8_t hysteresis;
    char    lasttemp;
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

int check_psu_status(void);
int check_fan_status(uint8_t *fault_num);
int update_fan(void);
int set_all_fans(uint8_t pwm);

#endif
