#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/slab.h>

#include "switch_system_fpga.h"
#include "switch_fpga_driver.h"

#define DRVNAME "drv_fpga_driver"
#define SWITCH_FPGA_DRIVER_VERSION "0.0.1"

unsigned int loglevel = 0;
//module_param(loglevel,int,S_IRUGO);

static struct platform_device *drv_fpga_device;


static char *fpga_alias_name[FPGA_TOTAL_NUM] = {
    "fpga"
};

static char *fpga_type_name[FPGA_TOTAL_NUM] = {
    "Xilinx" //Xilinx xc7a35t-2fgg484c
};

static int fpga_write(u8 device_id ,u8 reg, u32 value)
{
    return fpga_pcie_write( device_id, reg, value);
}

static int fpga_read(u8 device_id ,u8 reg)
{
    return fpga_pcie_read( device_id, reg);
}

ssize_t drv_get_alias(unsigned int index, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", fpga_alias_name[index-1]);
#else
    return sprintf(buf, "%s\n", fpga_alias_name[index-1]);
#endif
}

ssize_t drv_get_type(unsigned int index, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", fpga_type_name[index-1]);
#else
    return sprintf(buf, "%s\n", fpga_type_name[index-1]);
#endif

}

ssize_t drv_get_hw_version(unsigned int index, char *buf)
{
    unsigned int val;
    int len=0;
    val = fpga_read(index-1,FPGA_VER_OFFSET);
#ifdef C11_ANNEX_K
    len = sprintf_s(buf, PAGE_SIZE, "%x.%x\n", (val>>8)&0xff,val&0xff);
#else
    len = sprintf(buf, "%x.%x\n", (val>>8)&0xff,val&0xff);
#endif


    return len;
}

ssize_t drv_get_board_version(unsigned int index, char *buf)
{
    unsigned int val;
    int len=0;
    val = fpga_read(index-1,FPGA_VER_OFFSET);
#ifdef C11_ANNEX_K
    len = sprintf_s(buf, PAGE_SIZE, "%x\n", (val>>24)&0xff);
#else
    len = sprintf(buf, "%x\n", (val>>24)&0xff);
#endif


    return len;
}

ssize_t drv_get_reg_test(unsigned int index, char *buf)
{
    u32 val;
    val = fpga_read(index-1,FPGA_SCRATCHPAD_OFFSET);
#ifdef C11_ANNEX_K 
    return sprintf_s(buf, PAGE_SIZE, "0x%x\n", val);
#else
    return sprintf(buf, "0x%x\n", val);
#endif

}

ssize_t drv_set_reg_test(unsigned int index, unsigned int data)
{
    unsigned int val;
    val = fpga_write(index-1,FPGA_SCRATCHPAD_OFFSET,data);

    return val;
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
        "Use the following command to debug:\n"
        "busybox devmem 0xfbc00000 32\n");
#else
    return sprintf(buf, 
        "Use the following command to debug:\n"
        "busybox devmem 0xfbc00000 32\n");
#endif

}

ssize_t drv_debug(const char *buf, int count)
{
    return 0;
}


static struct fpga_drivers_t pfunc = {
    .get_alias = drv_get_alias,
    .get_type = drv_get_type,
    .get_hw_version = drv_get_hw_version,
    .get_board_version = drv_get_board_version,
    .get_reg_test = drv_get_reg_test,
    .set_reg_test = drv_set_reg_test,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
    .debug_help = drv_debug_help,
};

static int drv_fpga_probe(struct platform_device *pdev)
{
    s3ip_fpga_drivers_register(&pfunc);

    return 0;
}

static int drv_fpga_remove(struct platform_device *pdev)
{
    s3ip_fpga_drivers_unregister();

    return 0;
}

static struct platform_driver drv_fpga_driver = {
    .driver = {
        .name   = DRVNAME,
        .owner = THIS_MODULE,
    },
    .probe      = drv_fpga_probe,
    .remove     = drv_fpga_remove,
};

static int __init drv_fpga_init(void)
{
    int err=0;
    int retval=0;

    drv_fpga_device = platform_device_alloc(DRVNAME, 0);
    if(!drv_fpga_device)
    {
        FPGA_DEBUG("%s(#%d): platform_device_alloc fail\n", __func__, __LINE__);
        return -ENOMEM;
    }

    retval = platform_device_add(drv_fpga_device);
    if(retval)
    {
        FPGA_DEBUG("%s(#%d): platform_device_add failed\n", __func__, __LINE__);
        err = -retval;
        goto dev_add_failed;
    }

    retval = platform_driver_register(&drv_fpga_driver);
    if(retval)
    {
        FPGA_DEBUG("%s(#%d): platform_driver_register failed(%d)\n", __func__, __LINE__, err);
        err = -retval;
        goto dev_reg_failed;
    }

    return 0;

dev_reg_failed:
    platform_device_unregister(drv_fpga_device);

dev_add_failed:
    platform_device_put(drv_fpga_device);

    return err;
}

static void __exit drv_fpga_exit(void)
{
    platform_driver_unregister(&drv_fpga_driver);
    platform_device_unregister(drv_fpga_device);
    platform_device_put(drv_fpga_device);

    return;
}

MODULE_DESCRIPTION("S3IP FPGA Driver");
MODULE_VERSION(SWITCH_FPGA_DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_init(drv_fpga_init);
module_exit(drv_fpga_exit);
