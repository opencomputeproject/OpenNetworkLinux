#ifndef SWITCH_BMC_DRIVER_H
#define SWITCH_BMC_DRIVER_H

#include "switch.h"

#define BMC_ERR(fmt, args...)      LOG_ERR("bmc: ", fmt, ##args)
#define BMC_WARNING(fmt, args...)  LOG_WARNING("bmc: ", fmt, ##args)
#define BMC_INFO(fmt, args...)     LOG_INFO("bmc: ", fmt, ##args)
#define BMC_DEBUG(fmt, args...)    LOG_DBG("bmc: ", fmt, ##args)

#define CHECK_BIT(a, b)  (((a) >> (b)) & 1U)

#define SYS_CPLD_SOCKET_FULLIN_REG_OFFSET   0x10b
#define SYS_CPLD_REG_BMC_HB_STATE_OFFSET    0x1f10
#define SYS_CPLD_REG_BMC_DIS_ADDR           0x1f35

struct bmc_drivers_t{
    ssize_t (*get_status) (char* buf);
    void (*get_loglevel) (long *lev);
    void (*set_loglevel) (long lev);
    ssize_t (*get_enable) (char* buf);
};

ssize_t drv_get_status(char* buf);
void drv_get_loglevel(long *lev);
void drv_set_loglevel(long lev);

void s3ip_bmc_drivers_register(struct bmc_drivers_t *p_func);
void s3ip_bmc_drivers_unregister(void);

#endif /* SWITCH_BMC_DRIVER_H */
