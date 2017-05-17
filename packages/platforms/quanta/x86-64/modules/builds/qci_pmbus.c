/*
 * QUANTA Generic PMBUS driver
 *
 *
 * Based on generic pmbus driver and ltc2978 driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <../drivers/hwmon/pmbus/pmbus.h>
#include <linux/delay.h>

enum projects { ly8, ix1, ix2, ix1a };

#define DELAY_TIME		1000	/* uS	*/

/* Needed to access the mutex. Copied from pmbus_core.c */
#define PB_STATUS_BASE		0
#define PB_STATUS_VOUT_BASE	(PB_STATUS_BASE + PMBUS_PAGES)
#define PB_STATUS_IOUT_BASE	(PB_STATUS_VOUT_BASE + PMBUS_PAGES)
#define PB_STATUS_FAN_BASE	(PB_STATUS_IOUT_BASE + PMBUS_PAGES)
#define PB_STATUS_FAN34_BASE	(PB_STATUS_FAN_BASE + PMBUS_PAGES)
#define PB_STATUS_TEMP_BASE	(PB_STATUS_FAN34_BASE + PMBUS_PAGES)
#define PB_STATUS_INPUT_BASE	(PB_STATUS_TEMP_BASE + PMBUS_PAGES)
#define PB_STATUS_VMON_BASE	(PB_STATUS_INPUT_BASE + 1)
#define PB_NUM_STATUS_REG	(PB_STATUS_VMON_BASE + 1)
struct pmbus_data {
	struct device *dev;
	struct device *hwmon_dev;

	u32 flags;		/* from platform data */

	int exponent[PMBUS_PAGES];
				/* linear mode: exponent for output voltages */

	const struct pmbus_driver_info *info;

	int max_attributes;
	int num_attributes;
	struct attribute_group group;
	const struct attribute_group *groups[2];

	struct pmbus_sensor *sensors;

	struct mutex update_lock;
	bool valid;
	unsigned long last_updated;	/* in jiffies */

	/*
	 * A single status register covers multiple attributes,
	 * so we keep them all together.
	 */
	u8 status[PB_NUM_STATUS_REG];
	u8 status_register;

	u8 currpage;
};

static ssize_t qci_pmbus_show_mfr_id(struct device *dev,
                struct device_attribute *da, char *buf)
{
    int ret, len;
    u8 block_buffer[I2C_SMBUS_BLOCK_MAX + 1], *str;
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);


    ret = i2c_smbus_read_i2c_block_data(client, PMBUS_MFR_ID, I2C_SMBUS_BLOCK_MAX, block_buffer);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read Manufacturer ID\n");
        return ret;
    }
    len = block_buffer[0];
    block_buffer[(len+1)] = '\0';
    str = &(block_buffer[1]);

    return snprintf(buf, PAGE_SIZE, "%s\n", str);
}

static ssize_t qci_pmbus_show_mfr_model(struct device *dev,
                struct device_attribute *da, char *buf)
{
    int ret, len;
    u8 block_buffer[I2C_SMBUS_BLOCK_MAX + 1], *str;
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);

    ret = i2c_smbus_read_i2c_block_data(client, PMBUS_MFR_MODEL, I2C_SMBUS_BLOCK_MAX, block_buffer);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read Manufacturer Model\n");
        return ret;
    }
    len = block_buffer[0];
    block_buffer[(len+1)] = '\0';
    str = &(block_buffer[1]);

    return snprintf(buf, PAGE_SIZE, "%s\n", str);
}

static ssize_t qci_pmbus_show_mfr_revision(struct device *dev,
                struct device_attribute *da, char *buf)
{
    int ret, len;
    u8 block_buffer[I2C_SMBUS_BLOCK_MAX + 1], *str;
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);

    ret = i2c_smbus_read_i2c_block_data(client, PMBUS_MFR_REVISION, I2C_SMBUS_BLOCK_MAX, block_buffer);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read Manufacturer Revision\n");
        return ret;
    }
    len = block_buffer[0];
    block_buffer[(len+1)] = '\0';
    str = &(block_buffer[1]);

    return snprintf(buf, PAGE_SIZE, "%s\n", str);
}

static ssize_t qci_pmbus_show_mfr_location(struct device *dev,
                struct device_attribute *da, char *buf)
{
    int ret, len;
    u8 block_buffer[I2C_SMBUS_BLOCK_MAX + 1], *str;
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);

    ret = i2c_smbus_read_i2c_block_data(client, PMBUS_MFR_LOCATION, I2C_SMBUS_BLOCK_MAX, block_buffer);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read Manufacture Location\n");
        return ret;
    }
    len = block_buffer[0];
    block_buffer[(len+1)] = '\0';
    str = &(block_buffer[1]);

    return snprintf(buf, PAGE_SIZE, "%s\n", str);
}

static ssize_t qci_pmbus_show_mfr_serial(struct device *dev,
                struct device_attribute *da, char *buf)
{
    int ret, len;
    u8 block_buffer[I2C_SMBUS_BLOCK_MAX + 1], *str;
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);

    ret = i2c_smbus_read_i2c_block_data(client, PMBUS_MFR_SERIAL, I2C_SMBUS_BLOCK_MAX, block_buffer);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read Manufacturer Serial\n");
        return ret;
    }
    len = block_buffer[0];
    block_buffer[(len+1)] = '\0';
    str = &(block_buffer[1]);

    return snprintf(buf, PAGE_SIZE, "%s\n", str);
}


static DEVICE_ATTR(mfr_id, S_IRUGO, qci_pmbus_show_mfr_id, NULL);
static DEVICE_ATTR(mfr_model, S_IRUGO, qci_pmbus_show_mfr_model, NULL);
static DEVICE_ATTR(mfr_revision, S_IRUGO, qci_pmbus_show_mfr_revision, NULL);
static DEVICE_ATTR(mfr_location, S_IRUGO, qci_pmbus_show_mfr_location, NULL);
static DEVICE_ATTR(mfr_serial, S_IRUGO, qci_pmbus_show_mfr_serial, NULL);


static struct attribute *qci_pmbus_inventory_attrs[] = {
    &dev_attr_mfr_id.attr,
    &dev_attr_mfr_model.attr,
    &dev_attr_mfr_revision.attr,
    &dev_attr_mfr_location.attr,
    &dev_attr_mfr_serial.attr,
    NULL
};

static struct attribute_group qci_pmbus_inventory_attr_grp = {
    .attrs = qci_pmbus_inventory_attrs
};

/* FIXME: add project specific id here */
static const struct i2c_device_id qci_pmbus_id[] = {
    {"qci_pmbus_ly8", ly8},
    {"qci_pmbus_ix1", ix1},
    {"qci_pmbus_ix2", ix2},
    {}
};
MODULE_DEVICE_TABLE(i2c, qci_pmbus_id);

/*
 * Find sensor groups and status registers on each page.
 */
static void qci_pmbus_find_sensor_groups(struct i2c_client *client,
				     struct pmbus_driver_info *info)
{
	int page;

	/* Sensors detected on page 0 only */
	if (pmbus_check_word_register(client, 0, PMBUS_READ_VIN))
		info->func[0] |= PMBUS_HAVE_VIN;
	if (pmbus_check_word_register(client, 0, PMBUS_READ_VCAP))
		info->func[0] |= PMBUS_HAVE_VCAP;
	if (pmbus_check_word_register(client, 0, PMBUS_READ_IIN))
		info->func[0] |= PMBUS_HAVE_IIN;
	if (pmbus_check_word_register(client, 0, PMBUS_READ_PIN))
		info->func[0] |= PMBUS_HAVE_PIN;
	if (info->func[0]
	    && pmbus_check_byte_register(client, 0, PMBUS_STATUS_INPUT))
		info->func[0] |= PMBUS_HAVE_STATUS_INPUT;
	if (pmbus_check_byte_register(client, 0, PMBUS_FAN_CONFIG_12) &&
	    pmbus_check_word_register(client, 0, PMBUS_READ_FAN_SPEED_1)) {
		info->func[0] |= PMBUS_HAVE_FAN12;
		if (pmbus_check_byte_register(client, 0, PMBUS_STATUS_FAN_12))
			info->func[0] |= PMBUS_HAVE_STATUS_FAN12;
	}
	if (pmbus_check_byte_register(client, 0, PMBUS_FAN_CONFIG_34) &&
	    pmbus_check_word_register(client, 0, PMBUS_READ_FAN_SPEED_3)) {
		info->func[0] |= PMBUS_HAVE_FAN34;
		if (pmbus_check_byte_register(client, 0, PMBUS_STATUS_FAN_34))
			info->func[0] |= PMBUS_HAVE_STATUS_FAN34;
	}
	if (pmbus_check_word_register(client, 0, PMBUS_READ_TEMPERATURE_1))
		info->func[0] |= PMBUS_HAVE_TEMP;
	if (pmbus_check_word_register(client, 0, PMBUS_READ_TEMPERATURE_2))
		info->func[0] |= PMBUS_HAVE_TEMP2;
	if (pmbus_check_word_register(client, 0, PMBUS_READ_TEMPERATURE_3))
		info->func[0] |= PMBUS_HAVE_TEMP3;
	if (info->func[0] & (PMBUS_HAVE_TEMP | PMBUS_HAVE_TEMP2
			     | PMBUS_HAVE_TEMP3)
	    && pmbus_check_byte_register(client, 0,
					 PMBUS_STATUS_TEMPERATURE))
			info->func[0] |= PMBUS_HAVE_STATUS_TEMP;

	/* Sensors detected on all pages */
	for (page = 0; page < info->pages; page++) {
		if (pmbus_check_word_register(client, page, PMBUS_READ_VOUT)) {
			info->func[page] |= PMBUS_HAVE_VOUT;
			if (pmbus_check_byte_register(client, page,
						      PMBUS_STATUS_VOUT))
				info->func[page] |= PMBUS_HAVE_STATUS_VOUT;
		}
		if (pmbus_check_word_register(client, page, PMBUS_READ_IOUT)) {
			info->func[page] |= PMBUS_HAVE_IOUT;
			if (pmbus_check_byte_register(client, 0,
						      PMBUS_STATUS_IOUT))
				info->func[page] |= PMBUS_HAVE_STATUS_IOUT;
		}
		if (pmbus_check_word_register(client, page, PMBUS_READ_POUT))
			info->func[page] |= PMBUS_HAVE_POUT;
	}
}

/*
 * Identify chip parameters.
 */
static int qci_pmbus_identify(struct i2c_client *client,
			  struct pmbus_driver_info *info)
{
	int ret = 0;

	if (!info->pages) {
		/*
		 * Check if the PAGE command is supported. If it is,
		 * keep setting the page number until it fails or until the
		 * maximum number of pages has been reached. Assume that
		 * this is the number of pages supported by the chip.
		 */
		if (pmbus_check_byte_register(client, 0, PMBUS_PAGE)) {
			int page;

			for (page = 1; page < PMBUS_PAGES; page++) {
				if (pmbus_set_page(client, page) < 0)
					break;
			}
			pmbus_set_page(client, 0);
			info->pages = page;
		} else {
			info->pages = 1;
		}
	}

	if (pmbus_check_byte_register(client, 0, PMBUS_VOUT_MODE)) {
		int vout_mode;

		vout_mode = pmbus_read_byte_data(client, 0, PMBUS_VOUT_MODE);
		if (vout_mode >= 0 && vout_mode != 0xff) {
			switch (vout_mode >> 5) {
			case 0:
				break;
			case 1:
				info->format[PSC_VOLTAGE_OUT] = vid;
				break;
			case 2:
				info->format[PSC_VOLTAGE_OUT] = direct;
				break;
			default:
				ret = -ENODEV;
				goto abort;
			}
		}
	}

	/*
	 * We should check if the COEFFICIENTS register is supported.
	 * If it is, and the chip is configured for direct mode, we can read
	 * the coefficients from the chip, one set per group of sensor
	 * registers.
	 *
	 * To do this, we will need access to a chip which actually supports the
	 * COEFFICIENTS command, since the command is too complex to implement
	 * without testing it. Until then, abort if a chip configured for direct
	 * mode was detected.
	 */
	if (info->format[PSC_VOLTAGE_OUT] == direct) {
		ret = -ENODEV;
		goto abort;
	}

	/* if no function pre-defined, try to find sensor groups  */
	if (info->func[0] == 0) qci_pmbus_find_sensor_groups(client, info);
abort:
	return ret;
}

int qci_pmbus_set_page(struct i2c_client *client, u8 page)
{
	struct pmbus_data *data = i2c_get_clientdata(client);
	int rv = 0;
	int newpage;

	if (page != data->currpage) {
		rv = i2c_smbus_write_byte_data(client, PMBUS_PAGE, page);
		udelay(DELAY_TIME);
		newpage = i2c_smbus_read_byte_data(client, PMBUS_PAGE);
		if (newpage != page)
			rv = -EIO;
		else
			data->currpage = page;
	}
	return rv;
}

int qci_write_byte(struct i2c_client *client, int page, u8 value)
{
	int rv;

	if (page >= 0) {
		rv = qci_pmbus_set_page(client, page);
		if (rv < 0)
			return rv;
	}

	rv = i2c_smbus_write_byte(client, value);
	udelay(DELAY_TIME);
	return rv;
}

int qci_write_word_data(struct i2c_client *client, int page, int reg, u16 word)
{
	int rv;

	rv = qci_pmbus_set_page(client, page);
	if (rv < 0)
		return rv;

	rv = i2c_smbus_write_word_data(client, reg, word);
	udelay(DELAY_TIME);
	return rv;
}

static int qci_pmbus_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct pmbus_driver_info *info;
	int ret, i;

	dev_info(dev, "qci_pmbus_probe\n");

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_READ_WORD_DATA))
		return -ENODEV;

	info = devm_kzalloc(&client->dev, sizeof(struct pmbus_driver_info),
			    GFP_KERNEL);

	if (!info)
		return -ENOMEM;

	info->func[0] = 0;

	/* FIXME: add project specific function table here */
	switch (id->driver_data) {
	case ly8:
		info->pages = 1;
		info->func[0] = PMBUS_HAVE_VIN | PMBUS_HAVE_PIN | PMBUS_HAVE_STATUS_INPUT
		| PMBUS_HAVE_FAN12 | PMBUS_HAVE_STATUS_FAN12
		| PMBUS_HAVE_VOUT  | PMBUS_HAVE_STATUS_VOUT
		| PMBUS_HAVE_IOUT  | PMBUS_HAVE_STATUS_IOUT
		;
		break;
	case ix1:
	case ix2:
		info->pages = 1;
		info->func[0] = PMBUS_HAVE_VIN | PMBUS_HAVE_IIN
		  | PMBUS_HAVE_PIN | PMBUS_HAVE_STATUS_INPUT
		  | PMBUS_HAVE_FAN12 | PMBUS_HAVE_STATUS_FAN12
		  | PMBUS_HAVE_TEMP | PMBUS_HAVE_TEMP2
		  | PMBUS_HAVE_TEMP3 | PMBUS_HAVE_STATUS_TEMP
		  | PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT
		  | PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT
		  | PMBUS_HAVE_POUT
		  ;
		break;
	default:
		break;
	}
	info->write_word_data = qci_write_word_data;
	info->write_byte = qci_write_byte;
	info->identify = qci_pmbus_identify;	/* FIXME: reserve for future use */

	/* Register sysfs hooks */
	ret = sysfs_create_group(&dev->kobj, &qci_pmbus_inventory_attr_grp);
	if (ret) {
		dev_err(dev, "Failed to create sysfs entries\n");
		return -1;
	}

	return pmbus_do_probe(client, id, info);
}

/* This is the driver that will be inserted */
static struct i2c_driver qci_pmbus_driver = {
	.driver = {
		   .name = "qci-pmbus",
		   },
	.probe = qci_pmbus_probe,
	.remove = pmbus_do_remove,
	.id_table = qci_pmbus_id,
};

module_i2c_driver(qci_pmbus_driver);


MODULE_AUTHOR("Jonathan Tsai (jonathan.tsai@quantatw.com)");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("QUANTA generic PMBus driver");
MODULE_LICENSE("GPL");
