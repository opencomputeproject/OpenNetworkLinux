#ifndef __SFF_H
#define __SFF_H

#include "sff_spec.h"

#define READBACK_CHECK
#define DEBUG_LOG (0)

#define SFF_KSET ("swps")
#define TRY_NUM (5)
#define SFF_EEPROM_I2C_ADDR (0xA0 >> 1)
#define SFF_DDM_I2C_ADDR (0xA2 >> 1)
#define I2C_RETRY_DELAY (10)
#define WORD_SIZE (2)
#define SFP_LANE_NUM (1)
#define QSFP_LANE_NUM (4)
#define QSFP_DD_LANE_NUM (8)
#define VENDOR_INFO_BUF_SIZE (32)


#define SFF_POLLING_PERIOD    (msecs_to_jiffies(100))  /* msec */

#define DYNAMIC_KOBJ_SUPPORT (1)

#define PAGE_NUM  (256)
#define EEPROM_SIZE (256)
#define EEPROM_HALF_SIZE (128)
#define QSFP_INT_FLAG_SUPPORT

#if (DEBUG_LOG == 1)
#define SFF_MGR_DEBUG(fmt, args...) \
    printk (KERN_INFO "[SFF_MGR]%s: " fmt "\r\n",__FUNCTION__,  ##args)
#else
#define SFF_MGR_DEBUG(fmt, args...)
#endif
#define SFF_MGR_INFO(fmt, args...) printk (KERN_INFO "[SFF_MGR]%s:"fmt,__FUNCTION__,  ##args)
#define SFF_MGR_ERR(fmt, args...)  printk (KERN_ERR "{SFF_MGR]%s: " fmt "\r\n",__FUNCTION__,  ##args)

typedef enum {
    SFP_TYPE = 0,
    QSFP_TYPE,
    QSFP_DD_TYPE,
    SFF_TYPE_NUM
} sff_type;
#if 0
typedef enum {
    TX_ALL_CH_DISABLE_OFF = 0x00,
    TX_CH1_DISABLE_ON = 1 << 0,
    TX_CH2_DISABLE_ON = 1 << 1,
    TX_CH3_DISABLE_ON = 1 << 2,
    TX_CH4_DISABLE_ON = 1 << 3,
    TX_CH5_DISABLE_ON = 1 << 4,
    TX_CH6_DISABLE_ON = 1 << 5,
    TX_CH7_DISABLE_ON = 1 << 6,
    TX_CH8_DISABLE_ON = 1 << 7,
    TX_1CH_DISABLE_ON = TX_CH1_DISABLE_ON, /*sfp*/
    TX_4CH_DISABLE_ON = 0x0f, /*qsfp*/
    TX_8CH_DISABLE_ON = 0xff, /*qsfp-dd*/

} tx_disable_t;
#endif
/*so far it's for sfp only*/
typedef enum {
    SOFT_RX_RATE_RS0 = 1 << 0,
    SOFT_TX_RATE_RS1 = 1 << 1,

} rate_control_t;

typedef enum {
    TX_EQ_TYPE,
    RX_EM_TYPE,
    RX_AM_TYPE,

} sysfs_attrbute_index_ln_control_t;


typedef enum {
    LN_MONITOR_RX_PWR_TYPE,
    LN_MONITOR_TX_PWR_TYPE,
    LN_MONITOR_TX_BIAS_TYPE,

} sysfs_attrbute_index_ln_monitor_t;
typedef enum {
    LN_STATUS_RX_LOS_TYPE,
    LN_STATUS_TX_LOS_TYPE,
    LN_STATUS_TX_FAULT_TYPE,
    LN_STATUS_NUM,

} sysfs_attrbute_index_ln_status_t;

typedef enum {
    VENDOR_NAME_TYPE,
    VENDOR_PN_TYPE,
    VENDOR_SN_TYPE,
    VENDOR_REV_TYPE,

} sysfs_attrbute_index_vendor_info_t;

typedef enum {

    SFF_FSM_ST_REMOVED = 0,
    SFF_FSM_ST_INSERTED,
    SFF_FSM_ST_DETECTING,
    SFF_FSM_ST_INIT,
    SFF_FSM_ST_DETECTED,
    SFF_FSM_ST_IDLE,
    SFF_FSM_ST_FAULT,
    SFF_FSM_ST_RESET_ASSERTED,
    SFF_FSM_ST_RESET_DEASSERTED,
    SFF_FSM_ST_ISOLATED,
    SFF_FSM_ST_IDENTIFY,
    SFF_FSM_ST_MONITOR,
    /*qsfp-dd only {*/
    SFF_FSM_ST_MGMT_INIT,
    SFF_FSM_ST_MODULE_HW_INIT,
    SFF_FSM_ST_MODULE_LOOPBACK_INIT,
    SFF_FSM_ST_MODULE_READY,
    SFF_FSM_ST_MODULE_PWR_DOWN,
    SFF_FSM_ST_MODULE_CMIS_VER_CHECK,
    SFF_FSM_ST_MODULE_ADVERT_CHECK,
    SFF_FSM_ST_MODULE_SW_CONFIG,
    SFF_FSM_ST_MODULE_SW_CONFIG_CHECK,
    SFF_FSM_ST_MODULE_SW_CONTROL,
    SFF_FSM_ST_MODULE_READY_CHECK,
    /*qsfp-dd only }*/
    SFF_FSM_ST_UNKNOWN_TYPE,
    SFF_FSM_ST_END,
    SFF_FSM_ST_NUM,

} sff_fsm_state_t;
struct qsfp_dd_priv_data {

    int lane_num;
    u8 module_type;
    union qsfp_dd_app_advert_fields fields[APSEL_NUM];
};
struct qsfp_priv_data {
    u8 lane_st[LN_STATUS_NUM];
    u8 eeprom_cache[PAGE_NUM][EEPROM_SIZE];
};

union priv_data_t {

    struct qsfp_dd_priv_data qsfp_dd;
    struct qsfp_priv_data qsfp;
};
struct sff_kobj_t {
    struct kobject kobj;
    int port;
    struct sff_obj_t *sff_obj;
};

typedef struct sff_obj_t sff_obj_type;

struct sff_fsm_t {
    sff_fsm_state_t st;
    int (*task)(sff_obj_type *sff_obj);
    int cnt; /*used to count how many fsm loop's been running*/
    int delay_cnt; /*the target count for each state, will be reset during each state transition*/
    struct fsm_period_t *period_tbl;
};

struct qsfp_dd_fsm_func_t {

    int (*advert_check)(struct sff_obj_t *sff_obj, bool *pass);
    int (*sw_config)(struct sff_obj_t *sff_obj);
    int (*sw_config_check)(struct sff_obj_t *sff_obj, bool *pass);
    int (*sw_control)(struct sff_obj_t *sff_obj);
    int (*module_ready_check)(struct sff_obj_t *sff_obj, bool *ready);

};
struct sff_obj_t {
    int port;
    sff_type type;
    int transvr_type;
    char *name;
    struct sff_kobj_t *kobj;
    union priv_data_t priv_data;
    struct sff_fsm_t fsm;
    struct sff_mgr_t *mgr;
    struct func_tbl_t *func_tbl;
    struct qsfp_dd_fsm_func_t *dd_fsm_func;   
};

struct sff_mgr_t {
    int port_num;
    unsigned long prsL;
    struct sff_obj_t *obj;
    struct sff_kobj_t *mgr_kobj;
    struct sff_kobj_t *common_kobj;
    int (*prsL_scan)(struct sff_mgr_t *sff);
};
/*the driver structure contains the api doing hw dependent functions (IO + eeprom)*/ 
struct sff_driver_t {
    int (*prsL_all_get)(unsigned long *bitmap);
    int (*intr_all_get)(unsigned long *bitmap);
    int (*oc_all_get)(unsigned long *bitmap);
    int (*prsL_get)(int port, u8 *prsL);
    int (*intL_get)(int port, u8 *value);
    int (*rx_los_get)(int port, u8 *value);
    int (*tx_fault_get)(int port, u8 *value);
    int (*reset_set)(int port, u8 reset);
    int (*reset_get)(int port, u8 *reset);
    int (*lpmode_set)(int port, u8 value);
    int (*lpmode_get)(int port, u8 *value);
    int (*tx_disable_set)(int port, u8 value);
    int (*tx_disable_get)(int port, u8 *value);
    int (*mode_sel_set)(int port, u8 value);
    int (*mode_sel_get)(int port, u8 *value);
    int (*eeprom_read)(int port,
                       u8 slave_addr,
                       u8 offset,
                       u8 *buf,
                       size_t len);
    int (*eeprom_write)(int port,
                        u8 slave_addr,
                        u8 offset,
                        const u8 *buf,
                        size_t len);
    int (*io_mux_reset_all)(int value);

};

/* func table for each sff object*/
struct func_tbl_t {

    /* private function , for driver internal use {*/
    int (*eeprom_read)(struct sff_obj_t *sff_obj, u8 page_addr, u8 offset, u8 *buf, int len);
    int (*eeprom_write)(struct sff_obj_t *sff_obj, u8 page_addr, u8 offset, u8 *buf, int len);
    /* }*/ 
    /*sff low speed signals for both driver internal use and export to sysfs interface*/
    int (*prsL_get)(struct sff_obj_t *sff_obj, u8 *value);
    int (*lpmode_set)(struct sff_obj_t *sff_obj, u8 value);
    int (*lpmode_get)(struct sff_obj_t *sff_obj, u8 *value);
    int (*reset_set)(struct sff_obj_t *sff_obj, u8 value);
    int (*reset_get)(struct sff_obj_t *sff_obj, u8 *value);
    int (*mode_sel_set)(struct sff_obj_t *sff_obj, u8 value);
    int (*mode_sel_get)(struct sff_obj_t *sff_obj, u8 *value);
    int (*intL_get)(struct sff_obj_t *sff_obj, u8 *value);
    int (*tx_disable_set)(struct sff_obj_t *sff_obj, u8 value);
    int (*tx_disable_get)(struct sff_obj_t *sff_obj, u8 *value);
    int (*rx_los_get)(struct sff_obj_t *sff_obj, u8 *value);
    int (*tx_fault_get)(struct sff_obj_t *sff_obj, u8 *value);
    /*following function is only for export to sysfs interface*/
    int (*temperature_get)(struct sff_obj_t *sff_obj, u8 *buf, int buf_size);
    int (*voltage_get)(struct sff_obj_t *sff_obj, u8 *buf, int buf_size);
    int (*lane_control_set)(struct sff_obj_t *sff_obj, int type, u32 value);
    int (*lane_control_get)(struct sff_obj_t *sff_obj, int type, u32 *value);
    int (*lane_monitor_get)(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size);
    int (*vendor_info_get)(struct sff_obj_t *sff_obj, int type, u8 *buf, int buf_size);
    int (*lane_status_get)(struct sff_obj_t *sff_obj, int type, u8 *value);
    int (*module_st_get)(struct sff_obj_t *sff_obj, u8 *st);
    int (*id_get)(struct sff_obj_t *sff_obj, u8 *id);
    int (*eeprom_dump)(struct sff_obj_t *sff_obj, u8 *buf);
    int (*page_sel)(struct sff_obj_t *sff_obj, int page);
    int (*page_get)(struct sff_obj_t *sff_obj, u8 *page);
};
struct port_info_map_t {
    int port;
    sff_type type;
    char *name;
};
struct port_info_table_t {
    struct port_info_map_t *map;
    int size;
};
/*it is for compose monitor data into u16 format from u8*/
union monitor_data_t {
    u8 byte[2];
    u16 word;
    s16 signed_word;
};

struct monitor_para_t {
    int type;
    u16 divider;
    char *unit;
};

typedef enum {
    TRANSVR_CLASS_UNKNOWN=0,
    /* Transceiver class for Optical 10G */
    TRANSVR_CLASS_OPTICAL_10G_S_AOC =(27011),
    TRANSVR_CLASS_OPTICAL_10G_S_SR  =(27012),
    TRANSVR_CLASS_OPTICAL_10G_S_LR  =(27013),
    TRANSVR_CLASS_OPTICAL_10G_S_ER  =(27014),
    TRANSVR_CLASS_OPTICAL_10G_Q_AOC =(27015),
    TRANSVR_CLASS_OPTICAL_10G_Q_SR  =(27016),
    TRANSVR_CLASS_OPTICAL_10G_Q_LR  =(27017),
    TRANSVR_CLASS_OPTICAL_10G_Q_ER  =(27018),
    /* Transceiver class for Optical 25G */
    TRANSVR_CLASS_OPTICAL_25G_AOC   =(27021),
    TRANSVR_CLASS_OPTICAL_25G_SR    =(27022),
    TRANSVR_CLASS_OPTICAL_25G_LR    =(27023),
    TRANSVR_CLASS_OPTICAL_25G_ER    =(27024),
    /* Transceiver class for Optical 40G */
    TRANSVR_CLASS_OPTICAL_40G_AOC   =(27041),
    TRANSVR_CLASS_OPTICAL_40G_SR4   =(27042),
    TRANSVR_CLASS_OPTICAL_40G_LR4   =(27043),
    TRANSVR_CLASS_OPTICAL_40G_ER4   =(27044),
    /* Transceiver class for Optical 100G */
    TRANSVR_CLASS_OPTICAL_100G_AOC  =(27101),
    TRANSVR_CLASS_OPTICAL_100G_SR4  =(27102),
    TRANSVR_CLASS_OPTICAL_100G_LR4  =(27103),
    TRANSVR_CLASS_OPTICAL_100G_ER4  =(27104),
    TRANSVR_CLASS_OPTICAL_100G_PSM4 =(27105),
    TRANSVR_CLASS_OPTICAL_100G_CWDM4 =(27106),
    /* Transceiver class for Copper */
    TRANSVR_CLASS_COPPER_L1_1G      =(28001),
    TRANSVR_CLASS_COPPER_L1_10G     =(28011),
    TRANSVR_CLASS_COPPER_L4_10G     =(28012),
    TRANSVR_CLASS_COPPER_L1_25G     =(28021),
    TRANSVR_CLASS_COPPER_L4_40G     =(28041),
    TRANSVR_CLASS_COPPER_L4_100G    =(28101),

} transvr_type_t;
typedef enum {
    I2C_CRUSH_INIT_ST,
    I2C_CRUSH_IO_I2C_CHECK_ST,
    I2C_CRUSH_BAD_TRANSVR_DETECT_ST,
    I2C_CRUSH_I2C_RECHECK_ST,
    I2C_CRUSH_END_ST,

} i2c_crush_hande_st;

void transvr_type_set(struct sff_obj_t *sff_obj, int type);
int transvr_type_get(struct sff_obj_t *sff_obj);
u8 masked_bits_get(u8 val, int bit_s, int bit_e);
sff_fsm_state_t sff_fsm_st_get(struct sff_obj_t *sff_obj);
void sff_fsm_st_set(struct sff_obj_t *sff_obj, sff_fsm_state_t st);
void sff_fsm_state_change_process(struct sff_obj_t *sff_obj, sff_fsm_state_t cur_st, sff_fsm_state_t next_st);

struct monitor_para_t *monitor_para_find(int type);
int sff_fsm_kobj_change_event(struct sff_obj_t *sff_obj);
#endif /* __SFF_H */


