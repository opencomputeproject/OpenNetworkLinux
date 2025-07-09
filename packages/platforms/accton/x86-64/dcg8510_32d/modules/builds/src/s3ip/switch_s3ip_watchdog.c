#include <linux/module.h> /*
                            => module_init()
                            => module_exit()
                            => MODULE_LICENSE()
                            => MODULE_VERSION()
                            => MODULE_AUTHOR()
                            => struct module
                          */
#include <linux/init.h>  /*
                            => typedef int (*initcall_t)(void);
                                 Note: the 'initcall_t' function returns 0 when succeed.
                                       In the Linux kernel, error codes are negative numbers
                                       belonging to the set defined in <linux/errno.h>.
                            => typedef void (*exitcall_t)(void);
                            => __init
                            => __exit
                         */
#include <linux/moduleparam.h>  /*
                                  => moduleparam()
                                */
#include <linux/types.h>  /*
                             => dev_t  (u32)
                          */
#include <linux/kdev_t.h>  /*
                              => MAJOR()
                              => MINOR()
                           */
#include <linux/string.h>  /*
                              => void *memset()
                           */
#include <linux/slab.h>  /*
                            => void kfree()
                          */
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>/*
                                => struct spinlock
                           */
#include <linux/io.h>

#include <uapi/mtd/mtd-abi.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "switch_wdt_driver.h"

#define ERROR_DEBUG(...) printk(KERN_ALERT __VA_ARGS__)

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

#define SWITCH_S3IP_WATCHDOG_VERSION "0.0.0.1"

unsigned int loglevel = 0;

struct wdt_attribute{
    struct attribute attr;
    ssize_t (*show)(struct kobject *obj, struct wdt_attribute *attr, char *buf);
    ssize_t (*store)(struct kobject *obj, struct wdt_attribute *attr, const char *buf, size_t count);
};

struct wdt_drivers_t *cb_func = NULL;

/* For s3ip */
extern struct kobject *switch_kobj;
static struct kobject *watchdog_kobj;

enum wdt_attrs {
    DEBUG,
    LOGLEVEL,
    IDENTIFY,
    STATE,
    TIMELEFT,
    TIMEOUT,
    RESET,
    ENABLE,
    NUM_WDT_ATTR,
};

static ssize_t s3ip_debug_help(struct kobject *kobj, struct wdt_attribute *attr, char *buf)
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

static ssize_t s3ip_debug(struct kobject *kobj, struct wdt_attribute *attr, const char *buf, size_t count)
{
    return count;
}

static ssize_t s3ip_get_loglevel(struct kobject *kobj, struct wdt_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", loglevel);
#else
    return sprintf(buf, "%d\n", loglevel);
#endif
}

static ssize_t s3ip_set_loglevel(struct kobject *kobj, struct wdt_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    long lev;

    if(cb_func == NULL)
        return -1;

    retval = kstrtol(buf, 0, &lev);
    if(retval == 0)
    {
        WDT_DEBUG("lev:%ld \n", lev);
    }
    else
    {
        WDT_DEBUG("Error:%d, lev:%s \n", retval, buf);
        return -1;
    }

    loglevel = (unsigned int)lev;

    cb_func->set_loglevel(lev);

    return count;
}

static ssize_t s3ip_get_identify(struct kobject *kobj, struct wdt_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_identify(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_state(struct kobject *kobj, struct wdt_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_state(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_timeleft(struct kobject *kobj, struct wdt_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_timeleft(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_timeout(struct kobject *kobj, struct wdt_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_timeout(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_set_timeout(struct kobject *kobj, struct wdt_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    unsigned short timeout;

    if(cb_func == NULL)
        return -1;

    retval = kstrtou16(buf, 10, &timeout);
    if(retval == 0)
    {
        WDT_DEBUG("timeout:%d \n", timeout);
    }
    else
    {
        WDT_DEBUG("Error:%d, timeout:%s \n", retval, buf);
        return -1;
    }

    cb_func->set_timeout(timeout);

    return count;
}

static ssize_t s3ip_set_reset(struct kobject *kobj, struct wdt_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    unsigned int reset;

    if(cb_func == NULL)
        return -1;

    retval = kstrtouint(buf, 10, &reset);
    if(retval == 0)
    {
        WDT_DEBUG("reset:%d \n", reset);
    }
    else
    {
        WDT_DEBUG("Error:%d, reset:%s \n", retval, buf);
        return -1;
    }

    cb_func->set_reset(reset);

    return count;
}

static ssize_t s3ip_get_enable(struct kobject *kobj, struct wdt_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_enable(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_set_enable(struct kobject *kobj, struct wdt_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    unsigned int enable;

    if(cb_func == NULL)
        return -1;

    retval = kstrtouint(buf, 10, &enable);
    if(retval == 0)
    {
        WDT_DEBUG("enable:%d \n", enable);
    }
    else
    {
        WDT_DEBUG("Error:%d, enable:%s \n", retval, buf);
        return -1;
    }

    cb_func->set_enable(enable);

    return count;
}

static struct wdt_attribute wdt_attr[NUM_WDT_ATTR] = {
    [DEBUG]                     = {{.name = "debug",                .mode = S_IRUGO | S_IWUSR},         s3ip_debug_help,         s3ip_debug},
    [LOGLEVEL]                  = {{.name = "loglevel",             .mode = S_IRUGO | S_IWUSR},         s3ip_get_loglevel,       s3ip_set_loglevel},
    [IDENTIFY]                  = {{.name = "identify",             .mode = S_IRUGO},                   s3ip_get_identify,       NULL},
    [STATE]                     = {{.name = "state",                .mode = S_IRUGO},                   s3ip_get_state,          NULL},
    [TIMELEFT]                  = {{.name = "timeleft",             .mode = S_IRUGO},                   s3ip_get_timeleft,       NULL},
    [TIMEOUT]                   = {{.name = "timeout",              .mode = S_IRUGO | S_IWUSR},         s3ip_get_timeout,        s3ip_set_timeout},
    [RESET]                     = {{.name = "reset",                .mode = S_IWUSR},                   NULL,                    s3ip_set_reset},
    [ENABLE]                    = {{.name = "enable",               .mode = S_IRUGO | S_IWUSR},         s3ip_get_enable,         s3ip_set_enable},
};


void s3ip_wdt_drivers_register(struct wdt_drivers_t *pfunc)
{
    cb_func = pfunc;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_wdt_drivers_register);

void s3ip_wdt_drivers_unregister(void)
{
    cb_func = NULL;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_wdt_drivers_unregister);

static int __init switch_wdt_init(void)
{
    int retval = 0;
    int err=0;
    int i;

    /* For s3ip */
    watchdog_kobj = kobject_create_and_add("watchdog", switch_kobj);
    if(!watchdog_kobj)
    {
        WDT_DEBUG( "Failed to create 'watchdog'\n");
        return -ENOMEM;
    }

    for(i=0; i < NUM_WDT_ATTR; i++)
    {
        WDT_DEBUG( "sysfs_create_file /watchdog/%s\n", wdt_attr[i].attr.name);
        retval = sysfs_create_file(watchdog_kobj, &wdt_attr[i].attr);
        if(retval)
        {
            printk(KERN_ERR "Failed to create file '%s'\n", wdt_attr[i].attr.name);
            err = -retval;
            goto sysfs_create_s3ip_watchdog_attr_failed;
        }
    }

    return 0;

sysfs_create_s3ip_watchdog_attr_failed:
    if(watchdog_kobj)
    {
        kobject_put(watchdog_kobj);
        watchdog_kobj = NULL;
    }

    return err;
}

static void __exit switch_wdt_exit(void)
{
    int i;

    /* For s3ip */
    for(i=0; i < NUM_WDT_ATTR; i++)
        sysfs_remove_file(watchdog_kobj, &wdt_attr[i].attr);

    if(watchdog_kobj)
    {
        kobject_put(watchdog_kobj);
        watchdog_kobj = NULL;
    }

    cb_func = NULL;

    return;
}

MODULE_DESCRIPTION("Switch S3IP Watchdog Driver");
MODULE_VERSION(SWITCH_S3IP_WATCHDOG_VERSION);
MODULE_LICENSE("GPL");

module_init(switch_wdt_init);
module_exit(switch_wdt_exit);
