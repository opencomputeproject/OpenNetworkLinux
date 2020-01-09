#ifndef __CPU_CPLD_REG
#define __CPU_CPLD_REG

static int cpu_cpld_raw_read(struct device *dev, struct device_attribute *attr, char *buf,
    int reg_offset, int reg_width, int fld_shift, int fld_width, int fld_mask, char *reg_name);
static int cpu_cpld_raw_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count,
    int reg_offset, int reg_width, int fld_shift, int fld_width, int fld_mask, char *reg_name);

/* Generic CPLD read function */
#define FLD_RAW_RD_FUNC(_reg, _fld, _wdh) static ssize_t \
cpu_cpld_##_fld##_raw_read(struct device *dev, struct device_attribute *attr, char *buf) { \
    return cpu_cpld_raw_read(dev, attr, buf, _reg##_offset, _reg##_width, _fld##_shift, _fld##_width, _fld##_mask, #_reg); \
}

/* Generic CPLD write function */
#define FLD_RAW_WR_FUNC(_reg, _fld, _wdh) static ssize_t \
cpu_cpld_##_fld##_raw_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { \
    return cpu_cpld_raw_write(dev, attr, buf, count, _reg##_offset, _reg##_width, _fld##_shift, _fld##_width, _fld##_mask, #_reg); \
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

/* Declare CPU CPLD registers */
/*          register name                           offset  width */
/*          --------------------------------------- ------- ----- */
REG_DEF(    pwr_gd_1,                               0x00,   8)
REG_DEF(    pwr_gd_2,                               0x01,   8)
REG_DEF(    pwr_en_1,                               0x02,   8)
REG_DEF(    pwr_en_2,                               0x03,   8)
REG_DEF(    pwr_err_1,                              0x04,   8)
REG_DEF(    pwr_err_2,                              0x05,   8)
REG_DEF(    cpu_err,                                0x06,   8)
REG_DEF(    vrhot,                                  0x07,   8)
REG_DEF(    intr_reg,                               0x08,   8)
REG_DEF(    pwr_st_reg,                             0x09,   8)
REG_DEF(    misc_mb_bmc,                            0x0A,   8)
REG_DEF(    cpu_cpld_rev_reg,                       0x0B,   8)
REG_DEF(    cpu_board_rev_reg,                      0x0C,   8)
REG_DEF(    ctrl,                                   0x10,   8)

/* Declare CPU CPLD register's fields */
/*                 register name               field name                    shift  width */
/*                 ----------------------      ----------------              ------ ----- */
FLD_RAW_RO_DEF(    pwr_gd_1,                   p1v8_stby_pg,                 0,      1)
FLD_RAW_RO_DEF(    pwr_gd_1,                   pvnn_pg,                      1,      1)
FLD_RAW_RO_DEF(    pwr_gd_1,                   p1v05_pg,                     2,      1)
FLD_RAW_RO_DEF(    pwr_gd_1,                   p2v5_vpp_pg,                  3,      1)
FLD_RAW_RO_DEF(    pwr_gd_1,                   p1v2_vddq_pg,                 4,      1)
FLD_RAW_RO_DEF(    pwr_gd_1,                   pvccref_pg,                   5,      1)
FLD_RAW_RO_DEF(    pwr_gd_1,                   pvccsram_pg,                  6,      1)
FLD_RAW_RO_DEF(    pwr_gd_1,                   cpld_p3v3_pg,                 7,      1)

FLD_RAW_RO_DEF(    pwr_gd_2,                   pvccp_pg,                     0,      1)
FLD_RAW_RO_DEF(    pwr_gd_2,                   pvtt_pg,                      1,      1)
FLD_RAW_RO_DEF(    pwr_gd_2,                   soc_slp3,                     2,      1)
FLD_RAW_RO_DEF(    pwr_gd_2,                   soc_cpld_slp_s45,             3,      1)
FLD_RAW_RO_DEF(    pwr_gd_2,                   soc_pltrst,                   4,      1)
FLD_RAW_RO_DEF(    pwr_gd_2,                   rst_cpld_rsmrst,              5,      1)

FLD_RAW_RO_DEF(    pwr_en_1,                   cpld_p1v8_stby_en,            0,      1)
FLD_RAW_RO_DEF(    pwr_en_1,                   cpld_pvnn_en,                 1,      1)
FLD_RAW_RO_DEF(    pwr_en_1,                   cpld_p1v05_en,                2,      1)
FLD_RAW_RO_DEF(    pwr_en_1,                   rst_rsmrst,                   3,      1)
FLD_RAW_RO_DEF(    pwr_en_1,                   cpld_p2v5_vpp_en,             4,      1)
FLD_RAW_RO_DEF(    pwr_en_1,                   cpld_p1v2_vddq_en,            5,      1)
FLD_RAW_RO_DEF(    pwr_en_1,                   cpld_p0v6_vtt_dimm_en,        6,      1)
FLD_RAW_RO_DEF(    pwr_en_1,                   cpld_pvccref_en,              7,      1)

FLD_RAW_RO_DEF(    pwr_en_2,                   cpld_pvccsram_en,             0,      1)
FLD_RAW_RO_DEF(    pwr_en_2,                   cpld_p3v3_en,                 1,      1)
FLD_RAW_RO_DEF(    pwr_en_2,                   cpld_pvccp_en,                2,      1)
FLD_RAW_RO_DEF(    pwr_en_2,                   cpld_corepwrok,               3,      1)

FLD_RAW_RO_DEF(    pwr_err_1,                  p1v8_stby_err,                0,      1)
FLD_RAW_RO_DEF(    pwr_err_1,                  pvnn_err,                     1,      1)
FLD_RAW_RO_DEF(    pwr_err_1,                  p1v05_err,                    2,      1)
FLD_RAW_RO_DEF(    pwr_err_1,                  p2v5_vpp_err,                 3,      1)
FLD_RAW_RO_DEF(    pwr_err_1,                  p1v2_vddq_err,                4,      1)
FLD_RAW_RO_DEF(    pwr_err_1,                  pvtt_err,                     5,      1)
FLD_RAW_RO_DEF(    pwr_err_1,                  pvccref_err,                  6,      1)
FLD_RAW_RO_DEF(    pwr_err_1,                  pvccsram_err,                 7,      1)

FLD_RAW_RO_DEF(    pwr_err_2,                  p3v3_err,                     0,      1)
FLD_RAW_RO_DEF(    pwr_err_2,                  pvccp_err,                    1,      1)
FLD_RAW_RO_DEF(    pwr_err_2,                  memevent,                     2,      1)
FLD_RAW_RO_DEF(    pwr_err_2,                  dimm_event_co,                3,      1)

FLD_RAW_RO_DEF(    cpu_err,                    soc_cpld_mcerr,               0,      1)
FLD_RAW_RO_DEF(    cpu_err,                    cpld_soc_thermtrip,           1,      1)
FLD_RAW_RO_DEF(    cpu_err,                    soc_caterr,                   2,      1)
FLD_RAW_RO_DEF(    cpu_err,                    soc_err0,                     3,      1)
FLD_RAW_RO_DEF(    cpu_err,                    soc_err1,                     4,      1)
FLD_RAW_RO_DEF(    cpu_err,                    soc_err2,                     5,      1)

FLD_RAW_RO_DEF(    vrhot,                      cpld_vrhot,                   0,      1)
FLD_RAW_RO_DEF(    vrhot,                      pvnn_vrhot,                   1,      1)
FLD_RAW_RO_DEF(    vrhot,                      pvccsram_vrhot,               2,      1)
FLD_RAW_RO_DEF(    vrhot,                      soc_throttle,                 3,      1)
FLD_RAW_RO_DEF(    vrhot,                      soc_me_drive,                 4,      1)
FLD_RAW_RO_DEF(    vrhot,                      soc_prochot_disable,          5,      1)
FLD_RAW_RO_DEF(    vrhot,                      edge_fast_prochot,            6,      1)

FLD_RAW_RO_DEF(    intr_reg,                   intr,                         0,      6)

FLD_RAW_RO_DEF(    pwr_st_reg,                 pwr_st,                       0,      6)

FLD_RAW_RO_DEF(    misc_mb_bmc,                bmc_cpu_prsnt,                0,      1)
FLD_RAW_RO_DEF(    misc_mb_bmc,                bios_mux_sel,                 1,      1)
FLD_RAW_RO_DEF(    misc_mb_bmc,                cpld_pwrbtn_bmc,              2,      1)
FLD_RAW_RO_DEF(    misc_mb_bmc,                rst_sysrst,                   3,      1)

FLD_RAW_RO_DEF(    cpu_cpld_rev_reg,           cpu_cpld_rev,                 0,      4)

FLD_RAW_RO_DEF(    cpu_board_rev_reg,          cpu_board_rev,                0,      4)

FLD_RAW_RW_DEF(    ctrl,                       bios_update_en,               0,      1)

#endif /* __CPU_CPLD_REG */
