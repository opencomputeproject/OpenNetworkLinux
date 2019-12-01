#include <linux/init.h>
#include <linux/slab.h>
#include <linux/dmi.h>
#include <linux/version.h>
#include <linux/ctype.h>
#include <linux/i2c/pca954x.h>
#include <linux/i2c-mux.h>
#include <linux/i2c-mux-gpio.h>
#include "delta_agc7008s_common.h"


#define agc7008s_i2c_device_num(NUM) {                            \
    .name = "delta-agc7008s-i2c-device",                          \
    .id   = NUM,                                                \
    .dev  = {                                                   \
        .platform_data = &agc7008s_i2c_device_platform_data[NUM], \
        .release       = device_release,                        \
    },                                                          \
}

struct i2c_client * i2c_client_9548_1;
struct i2c_client * i2c_client_9548_2;
struct i2c_client * i2c_client_9548_3;

/* pca9548 - add 8 bus */
static struct pca954x_platform_mode pca954x_1_mode[] =
{
    {
        .adap_id = 2,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 3,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 4,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 5,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 6,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 7,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 8,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 9,
        .deselect_on_exit = 1,
    }
};

static struct pca954x_platform_mode pca954x_2_mode[] =
{
    {
        .adap_id = 20,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 21,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 22,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 23,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 24,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 25,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 26,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 27,
        .deselect_on_exit = 1,
    }
};

static struct pca954x_platform_mode pca954x_3_mode[] =
{
    {
        .adap_id = 28,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 29,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 30,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 31,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 32,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 33,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 34,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 35,
        .deselect_on_exit = 1,
    }
};


static struct pca954x_platform_data pca954x_data[] = 
{
    {
        .modes = pca954x_1_mode,
        .num_modes = ARRAY_SIZE(pca954x_1_mode),
    },
    {
        .modes = pca954x_2_mode,
        .num_modes = ARRAY_SIZE(pca954x_2_mode),
    },
    {
        .modes = pca954x_3_mode,
        .num_modes = ARRAY_SIZE(pca954x_3_mode),
    },
};

static struct i2c_board_info __initdata i2c_info_pca9548[] =
{
    {
        I2C_BOARD_INFO("pca9548", 0x71),
        .platform_data = &pca954x_data[0],
    },
    {
        I2C_BOARD_INFO("pca9548", 0x72),
        .platform_data = &pca954x_data[1],
    },
    {
        I2C_BOARD_INFO("pca9548", 0x72),
        .platform_data = &pca954x_data[2],
    },
};

void device_release(struct device *dev)
{
    return;
}
EXPORT_SYMBOL(device_release);

void msg_handler(struct ipmi_recv_msg *recv_msg, void* handler_data)
{
    struct completion *comp = recv_msg->user_msg_data;
    if (comp)
        complete(comp);
    else
        ipmi_free_recv_msg(recv_msg);
    return;
}
EXPORT_SYMBOL(msg_handler);

void dummy_smi_free(struct ipmi_smi_msg *msg)
{
    atomic_dec(&dummy_count);
}
EXPORT_SYMBOL(dummy_smi_free);

void dummy_recv_free(struct ipmi_recv_msg *msg)
{
    atomic_dec(&dummy_count);
}
EXPORT_SYMBOL(dummy_recv_free);

unsigned char dni_log2 (unsigned char num) {
    unsigned char num_log2 = 0;
    while (num > 0) {
        num = num >> 1;
        num_log2 += 1;
    }
    return num_log2 - 1;
}
EXPORT_SYMBOL(dni_log2);

/* --------------- IPMI - start --------------- */
int dni_create_user(void)
{
    int rv, i;

    for (i = 0, rv = 1; i < IPMI_MAX_INTF && rv; i++)
        rv = ipmi_create_user(i, &ipmi_hndlrs, NULL, &ipmi_mh_user);
    if (rv == 0)
    {
        printk("Enable IPMI protocol.\n");
        return rv;
    }
    return rv;
}
EXPORT_SYMBOL(dni_create_user);

int dni_bmc_cmd(char set_cmd, char *cmd_data, int cmd_data_len)
{
    int rv;
    struct ipmi_system_interface_addr addr;
    struct kernel_ipmi_msg msg;
    struct completion comp;

    addr.addr_type = IPMI_SYSTEM_INTERFACE_ADDR_TYPE;
    addr.channel = IPMI_BMC_CHANNEL;
    addr.lun = 0;

    msg.netfn = DELTA_NETFN;
    msg.cmd = set_cmd;
    msg.data_len = cmd_data_len;
    msg.data = cmd_data;

    init_completion(&comp);
    rv = ipmi_request_supply_msgs(ipmi_mh_user, (struct ipmi_addr*)&addr, 0, &msg, &comp, &halt_smi_msg, &halt_recv_msg, 0);
    if (rv) {
        return BMC_ERR;
    }
    wait_for_completion(&comp);

    switch (msg.cmd)
    {
        case CMD_GETDATA:
            if( rv == 0)
                return halt_recv_msg.msg.data[1];
            else
            {
                printk(KERN_ERR "IPMI get error!\n");
                return BMC_ERR;
            }
            break;
        case CMD_SETDATA:
            if (rv == 0)
                return rv;
            else
            {
                printk(KERN_ERR "IPMI set error!\n");
                return BMC_ERR;
            }
    }
    ipmi_free_recv_msg(&halt_recv_msg);

    return rv;
}
EXPORT_SYMBOL(dni_bmc_cmd);
/* --------------- IPMI - stop --------------- */

/* --------------- I2C device - start --------------- */
struct i2c_device_platform_data {
    int parent;
    struct i2c_board_info info;
    struct i2c_client *client;
};

static struct i2c_device_platform_data agc7008s_i2c_device_platform_data[] = {
    {
        // id eeprom
        .parent = 1,
        .info = { I2C_BOARD_INFO("24c02", 0x56) },
        .client = NULL,
    },
    {
        /* sfp+ P5 (0x50) */
        .parent = 20,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp+ P6 (0x50) */
        .parent = 21,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp+ P7 (0x50) */
        .parent = 22,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp+ P8 (0x50) */
        .parent = 23,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp P9 (0x50) */
        .parent = 24,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp P10 (0x50) */
        .parent = 25,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp P11 (0x50) */
        .parent = 26,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp P12 (0x50) */
        .parent = 27,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp P13 (0x50) */
        .parent = 28,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp P14 (0x50) */
        .parent = 29,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp P15 (0x50) */
        .parent = 30,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* sfp P16 (0x50) */
        .parent = 31,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
};

static struct platform_device agc7008s_i2c_device[] = {
    agc7008s_i2c_device_num(0),
    agc7008s_i2c_device_num(1),
    agc7008s_i2c_device_num(2),
    agc7008s_i2c_device_num(3),
    agc7008s_i2c_device_num(4),
    agc7008s_i2c_device_num(5),
    agc7008s_i2c_device_num(6),
    agc7008s_i2c_device_num(7),
    agc7008s_i2c_device_num(8),
    agc7008s_i2c_device_num(9),
    agc7008s_i2c_device_num(10),
    agc7008s_i2c_device_num(11),
    agc7008s_i2c_device_num(12),
};
/* --------------- I2C device - end --------------- */

/* --------------- I2C driver - start --------------- */
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
        parent = (pdata->client)->adapter;
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
        .name = "delta-agc7008s-i2c-device",
    }
};
/* --------------- I2C driver - end --------------- */

/* --------------- module initialization --------------- */
static int __init delta_agc7008s_platform_init(void)
{
    struct i2c_adapter *adapter;
    int ret, i = 0;

    printk("agc7008s_platform module initialization\n");

    adapter = i2c_get_adapter(BUS1);
    i2c_client_9548_1 = i2c_new_device(adapter, &i2c_info_pca9548[0]);
    i2c_put_adapter(adapter);

    adapter = i2c_get_adapter(BUS2);
    i2c_client_9548_2 = i2c_new_device(adapter, &i2c_info_pca9548[1]);
    i2c_put_adapter(adapter);

    adapter = i2c_get_adapter(BUS3);
    i2c_client_9548_3 = i2c_new_device(adapter, &i2c_info_pca9548[2]);
    i2c_put_adapter(adapter);

    ret = dni_create_user();
    if (ret != 0){
        printk(KERN_WARNING "Fail to create IPMI user\n");
    }

    // register the i2c devices
    ret = platform_driver_register(&i2c_device_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register i2c device driver\n");
        goto error_i2c_device_driver;
    }

    for (i = 0; i < ARRAY_SIZE(agc7008s_i2c_device); i++)
    {
        ret = platform_device_register(&agc7008s_i2c_device[i]);
        if (ret)
        {
            printk(KERN_WARNING "Fail to create i2c device %d\n", i);
            goto error_agc7008s_i2c_device;
        }
    }
    if (ret)
        goto error_agc7008s_i2c_driver;

    return 0;

error_agc7008s_i2c_device:
    i--;
    for (; i >= 0; i--) {
        platform_device_unregister(&agc7008s_i2c_device[i]);
    }
error_agc7008s_i2c_driver:
    platform_driver_unregister(&i2c_device_driver);
error_i2c_device_driver:
    i2c_unregister_device(i2c_client_9548_3);
    i2c_unregister_device(i2c_client_9548_2);
    i2c_unregister_device(i2c_client_9548_1);
    return ret;
}

static void __exit delta_agc7008s_platform_exit(void)
{
    int i = 0;

    for ( i = 0; i < ARRAY_SIZE(agc7008s_i2c_device); i++ ) {
        platform_device_unregister(&agc7008s_i2c_device[i]);
    }

    platform_driver_unregister(&i2c_device_driver);
    i2c_unregister_device(i2c_client_9548_3);
    i2c_unregister_device(i2c_client_9548_2);
    i2c_unregister_device(i2c_client_9548_1);
}

module_init(delta_agc7008s_platform_init);
module_exit(delta_agc7008s_platform_exit);

MODULE_DESCRIPTION("DELTA agc7008s Platform Support");
MODULE_AUTHOR("Jacky Liu <jacky.js.liu@deltaww.com>");
MODULE_LICENSE("GPL");
