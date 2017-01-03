/*
 * I2C multiplexer
 *
 * Copyright (C) 2014 Accton Technology Corporation.
 *
 * This module supports the accton cpld that hold the channel select
 * mechanism for other i2c slave devices, such as SFP.
 * This includes the:
 *	 Accton as6712_32x CPLD1/CPLD2/CPLD3
 *
 * Based on:
 *	pca954x.c from Kumar Gala <galak@kernel.crashing.org>
 * Copyright (C) 2006
 *
 * Based on:
 *	pca954x.c from Ken Harrenstien
 * Copyright (C) 2004 Google, Inc. (Ken Harrenstien)
 *
 * Based on:
 *	i2c-virtual_cb.c from Brian Kuschak <bkuschak@yahoo.com>
 * and
 *	pca9540.c from Jean Delvare <khali@linux-fr.org>.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/dmi.h>
#include <linux/version.h>

static struct dmi_system_id as6712_dmi_table[] = {
    {
        .ident = "Accton AS6712",
        .matches = {
            DMI_MATCH(DMI_BOARD_VENDOR, "Accton"),
            DMI_MATCH(DMI_PRODUCT_NAME, "AS6712"),
        },
    },
    {
        .ident = "Accton AS6712",
        .matches = {
            DMI_MATCH(DMI_SYS_VENDOR, "Accton"),
            DMI_MATCH(DMI_PRODUCT_NAME, "AS6712"),
        },
    },
};

int platform_accton_as6712_32x(void)
{
    return dmi_check_system(as6712_dmi_table);
}
EXPORT_SYMBOL(platform_accton_as6712_32x);

#define NUM_OF_CPLD1_CHANS 0x0
#define NUM_OF_CPLD2_CHANS 0x10
#define NUM_OF_CPLD3_CHANS 0x10
#define NUM_OF_ALL_CPLD_CHANS (NUM_OF_CPLD2_CHANS + NUM_OF_CPLD3_CHANS)
#define ACCTON_I2C_CPLD_MUX_MAX_NCHANS  NUM_OF_CPLD3_CHANS

static LIST_HEAD(cpld_client_list);
static struct mutex     list_lock;

struct cpld_client_node {
    struct i2c_client *client;
    struct list_head   list;
};

enum cpld_mux_type {
    as6712_32x_cpld2,
    as6712_32x_cpld3,
    as6712_32x_cpld1
};

struct accton_i2c_cpld_mux {
    enum cpld_mux_type type;
    struct i2c_adapter *virt_adaps[ACCTON_I2C_CPLD_MUX_MAX_NCHANS];
    u8 last_chan;  /* last register value */
};

struct chip_desc {
    u8   nchans;
    u8   deselectChan;
};

/* Provide specs for the PCA954x types we know about */
static const struct chip_desc chips[] = {
    [as6712_32x_cpld1] = {
    .nchans        = NUM_OF_CPLD1_CHANS,
    .deselectChan  = NUM_OF_CPLD1_CHANS,
    },
    [as6712_32x_cpld2] = {
    .nchans        = NUM_OF_CPLD2_CHANS,
    .deselectChan  = NUM_OF_CPLD2_CHANS,
    },
    [as6712_32x_cpld3] = {
    .nchans        = NUM_OF_CPLD3_CHANS,
    .deselectChan  = NUM_OF_CPLD3_CHANS,
    }
};

static const struct i2c_device_id accton_i2c_cpld_mux_id[] = {
    { "as6712_32x_cpld1", as6712_32x_cpld1 },
    { "as6712_32x_cpld2", as6712_32x_cpld2 },
    { "as6712_32x_cpld3", as6712_32x_cpld3 },
    { }
};
MODULE_DEVICE_TABLE(i2c, accton_i2c_cpld_mux_id);

/* Write to mux register. Don't use i2c_transfer()/i2c_smbus_xfer()
   for this as they will try to lock adapter a second time */
static int accton_i2c_cpld_mux_reg_write(struct i2c_adapter *adap,
			     struct i2c_client *client, u8 val)
{
#if 0
	int ret = -ENODEV;

	//if (adap->algo->master_xfer) {
        if (0)
		struct i2c_msg msg;
		char buf[2];

		msg.addr = client->addr;
		msg.flags = 0;
		msg.len = 2;
        buf[0] = 0x2;
		buf[1] = val;
		msg.buf = buf;
		ret = adap->algo->master_xfer(adap, &msg, 1);
	}
    else {
		union i2c_smbus_data data;
		ret = adap->algo->smbus_xfer(adap, client->addr,
					     client->flags,
					     I2C_SMBUS_WRITE,
					     0x2, I2C_SMBUS_BYTE, &data);
	}

	return ret;
#else
	unsigned long orig_jiffies;
        unsigned short flags;
	union i2c_smbus_data data;
	int try;
	s32 res = -EIO;

        data.byte = val;
        flags = client->flags;
	flags &= I2C_M_TEN | I2C_CLIENT_PEC;

	if (adap->algo->smbus_xfer) {
		/* Retry automatically on arbitration loss */
		orig_jiffies = jiffies;
		for (res = 0, try = 0; try <= adap->retries; try++) {
			res = adap->algo->smbus_xfer(adap, client->addr, flags,
                                                     I2C_SMBUS_WRITE, 0x2,
                                                     I2C_SMBUS_BYTE_DATA, &data);
			if (res != -EAGAIN)
				break;
			if (time_after(jiffies,
			        orig_jiffies + adap->timeout))
				break;
		}
	}

        return res;
#endif
}

static int accton_i2c_cpld_mux_select_chan(struct i2c_adapter *adap,
			       void *client, u32 chan)
{
	struct accton_i2c_cpld_mux *data = i2c_get_clientdata(client);
	u8 regval;
	int ret = 0;
        regval = chan;

	/* Only select the channel if its different from the last channel */
	if (data->last_chan != regval) {
		ret = accton_i2c_cpld_mux_reg_write(adap, client, regval);
		data->last_chan = regval;
	}

	return ret;
}

static int accton_i2c_cpld_mux_deselect_mux(struct i2c_adapter *adap,
				void *client, u32 chan)
{
	struct accton_i2c_cpld_mux *data = i2c_get_clientdata(client);

	/* Deselect active channel */
	data->last_chan = chips[data->type].deselectChan;

	return accton_i2c_cpld_mux_reg_write(adap, client, data->last_chan);
}

static void accton_i2c_cpld_add_client(struct i2c_client *client)
{
    struct cpld_client_node *node = kzalloc(sizeof(struct cpld_client_node), GFP_KERNEL);

    if (!node) {
        dev_dbg(&client->dev, "Can't allocate cpld_client_node (0x%x)\n", client->addr);
        return;
    }

    node->client = client;

	mutex_lock(&list_lock);
    list_add(&node->list, &cpld_client_list);
	mutex_unlock(&list_lock);
}

static void accton_i2c_cpld_remove_client(struct i2c_client *client)
{
    struct list_head        *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int found = 0;

	mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client == client) {
            found = 1;
            break;
        }
    }

    if (found) {
        list_del(list_node);
        kfree(cpld_node);
    }

	mutex_unlock(&list_lock);
}

static ssize_t show_cpld_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    u8 reg = 0x1;
    struct i2c_client *client;
    int len;

    client = to_i2c_client(dev);
    len = sprintf(buf, "%d", i2c_smbus_read_byte_data(client, reg));

    return len;
}

static struct device_attribute ver = __ATTR(version, 0600, show_cpld_version, NULL);

/*
 * I2C init/probing/exit functions
 */
static int accton_i2c_cpld_mux_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adap = to_i2c_adapter(client->dev.parent);
	int chan=0;
	struct accton_i2c_cpld_mux *data;
	int ret = -ENODEV;

	if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE))
		goto err;

	data = kzalloc(sizeof(struct accton_i2c_cpld_mux), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto err;
	}

	i2c_set_clientdata(client, data);

#if 0
	/* Write the mux register at addr to verify
	 * that the mux is in fact present.
	 */
	if (i2c_smbus_write_byte(client, 0) < 0) {
		dev_warn(&client->dev, "probe failed\n");
		goto exit_free;
	}
#endif

	data->type = id->driver_data;

    if (data->type == as6712_32x_cpld2 || data->type == as6712_32x_cpld3) {
    	data->last_chan = chips[data->type].deselectChan; /* force the first selection */

        /* Now create an adapter for each channel */
        for (chan = 0; chan < chips[data->type].nchans; chan++) {
            data->virt_adaps[chan] = i2c_add_mux_adapter(adap, &client->dev, client, 0, chan,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
                                                         I2C_CLASS_HWMON | I2C_CLASS_SPD,
#endif
                                                         accton_i2c_cpld_mux_select_chan,
                                                         accton_i2c_cpld_mux_deselect_mux);

            if (data->virt_adaps[chan] == NULL) {
                ret = -ENODEV;
                dev_err(&client->dev, "failed to register multiplexed adapter %d\n", chan);
                goto virt_reg_failed;
            }
        }

        dev_info(&client->dev, "registered %d multiplexed busses for I2C mux %s\n",
                    chan, client->name);
    }

    accton_i2c_cpld_add_client(client);

    ret = sysfs_create_file(&client->dev.kobj, &ver.attr);
    if (ret)
         goto virt_reg_failed;

    return 0;

virt_reg_failed:
	for (chan--; chan >= 0; chan--) {
		i2c_del_mux_adapter(data->virt_adaps[chan]);
    }
	kfree(data);
err:
	return ret;
}

static int accton_i2c_cpld_mux_remove(struct i2c_client *client)
{
    struct accton_i2c_cpld_mux *data = i2c_get_clientdata(client);
    const struct chip_desc *chip = &chips[data->type];
    int chan;

    sysfs_remove_file(&client->dev.kobj, &ver.attr);

    for (chan = 0; chan < chip->nchans; ++chan) {
        if (data->virt_adaps[chan]) {
            i2c_del_mux_adapter(data->virt_adaps[chan]);
            data->virt_adaps[chan] = NULL;
        }
    }

    kfree(data);
    accton_i2c_cpld_remove_client(client);

    return 0;
}

int as6712_32x_i2c_cpld_read(unsigned short cpld_addr, u8 reg)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EPERM;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client->addr == cpld_addr) {
            ret = i2c_smbus_read_byte_data(cpld_node->client, reg);
			break;
        }
    }

	mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(as6712_32x_i2c_cpld_read);

int as6712_32x_i2c_cpld_write(unsigned short cpld_addr, u8 reg, u8 value)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EIO;

	mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client->addr == cpld_addr) {
            ret = i2c_smbus_write_byte_data(cpld_node->client, reg, value);
			break;
        }
    }

	mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(as6712_32x_i2c_cpld_write);

static struct i2c_driver accton_i2c_cpld_mux_driver = {
	.driver		= {
		.name	= "as6712_32x_cpld",
		.owner	= THIS_MODULE,
	},
	.probe		= accton_i2c_cpld_mux_probe,
	.remove		= accton_i2c_cpld_mux_remove,
	.id_table	= accton_i2c_cpld_mux_id,
};

static int __init accton_i2c_cpld_mux_init(void)
{
    mutex_init(&list_lock);
    return i2c_add_driver(&accton_i2c_cpld_mux_driver);
}

static void __exit accton_i2c_cpld_mux_exit(void)
{
    i2c_del_driver(&accton_i2c_cpld_mux_driver);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("Accton I2C CPLD mux driver");
MODULE_LICENSE("GPL");

module_init(accton_i2c_cpld_mux_init);
module_exit(accton_i2c_cpld_mux_exit);
