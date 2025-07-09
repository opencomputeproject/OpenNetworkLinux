#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/i2c.h>

#include "pmbus.h"
#include "switch_cpld_driver.h"
#include "switch_bmc_driver.h"

#define DRVNAME "drv_bmc_driver"
#define SWITCH_BMC_DRIVER_VERSION "0.0.1"

unsigned int loglevel = 0;
static struct platform_device *drv_bmc_device;

struct mutex     update_lock;

bool ipmi_bmc_exist(void)
{
    unsigned char val;
    switch_cpld_reg_read(0, SYSCPLD_REG_BMC_STATUS, &val);
    
    if(CHECK_BIT(val, 0) == 0)
    {
        return true; //BMC is present
    }
    else
    {
        return false; //BMC is not present
    }   
}
EXPORT_SYMBOL_GPL(ipmi_bmc_exist);

bool ipmi_bmc_work(void)
{
    unsigned char val;

    /* Offset 0x29 BMC_STATUS_REGISTER Bit[2] BMC_RUN, 0:Normal; 1:Abnormal */
    switch_cpld_reg_read(0, SYSCPLD_REG_BMC_STATUS, &val);

    if(CHECK_BIT(val, 2) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
EXPORT_SYMBOL_GPL(ipmi_bmc_work);

bool ipmi_bmc_is_ok(void)
{
    if(ipmi_bmc_exist() && ipmi_bmc_work())
    {
       return true;
    }
   else
    {
       return false;
    }
}
EXPORT_SYMBOL_GPL(ipmi_bmc_is_ok);


ssize_t drv_get_status(char *buf)
{
    if(ipmi_bmc_exist()==false)
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "0\n"); //BMC is not present
#else
	    return sprintf(buf, "0\n"); //BMC is not present
#endif
    }
    else if(ipmi_bmc_work()==true)
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "1\n");//BMC is normal
#else
	    return sprintf(buf, "1\n");//BMC is normal
#endif
    }
    else
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "2\n");//BMC is abnormal
#else
	    return sprintf(buf, "2\n");//BMC is abnormal
#endif
    return -1;
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

// For s3ip
static struct bmc_drivers_t pfunc = {
    .get_status = drv_get_status,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
};

static int drv_bmc_probe(struct platform_device *pdev)
{
    s3ip_bmc_drivers_register(&pfunc);

    return 0;
}

static int drv_bmc_remove(struct platform_device *pdev)
{
    s3ip_bmc_drivers_unregister();

    return 0;
}

static struct platform_driver drv_bmc_driver = {
    .driver = {
        .name   = DRVNAME,
        .owner = THIS_MODULE,
    },
    .probe      = drv_bmc_probe,
    .remove     = drv_bmc_remove,
};

static int __init drv_bmc_init(void)
{
    int err=0;
    int retval=0;

    drv_bmc_device = platform_device_alloc(DRVNAME, 0);
    if(!drv_bmc_device)
    {
        BMC_DEBUG("%s(#%d): platform_device_alloc fail\n", __func__, __LINE__);
        return -ENOMEM;
    }

    retval = platform_device_add(drv_bmc_device);
    if(retval)
    {
        BMC_DEBUG("%s(#%d): platform_device_add failed\n", __func__, __LINE__);
        err = -retval;
        goto dev_add_failed;
    }

    retval = platform_driver_register(&drv_bmc_driver);
    if(retval)
    {
        BMC_DEBUG("%s(#%d): platform_driver_register failed(%d)\n", __func__, __LINE__, err);
        err = -retval;
        goto dev_reg_failed;
    }

    mutex_init(&update_lock);

    return 0;

dev_reg_failed:
    platform_device_unregister(drv_bmc_device);
    return err;

dev_add_failed:
    platform_device_put(drv_bmc_device);
    return err;
}

static void __exit drv_bmc_exit(void)
{
    platform_driver_unregister(&drv_bmc_driver);
    platform_device_unregister(drv_bmc_device);

    mutex_destroy(&update_lock);

    return;
}

MODULE_DESCRIPTION("S3IP BMC Driver");
MODULE_VERSION(SWITCH_BMC_DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_init(drv_bmc_init);
module_exit(drv_bmc_exit);
