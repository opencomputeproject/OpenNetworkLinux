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
 * An I2C multiplexer driver for the Quanta LY2
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

#define QUANTA_LY2_I2C_MUX_CHANNEL_FIRST 1

#define QUANTA_LY2_I2C_MUX_NUM_CHANNELS 16

/*
 * 16 read GPIOs, 8 write GPIOs,
 * treat them as a single GPIO chip,
 * with the read GPIOs occurring first
 */
#define QUANTA_LY2_I2C_MUX_NUM_READ_GPIOS 16
#define QUANTA_LY2_I2C_MUX_NUM_WRITE_GPIOS 8

#define QUANTA_LY2_I2C_MUX_CMD_GET_GPIO 0
#define QUANTA_LY2_I2C_MUX_CMD_SET_GPIO 1
#define QUANTA_LY2_I2C_MUX_CMD_SET_CHANNEL 2

struct quanta_ly2_i2c_mux {
	struct i2c_client *client;
	struct i2c_adapter *chan_adap[QUANTA_LY2_I2C_MUX_NUM_CHANNELS];
	struct gpio_chip gpio_chip;
	u8 last_chan;
	u8 gpio_write_val;
};

static const struct i2c_device_id quanta_ly2_i2c_mux_id[] = {
	{"quanta_ly_i2c_mux", 0},
	{"quanta_ly2_i2c_mux", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, quanta_ly2_i2c_mux_id);

/*
 * pld.c does 3-byte transactions, but none of the GPIO
 * definitions require more than 16 bits
 */
static int quanta_ly2_i2c_mux_reg_read(struct i2c_adapter *adap,
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
 * pld.c shows 3-byte output transactions;
 * in our case we only need 8 bits to
 * (1) select one of the 16 muxed i2c devices,
 * (2) drive one of the 8 defined GPIO outputs
 */
static int quanta_ly2_i2c_mux_reg_write(struct i2c_adapter *adap,
				struct i2c_client *client,
				u8 command, u8 val)
{
	int ret = -ENODEV;

	if (adap->algo->master_xfer) {
		struct i2c_msg msg;
		char buf[4];

		msg.addr = client->addr;
		msg.flags = 0;
		msg.len = 3;
		buf[0] = command;
		buf[1] = val;
                buf[2] = 0;
                buf[3] = 0;
		msg.buf = buf;
		ret = adap->algo->master_xfer(adap, &msg, 1);
	} else {
		union i2c_smbus_data data;

                data.block[0] = 3;
                data.block[1] = val;
                data.block[2] = 0;
                data.block[3] = 0;

		ret = adap->algo->smbus_xfer(adap, client->addr,
					     client->flags,
					     I2C_SMBUS_WRITE,
					     command,
					     I2C_SMBUS_I2C_BLOCK_DATA, &data);
	}

	return ret;
}

static void quanta_ly2_i2c_mux_gpio_set(struct gpio_chip *gc, unsigned offset,
				int val)
{
	struct quanta_ly2_i2c_mux *data = container_of(
		gc, struct quanta_ly2_i2c_mux, gpio_chip);

        /* ignore write attempts to input GPIOs */
        if (offset < QUANTA_LY2_I2C_MUX_NUM_READ_GPIOS) {
                dev_warn(&data->client->dev,
                         "ignoring GPIO write for input for pin %d\n",
                         offset);
                return;
        }
        offset -= QUANTA_LY2_I2C_MUX_NUM_READ_GPIOS;

	if (val)
		data->gpio_write_val |= (1 << offset);
	else
		data->gpio_write_val &= ~(1 << offset);

	quanta_ly2_i2c_mux_reg_write(
		data->client->adapter, data->client,
		QUANTA_LY2_I2C_MUX_CMD_SET_GPIO,
		data->gpio_write_val);
}

/*
 * "read" one of the GPIOs.
 * The first 16 (QUANTA_LY2_I2C_MUX_NUM_READ_GPIOS)
 * are actual input GPIOs.
 * the last 8 (QUANTA_LY2_I2C_MUX_NUM_WRITE_GPIOS)
 * are output GPIOs, so readback just returns the cached value.
 */
static int quanta_ly2_i2c_mux_gpio_get(struct gpio_chip *gc, unsigned offset)
{
        int ret;
        u16 buf;
	struct quanta_ly2_i2c_mux *data = container_of(
		gc, struct quanta_ly2_i2c_mux, gpio_chip);

        if (offset >= QUANTA_LY2_I2C_MUX_NUM_READ_GPIOS) {
                offset -= QUANTA_LY2_I2C_MUX_NUM_READ_GPIOS;
                return (data->gpio_write_val & (1 << offset)) ? 1 : 0;
        }

        /* else, do a proper gpio read */
        buf = 0;
        ret = quanta_ly2_i2c_mux_reg_read(data->client->adapter,
                                          data->client,
                                          QUANTA_LY2_I2C_MUX_CMD_GET_GPIO,
                                          &buf);
        if (ret < 0) {
                dev_err(&data->client->dev,
                        "quanta_ly2_i2c_mux_reg_read failed\n");
                return 0;
        }
        return (buf & (1 << offset)) ? 1 : 0;
}

static int quanta_ly2_i2c_mux_select_chan(struct i2c_adapter *adap,
					void *client, u32 chan)
{
	struct quanta_ly2_i2c_mux *data = i2c_get_clientdata(client);
	int ret = 0;
	u32 c = QUANTA_LY2_I2C_MUX_CHANNEL_FIRST + chan;

	if (data->last_chan != c) {
		ret = quanta_ly2_i2c_mux_reg_write(
			adap, client,
			QUANTA_LY2_I2C_MUX_CMD_SET_CHANNEL, c);
		data->last_chan = c;
	}

	return ret;
}

static int quanta_ly2_i2c_mux_release_chan(struct i2c_adapter *adap,
					  void *client, u32 chan)
{
	struct quanta_ly2_i2c_mux *data = i2c_get_clientdata(client);
	int ret = 0;

	ret = quanta_ly2_i2c_mux_reg_write(
		adap, client,
		QUANTA_LY2_I2C_MUX_CMD_SET_CHANNEL, 0);
	data->last_chan = 0;

	return ret;
}

static struct gpio_chip quanta_ly2_i2c_mux_gpio_chip = {
	.label = "quanta_ly2_i2c_mux_gpio_chip",
	.owner = THIS_MODULE,
	.ngpio = QUANTA_LY2_I2C_MUX_NUM_READ_GPIOS + QUANTA_LY2_I2C_MUX_NUM_WRITE_GPIOS,
	.base = -1,
	.set = quanta_ly2_i2c_mux_gpio_set,
	.get = quanta_ly2_i2c_mux_gpio_get,
};

static int quanta_ly2_i2c_mux_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct i2c_adapter *adap = client->adapter;
	int chan;
	struct quanta_ly2_i2c_mux *data;
	int ret = -ENODEV;

	if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE_DATA))
		goto err;

	data = kzalloc(sizeof(struct quanta_ly2_i2c_mux), GFP_KERNEL);
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

	i2c_lock_adapter(adap);
	quanta_ly2_i2c_mux_release_chan(adap, client, 0);
	i2c_unlock_adapter(adap);

	for (chan = 0; chan < QUANTA_LY2_I2C_MUX_NUM_CHANNELS; chan++) {
		data->chan_adap[chan] =
			i2c_add_mux_adapter(adap, &client->dev, client,
					0, chan, 0,
					quanta_ly2_i2c_mux_select_chan,
					quanta_ly2_i2c_mux_release_chan);

		if (data->chan_adap[chan] == NULL) {
			ret = -ENODEV;
			dev_err(&client->dev,
				"failed to register multiplexed adapter %d\n",
				chan);
			goto adap_reg_failed;
		}
	}

	dev_info(&client->dev,
		"registered %d multiplexed buses for I2C mux %s\n",
		chan, client->name);

	data->gpio_chip = quanta_ly2_i2c_mux_gpio_chip;
	data->gpio_chip.dev = &client->dev;
	data->gpio_write_val = 0xff;

	ret = gpiochip_add(&data->gpio_chip);
	if (ret) {
		dev_err(&client->dev, "failed to register GPIOs\n");
		goto adap_reg_failed;
	}

	dev_info(&client->dev,
		"registered GPIOs for I2C mux %s (%d read, %d write)\n",
		client->name,
		QUANTA_LY2_I2C_MUX_NUM_READ_GPIOS,
		QUANTA_LY2_I2C_MUX_NUM_WRITE_GPIOS);

	return 0;

adap_reg_failed:
	for (chan--; chan >= 0; chan--)
		i2c_del_mux_adapter(data->chan_adap[chan]);

exit_free:
	kfree(data);
err:
	return ret;
}

static int quanta_ly2_i2c_mux_remove(struct i2c_client *client)
{
	struct quanta_ly2_i2c_mux *data = i2c_get_clientdata(client);
	int chan, ret;

	for (chan = 0; chan < QUANTA_LY2_I2C_MUX_NUM_CHANNELS; chan++)
		if (data->chan_adap[chan]) {
			i2c_del_mux_adapter(data->chan_adap[chan]);
			data->chan_adap[chan] = NULL;
		}

	ret = gpiochip_remove(&data->gpio_chip);
	if (ret)
		return ret;

	kfree(data);
	return 0;
}

static struct i2c_driver quanta_ly2_i2c_mux_driver = {
	.driver = {
		   .name = "quanta_ly2_i2c_mux",
		   .owner = THIS_MODULE,
		   },
	.probe = quanta_ly2_i2c_mux_probe,
	.remove = quanta_ly2_i2c_mux_remove,
	.id_table = quanta_ly2_i2c_mux_id,
};

module_i2c_driver(quanta_ly2_i2c_mux_driver);

MODULE_AUTHOR("Big Switch Networks <support@bigswitch.com>");
MODULE_DESCRIPTION("Quanta LY2 I2C multiplexer driver");
MODULE_LICENSE("GPL");
