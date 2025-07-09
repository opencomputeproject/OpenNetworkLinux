#ifndef SWITCH_FPGA_DRIVER_H
#define SWITCH_FPGA_DRIVER_H

#include "switch.h"

#define FPGA_ERR(fmt, args...)      LOG_ERR("fpga: ", fmt, ##args)
#define FPGA_WARNING(fmt, args...)  LOG_WARNING("fpga: ", fmt, ##args)
#define FPGA_INFO(fmt, args...)     LOG_INFO("fpga: ", fmt, ##args)
#define FPGA_DEBUG(fmt, args...)    LOG_DBG("fpga: ", fmt, ##args)

#define FPGA_TOTAL_NUM			1
#define FPGA_VER_OFFSET         0x00
#define FPGA_SCRATCHPAD_OFFSET  0x04

struct fpga_drivers_t{
    ssize_t (*get_alias) (unsigned int index, char *buf);
    ssize_t (*get_type) (unsigned int index, char *buf);
    ssize_t (*get_hw_version) (unsigned int index, char *buf);
    ssize_t (*get_board_version) (unsigned int index, char *buf);
    ssize_t (*get_reg_test) (unsigned int index, char *buf);
    ssize_t (*set_reg_test) (unsigned int index, unsigned int data);
    void (*get_loglevel) (long *lev);
    void (*set_loglevel) (long lev);
    ssize_t (*debug_help) (char *buf);
    ssize_t (*debug) (const char *buf, int count);
};


ssize_t drv_get_alias(unsigned int index, char *buf);
ssize_t drv_get_type(unsigned int index, char *buf);
ssize_t drv_get_hw_version(unsigned int index, char *buf);
ssize_t drv_get_board_version(unsigned int index, char *buf);
ssize_t drv_get_reg_test(unsigned int index, char *buf);
ssize_t drv_set_reg_test(unsigned int index, unsigned int data);
void drv_get_loglevel(long *lev);
void drv_set_loglevel(long lev);
ssize_t drv_debug_help(char *buf);
ssize_t drv_debug(const char *buf, int count);

void s3ip_fpga_drivers_register(struct fpga_drivers_t *p_func);
void s3ip_fpga_drivers_unregister(void);

#endif /* SWITCH_FPGA_DRIVER_H */
