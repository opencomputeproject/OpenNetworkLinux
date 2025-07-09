#ifndef SWITCH_AVS_DRIVER_H
#define SWITCH_AVS_DRIVER_H

#include "switch.h"

#define AVS_ERR(fmt, args...)      LOG_ERR("avs: ", fmt, ##args)
#define AVS_WARNING(fmt, args...)  LOG_WARNING("avs: ", fmt, ##args)
#define AVS_INFO(fmt, args...)     LOG_INFO("avs: ", fmt, ##args)
#define AVS_DEBUG(fmt, args...)    LOG_DBG("avs: ", fmt, ##args)

#define CHECK_BIT(a, b)  (((a) >> (b)) & 1U)

#define TOTAL_MP2882_NUM         1
#define TOTAL_MP2975_NUM         1
#define TOTAL_FMEA_AVS_NUM       (TOTAL_MP2882_NUM + TOTAL_MP2975_NUM)
#define AVS_INDEX_START          0

struct avs_drivers_t{
    ssize_t (*fmea_get_work_status) (unsigned int index, char* buf, char* plt);
    ssize_t (*fmea_get_current_status) (unsigned int index, char* buf, char* plt);
    ssize_t (*fmea_get_pmbus_status) (unsigned int index, char* buf, char* plt);
    void (*get_loglevel) (long *lev);
    void (*set_loglevel) (long lev);
};

ssize_t drv_fmea_get_work_status(unsigned int index, char* buf, char* plt);
ssize_t drv_fmea_get_current_status(unsigned int index, char* buf, char* plt);
ssize_t drv_fmea_get_pmbus_status(unsigned int index, char* buf, char* plt);
void drv_get_loglevel(long *lev);
void drv_set_loglevel(long lev);

void s3ip_avs_drivers_register(struct avs_drivers_t *p_func);
void s3ip_avs_drivers_unregister(void);

#endif /* SWITCH_AVS_DRIVER_H */
