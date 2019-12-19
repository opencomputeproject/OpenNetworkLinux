#include <linux/module.h>
#include <linux/i2c.h>

#include "rseb-w1-32-system_cpld_reg.h"
#include "rseb-w1-32-system_cpld_sysfs.h"

static int debug_flag = 0;

struct system_cpld_data {
    struct mutex lock;
    struct i2c_client *client;
    struct device_attribute bin;
};
struct system_cpld_data *system_cpld;

static const struct i2c_device_id system_cpld_ids[] = {
    { "rseb_w1_32_sys_cpld", 0 },
    { /* END OF LIST */ }
};
MODULE_DEVICE_TABLE(i2c, system_cpld_ids);

static int system_cpld_raw_read(struct device *dev, struct device_attribute *attr, char *buf,
    int reg_offset, int reg_width, int fld_shift, int fld_width, int fld_mask, char *reg_name)
{
    unsigned int reg_val = 0, fld_val;
    static int debug_flag;
    struct system_cpld_data *data = dev_get_drvdata(dev);
    struct i2c_client *client = data->client;
    int err;

    if(reg_width != 8)
    {
        printk("%s: Register table width setting failed.\n", reg_name);
        return -EINVAL;
    }

    mutex_lock(&data->lock);

    if((err = i2c_smbus_read_byte_data(client, (u8)reg_offset)) < 0)
    {
        /* CPLD read error condition */;
        mutex_unlock(&data->lock);
        printk("%s: i2c read failed, error code = %d.\n", reg_name, err);
        return err;
    }
    reg_val = err;

    if(debug_flag)
    {
            printk("%s: reg_offset = %d, width = %d, cur value = 0x%x.\n", reg_name, reg_offset, reg_width, reg_val);
    }

    mutex_unlock(&data->lock);

    if(fld_width == reg_width)
    {
        fld_val = reg_val & fld_mask;
    }
    else
    {
        fld_val = (reg_val >> fld_shift) & fld_mask;
    }

    return sprintf(buf, "0x%x\n", fld_val);
}

static int system_cpld_raw_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count,
    int reg_offset, int reg_width, int fld_shift, int fld_width, int fld_mask, char *reg_name)
{
    int ret_code;
    unsigned int reg_val, fld_val;
    unsigned long val;
    static int debug_flag;
    struct system_cpld_data *data = dev_get_drvdata(dev);
    struct i2c_client *client = data->client;

    if(reg_width != 8)
    {
        printk("%s: Register table width setting failed.\n", reg_name);
        return -EINVAL;
    }

    /* Parse buf and store to fld_val */
    if((ret_code = kstrtoul(buf, 16, &val)))
    {
        printk("%s: Conversion value = %s failed, errno = %d.\n", reg_name, buf, ret_code);
        return ret_code;
    }
    fld_val = (unsigned int)val;

    mutex_lock(&data->lock);

    if((ret_code = i2c_smbus_read_byte_data(client, (u8)reg_offset)) < 0)
    {
        /* Handle CPLD read error condition */;
        mutex_unlock(&data->lock);
        printk("%s: i2c read failed, error code = %d.\n",  reg_name, ret_code);
        return ret_code;
    }
    reg_val = ret_code;

    if(debug_flag)
    {
        printk("%s: offset = %d, width = %d, cur value = 0x%x.\n", reg_name, reg_offset, reg_width, reg_val);
    }

    if(fld_width == reg_width)
    {
        reg_val = fld_val & fld_mask;
    }
    else
    {
        reg_val = (reg_val & ~(fld_mask << fld_shift)) |
                    ((fld_val & (fld_mask)) << fld_shift);
    }

    if((ret_code = i2c_smbus_write_byte_data(client, (u8)reg_offset, (u8)reg_val)) != 0)
    {
        /* Handle CPLD write error condition */;
        mutex_unlock(&data->lock);
        printk("%s: i2c write failed, error code = %d.\n",  reg_name, ret_code);
        return ret_code;
    }
    else if(debug_flag)
    {
        printk("%s: offset = %d, width = %d, new value = 0x%x.\n", reg_name, reg_offset, reg_width, reg_val);
    }

    mutex_unlock(&data->lock);

    return count;
}

/* ---------------------- Read function for specific fields --------------------------- */
static int system_cpld_ver_raw_read(struct device *dev, struct device_attribute *attr, char *buf,
    int reg_offset, char *fld_name)
{
    unsigned int fld_val = 0;
    static int debug_flag;
    struct system_cpld_data *data = dev_get_drvdata(dev);
    struct i2c_client *client = data->client;
    int err_lo, err_hi;

    mutex_lock(&data->lock);

    if((err_hi = i2c_smbus_read_byte_data(client, (u8)reg_offset)) < 0)
    {
        /* CPLD read error condition */;
        mutex_unlock(&data->lock);
        printk("%s: I2C read high byte failed, error code = %d.\n", fld_name, err_hi);
        return err_hi;
    }

    if((err_lo = i2c_smbus_read_byte_data(client, (u8)(reg_offset + 1))) < 0)
    {
        /* CPLD read error condition */;
        mutex_unlock(&data->lock);
        printk("%s: I2C read low byte failed, error code = %d.\n", fld_name, err_lo);
        return err_lo;
    }

    if(debug_flag)
    {
        printk("%s: reg_offset = %d, cur value = 0x%x.\n", fld_name, reg_offset, err_hi);
        printk("%s: reg_offset = %d, cur value = 0x%x.\n", fld_name, reg_offset + 1, err_lo);
    }

    mutex_unlock(&data->lock);

    fld_val = (err_hi << 8) + err_lo;

    return sprintf(buf, "0x%x\n", fld_val);
}

static int system_cpld_fan_raw_read(struct device *dev, struct device_attribute *attr, char *buf,
    int reg_offset, char *fld_name)
{
    unsigned int fld_val;
    static int debug_flag;
    struct system_cpld_data *data = dev_get_drvdata(dev);
    struct i2c_client *client = data->client;
    int err_lo, err_hi;

    mutex_lock(&data->lock);

    if((err_lo = i2c_smbus_read_byte_data(client, (u8)reg_offset)) < 0)
    {
        /* CPLD read error condition */;
        mutex_unlock(&data->lock);
        printk("%s: I2C read low byte failed, error code = %d.\n", fld_name, err_lo);
        return err_lo;
    }

    if((err_hi = i2c_smbus_read_byte_data(client, (u8)(reg_offset + 1))) < 0)
    {
        /* CPLD read error condition */;
        mutex_unlock(&data->lock);
        printk("%s: I2C read high byte failed, error code = %d.\n", fld_name, err_hi);
        return err_hi;
    }

    if(debug_flag)
    {
        printk("%s: reg_offset = %d, cur value = 0x%x.\n", fld_name, reg_offset, err_lo);
        printk("%s: reg_offset = %d, cur value = 0x%x.\n", fld_name, reg_offset + 1, err_hi);
    }

    mutex_unlock(&data->lock);

    if(err_lo == 255 && err_hi == 255)
    {
        fld_val = 0;
    }
    else
    {
        fld_val = (100000 * 60) / (err_hi * 255 + err_lo);
    }

    return sprintf(buf, "%d\n", fld_val);
}

/*-------------------- Special file for debug ---------------------- */
static ssize_t system_cpld_debug_read(struct device *dev, struct device_attribute *attr,
             char *buf)
{
    return sprintf(buf, "%d\n", debug_flag);
}


static ssize_t system_cpld_debug_write(struct device *dev, struct device_attribute *attr,
            const char *buf, size_t count)
{
    int temp;
    int error;

    error = kstrtoint(buf, 10, &temp);
    if(error)
    {
        printk(KERN_INFO "%s: Conversion value = %s failed.\n", __FUNCTION__, buf);
        return count;
    }
    debug_flag = temp;

    if(debug_flag)
        printk("%s, debug_flag = %d\n", __FUNCTION__, debug_flag);

    return count;
}
SYSFS_MISC_RW_ATTR_DEF(debug, system_cpld_debug_read, system_cpld_debug_write)


/* ---------------- Define misc group ---------------------------- */
static struct attribute *misc_attributes[] = {
    SYSFS_ATTR_PTR(pcb_rev),                      /* register: pcb_rev */

    SYSFS_ATTR_PTR(cpld_ver),                     /* register: cpld_ver_2 */

    SYSFS_ATTR_PTR(sys_rst),                      /* register: rst_ctrl_1 */
    SYSFS_ATTR_PTR(pcie_perst),                   /* register: rst_ctrl_1 */
    SYSFS_ATTR_PTR(qspi_rst),                     /* register: rst_ctrl_1 */
    SYSFS_ATTR_PTR(tps53681_rst),                 /* register: rst_ctrl_1 */
    SYSFS_ATTR_PTR(pcie_lan_rst),                 /* register: rst_ctrl_1 */
    SYSFS_ATTR_PTR(ptp_rst),                      /* register: rst_ctrl_1 */
    SYSFS_ATTR_PTR(ja_rst),                       /* register: rst_ctrl_1 */

    SYSFS_ATTR_PTR(pca9548_rst),                  /* register: rst_ctrl_2 */
    SYSFS_ATTR_PTR(zqsfp_rst),                    /* register: rst_ctrl_2 */
    SYSFS_ATTR_PTR(shift_rst),                    /* register: rst_ctrl_2 */
    SYSFS_ATTR_PTR(i2c0_rst),                     /* register: rst_ctrl_2 */
    SYSFS_ATTR_PTR(i2c1_rst),                     /* register: rst_ctrl_2 */
    SYSFS_ATTR_PTR(cpu_pltrst),                   /* register: rst_ctrl_2 */
    SYSFS_ATTR_PTR(cpu_cpld_rst),                 /* register: rst_ctrl_2 */
    SYSFS_ATTR_PTR(bcm56870_sys_rst),             /* register: rst_ctrl_2 */

    SYSFS_ATTR_PTR(i2c1_fan_rst),                 /* register: rst_ctrl_3 */
    SYSFS_ATTR_PTR(bmc_rst),                      /* register: rst_ctrl_3 */
    SYSFS_ATTR_PTR(fm_cpld_ngff_rst1),            /* register: rst_ctrl_3 */
    SYSFS_ATTR_PTR(fm_cpld_ngff_rst2),            /* register: rst_ctrl_3 */
    SYSFS_ATTR_PTR(pca9541a_1_rst),               /* register: rst_ctrl_3 */
    SYSFS_ATTR_PTR(pca9541a_2_rst),               /* register: rst_ctrl_3 */
    SYSFS_ATTR_PTR(mcp2210_rst),                  /* register: rst_ctrl_3 */

    SYSFS_ATTR_PTR(int_1),                        /* register: int_1_reg */

    SYSFS_ATTR_PTR(int_2),                        /* register: int_2_reg */

    SYSFS_ATTR_PTR(int_3),                        /* register: int_3_reg */

    SYSFS_ATTR_PTR(qsfp01_06_imask),              /* register: imask_1 */
    SYSFS_ATTR_PTR(qsfp07_12_imask),              /* register: imask_1 */
    SYSFS_ATTR_PTR(qsfp13_18_imask),              /* register: imask_1 */
    SYSFS_ATTR_PTR(qsfp19_24_imask),              /* register: imask_1 */
    SYSFS_ATTR_PTR(qsfp25_30_imask),              /* register: imask_1 */
    SYSFS_ATTR_PTR(qsfp31_32_imask),              /* register: imask_1 */
    SYSFS_ATTR_PTR(ja_imask),                     /* register: imask_1 */

    SYSFS_ATTR_PTR(temp1_alert_imask),            /* register: imask_2 */
    SYSFS_ATTR_PTR(temp2_alert_imask),            /* register: imask_2 */
    SYSFS_ATTR_PTR(temp3_alert_imask),            /* register: imask_2 */
    SYSFS_ATTR_PTR(lan_alert_imask),              /* register: imask_2 */
    SYSFS_ATTR_PTR(ucd9090_i2c_alert_imask),      /* register: imask_2 */
    SYSFS_ATTR_PTR(pcie_imask),                   /* register: imask_2 */
    SYSFS_ATTR_PTR(temp_fan_imask),               /* register: imask_2 */
    SYSFS_ATTR_PTR(i2c_peci_alert_imask),         /* register: imask_2 */

    SYSFS_ATTR_PTR(bmc_pwrbtn_imask),             /* register: imask_3 */
    SYSFS_ATTR_PTR(usb_fault_imask),              /* register: imask_3 */
    SYSFS_ATTR_PTR(mb_imask),                     /* register: imask_3 */
    SYSFS_ATTR_PTR(tps53681_smb_alert_imask),     /* register: imask_3 */
    SYSFS_ATTR_PTR(tps53681_vr_fault_imask),      /* register: imask_3 */
    SYSFS_ATTR_PTR(tps53681_vr_hot_imask),        /* register: imask_3 */
    SYSFS_ATTR_PTR(pwrbtn_imask),                 /* register: imask_3 */
    SYSFS_ATTR_PTR(lpc_serirq_imask),             /* register: imask_3 */

    SYSFS_ATTR_PTR(fan_pwm_1),                    /* register: fan_pwm_1_reg */

    SYSFS_ATTR_PTR(fan_pwm_2),                    /* register: fan_pwm_2_reg */

    SYSFS_ATTR_PTR(fan_pwm_3),                    /* register: fan_pwm_3_reg */

    SYSFS_ATTR_PTR(fan_pwm_4),                    /* register: fan_pwm_4_reg */

    SYSFS_ATTR_PTR(fan_pwm_5),                    /* register: fan_pwm_5_reg */

    SYSFS_ATTR_PTR(fan_pwm_6),                    /* register: fan_pwm_6_reg */

    SYSFS_ATTR_PTR(fan_tach_1_1),                 /* register: fan_tach_1_1a */

    SYSFS_ATTR_PTR(fan_tach_1_2),                 /* register: fan_tach_1_2a */

    SYSFS_ATTR_PTR(fan_tach_2_1),                 /* register: fan_tach_2_1a */

    SYSFS_ATTR_PTR(fan_tach_2_2),                 /* register: fan_tach_2_2a */

    SYSFS_ATTR_PTR(fan_tach_3_1),                 /* register: fan_tach_3_1a */

    SYSFS_ATTR_PTR(fan_tach_3_2),                 /* register: fan_tach_3_2a */

    SYSFS_ATTR_PTR(fan_tach_4_1),                 /* register: fan_tach_4_1a */

    SYSFS_ATTR_PTR(fan_tach_4_2),                 /* register: fan_tach_4_2a */

    SYSFS_ATTR_PTR(fan_tach_5_1),                 /* register: fan_tach_5_1a */

    SYSFS_ATTR_PTR(fan_tach_5_2),                 /* register: fan_tach_5_2a */

    SYSFS_ATTR_PTR(fan_tach_6_1),                 /* register: fan_tach_6_1a */

    SYSFS_ATTR_PTR(fan_tach_6_2),                 /* register: fan_tach_6_2a */

    SYSFS_ATTR_PTR(psu_ps_on_lt_1),               /* register: psu_ctrl_1 */
    SYSFS_ATTR_PTR(psu_ps_ok_lt_1),               /* register: psu_ctrl_1 */
    SYSFS_ATTR_PTR(psu_present_lt_1),             /* register: psu_ctrl_1 */
    SYSFS_ATTR_PTR(psu_smb_alert_lt_1),           /* register: psu_ctrl_1 */

    SYSFS_ATTR_PTR(psu_ps_on_lt_2),               /* register: psu_ctrl_2 */
    SYSFS_ATTR_PTR(psu_ps_ok_lt_2),               /* register: psu_ctrl_2 */
    SYSFS_ATTR_PTR(psu_present_lt_2),             /* register: psu_ctrl_2 */
    SYSFS_ATTR_PTR(psu_smb_alert_lt_2),           /* register: psu_ctrl_2 */

    SYSFS_ATTR_PTR(id_led),                       /* register: status_led_reg */
    SYSFS_ATTR_PTR(status_led),                   /* register: status_led_reg */

    SYSFS_ATTR_PTR(fan_led_1),                    /* register: fan_led */
    SYSFS_ATTR_PTR(fan_led_2),                    /* register: fan_led */
    SYSFS_ATTR_PTR(fan_led_3),                    /* register: fan_led */
    SYSFS_ATTR_PTR(fan_led_4),                    /* register: fan_led */
    SYSFS_ATTR_PTR(fan_led_5),                    /* register: fan_led */
    SYSFS_ATTR_PTR(fan_led_6),                    /* register: fan_led */

    SYSFS_ATTR_PTR(vdd_core_pg),                  /* register: pwr_good */
    SYSFS_ATTR_PTR(vdd_0v8_pg),                   /* register: pwr_good */
    SYSFS_ATTR_PTR(vdd_3v3_pg),                   /* register: pwr_good */
    SYSFS_ATTR_PTR(vdd_1v2_pg),                   /* register: pwr_good */
    SYSFS_ATTR_PTR(vdd_3v3sb_pg),                 /* register: pwr_good */
    SYSFS_ATTR_PTR(pwr_ok),                       /* register: pwr_good */
    SYSFS_ATTR_PTR(vdd_3v3clk_pg),                /* register: pwr_good */

    SYSFS_ATTR_PTR(fan_tach_rev),                 /* register: fan_tach_rev_reg */

    SYSFS_ATTR_PTR(wd_rst_en),                    /* register: wd_ctrl */
    SYSFS_ATTR_PTR(wd_rst_clear),                 /* register: wd_ctrl */

    SYSFS_ATTR_PTR(wdt_period),                   /* register: wdt_period */

    SYSFS_ATTR_PTR(debug),                        /* debug flag for print more messages */

    SYSFS_ATTR_PTR(fan_present_6),                /* register: fan_present */
    SYSFS_ATTR_PTR(fan_present_5),                /* register: fan_present */
    SYSFS_ATTR_PTR(fan_present_4),                /* register: fan_present */
    SYSFS_ATTR_PTR(fan_present_3),                /* register: fan_present */
    SYSFS_ATTR_PTR(fan_present_2),                /* register: fan_present */
    SYSFS_ATTR_PTR(fan_present_1),                /* register: fan_present */

    SYSFS_ATTR_PTR(fan_direction_6),              /* register: fan_direction */
    SYSFS_ATTR_PTR(fan_direction_5),              /* register: fan_direction */
    SYSFS_ATTR_PTR(fan_direction_4),              /* register: fan_direction */
    SYSFS_ATTR_PTR(fan_direction_3),              /* register: fan_direction */
    SYSFS_ATTR_PTR(fan_direction_2),              /* register: fan_direction */
    SYSFS_ATTR_PTR(fan_direction_1),              /* register: fan_direction */
    NULL
};
static const struct attribute_group system_cpld_group_misc = {
    .attrs = misc_attributes,
};

static int system_cpld_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int err;

    /* allocate memory to system_cpld */
    system_cpld = devm_kzalloc(&client->dev, sizeof(struct system_cpld_data), GFP_KERNEL);

    if(!system_cpld)
        return -ENOMEM;

    mutex_init(&system_cpld->lock);

    err = sysfs_create_group(&client->dev.kobj, &system_cpld_group_misc);
    if(err)
    {
        printk("%s: Error creeat misc group.\n", __FUNCTION__);
    }

    system_cpld->client = client;
    i2c_set_clientdata(client, system_cpld);

    printk(KERN_INFO "%s: System CPLD 5M1270 created.\n", __FUNCTION__);

    return 0;
}

static int system_cpld_remove(struct i2c_client *client)
{
    sysfs_remove_group(&client->dev.kobj, &system_cpld_group_misc);

    printk(KERN_INFO "%s: System CPLD removed.\n", __FUNCTION__);
    return 0;
}

static struct i2c_driver system_cpld_driver = {
    .driver = {
        .name = "rseb_w1_32_sys_cpld",
        .owner = THIS_MODULE,
    },
    .probe = system_cpld_probe,
    .remove = system_cpld_remove,
    .id_table = system_cpld_ids,
};

static int __init system_cpld_init(void)
{
    printk(KERN_INFO "%s: init.\n", __FUNCTION__);
    return i2c_add_driver(&system_cpld_driver);
}
module_init(system_cpld_init);

static void __exit system_cpld_exit(void)
{
    printk(KERN_INFO "%s: exit.\n", __FUNCTION__);
    i2c_del_driver(&system_cpld_driver);
}
module_exit(system_cpld_exit);

MODULE_DESCRIPTION("Driver for system CPLD");
MODULE_AUTHOR("Chris Chiang <Chris.Chiang@wnc.com.tw>");
MODULE_LICENSE("GPL");

