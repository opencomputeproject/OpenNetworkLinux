#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>

#include "switch_cpld_driver.h"

#define SWITCH_S3IP_CPLD_VERSION "0.0.0.1"

unsigned int loglevel = 0;

struct cpld_attribute{
    struct attribute attr;
    ssize_t (*show)(struct kobject *obj, struct cpld_attribute *attr, char *buf);
    ssize_t (*store)(struct kobject *obj, struct cpld_attribute *attr, const char *buf, size_t count);
};

struct cpld_drivers_t *cb_func = NULL;

/* For s3ip */
extern struct kobject *switch_kobj;
static struct kobject *cpld_kobj;
static struct kobject *cpld_index_kobj[CPLD_TOTAL_NUM];

static struct kobject *cpld_fmea_kobj[CPLD_TOTAL_NUM][5];

static char *cpld_fmea_kobj_name[CPLD_TOTAL_NUM][5] = {
    {"battery", "clock", "corrosion", "voltage"},
    {"battery", "clock", "corrosion", "voltage"},
    {"battery", "clock", "corrosion", "voltage"},
    {"battery", "clock", "corrosion", "voltage"}
};

static bool cpld_is_support_reset[CPLD_TOTAL_NUM] = {
    true, true, true, true
};

static bool cpld_is_support_interrupt[CPLD_TOTAL_NUM] = {
    true, true, true, true
};

enum cpld_attrs {
    DEBUG,
    LOGLEVEL,
    REBOOT_CAUSE,
    NUM,
    ALIAS,
    TYPE,
    HW_VERSION,
    BOARD_VERSION,
    REG_TEST,
    NUM_CPLD_ATTR,
};

enum cpld_fmea_attrs {
    RESET,
    INTERRUPT,
    STATUS,
    NUM_CPLD_FMEA_ATTR,
};

int get_cpld_index(struct kobject *kobj)
{
    int retval=0;
    unsigned int cpld_index;
    char cpld_index_str[2] = {0};
#ifdef C11_ANNEX_K
    if(memcpy_s(cpld_index_str, 2, (kobject_name(kobj)+4), 1) != 0)
	{
        return -ENOMEM;
    }
#else
    memcpy(cpld_index_str, (kobject_name(kobj)+4), 1);
#endif
    retval = kstrtoint(cpld_index_str, 10, &cpld_index);
    if(retval == 0)
    {
        CPLD_DEBUG("cpld_index:%d \n", cpld_index);
    }
    else
    {
        CPLD_DEBUG("Error:%d, cpld_index:%s \n", retval, cpld_index_str);
        return -1;
    }

    return cpld_index;
}

int get_cpld_index_from_fmea_kobj(struct kobject *kobj)
{
    int retval=0;
    unsigned int cpld_index;
    char cpld_index_str[2] = {0};
#ifdef C11_ANNEX_K
    if(memcpy_s(cpld_index_str, 2, (kobject_name(kobj->parent)+4), 1) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(cpld_index_str, (kobject_name(kobj->parent)+4), 1);
#endif

    retval = kstrtoint(cpld_index_str, 10, &cpld_index);
    if(retval == 0)
    {
        CPLD_DEBUG("cpld_index:%d \n", cpld_index);
    }
    else
    {
        CPLD_DEBUG("Error:%d, cpld_index:%s \n", retval, cpld_index_str);
        return -1;
    }

    return cpld_index;
}

static ssize_t s3ip_debug_help(struct kobject *kobj, struct cpld_attribute *attr, char *buf)
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

static ssize_t s3ip_debug(struct kobject *kobj, struct cpld_attribute *attr, const char *buf, size_t count)
{   
    return count;
}

static ssize_t s3ip_get_loglevel(struct kobject *kobj, struct cpld_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", loglevel);
#else
    return sprintf(buf, "%d\n", loglevel);
#endif
}

static ssize_t s3ip_set_loglevel(struct kobject *kobj, struct cpld_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    long lev;

    if(cb_func == NULL)
        return -1;

    retval = kstrtol(buf, 0, &lev);
    if(retval == 0)
    {
        CPLD_DEBUG("lev:%ld \n", lev);
    }
    else
    {
        CPLD_DEBUG("Error:%d, lev:%s \n", retval, buf);
        return -1;
    }

    loglevel = (unsigned int)lev;

    cb_func->set_loglevel(lev);

    return count;
}

static ssize_t s3ip_get_reboot_cause(struct kobject *kobj, struct cpld_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_reboot_cause(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_num(struct kobject *kobj, struct cpld_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", CPLD_TOTAL_NUM);
#else
    return sprintf(buf, "%d\n", CPLD_TOTAL_NUM);
#endif
}

static ssize_t s3ip_get_alias(struct kobject *kobj, struct cpld_attribute *attr, char *buf)
{
    int retval=0;
    int cpld_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    cpld_index = get_cpld_index(kobj);
    if(cpld_index < 0)
    {
        CPLD_DEBUG("Get cpld index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_alias(cpld_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_type(struct kobject *kobj, struct cpld_attribute *attr, char *buf)
{
    int retval=0;
    int cpld_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    cpld_index = get_cpld_index(kobj);
    if(cpld_index < 0)
    {
        CPLD_DEBUG("Get cpld index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_type(cpld_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_hw_version(struct kobject *kobj, struct cpld_attribute *attr, char *buf)
{
    int retval=0;
    int cpld_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    cpld_index = get_cpld_index(kobj);
    if(cpld_index < 0)
    {
        CPLD_DEBUG("Get cpld index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_hw_version(cpld_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_board_version(struct kobject *kobj, struct cpld_attribute *attr, char *buf)
{
    int retval=0;
    int cpld_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    cpld_index = get_cpld_index(kobj);
    if(cpld_index < 0)
    {
        CPLD_DEBUG("Get cpld index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_board_version(cpld_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_reg_test(struct kobject *kobj, struct cpld_attribute *attr, char *buf)
{
    int retval=0;
    int cpld_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    cpld_index = get_cpld_index(kobj);
    if(cpld_index < 0)
    {
        CPLD_DEBUG("Get cpld index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_fmea_selftest_status(cpld_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_set_reg_test(struct kobject *kobj, struct cpld_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    int cpld_index;
    int input_value;
    unsigned char data;

    if((cb_func == NULL)||(cb_func->set_reg_test == NULL))
    {
        return -EIO;
    }

    cpld_index = get_cpld_index(kobj);
    if(cpld_index < 0)
    {
        CPLD_DEBUG("Get cpld index failed.\n");
        return -EINVAL;
    }

    retval = kstrtoint(buf, 0, &input_value);
    if(retval == 0)
    {
        data = input_value;
        CPLD_DEBUG("cpld%d reg_test data:%d \n", cpld_index, data);
    }
    else
    {
        CPLD_DEBUG("Error:%d, cpld%d reg_test data:%s\n", retval, cpld_index, buf);
        return -EIO;
    }

    retval = cb_func->set_reg_test(cpld_index, data);
    if(retval < 0)
    {
        CPLD_DEBUG("Set cpld%d reg_test failed.\n", cpld_index);
        return -EIO;
    }

    return count;
}

static ssize_t s3ip_get_fmea_status(struct kobject *kobj, struct cpld_attribute *attr, char *buf)
{
    int len=0;
    int cpld_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    cpld_index = get_cpld_index_from_fmea_kobj(kobj);
    if(cpld_index < 0)
    {
        CPLD_DEBUG("Get cpld index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    if(!strcmp(kobject_name(kobj), "corrosion"))
    {
        len = cb_func->get_fmea_corrosion_status(cpld_index-1, buf);
    }
    else if(!strcmp(kobject_name(kobj), "voltage"))
    {
        len = cb_func->get_fmea_voltage_status(cpld_index-1, buf);
    }
    else if(!strcmp(kobject_name(kobj), "clock"))
    {
        len = cb_func->get_fmea_clock_status(cpld_index-1, buf);
    }
    else if(!strcmp(kobject_name(kobj), "battery"))
    {
        len = cb_func->get_fmea_battery_status(cpld_index-1, buf);
    }
    else;

    return len;
}

static ssize_t s3ip_get_fmea_reset(struct kobject *kobj, struct cpld_attribute *attr, char *buf)
{
    int cpld_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    cpld_index = get_cpld_index(kobj);
    if(cpld_index < 0)
    {
        CPLD_DEBUG("Get cpld index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    return cb_func->get_fmea_reset(cpld_index-1, buf);
}

static ssize_t s3ip_get_fmea_interrupt(struct kobject *kobj, struct cpld_attribute *attr, char *buf)
{
    int cpld_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    cpld_index = get_cpld_index(kobj);
    if(cpld_index < 0)
    {
        CPLD_DEBUG("Get cpld index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    return cb_func->get_fmea_interrupt(cpld_index-1, buf);
}

static ssize_t s3ip_set_fmea_reset(struct kobject *kobj, struct cpld_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    int reset;
    int cpld_index;

    if(cb_func == NULL)
    {
        return -ENXIO;
    }

    retval = kstrtoint(buf, 10, &reset);
    if(retval == 0)
    {
        CPLD_DEBUG("reset:%d \n", reset);
    }
    else
    {
        CPLD_DEBUG("Error:%d, reset:%s \n", retval, buf);
        return -ENXIO;
    }

    cpld_index = get_cpld_index(kobj);
    if(cpld_index < 0)
    {
        CPLD_DEBUG("Get cpld index failed.\n");
        return -ENXIO;
    }

    retval = cb_func->set_fmea_reset(cpld_index, reset);
    if(retval == false)
    {
        CPLD_DEBUG("Set CPLD reset failed.\n");
        return -ENXIO;
    }

    return count;
}

static struct cpld_attribute cpld_attr[NUM_CPLD_ATTR] = {
    [DEBUG]                 = {{.name = "debug",                .mode = S_IRUGO | S_IWUSR},     s3ip_debug_help,             s3ip_debug},
    [LOGLEVEL]              = {{.name = "loglevel",             .mode = S_IRUGO | S_IWUSR},     s3ip_get_loglevel,           s3ip_set_loglevel},
    [REBOOT_CAUSE]          = {{.name = "reboot_cause",         .mode = S_IRUGO},               s3ip_get_reboot_cause,       NULL},
    [NUM]                   = {{.name = "num",                  .mode = S_IRUGO},               s3ip_get_num,                NULL},
    [ALIAS]                 = {{.name = "alias",                .mode = S_IRUGO},               s3ip_get_alias,              NULL},
    [TYPE]                  = {{.name = "type",                 .mode = S_IRUGO},               s3ip_get_type,               NULL},
    [HW_VERSION]            = {{.name = "firmware_version",     .mode = S_IRUGO},               s3ip_get_hw_version,         NULL},
    [BOARD_VERSION]         = {{.name = "board_version",        .mode = S_IRUGO},               s3ip_get_board_version,      NULL},
    [REG_TEST]              = {{.name = "reg_test",             .mode = S_IRUGO | S_IWUSR},     s3ip_get_reg_test,           s3ip_set_reg_test},
};

static struct cpld_attribute cpld_fmea_attr[NUM_CPLD_FMEA_ATTR] = {
    [RESET]                 = {{.name = "reset",                .mode = S_IRUGO | S_IWUSR},     s3ip_get_fmea_reset,         s3ip_set_fmea_reset},
    [INTERRUPT]             = {{.name = "interrupt",            .mode = S_IRUGO},               s3ip_get_fmea_interrupt,     NULL},
    [STATUS]                = {{.name = "status",               .mode = S_IRUGO},               s3ip_get_fmea_status,        NULL},
};

void s3ip_cpld_drivers_register(struct cpld_drivers_t *pfunc)
{
    cb_func = pfunc;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_cpld_drivers_register);

void s3ip_cpld_drivers_unregister(void)
{
    cb_func = NULL;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_cpld_drivers_unregister);

static int __init switch_cpld_init(void)
{
    int err=0;
    int retval=0;
    int i;
    int cpld_index;
    char *cpld_index_str;
    int fmea_kobj_num;
    int fmea_kobj_index;

    cpld_index_str = (char *)kzalloc(7*sizeof(char), GFP_KERNEL);
    if(!cpld_index_str)
    {
        CPLD_DEBUG( "Fail to alloc cpld_index_str memory\n");
        return -ENOMEM;
    }

    /* For s3ip */
    cpld_kobj = kobject_create_and_add("cpld", switch_kobj);
    if(!cpld_kobj)
    {
        CPLD_DEBUG( "Failed to create 'cpld'\n");
        err = -ENOMEM;
        goto sysfs_create_kobject_cpld_failed; 
    }

    for(i = DEBUG; i <= NUM; i++)
    {
        CPLD_DEBUG( "sysfs_create_file /cpld/%s\n", cpld_attr[i].attr.name);
        retval = sysfs_create_file(cpld_kobj, &cpld_attr[i].attr);
        if(retval)
        {
            printk(KERN_ERR "Failed to create file '%s'\n", cpld_attr[i].attr.name);
            err = -retval;
            goto sysfs_create_s3ip_attr_failed;
        }
    }

    for(cpld_index = 0; cpld_index < CPLD_TOTAL_NUM; cpld_index++)
    {
#ifdef C11_ANNEX_K
        if(sprintf_s(cpld_index_str, 7, "cpld%d", cpld_index+1) < 0)
#else
        if(sprintf(cpld_index_str, "cpld%d", cpld_index+1) < 0)
#endif
        {
            err = -ENOMEM;
            goto sysfs_create_kobject_switch_cpld_index_failed;
        }
        cpld_index_kobj[cpld_index] = kobject_create_and_add(cpld_index_str, cpld_kobj);
        if(!cpld_index_kobj[cpld_index])
        {
            CPLD_DEBUG( "Failed to create 'cpld%d'\n", cpld_index+1);
            err = -ENOMEM;
            goto sysfs_create_kobject_switch_cpld_index_failed;
        }

        for(i = ALIAS; i < NUM_CPLD_ATTR; i++)
        {
            CPLD_DEBUG( "sysfs_create_file /cpld/cpld%d/%s\n", cpld_index+1, cpld_attr[i].attr.name);
            retval = sysfs_create_file(cpld_index_kobj[cpld_index], &cpld_attr[i].attr);
            if(retval)
            {
                printk(KERN_ERR "Failed to create file '%s'\n", cpld_attr[i].attr.name);
                err = -retval;
                goto sysfs_create_s3ip_cpld_attr_failed;
            }
        }

        fmea_kobj_num = sizeof(cpld_fmea_kobj_name[cpld_index])/sizeof(cpld_fmea_kobj_name[cpld_index][0]);
        for(fmea_kobj_index = 0; fmea_kobj_index < fmea_kobj_num; fmea_kobj_index++)
        {
            if(cpld_fmea_kobj_name[cpld_index][fmea_kobj_index] == NULL)
                continue;

            cpld_fmea_kobj[cpld_index][fmea_kobj_index] = kobject_create_and_add(cpld_fmea_kobj_name[cpld_index][fmea_kobj_index], cpld_index_kobj[cpld_index]);
            if(!cpld_fmea_kobj[cpld_index][fmea_kobj_index])
            {
                CPLD_DEBUG( "Failed to create '%s'\n", cpld_fmea_kobj_name[cpld_index][fmea_kobj_index]);
                err = -ENOMEM;
                goto sysfs_create_s3ip_cpld_fmea_kobj_failed;
            }

            for(i = STATUS; i < NUM_CPLD_FMEA_ATTR; i++)
            {
                CPLD_DEBUG( "sysfs_create_file /cpld/cpld%d/%s/%s\n", cpld_index, cpld_fmea_kobj_name[cpld_index][fmea_kobj_index], cpld_fmea_attr[i].attr.name);
                retval = sysfs_create_file(cpld_fmea_kobj[cpld_index][fmea_kobj_index], &cpld_fmea_attr[i].attr);
                if(retval)
                {
                    printk(KERN_ERR "Failed to create file '%s'\n", cpld_fmea_attr[i].attr.name);
                    err = -retval;
                    goto sysfs_create_s3ip_cpld_fmea_attr_failed;
                }
            }
        }

        if(cpld_is_support_reset[cpld_index])
        {
            for(i = RESET; i < INTERRUPT; i++)
            {
                CPLD_DEBUG( "sysfs_create_file /cpld/cpld%d/%s\n", cpld_index, cpld_fmea_attr[i].attr.name);
                retval = sysfs_create_file(cpld_index_kobj[cpld_index], &cpld_fmea_attr[i].attr);
                if(retval)
                {
                    printk(KERN_ERR "Failed to create file '%s'\n", cpld_fmea_attr[i].attr.name);
                    err = -retval;
                    goto sysfs_create_s3ip_fmea_attr_reset_failed;
                }
            }
        }

        if(cpld_is_support_interrupt[cpld_index])
        {
            for(i = INTERRUPT; i < STATUS; i++)
            {
                CPLD_DEBUG( "sysfs_create_file /cpld/cpld%d/%s\n", cpld_index, cpld_fmea_attr[i].attr.name);
                retval = sysfs_create_file(cpld_index_kobj[cpld_index], &cpld_fmea_attr[i].attr);
                if(retval)
                {
                    printk(KERN_ERR "Failed to create file '%s'\n", cpld_fmea_attr[i].attr.name);
                    err = -retval;
                    goto sysfs_create_s3ip_fmea_attr_reset_failed;
                }
            }
        }
    }

    kfree(cpld_index_str);

    return 0;

sysfs_create_s3ip_fmea_attr_reset_failed:
sysfs_create_s3ip_cpld_fmea_attr_failed:
sysfs_create_s3ip_cpld_fmea_kobj_failed:
sysfs_create_s3ip_cpld_attr_failed:
sysfs_create_kobject_switch_cpld_index_failed:
    for(i = DEBUG; i <= NUM; i++)
        sysfs_remove_file(cpld_kobj, &cpld_attr[i].attr);

    for(cpld_index = 0; cpld_index < CPLD_TOTAL_NUM; cpld_index++)
    {
        if(cpld_index_kobj[cpld_index])
        {
            for(i = ALIAS; i < NUM_CPLD_ATTR; i++)
                sysfs_remove_file(cpld_index_kobj[cpld_index], &cpld_attr[i].attr);

            fmea_kobj_num = sizeof(cpld_fmea_kobj_name[cpld_index])/sizeof(cpld_fmea_kobj_name[cpld_index][0]);
            for(fmea_kobj_index = 0; fmea_kobj_index < fmea_kobj_num; fmea_kobj_index++)
            {
                if(cpld_fmea_kobj[cpld_index][fmea_kobj_index])
                {
                    for(i = STATUS; i < NUM_CPLD_FMEA_ATTR; i++)
                        sysfs_remove_file(cpld_fmea_kobj[cpld_index][fmea_kobj_index], &cpld_fmea_attr[i].attr);

                    kobject_put(cpld_fmea_kobj[cpld_index][fmea_kobj_index]);
                    cpld_fmea_kobj[cpld_index][fmea_kobj_index] = NULL;
                }
            }

            if(cpld_is_support_reset[cpld_index])
            {
                for(i = RESET; i < INTERRUPT; i++)
                    sysfs_remove_file(cpld_index_kobj[cpld_index], &cpld_fmea_attr[i].attr);
            }

            if(cpld_is_support_interrupt[cpld_index])
            {
                for(i = INTERRUPT; i < STATUS; i++)
                    sysfs_remove_file(cpld_index_kobj[cpld_index], &cpld_fmea_attr[i].attr);
            }

            kobject_put(cpld_index_kobj[cpld_index]);
            cpld_index_kobj[cpld_index] = NULL;
        }
    }

sysfs_create_s3ip_attr_failed:
    if(cpld_kobj)
    {
        kobject_put(cpld_kobj);
        cpld_kobj = NULL;
    }

sysfs_create_kobject_cpld_failed:
    kfree(cpld_index_str);

    return err;
}

static void __exit switch_cpld_exit(void)
{
    int i;
    int cpld_index;
    int fmea_kobj_num;
    int fmea_kobj_index;

    /* For s3ip */
    for(i = DEBUG; i <= NUM; i++)
        sysfs_remove_file(cpld_kobj, &cpld_attr[i].attr);

    for(cpld_index = 0; cpld_index < CPLD_TOTAL_NUM; cpld_index++)
    {
        if(cpld_index_kobj[cpld_index])
        {
            for(i = ALIAS; i < NUM_CPLD_ATTR; i++)
                sysfs_remove_file(cpld_index_kobj[cpld_index], &cpld_attr[i].attr);

            fmea_kobj_num = sizeof(cpld_fmea_kobj_name[cpld_index])/sizeof(cpld_fmea_kobj_name[cpld_index][0]);
            for(fmea_kobj_index = 0; fmea_kobj_index < fmea_kobj_num; fmea_kobj_index++)
            {
                if(cpld_fmea_kobj[cpld_index][fmea_kobj_index])
                {
                    for(i = STATUS; i < NUM_CPLD_FMEA_ATTR; i++)
                        sysfs_remove_file(cpld_fmea_kobj[cpld_index][fmea_kobj_index], &cpld_fmea_attr[i].attr);

                    kobject_put(cpld_fmea_kobj[cpld_index][fmea_kobj_index]);
                    cpld_fmea_kobj[cpld_index][fmea_kobj_index] = NULL;
                }
            }

            if(cpld_is_support_reset[cpld_index])
            {
                for(i = RESET; i < INTERRUPT; i++)
                    sysfs_remove_file(cpld_index_kobj[cpld_index], &cpld_fmea_attr[i].attr);
            }

            if(cpld_is_support_interrupt[cpld_index])
            {
                for(i = INTERRUPT; i < STATUS; i++)
                    sysfs_remove_file(cpld_index_kobj[cpld_index], &cpld_fmea_attr[i].attr);
            }

            kobject_put(cpld_index_kobj[cpld_index]);
            cpld_index_kobj[cpld_index] = NULL;
        }
    }

    if(cpld_kobj)
    {
        kobject_put(cpld_kobj);
        cpld_kobj = NULL;
    }

    cb_func = NULL;

    return;
}

MODULE_DESCRIPTION("Switch S3IP CPLD Driver");
MODULE_VERSION(SWITCH_S3IP_CPLD_VERSION);
MODULE_LICENSE("GPL");

module_init(switch_cpld_init);
module_exit(switch_cpld_exit);
