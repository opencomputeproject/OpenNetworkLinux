#ifndef __SFF_H
#define __SFF_H

#define READBACK_CHECK
#define DEBUG_LOG (0)

#define SFF_KSET ("swps")
#define TRY_NUM (5)
#define SFF_EEPROM_I2C_ADDR (0xA0 >> 1)
#define I2C_RETRY_DELAY (10)
#define WORD_SIZE (2)
#define SFP_CHANNEL_NUM (1)
#define QSFP_CHANNEL_NUM (4)
#define VENDOR_INFO_BUF_SIZE (32)

#define QSFP_DD_CHANNEL_NUM (4)

#define SFF_POLLING_PERIOD    (msecs_to_jiffies(100))  /* msec */

#define DYNAMIC_KOBJ_SUPPORT (1)

#define PAGE_NUM  (256)
#define EEPROM_SIZE (256)
#define EEPROM_HALF_SIZE (128)
//#define QSFP_INT_FLAG_SUPPORT

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
    /*qsfp-dd only {*/
    SFF_FSM_ST_MGMT_INIT,
    SFF_FSM_ST_MODULE_HW_INIT,
    SFF_FSM_ST_MODULE_LOOPBACK_INIT,
    SFF_FSM_ST_MODULE_LOW_PWR_INIT,
    SFF_FSM_ST_MODULE_LOW_PWR_CONFIG_CHECK,
    SFF_FSM_ST_MODULE_LOW_PWR_CONTROL,
    SFF_FSM_ST_MODULE_WAITING_READY,
    SFF_FSM_ST_MODULE_READY,
    SFF_FSM_ST_MODULE_PWR_DOWN,
    /*qsfp-dd only }*/
    SFF_FSM_ST_UNKNOWN_TYPE,
    SFF_FSM_ST_END,

}sff_fsm_state_t;

typedef enum
{
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

}tx_disable_t;

/*so far it's for sfp only*/
typedef enum
{
    SOFT_RX_RATE_RS0 = 1 << 0,
    SOFT_TX_RATE_RS1 = 1 << 1,

}rate_control_t;

typedef enum
{
    TX_EQ_TYPE,
    RX_EM_TYPE,
    RX_AM_TYPE,

}sysfs_attrbute_index_ch_control_t;


typedef enum
{
    CH_MONITOR_RX_PWR_TYPE,
    CH_MONITOR_TX_PWR_TYPE,
    CH_MONITOR_TX_BIAS_TYPE,

}sysfs_attrbute_index_ch_monitor_t;
typedef enum
{
    CH_STATUS_RX_LOS_TYPE,
    CH_STATUS_TX_LOS_TYPE,
    CH_STATUS_TX_FAULT_TYPE,
    CH_STATUS_NUM,

}sysfs_attrbute_index_ch_status_t;

typedef enum
{
    VENDOR_NAME_TYPE,
    VENDOR_PN_TYPE,
    VENDOR_SN_TYPE,
    VENDOR_REV_TYPE,

}sysfs_attrbute_index_vendor_info_t;

typedef enum
{
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
    /* Transceiver class for Copper */
    TRANSVR_CLASS_COPPER_L1_1G      =(28001),
    TRANSVR_CLASS_COPPER_L1_10G     =(28011),
    TRANSVR_CLASS_COPPER_L4_10G     =(28012),
    TRANSVR_CLASS_COPPER_L1_25G     =(28021),
    TRANSVR_CLASS_COPPER_L4_40G     =(28041),
    TRANSVR_CLASS_COPPER_L4_100G    =(28101),

}transvr_type_t;
typedef enum {
    I2C_CRUSH_INIT_ST,
    I2C_CRUSH_IO_I2C_CHECK_ST,
    I2C_CRUSH_BAD_TRANSVR_DETECT_ST,
    I2C_CRUSH_I2C_RECHECK_ST,
    I2C_CRUSH_END_ST,

}i2c_crush_hande_st;

#endif /* __SFF_H */


