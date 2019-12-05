/*
 * Hardware monitoring driver for Texas Instruments TPS53681
 *
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
 */

#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <../drivers/hwmon/pmbus/pmbus.h>

#define TPS53681_PROT_VR12_5MV      0x01 /* VR12.0 mode, 5-mV DAC */
#define TPS53681_PROT_VR12_5_10MV   0x02 /* VR12.5 mode, 10-mV DAC */
#define TPS53681_PROT_VR13_10MV     0x04 /* VR13.0 mode, 10-mV DAC */
#define TPS53681_PROT_IMVP8_5MV     0x05 /* IMVP8 mode, 5-mV DAC */
#define TPS53681_PROT_VR13_5MV      0x07 /* VR13.0 mode, 5-mV DAC */
#define TPS53681_PAGE_NUM           2

/*
 * Index into status register array, per status register group
 */
#define TPS53681_STATUS_BASE        0
#define TPS53681_STATUS_VOUT_BASE   (TPS53681_STATUS_BASE + PMBUS_PAGES)
#define TPS53681_STATUS_IOUT_BASE   (TPS53681_STATUS_VOUT_BASE + PMBUS_PAGES)
#define TPS53681_STATUS_FAN_BASE    (TPS53681_STATUS_IOUT_BASE + PMBUS_PAGES)
#define TPS53681_STATUS_FAN34_BASE  (TPS53681_STATUS_FAN_BASE + PMBUS_PAGES)
#define TPS53681_STATUS_TEMP_BASE   (TPS53681_STATUS_FAN34_BASE + PMBUS_PAGES)
#define TPS53681_STATUS_INPUT_BASE  (TPS53681_STATUS_TEMP_BASE + PMBUS_PAGES)
#define TPS53681_STATUS_VMON_BASE   (TPS53681_STATUS_INPUT_BASE + 1)

#define TPS53681_NUM_STATUS_REG     (TPS53681_STATUS_VMON_BASE + 1)

#define TPS53681_NAME_SIZE          24

/* Acceptable values of VIN_UV_FAULT_LIMIT */
#define TPS53681_VIN_UVF_LIMIT_4P25         4250
#define TPS53681_VIN_UVF_LIMIT_5P5          5500
#define TPS53681_VIN_UVF_LIMIT_6P5          6500
#define TPS53681_VIN_UVF_LIMIT_7P5          7500
#define TPS53681_VIN_UVF_LIMIT_8P5          8500
#define TPS53681_VIN_UVF_LIMIT_9P5          9500
#define TPS53681_VIN_UVF_LIMIT_10P5         10500
#define TPS53681_VIN_UVF_LIMIT_11P5         11500

#define TPS53681_VIN_UV_FAULT_LIMIT_4P25    0xF011
#define TPS53681_VIN_UV_FAULT_LIMIT_5P5     0xF80B
#define TPS53681_VIN_UV_FAULT_LIMIT_6P5     0xF80D
#define TPS53681_VIN_UV_FAULT_LIMIT_7P5     0xF80F
#define TPS53681_VIN_UV_FAULT_LIMIT_8P5     0xF811
#define TPS53681_VIN_UV_FAULT_LIMIT_9P5     0xF813
#define TPS53681_VIN_UV_FAULT_LIMIT_10P5    0xF815
#define TPS53681_VIN_UV_FAULT_LIMIT_11P5    0xF817

struct tps53681_sensor {
	struct tps53681_sensor *next;
	char name[TPS53681_NAME_SIZE];      /* sysfs sensor name */
	struct device_attribute attribute;
	u8 page;                            /* page number */
	u16 reg;                            /* register */
	enum pmbus_sensor_classes class;    /* sensor class */
	bool update;                        /* runtime sensor update needed */
	bool convert;                       /* Whether or not to apply linear/vid/direct */
	int data;                           /* Sensor data. Negative if there was a read error */
};

struct tps53681_data {
	struct device *dev;
	struct device *hwmon_dev;
	u32 flags;                      /* from platform data */
	int exponent[PMBUS_PAGES];      /* linear mode: exponent for output voltages */
	const struct pmbus_driver_info *info;
	int max_attributes;
	int num_attributes;
	struct attribute_group group;
	const struct attribute_group *groups[2];
	struct dentry *debugfs;        /* debugfs device directory */
	struct tps53681_sensor *sensors;
	struct mutex update_lock;
	bool valid;
	unsigned long last_updated;    /* in jiffies */
	/*
	 * A single status register covers multiple attributes,
	 * so we keep them all together.
	 */
	u16 status[TPS53681_NUM_STATUS_REG];

	bool has_status_word;          /* device uses STATUS_WORD register */
	int (*read_status)(struct i2c_client *client, int page);
	u8 currpage;
};

static int tps53681_identify(struct i2c_client *client, struct pmbus_driver_info *info)
{
	u8 vout_params;
	int ret;

	/* Read the register with VOUT scaling value.*/
	ret = pmbus_read_byte_data(client, 0, PMBUS_VOUT_MODE);
	if (ret < 0)
    {
		return ret;
    }

	vout_params = ret & GENMASK(4, 0);

	switch (vout_params)
    {
        case TPS53681_PROT_VR13_10MV:
        case TPS53681_PROT_VR12_5_10MV:
            info->vrm_version = vr13;
            break;
        case TPS53681_PROT_VR13_5MV:
        case TPS53681_PROT_VR12_5MV:
        case TPS53681_PROT_IMVP8_5MV:
            info->vrm_version = vr12;
            break;
        default:
            return -EINVAL;
	}

	return 0;
}

/*
 * Convert linear sensor values to milli- or micro-units
 * depending on sensor type.
 */
static long tps53681_reg2data_linear(struct tps53681_data *data, struct tps53681_sensor *sensor, u16 word)
{
	s16 exponent;
	s32 mantissa;
	long val;

#if 0
    if (!sensor->convert)
    {
        return (long)word;
    }
#endif

    if (sensor->class == PSC_VOLTAGE_OUT) /* LINEAR16 */
    {
        exponent = data->exponent[sensor->page];
        mantissa = (u16)word;
    }
    else /* LINEAR11 */
    {				
        exponent = ((s16)word) >> 11;
        mantissa = ((s16)((word & 0x7ff) << 5)) >> 5;
    }

	val = mantissa;

	/* scale result to milli-units for all sensors except fans */
	if (sensor->class != PSC_FAN)
    {
		val = val * 1000L;
    }

	/* scale result to micro-units for power sensors */
	if (sensor->class == PSC_POWER)
    {
		val = val * 1000L;
    }

	if (exponent >= 0)
    {
		val <<= exponent;
    }
	else
    {
		val >>= -exponent;
    }

	return val;
}

static struct tps53681_sensor *tps53681_find_sensor(struct tps53681_data *data, int page, int reg)
{
    struct tps53681_sensor *sensor;

    for (sensor = data->sensors; sensor; sensor = sensor->next)
    {
        if (sensor->page == page && sensor->reg == reg)
        {
            return sensor;
        }
    }

    return ERR_PTR(-EINVAL);
}

int tps53681_write_word_data(struct i2c_client *client, int page, int reg, u16 word)
{
    long val;
	int rv;
    struct tps53681_data *data = i2c_get_clientdata(client);
    struct tps53681_sensor *sensor = tps53681_find_sensor(data, page, reg);

    /* Transfer from register to data in linear format */
    val = tps53681_reg2data_linear(data, sensor, word);

    /* Transfer data to the format that TPS53681 accepts */
    if (reg == PMBUS_VIN_UV_FAULT_LIMIT)
    {
        /* Acceptable values of VIN_UV_FAULT_LIMIT */
	    switch (val)
        {
            case TPS53681_VIN_UVF_LIMIT_4P25:
                val = TPS53681_VIN_UV_FAULT_LIMIT_4P25;
                break;
            case TPS53681_VIN_UVF_LIMIT_5P5:
                val = TPS53681_VIN_UV_FAULT_LIMIT_5P5;
                break;
            case TPS53681_VIN_UVF_LIMIT_6P5:
                val = TPS53681_VIN_UV_FAULT_LIMIT_6P5;
                break;
            case TPS53681_VIN_UVF_LIMIT_7P5:
                val = TPS53681_VIN_UV_FAULT_LIMIT_7P5;
                break;
            case TPS53681_VIN_UVF_LIMIT_8P5:
                val = TPS53681_VIN_UV_FAULT_LIMIT_8P5;
                break;
            case TPS53681_VIN_UVF_LIMIT_9P5:
                val = TPS53681_VIN_UV_FAULT_LIMIT_9P5;
                break;
            case TPS53681_VIN_UVF_LIMIT_10P5:
                val = TPS53681_VIN_UV_FAULT_LIMIT_10P5;
                break;
            case TPS53681_VIN_UVF_LIMIT_11P5:
                val = TPS53681_VIN_UV_FAULT_LIMIT_11P5;
                break;
            default:
                return -EINVAL;
	    }
    }
    else if ((reg == PMBUS_VOUT_OV_FAULT_LIMIT) || (reg == PMBUS_VOUT_UV_FAULT_LIMIT))
    {
        /* VOUT_OV_FAULT_LIMIT and VOUT_UV_FAULT_LIMIT are read only */
        return -ENXIO;
    }
    else
    {
        if (reg == PMBUS_VIN_OV_FAULT_LIMIT)
        {
            /* Valid values of the mantissa of VIN_OV_FAULT_LIMIT range from 0d to 31d */
            if ((val < 0) || (val > 31000))
            {
                return -ENXIO;
            }
        }
        else
        {
            /* Valid values of an 11-bit two's complement mantissa
               with a 5-bit two's complement exponent which is 0 */
            if ((val < -1022000) || (val > 1022000))
            {
                return -ENXIO;
            }
        }

        val = val / 1000L;
        if (val < 0)
        {
            /* Convert to an 11-bit two's complement mantissa
               with a 5-bit two's complement exponent which is 0 */
            u16 tmpWord = 0xfc00;

            tmpWord = tmpWord | (-val - 1);
			tmpWord = ~tmpWord;
			tmpWord = tmpWord | (1 << 10);
			val = (long)tmpWord;
        }
    }

	rv = pmbus_set_page(client, page);
    if (rv < 0)
    {
        return rv;
    }

    return i2c_smbus_write_word_data(client, reg, (u16)val);
}

static struct pmbus_driver_info tps53681_info = {
	.pages = TPS53681_PAGE_NUM,
	.format[PSC_VOLTAGE_IN] = linear,
	.format[PSC_VOLTAGE_OUT] = vid,
	.format[PSC_TEMPERATURE] = linear,
	.format[PSC_CURRENT_OUT] = linear,
	.format[PSC_POWER] = linear,
	.func[0] = PMBUS_HAVE_VIN | PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT |
		PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT |
		PMBUS_HAVE_TEMP | PMBUS_HAVE_STATUS_TEMP |
		PMBUS_HAVE_POUT,
	.func[1] = PMBUS_HAVE_VIN | PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT |
		PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT |
		PMBUS_HAVE_TEMP | PMBUS_HAVE_STATUS_TEMP |
		PMBUS_HAVE_POUT,
	.identify = tps53681_identify,
	.write_word_data = tps53681_write_word_data,
};

static int tps53681_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	return pmbus_do_probe(client, id, &tps53681_info);
}

static const struct i2c_device_id tps53681_id[] = {
	{"tps53681", 0},
	{"tps53622", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, tps53681_id);

static const struct of_device_id tps53681_of_match[] = {
	{.compatible = "ti,tps53681"},
	{.compatible = "ti,tps53622"},
	{}
};
MODULE_DEVICE_TABLE(of, tps53681_of_match);

static struct i2c_driver tps53681_driver = {
	.driver = {
		.name = "tps53681",
		.of_match_table = of_match_ptr(tps53681_of_match),
	},
	.probe = tps53681_probe,
	.remove = pmbus_do_remove,
	.id_table = tps53681_id,
};

module_i2c_driver(tps53681_driver);

MODULE_AUTHOR("Chris Chiang <Chris.Chiang@wnc.com.tw>");
MODULE_DESCRIPTION("PMBus driver for Texas Instruments TPS53681");
MODULE_LICENSE("GPL");