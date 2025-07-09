#ifndef SWITCH_CPLD_DRIVER_H
#define SWITCH_CPLD_DRIVER_H

#include <linux/cdev.h>
#include "switch.h"

#define CPLD_ERR(fmt, args...)      LOG_ERR("cpld: ", fmt, ##args)
#define CPLD_WARNING(fmt, args...)  LOG_WARNING("cpld: ", fmt, ##args)
#define CPLD_INFO(fmt, args...)     LOG_INFO("cpld: ", fmt, ##args)
#define CPLD_DEBUG(fmt, args...)    LOG_DBG("cpld: ", fmt, ##args)

#define CHECK_BIT(a, b)  (((a) >> (b)) & 1U)

#define CPLD_TOTAL_NUM                          4
#define CPLD_BOARD_VER_OFFSET                   0x00
#define CPLD_MAJOR_VER_OFFSET                   0x01
#define CPLD_MINOR_VER_OFFSET                   0x02
#define CPLD_YEAR_OFFSET                        0xEC
#define CPLD_MONTH_OFFSET                       0xED
#define CPLD_DAY_OFFSET                         0xEE
#define CPLD_HOUR_OFFSET                        0xEF

#define CPLD_TEST_REG_1_OFFSET                  0x05
#define CPLD_TEST_REG_2_OFFSET                  0x07

#define CPLD_REG_CPU_COOL_RST_RECORD_OFFSET     0x67
#define CPLD_RST_SRC_RECORD_REG_2_OFFSET        0x2A

#define SYS_CPLD_BUS_NR                         0
#define PORT_CPLD1_BUS_NR                       634
#define PORT_CPLD2_BUS_NR                       635
#define FAN_CPLD_BUS_NR                         143

#define BMC_PRESENT_BIT_OFFSET                  (1<<6)

/*clock hs alarm, clock alarm*/
#define SYSCPLD_REG_25M_CLK                     0x0F
#define SYSCPLD_REG_25M_CLK_MASK                0x0C
#define SYSCPLD_REG_100M_CLK                    0x14
#define SYSCPLD_REG_100M_CLK_MASK               0x0C
#define SYSCPLD_REG_156M_CLK                    0x18
#define SYSCPLD_REG_156M_CLK_MASK               0x0C

#define SYSCPLD_REG_SCRATCH_PAD                 0x05
/*PG status*/
#define SYSCPLD_REG_PG_STATUS0                  0x36
#define SYSCPLD_REG_PG_STATUS0_MASK             0xFF
#define SYSCPLD_REG_PG_STATUS1                  0x37
#define SYSCPLD_REG_PG_STATUS1_MASK             0xFF
#define SYSCPLD_REG_PG_STATUS2                  0x38
#define SYSCPLD_REG_PG_STATUS2_MASK             0xFF

/*syscpld reset status*/
#define SYSCPLD_REG_RESET_STATUS0               0x1C
#define SYSCPLD_REG_RESET_STATUS0_MASK          0xFF
#define SYSCPLD_REG_RESET_STATUS1               0x1D
#define SYSCPLD_REG_RESET_STATUS1_MASK          0xFF
#define SYSCPLD_REG_RESET_STATUS2               0x1E
#define SYSCPLD_REG_RESET_STATUS2_MASK          0xFF
#define SYSCPLD_REG_RESET_STATUS3               0x1F
#define SYSCPLD_REG_RESET_STATUS3_MASK          0xFF

/*portcpld reset status*/
#define PORTCPLD_REG_RESET_STATUS0               0x38
#define PORTCPLD_REG_RESET_STATUS0_MASK          0xFF
#define PORTCPLD_REG_RESET_STATUS1               0x39
#define PORTCPLD_REG_RESET_STATUS1_MASK          0xFF


#define SYSCPLD_REG_PRSNT_STATUS                0x11
#define SYSCPLD_REG_CORROSION_STATUS            0x12
#define SYSCPLD_REG_BMC_HEART_MASK              0x10
#define SYSCPLD_REG_CORROSION_MASK              0x07
#define SYSCPLD_REG_CORROSION_IOL_0_OFFSET      0
#define SYSCPLD_REG_CORROSION_IOL_1_OFFSET      1
#define SYSCPLD_REG_CORROSION_IOL_2_OFFSET      2

#define SYSCPLD_REG_DYING_RECORD_CTL            0xD0
#define SYSCPLD_REG_DYING_RECORD_CTL_EN         (1<<0)
#define SYSCPLD_REG_DYING_RECORD_CTL_CLR        (1<<1)
#define SYSCPLD_REG_DYING_RECORD_WDT_CNT        0xD1
#define SYSCPLD_REG_DYING_RECORD_DELAY_CNT      0xD2
#define SYSCPLD_REG_DYING_RECORD_RESET_CNT      0xD3

#define FANCPLD_REG_SCRATCH_PAD                 0x05

#define PORTCPLD_REG_SCRATCH_PAD                0x05

/*BMC status*/
#define SYSCPLD_REG_BMC_STATUS                  0x29


enum cpld_type {
    sys_cpld,
    fan_cpld,
    port_cpld1,
    port_cpld2,
};

enum cpld_index {
    SYS_CPLD,   /* SYS CPLD */
    FAN_CPLD,  /* FANU CPLD */
    PORT_CPLD1, /* IOU CPLD */
    PORT_CPLD2, /* Switch CPLD1 */
    NUM_CPLD_INDEX,
};

struct cpld_drvdata {
    struct class *cpld_class;
    struct device *cpld_udev;
    struct cdev cpld_cdev;
    dev_t devt;
    struct mutex sem;
};

struct cpld_drivers_t {
    ssize_t (*get_reboot_cause) (char *buf);
    ssize_t (*get_alias) (unsigned int index, char *buf);
    ssize_t (*get_type) (unsigned int index, char *buf);
    ssize_t (*get_hw_version) (unsigned int index, char *buf);
    ssize_t (*get_board_version) (unsigned int index, char *buf);
    ssize_t (*get_fmea_selftest_status) (unsigned int index, char *buf);
    ssize_t (*set_reg_test) (unsigned int index, unsigned char data);
    ssize_t (*get_fmea_corrosion_status) (unsigned int index, char *buf);
    ssize_t (*get_fmea_voltage_status) (unsigned int index, char *buf);
    ssize_t (*get_fmea_clock_status) (unsigned int index, char *buf);
    ssize_t (*get_fmea_battery_status) (unsigned int index, char *buf);
    ssize_t (*get_fmea_reset) (unsigned int index, char *buf);
    bool (*set_fmea_reset) (unsigned int index, int reset);
    ssize_t (*get_fmea_interrupt) (unsigned int index, char *buf);
    void (*get_loglevel) (long *lev);
    void (*set_loglevel) (long lev);
    ssize_t (*debug_help) (char *buf);
    ssize_t (*debug) (const char *buf, int count);
};

ssize_t drv_get_reboot_cause(char *buf);
ssize_t drv_get_alias(unsigned int index, char *buf);
ssize_t drv_get_type(unsigned int index, char *buf);
ssize_t drv_get_hw_version(unsigned int index, char *buf);
ssize_t drv_get_board_version(unsigned int index, char *buf);
ssize_t drv_get_reg_test(unsigned int index, char *buf);
ssize_t drv_get_fmea_corrosion_status(unsigned int index, char *buf);
ssize_t drv_get_fmea_voltage_status(unsigned int index, char *buf);
ssize_t drv_get_fmea_clock_status(unsigned int index, char *buf);
ssize_t drv_get_fmea_battery_status(unsigned int index, char *buf);
ssize_t drv_get_fmea_reset(unsigned int index, char *buf);
bool drv_set_fmea_reset(unsigned int index, int reset);
ssize_t drv_get_fmea_interrupt(unsigned int index, char *buf);
void drv_get_loglevel(long *lev);
void drv_set_loglevel(long lev);
ssize_t drv_debug_help(char *buf);
ssize_t drv_debug_help_hisonic(char *buf);
ssize_t drv_debug(const char *buf, int count);

int switch_cpld_reg_read(u8 cpld_index, u8 reg_offset, u8 *reg_value);
int switch_cpld_reg_write(u8 cpld_index, u8 reg_offset, u8 reg_value);

int switch_system_cpld_read(u8 reg, u8 *value);
int switch_system_cpld_write(u8 reg, u8 value);

int switch_i2c_cpld_read(u8 index, u8 reg, u8 *value);
int switch_i2c_cpld_write(u8 index, u8 reg, u8 value);

void s3ip_cpld_drivers_register(struct cpld_drivers_t *p_func);
void s3ip_cpld_drivers_unregister(void);

#endif /* SWITCH_CPLD_DRIVER_H */
