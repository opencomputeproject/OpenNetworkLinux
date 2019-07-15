
/*
 *
 * Copyright (C) 2017 Delta Networks, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/version.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#if 0
#include "x86-64-delta-ag8032-i2c-mux-cpld.h"
#else
struct i2c_mux_cpld_platform_data
{
	u8 cpld_bus;
	u8 cpld_addr;
	u8 cpld_reg;

	u8 parent_bus;

	u8 base_nr;

	const u8 *values;
	int n_values;
	bool idle_in_use;
	u8  idle;

	void *ctrl_adap;
};


#endif

//static DEFINE_MUTEX(locker);

struct cpldmux {
	struct i2c_mux_cpld_platform_data data;
};

static int i2c_mux_cpld_set(const struct cpldmux *mux, unsigned int chan_id)
{
	unsigned long orig_jiffies;
	unsigned short flags;
	union i2c_smbus_data data;
	struct i2c_adapter *ctrl_adap;
	int try;
	s32 res = -EIO;

	data.byte = chan_id;
	flags = 0;

	ctrl_adap = mux->data.ctrl_adap;
	if (!ctrl_adap)
		return res;

	// try to lock it
	if (ctrl_adap->algo->smbus_xfer) {
		/* Retry automatically on arbitration loss */
		orig_jiffies = jiffies;
		for (res = 0, try = 0; try <= ctrl_adap->retries; try++) {

			// modify the register
			res = ctrl_adap->algo->smbus_xfer(ctrl_adap,
				mux->data.cpld_addr, flags,
                             I2C_SMBUS_WRITE, mux->data.cpld_reg,
                             I2C_SMBUS_BYTE_DATA, &data);
			if (res && res != -EAGAIN)
				break;
			if (time_after(jiffies,
			    orig_jiffies + ctrl_adap->timeout))
				break;
		}
	}

	return 0;
}

static int i2c_mux_cpld_select (struct i2c_mux_core *muxc, u32 chan)
{
	struct cpldmux *mux = i2c_mux_priv(muxc);

	return i2c_mux_cpld_set(mux, chan);
}

static int i2c_mux_cpld_deselect (struct i2c_mux_core *muxc, u32 chan)
{
	struct cpldmux *mux = i2c_mux_priv(muxc);

	if (mux->data.idle_in_use)
		return i2c_mux_cpld_set(mux, mux->data.idle);

	return 0;
}

static int i2c_mux_cpld_probe(struct platform_device *pdev)
{
	struct i2c_mux_core *muxc;
	struct cpldmux *mux;
	struct i2c_adapter *parent;
	struct i2c_adapter *ctrl;
	int i, ret, nr;

	mux = devm_kzalloc(&pdev->dev, sizeof(*mux), GFP_KERNEL);
	if (!mux)
		return -ENOMEM;

	ctrl = NULL;
	parent = NULL;
	if (dev_get_platdata(&pdev->dev)) {
		memcpy(&mux->data, dev_get_platdata(&pdev->dev),
			sizeof(mux->data));

		parent = i2c_get_adapter(mux->data.parent_bus);
		if (!parent)
			return -EPROBE_DEFER;
		ctrl = i2c_get_adapter(mux->data.cpld_bus);
		if (!ctrl) {
			i2c_put_adapter(parent);
			return -EPROBE_DEFER;
		}

	}

	muxc = i2c_mux_alloc (parent, &pdev->dev,
			mux->data.n_values,
			0, 0,
			i2c_mux_cpld_select,
			i2c_mux_cpld_deselect);

	if (!muxc)
			return -ENOMEM;
	muxc->priv = mux;
	mux->data.ctrl_adap = ctrl;

	platform_set_drvdata(pdev, muxc);

	for (i = 0; i < mux->data.n_values; i++) {
		nr = mux->data.base_nr ? (mux->data.base_nr + i) : 0;

		ret = i2c_mux_add_adapter (muxc, nr, mux->data.values[i], 0);
		if (ret) {
			dev_err(&pdev->dev, "Failed to add adapter %d\n", i);
			goto add_adapter_failed;
		}
	}

	dev_dbg(&pdev->dev, "%d port mux on %s adapter\n",
		 mux->data.n_values, parent->name);

	return 0;

add_adapter_failed:

	i2c_put_adapter(ctrl);
	i2c_put_adapter(parent);
	i2c_mux_del_adapters(muxc);

	return ret;
}

static int i2c_mux_cpld_remove(struct platform_device *pdev)
{
	struct i2c_mux_core *muxc = platform_get_drvdata(pdev);
	struct cpldmux *mux = i2c_mux_priv(muxc);

	i2c_mux_del_adapters(muxc);
	i2c_put_adapter(mux->data.ctrl_adap);
	i2c_put_adapter(muxc->parent);

	return 0;
}

static struct platform_driver i2c_mux_cpld_driver = {
	.probe	= i2c_mux_cpld_probe,
	.remove	= i2c_mux_cpld_remove,
	.driver	= {
		.name	= "i2c-mux-cpld",
	},
};

module_platform_driver(i2c_mux_cpld_driver);

MODULE_AUTHOR("Dave Hu <dave.hu@deltaww.com>");
MODULE_DESCRIPTION("I2C CPLD mux driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:i2c-mux-cpld");

