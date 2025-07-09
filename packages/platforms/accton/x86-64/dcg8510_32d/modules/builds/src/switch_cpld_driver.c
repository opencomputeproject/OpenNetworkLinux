#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/fs.h>
//#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>

#include "switch_wdt_driver.h"
#include "switch_cpld_driver.h"
#include "sysfs_ipmi.h"
#include <asm/io.h>

//simon: try to support linux 4.x
#include <linux/version.h>

#define DRVNAME                     "drv_cpld_driver"
#define SWITCH_CPLD_DRIVER_VERSION  "0.0.1"
#define CPLD_DEV_NAME               "cpld"
#define CPLD_CLASS_NAME             "cpld_class"

#define PREVIOUS_REBOOT_CAUSE_FILE  "/host/reboot-cause/previous-reboot-cause.json"
#define IS_COLD_RESET_FILE "/tmp/isColdRst"

unsigned int loglevel = 0;
static struct platform_device *drv_cpld_device;
bool is_wdt_trigger = false;
static struct mutex cpld_sem;

/* function pointer for fan CPLD */
int (*fan_cpld_reg_read_func)(u8, u8, u8*);
int (*fan_cpld_reg_write_func)(u8, u8, u8);

static char *cpld_alias_name[CPLD_TOTAL_NUM] = {
    "sys_cpld",
    "fan_cpld",
    "port_cpld1",
    "port_cpld2"
};

static char *cpld_type[CPLD_TOTAL_NUM] = {
    "CPLD1",
    "CPLD2",
    "CPLD3",
    "CPLD4"
};

enum reboot_cause {
    REBOOT_CAUSE_NON_HARDWARE,
    REBOOT_CAUSE_POWER_LOSS,
    REBOOT_CAUSE_THERMAL_OVERLOAD_CPU,
    REBOOT_CAUSE_THERMAL_OVERLOAD_ASIC,
    REBOOT_CAUSE_THERMAL_OVERLOAD_OTHER,
    REBOOT_CAUSE_INSUFFICIENT_FAN_SPEED,
    REBOOT_CAUSE_WATCHDOG,
    REBOOT_CAUSE_HARDWARE_OTHER,
    REBOOT_CAUSE_CPU_COLD_RESET,
    REBOOT_CAUSE_CPU_WARM_RESET,
    REBOOT_CAUSE_BIOS_RESET,
    REBOOT_CAUSE_PSU_SHUTDOWN,
    REBOOT_CAUSE_BMC_SHUTDOWN,
    REBOOT_CAUSE_RESET_BUTTON_SHUTDOWN,
    REBOOT_CAUSE_RESET_BUTTON_COLD_SHUTDOWN,
    REBOOT_CAUSE_CPU_EC_NO_HB,
    REBOOT_CAUSE_CPU_CPU_ERROR,
    REBOOT_CAUSE_CPLD_CPU_PWR_CYCLE
};

void clear_cpld_reg(unsigned int cpld_index, unsigned int offset)
{
    unsigned char reg_val;

    // Write all 1 to clear register
    reg_val = 0xff;
    switch_cpld_reg_write(cpld_index, offset, reg_val);
}

int read_whole_file(struct file *filp, char *content)
{
    loff_t pos = 0;
    char tmp_buf[1];
    int i = 0;

    while(kernel_read(filp, tmp_buf, 1, &pos) == 1)
    {
        if(tmp_buf[0] != '\n')
        {
            content[i] = tmp_buf[0];
            i++;
        }
    }

    return i;
}

bool check_if_reboot_cause_power_loss(void)
{
    unsigned char reg_val;

    switch_cpld_reg_read(0, CPLD_RST_SRC_RECORD_REG_2_OFFSET, &reg_val);
    if((reg_val& 0x1) == 1)
        return true;

    return false;
}

void check_if_reboot_cause_watchdog(void)
{
    if(((inb(EC_LPC_IO_BASE_ADDR + EC_WDT_CFG_REG_OFFSET) >> 7) & 0x1) == 1)
    {
        outb(WDT_CFG_WDT_CLEAR, (EC_LPC_IO_BASE_ADDR + EC_WDT_CFG_REG_OFFSET)); //Clear previous watchdog trigger report
        is_wdt_trigger = true;
    }
    
    return;
}

int check_if_reboot_cause_warm_reset(void)
{
    struct file *fp = NULL;
    int err = 0;
    char content[1024];

    fp = filp_open(IS_COLD_RESET_FILE, O_RDONLY, 0);
    if(IS_ERR(fp)) 
    {
        CPLD_ERR("Open %s failed\n", IS_COLD_RESET_FILE);
        err = PTR_ERR(fp);
    }
    else
    {
        filp_close(fp, NULL);
        return REBOOT_CAUSE_CPU_COLD_RESET;
    }

    fp = filp_open(PREVIOUS_REBOOT_CAUSE_FILE, O_RDONLY, 0);
    if(IS_ERR(fp)) 
    {
        CPLD_ERR("Open %s failed\n", PREVIOUS_REBOOT_CAUSE_FILE);

        err = PTR_ERR(fp);
        return -EBADF;
    }

    if(!(read_whole_file(fp, content)))
    {
        CPLD_ERR("Read %s failed\n", PREVIOUS_REBOOT_CAUSE_FILE);
        filp_close(fp, NULL);
        return -EBADF;
    }
    filp_close(fp, NULL);

    if(strstr(content, "Unknown"))
        return REBOOT_CAUSE_NON_HARDWARE;
    else if(strstr(content, "warm-reboot") || strstr(content, "fast-reboot"))
        return REBOOT_CAUSE_CPU_WARM_RESET;
    else if(strstr(content, "reboot")) //Reboot command is modified to cold-reset via CPU 0xcf9 reg.
        return REBOOT_CAUSE_CPU_COLD_RESET;
    else
        return -EINVAL;
}

#if 0
bool check_if_reboot_cause_bios(void)
{
    unsigned char val;

    val = inb(EC_LPC_IO_BASE_ADDR + EC_BIOS_SELECT_STATUS_REG_OFFSET);
    if((val == 0x03) || (val == 0x30))
        return true;

    return false;
}
#endif

int check_if_reboot_cause_cold_reset(void)
{
    unsigned char val;

    switch_cpld_reg_read(0, CPLD_REG_CPU_COOL_RST_RECORD_OFFSET, &val);

    if(((val >> 0) & 0x1) == 1)
    {
        return REBOOT_CAUSE_CPLD_CPU_PWR_CYCLE;
    }
    else if(((val >> 1) & 0x1) == 1)
    {
        return REBOOT_CAUSE_CPU_EC_NO_HB;
    }
    else if(((val >> 2) & 0x1) == 1)
    {
        return REBOOT_CAUSE_CPU_CPU_ERROR;
    }
    else if(((val >> 3) & 0x1) == 1)
    {
        return REBOOT_CAUSE_BMC_SHUTDOWN;
    }
    else
    {
        return -EINVAL;
    }
}

ssize_t drv_get_reboot_cause(char *buf)
{
    int ret;

    // REBOOT_CAUSE_WATCHDOG
    check_if_reboot_cause_watchdog();
    if(is_wdt_trigger == true)
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", REBOOT_CAUSE_WATCHDOG);
#else
	    return sprintf(buf, "%d\n", REBOOT_CAUSE_WATCHDOG);
#endif
    }

    ret = check_if_reboot_cause_cold_reset();
    if(ret == REBOOT_CAUSE_CPU_EC_NO_HB)
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", REBOOT_CAUSE_CPU_EC_NO_HB);
#else
	    return sprintf(buf, "%d\n", REBOOT_CAUSE_CPU_EC_NO_HB);
#endif
    }
    else if(ret == REBOOT_CAUSE_CPU_CPU_ERROR)
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", REBOOT_CAUSE_CPU_CPU_ERROR);
#else
	    return sprintf(buf, "%d\n", REBOOT_CAUSE_CPU_CPU_ERROR);
#endif
    }
    else if(ret == REBOOT_CAUSE_BMC_SHUTDOWN)
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", REBOOT_CAUSE_BMC_SHUTDOWN);
#else
	    return sprintf(buf, "%d\n", REBOOT_CAUSE_BMC_SHUTDOWN);
#endif
    }
    else if(ret == REBOOT_CAUSE_CPLD_CPU_PWR_CYCLE)
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", REBOOT_CAUSE_CPLD_CPU_PWR_CYCLE);
#else
	return sprintf(buf, "%d\n", REBOOT_CAUSE_CPLD_CPU_PWR_CYCLE);
#endif
    }

    // REBOOT_CAUSE_POWER_LOSS
    if(check_if_reboot_cause_power_loss() == true)
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", REBOOT_CAUSE_POWER_LOSS);
#else
	    return sprintf(buf, "%d\n", REBOOT_CAUSE_POWER_LOSS);
#endif
    }

    // REBOOT_CAUSE_CPU_WARM_RESET
    ret = check_if_reboot_cause_warm_reset();
    if(ret == REBOOT_CAUSE_NON_HARDWARE)
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", REBOOT_CAUSE_NON_HARDWARE);
#else
	    return sprintf(buf, "%d\n", REBOOT_CAUSE_NON_HARDWARE);
#endif
    }
    else if(ret == REBOOT_CAUSE_CPU_COLD_RESET)
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", REBOOT_CAUSE_CPU_COLD_RESET);
#else
	    return sprintf(buf, "%d\n", REBOOT_CAUSE_CPU_COLD_RESET);
#endif
    }
    else if(ret == REBOOT_CAUSE_CPU_WARM_RESET)
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", REBOOT_CAUSE_CPU_WARM_RESET);
#else
	    return sprintf(buf, "%d\n", REBOOT_CAUSE_CPU_WARM_RESET);
#endif
    }
#if 0
    // REBOOT_CAUSE_BIOS_RESET
    if(check_if_reboot_cause_bios() == true)
        return sprintf_s(buf, PAGE_SIZE, "%d\n", REBOOT_CAUSE_BIOS_RESET);
#endif

    // For S3IP always return success with NA for HW fail.
    ERROR_RETURN_NA(-1);
}

ssize_t drv_get_alias(unsigned int index, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", cpld_alias_name[index-1]);
#else
    return sprintf(buf, "%s\n", cpld_alias_name[index-1]);
#endif
}

ssize_t drv_get_type(unsigned int index, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", cpld_type[index-1]);
#else
    return sprintf(buf, "%s\n", cpld_type[index-1]);
#endif
}

ssize_t drv_get_hw_version(unsigned int index, char *buf)
{
    unsigned char maj_ver;
    unsigned char min_ver;
    unsigned char maj_val = 0;
    unsigned char min_val = 0;

    switch_cpld_reg_read(index-1, CPLD_MAJOR_VER_OFFSET, &maj_val);
    maj_ver = maj_val;

    switch_cpld_reg_read(index-1, CPLD_MINOR_VER_OFFSET, &min_val);
    min_ver = min_val;
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%02x.%02x\n", maj_ver, min_ver);
#else
    return sprintf(buf, "%02x.%02x\n", maj_ver, min_ver);
#endif
}

ssize_t drv_get_board_version(unsigned int index, char *buf)
{
    unsigned char board_ver;
    unsigned char board_val = 0;

    switch_cpld_reg_read(index-1, CPLD_BOARD_VER_OFFSET, &board_val);
    board_ver = board_val;
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "0x%02x\n", board_ver);
#else
    return sprintf(buf, "0x%02x\n", board_ver);
#endif
}

ssize_t drv_get_reg_test(unsigned int index, char *buf)
{
    unsigned char reg_val;
    switch_cpld_reg_read(index-1, PORTCPLD_REG_SCRATCH_PAD, &reg_val);
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "0x%x\n", reg_val);
#else
    return sprintf(buf, "0x%x\n", reg_val);
#endif
}
ssize_t drv_set_reg_test(unsigned int index, unsigned char data)
{
    int ret = 0;
    ret = switch_cpld_reg_write(index-1, SYSCPLD_REG_SCRATCH_PAD, data);

    return ret;
}

ssize_t drv_get_fmea_corrosion_status(unsigned int index, char *buf)
{
    ERROR_RETURN_NA(-1);
}

ssize_t drv_get_fmea_voltage_status(unsigned int index, char *buf)
{
    int ret;
    unsigned int status=0;
    unsigned char data;
    unsigned int result = 0;

    switch(index)
    {
        case SYS_CPLD:
            ret = switch_cpld_reg_read(SYS_CPLD, SYSCPLD_REG_PG_STATUS0, &data);
            if(ret < 0)
                return ret;

            if((data & SYSCPLD_REG_PG_STATUS0_MASK) != 0)
            {
                /* PG_STATUS:  0: OK, 1: Fail  */
                result = 1;
            }
            status |= data;

            ret = switch_cpld_reg_read(SYS_CPLD, SYSCPLD_REG_PG_STATUS1, &data);
            if(ret < 0)
                return ret;

            if((data & SYSCPLD_REG_PG_STATUS1_MASK) != 0)
            {
                /* PG_STATUS:  0: OK, 1: Fail  */
                result = 1;
            }
            status |= (data << 8);

            ret = switch_cpld_reg_read(SYS_CPLD, SYSCPLD_REG_PG_STATUS2, &data);
            if(ret < 0)
                return ret;

            if((data & SYSCPLD_REG_PG_STATUS2_MASK) != 0)
            {
                /* PG_STATUS:  0: OK, 1: Fail  */
                result = 1;
            }
            status |= (data << 16);
            break;
        case FAN_CPLD:
        case PORT_CPLD1:
        case PORT_CPLD2:
            ERROR_RETURN_NA(-1);
            break;
        default:
            return -EINVAL;
            break;
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d 0x%x\n", result, status);
#else
    return sprintf(buf, "%d 0x%x\n", result, status);
#endif
}

int clock_check_fun(u8 cpld_index, u8 reg_offset,u8 mask, unsigned char *reg_value)
{
    int ret;
    unsigned int result = 0;
    ret = switch_cpld_reg_read(cpld_index, reg_offset, reg_value);
    if(ret < 0)
        return ret;

    if((*reg_value & mask) != 0)
    {
        /* CLK:  0: OK, 1: Fail  */
        result = 1;
        /*clear CLK alarm*/
        ret = switch_cpld_reg_write(cpld_index, reg_offset, 0x3);
        ret = switch_cpld_reg_write(cpld_index, reg_offset, 0x0);
        if(ret < 0)
            return ret;
    }
    return result;
}

ssize_t drv_get_fmea_clock_status(unsigned int index, char *buf)
{
    u32 status = 0;
    unsigned char data;
    unsigned int result = 0;

    switch(index)
    {
        case SYS_CPLD:
            /*25M CLK*/
            result = clock_check_fun(SYS_CPLD,SYSCPLD_REG_25M_CLK,SYSCPLD_REG_25M_CLK_MASK,&data);
            status |= data;
            
            /*100M CLK*/
            result = result | clock_check_fun(SYS_CPLD,SYSCPLD_REG_100M_CLK,SYSCPLD_REG_100M_CLK_MASK,&data);
            status |= (data << 8);

            /*156M CLK*/
            result = result | clock_check_fun(SYS_CPLD,SYSCPLD_REG_156M_CLK,SYSCPLD_REG_156M_CLK_MASK,&data);
            status |= (data << 16);

            break;
        default:
            return -EINVAL;
            break;
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d 0x%x\n", result, status);
#else
    return sprintf(buf, "%d 0x%x\n", result, status);
#endif
}

ssize_t drv_get_fmea_battery_status(unsigned int index, char *buf)
{
    ERROR_RETURN_NA(-1);
}

int reset_check_fun(u8 cpld_index, u8 reg_offset,u8 mask, unsigned char *reg_value)
{
    int ret;
    unsigned int result = 0;
    ret = switch_cpld_reg_read(cpld_index, reg_offset, reg_value);
    if(ret < 0)
        return ret;

    if((*reg_value & mask) == 0)
    {
        /* PG_STATUS:  0: OK, 1: Fail  */
        result = 1;
    }
    return result;
}

ssize_t drv_get_fmea_reset(unsigned int index, char *buf)
{
    unsigned int status=0;
    unsigned char data;
    unsigned int result = 0;

    switch(index)
    {
        case SYS_CPLD:
            result = reset_check_fun(SYS_CPLD,SYSCPLD_REG_RESET_STATUS0,SYSCPLD_REG_RESET_STATUS0_MASK,&data);
            status |= data;

            result = result | reset_check_fun(SYS_CPLD,SYSCPLD_REG_RESET_STATUS1,SYSCPLD_REG_RESET_STATUS1_MASK,&data);
            status |= (data << 8);

            result = result | reset_check_fun(SYS_CPLD,SYSCPLD_REG_RESET_STATUS2,SYSCPLD_REG_RESET_STATUS2_MASK,&data);
            status |= (data << 16);
            
            result = result | reset_check_fun(SYS_CPLD,SYSCPLD_REG_RESET_STATUS3,SYSCPLD_REG_RESET_STATUS3_MASK,&data);
            status |= (data << 24);
            break;
        case FAN_CPLD:
            ERROR_RETURN_NA(-1);
            break;
        case PORT_CPLD1:
        case PORT_CPLD2:
            result = reset_check_fun(SYS_CPLD,PORTCPLD_REG_RESET_STATUS0,PORTCPLD_REG_RESET_STATUS0_MASK,&data);
            status |= data;

            result = result | reset_check_fun(SYS_CPLD,PORTCPLD_REG_RESET_STATUS1,PORTCPLD_REG_RESET_STATUS1_MASK,&data);
            status |= (data << 8);
            break;
        default:
            return -EINVAL;
            break;
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d 0x%x\n", result, status);
#else
    return sprintf(buf, "%d 0x%x\n", result, status);
#endif
}

ssize_t drv_get_fmea_interrupt(unsigned int index, char *buf)
{
    ERROR_RETURN_NA(-1);
}

int set_reset_fun(u8 cpld_index, u8 reg_offset,u8 reg_value)
{
    int ret;
    ret = switch_cpld_reg_write(cpld_index, reg_offset, reg_value);
    if(ret < 0)
        return ret;
    return ret;
}

bool drv_set_fmea_reset(unsigned int index, int reset)
{
    switch(index)
    {
        case SYS_CPLD:
            set_reset_fun(SYS_CPLD,SYSCPLD_REG_RESET_STATUS0,reset);
            set_reset_fun(SYS_CPLD,SYSCPLD_REG_RESET_STATUS1,reset);
            set_reset_fun(SYS_CPLD,SYSCPLD_REG_RESET_STATUS2,reset);
            set_reset_fun(SYS_CPLD,SYSCPLD_REG_RESET_STATUS3,reset);
            break;
        case FAN_CPLD:
            return -EINVAL;
        case PORT_CPLD1:
        case PORT_CPLD2:
            set_reset_fun(SYS_CPLD,PORTCPLD_REG_RESET_STATUS0,reset);
            set_reset_fun(SYS_CPLD,PORTCPLD_REG_RESET_STATUS1,reset);
            break;
        default:
            return -EINVAL;
            break;
    }
    return true;
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
        "busybox devmem <reg> 8\n"
        "i2cget -f -y <bus_num> <dev_addr> <reg>\n"
        "sysname                debug cmd\n"
        "reboot_cause           busybox devmem 0xfc801020 8\n"
        "                       inb 0x6308 --hex\n"
        "                       busybox devmem 0xfc80102A 8\n"
        "                       cat /host/reboot-cause/previous-reboot-cause.json\n"
        "cpld1 board_version    busybox devmem 0xfc800000 8\n"
        "cpld1 hw_version       busybox devmem 0xfc800001 8\n"
        "                       busybox devmem 0xfc800002 8\n"
        "                       i2cget -f -y 633 0x62 0x01\n"
        "                       i2cget -f -y 633 0x62 0x02\n"
        "                       i2cget -f -y 634 0x64 0x01\n"
        "                       i2cget -f -y 634 0x64 0x02\n"
        );
#else
    return sprintf(buf,
        "Use the following command to debug:\n"
        "busybox devmem <reg> 8\n"
        "i2cget -f -y <bus_num> <dev_addr> <reg>\n"
        "sysname                debug cmd\n"
        "reboot_cause           busybox devmem 0xfc801020 8\n"
        "                       inb 0x6308 --hex\n"
        "                       busybox devmem 0xfc80102A 8\n"
        "                       cat /host/reboot-cause/previous-reboot-cause.json\n"
        "cpld1 board_version    busybox devmem 0xfc800000 8\n"
        "cpld1 hw_version       busybox devmem 0xfc800001 8\n"
        "                       busybox devmem 0xfc800002 8\n"
        "                       i2cget -f -y 633 0x62 0x01\n"
        "                       i2cget -f -y 633 0x62 0x02\n"
        "                       i2cget -f -y 634 0x64 0x01\n"
        "                       i2cget -f -y 634 0x64 0x02\n"
        );
#endif
}

ssize_t drv_debug(const char *buf, int count)
{
    return 0;
}

int switch_cpld_reg_read(u8 cpld_index, u8 reg_offset, u8 *reg_value)
{
    int ret = 0;

    switch(cpld_index)
    {
        case SYS_CPLD:
            ret = switch_system_cpld_read(reg_offset, reg_value);
            break;
        case FAN_CPLD:
            ret = fan_cpld_reg_read_func(cpld_index, reg_offset, reg_value);
            break;
        case PORT_CPLD1:
        case PORT_CPLD2:
            ret = switch_i2c_cpld_read(cpld_index, reg_offset, reg_value);
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
}
EXPORT_SYMBOL_GPL(switch_cpld_reg_read);


int switch_cpld_reg_write(u8 cpld_index, u8 reg_offset, u8 reg_value)
{
    int ret = 0;

    switch(cpld_index)
    {
        case SYS_CPLD:
            ret = switch_system_cpld_write(reg_offset, reg_value);
            break;
        case FAN_CPLD:
            ret = fan_cpld_reg_write_func(cpld_index, reg_offset, reg_value);
            break;
        case PORT_CPLD1:
        case PORT_CPLD2:
            ret = switch_i2c_cpld_write(cpld_index, reg_offset, reg_value);
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
}
EXPORT_SYMBOL_GPL(switch_cpld_reg_write);

// For s3ip
static struct cpld_drivers_t pfunc = {
    .get_reboot_cause = drv_get_reboot_cause,
    .get_alias = drv_get_alias,
    .get_type = drv_get_type,
    .get_hw_version = drv_get_hw_version,
    .get_board_version = drv_get_board_version,
    .get_fmea_selftest_status = drv_get_reg_test,
    .set_reg_test = drv_set_reg_test,
    .get_fmea_corrosion_status = drv_get_fmea_corrosion_status,
    .get_fmea_voltage_status = drv_get_fmea_voltage_status,
    .get_fmea_clock_status = drv_get_fmea_clock_status,
    .get_fmea_battery_status = drv_get_fmea_battery_status,
    .get_fmea_reset = drv_get_fmea_reset,
    .set_fmea_reset = drv_set_fmea_reset,
    .get_fmea_interrupt = drv_get_fmea_interrupt,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
    .debug_help = drv_debug_help,
};

#if 1 // For user space API uages
/* file_operations: ioctl */
struct cpld_access {
    /* intput output buff */
    unsigned char   cs;
    unsigned char   offset;
    unsigned char   value;
};

#define CPLD_MAGIC      0xA7
#define CPLD_IO_CMD     0x11

#define CPLD_IO_READ   _IOR(CPLD_MAGIC, CPLD_IO_CMD,   struct cpld_access)
#define CPLD_IO_WRITE  _IOW(CPLD_MAGIC, CPLD_IO_CMD,   struct cpld_access)
#endif

long cpld_ioctl(struct file *filp, unsigned int cmd, unsigned long user_addr)
{
    struct cpld_access acc = {0};
    int retval = 0;

    if (cmd == CPLD_IO_READ)
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
        if (!access_ok(VERIFY＿READ,user_addr, _IOC_SIZE(cmd)))
#else
        if (!access_ok(user_addr, _IOC_SIZE(cmd)))
#endif        
        {
            return -EINVAL;
        }

        retval = copy_from_user((void *) &acc, (const void *) user_addr, sizeof(struct cpld_access));
        mutex_lock(&cpld_sem);
        switch_cpld_reg_read(acc.cs, acc.offset, &acc.value);
        mutex_unlock(&cpld_sem);
        retval = copy_to_user((void *) user_addr, (const void *) &acc, sizeof(struct cpld_access));
        return retval;
    }
    else if (cmd == CPLD_IO_WRITE)
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
        if (!access_ok(VERIFY＿WRITE,user_addr, _IOC_SIZE(cmd)))
#else
        if (!access_ok(user_addr, _IOC_SIZE(cmd)))
#endif 
        {
            return -EINVAL;
        }

        retval = copy_from_user((void *) &acc, (const void *) user_addr, sizeof(struct cpld_access));
        mutex_lock(&cpld_sem);
        switch_cpld_reg_write(acc.cs, acc.offset, acc.value);
        mutex_unlock(&cpld_sem);
        retval = copy_to_user((void *) user_addr, (const void *) &acc, sizeof(struct cpld_access));
        return(retval);
    }
    else
    {
        CPLD_ERR("[%s] Unknown command\n", __func__);
        return -EINVAL;
    }
}

static const struct file_operations cpld_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = cpld_ioctl,
};

static int drv_cpld_probe(struct platform_device *pdev)
{
    struct cpld_drvdata *drvdata = NULL;
    unsigned char bmc_exist = 0, bmc_work = 0;
    int retval = 0;

    drvdata = kzalloc(sizeof(struct cpld_drvdata), GFP_KERNEL);
    if (!drvdata) {
        retval = -ENOMEM;
        goto failed0;
    }
    platform_set_drvdata(pdev, (void *)drvdata);

    mutex_lock(&cpld_sem);
    mutex_init(&drvdata->sem);

    /* Request the kernel for N_MINOR devices */
    retval = alloc_chrdev_region(&drvdata->devt, 0, 1, CPLD_DEV_NAME);
    if (retval)
    {
        dev_err(&pdev->dev, "alloc_chrdev_region() failed\n");
        goto failed1;
    }

    /* Initialize and create each of the device(cdev) */
    cdev_init(&drvdata->cpld_cdev, &cpld_fops);
    drvdata->cpld_cdev.owner = THIS_MODULE;
    retval = cdev_add(&drvdata->cpld_cdev, drvdata->devt, 1);
    if (retval)
    {
        dev_err(&pdev->dev, "cdev_add() failed\n");
        goto failed2;
    }

    /* Create a class : appears at /sys/class */
    drvdata->cpld_class = class_create(THIS_MODULE, CPLD_CLASS_NAME);
    if (IS_ERR(drvdata->cpld_class))
    {
        dev_err(&pdev->dev, "class_create() failed\n");
        retval = PTR_ERR(drvdata->cpld_class);
        goto failed3;
    }

    /* Create a device node for this device in /dev */
    drvdata->cpld_udev = device_create(drvdata->cpld_class, NULL, drvdata->devt, NULL, CPLD_DEV_NAME);
    if (IS_ERR(drvdata->cpld_udev))
    {
        dev_err(&pdev->dev, "device_create() failed\n");
        retval = PTR_ERR(drvdata->cpld_udev);
        goto failed4;
    }

    s3ip_cpld_drivers_register(&pfunc);
    switch_cpld_reg_read(SYS_CPLD, SYSCPLD_REG_PRSNT_STATUS, &bmc_exist);
    switch_cpld_reg_read(SYS_CPLD, SYSCPLD_REG_CORROSION_STATUS, &bmc_work);

    /* skip heartbeat condition before BMC heartbeat ready */
    if(!(bmc_exist & BMC_PRESENT_BIT_OFFSET))
    {
        fan_cpld_reg_read_func = &drv_cpld_read_from_bmc;
        fan_cpld_reg_write_func = &drv_cpld_write_from_bmc;
    }
    else
    {
        fan_cpld_reg_read_func = &switch_i2c_cpld_read;
        fan_cpld_reg_write_func = &switch_i2c_cpld_write;
    }
    return 0;


failed4:
    class_destroy(drvdata->cpld_class);

failed3:
    cdev_del(&drvdata->cpld_cdev);

failed2:
    unregister_chrdev_region(drvdata->devt, 1);

failed1:
    kfree(drvdata);

failed0:
    return retval;
}

static int drv_cpld_remove(struct platform_device *pdev)
{
    struct cpld_drvdata *drvdata;

    s3ip_cpld_drivers_unregister();

    drvdata = platform_get_drvdata(pdev);
    if (!drvdata)
        return 0;

    device_destroy(drvdata->cpld_class, drvdata->devt);
    class_destroy(drvdata->cpld_class);
    cdev_del(&drvdata->cpld_cdev);
    unregister_chrdev_region(drvdata->devt, 1);

    platform_set_drvdata(pdev, NULL);
    kfree(drvdata);

    return 0;
}

static struct platform_driver drv_cpld_driver = {
    .driver = {
        .name   = DRVNAME,
        .owner = THIS_MODULE,
    },
    .probe      = drv_cpld_probe,
    .remove     = drv_cpld_remove,
};

static int __init drv_cpld_init(void)
{
    int err=0;
    int retval=0;

    drv_cpld_device = platform_device_alloc(DRVNAME, 0);
    if(!drv_cpld_device)
    {
        CPLD_DEBUG("%s(#%d): platform_device_alloc fail\n", __func__, __LINE__);
        return -ENOMEM;
    }

    retval = platform_device_add(drv_cpld_device);
    if(retval)
    {
        CPLD_DEBUG("%s(#%d): platform_device_add failed\n", __func__, __LINE__);
        err = -retval;
        goto dev_add_failed;
    }

    retval = platform_driver_register(&drv_cpld_driver);
    if(retval)
    {
        CPLD_DEBUG("%s(#%d): platform_driver_register failed(%d)\n", __func__, __LINE__, err);
        err = -retval;
        goto dev_reg_failed;
    }

    return 0;

dev_reg_failed:
    platform_device_unregister(drv_cpld_device);
    return err;

dev_add_failed:
    platform_device_put(drv_cpld_device);
    return err;
}

static void __exit drv_cpld_exit(void)
{
    platform_driver_unregister(&drv_cpld_driver);
    platform_device_unregister(drv_cpld_device);

    return;
}

MODULE_DESCRIPTION("S3IP CPLD Driver");
MODULE_VERSION(SWITCH_CPLD_DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_init(drv_cpld_init);
module_exit(drv_cpld_exit);
