#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
 
#include "pmbus.h"
#include "switch_avs_driver.h"

#define SWITCH_S3IP_AVS_VERSION "0.0.0.1"

unsigned int loglevel = 0;
struct avs_drivers_t *cb_func = NULL;

struct avs_attribute{
    struct attribute attr;
    ssize_t (*show)(struct kobject *obj, struct avs_attribute *attr, char *buf);
    ssize_t (*store)(struct kobject *obj, struct avs_attribute *attr, const char *buf, size_t count);
};

/* For s3ip */
extern struct kobject *switch_kobj;
static struct kobject *avs_kobj;
static struct kobject *avs_index_kobj[TOTAL_FMEA_AVS_NUM];

enum avs_attrs {
    LOGLEVEL,
    WORK_STATUS,
    CURRENT_STATUS,
    PMBUS_STATUS,
    NUM_AVS_ATTR,
};

int get_avs_index(struct kobject *kobj)
{
    int retval=0;
    unsigned int avs_index;
    char avs_index_str[2] = {0};
#ifdef C11_ANNEX_K
    if(memcpy_s(avs_index_str, 2, (kobject_name(kobj)+3), 1) != 0)
	{
        return -ENOMEM;
    }
#else
	memcpy(avs_index_str, (kobject_name(kobj)+3), 1);
#endif
    retval = kstrtoint(avs_index_str, 10, &avs_index);
    if(retval == 0)
    {
        AVS_DEBUG("avs_index:%d \n", avs_index);
    }
    else
    {
        AVS_DEBUG("Error:%d, avs_index:%s \n", retval, avs_index_str);
        return -1;
    }

    return avs_index;
}

static ssize_t s3ip_get_loglevel(struct kobject *kobj, struct avs_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", loglevel);
#else
    return sprintf(buf, "%d\n", loglevel);
#endif
}

static ssize_t s3ip_set_loglevel(struct kobject *kobj, struct avs_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    long lev;

    if(cb_func == NULL)
        return -1;

    retval = kstrtol(buf, 0, &lev);
    if(retval == 0)
    {
        AVS_DEBUG("lev:%ld \n", lev);
    }
    else
    {
        AVS_DEBUG("Error:%d, lev:%s \n", retval, buf);
        return -1;
    }

    loglevel = (unsigned int)lev;

    cb_func->set_loglevel(lev);

    return count;
}

static ssize_t s3ip_fmea_get_work_status(struct kobject *kobj, struct avs_attribute *attr, char *buf)
{
    int retval=0;
    int avs_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    avs_index = get_avs_index(kobj);
    if(avs_index < 0)
    {
        AVS_DEBUG("Get avs index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->fmea_get_work_status(avs_index, buf, "s3ip");
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_fmea_get_current_status(struct kobject *kobj, struct avs_attribute *attr, char *buf)
{
    int retval=0;
    int avs_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    avs_index = get_avs_index(kobj);
    if(avs_index < 0)
    {
        AVS_DEBUG("Get avs index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->fmea_get_current_status(avs_index, buf, "s3ip");
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_fmea_get_pmbus_status(struct kobject *kobj, struct avs_attribute *attr, char *buf)
{
    int retval=0;
    int avs_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    avs_index = get_avs_index(kobj);
    if(avs_index < 0)
    {
        AVS_DEBUG("Get avs index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->fmea_get_pmbus_status(avs_index, buf, "s3ip");
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static struct avs_attribute avs_attr[NUM_AVS_ATTR] = {
    [LOGLEVEL]              = {{.name = "loglevel",            .mode = S_IRUGO | S_IWUSR},     s3ip_get_loglevel,              s3ip_set_loglevel},
    [WORK_STATUS]           = {{.name = "work_status",         .mode = S_IRUGO},               s3ip_fmea_get_work_status,      NULL},
    [CURRENT_STATUS]        = {{.name = "current_status",      .mode = S_IRUGO},               s3ip_fmea_get_current_status,   NULL},
    [PMBUS_STATUS]          = {{.name = "pmbus_status",        .mode = S_IRUGO},               s3ip_fmea_get_pmbus_status,     NULL},
};

void s3ip_avs_drivers_register(struct avs_drivers_t *pfunc)
{
    cb_func = pfunc;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_avs_drivers_register);

void s3ip_avs_drivers_unregister(void)
{
    cb_func = NULL;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_avs_drivers_unregister);

static int __init switch_avs_init(void)
{
    int err=0;
    int retval=0;
    int i;
    int avs_index;
    char *avs_index_str;

    avs_index_str = (char *)kzalloc(3*sizeof(char), GFP_KERNEL);
    if (!avs_index_str)
    {
        AVS_DEBUG( "Fail to alloc avs_index_str memory\n");
        return -ENOMEM;
    }

    /* For s3ip */
    avs_kobj = kobject_create_and_add("avs", switch_kobj);
    if(!avs_kobj)
    {
        AVS_DEBUG( "Failed to create 'avs'\n");
        err = -ENOMEM;
        goto sysfs_create_kobject_avs_failed;
    }

    for(i = 0; i < WORK_STATUS ; i++)
    {
        AVS_DEBUG( "sysfs_create_file /avs/%s\n", avs_attr[i].attr.name);
        retval = sysfs_create_file(avs_kobj, &avs_attr[i].attr);
        if(retval)
        {
            printk(KERN_ERR "Failed to create file '%s'\n", avs_attr[i].attr.name);
            err = -retval;
            goto sysfs_create_s3ip_attr_loglevel_failed;
        }
    }

    for(avs_index = 0; avs_index < TOTAL_FMEA_AVS_NUM; avs_index++)
    {
#ifdef C11_ANNEX_K
        if(sprintf_s(avs_index_str, 8, "avs%d", avs_index) < 0)
#else
        if(sprintf(avs_index_str, "avs%d", avs_index) < 0)
#endif
        {
            err = -ENOMEM;
            goto sysfs_create_kobject_avs_index_failed;
        }
        avs_index_kobj[avs_index] = kobject_create_and_add(avs_index_str, avs_kobj);
        if(!avs_index_kobj[avs_index])
        {
            AVS_DEBUG( "Failed to create %s\n", avs_index_str);
            err =  -ENOMEM;
            goto sysfs_create_kobject_avs_index_failed;
        }

        for(i = WORK_STATUS; i < NUM_AVS_ATTR ; i++)
        {
            AVS_DEBUG( "sysfs_create_file /avs/%s\n", avs_attr[i].attr.name);
            retval = sysfs_create_file(avs_index_kobj[avs_index], &avs_attr[i].attr);
            if(retval)
            {
                printk(KERN_ERR "Failed to create file '%s'\n", avs_attr[i].attr.name);
                err = -retval;
                goto sysfs_create_s3ip_attr_failed;
            }
        }
    }

    kfree(avs_index_str);

    return 0;

sysfs_create_s3ip_attr_failed:
sysfs_create_kobject_avs_index_failed:
    for(avs_index = 0; avs_index < TOTAL_FMEA_AVS_NUM; avs_index++)
    {
        if(avs_index_kobj[avs_index])
        {
            for(i = WORK_STATUS; i < NUM_AVS_ATTR ; i++)
                sysfs_remove_file(avs_index_kobj[avs_index], &avs_attr[i].attr);

            kobject_put(avs_index_kobj[avs_index]);
            avs_index_kobj[avs_index] = NULL;
        }
    }

    for(i = 0; i < WORK_STATUS ; i++)
        sysfs_remove_file(avs_kobj, &avs_attr[i].attr);

sysfs_create_s3ip_attr_loglevel_failed:
    if(avs_kobj)
    {
        kobject_put(avs_kobj);
        avs_kobj = NULL;
    }

sysfs_create_kobject_avs_failed:
    kfree(avs_index_str);

    return err;
}

static void __exit switch_avs_exit(void)
{
    int i;
    int avs_index;

    /* For s3ip */
    for(avs_index = 0; avs_index < TOTAL_FMEA_AVS_NUM; avs_index++)
    {
        if(avs_index_kobj[avs_index])
        {
            for(i = WORK_STATUS; i < NUM_AVS_ATTR ; i++)
                sysfs_remove_file(avs_index_kobj[avs_index], &avs_attr[i].attr);

            kobject_put(avs_index_kobj[avs_index]);
            avs_index_kobj[avs_index] = NULL;
        }
    }

    for(i = 0; i < WORK_STATUS ; i++)
        sysfs_remove_file(avs_kobj, &avs_attr[i].attr);

    if(avs_kobj)
    {
        kobject_put(avs_kobj);
        avs_kobj = NULL;
    }

    cb_func = NULL;

    return;
}

MODULE_DESCRIPTION("S3IP Switch S3IP AVS Driver");
MODULE_VERSION(SWITCH_S3IP_AVS_VERSION);
MODULE_LICENSE("GPL");

module_init(switch_avs_init);
module_exit(switch_avs_exit);
