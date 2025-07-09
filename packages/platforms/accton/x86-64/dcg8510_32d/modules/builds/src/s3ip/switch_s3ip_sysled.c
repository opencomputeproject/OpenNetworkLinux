#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>

#include "switch_led_driver.h"

#define SWITCH_S3IP_SYSLED_VERSION "0.0.0.1"

unsigned int loglevel = 0;

enum led_status {
    LED_ALL_OFF,
    LED_GREEN_ON,
    LED_YELLOW_ON,
    LED_RED_ON,
    LED_BLUE_ON,
    LED_GREEN_FLASH,
    LED_YELLOW_FLASH,
    LED_RED_FLASH,
    LED_BLUE_FLASH
};

struct led_attribute{
    struct attribute attr;
    ssize_t (*show)(struct kobject *obj, struct led_attribute *attr, char *buf);
    ssize_t (*store)(struct kobject *obj, struct led_attribute *attr, const char *buf, size_t count);
};

struct led_drivers_t *cb_func = NULL;

/* For s3ip */
extern struct kobject *switch_kobj;
static struct kobject *sysled_kobj;

enum led_attrs {
    DEBUG,
    LOGLEVEL,
    SYS_LED_STATUS,
    BMC_LED_STATUS,
    FAN_LED_STATUS,
    PSU_LED_STATUS,
    ID_LED_STATUS,
    NUM_LED_ATTR,
};

static ssize_t s3ip_debug_help(struct kobject *kobj, struct led_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->debug_help(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_debug(struct kobject *kobj, struct led_attribute *attr, const char *buf, size_t count)
{
    return count;
}

static ssize_t s3ip_get_loglevel(struct kobject *kobj, struct led_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", loglevel);
#else
    return sprintf(buf, "%d\n", loglevel);
#endif
}

static ssize_t s3ip_set_loglevel(struct kobject *kobj, struct led_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    long lev;

    if(cb_func == NULL)
        return -1;

    retval = kstrtol(buf, 0, &lev);
    if(retval == 0)
    {
        LED_DEBUG("[%s] lev:%ld \n", __func__, lev);
    }
    else
    {
        LED_DEBUG("[%s] Error:%d, lev:%s \n", __func__, retval, buf);
        return -1;
    }

    loglevel = (unsigned int)lev;

    cb_func->set_loglevel(lev);

    return count;
}

static ssize_t s3ip_get_sys_led(struct kobject *kobj, struct led_attribute *attr, char *buf)
{
    int retval=0;
    unsigned char led;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-ENXIO);
    }

    retval = cb_func->get_sys_led(&led);
    if(retval == false)
    {
        LED_DEBUG("[%s] Get sys led failed.\n", __func__);
        ERROR_RETURN_NA(-EPERM);
    }

    switch(led)
    {
        case SYS_LED_OFF:
            led = LED_ALL_OFF;
            break;
        case SYS_LED_GREEN_ON:
            led = LED_GREEN_ON;
            break;
        case SYS_LED_YELLOW_ON:
            led = LED_YELLOW_ON;
            break;
        case SYS_LED_RED_ON:
            led = LED_RED_ON;
            break;
        case SYS_LED_BLUE_ON:
            led = LED_BLUE_ON;
            break;
        case SYS_LED_GREEN_FLASH_COME_OK:
        case SYS_LED_GREEN_FLASH_BMC_OK:
            led = LED_GREEN_FLASH;
            break;
        case SYS_LED_YELLOW_FLASH_COME_OK:
        case SYS_LED_YELLOW_FLASH_BMC_OK:
            led = LED_YELLOW_FLASH;
            break;
        case SYS_LED_RED_FLASH:
            led = LED_RED_FLASH;
            break;
        case SYS_LED_BLUE_FLASH:
            led = LED_BLUE_FLASH;
            break;
        default:
            led = -EINVAL;
            ERROR_RETURN_NA(-EINVAL);
            break;
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", led);
#else
    return sprintf(buf, "%d\n", led);
#endif

}

ssize_t s3ip_set_sys_led(struct kobject *obj, struct led_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    unsigned int led;

    if(cb_func == NULL)
        return -EIO;

    retval = kstrtoint(buf, 10, &led);
    if(retval == 0)
    {
        LED_DEBUG("[%s] led:%d \n", __func__, led);
    }
    else
    {
        LED_DEBUG("[%s] Error:%d, led:%s \n", __func__, retval, buf);
        return -1;
    }

    switch(led)
    {
        case LED_ALL_OFF:
            retval = cb_func->set_sys_led(SYS_LED_OFF);
            break;
        case LED_GREEN_ON:
            retval = cb_func->set_sys_led(SYS_LED_GREEN_ON);
            break;
        case LED_YELLOW_ON:
            retval = cb_func->set_sys_led(SYS_LED_YELLOW_ON);
            break;
        case LED_RED_ON:
            retval = cb_func->set_sys_led(SYS_LED_RED_ON);
            break;
        case LED_BLUE_ON:
            retval = cb_func->set_sys_led(SYS_LED_BLUE_ON);
            break;
        case LED_GREEN_FLASH:
            retval = cb_func->set_sys_led(SYS_LED_GREEN_FLASH_COME_OK);
            break;
        case LED_YELLOW_FLASH:
            retval = cb_func->set_sys_led(SYS_LED_YELLOW_FLASH_COME_OK);
            break;
        case LED_RED_FLASH:
            retval = cb_func->set_sys_led(SYS_LED_RED_FLASH);
            break;
        case LED_BLUE_FLASH:
            retval = cb_func->set_sys_led(SYS_LED_BLUE_FLASH);
            break;
        default:
            return -EINVAL;
            break;
    }

    if(retval == false)
    {
        LED_DEBUG("[%s] Set sys led failed.\n", __func__);
        return -EPERM;
    }

    return count;
}
EXPORT_SYMBOL_GPL(s3ip_set_sys_led);

static ssize_t s3ip_get_bmc_led(struct kobject *kobj, struct led_attribute *attr, char *buf)
{
    int retval=0;
    unsigned char led;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-ENXIO);
    }

    retval = cb_func->get_bmc_led(&led);
    if(retval == false)
    {
        LED_DEBUG("[%s] Get bmc led failed.\n", __func__);
        ERROR_RETURN_NA(-EPERM);
    }

    switch(led)
    {
        case BMC_LED_OFF:
            led = LED_ALL_OFF;
            break;
        case BMC_LED_GREEN_ON:
            led = LED_GREEN_ON;
            break;
        case BMC_LED_YELLOW_ON:
            led = LED_YELLOW_ON;
            break;
        case BMC_LED_RED_ON:
            led = LED_RED_ON;
            break;
        case BMC_LED_BLUE_ON:
            led = LED_BLUE_ON;
            break;
        case BMC_LED_GREEN_FLASH:
            led = LED_GREEN_FLASH;
            break;
        case BMC_LED_YELLOW_FLASH:
            led = LED_YELLOW_FLASH;
            break;
        case BMC_LED_RED_FLASH:
            led = LED_RED_FLASH;
            break;
        case BMC_LED_BLUE_FLASH:
            led = LED_BLUE_FLASH;
            break;
        default:
            led = -EINVAL;
            ERROR_RETURN_NA(-EINVAL);
            break;
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", led);
#else
    return sprintf(buf, "%d\n", led);
#endif

}

ssize_t s3ip_set_bmc_led(struct kobject *obj, struct led_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    unsigned int led;

    if(cb_func == NULL)
        return -EIO;

    retval = kstrtoint(buf, 10, &led);
    if(retval == 0)
    {
        LED_DEBUG("[%s] led:%d \n", __func__, led);
    }
    else
    {
        LED_DEBUG("[%s] Error:%d, led:%s \n", __func__, retval, buf);
        return -1;
    }

    switch(led)
    {
        case LED_ALL_OFF:
            retval = cb_func->set_bmc_led(BMC_LED_OFF);
            break;
        case LED_GREEN_ON:
            retval = cb_func->set_bmc_led(BMC_LED_GREEN_ON);
            break;
        case LED_YELLOW_ON:
            retval = cb_func->set_bmc_led(BMC_LED_YELLOW_ON);
            break;
        case LED_RED_ON:
            retval = cb_func->set_bmc_led(BMC_LED_RED_ON);
            break;
        case LED_BLUE_ON:
            retval = cb_func->set_bmc_led(BMC_LED_BLUE_ON);
            break;
        case LED_GREEN_FLASH:
            retval = cb_func->set_bmc_led(BMC_LED_GREEN_FLASH);
            break;
        case LED_YELLOW_FLASH:
            retval = cb_func->set_bmc_led(BMC_LED_YELLOW_FLASH);
            break;
        case LED_RED_FLASH:
            retval = cb_func->set_bmc_led(BMC_LED_RED_FLASH);
            break;
        case LED_BLUE_FLASH:
            retval = cb_func->set_bmc_led(BMC_LED_BLUE_FLASH);
            break;
        default:
            return -EINVAL;
            break;
    }

    if(retval == false)
    {
        LED_DEBUG("[%s] Set sys led failed.\n", __func__);
        return -EPERM;
    }

    return count;
}
EXPORT_SYMBOL_GPL(s3ip_set_bmc_led);
static ssize_t s3ip_get_fan_led(struct kobject *kobj, struct led_attribute *attr, char *buf)
{
    int retval=0;
    unsigned char led;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-ENXIO);
    }

    retval = cb_func->get_fan_led(&led);
    if(retval == false)
    {
        LED_DEBUG("[%s] Get bmc led failed.\n", __func__);
        ERROR_RETURN_NA(-EPERM);
    }

    switch(led)
    {
        case FAN_LED_OFF:
            led = LED_ALL_OFF;
            break;
        case FAN_LED_GREEN_ON:
            led = LED_GREEN_ON;
            break;
        case FAN_LED_YELLOW_ON:
            led = LED_YELLOW_ON;
            break;
        case FAN_LED_RED_ON:
            led = LED_RED_ON;
            break;
        case FAN_LED_BLUE_ON:
            led = LED_BLUE_ON;
            break;
        default:
            led = -EINVAL;
            ERROR_RETURN_NA(-EINVAL);
            break;
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", led);
#else
    return sprintf(buf, "%d\n", led);
#endif

}

static ssize_t s3ip_get_psu_led(struct kobject *kobj, struct led_attribute *attr, char *buf)
{
    int retval=0;
    unsigned char led;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-ENXIO);
    }

    retval = cb_func->get_psu_led(&led);
    if(retval == false)
    {
        LED_DEBUG("[%s] Get bmc led failed.\n", __func__);
        ERROR_RETURN_NA(-EPERM);
    }

    switch(led)
    {
        case PSU_LED_OFF:
            led = LED_ALL_OFF;
            break;
        case PSU_LED_GREEN_ON:
            led = LED_GREEN_ON;
            break;
        case PSU_LED_YELLOW_ON:
            led = LED_YELLOW_ON;
            break;
        case PSU_LED_RED_ON:
            led = LED_RED_ON;
            break;
        case PSU_LED_BLUE_ON:
            led = LED_BLUE_ON;
            break;
        default:
            led = -EINVAL;
            ERROR_RETURN_NA(-EINVAL);
            break;
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", led);
#else
    return sprintf(buf, "%d\n", led);
#endif

}

static ssize_t s3ip_get_id_led(struct kobject *kobj, struct led_attribute *attr, char *buf)
{
    unsigned char led;
    led = -EINVAL;
    ERROR_RETURN_NA(-EINVAL);
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", led);
#else
    return sprintf(buf, "%d\n", led);
#endif

}

ssize_t s3ip_set_fan_led(struct kobject *obj, struct led_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    unsigned int led;

    if(cb_func == NULL)
    {
        return -ENXIO;
    }

    retval = kstrtoint(buf, 10, &led);
    if(retval == 0)
    {
        LED_DEBUG("[%s] fan_led:%d \n", __func__, led);
    }
    else
    {
        LED_DEBUG("[%s] Error:%d, fan_led:%s \n", __func__, retval, buf);
        return -1;
    }

    switch(led)
    {
        case LED_ALL_OFF:
            retval = cb_func->set_fan_led(FAN_LED_OFF);
            break;
        case LED_GREEN_ON:
            retval = cb_func->set_fan_led(FAN_LED_GREEN_ON);
            break;
        case LED_YELLOW_ON:
            retval = cb_func->set_fan_led(FAN_LED_YELLOW_ON);
            break;
        case LED_RED_ON:
            retval = cb_func->set_fan_led(FAN_LED_RED_ON);
            break;
        case LED_BLUE_ON:
            retval = cb_func->set_fan_led(FAN_LED_BLUE_ON);
            break;
        default:
            return -EINVAL;
            break;
    }

    if(retval == false)
    {
        LED_DEBUG("[%s] Set fan_led failed.\n", __func__);
        return -EPERM;
    }

    return count;
}
EXPORT_SYMBOL_GPL(s3ip_set_fan_led);

ssize_t s3ip_set_psu_led(struct kobject *obj, struct led_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    unsigned int led;

    if(cb_func == NULL)
    {
        return -EIO;
    }

    retval = kstrtoint(buf, 10, &led);
    if(retval == 0)
    {
        LED_DEBUG("[%s] psu_led:%d \n", __func__, led);
    }
    else
    {
        LED_DEBUG("[%s] Error:%d, psu_led:%s \n", __func__, retval, buf);
        return -1;
    }

    switch(led)
    {
        case LED_ALL_OFF:
            retval = cb_func->set_psu_led(PSU_LED_OFF);
            break;
        case LED_GREEN_ON:
            retval = cb_func->set_psu_led(PSU_LED_GREEN_ON);
            break;
        case LED_YELLOW_ON:
            retval = cb_func->set_psu_led(PSU_LED_YELLOW_ON);
            break;
        case LED_RED_ON:
            retval = cb_func->set_psu_led(PSU_LED_RED_ON);
            break;
        case LED_BLUE_ON:
            retval = cb_func->set_psu_led(PSU_LED_BLUE_ON);
            break;
        default:
            return -EINVAL;
            break;
    }

    if(retval == false)
    {
        LED_DEBUG("[%s] Set psu_led failed.\n", __func__);
        return -EPERM;
    }

    return count;
}
EXPORT_SYMBOL_GPL(s3ip_set_psu_led);

static ssize_t s3ip_set_id_led(struct kobject *obj, struct led_attribute *attr, const char *buf, size_t count)
{
    return -EPERM;
}


static struct led_attribute led_attr[NUM_LED_ATTR] = {
    [DEBUG]                 = {{.name = "debug",                        .mode = S_IRUGO | S_IWUSR},             s3ip_debug_help,             s3ip_debug},
    [LOGLEVEL]              = {{.name = "loglevel",                     .mode = S_IRUGO | S_IWUSR},             s3ip_get_loglevel,           s3ip_set_loglevel},
    [SYS_LED_STATUS]        = {{.name = "sys_led_status",               .mode = S_IRUGO | S_IWUSR},             s3ip_get_sys_led,            s3ip_set_sys_led},
    [BMC_LED_STATUS]        = {{.name = "bmc_led_status",               .mode = S_IRUGO | S_IWUSR},             s3ip_get_bmc_led,            s3ip_set_bmc_led},
    [FAN_LED_STATUS]        = {{.name = "fan_led_status",               .mode = S_IRUGO | S_IWUSR},             s3ip_get_fan_led,            s3ip_set_fan_led},
    [PSU_LED_STATUS]        = {{.name = "psu_led_status",               .mode = S_IRUGO | S_IWUSR},             s3ip_get_psu_led,            s3ip_set_psu_led},
    [ID_LED_STATUS]         = {{.name = "id_led_status",                .mode = S_IRUGO | S_IWUSR},             s3ip_get_id_led,             s3ip_set_id_led},
};

void s3ip_led_drivers_register(struct led_drivers_t *pfunc)
{
    cb_func = pfunc;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_led_drivers_register);

void s3ip_led_drivers_unregister(void)
{
    cb_func = NULL;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_led_drivers_unregister);

static int __init switch_sysled_init(void)
{
    int err=0;
    int retval=0;
    int i;

    /* For s3ip */
    sysled_kobj = kobject_create_and_add("sysled", switch_kobj);
    if(!sysled_kobj)
    {
        LED_DEBUG( "[%s]Failed to create 'sysled'\n", __func__);
        return -ENOMEM;
    }

    for(i=0; i < NUM_LED_ATTR; i++)
    {
        LED_DEBUG( "[%s]sysfs_create_file /sysled/%s\n", __func__, led_attr[i].attr.name);
        retval = sysfs_create_file(sysled_kobj, &led_attr[i].attr);
        if(retval)
        {
            printk(KERN_ERR "Failed to create file '%s'\n", led_attr[i].attr.name);
            err = -retval;
            goto sysfs_create_s3ip_attr_failed;
        }
    }

    return 0;

sysfs_create_s3ip_attr_failed:
    if(sysled_kobj)
    {
        kobject_put(sysled_kobj);
        sysled_kobj = NULL;
    }

    return err;
}

static void __exit switch_sysled_exit(void)
{
    int i;

    /* For s3ip */
    for(i=0; i < NUM_LED_ATTR; i++)
        sysfs_remove_file(sysled_kobj, &led_attr[i].attr);

    if(sysled_kobj)
    {
        kobject_put(sysled_kobj);
        sysled_kobj = NULL;
    }

    cb_func = NULL;

    return;
}

MODULE_DESCRIPTION("Switch S3IP SYSLED Driver");
MODULE_VERSION(SWITCH_S3IP_SYSLED_VERSION);
MODULE_LICENSE("GPL");

module_init(switch_sysled_init);
module_exit(switch_sysled_exit);