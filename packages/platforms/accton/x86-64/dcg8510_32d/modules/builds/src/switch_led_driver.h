#ifndef SWITCH_LED_DRIVER_H
#define SWITCH_LED_DRIVER_H

#include "switch.h"

#define LED_ERR(fmt, args...)      LOG_ERR("sysled: ", fmt, ##args)
#define LED_WARNING(fmt, args...)  LOG_WARNING("sysled: ", fmt, ##args)
#define LED_INFO(fmt, args...)     LOG_INFO("sysled: ", fmt, ##args)
#define LED_DEBUG(fmt, args...)    LOG_DBG("sysled: ", fmt, ##args)

#define CPLD_FAN_LED_CTL_OFFSET     0x31
#define CPLD_SYS_LED_CTL_OFFSET     0x32
#define CPLD_BMC_LED_CTL_OFFSET     0x33
#define CPLD_PSU_LED_CTL_OFFSET     0x34

#define SYS_LED_OFF                         0x00
#define SYS_LED_GREEN_ON                    0x0C
#define SYS_LED_YELLOW_ON                   0x0D
#define SYS_LED_RED_ON                      0x01
#define SYS_LED_BLUE_ON                     0x10
#define SYS_LED_GREEN_FLASH_COME_OK         0x08
#define SYS_LED_GREEN_FLASH_BMC_OK          0x04
#define SYS_LED_YELLOW_FLASH_COME_OK        0x0A
#define SYS_LED_YELLOW_FLASH_BMC_OK         0x06
#define SYS_LED_RED_FLASH                   0x02
#define SYS_LED_BLUE_FLASH                  0x20

#define BMC_LED_OFF                         0x00
#define BMC_LED_GREEN_ON                    0x04
#define BMC_LED_YELLOW_ON                   0x05
#define BMC_LED_RED_ON                      0x01
#define BMC_LED_BLUE_ON                     0x10
#define BMC_LED_GREEN_FLASH                 0x08
#define BMC_LED_YELLOW_FLASH                0x0A
#define BMC_LED_RED_FLASH                   0x02
#define BMC_LED_BLUE_FLASH                  0x20


#define FAN_LED_OFF                         0x00
#define FAN_LED_GREEN_ON                    0x04
#define FAN_LED_YELLOW_ON                   0x05
#define FAN_LED_RED_ON                      0x01
#define FAN_LED_BLUE_ON                     0x10

#define PSU_LED_OFF                         0x00
#define PSU_LED_GREEN_ON                    0x04
#define PSU_LED_YELLOW_ON                   0x05
#define PSU_LED_RED_ON                      0x01
#define PSU_LED_BLUE_ON                     0x10


struct led_drivers_t{
    bool (*get_location_led) (unsigned char *loc);
    bool (*set_location_led) (unsigned char loc);
    int  (*get_port_led_num) (void);
    bool (*get_port_led) (unsigned long *bitmap);
    bool (*set_port_led) (unsigned long *bitmap);
    bool (*get_sys_led) (unsigned char *led);
    bool (*set_sys_led) (unsigned char led);
    bool (*get_bmc_led) (unsigned char *led);
    bool (*set_bmc_led) (unsigned char led);
    bool (*get_fan_led) (unsigned char *led);
    bool (*set_fan_led) (unsigned char led);
    bool (*get_psu_led) (unsigned char *led);
    bool (*set_psu_led) (unsigned char led);
    bool (*get_id_led) (unsigned int *led, unsigned int cs);
    bool (*set_id_led) (unsigned int led, unsigned int cs);
    bool (*set_hw_control) (void);
    void (*get_loglevel) (long *lev);
    void (*set_loglevel) (long lev);
    ssize_t (*debug_help) (char *buf);
};

bool drv_get_location_led(unsigned int *loc);
bool drv_set_location_led(unsigned int loc);
int drv_get_port_led_num(void);
bool drv_get_port_led(unsigned long *bitmap);
bool drv_set_port_led(unsigned long *bitmap);
bool drv_get_sys_led(unsigned char *led);
bool drv_set_sys_led(unsigned char led);
bool drv_get_bmc_led(unsigned char *led);
bool drv_set_bmc_led(unsigned char led);
bool drv_get_fan_led(unsigned char *led);
bool drv_set_fan_led(unsigned char led);
bool drv_get_psu_led(unsigned char *led);
bool drv_set_psu_led(unsigned char led);
bool drv_get_id_led(unsigned int *led, unsigned int cs);
bool drv_set_id_led(unsigned int led, unsigned int cs);
bool drv_set_hw_control(void);
void drv_get_loglevel(long *lev);
void drv_set_loglevel(long lev);
ssize_t drv_debug_help(char *buf);

void s3ip_led_drivers_register(struct led_drivers_t *p_func);
void s3ip_led_drivers_unregister(void);

#endif /* SWITCH_LED_DRIVER_H */
