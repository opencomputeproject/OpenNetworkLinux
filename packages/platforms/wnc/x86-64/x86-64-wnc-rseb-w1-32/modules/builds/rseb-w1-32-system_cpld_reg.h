#ifndef __SYSTEM_CPLD_REG
#define __SYSTEM_CPLD_REG

static int system_cpld_raw_read(struct device *dev, struct device_attribute *attr, char *buf,
    int reg_offset, int reg_width, int fld_shift, int fld_width, int fld_mask, char *reg_name);
static int system_cpld_raw_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count,
    int reg_offset, int reg_width, int fld_shift, int fld_width, int fld_mask, char *reg_name);

/* Generic CPLD read function */
#define FLD_RAW_RD_FUNC(_reg, _fld, _wdh) static ssize_t \
system_cpld_##_fld##_raw_read(struct device *dev, struct device_attribute *attr, char *buf) { \
    return system_cpld_raw_read(dev, attr, buf, _reg##_offset, _reg##_width, _fld##_shift, _fld##_width, _fld##_mask, #_reg); \
}

/* Generic CPLD write function */
#define FLD_RAW_WR_FUNC(_reg, _fld, _wdh) static ssize_t \
system_cpld_##_fld##_raw_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { \
    return system_cpld_raw_write(dev, attr, buf, count, _reg##_offset, _reg##_width, _fld##_shift, _fld##_width, _fld##_mask, #_reg); \
}

/* CPLD register definition macros */
#define REG_DEF(_reg, _off, _wdh) \
static unsigned int _reg##_offset = (unsigned int)(_off); \
static unsigned int _reg##_width = (unsigned int)(_wdh);

/* CPLD register field definition macros, with generic read/write function */
#define FLD_RAW_RO_DEF(_reg, _fld, _sft, _wdh) \
static unsigned int _fld##_shift = (unsigned int)(_sft); \
static unsigned int _fld##_width = (unsigned int)(_wdh); \
static unsigned int _fld##_mask = ((((unsigned int)1) << (_wdh)) - 1); \
FLD_RAW_RD_FUNC(_reg, _fld, _wdh)

#define FLD_RAW_RW_DEF(_reg, _fld, _sft, _wdh) \
static unsigned int _fld##_shift = (unsigned int)(_sft); \
static unsigned int _fld##_width = (unsigned int)(_wdh); \
static unsigned int _fld##_mask = ((((unsigned int)1) << (_wdh)) - 1); \
FLD_RAW_RD_FUNC(_reg, _fld, _wdh) FLD_RAW_WR_FUNC(_reg, _fld, _wdh)

/* Declare system CPLD registers */
/*           register name                           offset  width */
/*           --------------------------------------- ------- ----- */
REG_DEF(     pcb_rev_reg,                            0x00,   8)
REG_DEF(     rst_ctrl_1,                             0x03,   8)
REG_DEF(     rst_ctrl_2,                             0x04,   8)
REG_DEF(     rst_ctrl_3,                             0x05,   8)
REG_DEF(     int_1_reg,                              0x06,   8)
REG_DEF(     int_2_reg,                              0x07,   8)
REG_DEF(     int_3_reg,                              0x08,   8)
REG_DEF(     imask_1,                                0x0A,   8)
REG_DEF(     imask_2,                                0x0B,   8)
REG_DEF(     imask_3,                                0x0C,   8)
REG_DEF(     fan_pwm_1_reg,                          0x13,   8)
REG_DEF(     fan_pwm_2_reg,                          0x12,   8)
REG_DEF(     fan_pwm_3_reg,                          0x11,   8)
REG_DEF(     fan_pwm_4_reg,                          0x10,   8)
REG_DEF(     fan_pwm_5_reg,                          0x0F,   8)
REG_DEF(     fan_pwm_6_reg,                          0x0E,   8)
REG_DEF(     psu_ctrl_1,                             0x2C,   8)
REG_DEF(     psu_ctrl_2,                             0x2D,   8)
REG_DEF(     status_led_reg,                         0x2E,   8)
REG_DEF(     fan_led,                                0x2F,   8)
REG_DEF(     pwr_good,                               0x30,   8)
REG_DEF(     fan_tach_rev_reg,                       0x32,   8)
REG_DEF(     wd_ctrl,                                0x33,   8)
REG_DEF(     wd_ctrl_period,                         0x34,   8)
REG_DEF(     fan_present,                            0x35,   8)
REG_DEF(     fan_direction,                          0x36,   8)

/* Declare system CPLD register's fields */
/*                      register name               field name                    shift  width */
/*                      ----------------------      ----------------              ------ ----- */
FLD_RAW_RO_DEF(         pcb_rev_reg,                pcb_rev,                      0,      4)

FLD_RAW_RW_DEF(         rst_ctrl_1,                 sys_rst,                      0,      1)
FLD_RAW_RW_DEF(         rst_ctrl_1,                 pcie_perst,                   1,      1)
FLD_RAW_RW_DEF(         rst_ctrl_1,                 qspi_rst,                     2,      1)
FLD_RAW_RW_DEF(         rst_ctrl_1,                 tps53681_rst,                 3,      1)
FLD_RAW_RW_DEF(         rst_ctrl_1,                 pcie_lan_rst,                 4,      1)
FLD_RAW_RW_DEF(         rst_ctrl_1,                 ptp_rst,                      5,      1)
FLD_RAW_RW_DEF(         rst_ctrl_1,                 ja_rst,                       6,      1)

FLD_RAW_RW_DEF(         rst_ctrl_2,                 pca9548_rst,                  0,      1)
FLD_RAW_RW_DEF(         rst_ctrl_2,                 zqsfp_rst,                    1,      1)
FLD_RAW_RW_DEF(         rst_ctrl_2,                 shift_rst,                    2,      1)
FLD_RAW_RW_DEF(         rst_ctrl_2,                 i2c0_rst,                     3,      1)
FLD_RAW_RW_DEF(         rst_ctrl_2,                 i2c1_rst,                     4,      1)
FLD_RAW_RW_DEF(         rst_ctrl_2,                 cpu_pltrst,                   5,      1)
FLD_RAW_RW_DEF(         rst_ctrl_2,                 cpu_cpld_rst,                 6,      1)
FLD_RAW_RW_DEF(         rst_ctrl_2,                 bcm56870_sys_rst,             7,      1)

FLD_RAW_RW_DEF(         rst_ctrl_3,                 i2c1_fan_rst,                 0,      1)
FLD_RAW_RW_DEF(         rst_ctrl_3,                 bmc_rst,                      2,      1)
FLD_RAW_RW_DEF(         rst_ctrl_3,                 fm_cpld_ngff_rst1,            3,      1)
FLD_RAW_RW_DEF(         rst_ctrl_3,                 fm_cpld_ngff_rst2,            4,      1)
FLD_RAW_RW_DEF(         rst_ctrl_3,                 pca9541a_1_rst,               5,      1)
FLD_RAW_RW_DEF(         rst_ctrl_3,                 pca9541a_2_rst,               6,      1)
FLD_RAW_RW_DEF(         rst_ctrl_3,                 mcp2210_rst,                  7,      1)

FLD_RAW_RO_DEF(         int_1_reg,                  int_1,                        0,      8)

FLD_RAW_RO_DEF(         int_2_reg,                  int_2,                        0,      8)

FLD_RAW_RO_DEF(         int_3_reg,                  int_3,                        0,      8)

FLD_RAW_RW_DEF(         imask_1,                    qsfp01_06_imask,              0,      1)
FLD_RAW_RW_DEF(         imask_1,                    qsfp07_12_imask,              1,      1)
FLD_RAW_RW_DEF(         imask_1,                    qsfp13_18_imask,              2,      1)
FLD_RAW_RW_DEF(         imask_1,                    qsfp19_24_imask,              3,      1)
FLD_RAW_RW_DEF(         imask_1,                    qsfp25_30_imask,              4,      1)
FLD_RAW_RW_DEF(         imask_1,                    qsfp31_32_imask,              5,      1)
FLD_RAW_RW_DEF(         imask_1,                    ja_imask,                     6,      1)

FLD_RAW_RW_DEF(         imask_2,                    temp1_alert_imask,            0,      1)
FLD_RAW_RW_DEF(         imask_2,                    temp2_alert_imask,            1,      1)
FLD_RAW_RW_DEF(         imask_2,                    temp3_alert_imask,            2,      1)
FLD_RAW_RW_DEF(         imask_2,                    lan_alert_imask,              3,      1)
FLD_RAW_RW_DEF(         imask_2,                    ucd9090_i2c_alert_imask,      4,      1)
FLD_RAW_RW_DEF(         imask_2,                    pcie_imask,                   5,      1)
FLD_RAW_RW_DEF(         imask_2,                    temp_fan_imask,               6,      1)
FLD_RAW_RW_DEF(         imask_2,                    i2c_peci_alert_imask,         7,      1)

FLD_RAW_RW_DEF(         imask_3,                    bmc_pwrbtn_imask,             0,      1)
FLD_RAW_RW_DEF(         imask_3,                    usb_fault_imask,              1,      1)
FLD_RAW_RW_DEF(         imask_3,                    mb_imask,                     2,      1)
FLD_RAW_RW_DEF(         imask_3,                    tps53681_smb_alert_imask,     3,      1)
FLD_RAW_RW_DEF(         imask_3,                    tps53681_vr_fault_imask,      4,      1)
FLD_RAW_RW_DEF(         imask_3,                    tps53681_vr_hot_imask,        5,      1)
FLD_RAW_RW_DEF(         imask_3,                    pwrbtn_imask,                 6,      1)
FLD_RAW_RW_DEF(         imask_3,                    lpc_serirq_imask,             7,      1)

FLD_RAW_RW_DEF(         fan_pwm_1_reg,              fan_pwm_1,                    0,      8)

FLD_RAW_RW_DEF(         fan_pwm_2_reg,              fan_pwm_2,                    0,      8)

FLD_RAW_RW_DEF(         fan_pwm_3_reg,              fan_pwm_3,                    0,      8)

FLD_RAW_RW_DEF(         fan_pwm_4_reg,              fan_pwm_4,                    0,      8)

FLD_RAW_RW_DEF(         fan_pwm_5_reg,              fan_pwm_5,                    0,      8)

FLD_RAW_RW_DEF(         fan_pwm_6_reg,              fan_pwm_6,                    0,      8)

FLD_RAW_RW_DEF(         psu_ctrl_1,                 psu_ps_on_lt_1,               0,      1)
FLD_RAW_RW_DEF(         psu_ctrl_1,                 psu_ps_ok_lt_1,               1,      1)
FLD_RAW_RW_DEF(         psu_ctrl_1,                 psu_present_lt_1,             2,      1)
FLD_RAW_RW_DEF(         psu_ctrl_1,                 psu_smb_alert_lt_1,           6,      1)

FLD_RAW_RW_DEF(         psu_ctrl_2,                 psu_ps_on_lt_2,               0,      1)
FLD_RAW_RW_DEF(         psu_ctrl_2,                 psu_ps_ok_lt_2,               1,      1)
FLD_RAW_RW_DEF(         psu_ctrl_2,                 psu_present_lt_2,             2,      1)
FLD_RAW_RW_DEF(         psu_ctrl_2,                 psu_smb_alert_lt_2,           6,      1)

FLD_RAW_RW_DEF(         status_led_reg,             id_led,                       0,      2)
FLD_RAW_RW_DEF(         status_led_reg,             status_led,                   2,      2)

FLD_RAW_RW_DEF(         fan_led,                    fan_led_1,                    5,      1)
FLD_RAW_RW_DEF(         fan_led,                    fan_led_2,                    4,      1)
FLD_RAW_RW_DEF(         fan_led,                    fan_led_3,                    3,      1)
FLD_RAW_RW_DEF(         fan_led,                    fan_led_4,                    2,      1)
FLD_RAW_RW_DEF(         fan_led,                    fan_led_5,                    1,      1)
FLD_RAW_RW_DEF(         fan_led,                    fan_led_6,                    0,      1)

FLD_RAW_RO_DEF(         pwr_good,                   vdd_core_pg,                  0,      1)
FLD_RAW_RO_DEF(         pwr_good,                   vdd_0v8_pg,                   1,      1)
FLD_RAW_RO_DEF(         pwr_good,                   vdd_3v3_pg,                   2,      1)
FLD_RAW_RO_DEF(         pwr_good,                   vdd_1v2_pg,                   3,      1)
FLD_RAW_RO_DEF(         pwr_good,                   vdd_3v3sb_pg,                 4,      1)
FLD_RAW_RO_DEF(         pwr_good,                   pwr_ok,                       5,      1)
FLD_RAW_RO_DEF(         pwr_good,                   vdd_3v3clk_pg,                6,      1)

FLD_RAW_RW_DEF(         fan_tach_rev_reg,           fan_tach_rev,                 0,      2)

FLD_RAW_RW_DEF(         wd_ctrl,                    wd_rst_en,                    0,      1)
FLD_RAW_RW_DEF(         wd_ctrl,                    wd_rst_clear,                 1,      1)

FLD_RAW_RW_DEF(         wd_ctrl_period,             wdt_period,                   0,      8)

FLD_RAW_RO_DEF(         fan_present,                fan_present_6,                0,      1)
FLD_RAW_RO_DEF(         fan_present,                fan_present_5,                1,      1)
FLD_RAW_RO_DEF(         fan_present,                fan_present_4,                2,      1)
FLD_RAW_RO_DEF(         fan_present,                fan_present_3,                3,      1)
FLD_RAW_RO_DEF(         fan_present,                fan_present_2,                4,      1)
FLD_RAW_RO_DEF(         fan_present,                fan_present_1,                5,      1)

FLD_RAW_RO_DEF(         fan_direction,              fan_direction_6,              0,      1)
FLD_RAW_RO_DEF(         fan_direction,              fan_direction_5,              1,      1)
FLD_RAW_RO_DEF(         fan_direction,              fan_direction_4,              2,      1)
FLD_RAW_RO_DEF(         fan_direction,              fan_direction_3,              3,      1)
FLD_RAW_RO_DEF(         fan_direction,              fan_direction_2,              4,      1)
FLD_RAW_RO_DEF(         fan_direction,              fan_direction_1,              5,      1)

/* ---------------------- Definitions of CPLD specific fields --------------------------- */
static int system_cpld_ver_raw_read(struct device *dev, struct device_attribute *attr, char *buf,
    int reg_offset, char *fld_name);
static int system_cpld_fan_raw_read(struct device *dev, struct device_attribute *attr, char *buf,
    int reg_offset, char *fld_name);

/* CPLD specific read functions */
#define FLD_VER_RAW_RO_DEF(_reg, _fld) static ssize_t \
system_cpld_##_fld##_raw_read(struct device *dev, struct device_attribute *attr, char *buf) { \
    return system_cpld_ver_raw_read(dev, attr, buf, _reg##_offset, #_fld); \
}
#define FLD_FAN_RAW_RO_DEF(_reg, _fld) static ssize_t \
system_cpld_##_fld##_raw_read(struct device *dev, struct device_attribute *attr, char *buf) { \
    return system_cpld_fan_raw_read(dev, attr, buf, _reg##_offset, #_fld); \
}

/* CPLD register definition macros */
#define REG_DEF_SPEC(_reg, _off) \
static unsigned int _reg##_offset = (unsigned int)(_off);

/* Declare CPLD specific registers */
/*                register name                           offset  */
/*                --------------------------------------- ------- */
REG_DEF_SPEC(     cpld_ver_1,                             0x01)
REG_DEF_SPEC(     fan_tach_1_1a,                          0x28)
REG_DEF_SPEC(     fan_tach_1_2a,                          0x2A)
REG_DEF_SPEC(     fan_tach_2_1a,                          0x24)
REG_DEF_SPEC(     fan_tach_2_2a,                          0x26)
REG_DEF_SPEC(     fan_tach_3_1a,                          0x20)
REG_DEF_SPEC(     fan_tach_3_2a,                          0x22)
REG_DEF_SPEC(     fan_tach_4_1a,                          0x1C)
REG_DEF_SPEC(     fan_tach_4_2a,                          0x1E)
REG_DEF_SPEC(     fan_tach_5_1a,                          0x18)
REG_DEF_SPEC(     fan_tach_5_2a,                          0x1A)
REG_DEF_SPEC(     fan_tach_6_1a,                          0x14)
REG_DEF_SPEC(     fan_tach_6_2a,                          0x16)

/* Declare CPLD specific register's fields */
/*                          register name               field name       */
/*                          ----------------------      ---------------- */
FLD_VER_RAW_RO_DEF(         cpld_ver_1,                 cpld_ver)
FLD_FAN_RAW_RO_DEF(         fan_tach_1_1a,              fan_tach_1_1)
FLD_FAN_RAW_RO_DEF(         fan_tach_1_2a,              fan_tach_1_2)
FLD_FAN_RAW_RO_DEF(         fan_tach_2_1a,              fan_tach_2_1)
FLD_FAN_RAW_RO_DEF(         fan_tach_2_2a,              fan_tach_2_2)
FLD_FAN_RAW_RO_DEF(         fan_tach_3_1a,              fan_tach_3_1)
FLD_FAN_RAW_RO_DEF(         fan_tach_3_2a,              fan_tach_3_2)
FLD_FAN_RAW_RO_DEF(         fan_tach_4_1a,              fan_tach_4_1)
FLD_FAN_RAW_RO_DEF(         fan_tach_4_2a,              fan_tach_4_2)
FLD_FAN_RAW_RO_DEF(         fan_tach_5_1a,              fan_tach_5_1)
FLD_FAN_RAW_RO_DEF(         fan_tach_5_2a,              fan_tach_5_2)
FLD_FAN_RAW_RO_DEF(         fan_tach_6_1a,              fan_tach_6_1)
FLD_FAN_RAW_RO_DEF(         fan_tach_6_2a,              fan_tach_6_2)

#endif /* __SYSTEM_CPLD_REG */
