/*
 * I2C for CPLD
 *
 * Copyright (C) 2017 Delta network Technology Corporation.
 * Masan Xu <masan.xu@deltaww.com>
 *
 * This module supports cpld that read and write.
 *
 * Based on:
 *  i2c_x86-64-agc7648-cpld.c from Aaron Chang <aaron.mh.chang@deltaww.com>
 * Copyright (C) 2017 Delta network Technology Corporation.
 * 
 * Based on:
 *  pca954x.c from Kumar Gala <galak@kernel.crashing.org>
 * Copyright (C) 2006
 *
 * Based on:
 *  pca954x.c from Ken Harrenstien
 * Copyright (C) 2004 Google, Inc. (Ken Harrenstien)
 *
 * Based on:
 *  i2c-virtual_cb.c from Brian Kuschak <bkuschak@yahoo.com>
 * and
 *  pca9540.c from Jean Delvare <khali@linux-fr.org>.
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
#include <linux/ctype.h>
#include <linux/stat.h>
#include <linux/hwmon-sysfs.h>
#include <linux/delay.h>

#define I2C_RW_RETRY_COUNT 10
#define I2C_RW_RETRY_INTERVAL 60 /* ms */
#define CPLD_CHANNEL_SELECT_REG 0x2
#define NUM_OF_CPLD1_CHANS 0x0
#define I2C_CPLD_MUX_MAX_NCHANS NUM_OF_CPLD1_CHANS

static LIST_HEAD(cpld_client_list);
static struct mutex   list_lock;
static unsigned char gReg = 0;

struct cpld_client_node
{
  struct i2c_client *client;
  struct list_head   list;
};

enum cpld_mux_type
{
  cpld
};

struct i2c_cpld_mux
{
  enum cpld_mux_type type;
  struct i2c_client *client;
  u8 last_chan;  /* last register value */
  struct device      *hwmon_dev;
  struct mutex        update_lock;
};

struct chip_desc
{
  u8   nchans;
  u8   deselectChan;
};

/* Provide specs for the PCA954x types we know about */
static const struct chip_desc chips[] =
{
  [cpld] = {
    .nchans      = NUM_OF_CPLD1_CHANS,
    .deselectChan  = NUM_OF_CPLD1_CHANS,
  },
};

static const struct i2c_device_id i2c_cpld_mux_id[] =
{
  { "cpld", cpld },
  { }
};
MODULE_DEVICE_TABLE(i2c, i2c_cpld_mux_id);

/* Write to mux register. Don't use i2c_transfer()/i2c_smbus_xfer()
   for this as they will try to lock adapter a second time */
static int i2c_cpld_mux_reg_write(struct i2c_adapter *adap,
                                  struct i2c_client *client, u8 val)
{
  unsigned long orig_jiffies;
  unsigned short flags;
  union i2c_smbus_data data;
  int try;
  s32 res = -EIO;

  data.byte = val;
  flags = client->flags;
  flags &= I2C_M_TEN | I2C_CLIENT_PEC;


  if (adap->algo->smbus_xfer)
  {
    /* Retry automatically on arbitration loss */
    orig_jiffies = jiffies;
    for (res = 0, try = 0;
           try <= adap->retries;
             try++)
          {
            res = adap->algo->smbus_xfer(adap, client->addr, flags,
                                         I2C_SMBUS_WRITE, CPLD_CHANNEL_SELECT_REG,
                                         I2C_SMBUS_BYTE_DATA, &data);
            if (res != -EAGAIN)
            {
              break;
            }
            if (time_after(jiffies,
                           orig_jiffies + adap->timeout))
            {
              break;
            }
          }
  }

  return res;
}

static int i2c_cpld_mux_select_chan(struct i2c_mux_core *muxc,
                                    u32 chan)
{
  struct i2c_cpld_mux *data = i2c_mux_priv(muxc);
  struct i2c_client *client = data->client;
  u8 regval;
  int ret = 0;

  regval = chan;

  /* Only select the channel if its different from the last channel */
  if (data->last_chan != regval)
  {
    ret = i2c_cpld_mux_reg_write(muxc->parent, client, regval);
    data->last_chan = regval;
  }

  return ret;
}

static int i2c_cpld_mux_deselect_mux(struct i2c_mux_core *muxc,
                                     u32 chan)
{
  struct i2c_cpld_mux *data = i2c_mux_priv(muxc);
  struct i2c_client *client = data->client;

  /* Deselect active channel */
  data->last_chan = chips[data->type].deselectChan;

  //return i2c_cpld_mux_reg_write(adap, client, data->last_chan);
  i2c_cpld_mux_reg_write(muxc->parent, client, data->last_chan);
  return 0;
}

static void i2c_cpld_add_client(struct i2c_client *client)
{
  struct cpld_client_node *node = kzalloc(sizeof(struct cpld_client_node),
                                          GFP_KERNEL);

  if (!node)
  {
    dev_dbg(&client->dev, "Can't allocate cpld_client_node (0x%x)\n", client->addr);
    return;
  }

  node->client = client;

  mutex_lock(&list_lock);
  list_add(&node->list, &cpld_client_list);
  mutex_unlock(&list_lock);
}

static void i2c_cpld_remove_client(struct i2c_client *client)
{
  struct list_head    *list_node = NULL;
  struct cpld_client_node *cpld_node = NULL;
  int found = 0;

  mutex_lock(&list_lock);

  list_for_each(list_node, &cpld_client_list)
  {
    cpld_node = list_entry(list_node, struct cpld_client_node, list);

    if (cpld_node->client == client)
    {
      found = 1;
      break;
    }
  }

  if (found)
  {
    list_del(list_node);
    kfree(cpld_node);
  }

  mutex_unlock(&list_lock);
}

static ssize_t
set_cpld_reg(struct device *dev, struct device_attribute *attr,
             const char *buf, size_t count)
{
  unsigned int val;
  char *endp;

  val = simple_strtoul(buf, &endp, 16);
  if (isspace(*endp))
  {
    endp++;
  }

  if (endp - buf != count)
  {
    return -1;
  }

  if (val > 255)
  {
    return -1;
  }
  gReg = val;

  return count;

}

static ssize_t get_cpld_reg(struct device *dev, struct device_attribute *attr,
                            char *buf)
{
  int len;

  len = sprintf(buf, "0x%x\n", gReg);

  return len;
}


static ssize_t
set_cpld_data(struct device *dev, struct device_attribute *attr,
              const char *buf, size_t count)
{
  struct i2c_client *client;
  unsigned int val;
  char *endp;
  char ret = -1;

  val = simple_strtoul(buf, &endp, 16);
  if (isspace(*endp))
  {
    endp++;
  }

  if (endp - buf != count)
  {
    return -1;
  }

  if (val > 255)
  {
    return -1;
  }

  mutex_lock(&list_lock);
  client = to_i2c_client(dev);

  ret = i2c_smbus_write_byte_data(client, gReg, val);

  mutex_unlock(&list_lock);
  return count;

}

static ssize_t
get_cpld_data(struct device *dev, struct device_attribute *attr, char *buf)
{
  struct i2c_client *client;
  int len;

  client = to_i2c_client(dev);
  len = sprintf(buf, "0x%x\n", i2c_smbus_read_byte_data(client, gReg));

  return len;
}

static struct device_attribute gData = __ATTR(data, 0600, get_cpld_data,
                                              set_cpld_data);
static struct device_attribute gAddr = __ATTR(addr, 0600, get_cpld_reg,
                                              set_cpld_reg);

static ssize_t show_cpld_version(struct device *dev,
                                 struct device_attribute *attr, char *buf)
{
  u8 reg = 0x1;
  struct i2c_client *client;
  int len;

  client = to_i2c_client(dev);
  len = sprintf(buf, "%d\n", i2c_smbus_read_byte_data(client, reg));

  return len;
}

static struct device_attribute ver = __ATTR(version, 0600, show_cpld_version,
                                            NULL);

/*
 * I2C init/probing/exit functions
 */
static int i2c_cpld_mux_probe(struct i2c_client *client,
                              const struct i2c_device_id *id)
{
  struct i2c_adapter *adap = to_i2c_adapter(client->dev.parent);
  int force, class;
  struct i2c_mux_core *muxc;
  struct i2c_cpld_mux *data;
  int ret = 0;
  int chan = 0;

  if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE))
    return -ENODEV;

  muxc = i2c_mux_alloc(adap, &client->dev,
			     chips[id->driver_data].nchans, sizeof(*data), 0,
			     i2c_cpld_mux_select_chan, i2c_cpld_mux_deselect_mux);
  if (!muxc)
    return -ENOMEM;

  i2c_set_clientdata(client, muxc);
  data = i2c_mux_priv(muxc);
  data->client = client;

  data->type = id->driver_data;
  data->last_chan =
    chips[data->type].deselectChan; /* force the first selection */

  mutex_init(&data->update_lock);
  /* Now create an adapter for each channel */
  for (chan = 0; chan < chips[data->type].nchans; chan++)
  {
    force = 0;  /* dynamic adap number */
    class = 0;  /* no class by default */

    ret = i2c_mux_add_adapter(muxc, force, chan, class);

    if (ret)
    {
      dev_err(&client->dev, "failed to register multiplexed adapter %d\n", chan);
      goto virt_reg_failed;
    }
  }

  dev_info(&client->dev, "registered %d multiplexed busses for I2C mux %s\n",
           chan, client->name);

  i2c_cpld_add_client(client);

  ret = sysfs_create_file(&client->dev.kobj, &ver.attr);
  if (ret)
  {
    goto virt_reg_failed;
  }
  ret = sysfs_create_file(&client->dev.kobj, &gData.attr);
  if (ret)
  {
    goto virt_reg_failed;
  }
  ret = sysfs_create_file(&client->dev.kobj, &gAddr.attr);
  if (ret)
  {
    goto virt_reg_failed;
  }

  return 0;

virt_reg_failed:
  i2c_mux_del_adapters(muxc);
  return ret;
}

static int i2c_cpld_mux_remove(struct i2c_client *client)
{
  struct i2c_mux_core *muxc = i2c_get_clientdata(client);

  sysfs_remove_file(&client->dev.kobj, &ver.attr);

  i2c_cpld_remove_client(client);
  i2c_mux_del_adapters(muxc);

  return 0;
}

static int i2c_cpld_read_internal(struct i2c_client *client, u8 reg)
{
  int status = 0, retry = I2C_RW_RETRY_COUNT;

  while (retry) 
  {
    status = i2c_smbus_read_byte_data(client, reg);
    if (unlikely(status < 0)) 
    {
      msleep(I2C_RW_RETRY_INTERVAL);
      retry--;
      continue;
    }
    break;
  }
  return status;
}

static int i2c_cpld_write_internal(struct i2c_client *client, u8 reg, u8 value)
{
  int status = 0, retry = I2C_RW_RETRY_COUNT;

  while (retry)
  {
    status = i2c_smbus_write_byte_data(client, reg, value);
    if (unlikely(status < 0))
    {
      msleep(I2C_RW_RETRY_INTERVAL);
      retry--;
      continue;
    }
    break;
  }
  return status;
}

int i2c_cpld_read(int bus, unsigned short cpld_addr, u8 reg)
{
  struct list_head   *list_node = NULL;
  struct cpld_client_node *cpld_node = NULL;
  int ret = -EPERM;

  mutex_lock(&list_lock);

  list_for_each(list_node, &cpld_client_list)
  {
    cpld_node = list_entry(list_node, struct cpld_client_node, list);

    if ((cpld_node->client->adapter->nr == bus) &&
        (cpld_node->client->addr == cpld_addr) )
    {
      ret = i2c_cpld_read_internal(cpld_node->client, reg);
      break;
    }
  }

  mutex_unlock(&list_lock);

  return ret;
}
EXPORT_SYMBOL(i2c_cpld_read);

int i2c_cpld_write(int bus, unsigned short cpld_addr, u8 reg, u8 value)
{
  struct list_head   *list_node = NULL;
  struct cpld_client_node *cpld_node = NULL;
  int ret = -EIO;

  mutex_lock(&list_lock);

  list_for_each(list_node, &cpld_client_list)
  {
    cpld_node = list_entry(list_node, struct cpld_client_node, list);

    if ((cpld_node->client->adapter->nr == bus) &&
        (cpld_node->client->addr == cpld_addr) )
    {
      ret = i2c_cpld_write_internal(cpld_node->client, reg, value);
      break;
    }
  }

  mutex_unlock(&list_lock);

  return ret;
}
EXPORT_SYMBOL(i2c_cpld_write);

static struct i2c_driver i2c_cpld_mux_driver =
{
  .driver   = {
    .name = "cpld",
    .owner  = THIS_MODULE,
  },
  .probe    = i2c_cpld_mux_probe,
  .remove   = i2c_cpld_mux_remove,
  .id_table = i2c_cpld_mux_id,
};

static int __init i2c_cpld_mux_init(void)
{
  mutex_init(&list_lock);
  return i2c_add_driver(&i2c_cpld_mux_driver);
}

static void __exit i2c_cpld_mux_exit(void)
{
  i2c_del_driver(&i2c_cpld_mux_driver);
}

MODULE_AUTHOR("masan.xu@deltaww.com");
MODULE_DESCRIPTION("I2C CPLD mux driver");
MODULE_LICENSE("GPL");

module_init(i2c_cpld_mux_init);
module_exit(i2c_cpld_mux_exit);
