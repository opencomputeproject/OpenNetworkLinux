#ifndef SWITCH_MB_DRIVER_H
#define SWITCH_MB_DRIVER_H

#include "switch.h"

#define MAINBOARD_ERR(fmt, args...)      LOG_ERR("mainboard: ", fmt, ##args)
#define MAINBOARD_WARNING(fmt, args...)  LOG_WARNING("mainboard: ", fmt, ##args)
#define MAINBOARD_INFO(fmt, args...)     LOG_INFO("mainboard: ", fmt, ##args)
#define MAINBOARD_DEBUG(fmt, args...)    LOG_DBG("mainboard: ", fmt, ##args)

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

struct mainboard_drivers_t{
    ssize_t (*get_odm_bsp_version) (char* buf);
    ssize_t (*get_mgmt_mac) (char* buf);
    ssize_t (*get_date) (char* buf);
    ssize_t (*get_hw_version) (char* buf);
    ssize_t (*get_name) (char* buf);
    ssize_t (*get_sn) (char* buf);
    ssize_t (*get_chassis_sn) (char* buf);
    ssize_t (*get_vendor) (char* buf);
    ssize_t (*get_syseeprom) (char* buf);
    void (*get_loglevel) (long *lev);
    void (*set_loglevel) (long lev);
    ssize_t (*debug_help) (char *buf);
    ssize_t (*debug) (const char *buf, int count);
};

ssize_t drv_get_odm_bsp_version(char *buf);
ssize_t drv_get_mgmt_mac(char *buf);
ssize_t drv_get_date(char *buf);
ssize_t drv_get_hw_version(char *buf);
ssize_t drv_get_name(char *buf);
ssize_t drv_get_sn(char *buf);
ssize_t drv_get_chassis_sn(char *buf);
ssize_t drv_get_vendor(char *buf);
ssize_t drv_get_syseeprom(char *buf);
void drv_get_loglevel(long *lev);
void drv_set_loglevel(long lev);
ssize_t drv_debug_help(char *buf);
ssize_t drv_debug(const char *buf, int count);

void s3ip_mainboard_drivers_register(struct mainboard_drivers_t *p_func);
void s3ip_mainboard_drivers_unregister(void);

#endif /* SWITCH_MB_DRIVER_H */
