// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * optoe.c - A driver to read and write the EEPROM on optical transceivers
 * (SFP, QSFP, CMIS (Common Management Interface Spec)
 * and similar I2C based devices)
 *
 * Copyright (C) 2014 Cumulus networks Inc.
 * Copyright (C) 2017 Finisar Corp.
 */

/*
 *	Description:
 *	a) Optical transceiver EEPROM read/write transactions are just like
 *		the at24 eeproms managed by the at24.c i2c driver
 *	b) The register/memory layout is up to 256 128 byte pages defined by
 *		a "pages valid" register and switched via a "page select"
 *		register as explained in below diagram.
 *	c) 256 bytes are mapped at a time. 'Lower page 00h' is the first 128
 *	        bytes of address space, and always references the same
 *	        location, independent of the page select register.
 *	        All mapped pages are mapped into the upper 128 bytes
 *	        (offset 128-255) of the i2c address.
 *	d) Devices with one I2C address (eg QSFP, CMIS) use I2C address 0x50
 *		(A0h in the spec), and map all pages in the upper 128 bytes
 *		of that address.
 *	e) Devices with two I2C addresses (eg SFP) have 256 bytes of data
 *		at I2C address 0x50, and 256 bytes of data at I2C address
 *		0x51 (A2h in the spec).  Page selection and paged access
 *		only apply to this second I2C address (0x51).
 *	e) The address space is presented, by the driver, as a linear
 *	        address space.  For devices with one I2C client at address
 *	        0x50 (eg QSFP, CMIS), offset 0-127 are in the lower
 *	        half of address 50/A0h/optoe_client.  Offset 128-255 are in
 *	        page 0, 256-383 are page 1, etc.  More generally, offset
 *	        'n' resides in page (n/128)-1.  ('page -1' is the lower
 *	        half, offset 0-127).
 *	f) For devices with two I2C clients at address 0x50 and 0x51 (eg SFP),
 *		the address space places offset 0-127 in the lower
 *	        half of 50/A0/optoe_client, offset 128-255 in the upper
 *	        half.  Offset 256-383 is in the lower half of 51/A2/dummy.
 *	        Offset 384-511 is in page 0, in the upper half of 51/A2/...
 *	        Offset 512-639 is in page 1, in the upper half of 51/A2/...
 *	        Offset 'n' is in page (n/128)-3 (for n > 383)
 *
 *	                    One I2c addressed (eg QSFP, CMIS) Memory Map
 *
 *	                    2-Wire Serial Address: 1010000x
 *
 *	                    Lower Page 00h (128 bytes)
 *	                    =====================
 *	                   |                     |
 *	                   |                     |
 *	                   |                     |
 *	                   |                     |
 *	                   |                     |
 *	                   |                     |
 *	                   |                     |
 *	                   |                     |
 *	                   |                     |
 *	                   |                     |
 *	                   |Page Select Byte(127)|
 *	                    =====================
 *	                              |
 *	                              |
 *	                              V
 *	     ------------------------------------------------------------
 *	    |                 |                  |                       |
 *	    |                 |                  |                       |
 *	    V                 V                  V                       V
 *	 ------------   --------------      ---------------     --------------
 *	|            | |              |    |               |   |              |
 *	|   Upper    | |     Upper    |    |     Upper     |   |    Upper     |
 *	|  Page 00h  | |    Page 01h  |    |    Page 02h   |   |   Page 03h   |
 *	|            | |   (Optional) |    |   (Optional)  |   |  (Optional   |
 *	|            | |              |    |               |   |   for Cable  |
 *	|            | |              |    |               |   |  Assemblies) |
 *	|    ID      | |     AST      |    |      User     |   |              |
 *	|  Fields    | |    Table     |    |   EEPROM Data |   |              |
 *	|            | |              |    |               |   |              |
 *	|            | |              |    |               |   |              |
 *	|            | |              |    |               |   |              |
 *	 ------------   --------------      ---------------     --------------
 *
 * The SFF 8636 (QSFP) spec only defines the 4 pages described above.
 * In anticipation of future applications and devices, this driver
 * supports access to the full architected range, 256 pages.
 *
 * The CMIS (Common Management Interface Specification) defines use of
 * considerably more pages (at least to page 0xAF), which this driver
 * supports.
 *
 * NOTE: This version of the driver ONLY SUPPORTS BANK 0 PAGES on CMIS
 * devices.
 *
 **/

/* #define DEBUG 1 */
#ifndef LATEST_KERNEL
/*
 * Note to my dev/test partners... LATEST_KERNEL conditionally
 * compiles code that only works on very new Linux kernels.
 * I am trying to submit this to the upstream kernel at 5.11.
 * There are NVMEM dependencies (NVMEM_DEVID_NONE) which require
 * header files from 5.9 or later kernel.  So, when submitting
 * upstream, I will unifdef 'LATEST_KERNEL', keeping the latest
 * code.  For testing on any earlier version of Linux, you
 * need the other side of that #ifdef.  That is actually what
 * I have been testing, so it should work.  You don't have
 * to modify this code at all.  LATEST_KERNEL is not defined,
 * so you will get the code that runs on older kernels.
 */
#define DEBUG 1
#endif

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/nvmem-provider.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/types.h>

/* The maximum length of a port name */
#define MAX_PORT_NAME_LEN 20

/* fundamental unit of addressing for EEPROM */
#define OPTOE_PAGE_SIZE 128
/*
 * Single address devices (eg QSFP, CMIS) have 256 pages, plus the unpaged
 * low 128 bytes.  If the device does not support paging, it is
 * only 2 'pages' long.
 */
#define OPTOE_ARCH_PAGES 256
#define ONE_ADDR_EEPROM_SIZE ((1 + OPTOE_ARCH_PAGES) * OPTOE_PAGE_SIZE)
#define ONE_ADDR_EEPROM_UNPAGED_SIZE (2 * OPTOE_PAGE_SIZE)
/*
 * Dual address devices (eg SFP) have 256 pages, plus the unpaged
 * low 128 bytes, plus 256 bytes at 0x50.  If the device does not
 * support paging, it is 4 'pages' long.
 */
#define TWO_ADDR_EEPROM_SIZE ((3 + OPTOE_ARCH_PAGES) * OPTOE_PAGE_SIZE)
#define TWO_ADDR_EEPROM_UNPAGED_SIZE (4 * OPTOE_PAGE_SIZE)
#define TWO_ADDR_NO_0X51_SIZE (2 * OPTOE_PAGE_SIZE)

/* a few constants to find our way around the EEPROM */
#define OPTOE_PAGE_SELECT_REG   0x7F
#define ONE_ADDR_PAGEABLE_REG 0x02
#define QSFP_NOT_PAGEABLE (1<<2)
#define CMIS_NOT_PAGEABLE (1<<7)
#define TWO_ADDR_PAGEABLE_REG 0x40
#define TWO_ADDR_PAGEABLE (1<<4)
#define TWO_ADDR_0X51_REG 92
#define TWO_ADDR_0X51_SUPP (1<<6)
#define OPTOE_READ_OP 0
#define OPTOE_WRITE_OP 1
#define OPTOE_EOF 0  /* used for access beyond end of device */

/* define optoe_client structure to tie the i2c client to its regmap */
struct optoe_client {
	struct i2c_client *client;
	struct regmap *regmap;
};

struct optoe_data {
	char port_name[MAX_PORT_NAME_LEN];
	u32 byte_len;		/* architected size of EEPROM */

	/*
	 * Lock protects against activities from other Linux tasks,
	 * but not from changes by other I2C masters.
	 */
	struct mutex lock;
	struct attribute_group attr_group;
	struct nvmem_device *nvmem;

	unsigned int write_max;

	/* dev_class: ONE_ADDR (QSFP), TWO_ADDR (SFP), or CMIS */
	int dev_class;

	/* client at 0x50 */
	struct optoe_client optoe_client;
	/* dummy at 0x51 for SFP devices */
	struct optoe_client optoe_dummy;
};

/*
 * specs often allow 5 msec for a page write, sometimes 20 msec;
 * it's important to recover from write timeouts.
 */
static unsigned int write_timeout = 25;

/*
 * flags to distinguish one-address (QSFP family) from two-address (SFP family)
 * and one-address Common Management Interface Specification (CMIS family)
 */
#define ONE_ADDR 1
#define TWO_ADDR 2
#define CMIS_ADDR 3

static const struct i2c_device_id optoe_ids[] = {
	{ "optoe1", ONE_ADDR },
	{ "optoe2", TWO_ADDR },
	{ "optoe3", CMIS_ADDR },
	{ /* END OF LIST */ }
};
MODULE_DEVICE_TABLE(i2c, optoe_ids);

/*-------------------------------------------------------------------------*/
/*
 * optoe_translate_offset() computes the addressing information to be used for
 * a given r/w request.
 *
 * Task is to calculate the client (optoe_client for addr 50,
 * optoe_dummy for addr 51) the page, and the offset.
 *
 * Handles both single address (QSFP), two address (SFP) and CMIS devices.
 *     For SFP, offset 0-255 are on optoe_client, >255 is on optoe_dummy
 *     Offset 256-383 are on the lower half of optoe_dummy
 *     Pages are accessible on the upper half of optoe_dummy
 *     Offset >383 are in 128 byte pages mapped into the upper half
 *
 *     For QSFP and CMIS, all offsets are on optoe_client
 *     offset 0-127 are on the lower half of optoe_client (no paging)
 *     Pages are accessible on the upper half of optoe_client.
 *     Offset >127 are in 128 byte pages mapped into the upper half
 *
 *     Callers must not read/write beyond the end of a client or a page
 *     without recomputing the client/page.  Hence offset (within page)
 *     plus length must be less than or equal to 128.  (Note that this
 *     routine does not have access to the length of the call, hence
 *     cannot do the validity check.)
 *
 * Offset within Lower Page 00h and Upper Page 00h are not recomputed
 */

static uint8_t optoe_translate_offset(struct optoe_data *optoe,
		loff_t *offset, struct optoe_client **optoe_client)
{
	unsigned int page = 0;

	*optoe_client = &optoe->optoe_client;

	/* if SFP style, offset > 255, shift to i2c addr 0x51 */
	if (optoe->dev_class == TWO_ADDR) {
		if (*offset > 255) {
			/* like QSFP, but shifted dummy client */
			*optoe_client = &optoe->optoe_dummy;
			*offset -= 256;
		}
	}

	/*
	 * if offset is in the range 0-128...
	 * page doesn't matter (using lower half), return 0.
	 * offset is already correct (don't add 128 to get to paged area)
	 */
	if (*offset < OPTOE_PAGE_SIZE)
		return page;

	/* note, page will always be positive since *offset >= 128 */
	page = (*offset >> 7)-1;
	/*
	 * OPTOE_PAGE_SIZE puts offset in the top half (the paged area),
	 * offset within the top half is last 7 bits
	 */
	*offset = OPTOE_PAGE_SIZE + (*offset & 0x7f);

	return page;  /* note also returning client and offset */
}

static int optoe_regmap_rw(struct optoe_data *optoe,
				struct optoe_client *optoe_client,
				char *buf,
				unsigned int offset,
				size_t count, int opcode)
{
	unsigned long timeout, access_time;
	struct i2c_client *client = optoe_client->client;
	struct regmap *regmap = optoe_client->regmap;
	int ret;

	/*
	 * Accesses fail if the previous write didn't complete yet. We may
	 * loop a few times until this one succeeds, waiting at least
	 * long enough for one entire page write to work.
	 */
	timeout = jiffies + msecs_to_jiffies(write_timeout);
	do {
		access_time = jiffies;

		if (opcode == OPTOE_READ_OP) {
#ifndef LATEST_KERNEL
			/*
			 * bug in regmap SMBUS code fails on reads
			 * longer than OR EQUAL TO SMBUS_BLOCK_MAX
			 * fixed in 4.17 kernel
			 */
			if (count >= I2C_SMBUS_BLOCK_MAX)
				count = I2C_SMBUS_BLOCK_MAX - 1;
#endif
			ret = regmap_bulk_read(regmap, offset, buf, count);
		} else {
			/* write_max is always 1 in this driver */
			if (count > optoe->write_max)
				count = optoe->write_max;
			ret = regmap_bulk_write(regmap, offset, buf, count);
		}
		dev_dbg(&client->dev, "regmap %s %zu@%d --> %d (%lu)\n",
			(opcode == OPTOE_READ_OP) ? "read" : "write",
			count, offset, ret, jiffies);
		if (!ret)   /* regmap_bulk_calls returns 0 on success */
			return count;

		usleep_range(1000, 2000);
	} while (time_before(access_time, timeout));

	return -ETIMEDOUT;
}

static int optoe_eeprom_update_client(struct optoe_data *optoe,
				char *buf, loff_t off,
				size_t count, int opcode)
{
	struct optoe_client *optoe_client;
	uint8_t page = 0;
	loff_t phy_offset = off;
	int retval = 0;
	int ret = 0;
	int status;
	struct device *dev = &optoe->optoe_client.client->dev;

	/* translate offset into page, 'offset within page' */
	page = optoe_translate_offset(optoe, &phy_offset, &optoe_client);
	dev_dbg(dev,
		"%s off %lld  page:%d phy_offset:%lld, count:%ld, opcode:%d\n",
		__func__, off, page, phy_offset, (long) count, opcode);
	/* set the page register */
	if (page > 0) {
		ret = optoe_regmap_rw(optoe, optoe_client, &page,
			OPTOE_PAGE_SELECT_REG, 1, OPTOE_WRITE_OP);
		if (ret < 0) {
			dev_dbg(dev,
				"Page register write, page %d failed:%d!\n",
				page, ret);
			return ret;
		}
	}

	/* read/write the data */
	while (count) {

		status =  optoe_regmap_rw(optoe, optoe_client,
			buf, phy_offset, count, opcode);

		if (status <= 0) {
			if (retval == 0)
				retval = status;
			break;
		}
		buf += status;
		phy_offset += status;
		count -= status;
		retval += status;
	}


	/*
	 * return the page register to page 0 - why?
	 * We either have to set the page register to 0 on every access
	 * to it, or restore it to 0 whenever we change it.  Otherwise,
	 * accesses to page 0 would actually go to whatever the last page
	 * was.  Assume more accesses to page 0 than all other pages
	 * combined, so less total accesses if we always leave it at page 0
	 */
	if (page > 0) {
		page = 0;
		ret = optoe_regmap_rw(optoe, optoe_client, &page,
			OPTOE_PAGE_SELECT_REG, 1, OPTOE_WRITE_OP);
		if (ret < 0) {
			dev_err(dev,
				"Restore page register to 0 failed:%d!\n", ret);
			/* error only if nothing has been transferred */
			if (retval == 0)
				retval = ret;
		}
	}
	return retval;
}

/*
 * Figure out if this access is within the range of supported pages.
 * Note this is called on every access because we don't know if the
 * module has been replaced since the last call.
 *
 * Returns updated len for this access:
 *     - entire access is legal, original len is returned.
 *     - access begins legal but is too long, len is truncated to fit.
 *     - initial offset exceeds supported pages, return OPTOE_EOF (zero)
 */
static int optoe_page_legal(struct optoe_data *optoe,
		loff_t off, size_t len)
{
	struct optoe_client *optoe_client = &optoe->optoe_client;
	u8 regval;
	int not_pageable;
	int status;
	size_t maxlen;
	struct device *dev = &optoe_client->client->dev;

	if (off < 0)
		return -EINVAL;
	if (optoe->dev_class == TWO_ADDR) {
		/* SFP case */
		/* if only using addr 0x50 (first 256 bytes) we're good */
		if ((off + len) <= TWO_ADDR_NO_0X51_SIZE)
			return len;
		/* if offset exceeds possible pages, we're not good */
		if (off >= TWO_ADDR_EEPROM_SIZE)
			return OPTOE_EOF;
		/* in between, are pages supported? */
		status = optoe_regmap_rw(optoe, optoe_client, &regval,
				TWO_ADDR_PAGEABLE_REG, 1, OPTOE_READ_OP);
		if (status < 0)
			return status;  /* error out (no module?) */
		if (regval & TWO_ADDR_PAGEABLE) {
			/* Pages supported, trim len to the end of pages */
			maxlen = TWO_ADDR_EEPROM_SIZE - off;
		} else {
			/* pages not supported, trim len to unpaged size */
			if (off >= TWO_ADDR_EEPROM_UNPAGED_SIZE)
				return OPTOE_EOF;

			/* will be accessing addr 0x51, is that supported? */
			/* byte 92, bit 6 implies DDM support, 0x51 support */
			status = optoe_regmap_rw(optoe, optoe_client, &regval,
						TWO_ADDR_0X51_REG, 1,
						OPTOE_READ_OP);
			if (status < 0)
				return status;
			if (regval & TWO_ADDR_0X51_SUPP) {
				/* addr 0x51 is OK */
				maxlen = TWO_ADDR_EEPROM_UNPAGED_SIZE - off;
			} else {
				/* addr 0x51 NOT supported, trim to 256 max */
				if (off >= TWO_ADDR_NO_0X51_SIZE)
					return OPTOE_EOF;
				maxlen = TWO_ADDR_NO_0X51_SIZE - off;
			}
		}
		len = (len > maxlen) ? maxlen : len;
	} else {
		/* QSFP case, CMIS case */
		/* if no pages needed, we're good */
		if ((off + len) <= ONE_ADDR_EEPROM_UNPAGED_SIZE)
			return len;
		/* if offset exceeds possible pages, we're not good */
		if (off >= ONE_ADDR_EEPROM_SIZE)
			return OPTOE_EOF;
		/* in between, are pages supported? */
		status = optoe_regmap_rw(optoe, optoe_client, &regval,
				ONE_ADDR_PAGEABLE_REG, 1, OPTOE_READ_OP);
		if (status < 0)
			return status;  /* error out (no module?) */

		/*
		 * note CMIS put the pageable bit in the same register
		 * as QSFP, but at a different bit :-(
		 */
		if (optoe->dev_class == ONE_ADDR)
			not_pageable = QSFP_NOT_PAGEABLE;
		else
			not_pageable = CMIS_NOT_PAGEABLE;
		if (regval & not_pageable) {
			/* pages not supported, trim len to unpaged size */
			if (off >= ONE_ADDR_EEPROM_UNPAGED_SIZE)
				return OPTOE_EOF;
			maxlen = ONE_ADDR_EEPROM_UNPAGED_SIZE - off;
		} else {
			/* Pages supported, trim len to the end of pages */
			maxlen = ONE_ADDR_EEPROM_SIZE - off;
		}
		len = (len > maxlen) ? maxlen : len;
	}
	dev_dbg(dev, "page_legal, class %d, off %lld len %ld\n",
		optoe->dev_class, off, (long) len);
	return len;
}

static int optoe_read_write(struct optoe_data *optoe,
		char *buf, loff_t off, size_t len, int opcode)
{
	struct i2c_client *client = optoe->optoe_client.client;
	int chunk;
	int status = 0;
	int retval;
	size_t pending_len = 0, chunk_len = 0;
	loff_t chunk_offset = 0, chunk_start_offset = 0;
	loff_t chunk_end_offset = 0;

	dev_dbg(&client->dev, "%s: off %lld  len:%ld, opcode:%s\n",
		__func__, off, (long) len,
		(opcode == OPTOE_READ_OP) ? "r" : "w");
	if (unlikely(!len))
		return len;

	/*
	 * Read data from chip, protecting against concurrent updates
	 * from this host, but not from other I2C masters.
	 */
	mutex_lock(&optoe->lock);

	/*
	 * Confirm this access fits within the device suppored addr range
	 */
	status = optoe_page_legal(optoe, off, len);

	/*
	 * returning 0 (OPTOE_EOF) on a write call gets into an infinite
	 * loop with the regmap/i2c code.  Returning an error on a read call
	 * will show up as an error with 'cat <eeprom file>'.  So, writing
	 * past EOF is an error, reading past EOF is just '0'
	 */
	if ((status == OPTOE_EOF) && (opcode == OPTOE_WRITE_OP))
		status = -EINVAL;

	if ((status == OPTOE_EOF) || (status < 0)) {
		mutex_unlock(&optoe->lock);
		return status;
	}
	len = status;

	/*
	 * For each (128 byte) chunk involved in this request, issue a
	 * separate call to optoe_eeprom_update_client(), to
	 * ensure that each access recalculates the client/page
	 * and writes the page register as needed.
	 * Note that chunk to page mapping is confusing, is different for
	 * QSFP/CMIS and SFP, and never needs to be done.  Don't try!
	 */
	pending_len = len; /* amount remaining to transfer */
	retval = 0;  /* amount transferred */
	for (chunk = off >> 7; chunk <= (off + len - 1) >> 7; chunk++) {

		/*
		 * Compute the offset and number of bytes to be read/written
		 *
		 * 1. start at an offset not equal to 0 (within the chunk)
		 *    and read/write less than the rest of the chunk
		 * 2. start at an offset not equal to 0 and read/write the rest
		 *    of the chunk
		 * 3. start at offset 0 (within the chunk) and read/write less
		 *    than entire chunk
		 * 4. start at offset 0 (within the chunk), and read/write
		 *    the entire chunk
		 */
		chunk_start_offset = chunk * OPTOE_PAGE_SIZE;
		chunk_end_offset = chunk_start_offset + OPTOE_PAGE_SIZE;

		if (chunk_start_offset < off) {
			chunk_offset = off;
			if ((off + pending_len) < chunk_end_offset)
				chunk_len = pending_len;
			else
				chunk_len = chunk_end_offset - off;
		} else {
			chunk_offset = chunk_start_offset;
			if (pending_len < OPTOE_PAGE_SIZE)
				chunk_len = pending_len;
			else
				chunk_len = OPTOE_PAGE_SIZE;
		}

		/*
		 * note: chunk_offset is from the start of the EEPROM,
		 * not the start of the chunk
		 */
		status = optoe_eeprom_update_client(optoe, buf,
				chunk_offset, chunk_len, opcode);
		if (status != chunk_len) {
			/* This is another 'no device present' path */
			dev_dbg(&client->dev,
			"o_u_c: chunk %d c_offset %lld c_len %ld failed %d!\n",
			chunk, chunk_offset, (long) chunk_len, status);
			if (status > 0)
				retval += status;
			if (retval == 0)
				retval = status;
			break;
		}
		buf += status;
		pending_len -= status;
		retval += status;
	}
	mutex_unlock(&optoe->lock);

	return retval;
}

static int optoe_nvmem_read(void *priv, unsigned int off,
			    void *buf, size_t count)
{
	struct optoe_data *optoe = priv;

	return optoe_read_write(optoe, buf, off, count, OPTOE_READ_OP);
}

static int optoe_nvmem_write(void *priv, unsigned int off,
			    void *buf, size_t count)
{
	struct optoe_data *optoe = priv;

	return optoe_read_write(optoe, buf, off, count, OPTOE_WRITE_OP);
}

static int optoe_remove(struct i2c_client *client)
{
	struct optoe_data *optoe;

	optoe = i2c_get_clientdata(client);
	sysfs_remove_group(&client->dev.kobj, &optoe->attr_group);
#ifndef LATEST_KERNEL
	nvmem_unregister(optoe->nvmem);
	/*
	 * note, optoe did not register 'client', so don't unregister it
	 * optoe DID register the dummy client, so unregister it
	 */
	if (optoe->optoe_dummy.client)
		i2c_unregister_device(optoe->optoe_dummy.client);
#endif
	kfree(optoe);
	return 0;
}

#ifndef LATEST_KERNEL
static void null_regmap_lock(void *foo)
{
}
#endif

/*
 * optoe_make_regmap creates the regmap for the client.
 * IMPORTANT: Don't call the regmap read/write calls directly
 * for these devices.  These devices are paged, and you have to
 * set the page register before accessing the data in that page.
 * Use the nvmem interfaces, those read/write calls use this
 * driver to manage pages correctly.
 */
static struct regmap *optoe_make_regmap(struct i2c_client *client)
{
	struct regmap_config regmap_config = { };
	struct regmap *regmap;

	/* setup a minimal regmap - 8 bits, 8 bit addresses */
	regmap_config.val_bits = 8;
	regmap_config.reg_bits = 8;

	/* I'll handle the locking */
#ifdef LATEST_KERNEL
	regmap_config.disable_locking = true;
#else
	regmap_config.lock = null_regmap_lock;
	regmap_config.unlock = null_regmap_lock;
	regmap_config.lock_arg = client;
#endif
	regmap = devm_regmap_init_i2c(client, &regmap_config);
	return regmap;
}

/*
 * optoe_make_nvmem() unregisters the existing optoe->nvmem if it
 * exists, then registers a new one.  Convenient when the size
 * of an EEPROM device changes.
 */
static int optoe_make_nvmem(struct optoe_data *optoe)
{
	struct nvmem_config nvmem_config = { };
	struct i2c_client *client = optoe->optoe_client.client;
	struct device *dev = &client->dev;

	nvmem_config.name = optoe->port_name;
#ifdef LATEST_KERNEL
	/* NVMEM_DEVID_NONE tells nvmem not to append '0' to name */
	nvmem_config.id = NVMEM_DEVID_NONE;
#endif
	nvmem_config.dev = dev;
	nvmem_config.read_only = false;
	nvmem_config.root_only = false;
	nvmem_config.owner = THIS_MODULE;
	nvmem_config.compat = true;
	nvmem_config.base_dev = dev;
	nvmem_config.reg_read = optoe_nvmem_read;
	nvmem_config.reg_write = optoe_nvmem_write;
	nvmem_config.priv = optoe;
	nvmem_config.stride = 1;
	nvmem_config.word_size = 1;
	nvmem_config.size = optoe->byte_len;
#ifdef LATEST_KERNEL
	if (optoe->nvmem)
		devm_nvmem_unregister(dev, optoe->nvmem);
	optoe->nvmem = devm_nvmem_register(dev, &nvmem_config);
#else
	if (optoe->nvmem)
		nvmem_unregister(optoe->nvmem);
	optoe->nvmem = nvmem_register(&nvmem_config);
#endif
	dev_info(dev, "%u byte class %d EEPROM\n",
		optoe->byte_len, optoe->dev_class);
	return 0;
}

static ssize_t dev_class_show(struct device *dev,
			struct device_attribute *dattr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct optoe_data *optoe = i2c_get_clientdata(client);
	ssize_t count;

	mutex_lock(&optoe->lock);
	count = sprintf(buf, "%d\n", optoe->dev_class);
	mutex_unlock(&optoe->lock);

	return count;
}

static ssize_t dev_class_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct optoe_data *optoe = i2c_get_clientdata(client);
	int dev_class;
	struct regmap *regmap;
	ssize_t err;

	/*
	 * dev_class is actually the number of i2c addresses used, thus
	 * legal values are "1" (QSFP class) and "2" (SFP class)
	 * And...  CMIS spec is 1 i2c address, but puts the pageable
	 * bit in a different location, so CMIS devices are "3"
	 */

	if (kstrtoint(buf, 0, &dev_class) != 0 ||
		dev_class < 1 || dev_class > 3)
		return -EINVAL;

	if (optoe->dev_class == dev_class)  /* no change, NOP */
		return(count);

	mutex_lock(&optoe->lock);
	if (dev_class == TWO_ADDR) {
		/* SFP family */
		/* if it doesn't exist, create 0x51 i2c address */
		if (!optoe->optoe_dummy.client) {
#ifdef LATEST_KERNEL
			optoe->optoe_dummy.client =
				devm_i2c_new_dummy_device(dev,
							  client->adapter,
							  0x51);
#else
			optoe->optoe_dummy.client =
				i2c_new_dummy(client->adapter, 0x51);
#endif
			if (!optoe->optoe_dummy.client) {
				dev_err(&client->dev,
					"address 0x51 unavailable\n");
				mutex_unlock(&optoe->lock);
				return -EADDRINUSE;
			}
			regmap = optoe_make_regmap(
					optoe->optoe_dummy.client);
			if (IS_ERR(regmap)) {
				mutex_unlock(&optoe->lock);
				return PTR_ERR(regmap);
			}
			optoe->optoe_dummy.regmap = regmap;
		}
		optoe->byte_len = TWO_ADDR_EEPROM_SIZE;
	} else {
		/* one-address (eg QSFP) and CMIS family */
		/* note, no need to delete the dummy i2c device */
		optoe->byte_len = ONE_ADDR_EEPROM_SIZE;
	}
	optoe->dev_class = dev_class;
	optoe_make_nvmem(optoe);  /* updates the reported size of EEPROM */
	err = (IS_ERR(optoe->nvmem)) ? PTR_ERR(optoe->nvmem) : 0;
	mutex_unlock(&optoe->lock);

	return err;
}

static ssize_t port_name_show(struct device *dev,
			struct device_attribute *dattr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct optoe_data *optoe = i2c_get_clientdata(client);
	ssize_t count;

	mutex_lock(&optoe->lock);
	count = sprintf(buf, "%s\n", optoe->port_name);
	mutex_unlock(&optoe->lock);

	return count;
}

static ssize_t port_name_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct optoe_data *optoe = i2c_get_clientdata(client);
	char port_name[MAX_PORT_NAME_LEN];

	/* no checking, this value is not used except by port_name_show */

	if (sscanf(buf, "%19s", port_name) != 1)
		return -EINVAL;

	mutex_lock(&optoe->lock);
	strcpy(optoe->port_name, port_name);
	optoe_make_nvmem(optoe);  /* updates the name of the nvmem */
	mutex_unlock(&optoe->lock);

	return count;
}

static DEVICE_ATTR_RW(port_name);

static DEVICE_ATTR_RW(dev_class);


static struct attribute *optoe_attrs[] = {
	&dev_attr_port_name.attr,
	&dev_attr_dev_class.attr,
	NULL,
};

static struct attribute_group optoe_attr_group = {
	.attrs = optoe_attrs,
};

#ifdef LATEST_KERNEL
static int optoe_probe(struct i2c_client *client)
#else
static int optoe_probe(struct i2c_client *client,
		       const struct i2c_device_id *id)
#endif
{
	struct regmap *regmap;
	int err;
	struct optoe_data *optoe;
	const char *of_port_name = NULL;
	struct device *dev = &client->dev;

	if (client->addr != 0x50) {
		dev_dbg(dev, "probe, bad i2c addr: 0x%x\n", client->addr);
		err = -EINVAL;
		goto exit;
	}

	optoe = kzalloc(sizeof(struct optoe_data), GFP_KERNEL);
	if (!optoe) {
		err = -ENOMEM;
		goto exit;
	}
	mutex_init(&optoe->lock);

	/*
	 * if the port_name property is defined, use it, else use the
	 * device name as port_name
	 */
	if (device_property_present(dev, "port_name")) {
		err = device_property_read_string(dev, "port_name",
						  &of_port_name);
		if (err)
			of_port_name = dev_name(dev);
	} else {
		of_port_name = dev_name(dev);
	}
	memcpy(optoe->port_name, of_port_name, MAX_PORT_NAME_LEN);

	/* determine whether this is a one-address or two-address module */
	if (strcmp(client->name, "optoe1") == 0) {
		/* QSFP family */
		optoe->dev_class = ONE_ADDR;
		optoe->byte_len = ONE_ADDR_EEPROM_SIZE;
	} else if (strcmp(client->name, "optoe2") == 0) {
		/* SFP family */
		optoe->dev_class = TWO_ADDR;
		optoe->byte_len = TWO_ADDR_EEPROM_SIZE;
	} else if (strcmp(client->name, "optoe3") == 0) {
		/* CMIS spec */
		optoe->dev_class = CMIS_ADDR;
		optoe->byte_len = ONE_ADDR_EEPROM_SIZE;
	} else {     /* those were the only choices */
		err = -EINVAL;
		goto err_struct;
	}

	/*
	 * Old application notes recommend 1 byte writes for some
	 * modules.  This could probably be lifted, but lacking
	 * a broad base of devices and systems to test, I'm leaving
	 * this as is.  If this is ever raised, the max would automatically
	 * be 128 bytes as anything larger would cross page
	 * boundaries with wraparound effects.
	 * TODO: Consider making this a device tree property
	 */

	optoe->write_max = 1;

	regmap = optoe_make_regmap(client);
	if (IS_ERR(regmap)) {
		err = PTR_ERR(regmap);
		goto err_struct;
	}

	optoe->optoe_client.client = client;
	optoe->optoe_client.regmap = regmap;

	/* SFF-8472 spec requires that the second I2C address be 0x51 */
	if (optoe->dev_class == TWO_ADDR) {
#ifdef LATEST_KERNEL
		optoe->optoe_dummy.client =
			devm_i2c_new_dummy_device(dev, client->adapter, 0x51);
#else
		optoe->optoe_dummy.client =
			i2c_new_dummy(client->adapter, 0x51);
#endif
		if (!optoe->optoe_dummy.client) {
			dev_err(dev, "address 0x51 unavailable\n");
			err = -EADDRINUSE;
			goto err_struct;
		}
		regmap = optoe_make_regmap(optoe->optoe_dummy.client);
		if (IS_ERR(regmap)) {
			err = PTR_ERR(regmap);
			goto err_struct;
		}
		optoe->optoe_dummy.regmap = regmap;
	}

	optoe->attr_group = optoe_attr_group;

	err = sysfs_create_group(&client->dev.kobj, &optoe->attr_group);
	if (err) {
		dev_err(dev, "failed to create sysfs attribute group.\n");
		goto err_struct;
	}

	i2c_set_clientdata(client, optoe);
	optoe_make_nvmem(optoe);
	if (IS_ERR(optoe->nvmem)) {
		err = PTR_ERR(optoe->nvmem);
		goto err_struct;
	}

	return 0;

err_struct:
#ifndef LATEST_KERNEL
	if (optoe->optoe_dummy.client)
		i2c_unregister_device(optoe->optoe_dummy.client);
#endif
	kfree(optoe);
exit:
	dev_dbg(dev, "probe error %d\n", err);

	return err;
}

/*-------------------------------------------------------------------------*/

static struct i2c_driver optoe_driver = {
	.driver = {
		.name = "optoe",
		.owner = THIS_MODULE,
	},
#ifdef LATEST_KERNEL
	.probe_new = optoe_probe,
#else
	.probe = optoe_probe,
#endif
	.remove = optoe_remove,
	.id_table = optoe_ids,
};

static int __init optoe_init(void)
{
	return i2c_add_driver(&optoe_driver);
}
module_init(optoe_init);

static void __exit optoe_exit(void)
{
	i2c_del_driver(&optoe_driver);
}
module_exit(optoe_exit);

MODULE_DESCRIPTION("Driver for optical transceiver (SFP/QSFP/CMIS) EEPROMs");
MODULE_AUTHOR("DON BOLLINGER <don@thebollingers.org>");
MODULE_LICENSE("GPL");
