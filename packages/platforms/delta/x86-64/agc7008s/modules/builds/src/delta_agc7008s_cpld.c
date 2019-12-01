#include "delta_agc7008s_common.h"

unsigned char cpupld_reg_addr;
/* --------------- CPLD - start --------------- */
/* CPLD device */
static struct platform_device agc7008s_cpld = {
    .name  = "delta-agc7008s-cpld",
    .id    = 0,
    .dev   = {
        .platform_data  = agc7008s_cpu_cpld_platform_data,
        .release        = device_release
    },
};

static ssize_t get_cpld_reg(struct device *dev, struct device_attribute *dev_attr, char *buf)
{
    int ret;
    int mask;
    int value;
    char note[ATTRIBUTE_NOTE_SIZE];
    unsigned char reg;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);
    struct cpld_platform_data *pdata = dev->platform_data;

    switch (attr->index) {
        case CPU_CPLD_REG_ADDR:
            return sprintf(buf, "0x%02x\n", cpupld_reg_addr);
        case CPU_CPLD_REG_VALUE:
            ret = i2c_smbus_read_byte_data(pdata[cpu_cpld].client, cpupld_reg_addr);
            return sprintf(buf, "0x%02x\n", ret);
        case CPU_CPLD_VER ... USB_OVER_CURRENT_MASK:
            reg   = attribute_data[attr->index].reg;
            mask  = attribute_data[attr->index].mask;
            value = i2c_smbus_read_byte_data(pdata[cpu_cpld].client, reg);
            sprintf(note, "\n%s\n",attribute_data[attr->index].note);
            value = (value & mask);
            break;
        default:
            return sprintf(buf, "%d not found", attr->index);
    }

    switch (mask) {
        case 0xFF:
            return sprintf(buf, "0x%02x%s", value, note);
        case 0x0F:
            return sprintf(buf, "0x%01x%s", value, note);
        case 0xF0:
            value = value >> 4;
            return sprintf(buf, "0x%01x%s", value, note);
        default :
            value = value >> dni_log2(mask);
            return sprintf(buf, "%d%s", value, note);
    }
}

static ssize_t set_cpld_reg(struct device *dev, struct device_attribute *dev_attr,
             const char *buf, size_t count)
{
    int err;
    int value;
    int set_data;
    unsigned long set_data_ul;
    unsigned char reg;
    unsigned char mask;
    unsigned char mask_out;

    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);
    struct cpld_platform_data *pdata = dev->platform_data;

    err = kstrtoul(buf, 0, &set_data_ul);
    if (err){
        return err;
    }

    set_data = (int)set_data_ul;
    if (set_data > 0xff) {
        printk(KERN_ALERT "address out of range (0x00-0xFF)\n");
        return count;
    }

    switch (attr->index) {
        case CPU_CPLD_REG_ADDR:
            cpupld_reg_addr = set_data;
            return count;
        case CPU_CPLD_REG_VALUE:
            i2c_smbus_write_byte_data(pdata[cpu_cpld].client, cpupld_reg_addr, set_data);
            return count;
        case CPU_CPLD_VER ... USB_OVER_CURRENT_MASK:
            reg   = attribute_data[attr->index].reg;
            mask  = attribute_data[attr->index].mask;
            value = i2c_smbus_read_byte_data(pdata[cpu_cpld].client, reg);
            mask_out = value & ~(mask);
            break;
        default:
            return sprintf((char *)buf, "%d not found", attr->index);
    }

    switch (mask) {
        case 0xFF:
            set_data = mask_out | (set_data & mask);
            break;
        case 0x0F:
            set_data = mask_out | (set_data & mask);
            break;
        case 0xF0:
            set_data = set_data << 4;
            set_data = mask_out | (set_data & mask);
            break;
        default :
            set_data = mask_out | (set_data << dni_log2(mask) );
    }

    i2c_smbus_write_byte_data(pdata[cpu_cpld].client, reg, set_data);
    return count;
}

static SENSOR_DEVICE_ATTR(cpu_cpld_reg_addr,     S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, CPU_CPLD_REG_ADDR);
static SENSOR_DEVICE_ATTR(cpu_cpld_reg_value,    S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, CPU_CPLD_REG_VALUE);
static SENSOR_DEVICE_ATTR(cpu_cpld_ver,          S_IRUGO,           get_cpld_reg, NULL,         CPU_CPLD_VER);
static SENSOR_DEVICE_ATTR(cpu_pcb_ver,           S_IRUGO,           get_cpld_reg, NULL,         CPU_PCB_VER);
static SENSOR_DEVICE_ATTR(thermal_int,           S_IRUGO,           get_cpld_reg, NULL,         THERMAL_INT);
static SENSOR_DEVICE_ATTR(tps53622_over_thermal, S_IRUGO,           get_cpld_reg, NULL,         TPS53622_OVER_THERMAL);
static SENSOR_DEVICE_ATTR(cpu_thermal_trip,      S_IRUGO,           get_cpld_reg, NULL,         CPU_THERMAL_TRIP);
static SENSOR_DEVICE_ATTR(rtc_test,              S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, RTC_TEST);
static SENSOR_DEVICE_ATTR(rtc_rst,               S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, RTC_RST);
static SENSOR_DEVICE_ATTR(cpld_tpm_rst,          S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, CPLD_TPM_RST);
static SENSOR_DEVICE_ATTR(force_rst,             S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, FORCE_RST);
static SENSOR_DEVICE_ATTR(backup_bios_wp,        S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, BACKUP_BIOS_WP);
static SENSOR_DEVICE_ATTR(main_bios_wp,          S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, MAIN_BIOS_WP);
static SENSOR_DEVICE_ATTR(bios_chip_sel,         S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, BIOS_CHIP_SEL);
static SENSOR_DEVICE_ATTR(bios_chip_sel_ctrl,    S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, BIOS_CHIP_SEL_CTRL);
static SENSOR_DEVICE_ATTR(main_board_int,        S_IRUGO,           get_cpld_reg, NULL,         MAIN_BOARD_INT);
static SENSOR_DEVICE_ATTR(tpm_int,               S_IRUGO,           get_cpld_reg, NULL,         TPM_INT);
static SENSOR_DEVICE_ATTR(lpc_int,               S_IRUGO,           get_cpld_reg, NULL,         LPC_INT);
static SENSOR_DEVICE_ATTR(usb_over_current,      S_IRUGO,           get_cpld_reg, NULL,         USB_OVER_CURRENT);
static SENSOR_DEVICE_ATTR(main_board_int_mask,   S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, MAIN_BOARD_INT_MASK);
static SENSOR_DEVICE_ATTR(tpm_int_mask,          S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, TPM_INT_MASK);
static SENSOR_DEVICE_ATTR(lpc_int_mask,          S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, LPC_INT_MASK);
static SENSOR_DEVICE_ATTR(usb_over_current_mask, S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, USB_OVER_CURRENT_MASK);

static struct attribute *agc7008s_cpld_attrs[] = {
    &sensor_dev_attr_cpu_cpld_reg_value.dev_attr.attr,
    &sensor_dev_attr_cpu_cpld_reg_addr.dev_attr.attr,
    &sensor_dev_attr_cpu_cpld_ver.dev_attr.attr,
    &sensor_dev_attr_cpu_pcb_ver.dev_attr.attr,
    &sensor_dev_attr_thermal_int.dev_attr.attr,
    &sensor_dev_attr_tps53622_over_thermal.dev_attr.attr,
    &sensor_dev_attr_cpu_thermal_trip.dev_attr.attr,
    &sensor_dev_attr_rtc_test.dev_attr.attr,
    &sensor_dev_attr_rtc_rst.dev_attr.attr,
    &sensor_dev_attr_cpld_tpm_rst.dev_attr.attr,
    &sensor_dev_attr_force_rst.dev_attr.attr,
    &sensor_dev_attr_backup_bios_wp.dev_attr.attr,
    &sensor_dev_attr_main_bios_wp.dev_attr.attr,
    &sensor_dev_attr_bios_chip_sel.dev_attr.attr,
    &sensor_dev_attr_bios_chip_sel_ctrl.dev_attr.attr,
    &sensor_dev_attr_main_board_int.dev_attr.attr,
    &sensor_dev_attr_tpm_int.dev_attr.attr,
    &sensor_dev_attr_lpc_int.dev_attr.attr,
    &sensor_dev_attr_usb_over_current.dev_attr.attr,
    &sensor_dev_attr_main_board_int_mask.dev_attr.attr,
    &sensor_dev_attr_tpm_int_mask.dev_attr.attr,
    &sensor_dev_attr_lpc_int_mask.dev_attr.attr,
    &sensor_dev_attr_usb_over_current_mask.dev_attr.attr,
    NULL,
};

static struct attribute_group agc7008s_cpld_attr_grp = {
    .attrs = agc7008s_cpld_attrs,
};

static int __init cpld_probe(struct platform_device *pdev)
{
    struct cpld_platform_data *pdata;
    struct i2c_adapter *parent;
    int ret;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "CPLD platform data not found\n");
        return -ENODEV;
    }

    parent = i2c_get_adapter(BUS1);
    if (!parent) {
        printk(KERN_WARNING "Parent adapter (%d) not found\n",BUS1);
        return -ENODEV;
    }

    pdata[cpu_cpld].client = i2c_new_dummy(parent, pdata[cpu_cpld].reg_addr);
    if (!pdata[cpu_cpld].client) {
        printk(KERN_WARNING "Fail to create dummy i2c client for addr %d\n", pdata[cpu_cpld].reg_addr);
        goto error;
    }

    ret = sysfs_create_group(&pdev->dev.kobj, &agc7008s_cpld_attr_grp);
    if (ret) {
        printk(KERN_WARNING "Fail to create cpld attribute group");
        goto error;
    }

    return 0;

error:
    i2c_unregister_device(pdata[cpu_cpld].client);
    i2c_put_adapter(parent);

    return -ENODEV;
}

static int __exit cpld_remove(struct platform_device *pdev)
{
    struct i2c_adapter *parent = NULL;
    struct cpld_platform_data *pdata = pdev->dev.platform_data;
    sysfs_remove_group(&pdev->dev.kobj, &agc7008s_cpld_attr_grp);

    if (!pdata) {
        dev_err(&pdev->dev, "Missing platform data\n");
    }
    else {
        if (pdata[cpu_cpld].client) {
            if (!parent) {
                parent = (pdata[cpu_cpld].client)->adapter;
            }
        i2c_unregister_device(pdata[cpu_cpld].client);
        }
    }
    i2c_put_adapter(parent);

    return 0;
}

static struct platform_driver cpld_driver = {
    .probe  = cpld_probe,
    .remove = __exit_p(cpld_remove),
    .driver = {
        .owner  = THIS_MODULE,
        .name   = "delta-agc7008s-cpld",
    },
};
/* --------------- CPLD - end --------------- */

/* --------------- module initialization --------------- */
static int __init delta_agc7008s_cpupld_init(void)
{
    int ret;
    printk(KERN_WARNING "agc7008s_platform_cpupld module initialization\n");

    ret = dni_create_user();
    if (ret != 0){
        printk(KERN_WARNING "Fail to create IPMI user\n");
    }

    // set the CPUPLD prob and remove
    ret = platform_driver_register(&cpld_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register cpupld driver\n");
        goto error_cpupld_driver;
    }

    // register the CPUPLD
    ret = platform_device_register(&agc7008s_cpld);
    if (ret) {
        printk(KERN_WARNING "Fail to create cpupld device\n");
        goto error_agc7008s_cpupld;
    }
    return 0;

error_agc7008s_cpupld:
    platform_driver_unregister(&cpld_driver);
error_cpupld_driver:
    return ret;
}

static void __exit delta_agc7008s_cpupld_exit(void)
{
    platform_device_unregister(&agc7008s_cpld);
    platform_driver_unregister(&cpld_driver);
}
module_init(delta_agc7008s_cpupld_init);
module_exit(delta_agc7008s_cpupld_exit);

MODULE_DESCRIPTION("DNI agc7008s CPLD Platform Support");
MODULE_AUTHOR("Jacky Liu <jacky.js.liu@deltaww.com>");
MODULE_LICENSE("GPL");
