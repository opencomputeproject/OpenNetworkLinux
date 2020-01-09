#include <linux/module.h>
#include <linux/i2c.h>

#include "rseb-w1-32-cpu_cpld_reg.h"
#include "rseb-w1-32-cpu_cpld_sysfs.h"

static int debug_flag = 0;

struct cpu_cpld_data {
    struct mutex lock;
    struct i2c_client *client;
    struct device_attribute bin;
};
struct cpu_cpld_data *cpu_cpld;

static const struct i2c_device_id cpu_cpld_ids[] = {
    { "rseb_w1_32_cpu_cpld", 0 },
    { /* END OF LIST */ }
};
MODULE_DEVICE_TABLE(i2c, cpu_cpld_ids);

static int cpu_cpld_raw_read(struct device *dev, struct device_attribute *attr, char *buf,
    int reg_offset, int reg_width, int fld_shift, int fld_width, int fld_mask, char *reg_name)
{
    unsigned int reg_val = 0, fld_val;
    static int debug_flag;
    struct cpu_cpld_data *data = dev_get_drvdata(dev);
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

static int cpu_cpld_raw_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count,
    int reg_offset, int reg_width, int fld_shift, int fld_width, int fld_mask, char *reg_name)
{
    int ret_code;
    unsigned int reg_val, fld_val;
    unsigned long val;
    static int debug_flag;
    struct cpu_cpld_data *data = dev_get_drvdata(dev);
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

/*-------------------- Special file for debug ---------------------- */
static ssize_t cpu_cpld_debug_read(struct device *dev, struct device_attribute *attr,
             char *buf)
{
    return sprintf(buf, "%d\n", debug_flag);
}


static ssize_t cpu_cpld_debug_write(struct device *dev, struct device_attribute *attr,
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
SYSFS_MISC_RW_ATTR_DEF(debug, cpu_cpld_debug_read, cpu_cpld_debug_write)


/* ---------------- Define misc group ---------------------------- */
static struct attribute *misc_attributes[] = {
    SYSFS_ATTR_PTR(p1v8_stby_pg),                 /* register: pwr_gd_1 */
    SYSFS_ATTR_PTR(pvnn_pg),                      /* register: pwr_gd_1 */
    SYSFS_ATTR_PTR(p1v05_pg),                     /* register: pwr_gd_1 */
    SYSFS_ATTR_PTR(p2v5_vpp_pg),                  /* register: pwr_gd_1 */
    SYSFS_ATTR_PTR(p1v2_vddq_pg),                 /* register: pwr_gd_1 */
    SYSFS_ATTR_PTR(pvccref_pg),                  /* register: pwr_gd_1 */
    SYSFS_ATTR_PTR(pvccsram_pg),                  /* register: pwr_gd_1 */
    SYSFS_ATTR_PTR(cpld_p3v3_pg),                 /* register: pwr_gd_1 */

    SYSFS_ATTR_PTR(pvccp_pg),                     /* register: pwr_gd_2 */
    SYSFS_ATTR_PTR(pvtt_pg),                      /* register: pwr_gd_2 */
    SYSFS_ATTR_PTR(soc_slp3),                     /* register: pwr_gd_2 */
    SYSFS_ATTR_PTR(soc_cpld_slp_s45),             /* register: pwr_gd_2 */
    SYSFS_ATTR_PTR(soc_pltrst),                   /* register: pwr_gd_2 */
    SYSFS_ATTR_PTR(rst_cpld_rsmrst),              /* register: pwr_gd_2 */

    SYSFS_ATTR_PTR(cpld_p1v8_stby_en),            /* register: pwr_en_1 */
    SYSFS_ATTR_PTR(cpld_pvnn_en),                 /* register: pwr_en_1 */
    SYSFS_ATTR_PTR(cpld_p1v05_en),                /* register: pwr_en_1 */
    SYSFS_ATTR_PTR(rst_rsmrst),                   /* register: pwr_en_1 */
    SYSFS_ATTR_PTR(cpld_p2v5_vpp_en),             /* register: pwr_en_1 */
    SYSFS_ATTR_PTR(cpld_p1v2_vddq_en),            /* register: pwr_en_1 */
    SYSFS_ATTR_PTR(cpld_p0v6_vtt_dimm_en),        /* register: pwr_en_1 */
    SYSFS_ATTR_PTR(cpld_pvccref_en),              /* register: pwr_en_1 */

    SYSFS_ATTR_PTR(cpld_pvccsram_en),             /* register: pwr_en_2 */
    SYSFS_ATTR_PTR(cpld_p3v3_en),                 /* register: pwr_en_2 */
    SYSFS_ATTR_PTR(cpld_pvccp_en),                /* register: pwr_en_2 */
    SYSFS_ATTR_PTR(cpld_corepwrok),               /* register: pwr_en_2 */

    SYSFS_ATTR_PTR(p1v8_stby_err),                /* register: pwr_err_1 */
    SYSFS_ATTR_PTR(pvnn_err),                     /* register: pwr_err_1 */
    SYSFS_ATTR_PTR(p1v05_err),                    /* register: pwr_err_1 */
    SYSFS_ATTR_PTR(p2v5_vpp_err),                 /* register: pwr_err_1 */
    SYSFS_ATTR_PTR(p1v2_vddq_err),                /* register: pwr_err_1 */
    SYSFS_ATTR_PTR(pvtt_err),                     /* register: pwr_err_1 */
    SYSFS_ATTR_PTR(pvccref_err),                  /* register: pwr_err_1 */
    SYSFS_ATTR_PTR(pvccsram_err),                 /* register: pwr_err_1 */

    SYSFS_ATTR_PTR(p3v3_err),                     /* register: pwr_err_2 */
    SYSFS_ATTR_PTR(pvccp_err),                    /* register: pwr_err_2 */
    SYSFS_ATTR_PTR(memevent),                     /* register: pwr_err_2 */
    SYSFS_ATTR_PTR(dimm_event_co),                /* register: pwr_err_2 */

    SYSFS_ATTR_PTR(soc_cpld_mcerr),               /* register: cpu_err */
    SYSFS_ATTR_PTR(cpld_soc_thermtrip),           /* register: cpu_err */
    SYSFS_ATTR_PTR(soc_caterr),                   /* register: cpu_err */
    SYSFS_ATTR_PTR(soc_err0),                     /* register: cpu_err */
    SYSFS_ATTR_PTR(soc_err1),                     /* register: cpu_err */
    SYSFS_ATTR_PTR(soc_err2),                     /* register: cpu_err */

    SYSFS_ATTR_PTR(cpld_vrhot),                   /* register: vrhot */
    SYSFS_ATTR_PTR(pvnn_vrhot),                   /* register: vrhot */
    SYSFS_ATTR_PTR(pvccsram_vrhot),               /* register: vrhot */
    SYSFS_ATTR_PTR(soc_throttle),                 /* register: vrhot */
    SYSFS_ATTR_PTR(soc_me_drive),                 /* register: vrhot */
    SYSFS_ATTR_PTR(soc_prochot_disable),          /* register: vrhot */
    SYSFS_ATTR_PTR(edge_fast_prochot),            /* register: vrhot */

    SYSFS_ATTR_PTR(intr),                         /* register: intr_reg */

    SYSFS_ATTR_PTR(pwr_st),                       /* register: pwr_st_reg */

    SYSFS_ATTR_PTR(bmc_cpu_prsnt),                /* register: misc_mb_bmc */
    SYSFS_ATTR_PTR(bios_mux_sel),                 /* register: misc_mb_bmc */
    SYSFS_ATTR_PTR(cpld_pwrbtn_bmc),              /* register: misc_mb_bmc */
    SYSFS_ATTR_PTR(rst_sysrst),                   /* register: misc_mb_bmc */

    SYSFS_ATTR_PTR(cpu_cpld_rev),                 /* register: cpu_cpld_rev_reg */

    SYSFS_ATTR_PTR(cpu_board_rev),                /* register: cpu_board_rev_reg */

    SYSFS_ATTR_PTR(bios_update_en),               /* register: ctrl */

    SYSFS_ATTR_PTR(debug),                        /* debug flag for print more messages */
    NULL
};
static const struct attribute_group cpu_cpld_group_misc = {
    .attrs = misc_attributes,
};

static int cpu_cpld_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int err;

    /* allocate memory to cpu_cpld */
    cpu_cpld = devm_kzalloc(&client->dev, sizeof(struct cpu_cpld_data), GFP_KERNEL);

    if(!cpu_cpld)
        return -ENOMEM;

    mutex_init(&cpu_cpld->lock);

    err = sysfs_create_group(&client->dev.kobj, &cpu_cpld_group_misc);
    if(err)
    {
        printk("%s: Error creeat misc group.\n", __FUNCTION__);
    }

    cpu_cpld->client = client;
    i2c_set_clientdata(client, cpu_cpld);

    printk(KERN_INFO "%s: CPU CPLD created.\n", __FUNCTION__);

    return 0;
}

static int cpu_cpld_remove(struct i2c_client *client)
{
    sysfs_remove_group(&client->dev.kobj, &cpu_cpld_group_misc);

    printk(KERN_INFO "%s: CPU CPLD removed.\n", __FUNCTION__);
    return 0;
}

static struct i2c_driver cpu_cpld_driver = {
    .driver = {
        .name = "rseb_w1_32_cpu_cpld",
        .owner = THIS_MODULE,
    },
    .probe = cpu_cpld_probe,
    .remove = cpu_cpld_remove,
    .id_table = cpu_cpld_ids,
};

static int __init cpu_cpld_init(void)
{
    printk(KERN_INFO "%s: init.\n", __FUNCTION__);
    return i2c_add_driver(&cpu_cpld_driver);
}
module_init(cpu_cpld_init);

static void __exit cpu_cpld_exit(void)
{
    printk(KERN_INFO "%s: exit.\n", __FUNCTION__);
    i2c_del_driver(&cpu_cpld_driver);
}
module_exit(cpu_cpld_exit);

MODULE_DESCRIPTION("Driver for CPU CPLD");
MODULE_AUTHOR("Chris Chiang <Chris.Chiang@wnc.com.tw>");
MODULE_LICENSE("GPL");

