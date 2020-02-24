#include <linux/init.h>
#include <linux/slab.h>
#include <linux/dmi.h>
#include <linux/version.h>
#include <linux/ctype.h>
#include <linux/i2c/pca954x.h>
#include <linux/i2c-mux.h>
#include <linux/i2c-mux-gpio.h>
#include "delta_ag9064_common.h"


#define ag9064_i2c_device_num(NUM) {                            \
    .name = "delta-ag9064-i2c-device",                          \
    .id   = NUM,                                                \
    .dev  = {                                                   \
        .platform_data = &ag9064_i2c_device_platform_data[NUM], \
        .release       = device_release,                        \
    },                                                          \
}

struct i2c_client * i2c_client_9548;

/* pca9548 - add 8 bus */
static struct pca954x_platform_mode pca954x_mode[] =
{
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
    },
    {
        .adap_id = 10,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 11,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 12,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 13,
        .deselect_on_exit = 1,
    },
    {
        .adap_id = 14,
        .deselect_on_exit = 1,
    }
};

static struct pca954x_platform_data pca954x_data =
{
    .modes = pca954x_mode,
    .num_modes = ARRAY_SIZE(pca954x_mode),
};

static struct i2c_board_info __initdata i2c_info_pca9548[] =
{
    {
        I2C_BOARD_INFO("pca9548", 0x70),
        .platform_data = &pca954x_data,
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

static struct i2c_device_platform_data ag9064_i2c_device_platform_data[] = {
    {
        /* tmp75 (0x4d) */
        .parent = 1,
        .info = { I2C_BOARD_INFO("tmp75", 0x4d) },
        .client = NULL,
    },
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
        /* qsfp 5 (0x50) */
        .parent = 24,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 6 (0x50) */
        .parent = 25,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 7 (0x50) */
        .parent = 26,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 8 (0x50) */
        .parent = 27,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 9 (0x50) */
        .parent = 28,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 10 (0x50) */
        .parent = 29,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 11 (0x50) */
        .parent = 30,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 12 (0x50) */
        .parent = 31,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 13 (0x50) */
        .parent = 32,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 14 (0x50) */
        .parent = 33,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 15 (0x50) */
        .parent = 34,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 16 (0x50) */
        .parent = 35,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 17 (0x50) */
        .parent = 36,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 18 (0x50) */
        .parent = 37,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 19 (0x50) */
        .parent = 38,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 20 (0x50) */
        .parent = 39,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 21 (0x50) */
        .parent = 40,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 22 (0x50) */
        .parent = 41,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 23 (0x50) */
        .parent = 42,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 24 (0x50) */
        .parent = 43,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 25 (0x50) */
        .parent = 44,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 26 (0x50) */
        .parent = 45,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 27 (0x50) */
        .parent = 46,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 28 (0x50) */
        .parent = 47,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 29 (0x50) */
        .parent = 48,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 30 (0x50) */
        .parent = 49,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 31 (0x50) */
        .parent = 50,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 32 (0x50) */
        .parent = 51,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 33 (0x50) */
        .parent = 52,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 34 (0x50) */
        .parent = 53,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 35 (0x50) */
        .parent = 54,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 36 (0x50) */
        .parent = 55,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 37 (0x50) */
        .parent = 56,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 38 (0x50) */
        .parent = 57,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 39 (0x50) */
        .parent = 58,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 40 (0x50) */
        .parent = 59,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 41 (0x50) */
        .parent = 60,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 42 (0x50) */
        .parent = 61,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 43 (0x50) */
        .parent = 62,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 44 (0x50) */
        .parent = 63,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 45 (0x50) */
        .parent = 64,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 46 (0x50) */
        .parent = 65,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 47 (0x50) */
        .parent = 66,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 48 (0x50) */
        .parent = 67,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 49 (0x50) */
        .parent = 68,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 50 (0x50) */
        .parent = 69,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 51 (0x50) */
        .parent = 70,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 52 (0x50) */
        .parent = 71,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 53 (0x50) */
        .parent = 72,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 54 (0x50) */
        .parent = 73,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 55 (0x50) */
        .parent = 74,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 56 (0x50) */
        .parent = 75,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 57 (0x50) */
        .parent = 76,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 58 (0x50) */
        .parent = 77,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 59 (0x50) */
        .parent = 78,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 60 (0x50) */
        .parent = 79,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 61 (0x50) */
        .parent = 80,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 62 (0x50) */
        .parent = 81,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 63 (0x50) */
        .parent = 82,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        /* qsfp 64 (0x50) */
        .parent = 83,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    }
};

static struct platform_device ag9064_i2c_device[] = {
    ag9064_i2c_device_num(0),
    ag9064_i2c_device_num(1),
    ag9064_i2c_device_num(2),
    ag9064_i2c_device_num(3),
    ag9064_i2c_device_num(4),
    ag9064_i2c_device_num(5),
    ag9064_i2c_device_num(6),
    ag9064_i2c_device_num(7),
    ag9064_i2c_device_num(8),
    ag9064_i2c_device_num(9),
    ag9064_i2c_device_num(10),
    ag9064_i2c_device_num(11),
    ag9064_i2c_device_num(12),
    ag9064_i2c_device_num(13),
    ag9064_i2c_device_num(14),
    ag9064_i2c_device_num(15),
    ag9064_i2c_device_num(16),
    ag9064_i2c_device_num(17),
    ag9064_i2c_device_num(18),
    ag9064_i2c_device_num(19),
    ag9064_i2c_device_num(20),
    ag9064_i2c_device_num(21),
    ag9064_i2c_device_num(22),
    ag9064_i2c_device_num(23),
    ag9064_i2c_device_num(24),
    ag9064_i2c_device_num(25),
    ag9064_i2c_device_num(26),
    ag9064_i2c_device_num(27),
    ag9064_i2c_device_num(28),
    ag9064_i2c_device_num(29),
    ag9064_i2c_device_num(30),
    ag9064_i2c_device_num(31),
    ag9064_i2c_device_num(32),
    ag9064_i2c_device_num(33),
    ag9064_i2c_device_num(34),
    ag9064_i2c_device_num(35),
    ag9064_i2c_device_num(36),
    ag9064_i2c_device_num(37),
    ag9064_i2c_device_num(38),
    ag9064_i2c_device_num(39),
    ag9064_i2c_device_num(40),
    ag9064_i2c_device_num(41),
    ag9064_i2c_device_num(42),
    ag9064_i2c_device_num(43),
    ag9064_i2c_device_num(44),
    ag9064_i2c_device_num(45),
    ag9064_i2c_device_num(46),
    ag9064_i2c_device_num(47),
    ag9064_i2c_device_num(48),
    ag9064_i2c_device_num(49),
    ag9064_i2c_device_num(50),
    ag9064_i2c_device_num(51),
    ag9064_i2c_device_num(52),
    ag9064_i2c_device_num(53),
    ag9064_i2c_device_num(54),
    ag9064_i2c_device_num(55),
    ag9064_i2c_device_num(56),
    ag9064_i2c_device_num(57),
    ag9064_i2c_device_num(58),
    ag9064_i2c_device_num(59),
    ag9064_i2c_device_num(60),
    ag9064_i2c_device_num(61),
    ag9064_i2c_device_num(62),
    ag9064_i2c_device_num(63),
    ag9064_i2c_device_num(64)
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
        .name = "delta-ag9064-i2c-device",
    }
};
/* --------------- I2C driver - end --------------- */

/* --------------- MUX - start --------------- */
struct swpld_mux_platform_data {
    int parent;
    int base_nr;
    struct i2c_client *cpld;
};

struct swpld_mux {
    struct i2c_adapter *parent;
    struct i2c_adapter **child;
    struct swpld_mux_platform_data data;
};

static struct swpld_mux_platform_data ag9064_swpld_mux_platform_data[] = {
    {
        .parent  = BUS9,
        .base_nr = BUS9_BASE_NUM,
        .cpld    = NULL,
    },
};

static struct platform_device ag9064_swpld_mux[] =
{
    {
        .name  = "delta-ag9064-swpld-mux",
        .id    = 0,
        .dev   = {
            .platform_data  = &ag9064_swpld_mux_platform_data[0],
            .release        = device_release,
        },
    },
};

static int swpld_mux_select(struct i2c_mux_core *muxc, u32 chan)
{
    uint8_t cmd_data[CMD_DATA_SIZE] = {0};
    struct swpld_mux *mux = i2c_mux_priv(muxc);
    uint8_t set_cmd;
    int cmd_data_len;

    if (mux->data.base_nr == BUS9_BASE_NUM)
    {
        set_cmd = CMD_SETDATA;
        cmd_data[0] = BMC_BUS_5;
        cmd_data[1] = SWPLD2_ADDR;
        cmd_data[2] = QSFP_PORT_MUX_REG;
        cmd_data[3] = chan + 1;
        cmd_data_len = sizeof(cmd_data);
        return dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
    }
    else
    {
        printk(KERN_ERR "Swpld mux QSFP select port error\n");
        return 0;
    }
}

static int __init swpld_mux_probe(struct platform_device *pdev)
{
    struct i2c_mux_core *muxc;
    struct swpld_mux *mux;
    struct swpld_mux_platform_data *pdata;
    struct i2c_adapter *parent;
    int i, ret, dev_num;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "SWPLD platform data not found\n");
        return -ENODEV;
    }
    mux = kzalloc(sizeof(*mux), GFP_KERNEL);
    if (!mux) {
        printk(KERN_ERR "Failed to allocate memory for mux\n");
        return -ENOMEM;
    }
    mux->data = *pdata;

    parent = i2c_get_adapter(pdata->parent);
    if (!parent) {
        kfree(mux);
        dev_err(&pdev->dev, "Parent adapter (%d) not found\n", pdata->parent);
        return -ENODEV;
    }
    /* Judge bus number to decide how many devices*/
    switch (pdata->parent) {
        case BUS9:
            dev_num = BUS9_DEV_NUM;
            break;
        default :
            dev_num = DEFAULT_NUM;
            break;
    }

    muxc = i2c_mux_alloc(parent, &pdev->dev, dev_num, 0, 0,swpld_mux_select, NULL);
    if (!muxc) {
        ret = -ENOMEM;
        goto alloc_failed;
    }
    muxc->priv = mux;
    platform_set_drvdata(pdev, muxc);

    for (i = 0; i < dev_num; i++)
    {
        int nr = pdata->base_nr + i;
        unsigned int class = 0;
        ret = i2c_mux_add_adapter(muxc, nr, i, class);
        if (ret) {
            dev_err(&pdev->dev, "Failed to add adapter %d\n", i);
            goto add_adapter_failed;
        }
    }
    dev_info(&pdev->dev, "%d port mux on %s adapter\n", dev_num, parent->name);
    return 0;

add_adapter_failed:
    i2c_mux_del_adapters(muxc);
alloc_failed:
    i2c_put_adapter(parent);

    return ret;
}

static int __exit swpld_mux_remove(struct platform_device *pdev)
{
    struct i2c_mux_core *muxc = platform_get_drvdata(pdev);
    struct i2c_adapter *parent=muxc->parent;

    i2c_mux_del_adapters(muxc);
    i2c_put_adapter(parent);

    return 0;
}

static struct platform_driver swpld_mux_driver = {
    .probe  = swpld_mux_probe,
    .remove = __exit_p(swpld_mux_remove), /* TODO */
    .driver = {
        .owner  = THIS_MODULE,
        .name   = "delta-ag9064-swpld-mux",
    },
};

/* --------------- MUX - end --------------- */

/* --------------- module initialization --------------- */
static int __init delta_ag9064_platform_init(void)
{
    struct i2c_adapter *adapter;
    struct swpld_mux_platform_data *swpld_pdata;
    int ret, i = 0;

    printk("ag9064_platform module initialization\n");

    adapter = i2c_get_adapter(BUS2);
    i2c_client_9548 = i2c_new_device(adapter, &i2c_info_pca9548[0]);
    i2c_put_adapter(adapter);

    ret = dni_create_user();
    if (ret != 0){
        printk(KERN_WARNING "Fail to create IPMI user\n");
    }

    // register the mux prob which call the SWPLD
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

    swpld_pdata = ag9064_swpld_mux[0].dev.platform_data;
    ret = platform_device_register(&ag9064_swpld_mux[0]);
    if (ret) {
        printk(KERN_WARNING "Fail to create swpld mux\n");
        goto error_ag9064_swpld_mux;
    }

    for (i = 0; i < ARRAY_SIZE(ag9064_i2c_device); i++)
    {
        ret = platform_device_register(&ag9064_i2c_device[i]);
        if (ret)
        {
            printk(KERN_WARNING "Fail to create i2c device %d\n", i);
            goto error_ag9064_i2c_device;
        }
    }
    if (ret)
        goto error_ag9064_swpld_mux;

    return 0;

error_ag9064_i2c_device:
    i--;
    for (; i >= 0; i--) {
        platform_device_unregister(&ag9064_i2c_device[i]);
    }
    i = ARRAY_SIZE(ag9064_swpld_mux);
error_ag9064_swpld_mux:
    platform_device_unregister(&ag9064_swpld_mux[0]);
    platform_driver_unregister(&i2c_device_driver);
error_i2c_device_driver:
    platform_driver_unregister(&swpld_mux_driver);
error_swpld_mux_driver:
    return ret;
}

static void __exit delta_ag9064_platform_exit(void)
{
    int i = 0;

    for ( i = 0; i < ARRAY_SIZE(ag9064_i2c_device); i++ ) {
        platform_device_unregister(&ag9064_i2c_device[i]);
    }

    platform_device_unregister(&ag9064_swpld_mux[0]);
    platform_driver_unregister(&i2c_device_driver);
    platform_driver_unregister(&swpld_mux_driver);
    i2c_unregister_device(i2c_client_9548);
}

module_init(delta_ag9064_platform_init);
module_exit(delta_ag9064_platform_exit);

MODULE_DESCRIPTION("DELTA ag9064 Platform Support");
MODULE_AUTHOR("Jeff Chen <jeff.sj.chen@deltaww.com>");
MODULE_LICENSE("GPL");
