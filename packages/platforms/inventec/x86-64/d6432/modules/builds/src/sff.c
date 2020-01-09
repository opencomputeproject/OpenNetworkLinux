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
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/delay.h>

#include "inv_def.h"
//#include "sff_spec.h"
#include "sff.h"
#include "port_info/port_info.h"
#include "sff_io.h"
#include "sff_eeprom.h"
#include "sfp.h"
#include "qsfp.h"
#include "qsfp_dd.h"

int io_no_init = 0;
module_param(io_no_init, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
unsigned long io_no_init_done_bitmap = 0;

//static int scan_100ms = 0;
bool int_flag_monitor_en = false;
#define HEALTH_SCAN_TIME (10)
static char work_buf[BUF_SIZE];

struct port_info_table_t *Port_Info_Tbl = NULL;
static void sff_polling_task(struct work_struct *work);
static DECLARE_DELAYED_WORK(sff_polling, sff_polling_task);
static u8 sff_polling_enabled = 1;
#if 0
static int i2c_crush_handler(void);
static void _isolated_transvr_report(void);
static int transvr_health_monitor(void);
static void bad_transvr_detect(void);
static bool transvr_is_i2c_ready(void);
static int transvr_port = 0;
#endif
static int sff_polling_task_start(void);
static int sff_polling_task_stop(void);
static struct sff_kobj_t *sff_kobj_add(char *name,
                                       struct kobject *parent,
                                       struct attribute_group *attr_group);

static void io_no_init_done_bitmap_set(struct sff_obj_t *sff_obj);
static void io_no_init_handler(struct sff_mgr_t *sff);
static int io_no_init_done_fsm_run(struct sff_obj_t *sff_obj);
#define MASK_SET(port) ((1L << (port)) - 1L)
/*static int io_mux_reset_all_seq(void);*/

static struct sff_driver_t sffDrv = {
    .eeprom_read = sff_eeprom_read,
    .eeprom_write = sff_eeprom_write,
    .prsL_get = sff_io_prsL_get,
    .prsL_all_get = sff_io_prsL_all_get,
    .reset_set = sff_io_reset_set,
    .reset_get = sff_io_reset_get,
    .lpmode_set = sff_io_lpmode_set,
    .lpmode_get = sff_io_lpmode_get,
    .tx_disable_set = sff_io_tx_disable_set,
    .tx_disable_get = sff_io_tx_disable_get,
    .io_mux_reset_all = sff_io_mux_reset_all,
    .mode_sel_set = sff_io_mode_sel_set,
    .mode_sel_get = sff_io_mode_sel_get,
    .intL_get = sff_io_intL_get,
    .rx_los_get = sff_io_rx_los_get,
    .tx_fault_get = sff_io_tx_fault_get,
};
static inline void sff_polling_set(u8 enable)
{
    sff_polling_enabled = enable;
}

static inline u8 sff_polling_is_enabled(void)
{
    return sff_polling_enabled;
}
struct fsm_period_t {
    sff_fsm_state_t st;
    int delay_cnt;
};
struct fsm_period_t fsm_period_tbl[] = {
    {SFF_FSM_ST_REMOVED, 0},
    {SFF_FSM_ST_INSERTED, 3},
    {SFF_FSM_ST_DETECTING, 1},
    {SFF_FSM_ST_INIT, 1},
    {SFF_FSM_ST_DETECTED, 1},
    {SFF_FSM_ST_IDLE, 20},
    {SFF_FSM_ST_FAULT, 30},
    {SFF_FSM_ST_RESET_ASSERTED, 1},
    {SFF_FSM_ST_RESET_DEASSERTED, 1},
    {SFF_FSM_ST_IDENTIFY, 1},
    {SFF_FSM_ST_MONITOR, 30},
    /*qsfp-dd only {*/
    {SFF_FSM_ST_MGMT_INIT, 3},
    {SFF_FSM_ST_MODULE_HW_INIT, 3},
    {SFF_FSM_ST_MODULE_LOOPBACK_INIT, 3},
    {SFF_FSM_ST_MODULE_READY, 3},
    {SFF_FSM_ST_MODULE_PWR_DOWN, 3},
    {SFF_FSM_ST_MODULE_CMIS_VER_CHECK, 1},
    {SFF_FSM_ST_MODULE_ADVERT_CHECK, 3},
    {SFF_FSM_ST_MODULE_SW_CONFIG, 3},
    {SFF_FSM_ST_MODULE_SW_CONFIG_CHECK, 3},
    {SFF_FSM_ST_MODULE_SW_CONTROL, 3},
    {SFF_FSM_ST_MODULE_READY_CHECK, 3},
    /*qsfp-dd only }*/
    {SFF_FSM_ST_UNKNOWN_TYPE, 30},
    {SFF_FSM_ST_END, 0xff}, /*keep it at the end of table*/
};

const char *sff_fsm_st_str[SFF_FSM_ST_NUM] = {
    "SFF_FSM_ST_REMOVED",
    "SFF_FSM_ST_INSERTED",
    "SFF_FSM_ST_DETECTING",
    "SFF_FSM_ST_INIT",
    "SFF_FSM_ST_DETECTED",
    "SFF_FSM_ST_IDLE",
    "SFF_FSM_ST_FAULT",
    "SFF_FSM_ST_RESET_ASSERTED",
    "SFF_FSM_ST_RESET_DEASSERTED",
    "SFF_FSM_ST_ISOLATED",
    "SFF_FSM_ST_IDENTIFY",
    "SFF_FSM_ST_MONITOR",
    /*qsfp-dd only {*/
    "SFF_FSM_ST_MGMT_INIT",
    "SFF_FSM_ST_MODULE_HW_INIT",
    "SFF_FSM_ST_MODULE_LOOPBACK_INIT",
    "SFF_FSM_ST_MODULE_READY",
    "SFF_FSM_ST_MODULE_PWR_DOWN",
    "SFF_FSM_ST_MODULE_CMIS_VER_CHECK",
    "SFF_FSM_ST_MODULE_ADVERT_CHECK",
    "SFF_FSM_ST_MODULE_SW_CONFIG",
    "SFF_FSM_ST_MODULE_SW_CONFIG_CHECK",
    "SFF_FSM_ST_MODULE_SW_CONTROL",
    "SFF_FSM_ST_MODULE_READY_CHECK",
    /*qsfp-dd only }*/
    "SFF_FSM_ST_UNKNOWN_TYPE",
    "SFF_FSM_ST_END", /*keep it at the bottom*/
};
/*struct sff_obj_t;*/

struct kset *Sff_Kset = NULL;


int maxPortNum = 0;
static struct sff_mgr_t sffMgr;
static void sff_fsm_init(struct sff_obj_t *obj, int type);
static void sff_data_init(struct sff_mgr_t *sff, int port_num);
static void func_tbl_init(struct sff_obj_t *obj, int type);

/*struct sff_mgr_t Sff;*/

#define to_sff_kobj(x) container_of(x, struct sff_kobj_t, kobj)


static struct monitor_para_t monitor_para_table[] = {
    /*  refer to sff 8636- ABLE 6-8  CHANNEL MONITORING VALUES (PAGE 00H BYTES 34-81)*/

    {LN_MONITOR_RX_PWR_TYPE, 10000, "mW"},
    {LN_MONITOR_TX_PWR_TYPE, 10000, "mW"},
    {LN_MONITOR_TX_BIAS_TYPE, 500, "mA"},

};

struct monitor_para_t *monitor_para_find(int type)
{
    int idx = 0;
    int size = ARRAY_SIZE(monitor_para_table);
    for (idx = 0; idx < size; idx++) {

        if (type == monitor_para_table[idx].type)
            break;
    }
    if (idx >= size)
        return NULL;

    return &monitor_para_table[idx];
}

static inline bool transvr_is_detected(struct sff_obj_t *sff_obj)
{
    return (sff_obj->fsm.st == SFF_FSM_ST_DETECTED ? (true) : (false));

}
void transvr_type_set(struct sff_obj_t *sff_obj, int type)
{
    sff_obj->transvr_type=type;
}

int transvr_type_get(struct sff_obj_t *sff_obj)
{
    return sff_obj->transvr_type;
}
//static int transvr_is_prsL(struct sff_obj_t *sff_obj);

static int sfp_eeprom_read(struct sff_obj_t *sff_obj, u8 slave_addr, u8 offset, u8 *buf, int len);
static int sfp_eeprom_write(struct sff_obj_t *sff_obj,u8 slave_addr, u8 offset, u8 *buf, int len);
static int qsfp_eeprom_read(struct sff_obj_t *sff_obj, u8 page, u8 offset, u8 *buf, int len);
static int qsfp_eeprom_write(struct sff_obj_t *sff_obj, u8 page, u8 offset, u8 *buf, int len);
static int qsfp_lpmode_set(struct sff_obj_t *sff_obj, u8 value);
static int qsfp_lpmode_get(struct sff_obj_t *sff_obj, u8 *value);
static int qsfp_intL_get(struct sff_obj_t *sff_obj, u8 *value);
static int prsL_get(struct sff_obj_t *sff_obj, u8 *prsL);
static int qsfp_reset_set(struct sff_obj_t *sff_obj, u8 value);
static int qsfp_reset_get(struct sff_obj_t *sff_obj, u8 *value);
static int mode_sel_set(struct sff_obj_t *sff_obj, u8 value);
static int mode_sel_get(struct sff_obj_t *sff_obj, u8 *value);
static int qsfp_eeprom_dump(struct sff_obj_t *sff_obj, u8 *buf);
static int qsfp_page_sel(struct sff_obj_t *sff_obj, int page);
static int qsfp_page_get(struct sff_obj_t *sff_obj, u8 *page);
static int sfp_tx_disable_set(struct sff_obj_t *sff_obj, u8 value);
static int sfp_tx_disable_get(struct sff_obj_t *sff_obj, u8 *value);
static int sfp_rx_los_get(struct sff_obj_t *sff_obj, u8 *value);
static int sfp_tx_fault_get(struct sff_obj_t *sff_obj, u8 *value);

static int dummy_rx_los_get(struct sff_obj_t *sff_obj, u8 *value);
static int dummy_tx_fault_get(struct sff_obj_t *sff_obj, u8 *value);
static int dummy_reset_set(struct sff_obj_t *sff_obj, u8 value);
static int dummy_reset_get(struct sff_obj_t *sff_obj, u8 *value);
static int dummy_lpmode_set(struct sff_obj_t *sff_obj, u8 value);
static int dummy_lpmode_get(struct sff_obj_t *sff_obj, u8 *value);
static int dummy_mode_sel_set(struct sff_obj_t *sff_obj, u8 value);
static int dummy_mode_sel_get(struct sff_obj_t *sff_obj, u8 *value);
static int dummy_intL_get(struct sff_obj_t *sff_obj, u8 *value);
static int dummy_module_st_get(struct sff_obj_t *sff_obj, u8 *st);
static int dummy_eeprom_dump(struct sff_obj_t *sff_obj, u8 *buf);
static int dummy_page_sel(struct sff_obj_t *sff_obj, int page);
static int dummy_page_sel(struct sff_obj_t *sff_obj, int page);
static int dummy_page_get(struct sff_obj_t *sff_obj, u8 *page);
static int dummy_lane_control_set(struct sff_obj_t *sff_obj, int type, u32 value);
static int dummy_lane_control_get(struct sff_obj_t *sff_obj, int type, u32 *value);
static int dummy_lane_status_get(struct sff_obj_t *sff_obj, int type, u8 *st);
/*utility*/
static int sscanf_to_int(const char *buf, int *value);
static bool match(const char *str1, const char *str2);


struct func_tbl_t sfp_func_tbl = {
    .eeprom_read = sfp_eeprom_read,
    .eeprom_write = sfp_eeprom_write,
    .prsL_get = prsL_get,
    .lpmode_set = dummy_lpmode_set,
    .lpmode_get = dummy_lpmode_get,
    .reset_set = dummy_reset_set,
    .reset_get = dummy_reset_get,
    .mode_sel_set = dummy_mode_sel_set,
    .mode_sel_get = dummy_mode_sel_get,
    .intL_get = dummy_intL_get,
    .tx_disable_set = sfp_tx_disable_set,
    .tx_disable_get = sfp_tx_disable_get,
    .rx_los_get = sfp_rx_los_get,
    .tx_fault_get = sfp_tx_fault_get,
    .temperature_get = sfp_temperature_get,
    .voltage_get = sfp_voltage_get,
    .lane_control_set = sfp_lane_control_set,
    .lane_control_get = sfp_lane_control_get,
    .lane_monitor_get = sfp_lane_monitor_get,
    .vendor_info_get = sfp_vendor_info_get,
    .lane_status_get =  sfp_lane_status_get,
    .module_st_get = dummy_module_st_get,
    .id_get = sfp_id_get,
    .eeprom_dump = dummy_eeprom_dump,
    .page_sel = dummy_page_sel,
    .page_get = dummy_page_get,
};
struct func_tbl_t qsfp_func_tbl = {
    .eeprom_read = qsfp_eeprom_read,
    .eeprom_write = qsfp_eeprom_write,
    .prsL_get = prsL_get,
    .lpmode_set = qsfp_lpmode_set,
    .lpmode_get = qsfp_lpmode_get,
    .reset_set = qsfp_reset_set,
    .reset_get = qsfp_reset_get,
    .mode_sel_set = mode_sel_set,
    .mode_sel_get = mode_sel_get,
    .intL_get = qsfp_intL_get,
    .tx_disable_set = qsfp_tx_disable_set,
    .tx_disable_get = qsfp_tx_disable_get,
    .rx_los_get = dummy_rx_los_get,
    .tx_fault_get = dummy_tx_fault_get,
    .temperature_get = qsfp_temperature_get,
    .voltage_get = qsfp_voltage_get,
    .lane_control_set = qsfp_lane_control_set,
    .lane_control_get = qsfp_lane_control_get,
    .lane_monitor_get = qsfp_lane_monitor_get,
    .vendor_info_get = qsfp_vendor_info_get,
    .lane_status_get =  qsfp_lane_status_get,
    .module_st_get = dummy_module_st_get,
    .id_get = qsfp_id_get,
    .eeprom_dump = qsfp_eeprom_dump,
    .page_sel = qsfp_page_sel,
    .page_get = qsfp_page_get,
};
struct func_tbl_t qsfp_dd_func_tbl = {
    .eeprom_read = qsfp_eeprom_read,
    .eeprom_write = qsfp_eeprom_write,
    .prsL_get = prsL_get,
    .lpmode_set = qsfp_lpmode_set,
    .lpmode_get = qsfp_lpmode_get,
    .reset_set = qsfp_reset_set,
    .reset_get = qsfp_reset_get,
    .mode_sel_set = mode_sel_set,
    .mode_sel_get = mode_sel_get,
    .intL_get = qsfp_intL_get,
    .tx_disable_set = qsfp_dd_tx_disable_set,
    .tx_disable_get = qsfp_dd_tx_disable_get,
    .rx_los_get = dummy_rx_los_get,
    .tx_fault_get = dummy_tx_fault_get,
    .temperature_get = qsfp_dd_temperature_get,
    .voltage_get = qsfp_dd_voltage_get,
    .lane_control_set = dummy_lane_control_set,
    .lane_control_get = dummy_lane_control_get,
    .lane_monitor_get = qsfp_dd_lane_monitor_get,
    .vendor_info_get = qsfp_dd_vendor_info_get,
    .lane_status_get =  dummy_lane_status_get,
    .module_st_get = qsfp_dd_module_st_get,
    .id_get = qsfp_dd_id_get,
    .eeprom_dump = qsfp_eeprom_dump,
    .page_sel = qsfp_page_sel,
    .page_get = qsfp_page_get,
};

/*debug*/
static char *str_head[BUF_SIZE];
static u8 debug_read_buf[BUF_SIZE];
struct debug_eeprom_func_t {
    int type;
    int (*read)(struct sff_obj_t *, u8, u8,u8 *, int);
    int (*write)(struct sff_obj_t * , u8, u8,u8 *, int);
};

static struct debug_eeprom_func_t debug_eeprom_func_tbl[] = {
    {SFP_TYPE, sfp_eeprom_read, sfp_eeprom_write},
    {QSFP_TYPE, qsfp_eeprom_read, qsfp_eeprom_write},
    {QSFP_DD_TYPE, qsfp_eeprom_read, qsfp_eeprom_write},
};
struct debug_eeprom_func_t *find_debug_eeprom_func(int type)
{
    int count = 0;
    int size = sizeof(debug_eeprom_func_tbl) / sizeof(debug_eeprom_func_tbl[0]);
    for (count = 0; count < size; count++) {

        if (type == debug_eeprom_func_tbl[count].type) {

            return &debug_eeprom_func_tbl[count];
        }
    }
    return NULL;
}

/*fsm functions declaration*/
static void sff_fsm_delay_cnt_reset(struct sff_obj_t *sff_obj, sff_fsm_state_t st);
static void sff_fsm_cnt_run(struct sff_obj_t *sff_obj);
static bool sff_fsm_delay_cnt_is_hit(struct sff_obj_t *sff_obj);
static int sff_fsm_run(struct sff_mgr_t *sff);

//static int debug_eeprom_access(struct sff_obj_t *sff_obj, int type, const char *str, int len);
#if 0
static inline bool transvr_is_isolated(struct sff_obj_t *sff_obj)
{
    return (SFF_FSM_ST_FAULT == sff_fsm_st_get(sff_obj) ? (true) : (false));
}
#endif
static inline unsigned long prsL_bitmap_get(struct sff_mgr_t *sff)
{
    return sff->prsL;
}
static inline void prsL_bitmap_update(struct sff_mgr_t *sff, unsigned long bitmap)
{
    sff->prsL = bitmap;
}

static int port_num_get(void)
{
    return Port_Info_Tbl->size;
}
static int prsL_get(struct sff_obj_t *sff_obj, u8 *prsL)
{
    return sffDrv.prsL_get(sff_obj->port, prsL);
}
static int sfp_eeprom_read(struct sff_obj_t *sff_obj,
                    u8 slave_addr,
                    u8 offset,
                    u8 *buf,
                    int len)
{
    if (slave_addr != SFF_EEPROM_I2C_ADDR &&
            slave_addr != SFF_DDM_I2C_ADDR) {
        SFF_MGR_ERR("addr out of range:0x%x\n", slave_addr);
        return -EINVAL;
    }

    return sffDrv.eeprom_read(sff_obj->port, slave_addr, offset, buf, len);
}
static int sfp_eeprom_write(struct sff_obj_t *sff_obj,
                     u8 slave_addr,
                     u8 offset,
                     u8 *buf,
                     int len)
{
    if (slave_addr != SFF_EEPROM_I2C_ADDR &&
            slave_addr != SFF_DDM_I2C_ADDR) {
        SFF_MGR_ERR("addr out of range:0x%x\n", slave_addr);
        return -EINVAL;
    }
    return sffDrv.eeprom_write(sff_obj->port, slave_addr, offset, buf, len);
}
static int qsfp_page_get(struct sff_obj_t *sff_obj, u8 *page)
{
    int ret = 0;
    u8 slave_addr = SFF_EEPROM_I2C_ADDR;
    u8 offset_page_sel = 127;
    u8 reg = 0;

    ret = sffDrv.eeprom_read(sff_obj->port, slave_addr, offset_page_sel, &reg, 1);
    if (ret < 0) {
        SFF_MGR_ERR("fail\n");
        return ret;
    }
    *page = reg;
    return 0;
}
static int _page_switch(struct sff_obj_t *sff_obj, u8 page)
{

    int rec = 0;
    u8 slave_addr = SFF_EEPROM_I2C_ADDR;
    u8 offset_page_sel = 127;
    u8 cur_page = 0;

    rec = qsfp_page_get(sff_obj, &cur_page);
    if (rec < 0) {
        return rec;
    }
    if (cur_page == page) {
        return 0;
    }
    rec = sffDrv.eeprom_write(sff_obj->port, slave_addr, offset_page_sel, &page, 1);

    if (rec < 0) {
        SFF_MGR_ERR("switch page fail\n");
        return rec;
    }
    return 0;
}
static int dummy_page_get(struct sff_obj_t *sff_obj, u8 *page)
{
    return -ENOSYS;
}
static int qsfp_page_sel(struct sff_obj_t *sff_obj, int page)
{
    if (page < QSFP_PAGE0 ||  page >= PAGE_NUM) {

        return -EINVAL;
    }

    return _page_switch(sff_obj, page);
}
static int dummy_page_sel(struct sff_obj_t *sff_obj, int page)
{
    return -ENOSYS;
}
#if 0
static int eeprom_dump_test(struct sff_obj_t *sff_obj)
{
    u8 data[EEPROM_SIZE];
    int i = 0;
    int ret = 0;

    SFF_MGR_DEBUG("start\n");
    for (i = 0; i < 128; i++) {
        ret = qsfp_eeprom_read(sff_obj, 0, 0, data, EEPROM_SIZE - 1);
        if (ret < 0) {
            SFF_MGR_ERR("ret:%s read fail\n", ret);
            break;
        }
    }

    SFF_MGR_DEBUG("end\n");

    return 0;
}
#endif
static int qsfp_eeprom_dump(struct sff_obj_t *sff_obj, u8 *buf)
{
    int ret = 0;
    u8 page = 0;
    int i = 0;
    int j = 0;
    int cnt = 0;
    u8 *data = NULL;
    u8 offset = 0;
    int last = EEPROM_SIZE;
    if ((ret = qsfp_page_get(sff_obj, &page)) < 0) {

        return ret;
    }
    if (page < QSFP_PAGE0 ||  page >= PAGE_NUM) {

        return -EBFONT;
    }
    cnt += snprintf(buf+cnt, BUF_SIZE - cnt, "page%d\n", page);
    data = sff_obj->priv_data.qsfp.eeprom_cache[page];
    memset(data, 0, sizeof(EEPROM_SIZE));
    if (QSFP_PAGE0 == page) {

        offset = 0;
        ret = qsfp_eeprom_read(sff_obj, page, 0, data, EEPROM_SIZE);
        /*ret = qsfp_eeprom_read(sff_obj, page, 128, data+128, 64);
        ret = qsfp_eeprom_read(sff_obj, page, 192, data+192, 64);
        */

    } else {

        offset = 128;
        ret = qsfp_eeprom_read(sff_obj, page, offset, data+offset, EEPROM_HALF_SIZE);

    }

    if (ret < 0) {
        SFF_MGR_ERR("ret:%d read fail\n", ret);
        return ret;
    }

#if 0
    for (i = 0; i < 256; i++) {
        SFF_MGR_DEBUG("reg[%d]:%x\n", i, data[i]);
    }
#endif
    /*print out offset*/
#if 1
    cnt += snprintf(buf+cnt, BUF_SIZE - cnt, "    ");
    for (i = 0; i < 16; i++) {
        cnt += snprintf(buf+cnt, BUF_SIZE - cnt, "%01x  ", i);
    }
    cnt += snprintf(buf+cnt, BUF_SIZE - cnt, "\n");
#endif
    for (i = offset, j=0; i < last; i++) {

        j++;

        if (1 == j) {

            cnt += snprintf(buf+cnt, BUF_SIZE - cnt, "%02x: ", i);

        }

        cnt += snprintf(buf+cnt, BUF_SIZE - cnt, "%02x ", data[i]);

        if (16 == j) {

            cnt += snprintf(buf+cnt, BUF_SIZE - cnt, "\n");
            j = 0;
        }

    }
    return 0;
}
static int dummy_eeprom_dump(struct sff_obj_t *sff_obj, u8 *buf)
{
    return -ENOSYS;
}

static int qsfp_eeprom_read(struct sff_obj_t *sff_obj,
                     u8 page,
                     u8 offset,
                     u8 *buf,
                     int len)
{
    int ret = 0;
    u8 slave_addr = SFF_EEPROM_I2C_ADDR;
    if (offset > 127) {
        ret = _page_switch(sff_obj, page);
        if (ret < 0) {
            return ret;
        }
    }
    return sffDrv.eeprom_read(sff_obj->port, slave_addr, offset, buf, len);
}
static int qsfp_eeprom_write(struct sff_obj_t *sff_obj,
                      u8 page,
                      u8 offset,
                      u8 *buf,
                      int len)
{
    int ret = 0;
    u8 slave_addr = SFF_EEPROM_I2C_ADDR; //it's fixed

    if (offset > 127) {
        ret = _page_switch(sff_obj, page);
        if (ret < 0) {
            return ret;
        }
    }
    return sffDrv.eeprom_write(sff_obj->port, slave_addr, offset, buf, len);
}
static bool match(const char *str1, const char *str2)
{
    bool is_match = false;
    if(strcmp(str1, str2) == 0) {
        is_match = true;
    }
    return is_match;
}
#if 1
/*the function is for debugging , access eeprom through command format ex:
 * for sfp
 * set 0x51 offset data
 * get 0x51 offset data_len
 * for qsfp
 * set page offset data
 * get page offset data_len
 */
static int debug_eeprom_access(struct sff_obj_t *sff_obj, int type, const char *str, int len)
{
    char *ptr = NULL;
    int count = 0;
    int i = 0;
    int ret = 0;
    u8 addr_or_page = 0;
    u8 offset = 0;
    u8 data = 0;
    long tmp = 0;
    int data_len = 0;
    int str_len = 0;
    struct debug_eeprom_func_t *pfunc = NULL;
    memset(debug_read_buf, 0, sizeof(debug_read_buf));
    if (len >= BUF_SIZE) {

        SFF_MGR_ERR("input out of range:%d\n", len);
        goto ERR_EXIT;
    }

    str_len = strlen(str);
    memset(work_buf, '\0', sizeof(work_buf));
    memcpy(work_buf, str, str_len);

    SFF_MGR_DEBUG("work_buf:%s\n", work_buf);
    ptr = work_buf;
    str_head[count++] = ptr;

    while (*ptr != '\0') {
        if (' ' == *ptr) {
            *ptr = '\0';
            str_head[count++] = ptr+1;
        }
        ptr++;
    }


    for (i = 0; i < count; i++) {
        SFF_MGR_DEBUG("str_head:%s\n", str_head[i]);
    }

    pfunc = find_debug_eeprom_func(type);
    if (NULL == pfunc) {
        SFF_MGR_ERR("cant find func\n");
        goto ERR_EXIT;
    }
    if (!match(str_head[0], "set") &&
            !match(str_head[0], "get")) {

        SFF_MGR_ERR("invalid cmd:%s\n", str_head[0]);
        goto ERR_EXIT;
    }
    ptr = str_head[0];
    ret = kstrtol(str_head[1], 0, &tmp);
    if (ret < 0) {
        return ret;
    }
    addr_or_page = tmp;
    ret = kstrtol(str_head[2], 0, &tmp);
    if (ret < 0) {
        return ret;
    }
    offset = tmp;

    if (match(ptr, "set")) {
        ret = kstrtol(str_head[3], 0, &tmp);
        if (ret < 0) {
            return ret;
        }
        data = tmp;
        SFF_MGR_DEBUG("addr:%d offset:%d tmp:%d\n", addr_or_page, offset, (int)tmp);
        if(pfunc->write(sff_obj, addr_or_page, offset, &data, 1) < 0) {

            SFF_MGR_ERR("write fail\n");
            goto ERR_EXIT;
        }
    } else if (match(ptr, "get")) {
        ret = kstrtol(str_head[3], 0, &tmp);
        if (ret < 0) {
            return ret;
        }
        data_len = tmp;
        if(pfunc->read(sff_obj, addr_or_page, offset, debug_read_buf, data_len) < 0) {

            SFF_MGR_ERR("read fail\n");
            goto ERR_EXIT;
        }

        for (count = 0; count < data_len; count++) {
            SFF_MGR_DEBUG("read_data[%d]:0x%x \n", count+offset, debug_read_buf[count]);
        }
    }

    return 0;

ERR_EXIT: /*in order to show error log return 0, otherwise will get permission denied*/
    return 0;
}
#endif
static int sff_type_get(struct sff_obj_t *sff_obj)
{
    int type = SFP_TYPE;
    type = sff_obj->type;
    return type;

}
/*sysfs attr*/
/* a custom attribute that works just for a struct sff_kobj. */
struct sff_attribute {
    struct attribute attr;
    ssize_t (*show)(struct sff_kobj_t *sff, struct sff_attribute *attr, char *buf);
    ssize_t (*store)(struct sff_kobj_t *sff, struct sff_attribute *attr, const char *buf, size_t count);
};
#define to_sff_attr(x) container_of(x, struct sff_attribute, attr)

static ssize_t sff_attr_show(struct kobject *kobj, struct attribute *attr,
                             char *buf)
{
    struct sff_attribute *attribute;
    struct sff_kobj_t *obj;

    attribute = to_sff_attr(attr);
    obj = to_sff_kobj(kobj);

    if (!attribute->show) {
        return -EIO;
    }

    return attribute->show(obj, attribute, buf);

}


/*sysfs_ops的store函数实现*/
static ssize_t sff_attr_store(struct kobject *kobj, struct attribute *attr,
                              const char *buf, size_t count)
{
    struct sff_attribute *attribute;
    struct sff_kobj_t *obj;

    attribute = to_sff_attr(attr);
    obj = to_sff_kobj(kobj);

    if (!attribute->store)
        return -ENOSYS;

    return attribute->store(obj, attribute, buf, count);

}

/*struct sysfs_ops变量*/
static struct sysfs_ops sff_sys_ops = {
    .show   = sff_attr_show,
    .store  = sff_attr_store,
};

/**********************************************************/
/*kobj_type的release函数实现*/
void sff_kobj_release(struct kobject *kobj)
{
    struct sff_kobj_t *obj;
    obj = to_sff_kobj(kobj);

    if (obj) {
        SFF_MGR_INFO("%s\n", obj->kobj.name);
        kfree(obj);
    }
}

static struct kobj_type sff_ktype = {
    .release = sff_kobj_release,
    .sysfs_ops = &sff_sys_ops,
    .default_attrs = NULL,
};


static ssize_t sff_page_store(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                              const char *buf, size_t count)
{
    int ret = 0;
    int page = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    ret = sscanf_to_int(buf, &page);
    if(ret < 0) {
        return ret;
    }

    ret = sff_obj->func_tbl->page_sel(sff_obj, page);

    if (ret < 0) {
        return ret;
    }

    return count;
}
static ssize_t sff_eeprom_dump_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                    char *buf)
{
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    ret = sff_obj->func_tbl->eeprom_dump(sff_obj, buf);

    if(ret < 0) {
        return ret;
    }

    return scnprintf(buf, BUF_SIZE, "%s", buf);
}
static ssize_t id_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                       char *buf)
{
    unsigned  char id = 0;
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    ret = sff_obj->func_tbl->id_get(sff_obj, &id);

    if(ret < 0) {
        return ret;
    }

    return scnprintf(buf, BUF_SIZE, "0x%x\n", id);
}
/*qsfp_dd only*/
static ssize_t module_st_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                              char *buf)
{
    u8 val = 0;
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    ret = sff_obj->func_tbl->module_st_get(sff_obj, &val);

    if(ret < 0) {
        return ret;
    }
    return scnprintf(buf, BUF_SIZE, "0x%x\n", val);
}
static ssize_t type_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                         char *buf)
{
    int type = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    type = sff_type_get(sff_obj);
    if (SFP_TYPE == type) {
        return scnprintf(buf, BUF_SIZE, "%s\n", "SFP_TYPE");
    } else if (QSFP_TYPE == type) {
        return scnprintf(buf, BUF_SIZE, "%s\n", "QSFP_TYPE");
    } else {

        return scnprintf(buf, BUF_SIZE, "%s\n", "QSFP_DD_TYPE");

    }

}

static ssize_t swps_version_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                 char *buf)
{
    return scnprintf(buf, BUF_SIZE, "%s\n", SWPS_VERSION);
}
static int
sscanf_to_int(const char *buf, int *value)
{
#if 1
    int   result  = 0;
    char hex_tag[] = "0x";
    char *ptr = NULL;
    ptr = strstr(buf, hex_tag);
    if (ptr) {
        if (strcspn(buf, hex_tag) == 0) { /*first index*/
            if (!sscanf(buf,"%x",&result)) {
                goto exit_err;
            }
        } else {
            goto exit_err;
        }
    } else {
        if (!sscanf(buf,"%d",&result)) {
            goto exit_err;
        }
    }
    *value = result;
    return 0;

exit_err:
    return -EBADRQC;
#else

    int ldata = 0;
    int ret = 0;
    /*use kernel api instead*/
    if (!buf || !value) {
        return -1;
    }
    ret = kstrtol(buf, 16, &ldata);
    if (ret < 0) {

        return ret;
    }
    *value = (int)ldata;
    return 0;

#endif
}
static ssize_t lane_control_store(int ctrl_type, struct sff_kobj_t *sff_kobj,
                                     const char *buf, size_t count)
{
    int ch_ctrl = 0;
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    ret = sscanf_to_int(buf, &ch_ctrl);
    SFF_MGR_DEBUG("ch_ctrl:0x%x ret:%d\n", ch_ctrl, ret);

    if(ret < 0) {
        return ret;
    }
    ret = sff_obj->func_tbl->lane_control_set(sff_obj, ctrl_type, ch_ctrl);

    if(ret < 0) {
        return ret;
    }
    return count;
}
static ssize_t lane_control_show(int ctrl_type, struct sff_kobj_t *sff_kobj, char *buf)
{
    u32 ch_ctrl = 0;
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    ret = sff_obj->func_tbl->lane_control_get(sff_obj, ctrl_type, &ch_ctrl);

    if(ret < 0) {
        return ret;
    }
    return scnprintf(buf, BUF_SIZE, "0x%x\n", ch_ctrl);
}
static ssize_t sff_tx_eq_store(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                               const char *buf, size_t count)
{
    return lane_control_store(TX_EQ_TYPE, kobj, buf, count);

}
static ssize_t sff_rx_em_store(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                               const char *buf, size_t count)
{
    return lane_control_store(RX_EM_TYPE, kobj, buf, count);
}

static ssize_t sff_rx_am_store(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                               const char *buf, size_t count)
{
    return lane_control_store(RX_AM_TYPE, kobj, buf, count);
}

static ssize_t sff_tx_eq_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                              char *buf)
{
    return lane_control_show(TX_EQ_TYPE, kobj, buf);
}
static ssize_t sff_rx_em_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                              char *buf)
{
    return lane_control_show(RX_EM_TYPE, kobj, buf);
}
static ssize_t sff_rx_am_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                              char *buf)
{
    return lane_control_show(RX_AM_TYPE, kobj, buf);
}
static ssize_t lane_status_show(int st_type, struct sff_kobj_t *sff_kobj, char *buf)
{
    u8 ch_status = 0;
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    ret = sff_obj->func_tbl->lane_status_get(sff_obj, st_type, &ch_status);

    if(ret < 0) {
        return ret;
    }
    return scnprintf(buf, BUF_SIZE, "0x%x\n", ch_status);

}
static ssize_t sff_rx_los_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                               char *buf)
{
    return lane_status_show(LN_STATUS_RX_LOS_TYPE, kobj, buf);
}
static ssize_t sff_tx_los_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                               char *buf)
{
    return lane_status_show(LN_STATUS_TX_LOS_TYPE, kobj, buf);
}
static ssize_t sff_tx_fault_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                                 char *buf)
{
    return lane_status_show(LN_STATUS_TX_FAULT_TYPE, kobj, buf);
}
static ssize_t lane_monitor_show(int moni_type, struct sff_kobj_t *sff_kobj, char *buf)
{
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    ret = sff_obj->func_tbl->lane_monitor_get(sff_obj, moni_type, buf, BUF_SIZE);
    if(ret < 0) {
        return ret;
    }
    return scnprintf(buf, BUF_SIZE, "%s", buf);
}
static ssize_t sff_tx_power_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                                 char *buf)
{
    return lane_monitor_show(LN_MONITOR_TX_PWR_TYPE, kobj, buf);
}
static ssize_t sff_rx_power_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                                 char *buf)
{
    return lane_monitor_show(LN_MONITOR_RX_PWR_TYPE, kobj, buf);
}
static ssize_t sff_tx_bias_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                                char *buf)
{
    return lane_monitor_show(LN_MONITOR_TX_BIAS_TYPE, kobj, buf);
}

static ssize_t vendor_info_show(int info_type, struct sff_kobj_t *sff_kobj, char *buf)
{
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    ret = sff_obj->func_tbl->vendor_info_get(sff_obj, info_type, buf, BUF_SIZE);

    if(ret < 0) {
        return ret;
    }
    return scnprintf(buf, BUF_SIZE, "%s", buf);
}
static ssize_t vendor_name_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                                char *buf)
{
    return vendor_info_show(VENDOR_NAME_TYPE, kobj, buf);
}
static ssize_t vendor_pn_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                              char *buf)
{
    return vendor_info_show(VENDOR_PN_TYPE, kobj, buf);
}
static ssize_t vendor_sn_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                              char *buf)
{
    return vendor_info_show(VENDOR_SN_TYPE, kobj, buf);
}
static ssize_t vendor_rev_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                               char *buf)
{
    return vendor_info_show(VENDOR_REV_TYPE, kobj, buf);
}
static ssize_t temperature_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                char *buf)
{
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    ret = sff_obj->func_tbl->temperature_get(sff_obj, buf, BUF_SIZE);
    if (ret < 0) {
        return ret;
    }

    return scnprintf(buf, BUF_SIZE, "%s", buf);
}
static ssize_t voltage_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                            char *buf)
{
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    ret = sff_obj->func_tbl->voltage_get(sff_obj, buf, BUF_SIZE);
    if (ret < 0) {
        return ret;
    }

    return scnprintf(buf, BUF_SIZE, "%s", buf);
}

static ssize_t transvr_type_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                 char *buf)
{
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    return scnprintf(buf, BUF_SIZE, "%d\n", transvr_type_get(sff_obj));
}

static ssize_t transvr_st_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                               char *buf)
{
    int st = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    st = sff_fsm_st_get(sff_obj);
    return scnprintf(buf, BUF_SIZE, "%s\n", sff_fsm_st_str[st]);
}
static ssize_t sff_reset_store(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                               const char *buf, size_t count)
{
    int ret = 0;
    int resetL = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    ret = sscanf_to_int(buf, &resetL);

    if(ret < 0) {
        return ret;
    }

    ret = sff_obj->func_tbl->reset_set(sff_obj, resetL);

    if(ret < 0) {
        return ret;
    }

    return count;
}
static ssize_t sff_reset_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                              char *buf)
{
    u8 reset = 0;
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    ret = sff_obj->func_tbl->reset_get(sff_obj, &reset);

    if (ret < 0) {
        return ret;
    }
    return scnprintf(buf, BUF_SIZE, "%d\n", reset);
}
#if 1  /*<TBD>*/
static ssize_t sff_debug_store(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                               const char *buf, size_t count)
{
    int err_code = -1;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    int type = sff_type_get(sff_obj);
    err_code = debug_eeprom_access(sff_obj, type, buf, count);

    if(err_code < 0) {
        return err_code;
    }
    return count;
}
#endif

static ssize_t sff_debug_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                              char *buf)
{
    int cnt = 0;
    char eeprom_access_str[] = "/*eeprom access*/";
    char sfp_set_str[] = "set 0x50 offset data";
    char sfp_get_str[] = "get 0x50 offset data_len";
    char qsfp_set_str[] = "set page offset data";
    char qsfp_get_str[] = "get page offset data_len";

    cnt += snprintf(buf, BUF_SIZE - cnt, "%s\n%s\n%s\n%s\n%s\n",
                    eeprom_access_str, sfp_set_str, sfp_get_str, qsfp_set_str, qsfp_get_str);
    return cnt;
}
static ssize_t gpio_mux_reset_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                   char *buf)
{
    int val = 0;
    val = sff_io_mux_reset_get();

    return scnprintf(buf, BUF_SIZE, "%d\n", val);
}
static ssize_t gpio_mux_reset_store(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                    const char *buf, size_t count)
{
    int err_code = 0;
    int val = 0;
    err_code = sscanf_to_int(buf, &val);
    if(err_code < 0) {

        SFF_MGR_ERR("1\n");
        return err_code;
    }
    err_code = sffDrv.io_mux_reset_all(val);

    if(err_code < 0) {
        SFF_MGR_ERR("2\n");
        return err_code;
    }

    return count;
}
static ssize_t sff_lpmode_store(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                const char *buf, size_t count)
{
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    int ret = 0;
    int lpmode = 0;

    ret = sscanf_to_int(buf, &lpmode);

    if(ret < 0) {
        return ret;
    }
    ret = sff_obj->func_tbl->lpmode_set(sff_obj, lpmode);

    if(ret < 0) {
        return ret;
    }
    return count;
}
static ssize_t sff_lpmode_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                               char *buf)
{
    u8 lpmode = 0;
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    ret = sff_obj->func_tbl->lpmode_get(sff_obj, &lpmode);
    if (ret < 0) {
        return ret;
    }
    return scnprintf(buf, BUF_SIZE, "%d\n", lpmode);
}
static ssize_t sff_intL_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                             char *buf)
{
    u8 intL = 0;
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    ret = sff_obj->func_tbl->intL_get(sff_obj, &intL);
    if (ret < 0) {
        return ret;
    }

    return scnprintf(buf, BUF_SIZE, "%d\n", intL);
}
static ssize_t sff_mode_sel_store(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                  const char *buf, size_t count)
{
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    int err_code = -1;
    int mode_sel = 0;
    int type = 0;
    type = sff_type_get(sff_obj);

    if (SFP_TYPE == type) {

        return err_code;
    }
    err_code = sscanf_to_int(buf, &mode_sel);

    if(err_code < 0) {
        return err_code;
    }

    err_code = mode_sel_set(sff_obj, mode_sel);

    if(err_code < 0) {
        return err_code;
    }
    return count;
}
static ssize_t sff_mode_sel_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                 char *buf)
{
    u8 mode_sel = 0;
    int err_code = -1;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    int type = sff_type_get(sff_obj);

    if (SFP_TYPE != type) {
        if (mode_sel_get(sff_obj, &mode_sel) < 0) {
            return scnprintf(buf, BUF_SIZE, "%d\n", err_code);
        }
    } else {

        return scnprintf(buf, BUF_SIZE, "%d\n", err_code);
    }

    return scnprintf(buf, BUF_SIZE, "%d\n", mode_sel);
}
static ssize_t sff_tx_disable_store(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                    const char *buf, size_t count)
{
    int ret = 0;
    int tx_disable = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    ret = sscanf_to_int(buf, &tx_disable);

    if(ret < 0) {
        return ret;
    }

    ret = sff_obj->func_tbl->tx_disable_set(sff_obj, tx_disable);

    if(ret < 0) {
        return ret;
    }
    return count;
}
static ssize_t sff_tx_disable_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                   char *buf)
{
    u8 tx_disable = 0;
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    ret = sff_obj->func_tbl->tx_disable_get(sff_obj, &tx_disable);

    if(ret < 0) {
        return ret;
    }


    return scnprintf(buf, BUF_SIZE, "%d\n", tx_disable);
}

static ssize_t sff_prsL_all_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                 char *buf)
{
    int err_code = -1;
    unsigned long bitmap = 0;
    int port = 0;
    int port_num = port_num_get();

    if (sffDrv.prsL_all_get(&bitmap) < 0) {
        //return scnprintf(buf, BUF_SIZE, "%d\n", err_code);
        return err_code;
    }
    for (port = 0; port < port_num; port++) {

        buf[port] = test_bit(port, &bitmap) ? '1' : '0';
    }
    buf[port] = '\n';
    return (ssize_t)strlen(buf);
    //bitmap = ~bitmap;
    //return scnprintf(buf, BUF_SIZE, "0x%lx\n", bitmap);
}
static ssize_t sff_polling_store(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                 const char *buf, size_t count)
{
    int enable = 0;
    int err_code=0;
    err_code = sscanf_to_int(buf, &enable);
    if(err_code < 0)
        return err_code;

    if(enable == sff_polling_is_enabled()) {
        return count;
    }

    if(enable) {
        sff_polling_task_start();
    } else {
        sff_polling_task_stop();
    }
    sff_polling_set(enable);

    return count;
}
static ssize_t sff_polling_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                                char *buf)
{

    return scnprintf(buf, BUF_SIZE, "%d\n", sff_polling_is_enabled());
}

static ssize_t sff_int_flag_monitor_store(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
        const char *buf, size_t count)
{
    int enable = 0;
    int err_code=0;
    err_code = sscanf_to_int(buf, &enable);
    if(err_code < 0)
        return err_code;

    int_flag_monitor_en = ((enable == 1) ? true : false);

    return count;
}
static ssize_t sff_int_flag_monitor_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
        char *buf)
{
    int enable = 0;

    enable = ((int_flag_monitor_en == true) ? 1 : 0);
    return scnprintf(buf, BUF_SIZE, "%d\n", enable);
}

static ssize_t sff_page_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                             char *buf)
{
    int ret = 0;
    u8 page = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    ret = sff_obj->func_tbl->page_get(sff_obj, &page);
    if (ret < 0) {
        return ret;
    }
    return scnprintf(buf, BUF_SIZE, "%d\n", page);
}

static struct sff_attribute sff_type_attr =
    __ATTR(type, S_IRUGO, type_show, NULL);

static struct sff_attribute sff_id_attr =
    __ATTR(id, S_IRUGO, id_show, NULL);

static struct sff_attribute sff_transvr_type_attr =
    __ATTR(transvr_type, S_IRUGO, transvr_type_show, NULL);

static struct sff_attribute sff_transvr_st_attr =
    __ATTR(transvr_st, S_IRUGO, transvr_st_show, NULL);

static struct sff_attribute sff_prsL_all_attr =
    __ATTR(prsL, S_IRUGO, sff_prsL_all_show, NULL);

static struct sff_attribute sff_polling_attr =
    __ATTR(sff_polling, S_IWUSR|S_IRUGO, sff_polling_show, sff_polling_store);

static struct sff_attribute sff_int_flag_monitor_attr =
    __ATTR(sff_int_flag_monitor, S_IWUSR|S_IRUGO, sff_int_flag_monitor_show, sff_int_flag_monitor_store);

static struct sff_attribute sff_reset_attr =
    __ATTR(reset, S_IWUSR|S_IRUGO, sff_reset_show, sff_reset_store);

static struct sff_attribute sff_intL_attr =
    __ATTR(intL, S_IRUGO, sff_intL_show, NULL);

static struct sff_attribute sff_lpmode_attr =
    __ATTR(lpmode, S_IWUSR|S_IRUGO, sff_lpmode_show, sff_lpmode_store);

static struct sff_attribute sff_mode_sel_attr =
    __ATTR(mode_sel, S_IWUSR|S_IRUGO, sff_mode_sel_show, sff_mode_sel_store);

static struct sff_attribute sff_tx_disable_attr =
    __ATTR(tx_disable, S_IWUSR|S_IRUGO, sff_tx_disable_show, sff_tx_disable_store);

static struct sff_attribute sff_rx_los_attr =
    __ATTR(rx_los, S_IRUGO, sff_rx_los_show, NULL);

static struct sff_attribute sff_tx_los_attr =
    __ATTR(tx_los, S_IRUGO, sff_tx_los_show, NULL);

static struct sff_attribute sff_tx_fault_attr =
    __ATTR(tx_fault, S_IRUGO, sff_tx_fault_show, NULL);

static struct sff_attribute sff_tx_eq_attr =
    __ATTR(tx_eq, S_IWUSR|S_IRUGO, sff_tx_eq_show, sff_tx_eq_store);

static struct sff_attribute sff_rx_em_attr =
    __ATTR(rx_em, S_IWUSR|S_IRUGO, sff_rx_em_show, sff_rx_em_store);

static struct sff_attribute sff_rx_am_attr =
    __ATTR(rx_am, S_IWUSR|S_IRUGO, sff_rx_am_show, sff_rx_am_store);

static struct sff_attribute sff_tx_power_attr =
    __ATTR(tx_power, S_IRUGO, sff_tx_power_show, NULL);

static struct sff_attribute sff_rx_power_attr =
    __ATTR(rx_power, S_IRUGO, sff_rx_power_show, NULL);

static struct sff_attribute sff_tx_bias_attr =
    __ATTR(tx_bias, S_IRUGO, sff_tx_bias_show, NULL);

static struct sff_attribute sff_vendor_name_attr =
    __ATTR(vendor_name, S_IRUGO, vendor_name_show, NULL);

static struct sff_attribute sff_vendor_part_number_attr =
    __ATTR(vendor_pn, S_IRUGO, vendor_pn_show, NULL);

static struct sff_attribute sff_vendor_serial_number_attr =
    __ATTR(vendor_sn, S_IRUGO, vendor_sn_show, NULL);

static struct sff_attribute sff_vendor_rev_attr =
    __ATTR(vendor_rev, S_IRUGO, vendor_rev_show, NULL);

static struct sff_attribute sff_temperature_attr =
    __ATTR(temperature, S_IRUGO, temperature_show, NULL);

static struct sff_attribute sff_voltage_attr =
    __ATTR(voltage, S_IRUGO, voltage_show, NULL);

static struct sff_attribute sff_debug_attr =
    __ATTR(debug, S_IWUSR|S_IRUGO, sff_debug_show, sff_debug_store);

static struct sff_attribute sff_eeprom_dump_attr =
    __ATTR(eeprom_dump, S_IRUGO, sff_eeprom_dump_show, NULL);
static struct sff_attribute sff_page_attr =
    __ATTR(page, S_IWUSR|S_IRUGO, sff_page_show, sff_page_store);
static struct sff_attribute sff_module_st_attr =
    __ATTR(module_st, S_IRUGO, module_st_show, NULL);

static struct sff_attribute mux_reset_attr =
    __ATTR(mux_reset, S_IWUSR|S_IRUGO, gpio_mux_reset_show, gpio_mux_reset_store);
static struct sff_attribute swps_version_attr =
    __ATTR(swps_version, S_IRUGO, swps_version_show, NULL);

static struct attribute *sfp_attributes[] = {

    /*io pin attribute*/
    &sff_tx_disable_attr.attr,
    /*eeprom attribute*/
    &sff_type_attr.attr,
    &sff_id_attr.attr,
    &sff_tx_eq_attr.attr,
    &sff_rx_em_attr.attr,
    &sff_rx_am_attr.attr,
    &sff_rx_los_attr.attr,
    &sff_tx_los_attr.attr,
    &sff_tx_fault_attr.attr,
    &sff_tx_power_attr.attr,
    &sff_rx_power_attr.attr,
    &sff_tx_bias_attr.attr,
    &sff_temperature_attr.attr,
    &sff_voltage_attr.attr,
    &sff_vendor_name_attr.attr,
    &sff_vendor_part_number_attr.attr,
    &sff_vendor_serial_number_attr.attr,
    &sff_vendor_rev_attr.attr,
    /*transceiver identified info attribute*/
    &sff_transvr_type_attr.attr,
    &sff_transvr_st_attr.attr,
    &sff_debug_attr.attr,
    //&sff_eeprom_dump_attr.attr,
    NULL

};

static struct attribute_group sfp_group = {
    .attrs = sfp_attributes,
};

static struct attribute *qsfp_attributes[] = {
    /*io pin attribute*/
    &sff_reset_attr.attr,
    &sff_lpmode_attr.attr,
    &sff_mode_sel_attr.attr,
    &sff_intL_attr.attr,
    /*eeprom attribute*/
    &sff_type_attr.attr,
    &sff_id_attr.attr,
    &sff_tx_eq_attr.attr,
    &sff_rx_em_attr.attr,
    &sff_rx_am_attr.attr,
    &sff_rx_los_attr.attr,
    &sff_tx_los_attr.attr,
    &sff_tx_fault_attr.attr,
    &sff_tx_power_attr.attr,
    &sff_rx_power_attr.attr,
    &sff_tx_bias_attr.attr,
    &sff_temperature_attr.attr,
    &sff_voltage_attr.attr,
    &sff_tx_disable_attr.attr,
    &sff_vendor_name_attr.attr,
    &sff_vendor_part_number_attr.attr,
    &sff_vendor_serial_number_attr.attr,
    &sff_vendor_rev_attr.attr,
    /*transceiver identified info attribute*/
    &sff_transvr_type_attr.attr,
    &sff_transvr_st_attr.attr,
    &sff_debug_attr.attr,
    &sff_eeprom_dump_attr.attr,
    &sff_page_attr.attr,
    NULL
};
static struct attribute_group qsfp_group = {
    .attrs = qsfp_attributes,
};
static struct attribute *qsfp_dd_attributes[] = {
    /*io pin attribute*/
    &sff_reset_attr.attr,
    &sff_lpmode_attr.attr,
    &sff_mode_sel_attr.attr,
    &sff_intL_attr.attr,
    /*eeprom attribute*/
    &sff_type_attr.attr,
    &sff_id_attr.attr,
    &sff_rx_los_attr.attr,
    &sff_tx_los_attr.attr,
    &sff_tx_fault_attr.attr,
    &sff_tx_power_attr.attr,
    &sff_rx_power_attr.attr,
    &sff_tx_bias_attr.attr,
    &sff_temperature_attr.attr,
    &sff_voltage_attr.attr,
    &sff_tx_disable_attr.attr,
    &sff_vendor_name_attr.attr,
    &sff_vendor_part_number_attr.attr,
    &sff_vendor_serial_number_attr.attr,
    &sff_vendor_rev_attr.attr,
    /*transceiver identified info attribute*/
    //&sff_transvr_type_attr.attr,
    &sff_transvr_st_attr.attr,
    &sff_debug_attr.attr,
    &sff_eeprom_dump_attr.attr,
    &sff_page_attr.attr,
    //&sff_app_advert_field_dump_attr.attr,
    &sff_module_st_attr.attr,
    NULL
};
static struct attribute_group qsfp_dd_group = {
    .attrs = qsfp_dd_attributes,
};
/*attribute of all sff modules , mainly reprsLed as bitmap form*/
static struct attribute *sff_mgr_attributes[] = {

    &sff_prsL_all_attr.attr,
    &sff_int_flag_monitor_attr.attr,
    NULL
};
static struct attribute_group sff_mgr_group = {
    .attrs = sff_mgr_attributes,
};

static struct attribute *sff_common_attributes[] = {

    &sff_polling_attr.attr,
    &mux_reset_attr.attr,
    &swps_version_attr.attr,
    NULL
};
static struct attribute_group sff_common_group = {
    .attrs = sff_common_attributes,
};

static void sff_objs_destroy(struct sff_mgr_t *sff)
{

    if (sff->obj) {
        kfree(sff->obj);
    }
}

static int sff_polling_task_start(void)
{
    /*<TBD> check what's the right way to reset the fsm state*/
    schedule_delayed_work(&sff_polling, SFF_POLLING_PERIOD);
    return 0;

}
static int sff_polling_task_stop(void)
{

    cancel_delayed_work_sync(&sff_polling);
    return 0;

}
static int sff_objs_init(struct sff_mgr_t *sff, struct port_info_table_t *tbl)
{

    struct port_info_map_t *map = NULL;
    struct sff_obj_t *sff_obj = NULL;
    int port_num = 0;
    int port = 0;
    map = tbl->map;
    port_num = tbl->size;

    if (!tbl) {
        return -EBADRQC;
    }

    if (!map) {
        return -EBADRQC;
    }

    sff_obj = sff->obj;

    for (port = 0; port < port_num; port++) {

        sff_obj[port].port = port;
        sff_obj[port].type = map[port].type;
        sff_obj[port].name = map[port].name;
        sff_obj[port].mgr = sff;
        sff_fsm_init(&sff_obj[port], map[port].type);
        func_tbl_init(&sff_obj[port], map[port].type);
    }
    return 0;
}
/*size of sff_object is the max port numbers on line card
 *ex: if 400 line card contain 8 ports
         100 line card contain 32 ports
 *       we will define size of sff obj = 32
 *       however we'll only init 8 ports if it's 400g line card
 * */
static int sff_obj_create(struct sff_mgr_t *sff, int size)
{

    struct sff_obj_t *sff_obj = NULL;

    sff_obj = kzalloc(sizeof(struct sff_obj_t) * size, GFP_KERNEL);
    if(!sff_obj) {
        return -ENOMEM;
    }

    sff->obj = sff_obj;
    return 0;


}
void sff_kobj_del(struct sff_kobj_t **obj)
{
    if (*obj) {
        kobject_put(&((*obj)->kobj));
        *obj = NULL;
    }
}
/*the function is for adding kobject of port dynamically*/
static struct sff_kobj_t *sff_kobj_add(char *name,
                                       struct kobject *parent,
                                       struct attribute_group *attr_group)
{
    int ret = 0;
    struct sff_kobj_t *obj = NULL;
    obj = kzalloc(sizeof(struct sff_kobj_t), GFP_KERNEL);
    if (!obj) {
        SFF_MGR_ERR("sff_kobject_create %s kzalloc error", name);
        return NULL;
    }
    obj->kobj.kset = Sff_Kset;

    ret = kobject_init_and_add(&obj->kobj, &sff_ktype, parent, name);
    if (ret < 0) {

        kobject_put(&obj->kobj);
        SFF_MGR_ERR("kobject creat fail1\n");
        return NULL;
    }
    if (NULL != attr_group) {

        SFF_MGR_DEBUG("sysfs_create_group: %s\n", name);
        if ((ret = sysfs_create_group(&obj->kobj, attr_group)) != 0) {
            sysfs_remove_group(&obj->kobj, attr_group);
            sff_kobj_del(&obj);
            SFF_MGR_ERR("create sff:%s attrs error.ret:%d\n", name, ret);
            return NULL;
        }
    }

    kobject_uevent(&obj->kobj, KOBJ_ADD);
    return obj;
}

int sff_fsm_kobj_change_event(struct sff_obj_t *sff_obj)
{

    struct sff_kobj_t *sff_kobj = sff_obj->kobj;
    char *uevent_envp[4];
    char tmp_str_1[32];
    snprintf(tmp_str_1, sizeof(tmp_str_1), "%s=%s", "IF_TYPE", "SR4");
    uevent_envp[0] = tmp_str_1;
    uevent_envp[1] = NULL;

    kobject_uevent_env(&sff_kobj->kobj, KOBJ_CHANGE, uevent_envp);
    return 0;
}
static int sff_fsm_kobj_add(struct sff_obj_t *sff_obj)
{
    struct attribute_group *attr_group = &sfp_group;
    struct sff_kobj_t *mgr_kobj = NULL;
    SFF_MGR_DEBUG("port:%d start\n", sff_obj->port);
    if (SFP_TYPE == sff_obj->type) {
        attr_group = &sfp_group;
    } else if (QSFP_TYPE == sff_obj->type) {

        attr_group = &qsfp_group;

    } else {

        attr_group = &qsfp_dd_group;

    }
    mgr_kobj = sff_obj->mgr->mgr_kobj;
    sff_obj->kobj = sff_kobj_add(sff_obj->name, &(mgr_kobj->kobj), attr_group);
    if(!sff_obj->kobj) {

        return -EBADRQC;
    }

    sff_obj->kobj->sff_obj = sff_obj;
    SFF_MGR_DEBUG("port:%d ok\n", sff_obj->port);
    return 0;
}

static int sff_fsm_kobj_del(struct sff_obj_t *sff_obj)
{
    struct sff_kobj_t **sff_kobj = &(sff_obj->kobj);

    if((*sff_kobj)) {
        sff_kobj_del(sff_kobj);
    }
    return 0;
}
static int sff_kset_create_init(void)
{

    Sff_Kset = kset_create_and_add(SFF_KSET, NULL, NULL);

    if(!Sff_Kset) {
        SFF_MGR_ERR("kset creat fail\n");
        return -EBADRQC;
    }
    return 0;
}
static int sff_kset_deinit(void)
{

    if(Sff_Kset) {
        kset_unregister(Sff_Kset);
    }
    return 0;
}

static int sff_public_kobj_init_create(struct sff_mgr_t *sff)
{
    sff->mgr_kobj = sff_kobj_add("sff",  &Sff_Kset->kobj, &sff_mgr_group);
    if(!(sff->mgr_kobj)) {

        return -EBADRQC;
    }

    sff->common_kobj = sff_kobj_add("common",  &Sff_Kset->kobj, &sff_common_group);
    if(!(sff->common_kobj)) {

        if((sff->mgr_kobj)) {
            sff_kobj_del(&(sff->mgr_kobj));
        }
        return -EBADRQC;
    }

    /*this link will let card kobj able to link to sff_obj*/
    sff->mgr_kobj->sff_obj = sff->obj;
    sff->common_kobj->sff_obj = sff->obj;

    return 0;

}

static void sff_public_kobj_destroy(struct sff_mgr_t *sff)
{
    if((sff->mgr_kobj)) {
        sff_kobj_del(&(sff->mgr_kobj));
    }
    if((sff->common_kobj)) {
        sff_kobj_del(&(sff->common_kobj));
    }
}
/*@note: kobj num is identical to valid port num*/
static void sff_kobjs_destroy(struct sff_mgr_t *sff)
{
    int port;
    int port_num = sff->port_num;
    struct sff_obj_t *sff_obj = NULL;
    for (port = 0; port < port_num; port++) {
        sff_obj = &(sff->obj[port]);
        sff_fsm_kobj_del(sff_obj);
    }
}
static void transvr_insert(struct sff_obj_t *sff_obj)
{
    SFF_MGR_DEBUG("transvr:%d insert\n", sff_obj->port);
    sff_fsm_st_set(sff_obj, SFF_FSM_ST_INSERTED);
    transvr_type_set(sff_obj, TRANSVR_CLASS_UNKNOWN);
    sff_fsm_kobj_add(sff_obj);
}
static void transvr_remove(struct sff_obj_t *sff_obj)
{
    SFF_MGR_DEBUG("transvr:%d remove\n", sff_obj->port);
    sff_fsm_st_set(sff_obj, SFF_FSM_ST_REMOVED);
    transvr_type_set(sff_obj, TRANSVR_CLASS_UNKNOWN);
    sff_fsm_kobj_del(sff_obj);
}
#if 0
static int transvr_is_prsL(struct sff_obj_t *sff_obj)
{
    int is_prsL = 0;
    unsigned long bitmap = 0;

    bitmap = Sff.prsL;
    if(bitmap & 1L << port) {
        is_prsL = 1;
    }
    return is_prsL;
}
#endif
static int io_no_init_scan(struct sff_mgr_t *sff)
{
    int ret = 0;
    int port = 0;
    int port_num = sff->port_num;
    struct sff_obj_t *sff_obj = NULL;
    unsigned long bitmap=0;


    if((ret = sffDrv.prsL_all_get(&bitmap)) < 0) {
        return ret;
    }
    prsL_bitmap_update(sff, bitmap); //update current prsL_bitmap

    for (port = 0; port < port_num; port++) {

        sff_obj = &sff->obj[port];
        ret = io_no_init_done_fsm_run(sff_obj);
        if (ret < 0) {
            break;
        }
    }

    return ret;
}
static int prsL_scan(struct sff_mgr_t *sff)
{
    unsigned long bitmap=0;
    unsigned long prsL_change = 0;
    int port = 0;
    int try = 0;
    struct sff_obj_t *sff_obj = NULL;
    int port_num = sff->port_num;
    /*<TBD> probably can't get all prsL pins status at once
     in case  one bad transciver module is plugged in, it will lead to i2c bus hang up
     we need to figure out the way to detect this situation and skip it and send out the warning */
    for (try = 0; try < TRY_NUM; try++) {
        if(0 == sffDrv.prsL_all_get(&bitmap)) {
            break;
        }
    }
    /*try fail, so far it only happens
    * when other app access broken transciver eeprom*/
    if (try >= TRY_NUM) {
            //SFF_MGR_ERR("fail\n");
            //i2c_err_handle_task_invoke();
            return -EBADRQC;
        }
    /*check which bits are updated*/
    //bitmap = ~bitmap;  /*reverse it to be human readable format*/
    prsL_change = bitmap ^ prsL_bitmap_get(sff);
    prsL_bitmap_update(sff, bitmap); //update current prsL_bitmap

    for (port = 0; port < port_num; port++) {
        sff_obj = &(sff->obj[port]);
        if(prsL_change & 0x01) {
            if(!(bitmap & 0x01)) {
                transvr_insert(sff_obj);
            } else {
                transvr_remove(sff_obj);
            }
        }

        bitmap >>= 1;
        prsL_change >>= 1;
    }
    return 0;
}
#if 0 /*do it later*/
static int io_mux_reset_all_seq(struct sff_mgr_t *sff)
{
    int try = 0;

    for (try = 0; try < 3; try++) {
                    /*pull low*/
                    if(sff->drv->io_mux_reset_all(0) < 0) {
                        SFF_MGR_ERR("fail\n");
                        return -1;
                    }
                    mdelay(1);
                    /*pull high*/
                    if(sff->drv->io_mux_reset_all(1) < 0) {
                        return -1;
                    }
                    mdelay(10);

                    if (ioexp_scan_ready()) {
                        SFF_MGR_DEBUG("pass\n");
                        break;
                    }
                }
    if (try >= 3) {
            SFF_MGR_DEBUG("try fail\n");
            return -1;
        }
    return 0;

}
static int i2c_crush_handler(void)
{
    int st = I2C_CRUSH_INIT_ST;
    SFF_MGR_ERR("start\n");

    while (st != I2C_CRUSH_END_ST) {

        SFF_MGR_ERR("state:%d\n", st);
        switch (st) {

        case I2C_CRUSH_INIT_ST: {
            if (io_mux_reset_all_seq() < 0) {
                st = I2C_CRUSH_END_ST;
                break;
            }
            st = I2C_CRUSH_IO_I2C_CHECK_ST;
        }
        break;

        case I2C_CRUSH_IO_I2C_CHECK_ST: {
            if (!ioexp_is_i2c_ready()) {
                SFF_MGR_ERR("ioexp i2c is NOT ready\n");
                st = I2C_CRUSH_END_ST;
                break;

            }
            SFF_MGR_ERR("ioexp i2c is ready\n");
            st = I2C_CRUSH_BAD_TRANSVR_DETECT_ST;
        }
        break;

        case I2C_CRUSH_BAD_TRANSVR_DETECT_ST: {
            bad_transvr_detect();
            st = I2C_CRUSH_I2C_RECHECK_ST;

        }
        break;

        case I2C_CRUSH_I2C_RECHECK_ST: {
            if (ioexp_is_i2c_ready() &&
                    transvr_is_i2c_ready()) {

                SFF_MGR_ERR("reset i2c ok\n");
                _isolated_transvr_report();

            } else {

                SFF_MGR_ERR("reset i2c fail\n");

            }
            st = I2C_CRUSH_END_ST;

        }
        break;

        }

    }
    return 0;
}
#endif
static void sff_polling_task(struct work_struct *work)
{
    int ret = 0;

    if (ioexp_input_handler() < 0) {
        //goto i2c_crush;
    }

    if ((ret = sffMgr.prsL_scan(&sffMgr)) < 0) {

        //goto i2c_crush;
    }
    if ((ret = sff_fsm_run(&sffMgr)) < 0) {
        //goto i2c_crush;
    }

    io_no_init_handler(&sffMgr);
#if 0
    if (scan_100ms++ >= HEALTH_SCAN_TIME) {
        scan_100ms = 0;
        if (ioexp_health_monitor() < 0) {
            goto i2c_crush;
        }
        if (transvr_health_monitor() < 0) {

            goto i2c_crush;
        }
    }
#endif
    schedule_delayed_work(&sff_polling, SFF_POLLING_PERIOD);

#if 0 /*do it later*/
i2c_crush:
    i2c_crush_handler();
    schedule_delayed_work(&sff_polling, SFF_POLLING_PERIOD);
#endif
}


/*fsm functions*/

inline sff_fsm_state_t sff_fsm_st_get(struct sff_obj_t *sff_obj)
{
    return sff_obj->fsm.st;
}
inline void sff_fsm_st_set(struct sff_obj_t *sff_obj, sff_fsm_state_t st)
{
    sff_fsm_state_t old_st = sff_obj->fsm.st;
    sff_fsm_state_change_process(sff_obj, old_st, st);
    sff_obj->fsm.st = st;
}
/*set target count to new fsm state*/
static void sff_fsm_delay_cnt_reset(struct sff_obj_t *sff_obj, sff_fsm_state_t st)
{
    int found = 0;
    int idx = 0;
    struct sff_fsm_t *fsm = &(sff_obj->fsm);
    struct fsm_period_t *table = fsm->period_tbl;
    for (idx = 0; table[idx].st != SFF_FSM_ST_END; idx++) {
        if (st == table[idx].st) {
            found = 1;
            break;
        }
    }
    fsm->cnt = 0;

    if (found) {
        fsm->delay_cnt = table[idx].delay_cnt;
    } else {
        fsm->delay_cnt = 0;
    }
}
static void sff_fsm_cnt_run(struct sff_obj_t *sff_obj)
{

    (sff_obj->fsm.cnt)++;
}
static bool sff_fsm_delay_cnt_is_hit(struct sff_obj_t *sff_obj)
{
    bool is_hit = false;
    struct sff_fsm_t *fsm = &(sff_obj->fsm);


    if(fsm->cnt >= fsm->delay_cnt) {
        fsm->cnt = 0;
        is_hit = true;
    }
    return is_hit;
}

static int dummy_lane_control_set(struct sff_obj_t *sff_obj, int type, u32 value)
{
    return -ENOSYS;
}
static int dummy_lane_control_get(struct sff_obj_t *sff_obj, int type, u32 *value)
{
    return -ENOSYS;
}

int dummy_lane_status_get(struct sff_obj_t *sff_obj, int type, u8 *st)
{
    return -ENOSYS;
}

static int sfp_tx_disable_set(struct sff_obj_t *sff_obj, u8 value)
{
    return sffDrv.tx_disable_set(sff_obj->port, value);
}

static int sfp_tx_disable_get(struct sff_obj_t *sff_obj, u8 *value)
{
    return sffDrv.tx_disable_get(sff_obj->port, value);

}
static int sfp_rx_los_get(struct sff_obj_t *sff_obj, u8 *value)
{
    return sffDrv.rx_los_get(sff_obj->port, value);
}
static int sfp_tx_fault_get(struct sff_obj_t *sff_obj, u8 *value)
{
    return sffDrv.tx_fault_get(sff_obj->port, value);
}
static int qsfp_intL_get(struct sff_obj_t *sff_obj, u8 *value)
{
    return sffDrv.intL_get(sff_obj->port, value);
}
static int qsfp_reset_set(struct sff_obj_t *sff_obj, u8 value)
{
    int ret = 0;
    u8 prsL = 0;

    if ((ret = sffDrv.reset_set(sff_obj->port, value)) < 0) {
        return ret;
    }

    if ((ret = sff_obj->func_tbl->prsL_get(sff_obj, &prsL)) < 0) {
        return ret;
    }
    if (!prsL) {
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_INSERTED);
        transvr_type_set(sff_obj, TRANSVR_CLASS_UNKNOWN);
    }
    return 0;
}
static int qsfp_reset_get(struct sff_obj_t *sff_obj, u8 *value)
{
    return sffDrv.reset_get(sff_obj->port, value);
}
static int qsfp_lpmode_set(struct sff_obj_t *sff_obj, u8 value)
{
    return sffDrv.lpmode_set(sff_obj->port, value);
}
static int qsfp_lpmode_get(struct sff_obj_t *sff_obj, u8 *value)
{

    return sffDrv.lpmode_get(sff_obj->port, value);

}
static int mode_sel_set(struct sff_obj_t *sff_obj, u8 value)
{

    return sffDrv.mode_sel_set(sff_obj->port, value);

}
static int mode_sel_get(struct sff_obj_t *sff_obj, u8 *value)
{

    return sffDrv.mode_sel_get(sff_obj->port, value);

}

static int dummy_rx_los_get(struct sff_obj_t *sff_obj, u8 *value)
{
    return -ENOSYS;

}
static int dummy_tx_fault_get(struct sff_obj_t *sff_obj, u8 *value)
{
    return -ENOSYS;
}

static int dummy_intL_get(struct sff_obj_t *sff_obj, u8 *value)
{
    return -ENOSYS;

}
static int dummy_reset_set(struct sff_obj_t *sff_obj, u8 value)
{
    return -ENOSYS;
}
static int dummy_reset_get(struct sff_obj_t *sff_obj, u8 *value)
{

    return -ENOSYS;

}
static int dummy_lpmode_set(struct sff_obj_t *sff_obj, u8 value)
{
    return -ENOSYS;
}
static int dummy_lpmode_get(struct sff_obj_t *sff_obj, u8 *value)
{

    return -ENOSYS;

}
static int dummy_mode_sel_set(struct sff_obj_t *sff_obj, u8 value)
{

    return -ENOSYS;

}
static int dummy_mode_sel_get(struct sff_obj_t *sff_obj, u8 *value)
{

    return -ENOSYS;

}
void sff_fsm_state_change_process(struct sff_obj_t *sff_obj,
                                  sff_fsm_state_t cur_st,
                                  sff_fsm_state_t next_st)
{


    if (cur_st != next_st) {

        sff_fsm_delay_cnt_reset(sff_obj, next_st);
        //SFF_MGR_DEBUG("port:%d st change:%d -> %d\n",
        //            port, st,sff_fsm_st_get(sff_obj));
        SFF_MGR_INFO("port:%d st change:%s -> %s\n",
                     sff_obj->port, sff_fsm_st_str[cur_st],
                     sff_fsm_st_str[next_st]);

        switch(next_st) {
        case SFF_FSM_ST_FAULT:
            //i2c_err_handle_task_invoke();
            //SFF_MGR_ERR("port:%d transvr will be isolated\n", port);
            break;
        default:
            break;
        }


    }

}
#if 0 /*do it later*/
static bool transvr_is_i2c_ready(void)
{
    int ret = 0;
    int port = 0;
    u8 addr = SFF_EEPROM_I2C_ADDR;
    u8 buf = 0;
    int fail_count = 0;
    int port_num = port_num_get();

    for (sff_obj = 0; port < port_num; port++) {
        buf = 0;
        if (SFF_FSM_ST_ISOLATED != sff_fsm_st_get(sff_obj)) {

            ret = Sff.drv->eeprom_read(sff_obj, addr, 0, &buf, 1);
            if (ret < 0) {
                if (!ioexp_is_lane_ready()) {
                    SFF_MGR_ERR("transvr[%d]:cause i2c crush\n", port);
                    io_mux_reset_all_seq();
                    fail_count++;
                    continue;
                }
            } else {
                SFF_MGR_DEBUG("transvr[%d]:ok ret:%d\n", port, buf);
            }
        }
    }
    if (fail_count > 0) {
        return false;
    }
    SFF_MGR_DEBUG("pass\n");
    return true;
}
static int eeprom_read_retry(struct sff_obj_t *sff_obj, u8 addr, u8 offset, u8 *buf, int len)
{
    int try = 0;
    int ret = 0;
    for (try = 0; try < 1; try++) {
                    ret = sffDrv.eeprom_read(sff_obj->port, addr, offset, buf, len);

                    if (ret < 0) {
                        //msleep(100);
                        continue;
                    }
                    break;
                }
    if (try >= 3) {

            SFF_MGR_ERR("transvr[%d]:read retry fail ret:%d\n", port, ret);
            return ret;
        }

    return 0;
}
static void _isolated_transvr_report(void)
{
    int port = 0;
    int port_num = port_num_get();
    for (sff_obj = 0; port < port_num; port++) {

        if (SFF_FSM_ST_ISOLATED == sff_fsm_st_get(sff_obj)) {
            SFF_MGR_ERR("transvr[%d] isolated\n", port);
        }
    }
}
static void _bad_transvr_detect_phase0(void)
{
    int ret = 0;
    int port = 0;
    int old_st = 0;
    u8 addr = SFF_EEPROM_I2C_ADDR;
    u8 buf = 0;
    int port_num = port_num_get();
    /*due to pca9548 mux driver will remember the lane, even mux is reset , the selected lane is not reset
     * so read the data of that lane after mux is reset will fail , so need to switch to other lanes first */

    /*phase 0 loop is used to switch lanes to avoid the problem above*/
    for (sff_obj = 1; port < port_num; port+=8) {
        buf = 0;

        if (SFF_FSM_ST_ISOLATED != sff_fsm_st_get(sff_obj)) {
            ret = eeprom_read_retry(sff_obj, addr, 0, &buf, 1);

            if (ret >= 0) {

                if (buf == 0x03 || buf == 0x11) {

                    //SFF_MGR_ERR("transvr[%d]:read! ok buf:0x%x ret:%d\n", port, buf, ret);
                    continue;
                }
            }
            //SFF_MGR_ERR("transvr[%d]:read! buf:0x%x ret:%d\n", port, buf, ret);
            if (!ioexp_is_lane_ready()) {
                //msleep(100);
                io_mux_reset_all_seq();
                if (SFF_FSM_ST_IDLE != sff_fsm_st_get(sff_obj)) {
                    old_st = sff_fsm_st_get(sff_obj);
                    SFF_MGR_ERR("transvr[%d]:isolated\n", port);
                    sff_fsm_st_set(sff_obj, SFF_FSM_ST_ISOLATED);
                    sff_fsm_state_change_process(sff_obj, old_st, sff_fsm_st_get(sff_obj));
                }
                break;
            }
        }

    }
}
static void _bad_transvr_detect_phase1(void)
{
    int ret = 0;
    u8 addr = SFF_EEPROM_I2C_ADDR;
    u8 buf = 0;
    int port = 0;
    int port_num = port_num_get();
    int old_st =0;

    for (sff_obj = 0; port < port_num; port++) {
        buf = 0;

        if (SFF_FSM_ST_ISOLATED != sff_fsm_st_get(sff_obj)) {
            ret = eeprom_read_retry(sff_obj, addr, 0, &buf, 1);
            if (ret >= 0) {
                if (buf == 0x03 || buf == 0x11) {

                    //SFF_MGR_ERR("transvr[%d]:read! ok buf:0x%x ret:%d\n", port, buf, ret);
                    continue;
                }
            }
            //if (buf != 0x03 || ret < 0) {
            //SFF_MGR_ERR("transvr[%d]:read! buf:0x%x ret:%d\n", port, buf, ret);
            if (!ioexp_is_lane_ready()) {
                //msleep(100);
                io_mux_reset_all_seq();
                if (SFF_FSM_ST_IDLE != sff_fsm_st_get(sff_obj)) {
                    old_st = sff_fsm_st_get(sff_obj);
                    SFF_MGR_ERR("transvr[%d]:isolated\n", port);
                    sff_fsm_st_set(sff_obj, SFF_FSM_ST_ISOLATED);
                    sff_fsm_state_change_process(sff_obj, old_st, sff_fsm_st_get(sff_obj));
                }
                break;
            }
        }

    }
}
static void bad_transvr_detect(void)
{
    int i = 0;
    int tol = port_num_get();
    for (i = 0; i < tol; i++) {

        SFF_MGR_DEBUG("count[%d]:\n", i);
        _bad_transvr_detect_phase0();
        _bad_transvr_detect_phase1();

    }

}
static int transvr_health_monitor(void)
{
    int ret = 0;
    u8 addr = SFF_EEPROM_I2C_ADDR;
    u8 buf = 0;
    int port_num = port_num_get();

    if (transvr_port == port_num) {
        transvr_port = 0;
    }
    if (SFF_FSM_ST_DETECTED == sff_fsm_st_get(transvr_port)) {
        ret = Sff.drv->eeprom_read(transvr_port, addr, 0, &buf, 1);
        if (ret < 0) {
            if (!ioexp_is_lane_ready()) {
                SFF_MGR_ERR("transr_port[%d] i2c crush\n", transvr_port);
                return -1;
            }
        } else {
            //SFF_MGR_ERR("transvr[%d]:ok ret:%d\n", transvr_port, buf);
        }
    }
    transvr_port++;
    return 0;
}
#endif
/*io_no_init is special functions when remove and re insert modules don't do sff io control to avoid cause port flap*/
static void io_no_init_done_bitmap_set(struct sff_obj_t *sff_obj)
{
    set_bit(sff_obj->port, &io_no_init_done_bitmap);

}
static int io_no_init_done_fsm_run(struct sff_obj_t *sff_obj)
{
    u8 prsL = 0;
    int ret = 0;
    if ((ret = prsL_get(sff_obj, &prsL)) < 0) {
        return ret;
    }

    if (!prsL) {
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTED);
        sff_fsm_kobj_add(sff_obj);

    } else {
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_REMOVED);
        sff_fsm_kobj_del(sff_obj);
    }

    io_no_init_done_bitmap_set(sff_obj);

    return ret;
}
static void io_no_init_handler(struct sff_mgr_t *sff)
{
    unsigned long mask = 0;
    int port_num = sff->port_num;
    if (io_no_init) {

        mask = MASK_SET(port_num);

        SFF_MGR_DEBUG("done_bitmap:0x%lx mask:0x%lx\n", io_no_init_done_bitmap, mask);
        if (mask == io_no_init_done_bitmap) {

            SFF_MGR_DEBUG("io_no_init done\n");
            io_no_init = 0;
            /*switch back to normal present scan function*/
            sff->prsL_scan = prsL_scan;

        }

    }

}
static int sff_fsm_run(struct sff_mgr_t *sff)
{
    int port = 0;
    int ret = 0;
    struct sff_obj_t *sff_obj = NULL;
    int port_num = sff->port_num;

    for(port = 0; port < port_num; port++) {
        sff_obj = &(sff->obj[port]);
        if (sff_fsm_delay_cnt_is_hit(sff_obj)) {
            ret = sff_obj->fsm.task(sff_obj);
            if (ret < 0) {
                return ret;
            }
        }
        sff_fsm_cnt_run(sff_obj);
    }
    return 0;
}
/*extract the bits out of the val, bits need to be contious, bit start to end*/
u8 masked_bits_get(u8 val, int bit_s, int bit_e)
{
    u8 mask = 0;
    int bit = 0;

    for (bit = bit_s; bit <= bit_e; bit++) {
        mask |= (1 << bit);
    }
    val = val & (mask);
    val = val >> bit_s;

    return val;
}

static int dummy_module_st_get(struct sff_obj_t *sff_obj, u8 *st)
{
    return -ENOSYS;
}

static void func_tbl_init(struct sff_obj_t *obj, int type)
{
    if(SFP_TYPE == type) {
        obj->func_tbl = &sfp_func_tbl;

    } else if (QSFP_TYPE == type) {

        obj->func_tbl = &qsfp_func_tbl;

    } else if (QSFP_DD_TYPE == type) {

        obj->func_tbl = &qsfp_dd_func_tbl;

    }

}
static void sff_fsm_init(struct sff_obj_t *obj, int type)
{
    struct sff_fsm_t *fsm = NULL;

    fsm = &(obj->fsm);
    if(SFP_TYPE == type) {
        fsm->task = sff_fsm_sfp_task;

    } else if (QSFP_TYPE == type) {

        fsm->task = sff_fsm_qsfp_task;

    } else if (QSFP_DD_TYPE == type) {

        fsm->task = sff_fsm_qsfp_dd_task;

    }
    fsm->period_tbl = fsm_period_tbl;
    fsm->st = SFF_FSM_ST_IDLE;
    fsm->cnt = 0;
    fsm->delay_cnt = 0;
}

static struct platform_port_info_t *_platform_port_info_load(int platform_name)
{
    int i = 0;
    for (i = 0; platform_port_info_tbl[i].platform_name != PLATFORM_END; i++) {
        if (platform_port_info_tbl[i].platform_name == platform_name) {

            return &platform_port_info_tbl[i];
        }

    }

    return NULL;
}
static int port_info_table_load(void)
{
    int platform_name = PLATFORM_NAME;
    struct platform_port_info_t *platform_info = NULL;
    platform_info = _platform_port_info_load(platform_name);
    if (!platform_info) {

        return -EBADRQC;
    }
    Port_Info_Tbl = platform_info->tbl;
    /*if line card is not plugable, port_info_table size = line card port number*/
    maxPortNum =  Port_Info_Tbl->size;
    return 0;
}
#if 1 /*do it later*/
/*qsfp_dd_only
 *
 *this seq is preventing the case : insmod the sff driver with optic module's already plugged in
 since the lpmode is not set yet , so the module may enter hw init mode right before insmod the driver
 *
 * */
static int sff_module_init_seq(struct sff_mgr_t *sff)
{
    int port = 0;
    int port_num = sff->port_num;
    int type = 0;
    int ret = 0;
    u8 prsL = 0;
    struct sff_obj_t *sff_obj = NULL;
    for (port = 0; port < port_num; port++) {

        sff_obj = &(sff->obj[port]);
        type = sff_type_get(sff_obj);
        if (QSFP_DD_TYPE != type) {
            continue;
        }

        if ((ret = prsL_get(sff_obj, &prsL)) < 0) {
            return ret;
        }
        if (!prsL) {
            /*assert resetL*/
            if ((ret = sffDrv.reset_set(sff_obj->port, 0)) < 0) {
                return ret;
            }
            msleep(RESET_ASSERT_TIME);
            /*de-assert resetL*/
            if ((ret = sffDrv.reset_set(sff_obj->port, 1)) < 0) {
                return ret;
            }

        }

    }
    return 0;
}
#endif
static void sff_data_init(struct sff_mgr_t *sff, int port_num)
{
    memset(&(sff->prsL), 0xff, sizeof(sff->prsL));
    sff->port_num = port_num;
    if (io_no_init) {

        sff->prsL_scan = io_no_init_scan;
    } else {

        sff->prsL_scan = prsL_scan;
    }
}
static int __init sff_init(void)
{

    if (port_info_table_load() < 0) {

        SFF_MGR_ERR("port_info_table_load fail\n");
        goto exit_err;
    }
    if (sff_io_init(io_no_init) < 0) {

        goto exit_err;
        SFF_MGR_ERR("sff_io_init fail\n");
    }
    if (sff_eeprom_init() < 0) {
        goto exit_err;
        SFF_MGR_ERR("sff_eeprom_init fail\n");
    }

    if (sff_kset_create_init() < 0) {
        goto exit_err;
    }

    sff_data_init(&sffMgr, maxPortNum);

    if (sff_obj_create(&sffMgr, maxPortNum) < 0) {

        goto exit_sff_kset_deinit;

    }

    if(sff_objs_init(&sffMgr, Port_Info_Tbl) < 0) {
        goto exit_free_sff_objs;
    }
    if (sff_module_init_seq(&sffMgr) < 0) {

        goto exit_free_sff_objs;
    }
    if (sff_public_kobj_init_create(&sffMgr) < 0) {

        goto exit_free_sff_objs;
    }

    if(sff_polling_is_enabled()) {
        sff_polling_task_start();
    }
    SFF_MGR_INFO("sff:%d  init ok\n", PLATFORM_NAME);
    return 0;
exit_free_sff_objs:
    sff_objs_destroy(&sffMgr);
exit_sff_kset_deinit:
    sff_kset_deinit();

exit_err:
    return -EBADRQC;
}

static void __exit sff_exit(void)
{

    if(sff_polling_is_enabled()) {
        sff_polling_task_stop();
    }

    sff_eeprom_deinit();
    sff_io_deinit();
    sff_kobjs_destroy(&sffMgr);
    sff_public_kobj_destroy(&sffMgr);
    sff_objs_destroy(&sffMgr);

    sff_kset_deinit();

    SFF_MGR_INFO("sff %d deinit ok\n", PLATFORM_NAME);
}


module_init(sff_init);
module_exit(sff_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alang <huang.alang@inventec.com>");
