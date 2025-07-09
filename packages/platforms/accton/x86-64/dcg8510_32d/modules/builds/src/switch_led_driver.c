#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/slab.h>

#include "switch_led_driver.h"
#include "switch_cpld_driver.h"

#define DRVNAME "drv_led_driver"
#define SWITCH_LED_DRIVER_VERSION "0.0.1"

unsigned int loglevel = 0;
static struct platform_device *drv_led_device;

bool drv_get_location_led(unsigned int *loc)
{
    return true;
}

bool drv_set_location_led(unsigned int loc)
{
    return true;
}

int drv_get_port_led_num(void)
{
    return true;
}

bool drv_get_port_led(unsigned long *bitmap)
{
    return true;
}

bool drv_set_port_led(unsigned long *bitmap)
{
    return true;
}

bool drv_get_sys_led(unsigned char *led)
{
    int ret = 0;
    unsigned char reg_value = 0;
    ret = switch_cpld_reg_read(SYS_CPLD, CPLD_SYS_LED_CTL_OFFSET, &reg_value);
    *led = (reg_value & 0x3F);
    if(ret < 0)
        return false;
    return true;
}

bool drv_set_sys_led(unsigned char led)
{
    int ret = 0;
    unsigned char reg_value = 0;
    reg_value = (0x40 | led);
    ret = switch_cpld_reg_write(SYS_CPLD, CPLD_SYS_LED_CTL_OFFSET, reg_value);
    if(ret < 0)
        return false;
    return true;
}

bool drv_get_bmc_led(unsigned char *led)
{
    int ret = 0;
    ret = switch_cpld_reg_read(SYS_CPLD, CPLD_BMC_LED_CTL_OFFSET, led);
    if(ret < 0)
        return false;
    return true;
}

bool drv_set_bmc_led(unsigned char led)
{
    int ret = 0;
    ret = switch_cpld_reg_write(SYS_CPLD, CPLD_BMC_LED_CTL_OFFSET, led);
    if(ret < 0)
        return false;
    return true;
}

bool drv_get_fan_led(unsigned char *led)
{
    int ret = 0;
    ret = switch_cpld_reg_read(SYS_CPLD, CPLD_FAN_LED_CTL_OFFSET, led);
    if(ret < 0)
        return false;
    return true;
}

bool drv_set_fan_led(unsigned char led)
{
    int ret = 0;
    ret = switch_cpld_reg_write(SYS_CPLD, CPLD_FAN_LED_CTL_OFFSET, led);
    if(ret < 0)
        return false;
    return true;
}

bool drv_get_psu_led(unsigned char *led)
{
    int ret = 0;
    ret = switch_cpld_reg_read(SYS_CPLD, CPLD_PSU_LED_CTL_OFFSET, led);
    if(ret < 0)
        return false;
    return true;
}

bool drv_set_psu_led(unsigned char led)
{
    int ret = 0;
    ret = switch_cpld_reg_write(SYS_CPLD, CPLD_PSU_LED_CTL_OFFSET, led);
    if(ret < 0)
        return false;
    return true;
}

bool drv_get_id_led(unsigned int *led, unsigned int cs)
{
    return true;
}
bool drv_set_id_led(unsigned int led, unsigned int cs)
{
    return true;
}

bool drv_set_hw_control(void)
{
    return true;
}

void drv_get_loglevel(long *lev)
{
    *lev = (long)loglevel;

    return;
}

void drv_set_loglevel(long lev)
{
    loglevel = (unsigned int)lev;

    return;
}

ssize_t drv_debug_help(char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE,
        "sysname              debug cmd\n"
        "fan                  busybox devmem 0xfc800031 8\n"
        "sys                  busybox devmem 0xfc800032 8\n"
        "bmc                  busybox devmem 0xfc800033 8\n"
        "psu                  busybox devmem 0xfc800034 8\n"
        "                     \n");
#else
    return sprintf(buf,
        "sysname              debug cmd\n"
        "fan                  busybox devmem 0xfc800031 8\n"
        "sys                  busybox devmem 0xfc800032 8\n"
        "bmc                  busybox devmem 0xfc800033 8\n"
        "psu                  busybox devmem 0xfc800034 8\n"
        "                     \n");
#endif
}

ssize_t drv_debug(const char *buf, int count)
{
    return 0;
}

// For s3ip
static struct led_drivers_t pfunc = {
    .get_sys_led = drv_get_sys_led,
    .set_sys_led = drv_set_sys_led,
    .get_bmc_led = drv_get_bmc_led,
    .set_bmc_led = drv_set_bmc_led,
    .get_fan_led = drv_get_fan_led,
    .set_fan_led = drv_set_fan_led,
    .get_psu_led = drv_get_psu_led,
    .set_psu_led = drv_set_psu_led,
    .get_id_led = drv_get_id_led,
    .set_id_led = drv_set_id_led,
    .set_hw_control = drv_set_hw_control,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
    .debug_help = drv_debug_help,
};

static int drv_led_probe(struct platform_device *pdev)
{
    s3ip_led_drivers_register(&pfunc);

    return 0;
}

static int drv_led_remove(struct platform_device *pdev)
{
    s3ip_led_drivers_unregister();

    return 0;
}

static struct platform_driver drv_led_driver = {
    .driver = {
        .name   = DRVNAME,
        .owner = THIS_MODULE,
    },
    .probe      = drv_led_probe,
    .remove     = drv_led_remove,
};

static int __init drv_led_init(void)
{
    int err=0;
    int retval;

    drv_led_device = platform_device_alloc(DRVNAME, 0);
    if(!drv_led_device)
    {
        LED_DEBUG("%s(#%d): platform_device_alloc fail\n", __func__, __LINE__);
        return -ENOMEM;
    }

    retval = platform_device_add(drv_led_device);
    if(retval)
    {
        LED_DEBUG("%s(#%d): platform_device_add failed\n", __func__, __LINE__);
        err = -retval;
        goto dev_add_failed;
    }

    retval = platform_driver_register(&drv_led_driver);
    if(retval)
    {
        LED_DEBUG("%s(#%d): platform_driver_register failed(%d)\n", __func__, __LINE__, err);
        err = -retval;
        goto dev_reg_failed;
    }

    return 0;

dev_reg_failed:
    platform_device_unregister(drv_led_device);
    return err;

dev_add_failed:
    platform_device_put(drv_led_device);
    return err;
}

static void __exit drv_led_exit(void)
{
    platform_driver_unregister(&drv_led_driver);
    platform_device_unregister(drv_led_device);

    return;
}

MODULE_DESCRIPTION("S3IP LED Driver");
MODULE_VERSION(SWITCH_LED_DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_init(drv_led_init);
module_exit(drv_led_exit);