#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>

#include "pmbus.h"
#include "switch_bmc_driver.h"

#define SWITCH_S3IP_BMC_VERSION "0.0.0.1"

unsigned int loglevel = 0;
struct bmc_drivers_t *cb_func = NULL;

struct bmc_attribute{
    struct attribute attr;
    ssize_t (*show)(struct kobject *obj, struct bmc_attribute *attr, char *buf);
    ssize_t (*store)(struct kobject *obj, struct bmc_attribute *attr, const char *buf, size_t count);
};

/* For s3ip */
extern struct kobject *switch_kobj;
static struct kobject *bmc_kobj;

enum bmc_attrs {
    LOGLEVEL,
    STATUS,
    NUM_BMC_ATTR,
};

static ssize_t s3ip_get_loglevel(struct kobject *kobj, struct bmc_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", loglevel);
#else
    return sprintf(buf, "%d\n", loglevel);
#endif
}

static ssize_t s3ip_set_loglevel(struct kobject *kobj, struct bmc_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    long lev;

    if(cb_func == NULL)
        return -1;

    retval = kstrtol(buf, 0, &lev);
    if(retval == 0)
    {
        BMC_DEBUG("lev:%ld \n", lev);
    }
    else
    {
        BMC_DEBUG("Error:%d, lev:%s \n", retval, buf);
        return -1;
    }

    loglevel = (unsigned int)lev;

    cb_func->set_loglevel(lev);

    return count;
}

static ssize_t s3ip_get_status(struct kobject *kobj, struct bmc_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_status(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static struct bmc_attribute bmc_attr[NUM_BMC_ATTR] = {
    [LOGLEVEL]              = {{.name = "loglevel",            .mode = S_IRUGO | S_IWUSR},     s3ip_get_loglevel,        s3ip_set_loglevel},
    [STATUS]                = {{.name = "status",              .mode = S_IRUGO},               s3ip_get_status,          NULL},
};

void s3ip_bmc_drivers_register(struct bmc_drivers_t *pfunc)
{
    cb_func = pfunc;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_bmc_drivers_register);

void s3ip_bmc_drivers_unregister(void)
{
    cb_func = NULL;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_bmc_drivers_unregister);

static int __init switch_bmc_init(void)
{
    int err=0;
    int retval=0;
    int i;

    /* For s3ip */
    bmc_kobj = kobject_create_and_add("bmc", switch_kobj);
    if(!bmc_kobj)
    {
        BMC_DEBUG( "Failed to create 'bmc'\n");
        return -ENOMEM;
    }

    for(i = 0; i < NUM_BMC_ATTR ; i++)
    {
        BMC_DEBUG( "sysfs_create_file /bmc/%s\n", bmc_attr[i].attr.name);
        retval = sysfs_create_file(bmc_kobj, &bmc_attr[i].attr);
        if(retval)
        {
            printk(KERN_ERR "Failed to create file '%s'\n", bmc_attr[i].attr.name);
            err = -retval;
            goto sysfs_create_s3ip_bmc_attr_failed;
        }
    }

    return 0;

sysfs_create_s3ip_bmc_attr_failed:
    for(i = 0; i < NUM_BMC_ATTR ; i++)
        sysfs_remove_file(bmc_kobj, &bmc_attr[i].attr);

    if(bmc_kobj)
    {
        kobject_put(bmc_kobj);
        bmc_kobj = NULL;
    }

    return err;
}

static void __exit switch_bmc_exit(void)
{
    int i;

    /* For s3ip */
    for(i = 0; i < NUM_BMC_ATTR ; i++)
        sysfs_remove_file(bmc_kobj, &bmc_attr[i].attr);

    if(bmc_kobj)
    {
        kobject_put(bmc_kobj);
        bmc_kobj = NULL;
    }

    cb_func = NULL;

    return;
}

MODULE_DESCRIPTION("S3IP Switch S3IP BMC Driver");
MODULE_VERSION(SWITCH_S3IP_BMC_VERSION);
MODULE_LICENSE("GPL");

module_init(switch_bmc_init);
module_exit(switch_bmc_exit);
