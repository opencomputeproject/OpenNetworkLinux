#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>

#include "switch_fpga_driver.h"

#define ERROR_DEBUG(...) printk(KERN_ALERT __VA_ARGS__)

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

#define SWITCH_S3IP_FPGA_VERSION "0.0.0.1"

unsigned int loglevel = 0;

struct fpga_attribute{
    struct attribute attr;
    ssize_t (*show)(struct kobject *obj, struct fpga_attribute *attr, char *buf);
    ssize_t (*store)(struct kobject *obj, struct fpga_attribute *attr, const char *buf, size_t count);
};

struct fpga_drivers_t *cb_func = NULL;

/* For s3ip */
extern struct kobject *switch_kobj;
static struct kobject *fpga_kobj;
static struct kobject *fpga_index_kobj[FPGA_TOTAL_NUM];

enum fpga_attrs {
    DEBUG,
    LOGLEVEL,
    NUM,
    ALIAS,
    TYPE,
    HW_VERSION,
    BOARD_VERSION,
    REG_TEST,
    NUM_FPGA_ATTR,
};

int get_fpga_index(struct kobject *kobj)
{
    int retval=0;
    unsigned int fpga_index;
    char fpga_index_str[2] = {0};
#ifdef C11_ANNEX_K
    if(memcpy_s(fpga_index_str, 2, (kobject_name(kobj)+4), 1) != 0)
	{
        return -ENOMEM;
    }
#else
    memcpy(fpga_index_str, (kobject_name(kobj)+4), 1);
#endif
    retval = kstrtoint(fpga_index_str, 10, &fpga_index);
    if(retval == 0)
    {
        FPGA_DEBUG("fpag_index:%d \n", fpga_index);
    }
    else
    {
        FPGA_DEBUG("Error:%d, fpga_index:%s \n", retval, fpga_index_str);
        return -1;
    }

    return fpga_index;
}


static ssize_t s3ip_debug_help(struct kobject *kobj, struct fpga_attribute *attr, char *buf)
{
    int retval=0;

    if((cb_func == NULL)||(cb_func->debug_help == NULL))
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

static ssize_t s3ip_debug(struct kobject *kobj, struct fpga_attribute *attr, const char *buf, size_t count)
{
    return count;
}

static ssize_t s3ip_get_loglevel(struct kobject *kobj, struct fpga_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", loglevel);
#else
    return sprintf(buf, "%d\n", loglevel);
#endif
}

static ssize_t s3ip_set_loglevel(struct kobject *kobj, struct fpga_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    long lev;

    if((cb_func == NULL)||(cb_func->set_loglevel == NULL))
        return -1;

    retval = kstrtol(buf, 0, &lev);
    if(retval == 0)
    {
        FPGA_DEBUG("lev:%ld \n", lev);
    }
    else
    {
        FPGA_DEBUG("Error:%d, lev:%s \n", retval, buf);
        return -1;
    }

    loglevel = (unsigned int)lev;

    cb_func->set_loglevel(lev);

    return count;
}

static ssize_t s3ip_get_num(struct kobject *kobj, struct fpga_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", FPGA_TOTAL_NUM);
#else
    return sprintf(buf, "%d\n", FPGA_TOTAL_NUM);
#endif
}

static ssize_t s3ip_get_alias(struct kobject *kobj, struct fpga_attribute *attr, char *buf)
{
    int retval=0;
    int fpga_index;

    if((cb_func == NULL)||(cb_func->get_alias == NULL))
    {
        ERROR_RETURN_NA(-1);
    }

    fpga_index = get_fpga_index(kobj);
    if(fpga_index < 0)
    {
        FPGA_DEBUG("Get fpga index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_alias(fpga_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_type(struct kobject *kobj, struct fpga_attribute *attr, char *buf)
{
    int retval=0;
    int fpga_index;

    if((cb_func == NULL)||(cb_func->get_type == NULL))
    {
        ERROR_RETURN_NA(-1);
    }

    fpga_index = get_fpga_index(kobj);
    if(fpga_index < 0)
    {
        FPGA_DEBUG("Get fpga index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_type(fpga_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_hw_version(struct kobject *kobj, struct fpga_attribute *attr, char *buf)
{
    int retval=0;
    int fpga_index;

    if((cb_func == NULL)||(cb_func->get_hw_version == NULL))
    {
        ERROR_RETURN_NA(-1);
    }

    fpga_index = get_fpga_index(kobj);
    if(fpga_index < 0)
    {
        FPGA_DEBUG("Get fpga index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_hw_version(fpga_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_board_version(struct kobject *kobj, struct fpga_attribute *attr, char *buf)
{
    int retval=0;
    int fpga_index;

    if((cb_func == NULL)||(cb_func->get_board_version == NULL))
    {
        ERROR_RETURN_NA(-1);
    }

    fpga_index = get_fpga_index(kobj);
    if(fpga_index < 0)
    {
        FPGA_DEBUG("Get fpga index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_board_version(fpga_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_reg_test(struct kobject *kobj, struct fpga_attribute *attr, char *buf)
{
    int retval=0;
    int fpga_index;

    if((cb_func == NULL)||(cb_func->get_reg_test == NULL))
    {
        ERROR_RETURN_NA(-1);
    }

    fpga_index = get_fpga_index(kobj);
    if(fpga_index < 0)
    {
        FPGA_DEBUG("Get fpga index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_reg_test(fpga_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_set_reg_test(struct kobject *kobj, struct fpga_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    int fpga_index;
    long data;

    if((cb_func == NULL)||(cb_func->set_reg_test == NULL))
    {
        return -EIO;
    }

    fpga_index = get_fpga_index(kobj);
    if(fpga_index < 0)
    {
        FPGA_DEBUG("Get fpga index failed.\n");
        return -EINVAL;
    }

    retval = kstrtol(buf, 0, &data);
    if(retval == 0)
    {
        FPGA_DEBUG("fpga%d reg_test data:%ld \n", fpga_index, data);
    }
    else
    {
        FPGA_DEBUG("Error:%d, fpga%d reg_test data:%s\n", retval, fpga_index, buf);
        return -EIO;
    }

    retval = cb_func->set_reg_test(fpga_index, data);
    if(retval < 0)
    {
        FPGA_DEBUG("Set fpga%d reg_test failed.\n", fpga_index);
        return -EIO;
    }

    return count;
}

static struct fpga_attribute fpga_attr[NUM_FPGA_ATTR] = {
    [DEBUG]                 = {{.name = "debug",                .mode = S_IRUGO | S_IWUSR},     s3ip_debug_help,             s3ip_debug},
    [LOGLEVEL]              = {{.name = "loglevel",             .mode = S_IRUGO | S_IWUSR},     s3ip_get_loglevel,           s3ip_set_loglevel},
    [NUM]                   = {{.name = "num",                  .mode = S_IRUGO},               s3ip_get_num,                NULL},
    [ALIAS]                 = {{.name = "alias",                .mode = S_IRUGO},               s3ip_get_alias,              NULL},
    [TYPE]                  = {{.name = "type",                 .mode = S_IRUGO},               s3ip_get_type,               NULL},
    [HW_VERSION]            = {{.name = "firmware_version",     .mode = S_IRUGO},               s3ip_get_hw_version,         NULL},
    [BOARD_VERSION]         = {{.name = "board_version",        .mode = S_IRUGO},               s3ip_get_board_version,      NULL},
    [REG_TEST]              = {{.name = "reg_test",             .mode = S_IRUGO | S_IWUSR},     s3ip_get_reg_test,           s3ip_set_reg_test},
};



void s3ip_fpga_drivers_register(struct fpga_drivers_t *pfunc)
{
    cb_func = pfunc;
    return;
}
EXPORT_SYMBOL_GPL(s3ip_fpga_drivers_register);

void s3ip_fpga_drivers_unregister(void)
{
    cb_func = NULL;
    return;
}
EXPORT_SYMBOL_GPL(s3ip_fpga_drivers_unregister);

static int __init switch_fpga_init(void)
{
    int fpga_index;
    char *fpga_index_str;
    int retval = 0;
    int err=0;
    int i;

    fpga_index_str = (char *)kzalloc(7*sizeof(char), GFP_KERNEL);
    if(!fpga_index_str)
    {
        FPGA_DEBUG("Fail to alloc fpga_index_str memory\n");
        return -ENOMEM;
    }

    /* For s3ip */
    fpga_kobj = kobject_create_and_add("fpga", switch_kobj);
    if(!fpga_kobj)
    {
        FPGA_DEBUG("Failed to create 'fpga_kobj'\n");
        err = -ENOMEM;
        goto sysfs_create_kobject_fpga_failed;
    }

    for(i = DEBUG; i <= NUM; i++)
    {
        FPGA_DEBUG("sysfs_create_file /fpga/%s\n", fpga_attr[i].attr.name);
        retval = sysfs_create_file(fpga_kobj, &fpga_attr[i].attr);
        if(retval)
        {
            printk(KERN_ERR "Failed to create file '%s'\n", fpga_attr[i].attr.name);
            err = -retval;
            goto sysfs_create_s3ip_fpga_attr_failed;
        }
    }

    for(fpga_index = 0; fpga_index < FPGA_TOTAL_NUM; fpga_index++)
    {
#ifdef C11_ANNEX_K
        if(sprintf_s(fpga_index_str, 7, "fpga%d", fpga_index+1) < 0)
#else
        if(sprintf(fpga_index_str, "fpga%d", fpga_index+1) < 0)
#endif
        {
            err = -ENOMEM;
            goto sysfs_create_kobject_fpga_index_failed;
        }

        fpga_index_kobj[fpga_index] = kobject_create_and_add(fpga_index_str, fpga_kobj);
        if(!fpga_index_kobj[fpga_index])
        {
            FPGA_DEBUG("Failed to create 'fpga%d'\n", fpga_index+1);
            err = -ENOMEM;
            goto sysfs_create_kobject_fpga_index_failed;
        }

        for(i = ALIAS; i < NUM_FPGA_ATTR; i++)
        {
            FPGA_DEBUG( "sysfs_create_file /fpga/fpga%d/%s\n", fpga_index+1, fpga_attr[i].attr.name);
            retval = sysfs_create_file(fpga_index_kobj[fpga_index], &fpga_attr[i].attr);
            if(retval)
            {
                printk(KERN_ERR "Failed to create file '%s'\n", fpga_attr[i].attr.name);
                err = -retval;
                goto sysfs_create_s3ip_fpga_index_attr_failed;
            }
        }
    }

    return 0;

sysfs_create_s3ip_fpga_index_attr_failed:
sysfs_create_kobject_fpga_index_failed:
    for(fpga_index = 0; fpga_index < FPGA_TOTAL_NUM; fpga_index++)
    {
        if(fpga_index_kobj[fpga_index])
        {
            for(i = ALIAS; i < NUM_FPGA_ATTR; i++)
                sysfs_remove_file(fpga_index_kobj[fpga_index], &fpga_attr[i].attr);

            kobject_put(fpga_index_kobj[fpga_index]);
            fpga_index_kobj[fpga_index] = NULL;
        }
    }

sysfs_create_s3ip_fpga_attr_failed:
    for(i = DEBUG; i <= NUM; i++)
        sysfs_remove_file(fpga_kobj, &fpga_attr[i].attr);

    kobject_put(fpga_kobj);
    fpga_kobj = NULL;

sysfs_create_kobject_fpga_failed:
    kfree(fpga_index_str);

    return err;
}

static void __exit switch_fpga_exit(void)
{
    int i;
    int fpga_index;

    /* For s3ip */
    for(i = DEBUG; i <= NUM; i++)
        sysfs_remove_file(fpga_kobj, &fpga_attr[i].attr);

    for(fpga_index = 0; fpga_index < FPGA_TOTAL_NUM; fpga_index++)
    {
        if(fpga_index_kobj[fpga_index])
        {
            for(i = ALIAS; i < NUM_FPGA_ATTR; i++)
                sysfs_remove_file(fpga_index_kobj[fpga_index], &fpga_attr[i].attr);

            kobject_put(fpga_index_kobj[fpga_index]);
            fpga_index_kobj[fpga_index] = NULL;
        }
    }

    if(fpga_kobj)
    {
        kobject_put(fpga_kobj);
        fpga_kobj = NULL;
    }

    cb_func = NULL;

    return;
}

MODULE_DESCRIPTION("Switch S3IP fpga Driver");
MODULE_VERSION(SWITCH_S3IP_FPGA_VERSION);
MODULE_LICENSE("GPL");

module_init(switch_fpga_init);
module_exit(switch_fpga_exit);
