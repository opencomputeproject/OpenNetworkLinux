#ifndef SWITCH_WDT_DRIVER_H
#define SWITCH_WDT_DRIVER_H

#include "switch.h"

#define WDT_ERR(fmt, args...)      LOG_ERR("watchdog: ", fmt, ##args)
#define WDT_WARNING(fmt, args...)  LOG_WARNING("watchdog: ", fmt, ##args)
#define WDT_INFO(fmt, args...)     LOG_INFO("watchdog: ", fmt, ##args)
#define WDT_DEBUG(fmt, args...)    LOG_DBG("watchdog: ", fmt, ##args)

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

#define BIT7                    (1 << 7)
#define BIT6                    (1 << 6)
#define BIT5                    (1 << 5)
#define BIT4                    (1 << 4)
#define BIT3                    (1 << 3)
#define BIT2                    (1 << 2)
#define BIT1                    (1 << 1)
#define BIT0                    (1 << 0)

#define WDT_CFG_WDT_TRIGGER_REPORT      BIT7
#define WDT_CFG_WDT_M1_DELAY_TRIGGER    BIT6
#define WDT_CFG_WDT_M1_MODE             BIT5
#define WDT_CFG_WDT_CLEAR               BIT4
#define WDT_CFG_WDT_M3_ENABLE           BIT3
#define WDT_CFG_WDT_M2_ENABLE           BIT2
#define WDT_CFG_WDT_M1_ENABLE           BIT1
#define WDT_CFG_WDT_ENABLE              BIT0

#define EC_LPC_IO_BASE_ADDR     0x6300
#define EC_WDT_TIMER_MSB_REG_OFFSET         0x06
#define EC_WDT_TIMER_LSB_REG_OFFSET         0x07
#define EC_WDT_CFG_REG_OFFSET               0x08
#define EC_WDT_TIMER_MSB_RESET_REG_OFFSET   0x0c
#define EC_WDT_TIMER_LSB_RESET_REG_OFFSET   0x0d

#define GPIO_IO_BASE                    0x500
#define GPIO_USE_SEL2_BYTE2             0x32 //gpio[48~55]
#define GP_IO_SEL2_BYTE2                0x36 //gpio[48~55]
#define GP_LVL2_BYTE2                   0x3A //gpio[48~55]


struct wdt_drivers_t{
    ssize_t (*get_identify) (char *buf);
    ssize_t (*get_state) (char *buf);
    ssize_t (*get_timeleft) (char *buf);
    ssize_t (*get_timeout) (char *buf);
    void (*set_timeout) (unsigned short timeout);
    void (*set_reset) (unsigned int reset);
    ssize_t (*get_enable) (char *buf);
    bool (*set_enable) (unsigned int enable);
    void (*get_loglevel) (long *lev);
    void (*set_loglevel) (long lev);
    ssize_t (*debug_help) (char *buf);
    ssize_t (*debug) (const char *buf, int count);
};

ssize_t drv_get_identify(char *buf);
ssize_t drv_get_state(char *buf);
ssize_t drv_get_timeleft(char *buf);
ssize_t drv_get_timeout(char *buf);
void drv_set_timeout(unsigned short timeout);
void drv_set_reset(unsigned int reset);
ssize_t drv_get_enable(char *buf);
bool drv_set_enable(unsigned int enable);
void drv_get_loglevel(long *lev);
void drv_set_loglevel(long lev);
ssize_t drv_debug_help(char *buf);
ssize_t drv_debug(const char *buf, int count);

void s3ip_wdt_drivers_register(struct wdt_drivers_t *p_func);
void s3ip_wdt_drivers_unregister(void);

#endif /* SWITCH_MB_DRIVER_H */
