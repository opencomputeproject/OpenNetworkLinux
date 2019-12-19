#ifndef __CPU_CPLD_SYSFS
#define __CPU_CPLD_SYSFS

/* Generic CPLD sysfs file definition macros */
#define SYSFS_RAW_RO_ATTR_DEF(field)  \
struct device_attribute field  \
    = __ATTR(field, S_IRUGO, cpu_cpld_##field##_raw_read, NULL);

#define SYSFS_RAW_RW_ATTR_DEF(field)  \
struct device_attribute field  \
    = __ATTR(field, S_IRUGO | S_IWUSR, cpu_cpld_##field##_raw_read, cpu_cpld_##field##_raw_write);

#define SYSFS_MISC_RO_ATTR_DEF(field, _read)  \
struct device_attribute field  \
    = __ATTR(field, S_IRUGO, _read, NULL);

#define SYSFS_MISC_RW_ATTR_DEF(field, _read, _write)  \
struct device_attribute field  \
    = __ATTR(field, S_IRUGO | S_IWUSR, _read, _write);

#define SYSFS_ATTR_PTR(field)  \
&field.attr


/* Declare CPU CPLD file system */
SYSFS_RAW_RO_ATTR_DEF(p1v8_stby_pg)
SYSFS_RAW_RO_ATTR_DEF(pvnn_pg)
SYSFS_RAW_RO_ATTR_DEF(p1v05_pg)
SYSFS_RAW_RO_ATTR_DEF(p2v5_vpp_pg)
SYSFS_RAW_RO_ATTR_DEF(p1v2_vddq_pg)
SYSFS_RAW_RO_ATTR_DEF(pvccref_pg)
SYSFS_RAW_RO_ATTR_DEF(pvccsram_pg)
SYSFS_RAW_RO_ATTR_DEF(cpld_p3v3_pg)

SYSFS_RAW_RO_ATTR_DEF(pvccp_pg)
SYSFS_RAW_RO_ATTR_DEF(pvtt_pg)
SYSFS_RAW_RO_ATTR_DEF(soc_slp3)
SYSFS_RAW_RO_ATTR_DEF(soc_cpld_slp_s45)
SYSFS_RAW_RO_ATTR_DEF(soc_pltrst)
SYSFS_RAW_RO_ATTR_DEF(rst_cpld_rsmrst)

SYSFS_RAW_RO_ATTR_DEF(cpld_p1v8_stby_en)
SYSFS_RAW_RO_ATTR_DEF(cpld_pvnn_en)
SYSFS_RAW_RO_ATTR_DEF(cpld_p1v05_en)
SYSFS_RAW_RO_ATTR_DEF(rst_rsmrst)
SYSFS_RAW_RO_ATTR_DEF(cpld_p2v5_vpp_en)
SYSFS_RAW_RO_ATTR_DEF(cpld_p1v2_vddq_en)
SYSFS_RAW_RO_ATTR_DEF(cpld_p0v6_vtt_dimm_en)
SYSFS_RAW_RO_ATTR_DEF(cpld_pvccref_en)

SYSFS_RAW_RO_ATTR_DEF(cpld_pvccsram_en)
SYSFS_RAW_RO_ATTR_DEF(cpld_p3v3_en)
SYSFS_RAW_RO_ATTR_DEF(cpld_pvccp_en)
SYSFS_RAW_RO_ATTR_DEF(cpld_corepwrok)

SYSFS_RAW_RO_ATTR_DEF(p1v8_stby_err)
SYSFS_RAW_RO_ATTR_DEF(pvnn_err)
SYSFS_RAW_RO_ATTR_DEF(p1v05_err)
SYSFS_RAW_RO_ATTR_DEF(p2v5_vpp_err)
SYSFS_RAW_RO_ATTR_DEF(p1v2_vddq_err)
SYSFS_RAW_RO_ATTR_DEF(pvtt_err)
SYSFS_RAW_RO_ATTR_DEF(pvccref_err)
SYSFS_RAW_RO_ATTR_DEF(pvccsram_err)

SYSFS_RAW_RO_ATTR_DEF(p3v3_err)
SYSFS_RAW_RO_ATTR_DEF(pvccp_err)
SYSFS_RAW_RO_ATTR_DEF(memevent)
SYSFS_RAW_RO_ATTR_DEF(dimm_event_co)

SYSFS_RAW_RO_ATTR_DEF(soc_cpld_mcerr)
SYSFS_RAW_RO_ATTR_DEF(cpld_soc_thermtrip)
SYSFS_RAW_RO_ATTR_DEF(soc_caterr)
SYSFS_RAW_RO_ATTR_DEF(soc_err0)
SYSFS_RAW_RO_ATTR_DEF(soc_err1)
SYSFS_RAW_RO_ATTR_DEF(soc_err2)

SYSFS_RAW_RO_ATTR_DEF(cpld_vrhot)
SYSFS_RAW_RO_ATTR_DEF(pvnn_vrhot)
SYSFS_RAW_RO_ATTR_DEF(pvccsram_vrhot)
SYSFS_RAW_RO_ATTR_DEF(soc_throttle)
SYSFS_RAW_RO_ATTR_DEF(soc_me_drive)
SYSFS_RAW_RO_ATTR_DEF(soc_prochot_disable)
SYSFS_RAW_RO_ATTR_DEF(edge_fast_prochot)

SYSFS_RAW_RO_ATTR_DEF(intr)

SYSFS_RAW_RO_ATTR_DEF(pwr_st)

SYSFS_RAW_RO_ATTR_DEF(bmc_cpu_prsnt)
SYSFS_RAW_RO_ATTR_DEF(bios_mux_sel)
SYSFS_RAW_RO_ATTR_DEF(cpld_pwrbtn_bmc)
SYSFS_RAW_RO_ATTR_DEF(rst_sysrst)

SYSFS_RAW_RO_ATTR_DEF(cpu_cpld_rev)

SYSFS_RAW_RO_ATTR_DEF(cpu_board_rev)

SYSFS_RAW_RW_ATTR_DEF(bios_update_en)

#endif /* __CPU_CPLD_SYSFS */
