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

#define ERROR_DEBUG(...) printk(KERN_ALERT __VA_ARGS__)

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

#define SWITCH_S3IP_SWITCH_VERSION "0.0.0.1"

/* For s3ip */
static struct kobject *switch_kobj;
EXPORT_SYMBOL_GPL(switch_kobj);

static int __init switch_switch_init(void)
{
    /* For s3ip */
    switch_kobj = kobject_create_and_add("switch", NULL);
    if(!switch_kobj)
    {
        ERROR_DEBUG( "[%s]Failed to create 'switch'\n", __func__);
        return -ENOMEM;
    }

    return 0;
}

static void __exit switch_switch_exit(void)
{
    if(switch_kobj)
    {
        kobject_put(switch_kobj);
        switch_kobj = NULL;
    }

    return;
}

MODULE_DESCRIPTION("Switch S3IP Switch Entry Driver");
MODULE_VERSION(SWITCH_S3IP_SWITCH_VERSION);
MODULE_LICENSE("GPL");

module_init(switch_switch_init);
module_exit(switch_switch_exit);
