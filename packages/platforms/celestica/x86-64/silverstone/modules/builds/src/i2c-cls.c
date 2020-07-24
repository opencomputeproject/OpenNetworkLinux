// SPDX-License-Identifier: GPL-2.0+
/*
 * i2c-cls.c - I2C bus driver for Celestica pci-to-i2c controller.
 *
 * Pradchaya Phucharoen <pphuchar@celestica.com>
 * Copyright (C) 2019 Celestica Corp.
 */

/* Based on:  i2c-mpc.c */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/log2.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/sched/signal.h>
#include "i2c-cls.h"

/* The registers for one core */
#define CLS_I2C_FDR      0
#define CLS_I2C_CR	 1
#define CLS_I2C_SR	 2
#define CLS_I2C_DR	 3
#define CLS_I2C_PORT_ID  4

#define CCR_MEN  0x80
#define CCR_MIEN 0x40
#define CCR_MSTA 0x20
#define CCR_MTX  0x10
#define CCR_TXAK 0x08
#define CCR_RSTA 0x04

#define CSR_MCF  0x80
#define CSR_MAAS 0x40
#define CSR_MBB  0x20
#define CSR_MAL  0x10
#define CSR_SRW  0x04
#define CSR_MIF  0x02
#define CSR_RXAK 0x01

/* 
 * struct cls_i2c - private data for cls i2c core 
 * @base:		virtual base address
 * @reg_shift:		number of bytes between register
 * @port_id:		i2c core id
 * @timeout_us:		adapter timeout in jiffies
 * @fdr: 		a frequency divier corresponding to each bus speed
 * @bus_clk_khz:	bus clock for reference
 * @adap:		i2c adapter instance
 * @setreg:		accessor function to set register
 * @getreg:		accessor function to get register
 */
struct cls_i2c {
	void __iomem *base;
	unsigned int reg_shift;
	unsigned int port_id;
	unsigned int timeout_us;
	unsigned int fdr;
	unsigned int bus_clk_khz;
	struct i2c_adapter adap;
	void (*setreg)(struct cls_i2c *i2c, int reg, u8 value);
	u8 (*getreg)(struct cls_i2c *i2c, int reg);
};

/* accessor function for 8 bit io space */
static void setreg_8(struct cls_i2c *i2c, int reg, u8 value)
{
	iowrite8(value, i2c->base + (reg * i2c->reg_shift));
}

static inline u8 getreg_8(struct cls_i2c *i2c, int reg)
{
	return ioread8(i2c->base + (reg * i2c->reg_shift));
}

/* Accesser wrapper */
static inline void cls_setreg(struct cls_i2c *i2c, int reg, u8 value)
{
	i2c->setreg(i2c, reg, value);
}

static inline u8 cls_getreg(struct cls_i2c *i2c, int reg)
{
	return i2c->getreg(i2c, reg);
}

/* Sometimes 9th clock pulse isn't generated, and slave doesn't release
 * the bus, because it wants to send ACK.
 * Following sequence of enabling/disabling and sending start/stop generates
 * the 9 pulses, so it's all OK.
 */
static void cls_i2c_fixup(struct cls_i2c *i2c)
{
	int k;
	u32 delay_us = 1000000 / i2c->bus_clk_khz + 1;

	if (delay_us < 2)
		delay_us = 2;

	for (k = 9; k; k--) {
		cls_setreg(i2c, CLS_I2C_CR, 0);
		cls_setreg(i2c, CLS_I2C_CR, CCR_MSTA | CCR_MTX | CCR_MEN);
		cls_getreg(i2c, CLS_I2C_DR);
		cls_setreg(i2c, CLS_I2C_CR, CCR_MEN);
		udelay(delay_us << 1); /* Why delay x2 here? */
	}
}

static int i2c_wait(struct cls_i2c *i2c, unsigned timeout, int writing)
{
	unsigned long orig_jiffies = jiffies;
	u32 cmd_err;
	int result = 0;

	while (!(cls_getreg(i2c, CLS_I2C_SR) & CSR_MIF)) {
		schedule();
		if (time_after(jiffies, orig_jiffies + timeout)) {
			dev_dbg(&i2c->adap.dev, "timeout\n");
			cls_setreg(i2c, CLS_I2C_CR, 0);
			result = -ETIMEDOUT;
			break;
		}
	}
	cmd_err = cls_getreg(i2c, CLS_I2C_SR);
	cls_setreg(i2c, CLS_I2C_SR, 0);

	if (result < 0)
		return result;

	if (!(cmd_err & CSR_MCF)) {
		dev_dbg(&i2c->adap.dev, "unfinished\n");
		return -EIO;
	}

	if (cmd_err & CSR_MAL) {
		dev_dbg(&i2c->adap.dev, "MAL\n");
		return -EAGAIN;
	}

	if (writing && (cmd_err & CSR_RXAK)) {
		dev_dbg(&i2c->adap.dev, "No RXAK\n");
		/* generate stop */
		cls_setreg(i2c, CLS_I2C_CR, CCR_MEN);
		return -ENXIO;
	}
	return 0;
}

static void cls_i2c_start(struct cls_i2c *i2c)
{
	/* Clear arbitration */
	cls_setreg(i2c, CLS_I2C_SR, 0);
	/* Start with MEN */
	cls_setreg(i2c, CLS_I2C_CR, CCR_MEN);
}

static void cls_i2c_stop(struct cls_i2c *i2c)
{
	cls_setreg(i2c, CLS_I2C_CR, CCR_MEN);
}

static int cls_write(struct cls_i2c *i2c, int target,
			const u8 *data, int length, int restart)
{
	int i, result;
	unsigned timeout = i2c->adap.timeout;
	u32 flags = restart ? CCR_RSTA : 0;

	/* Start as master */
	cls_setreg(i2c, CLS_I2C_CR, 
			CCR_MIEN | CCR_MEN | CCR_MSTA | CCR_MTX | flags);
	/* Write target byte */
	cls_setreg(i2c, CLS_I2C_DR, (target << 1));

	result = i2c_wait(i2c, timeout, 1);
	if (result < 0)
		return result;

	for (i = 0; i < length; i++) {
		/* Write data byte */
		cls_setreg(i2c, CLS_I2C_DR, data[i]);

		result = i2c_wait(i2c, timeout, 1);
		if (result < 0)
			return result;
	}

	return 0;
}

static int cls_read(struct cls_i2c *i2c, int target,
			u8 *data, int length, int restart, bool recv_len)
{
	unsigned timeout = i2c->adap.timeout;
	int i, result;
	u32 flags = restart ? CCR_RSTA : 0;

	/* Switch to read - restart */
	cls_setreg(i2c, CLS_I2C_CR, 
		   CCR_MIEN | CCR_MEN | CCR_MSTA | CCR_MTX | flags);
	/* Write target address byte - this time with the read flag set */
	cls_setreg(i2c, CLS_I2C_DR, (target << 1) | 1);

	result = i2c_wait(i2c, timeout, 1);
	if (result < 0)
		return result;

	if (length) {
		if (length == 1 && !recv_len)
			cls_setreg(i2c, CLS_I2C_CR,
			           CCR_MIEN | CCR_MEN | CCR_MSTA | CCR_TXAK);
		else
			cls_setreg(i2c, CLS_I2C_CR, 
				   CCR_MIEN | CCR_MEN | CCR_MSTA);
		/* Dummy read */
		cls_getreg(i2c, CLS_I2C_DR);
	}

	for (i = 0; i < length; i++) {
		u8 byte;

		result = i2c_wait(i2c, timeout, 0);
		if (result < 0)
			return result;

		/*
		 * For block reads, we have to know the total length (1st byte)
		 * before we can determine if we are done.
		 */
		if (i || !recv_len) {
			/* Generate txack on next to last byte */
			if (i == length - 2)
				cls_setreg(i2c, CLS_I2C_CR, CCR_MIEN 
						| CCR_MEN | CCR_MSTA
						| CCR_TXAK);
			/* Do not generate stop on last byte */
			if (i == length - 1)
				cls_setreg(i2c, CLS_I2C_CR, CCR_MIEN 
						| CCR_MEN | CCR_MSTA
						| CCR_MTX);
		}

		byte = cls_getreg(i2c, CLS_I2C_DR);

		/*
		 * Adjust length if first received byte is length.
		 * The length is 1 length byte plus actually data length
		 */
		if (i == 0 && recv_len) {
			if (byte == 0 || byte > I2C_SMBUS_BLOCK_MAX)
				return -EPROTO;
			length += byte;
			/*
			 * For block reads, generate txack here if data length
			 * is 1 byte (total length is 2 bytes).
			 */
			if (length == 2)
				cls_setreg(i2c, CLS_I2C_CR, CCR_MIEN 
						| CCR_MEN | CCR_MSTA 
						| CCR_TXAK);
		}
		data[i] = byte;
	}

	return length;
}

static int cls_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	struct i2c_msg *pmsg;
	int i;
	int ret = 0;
	unsigned long orig_jiffies = jiffies;
	struct cls_i2c *i2c = i2c_get_adapdata(adap);

	cls_i2c_start(i2c);

	/* Allow bus up to 1s to become not busy */
	while (cls_getreg(i2c, CLS_I2C_SR) & CSR_MBB) {
		if (signal_pending(current)) {
			dev_dbg(&i2c->adap.dev, "Interrupted\n");
			cls_setreg(i2c, CLS_I2C_CR, 0);
			return -EINTR;
		}
		if (time_after(jiffies, orig_jiffies + HZ)) {
			u8 status = cls_getreg(i2c, CLS_I2C_SR);

			dev_dbg(&i2c->adap.dev, "timeout\n");
			if ((status & (CSR_MCF | CSR_MBB | CSR_RXAK)) != 0) {
				cls_setreg(i2c, CLS_I2C_SR, status & ~CSR_MAL);
				cls_i2c_fixup(i2c);
			}
			return -EIO;
		}
		schedule();
	}

	for (i = 0; ret >= 0 && i < num; i++) {
		pmsg = &msgs[i];
		dev_dbg(&i2c->adap.dev,
			"Doing %s %d bytes to 0x%02x - %d of %d messages\n",
			pmsg->flags & I2C_M_RD ? "read" : "write",
			pmsg->len, pmsg->addr, i + 1, num);
		if (pmsg->flags & I2C_M_RD) {
			bool recv_len = pmsg->flags & I2C_M_RECV_LEN;

			ret = cls_read(i2c, pmsg->addr, pmsg->buf, pmsg->len, 
					i, recv_len);
			if (recv_len && ret > 0)
				pmsg->len = ret;
		} else {
			ret = cls_write(i2c, pmsg->addr, pmsg->buf, 
					pmsg->len, i);
		}
	}
	cls_i2c_stop(i2c); /* Initiate STOP */
	orig_jiffies = jiffies;
	/* Wait until STOP is seen, allow up to 1 s */
	while (cls_getreg(i2c, CLS_I2C_SR) & CSR_MBB) {
		if (time_after(jiffies, orig_jiffies + HZ)) {
			u8 status = readb(i2c->base + CLS_I2C_SR);

			dev_dbg(&i2c->adap.dev, "timeout\n");
			if ((status & (CSR_MCF | CSR_MBB | CSR_RXAK)) != 0) {
				cls_setreg(i2c, CLS_I2C_SR, status & ~CSR_MAL);
				cls_i2c_fixup(i2c);
			}
			return -EIO;
		}
		cond_resched();
	}
	return (ret < 0) ? ret : num;
}

static u32 cls_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL
		| I2C_FUNC_SMBUS_READ_BLOCK_DATA 
		| I2C_FUNC_SMBUS_BLOCK_PROC_CALL;
}

static struct i2c_algorithm cls_i2c_algorithm = {
	.master_xfer = cls_xfer,
	.functionality = cls_i2c_func,
};

static const struct i2c_adapter cls_i2c_adapter = {
	.owner = THIS_MODULE,
	.class = I2C_CLASS_DEPRECATED,
	.algo = &cls_i2c_algorithm,
	.nr = -1,
};

static int cls_i2c_probe(struct platform_device *pdev)
{
	struct cls_i2c *i2c;
	struct cls_i2c_platform_data *pdata;
	struct resource *res;
	int ret;
	int i;

	i2c = devm_kzalloc(&pdev->dev, sizeof(*i2c), GFP_KERNEL);
	if (!i2c){
		ret = -ENOMEM;
		goto err_exit;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res) {
		i2c->base = devm_ioremap_resource(&pdev->dev, res);
		if (IS_ERR(i2c->base)){
			ret = PTR_ERR(i2c->base);
			goto err_exit;
		}
	}
	
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata) {
		i2c->port_id = pdata->port_id;
		i2c->reg_shift = pdata->reg_shift;

		switch(pdata->bus_clk_mode){
		case CLK50KHZ:
			i2c->fdr = 63;
			i2c->bus_clk_khz = 50000;
			break;
		case CLK100KHZ:
			i2c->fdr = 31;
			i2c->bus_clk_khz = 100000;
			break;
		case CLK200KHZ:
			i2c->fdr = 15;
			i2c->bus_clk_khz = 200000;
			break;
		case CLK400KHZ:
			i2c->fdr = 7;
			i2c->bus_clk_khz = 400000;
			break;
		default:
			/* defaults set bus clock to 100Khz */
			i2c->fdr = 31;
			i2c->bus_clk_khz = 100000;
		break;
		}

		if(pdata->timeout_us)
			i2c->timeout_us = pdata->timeout_us;
		else	
			/* defaults set timeout to 1ms */
			i2c->timeout_us = 10000;
	}

	i2c->setreg = setreg_8;
	i2c->getreg = getreg_8;

	/* init the i2c register */
	cls_setreg(i2c, CLS_I2C_FDR, i2c->fdr);
	cls_setreg(i2c, CLS_I2C_PORT_ID, i2c->port_id);

	/* hook up driver to tree */
	platform_set_drvdata(pdev, i2c);
	i2c->adap = cls_i2c_adapter;
	scnprintf(i2c->adap.name, sizeof(i2c->adap.name),
			"I2C adapter at %llx", (unsigned long long)res->start);
	i2c_set_adapdata(&i2c->adap, i2c);
	i2c->adap.timeout = usecs_to_jiffies(i2c->timeout_us);
	i2c->adap.dev.parent = &pdev->dev;
	i2c->adap.nr = -1;

	if(pdata){
		if(pdata->adap_id >= 0)
			i2c->adap.nr = pdata->adap_id;
	}

	/* add i2c adapter to i2c tree */
	ret = i2c_add_numbered_adapter(&i2c->adap);
	if (ret)
		goto err_exit;

	/* add in known devices to the bus */
	if (pdata) {
		for (i = 0; i < pdata->num_devices; i++)
			i2c_new_device(&i2c->adap, pdata->devices + i);
	}

	return 0;

err_exit:
	return ret;
}

static int cls_i2c_remove(struct platform_device *pdev)
{
	struct cls_i2c *i2c = platform_get_drvdata(pdev);
	
	/* remove adapter & data */
	i2c_del_adapter(&i2c->adap);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int cls_i2c_suspend(struct device *dev)
{
	struct cls_i2c *i2c = dev_get_drvdata(dev);

	i2c->fdr = cls_getreg(i2c, CLS_I2C_FDR);
	i2c->port_id = cls_getreg(i2c, CLS_I2C_PORT_ID);

	return 0;
}

static int cls_i2c_resume(struct device *dev)
{
	struct cls_i2c *i2c = dev_get_drvdata(dev);

	cls_setreg(i2c, CLS_I2C_FDR, i2c->fdr);
	cls_setreg(i2c, CLS_I2C_PORT_ID, i2c->port_id);

	return 0;
}

static SIMPLE_DEV_PM_OPS(cls_i2c_pm, cls_i2c_suspend, cls_i2c_resume);
#define CLS_I2C_PM	(&cls_i2c_pm)
#else
#define CLS_I2C_PM	NULL
#endif


static struct platform_driver cls_i2c_driver = {
	.probe = cls_i2c_probe,
	.remove = cls_i2c_remove,
	.driver = {
		.name = "cls-i2c",
		.pm = CLS_I2C_PM,
	},
};

module_platform_driver(cls_i2c_driver);

MODULE_AUTHOR("Pradchaya Phucharoen<pphuchar@celestica.com>");
MODULE_DESCRIPTION("Celestica PCI to I2C bus driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:cls-i2c");