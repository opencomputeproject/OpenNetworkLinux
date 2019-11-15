/*inv sff ,handling sff modules */
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
#include "sff.h"
#include "sff_spec.h"
#include "port_info/port_info.h"
#include "sff_io.h"
#include "sff_eeprom.h"

#if (DEBUG_LOG == 1)
#define SFF_MGR_DEBUG(fmt, args...) \
    printk (KERN_INFO "[SFF_MGR]%s: " fmt "\r\n",__FUNCTION__,  ##args)
#else
#define SFF_MGR_DEBUG(fmt, args...) 
#endif
#define SFF_MGR_INFO(fmt, args...) printk (KERN_INFO "[SFF_MGR]%s:"fmt,__FUNCTION__,  ##args)
#define SFF_MGR_ERR(fmt, args...)  printk_ratelimited (KERN_ERR "{SFF_MGR]%s: " fmt "\r\n",__FUNCTION__,  ##args)

static int scan_100ms = 0;
static bool int_flag_monitor_en = false;
#define HEALTH_SCAN_TIME (10)
static char work_buf[BUF_SIZE];

struct port_info_table_t *Port_Info_Tbl = NULL;
static void sff_polling_task(struct work_struct *work);
static DECLARE_DELAYED_WORK(sff_polling, sff_polling_task);
static u8 sff_polling_enabled = 1;

static int i2c_crush_handler(void);
static void _isolated_transvr_report(void);

static int sff_polling_task_start(void);
static int sff_polling_task_stop(void);
static int transvr_health_monitor(void);
static void bad_transvr_detect(void);
static bool transvr_is_i2c_ready(void);
static struct sff_kobj_t *sff_kobj_add(char *name, 
                                          struct kobject *parent, 
                                          struct attribute_group *attr_group);
/*static int io_mux_reset_all_seq(void);*/

static struct sff_driver_t Sff_Drv =
{
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
static int transvr_port = 0;
struct fsm_period_t
{
    sff_fsm_state_t st;
    int delay_cnt;
};
struct fsm_period_t fsm_period_tbl[] =
{
    {SFF_FSM_ST_REMOVED, 0},
    {SFF_FSM_ST_INSERTED, 3},
    {SFF_FSM_ST_DETECTING, 1},
    {SFF_FSM_ST_INIT, 1},
    {SFF_FSM_ST_DETECTED, 30},
    {SFF_FSM_ST_IDLE, 20},
    {SFF_FSM_ST_FAULT, 30},
    {SFF_FSM_ST_RESET_ASSERTED, 1},
    {SFF_FSM_ST_RESET_DEASSERTED, 1},
    {SFF_FSM_ST_ISOLATED, 30},
     /*qsfp-dd only {*/
    {SFF_FSM_ST_MGMT_INIT, 3},
    {SFF_FSM_ST_MODULE_HW_INIT, 3},
    {SFF_FSM_ST_MODULE_LOOPBACK_INIT, 3},
    {SFF_FSM_ST_MODULE_LOW_PWR_INIT, 3},
    {SFF_FSM_ST_MODULE_LOW_PWR_CONFIG_CHECK, 3},
    {SFF_FSM_ST_MODULE_LOW_PWR_CONTROL, 3},
    {SFF_FSM_ST_MODULE_WAITING_READY, 3},
    {SFF_FSM_ST_MODULE_READY, 3},
    {SFF_FSM_ST_MODULE_PWR_DOWN, 3},
    /*qsfp-dd only }*/
    {SFF_FSM_ST_UNKNOWN_TYPE, 30},    
    {SFF_FSM_ST_END, 0xff}, /*keep it at the end of table*/    
};

const char *sff_fsm_st_str[] =
{
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
     /*qsfp-dd only {*/
    "SFF_FSM_ST_MGMT_INIT",
    "SFF_FSM_ST_MODULE_HW_INIT",
    "SFF_FSM_ST_MODULE_LOOPBACK_INIT",
    "SFF_FSM_ST_MODULE_LOW_PWR_INIT",
    "SFF_FSM_ST_MODULE_LOW_PWR_CONFIG_CHECK",
    "SFF_FSM_ST_MODULE_LOW_PWR_CONTROL",
    "SFF_FSM_ST_MODULE_WAITING_READY",
    "SFF_FSM_ST_MODULE_READY",
    "SFF_FSM_ST_MODULE_PWR_DOWN",
    /*qsfp-dd only }*/
    "SFF_FSM_ST_UNKNOWN_TYPE",
    "SFF_FSM_ST_END",
};
/*struct sff_obj_t;*/



struct qsfp_dd_priv_data {

    int lane_num;
    u8 module_type;
    union qsfp_dd_app_advert_fields fields[APSEL_NUM];

};
struct qsfp_priv_data {
    u8 lane_st[CH_STATUS_NUM];
    u8 eeprom_cache[PAGE_NUM][EEPROM_SIZE];
    int cur_page;
};

union priv_data_t {

    struct qsfp_dd_priv_data qsfp_dd;
    struct qsfp_priv_data qsfp;
};
struct sff_kobj_t
{
    struct kobject kobj; 
    int port;
    struct sff_obj_t *sff_obj;
};


struct kset *Sff_Kset = NULL;

typedef struct sff_obj_t sff_obj_type;
struct sff_fsm_t
{
    sff_fsm_state_t st;      
    int (*task)(sff_obj_type *sff_obj);
    int cnt; /*used to count how many fsm loop's been running*/
    int delay_cnt; /*the target count for each state, will be reset during each state transition*/
    struct fsm_period_t *period_tbl; 
};

struct sff_obj_t 
{
    int port;
    enum sff_type type;
    int transvr_type;
    char *name;
    struct sff_kobj_t *kobj;
    union priv_data_t priv_data;
    struct sff_fsm_t fsm;
    struct sff_mgr_t *mgr;
};   

struct sff_mgr_t
{
    int port_num;
    unsigned long prsL;
    struct sff_obj_t *obj;
    struct sff_driver_t *drv;
    struct sff_kobj_t *parent_kobj;
};

int maxPortNum = 0;
static struct sff_mgr_t sffMgr;
static void sff_fsm_init(struct sff_obj_t *obj, int type);
static void sff_data_init(struct sff_mgr_t *sff, int port_num);

/*struct sff_mgr_t Sff;*/

#define to_sff_kobj(x) container_of(x, struct sff_kobj_t, kobj)

static void sff_fsm_state_change_process(struct sff_obj_t *sff_obj, sff_fsm_state_t cur_st, sff_fsm_state_t next_st);

static inline bool transvr_is_detected(struct sff_obj_t *sff_obj)
{
    return (sff_obj->fsm.st == SFF_FSM_ST_DETECTED ? (true) : (false));  

}    
static inline void transvr_type_set(struct sff_obj_t *sff_obj, int type)
{
   sff_obj->transvr_type=type; 
}    

static inline int transvr_type_get(struct sff_obj_t *sff_obj)
{
   return sff_obj->transvr_type; 
}    
//static int transvr_is_prsL(struct sff_obj_t *sff_obj);


/*it is for compose monitor data into u16 format from u8*/
union monitor_data_t
{
        u8 byte[2];
        u16 word;
        s16 signed_word;
};

struct monitor_para_t
{
    int type;
    u16 divider;
    char *unit;
};

static struct monitor_para_t monitor_para_table[] =
{
 /*  refer to sff 8636- ABLE 6-8  CHANNEL MONITORING VALUES (PAGE 00H BYTES 34-81)*/

    {CH_MONITOR_RX_PWR_TYPE, 10000, "mW"},
    {CH_MONITOR_TX_PWR_TYPE, 10000, "mW"},
    {CH_MONITOR_TX_BIAS_TYPE, 500, "mA"},  

};    
static struct monitor_para_t *monitor_para_find(int type)
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

static int sfp_eeprom_read(struct sff_obj_t *sff_obj, u8 slave_addr, u8 offset, u8 *buf, int len);
static int sfp_eeprom_write(struct sff_obj_t *sff_obj,u8 slave_addr, u8 offset, u8 *buf, int len);
static int qsfp_eeprom_read(struct sff_obj_t *sff_obj, u8 page, u8 offset, u8 *buf, int len);
static int qsfp_eeprom_write(struct sff_obj_t *sff_obj, u8 page, u8 offset, u8 *buf, int len);
/*mgr function declaration*/

static int lpmode_set(struct sff_obj_t *sff_obj, u8 value);
static int lpmode_get(struct sff_obj_t *sff_obj, u8 *value);
static int mode_sel_set(struct sff_obj_t *sff_obj, u8 value);
static int mode_sel_get(struct sff_obj_t *sff_obj, u8 *value);
static int tx_disable_set(struct sff_obj_t *sff_obj, u8 value);
static int tx_disable_get(struct sff_obj_t *sff_obj, u8 *value);
static int qsfp_temperature_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size);
static int qsfp_voltage_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size);
static int qsfp_cdr_control(struct sff_obj_t *sff_obj);
static int qsfp_lpmode_control(struct sff_obj_t *sff_obj);
static int qsfp_tx_disable_set(struct sff_obj_t *sff_obj, u8 value);
static int qsfp_tx_disable_get(struct sff_obj_t *sff_obj, u8 *value);
static int qsfp_intL_get(struct sff_obj_t *sff_obj, u8 *value);
static int sfp_tx_disable_set(struct sff_obj_t *sff_obj, u8 value);
static int sfp_tx_disable_get(struct sff_obj_t *sff_obj, u8 *value);

static int qsfp_channel_control_set(struct sff_obj_t *sff_obj, int type, u32 value);
static int qsfp_channel_control_get(struct sff_obj_t *sff_obj, int type, u32 *value);
static int qsfp_channel_monitor_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size);
static int qsfp_vendor_info_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size);
static int qsfp_channel_status_get(struct sff_obj_t *sff_obj, int type, u8 *value);
static int sfp_channel_status_get(struct sff_obj_t *sff_obj, int type, u8 *value);
int qsfp_lane_status_get(struct sff_obj_t *sff_obj, int type, u8 *st);
int qsfp_lane_status_update(struct sff_obj_t *sff_obj, int type, u8 value);
//static int qsfp_int_flag_read(struct sff_obj_t *sff_obj);

static int sfp_channel_control_set(struct sff_obj_t *sff_obj, int type, u32 value);
static int sfp_channel_control_get(struct sff_obj_t *sff_obj, int type, u32 *value);
/*qsfp_dd function*/
static bool is_bank_num_valid(struct sff_obj_t *sff_obj);
static int qsfp_dd_channel_control_get(struct sff_obj_t *sff_obj, int type, u32 *value);
static int qsfp_dd_active_control_set_indicator(struct sff_obj_t *sff_obj);
//static int qsfp_dd_id_status_get(struct sff_obj_t *sff_obj, struct qsfp_dd_id_status_t *attrta);
static int qsfp_dd_lane_monitor_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size);
static int qsfp_dd_tx_disable_set(struct sff_obj_t *sff_obj, u8 val);
static int qsfp_dd_tx_disable_get(struct sff_obj_t *sff_obj, u8 *val);
static int qsfp_dd_id_get(struct sff_obj_t *sff_obj, u8 *val);
static int qsfp_dd_vendor_info_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size);
//static int qsfp_dd_force_low_pwr(struct sff_obj_t *sff_obj, int en);
//static int app_advert_field_dump(struct sff_obj_t *sff_obj, u8 *buf, int buf_size);
static int qsfp_dd_module_st_get(struct sff_obj_t *sff_obj, u8 *st);
static int qsfp_dd_temperature_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size);
static int qsfp_dd_voltage_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size);
//static int qsfp_dd_interrupt_flags_read(struct sff_obj_t *sff_obj);
/*utility*/
static int sscanf_to_int(const char *buf, int *value);
static bool match(const char *str1, const char *str2);
static u8 masked_bits_get(u8 val, int bit_s, int bit_e);
/*debug*/
static char *str_head[BUF_SIZE];
static u8 debug_read_buf[BUF_SIZE];
struct debug_eeprom_func_t
{
    int type;
    int (*read)(struct sff_obj_t *, u8, u8,u8 *, int);
    int (*write)(struct sff_obj_t * , u8, u8,u8 *, int);   
};

static struct debug_eeprom_func_t debug_eeprom_func_tbl[] =
{
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
static inline int sff_fsm_st_get(struct sff_obj_t *sff_obj);
static inline void sff_fsm_st_set(struct sff_obj_t *sff_obj, sff_fsm_state_t st);
static void sff_fsm_delay_cnt_reset(struct sff_obj_t *sff_obj, sff_fsm_state_t st);
static void sff_fsm_cnt_run(struct sff_obj_t *sff_obj);
static bool sff_fsm_delay_cnt_is_hit(struct sff_obj_t *sff_obj);

static int sff_fsm_sfp_task(struct sff_obj_t *sff_obj);
static int sff_fsm_qsfp_task(struct sff_obj_t *sff_obj);

static int sff_fsm_run(struct sff_mgr_t *sff);

static int valid_lane_num_get(struct sff_obj_t *sff_obj);
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
    if(IS_ERR_OR_NULL(sff_obj->mgr->drv))
    {
        //<TBD>
        return -1;
    }    
    return sff_obj->mgr->drv->prsL_get(sff_obj->port, prsL);
}

/* return sff_field_info_t for a given field in qsfp_field_map[] */
static sff_sfp_field_info_t *get_sff_sfp_field_addr(const Sff_field field) {
    int i = 0; 
    int cnt = sizeof(sfp_field_map) / sizeof(sff_sfp_field_map_t);

    for (i = 0; i < cnt; i++) {
        if (sfp_field_map[i].field == field) {
        return (&(sfp_field_map[i].data));
         }
    }
    return NULL;
}
/* return the contents of the  sff_field_info_t for a given field */
static int get_sfp_field_addr(Sff_field field,
                                u8 *slave_addr,
                                u8 *offset,
                                int *length) {
    sff_sfp_field_info_t *info_data = NULL;
    info_data = get_sff_sfp_field_addr(field);
    
    if (field >= SFF_FIELD_MAX) {
        return -1;
    }
    if (!info_data) {
        return -1;
    }
    *offset = info_data->offset;
    *slave_addr = info_data->slave_addr;
    *length = info_data->length;
    return 0;
}
/* return sff_field_info_t for a given field in qsfp_field_map[] */
static sff_qsfp_field_info_t *get_sff_qsfp_field_addr(const Sff_field field) {
    int i = 0; 
    int cnt = 0;
    cnt = sizeof(qsfp_field_map) / sizeof(sff_qsfp_field_map_t);

    for (i = 0; i < cnt; i++) {
        if (qsfp_field_map[i].field == field) {
            return (&(qsfp_field_map[i].data));
        }
    }
    return NULL;
}
/* return the contents of the  sff_field_info_t for a given field */
static int get_qsfp_field_addr(Sff_field field,
                                u8 *page,
                                u8 *offset,
                                int *length) {
    sff_qsfp_field_info_t *info_data = NULL; 
    info_data = get_sff_qsfp_field_addr(field);
    
    if (field >= SFF_FIELD_MAX) {
        return -1;
    }
    if (!info_data) {
        return -1;
    }
    *offset = info_data->offset;
    *page = info_data->page;
    *length = info_data->length;
    return 0;
}
static int sfp_eeprom_read(struct sff_obj_t *sff_obj,
                            u8 slave_addr,
                            u8 offset,
                            u8 *buf,
                            int len)
{
    if (slave_addr != 0x50 && 
        slave_addr != 0x51) {
        SFF_MGR_ERR("addr out of range:0x%x\n", slave_addr);
        return -1;
    }

    return sff_obj->mgr->drv->eeprom_read(sff_obj->port, slave_addr, offset, buf, len);
}
static int sfp_eeprom_write(struct sff_obj_t *sff_obj,
                            u8 slave_addr,
                            u8 offset,
                            u8 *buf,
                            int len)
{
    if (slave_addr != 0x50 && 
        slave_addr != 0x51) {
        SFF_MGR_ERR("addr out of range:0x%x\n", slave_addr);
        return -1;
    }
    return sff_obj->mgr->drv->eeprom_write(sff_obj->port, slave_addr, offset, buf, len);
}
static int cur_page_update(struct sff_obj_t *sff_obj)
{
    int rec = 0;
    u8 slave_addr = SFF_EEPROM_I2C_ADDR;
    u8 offset_page_sel = 127;
    u8 reg_page = 0;

    /*need to switch page*/
    
    rec = sff_obj->mgr->drv->eeprom_read(sff_obj->port, slave_addr, offset_page_sel, &reg_page, 1);
    if (rec < 0)
    {
        SFF_MGR_ERR(" page get fail\n");
        sff_obj->priv_data.qsfp.cur_page = rec; 
        return rec;
    }
    sff_obj->priv_data.qsfp.cur_page = reg_page;
    return 0;

}    
static int _page_switch(struct sff_obj_t *sff_obj, u8 page) 
{
    
    int rec = 0;
    u8 slave_addr = SFF_EEPROM_I2C_ADDR;
    u8 offset_page_sel = 127;
    u8 reg = 0;
    if (sff_obj->priv_data.qsfp.cur_page == (int)page) {
        
        return 0;
    }
    /*need to switch page*/
    reg = page;
    rec = sff_obj->mgr->drv->eeprom_write(sff_obj->port, slave_addr, offset_page_sel, &reg, 1);
    if (rec < 0)
    {
        sff_obj->priv_data.qsfp.cur_page = rec;
        SFF_MGR_ERR("switch page fail\n");
        return rec;
    }
    sff_obj->priv_data.qsfp.cur_page = page;
    return 0;
}
static int sysfs_page_sel(struct sff_obj_t *sff_obj, int page)
{
    if (page < QSFP_PAGE0 ||  page >= PAGE_NUM) {

        return -1;
    } 
   
    return _page_switch(sff_obj, page);
}
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
static int qsfp_eeprom_dump(struct sff_obj_t *sff_obj, u8 *buf)
{
    int ret = 0; 
    int page = 0;
    int i = 0;
    int j = 0;
    int cnt = 0;
    u8 *data = NULL;
    u8 offset = 0;
    int last = EEPROM_SIZE;
    page = sff_obj->priv_data.qsfp.cur_page;
    if (page < QSFP_PAGE0 ||  page >= PAGE_NUM) {

        return -1;
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
        SFF_MGR_ERR("ret:%s read fail\n", ret);
        return ret;
    }
     
    #if 1
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
static int qsfp_eeprom_read(struct sff_obj_t *sff_obj,
                             u8 page,
                             u8 offset,
                             u8 *buf,
                             int len)
{
    int ret = 0;
    u8 slave_addr = SFF_EEPROM_I2C_ADDR;
    ret = _page_switch(sff_obj, page); 
    if (ret < 0) {
        return ret;
    }
    return sff_obj->mgr->drv->eeprom_read(sff_obj->port, slave_addr, offset, buf, len);         
}
static int qsfp_eeprom_write(struct sff_obj_t *sff_obj,
                             u8 page,
                             u8 offset,
                             u8 *buf,
                             int len)
{
    int ret = 0;
    u8 slave_addr = SFF_EEPROM_I2C_ADDR; //it's fixed
    ret = _page_switch(sff_obj, page); 
    if (ret < 0) {
        return ret;
    }
    return sff_obj->mgr->drv->eeprom_write(sff_obj->port, slave_addr, offset, buf, len);         
}
static int _sfp_id_read(struct sff_obj_t *sff_obj, u8 *id)
{
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int len = 0;
    get_sfp_field_addr(IDENTIFIER, &slave_addr, &offset, &len);
    return sfp_eeprom_read(sff_obj, slave_addr , offset, id, len);
}
static int _qsfp_id_read(struct sff_obj_t *sff_obj, u8 *id)
{
    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    get_qsfp_field_addr(IDENTIFIER, &page, &offset, &len);
    return qsfp_eeprom_read(sff_obj, page, offset, id, len);
}
static int _sfp_data_ready_check(struct sff_obj_t *sff_obj, u8 *is_ready)
{
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int data_len = 0;
    int ret = 0;
    u8 status = 0;
    if(IS_ERR_OR_NULL(is_ready))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }
    
    get_sfp_field_addr(STATUS_CONTROL, &slave_addr, &offset, &data_len);
    if((ret = sfp_eeprom_read(sff_obj, slave_addr, offset, &status, data_len)) < 0)
    {
        return ret;
        //SFF_MGR_ERR("port:%d _eeprom_read fail", port);
        //return -1;
    }
    status = ~(status & 0x01) & 0x01;
    *is_ready = status;
    
    return 0;
    #if 0  
    SFF_MGR_DEBUG("special_case of checking data ready\n");
    /*some cables don't support status control, in this case, check id item instead*/
    get_sfp_field_addr(IDENTIFIER, &slave_addr, &offset, &data_len);
    if (data_len != sizeof(status)) {
        goto exit_err;
    }   
    if(sfp_eeprom_read(sff_obj, slave_addr, offset, &status, data_len) < 0) {
        goto exit_err;
     }    
    *is_ready = 1;

    return 0;
    exit_err:
   return -1;
    #endif
}

static int _qsfp_data_ready_check(struct sff_obj_t *sff_obj, u8 *is_ready)
{
    u8 page = 0xff;
    u8 offset = 0;
    int data_len = 0;
    int ret = 0;
    u8 val = 0;
    u8 status[2] = {0xff};
    if(IS_ERR_OR_NULL(is_ready))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }
    get_qsfp_field_addr(STATUS, &page, &offset, &data_len);
    if(data_len != ARRAY_SIZE(status))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if((ret = qsfp_eeprom_read(sff_obj, page, offset, status, data_len)) < 0)
    {
        return ret;
    }
    val = ~(status[1] & 0x01) & 0x01;
    *is_ready = val;
    
    return 0;
    
    #if 0 
    /*<TBD> special case:ex: AVAGO sn:AF1741GG04J pn: AFBR-89CDDZ*/

    SFF_MGR_DEBUG("special_case of checking data ready\n");
    if((ret = _sfp_id_read(sff_obj, &val)) < 0) {
        return ret;
    }
    *is_ready = 1;
    return 0;
    #endif
}


static int _qsfp_ethernet_compliance_read(struct sff_obj_t *sff_obj, u8 *eth_comp)
{
    u8 page = 0xff;
    u8 offset = 0;
    int data_len = 0;
    u8 data = 0;
    if(IS_ERR_OR_NULL(eth_comp))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }
    get_qsfp_field_addr(ETHERNET_COMPLIANCE, &page, &offset, &data_len);
    if(data_len != sizeof(data))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if(qsfp_eeprom_read(sff_obj, page, offset, &data, data_len) < 0)
    {
        SFF_MGR_ERR("qsfp_eeprom_read fail");
        return -1;
    }
    *eth_comp = data;

    return 0;
}
static int _sfp_transvr_codes_read(struct sff_obj_t *sff_obj, u8 *transvr_codes, int size)
{
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int data_len = 0;
    
    if(IS_ERR_OR_NULL(transvr_codes))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }
    get_sfp_field_addr(TRANSCEIVER_CODE, &slave_addr, &offset, &data_len);
    if(data_len != size)
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if(sfp_eeprom_read(sff_obj, slave_addr, offset, transvr_codes, data_len) < 0)
    {
        SFF_MGR_ERR("qsfp_eeprom_read fail");
        return -1;
    }

    return 0;
}
static int _sfp_eth_extended_compliance_get(struct sff_obj_t *sff_obj, u8 *eth_ext_comp)
{
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int data_len = 0;
    u8 data = 0;
    if(IS_ERR_OR_NULL(eth_ext_comp))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }
    get_sfp_field_addr(ETHERNET_EXTENDED_COMPLIANCE, &slave_addr, &offset, &data_len);
    if(data_len != sizeof(data))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if(sfp_eeprom_read(sff_obj, slave_addr, offset, &data, data_len) < 0)
    {
        SFF_MGR_ERR("sfp_eeprom_read fail");
        return -1;
    }
    *eth_ext_comp = data;

    return 0;
}
static int _qsfp_eth_extended_compliance_get(struct sff_obj_t *sff_obj, u8 *eth_ext_comp)
{
    u8 page = 0xff;
    u8 offset = 0;
    int data_len = 0;
    u8 data = 0;
    if(IS_ERR_OR_NULL(eth_ext_comp))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }
    get_qsfp_field_addr(ETHERNET_EXTENDED_COMPLIANCE, &page, &offset, &data_len);
    if(data_len != sizeof(data))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if(qsfp_eeprom_read(sff_obj, page, offset, &data, data_len) < 0)
    {
        SFF_MGR_ERR("qsfp_eeprom_read fail");
        return -1;
    }
    *eth_ext_comp = data;

    return 0;
}
static int _sfp_connector_type_read(struct sff_obj_t *sff_obj, u8 *conn_type)
{
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int data_len = 0;
    u8 data = 0;
    if(NULL == conn_type)
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }
    get_sfp_field_addr(CONNECTOR_TYPE, &slave_addr, &offset, &data_len);
    if(data_len != sizeof(data))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if(sfp_eeprom_read(sff_obj, slave_addr, offset, &data, data_len) < 0)
    {
        SFF_MGR_ERR("qsfp_eeprom_read fail");
        return -1;
    }
    *conn_type = data;

    return 0;
}
#if 0
static int _qsfp_connector_type_read(struct sff_obj_t *sff_obj, u8 *conn_type)
{
    u8 page = 0xff;
    u8 offset = 0;
    int data_len = 0;
    u8 data = 0;
    if(IS_ERR_OR_NULL(conn_type))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }
    get_qsfp_field_addr(CONNECTOR_TYPE, &page, &offset, &data_len);
    if(data_len != sizeof(data))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if(qsfp_eeprom_read(sff_obj, page, offset, &data, data_len) < 0)
    {
        SFF_MGR_ERR("qsfp_eeprom_read fail");
        return -1;
    }
    *conn_type = data;

    return 0;
}

/*Media Connector Type offset 203 (Page 00h) Type of connector prsL in the module. See 
 * SFF-8024 for codes.*/
static int qsfp_dd_connector_type_read(struct sff_obj_t *sff_obj, u8 *conn_type)
{
    u8 data = 0;
    
    if(IS_ERR_OR_NULL(conn_type))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if(qsfp_eeprom_read(sff_obj, QSFP_PAGE0, 203, &data, 1) < 0)
    {
        SFF_MGR_ERR("qsfp_eeprom_read fail");
        *conn_type = 0xff; /*non define*/
        return -1;
    }
    *conn_type = data;

    return 0;
}
#endif
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
    
    while (*ptr != '\0')
    {
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
#if 0
static int port_get(const char *name)
{
    int port = 0;
    int port_num = port_num_get();
    for (sff_obj = 0; port < port_num; port++) {

        if (match(Sff.obj[port].name, name)) {
           
           break;     
        }   
    }

    //sscanf(name,"%d", &port);
    //SFF_MGR_DEBUG("name:%s port:%d\n", name, port);
    return port;
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

    ret = sysfs_page_sel(sff_obj, page);
    if (ret < 0) {
        return ret;
    } else {
        return count;
    }
}
static ssize_t sff_eeprom_dump_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                       char *buf)
{
    
    int ret = 0;
    int type = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    type = sff_type_get(sff_obj);

    if (SFP_TYPE == type)
    {
    }
    else if (QSFP_TYPE == type) 
    {
       ret = qsfp_eeprom_dump(sff_obj, buf); 
    } else {

       ret = qsfp_eeprom_dump(sff_obj, buf); 
    }    

    if(ret < 0) {
        return scnprintf(buf, BUF_SIZE, "%d\n", ret);
    }
    else {    
        return scnprintf(buf, BUF_SIZE, "%s", buf);
    }
}
static ssize_t id_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                       char *buf)
{
    unsigned  char id = 0; 
    int ret = 0;
    int type = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    type = sff_type_get(sff_obj);
    
    if (SFP_TYPE == type)
    {
      ret = _sfp_id_read(sff_obj, &id);   
    }
    else if (QSFP_TYPE == type) 
    {
      ret = _qsfp_id_read(sff_obj, &id);   
    } else {

      ret = qsfp_dd_id_get(sff_obj, &id);   
    }    

    if(ret < 0)
        return scnprintf(buf, BUF_SIZE, "%d\n", ret);
    else    
        return scnprintf(buf, BUF_SIZE, "0x%x\n", id);
}
/*qsfp_dd only*/
static ssize_t module_st_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                       char *buf)
{
    u8 val = 0; 
    int ret = 0;
    int type = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    type = sff_type_get(sff_obj);
    if (QSFP_DD_TYPE == type)
    {
      ret = qsfp_dd_module_st_get(sff_obj, &val);   
    }

    if(ret < 0)
        return scnprintf(buf, BUF_SIZE, "%d\n", ret);
    else    
        return scnprintf(buf, BUF_SIZE, "0x%x\n", val);
}
static ssize_t type_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                         char *buf)
{
    int type = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

    type = sff_type_get(sff_obj);
    if (SFP_TYPE == type)
    {
        return scnprintf(buf, BUF_SIZE, "%s\n", "SFP_TYPE");
    }
    else if (QSFP_TYPE == type)
    {
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
sscanf_to_int(const char *buf, int *value)  {



#if 1
    int   result  = 0;
    char hex_tag[] = "0x";
    char *ptr = NULL;
    ptr = strstr(buf, hex_tag);
    if (!IS_ERR_OR_NULL(ptr)) 
    {
        if (strcspn(buf, hex_tag) == 0) /*first index*/
        {    
            if (!sscanf(buf,"%x",&result)) {
                goto exit_err;
            }
        }
        else
        {
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
    return -1;
#else

int ldata = 0;
int ret = 0;
/*use kernel api instead*/
if (!buf || !value) {
    return -1;
} 
ret = kstrtol(buf, 0, &ldata);
if (ret < 0) {

    return ret;
}
*value = (int)ldata;
return 0;

#endif
}
static ssize_t channel_control_store(int ctrl_type, struct sff_kobj_t *sff_kobj,
                                          const char *buf, size_t count)
{
    int ch_ctrl = 0;
    int ret = 0;
    int type = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    type = sff_type_get(sff_obj);
    ret = sscanf_to_int(buf, &ch_ctrl);
    SFF_MGR_DEBUG("ch_ctrl:0x%x ret:%d\n", ch_ctrl, ret); 
    if(ret < 0)
        return ret;
   
    if (SFP_TYPE == type)
    {
      ret = sfp_channel_control_set(sff_obj, ctrl_type, ch_ctrl);
    }
    else
    {
      ret = qsfp_channel_control_set(sff_obj, ctrl_type, ch_ctrl);
      if(ret < 0)
      {
        return ret;
      }        
    }    
    return count;
}
static ssize_t channel_control_show(int ctrl_type, struct sff_kobj_t *sff_kobj, char *buf)
{
    u32 ch_ctrl = 0;
    int ret = 0;
    int type = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    
    type = sff_type_get(sff_obj);
    if (SFP_TYPE == type)
    {
      ret = sfp_channel_control_get(sff_obj, ctrl_type, &ch_ctrl); 
    }
    else if (QSFP_TYPE == type)
    {
      ret = qsfp_channel_control_get(sff_obj, ctrl_type, &ch_ctrl); 
    }
    else
    {
      ret = qsfp_dd_channel_control_get(sff_obj, ctrl_type, &ch_ctrl);  
      ret = qsfp_dd_active_control_set_indicator(sff_obj); /*<TBD>testing*/        
    }    

     if(ret < 0)
       return scnprintf(buf, BUF_SIZE, "%d\n", ret);
     else    
       return scnprintf(buf, BUF_SIZE, "0x%x\n", ch_ctrl);
}
static ssize_t sff_tx_eq_store(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                                          const char *buf, size_t count)
{
    return channel_control_store(TX_EQ_TYPE, kobj, buf, count);

}    
static ssize_t sff_rx_em_store(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                                          const char *buf, size_t count)
{
    return channel_control_store(RX_EM_TYPE, kobj, buf, count);
}    

static ssize_t sff_rx_am_store(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                                          const char *buf, size_t count)
{
    return channel_control_store(RX_AM_TYPE, kobj, buf, count);
} 

static ssize_t sff_tx_eq_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                       char *buf)
{
    return channel_control_show(TX_EQ_TYPE, kobj, buf);
}
static ssize_t sff_rx_em_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                       char *buf)
{
    return channel_control_show(RX_EM_TYPE, kobj, buf);
}
static ssize_t sff_rx_am_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                       char *buf)
{
    return channel_control_show(RX_AM_TYPE, kobj, buf);
}
static ssize_t channel_status_show(int st_type, struct sff_kobj_t *sff_kobj, char *buf)
{
    u8 ch_status = 0;
    int ret = 0;
    int type = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    type = sff_type_get(sff_obj);

#if defined (QSFP_INT_FLAG_SUPPORT)
    u8 intL = 0;
    if ((ret = qsfp_intL_get(sff_obj, &intL)) < 0) {
        goto err_exit;
    }
    if (intL) {
        if ((ret = qsfp_int_flag_read(sff_obj)) < 0) {
            goto err_exit;
        }
    }
    
    if (SFP_TYPE == type)
    {
      ret = sfp_channel_status_get(sff_obj, st_type, &ch_status); 
    }
    else
    {
      ret = qsfp_lane_status_get(sff_obj, st_type, &ch_status); 
    }    

     if(ret < 0) {
        goto err_exit;
     }
     return scnprintf(buf, BUF_SIZE, "0x%x\n", ch_status);
      
      err_exit:
      return scnprintf(buf, BUF_SIZE, "%d\n", ret);
#else

    if (SFP_TYPE == type)
    {
      ret = sfp_channel_status_get(sff_obj, st_type, &ch_status); 
    }
    else
    {
      ret = qsfp_channel_status_get(sff_obj, st_type, &ch_status); 
    }    

     if(ret < 0) {
        goto err_exit;
     }
     return scnprintf(buf, BUF_SIZE, "0x%x\n", ch_status);
      
      err_exit:
     return scnprintf(buf, BUF_SIZE, "%d\n", ret);


#endif      
       
}
static ssize_t sff_rx_los_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                       char *buf)
{
    return channel_status_show(CH_STATUS_RX_LOS_TYPE, kobj, buf);
}
static ssize_t sff_tx_los_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                       char *buf)
{
    return channel_status_show(CH_STATUS_TX_LOS_TYPE, kobj, buf);
}
static ssize_t sff_tx_fault_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                       char *buf)
{
    return channel_status_show(CH_STATUS_TX_FAULT_TYPE, kobj, buf);
}
static ssize_t channel_monitor_show(int moni_type, struct sff_kobj_t *sff_kobj, char *buf)
{
    int ret = 0;
    int type = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    type = sff_type_get(sff_obj);
    if (SFP_TYPE == type)
    {
    
    } else if (QSFP_TYPE == type) {
      ret = qsfp_channel_monitor_get(sff_obj, moni_type, buf, BUF_SIZE); 
    } else {

      ret = qsfp_dd_lane_monitor_get(sff_obj, moni_type, buf, BUF_SIZE); 
    }    

     if(ret < 0)
       return scnprintf(buf, BUF_SIZE, "%d\n", ret);
     else    
       return scnprintf(buf, BUF_SIZE, "%s", buf);
}
static ssize_t sff_tx_power_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                       char *buf)
{
    return channel_monitor_show(CH_MONITOR_TX_PWR_TYPE, kobj, buf);
}
#if 0
static ssize_t sff_app_advert_field_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                       char *buf)
{

    int port = port_get(sff_kobj->kobj.name);
    int ret = 0;
    int type = 0;
    type = sff_type_get(sff_obj);
    if (SFP_TYPE == type)
    {
    
    } else if (QSFP_TYPE == type) {
    } else {

      ret = app_advert_field_dump(sff_obj, buf, BUF_SIZE); 
    }    

     if(ret < 0)
       return scnprintf(buf, BUF_SIZE, "%d\n", ret);
     else    
       return scnprintf(buf, BUF_SIZE, "%s", buf);

}
#endif
static ssize_t sff_rx_power_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                       char *buf)
{
    return channel_monitor_show(CH_MONITOR_RX_PWR_TYPE, kobj, buf);
}
static ssize_t sff_tx_bias_show(struct sff_kobj_t *kobj, struct sff_attribute *attr,
                       char *buf)
{
    return channel_monitor_show(CH_MONITOR_TX_BIAS_TYPE, kobj, buf);
}

static ssize_t vendor_info_show(int info_type, struct sff_kobj_t *sff_kobj, char *buf)
{
    int ret = 0;
    int type = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    type = sff_type_get(sff_obj);
    if (SFP_TYPE == type)
    {
    }
    else if (QSFP_TYPE == type)
    {
      ret = qsfp_vendor_info_get(sff_obj, info_type, buf, BUF_SIZE); 
    } else {


      ret = qsfp_dd_vendor_info_get(sff_obj, info_type, buf, BUF_SIZE); 
    }    

     if(ret < 0)
       return scnprintf(buf, BUF_SIZE, "%d\n", ret);
     else    
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
    int type =  sff_type_get(sff_obj);
    
    if (SFP_TYPE == type) {
    
    } else if (QSFP_TYPE == type) {
      ret = qsfp_temperature_get(sff_obj, buf, BUF_SIZE); 
    } else {
      ret = qsfp_dd_temperature_get(sff_obj, buf, BUF_SIZE); 
    }    

     if(ret < 0)
       return scnprintf(buf, BUF_SIZE, "%d\n", ret);
     else    
       return scnprintf(buf, BUF_SIZE, "%s", buf);
}
static ssize_t voltage_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                       char *buf)
{
    
    int ret = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    
    int type = 0;
    type = sff_type_get(sff_obj);
    
    if (SFP_TYPE == type)
    {
    } else if (QSFP_TYPE == type) {
        ret = qsfp_voltage_get(sff_obj, buf, BUF_SIZE); 
    } else {
        ret = qsfp_dd_voltage_get(sff_obj, buf, BUF_SIZE); 
    }    

     if(ret < 0)
       return scnprintf(buf, BUF_SIZE, "%d\n", ret);
     else    
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
    int old_st = sff_fsm_st_get(sff_obj);
    ret = sscanf_to_int(buf, &resetL); 

    if(ret < 0)
    {
        return ret;
    }    
    if ((ret = sff_obj->mgr->drv->reset_set(sff_obj->port, resetL)) < 0)
    {
        return ret;
    }
    if (!resetL) { 
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_RESET_ASSERTED);
    } else {
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_RESET_DEASSERTED);
    }
    
    sff_fsm_state_change_process(sff_obj, old_st, sff_fsm_st_get(sff_obj));    
    transvr_type_set(sff_obj, TRANSVR_CLASS_UNKNOWN);    
    return count;
}
static ssize_t sff_reset_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                             char *buf)
{
    u8 reset = 0;
    int err_code = -1;
    int type = 0; 
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    type = sff_type_get(sff_obj);
    if (SFP_TYPE == type) { 
        
        return err_code;
    }   
    
    if (sff_obj->mgr->drv->reset_get(sff_obj->port, &reset) < 0)
    {
        return scnprintf(buf, BUF_SIZE, "%d\n", err_code);
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

    if(err_code < 0)
    {
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
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    err_code = sscanf_to_int(buf, &val); 
    if(err_code < 0)
    {
    
        SFF_MGR_ERR("1\n");
        return err_code;
    }    
    err_code = sff_obj->mgr->drv->io_mux_reset_all(val);
    
    if(err_code < 0)
    {
        SFF_MGR_ERR("2\n");
        return err_code;
    }    

    return count;
}
static ssize_t sff_lpmode_store(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                     const char *buf, size_t count)
{
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    int err_code = -1;
    int lpmode = 0;
    int type = 0;
    type = sff_type_get(sff_obj);
    
    if (SFP_TYPE == type) { 
        
        return err_code;
    }   
    err_code = sscanf_to_int(buf, &lpmode); 

    if(err_code < 0)
    {
        return err_code;
    }    
    err_code = lpmode_set(sff_obj, lpmode);

    if(err_code < 0)
    {
        return err_code;
    }    
    return count;
}
static ssize_t sff_lpmode_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                             char *buf)
{
    u8 lpmode = 0;
    int err_code = -1;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    int type = sff_type_get(sff_obj); 
    
    if (SFP_TYPE != type) {
        if (lpmode_get(sff_obj, &lpmode) < 0)
        {
            return scnprintf(buf, BUF_SIZE, "%d\n", err_code);
        }
    } else {

        return scnprintf(buf, BUF_SIZE, "%d\n", err_code);
    }    
        
    return scnprintf(buf, BUF_SIZE, "%d\n", lpmode);
}
static ssize_t sff_intL_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                             char *buf)
{
    u8 intL = 0;
    int err_code = -1;
    int type = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    type = sff_type_get(sff_obj);
   
    if (SFP_TYPE != type) {
        if (qsfp_intL_get(sff_obj, &intL) < 0)
        {
            return scnprintf(buf, BUF_SIZE, "%d\n", err_code);
        }
    } else {

        return scnprintf(buf, BUF_SIZE, "%d\n", err_code);
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

    if(err_code < 0)
    {
        return err_code;
    }    
    
    err_code = mode_sel_set(sff_obj, mode_sel);

    if(err_code < 0)
    {
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
        if (mode_sel_get(sff_obj, &mode_sel) < 0)
        {
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
    int err_code = -1;
    int tx_disable = 0;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    err_code = sscanf_to_int(buf, &tx_disable); 

    if(err_code < 0)
    {
        return err_code;
    }    
    
    err_code = tx_disable_set(sff_obj, tx_disable);

    if(err_code < 0)
    {
        return err_code;
    }    
    return count;
}
static ssize_t sff_tx_disable_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                             char *buf)
{
    u8 tx_disable = 0;
    int err_code = -1;
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    
    if (tx_disable_get(sff_obj, &tx_disable) < 0)
    {
        return scnprintf(buf, BUF_SIZE, "%d\n", err_code);
    }

    return scnprintf(buf, BUF_SIZE, "%d\n", tx_disable);
}
static ssize_t sff_prsL_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                     char *buf)
{
    u8 prsL = 0; 
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;

        
    int err_code = -1;
    if (prsL_get(sff_obj, &prsL) < 0)
    {
        return scnprintf(buf, BUF_SIZE, "%d\n", err_code);
    }    

    return scnprintf(buf, BUF_SIZE, "%d\n", prsL);
}

static ssize_t sff_prsL_all_show(struct sff_kobj_t *sff_kobj, struct sff_attribute *attr,
                     char *buf)
{
    int err_code = -1;
    unsigned long bitmap = 0;
    int port = 0;
    int port_num = port_num_get();
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
    
    if (sff_obj->mgr->drv->prsL_all_get(&bitmap) < 0)
    {
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

    if(enable == sff_polling_is_enabled())
    {
        return count;
    }    

    if(enable) {
        sff_polling_task_start();
    }
    else {
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
    struct sff_obj_t *sff_obj = sff_kobj->sff_obj;
        
    return scnprintf(buf, BUF_SIZE, "%d\n", sff_obj->priv_data.qsfp.cur_page);
}

static struct sff_attribute sff_type_attr =
    __ATTR(type, S_IRUGO, type_show, NULL);

static struct sff_attribute sff_id_attr =
    __ATTR(id, S_IRUGO, id_show, NULL);

static struct sff_attribute sff_transvr_type_attr =
    __ATTR(transvr_type, S_IRUGO, transvr_type_show, NULL);

static struct sff_attribute sff_transvr_st_attr =
    __ATTR(transvr_st, S_IRUGO, transvr_st_show, NULL);

static struct sff_attribute sff_prsL_attr =
    __ATTR(prsL, S_IRUGO, sff_prsL_show, NULL);

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
#if 0
static struct sff_attribute sff_app_advert_field_dump_attr =  
    __ATTR(app_advert_field_dump, S_IRUGO, NULL, sff_app_advert_field_show);
#endif
static struct sff_attribute sff_module_st_attr =
    __ATTR(module_st, S_IRUGO, module_st_show, NULL);

static struct sff_attribute mux_reset_attr =  
    __ATTR(mux_reset, S_IWUSR|S_IRUGO, gpio_mux_reset_show, gpio_mux_reset_store);
static struct sff_attribute swps_version_attr =  
    __ATTR(swps_version, S_IRUGO, swps_version_show, NULL);

static struct attribute *sfp_attributes[] = {

    /*io pin attribute*/
    &sff_prsL_attr.attr,
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
    &sff_prsL_attr.attr,
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
    &sff_prsL_attr.attr,
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
    //&sff_transvr_type_attr.attr,
    &sff_transvr_st_attr.attr,
    //&sff_debug_attr.attr,
    //&sff_eeprom_dump_attr.attr,
    //&sff_page_attr.attr,
    //&sff_app_advert_field_dump_attr.attr,
    &sff_module_st_attr.attr,
    NULL
};
static struct attribute_group qsfp_dd_group = {
        .attrs = qsfp_dd_attributes,
};
/*attribute of all sff modules , mainly reprsLed as bitmap form*/
static struct attribute *sff_common_attributes[] = {

    &sff_prsL_all_attr.attr,
    &sff_polling_attr.attr,
    &mux_reset_attr.attr,
    &sff_int_flag_monitor_attr.attr,
    &swps_version_attr.attr,
    NULL
};
static struct attribute_group sff_common_group = {
        .attrs = sff_common_attributes,
};

static void sff_objs_destroy(struct sff_mgr_t *sff)
{
    
    if (!IS_ERR_OR_NULL(sff->obj)) {
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
static int sff_driver_register(struct sff_mgr_t *sff, struct sff_driver_t *drv)
{
    if(IS_ERR_OR_NULL(drv))
    {
        SFF_MGR_ERR("NULL\n");
        return -1;
    }    
    sff->drv = drv;
    return 0;
}

static int sff_driver_unregister(void)
{
    if(sff_polling_is_enabled())
    {    
         sff_polling_task_stop();
    }    
    
    SFF_MGR_DEBUG("OK\n");
    return 0;
}
/*
 sff init sequence

 1) create sff mgr
 2) create sff object
 3) register sff driver  <TBD> first step use EXPORT SYMPOL , can try C1 method later on
 4) create device and sysfs interface

*/
#if 0
static struct port_info_map_t *_find_port_info(struct sff_obj_t *sff_obj, struct port_info_map_t map[], int obj_num)
{
    int id = 0;

    for (id = 0; id < obj_num; id++)
    {    
        if (sff_obj == map[id].port) {
            return &map[id];
        }
    }
    return NULL;

}
#endif
static int sff_objs_init(struct sff_mgr_t *sff, struct port_info_table_t *tbl)
{

    struct port_info_map_t *map = NULL; 
    struct sff_obj_t *sff_obj = NULL;
    int port_num = 0;
    int port = 0;
    map = tbl->map;
    port_num = tbl->size;
    
    if (IS_ERR_OR_NULL(tbl)) 
    {
        return -EBADRQC;
    }
    
    if (IS_ERR_OR_NULL(map)) 
    {
        return -EBADRQC;
    }
    
    sff_obj = sff->obj;
    
    for (port = 0; port < port_num; port++) {

        sff_obj[port].port = port;
        sff_obj[port].type = map[port].type;
        sff_obj[port].name = map[port].name;
        sff_obj[port].mgr = sff;
        sff_fsm_init(&sff_obj[port], map[port].type);
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
    if(IS_ERR_OR_NULL(sff_obj))
    {
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

static int sff_fsm_kobj_change_event(struct sff_obj_t *sff_obj)
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
    struct sff_kobj_t *parent_kobj = NULL;
    SFF_MGR_DEBUG("port:%d start\n", sff_obj->port);
    if (SFP_TYPE == sff_obj->type) {
        attr_group = &sfp_group;
    } else if (QSFP_TYPE == sff_obj->type) {

        attr_group = &qsfp_group;

    } else {

        attr_group = &qsfp_dd_group;

    }
    parent_kobj = sff_obj->mgr->parent_kobj;
    sff_obj->kobj = sff_kobj_add(sff_obj->name, &(parent_kobj->kobj), attr_group);
    sff_obj->kobj->sff_obj = sff_obj;
    if(IS_ERR_OR_NULL(sff_obj->kobj)) {
        
        return -1;
    }

    SFF_MGR_DEBUG("port:%d ok\n", sff_obj->port);
    return 0;
}

static int sff_fsm_kobj_del(struct sff_obj_t *sff_obj) 
{
    struct sff_kobj_t **sff_kobj = &(sff_obj->kobj); 
    
    if(!IS_ERR_OR_NULL(*sff_kobj)) {
        sff_kobj_del(sff_kobj);
    }
    return 0;
}   
static int sff_kset_create_init(void)
{

    Sff_Kset = kset_create_and_add(SFF_KSET, NULL, NULL);
    
    if(IS_ERR_OR_NULL(Sff_Kset))
    {
        SFF_MGR_ERR("kset creat fail\n");
        return -1;
    }
    return 0;
}
static int sff_kset_deinit(void)
{

    if(!IS_ERR_OR_NULL(Sff_Kset)) {
        kset_unregister(Sff_Kset);
    }
    return 0;
}

#if 1
static int sff_kobj_init_create(struct sff_mgr_t *sff)
{
    
    sff->parent_kobj = sff_kobj_add("sff",  &Sff_Kset->kobj, &sff_common_group); 
    if(IS_ERR_OR_NULL(sff->parent_kobj)) {

        return -1;
    }
    
    /*this link will let card kobj able to link to sff_obj*/
    sff->parent_kobj->sff_obj = sff->obj;
    return 0;
 
    
}
#endif

static void sff_parent_kobj_destroy(struct sff_mgr_t *sff)
{
    if(!IS_ERR_OR_NULL(sff->parent_kobj)) {
        sff_kobj_del(&(sff->parent_kobj));
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
    int old_st = sff_fsm_st_get(sff_obj);
    SFF_MGR_DEBUG("transvr:%d insert\n", sff_obj->port);
    sff_fsm_st_set(sff_obj, SFF_FSM_ST_INSERTED);
    sff_fsm_state_change_process(sff_obj, old_st, sff_fsm_st_get(sff_obj));    
    transvr_type_set(sff_obj, TRANSVR_CLASS_UNKNOWN);    
    sff_fsm_kobj_add(sff_obj);
}
static void transvr_remove(struct sff_obj_t *sff_obj)
{
    int old_st = sff_fsm_st_get(sff_obj);
    SFF_MGR_DEBUG("transvr:%d remove\n", sff_obj->port);
    sff_fsm_st_set(sff_obj, SFF_FSM_ST_REMOVED);        
    sff_fsm_state_change_process(sff_obj, old_st, sff_fsm_st_get(sff_obj));    
    transvr_type_set(sff_obj, TRANSVR_CLASS_UNKNOWN);    
    sff_fsm_kobj_del(sff_obj);
}
#if 0
static int transvr_is_prsL(struct sff_obj_t *sff_obj)
{
    int is_prsL = 0;
    unsigned long bitmap = 0;
    
    bitmap = Sff.prsL;
    if(bitmap & 1L << port)
    {
       is_prsL = 1; 
    }    
    return is_prsL;
}
#endif
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
    for (try = 0; try < TRY_NUM; try++)
    {   
        if (!sff->drv) {
            
            SFF_MGR_ERR("NULL ptr\n");  
            return 0;
        } 
        if(0 == sff->drv->prsL_all_get(&bitmap))
        {
            break;
        }    
    }
    /*try fail, so far it only happens 
    * when other app access broken transciver eeprom*/
    if (try >= TRY_NUM) 
    {
      //SFF_MGR_ERR("fail\n");
      //i2c_err_handle_task_invoke();
      return -1;    
    }    
    /*check which bits are updated*/
    //bitmap = ~bitmap;  /*reverse it to be human readable format*/
    prsL_change = bitmap ^ prsL_bitmap_get(sff); 
    prsL_bitmap_update(sff, bitmap); //update current prsL_bitmap 

    for (port = 0; port < port_num; port++) {
        sff_obj = &(sff->obj[port]);   
        if(prsL_change & 0x01)
        {
            if(!(bitmap & 0x01))
            {
                transvr_insert(sff_obj);
            }
            else
            {
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
        if(sff->drv->io_mux_reset_all(0) < 0)
        {
            SFF_MGR_ERR("fail\n");  
            return -1;
        }   
        mdelay(1); 
        /*pull high*/
        if(sff->drv->io_mux_reset_all(1) < 0)
        {
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

            case I2C_CRUSH_INIT_ST:
            {
                if (io_mux_reset_all_seq() < 0) {
                    st = I2C_CRUSH_END_ST;
                    break;
                }
                st = I2C_CRUSH_IO_I2C_CHECK_ST;
            }
            break;

            case I2C_CRUSH_IO_I2C_CHECK_ST:
            {
                if (!ioexp_is_i2c_ready()) {
                    SFF_MGR_ERR("ioexp i2c is NOT ready\n");
                    st = I2C_CRUSH_END_ST;
                    break;
                
                }
                SFF_MGR_ERR("ioexp i2c is ready\n");
                st = I2C_CRUSH_BAD_TRANSVR_DETECT_ST;
            }
            break;

            case I2C_CRUSH_BAD_TRANSVR_DETECT_ST:
            {
                bad_transvr_detect();
                st = I2C_CRUSH_I2C_RECHECK_ST;

            }
            break;

            case I2C_CRUSH_I2C_RECHECK_ST:
            {
                if (ioexp_is_i2c_ready() && 
                    transvr_is_i2c_ready()){ 
                
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
    int i = 0;
    int ret = 0;
    if (ioexp_input_handler() < 0) {
        goto i2c_crush;
    }

    if ((ret = prsL_scan(&sffMgr)) < 0) {
        goto i2c_crush;
    }
    if ((ret = sff_fsm_run(&sffMgr)) < 0) {
        goto i2c_crush;
    }
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
    return;
     
     i2c_crush:
         #if 0 /*do it later*/
         i2c_crush_handler();
         #endif
         schedule_delayed_work(&sff_polling, SFF_POLLING_PERIOD);
}    


/*fsm functions*/

static inline int sff_fsm_st_get(struct sff_obj_t *sff_obj)
{
    return sff_obj->fsm.st;
} 
static inline void sff_fsm_st_set(struct sff_obj_t *sff_obj, sff_fsm_state_t st)
{
   sff_obj->fsm.st = st;
}
/*set target count to new fsm state*/
static void sff_fsm_delay_cnt_reset(struct sff_obj_t *sff_obj, sff_fsm_state_t st)
{
   int found = 0;
   int idx = 0;
   struct sff_fsm_t *fsm = &(sff_obj->fsm);
   struct fsm_period_t *table = fsm->period_tbl;
   for (idx = 0; table[idx].st != SFF_FSM_ST_END; idx++)
   {
        if (st == table[idx].st)
        {
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

    
    if(fsm->cnt >= fsm->delay_cnt)
    {  
         fsm->cnt = 0; 
         is_hit = true;
    }   
    return is_hit;
}

static bool sfp_eth_comp_is_supported(struct sff_obj_t *sff_obj, u8 eth_comp)
{
    bool is_supported = false;
    int size = ARRAY_SIZE(Sfp_Eth_Comp_Table);
    int idx = 0;
    u8 comp_codes = 0;
    for (idx = 0; idx < size; idx++)
    {
        if(eth_comp & Sfp_Eth_Comp_Table[idx])
        {
            SFF_MGR_DEBUG("port:%d known eth_cmp:0x%x\n", sff_obj->port, eth_comp & Sfp_Eth_Comp_Table[idx]);
            comp_codes = eth_comp & Sfp_Eth_Comp_Table[idx];
            break;
        }    
    }    
    if(idx >= size)
    {

      SFF_MGR_DEBUG("port:%d unknown eth_cmp:0x%x\n", sff_obj->port, (eth_comp & 0xf0) >> 4);
    }
    
    switch (comp_codes)
    {
        case SR_10GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_10G_S_SR);
            break;
        case LR_10GBASE:
        case LRM_10GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_10G_S_LR);
            break;
        case ER_10GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_10G_S_ER);
            break;
        default:
        break;

    }    
    if (TRANSVR_CLASS_UNKNOWN != transvr_type_get(sff_obj))
    {
        is_supported = true;
        SFF_MGR_DEBUG("port:%d known eth_ext_cmp:0x%x transvr:%d\n", 
                      sff_obj->port, eth_comp, transvr_type_get(sff_obj));
    }    
    return is_supported;
}    
static bool qsfp_eth_comp_is_supported(struct sff_obj_t *sff_obj, u8 eth_comp)
{
    bool is_supported = false;
    int size = ARRAY_SIZE(Qsfp_Eth_Comp_Table);
    int idx = 0;
    u8 comp_codes = 0;
    for (idx = 0; idx < size; idx++)
    {
        if(eth_comp & Qsfp_Eth_Comp_Table[idx])
        {
            comp_codes = eth_comp & Qsfp_Eth_Comp_Table[idx];
            SFF_MGR_DEBUG("port:%d known eth_cmp:0x%x\n", sff_obj->port, eth_comp & Qsfp_Eth_Comp_Table[idx]);
            break;
        }    
    }    
    if(idx >= size)
    {
    
      SFF_MGR_DEBUG("port:%d unknown eth_cmp:0x%x\n", sff_obj->port, eth_comp);
    }    
   
    switch (comp_codes)
    {
        case ACTIVE_CABLE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_40G_AOC);
            break;
        case LR4_40GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_40G_LR4);
            break;
        case SR4_40GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_40G_SR4);
            break;
        case CR4_40GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_COPPER_L4_40G);
            break;
        default:
            break;
    }    
   
    if (TRANSVR_CLASS_UNKNOWN != transvr_type_get(sff_obj))
    {
        is_supported = true;
        SFF_MGR_DEBUG("port:%d known eth_ext_cmp:0x%x transvr:%d\n", 
                      sff_obj->port, eth_comp, transvr_type_get(sff_obj));
    }    
    return is_supported;
}    
static bool sfp_eth_ext_is_supported(struct sff_obj_t *sff_obj, u8 eth_ext_comp)
{
    bool is_supported = false;
    Ethernet_extended_compliance eth_ext_code = eth_ext_comp;
    switch(eth_ext_code)
    {
        case AOC_100G_BER_5:
        case AOC_100G_BER_12: /* 100G AOC or 25G AUI C2M AOC 10^^-12 BER */
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_25G_AOC);
            break;
        case SR4_100GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_25G_SR);
            break;
        case LR4_100GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_25G_LR);
            break;
        case ER4_100GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_25G_ER);
            break;
        case CR4_100GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_COPPER_L1_25G);    
            break;
        case ACC_100G_BER_5:
        case ACC_100G_BER_12: /* 100G ACC or 25G AUI C2M ACC 10^^-12 BER */
        case CR_25GBASE_CA_S:
        case CR_25GBASE_CA_N:
        case T_10BASE_SFI:  /* 10BASE-T with SFI electrical interface */
        {    
           /*<to do> how about these types!?*/   
            SFF_MGR_DEBUG("port:%d known eth_ext_cmp:0x%x\n", sff_obj->port, eth_ext_comp);
        }
        break;    
        default:
            SFF_MGR_DEBUG("port:%d unknown eth_ext_cmp:%d\n", sff_obj->port, eth_ext_comp);
       
        break;    
    }
    if (TRANSVR_CLASS_UNKNOWN != transvr_type_get(sff_obj))
    {
        is_supported = true;
        SFF_MGR_DEBUG("port:%d known eth_ext_cmp:0x%x transvr_type:%d\n", 
                      sff_obj->port, eth_ext_comp, transvr_type_get(sff_obj));
    }    
    return is_supported;
}  
static bool qsfp_eth_ext_is_supported(struct sff_obj_t *sff_obj, u8 eth_ext_comp)
{
    bool is_supported = false;
    Ethernet_extended_compliance eth_ext_code = eth_ext_comp;

    switch(eth_ext_code)
    {
        case AOC_100G_BER_5:
        case AOC_100G_BER_12:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_100G_AOC);
            break;
        case SR4_100GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_100G_SR4);
            break;
        case LR4_100GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_100G_LR4);
            break;
        case ER4_100GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_100G_ER4);
            break;
        case PSM4_100G_SMF:
            transvr_type_set(sff_obj, TRANSVR_CLASS_OPTICAL_100G_PSM4);
            break;
        case CR4_100GBASE:
            transvr_type_set(sff_obj, TRANSVR_CLASS_COPPER_L4_100G);
            break;
        case SR10_100GBASE:
        case CWDM4_100G:
        case ACC_100G_BER_5:
        case ACC_100G_BER_12:
        case ER4_40GBASE:
        case SR_10GBASE_4:
        case PSM4_40G_SMF:
        case CLR4_100G:
        case DWDM2_100GE:
        {    
            SFF_MGR_DEBUG("port:%d known eth_ext_cmp:0x%x\n", sff_obj->port, eth_ext_comp);
        }
        break;    
        default:
            SFF_MGR_DEBUG("port:%d unknown eth_ext_cmp:%d\n", sff_obj->port, eth_ext_comp);
       
        break;    
    }

    if (TRANSVR_CLASS_UNKNOWN != transvr_type_get(sff_obj))
    {
        is_supported = true;
        SFF_MGR_DEBUG("port:%d known eth_ext_cmp:0x%x transvr:%d\n", 
                      sff_obj->port, eth_ext_comp, transvr_type_get(sff_obj));
    }    
    return is_supported;
}  
static int sfp_type_identify(struct sff_obj_t *sff_obj, u8 *is_found)
{
    u8 id = 0;
    u8 transvr_codes[8];
    u8 conn_type = 0;
    u8 eth_ext_comp = 0;
    int idx = 0;
    int ret = 0;
    if((ret = _sfp_id_read(sff_obj, &id)) < 0)
    {
        goto exit_err;  
    }   
    if(0x03 != id) //sfp/sfp+/sfp28 
    {
        SFF_MGR_ERR("unknown id:0x%x\n", id);    
        goto exit_non_support_err; /*<TBD> error handling in the future*/
    }    
    
    if((ret = _sfp_eth_extended_compliance_get(sff_obj, &eth_ext_comp)) < 0)
    {
        goto exit_err;
    }
    
    if(!sfp_eth_ext_is_supported(sff_obj, eth_ext_comp))
    {
  
        if((ret = _sfp_transvr_codes_read(sff_obj, transvr_codes, ARRAY_SIZE(transvr_codes))) < 0)
        {
            goto exit_err;
        }    
         
        for (idx = 0; idx < 8; idx++)
        {
            SFF_MGR_DEBUG("port:%d transvr[%d]:0x%x\n", sff_obj->port, idx, transvr_codes[idx]);    
        }    
        
        if((ret = _sfp_connector_type_read(sff_obj, &conn_type)) < 0)
        {
            goto exit_err;
        }    
            
        SFF_MGR_DEBUG("port:%d conn_type:0x%x\n", sff_obj->port,conn_type);    
        
        /*check connector type first*/
         /*check transvr codes start from offset:3 */
        if (0x07 == conn_type)
        {    
            if(!sfp_eth_comp_is_supported(sff_obj, transvr_codes[0]))
            {
                goto exit_non_support_err;
            }
        }
        else if (0x0b == conn_type)
        {

            
        }    
        else if (0x21 == conn_type)
        {
           /*check SFP+ Cable Technology*/
           /*offset 8, bit2: Passive Cable *8 
            *          bit3: Active Cable *8 */
            if (transvr_codes[5] & 0x02)
            {
                /*passive*/        
            }
            else if (transvr_codes[5] & 0x03)
            {
               /*active*/ 
            }
            else
            {
                
                goto exit_non_support_err; /*<TBD> error handling in the future*/
            }    
        }    

    }
    
    *is_found = 1;
    return 0;
    exit_non_support_err: 
    *is_found = 0;
    return 0;
    exit_err: /*could be i2c fail , need to define err code in the future <TBD>*/
    return ret;    
}    
/* */
static int qsfp_type_identify(struct sff_obj_t *sff_obj, u8 *is_found)
{
    u8 id = 0;
    u8 eth_comp = 0;
    u8 eth_ext_comp = 0;
    int ret = 0; 
    if((ret = _qsfp_id_read(sff_obj, &id)) < 0)
    {
        goto exit_err;  
    }   
    if(0x0d != id && 
       0x11 != id &&
       0x0c != id)  // not qsfp+ (0x0d), qsfp28 (0x11), qsfp(INF 8438)(0xc)
    {
        SFF_MGR_ERR("unknown id:0x%x\n", id);    
        goto exit_non_support_err; /*<TBD> error handling in the future*/
    }    
    
    if((ret = _qsfp_ethernet_compliance_read(sff_obj, &eth_comp)) < 0)
    {
        goto exit_err;
    }    

    if (eth_comp & 0x80) //bit7 == 1 
    {
       if((ret = _qsfp_eth_extended_compliance_get(sff_obj, &eth_ext_comp)) < 0)
       {
            goto exit_err;
       }    
        
        if(!qsfp_eth_ext_is_supported(sff_obj, eth_ext_comp))
        {
            goto exit_non_support_err;
        }    
    }
    else /*40G*/
    {
       /* ACTIVE_CABLE = 1 << 0,
        *     LR4_40GBASE = 1 << 1,
        *         SR4_40GBASE = 1 << 2,
        *             CR4_40GBASE = 1 << 3,
        *
        */
        
        if(!qsfp_eth_comp_is_supported(sff_obj, eth_comp))
        {
            goto exit_non_support_err;
        }
    }
    *is_found = 1;
    return 0;
    
    exit_non_support_err: 
    *is_found = 0;
    return 0;
    exit_err: /*could be i2c fail , need to define err code in the future <TBD>*/
    return ret;    
}   
/*reference sff 8472: TABLE 9-12  ALARM AND WARNING FLAG BITS
 * start from offset 112 -> 117 : total size 6 bytes
 * offset 114 : 7-4 : tx input equalization control RATE=HIGH, 3-0: tx input equalization control RATE=LOW 
 * offset 115 : 7-4 : RX output emphasis control RATE=HIGH, 3-0: RX output emphasis control RATE=LOW 
 
 * */
static int sfp_channel_control_set(struct sff_obj_t *sff_obj, int type, u32 value)
{
    
    u8 slave_addr = 0xff;
    u8 offset = 0;
    u8 index = 0;
    int len = 0;
    
    u8 data = (u8)value;
    get_sfp_field_addr(ALARM_WARN_FLAGS, &slave_addr, &offset, &len);
    
    switch(type)
    {
        case TX_EQ_TYPE:
            {    
                index = 2;

            }
            break;
        case RX_EM_TYPE:
            {    
                index = 3;
            }
            break;
        default:
            goto exit_err;
            break;
    }    
    
    if( sfp_eeprom_write(sff_obj, slave_addr, offset+index, &data, sizeof(data)) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }
    
    return 0;
    
    exit_err:
        return -1;
}
/*reference sff 8472: TABLE 9-12  ALARM AND WARNING FLAG BITS
 * start from offset 112 -> 117 : total size 6 bytes
 * offset 114 : 7-4 : tx input equalization control RATE=HIGH, 3-0: tx input equalization control RATE=LOW 
 * offset 115 : 7-4 : RX output emphasis control RATE=HIGH, 3-0: RX output emphasis control RATE=LOW 
 
 * */
static int sfp_channel_control_get(struct sff_obj_t *sff_obj, int type, u32 *value)
{
    
    u8 slave_addr = 0xff;
    u8 offset = 0;
    u8 index = 0;
    int len = 0;
    
    u8 data = 0;
    get_sfp_field_addr(ALARM_WARN_FLAGS, &slave_addr, &offset, &len);
    
    switch(type)
    {
        case TX_EQ_TYPE:
            {    
                index = 2;

            }
            break;
        case RX_EM_TYPE:
            {    
                index = 3;
            }
            break;
        default:
            goto exit_err;
            break;
    }    
    
    if( sfp_eeprom_read(sff_obj, slave_addr, offset+index, &data, sizeof(data)) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }
    *value = (u32)data;
    return 0;
    
    exit_err:
        return -1;
}
static int qsfp_channel_control_set(struct sff_obj_t *sff_obj, int type, u32 value)
{
    
    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    
    u8 ch_ctrl[2];
    ch_ctrl[0] = (value & 0x0000ff00) >> 8;
    ch_ctrl[1] = (value & 0x000000ff);                
    
    switch(type)
    {
        case TX_EQ_TYPE:
            {    
                get_qsfp_field_addr(OPTIIONAL_CHANNEL_CONTROL_TX_EQ, &page, &offset, &len);
                SFF_MGR_DEBUG("port:%d tx eq supported\n", sff_obj->port);

            }
            break;
        case RX_EM_TYPE:
            {    
                get_qsfp_field_addr(OPTIIONAL_CHANNEL_CONTROL_RX_EM, &page, &offset, &len);
                SFF_MGR_DEBUG("port:%d tx eq supported\n", sff_obj->port);

            }
            break;
        case RX_AM_TYPE:
            {    
                get_qsfp_field_addr(OPTIIONAL_CHANNEL_CONTROL_RX_AM, &page, &offset, &len);
                SFF_MGR_DEBUG("port:%d tx eq supported\n", sff_obj->port);

            }
            break;
        default:
            goto exit_err;
            break;
    }    

    if(len != ARRAY_SIZE(ch_ctrl))
    {
        goto exit_err;
    }    
    if( qsfp_eeprom_write(sff_obj, page, offset, ch_ctrl, len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }
    return 0;
    
    exit_err:
        return -1;
}

static int qsfp_channel_control_get(struct sff_obj_t *sff_obj, int type, u32 *value)
{

    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    u8 opt_indicator[2];
    u8 ch_ctrl[2];
    qsfp_optional_indicator_t ind;
    get_qsfp_field_addr(OPTIIONAL_INDICATOR, &page, &offset, &len);
    if(len != ARRAY_SIZE(opt_indicator))
    {
        goto exit_err;
    }    
    if( qsfp_eeprom_read(sff_obj, page, offset, opt_indicator, len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }    
    memcpy(&ind, opt_indicator, len);
    
    SFF_MGR_DEBUG("port:%d opt_indicator arr:0x%x\n", sff_obj->port, opt_indicator[0]);
    //SFF_MGR_DEBUG("port:%d opt_indicator:0x%x\n", port, ind);
    switch(type)
    {
        case TX_EQ_TYPE:
            {    
                /*if(ind.tx_input_eq)*/
                {
                    
                    get_qsfp_field_addr(OPTIIONAL_CHANNEL_CONTROL_TX_EQ, &page, &offset, &len);
                    SFF_MGR_DEBUG("port:%d tx eq supported\n", sff_obj->port);

                }
            }
            break;
        case RX_EM_TYPE:
            {    
                {
                    
                    get_qsfp_field_addr(OPTIIONAL_CHANNEL_CONTROL_RX_EM, &page, &offset, &len);

                }
            }
            break;
        case RX_AM_TYPE:
            {    
                {
                    
                    get_qsfp_field_addr(OPTIIONAL_CHANNEL_CONTROL_RX_AM, &page, &offset, &len);

                }
            }
            break;
        default:
            goto exit_err;
            break;
    }    

    if(len != ARRAY_SIZE(ch_ctrl))
    {
        goto exit_err;
    }    
    if( qsfp_eeprom_read(sff_obj, page, offset, ch_ctrl, len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }
    
    *value = ((u32)ch_ctrl[0] << 8) | ((u32)ch_ctrl[1]);                     
    return 0;
    exit_err:
    return -1;
}
#if 0
/*Table 23- Module Global and Squelch Mode Controls (Lower Page, active modules only)
 * byte 26 bit 4 */
static int qsfp_dd_force_low_pwr(struct sff_obj_t *sff_obj, int en)
{
    u8 buf = 0; 
    int ret = 0;
    u8 page = QSFP_PAGE0;
    u8 offset = 26;
    if ((ret = qsfp_eeprom_read(sff_obj, page, offset, &buf, 1)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return ret;
    }
    buf = ret;
    if (en) {
        set_bit(4, (unsigned long *)&buf);
    } else {

        clear_bit(4, (unsigned long *)&buf);
    }

     if( (ret = qsfp_eeprom_write(sff_obj, page, offset, &buf, 1)) < 0)
     {
        SFF_MGR_ERR("write fail\n");
        return ret;
     }
    return 0;
}
#endif
/*(Page 11h, active modules only) 
 * app selected: offset 206-213 (Table 73)
 * tx control offset 214-221 (Table 74)
 * rx control offset 222-239 (Table 75)*/
static int qsfp_dd_active_control_set_indicator(struct sff_obj_t *sff_obj)
{
    u8 data[34];
    int ret = 0;
    int count = 0;
    int size = sizeof(data);
    if (!is_bank_num_valid(sff_obj)) {
        return -1;
    }
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE_11h, 206, data, size)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", sff_obj->port);
        return ret;
    }
    
    for (count = 0; count < size; count++) {
        
        //SFF_MGR_DEBUG("reg[%d]:0x%x\n", 206+count, data[count]);
    }

    return 0;
}    
/*Table 46- Implemented Signal Integrity Controls (Page 01h) 
 *
 * */
static int qsfp_dd_channel_control_get(struct sff_obj_t *sff_obj, int type, u32 *value)
{
    int ret = 0;
    u8 data[2];
    int port = sff_obj->port;
    if (!is_bank_num_valid(sff_obj)) {
        return -1;
    }
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE_01h, 161, data, 2)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return -1;
    }
   
    /*offset:161*/
    if (data[0] | TX_CDR_BIT) {
        
        SFF_MGR_DEBUG("port:%d Tx CDR implemented", port);
    }  
    if (data[0] | TX_CDR_BYPASS_BIT) {
        
        SFF_MGR_DEBUG("port:%d Tx CDR Bypass control implemented ", port);
    } 
    if (data[0] | TX_INPUT_FIX_MANUAL_BIT) {

        SFF_MGR_DEBUG("port:%d Tx Input Eq fixed manual", port);
    }  
    if (data[0] | ADAPTIVE_TX_INPUT_FIX_MANUAL_BIT) {

        SFF_MGR_DEBUG("port:%d Adaptive Tx Input Eq", port);
    }  
    if (data[0] | TX_INPUT_FREEZE_BIT) {

        SFF_MGR_DEBUG("port:%d Tx Input Eq Freeze", port);
    }  
    /*offset:162*/
    if (data[1] | RX_CDR_BIT) {

        SFF_MGR_DEBUG("port:%d Rx CDR implemented", port);
    }  
    if (data[1] | RX_CDR_BYPASS_BIT) {

        SFF_MGR_DEBUG("port:%d Rx CDR Bypass control implemented ", port);
    }  
    if (data[1] | RX_OUTPUT_AMP_CONTROL_BIT) {

        SFF_MGR_DEBUG("port:%d Rx output amp", port);
    }  
    if (data[1] | RX_OUTPUT_EQ_CONTROL_BIT) {

        SFF_MGR_DEBUG("port:%d Rx output eq", port);
    }  
    if (data[1] | STAGE_SET1_BIT) {

        SFF_MGR_DEBUG("port:%d stage_set1", port);
    }  

    return 0;
} 
int sfp_channel_status_get(struct sff_obj_t *sff_obj, int type, u8 *value)
{

    u8 pin = 0xff;
    int ret = 0; 
    switch(type)
    {
        case CH_STATUS_RX_LOS_TYPE:
            {    
                ret = sff_obj->mgr->drv->rx_los_get(sff_obj->port, &pin); 
            }
            break;
        case CH_STATUS_TX_FAULT_TYPE:
            {    
                ret = sff_obj->mgr->drv->tx_fault_get(sff_obj->port, &pin); 
            }
            break;
        default:
            goto exit_err;
            break;
    }    
    if (ret < 0) {
        goto exit_err;
    } else { 
        
        /*if ret == io_unsupported*/

        
        *value = pin; 
        
    }    
    return 0;

    exit_err:
    return -1;

}
int qsfp_lane_status_get(struct sff_obj_t *sff_obj, int type, u8 *st)
{
    *st = sff_obj->priv_data.qsfp.lane_st[type];
    return 0;
}    

int qsfp_lane_status_update(struct sff_obj_t *sff_obj, int type, u8 value)
{

    if (type > CH_STATUS_NUM ||
        type < CH_STATUS_RX_LOS_TYPE) {
        return -1;
    }    

    sff_obj->priv_data.qsfp.lane_st[type] = value;
    
    return 0;

}    
int qsfp_channel_status_get(struct sff_obj_t *sff_obj, int type, u8 *value)
{

    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    struct qsfp_interrupt_flag_t interrupt_flag;
    get_qsfp_field_addr(INTERRUPT_FLAG, &page, &offset, &len);
    if(len != sizeof(interrupt_flag))
    {
        
        SFF_MGR_ERR("size doesnt match\n");
        goto exit_err;
    }    
    if( qsfp_eeprom_read(sff_obj, page, offset, (u8 *)(&interrupt_flag), len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }    
    
    switch(type)
    {
        case CH_STATUS_RX_LOS_TYPE:
            {    
                *value = interrupt_flag.los_ind & 0x0f; 

            }
            break;
        case CH_STATUS_TX_LOS_TYPE:
            {    
                    
                *value = (interrupt_flag.los_ind & 0xf0) >> 4; 

            }
            break;
        case CH_STATUS_TX_FAULT_TYPE:
            {    
                    
                *value = interrupt_flag.eq_laser_fault & 0x0f;
            }
            break;
        default:
            goto exit_err;
            break;
    }    

    
    return 0;

    exit_err:
    return -1;

}    
static int qsfp_temperature_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size)
{
    
    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    u8 byte_data[WORD_SIZE];
    s16 temp_data = 0;
    union monitor_data_t monitor_data;

    s16 divider = 256;
    s16 decimal = 0; 
    char *unit = "c";
    
    get_qsfp_field_addr(TEMPERATURE, &page, &offset, &len);
                

    if(len != ARRAY_SIZE(byte_data))
    {
        goto exit_err;
    }    
    if( qsfp_eeprom_read(sff_obj, page, offset, byte_data, len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }

    monitor_data.byte[1] = byte_data[0];
    monitor_data.byte[0] = byte_data[1];
    temp_data = monitor_data.signed_word;
    
    decimal = ((temp_data/divider)*divider)-temp_data;
    decimal = abs(decimal);
    decimal = decimal*1000/divider;

    scnprintf(buf, buf_size, 
                            "%d.%d %s\n", 
                            temp_data/divider, 
                            decimal, unit);

    return 0;
    exit_err:
    return -1;
}
static int qsfp_voltage_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size)
{
    
    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    u8 byte_data[WORD_SIZE];
    u16 vol_data = 0;
    union monitor_data_t monitor_data;

    u16 divider = 10000;
    char *unit = "v";
    
    get_qsfp_field_addr(VCC, &page, &offset, &len);
                

    if(len != ARRAY_SIZE(byte_data))
    {
        goto exit_err;
    }    
    if( qsfp_eeprom_read(sff_obj, page, offset, byte_data, len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }

    monitor_data.byte[1] = byte_data[0];
    monitor_data.byte[0] = byte_data[1];
    vol_data = monitor_data.word;
    
    scnprintf(buf, buf_size, 
                            "%d.%d %s\n", 
                            vol_data/divider, 
                            vol_data%divider, unit);

    return 0;
    exit_err:
    return -1;
}
static int qsfp_channel_monitor_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size)
{
    
    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    int count = 0;
    int idx = 0;
    u8 ch_monitor[QSFP_CHANNEL_NUM*WORD_SIZE]; /* 2(u16 data) *4 (channel)*/
    union monitor_data_t monitor_data;
    u16 divider = 10000;
    char *unit = NULL;
    struct monitor_para_t *para = NULL;
    switch(type)
    {
        case CH_MONITOR_TX_PWR_TYPE:
            {    
                get_qsfp_field_addr(CHANNEL_TX_PWR, &page, &offset, &len);
            }
            break;
        case CH_MONITOR_RX_PWR_TYPE:
            {    
                get_qsfp_field_addr(CHANNEL_RX_PWR, &page, &offset, &len);
            }
            break;
        case CH_MONITOR_TX_BIAS_TYPE:
            {    
                get_qsfp_field_addr(CHANNEL_TX_BIAS, &page, &offset, &len);
            }
            break;
        default:
            goto exit_err;
            break;
    }    

    if(len != ARRAY_SIZE(ch_monitor))
    {
        goto exit_err;
    }    
    if( qsfp_eeprom_read(sff_obj, page, offset, ch_monitor, len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }
    
    para = monitor_para_find(type);
    if (IS_ERR_OR_NULL(para)) {
        goto exit_err;
    } 
    divider = para->divider;
    unit = para->unit;   
    len = QSFP_CHANNEL_NUM*WORD_SIZE;
    for (idx = 0; idx < len; idx += WORD_SIZE) {
        /*big edian*/
        monitor_data.byte[1] = ch_monitor[idx];
        monitor_data.byte[0] = ch_monitor[idx+1];
        
        count += scnprintf(buf+count, buf_size-count, 
                            "ch%d: %d.%d %s\n", 
                            (idx >> 1) + 1, monitor_data.word/divider, 
                            monitor_data.word%divider, unit);
    }
    
    return 0;
    exit_err:
    return -1;
}
/*Table 22- Module Monitors (Lower Page, active modules only)
 * offset 14,15*/ 
static int qsfp_dd_temperature_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size)
{
    
    u8 page = 0xff;
    u8 offset = 0;
    u8 byte_data[WORD_SIZE];
    s16 temp_data = 0;
    union monitor_data_t monitor_data;

    s16 divider = 256;
    s16 decimal = 0; 
    char *unit = "c";
    
    page = QSFP_PAGE0;
    offset = 14;    

    if( qsfp_eeprom_read(sff_obj, page, offset, byte_data, WORD_SIZE) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }

    monitor_data.byte[1] = byte_data[0];
    monitor_data.byte[0] = byte_data[1];
    temp_data = monitor_data.signed_word;
    
    decimal = ((temp_data/divider)*divider)-temp_data;
    decimal = abs(decimal);
    decimal = decimal*1000/divider;

    scnprintf(buf, buf_size, 
                            "%d.%d %s\n", 
                            temp_data/divider, 
                            decimal, unit);

    return 0;
    exit_err:
    return -1;
}
/*Table 22- Module Monitors (Lower Page, active modules only)
 * offset 16,17*/ 
static int qsfp_dd_voltage_get(struct sff_obj_t *sff_obj, u8 *buf, int buf_size)
{
    
    u8 page = 0xff;
    u8 offset = 0;
    u8 byte_data[WORD_SIZE];
    u16 vol_data = 0;
    union monitor_data_t monitor_data;

    u16 divider = 10000;
    char *unit = "v";
    
    page = QSFP_PAGE0;
    offset = 16;    
    
    if( qsfp_eeprom_read(sff_obj, page, offset, byte_data, WORD_SIZE) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }

    monitor_data.byte[1] = byte_data[0];
    monitor_data.byte[0] = byte_data[1];
    vol_data = monitor_data.word;
    
    scnprintf(buf, buf_size, 
                            "%d.%d %s\n", 
                            vol_data/divider, 
                            vol_data%divider, unit);

    return 0;
    exit_err:
    return -1;
}
/*1.7.7.3  Lane-Specific Monitors */
/*Table 70- Lane-Specific Monitors (Page 11h, active modules only) */
static int qsfp_dd_lane_monitor_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size)
{
    
    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    int count = 0;
    int idx = 0;
    u8 ch_monitor[QSFP_DD_CHANNEL_NUM*WORD_SIZE]; /* 2(u16 data) *4 (channel)*/
    union monitor_data_t monitor_data;
    u16 divider = 10000;
    char *unit = NULL;
    struct monitor_para_t *para = NULL;
    page = QSFP_PAGE_11h;
    len = ARRAY_SIZE(ch_monitor);
    switch(type)
    {
        case CH_MONITOR_TX_PWR_TYPE:
        {    
            offset = 154;     
        }
        break;
        case CH_MONITOR_RX_PWR_TYPE:
        {    
            offset = 186;     
        }
        break;
        case CH_MONITOR_TX_BIAS_TYPE:
        {    
            offset = 170;     
        }
        break;
        default:
        goto exit_err;
        break;
    }    

    if( qsfp_eeprom_read(sff_obj, page, offset, ch_monitor, len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }
    
    para = monitor_para_find(type);
    if (IS_ERR_OR_NULL(para)) {
        goto exit_err;
    } 
    divider = para->divider;
    unit = para->unit;   
    len = QSFP_CHANNEL_NUM*WORD_SIZE;
    for (idx = 0; idx < len; idx += WORD_SIZE) {
        /*big edian*/
        monitor_data.byte[1] = ch_monitor[idx];
        monitor_data.byte[0] = ch_monitor[idx+1];
        
        count += scnprintf(buf+count, buf_size-count, 
                            "ch%d: %d.%d %s\n", 
                            (idx >> 1) + 1, monitor_data.word/divider, 
                            monitor_data.word%divider, unit);
    }
    
    return 0;
    exit_err:
    return -1;
}
static int qsfp_vendor_info_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size)
{
    
    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    u8 byte_data[VENDOR_INFO_BUF_SIZE]; 
    
    switch(type)
    {
        case VENDOR_NAME_TYPE:
        {    
            get_qsfp_field_addr(VENDOR_NAME, &page, &offset, &len);
        }
        break;
        case VENDOR_PN_TYPE:
        {    
            get_qsfp_field_addr(PART_NUMBER, &page, &offset, &len);
        }
        break;
        case VENDOR_SN_TYPE:
        {    
            get_qsfp_field_addr(VENDOR_SERIAL_NUMBER, &page, &offset, &len);
        }
        break;
        case VENDOR_REV_TYPE:
        {    
            get_qsfp_field_addr(REVISION_NUMBER, &page, &offset, &len);
        }
        break;
        default:
            goto exit_err;
            break;
    }    
    /*use a big enough buf to handle all the vendor info*/
    if (len >= VENDOR_INFO_BUF_SIZE) {
        goto exit_err;
    }    
    
    if( qsfp_eeprom_read(sff_obj, page, offset, byte_data, len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }
    /*add terminal of string*/
    byte_data[len] = '\0';   
        
    scnprintf(buf, buf_size, "%s\n", byte_data);
    
    return 0;
    exit_err:
    return -1;
}
/*rev3 1.7.3.2*/
static int qsfp_dd_vendor_info_get(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size)
{
    
    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    u8 byte_data[VENDOR_INFO_BUF_SIZE]; 
    page = QSFP_PAGE0;
    switch(type)
    {
        case VENDOR_NAME_TYPE:
        {    
            offset = 129;
            len = 16;
        }
        break;
        case VENDOR_PN_TYPE:
        {    
            offset = 148;
            len = 16;
        }
        break;
        case VENDOR_SN_TYPE:
        {    
            offset = 166;
            len = 16;
        }
        break;
        case VENDOR_REV_TYPE:
        {    
            offset = 164;
            len = 2;
        }
        break;
        default:
            goto exit_err;
            break;
    }    
    /*use a big enough buf to handle all the vendor info*/
    if (len >= VENDOR_INFO_BUF_SIZE) {
        goto exit_err;
    }    
    
    if( qsfp_eeprom_read(sff_obj, page, offset, byte_data, len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }
    /*add terminal of string*/
    byte_data[len] = '\0';   
        
    scnprintf(buf, buf_size, "%s\n", byte_data);
    
    return 0;
    exit_err:
    return -1;
}
static int qsfp_cdr_control(struct sff_obj_t *sff_obj)
{
    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    u8 ext_id = 0;
    u8 cdr_ctrl = 0x00;
    get_qsfp_field_addr(EXTENDED_IDENTIFIER, &page, &offset, &len);
    if( qsfp_eeprom_read(sff_obj, page, offset, &ext_id, len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }    
    
    /*sff-8436 cdr prsL takes up bit 2,3 in  Extended Identifier Values
     *
     *
     *
     * */
    if (ext_id & 0x0c) //cdr tx/rx are prsL , enable cdr control
    {
        get_qsfp_field_addr(CDR_CONTROL, &page, &offset, &len);
        
        /*debugging purpose*/

         if(qsfp_eeprom_read(sff_obj, page, offset, &cdr_ctrl, len) < 0)
         {
            goto exit_err;
         }    
         
         SFF_MGR_DEBUG("default cdr:0x%x\n", cdr_ctrl);

         if(cdr_ctrl != 0xff)
         {    
             cdr_ctrl = 0xff;   
             if( qsfp_eeprom_write(sff_obj, page, offset, &cdr_ctrl, len) < 0)
             {
                SFF_MGR_ERR("cdr set fail\n");
                goto exit_err;
             }
         }
         SFF_MGR_DEBUG("OK\n");
    }    
    else
    {

        SFF_MGR_DEBUG("not supported:ext_id:0x%x\n", ext_id);
    }    
    return 0;  

    exit_err:
    return -1;    

}

static int sfp_tx_disable_set(struct sff_obj_t *sff_obj, u8 value)
{

    u8 slave_addr = 0xff;
    u8 offset = 0;
    int len = 0;
    u8 status = 0;
    int rec = 0;
    int port = sff_obj->port;
    if (IS_ERR_OR_NULL(sff_obj->mgr->drv->tx_disable_set)) {

        goto exit_err;    
    }   
     
    rec =  sff_obj->mgr->drv->tx_disable_set(sff_obj->port, value);
    if (rec < 0) {
        goto exit_err;
    }    
    /*io feature unsupported , control via eeprom instead*/
    if (REC_SFF_IO_UNSUPPORTED == rec) {  
        get_sfp_field_addr(STATUS_CONTROL, &slave_addr, &offset, &len);
        if( sfp_eeprom_read(sff_obj, slave_addr , offset, &status, len) < 0)
        {
          goto exit_err;   

        }
        
        SFF_MGR_DEBUG("io unsupported, port:%d status:0x%x\n", port, status);
        /*set bit 6:  Soft TX Disable Select*/
        if (value) {
            set_bit(6, (unsigned long *)&status);
        } else {
            clear_bit(6, (unsigned long *)&status);
        }    
        if( sfp_eeprom_write(sff_obj, slave_addr , offset, &status, len) < 0)
        {
            goto exit_err;
        }
        #ifdef READBACK_CHECK
        if( sfp_eeprom_read(sff_obj, slave_addr , offset, &status, len) < 0)
        {
          goto exit_err;   

        }
        SFF_MGR_DEBUG("port:%d status:0x%x\n", port, status);
        #endif
    }
    
    return 0;

    exit_err:
    return -1;    
}
static int sfp_tx_disable_get(struct sff_obj_t *sff_obj, u8 *value)
{

    u8 slave_addr = 0xff;
    u8 offset = 0;
    int len = 0;
    u8 status = 0;
    int rec = 0;
    
    if (IS_ERR_OR_NULL(sff_obj->mgr->drv->tx_disable_get)) {

        goto exit_err;    
    }   
     
    if (IS_ERR_OR_NULL(value)) {

        goto exit_err;    
    }   
    rec = sff_obj->mgr->drv->tx_disable_get(sff_obj->port, value);

    if (rec < 0) {
        goto exit_err;
    }    
    /*io feature unsupported , control via eeprom instead*/
    if (REC_SFF_IO_UNSUPPORTED == rec) {  
        get_sfp_field_addr(STATUS_CONTROL, &slave_addr, &offset, &len);
        if( sfp_eeprom_read(sff_obj, slave_addr , offset, &status, len) < 0)
        {
          goto exit_err;   

        }
        SFF_MGR_DEBUG("io unsupported, port:%d status:0x%x\n", sff_obj->port, status);
        /*get bit 6:  Soft TX Disable Select*/
        *value = (status >> 6) & 0x01;     
   
    }
    return 0;

    exit_err:
    return -1;    
}
static int qsfp_tx_disable_set(struct sff_obj_t *sff_obj, u8 value)
{
    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    u8 tx_disable = 0;
    int port = sff_obj->port; 
    get_qsfp_field_addr(CHANNEL_TX_DISABLE, &page, &offset, &len);
    
    if( qsfp_eeprom_read(sff_obj, page, offset, &tx_disable, len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }

    SFF_MGR_DEBUG("1.port:%d, tx_disable:%d\n", port, tx_disable);
    
    if( qsfp_eeprom_write(sff_obj, page, offset, &value, len) < 0)
    {
        SFF_MGR_ERR("NG2\n");
        goto exit_err;
    }
    #ifdef READBACK_CHECK
    if( qsfp_eeprom_read(sff_obj, page, offset, &tx_disable, len) < 0)
    {
        SFF_MGR_ERR("NG3\n");
        goto exit_err;
    }

    SFF_MGR_DEBUG("port:%d 2.tx_disable:%d\n", port, tx_disable);
    #endif

    return 0;

    exit_err:
    return -1;    
}
static int qsfp_tx_disable_get(struct sff_obj_t *sff_obj, u8 *value)
{
    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    u8 tx_disable = 0;
    get_qsfp_field_addr(CHANNEL_TX_DISABLE, &page, &offset, &len);
    
    if( qsfp_eeprom_read(sff_obj, page, offset, &tx_disable, len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }
    *value = tx_disable; 
    return 0;

    exit_err:
    return -1;    
}
static int tx_disable_set(struct sff_obj_t *sff_obj, u8 value)
{
    int type = sff_obj->type;
    if (SFP_TYPE == type) {
        return sfp_tx_disable_set(sff_obj, value);
    } else if (QSFP_TYPE == type) {
       
        return qsfp_tx_disable_set(sff_obj, value);
 
    } else { /*reserve for qsfp_dd*/

        return qsfp_dd_tx_disable_set(sff_obj, value);
    }
    return 0;    
}    

static int tx_disable_get(struct sff_obj_t *sff_obj, u8 *value)
{
    int type = sff_obj->type;
    
    if (SFP_TYPE == type) {
        return sfp_tx_disable_get(sff_obj, value);
    } else if (QSFP_TYPE == type) {
        return qsfp_tx_disable_get(sff_obj, value);
    } else { /*reserve for qsfp_dd*/

        return qsfp_dd_tx_disable_get(sff_obj, value);
    }
    return 0;    

}    
static int sfp_rate_select(struct sff_obj_t *sff_obj, u8 rate_bitmap)
{
    
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int len = 0;
    u8 status = 0;
    u8 ext_status = 0;
    int port = sff_obj->port; 
    if(rate_bitmap & SOFT_RX_RATE_RS0)
    {    
        get_sfp_field_addr(STATUS_CONTROL, &slave_addr, &offset, &len);
        if( sfp_eeprom_read(sff_obj, slave_addr , offset, &status, len) < 0)
        {
          goto exit_err;   

        }
        
        SFF_MGR_DEBUG("port:%d status:0x%x\n", port, status);
        /*set bit 3:  Soft Rate_Select 
         * Select * [aka. "RS(0)"] */
        set_bit(3, (unsigned long *)&status);
        if( sfp_eeprom_write(sff_obj, slave_addr , offset, &status, len) < 0)
        {
            goto exit_err;
        }
        #ifdef READBACK_CHECK
        if( sfp_eeprom_read(sff_obj, slave_addr , offset, &status, len) < 0)
        {
          goto exit_err;   

        }
        
        SFF_MGR_DEBUG("port:%d status:0x%x\n", port, status);
        #endif
    }
    if(rate_bitmap & SOFT_TX_RATE_RS1)
    {    
        get_sfp_field_addr(EXTENDED_STATUS_CONTROL, &slave_addr, &offset, &len);
        if( sfp_eeprom_read(sff_obj, slave_addr , offset, &ext_status, len) < 0)
        {
          goto exit_err;   

        }
        
        SFF_MGR_DEBUG("port:%d status:0x%x\n", port, ext_status);
        /*set bit 3:  Soft Rate_Select 
         * Select * [aka. "RS(0)"] */
        set_bit(3, (unsigned long *)&ext_status);
        if( sfp_eeprom_write(sff_obj, slave_addr , offset, &ext_status, len) < 0)
        {
            goto exit_err;
        }
        #ifdef READBACK_CHECK
        if( sfp_eeprom_read(sff_obj, slave_addr , offset, &ext_status, len) < 0)
        {
          goto exit_err;   

        }
        
        SFF_MGR_DEBUG("port:%d status:0x%x\n", port, status);
        #endif
    }


    return 0;
    exit_err:
    return -1;
}    
static int sfp_rate_select_control(struct sff_obj_t *sff_obj)
{
    
    u8 rate_id = 0;
    u8 slave_addr = 0xff;
    u8 offset = 0;
    int len = 0;
    bool found = false; 
    int port = sff_obj->port;
    /*read rate id*/

    get_sfp_field_addr(RATE_IDENTIFIER, &slave_addr, &offset, &len);
    if( sfp_eeprom_read(sff_obj, slave_addr , offset, &rate_id, len) < 0)
    {
        goto exit_err;
    }    
    
    SFF_MGR_DEBUG("port:%d rate_id:0x%x\n", port, rate_id);
    switch (rate_id) {
        case 0x00: /* Unspecified */
        case 0x03: /* Unspecified */
        case 0x05: /* Unspecified */
        case 0x07: /* Unspecified */
        case 0x09: /* Unspecified */
        case 0x0B: /* Unspecified */
        case 0x0D: /* Unspecified */
        case 0x0F: /* Unspecified */

        break;
        case 0x02: /* SFF-8431 (8/4/2G Rx Rate_Select only) */
        if(sfp_rate_select(sff_obj, SOFT_RX_RATE_RS0) < 0)
        {
            goto exit_err;
        }
        found = true;    
        break;
        case 0x04: /* SFF-8431 (8/4/2G Tx Rate_Select only) */
        if(sfp_rate_select(sff_obj, SOFT_TX_RATE_RS1) < 0)
        {
            goto exit_err;
        }
        found = true;    
        break;
        case 0x06: /* SFF-8431 (8/4/2G Independent Rx & Tx Rate_select) */
        if(sfp_rate_select(sff_obj, SOFT_RX_RATE_RS0|SOFT_TX_RATE_RS1) < 0)
        {
            goto exit_err;
        }
        found = true;    
        break;
        case 0x01: /* SFF-8079 (4/2/1G Rate_Select & AS0/AS1) */
        case 0x08: /* FC-PI-5 (16/8/4G Rx Rate_select only)
        * High=16G only, Low=8G/4G
        *                     */
        case 0x0A: /* FC-PI-5 (16/8/4G Independent Rx, Tx Rate_select)
        * High=16G only, Low=8G/4G
        *                     */
        case 0x0C: /* FC-PI-6 (32/16/8G Independent Rx, Tx Rate_Select)
        * High=32G only, Low = 16G/8G
        *                     */
        case 0x0E: /* 10/8G Rx and Tx Rate_Select controlling the operation or
        * locking modes of the internal signal conditioner, retimer
        *                     * or CDR, according to the logic table defined in Table 10-2,
        *                                         * High Bit Rate (10G) =9.95-11.3 Gb/s; Low Bit Rate (8G) =
        *                                                             * 8.5 Gb/s. In this mode, the default value of bit 110.3 (Soft
        *                                                                                 * Rate Select RS(0), Table 9-11) and of bit 118.3 (Soft Rate
        *                                                                                                     * Select RS(1), Table 10-1) is 1.
        *                                                                                                                         */
        default:
        break;
    } 
    if(!found)
    {
        SFF_MGR_DEBUG("port:%d no support\n", port);
    }    

    return 0;
    exit_err:
    return -1;
}    
static int qsfp_intL_get(struct sff_obj_t *sff_obj, u8 *value)
{
 
 return sff_obj->mgr->drv->intL_get(sff_obj->port, value);   

}    
static int lpmode_set(struct sff_obj_t *sff_obj, u8 value)
{
 
 return sff_obj->mgr->drv->lpmode_set(sff_obj->port, value);   

}    
static int lpmode_get(struct sff_obj_t *sff_obj, u8 *value)
{
 
 return sff_obj->mgr->drv->lpmode_get(sff_obj->port, value);   

}    
static int mode_sel_set(struct sff_obj_t *sff_obj, u8 value)
{
 
 return sff_obj->mgr->drv->mode_sel_set(sff_obj->port, value);   

}    
static int mode_sel_get(struct sff_obj_t *sff_obj, u8 *value)
{
 
 return sff_obj->mgr->drv->mode_sel_get(sff_obj->port, value);   

}    
static int qsfp_lpmode_control(struct sff_obj_t *sff_obj)
{
    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    u8 ext_id = 0;
    u8 power_class = 0xff;
    u8 lpmode = 0;
    int port = sff_obj->port;
    get_qsfp_field_addr(EXTENDED_IDENTIFIER, &page, &offset, &len);
    if( qsfp_eeprom_read(sff_obj, page, offset, &ext_id, len) < 0)
    {
        SFF_MGR_ERR("NG1\n");
        goto exit_err;
    }   
    /*sff-8436 power class takes up bit 6,7 in  Extended Identifier Values
     *
     *
     *
     * */

    power_class = ext_id >> 6; 
    SFF_MGR_DEBUG("port:%d power_class:0x%x\n", port, power_class);
   
    switch(power_class)
    {
        case POWER_CLASS_1_MODULE:/*lower power mode*/
            lpmode = 1;
            break;    
        case POWER_CLASS_2_MODULE:
        case POWER_CLASS_3_MODULE:
        case POWER_CLASS_4_MODULE:
        default:
            lpmode = 0;
            break;    
        break;    
    }    
    if(lpmode_set(sff_obj, lpmode) < 0)
    {
        goto exit_err;
    }

    /*debugging check*/
    lpmode = 0xff;
    if(lpmode_get(sff_obj, &lpmode) < 0)
    {

        goto exit_err;
    }

    SFF_MGR_DEBUG("port:%d lpmode read:0x%x\n", port, lpmode);
    
    return 0;  

    exit_err:
    return -1;    

}
static void sff_fsm_state_change_process(struct sff_obj_t *sff_obj, 
                                        sff_fsm_state_t cur_st,   
                                        sff_fsm_state_t next_st)
{


    if (cur_st != next_st)
    {
        
        sff_fsm_delay_cnt_reset(sff_obj, next_st); 
        //SFF_MGR_DEBUG("port:%d st change:%d -> %d\n", 
          //            port, st,sff_fsm_st_get(sff_obj)); 
        SFF_MGR_INFO("port:%d st change:%s -> %s\n", 
                      sff_obj->port, sff_fsm_st_str[cur_st], 
                      sff_fsm_st_str[next_st]); 
    
       switch(next_st)
       {
           case SFF_FSM_ST_FAULT:
                //i2c_err_handle_task_invoke();
                //SFF_MGR_ERR("port:%d transvr will be isolated\n", port);
                break; 
           default:
            break;     
       }    
    
    
    }    

}    
static int sff_fsm_sfp_task(struct sff_obj_t *sff_obj)
{
    sff_fsm_state_t st = sff_fsm_st_get(sff_obj);
    int ret = 0;
    switch (st) {

    case SFF_FSM_ST_IDLE:
        break;     
    case SFF_FSM_ST_REMOVED:
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_IDLE);        
        break;
    case SFF_FSM_ST_INSERTED:
        {
            u8 is_ready = 0;
            ret = _sfp_data_ready_check(sff_obj, &is_ready);
            if (ret < 0)  
            {
                break;
            }
            
            if(is_ready) {
                sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTING);
            }  
        }
        break;

    case SFF_FSM_ST_DETECTING:
        {
            u8 is_found = 0;
            if((ret = sfp_tx_disable_set(sff_obj, TX_1CH_DISABLE_ON)) < 0)
            {
                break;
            }
            if ((ret = sfp_rate_select_control(sff_obj)) < 0) {
                break;
            }
            if((ret = sfp_type_identify(sff_obj, &is_found)) < 0)
            {    
                break;
            }
            
            if(is_found)
            {
                sff_fsm_st_set(sff_obj, SFF_FSM_ST_INIT);
            }
            else
            {
              /*unknown type: special handling*/
                
                sff_fsm_st_set(sff_obj, SFF_FSM_ST_UNKNOWN_TYPE);
            }    

        }
        break;    

    case SFF_FSM_ST_DETECTED:

        break;
    case SFF_FSM_ST_ISOLATED:

        break;
    case SFF_FSM_ST_INIT:
        if((ret = sfp_tx_disable_set(sff_obj, TX_ALL_CH_DISABLE_OFF)) < 0)
        {
            break;
        }
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTED);
        break;
    case SFF_FSM_ST_FAULT:
        break;    
    case SFF_FSM_ST_UNKNOWN_TYPE:     
    
        break;
    default:
        SFF_MGR_ERR("unknown fsm st:%d\n", st);
        break;

    }    
    
    if(ret < 0)
    {
        if (!ioexp_is_channel_ready()){
            SFF_MGR_ERR("i2c_crush port:%d\n", sff_obj->port);
            return (-1);
        }
    }    
    sff_fsm_state_change_process(sff_obj, st, sff_fsm_st_get(sff_obj));
    return 0;
}   
#if 0 /*<TBD>*/
/*6.2.3  Interrupt Flags Bytes 3-21 consist of interrupt flags for LOS, TX Fault, warnings and alarms.*/
/*TABLE 6-4  CHANNEL STATUS INTERRUPT FLAGS (PAGE 00H BYTES 3-5) */
/*offset 3 , 0-4: rxlos; 4-7 txlos*/
/*offset 4 , 0-4: txfault*/
static int qsfp_int_flag_read(struct sff_obj_t *sff_obj)
{
    int ret = 0;
    int i = 0;
    u8 buf = 0;
    u8 data = 0;
    u8 reg[17];

    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE0, 0x03, &buf, 1)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return ret;
    }       
    data = masked_bits_get(buf, 0, 4);
    //SFF_MGR_DEBUG("port:%d rxlos 0x%x", port, data);
    qsfp_lane_status_update(sff_obj, CH_STATUS_RX_LOS_TYPE, data);
    //if (data) {
        SFF_MGR_DEBUG("port:%d rxlos 0x%x", port, data);
    //}
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE0, 0x04, &buf, 1)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return ret;
    }       
    data = masked_bits_get(buf, 0, 4);
    //SFF_MGR_DEBUG("port:%d txfault 0x%x", port, data);
    
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE0, 0x05, reg, 17)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return ret;
    }       
    
    for (i = 0; i < 17; i++) {
        //SFF_MGR_DEBUG("port:%d reg[%d] 0x%x", port, i+5, reg[i]);
    }
    
    return 0;
}
#endif
static int sff_fsm_qsfp_task(struct sff_obj_t *sff_obj)
{
    sff_fsm_state_t st = sff_fsm_st_get(sff_obj);
    int ret = 0;
    switch (st) {

    case SFF_FSM_ST_IDLE:
        break;     
    case SFF_FSM_ST_REMOVED:
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_IDLE);        
        break;
    case SFF_FSM_ST_INSERTED:
        {
            u8 is_ready = 0;
            ret = _qsfp_data_ready_check(sff_obj, &is_ready);
            if (ret < 0)  
            {
                break;
            }
            
            if(is_ready) {
                sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTING);
            }  
        }
        break;

    case SFF_FSM_ST_DETECTING:
        {
            u8 is_found = 0;
            if ((ret = cur_page_update(sff_obj)) < 0) {
                break;
            }
            if ((ret = qsfp_tx_disable_set(sff_obj, TX_4CH_DISABLE_ON)) < 0) {
            
                break;
            }
            if((ret = qsfp_type_identify(sff_obj, &is_found)) < 0)
            {    
                break;
            }
            
            if(is_found)
            {
                sff_fsm_st_set(sff_obj, SFF_FSM_ST_INIT);
            }
            else
            {
              /*unknown type: special handling*/
                sff_fsm_st_set(sff_obj, SFF_FSM_ST_UNKNOWN_TYPE);
            }    

        }
        break;    

    case SFF_FSM_ST_ISOLATED:

        break;
    case SFF_FSM_ST_DETECTED:
        {
#if defined (QSFP_INT_FLAG_SUPPORT)
            u8 intL = 0;
            if ((ret = qsfp_intL_get(sff_obj, &intL)) < 0) {
                break;
            }
            if (!intL) {
                if ((ret = qsfp_int_flag_read(sff_obj)) < 0) {
                    break;
                }
            }
#endif     
        }
        break;
    case SFF_FSM_ST_INIT:
        {
            /*cdr control*/
            if((ret = qsfp_cdr_control(sff_obj)) < 0)
            {
                break;
            }

            /*low power mode control*/
            if((ret = qsfp_lpmode_control(sff_obj)) < 0)
            {
                break;

            }    
           /*tx_disable control*/
            
            if((ret = qsfp_tx_disable_set(sff_obj, TX_ALL_CH_DISABLE_OFF)) < 0)
            {
                break;
            }
            sff_fsm_kobj_change_event(sff_obj);    
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTED);
        }
        break;

    case SFF_FSM_ST_RESET_ASSERTED:

        break;
    case SFF_FSM_ST_RESET_DEASSERTED:
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_INSERTED);        
        break;
    case SFF_FSM_ST_FAULT:/*i2c bus fail ... etc*/

        break;   
    case SFF_FSM_ST_UNKNOWN_TYPE:     
    
        break;
    default:
        SFF_MGR_ERR("unknown fsm st:%d\n", st);
        break;

    }    
    
    if(ret < 0)
    {
        if (!ioexp_is_channel_ready()){
            SFF_MGR_ERR("i2c_crush port:%d\n", sff_obj->port);
            return (-1);
        }
    }    

    sff_fsm_state_change_process(sff_obj, st, sff_fsm_st_get(sff_obj));
    return 0;
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
               if (!ioexp_is_channel_ready()){
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
        ret = sff_obj->mgr->drv->eeprom_read(sff_obj->port, addr, offset, buf, len);         

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
   /*due to pca9548 mux driver will remember the channel, even mux is reset , the selected channel is not reset
    * so read the data of that channel after mux is reset will fail , so need to switch to other channels first */ 
   
    /*phase 0 loop is used to switch channels to avoid the problem above*/ 
    for (sff_obj = 1; port < port_num; port+=8) { 
        buf = 0;      
        
        if (SFF_FSM_ST_ISOLATED != sff_fsm_st_get(sff_obj)) 
        {
            ret = eeprom_read_retry(sff_obj, addr, 0, &buf, 1);         
            
            if (ret >= 0) {
                
                if (buf == 0x03 || buf == 0x11) {
                        
                    //SFF_MGR_ERR("transvr[%d]:read! ok buf:0x%x ret:%d\n", port, buf, ret);
                    continue;
                }
             }
                //SFF_MGR_ERR("transvr[%d]:read! buf:0x%x ret:%d\n", port, buf, ret);
                if (!ioexp_is_channel_ready())
                {
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
        
        if (SFF_FSM_ST_ISOLATED != sff_fsm_st_get(sff_obj)) 
        {
            ret = eeprom_read_retry(sff_obj, addr, 0, &buf, 1);         
            if (ret >= 0) {
                if (buf == 0x03 || buf == 0x11) {
                        
                    //SFF_MGR_ERR("transvr[%d]:read! ok buf:0x%x ret:%d\n", port, buf, ret);
                    continue;
                }
             }
            //if (buf != 0x03 || ret < 0) {
                //SFF_MGR_ERR("transvr[%d]:read! buf:0x%x ret:%d\n", port, buf, ret);
                if (!ioexp_is_channel_ready())
                {
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
    if (SFF_FSM_ST_DETECTED == sff_fsm_st_get(transvr_port)) 
    {
        ret = Sff.drv->eeprom_read(transvr_port, addr, 0, &buf, 1);         
        if (ret < 0) {
           if (!ioexp_is_channel_ready()){
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
static int sff_fsm_run(struct sff_mgr_t *sff)
{
    int port = 0;
    int ret = 0;
    struct sff_obj_t *sff_obj = NULL;
    int port_num = sff->port_num;

    for(port = 0; port < port_num; port++)
    {
        sff_obj = &(sff->obj[port]);
        if (sff_fsm_delay_cnt_is_hit(sff_obj))
        {    
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
static u8 masked_bits_get(u8 val, int bit_s, int bit_e)
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

/*Table 18- Identifier and Status Summary (Lower Page) byte3 , 3-1 Module state*/ 
static int qsfp_dd_module_st_get(struct sff_obj_t *sff_obj, u8 *st)
{
    u8 data = 0;
    int ret = 0;
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE0, 0x03, &data, 1)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", sff_obj->port);
        return ret;
    }       
    
    *st = masked_bits_get(data, 1, 3);
    return 0;
#if 0
    struct qsfp_dd_id_status_t data;
    if (qsfp_dd_id_status_get(sff_obj, &data) < 0)
    {
        return -1;
    }
    *value = data.offset3.module_state;    
    return 0;
#endif    

}    
/*value : the status of 8 lane, one lane: 4 bits*/
/*table 65 page 11h offset 128~131*/
static int qsfp_dd_data_path_st_get(struct sff_obj_t *sff_obj, u32 *st)
{
    u32 data = 0;   
    
    if(IS_ERR_OR_NULL(st))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if(qsfp_eeprom_read(sff_obj, QSFP_PAGE_11h, 128, (u8*)&data, 4) < 0)
    {
        SFF_MGR_ERR("qsfp_eeprom_read fail");
        *st = 0; 
        return -1;
    }
    *st = data;

    return 0;

}
static bool qsfp_dd_is_data_path_activated(struct sff_obj_t *sff_obj)
{
    u32 data_path_st;
    u32 st_lane = 0;
    int ln = 0;
    bool is_active = false; 
    int lane_num = valid_lane_num_get(sff_obj); 
    
    if (qsfp_dd_data_path_st_get(sff_obj, &data_path_st) < 0) {
        return is_active;
    }    
    /*TBD check 8 lane as default*/
    for (ln = 0; ln < lane_num; ln++)
    {
        st_lane = data_path_st & 0x0f;
        if (DATA_PATH_ACTIVATED_ST_ENCODE != st_lane) {
            break;
        }
        st_lane = st_lane >> 4;
    }
    if (ln >= lane_num)
    {
        is_active = true; 
    }     
    return is_active;
}    


/*Table 18- Identifier and Status Summary (Lower Page) 
 * lower page :offset 0　*/
static int qsfp_dd_id_get(struct sff_obj_t *sff_obj, u8 *val)
{
    u8 buf = 0;
    int ret = 0;
    
    if (!val) {

        return -1;
    }
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE0, 0, &buf, 1)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", sff_obj->port);
        return ret;
    }       
    *val = buf;
    return 0;

        
}   
#if 0
/*Table 18- Identifier and Status Summary (Lower Page) (read only)*/
static int qsfp_dd_id_status_get(struct sff_obj_t *sff_obj, struct qsfp_dd_id_status_t *data)
{

    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    struct qsfp_dd_id_status_t tmp;
    if (IS_ERR_OR_NULL(data)) {

       return -1;     
    }   

    get_qsfp_field_addr(ID_STATUS_SUMMARY, &page, &offset, &len);
    if (len != sizeof(tmp)) {
        SFF_MGR_ERR("size doesn't match\n");
        return -1;
    }        
    if (qsfp_eeprom_read(sff_obj, page, offset, (u8 *)(&tmp), len) < 0) {
        return -1;
    }
    memcpy(data, &tmp, len);
    return 0;
}
#endif
#if 0
/*
 * Table 21- Module  Flags (Lower Page, active modules only) 
 * byte 8 bit0 : L-Module state changed, refer to table 3 
 */
static int qsfp_dd_module_state_is_changed(struct sff_obj_t *sff_obj, bool *is_changed)
{

    u8 page = QSFP_PAGE0;
    u8 offset = 8;
    u8 val = 0;
    int ret = 0;
    if ((ret = qsfp_eeprom_read(sff_obj, page, offset, &val, 1)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return ret;
    }       
    *is_changed = ((val&0x1) ? true : false);
    return 0;



#if 0
    u8 page = 0xff;
    u8 offset = 0;
    int len = 0;
    struct qsfp_dd_module_flags_t flags;
    
    if (IS_ERR_OR_NULL(is_changed)) {

       return -1;     
    }   

    get_qsfp_field_addr(MODULE_FLAGS, &page, &offset, &len);
    if (len != sizeof(flags)) {
        SFF_MGR_ERR("size doesn't match\n");
        return -1;
    }        
    if (qsfp_eeprom_read(sff_obj, page, offset, (u8 *)(&flags), len) < 0)
    {
        return -1;
    }
    *is_changed = (flags.state_change_flag ? true : false); 
    return 0;
#endif    
    
}

/*table 18 : offset 3 bit 0*/
static int qsfp_dd_digital_int_get(struct sff_obj_t *sff_obj, u8 *value)
{
    u8 st = 0;
    int ret = 0;
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE0, 0x03, &st, 1)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return ret;
    }       
    *value = st & 0x1;
    return ret;

}
#endif
static int qsfp_dd_intL_get(struct sff_obj_t *sff_obj, u8 *value)
{
    return sff_obj->mgr->drv->intL_get(sff_obj->port, value);
}   
static bool is_bank_num_valid(struct sff_obj_t *sff_obj)
{
    u8 bank_no = 0xff;
    bool valid = false;
    /*check if it's bank0? */
    if (qsfp_eeprom_read(sff_obj, QSFP_PAGE0, 126, &bank_no, 1) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", sff_obj->port);
        return valid;
    }       
    if (0x00 == bank_no) {

        valid = true;
    } else {
        SFF_MGR_ERR("port:%d bank no is not 0", sff_obj->port);
    } 
    
    return valid;
}
#if 0
/*table 36: byte 212 :that defines aspects of the device or cable technology*/
static int qsfp_dd_media_interface_tech_get(struct sff_obj_t *sff_obj, u8 *interface)
{

    u8 data = 0;
    
    if(IS_ERR_OR_NULL(interface))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }

    if(qsfp_eeprom_read(sff_obj, QSFP_PAGE0, 212, &data, 1) < 0)
    {
        SFF_MGR_ERR("qsfp_eeprom_read fail");
        *interface = 0xff; /*non define*/
        return -1;
    }
    *interface = data;

    return 0;

}
#endif
static bool is_loopback_module(struct sff_obj_t *sff_obj) 
{
    /*<TBD> need check more loopback modules*/
    if (0 == sff_obj->priv_data.qsfp_dd.module_type) {

        return true;
    }
    return false;
}
/*table 26*: lower page:00*/ 
static int app_advertising_field_get(struct sff_obj_t *sff_obj)
{
    u8 module_type = 0;
    union qsfp_dd_app_advert_fields *fields = sff_obj->priv_data.qsfp_dd.fields;
    int ret = 0;
    int apsel = 0;
    int i = 0;
    bool end_of_table = false;
    int port = sff_obj->port;
    
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE0, 85, &module_type, 1)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return ret;
    }
    
    SFF_MGR_DEBUG("port:%d module_type:0x%x\n", port, module_type);
    sff_obj->priv_data.qsfp_dd.module_type = module_type;
    for (apsel = APSEL_1; apsel < APSEL_NUM; apsel++) {
       for (i = 0; i < APP_ADVERT_FIELD_NUM; i++) { 
       
           if ((ret =qsfp_eeprom_read(sff_obj, app_advert_fields_page[i], 
                                       app_advert_fields_offset[apsel][i], 
                                       &fields[apsel].reg[i], 1)) < 0) {

                SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
                return ret;
            }
            if (ret == 0xff) {
                end_of_table = true; 
                break;
            }
       }
       if (end_of_table) {
            break;
       }

    }
    
    for (apsel = 0; apsel < APSEL_NUM; apsel++) {
        SFF_MGR_DEBUG("port:%d host_electrical_interface_code[%d]:0x%x\n", 
                     port, apsel, fields[apsel].data.host_electrical_interface_code);
        SFF_MGR_DEBUG("port:%d module_media_interface_code[%d]:0x%x\n", 
                    port, apsel, fields[apsel].data.module_media_interface_code);
        SFF_MGR_DEBUG("port:%d lane_count[%d]:0x%x\n", 
                    port, apsel, fields[apsel].data.lane_count);
        SFF_MGR_DEBUG("port:%d  host_lane_assignment_options[%d]:0x%x\n", 
                    port, apsel, fields[apsel].data.host_lane_assignment_options);
    }    
    return 0;
}   
#if 0
static int app_advert_field_dump(struct sff_obj_t *sff_obj, u8 *buf, int buf_size)
{
    int apsel = APSEL_1;   
    int ret = 0;
    union qsfp_dd_app_advert_fields *fields = Sff.obj[port].priv_data.qsfp_dd.fields;
    /*<TBD> the field data will be cached when module is plugged in , so may not need to capture again in the future*/   
    ret = app_advertising_field_get(sff_obj);
    if (ret < 0) {
        return ret;
    }
    scnprintf(buf, buf_size,
               "module_type 0x%x \
               host electrical intf 0x%x \
               module media intf 0x%x  \
               host_media lane count %d \
               host lane assignment 0x%x \
               media lane assignment 0x%x\n",
              Sff.obj[port].priv_data.qsfp_dd.module_type,
              fields[apsel].data.host_electrical_interface_code,
              fields[apsel].data.module_media_interface_code,
              fields[apsel].data.lane_count,
              fields[apsel].data.host_lane_assignment_options,
              fields[apsel].data.media_lane_assignment_options
              
              ); 
    return 0; 

}
#endif
/*TBD lan number can be changed*/
static int valid_lane_num_get(struct sff_obj_t *sff_obj)
{

    int num = 8;
    return num;

}    
/*note: so far only page >= 0x10 , need to check bank number*/
static int stage_control_set0(struct sff_obj_t *sff_obj)
{
    u8 apply_stage_control_set = 0;
    u8 stage_set_offset_begin = 145;
    u8 stage_set_offset_end = stage_set_offset_begin + valid_lane_num_get(sff_obj) - 1;
    u8 offset = 0;
    struct stage_set_t stage_set;   
    if (!is_bank_num_valid(sff_obj)) {

        return -1;
    }
    /*table 56 stage control set 0, application select controls (Page 10h, active modules only) */
    /*case1: normal case 400G lane = 8*/ 
    stage_set.explicit_control = 0;
    stage_set.datapath_code = 0x00; 
    stage_set.app_code = 0x01;

    for (offset = stage_set_offset_begin; offset <= stage_set_offset_end; offset++) {
        if(qsfp_eeprom_write(sff_obj, QSFP_PAGE_10h, offset, (u8*)(&stage_set), 1) < 0) {
            return -1;
        }
    }
    /*table 55 Apply_DataPathInit*/  
    apply_stage_control_set = 0xff;
    if(qsfp_eeprom_write(sff_obj, QSFP_PAGE_10h, 143, &apply_stage_control_set, 1) < 0)
    {
        return -1;
    }
    return 0;
} 

/*qsfp-dd used only*/

/*page 11 
 * table 71/72 : active modules only
 * Table 72- Configuration Error Codes
 * para:
 * port(in) 
 * is_pass(out)*/
static int config_error_code_check(struct sff_obj_t *sff_obj, bool *is_pass)
{
    u8 data[4];
    int lane = 0;
    int i = 0;
    /*since the one reg reprsL 2 lanes*/
    int lane_num = valid_lane_num_get(sff_obj);
    u8 config_err_code[lane_num]; /*lane number 8*/
    int port = sff_obj->port;

    if (NULL == is_pass) {
        return -1;
    }
    *is_pass = false; 
    if (!is_bank_num_valid(sff_obj)) {
        return -1;
    }
    if (qsfp_eeprom_read(sff_obj, QSFP_PAGE_11h, 202, data, 4) < 0) {
        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", sff_obj->port);
        return -1;
    }
    lane = 0;
    for (i = 0; i < 4; i++) {
        config_err_code[lane++] = data[i] & 0xf;
        config_err_code[lane++] = (data[i] & 0xf0) >> 4;
    }
    
    for (lane = 0; lane < lane_num; lane++) {
        
        SFF_MGR_DEBUG("port:%d lane:%d err_code:0x%x", port, lane, config_err_code[lane]);
        if (CONFIG_ACCEPTED == config_err_code[lane]) {
        } else {
            break;
        }
    }
    if (lane >= lane_num) {
        *is_pass = true;
    }
    return 0;
}   
/*table 54 page 10h offset 130 */
static int qsfp_dd_tx_disable_set(struct sff_obj_t *sff_obj, u8 val)
{
    
    if (!is_bank_num_valid(sff_obj)) {

        return -1;
    }  
    if(qsfp_eeprom_write(sff_obj, QSFP_PAGE_10h, 130, &val, 1) < 0)
    {
        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", sff_obj->port);
        return -1;
    }

    return 0;
}  
/*table 54 page 10h offset 130 */
static int qsfp_dd_tx_disable_get(struct sff_obj_t *sff_obj, u8 *val)
{
    u8 buf = 0;
    if (!val) {
        return -1;
    }
    if (!is_bank_num_valid(sff_obj)) {

        return -1;
    }  
    if(qsfp_eeprom_read(sff_obj, QSFP_PAGE_10h, 130, &buf, 1) < 0)
    {
        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", sff_obj->port);
        return -1;
    }
    *val = buf;
    return 0;
}  
/*table 53 */
static int data_path_power_up(struct sff_obj_t *sff_obj, bool up)
{

    u8 value = 0;

    if (up) {
        value = 0xff;
    }
    if (!is_bank_num_valid(sff_obj)) {

        return -1;
    }  
    if(qsfp_eeprom_write(sff_obj, QSFP_PAGE_10h, 128, &value, 1) < 0)
    {
        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", sff_obj->port);
        return -1;
    }

    return 0;
}
/*Table 18- Identifier and Status Summary (Lower Page) offset 2 bit 7:Flat_mem*/
static int qsfp_dd_is_flat_mem(struct sff_obj_t *sff_obj, bool *is_flat)
{
    u8 buf = 0;
    u8 offset = 2;
    
    if(IS_ERR_OR_NULL(is_flat))
    {
        SFF_MGR_ERR("invalid para");
        return -1;
    }
    if (qsfp_eeprom_read(sff_obj, QSFP_PAGE0, offset, &buf, 1) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", sff_obj->port);
        *is_flat = false;
        return -1;
    }
     
    if (buf & 0x80) {
        *is_flat = true;
    } else {
        *is_flat = false;
    }
    return 0;
}

/*optical: true copper: false*/
static bool is_copper(struct sff_obj_t *sff_obj)
{
    bool is_copper = false;
    bool is_flat = false;
    /*there are serveral ways to check if it's optic module
     *1. we may check if it's flat memory-->  passive copper cables*/
    if (qsfp_dd_is_flat_mem(sff_obj, &is_flat) < 0)
    {
       return is_copper; 
    }
    if (is_flat) {
        is_copper = true;  
    }    
    return is_copper;
} 
#if 0
/*<TBD> so far it's used to de-assert- intL*/ 
static int qsfp_dd_interrupt_flags_read(struct sff_obj_t *sff_obj)
{
    int ret = 0;
    u8 module_flags[4];
    u8 lane_flags[19];
    /*table 15 module flags*/ 
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE0, 8, module_flags, 4)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return ret;
    }
    /* table 16 Lane-Specific Flag*/ 
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE_11h, 134, lane_flags, 19)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return ret;
    }

    return 0;
}
#endif
static char *module_flag_str[] = {
    
    "FW_FAULT",
    "L_VCC_TEMP_WARN",
    "L_AUX_ALARM_WARN",
    "VENDOR_DEFINED_ERR",
}; 

static char *lane_tx_flag_str[] = {
    
    "L_TX_FAULT",
    "L_TX_LOS",
    "L_TX_CDR_LOL",
    "L_TX_APAP_EQ_INPUT_FAULT",
    "L_TX_POWER_HIGH_ALARM",
    "L_TX_POWER_LOW_ALARM",
    "L_TX_POWER_HIGH_WARN",
    "L_TX_POWER_LOW_WARN",
    "L_TX_BIAS_HIGH_ALARM",
    "L_TX_BIAS_LOW_ALARM",
    "L_TX_BIAS_HIGH_WARN",
    "L_TX_BIAS_LOW_WARN",

};
static char *lane_rx_flag_str[] = {
    
    "L_RX_LOS",
    "L_RX_CDR_LOL",
    "L_RX_POWER_HIGH_ALARM",
    "L_RX_POWER_LOW_ALARM",
    "L_RX_POWER_HIGH_WARN",
    "L_RX_POWER_LOW_WARN",
}; 

/*table 15*/
static int qsfp_dd_module_flag_monitor(struct sff_obj_t *sff_obj)
{

    int ret = 0;
    int i = 0;
    u8 module_flag[4];
    int port = sff_obj->port;
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE0, 8, module_flag, 4)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return ret;
    }
    for (i = 0; i < 4; i++) {

        if (module_flag[i] != 0x00) {

            SFF_MGR_ERR("port:%d error %s[%d]:0x%x", port, module_flag_str[i], i+8, module_flag[i]);

        }
    } 

    return 0;


}   
/*Table 68 TX Flags , 69- RX Flags (Page 11h, active modules only)*/
static int qsfp_dd_lane_flag_monitor(struct sff_obj_t *sff_obj)
{

    int ret = 0;
    int i = 0;
    u8 rx_flag[6];
    u8 tx_flag[12];
    u8 data_path_st_change = 0;
    int port = sff_obj->port;
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE_11h, 134, &data_path_st_change, 1)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return ret;
    }
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE_11h, 135, tx_flag, 12)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return ret;
    }
    
    if ((ret = qsfp_eeprom_read(sff_obj, QSFP_PAGE_11h, 147, rx_flag, 6)) < 0) {

        SFF_MGR_ERR("port:%d qsfp_eeprom_read fail", port);
        return ret;
    }
    SFF_MGR_DEBUG("port:%d  data path st change:0x%x\n", port, data_path_st_change);
    for (i = 0; i < 12; i++) {

        if (tx_flag[i] != 0x00) {

            SFF_MGR_ERR("port:%d error %s[%d]:0x%x", port, lane_tx_flag_str[i], i+135, tx_flag[i]);

        }
    } 
    for (i = 0; i < 6; i++) {

        if (rx_flag[i] != 0x00) {

            SFF_MGR_ERR("port:%d error %s[%d]:0x%x", port, lane_rx_flag_str[i], i+147, rx_flag[i]);

        }
    } 

    return 0;


}    
static int sff_fsm_qsfp_dd_task(struct sff_obj_t *sff_obj)
{
    sff_fsm_state_t st = sff_fsm_st_get(sff_obj);
    int ret = 0;
    switch (st) {

    case SFF_FSM_ST_IDLE:
        break;     
    case SFF_FSM_ST_REMOVED:
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_IDLE);        
        break;
    case SFF_FSM_ST_INSERTED:
        {
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_MGMT_INIT);
        }    
        break;

    case SFF_FSM_ST_MGMT_INIT:
        {
            u8 value = 0;
            //u8 intL = 0;
            //bool is_changed = false;
            u8 lpmode = 0;
            /*
             * module make a transition
            *check if Software Init mode 
             * AND (Module Mgmt 
             * Interface ready OR 
             * t_init timeout) (See 
             * Hardware 
 
             * Specification) */
            /*note: active module only , how about loopback and copper module? */ 
#if 0 
            if (qsfp_dd_intL_get(sff_obj, &intL) < 0)
            {
                break;
            }    
            if (intL) {
                
                break;
            }
    
            if (qsfp_dd_module_state_is_changed(sff_obj, &is_changed) < 0) {
                break;
            }

            if(!is_changed) {    
                break;
            }
#endif
            /*check module state register if it transit to module low power state*/
            if ((ret = qsfp_dd_module_st_get(sff_obj, &value)) < 0) {
               
                break;
            }    
            //SFF_MGR_DEBUG("modele_st[%d]:%d\n", port, value);
            if (is_copper(sff_obj)) {
                sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTED);   
            } else {
            
                if ((ret = lpmode_get(sff_obj, &lpmode)) < 0) {
                    break;
                }
                if (lpmode) {
                    
                    if (MODULE_LOW_PWR_ST_ENCODE == value) 
                    {
                        sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_LOW_PWR_INIT);
                    }   
                } else {
                    sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_HW_INIT);   
                }       
            }
        }    
        break;
    case SFF_FSM_ST_MODULE_LOW_PWR_INIT:
        {
            /*Host requests an 
             * action that requires 
             * High Power mode 
            ex:
            -init mode set to 0 (low power mode = 0)
            -tx_disable false
            */   
            /*1) read module info advertised application control (table 26)*/
            ret = qsfp_dd_active_control_set_indicator(sff_obj);
            if (ret < 0) {
                break;
            }
            if ((ret = app_advertising_field_get(sff_obj)) < 0) {
                /*print error code*/
                break;
            }
            if (is_loopback_module(sff_obj)) {

                if ((ret = data_path_power_up(sff_obj, true)) < 0) {
                    break;
                }       
                sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_LOOPBACK_INIT);   
            } else {
                /* host select application*/
                /*stage control set 0*/
                if ((ret = stage_control_set0(sff_obj)) < 0) {
                    /*print error code*/
                    break;
                }
                sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_LOW_PWR_CONFIG_CHECK);  
            } 
        }    
        break;
    case SFF_FSM_ST_MODULE_LOW_PWR_CONFIG_CHECK:
        {
            bool is_pass = false;
            if ((ret = config_error_code_check(sff_obj, &is_pass)) < 0) {

                break;
            }    
        
            if (is_pass) {

                sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_LOW_PWR_CONTROL);   
            }
        }    
        break;
    case SFF_FSM_ST_MODULE_LOW_PWR_CONTROL:
        {
            /*set tx_disable true to avoid link flapping*/
            if ((ret = qsfp_dd_tx_disable_set(sff_obj, TX_8CH_DISABLE_ON)) < 0)
            {
                break;
            }    
             /*Host sets the 
             * DataPathPwrUp bits for 
             * all host lanes in the 
             * data path(s) that it 
             * wants to enable (See 
             * Table 53, Page 10h 
             * Register 128)*/
            
            if ((ret = data_path_power_up(sff_obj, true)) < 0) {
                break;
            }       
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_WAITING_READY);   
        }    
        break;
    case SFF_FSM_ST_MODULE_HW_INIT:
    case SFF_FSM_ST_MODULE_LOOPBACK_INIT:
        {
            u8 module_st;
            
            if ((ret = qsfp_dd_module_st_get(sff_obj, &module_st)) < 0) {
                break;
            }
            /*check module state register if it transit to module ready*/
            if (MODULE_READY_ST_ENCODE == module_st) {
                sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTED);        
            }    
        }
        break;
    case SFF_FSM_ST_MODULE_WAITING_READY:
        {
            u8 module_st;
            //bool is_changed = false;
            //u8 intL = 0;
            if ((ret = qsfp_dd_module_st_get(sff_obj, &module_st)) < 0) {
                break;
            }
#if 0 /*<TBD> should we use intL in the future*/   
            if (qsfp_dd_digital_int_get(sff_obj ,&intL) < 0) {
                
                break;
            }
            /*intL stays high (de-assert)*/
            if (intL) {
                break;
            }
            
            /*go down here when intL is asserted*/ 
            if (qsfp_dd_module_state_is_changed(sff_obj, &is_changed) < 0) {
                break;
            }
            if(is_changed) {    
                /*print log*/    
                SFF_MGR_DEBUG("module st change st:%d\n", module_st);
            }    
#endif
            /*check module state register if it transit to module ready*/
            if (MODULE_READY_ST_ENCODE == module_st && 
                qsfp_dd_is_data_path_activated(sff_obj)) {
                sff_fsm_st_set(sff_obj, SFF_FSM_ST_MODULE_READY);        
            }    
        }    
        break;
    case SFF_FSM_ST_MODULE_READY:
        {
            /*set tx_disable false, */
            if ((ret = qsfp_dd_tx_disable_set(sff_obj, TX_ALL_CH_DISABLE_OFF)) < 0)
            {
                break;
            }    
            sff_fsm_st_set(sff_obj, SFF_FSM_ST_DETECTED);        
        }    
        break;
    case SFF_FSM_ST_DETECTED:
        { 
            u8 intL = 0;
            if (int_flag_monitor_en) {
                if (!is_copper(sff_obj)) { 
                    if ((ret = qsfp_dd_intL_get(sff_obj, &intL)) < 0)
                    {
                        break;
                    }    
                    if (!intL) {
                        if ((ret = qsfp_dd_module_flag_monitor(sff_obj)) < 0) {
                            break;
                        }
                        if ((ret = qsfp_dd_lane_flag_monitor(sff_obj)) < 0 ) {
                            break;
                        }
                    }
                }
            }
        }
        break;
    case SFF_FSM_ST_RESET_ASSERTED:
    {
    }
        break;
    case SFF_FSM_ST_RESET_DEASSERTED:
        sff_fsm_st_set(sff_obj, SFF_FSM_ST_INSERTED);        
        break;
    case SFF_FSM_ST_FAULT:/*i2c bus fail ... etc*/

        break;   
    case SFF_FSM_ST_UNKNOWN_TYPE:     
    
        break;
    default:
        SFF_MGR_ERR("unknown fsm st:%d\n", st);
        break;

    }    
    
    if(ret < 0)
    {
        if (!ioexp_is_channel_ready()){
            SFF_MGR_ERR("i2c_crush port:%d\n", sff_obj->port);
            return (-1);
        }
    }    

    sff_fsm_state_change_process(sff_obj, st, sff_fsm_st_get(sff_obj));
    return 0;
}    
static void sff_fsm_init(struct sff_obj_t *obj, int type)
{
    int port = 0;
    int port_num = 0;
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
            if ((ret = sff->drv->reset_set(sff_obj->port, 0)) < 0) {
                return ret;
            }
            msleep(RESET_ASSERT_TIME);
            /*de-assert resetL*/   
            if ((ret = sff->drv->reset_set(sff_obj->port, 1)) < 0) {
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
}    
static int __init sff_init(void)
{
    
    if (port_info_table_load() < 0) {
        
        SFF_MGR_ERR("port_info_table_load fail\n");
        goto exit_err;
    }    
    if (sff_io_init() < 0) {

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
    if (sff_driver_register(&sffMgr, &Sff_Drv) < 0) {

        goto exit_sff_kset_deinit;
        SFF_MGR_ERR("sff_driver_register fail\n");
    }

    if (sff_obj_create(&sffMgr, maxPortNum) < 0) {

        goto exit_sff_kset_deinit;
            
    }
    
    if(sff_objs_init(&sffMgr, Port_Info_Tbl) < 0) {
        goto exit_free_sff_objs;
    }
    if (sff_module_init_seq(&sffMgr) < 0) {

        goto exit_free_sff_objs;
    }
    if (sff_kobj_init_create(&sffMgr) < 0) {

        goto exit_free_sff_objs;
    }       
    
    if(sff_polling_is_enabled())
    {    
        sff_polling_task_start();
    }
    SFF_MGR_INFO("sff:%d  init ok\n", PLATFORM_NAME);
    return 0;    
    exit_free_sff_objs:
    sff_objs_destroy(&sffMgr);
    exit_sff_kset_deinit:
    sff_kset_deinit();
    
    exit_err:
    return -1;
}

static void __exit sff_exit(void)
{
    
    if(sff_polling_is_enabled())
    {    
         sff_polling_task_stop();
    }    
    
    sff_eeprom_deinit();
    sff_io_deinit();
    sff_kobjs_destroy(&sffMgr);
    sff_parent_kobj_destroy(&sffMgr);
    sff_objs_destroy(&sffMgr);

    sff_kset_deinit();

    SFF_MGR_INFO("sff %d deinit ok\n", PLATFORM_NAME);
}


module_init(sff_init);
module_exit(sff_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alang <huang.alang@inventec.com>");
