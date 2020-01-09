/*sff_io.c
 * handle sff io pin control , mux gpio .. etc*/
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include "sff_io.h"
#include "inv_def.h"
#include "io_config/io_config.h"

#define DEBUG_MODE (0)
//#define TEST_CODE
#if (DEBUG_MODE == 1)
#define SFF_IO_DEBUG(fmt, args...) \
    printk (KERN_INFO "%s: " fmt "\r\n", __FUNCTION__,  ##args)
#else
#define SFF_IO_DEBUG(fmt, args...)
#endif
#define SFF_IO_INFO(fmt, args...) printk( KERN_INFO "[SFF_IO]%s " fmt, __FUNCTION__, ##args)
#define SFF_IO_ERR(fmt, args...)  printk_ratelimited (KERN_ERR "{SFF_IO]%s: " fmt "\r\n",__FUNCTION__,  ##args)
//#define SFF_IO_ERR_RATE_LIMITED(fmt, args...)  printk_ratelimited (KERN_ERR "{SFF_IO]%s: " fmt "\r\n",__FUNCTION__,  ##args)

struct cpld_client_t {

    struct i2c_client *client;
    struct mutex lock;
};
struct cpld_io_t {

    struct cpld_config_t *config;
    int *io_ch;
    struct cpld_client_t cpld_client[IO_CH_ID_NUM];
    int int_gpio;
    u8 ioexp_int_status_offset;
    u8 ioexp_int_enable_offset;
    int (*init)(struct cpld_io_t *self);
    int (*deinit)(struct cpld_io_t *self);
    int (*ioexp_int_status_get)(struct cpld_io_t *self, int *reg);
    int (*ioexp_int_enable)(struct cpld_io_t *self);
    // int (*ioexp_int_status_clear)(struct cpld_io_t *self);
    //u8 ioexp_int_clear_offset;
};
static int io_port_num = 0;
struct cpld_io_t cpldIo;

struct ioexp_obj_t {

    struct ioexp_func_t *func;
    int *st;
    char *name;
};

struct sff_io_t {

    struct input_change_table_t *input_table;
    int (*input_handler)(struct sff_io_t *);
};
struct pca95xx_type_t *pca95xx_type_tbl = NULL;
int *io_i2c_tbl = NULL;
struct ioexp_config_t *ioexp_config = NULL;
typedef enum {
    PCA95XX_INPUT,
    PCA95XX_OUTPUT,
    PCA95XX_CONFIG,
    PCA95XX_NUM,

} reg_offset_type;

struct pca95xx_func_t {
    u8 offset[PCA95XX_NUM];
    int (*read)(u8 ch, u8 addr, u8 offset);
    int (*write)(u8 ch, u8 addr, u8 offset, u16 val);
};


struct sff_io_t SffIo;
struct ioexp_config_t *ioexp_head = NULL;
struct ioexp_config_t *ioexp_tail = NULL;
struct ioexp_obj_t ioexp_obj[IO_IDX_NUM];
struct sff_io_obj_t *Sff_Io = NULL;

static int inv_i2c_smbus_read_byte_data(struct i2c_client *client, u8 offset);
static int ioexp_count = 0;
static int health_fail_count = 0;
static int pca9555_word_read(u8 ch, u8 addr, u8 offset);
static int pca9555_word_write(u8 ch, u8 addr, u8 offset, u16 val);
static int pca9555_byte_read(u8 ch, u8 addr, u8 offset);
static int pca9555_byte_write(u8 ch, u8 addr, u8 offset, u16 val);
static int mux_reset_gpio = 0;
static int gpio_base = 0;
static int cpld_client_deinit(struct cpld_io_t *self);

struct pca95xx_func_t pca95xx_func[PCA95XX_TYPE_NUM] = {
    [PCA9555_TYPE] =
    {   {0x00,0x02,0x06}, /*pca9555*/
        .read = pca9555_word_read,
        .write =  pca9555_word_write
    },

    [PCA9554_TYPE] =
    {   {0x00,0x01,0x03},/*pca9554*/
        .read = pca9555_byte_read,
        .write =  pca9555_byte_write
    },
};
static int port_num_get(void)
{
    return io_port_num;
}

struct sff_io_client_node {
    struct i2c_client *client;
    int i2c_ch;
    struct list_head list;
    struct mutex lock;
};
static LIST_HEAD(sff_io_client_list);
/*isr mode*/

int ioexp_obj_set(struct ioexp_obj_t *self, int port, int val);
int ioexp_obj_get(struct ioexp_obj_t *self, int port);
static struct sff_io_client_node *sff_io_client_find(int i2c_ch, u8 slave_addr)
{
    struct list_head   *list_node = NULL;
    struct sff_io_client_node *node = NULL;
    int find = 0;
    list_for_each(list_node, &sff_io_client_list) {
        node = list_entry(list_node, struct sff_io_client_node, list);

        if (node->i2c_ch == i2c_ch) {
            find = 1;
            node->client->addr = slave_addr;
            break;
        }
    }

    if(!find) {
        SFF_IO_ERR("%s fail/n", __FUNCTION__);
        return NULL;
    }
    return node;
}
static int inv_i2c_smbus_write_byte_data(struct i2c_client *client, u8 offset, u8 buf)
{
    int i;
    int ret = 0;
    for(i=0; i< RETRY_COUNT; i++) {
        ret = i2c_smbus_write_byte_data(client, offset, buf);

        if (ret < 0) {
            msleep(RETRY_DELAY_MS);
            continue;
        }
        break;
    }
    if (i >= RETRY_COUNT) {
        SFF_IO_ERR("%s fail:offset:0x%x try %d/%d! Error Code: %d\n", __func__, offset, i, RETRY_COUNT, ret);
    }
    return ret;
}
static int inv_i2c_smbus_read_byte_data(struct i2c_client *client, u8 offset)
{
    int ret = 0;
    int i = 0;

    for (i = 0; i < RETRY_COUNT; i++) {

        ret = i2c_smbus_read_byte_data(client, offset);
        if (ret < 0) {
            msleep(RETRY_DELAY_MS);
            continue;
        }
        break;
    }

    if (i >= RETRY_COUNT) {
        printk("%s fail:offset:0x%x try %d/%d! Error Code: %d\n", __func__, offset, i, RETRY_COUNT, ret);
    }

    return ret;
}
static int inv_i2c_smbus_read_word_data(struct i2c_client *client, u8 offset)
{
    int ret = 0;
    int i = 0;

#if 1
    for (i = 0; i < RETRY_COUNT; i++) {

        ret = i2c_smbus_read_word_data(client, offset);
        if (ret < 0) {
            msleep(RETRY_DELAY_MS);
            continue;
        }
        break;
    }
    if (i >= RETRY_COUNT) {
        SFF_IO_ERR("%s fail:offset:0x%x try %d/%d! Error Code: %d\n", __func__, offset, i, RETRY_COUNT, ret);
    }

    return ret;
#else

    u8 buf[2];
    for (i = 0; i < 2; i++) {

        ret = inv_i2c_smbus_read_byte_data(client, offset);
        if (ret < 0) {
            return ret;
        }
        buf[i] = ret;
    }

    ret = 0x0000ffff | (buf[0] | (buf[1] << 8));
    return ret;

#endif
}
static int inv_i2c_smbus_write_word_data(struct i2c_client *client, u8 offset, u16 buf)
{
    int i;
    int ret = 0;
    for(i=0; i< RETRY_COUNT; i++) {
        ret = i2c_smbus_write_word_data(client, offset, buf);

        if (ret < 0) {
            msleep(RETRY_DELAY_MS);
            continue;
        }
        break;
    }
    if (i >= RETRY_COUNT) {
        SFF_IO_ERR("%s fail:offset:0x%x try %d/%d! Error Code: %d\n", __func__, offset, i, RETRY_COUNT, ret);
    }
    return ret;
}

static struct pca95xx_func_t *pca95xx_func_find(int ch, u8 addr)
{
    int i = 0;
    if (!pca95xx_type_tbl) {
        SFF_IO_ERR("NULL ptr\n");
        return NULL;
    }
    for (i = 0; pca95xx_type_tbl[i].ch != END_OF_TABLE; i++) {

        if (pca95xx_type_tbl[i].ch == ch &&
                pca95xx_type_tbl[i].addr == addr) {
            return &pca95xx_func[pca95xx_type_tbl[i].type];
        }
    }

    return NULL;

}
static int pca95xx_reg_read(u8 ch, u8 addr, u8 offset_type)
{
    struct pca95xx_func_t *func = NULL;
    func = pca95xx_func_find(ch, addr);
    if (!func) {
        return -1;
    }
    return func->read(ch, addr, func->offset[offset_type]);
}
static int pca95xx_reg_write(u8 ch, u8 addr, u8 offset_type, u16 val)
{
    struct pca95xx_func_t *func = NULL;
    func = pca95xx_func_find(ch, addr);
    if (!func) {
        return -1;
    }
    return func->write(ch, addr, func->offset[offset_type], val);
}

static int pca9555_byte_read(u8 ch, u8 addr, u8 offset)
{

    struct sff_io_client_node *node = NULL;
    int ret = 0;
    node = sff_io_client_find(ch, addr);

    if(!node) {
        return -1;
    }
    mutex_lock(&node->lock);
    ret = inv_i2c_smbus_read_byte_data(node->client, offset);
    mutex_unlock(&node->lock);

    return ret;
}

static int pca9555_byte_write(u8 ch, u8 addr, u8 offset, u16 val)
{
    struct sff_io_client_node *node = NULL;
    int ret = 0;
    node = sff_io_client_find(ch, addr);

    if(!node) {
        return -1;
    }
    mutex_lock(&node->lock);
    ret = inv_i2c_smbus_write_byte_data(node->client, offset, (u8)val);
    mutex_unlock(&node->lock);

    return ret;
}
static int pca9555_word_read(u8 ch, u8 addr, u8 offset)
{

    struct sff_io_client_node *node = NULL;
    int ret = 0;
    node = sff_io_client_find(ch, addr);

    if(!node) {
        return -1;
    }
    mutex_lock(&node->lock);
    ret = inv_i2c_smbus_read_word_data(node->client, offset);
    mutex_unlock(&node->lock);

    return ret;
}

static int pca9555_word_write(u8 ch, u8 addr, u8 offset, u16 val)
{
    struct sff_io_client_node *node = NULL;
    int ret = 0;
    node = sff_io_client_find(ch, addr);

    if(!node) {
        return -1;
    }
    mutex_lock(&node->lock);
    ret = inv_i2c_smbus_write_word_data(node->client, offset, val);
    mutex_unlock(&node->lock);

    return ret;
}
int sff_io_prsL_get(int port, u8 *val)
{
    int ret = 0;
    ret = ioexp_obj_get(&ioexp_obj[IO_PRESENT], port);
    if (ret < 0 ||
            ret == REC_SFF_IO_UNSUPPORTED) {
        return ret;
    }
    *val = (u8)ret;
    return 0;
}

EXPORT_SYMBOL(sff_io_prsL_get);

/*[note] when sff module is prsL , corrensponding bit be set*/
int sff_io_prsL_all_get(unsigned long *bitmap)
{
    int port = 0;
    int port_num = port_num_get();
    unsigned long tmp = 0;
    u8 prsL = 0;
    for(port = 0; port < port_num; port++) {
        if (sff_io_prsL_get(port, &prsL) < 0) {
            SFF_IO_ERR("port:%d", port);
            *bitmap = (~(0L));
            return -1;
        }

        if(prsL) {
            set_bit(port, &tmp);
        } else {
            clear_bit(port, &tmp);
        }
    }

    *bitmap = tmp;
    return 0;
}
EXPORT_SYMBOL(sff_io_prsL_all_get);

int sff_io_reset_set(int port, u8 val)
{
    return ioexp_obj_set(&ioexp_obj[IO_RESET], port, val);
}
EXPORT_SYMBOL(sff_io_reset_set);

int sff_io_reset_get(int port, u8 *val)
{
    int ret = 0;
    ret = ioexp_obj_get(&ioexp_obj[IO_RESET], port);
    if (ret < 0 ||
            ret == REC_SFF_IO_UNSUPPORTED) {
        return ret;
    }
    *val = (u8)ret;
    return 0;
}
EXPORT_SYMBOL(sff_io_reset_get);

int sff_io_lpmode_set(int port, u8 val)
{
    return ioexp_obj_set(&ioexp_obj[IO_LPMODE], port, val);
}
EXPORT_SYMBOL(sff_io_lpmode_set);

int sff_io_lpmode_get(int port, u8 *lpmode)
{
    int ret = 0;
    ret = ioexp_obj_get(&ioexp_obj[IO_LPMODE], port);
    if (ret < 0 ||
            ret == REC_SFF_IO_UNSUPPORTED) {
        return ret;
    }
    *lpmode = (u8)ret;
    return 0;
}
EXPORT_SYMBOL(sff_io_lpmode_get);

int sff_io_tx_disable_set(int port, u8 value)
{
    return ioexp_obj_set(&ioexp_obj[IO_TXDISABLE], port, value);
}
EXPORT_SYMBOL(sff_io_tx_disable_set);

int sff_io_tx_disable_get(int port, u8 *value)
{
    int ret = 0;
    ret = ioexp_obj_get(&ioexp_obj[IO_TXDISABLE], port);
    if (ret < 0 ||
            ret == REC_SFF_IO_UNSUPPORTED) {
        return ret;
    }
    *value = (u8)ret;
    return 0;

}
EXPORT_SYMBOL(sff_io_tx_disable_get);


int sff_io_mode_sel_set(int port, u8 val)
{
    return ioexp_obj_set(&ioexp_obj[IO_MODE_SEL], port, val);
}
EXPORT_SYMBOL(sff_io_mode_sel_set);

int sff_io_mode_sel_get(int port, u8 *val)
{
    int ret = 0;
    ret = ioexp_obj_get(&ioexp_obj[IO_MODE_SEL], port);
    if (ret < 0 ||
            ret == REC_SFF_IO_UNSUPPORTED) {
        return ret;
    }
    *val = (u8)ret;
    return 0;
}
EXPORT_SYMBOL(sff_io_mode_sel_get);

int sff_io_intL_get(int port, u8 *val)
{

    int ret = 0;
    ret = ioexp_obj_get(&ioexp_obj[IO_INTL], port);
    if (ret < 0 ||
            ret == REC_SFF_IO_UNSUPPORTED) {
        return ret;
    }
    *val = (u8)ret;
    return 0;
}
EXPORT_SYMBOL(sff_io_intL_get);

int sff_io_rx_los_get(int port, u8 *val)
{
    int ret = 0;
    ret = ioexp_obj_get(&ioexp_obj[IO_RX_LOS], port);
    if (ret < 0 ||
            ret == REC_SFF_IO_UNSUPPORTED) {
        return ret;
    }
    *val = (u8)ret;
    return 0;
}
EXPORT_SYMBOL(sff_io_rx_los_get);

int sff_io_tx_fault_get(int port, u8 *val)
{
    int ret = 0;
    ret = ioexp_obj_get(&ioexp_obj[IO_TX_FAULT], port);
    if (ret < 0 ||
            ret == REC_SFF_IO_UNSUPPORTED) {
        return ret;
    }
    *val = (u8)ret;
    return 0;
}
EXPORT_SYMBOL(sff_io_tx_fault_get);

static void sff_io_client_destroy(int i2c_ch)
{
    struct list_head   *list_node = NULL;
    struct sff_io_client_node *sff_io_client_node = NULL;
    bool found = false;
    list_for_each(list_node, &sff_io_client_list) {
        sff_io_client_node = list_entry(list_node, struct sff_io_client_node, list);
        if(i2c_ch == sff_io_client_node->i2c_ch) {
            found = true;
            break;
        }
    }

    if(found) {
        list_del(list_node);
        if (sff_io_client_node) {
            if (sff_io_client_node->client) {
                if (sff_io_client_node->client->adapter) {
                    i2c_put_adapter(sff_io_client_node->client->adapter);
                }
                kfree(sff_io_client_node->client);
            }
            kfree(sff_io_client_node);
        }
    }
}
static void sff_io_client_destroy_all(void)
{
    int idx = 0;
    for(idx = 0; io_i2c_tbl[idx] != END_OF_TABLE; idx++) {
        sff_io_client_destroy(io_i2c_tbl[idx]);
    }
}
static int sff_io_client_create(int ch)
{
    struct i2c_client *client = NULL;
    struct sff_io_client_node *node = NULL;
    struct i2c_adapter *adap;

    node = kzalloc(sizeof(struct sff_io_client_node), GFP_KERNEL);
    if(!node) {
        goto exit_err;
    }
    client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
    if(!client) {
        goto exit_kfree_node;
    }

    /*get i2c adapter*/
    adap = i2c_get_adapter(ch);
    if (!adap) {
        SFF_IO_ERR("fail to get adapter ch:%d\n", ch);
        goto exit_kfree_sff_io_client;
    }
    client->adapter = adap;

    node->client = client;
    node->i2c_ch = ch;
    mutex_init(&node->lock);
    list_add(&node->list, &sff_io_client_list);
    return 0;

exit_kfree_sff_io_client:
    if (client) {
        kfree(client);
    }
exit_kfree_node:
    if (node) {
        kfree(node);
    }
exit_err:
    return -1;

}
static int sff_io_client_create_all(void)
{
    int idx = 0;
    int ret = 0;
    for(idx = 0; io_i2c_tbl[idx] != END_OF_TABLE; idx++) {
        if((ret = sff_io_client_create(io_i2c_tbl[idx])) < 0) {
            break;
        }
    }
    if (ret < 0) {
        sff_io_client_destroy_all();
        return -EBADRQC;
    }
    return 0;
}

int sff_io_mux_init(void)
{
    /*pull high*/
    int ret = 0;
    /*<TBD> insmod gpiobase later on*/
    mux_reset_gpio += gpio_base;
    ret = gpio_is_valid(mux_reset_gpio);
    if (ret < 0) {
        SFF_IO_ERR("invaid gpio:%d ret:%d\n", mux_reset_gpio, ret);
        return -1;
    }
/*don't set direction since , it's decided at bios stage*/
#if 0
    if(gpio_direction_output(mux_reset_gpio, 1) < 0) {
        SFF_IO_ERR("init NG\n");
        return -1;
    }
#endif    
    mdelay(1); /*1ms*/

    SFF_IO_ERR("init Pass\n");
    return 0;

}

int sff_io_mux_reset_get(void)
{
    int val = 0;
    val = gpio_get_value(mux_reset_gpio);
    return val;
}

#if 1
/*io mux control*/
int sff_io_mux_reset_all(int value)
{
    gpio_set_value(mux_reset_gpio, value);
    return 0;
}
#else
int sff_io_mux_reset_all(int val)
{
    /*pull low*/
    if(gpio_direction_output(mux_reset_gpio, val) < 0) {
        SFF_IO_ERR("pull low NG\n");
        return -1;
    }
    SFF_IO_DEBUG("pass\n");
    return 0;

}
#endif
EXPORT_SYMBOL(sff_io_mux_reset_all);
/*isr mode*/
struct ioexp_data_t *find_ioexp_by_port(struct ioexp_data_t map[], int port)
{

    int i = 0;
    int port_max = 0;
    for (i = 0; map[i].ch != END_OF_TABLE; i++) {
        port_max = map[i].port_min + (map[i].bit_max - map[i].bit_min);
        if (port <= port_max) {
            return &map[i];
        }
    }
    return NULL;
}
struct ioexp_data_t *find_ioexp_by_ch_addr(struct ioexp_data_t map[], int ch, u8 addr)
{

    int i = 0;
    for (i = 0; map[i].ch != END_OF_TABLE; i++) {
        if (ch == map[i].ch &&
                addr == map[i].addr) {
            return &map[i];
        }
    }
    return NULL;
}
struct ioexp_obj_t *find_ioexp_obj(int idx)
{
    return &ioexp_obj[idx];
}
/*port: front port*/
int ioexp_obj_set(struct ioexp_obj_t *self, int port, int val)
{
    int ret = 0;
    struct ioexp_data_t *ioexp = NULL;
    int bit = 0;
    int ioexp_port = 0;
    if (!(self->func->map)) {
        return REC_SFF_IO_UNSUPPORTED;
    }
    ioexp_port = self->func->port_2_ioPort[port];
    ioexp = find_ioexp_by_port(self->func->map, ioexp_port);
    if (NULL == ioexp) {
        SFF_IO_ERR("fail:%d\n", port);
        return 0;
    }
    ret = pca95xx_reg_read(ioexp->ch, ioexp->addr, PCA95XX_OUTPUT);
    if (ret < 0) {
        SFF_IO_ERR("%s read word fail: %d\n", self->name, ioexp->ch);
        return ret;
    }
    bit = ioexp->bit_min + (ioexp_port - ioexp->port_min);

    if (val) {
        set_bit(bit, (unsigned long *)&ret);
    } else {
        clear_bit(bit, (unsigned long *)&ret);
    }

    SFF_IO_DEBUG("ch:%d addr:0x%x\n", ioexp->ch, ioexp->addr);
    SFF_IO_DEBUG("%s io_port:%d port:%d ret:0x%x bit:%d\n", self->name, ioexp_port, port, ret, bit);
    ret = pca95xx_reg_write(ioexp->ch, ioexp->addr, PCA95XX_OUTPUT, ret);
    if (ret < 0) {
        SFF_IO_ERR("%s write word fail: %d\n", self->name, ioexp->ch);
        return ret;
    }
    self->st[port] = val;
    return 0;

}
/*port: front port*/
int ioexp_obj_get(struct ioexp_obj_t *self, int port)
{
    if (!(self->func->map)) {
        return REC_SFF_IO_UNSUPPORTED;
    }

    return self->st[port];
}
static bool _ioexp_is_channel_ready( struct ioexp_config_t *config)
{
    int ret = 0;
    if (!config) {

        SFF_IO_ERR("NULL ptr\n");
        return true;
    }
    ret = pca95xx_reg_read(config->ch, config->addr, PCA95XX_CONFIG);
    if (ret < 0) {
        SFF_IO_ERR("ch not ready\n");
        return false;
    }
    return true;
}

bool ioexp_is_channel_ready(void)
{
    if (_ioexp_is_channel_ready(ioexp_head) &&
            _ioexp_is_channel_ready(ioexp_tail)) {

        return true;
    }
    return false;
}
/*only for mux reset check, one ioexpander read pass = pass*/
bool ioexp_scan_ready(void)
{
    /*should be sufficent to check each channel*/
    int count = 0;
    int ret = 0;
    struct ioexp_config_t *config = NULL;
    int ch = END_OF_TABLE;
    /*config io exp*/
    for (count = 0; ioexp_config[count].ch != END_OF_TABLE; count++) {
        config = &ioexp_config[count];

        if (ch != config->ch) {
            ret = pca95xx_reg_read(config->ch, config->addr, PCA95XX_CONFIG);
            if (ret >= 0) {
                SFF_IO_DEBUG("pass\n");
                return true;
            }
            // SFF_IO_ERR("io config ok: ch:0x%x addr:0x%x val:0x%x\n", config->ch, config->addr, config->val);
        }
        ch = config->ch;
    }

    return false;
}
bool ioexp_is_i2c_ready(void)
{
    int count = 0;
    int ret = 0;
    struct ioexp_config_t *config = NULL;
    int ch = END_OF_TABLE;
    int fail_count = 0;
    /*config io exp*/
    for (count = 0; ioexp_config[count].ch != END_OF_TABLE; count++) {
        config = &ioexp_config[count];

        if (ch != config->ch) {
            ret = pca95xx_reg_read(config->ch, config->addr, PCA95XX_CONFIG);
            if (ret < 0) {
                SFF_IO_ERR("io config fail: ch:0x%x addr:0x%x val:0x%x\n", config->ch, config->addr, config->val);
                fail_count++;
                continue;
            }

            // SFF_IO_ERR("io config ok: ch:0x%x addr:0x%x val:0x%x\n", config->ch, config->addr, config->val);
        }
        ch = config->ch;
    }
    if (fail_count > 0) {
        return false;
    }

    SFF_IO_DEBUG("pass\n");
    return true;

}
int ioexp_health_monitor(void)
{
    int ret = 0;
    struct ioexp_config_t *config = NULL;
    int val = 0;
    val = gpio_get_value(cpldIo.int_gpio);
    /*when interrupt is happening, skip io expander checking*/
    if (0 == val) {
        return 0;
    }
    /*config io exp*/
    if (ioexp_config[ioexp_count].ch == END_OF_TABLE) {
        ioexp_count = 0;
        if (0 == health_fail_count) {
            //SFF_IO_DEBUG("io configs check ok\n");
        } else {
            SFF_IO_ERR("io configs check fail\n");
        }
        health_fail_count = 0;
    }
    config = &ioexp_config[ioexp_count++];
    ret = pca95xx_reg_read(config->ch, config->addr, PCA95XX_CONFIG);
    if (ret < 0) {
        SFF_IO_ERR("io config fail: ch:0x%x addr:0x%x val:0x%x\n", config->ch, config->addr, config->val);
        health_fail_count++;
        if (!ioexp_is_channel_ready()) {

            health_fail_count = 0;
            SFF_IO_ERR("i2c bus crush!\n");
            return -1;
        }
    }
    //SFF_IO_DEBUG("io config ok: ch:0x%x addr:0x%x val:0x%x\n", config->ch, config->addr, config->val);
    return 0;
}
int ioexp_config_init(void)
{
    int count = 0;
    int ret = 0;
    struct ioexp_config_t *config = NULL;
    /*config io exp*/
    for (count = 0; ioexp_config[count].ch != END_OF_TABLE; count++) {
        config = &ioexp_config[count];
        ret = pca95xx_reg_write(config->ch, config->addr, PCA95XX_CONFIG, config->val);
        if (ret < 0) {
            SFF_IO_ERR("io config fail: ch:0x%x addr:0x%x val:0x%x\n", config->ch, config->addr, config->val);
            return ret;
        }
    }
    /*assign for ioexp channel check*/
    ioexp_head = &ioexp_config[0];
    ioexp_tail = &ioexp_config[count-1];
    return 0;
}
static struct i2c_client *_cpld_client_init(int ch)
{
    struct i2c_client *client = NULL;
    struct i2c_adapter *adap = NULL;
    client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);

    if (!client) {
        goto exit_err;
    }
    SFF_IO_DEBUG("start\n");
    /*get i2c adapter*/
    adap = i2c_get_adapter(ch);
    if (!adap) {
        SFF_IO_ERR("fail to get adapter ch:%d\n", ch);
        goto exit_kfree_i2c_client;
    }
    client->adapter = adap;

    SFF_IO_DEBUG("init ok\n");
    return client;

exit_kfree_i2c_client:
    if (client) {
        kfree(client);
    }
exit_err:
    return NULL;
}
static int cpld_client_init(struct cpld_io_t *self)
{

    struct i2c_client *client = NULL;
    int id = 0;
    int ret = 0;
    if (!self) {
        return -EINVAL;
    }

    for (id = 0; END_OF_TABLE != self->io_ch[id] && id < IO_CH_ID_NUM; id++) {

        self->cpld_client[id].client = _cpld_client_init(self->io_ch[id]);
        client = self->cpld_client[id].client;
        if (!client) {
            ret = -EBADRQC;
            break;
        }
        mutex_init(&(self->cpld_client[id].lock));
    }
    if (-EBADRQC == ret) {

        cpld_client_deinit(self);
        return ret;
    }

    return 0;

}
static int cpld_client_deinit(struct cpld_io_t *self)
{
    struct i2c_client *client = NULL;
    int id = 0;
    if (!self) {
        return -1;
    }

    for (id = 0; END_OF_TABLE != self->io_ch[id] && id < IO_CH_ID_NUM; id++) {
        client = self->cpld_client[id].client;
        if (client) {
            if (client->adapter) {
                i2c_put_adapter(client->adapter);
            }
            kfree(client);
        }
    }
    return 0;
}
int cpld_ioexp_no_support(struct cpld_io_t *self)
{

    SFF_IO_ERR("NO support\n");
    return 0;
}
/*argument:
 * self [in] config [in]
 * cpld_client[out]*/
static void cpld_client_get(struct cpld_io_t *self, struct cpld_config_t *config,
                            struct cpld_client_t **cpld_client)
{
    int io_ch_id = config->io_ch_id;
    u8 addr = config->addr;

    (*cpld_client) = &(self->cpld_client[io_ch_id]);
    (*cpld_client)->client->addr = addr;
}
static int cpld_i2c_smbus_write_byte_data(struct cpld_client_t *cpld_client, u8 offset, u8 val)
{
    int ret = 0;
    mutex_lock(&cpld_client->lock);
    ret = inv_i2c_smbus_write_byte_data(cpld_client->client, offset, val);
    mutex_unlock(&cpld_client->lock);
    if (ret < 0) {
        return ret;
    }
    return 0;

}
static int cpld_i2c_smbus_read_word_data(struct cpld_client_t *cpld_client, u8 offset)
{
    int ret = 0;
    mutex_lock(&cpld_client->lock);
    ret = inv_i2c_smbus_read_word_data(cpld_client->client, offset);
    mutex_unlock(&cpld_client->lock);
    return ret;

}
static int cpld_i2c_smbus_read_byte_data(struct cpld_client_t *cpld_client, u8 offset)
{
    int ret = 0;
    mutex_lock(&cpld_client->lock);
    ret = inv_i2c_smbus_read_byte_data(cpld_client->client, offset);
    mutex_unlock(&cpld_client->lock);
    return ret;

}

/*enable cpld ioexp isr mode*/
int cpld_ioexp_int_enable(struct cpld_io_t *self)
{
    int ret = 0;
    struct cpld_client_t *cpld_client = NULL;

    cpld_client_get(self, &self->config[CPLD_ID2], &cpld_client);
    ret = cpld_i2c_smbus_write_byte_data(cpld_client, self->ioexp_int_enable_offset, 0x01);

    if (ret < 0) {
        return ret;
    }
    return 0;
}
int cpld_ioexp_int_status_get_type1(struct cpld_io_t *self, int *reg)
{
    int ret = 0;
    int sfp = 0;
    int qsfp = 0;
    struct cpld_client_t *cpld_client = NULL;

    if(!reg) {
        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    cpld_client_get(self, &self->config[CPLD_ID2], &cpld_client);

    ret = cpld_i2c_smbus_read_word_data(cpld_client, self->ioexp_int_status_offset);
    if (ret < 0) {
        SFF_IO_ERR("2 read fail\n");
        return ret;
    }
    /*sfp 0x35 bit 0:7 0x36 bit 0:4
     * qsfp 0x36 bit 7, merge it into one data*/
    sfp = ret & 0xfff;
    qsfp = (ret & 0x8000) >> 3;

    /*bit = 0: interrupt occurs , reverse it for easy logic*/

    *reg = (~(sfp|qsfp)) & 0x1fff;
    return 0;
}
/*type2 is qsfp only: so far for banyan*/
int cpld_ioexp_int_status_get_type2(struct cpld_io_t *self, int *reg)
{
    int ret = 0;
    int qsfp = 0;
    struct cpld_client_t *cpld_client = NULL;

    if(!reg) {
        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    cpld_client_get(self, &self->config[CPLD_ID2], &cpld_client);
    ret = cpld_i2c_smbus_read_byte_data(cpld_client, self->ioexp_int_status_offset);
    if (ret < 0) {
        SFF_IO_ERR("2 read fail\n");
        return ret;
    }

    qsfp = (ret & 0xf);
    /*bit = 0: interrupt occurs , reverse it for easy logic*/
    *reg = (~qsfp) & 0xf;
    return 0;
}
/*so far is for cedar only*/
int cpld_ioexp_int_status_get_type3(struct cpld_io_t *self, int *reg)
{
    int ret = 0;
    int qsfp = 0;
    struct cpld_client_t *cpld_client = NULL;
    u16 bit_mask[4] = {0x4, 0x80, 0x400, 0x8000};
    int i = 0;


    if(!reg) {
        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    cpld_client_get(self, &self->config[CPLD_ID2], &cpld_client);

    ret = cpld_i2c_smbus_read_word_data(cpld_client, self->ioexp_int_status_offset);
    if (ret < 0) {
        SFF_IO_ERR("2 read fail\n");
        return ret;
    }
    SFF_IO_DEBUG("ret:0x%x\n", ret);
    for (i = 0; i < 4; i++) {

        if (!(ret & bit_mask[i])) {
            qsfp |= 1 << i;
        }
    }
    *reg = qsfp;
    return 0;
}
/*update the pin status of corresonding ioexpander control(ex: prsL) by port*/
int ioexp_st_update(int data,
                    struct ioexp_data_t *ioexp,
                    struct ioexp_obj_t *obj)
{
    int port = 0;
    int ioexp_port = 0;
    int bit = 0;
    int bit_min = 0;
    int bit_max = 0;
    int val = 0;
    int *ioPort_2_port = NULL;
    int *st = NULL;
    u8 *name = NULL;

    if (!obj) {

        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    ioPort_2_port = obj->func->ioPort_2_port;
    st = obj->st;
    name = obj->name;

    ioexp_port = ioexp->port_min;
    bit_min = ioexp->bit_min;
    bit_max = ioexp->bit_max;

    for (bit = bit_min; bit <= bit_max; bit++) {
        val = ((data & (1 << bit)) ? 1 : 0);
        port = ioPort_2_port[ioexp_port];

        if (val != st[port]) {
            if (val) {
                SFF_IO_DEBUG(" port:%d %s go high", port, name);
            } else {
                SFF_IO_DEBUG(" port:%d %s go low", port, name);
            }
        }
        st[port] = val;

        //SFF_IO_DEBUG("%s data:%d ch:%d addr:0x%x st[%d]:%d\n", name, data, ioexp->ch, ioexp->addr, port, val);
        ioexp_port++;
    }

    return 0;

}
int ioexp_st_update_fail(int data,
                         struct ioexp_data_t *ioexp,
                         struct ioexp_obj_t *obj)
{
    int port = 0;
    int ioexp_port = 0;
    int bit = 0;
    int bit_min = 0;
    int bit_max = 0;
    int err_code = data; /*data is error code*/
    int *ioPort_2_port = NULL;
    int *st = NULL;
    u8 *name = NULL;

    if (!obj) {

        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    ioPort_2_port = obj->func->ioPort_2_port;
    st = obj->st;
    name = obj->name;
    //SFF_IO_DEBUG("port:%d", port);

    ioexp_port = ioexp->port_min;
    bit_min = ioexp->bit_min;
    bit_max = ioexp->bit_max;

    for (bit = bit_min; bit <= bit_max; bit++) {

        port = ioPort_2_port[ioexp_port];
        st[port] = err_code;
        SFF_IO_ERR(" port:%d %s err_code:%d\n", port, name, err_code);
        ioexp_port++;
    }

    return 0;

}

int qsfp_ioexp_int_handler(struct sff_io_t *self)
{
    int idx = 0;
    int input_change;
    int val = 0;
    int ret = 0;
    int size = 0;
    int *input_change_table =  NULL;
    struct ioexp_data_t *io_prsL = NULL;
    struct ioexp_data_t *io_intL = NULL;
    struct ioexp_obj_t *obj_prsL =  &ioexp_obj[IO_PRESENT];
    struct ioexp_obj_t *obj_intL =  &ioexp_obj[IO_INTL];
    val = gpio_get_value(cpldIo.int_gpio);

    if (!self) {

        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    if (!(self->input_table)) {

        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    size = self->input_table->size;
    input_change_table = self->input_table->table;

    if (val == 0) {
        SFF_IO_DEBUG("1.gpio level: %d\n",val);
        if ((ret = cpldIo.ioexp_int_status_get(&cpldIo, &input_change)) < 0 ) {
            SFF_IO_ERR("read fail\n");
            return ret;
        }
        SFF_IO_DEBUG("input_change:0x%x\n", input_change);
        for (idx = 0; idx < size; idx++) {

            if (input_change & (1 << idx)) {
                /*prsL scan*/
                io_prsL = find_ioexp_by_port(obj_prsL->func->map, input_change_table[idx]);
                io_intL = find_ioexp_by_ch_addr(obj_intL->func->map, io_prsL->ch, io_prsL->addr);

                if (!io_prsL ||
                    !io_intL) {

                    SFF_IO_ERR("NULL ptr\n");
                    return -1;
                }

                ret = pca95xx_reg_read(io_prsL->ch, io_prsL->addr, PCA95XX_INPUT);
                if (ret < 0) {
                    ioexp_st_update_fail(ret, io_prsL, obj_prsL);
                    ioexp_st_update_fail(ret, io_intL, obj_intL);
                    return ret;
                }
                ioexp_st_update(ret, io_prsL, obj_prsL);
                ioexp_st_update(ret, io_intL, obj_intL);
            }
        }
    }
    return 0;
}
int qsfp_ioexp_polling(struct sff_io_t *self)
{
    int idx = 0;
    int ret = 0;
    int size = 0;
    int *input_change_table =  NULL;
    struct ioexp_data_t *io_prsL = NULL;
    struct ioexp_data_t *io_intL = NULL;
    struct ioexp_obj_t *obj_prsL =  &ioexp_obj[IO_PRESENT];
    struct ioexp_obj_t *obj_intL =  &ioexp_obj[IO_INTL];

    //SFF_IO_DEBUG("start\n");
    if (!self) {

        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    if (!self->input_table) {

        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    size = self->input_table->size;
    input_change_table = self->input_table->table;

    for (idx = 0; idx < size; idx++) {

        /*prsL scan*/
        io_prsL = find_ioexp_by_port(obj_prsL->func->map, input_change_table[idx]);
        io_intL = find_ioexp_by_ch_addr(obj_intL->func->map, io_prsL->ch, io_prsL->addr);

        if (!io_prsL ||
            !io_intL) {

            SFF_IO_ERR("NULL ptr\n");
            return -1;
        }

        ret = pca95xx_reg_read(io_prsL->ch, io_prsL->addr, PCA95XX_INPUT);
        if (ret < 0) {
            /*prsL scan*/
            ioexp_st_update_fail(ret, io_prsL, obj_prsL);
            /*intL scan*/
            ioexp_st_update_fail(ret, io_intL, obj_intL);
            return ret;
        }
        /*prsL scan*/
        ioexp_st_update(ret, io_prsL, obj_prsL);
        /*intL scan*/
        ioexp_st_update(ret, io_intL, obj_intL);

    }

    return 0;
}
int  sfp_qsfp_ioexp_polling(struct sff_io_t *self)
{
    int idx = 0;
    int ret = 0;
    int size = 0;
    int *input_change_table =  NULL;
    struct ioexp_data_t *ioexp[4];
    struct ioexp_obj_t *obj[4];

    obj[IO_PRESENT] = &ioexp_obj[IO_PRESENT];
    obj[IO_RX_LOS] = &ioexp_obj[IO_RX_LOS];
    obj[IO_TX_FAULT] = &ioexp_obj[IO_TX_FAULT];
    obj[IO_INTL] = &ioexp_obj[IO_INTL];

    if (!self) {
        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    if (!(self->input_table)) {
        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    size = self->input_table->size;
    input_change_table = self->input_table->table;
    /*sfp*/
    for (idx = 0; idx < size -1; idx++) {

        ioexp[IO_PRESENT] = find_ioexp_by_port(obj[IO_PRESENT]->func->map, input_change_table[idx]);
        ioexp[IO_RX_LOS] = find_ioexp_by_ch_addr(obj[IO_RX_LOS]->func->map, ioexp[IO_PRESENT]->ch, ioexp[IO_PRESENT]->addr);
        ioexp[IO_TX_FAULT] = find_ioexp_by_ch_addr(obj[IO_TX_FAULT]->func->map, ioexp[IO_PRESENT]->ch, ioexp[IO_PRESENT]->addr);

        if (!(ioexp[IO_PRESENT]) ||
            !(ioexp[IO_RX_LOS]) ||
            !(ioexp[IO_TX_FAULT])) {

            SFF_IO_ERR("NULL ptr\n");
            return -1;
        }
        ret = pca95xx_reg_read(ioexp[IO_PRESENT]->ch, ioexp[IO_PRESENT]->addr, PCA95XX_INPUT);
        if (ret < 0) {
            ioexp_st_update_fail(ret, ioexp[IO_PRESENT], obj[IO_PRESENT]);
            ioexp_st_update_fail(ret, ioexp[IO_RX_LOS], obj[IO_RX_LOS]);
            ioexp_st_update_fail(ret, ioexp[IO_TX_FAULT], obj[IO_TX_FAULT]);
            return ret;
        }
        ioexp_st_update(ret, ioexp[IO_PRESENT],obj[IO_PRESENT]);
        ioexp_st_update(ret, ioexp[IO_RX_LOS], obj[IO_RX_LOS]);
        ioexp_st_update(ret, ioexp[IO_TX_FAULT], obj[IO_TX_FAULT]);
    }
    /*qsfp*/
    ioexp[IO_PRESENT] = find_ioexp_by_port(obj[IO_PRESENT]->func->map, input_change_table[idx]);
    ioexp[IO_INTL] = find_ioexp_by_ch_addr(obj[IO_INTL]->func->map, ioexp[IO_PRESENT]->ch, ioexp[IO_PRESENT]->addr);

    if (!(ioexp[IO_PRESENT]) ||
        !(ioexp[IO_INTL])) {

        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }

    ret = pca95xx_reg_read(ioexp[IO_PRESENT]->ch, ioexp[IO_PRESENT]->addr, PCA95XX_INPUT);
    if (ret < 0) {
        ioexp_st_update_fail(ret, ioexp[IO_PRESENT], obj[IO_PRESENT]);
        ioexp_st_update_fail(ret, ioexp[IO_INTL], obj[IO_INTL]);
        return ret;
    }
    ioexp_st_update(ret, ioexp[IO_PRESENT], obj[IO_PRESENT]);
    ioexp_st_update(ret, ioexp[IO_INTL], obj[IO_INTL]);
    return 0;
}
int sfp_qsfp_ioexp_int_handler(struct sff_io_t *self)
{
    int idx = 0;
    int input_change;
    int val = 0;
    int ret = 0;
    int size = 0;
    int *input_change_table =  NULL;
    struct ioexp_data_t *ioexp[4];
    struct ioexp_obj_t *obj[4];
    val = gpio_get_value(cpldIo.int_gpio);

    obj[IO_PRESENT] = &ioexp_obj[IO_PRESENT];
    obj[IO_RX_LOS] = &ioexp_obj[IO_RX_LOS];
    obj[IO_TX_FAULT] = &ioexp_obj[IO_TX_FAULT];
    obj[IO_INTL] = &ioexp_obj[IO_INTL];
    if (!(self)) {

        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    if (!(self->input_table)) {

        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    size = self->input_table->size;
    input_change_table = self->input_table->table;

    if (val == 0) {
        SFF_IO_DEBUG("1.gpio level: %d\n",val);
        if ((ret = cpldIo.ioexp_int_status_get(&cpldIo, &input_change)) < 0 ) {
            SFF_IO_ERR("read fail\n");
            return ret;
        }
        SFF_IO_DEBUG("input_change:0x%x\n", input_change);
        for (idx = 0; idx < size -1; idx++) {

            if (input_change & (1 << idx)) {
                /*prsL scan*/
                ioexp[IO_PRESENT] = find_ioexp_by_port(obj[IO_PRESENT]->func->map, input_change_table[idx]);
                ioexp[IO_RX_LOS] = find_ioexp_by_ch_addr(obj[IO_RX_LOS]->func->map, ioexp[IO_PRESENT]->ch, ioexp[IO_PRESENT]->addr);
                ioexp[IO_TX_FAULT] = find_ioexp_by_ch_addr(obj[IO_TX_FAULT]->func->map, ioexp[IO_PRESENT]->ch, ioexp[IO_PRESENT]->addr);

                if (!(ioexp[IO_PRESENT]) ||
                    !(ioexp[IO_RX_LOS]) ||
                    !(ioexp[IO_TX_FAULT])) {

                    SFF_IO_ERR("NULL ptr\n");
                    return -1;
                }
                ret = pca95xx_reg_read(ioexp[IO_PRESENT]->ch, ioexp[IO_PRESENT]->addr, PCA95XX_INPUT);
                if (ret < 0) {
                    ioexp_st_update_fail(ret, ioexp[IO_PRESENT], obj[IO_PRESENT]);
                    ioexp_st_update_fail(ret, ioexp[IO_RX_LOS], obj[IO_RX_LOS]);
                    ioexp_st_update_fail(ret, ioexp[IO_TX_FAULT], obj[IO_TX_FAULT]);
                    return ret;
                }
                ioexp_st_update(ret, ioexp[IO_PRESENT],obj[IO_PRESENT]);
                ioexp_st_update(ret, ioexp[IO_RX_LOS], obj[IO_RX_LOS]);
                ioexp_st_update(ret, ioexp[IO_TX_FAULT], obj[IO_TX_FAULT]);
            }
        }

        if (input_change & (1 << idx)) {
            ioexp[IO_PRESENT] = find_ioexp_by_port(obj[IO_PRESENT]->func->map, input_change_table[idx]);
            ioexp[IO_INTL] = find_ioexp_by_ch_addr(obj[IO_INTL]->func->map, ioexp[IO_PRESENT]->ch, ioexp[IO_PRESENT]->addr);

            if (!(ioexp[IO_PRESENT]) ||
                !(ioexp[IO_INTL])) {

                SFF_IO_ERR("NULL ptr\n");
                return -1;
            }

            ret = pca95xx_reg_read(ioexp[IO_PRESENT]->ch, ioexp[IO_PRESENT]->addr, PCA95XX_INPUT);
            if (ret < 0) {
                ioexp_st_update_fail(ret, ioexp[IO_PRESENT], obj[IO_PRESENT]);
                ioexp_st_update_fail(ret, ioexp[IO_INTL], obj[IO_INTL]);
                return ret;
            }
            ioexp_st_update(ret, ioexp[IO_PRESENT], obj[IO_PRESENT]);
            ioexp_st_update(ret, ioexp[IO_INTL], obj[IO_INTL]);
        }
    }
    return 0;
}
int ioexp_input_handler(void)
{
    if (SffIo.input_handler) {
        return SffIo.input_handler(&SffIo);
    }

    SFF_IO_ERR("NULL func ptr\n");
    return 0;
}
int ioexp_input_init(void)
{
    int count = 0;
    int ret = 0;
    int i = 0;
    struct ioexp_data_t *ioexp_input = NULL;
    struct ioexp_obj_t *obj[4];
    struct ioexp_data_t *ioexp = NULL;
    /*fist prsL scan*/

    /*assign io_obj input*/
    obj[0] = &ioexp_obj[IO_PRESENT];
    obj[1] = &ioexp_obj[IO_RX_LOS];
    obj[2] = &ioexp_obj[IO_TX_FAULT];
    obj[3] = &ioexp_obj[IO_INTL];

    for (i = 0; i < 4; i++) {
        if (!obj[i]->func->map) {
            SFF_IO_DEBUG("%s not support\n", obj[i]->name);
            continue;
        }
        ioexp_input = obj[i]->func->map;
        for (count = 0; ioexp_input[count].ch != END_OF_TABLE; count++) {
            ioexp = &ioexp_input[count];
            if (!(ioexp)) {
                SFF_IO_ERR("NULL ptr\n");
                return -1;
            }

            ret = pca95xx_reg_read(ioexp->ch, ioexp->addr, PCA95XX_INPUT);
            SFF_IO_DEBUG("read word ok: idx: %d ch %d\n", i, ioexp->ch);
            if (ret < 0) {
                SFF_IO_ERR("read word fail: idx: %d ch %d\n", i, ioexp->ch);
                ioexp_st_update_fail(ret, ioexp, obj[i]);
                return ret;
            }

            ret = ioexp_st_update(ret, ioexp, obj[i]);
            if (ret < 0) {
                return ret;
            }
        }
    }

    return 0;
}
int _ioexp_output_config(struct ioexp_data_t *ioexp, int default_val, int *data)
{

    int ret = 0;
    int bit = 0;
    int bit_min = 0;
    int bit_max = 0;
    bit_min = ioexp->bit_min;
    bit_max = ioexp->bit_max;

    ret = *data;
    if (default_val) {
        for (bit = bit_min; bit <= bit_max; bit++) {
            ret |= (1 << bit);
        }
    } else {
        for (bit = bit_min; bit <= bit_max; bit++) {
            ret &= (~(1 << bit));
        }

    }
    //SFF_IO_DEBUG("bit_min %d ch:%d addr:0x%x ret:%d\n", bit_min, ioexp->ch, ioexp->addr, ret);
    *data = ret;
    return 0;
}
int ioexp_output_init(int io_no_init)
{

    int def_val[4];
    int count = 0;
    int ret = 0;
    int data = 0;
    int i = 0;
    struct ioexp_data_t *ioexp_output = NULL;
    struct ioexp_obj_t *obj[4];
    struct ioexp_data_t *ioexp = NULL;

    /*assign io_obj output*/
    obj[0] = &ioexp_obj[IO_LPMODE];
    obj[1] = &ioexp_obj[IO_RESET];
    obj[2] = &ioexp_obj[IO_TXDISABLE];
    obj[3] = &ioexp_obj[IO_MODE_SEL];
    /*assign io_obj output deafult value*/

    def_val[0] = IO_LPMODE_DEFAULT;
    def_val[1] = IO_RESET_DEFAULT;
    def_val[2] = IO_TXDISABLE_DEFAULT;
    def_val[3] = IO_MODESEL_DEFAULT;

    /*assign ioexp out*/
    for (i = 0; i < 4; i++) {
        if (!obj[i]->func->map) {
            SFF_IO_DEBUG("%s not support\n", obj[i]->name);
            continue;
        }
        ioexp_output = obj[i]->func->map;
        for (count = 0; ioexp_output[count].ch != END_OF_TABLE; count++) {
            ioexp = &ioexp_output[count];
            ret = pca95xx_reg_read(ioexp->ch, ioexp->addr, PCA95XX_OUTPUT);
            if (ret < 0) {
                SFF_IO_ERR("read word fail: idx: %d ch %d\n", i, ioexp->ch);
                ioexp_st_update_fail(ret, ioexp, obj[i]);
                return ret;
            }
            data = ret;
            if (!io_no_init) {
                _ioexp_output_config(ioexp, def_val[i], &data);
                ret = pca95xx_reg_write(ioexp->ch, ioexp->addr, PCA95XX_OUTPUT, data);
                if (ret < 0) {
                    SFF_IO_ERR("write word fail: idx: %d ch %d\n", i, ioexp->ch);
                    ioexp_st_update_fail(ret, ioexp, obj[i]);
                    return ret;
                }
            }

            ret = ioexp_st_update(data, ioexp, obj[i]);
            if (ret < 0) {
                return -1;
            }
        }
    }
    return 0;
}

int cpld_io_int_gpio_init(int gpio_no)
{
    int result;
    int value = 0x0;

    /*cpld ioexp int gpio init*/
    result = gpio_is_valid(gpio_no);
    if (result < 0) {
        SFF_IO_ERR("valid gpio:%d ret:%d\n", gpio_no, result);
        return result;
    }
/*don't set direction since , it's decided at bios stage*/
#if 0
    value = gpio_direction_input(gpio_no);
    if (value < 0 ) {
        SFF_IO_ERR("gpio:%d dir set. err code:%d\n", gpio_no, value);
        return value;
    }
#endif    
    value = gpio_get_value(gpio_no);
    SFF_IO_DEBUG("ok gpio:%d value:%d\n", gpio_no, value);

    return 0;
}
static int _cpld_io_init(struct cpld_io_t *self)
{
    if (cpld_client_init(self) < 0) {
        goto exit_err;
    }
    if (cpld_io_int_gpio_init(self->int_gpio) < 0) {
        goto exit_kfree_client;
    }
    /*enable cpld ioexp function*/
    if(self->ioexp_int_enable(self) < 0) {
        goto exit_kfree_client;
    }

    return 0;

exit_kfree_client:
    cpld_client_deinit(self);
exit_err:
    return -EBADRQC;
}
static void cpld_deinit(void)
{
    cpldIo.deinit(&cpldIo);
}

static int cpld_init(void)
{
    return cpldIo.init(&cpldIo);
}
#if 0
int ioexp_func_map_load(struct ioexp_obj_t obj[], struct ioexp_func_map_t map[])
{
    int i = 0;
    int j = 0;

    for (i = IO_PRESENT; i < IO_IDX_NUM; i++) {

        for (j = 0; map[j].idx != IO_IDX_NUM; j++) {

            if (i == map[j].idx) {

                obj[i].func = map[j].func;
                break;
            }
        }
        if (map[j].idx == IO_IDX_NUM) {

            SFF_IO_ERR("cant find func idx:%d\n", i);
            return -1;
        }
    }

    SFF_IO_DEBUG("OK\n");
    return 0;
}
#endif

int ioexp_func_map_load(struct ioexp_obj_t obj[], struct ioexp_func_t map[])
{
    int i = 0;

    for (i = IO_PRESENT; i < IO_IDX_NUM; i++) {

        obj[i].func = &map[i];
    }

    SFF_IO_DEBUG("OK\n");
    return 0;
}

int ioexp_funcs_init(void)
{
    int port_num = port_num_get();
    int i = 0;

    for (i = IO_PRESENT; i < IO_IDX_NUM; i++) {

        ioexp_obj[i].st = kzalloc(sizeof(int) *port_num, GFP_KERNEL);
        if (!(ioexp_obj[i].st)) {
            goto exit_kfree_st;
        }
    }

    ioexp_obj[IO_PRESENT].name = "io_prsL";
    ioexp_obj[IO_RX_LOS].name = "io_rxlos";
    ioexp_obj[IO_TX_FAULT].name = "io_txfault";
    ioexp_obj[IO_INTL].name = "io_intL";
    ioexp_obj[IO_RESET].name = "io_reset";
    ioexp_obj[IO_LPMODE].name = "io_lpmode";
    ioexp_obj[IO_MODE_SEL].name = "io_modesel";
    ioexp_obj[IO_TXDISABLE].name = "io_txdisable";
    return 0;

exit_kfree_st:

    for (i = IO_PRESENT; i < IO_IDX_NUM; i++) {

        if (ioexp_obj[i].st) {
            kfree(ioexp_obj[i].st);
            ioexp_obj[i].st = NULL;
        }
    }
    return -1;
}

void ioexp_funcs_deinit(void)
{
    int i = 0;
    for (i = IO_PRESENT; i < IO_IDX_NUM; i++) {

        if (ioexp_obj[i].st) {
            kfree(ioexp_obj[i].st);
            ioexp_obj[i].st = NULL;
        }
    }

}
static struct platform_io_info_t *platform_io_info_load(int platform_name)
{

    int i = 0;
    for (i = 0; platform_io_info_tbl[i].platform_name != PLATFORM_END; i++) {
        if (platform_io_info_tbl[i].platform_name == platform_name) {

            return &platform_io_info_tbl[i];
        }

    }
    return NULL;

}
static int table_load(void)
{
    struct platform_io_info_t *io_info = NULL;
    int platform_name = PLATFORM_NAME;
    io_info = platform_io_info_load(platform_name);
    if (!io_info) {
        return -1;
    }
    pca95xx_type_tbl = io_info->pca95xx_type_tbl;
    io_port_num = io_info->io_port_num;
    io_i2c_tbl = io_info->io_i2c_tbl;
    SffIo.input_table = io_info->input_change;
    ioexp_config = io_info->io_config;
    if (ioexp_func_map_load(ioexp_obj, io_info->func_map) < 0) {

        return -1;
    }
    mux_reset_gpio = io_info->mux_reset_gpio;
    if (io_info->cpld_io_info) { /*ioexp interrupt mode supported*/

        if (IO_SFP_QSFP_TYPE == io_info->io_sff_type) {
            SffIo.input_handler =  sfp_qsfp_ioexp_int_handler;
        } else {
            SffIo.input_handler =  qsfp_ioexp_int_handler;
        }
        /*initialize cpld io structure*/
        cpldIo.ioexp_int_status_offset = io_info->cpld_io_info->io_int_status_reg;
        cpldIo.ioexp_int_enable_offset = io_info->cpld_io_info->io_int_enable_reg;
        cpldIo.int_gpio = io_info->cpld_io_info->int_gpio;
        cpldIo.io_ch = io_info->cpld_io_info->io_ch;
        cpldIo.config = io_info->cpld_io_info->config;

        cpldIo.init = _cpld_io_init;
        cpldIo.deinit = cpld_client_deinit;
        cpldIo.ioexp_int_enable = cpld_ioexp_int_enable;

        if (IO_QSFP_TYPE == io_info->io_sff_type) {

            if (platform_name == PLATFORM_BANYAN) {

                cpldIo.ioexp_int_status_get = cpld_ioexp_int_status_get_type2;
            } else if (platform_name == PLATFORM_CEDAR) {

                cpldIo.ioexp_int_status_get = cpld_ioexp_int_status_get_type3;
            }
        } else {

            cpldIo.ioexp_int_status_get = cpld_ioexp_int_status_get_type1;
        }

    } else {

        if (IO_QSFP_TYPE == io_info->io_sff_type) {
            SffIo.input_handler = qsfp_ioexp_polling;
        } else {

            SffIo.input_handler = sfp_qsfp_ioexp_polling;
        }
        cpldIo.init = cpld_ioexp_no_support;
        cpldIo.deinit = cpld_ioexp_no_support;
        cpldIo.ioexp_int_enable = cpld_ioexp_no_support;
    }
    return 0;
}

int sff_io_init(int io_no_init)
{
    if (table_load() < 0) {

        SFF_IO_ERR("sff_io_table load fail\n");
        goto exit_err;
    }
    if(sff_io_mux_init() < 0) {
        SFF_IO_ERR("sff_io_mux_init fail\n");
    }
    if (cpld_init() < 0) {
        SFF_IO_ERR("cpld_io_init fail\n");
        goto exit_err;
    }

    if(sff_io_client_create_all() < 0) {
        goto exit_err;
    }

    if (!io_no_init) {
        if (ioexp_config_init() < 0) {

            SFF_IO_ERR("ioexp_config_init fail\n");
            goto exit_kfree_client;
        }
    }

    if (ioexp_funcs_init() < 0) {
        SFF_IO_ERR("ioexp_funcs_init fail\n");
        goto exit_kfree_client;
    }

    if (ioexp_input_init() < 0) {

        SFF_IO_ERR("ioexp_input_init fail\n");
        goto exit_kfree_client;
    }
    if (ioexp_output_init(io_no_init) < 0) {
        SFF_IO_ERR("ioexp_out_init fail\n");
        goto exit_kfree_client;
    }
    SFF_IO_DEBUG("OK\n");
    return 0;
exit_kfree_client:
    sff_io_client_destroy_all();
exit_err:
    return -EBADRQC;
}
EXPORT_SYMBOL(sff_io_init);

void sff_io_deinit(void)
{
    ioexp_funcs_deinit();
    cpld_deinit();
    sff_io_client_destroy_all();
    SFF_IO_DEBUG("OK\n");
}
EXPORT_SYMBOL(sff_io_deinit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alang <huang.alang@inventec.com>");
MODULE_AUTHOR("Alang huang <@inventec.com>");




