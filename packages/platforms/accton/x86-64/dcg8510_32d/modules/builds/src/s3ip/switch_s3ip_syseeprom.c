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

#include "switch_mb_driver.h"

#define ERROR_DEBUG(...) printk(KERN_ALERT __VA_ARGS__)

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

#define SWITCH_S3IP_SYSEEPROM_VERSION "0.0.0.1"

unsigned int loglevel = 0;

struct syseeprom_attribute{
    struct attribute attr;
    ssize_t (*show)(struct kobject *obj, struct syseeprom_attribute *attr, char *buf);
    ssize_t (*store)(struct kobject *obj, struct syseeprom_attribute *attr, const char *buf, size_t count);
};

struct mainboard_drivers_t *cb_func = NULL;

/* For s3ip */
extern struct kobject *switch_kobj;
static struct kobject *syseeprom_kobj;

enum syseeprom_attrs {
    EEPROM,
    BSP_VERSION,
    MGMT_MAC,  
    DATE,    
    NAME,      
    SN,        
    VENDOR,
    DEBUG,
    LOGLEVEL,
    NUM_SYSEEPROM_ATTR,
};

static ssize_t s3ip_get_eeprom(struct kobject *kobj, struct syseeprom_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        return -EIO;
    }
    
    retval = cb_func->get_syseeprom(buf);
    if(retval < 0)
    {
        return -EIO;
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_bsp_version(struct kobject *kobj, struct syseeprom_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }
    
    retval = cb_func->get_odm_bsp_version(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_mgmt_mac(struct kobject *kobj, struct syseeprom_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_mgmt_mac(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_date(struct kobject *kobj, struct syseeprom_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_date(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_name(struct kobject *kobj, struct syseeprom_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_name(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_sn(struct kobject *kobj, struct syseeprom_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_sn(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_switch_vendor(struct kobject *kobj, struct syseeprom_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_vendor(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_debug_help(struct kobject *kobj, struct syseeprom_attribute *attr, char *buf)
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

static ssize_t s3ip_debug(struct kobject *kobj, struct syseeprom_attribute *attr, const char *buf, size_t count)
{
    return count;
}

static ssize_t s3ip_get_loglevel(struct kobject *kobj, struct syseeprom_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", loglevel);
#else
    return sprintf(buf, "%d\n", loglevel);
#endif
}

static ssize_t s3ip_set_loglevel(struct kobject *kobj, struct syseeprom_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    long lev;
    
    if(cb_func == NULL)
        return -1;
    
    retval = kstrtol(buf, 0, &lev);
    if(retval == 0)
    {
        MAINBOARD_DEBUG("lev:%ld \n", lev);
    }
    else
    {
        MAINBOARD_DEBUG("Error:%d, lev:%s \n", retval, buf);
        return -1;
    }
    
    loglevel = (unsigned int)lev;
    
    cb_func->set_loglevel(lev);
    
    return count;
}

static struct syseeprom_attribute syseeprom_attr[NUM_SYSEEPROM_ATTR] = {
    [EEPROM]                    = {{.name = "eeprom",               .mode = S_IRUGO},               s3ip_get_eeprom,         NULL},
    [BSP_VERSION]               = {{.name = "bsp_version",          .mode = S_IRUGO},               s3ip_get_bsp_version,    NULL},
    [MGMT_MAC]                  = {{.name = "base_mac",             .mode = S_IRUGO},               s3ip_get_mgmt_mac,       NULL},
    [DATE]                      = {{.name = "date",                 .mode = S_IRUGO},               s3ip_get_date,           NULL},
    [NAME]                      = {{.name = "name",                 .mode = S_IRUGO},               s3ip_get_name,           NULL},
    [SN]                        = {{.name = "sn",                   .mode = S_IRUGO},               s3ip_get_sn,             NULL},
    [VENDOR]                    = {{.name = "vendor",               .mode = S_IRUGO},               s3ip_get_switch_vendor,  NULL},
    [DEBUG]                     = {{.name = "debug",                .mode = S_IRUGO | S_IWUSR},     s3ip_debug_help,         s3ip_debug},
    [LOGLEVEL]                  = {{.name = "loglevel",             .mode = S_IRUGO | S_IWUSR},     s3ip_get_loglevel,       s3ip_set_loglevel},
};


void s3ip_mainboard_drivers_register(struct mainboard_drivers_t *pfunc)
{    
    cb_func = pfunc;
    
    return;
}
EXPORT_SYMBOL_GPL(s3ip_mainboard_drivers_register);

void s3ip_mainboard_drivers_unregister(void)
{
    cb_func = NULL;
    
    return;
}
EXPORT_SYMBOL_GPL(s3ip_mainboard_drivers_unregister);

static int __init switch_syseeprom_init(void)
{
    int retval = 0;
    int err=0;
    int i;

    /* For s3ip */
    syseeprom_kobj = kobject_create_and_add("syseeprom", switch_kobj);
    if(!syseeprom_kobj)
    {
        MAINBOARD_DEBUG( "[%s]Failed to create 'syseeprom'\n", __func__);
        return -ENOMEM;
    }

    for(i=0; i < NUM_SYSEEPROM_ATTR; i++)
    {
        MAINBOARD_DEBUG( "[%s]sysfs_create_file /syseeprom/%s\n", __func__, syseeprom_attr[i].attr.name);
        retval = sysfs_create_file(syseeprom_kobj, &syseeprom_attr[i].attr);
        if(retval)
        {
            printk(KERN_ERR "Failed to create file '%s'\n", syseeprom_attr[i].attr.name);
            err = -retval;
            goto sysfs_create_s3ip_syseeprom_attr_failed;
        }
    }

    return 0;

sysfs_create_s3ip_syseeprom_attr_failed:
    if(syseeprom_kobj)
    {
        kobject_put(syseeprom_kobj);
        syseeprom_kobj = NULL;
    }

    return err;
}

static void __exit switch_syseeprom_exit(void)
{
    int i;

    /* For s3ip */
    for(i=0; i < NUM_SYSEEPROM_ATTR; i++)
        sysfs_remove_file(syseeprom_kobj, &syseeprom_attr[i].attr);

    if(syseeprom_kobj)
    {
        kobject_put(syseeprom_kobj);
        syseeprom_kobj = NULL;
    }
    
    cb_func = NULL;

    return;
}

MODULE_DESCRIPTION("Switch S3IP SYSEEPROM Driver");
MODULE_VERSION(SWITCH_S3IP_SYSEEPROM_VERSION);
MODULE_LICENSE("GPL");

module_init(switch_syseeprom_init);
module_exit(switch_syseeprom_exit);
