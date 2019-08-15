#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/dmi.h>
#include <linux/version.h>
#include <linux/ctype.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c/pca954x.h>
#include <linux/i2c-mux.h>
#include <linux/i2c-mux-gpio.h>
#include <linux/i2c/sff-8436.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#define CPLD_A_REG  0x33
#define CPLD_B_REG  0x32
#define BUS3_MUX_REG  0x11
#define BUS8_MUX_REG  0x19

#define QSFP_P0_VAL  0x04
#define QSFP_P1_VAL  0x03
#define QSFP_P2_VAL  0x02
#define QSFP_P3_VAL  0x00

#define SFP_PRESENCE_1  0x02
#define SFP_PRESENCE_2  0x03
#define SFP_PRESENCE_3  0x04
#define SFP_PRESENCE_4  0x05
#define SFP_PRESENCE_5  0x06
#define SFP_PRESENCE_6  0x07
#define QSFP_PRESENCE_1 0x0d
#define QSFP_LP_MODE_1  0x0c
#define QSFP_RESET_1    0x0e

#define DEFAULT_DISABLE 0x00
#define QSFP_DEFAULT_DISABLE 0x01

#define BUS3_DEV_NUM  4
#define BUS8_DEV_NUM  48
#define DEFAULT_NUM   1
#define BUS3_BASE_NUM  20
#define BUS8_BASE_NUM  30

#define ak7448_i2c_device_num(NUM){                                           \
        .name                   = "delta-ak7448-i2c-device",                  \
        .id                     = NUM,                                        \
        .dev                    = {                                           \
                    .platform_data = &ak7448_i2c_device_platform_data[NUM],   \
                    .release       = device_release,                          \
        },                                                                    \
}

/* Check cpld read results */
#define VALIDATED_READ(_buf, _rv, _read, _invert)		\
	do {							\
		_rv = _read;					\
		if (_rv < 0) {					\
			return sprintf(_buf, "READ ERROR\n");	\
		}						\
		if (_invert) {					\
			_rv = ~_rv;				\
		}						\
		_rv &= 0xFF;					\
	} while(0)						\


/*Define struct to get client of i2c_new_deivce */
struct i2c_client * i2c_client_9548;

enum{
    BUS0 = 0,
    BUS1,
    BUS2,
    BUS3,
    BUS4,
    BUS5,
    BUS6,
    BUS7,
    BUS8,
};

long sfp_port_data = 0;

static LIST_HEAD(cpld_client_list);

struct cpld_client_node
{
  struct i2c_client *client;
  struct list_head   list;
};

int i2c_cpld_read(int bus, unsigned short cpld_addr, u8 reg)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EPERM;

    list_for_each(list_node, &cpld_client_list){
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if ((cpld_node->client->adapter->nr == bus) && (cpld_node->client->addr == cpld_addr) ){
            ret = i2c_smbus_read_byte_data(cpld_node->client, reg);
            break;
        }
    }
    return ret;
}
EXPORT_SYMBOL(i2c_cpld_read);

int i2c_cpld_write(int bus, unsigned short cpld_addr, u8 reg, u8 value)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EIO;

    list_for_each(list_node, &cpld_client_list){
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if ((cpld_node->client->adapter->nr == bus) && (cpld_node->client->addr == cpld_addr) ){
            ret = i2c_smbus_write_byte_data(cpld_node->client, reg, value);
            break;
        }
    }

    return ret;
}
EXPORT_SYMBOL(i2c_cpld_write);

/*----------------   I2C device   - start   ------------- */
static void device_release(struct device *dev)
{
    return;
}

struct i2c_device_platform_data {
    int parent;
    struct i2c_board_info           info;
    struct i2c_client              *client;
};

static struct i2c_device_platform_data ak7448_i2c_device_platform_data[] = {
    {
        /* qsfp 1 (0x50) */
        .parent = 20,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 2 (0x50) */
        .parent = 21,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 3 (0x50) */
        .parent = 22,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 4 (0x50) */
        .parent = 23,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 1 (0x50) */
        .parent = 30,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 2 (0x50) */
        .parent = 31,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 3 (0x50) */
        .parent = 32,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 4 (0x50) */
        .parent = 33,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 5 (0x50) */
        .parent = 34,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 6 (0x50) */
        .parent = 35,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 7 (0x50) */
        .parent = 36,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 8 (0x50) */
        .parent = 37,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 9 (0x50) */
        .parent = 38,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 10 (0x50) */
        .parent = 39,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 11 (0x50) */
        .parent = 40,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 12 (0x50) */
        .parent = 41,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 13 (0x50) */
        .parent = 42,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 14 (0x50) */
        .parent = 43,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 15 (0x50) */
        .parent = 44,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 16 (0x50) */
        .parent = 45,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 17 (0x50) */
        .parent = 46,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 18 (0x50) */
        .parent = 47,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 19 (0x50) */
        .parent = 48,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 20 (0x50) */
        .parent = 49,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 21 (0x50) */
        .parent = 50,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 22 (0x50) */
        .parent = 51,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 23 (0x50) */
        .parent = 52,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 24 (0x50) */
        .parent = 53,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 25 (0x50) */
        .parent = 54,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 26 (0x50) */
        .parent = 55,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 27 (0x50) */
        .parent = 56,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 28 (0x50) */
        .parent = 57,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 29 (0x50) */
        .parent = 58,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 30 (0x50) */
        .parent = 59,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 31 (0x50) */
        .parent = 60,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 32 (0x50) */
        .parent = 61,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 33 (0x50) */
        .parent = 62,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 34 (0x50) */
        .parent = 63,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 35 (0x50) */
        .parent = 64,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 36 (0x50) */
        .parent = 65,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 37 (0x50) */
        .parent = 66,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 38 (0x50) */
        .parent = 67,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 39 (0x50) */
        .parent = 68,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 40 (0x50) */
        .parent = 69,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 41 (0x50) */
        .parent = 70,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 42 (0x50) */
        .parent = 71,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 43 (0x50) */
        .parent = 72,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 44 (0x50) */
        .parent = 73,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 45 (0x50) */
        .parent = 74,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 46 (0x50) */
        .parent = 75,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 47 (0x50) */
        .parent = 76,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp 48 (0x50) */
        .parent = 77,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
};


static struct platform_device ak7448_i2c_device[] = {
    ak7448_i2c_device_num(0),
    ak7448_i2c_device_num(1),
    ak7448_i2c_device_num(2),
    ak7448_i2c_device_num(3),
    ak7448_i2c_device_num(4),
    ak7448_i2c_device_num(5),
    ak7448_i2c_device_num(6),
    ak7448_i2c_device_num(7),
    ak7448_i2c_device_num(8),
    ak7448_i2c_device_num(9),
    ak7448_i2c_device_num(10),
    ak7448_i2c_device_num(11),
    ak7448_i2c_device_num(12),
    ak7448_i2c_device_num(13),
    ak7448_i2c_device_num(14),
    ak7448_i2c_device_num(15),
    ak7448_i2c_device_num(16),
    ak7448_i2c_device_num(17),
    ak7448_i2c_device_num(18),
    ak7448_i2c_device_num(19),
    ak7448_i2c_device_num(20),
    ak7448_i2c_device_num(21),
    ak7448_i2c_device_num(22),
    ak7448_i2c_device_num(23),
    ak7448_i2c_device_num(24),
    ak7448_i2c_device_num(25),
    ak7448_i2c_device_num(26),
    ak7448_i2c_device_num(27),
    ak7448_i2c_device_num(28),
    ak7448_i2c_device_num(29),
    ak7448_i2c_device_num(30),
    ak7448_i2c_device_num(31),
    ak7448_i2c_device_num(32),
    ak7448_i2c_device_num(33),
    ak7448_i2c_device_num(34),
    ak7448_i2c_device_num(35),
    ak7448_i2c_device_num(36),
    ak7448_i2c_device_num(37),
    ak7448_i2c_device_num(38),
    ak7448_i2c_device_num(39),
    ak7448_i2c_device_num(40),
    ak7448_i2c_device_num(41),
    ak7448_i2c_device_num(42),
    ak7448_i2c_device_num(43),
    ak7448_i2c_device_num(44),
    ak7448_i2c_device_num(45),
    ak7448_i2c_device_num(46),
    ak7448_i2c_device_num(47),
    ak7448_i2c_device_num(48),
    ak7448_i2c_device_num(49),
    ak7448_i2c_device_num(50),
    ak7448_i2c_device_num(51),
};
 
/*----------------   I2C device   - end   ------------- */

/*----------------   I2C driver   - start   ------------- */
static int __init i2c_device_probe(struct platform_device *pdev)
{
    struct i2c_device_platform_data *pdata;
    struct i2c_adapter *parent;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "Missing platform data\n");
        return -ENODEV;
    }

    parent = i2c_get_adapter(pdata->parent);
    if (!parent) {
        dev_err(&pdev->dev, "Parent adapter (%d) not found\n",
            pdata->parent);
        return -ENODEV;
    }

    pdata->client = i2c_new_device(parent, &pdata->info);
    if (!pdata->client) {
        dev_err(&pdev->dev, "Failed to create i2c client %s at %d\n",
            pdata->info.type, pdata->parent);
        return -ENODEV;
    }

    return 0;
}

static int __exit i2c_deivce_remove(struct platform_device *pdev)
{
    struct i2c_adapter *parent;
    struct i2c_device_platform_data *pdata;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "Missing platform data\n");
        return -ENODEV;
    }

    if (pdata->client) {
        parent = i2c_get_adapter(pdata->parent);
        i2c_unregister_device(pdata->client);
        i2c_put_adapter(parent);
    }

    return 0;
}
static struct platform_driver i2c_device_driver = {
    .probe = i2c_device_probe,
    .remove = __exit_p(i2c_deivce_remove),
    .driver = {
        .owner = THIS_MODULE,
        .name = "delta-ak7448-i2c-device",
    }
};

/*----------------   I2C driver   - end   ------------- */

/*----------------    CPLD - start   ------------- */

/*    CPLD  -- device   */

enum cpld_type {
    cpld_a,
    cpld_b,
};

struct cpld_platform_data {
    int reg_addr;
    struct i2c_client *client;
};

static struct cpld_platform_data ak7448_cpld_platform_data[] = {
    [cpld_a] = {
        .reg_addr = CPLD_A_REG,
    },
    [cpld_b] = {
        .reg_addr = CPLD_B_REG,
    },
};

static struct platform_device ak7448_cpld = {
    .name               = "delta-ak7448-cpld",
    .id                 = 0,
    .dev                = {
                .platform_data   = ak7448_cpld_platform_data,
                .release         = device_release
    },
};

static ssize_t for_status(struct device *dev, struct device_attribute *dev_attr, char *buf);

enum ak7448_sfp_sysfs_attributes 
{
  SFP_SELECT_PORT,
  SFP_IS_PRESENT,
  SFP_IS_PRESENT_ALL,
  SFP_LP_MODE,
  SFP_RESET,
  ADDR,
  DATA,
};

static struct kobject *kobj_swpld;

static ssize_t set_w_port_data(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);
    struct device *i2cdev = kobj_to_dev(kobj_swpld);
    struct cpld_platform_data *pdata = i2cdev->platform_data;
    long data;
    int error;
    long port_t = 0;
    u8 reg_t = 0x00;

    error = kstrtol(buf, 10, &data);
    if(error)
        return error;

    /* Disable SFP channel */
    if (i2c_smbus_write_byte_data(pdata[cpld_b].client, BUS8_MUX_REG, (u8)DEFAULT_DISABLE) < 0) {
        return -EIO;
    }

    /* Disable QSFP channel */
    if (i2c_smbus_write_byte_data(pdata[cpld_b].client, BUS3_MUX_REG, (u8)QSFP_DEFAULT_DISABLE) < 0) {
        return -EIO;
    }

    sfp_port_data = data;	
    return count;
}

static ssize_t for_r_port_data(struct device *dev, struct device_attribute *dev_attr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);
    struct device *i2cdev = kobj_to_dev(kobj_swpld);
    struct cpld_platform_data *pdata = i2cdev->platform_data;

    if (sfp_port_data == DEFAULT_DISABLE) 
    {
        /* Disable SFP channel */
        if (i2c_smbus_write_byte_data(pdata[cpld_b].client, BUS8_MUX_REG, (u8)DEFAULT_DISABLE) < 0) {
            return -EIO;
        }

        /* Disable QSFP channel */
        if (i2c_smbus_write_byte_data(pdata[cpld_b].client, BUS3_MUX_REG, (u8)QSFP_DEFAULT_DISABLE) < 0) {
            return -EIO;
        }
    }

    return sprintf(buf, "%d\n", sfp_port_data);

}

static ssize_t set_w_lp_mode_data(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);
    struct device *i2cdev = kobj_to_dev(kobj_swpld);
    struct cpld_platform_data *pdata = i2cdev->platform_data;
    long data;
    int error;
    long port_t = 0;
    int bit_t = 0x00;
    int values = 0x00;

    error = kstrtol(buf, 10, &data);
    if (error)
        return error;

    port_t = sfp_port_data;
	
    if (port_t > 48 && port_t < 53) 
    {   /* QSFP Port 48-53 */
        values = i2c_smbus_read_byte_data(pdata[cpld_b].client, QSFP_LP_MODE_1);
        if (values < 0)
            return -EIO;

		/* Indicate the module is in LP mode or not
		 * 0 = Disable
		 * 1 = Enable
		 */
        if (data == 0) 
        {
            bit_t = ~(1 << ((port_t - 1) % 8));
            values = values & bit_t;
        }
        else if (data == 1) 
        {
            bit_t = 1 << ((port_t - 1) % 8);
            values = values | bit_t;
        } 
        else  
        {
            return -EINVAL;
        }

        if (i2c_smbus_write_byte_data(pdata[cpld_b].client, QSFP_LP_MODE_1, (u8)values) < 0) 
        {
            return -EIO;
        }
    }

    return count;
}

static ssize_t set_w_reset_data(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);
    struct device *i2cdev = kobj_to_dev(kobj_swpld);
    struct cpld_platform_data *pdata = i2cdev->platform_data;
    long data;
    int error;
    long port_t = 0;
    int bit_t = 0x00;
    int values = 0x00;

    error = kstrtol(buf, 10, &data);
    if (error)
        return error;

    port_t = sfp_port_data;
	
    if (port_t > 48 && port_t < 53) 
    { /* QSFP Port 48-53 */
        values = i2c_smbus_read_byte_data(pdata[cpld_b].client, QSFP_RESET_1);
        if (values < 0)
            return -EIO;

		/* Indicate the module Reset or not
		 * 0 = Reset
		 * 1 = Normal
		 */
        if (data == 0) 
        {
            bit_t = ~(1 << ((port_t - 1) % 8));
            values = values & bit_t;
        } 
        else if (data == 1) 
        {
            bit_t = 1 << ((port_t - 1) % 8);
            values = values | bit_t;
        } 
        else 
        {
            return -EINVAL;
        }

        if (i2c_smbus_write_byte_data(pdata[cpld_b].client, QSFP_RESET_1, (u8)values) < 0) 
        {
            return -EIO;
        }
    }

    return count;
}
	
static ssize_t for_status(struct device *dev, struct device_attribute *dev_attr, char *buf){
    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);
    struct device *i2cdev = kobj_to_dev(kobj_swpld);
    struct cpld_platform_data *pdata = i2cdev->platform_data;
    long port_t = 0;
    u8 reg_t = 0x00;
    u8 cpld_addr_t = 0x00;
    int values[7] = {'\0'};
    int bit_t = 0x00;

    switch (attr->index) {
        case SFP_IS_PRESENT:
            port_t = sfp_port_data;

            if (port_t > 0 && port_t < 9) {          /* SFP Port 1-8 */
                reg_t = SFP_PRESENCE_1;
            } else if (port_t > 8 && port_t < 17) {  /* SFP Port 9-16 */
                reg_t = SFP_PRESENCE_2;
            } else if (port_t > 16 && port_t < 25) { /* SFP Port 17-24 */
                reg_t = SFP_PRESENCE_3;
            } else if (port_t > 24 && port_t < 33) { /* SFP Port 25-32 */
                reg_t = SFP_PRESENCE_4;
            } else if (port_t > 32 && port_t < 41) { /* SFP Port 33-40 */
                reg_t = SFP_PRESENCE_5;
            } else if (port_t > 40 && port_t < 49) { /* SFP Port 41-48 */
                reg_t = SFP_PRESENCE_6;
            } else if (port_t > 48 && port_t < 53) { /* QSFP Port 49-54 */
                reg_t = QSFP_PRESENCE_1;
            } else {
                values[0] = 1; /* return 1, module NOT present */
                return sprintf(buf, "%d\n", values[0]);
            }
            
            if(port_t > 48 && port_t < 53){
                VALIDATED_READ(buf, values[0], i2c_smbus_read_byte_data(pdata[cpld_b].client, reg_t), 0);
            }
            else{
                VALIDATED_READ(buf, values[0], i2c_smbus_read_byte_data(pdata[cpld_a].client, reg_t), 0);
            }

            /* SWPLD QSFP module respond */
            bit_t = 1 << ((port_t - 1) % 8);
            values[0] = values[0] & bit_t;
            values[0] = values[0] / bit_t;

            /* sfp_is_present value
             * return 0 is module present
             * return 1 is module NOT present*/
            return sprintf(buf, "%d\n", values[0]);

        case SFP_IS_PRESENT_ALL:
             /*
              * Report the SFP ALL PRESENCE status
	      * This data information form CPLD.*/

            /* SFP_PRESENT Ports 1-8 */
            VALIDATED_READ(buf, values[0],
                i2c_smbus_read_byte_data(pdata[cpld_a].client, SFP_PRESENCE_1), 0);
            /* SFP_PRESENT Ports 9-16 */
            VALIDATED_READ(buf, values[1],
                i2c_smbus_read_byte_data(pdata[cpld_a].client, SFP_PRESENCE_2), 0);
            /* SFP_PRESENT Ports 17-24 */
            VALIDATED_READ(buf, values[2],
                i2c_smbus_read_byte_data(pdata[cpld_a].client, SFP_PRESENCE_3), 0);
            /* SFP_PRESENT Ports 25-32 */
            VALIDATED_READ(buf, values[3],
                i2c_smbus_read_byte_data(pdata[cpld_a].client, SFP_PRESENCE_4), 0);
            /* SFP_PRESENT Ports 33-40 */
            VALIDATED_READ(buf, values[4],
                i2c_smbus_read_byte_data(pdata[cpld_a].client, SFP_PRESENCE_5), 0);
            /* SFP_PRESENT Ports 41-48 */
            VALIDATED_READ(buf, values[5],
                i2c_smbus_read_byte_data(pdata[cpld_a].client, SFP_PRESENCE_6), 0);
            /* QSFP_PRESENT Ports 49-52 */
            VALIDATED_READ(buf, values[6],
                i2c_smbus_read_byte_data(pdata[cpld_b].client, QSFP_PRESENCE_1), 0);


		    /* sfp_is_present_all value
		     * return 0 is module present
		     * return 1 is module NOT present
		     */
            return sprintf(buf, "%02X %02X %02X %02X %02X %02X %02X \n",values[0], values[1], values[2],values[3], values[4], values[5],values[6]); 
        case SFP_LP_MODE:
            port_t = sfp_port_data;
            if (port_t > 48 && port_t < 53) { 
                /* QSFP Port 49-52 */
                VALIDATED_READ(buf, values[0], i2c_smbus_read_byte_data(pdata[cpld_b].client, QSFP_LP_MODE_1), 0);
            } else {
                /* In AK7448 only QSFP support control LP MODE */
                values[0] = 0;
                return sprintf(buf, "%d\n", values[0]);
            }

            bit_t = 1 << ((port_t - 1) % 8);
            values[0] = values[0] & bit_t;
            values[0] = values[0] / bit_t;

		    /* sfp_lp_mode value
		     * return 0 is module NOT in LP mode
		     * return 1 is module in LP mode
		     */

            return sprintf(buf, "%d\n", values[0]);

         case SFP_RESET:
             port_t = sfp_port_data;
             if (port_t > 48 && port_t < 53) { 
                 /* QSFP Port 49-54 */
                 VALIDATED_READ(buf, values[0], i2c_smbus_read_byte_data(pdata[cpld_b].client, QSFP_RESET_1), 0);
             } else {
                 /* In AK7448 only QSFP support control RESET MODE */
                 values[0] = 1;
                 return sprintf(buf, "%d\n", values[0]);
             }

		     /* SWPLD QSFP module respond */
             bit_t = 1 << ((port_t - 1) % 8);
             values[0] = values[0] & bit_t;
             values[0] = values[0] / bit_t;

		     /* sfp_reset value
		      * return 0 is module Reset
		      * return 1 is module Normal*/
             return sprintf(buf, "%d\n", values[0]);	

        default:
            return sprintf(buf, "%d not found", attr->index);
    }
}

static unsigned char swpld_reg_addr;

static ssize_t
get_cpld_data(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;
    struct cpld_platform_data *pdata = dev->platform_data;

    ret = sprintf(buf, "0x%x\n",i2c_smbus_read_byte_data(pdata[cpld_b].client, swpld_reg_addr));
    return ret;
}

static ssize_t
set_cpld_data(struct device *dev, struct device_attribute *attr,
              const char *buf, size_t count)
{
    struct cpld_platform_data *pdata = dev->platform_data;
    unsigned int val;
    char *endp;
    char ret = -1;

    val = simple_strtoul(buf, &endp, 16);
    if (isspace(*endp)){
        endp++;
    }

    if (endp - buf != count){
        return -1;
    }

    if (val > 255){
        return -1;
    }

    ret = i2c_smbus_write_byte_data(pdata[cpld_b].client, swpld_reg_addr, val);

    return count;
}

static ssize_t get_cpld_reg(struct device *dev, struct device_attribute *attr,
                            char *buf)
{
   int len;
   len = sprintf(buf, "0x%x\n", swpld_reg_addr);
   return len;
}

static ssize_t
set_cpld_reg(struct device *dev, struct device_attribute *attr,
             const char *buf, size_t count)
{
    unsigned int val;
    char *endp;

    val = simple_strtoul(buf, &endp, 16);
    if (isspace(*endp)){
       endp++;
    }

    if (endp - buf != count){
       return -1;
    }

    if (val > 255)
    {
       return -1;
    }
    swpld_reg_addr = val;
    return count;
}


static SENSOR_DEVICE_ATTR(sfp_select_port, S_IWUSR | S_IRUGO, for_r_port_data, set_w_port_data, SFP_SELECT_PORT);
static SENSOR_DEVICE_ATTR(sfp_is_present, S_IRUGO, for_status, NULL, SFP_IS_PRESENT);
static SENSOR_DEVICE_ATTR(sfp_is_present_all, S_IRUGO, for_status, NULL, SFP_IS_PRESENT_ALL);
static SENSOR_DEVICE_ATTR(sfp_lp_mode,        S_IWUSR | S_IRUGO, for_status, set_w_lp_mode_data, SFP_LP_MODE);
static SENSOR_DEVICE_ATTR(sfp_reset,          S_IWUSR | S_IRUGO, for_status, set_w_reset_data, SFP_RESET);
static SENSOR_DEVICE_ATTR(data, S_IWUSR | S_IRUGO, get_cpld_data, set_cpld_data, DATA);
static SENSOR_DEVICE_ATTR(addr, S_IWUSR | S_IRUGO, get_cpld_reg, set_cpld_reg, ADDR);


static struct attribute *ak7448_cpld_attrs[] = {
    &sensor_dev_attr_sfp_select_port.dev_attr.attr,
    &sensor_dev_attr_sfp_is_present.dev_attr.attr,
    &sensor_dev_attr_sfp_is_present_all.dev_attr.attr,
    &sensor_dev_attr_sfp_lp_mode.dev_attr.attr,
    &sensor_dev_attr_sfp_reset.dev_attr.attr,
    &sensor_dev_attr_data.dev_attr.attr,
    &sensor_dev_attr_addr.dev_attr.attr,
    NULL,
};

static struct attribute_group ak7448_cpld_attr_grp = {
    .attrs = ak7448_cpld_attrs,
};


/*    CPLD  -- driver   */
static int __init cpld_probe(struct platform_device *pdev)
{
    struct cpld_platform_data *pdata;
    struct i2c_adapter *parent;
    int ret,i;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "CPLD platform data not found\n");
        return -ENODEV;
    }

    parent = i2c_get_adapter(BUS5);
    if (!parent) {
        printk(KERN_WARNING "Parent adapter (%d) not found\n",BUS5);
        return -ENODEV;
    }

    for (i = 0; i < ARRAY_SIZE(ak7448_cpld_platform_data); i++) {
        pdata[i].client = i2c_new_dummy(parent, pdata[i].reg_addr);
        if (!pdata[i].client) {
            printk(KERN_WARNING "Fail to create dummy i2c client for addr %d\n", pdata[i].reg_addr);
            goto error;
        }
    }

    kobj_swpld = &pdev->dev.kobj;
    ret = sysfs_create_group(&pdev->dev.kobj, &ak7448_cpld_attr_grp);
    if (ret) {
        printk(KERN_WARNING "Fail to create cpld attribute group");
        goto error;
    }
    
    return 0;

error:   
    kobject_put(kobj_swpld); 
    i--;
    for (; i >= 0; i--) {
        if (pdata[i].client) {
            i2c_unregister_device(pdata[i].client);
        }
    }
    i2c_put_adapter(parent);
    
    return -ENODEV; 
}

static int __exit cpld_remove(struct platform_device *pdev)
{
    struct i2c_adapter *parent = NULL;
    struct cpld_platform_data *pdata = pdev->dev.platform_data;
    int i;
    sysfs_remove_group(&pdev->dev.kobj, &ak7448_cpld_attr_grp);

    if (!pdata) {
        dev_err(&pdev->dev, "Missing platform data\n");
    } 
    else {
        kobject_put(kobj_swpld);
        for (i = 0; i < ARRAY_SIZE(ak7448_cpld_platform_data); i++) {
            if (pdata[i].client) {
                if (!parent) {
                    parent = (pdata[i].client)->adapter;
                }
                i2c_unregister_device(pdata[i].client);
            }
        }
    }

    i2c_put_adapter(parent);

    return 0;
}

static struct platform_driver cpld_driver = {
    .probe  = cpld_probe,
    .remove = __exit_p(cpld_remove),
    .driver = {
        .owner  = THIS_MODULE,
        .name   = "delta-ak7448-cpld",
    },
};

/*----------------    CPLD  - end   ------------- */

/*----------------    MUX   - start   ------------- */

struct swpld_mux_platform_data {
    int parent;
    int base_nr;
    int reg_addr;
    struct i2c_client *cpld;
};

struct swpld_mux {
    struct i2c_adapter *parent;
    struct i2c_adapter **child;
    struct swpld_mux_platform_data data;
};

static struct swpld_mux_platform_data ak7448_swpld_mux_platform_data[] = {
    {
        .parent         = BUS3, 
        .base_nr        = BUS3_BASE_NUM, 
        .cpld           = NULL,
        .reg_addr       = BUS3_MUX_REG ,
    },
    {
        .parent         = BUS8,
        .base_nr        = BUS8_BASE_NUM,
        .cpld           = NULL,
        .reg_addr       = BUS8_MUX_REG ,
    },
};


static struct platform_device ak7448_swpld_mux[] = {
    {
        .name           = "delta-ak7448-swpld-mux",
        .id             = 0,
        .dev            = {
                .platform_data   = &ak7448_swpld_mux_platform_data[0],
                .release         = device_release,
        },
    },
    {
        .name           = "delta-ak7448-swpld-mux",
        .id             = 1,
        .dev            = {
                .platform_data   = &ak7448_swpld_mux_platform_data[1],
                .release         = device_release,
        },
    },
};


static int cpld_reg_write_byte(struct i2c_client *client, u8 regaddr, u8 val)
{
    union i2c_smbus_data data;

    data.byte = val;
    return client->adapter->algo->smbus_xfer(client->adapter, client->addr,
                                             client->flags,
                                             I2C_SMBUS_WRITE,
                                             regaddr, I2C_SMBUS_BYTE_DATA, &data);
}

static int swpld_mux_select(struct i2c_adapter *adap, void *data, u8 chan)
{
    struct swpld_mux *mux = data;
    u8 swpld_mux_val=0; 
    if ( mux->data.base_nr == BUS3_BASE_NUM )
    {
        switch (chan) {
            case 0:
                swpld_mux_val = QSFP_P0_VAL;
                break;
            case 1:
                swpld_mux_val = QSFP_P1_VAL;
                break;
            case 2:
                swpld_mux_val = QSFP_P2_VAL;
                break;
            case 3:
                swpld_mux_val = QSFP_P3_VAL;
                break;
        }
    }
    else if ( mux->data.base_nr == BUS8_BASE_NUM ){
        if (chan < 16){
            swpld_mux_val = (u8)(chan);
        }
        else if (15 < chan && chan < 32){
            swpld_mux_val = (u8)(chan - 16) + 0x10;
        }
        else if (31 < chan && chan < 48){
            swpld_mux_val = (u8)(chan - 32) + 0x20;
        }
        else{
            swpld_mux_val = 0x00;
        }
    }
    else
    {
        swpld_mux_val = 0x00;
    }
    swpld_mux_val=swpld_mux_val & (u8)0x3F;

    return cpld_reg_write_byte(mux->data.cpld, mux->data.reg_addr, (u8)(swpld_mux_val & 0xff));
}

static int __init swpld_mux_probe(struct platform_device *pdev)
{
    struct swpld_mux *mux;
    struct swpld_mux_platform_data *pdata;
    struct i2c_adapter *parent;
    int i, ret, dev_num;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "SWPLD platform data not found\n");
        return -ENODEV;
    }

    parent = i2c_get_adapter(pdata->parent);
    if (!parent) {
        dev_err(&pdev->dev, "Parent adapter (%d) not found\n", pdata->parent);
        return -ENODEV;
    }

    /* Judge bus number to decide how many devices*/
    switch (pdata->parent) {
        case BUS3:
            dev_num = BUS3_DEV_NUM;
            break;
        case BUS8:
            dev_num = BUS8_DEV_NUM;
            break;
        default :
            dev_num = DEFAULT_NUM;  
            break;
    }

    mux = kzalloc(sizeof(*mux), GFP_KERNEL);
    if (!mux) {
        ret = -ENOMEM;
        printk(KERN_ERR "Failed to allocate memory for mux\n");
        goto alloc_failed;
    }

    mux->parent = parent;
    mux->data = *pdata;
    mux->child = kzalloc(sizeof(struct i2c_adapter *) * dev_num, GFP_KERNEL);
    if (!mux->child) {
        ret = -ENOMEM;
        printk(KERN_ERR "Failed to allocate memory for device on mux\n");
        goto alloc_failed2;
    }

    for (i = 0; i < dev_num; i++) {
        int nr = pdata->base_nr + i;
        unsigned int class = 0;

        mux->child[i] = i2c_add_mux_adapter(parent, &pdev->dev, mux,
                           nr, i, class,
                           swpld_mux_select, NULL);
        if (!mux->child[i]) {
            ret = -ENODEV;
            dev_err(&pdev->dev, "Failed to add adapter %d\n", i);
            goto add_adapter_failed;
        }
    }

    platform_set_drvdata(pdev, mux);
    return 0;

add_adapter_failed:
    for (; i > 0; i--)
        i2c_del_mux_adapter(mux->child[i - 1]);
    kfree(mux->child);
alloc_failed2:
    kfree(mux);
alloc_failed:
    i2c_put_adapter(parent);

    return ret;
}


static int __exit swpld_mux_remove(struct platform_device *pdev)
{
    int i;
    struct swpld_mux *mux = platform_get_drvdata(pdev);
    struct swpld_mux_platform_data *pdata;
    struct i2c_adapter *parent;
    int dev_num;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "SWPLD platform data not found\n");
        return -ENODEV;
    }

    parent = i2c_get_adapter(pdata->parent);
    if (!parent) {
        dev_err(&pdev->dev, "Parent adapter (%d) not found\n",
            pdata->parent);
        return -ENODEV;
    }
    switch (pdata->parent) {
        case BUS3:
            dev_num = BUS3_DEV_NUM;
            break;
        case BUS8:
            dev_num = BUS8_DEV_NUM;
            break;
        default :
            dev_num = DEFAULT_NUM;  
            break;
    }

    for (i = 0; i < dev_num; i++)
        i2c_del_mux_adapter(mux->child[i]);

    platform_set_drvdata(pdev, NULL);
    i2c_put_adapter(mux->parent);
    kfree(mux->child);
    kfree(mux);

    return 0;
}

static struct platform_driver swpld_mux_driver = {
    .probe  = swpld_mux_probe,
    .remove = __exit_p(swpld_mux_remove), /* TODO */
    .driver = {
        .owner  = THIS_MODULE,
        .name   = "delta-ak7448-swpld-mux",
    },
};
/*----------------    MUX   - end   ------------- */


/*----------------   module initialization     ------------- */
static void __init delta_ak7448_platform_init(void)
{
    struct i2c_client *client;
    struct i2c_adapter *adapter;
    struct cpld_platform_data *cpld_pdata;
    struct swpld_mux_platform_data *swpld_pdata;
    int ret,i = 0;

    // set the CPLD prob and remove
    ret = platform_driver_register(&cpld_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register cpld driver\n");
        goto error_cpld_driver;
    }

    // register the mux prob which call the CPLD
    ret = platform_driver_register(&swpld_mux_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register swpld mux driver\n");
        goto error_swpld_mux_driver;
    }

    // register the i2c devices    
    ret = platform_driver_register(&i2c_device_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register i2c device driver\n");
        goto error_i2c_device_driver;
    }

    // register the CPLD
    ret = platform_device_register(&ak7448_cpld);
    if (ret) {
        printk(KERN_WARNING "Fail to create cpld device\n");
        goto error_ak7448_cpld;
    }
    // link the CPLD and the Mux
    cpld_pdata = ak7448_cpld.dev.platform_data;

    for (i = 0; i < ARRAY_SIZE(ak7448_swpld_mux); i++){
        swpld_pdata = ak7448_swpld_mux[i].dev.platform_data;
        swpld_pdata->cpld = cpld_pdata[cpld_b].client;  
        ret = platform_device_register(&ak7448_swpld_mux[i]);          
        if (ret) {
            printk(KERN_WARNING "Fail to create swpld mux %d\n", i);
            goto error_ak7448_swpld_mux;
        }
    }
    
    for (i = 0; i < ARRAY_SIZE(ak7448_i2c_device); i++)
    {
        ret = platform_device_register(&ak7448_i2c_device[i]);
        if (ret) {
            printk(KERN_WARNING "Fail to create i2c device %d\n", i);
            goto error_ak7448_i2c_device;
        }
    }

    if (ret)
        goto error_ak7448_swpld_mux;

    return 0;

error_ak7448_i2c_device:
    i--;
    for (; i >= 0; i--) {
        platform_device_unregister(&ak7448_i2c_device[i]);
    }
    i = ARRAY_SIZE(ak7448_swpld_mux);    
error_ak7448_swpld_mux:
    i--;
    for (; i >= 0; i--) {
        platform_device_unregister(&ak7448_swpld_mux[i]);
    }
    platform_driver_unregister(&ak7448_cpld);
error_ak7448_cpld:
    platform_driver_unregister(&i2c_device_driver);
error_i2c_device_driver:
    platform_driver_unregister(&swpld_mux_driver);
error_swpld_mux_driver:
    platform_driver_unregister(&cpld_driver);
error_cpld_driver:
    return ret;
}

static void __exit delta_ak7448_platform_exit(void)
{
    int i = 0;

    for ( i = 0; i < ARRAY_SIZE(ak7448_i2c_device); i++ ) {
        platform_device_unregister(&ak7448_i2c_device[i]);
    }

    for (i = 0; i < ARRAY_SIZE(ak7448_swpld_mux); i++) {
        platform_device_unregister(&ak7448_swpld_mux[i]);
    }

    platform_device_unregister(&ak7448_cpld);
    platform_driver_unregister(&i2c_device_driver);
    platform_driver_unregister(&cpld_driver);
    platform_driver_unregister(&swpld_mux_driver);

}

module_init(delta_ak7448_platform_init);
module_exit(delta_ak7448_platform_exit);

MODULE_DESCRIPTION("DNI ak7448 Platform Support");
MODULE_AUTHOR("Johnson Lu <johsnon.lu@deltaww.com>");
MODULE_LICENSE("GPL");    
