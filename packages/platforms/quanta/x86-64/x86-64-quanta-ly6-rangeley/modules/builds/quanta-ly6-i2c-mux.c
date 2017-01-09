/*
 * <bsn.cl fy=2013 v=gpl>
 *
 *        Copyright 2013, 2014 BigSwitch Networks, Inc.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under  the terms ofthe GNU General Public License as
 * published by the Free Software Foundation;  either version 2 of the  License,
 * or (at your option) any later version.
 *
 *
 * </bsn.cl>
 *
 * An I2C multiplexer driver for the Quanta LY6
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/gpio.h>
#include <linux/platform_data/pca953x.h>

/*
 * Disable QSFP module reset (n=1..16)
 *       RST_N_Px1   INTL_Px1    MOD_ASB_Px1 LPMODE_Px1  RST_N_Px2   INTL_Px2    MOD_ASB_Px2 LPMODE_Px2
 *       7           6           5           4           3           2           1           0
 * port  2(n-1)+1    2(n-1)+1    2(n-1)+1    2(n-1)+1           2(n-1)      2(n-1)      2(n-1)      2(n-1)
 */
#define QUANTA_LY6_I2C_MUX_NUM_READ_GPIOS 32
#define QUANTA_LY6_I2C_MUX_NUM_WRITE_GPIOS 32
#define QUANTA_LY6_I2C_MUX_NUM_GPIO_GROUPS 4
#define QUANTA_LY6_I2C_MUX_NUM_GPIOS (QUANTA_LY6_I2C_MUX_NUM_READ_GPIOS + QUANTA_LY6_I2C_MUX_NUM_WRITE_GPIOS)
#define QUANTA_LY6_I2C_MUX_NUM_GPIO_PINS_PER_GROUP (QUANTA_LY6_I2C_MUX_NUM_GPIOS / QUANTA_LY6_I2C_MUX_NUM_GPIO_GROUPS)

struct quanta_ly6_i2c_mux {
	struct i2c_client *client;
	struct gpio_chip gpio_chip;
	u16 gpio_write_val;
};

static const struct i2c_device_id quanta_ly6_i2c_mux_id[] = {
	{"quanta_ly6_i2c_mux", 0xf600},
	{}
};

MODULE_DEVICE_TABLE(i2c, quanta_ly6_i2c_mux_id);

/*
 * Read 2 bytes once per command, and command should be group 1~4
 */
static int quanta_ly6_i2c_mux_reg_read(struct i2c_adapter *adap,
				struct i2c_client *client,
				u8 command, u16 *val)
{
	int ret = -ENODEV;

	if (adap->algo->master_xfer) {
		struct i2c_msg msg[2];
		char buf[4];

		msg[0].addr = client->addr;
		msg[0].flags = 0;
		msg[0].len = 1;
		buf[0] = command;
		msg[0].buf = &buf[0];

		/* always receive 3 bytes, else the PLD freaks out */
		msg[1].addr = client->addr;
		msg[1].flags = I2C_M_RD;
		msg[1].len = 3;
		msg[1].buf = &buf[1];

		ret = adap->algo->master_xfer(adap, msg, 2);
		if (val != NULL && ret > -1)
			*val = ((buf[2] << 8) | buf[1]);
	} else {
		union i2c_smbus_data data;
                data.block[0] = 3; /* block transfer length */
		ret = adap->algo->smbus_xfer(adap, client->addr,
						client->flags,
						I2C_SMBUS_READ,
						command,
						I2C_SMBUS_I2C_BLOCK_DATA, &data);
		if (val != NULL)
			*val = ((data.block[2] << 8) | data.block[1]);
	}

	return ret;
}

/*
 * Write 3 bytes once per command, and command should be group 1~4
 */
static int quanta_ly6_i2c_mux_reg_write(struct i2c_adapter *adap,
				struct i2c_client *client,
				u8 command, u16 val)
{
	int ret = -ENODEV;

	if (adap->algo->master_xfer) {
		struct i2c_msg msg;
		char buf[4];

		msg.addr = client->addr;
		msg.flags = 0;
		msg.len = 3;
		buf[0] = command;
		buf[1] = (val & 0xff);
		buf[2] = ((val >> 8) & 0xff);
		buf[3] = 0;
		msg.buf = buf;
		ret = adap->algo->master_xfer(adap, &msg, 1);
	} else {
		union i2c_smbus_data data;

		data.block[0] = 3;
		data.block[1] = (val & 0xff);
		data.block[2] = ((val >> 8) & 0xff);
		data.block[3] = 0;

		ret = adap->algo->smbus_xfer(adap, client->addr,
						client->flags,
						I2C_SMBUS_WRITE,
						command,
						I2C_SMBUS_I2C_BLOCK_DATA, &data);
	}

	return ret;
}

static void quanta_ly6_i2c_mux_gpio_set(struct gpio_chip *gc, unsigned offset,
				int val)
{
	struct quanta_ly6_i2c_mux *data = container_of(
		gc, struct quanta_ly6_i2c_mux, gpio_chip);
	int ret;
	u32 group;

	/* ignore write attempts to input GPIOs */
	if ((offset % 4) == 1 || (offset % 4) == 2) {
		dev_warn(&data->client->dev,
			"ignoring GPIO write for input for pin %d\n",
			offset);
		return;
	}

	group = (offset / QUANTA_LY6_I2C_MUX_NUM_GPIO_PINS_PER_GROUP) + 1;
	ret = quanta_ly6_i2c_mux_reg_read(data->client->adapter,
		data->client,
		group,
		&data->gpio_write_val);

	if (ret < 0) {
		dev_err(&data->client->dev,
			"quanta_ly6_i2c_mux_reg_read failed\n");
		return;
	}

	if (val)
		data->gpio_write_val |= (1 << (offset % QUANTA_LY6_I2C_MUX_NUM_GPIO_PINS_PER_GROUP));
	else
		data->gpio_write_val &= ~(1 << (offset % QUANTA_LY6_I2C_MUX_NUM_GPIO_PINS_PER_GROUP));

	quanta_ly6_i2c_mux_reg_write(
		data->client->adapter, data->client,
		group,
		data->gpio_write_val);
}

static int quanta_ly6_i2c_mux_gpio_get(struct gpio_chip *gc, unsigned offset)
{
	int ret;
	u16 buf;
	u32 group;
	struct quanta_ly6_i2c_mux *data = container_of(
		gc, struct quanta_ly6_i2c_mux, gpio_chip);

	if (offset >= (QUANTA_LY6_I2C_MUX_NUM_GPIOS)) {
		offset -= (QUANTA_LY6_I2C_MUX_NUM_GPIOS);
	}

	group = (offset / QUANTA_LY6_I2C_MUX_NUM_GPIO_PINS_PER_GROUP) + 1;
	buf = 0;
	ret = quanta_ly6_i2c_mux_reg_read(data->client->adapter,
		data->client,
		group,
		&buf);

	if (ret < 0) {
		dev_err(&data->client->dev,
			"quanta_ly6_i2c_mux_reg_read failed\n");
		return 0;
	}
	return (buf & (1 << (offset % QUANTA_LY6_I2C_MUX_NUM_GPIO_PINS_PER_GROUP))) ? 1 : 0;
}

static struct gpio_chip quanta_ly6_i2c_mux_gpio_chip = {
	.label = "quanta_ly6_i2c_mux_gpio_chip",
	.owner = THIS_MODULE,
	.ngpio = QUANTA_LY6_I2C_MUX_NUM_READ_GPIOS + QUANTA_LY6_I2C_MUX_NUM_WRITE_GPIOS,
	.base = -1,
	.set = quanta_ly6_i2c_mux_gpio_set,
	.get = quanta_ly6_i2c_mux_gpio_get,
};

static int quanta_ly6_i2c_mux_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct i2c_adapter *adap = client->adapter;
	struct pca953x_platform_data *pdata;
	struct quanta_ly6_i2c_mux *data;
	int ret = -ENODEV, gpio_base = -1;

	if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE_DATA))
		goto err;

	data = kzalloc(sizeof(struct quanta_ly6_i2c_mux), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto err;
	}

	i2c_set_clientdata(client, data);
	data->client = client;

	if (i2c_smbus_write_byte(client, 0) < 0) {
		dev_warn(&client->dev, "probe failed\n");
		goto exit_free;
	}

	data->gpio_chip = quanta_ly6_i2c_mux_gpio_chip;
	data->gpio_chip.dev = &client->dev;
	data->gpio_write_val = 0xff;

	pdata = client->dev.platform_data;
	if(pdata) {
		gpio_base = (int) pdata->gpio_base;
	}
	data->gpio_chip.base = gpio_base;

	ret = gpiochip_add(&data->gpio_chip);
	if (ret) {
		dev_err(&client->dev, "failed to register GPIOs\n");
		goto exit_free;
	}

	dev_info(&client->dev,
		"registered GPIOs for I2C mux %s (%d read, %d write)\n",
		client->name,
		QUANTA_LY6_I2C_MUX_NUM_READ_GPIOS,
		QUANTA_LY6_I2C_MUX_NUM_WRITE_GPIOS);

	return 0;

exit_free:
	kfree(data);
err:
	return ret;
}

static int quanta_ly6_i2c_mux_remove(struct i2c_client *client)
{
	struct quanta_ly6_i2c_mux *data = i2c_get_clientdata(client);
	int ret;

	ret = gpiochip_remove(&data->gpio_chip);
	if (ret)
		return ret;

	kfree(data);
	return 0;
}

static struct i2c_driver quanta_ly6_i2c_mux_driver = {
	.driver = {
		   .name = "quanta_ly6_i2c_mux",
		   .owner = THIS_MODULE,
		   },
	.probe = quanta_ly6_i2c_mux_probe,
	.remove = quanta_ly6_i2c_mux_remove,
	.id_table = quanta_ly6_i2c_mux_id,
};

module_i2c_driver(quanta_ly6_i2c_mux_driver);

MODULE_AUTHOR("QCT Technical <support@quantaqct.com>");
MODULE_DESCRIPTION("Quanta LY6 I2C multiplexer driver");
MODULE_LICENSE("GPL");
