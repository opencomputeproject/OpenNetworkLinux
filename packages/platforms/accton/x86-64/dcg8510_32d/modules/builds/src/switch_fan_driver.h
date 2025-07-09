#ifndef SWITCH_FAN_DRIVER_H
#define SWITCH_FAN_DRIVER_H

#include "switch.h"

#define FAN_ERR(fmt, args...)      LOG_ERR("fan: ", fmt, ##args)
#define FAN_WARNING(fmt, args...)  LOG_WARNING("fan: ", fmt, ##args)
#define FAN_INFO(fmt, args...)     LOG_INFO("fan: ", fmt, ##args)
#define FAN_DEBUG(fmt, args...)    LOG_DBG("fan: ", fmt, ##args)

#define MAX_FAN_NUM     6
#define MAX_MOTOR_NUM   2
#define FAN_DIRECT_F2B  0
#define FAN_DIRECT_B2F  1

#define FAN_EEPROM_SIZE 512

#define FAN_ONLINE_OFFSET               0x00
#define FAN_DIRECTION_OFFSET            0x02
#define FAN_PLUG_HIS_OFFSET             0x04
#define FAN_PWM_CTL1_OFFSET             0x08
#define FAN_PWM_CTL2_OFFSET             0x09
#define FAN_PWM_CTL3_OFFSET             0x0a
#define FAN_PWM_CTL4_OFFSET             0x0b
#define FAN_PWM_CTL5_OFFSET             0x0c
#define FAN_LED_CTL1_OFFSET             0x20
#define FAN_LED_CTL2_OFFSET             0x21
#define FAN_POWERGOOD_OFFSET            0x24

#define LED_ALL_ON                      0x00
#define LED_GREEN_ON                    0x0c
#define LED_GREEN_FLASH_SLOW            0x0d
#define LED_GREEN_FLASH_FAST            0x0e
#define LED_RED_ON                      0x03
#define LED_RED_FLASH_SLOW              0x07
#define LED_RED_FLASH_FAST              0x0b
#define LED_ALL_OFF                     0x0f

struct fan_drivers_t{
    ssize_t (*get_model_name) (unsigned int index, char* buf);
    ssize_t (*get_sn) (unsigned int index, char* buf);
    ssize_t (*get_vendor) (unsigned int index, char* buf);
    ssize_t (*get_part_number) (unsigned int index, char* buf);
    ssize_t (*get_hw_version) (unsigned int index, char* buf);
    unsigned int (*get_number) (void);
    bool (*get_alarm) (unsigned int index, int* alarm);
    ssize_t (*get_speed) (unsigned int fan_index, unsigned int motor_index, unsigned int *speed);
    ssize_t (*get_speed_target) (unsigned int fan_index, unsigned int motor_index, unsigned int *speed_target);
    ssize_t (*get_speed_max) (unsigned int fan_index, unsigned int motor_index, unsigned int *speed);
    ssize_t (*get_speed_min) (unsigned int fan_index, unsigned int motor_index, unsigned int *speed);
    ssize_t (*get_speed_tolerance) (unsigned int fan_index, unsigned int motor_index, unsigned int *speed_tolerance);
    unsigned int (*get_status) (unsigned int fan_index);
    ssize_t (*get_pwm) (unsigned int index, int *pwm);
    ssize_t (*set_pwm) (unsigned int index, int pwm);
    ssize_t (*get_wind) (unsigned int fan_index, unsigned int *wind);
    ssize_t (*get_led_status) (unsigned int fan_index, unsigned int *led);
    ssize_t (*set_led_status) (unsigned int fan_index, unsigned int led);
    void (*get_loglevel) (long *lev);
    void (*set_loglevel) (long lev);
    ssize_t (*get_alarm_threshold) (char *buf);
    ssize_t (*get_scan_period) (char *buf);
    void (*set_scan_period) (unsigned int period);
    ssize_t (*debug_help) (char *buf);
    ssize_t (*debug) (const char *buf, int count);
    int (*get_fmea_status) (unsigned int fan_index);
    void (*set_fmea_status) (unsigned int fan_index, int status);
};

ssize_t drv_get_model_name(unsigned int fan_index, char *buf);
ssize_t drv_get_sn(unsigned int fan_index, char *buf);
ssize_t drv_get_vendor(unsigned int fan_index, char *buf);
ssize_t drv_get_part_number(unsigned int fan_index, char *buf);
ssize_t drv_get_hw_version(unsigned int fan_index, char *buf);
unsigned int drv_get_number(void);
bool drv_get_plug_his(unsigned int fan_index);
bool drv_get_alarm(unsigned int fan_index, int *alarm);
ssize_t drv_get_speed(unsigned int fan_index, unsigned int motor_index, unsigned int *speed);
ssize_t drv_get_speed_target(unsigned int fan_index, unsigned int motor_index, unsigned int *speed_target);
ssize_t drv_get_speed_max(unsigned int fan_index, unsigned int motor_index, unsigned int *speed);
ssize_t drv_get_speed_min(unsigned int fan_index, unsigned int motor_index, unsigned int *speed);
ssize_t drv_get_speed_tolerance(unsigned int fan_index, unsigned int motor_index, unsigned int *speed_tolerance);
unsigned int drv_get_status(unsigned int fan_index);
ssize_t drv_get_pwm(unsigned int fan_index, int *pwm);
ssize_t drv_set_pwm(unsigned int fan_index, int pwm);
ssize_t drv_get_alarm_threshold(char *buf);
ssize_t drv_get_wind(unsigned int fan_index, unsigned int *wind);
ssize_t drv_get_led_status(unsigned int fan_index, unsigned int *led);
ssize_t drv_set_led_status(unsigned int fan_index, unsigned int led);
ssize_t drv_get_scan_period(char *buf);
void drv_set_scan_period(unsigned int period);
void drv_get_loglevel(long *lev);
void drv_set_loglevel(long lev);
ssize_t drv_debug_help(char *buf);
ssize_t drv_debug(const char *buf, int count);
int drv_get_fmea_status(unsigned int fan_index);
void drv_set_fmea_status(unsigned int fan_index, int status);

void s3ip_fan_drivers_register(struct fan_drivers_t *p_func);
void s3ip_fan_drivers_unregister(void);

#endif /* SWITCH_FAN_DRIVER_H */
