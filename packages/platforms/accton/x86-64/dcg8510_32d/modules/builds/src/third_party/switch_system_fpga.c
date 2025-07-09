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
#include <linux/fs.h>  /*
                          => register_chrdev_region()
                          => unregister_chrdev_region()
                          => alloc_chrdev_region()
                          => struct file_operations
                          => struct file
                          => struct inode
                          => unsigned int imajor()
                          => unsigned int iminor()
                       */
#include <linux/cdev.h>  /*
                            => struct cdev
                         */
#include <linux/string.h>  /*
                              => void *memset()
                           */
#include <linux/slab.h>  /*
                            => void kfree()
                          */
#include <linux/device.h>  /*
                              => class_create()
                              => device_create()
                           */
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>  /*
                            => void cdev_init()
                         */
#include <linux/uaccess.h> /*
                              => access_ok()
                           */
#include <asm/current.h>  /*
                              => current
                          */
#include <linux/semaphore.h> /*
                                => struct semaphore
                             */
#include <linux/wait.h> /*
                           => struct wait_queue_head_t
                           => init_waitqueue_head()
                        */
#include <linux/fcntl.h> /*
                           => O_NONBLOCK
                         */
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/printk.h>

//simon: try to support 4.x kernel
#include <linux/version.h>

#define I2C_DEBUG(...)
/*#define I2C_DEBUG(...) printk(KERN_ALERT __VA_ARGS__)*/
#define ERROR_DEBUG(...) printk(KERN_ALERT __VA_ARGS__)

#ifndef ACCESS_ONCE
#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
#endif

#define FPGA_DRIVER_VERSION "0.0.f"

MODULE_LICENSE("GPL");
MODULE_VERSION(FPGA_DRIVER_VERSION);
MODULE_AUTHOR("Will Chen, Junhao He");

#define FPGA_DEV_NAME       "fpga"
#define FPGA_CLASS_NAME     "fpga_class"
#define FPGA_DRIVER_NAME    "fpgaio"
#define FPGA_REGION_NAME    "fpga_register"
#define FPGA_SMBUS_NAME     "Accton_FPGA_SMBus_"
#define BAR0                0

#if 1
/*Please copy the following definition into the c files if you want to use fpga_access*/
#define MAX_BUFF_SIZE   128
#define MAX_EEPROM_SIZE 256

/* file_operations: ioctl */
struct fpga_access {
    /* input */
    unsigned char   addr;       /* 0x0 - 0x7F */
    unsigned char   offset;
    unsigned char   length;
    unsigned char   channel;
    /* intput output buff */
    unsigned char   buff[MAX_BUFF_SIZE];
};

#define FPGA_MAGIC      0xA7
#define FPGA_IO_CMD     0x10
#define FPGA_SPI_IO     0x11
#define FPGA_LOCK_CMD   0xEE
#define FPGA_UNLOCK_CMD 0xDD

#define FPGA_READ       _IOR(FPGA_MAGIC, FPGA_IO_CMD,     struct fpga_access)
#define FPGA_WRITE      _IOW(FPGA_MAGIC, FPGA_IO_CMD,     struct fpga_access)
#define FPGA_LOCK       _IOW(FPGA_MAGIC, FPGA_LOCK_CMD,   struct fpga_access)
#define FPGA_UNLOCK     _IOW(FPGA_MAGIC, FPGA_UNLOCK_CMD, struct fpga_access)
#define FPGA_SPI_READ   _IOR(FPGA_MAGIC, FPGA_SPI_IO,     struct fpga_access)
#define FPGA_SPI_WRITE  _IOW(FPGA_MAGIC, FPGA_SPI_IO,     struct fpga_access)
/*Please copy the above definition into the c files if you want to use fpga_access*/
#endif

/* FPGA Bus control parameters */
static unsigned int max_retry_time = 20;
static unsigned int delay_ack = 50;
static unsigned int delay_tip = 26;
static unsigned int delay_busy = 10;
static unsigned int fpga_dev_count = 1;
static unsigned int disable_iic_access = 0;

struct spinlock     fpga_pcie_lock;

module_param(max_retry_time, uint, S_IRUGO);
MODULE_PARM_DESC(max_retry_time, "Max retry times to wait TIP/ACK (20 by default)");
module_param(delay_ack, uint, S_IRUGO);
MODULE_PARM_DESC(delay_ack, "Delay time before the next time to check ACK bit (50 us by default)");
module_param(delay_tip, uint, S_IRUGO);
MODULE_PARM_DESC(delay_tip, "Delay time before the next time to check TIP bit (26 us by default)");
module_param(delay_busy, uint, S_IRUGO);
MODULE_PARM_DESC(delay_busy, "Delay time before the next time to check BUSY bit (10 us by default)");
module_param(disable_iic_access, uint, 0644);
MODULE_PARM_DESC(disable_iic_access, "Disable i2c access when set to NOT 0.");


#define MAX_CHANNEL             35
#define FPGA_SLEEP(X)           usleep_range((X*8/10), X)
#define I2C_PRESCALE_LOW_OFFSET     0x0400  // PRESCALE registers, PRERlo
#define I2C_PRESCALE_HIGH_OFFSET    0x0404  // PRESCALE registers, PRERhi
#define I2C_CONTROLLER_OFFSET       0x0408  // Control Register, CTR
#define I2C_TRANSMIT_RECEIVE_OFFSET 0x040C  // Transmit Register, TXR
#define I2C_COMMAND_STATUS_OFFSET   0x0410  // Command Register CR/Status Registesr, SR

//#define PRER_LO_DEFAULT             0x007f
#define PRER_HI_DEFAULT             0x0000
#define CONTROL_DISABLE             0x0000
#define CONTROL_ENABLE              0x0080
#define COMMAND_START_WRITE         0x0090
#define COMMAND_ENABLE_WRITE        0x0010
#define COMMAND_READ                0x0020
#define COMMAND_ACK_STOP_READ       0x0068
#define COMMAND_STOP                0x0040
#define COMMAND_STOP_WRITE          0x0050
#define BUSY_BIT                    0x40
#define TIP_BIT                     0x2
#define ACK_BIT                     0x80
#define SET_TXR_WRITE(x)            (x &= ~(1UL))
#define SET_TXR_READ(x)             (x |= (1UL))
#define MODULE_ATTR_NUM 4
#define tBUF_TIME_NS                20000
#define tBUF_ACTUAL_TIME_OFFSET_NS  3000
#define tBUF_MAX_DELAY_US           20

static unsigned int PRER_LO_DEFAULT = 0x001f;

module_param(PRER_LO_DEFAULT, uint, S_IRUGO);
MODULE_PARM_DESC(PRER_LO_DEFAULT, "set i2c freq(default 0x001f)");


enum iic_access_type {
    IIC_READ_NO_ACK         = 0,
    IIC_WRITE_NO_ACK        = 1,
    IIC_ACCESS_NO_ACK_MAX   = 2,
};


struct fpga_pci_device {
    char                *name;
    dev_t               fpga_dev_number;
    struct class        *fpga_class;
    struct device       *fpga_udev;
    struct cdev         fpga_cdev;
    resource_size_t     start;
    resource_size_t     len;
    void                *fpga_base;
    int                 irq_vec_count;
    unsigned int        irq;
    int                 pci_enabled;
    int                 pci_region_requested;
    int                 cdev_added;
    struct semaphore    sem[MAX_CHANNEL];
    struct i2c_adapter  adapter[MAX_CHANNEL];
    unsigned long long  last_stop_time[MAX_CHANNEL];
};

#define FPGA_ACCESS(cmd) \
    do { \
        unsigned long flags; \
        spin_lock_irqsave(&(fpga_pcie_lock), flags); \
        cmd \
        spin_unlock_irqrestore(&(fpga_pcie_lock), flags); \
    }while(0);

/* 10ee:7021 Maybe need modify*/
static struct pci_device_id fpga_id_table[] = { \
    {
        0x10ee,     /* Vendor ID */
        0x7021,     /* Device ID */
        PCI_ANY_ID, /* Sub-vendor ID */
        PCI_ANY_ID, /* Sub-device ID */
        0,          /* Class */
        0,          /* Class mask */
        0,          /* Driver data */
    },
    {0,},
};

static unsigned long no_ack_counter[MAX_CHANNEL][IIC_ACCESS_NO_ACK_MAX] = {0};

static struct kobject *fpga_kobj;

struct fpga_base_data {
    struct mutex     update_lock;
    void             *fpga_base;
};
struct fpga_base_data fpga_data;

int fpga_pcie_write(u8 device_id, u32 reg, u32 value)
{
    int status = 0 ;
    FPGA_ACCESS(*((unsigned int *) (fpga_data.fpga_base + reg)) = value;)
    I2C_DEBUG("fpga_pcie_write() 0x%x reg 0x%x\n",value,reg);
    return status;
}
EXPORT_SYMBOL(fpga_pcie_write);

int fpga_pcie_read(u8 device_id, u32 reg)
{
    int status = 0;

    FPGA_ACCESS(status = ACCESS_ONCE(*((unsigned int *)(fpga_data.fpga_base + reg)));)
    I2C_DEBUG("fpga_pcie_read() 0x%x reg 0x%x\n",status,reg);
    return status;
}
EXPORT_SYMBOL(fpga_pcie_read);

static int fpga_smbus_check_busy(struct fpga_pci_device *fpga_pci_dev, unsigned int i2c_cmd_stat_addr, unsigned int *i2c_stat)
{
    unsigned int retry_times = 0;
    unsigned int transaction_delay = delay_busy;

    for(retry_times = 0; retry_times < max_retry_time; retry_times++)
    {
        FPGA_SLEEP(transaction_delay);
        FPGA_ACCESS(*i2c_stat = ACCESS_ONCE(*((unsigned int *)(fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)));)
        I2C_DEBUG("[check_busy] i2c_stat:%x\n", *i2c_stat);
        if((*i2c_stat & BUSY_BIT) == 0)
        {
            return 0;
        }
    }

    return -1;
}

static int fpga_smbus_check_tip(struct fpga_pci_device *fpga_pci_dev, unsigned int i2c_cmd_stat_addr, unsigned int *i2c_stat)
{
    unsigned int retry_times = 0;
    unsigned int transaction_delay = delay_tip;

    for(retry_times = 0; retry_times < max_retry_time; retry_times++)
    {
        FPGA_SLEEP(transaction_delay);
        FPGA_ACCESS(*i2c_stat = ACCESS_ONCE(*((unsigned int *)(fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)));)
        I2C_DEBUG("[check_tip] i2c_stat:%x\n", *i2c_stat);
        if((*i2c_stat & TIP_BIT) == 0)
        {
            return 0;
        }
    }

    return -1;
}

static int fpga_smbus_check_ack(struct fpga_pci_device *fpga_pci_dev, unsigned int i2c_cmd_stat_addr, unsigned int *i2c_stat)
{
    unsigned int retry_times = 0;
    unsigned int ack_transaction_delay = delay_ack;

    for(retry_times = 0; retry_times < max_retry_time; retry_times++)
    {
        I2C_DEBUG("[check_ack] i2c_stat:%x\n", *i2c_stat);
        if((*i2c_stat & ACK_BIT) == 0)
        {
            return 0;
        }
        FPGA_SLEEP(ack_transaction_delay);
        FPGA_ACCESS(*i2c_stat = ACCESS_ONCE(*((unsigned int *)(fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)));)
    }

    if(*i2c_stat & ACK_BIT)
    {
        return -1;
    }

    return 0;
}


static int check_disable_iic_access(void)
{
    return (disable_iic_access != 0);
}


int fpga_smbus_read(struct fpga_pci_device *fpga_pci_dev,
    unsigned char channel, unsigned char addr, unsigned char offset, unsigned char length,
    unsigned char *data)
{
    int retval = 0;
    int stop_step = 0;
    unsigned int i2c_txr_addr = 0, i2c_cmd_stat_addr = 0;
    unsigned int i2c_txr_data = 0, i2c_cmd_stat_data = 0;
    unsigned int i2c_stat = 0;
    unsigned char byte_to_rw = 0, temp_offset = 0;
    unsigned long long diff_time_ns = 0;
    unsigned char buff[MAX_BUFF_SIZE];
    if(data == NULL)
    {
        return(-EINVAL);
    }

    I2C_DEBUG("[%s] READ: %x \n", __func__, data);

    if (addr > 0x7F || channel >= MAX_CHANNEL)
    {
        return(-EINVAL);
    }

    if( check_disable_iic_access() ) {
        return -ETIMEDOUT;
    }

    retval = down_interruptible(&(fpga_pci_dev->sem[channel]));
    if (retval)
    {
        return(-ERESTARTSYS);
    }

    i2c_txr_addr = I2C_TRANSMIT_RECEIVE_OFFSET + channel*0x20;
    i2c_cmd_stat_addr = I2C_COMMAND_STATUS_OFFSET + channel*0x20;

    /*
    1. Set the Transmit Register TXR with a value of Slave address + Write bit.
    2. Set the Command Register CR to 8'h90 to enable the START and WRITE. This starts the transmission on the I2C bus.
    3. Check the Transfer In Progress (TIP) bit of the Status Register, SR, to make sure the command is done.
       Check the Received Acknowledge (RxACK) bit of the Status Register, SR, to make sure I2C slave has ACK.
    4. Set TRX with the slave memory address, where the data is to be read from.
    5. Set CR with 8'h10 to enable a WRITE to send to the slave memory address.
    6. Check the TIP bit of SR, to make sure the command is done.
       Check the Received Acknowledge (RxACK) bit of the Status Register, SR, to make sure I2C slave has ACK.
    7. Set TRX with a value of Slave address + READ bit.
    8. Set CR with the 8'h90 to enable the START (repeated START in this case) and WRITE the value in TXR to the slave device.
    9. Check the TIP bit of SR, to make sure the command is done.
       Check the Received Acknowledge (RxACK) bit of the Status Register, SR, to make sure I2C slave has ACK.
    10. Set CR with 8'h20 to issue a READ command and then an ACK command. This enables the reading of data from the slave device.
    11. Check the TIP bit of SR, to make sure the command is done.
    12. Repeat steps 10 and 11 to continue to read data from the slave device.
    13. When the Master is ready to stop reading from the Slave, set CR to 8'h28. This will read the last byte of data and then issue a NACK.
        Check the TIP bit of SR, to make sure the command is done.
    */

    /* 1. Set the Transmit Register TXR with a value of Slave address + Write bit.*/
    i2c_txr_data = (addr) << 1;
    SET_TXR_WRITE(i2c_txr_data);
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_txr_addr)) = i2c_txr_data;)
    I2C_DEBUG("[1] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);

    diff_time_ns = (ktime_to_ns(ktime_get()) - fpga_pci_dev->last_stop_time[channel]) + tBUF_ACTUAL_TIME_OFFSET_NS;
    if(diff_time_ns < tBUF_TIME_NS)
    {
       udelay(((tBUF_TIME_NS - diff_time_ns) / 1000) > tBUF_MAX_DELAY_US ? tBUF_MAX_DELAY_US : ((tBUF_TIME_NS - diff_time_ns) / 1000));
    }

    /*2. Set the Command Register CR to 8'h90 to enable the START and WRITE. This starts the transmission on the I2C bus.*/
    i2c_cmd_stat_data = COMMAND_START_WRITE;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)
    I2C_DEBUG("[2] i2c_cmd_stat_addr:%x i2c_cmd_stat_data:%x\n", i2c_cmd_stat_addr, i2c_cmd_stat_data);

    /*3. Check the Transfer In Progress (TIP) bit of the Status Register, SR, to make sure the command is done.*/
    if(fpga_smbus_check_tip(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        stop_step = 3;
        goto device_busy;
    }

    /*Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
    if(fpga_smbus_check_ack(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        stop_step = 3;
        goto no_ack_response;
    }

    /*4. Set TRX with the slave memory address, where the data is to be read from.*/
    i2c_txr_data = offset;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_txr_addr)) = i2c_txr_data;)
    I2C_DEBUG("[4] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);

    /*5. Set CR with 8'h10 to enable a WRITE to send to the slave memory address.*/
    i2c_cmd_stat_data = COMMAND_ENABLE_WRITE;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)
    I2C_DEBUG("[5] i2c_cmd_stat_addr:%x i2c_cmd_stat_data:%x\n", i2c_cmd_stat_addr, i2c_cmd_stat_data);

    /*6. Check the TIP bit of SR, to make sure the command is done.*/
    if(fpga_smbus_check_tip(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        stop_step = 6;
        goto device_busy;
    }

    /*Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
    if(fpga_smbus_check_ack(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        stop_step = 6;
        goto no_ack_response;
    }

    /*7. Set TRX with a value of Slave address + READ bit.*/
    i2c_txr_data = (addr) << 1;
    SET_TXR_READ(i2c_txr_data);
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_txr_addr)) = i2c_txr_data;)
    I2C_DEBUG("[7] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);

    /*8. Set CR with the 8'h90 to enable the START (repeated START in this case) and WRITE the value in TXR to the slave device.*/
    i2c_cmd_stat_data = COMMAND_START_WRITE;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)
    I2C_DEBUG("[8] i2c_cmd_stat_addr:%x i2c_cmd_stat_data:%x\n", i2c_cmd_stat_addr, i2c_cmd_stat_data);

    /*9. Check the TIP bit of SR, to make sure the command is done.*/
    if(fpga_smbus_check_tip(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        stop_step = 9;
        goto device_busy;
    }

    /*Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
    if(fpga_smbus_check_ack(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        stop_step = 9;
        goto no_ack_response;
    }

    /*12. Repeat steps 10 and 11 to continue to read data from the slave device.*/
    byte_to_rw = length;
    temp_offset = 0;
    while(byte_to_rw > 1)
    {
        /*10. Set CR with 8'h20 to issue a READ command and then an ACK command. This enables the reading of data from the slave device.*/
        i2c_cmd_stat_data = COMMAND_READ;
        FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)
        I2C_DEBUG("[10] i2c_cmd_stat_addr:%x i2c_cmd_stat_data:%x\n", i2c_cmd_stat_addr, i2c_cmd_stat_data);

        /*11. Check the TIP bit of SR, to make sure the command is done.*/
        if(fpga_smbus_check_tip(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
        {
            stop_step = 11;
            goto device_busy;
        }

        /*Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
        if(fpga_smbus_check_ack(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
        {
            stop_step = 11;
            goto no_ack_response;
        }

        /*Get TXR with 8-bit data from the slave device.*/
        FPGA_ACCESS(i2c_txr_data = ACCESS_ONCE(*((unsigned int *)(fpga_pci_dev->fpga_base + i2c_txr_addr)));)
        buff[temp_offset] = i2c_txr_data;
        I2C_DEBUG("[11-2] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);

        byte_to_rw--;
        temp_offset++;
    }

    /*13.When the Master is ready to stop reading from the Slave, set CR to 8'h28. This will read the last byte of data and then issue a NACK.*/
    i2c_cmd_stat_data = COMMAND_ACK_STOP_READ;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)
    I2C_DEBUG("[13-1] i2c_cmd_stat_addr:%x i2c_cmd_stat_data:%x\n", i2c_cmd_stat_addr, i2c_cmd_stat_data);

    /*Check the BUSY bit of SR, to make sure the command is done.*/
    if(fpga_smbus_check_busy(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        stop_step = 13;
        goto device_busy;
    }
    fpga_pci_dev->last_stop_time[channel] = ktime_to_ns(ktime_get());

    if(byte_to_rw)
    {
        /*Get the last byte from TXR with 8-bit data from the slave device.*/
        FPGA_ACCESS(i2c_txr_data = ACCESS_ONCE(*((unsigned int *)(fpga_pci_dev->fpga_base + i2c_txr_addr)));)
        buff[temp_offset] = i2c_txr_data;
        I2C_DEBUG("[13-3] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);
    }

    memcpy(data, buff, length);

    up(&(fpga_pci_dev->sem[channel]));
    return(0);

device_busy:
    I2C_DEBUG("device_busy, write failed, channel:%d addr:0x%x, length:0x%x, offset:0x%x, stop_step:%d, i2c_stat:0x%x.\n",
            channel, addr, length, offset, stop_step, i2c_stat);

    /*Re-initailize BUS.*/
    /*0. Disable the core by writing 8'h00 to the Control Register, CTR.*/
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + I2C_CONTROLLER_OFFSET + channel*0x20)) = CONTROL_DISABLE;)
    /*1. Program the clock PRESCALE registers, PRERlo and PRERhi, with the desired value.
    This value is determined by the clock frequency and the speed of the I2C bus.*/
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + I2C_PRESCALE_LOW_OFFSET + channel*0x20)) = PRER_LO_DEFAULT;)
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + I2C_PRESCALE_HIGH_OFFSET + channel*0x20)) = PRER_HI_DEFAULT;)
    /*2. Enable the core by writing 8'h80 to the Control Register, CTR.*/
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + I2C_CONTROLLER_OFFSET + channel*0x20)) = CONTROL_ENABLE;)
    FPGA_SLEEP(500);

    /* Set CR with 8'h20 to issue a READ command. This is going to issue a 9-clock command. */
    i2c_cmd_stat_data = COMMAND_READ;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)

    /* Check the TIP bit of SR, to make sure the command is done. */
    if(fpga_smbus_check_tip(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        I2C_DEBUG(KERN_NOTICE "device_busy read, fail to check TIP, i2c_stat:0x%x.\n", i2c_stat);
    }

    /*Set CR to 8'h40 to issue a STOP command to avoid error.*/
    i2c_cmd_stat_data = COMMAND_STOP;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)
    /*Check the BUSY bit of SR, to make sure STOP is sent.*/
    if(fpga_smbus_check_busy(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        I2C_DEBUG(KERN_NOTICE "device_busy read, fail to check STOP, i2c_stat:0x%x.\n", i2c_stat);
    }

    I2C_DEBUG(KERN_NOTICE "device_busy, read failed, channel:%d addr:0x%x, length:0x%x, offset:0x%x, stop_step:%d, i2c_stat:0x%x.\n",
             channel, addr, length, offset, stop_step, i2c_stat);
    up(&(fpga_pci_dev->sem[channel]));
    return (-EBUSY);

no_ack_response:
    /* Set CR with 8'h20 to issue a READ command. This is going to issue a 9-clock command. */
    i2c_cmd_stat_data = COMMAND_READ;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)

    /* Check the TIP bit of SR, to make sure the command is done. */
    if(fpga_smbus_check_tip(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        I2C_DEBUG(KERN_NOTICE "no_ack_response read, fail to check TIP, i2c_stat:0x%x.\n", i2c_stat);
    }

    /*Set CR to 8'h40 to issue a STOP command to avoid error.*/
    i2c_cmd_stat_data = COMMAND_STOP;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)

    if(no_ack_counter[channel][IIC_READ_NO_ACK] < 3) {
        I2C_DEBUG(KERN_ERR "no_ack_response, read failed, channel:%d addr:0x%x, length:0x%x, offset:0x%x, stop_step:%d, i2c_stat:0x%x.\n",
                channel, addr, length, offset, stop_step, i2c_stat);
    }
    else {
        I2C_DEBUG("no_ack_response, read failed, channel:%d addr:0x%x, length:0x%x, offset:0x%x, stop_step:%d, i2c_stat:0x%x.\n",
                channel, addr, length, offset, stop_step, i2c_stat);
    }
    no_ack_counter[channel][IIC_READ_NO_ACK] += 1;

    /*Check the BUSY bit of SR, to make sure STOP is sent.*/
    if(fpga_smbus_check_busy(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        stop_step = 14;
        goto device_busy;
    }

    up(&(fpga_pci_dev->sem[channel]));
    return (-ETIMEDOUT);
}

int fpga_smbus_write(struct fpga_pci_device *fpga_pci_dev,
    unsigned char channel, unsigned char addr, unsigned char offset, unsigned char length,
    unsigned char *data)
{
    int retval = 0;
    int stop_step = 0;
    unsigned int i2c_txr_addr = 0, i2c_cmd_stat_addr = 0;
    unsigned int i2c_txr_data = 0, i2c_cmd_stat_data = 0;
    unsigned int i2c_stat = 0;
    unsigned char byte_to_rw = 0, temp_offset = 0;
    unsigned long long diff_time_ns = 0;
    unsigned char buff[MAX_BUFF_SIZE];
    if(data == NULL)
    {
        return(-EINVAL);
    }

    I2C_DEBUG("[%s] WRITE\n", __func__);

    if (addr > 0x7F || channel >= MAX_CHANNEL)
    {
        return(-EINVAL);
    }

    if( check_disable_iic_access() ) {
        return -ETIMEDOUT;
    }

    retval = down_interruptible(&(fpga_pci_dev->sem[channel]));
    if (retval)
    {
        return(-ERESTARTSYS);
    }

    i2c_txr_addr = I2C_TRANSMIT_RECEIVE_OFFSET + channel*0x20;
    i2c_cmd_stat_addr = I2C_COMMAND_STATUS_OFFSET + channel*0x20;

    memcpy(buff, data, length);

    /*
    1. Set the Transmit Register TXR with a value of Slave address + Write bit.
    2. Set the Command Register CR to 8'h90 to enable the START and WRITE. This starts the transmission on the I2C bus.
    3. Check the Transfer In Progress (TIP) bit of the Status Register, SR, to make sure the command is done.
       Check the Received Acknowledge (RxACK) bit of the Status Register, SR, to make sure I2C slave has ACK.
    4. Set TXR with a slave memory address for the data to be written to.
    5. Set CR with 8'h10 to enable a WRITE to send to the slave memory address.
    6. Check the TIP bit of SR, to make sure the command is done.
       Check the Received Acknowledge (RxACK) bit of the Status Register, SR, to make sure I2C slave has ACK.
    7. Set TXR with 8-bit data for the slave device.
    8. Set CR to 8'h10 to enable a WRITE to send data.
    9. Check the TIP bit of SR, to make sure the command is done.
       Check the Received Acknowledge (RxACK) bit of the Status Register, SR, to make sure I2C slave has ACK.
    10. Repeat steps 7 to 9 to continue to send data to the slave device.
    11. Set CR to 8'h40 to then issue a STOP command.
        Check the TIP bit of SR, to make sure the command is done.
    */

    /* 1. Set the Transmit Register TXR with a value of Slave address + Write bit.*/
    i2c_txr_data = (addr) << 1;
    SET_TXR_WRITE(i2c_txr_data);
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_txr_addr)) = i2c_txr_data;)
    I2C_DEBUG("[1] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);

    diff_time_ns = (ktime_to_ns(ktime_get()) - fpga_pci_dev->last_stop_time[channel]) + tBUF_ACTUAL_TIME_OFFSET_NS;
    if(diff_time_ns < tBUF_TIME_NS)
    {
       udelay(((tBUF_TIME_NS - diff_time_ns) / 1000) > tBUF_MAX_DELAY_US ? tBUF_MAX_DELAY_US : ((tBUF_TIME_NS - diff_time_ns) / 1000));
    }

    /* 2. Set the Command Register CR to 8'h90 to enable the START and WRITE. This starts the transmission on the I2C bus.*/
    i2c_cmd_stat_data = COMMAND_START_WRITE;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)
    I2C_DEBUG("[2] i2c_cmd_stat_addr:%x i2c_cmd_stat_data:%x\n", i2c_cmd_stat_addr, i2c_cmd_stat_data);

    /* 3. Check the Transfer In Progress (TIP) bit of the Status Registesr, SR, to make sure the command is done.*/
    if(fpga_smbus_check_tip(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        stop_step = 3;
        goto device_busy;
    }

    /* Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
    if(fpga_smbus_check_ack(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        stop_step = 3;
        goto no_ack_response;
    }

    /* 4. Set TXR with a slave memory address for the data to be written to.*/
    i2c_txr_data = offset;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_txr_addr)) = i2c_txr_data;)
    I2C_DEBUG("[4] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);

    /* 5. Set CR with 8'h10 to enable a WRITE to send to the slave memory address.*/
    i2c_cmd_stat_data = COMMAND_ENABLE_WRITE;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)
    I2C_DEBUG("[5] i2c_cmd_stat_addr:%x i2c_cmd_stat_data:%x\n", i2c_cmd_stat_addr, i2c_cmd_stat_data);

    /* 6. Check the TIP bit of SR, to make sure the command is done.*/
    if(fpga_smbus_check_tip(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        stop_step = 6;
        goto device_busy;
    }

    /* Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
    if(fpga_smbus_check_ack(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        stop_step = 6;
        goto no_ack_response;
    }

    /* 10. Repeat steps 7 to 9 to continue to send data to the slave device.*/
    byte_to_rw = length;
    temp_offset = 0;
    while(byte_to_rw > 0)
    {
        /* 7. Set TXR with 8-bit data for the slave device.*/
        i2c_txr_data = buff[temp_offset];
        FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_txr_addr)) = i2c_txr_data;)
        I2C_DEBUG("[7] i2c_txr_addr:%x i2c_txr_data:%x\n", i2c_txr_addr, i2c_txr_data);

        /* 8. Set CR to 8'h10 to enable a WRITE to send data.*/
        i2c_cmd_stat_data = COMMAND_ENABLE_WRITE;
        FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)
        I2C_DEBUG("[8] i2c_cmd_stat_addr:%x i2c_cmd_stat_data:%x\n", i2c_cmd_stat_addr, i2c_cmd_stat_data);

        /* 9. Check the TIP bit of SR, to make sure the command is done.*/
        if(fpga_smbus_check_tip(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
        {
            stop_step = 9;
            goto device_busy;
        }

        /* Check the ACK bit of the Status Registesr, SR, to make sure ACK is received.*/
        if(fpga_smbus_check_ack(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
        {
            stop_step = 9;
            goto no_ack_response;
        }

        byte_to_rw--;
        temp_offset++;
    }

    /* 11. Set CR to 8'h40 to issue a STOP command.*/
    i2c_cmd_stat_data = COMMAND_STOP;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)
    I2C_DEBUG("[12] i2c_cmd_stat_addr:%x i2c_cmd_stat_data:%x\n", i2c_cmd_stat_addr, i2c_cmd_stat_data);

    /* Check the BUSY bit of SR, to make sure STOP is sent.*/
    if(fpga_smbus_check_busy(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        stop_step = 12;
        goto device_busy;
    }
    fpga_pci_dev->last_stop_time[channel] = ktime_to_ns(ktime_get());

    up(&(fpga_pci_dev->sem[channel]));
    return(0);

device_busy:
    I2C_DEBUG("device_busy, write failed, channel:%d addr:0x%x, length:0x%x, offset:0x%x, stop_step:%d, i2c_stat:0x%x.\n",
            channel, addr, length, offset, stop_step, i2c_stat);

    /* Re-initailize BUS.*/
    /* 0. Disable the core by writing 8'h00 to the Control Register, CTR.*/
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + I2C_CONTROLLER_OFFSET + channel*0x20)) = CONTROL_DISABLE;)
    /* 1. Program the clock PRESCALE registers, PRERlo and PRERhi, with
    the desired value. This value is determined by the clock frequency and the speed of the I2C bus.*/
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + I2C_PRESCALE_LOW_OFFSET + channel*0x20)) = PRER_LO_DEFAULT;)
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + I2C_PRESCALE_HIGH_OFFSET + channel*0x20)) = PRER_HI_DEFAULT;)
    /* 2. Enable the core by writing 8'h80 to the Control Register, CTR.*/
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + I2C_CONTROLLER_OFFSET + channel*0x20)) = CONTROL_ENABLE;)
    FPGA_SLEEP(500);

    /* Set CR with 8'h20 to issue a READ command. This is going to issue a 9-clock command. */
    i2c_cmd_stat_data = COMMAND_READ;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)

    /* Check the TIP bit of SR, to make sure the command is done. */
    if(fpga_smbus_check_tip(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        printk(KERN_NOTICE "device_busy write, fail to check TIP, i2c_stat:0x%x.\n", i2c_stat);
    }

    /*Set CR to 8'h40 to issue a STOP command to avoid error.*/
    i2c_cmd_stat_data = COMMAND_STOP;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)
    /* Check the BUSY bit of SR, to make sure STOP is sent.*/
    if(fpga_smbus_check_busy(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        printk(KERN_NOTICE "device_busy write, fail to check BUSY, i2c_stat:0x%x.\n", i2c_stat);
    }

    up(&(fpga_pci_dev->sem[channel]));
    return (-EBUSY);

no_ack_response:
    /* Set CR with 8'h20 to issue a READ command. This is going to issue a 9-clock command. */
    i2c_cmd_stat_data = COMMAND_READ;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)

    /* Check the TIP bit of SR, to make sure the command is done. */
    if(fpga_smbus_check_tip(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        printk(KERN_NOTICE "no_ack_response write, fail to check TIP, i2c_stat:0x%x.\n", i2c_stat);
    }

    /* Set CR to 8'h40 to issue a STOP command to avoid error.*/
    i2c_cmd_stat_data = COMMAND_STOP;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_cmd_stat_addr)) = i2c_cmd_stat_data;)

    if(no_ack_counter[channel][IIC_WRITE_NO_ACK] < 3) {
        printk(KERN_ERR "no_ack_response, write failed, channel:%d addr:0x%x, length:0x%x, offset:0x%x, stop_step:%d, i2c_stat:0x%x.\n",
                channel, addr, length, offset, stop_step, i2c_stat);
    }
    else {
        I2C_DEBUG("no_ack_response, write failed, channel:%d addr:0x%x, length:0x%x, offset:0x%x, stop_step:%d, i2c_stat:0x%x.\n",
                 channel, addr, length, offset, stop_step, i2c_stat);
    }
    no_ack_counter[channel][IIC_WRITE_NO_ACK] += 1;

    /* Check the BUSY bit of SR, to make sure STOP is sent.*/
    if(fpga_smbus_check_busy(fpga_pci_dev, i2c_cmd_stat_addr, &i2c_stat))
    {
        printk(KERN_NOTICE "no_ack_response write, fail to check BUSY, i2c_stat:0x%x.\n", i2c_stat);
        stop_step = 13;
        goto device_busy;
    }

    up(&(fpga_pci_dev->sem[channel]));
    return (-ETIMEDOUT);
}

static u32 fpga_smbus_func(struct i2c_adapter *adapter)
{
	return I2C_FUNC_SMBUS_QUICK | I2C_FUNC_SMBUS_BYTE |
	       I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA |
	       I2C_FUNC_SMBUS_BLOCK_DATA | I2C_FUNC_SMBUS_WRITE_I2C_BLOCK;
}

/* Return negative errno on error. */
static s32 fpga_smbus_access(struct i2c_adapter *adap, u16 addr,
		       unsigned short flags, char read_write, u8 command,
		       int size, union i2c_smbus_data *data)
{
    struct fpga_pci_device *fpga_pci_dev;
    int retval = 0;
    unsigned char length = 0, channel = 0;
    char cha_num_str[3] = {0};

    I2C_DEBUG("[%s]name:%s addr:0x%x flags:0x%x read_write:0x%x command:0x%x size:0x%x\n",
        __func__, adap->name, addr, flags, read_write, command, size);

    switch(size)
    {
        case I2C_SMBUS_BYTE:
            length = 1;
            break;
        case I2C_SMBUS_BYTE_DATA:
            length = 1;
            break;
        case I2C_SMBUS_WORD_DATA:
            length = 2;
            break;
        case I2C_SMBUS_BLOCK_DATA:
            length = data->block[0];
            break;
        case I2C_SMBUS_I2C_BLOCK_DATA:
            length = 32;
            break;
        case I2C_SMBUS_QUICK:
            length = 0;
            break;
        default:
            length = 1;
            break;
    }

    memcpy(cha_num_str, (adap->name+18), 2);
    retval = kstrtou8(cha_num_str, 10, &channel);
    if(retval == 0)
    {
        I2C_DEBUG("channel:%d \n", channel);
    }
    else
    {
        ERROR_DEBUG("Error:%d, channel:%s \n", retval, cha_num_str);
    }

    fpga_pci_dev = i2c_get_adapdata(adap);
    if(fpga_pci_dev == NULL)
    {
        return(-EINVAL);
    }

    if(length > 32+2)
    {
        return(-EINVAL);
    }
    if(data == NULL)
    {
        return(-EINVAL);
    }

    if(I2C_SMBUS_READ == read_write)
    {
        retval = fpga_smbus_read(fpga_pci_dev, channel, addr, command, length, (unsigned char *)data);
    }
    else if(I2C_SMBUS_WRITE == read_write)
    {
        if(size==I2C_SMBUS_BLOCK_DATA)
            retval = fpga_smbus_write(fpga_pci_dev, channel, addr, command, length, (unsigned char *)data->block+1);
        else
            retval = fpga_smbus_write(fpga_pci_dev, channel, addr, command, length, (unsigned char *)data);
    }

    return retval;
}

static const struct i2c_algorithm smbus_algorithm = {
	.smbus_xfer	= fpga_smbus_access,
	.functionality	= fpga_smbus_func,
};

static pci_ers_result_t fpga_error_detected(struct pci_dev *pdev, pci_channel_state_t error)
{
    I2C_DEBUG( "[%s]\n", __func__);
    return PCI_ERS_RESULT_CAN_RECOVER;
}

static pci_ers_result_t fpga_mmio_enabled(struct pci_dev *pdev)
{
    I2C_DEBUG( "[%s]\n", __func__);
    return PCI_ERS_RESULT_CAN_RECOVER;
}

static pci_ers_result_t fpga_slot_reset(struct pci_dev *pdev)
{
    I2C_DEBUG( "[%s]\n", __func__);
    return PCI_ERS_RESULT_CAN_RECOVER;
}

static void fpga_resume(struct pci_dev *pdev)
{
    I2C_DEBUG( "[%s]\n", __func__);
    return;
}

static const struct pci_error_handlers fpga_error_handlers = {
    .error_detected = fpga_error_detected,
    .mmio_enabled   = fpga_mmio_enabled,
    .slot_reset     = fpga_slot_reset,
    .resume         = fpga_resume,
};

static int fpga_open(struct inode *inode, struct file *filp)
{
    I2C_DEBUG( "[%s]\n", __func__);
    filp->private_data = (void *) container_of(inode->i_cdev, struct fpga_pci_device, fpga_cdev);

    return(0);
}

static int fpga_release(struct inode *inode, struct file *filp)
{
    I2C_DEBUG( "[%s]\n", __func__);
    return(0);
}

ssize_t fpga_read(struct file *filp, char __user *user_data, size_t size, loff_t *offset)
{
    struct fpga_pci_device *fpga_pci_dev = NULL;

    fpga_pci_dev = (struct fpga_pci_device *) filp->private_data;
    I2C_DEBUG( "[%s] %s\n", __func__, fpga_pci_dev->name);

    return(0);
}

ssize_t fpga_write(struct file *filp, const char __user *user_data, size_t size, loff_t *offset)
{
    struct fpga_pci_device *fpga_pci_dev = NULL;

    fpga_pci_dev = (struct fpga_pci_device *) filp->private_data;
    I2C_DEBUG( "[%s] %s\n", __func__, fpga_pci_dev->name);

    return(size);
}

long fpga_init_smbus(struct fpga_pci_device *fpga_pci_dev)
{
    unsigned int i2c_ctrl_addr = 0;
    unsigned int i2c_ctrl_data = 0;
    unsigned int i2c_pre_low_addr = 0, i2c_pre_high_addr = 0;
    unsigned int i2c_pre_low_data = 0, i2c_pre_high_data = 0;
    int temp_channel = 0;

    for(temp_channel = 0; temp_channel<MAX_CHANNEL; temp_channel++)
    {
        /*
        1. Program the clock PRESCALE registers, PRERlo and PRERhi, with the desired value.
        This value is determined by the clock frequency and the speed of the I2C bus.
        2. Enable the core by writing 8'h80 to the Control Register, CTR.
        */
        i2c_pre_low_addr = I2C_PRESCALE_LOW_OFFSET + temp_channel*0x20;
        i2c_pre_high_addr = I2C_PRESCALE_HIGH_OFFSET + temp_channel*0x20;
        i2c_ctrl_addr = I2C_CONTROLLER_OFFSET + temp_channel*0x20;

        /* 1. Program the clock PRESCALE registers, PRERlo and PRERhi,
        with the desired value. This value is determined by the clock frequency and the speed of the I2C bus.*/
        i2c_pre_low_data = PRER_LO_DEFAULT;
        FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_pre_low_addr)) = i2c_pre_low_data;)
        I2C_DEBUG( "[pre1-1] i2c_pre_low_addr:%x %x\n", i2c_pre_low_addr, i2c_pre_low_addr);

        i2c_pre_high_data = PRER_HI_DEFAULT;
        FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_pre_high_addr)) = i2c_pre_high_data;)
        I2C_DEBUG( "[pre1-2] i2c_pre_high_addr:%x %x\n", i2c_pre_high_addr, i2c_pre_high_addr);

        /* 2. Enable the core by writing 8'h80 to the Control Register, CTR.*/
        i2c_ctrl_data= CONTROL_ENABLE;
        FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + i2c_ctrl_addr)) = i2c_ctrl_data;)
        I2C_DEBUG( "[pre1] i2c_ctrl_addr:%x %x\n", i2c_ctrl_addr, i2c_ctrl_addr);
    }

    return 0;
}

long fpga_mask_all_intr(struct fpga_pci_device *fpga_pci_dev)
{
    unsigned int SI5395_INT;
    /* 1. Mask all I2C Module Interrupt*/
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + 0x5c)) = 0x01FFFFF;)

    /* 2. Mask CDR Interrupt Mask*/
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + 0x44)) = 0x00001FFF;)

    /* 3. Mask TEMP Interrupt Mask*/
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + 0x48)) = 0x00000003;)

    /* 4. Mask QSFP Interrupt Mask*/
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + 0x4c)) = 0x0000FFFF;)

    /* 5. Mask QDD Interrupt Mask*/
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + 0x50)) = 0x0000000F;)

    /* 6. Mask SI5395_INT_mask Interrupt Mask*/
    FPGA_ACCESS(SI5395_INT = ACCESS_ONCE(*((unsigned int *)(fpga_pci_dev->fpga_base + 0x34)));)
    SI5395_INT |= 0x00110000;
    FPGA_ACCESS(*((unsigned int *) (fpga_pci_dev->fpga_base + 0x34)) = SI5395_INT;)

    return 0;
}

long fpga_ioctl(struct file *filp, unsigned int cmd, unsigned long user_addr)
{
    struct fpga_pci_device *fpga_pci_dev = NULL;
    struct fpga_access acc = {0};
    int retval = 0;
    unsigned short spi_offset = 0;

    if (cmd == FPGA_READ)
    {
        I2C_DEBUG( "[%s] READ\n", __func__);
        fpga_pci_dev = (struct fpga_pci_device *) filp->private_data;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
        if (!access_ok(VERIFY＿READ,user_addr, _IOC_SIZE(cmd)))
#else
        if (!access_ok(user_addr, _IOC_SIZE(cmd)))
#endif   
        {
          I2C_DEBUG( "[%s] Access NOT OK\n", __func__);
          return(-EINVAL);
        }

        retval = copy_from_user((void *) &acc, (const void *) user_addr, sizeof(struct fpga_access));
        if (retval)
        {
            return(-EFAULT);
        }

        if (acc.length + acc.offset > MAX_EEPROM_SIZE)
        {
            return(-EINVAL);
        }

        retval = fpga_smbus_read(fpga_pci_dev, acc.channel, acc.addr, acc.offset, acc.length, acc.buff);
        if(retval != 0)
        {
            return retval;
        }

        retval = copy_to_user((void *) user_addr, (const void *) &acc, sizeof(struct fpga_access));
        if (retval)
        {
            return(-EFAULT);
        }

        return(0);
    }
    else if (cmd == FPGA_WRITE)
    {
        fpga_pci_dev = (struct fpga_pci_device *) filp->private_data;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
        if (!access_ok(VERIFY_WRITE,user_addr, _IOC_SIZE(cmd)))
#else
        if (!access_ok(user_addr, _IOC_SIZE(cmd)))
#endif   
        {
            I2C_DEBUG( "[%s] Access NOT OK\n", __func__);
            return(-EINVAL);
        }

        retval = copy_from_user((void *) &acc, (const void *) user_addr, sizeof(struct fpga_access));
        if (retval)
        {
            return(-EFAULT);
        }

        if (acc.length + acc.offset > MAX_EEPROM_SIZE)
        {
            return(-EINVAL);
        }

        retval = fpga_smbus_write(fpga_pci_dev, acc.channel, acc.addr, acc.offset, acc.length, acc.buff);
        if(retval != 0)
        {
            return retval;
        }

        return(0);
    }
    else if (cmd == FPGA_LOCK)
    {
        return(0);
    }
    else if (cmd == FPGA_UNLOCK)
    {
        return(0);
    }
    else if (cmd == FPGA_SPI_READ || cmd == FPGA_SPI_WRITE)
    {
        fpga_pci_dev = (struct fpga_pci_device *) filp->private_data;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
        if (!access_ok(VERIFY_WRITE,user_addr, _IOC_SIZE(cmd)))
#else
        if (!access_ok(user_addr, _IOC_SIZE(cmd)))
#endif   
        {
            I2C_DEBUG( "[%s] Access NOT OK\n", __func__);
            return(-EINVAL);
        }

        retval = copy_from_user((void *) &acc, (const void *) user_addr, sizeof(struct fpga_access));
        if (retval)
        {
            return(-EFAULT);
        }

        if (retval)
        {
            return(-ERESTARTSYS);
        }
        spi_offset = (acc.buff[0]<<8) + acc.buff[1];
        if (cmd == FPGA_SPI_READ)
        {
            FPGA_ACCESS(acc.buff[2] = ACCESS_ONCE(*((unsigned char *)(fpga_pci_dev->fpga_base + spi_offset)));)
        }
        else if (cmd == FPGA_SPI_WRITE)
        {
            FPGA_ACCESS(*((unsigned char *) (fpga_pci_dev->fpga_base + spi_offset)) = acc.buff[2];)
        }
        retval = copy_to_user((void *) user_addr, (const void *) &acc, sizeof(struct fpga_access));

        return(0);
    }
    else
    {
        ERROR_DEBUG( "[%s] Unknown command\n", __func__);
        return(-EINVAL);
    }
}

#ifdef INTERRUPT_SUPPORT
static irqreturn_t fpga_intr_msi(int irq, void *data)
{
    static int count=0;

    disable_irq_nosync(irq);

    enable_irq(irq);

    I2C_DEBUG( "MSI irq:%d, count=%d\n", irq, count);
    count++;

    return IRQ_HANDLED;
}
#endif

static struct file_operations fpga_fops = {
    .owner          = THIS_MODULE,
    .open           = fpga_open,
    .read           = fpga_read,
    .write          = fpga_write,
    .unlocked_ioctl = fpga_ioctl,
    .release        = fpga_release,
};

static int fpga_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
    int retval = 0;
    struct fpga_pci_device *fpga_pci_dev = NULL;
    int smbus_temp_num,smbus_adapter_num = 0;

    fpga_pci_dev = (struct fpga_pci_device *) kzalloc(sizeof(struct fpga_pci_device), GFP_KERNEL);
    if (!fpga_pci_dev)
    {
        ERROR_DEBUG( "[%s] Fail to alloc fpga_pci_dev memory\n", __func__);
        retval = -ENOMEM;
        goto fail_to_alloc_fpga_pci_dev;
    }

    pci_set_drvdata(pdev, (void *) fpga_pci_dev);

    fpga_pci_dev->name = "Accton FPGA";

    retval = alloc_chrdev_region(&(fpga_pci_dev->fpga_dev_number), 0, fpga_dev_count, FPGA_DEV_NAME);
    if (retval)
    {
        ERROR_DEBUG( "[%s] Fail to alloc fpga_dev_number %d\n", __func__, fpga_pci_dev->fpga_dev_number);
        goto fail_to_alloc_dev_number;
    }

    fpga_pci_dev->fpga_class = class_create(THIS_MODULE, FPGA_CLASS_NAME);
    if (IS_ERR(fpga_pci_dev->fpga_class))
    {
        ERROR_DEBUG( "[%s] Fail to create %s [fpga_class: %p]\n", __func__, FPGA_CLASS_NAME, fpga_pci_dev->fpga_class);
        retval = PTR_ERR(fpga_pci_dev->fpga_class);
        goto fail_to_create_fpga_class;
    }
    I2C_DEBUG( "[%s] fpga_class created\n", __func__);

    fpga_pci_dev->fpga_udev = device_create(fpga_pci_dev->fpga_class, NULL, fpga_pci_dev->fpga_dev_number,
            NULL, FPGA_DEV_NAME);

    if (IS_ERR(fpga_pci_dev->fpga_udev))
    {
        ERROR_DEBUG( "[%s] Fail to create fpga_udev\n", __func__);
        retval = PTR_ERR(fpga_pci_dev->fpga_udev);
        goto fail_to_create_fpga_udev;
    }
    I2C_DEBUG( "[%s] fpga_udev created\n", __func__);

    retval = pci_enable_device(pdev);
    if (retval)
    {
        ERROR_DEBUG( "[%s] Fail to enable PCIe device\n", __func__);
        goto fail_to_enable_pci_device;
    }
    fpga_pci_dev->pci_enabled = 1;

    fpga_pci_dev->start = pci_resource_start(pdev, BAR0);
    I2C_DEBUG( "[%s] Get PCI resource address 0x%llx\n", __func__, fpga_pci_dev->start);

    fpga_pci_dev->len = pci_resource_len(pdev, BAR0);
    I2C_DEBUG( "[%s] Get PCI resource length %llu\n", __func__, fpga_pci_dev->len);

    retval = pci_request_region(pdev, BAR0, FPGA_REGION_NAME);
    if (retval)
    {
        ERROR_DEBUG( "[%s] Fail to request PCI resource region\n", __func__);
        goto fail_to_request_pci_region;
    }
    fpga_pci_dev->pci_region_requested = 1;

    fpga_pci_dev->fpga_base = pci_iomap(pdev, BAR0, fpga_pci_dev->len);
    if (!fpga_pci_dev->fpga_base) {
        ERROR_DEBUG( "[%s] Fail to remap PCI resource address\n", __func__);
        retval = -ENODEV;
        goto fail_to_remap_pci_resource;
    }
    else
    {
        I2C_DEBUG( "[%s] Remap PCI resource address 0x%p\n", __func__, fpga_pci_dev->fpga_base);
    }

    /* Init these items before interrupt is enabled */
    for(smbus_temp_num = 0; smbus_temp_num < MAX_CHANNEL; smbus_temp_num++)
    {
        sema_init(&(fpga_pci_dev->sem[smbus_temp_num]), 1);
    }
    spin_lock_init(&(fpga_pcie_lock));

    pci_set_master(pdev);

    /* Register sysfs hooks */
    fpga_data.fpga_base = fpga_pci_dev->fpga_base;

#ifdef INTERRUPT_SUPPORT
    fpga_mask_all_intr(fpga_pci_dev);

    /* Register interrupt */
    retval = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_MSI);
    if (retval > 0)
    {
        fpga_pci_dev->irq_vec_count = retval;
        I2C_DEBUG( "[%s] PCI irq vector count %d IRQ number: %u\n", __func__, retval, pdev->irq);
    }
    else
    {
        I2C_DEBUG( "[%s] Fail to alloc PCI irq vectors\n", __func__);
        goto fail_to_alloc_irq_vectors;
    }

    retval = request_irq(pci_irq_vector(pdev, 0), fpga_intr_msi, 0, "FPGA MSI", (void *) fpga_pci_dev);
    if (retval < 0) {
        I2C_DEBUG( "[%s] Fail to request irq\n", __func__);
        goto fail_to_request_irq;
      } else {
        I2C_DEBUG( "[%s] Request irq no. %d\n", __func__, pdev->irq);
        fpga_pci_dev->irq = pdev->irq;
    }
#endif

    cdev_init(&(fpga_pci_dev->fpga_cdev), &fpga_fops);
    fpga_pci_dev->fpga_cdev.owner = THIS_MODULE;

    retval = cdev_add(&(fpga_pci_dev->fpga_cdev), fpga_pci_dev->fpga_dev_number, fpga_dev_count);
    if (retval)
    {
        goto fail_to_add_fpga_cdev;
    }
    fpga_pci_dev->cdev_added = 1;

    for(smbus_temp_num = 0; smbus_temp_num < MAX_CHANNEL; smbus_temp_num++)
    {
        i2c_set_adapdata(&fpga_pci_dev->adapter[smbus_temp_num], fpga_pci_dev);
        fpga_pci_dev->adapter[smbus_temp_num].owner = THIS_MODULE;
        fpga_pci_dev->adapter[smbus_temp_num].class = I2C_CLASS_HWMON | I2C_CLASS_SPD;;
        fpga_pci_dev->adapter[smbus_temp_num].algo = &smbus_algorithm;
        fpga_pci_dev->adapter[smbus_temp_num].dev.parent = &pdev->dev;
        fpga_pci_dev->adapter[smbus_temp_num].retries = 3;

        /* Default timeout in interrupt mode: 200 ms */
    	fpga_pci_dev->adapter[smbus_temp_num].timeout = HZ / 5;

        snprintf(fpga_pci_dev->adapter[smbus_temp_num].name, sizeof(fpga_pci_dev->adapter[smbus_temp_num].name),
    		"%s%02d adapter", FPGA_SMBUS_NAME, smbus_temp_num);

        fpga_pci_dev->adapter[smbus_temp_num].nr = 601+smbus_temp_num;
        retval = i2c_add_numbered_adapter(&fpga_pci_dev->adapter[smbus_temp_num]);

    	if (retval) {
            ERROR_DEBUG( "[%s] err: 0x%x\n", __func__, retval);
    		goto fail_to_add_i2c_adapter;
    	}
    }

    fpga_init_smbus(fpga_pci_dev);

    dev_info(&(pdev->dev), "FPGA SMBus is running with delay_ack %d us, delay_tip %d us, delay_busy %d us, and max_retry_time %d, PRER_LO_DEFAULT: 0x%x \n",
            delay_ack, delay_tip, delay_busy, max_retry_time, PRER_LO_DEFAULT);

    return(0);

fail_to_add_i2c_adapter:
    for(smbus_adapter_num = 0; smbus_adapter_num < smbus_temp_num; smbus_adapter_num++)
    {
        i2c_del_adapter(&fpga_pci_dev->adapter[smbus_adapter_num]);
    }

fail_to_add_fpga_cdev:
    free_irq(pdev->irq, (void *) fpga_pci_dev);
    fpga_pci_dev->cdev_added = 0;
#ifdef INTERRUPT_SUPPORT
fail_to_request_irq:
    pci_free_irq_vectors(pdev);
    fpga_pci_dev->irq = 0;
fail_to_alloc_irq_vectors:
    iounmap(fpga_pci_dev->fpga_base);
    fpga_pci_dev->irq_vec_count = 0;
#endif
fail_to_remap_pci_resource:
    pci_release_region(pdev, BAR0);
    fpga_pci_dev->fpga_base = NULL;
fail_to_request_pci_region:
    pci_disable_device(pdev);
    fpga_pci_dev->pci_region_requested = 0;
fail_to_enable_pci_device:
    device_destroy(fpga_pci_dev->fpga_class, fpga_pci_dev->fpga_dev_number);
    fpga_pci_dev->pci_enabled = 0;
fail_to_create_fpga_udev:
    class_destroy(fpga_pci_dev->fpga_class);
    fpga_pci_dev->fpga_udev = NULL;
fail_to_create_fpga_class:
    unregister_chrdev_region(fpga_pci_dev->fpga_dev_number, fpga_dev_count);
    fpga_pci_dev->fpga_class = NULL;
fail_to_alloc_dev_number:
    fpga_pci_dev->fpga_dev_number = 0;
    kfree(fpga_pci_dev);
fail_to_alloc_fpga_pci_dev:
    pci_set_drvdata(pdev, (void *) NULL);

    return(retval);
}

static void fpga_pci_remove(struct pci_dev *pdev)
{
    struct fpga_pci_device *fpga_pci_dev = NULL;
    int smbus_temp_num;

    I2C_DEBUG( "[%s]\n", __func__);

    fpga_pci_dev = (struct fpga_pci_device *) pci_get_drvdata(pdev);
    if (fpga_pci_dev)
    {
        if (fpga_pci_dev->cdev_added)
        {
            cdev_del(&(fpga_pci_dev->fpga_cdev));
        }

        if (fpga_pci_dev->irq > 0)
        {
            free_irq(pci_irq_vector(pdev, 0), (void *) fpga_pci_dev);
            I2C_DEBUG( "[%s] free_irq()\n", __func__);
        }

        if (fpga_pci_dev->irq_vec_count > 0)
        {
            pci_free_irq_vectors(pdev);
            I2C_DEBUG( "[%s] pci_free_irq_vectors()\n", __func__);
        }

        if (fpga_pci_dev->fpga_base != NULL)
        {
            iounmap(fpga_pci_dev->fpga_base);
            I2C_DEBUG( "[%s] iounmap()\n", __func__);
        }

        if (fpga_pci_dev->pci_region_requested)
        {
            pci_release_region(pdev, BAR0);
            I2C_DEBUG( "[%s] pci_release_region()\n", __func__);
        }

        if (fpga_pci_dev->pci_enabled)
        {
            pci_disable_device(pdev);
            I2C_DEBUG( "[%s] pci_disable_device()\n", __func__);
        }

        if (fpga_pci_dev->fpga_udev)
        {
            device_destroy(fpga_pci_dev->fpga_class, fpga_pci_dev->fpga_dev_number);
        }

        if (fpga_pci_dev->fpga_class)
        {
            class_destroy(fpga_pci_dev->fpga_class);
            I2C_DEBUG( "[%s] class_destroy()\n", __func__);
        }

        if (fpga_pci_dev->fpga_dev_number)
        {
            unregister_chrdev_region(fpga_pci_dev->fpga_dev_number, fpga_dev_count);
            I2C_DEBUG( "[%s] unregister_chrdev_region()\n", __func__);
        }

        for(smbus_temp_num = 0; smbus_temp_num < MAX_CHANNEL; smbus_temp_num++)
        {
            i2c_del_adapter(&fpga_pci_dev->adapter[smbus_temp_num]);
        }

        kfree(fpga_pci_dev);
        I2C_DEBUG( "[%s] kfree(fpga_pci_dev)\n", __func__);
    }

    return;
}

static struct pci_driver fpga_pci_driver = {
    .name        = FPGA_DRIVER_NAME,
    .id_table    = fpga_id_table,
    .probe       = fpga_pci_probe,
    .remove      = fpga_pci_remove,
    .err_handler = &fpga_error_handlers,
};

static int __init fpga_module_init(void)
{
    I2C_DEBUG( "[%s]\n", __func__);
    mutex_init(&fpga_data.update_lock);
    return pci_register_driver(&fpga_pci_driver);
}

static void __exit fpga_module_cleanup(void)
{
    I2C_DEBUG( "[%s]\n", __func__);
    if(fpga_kobj){
            kobject_put(fpga_kobj);
    }
    mutex_destroy(&fpga_data.update_lock);
    pci_unregister_driver(&fpga_pci_driver);
    return;
}

module_init(fpga_module_init);
module_exit(fpga_module_cleanup);

MODULE_DEVICE_TABLE(pci, fpga_id_table);

