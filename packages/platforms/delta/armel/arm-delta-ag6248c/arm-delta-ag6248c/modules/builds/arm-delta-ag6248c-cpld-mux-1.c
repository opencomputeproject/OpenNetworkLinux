/*
 * An I2C multiplexer dirver for delta as5812 CPLD
 *
 * Copyright (C) 2017 Delta Networks, Inc.
 * Brandon Chuang <brandon_chuang@delta.com.tw>
 *
 * This module supports the delta cpld that hold the channel select
 * mechanism for other i2c slave devices, such as SFP.
 * This includes the:
 *	 Delta ag7648c CPLD1/CPLD2/CPLD3
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
#include <linux/version.h>

#define CTRL_CPLD_BUS			0x0
#define CTRL_CPLD_I2C_ADDR		0x28
#define PARENT_CHAN			0x0
#define NUM_OF_CPLD_CHANS		0x2

#define CPLD_CHANNEL_SELECT_REG		0x19
#define CPLD_CHANNEL_SELECT_MASK	0x3
#define CPLD_CHANNEL_SELECT_OFFSET	0x0

#define CPLD_DESELECT_CHANNEL		0xff

#define CPLD_MUX_MAX_NCHANS		0x2
enum cpld_mux_type {
    delta_cpld_mux
};

struct delta_i2c_cpld_mux {
    enum cpld_mux_type type;
    struct i2c_adapter *virt_adaps[CPLD_MUX_MAX_NCHANS];
    u8 last_chan;  /* last register value */
};

struct chip_desc {
    u8   nchans;
    u8   deselectChan;
};

/* Provide specs for the PCA954x types we know about */
static const struct chip_desc chips[] = {
    [delta_cpld_mux] = {
    .nchans        = NUM_OF_CPLD_CHANS,
    .deselectChan  = CPLD_DESELECT_CHANNEL,
    }
};

static struct delta_i2c_cpld_mux *cpld_mux_data;

static struct device dump_dev;

/* Write to mux register. Don't use i2c_transfer()/i2c_smbus_xfer()
   for this as they will try to lock adapter a second time */
static int delta_i2c_cpld_mux_reg_write(struct i2c_adapter *adap,
			     struct i2c_client *client, u8 val)
{
	unsigned long orig_jiffies;
	unsigned short flags;
	union i2c_smbus_data data;
	struct i2c_adapter *ctrl_adap;
	int try;
	s32 res = -EIO;
	u8  reg_val = 0;

	data.byte = val;
	flags = 0;

	ctrl_adap = i2c_get_adapter(CTRL_CPLD_BUS);
	if (!ctrl_adap)
		return res;

	// try to lock it
	if (ctrl_adap->algo->smbus_xfer) {
		/* Retry automatically on arbitration loss */
		orig_jiffies = jiffies;
		for (res = 0, try = 0; try <= ctrl_adap->retries; try++) {
			// read first
			res = ctrl_adap->algo->smbus_xfer(ctrl_adap, CTRL_CPLD_I2C_ADDR, flags,
                             I2C_SMBUS_READ, CPLD_CHANNEL_SELECT_REG,
                             I2C_SMBUS_BYTE_DATA, &data);
			if (res && res != -EAGAIN)
				break;

			// modify the field we wanted
			data.byte &= ~(CPLD_CHANNEL_SELECT_MASK << CPLD_CHANNEL_SELECT_OFFSET);
			reg_val   |=  ((val & CPLD_CHANNEL_SELECT_MASK) << CPLD_CHANNEL_SELECT_OFFSET);
			data.byte |= reg_val;

			// modify the register
			res = ctrl_adap->algo->smbus_xfer(ctrl_adap, CTRL_CPLD_I2C_ADDR, flags,
                             I2C_SMBUS_WRITE, CPLD_CHANNEL_SELECT_REG,
                             I2C_SMBUS_BYTE_DATA, &data);
			if (res && res != -EAGAIN)
				break;
			if (time_after(jiffies,
			    orig_jiffies + ctrl_adap->timeout))
				break;
		}
	}

    return res;
}

static int delta_i2c_cpld_mux_select_chan(struct i2c_adapter *adap,
			       void *client, u32 chan)
{
	u8 regval;
	int ret = 0;
	regval = chan;

	/* Only select the channel if its different from the last channel */
	if (cpld_mux_data->last_chan != regval) {
		ret = delta_i2c_cpld_mux_reg_write(NULL, NULL, regval);
		cpld_mux_data->last_chan = regval;
	}

	return ret;
}

static int delta_i2c_cpld_mux_deselect_mux(struct i2c_adapter *adap,
				void *client, u32 chan)
{
	/* Deselect active channel */
	cpld_mux_data->last_chan = chips[cpld_mux_data->type].deselectChan;

	return delta_i2c_cpld_mux_reg_write(NULL, NULL, cpld_mux_data->last_chan);
}

/*
 * I2C init/probing/exit functions
 */
static int __delta_i2c_cpld_mux_init(void)
{
	struct i2c_adapter *adap = i2c_get_adapter(PARENT_CHAN);
	int chan=0;
	int ret = -ENODEV;

	memset (&dump_dev, 0, sizeof(dump_dev));

	if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE))
		goto err;

	if (!adap)
		goto err;

	cpld_mux_data = kzalloc(sizeof(struct delta_i2c_cpld_mux), GFP_KERNEL);
	if (!cpld_mux_data) {
		ret = -ENOMEM;
		goto err;
	}

	cpld_mux_data->type = delta_cpld_mux;
	cpld_mux_data->last_chan = chips[cpld_mux_data->type].deselectChan; /* force the first selection */

	/* Now create an adapter for each channel */
	for (chan = 0; chan < NUM_OF_CPLD_CHANS; chan++) {
		cpld_mux_data->virt_adaps[chan] = i2c_add_mux_adapter(adap, &dump_dev, NULL, 0,
					chan,
					delta_i2c_cpld_mux_select_chan,
					delta_i2c_cpld_mux_deselect_mux);

		if (cpld_mux_data->virt_adaps[chan] == NULL) {
			ret = -ENODEV;
			printk("failed to register multiplexed adapter %d, parent %d\n", chan, PARENT_CHAN);
			goto virt_reg_failed;
		}
	}

	printk("registered %d multiplexed busses for I2C mux bus %d\n",
		chan, PARENT_CHAN);

	return 0;

virt_reg_failed:
	for (chan--; chan >= 0; chan--) {
		i2c_del_mux_adapter(cpld_mux_data->virt_adaps[chan]);
	}

	kfree(cpld_mux_data);
err:
	return ret;
}

static int __delta_i2c_cpld_mux_remove(void)
{
    const struct chip_desc *chip = &chips[cpld_mux_data->type];
    int chan;

    for (chan = 0; chan < chip->nchans; ++chan) {
	if (cpld_mux_data->virt_adaps[chan]) {
		i2c_del_mux_adapter(cpld_mux_data->virt_adaps[chan]);
		cpld_mux_data->virt_adaps[chan] = NULL;
	}
    }

    kfree(cpld_mux_data);

    return 0;
}

static int __init delta_i2c_cpld_mux_init(void)
{
	return __delta_i2c_cpld_mux_init ();
}

static void __exit delta_i2c_cpld_mux_exit(void)
{
	__delta_i2c_cpld_mux_remove ();
}

MODULE_AUTHOR("Dave Hu <dave.hu@deltasystems.com>");
MODULE_DESCRIPTION("Delta I2C CPLD mux driver");
MODULE_LICENSE("GPL");

module_init(delta_i2c_cpld_mux_init);
module_exit(delta_i2c_cpld_mux_exit);

