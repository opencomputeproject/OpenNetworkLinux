/*
 * A hwmon driver for the Wedeg100bf-32x/64x
 *
 * Copyright (C) 2018 Accton Technology Corporation.
 * Roy Lee <roy_lee@accton.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * This driver retrieves sensors' data through uart interface by opening
 * an user-space tty device node.
 * Designed pariticular for ethernet switch with BMC, and verified
 * at model accton_wedge100bf_65x and accton_wedge100bf_32x.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <asm/uaccess.h>


//#define DEBUG

#ifdef DEBUG
#define DEBUG_INTR(fmt, ...)	pr_err(fmt, ##__VA_ARGS__)
#else
#define DEBUG_INTR(fmt...)	do { } while (0)
#endif
#define DEBUG_LEX(fmt, ...)	    do { } while (0)


#define DRVNAME "wedge_psensor"     /*Platform Sensor*/
enum mtype {
    MTYPE_BF100_65X = 0,
    MTYPE_BF100_32X = 1,
    MTYPE_MAX,
};

#define SENSOR_DATA_UPDATE_INTERVAL     (5*HZ)
#define MAX_THERMAL_COUNT (7)
#define MAX_FAN_COUNT     (10)
#define CHASSIS_PSU_CHAR_COUNT     (2)    /*2 for input and output.*/
#define CHASSIS_LED_COUNT     (2)
#define CHASSIS_PSU_VOUT_COUNT     (1)    /*V output only.*/
#define CHASSIS_PSU_VOUT_INDEX     (1)    /*V output start index.*/

#define BF100_65X_THERMAL_COUNT  MAX_THERMAL_COUNT
#define BF100_65X_FAN_COUNT      MAX_FAN_COUNT

#define BF100_32X_THERMAL_COUNT  (7)
#define BF100_32X_FAN_COUNT      MAX_FAN_COUNT



#define ATTR_ALLOC_EXTRA	        1   /*For last attribute which is NUll.*/
#define ATTR_NAME_SIZE		        24
#define ATTR_NAME_OUTFIT_SIZE		12
#define ATTR_MAX_LIST   		    8

#define TTY_DEVICE                      "/dev/ttyACM0"
#define TTY_PROMPT                      "@bmc"
#define TTY_USER                        "root"
#define TTY_PASSWORD                    "0penBmc"
#define TTY_BAUDRATE                    (57600)
#define TTY_I2C_TIMEOUT                 800000
#define TTY_BMC_LOGIN_TIMEOUT           1000000
#define TTY_LOGIN_RETRY                 20
#define TTY_LOGIN_RETRY_INTV            (300)
#define TTY_RETRY_INTERVAL              (50)  /*mini-seconds*/
#define MAXIMUM_TTY_BUFFER_LENGTH       1024
#define MAXIMUM_TTY_STRING_LENGTH       (MAXIMUM_TTY_BUFFER_LENGTH - 1)
#define TTY_CMD_RETRY                   3
#define TTY_RX_RETRY                    3
#define TTY_CMD_MAX_LEN          (64)
#define TTY_READ_MAX_LEN        (256)

#define TTY_RESP_SEPARATOR      '|'     /*For the ease to debug*/
#define MAX_ATTR_PATTERN        (8)
#define MIN_FAN_RPM             (0)
#define MAX_PSU_VOUT            (12000*1005/1000) /*12 + 0.5%*/
#define MIN_PSU_VOUT            (12000*995/1000)    /*12 - 0.5%*/

enum sensor_type {
    SENSOR_TYPE_THERMAL_IN = 0,
    SENSOR_TYPE_THERMAL_MAX,
    SENSOR_TYPE_THERMAL_MAX_HYST,
    SENSOR_TYPE_FAN_RPM,
    SENSOR_TYPE_FAN_RPM_DOWN,
    SENSOR_TYPE_PSU1,
    SENSOR_TYPE_PSU2,
    SENSOR_TYPE_MAX,
};


/* Pmbus reg defines are copied from drivers/hwmon/pmbus/pmbus.h*/
#define PMBUS_READ_VIN			0x88
#define PMBUS_READ_IIN			0x89
#define PMBUS_READ_VOUT			0x8B
#define PMBUS_READ_IOUT			0x8C
#define PMBUS_READ_POUT			0x96
#define PMBUS_READ_PIN			0x97

#define PMBUS_REG_START  PMBUS_READ_VIN
#define PMBUS_REG_END    PMBUS_READ_PIN

enum psu_data {
    PSU_DATA_VIN = 0,
    PSU_DATA_VOUT,
    PSU_DATA_IIN,
    PSU_DATA_IOUT,
    PSU_DATA_PIN,
    PSU_DATA_POUT,
    PSU_DATA_MAX,
};

struct pmbus_reg {
    u8   addr;
    bool power;     /*power attribute is in unit of uW instead of mW.*/
} pmbus_regs[] = {
    [PSU_DATA_VIN ] = {PMBUS_READ_VIN, false},
    [PSU_DATA_VOUT] = {PMBUS_READ_VOUT, false},
    [PSU_DATA_IIN ] = {PMBUS_READ_IIN, false},
    [PSU_DATA_IOUT] = {PMBUS_READ_IOUT, false},
    [PSU_DATA_PIN ] = {PMBUS_READ_PIN, true},
    [PSU_DATA_POUT] = {PMBUS_READ_POUT, true},
};


struct sensor_data {
    int lm75_input[MAX_THERMAL_COUNT];
    int lm75_max[MAX_THERMAL_COUNT];
    int lm75_max_hyst[MAX_THERMAL_COUNT];
    int fan_rpm[MAX_FAN_COUNT];
    int fan_rpm_dn[MAX_FAN_COUNT];
    int psu1_data [PSU_DATA_MAX];
    int psu2_data [PSU_DATA_MAX];
    int led_bright[CHASSIS_LED_COUNT];
};

enum sysfs_attributes_index {
    INDEX_VERSION,
    INDEX_NAME,
    INDEX_THRM_IN_START = 100,
    INDEX_THRM_MAX_START = 150,
    INDEX_THRM_MAX_HYST_START = 170,
    INDEX_FAN_RPM_START = 200,
    INDEX_FAN_RPM_START_DN  = 300,
    INDEX_PSU1_START    = 400,
    INDEX_PSU2_START    = 500,
};

struct wedge_sensor {
    struct wedge_sensor *next;
    char name[ATTR_NAME_SIZE+1];	/* sysfs sensor name */
    struct sensor_device_attribute sensor_dev_attr;
};

struct wedge100_data {
    struct platform_device *pdev;
    struct device	    *dev;
    struct device	    *hwmon_dev;
    struct mutex	    update_lock;
    struct tty_struct   *tty;
    struct ktermios     old_ktermios;
    bool			 valid[SENSOR_TYPE_MAX];
    unsigned long	 last_updated[SENSOR_TYPE_MAX];	  /* In jiffies */
    struct sensor_data sdata;
    int num_attributes;
    struct attribute_group group;
};

typedef ssize_t (*show_func)( struct device *dev,
                              struct device_attribute *attr, char *buf);
typedef ssize_t (*store_func)(struct device *dev,
                              struct device_attribute *attr, const char *buf,
                              size_t count);
static ssize_t show_name(struct device *dev, struct device_attribute *da,
                         char *buf);
static ssize_t show_thermal(struct device *dev, struct device_attribute *da,
                            char *buf);
static ssize_t show_thermal_max(struct device *dev, struct device_attribute *da,
                                char *buf);
static ssize_t show_thermal_max_hyst(struct device *dev, struct device_attribute *da,
                                     char *buf);
static ssize_t show_fan(struct device *dev, struct device_attribute *da,
                        char *buf);
static ssize_t show_fan_dn(struct device *dev, struct device_attribute *da,
                           char *buf);
static ssize_t show_fan_min(struct device *dev, struct device_attribute *da,
                            char *buf);
static ssize_t show_psu1(struct device *dev, struct device_attribute *da,
                         char *buf);
static ssize_t show_psu2(struct device *dev, struct device_attribute *da,
                         char *buf);
static ssize_t show_psu_vout_min(struct device *dev, struct device_attribute *da,
                                 char *buf);
static ssize_t show_psu_vout_max(struct device *dev, struct device_attribute *da,
                                 char *buf);
static int add_attr2group(struct wedge100_data *data, struct attribute *attr);


struct attr_pattern {
    int  count;              /*How many attr of this pattern*/
    int  index_addon;        /* For attribute naming, e.g.
                              * X of "fanX_input",start from a number but 0.
                              */
    char prefix[ATTR_NAME_OUTFIT_SIZE+1];        /*e.g. "temp"4_input*/
    char postfix[ATTR_NAME_OUTFIT_SIZE+1];       /*e.g. temp4"_input"*/
    umode_t     mode;
    show_func   show;
    store_func  store;
};

struct sensor_set {
    int  total;                             /*How many attr of this type*/
    int  start_index;
    int  ptn_cnt;
    struct attr_pattern ptn[MAX_ATTR_PATTERN];
};

struct sensor_set ss_w100_65x[SENSOR_TYPE_MAX] =
{
    [SENSOR_TYPE_THERMAL_IN] = {
        BF100_65X_THERMAL_COUNT, INDEX_THRM_IN_START,
        1,
        {   [0] = {BF100_65X_THERMAL_COUNT, 0, "temp","_input",
                S_IRUGO, show_thermal, NULL
            },
        }
    },
    [SENSOR_TYPE_THERMAL_MAX] = {
        BF100_65X_THERMAL_COUNT, INDEX_THRM_MAX_START,
        1,
        {   [0] = {BF100_65X_THERMAL_COUNT, 0, "temp","_max",
                S_IRUGO, show_thermal_max, NULL
            },
        }
    },
    [SENSOR_TYPE_THERMAL_MAX_HYST] = {
        BF100_65X_THERMAL_COUNT, INDEX_THRM_MAX_HYST_START,
        1,
        {   [0] = {BF100_65X_THERMAL_COUNT, 0, "temp","_max_hyst",
                S_IRUGO, show_thermal_max_hyst, NULL
            },
        }
    },
    [SENSOR_TYPE_FAN_RPM] = {
        BF100_65X_FAN_COUNT, INDEX_FAN_RPM_START,
        2,
        {   [0] = {BF100_65X_FAN_COUNT, 0, "fan","_input",
                S_IRUGO, show_fan, NULL
            },
            [1] = {
                BF100_65X_FAN_COUNT, 0, "fan","_min",
                S_IRUGO, show_fan_min, NULL
            },
        }
    },
    [SENSOR_TYPE_FAN_RPM_DOWN] = {
        BF100_65X_FAN_COUNT, INDEX_FAN_RPM_START_DN,
        2,
        {   [0] = {BF100_65X_FAN_COUNT, MAX_FAN_COUNT, "fan","_input",
                S_IRUGO, show_fan_dn, NULL
            },
            [1] = {
                BF100_65X_FAN_COUNT, MAX_FAN_COUNT, "fan","_min",
                S_IRUGO, show_fan_min, NULL
            },
        }
    },
    [SENSOR_TYPE_PSU1] = {
        PSU_DATA_MAX, INDEX_PSU1_START,
        5,
        {   [0] ={CHASSIS_PSU_CHAR_COUNT, 0, "in","_input",
                S_IRUGO, show_psu1, NULL
            },
            [1] ={
                CHASSIS_PSU_CHAR_COUNT, 0, "curr","_input",
                S_IRUGO, show_psu1, NULL
            },
            [2] ={
                CHASSIS_PSU_CHAR_COUNT, 0, "power","_input",
                S_IRUGO, show_psu1, NULL
            },
            [3] ={
                CHASSIS_PSU_VOUT_COUNT, CHASSIS_PSU_VOUT_INDEX, "in","_max",
                S_IRUGO, show_psu_vout_max, NULL
            },
            [4] ={
                CHASSIS_PSU_VOUT_COUNT, CHASSIS_PSU_VOUT_INDEX, "in","_min",
                S_IRUGO, show_psu_vout_min, NULL
            },
        }
    },
    [SENSOR_TYPE_PSU2] = {
        PSU_DATA_MAX, INDEX_PSU2_START,
        5,
        {   [0] ={CHASSIS_PSU_CHAR_COUNT, CHASSIS_PSU_CHAR_COUNT, "in","_input",
                S_IRUGO, show_psu2, NULL
            },
            [1] ={
                CHASSIS_PSU_CHAR_COUNT, CHASSIS_PSU_CHAR_COUNT, "curr","_input",
                S_IRUGO, show_psu2, NULL
            },
            [2] ={
                CHASSIS_PSU_CHAR_COUNT, CHASSIS_PSU_CHAR_COUNT, "power","_input",
                S_IRUGO, show_psu2, NULL
            },
            [3] ={
                CHASSIS_PSU_VOUT_COUNT,
                CHASSIS_PSU_CHAR_COUNT+CHASSIS_PSU_VOUT_INDEX, "in","_max",
                S_IRUGO, show_psu_vout_max, NULL
            },
            [4] ={
                CHASSIS_PSU_VOUT_COUNT,
                CHASSIS_PSU_CHAR_COUNT+CHASSIS_PSU_VOUT_INDEX, "in","_min",
                S_IRUGO, show_psu_vout_min, NULL
            },
        }
    },
};

struct sensor_set ss_w100_32x[SENSOR_TYPE_MAX] =
{
    [SENSOR_TYPE_THERMAL_IN] = {
        BF100_32X_THERMAL_COUNT, INDEX_THRM_IN_START,
        1,
        {   [0] = {BF100_32X_THERMAL_COUNT, 0, "temp","_input",
                S_IRUGO, show_thermal, NULL
            },
        }
    },
    [SENSOR_TYPE_THERMAL_MAX] = {
        BF100_65X_THERMAL_COUNT, INDEX_THRM_MAX_START,
        1,
        {   [0] = {BF100_65X_THERMAL_COUNT, 0, "temp","_max",
                S_IRUGO, show_thermal_max, NULL
            },
        }
    },
    [SENSOR_TYPE_THERMAL_MAX_HYST] = {
        BF100_65X_THERMAL_COUNT, INDEX_THRM_MAX_HYST_START,
        1,
        {   [0] = {BF100_65X_THERMAL_COUNT, 0, "temp","_max_hyst",
                S_IRUGO, show_thermal_max_hyst, NULL
            },
        }
    },
    [SENSOR_TYPE_FAN_RPM] = {
        BF100_32X_FAN_COUNT, INDEX_FAN_RPM_START,
        2,
        {   [0] = {BF100_32X_FAN_COUNT, 0, "fan","_input",
                S_IRUGO, show_fan, NULL
            },
            [1] = {
                BF100_65X_FAN_COUNT, 0, "fan","_min",
                S_IRUGO, show_fan_min, NULL
            },
        }
    },
    [SENSOR_TYPE_FAN_RPM_DOWN] = {0},
    [SENSOR_TYPE_PSU1] = {
        PSU_DATA_MAX, INDEX_PSU1_START,
        5,
        {   [0] ={CHASSIS_PSU_CHAR_COUNT, 0, "in","_input",
                S_IRUGO, show_psu1, NULL
            },
            [1] ={
                CHASSIS_PSU_CHAR_COUNT, 0, "curr","_input",
                S_IRUGO, show_psu1, NULL
            },
            [2] ={
                CHASSIS_PSU_CHAR_COUNT, 0, "power","_input",
                S_IRUGO, show_psu1, NULL
            },
            [3] ={
                CHASSIS_PSU_VOUT_COUNT, CHASSIS_PSU_VOUT_INDEX, "in","_max",
                S_IRUGO, show_psu_vout_max, NULL
            },
            [4] ={
                CHASSIS_PSU_VOUT_COUNT, CHASSIS_PSU_VOUT_INDEX, "in","_min",
                S_IRUGO, show_psu_vout_min, NULL
            },
        }
    },
    [SENSOR_TYPE_PSU2] = {
        PSU_DATA_MAX, INDEX_PSU2_START,
        5,
        {   [0] ={CHASSIS_PSU_CHAR_COUNT, CHASSIS_PSU_CHAR_COUNT, "in","_input",
                S_IRUGO, show_psu2, NULL
            },
            [1] ={
                CHASSIS_PSU_CHAR_COUNT, CHASSIS_PSU_CHAR_COUNT, "curr","_input",
                S_IRUGO, show_psu2, NULL
            },
            [2] ={
                CHASSIS_PSU_CHAR_COUNT, CHASSIS_PSU_CHAR_COUNT, "power","_input",
                S_IRUGO, show_psu2, NULL
            },
            [3] ={
                CHASSIS_PSU_VOUT_COUNT,
                CHASSIS_PSU_CHAR_COUNT+CHASSIS_PSU_VOUT_INDEX, "in","_max",
                S_IRUGO, show_psu_vout_max, NULL
            },
            [4] ={
                CHASSIS_PSU_VOUT_COUNT,
                CHASSIS_PSU_CHAR_COUNT+CHASSIS_PSU_VOUT_INDEX, "in","_min",
                S_IRUGO, show_psu_vout_min, NULL
            },
        }
    },
};

struct sensor_set *model_ssets[SENSOR_TYPE_MAX] = {ss_w100_65x, ss_w100_32x};

static char tty_cmd[SENSOR_TYPE_MAX][TTY_CMD_MAX_LEN] = {
    "cat /sys/bus/i2c/devices/[38]-004*/temp1_input\r\n",
    "cat /sys/bus/i2c/devices/[38]-004*/temp1_max\r\n",
    "cat /sys/bus/i2c/devices/[38]-004*/temp1_max_hyst\r\n",
    "ls -v /sys/bus/i2c/devices/8-0033/fan*_input | xargs cat\r\n",
    "ls -v /sys/bus/i2c/devices/9-0033/fan*_input | xargs cat\r\n",
    "i2cset -y -f 7 0x70 0 2; i2cdump -y -f -r "\
    __stringify(PMBUS_REG_START)"-" __stringify(PMBUS_REG_END)\
    " 7 0x59 w\r\n",
    "i2cset -y -f 7 0x70 0 1; i2cdump -y -f -r "\
    __stringify(PMBUS_REG_START)"-" __stringify(PMBUS_REG_END)\
    " 7 0x5a w\r\n",
};

static struct wedge100_data *wedge_data = NULL;


/* Specify which model_id is engaged. Default is BF100_65X. */
static int model_id = MTYPE_BF100_65X;
module_param(model_id, uint, S_IRUGO);
MODULE_PARM_DESC(model_id, "Default is BF100_65X.");

static int _tty_wait(struct file *tty_fd, int mdelay) {
    msleep(mdelay);
    return 0;
}

static int _tty_open(struct file **fd)
{
    struct ktermios kt;
    struct tty_struct *tty;
    speed_t baudrate = 57600;
    struct file *tty_fd = *fd;

    /*TTY must be not opened.*/
    if (tty_fd != NULL) {
        return -EINVAL;
    }

    tty_fd = filp_open(TTY_DEVICE, O_RDWR|O_NOCTTY|O_NDELAY, 0644);
    if (IS_ERR(tty_fd)) {
        DEBUG_INTR("Failed to open file(%s)\r\n", TTY_DEVICE);
        return -ENOENT;
    } else {
        tty = ((struct tty_file_private *)tty_fd ->private_data)->tty;
        wedge_data->old_ktermios = (tty->termios);

        kt = tty->termios;
        tty_termios_encode_baud_rate(&kt, baudrate, baudrate);
        kt.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
        kt.c_iflag = IGNPAR;
        kt.c_oflag = 0;
        kt.c_lflag = 0;
        kt.c_cc[VMIN] = (unsigned char)
                        ((MAXIMUM_TTY_STRING_LENGTH > 0xFF) ?
                         0xFF : MAXIMUM_TTY_STRING_LENGTH);
        kt.c_cc[VTIME] = 0;
        tty_set_termios(tty, &kt);

        wedge_data->tty = tty ;
        *fd = tty_fd;
        return 0;
    }
    return -ENXIO;
}

static int _tty_close(struct file **tty_fd)
{
    if(*tty_fd == NULL) {
        return -EINVAL;
    }
    filp_close(*tty_fd, 0);
    *tty_fd = NULL;

    return 0;
}

static int _tty_tx(struct file *tty_fd, const char *str)
{
    int rc;

    /*Sanity check*/
    if (tty_fd == NULL)
        return -EINVAL;
    if(!(tty_fd->f_op) || !(tty_fd->f_op->read) ||!(tty_fd->f_op->write)) {
        return -EINVAL;
    }

    rc = tty_fd->f_op->write(tty_fd, str, strlen(str)+1,0);
    if (rc < 0) {
        pr_err( "failed to write(%d)\n", rc);
        return -EBUSY;
    }
    DEBUG_INTR("[TX]%s-%d, %d BYTES, write:\n\"%s\"\n", __func__, __LINE__,rc, str);
    return rc;
}

static int _tty_rx(struct file *tty_fd, char *buf, int max_len)
{
    int rc;
    u32 timeout = 0;

    /*Sanity check*/
    if (tty_fd == NULL)
        return -EINVAL;
    if(!(tty_fd->f_op) || !(tty_fd->f_op->read) ||!(tty_fd->f_op->write)) {
        return -EINVAL;
    }

    /*Clear for remained data cause ambiguous string*/
    memset(buf, 0, max_len);
    do {
        rc = tty_fd->f_op->read(tty_fd, buf, max_len, 0);
        if (rc == 0) {  /*Buffer Empty, waits. */
            timeout++;
            msleep(TTY_RETRY_INTERVAL);
            continue;
        } else {
            break;
        }
    } while (rc < 0 && timeout < TTY_RX_RETRY);

    if (timeout == TTY_RX_RETRY) {
        rc = -EAGAIN;
    }
    DEBUG_INTR("[RX]%s-%d, %d BYTES, read:\n\"%s\"\n", __func__, __LINE__,rc, buf);
    return rc;
}

/*Clear Rx buffer by reading it out.*/
static int _tty_clear_rxbuf(struct file *tty_fd, char* buf, size_t max_size) {
    int rc;
    mm_segment_t old_fs;

    if (tty_fd == NULL) {
        return -EINVAL;
    }
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    do {
        _tty_wait(tty_fd, 0);
        rc = tty_fd->f_op->read(tty_fd, buf, max_size, 0);
        memset(buf, 0, max_size);
    } while (rc > 0);

    set_fs(old_fs);
    return rc;
}

static int _tty_writeNread(struct file *tty_fd,
                           char *wr_p, char *rd_p, int rx_max_len, u32 mdelay)
{
    int     rc;
    mm_segment_t old_fs;

    /*Presumed file is opened!*/
    if (tty_fd == NULL)
        return -EINVAL;

    if(!(tty_fd->f_op) || !(tty_fd->f_op->read) ||!(tty_fd->f_op->write)) {
        pr_err("file %s cann't readable or writable?\n", TTY_DEVICE);
        return -EINVAL;
    }

    memset(rd_p, 0, rx_max_len);
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    rc = _tty_tx(tty_fd, wr_p);
    if (rc < 0) {
        printk( "failed to write(%d)\n", rc);
        goto exit;
    }
    _tty_wait(tty_fd, mdelay);
    rc = _tty_rx(tty_fd, rd_p, rx_max_len);
    if (rc < 0) {
        printk( "failed to read(%d)\n", rc);
        goto exit;
    }

exit:
    set_fs(old_fs);
    if(rc < 0) {
        dev_err(wedge_data->dev, "Failed on %s ret:%d\n", __func__, rc);
    }
    return rc;
}

static bool _is_logged_in(char *buf)
{
    DEBUG_INTR("%s-%d, tty_buf:%s\n", __func__, __LINE__, buf);
    /*Check if logined by compare BMC's cmd prompt.*/
    if (strstr(buf, TTY_PROMPT) != NULL) {
        return true;
    } else {
        return false;
    }
}

static int _tty_login(struct file *tty_fd, char* buf, size_t max_size)
{
    int i, ret;

    if(!tty_fd)
        return -EINVAL;

    for (i = 1; i <= TTY_LOGIN_RETRY; i++) {
        ret = _tty_writeNread(tty_fd, "\r\r", buf, max_size, TTY_RETRY_INTERVAL);
        if (ret < 0) {
            DEBUG_INTR("%s-%d, failed ret:%d\n", __func__, __LINE__, ret);
            continue;
        }
        if (_is_logged_in(buf))
            return 0;

        DEBUG_INTR("%s-%d, tty_buf:%s\n", __func__, __LINE__, buf);
        if ((strstr(buf, "bmc") != NULL) &&
                (strstr(buf, "login:") != NULL))
        {
            ret = _tty_writeNread(tty_fd, TTY_USER"\r",
                                  buf, max_size, TTY_LOGIN_RETRY_INTV);
            if (ret < 0) {
                DEBUG_INTR("%s-%d, failed ret:%d\n", __func__, __LINE__, ret);
                continue;
            }
            DEBUG_INTR("%s-%d, tty_buf:%s\n", __func__, __LINE__, buf);
            if (strstr(buf, "Password:") != NULL) {
                DEBUG_INTR("%s-%d, tty_buf:%s\n", __func__, __LINE__, buf);
                ret = _tty_writeNread(tty_fd, TTY_PASSWORD"\r", buf, max_size, 0);
                if (ret < 0) {
                    DEBUG_INTR("%s-%d, failed ret:%d\n", __func__, __LINE__, ret);
                    continue;
                }
                if (strstr(buf, TTY_PROMPT) != NULL) {
                    DEBUG_INTR("%s-%d, tty_buf:%s\n", __func__, __LINE__, buf);
                    return 0;
                }
            }
        }
        msleep(TTY_RETRY_INTERVAL);
    }

    dev_err(wedge_data->dev, "Failed on %s ret:%d\n", __func__, ret);
    return -EAGAIN;
}

static int
bmc_transaction(char *cmd, char* resp, int max)
{
    u32  i;
    struct file *tty_fd = NULL;
    char *buf;
    int buf_size = MAXIMUM_TTY_BUFFER_LENGTH;
    int ret = 0;

    if(!cmd || !resp)
        return -EINVAL;

    if (_tty_open(&tty_fd) != 0) {
        DEBUG_INTR("ERROR: Cannot open TTY device\n");
        return -EAGAIN;
    }

    buf = (char *)kmalloc(buf_size, GFP_KERNEL);
    if (!buf) {
        ret = -ENOMEM;
        goto exit;
    }

    _tty_clear_rxbuf(tty_fd, buf, buf_size);
    if (_tty_login(tty_fd, buf, buf_size) != 0) {
        dev_err(wedge_data->dev, "Failed to login TTY device\n");
        _tty_close(&tty_fd);
        ret = -ENOENT;
        goto exit;
    }

    i = 0;
    do {
        ret = _tty_writeNread(tty_fd, cmd, buf, buf_size, 200);
        if (ret < 0) {
            goto exit;
        }
        i++;
    } while(strstr(buf, TTY_PROMPT) == NULL && i <= TTY_CMD_RETRY);
    if (i > TTY_CMD_RETRY) {
        dev_err(wedge_data->dev, "Failed on tty_transaction\n");
        ret = -ENOENT;
        goto exit;
    }

    strncpy(resp, buf, max);
exit:
    kfree(buf);
    _tty_close(&tty_fd);
    return ret;
}


static void dev_attr_init(struct device_attribute *dev_attr,
                          const char *name, umode_t mode,
                          show_func show, store_func store)
{
    sysfs_attr_init(&dev_attr->attr);
    dev_attr->attr.name = name;
    dev_attr->attr.mode = mode;
    dev_attr->show = show;
    dev_attr->store = store;
}

/*Allowcat sensor_device_attributes and adds them to a group.*/
static int attributs_init(struct wedge100_data *data)
{
    struct sensor_set *model = model_ssets[model_id];
    char name[64] = {0};
    int start, pi, si, ai, num, ret, acc;
    struct wedge_sensor *sensor;
    struct sensor_device_attribute *sensor_dattr;
    struct device_attribute *dev_attr;

    /*name*/
    sensor = devm_kzalloc(data->dev, sizeof(*sensor), GFP_KERNEL);
    if (!sensor)
        return -ENOENT;
    sensor_dattr = &sensor->sensor_dev_attr;
    dev_attr = &sensor_dattr->dev_attr;
    snprintf(sensor->name, sizeof(sensor->name), "name");
    dev_attr_init(dev_attr, sensor->name, S_IRUGO, show_name, NULL);
    sensor_dattr->index = INDEX_NAME;
    ret = add_attr2group(data, &dev_attr->attr);
    if (ret)
        return ret;

    /*types*/
    for (si = 0; si < SENSOR_TYPE_MAX; si++)
    {
        struct sensor_set *ss = model+si;
        int ptn_cnt = ss->ptn_cnt;
        num = ss->total;
        start = ss->start_index;
        acc = start;
        for (pi = 0; pi < ptn_cnt; pi++)
        {
            struct attr_pattern  *ptn = &ss->ptn[pi];
            char *prefix, *postfix;
            prefix = ptn->prefix;
            postfix = ptn->postfix;
            for (ai = 0; ai < ptn->count ; ai++) {
                u32 ptn_index =  ai + ptn->index_addon + 1 ;
                snprintf(name, sizeof(name), "%s%d%s",prefix, ptn_index, postfix);
                sensor = devm_kzalloc(data->dev, sizeof(*sensor), GFP_KERNEL);
                if (!sensor)
                    return -ENOENT;

                sensor_dattr = &sensor->sensor_dev_attr;
                dev_attr = &sensor_dattr->dev_attr;
                snprintf(sensor->name, sizeof(sensor->name), name);
                dev_attr_init(dev_attr, sensor->name, ptn->mode,
                              ptn->show, ptn->store);
                sensor_dattr->index = acc + ai;

                DEBUG_INTR("%s-%d, name:%s index:%d\n", __func__, __LINE__,
                           name, sensor_dattr->index);
                ret = add_attr2group(data, &dev_attr->attr);
                if (ret)
                    return ret;
            }
            acc += ptn->count;
        }
    }

    return 0 ;
}
static void wedge_data_init(struct wedge100_data *data)
{

}

static int extract_numbers(char *buf, int *out, int out_cnt)
{
    char *ptr;
    int  cnt, x;

    ptr = buf;
    DEBUG_LEX("%s-%d, out_cnt (%d)\n", __func__, __LINE__,  out_cnt);
    /*replace non-digits into '|', for sscanf(%s)'s ease to handle it.*/
    for (x = 0; x < strlen(ptr); x++) {
        if( (ptr[x]<'0' || ptr[x] >'9') && ptr[x] != '-')
            ptr[x] = TTY_RESP_SEPARATOR;
    }

    DEBUG_LEX("%s-%d, (%lu) resp:%s\n", __func__, __LINE__, strlen(ptr), ptr);
    cnt = 0;
    while (strchr(ptr, TTY_RESP_SEPARATOR))
    {
        if (sscanf(ptr,"%d%s",out,ptr)) {
            DEBUG_LEX("%s-%d,  %d @(%d)\n", __func__, __LINE__,  *(out), cnt);
            cnt++;
            out++;
            if (cnt == out_cnt) {
                return 0;
            }
        } else {
            ptr++;
        }
    }

    return  -EINVAL;
}

static int extract_2byteHex(char *buf, int *out, int out_cnt)
{
    char tmp[TTY_READ_MAX_LEN];
    char *ptr;
    int  cnt, x;
    long y;

    ptr = buf;
    /*replace non-hex into '|', for sscanf(%s)'s ease to handle it.*/
    for (x = 0; x < strlen(ptr); x++) {
        if((ptr[x]>='0' && ptr[x] <= '9') || (ptr[x] >= 'a' && ptr[x] <= 'f'))
            ;
        else
            ptr[x] = TTY_RESP_SEPARATOR;
    }

    DEBUG_LEX("%s-%d, (%lu) resp:%s\n", __func__, __LINE__, strlen(ptr), ptr);
    cnt = 0;
    while (strchr(ptr, TTY_RESP_SEPARATOR))
    {
        ptr++;
        if (strlen(ptr) && *ptr == TTY_RESP_SEPARATOR) {
            continue;
        }
        DEBUG_LEX("%s-%d, (%lu) :%s\n", __func__, __LINE__, strlen(ptr), ptr);
        if (sscanf(ptr,"%4s",tmp)) {
            if(!strchr(tmp, TTY_RESP_SEPARATOR)) {
                DEBUG_LEX("%s-%d,  %s @(%d)\n", __func__, __LINE__,  tmp, cnt);
                if (kstrtol(tmp, 16, &y) < 0)
                    return -EINVAL;
                *out = (int)y;
                cnt++;
                out++;
                if (cnt == out_cnt) {
                    return 0;
                }
                ptr += 4;
            }
        } else {
            break;
        }
    }

    return  -EINVAL;
}

int pmbus_linear11(int in, bool power) {
    int exponent;
    int mantissa, val;
    int data = in;

    exponent = ((short)data) >> 11;
    mantissa = ((short)((data & 0x7ff) << 5)) >> 5;

    val = mantissa*1000;
    if (power)
        val *= 1000;
    if (exponent >= 0)
        val <<= exponent;
    else
        val >>= -exponent;

    return val;
}

static int get_pmbus_regs_partial(int *in, int in_cnt, int *out, int *out_cnt)
{
    int i, j, k;
    k = 0;
    for (j = 0; j < ARRAY_SIZE(pmbus_regs); j++) {
        for (i = 0; i < in_cnt; i++) {
            if (i == (pmbus_regs[j].addr - PMBUS_REG_START))  {
                *out = pmbus_linear11(in[i], pmbus_regs[j].power);
                out++;
                k++;
                break;
            }
        }
    }
    if (k == 0)
        return -EINVAL;

    *out_cnt = k;
    return 0;
}


static int comm2BMC(enum sensor_type type, int *out, int out_cnt)
{
    char cmd[TTY_CMD_MAX_LEN], resp[TTY_READ_MAX_LEN];
    char *ptr;
    int ret;

    if (out == NULL)
        return -EINVAL;
    if (out_cnt == 0)
        return 0;

    snprintf(cmd, sizeof(cmd), tty_cmd[type]);
    DEBUG_INTR("%s-%d, cmd:%s\n", __func__, __LINE__, cmd);
    ret = bmc_transaction(cmd, resp, sizeof(resp)-1);
    if (ret < 0)
        return ret;

    /*Strip off string of command just sent, if any.*/
    if (strstr(resp, cmd) != NULL) {
        ptr = resp + strlen(cmd);
    } else {
        ptr = resp;
    }

    switch (type) {
    case SENSOR_TYPE_THERMAL_IN:
    case SENSOR_TYPE_THERMAL_MAX:
    case SENSOR_TYPE_THERMAL_MAX_HYST:
    case SENSOR_TYPE_FAN_RPM:
    case SENSOR_TYPE_FAN_RPM_DOWN:
        ret = extract_numbers(ptr, out, out_cnt);
        break;
    case SENSOR_TYPE_PSU1:
    case SENSOR_TYPE_PSU2:
    {
        int reg[(PMBUS_REG_END - PMBUS_REG_START) +1];
        int total = (PMBUS_REG_END - PMBUS_REG_START) +1;
        ret = extract_2byteHex(ptr, reg, total);
        get_pmbus_regs_partial(reg, total, out, &out_cnt);
        break;
    }
    default:
        return -EINVAL;
    }
    if (ret)
        return ret;
    return 0;
}

static int get_type_data (
    struct sensor_data *data, enum sensor_type type, int index,
    int **out, int *count)
{
    struct sensor_set *model = model_ssets[model_id];

    switch (type) {
    case SENSOR_TYPE_THERMAL_IN:
        *out = &data->lm75_input[index];
        break;
    case SENSOR_TYPE_THERMAL_MAX:
        *out = &data->lm75_max[index];
        break;
    case SENSOR_TYPE_THERMAL_MAX_HYST:
        *out = &data->lm75_max_hyst[index];
        break;
    case SENSOR_TYPE_FAN_RPM:
        *out = &data->fan_rpm[index];
        break;
    case SENSOR_TYPE_FAN_RPM_DOWN:
        *out = &data->fan_rpm_dn[index];
        break;
    case SENSOR_TYPE_PSU1:
        *out = &data->psu1_data[index];
        break;
    case SENSOR_TYPE_PSU2:
        *out = &data->psu2_data[index];
        break;
    default:
        return -EINVAL;
    }
    *count = model[type].total;
    return 0;
}

static struct sensor_data*
update_data(struct device *dev, enum sensor_type type) {
    struct wedge100_data *data = wedge_data;
    bool			*valid = &data->valid[type];
    unsigned long   *last_updated = &data->last_updated[type];
    struct sensor_data* ret = NULL;
    int *data_ptr = NULL;
    int data_cnt, rc;

    mutex_lock(&data->update_lock);
    if (time_after(jiffies, (*last_updated) + SENSOR_DATA_UPDATE_INTERVAL)
            || !(*valid))
    {
        rc = get_type_data(&data->sdata, type, 0, &data_ptr, &data_cnt);
        if (rc < 0)
            goto exit_err;

        DEBUG_INTR("%s-%d, type:%d cnt:%d\n", __func__, __LINE__, type, data_cnt);
        rc = comm2BMC(type, data_ptr, data_cnt);
        if (rc < 0) {
            *valid = 0;
            /*Clear data if failed.*/
            if (data_ptr != NULL)
                memset(data_ptr, 0, sizeof(*data_ptr)*data_cnt);
        } else {
            *valid = 1;
        }
        *last_updated = jiffies;
    }
    ret =  &data->sdata;
exit_err:
    mutex_unlock(&data->update_lock);
    return ret;
}

static ssize_t show_name(struct device *dev, struct device_attribute *da,
                         char *buf)
{
    return sprintf(buf, "%s\n", DRVNAME);
}

static ssize_t _attr_show(struct device *dev, struct device_attribute *da,
                          char *buf, enum sensor_type type,  int index_start)
{
    int index, count, rc;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct sensor_data* data;
    int *out = NULL;

    DEBUG_INTR("%s-%d, type:%d start:%d\n", __func__, __LINE__, type, index_start);
    data = update_data(dev, type);
    if (data == NULL)
        return -EINVAL;

    index = attr->index - index_start;
    rc = get_type_data(data, type, index, &out, &count);
    if (rc < 0 || out == NULL)
        return -EINVAL;

    if( index > count)
        return -EINVAL;

    return sprintf(buf, "%d\n",  *out);
}

static ssize_t show_thermal(struct device *dev, struct device_attribute *da,
                            char *buf)
{
    return _attr_show(dev, da, buf,
                      SENSOR_TYPE_THERMAL_IN, INDEX_THRM_IN_START);
}
static ssize_t show_thermal_max(struct device *dev, struct device_attribute *da,
                                char *buf)
{
    return _attr_show(dev, da, buf,
                      SENSOR_TYPE_THERMAL_MAX, INDEX_THRM_MAX_START);
}
static ssize_t show_thermal_max_hyst(struct device *dev,
                                     struct device_attribute *da,  char *buf)
{
    return _attr_show(dev, da, buf,
                      SENSOR_TYPE_THERMAL_MAX_HYST, INDEX_THRM_MAX_HYST_START);
}
static ssize_t show_fan(struct device *dev, struct device_attribute *da,
                        char *buf)
{
    return _attr_show(dev, da, buf,
                      SENSOR_TYPE_FAN_RPM, INDEX_FAN_RPM_START);
}

static ssize_t show_fan_min(struct device *dev, struct device_attribute *da,
                            char *buf)
{
    return sprintf(buf, "%d\n",  MIN_FAN_RPM);
}

static ssize_t show_fan_dn(struct device *dev, struct device_attribute *da,
                           char *buf)
{
    return _attr_show(dev, da, buf,
                      SENSOR_TYPE_FAN_RPM_DOWN, INDEX_FAN_RPM_START_DN);
}

static ssize_t show_psu1(struct device *dev, struct device_attribute *da,
                         char *buf)
{
    return _attr_show(dev, da, buf,
                      SENSOR_TYPE_PSU1, INDEX_PSU1_START);
}

static ssize_t show_psu2(struct device *dev, struct device_attribute *da,
                         char *buf)
{
    return _attr_show(dev, da, buf,
                      SENSOR_TYPE_PSU2, INDEX_PSU2_START);
}

static ssize_t show_psu_vout_max(struct device *dev, struct device_attribute *da,
                                 char *buf)
{
    return sprintf(buf, "%d\n",  MAX_PSU_VOUT);
}

static ssize_t show_psu_vout_min(struct device *dev, struct device_attribute *da,
                                 char *buf)
{
    return sprintf(buf, "%d\n",  MIN_PSU_VOUT);
}

static int add_attr2group(struct wedge100_data *data, struct attribute *attr)
{
    int new_max_attrs = ++data->num_attributes + ATTR_ALLOC_EXTRA;
    void *new_attrs = krealloc(data->group.attrs,
                               new_max_attrs * sizeof(void *),
                               GFP_KERNEL);
    DEBUG_INTR("%s-%d, num_attr:%d\n", __func__, __LINE__, new_max_attrs);
    if (!new_attrs)
        return -ENOMEM;
    data->group.attrs = new_attrs;


    data->group.attrs[data->num_attributes-1] = attr;
    data->group.attrs[data->num_attributes] = NULL;

    return 0;
}

static int wedge100_probe(struct platform_device *pdev)
{
    int status = -1;

    wedge_data->dev = &pdev->dev;
    status = attributs_init(wedge_data);
    DEBUG_INTR("%s-%d, status:%d\n", __func__, __LINE__, status);
    if (status) {
        goto exit;
    }

    /* Register sysfs hooks */
    status = sysfs_create_group(&pdev->dev.kobj, &wedge_data->group);
    DEBUG_INTR("%s-%d, status:%d\n", __func__, __LINE__, status);

    if (status) {
        goto exit_kfree;
    }

    wedge_data->hwmon_dev = hwmon_device_register(&pdev->dev);
    if (IS_ERR(wedge_data->hwmon_dev)) {
        status = PTR_ERR(wedge_data->hwmon_dev);
        goto exit_remove;
    }
    dev_info(&pdev->dev, "wedge100bf sensors found\n");
    return 0;

exit_remove:
    sysfs_remove_group(&pdev->dev.kobj, &wedge_data->group);
exit_kfree:
    kfree(wedge_data->group.attrs);
exit:
    return status;
}

static int wedge100_remove(struct platform_device *pdev)
{
    hwmon_device_unregister(wedge_data->hwmon_dev);
    sysfs_remove_group(&pdev->dev.kobj, &wedge_data->group);
    kfree(wedge_data->group.attrs);
    return 0;
}

static struct platform_driver wedge100_driver = {
    .probe		= wedge100_probe,
    .remove		= wedge100_remove,
    .driver		= {
        .name	= DRVNAME,
        .owner	= THIS_MODULE,
    },
};


static int __init wedge100_init(void)
{
    int ret;

    ret = platform_driver_register(&wedge100_driver);
    if (ret < 0) {
        goto exit;
    }
    wedge_data = kzalloc(sizeof(struct wedge100_data), GFP_KERNEL);
    if (!wedge_data) {
        ret = -ENOMEM;
        platform_driver_unregister(&wedge100_driver);
        goto exit;
    }
    mutex_init(&wedge_data->update_lock);
    wedge_data_init(wedge_data);

    wedge_data->pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
    if (IS_ERR(wedge_data->pdev)) {
        ret = PTR_ERR(wedge_data->pdev);
        platform_driver_unregister(&wedge100_driver);
        kfree(wedge_data);
        goto exit;
    }
exit:
    return ret;
}

static void __exit wedge100_exit(void)
{
    if (!wedge_data) {
        return;
    }
    platform_device_unregister(wedge_data->pdev);
    platform_driver_unregister(&wedge100_driver);
    kfree(wedge_data);
}

module_init(wedge100_init);
module_exit(wedge100_exit);

MODULE_AUTHOR("Roy Lee <roy_lee@accton.com>");
MODULE_DESCRIPTION("wedge100bf platform sensors driver");
MODULE_LICENSE("GPL");
