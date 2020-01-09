#ifndef __SYSTEM_CPLD_SYSFS
#define __SYSTEM_CPLD_SYSFS

/* Generic CPLD sysfs file definition macros */
#define SYSFS_RAW_RO_ATTR_DEF(field)  \
struct device_attribute field  \
    = __ATTR(field, S_IRUGO, system_cpld_##field##_raw_read, NULL);

#define SYSFS_RAW_RW_ATTR_DEF(field)  \
struct device_attribute field  \
    = __ATTR(field, S_IRUGO | S_IWUSR, system_cpld_##field##_raw_read, system_cpld_##field##_raw_write);

#define SYSFS_MISC_RO_ATTR_DEF(field, _read)  \
struct device_attribute field  \
    = __ATTR(field, S_IRUGO, _read, NULL);

#define SYSFS_MISC_RW_ATTR_DEF(field, _read, _write)  \
struct device_attribute field  \
    = __ATTR(field, S_IRUGO | S_IWUSR, _read, _write);

#define SYSFS_ATTR_PTR(field)  \
&field.attr


/* Declare system CPLD file system */
SYSFS_RAW_RO_ATTR_DEF(pcb_rev)

SYSFS_RAW_RO_ATTR_DEF(cpld_ver)

SYSFS_RAW_RW_ATTR_DEF(sys_rst)
SYSFS_RAW_RW_ATTR_DEF(pcie_perst)
SYSFS_RAW_RW_ATTR_DEF(qspi_rst)
SYSFS_RAW_RW_ATTR_DEF(tps53681_rst)
SYSFS_RAW_RW_ATTR_DEF(pcie_lan_rst)
SYSFS_RAW_RW_ATTR_DEF(ptp_rst)
SYSFS_RAW_RW_ATTR_DEF(ja_rst)

SYSFS_RAW_RW_ATTR_DEF(pca9548_rst)
SYSFS_RAW_RW_ATTR_DEF(zqsfp_rst)
SYSFS_RAW_RW_ATTR_DEF(shift_rst)
SYSFS_RAW_RW_ATTR_DEF(i2c0_rst)
SYSFS_RAW_RW_ATTR_DEF(i2c1_rst)
SYSFS_RAW_RW_ATTR_DEF(cpu_pltrst)
SYSFS_RAW_RW_ATTR_DEF(cpu_cpld_rst)
SYSFS_RAW_RW_ATTR_DEF(bcm56870_sys_rst)

SYSFS_RAW_RW_ATTR_DEF(i2c1_fan_rst)
SYSFS_RAW_RW_ATTR_DEF(bmc_rst)
SYSFS_RAW_RW_ATTR_DEF(fm_cpld_ngff_rst1)
SYSFS_RAW_RW_ATTR_DEF(fm_cpld_ngff_rst2)
SYSFS_RAW_RW_ATTR_DEF(pca9541a_1_rst)
SYSFS_RAW_RW_ATTR_DEF(pca9541a_2_rst)
SYSFS_RAW_RW_ATTR_DEF(mcp2210_rst)

SYSFS_RAW_RO_ATTR_DEF(int_1)

SYSFS_RAW_RO_ATTR_DEF(int_2)

SYSFS_RAW_RO_ATTR_DEF(int_3)

SYSFS_RAW_RW_ATTR_DEF(qsfp01_06_imask)
SYSFS_RAW_RW_ATTR_DEF(qsfp07_12_imask)
SYSFS_RAW_RW_ATTR_DEF(qsfp13_18_imask)
SYSFS_RAW_RW_ATTR_DEF(qsfp19_24_imask)
SYSFS_RAW_RW_ATTR_DEF(qsfp25_30_imask)
SYSFS_RAW_RW_ATTR_DEF(qsfp31_32_imask)
SYSFS_RAW_RW_ATTR_DEF(ja_imask)

SYSFS_RAW_RW_ATTR_DEF(temp1_alert_imask)
SYSFS_RAW_RW_ATTR_DEF(temp2_alert_imask)
SYSFS_RAW_RW_ATTR_DEF(temp3_alert_imask)
SYSFS_RAW_RW_ATTR_DEF(lan_alert_imask)
SYSFS_RAW_RW_ATTR_DEF(ucd9090_i2c_alert_imask)
SYSFS_RAW_RW_ATTR_DEF(pcie_imask)
SYSFS_RAW_RW_ATTR_DEF(temp_fan_imask)
SYSFS_RAW_RW_ATTR_DEF(i2c_peci_alert_imask)

SYSFS_RAW_RW_ATTR_DEF(bmc_pwrbtn_imask)
SYSFS_RAW_RW_ATTR_DEF(usb_fault_imask)
SYSFS_RAW_RW_ATTR_DEF(mb_imask)
SYSFS_RAW_RW_ATTR_DEF(tps53681_smb_alert_imask)
SYSFS_RAW_RW_ATTR_DEF(tps53681_vr_fault_imask)
SYSFS_RAW_RW_ATTR_DEF(tps53681_vr_hot_imask)
SYSFS_RAW_RW_ATTR_DEF(pwrbtn_imask)
SYSFS_RAW_RW_ATTR_DEF(lpc_serirq_imask)

SYSFS_RAW_RW_ATTR_DEF(fan_pwm_1)

SYSFS_RAW_RW_ATTR_DEF(fan_pwm_2)

SYSFS_RAW_RW_ATTR_DEF(fan_pwm_3)

SYSFS_RAW_RW_ATTR_DEF(fan_pwm_4)

SYSFS_RAW_RW_ATTR_DEF(fan_pwm_5)

SYSFS_RAW_RW_ATTR_DEF(fan_pwm_6)

SYSFS_RAW_RO_ATTR_DEF(fan_tach_1_1)

SYSFS_RAW_RO_ATTR_DEF(fan_tach_1_2)

SYSFS_RAW_RO_ATTR_DEF(fan_tach_2_1)

SYSFS_RAW_RO_ATTR_DEF(fan_tach_2_2)

SYSFS_RAW_RO_ATTR_DEF(fan_tach_3_1)

SYSFS_RAW_RO_ATTR_DEF(fan_tach_3_2)

SYSFS_RAW_RO_ATTR_DEF(fan_tach_4_1)

SYSFS_RAW_RO_ATTR_DEF(fan_tach_4_2)

SYSFS_RAW_RO_ATTR_DEF(fan_tach_5_1)

SYSFS_RAW_RO_ATTR_DEF(fan_tach_5_2)

SYSFS_RAW_RO_ATTR_DEF(fan_tach_6_1)

SYSFS_RAW_RO_ATTR_DEF(fan_tach_6_2)

SYSFS_RAW_RW_ATTR_DEF(psu_ps_on_lt_1)
SYSFS_RAW_RW_ATTR_DEF(psu_ps_ok_lt_1)
SYSFS_RAW_RW_ATTR_DEF(psu_present_lt_1)
SYSFS_RAW_RW_ATTR_DEF(psu_smb_alert_lt_1)

SYSFS_RAW_RW_ATTR_DEF(psu_ps_on_lt_2)
SYSFS_RAW_RW_ATTR_DEF(psu_ps_ok_lt_2)
SYSFS_RAW_RW_ATTR_DEF(psu_present_lt_2)
SYSFS_RAW_RW_ATTR_DEF(psu_smb_alert_lt_2)

SYSFS_RAW_RW_ATTR_DEF(id_led)
SYSFS_RAW_RW_ATTR_DEF(status_led)

SYSFS_RAW_RW_ATTR_DEF(fan_led_1)
SYSFS_RAW_RW_ATTR_DEF(fan_led_2)
SYSFS_RAW_RW_ATTR_DEF(fan_led_3)
SYSFS_RAW_RW_ATTR_DEF(fan_led_4)
SYSFS_RAW_RW_ATTR_DEF(fan_led_5)
SYSFS_RAW_RW_ATTR_DEF(fan_led_6)

SYSFS_RAW_RO_ATTR_DEF(vdd_core_pg)
SYSFS_RAW_RO_ATTR_DEF(vdd_0v8_pg)
SYSFS_RAW_RO_ATTR_DEF(vdd_3v3_pg)
SYSFS_RAW_RO_ATTR_DEF(vdd_1v2_pg)
SYSFS_RAW_RO_ATTR_DEF(vdd_3v3sb_pg)
SYSFS_RAW_RO_ATTR_DEF(pwr_ok)
SYSFS_RAW_RO_ATTR_DEF(vdd_3v3clk_pg)

SYSFS_RAW_RW_ATTR_DEF(fan_tach_rev)

SYSFS_RAW_RW_ATTR_DEF(wd_rst_en)
SYSFS_RAW_RW_ATTR_DEF(wd_rst_clear)

SYSFS_RAW_RW_ATTR_DEF(wdt_period)

SYSFS_RAW_RO_ATTR_DEF(fan_present_6)
SYSFS_RAW_RO_ATTR_DEF(fan_present_5)
SYSFS_RAW_RO_ATTR_DEF(fan_present_4)
SYSFS_RAW_RO_ATTR_DEF(fan_present_3)
SYSFS_RAW_RO_ATTR_DEF(fan_present_2)
SYSFS_RAW_RO_ATTR_DEF(fan_present_1)

SYSFS_RAW_RO_ATTR_DEF(fan_direction_6)
SYSFS_RAW_RO_ATTR_DEF(fan_direction_5)
SYSFS_RAW_RO_ATTR_DEF(fan_direction_4)
SYSFS_RAW_RO_ATTR_DEF(fan_direction_3)
SYSFS_RAW_RO_ATTR_DEF(fan_direction_2)
SYSFS_RAW_RO_ATTR_DEF(fan_direction_1)

#endif /* __SYSTEM_CPLD_SYSFS */
