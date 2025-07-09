#ifndef SWITCH_TRANSCEIVER_DRIVER_H
#define SWITCH_TRANSCEIVER_DRIVER_H

#include "switch.h"

#define TRANSCEIVER_ERR(fmt, args...)      LOG_ERR("transceiver: ", fmt, ##args)
#define TRANSCEIVER_WARNING(fmt, args...)  LOG_WARNING("transceiver: ", fmt, ##args)
#define TRANSCEIVER_INFO(fmt, args...)     LOG_INFO("transceiver: ", fmt, ##args)
#define TRANSCEIVER_DEBUG(fmt, args...)    LOG_DBG("transceiver: ", fmt, ##args)

#define TRANSCEIVER_NAME_STRING "transceiver"
#define TRANSCEIVER_TOTAL_NUM   32
#define LSW_CPLD0_CS            1
#define LSW_CPLD1_CS            2

#define QSFP_POWER_ON1           0x40

#define QSFP_RST1                0x38

#define QSFP_LED                 0xA0

#define QSFP_LPWN1               0x30
#define QSFP_PRESENT1            0x20
#define QSFP_INTR1               0x10

#define TRANSCEIVER_IDENTIIFIER_OFFSET             0x0
#define TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET    0x80

#define SFP_DDM_TYPE_OFFSET       92
#define SFP_DDM_TYPE_MONITOR_IMPLEMENTED_OFFSET (1<<6)
#define SFP_TEMPERATURE_OFFSET    352
#define SFP_VOLTAGE_OFFSET        354
#define SFP_BIAS_OFFSET           356
#define SFP_TX_POWER_OFFSET       358
#define SFP_RX_POWER_OFFSET       360
#define SFP_LANE_NUM                        1
#define SFP_RX_LOS_OFFSET                   366
#define SFP_RX_LOS_BITMAP           (0x02)
#define SFP_RX_LOS_MASK             0x1
#define SFP_RX_LOS_BIT_OFFSET       1
#define SFP_TX_DISABLE_OFFSET               366
#define SFP_TX_DISABLE_BITMAP       (0x80)
#define SFP_SOFT_TX_DISABLE_BITMAP (0x40)
#define SFP_TX_DISABLE_MASK         0x1
#define SFP_TX_DISABLE_BIT_OFFSET   8
#define SFP_TX_CDR_LOL_OFFSET               375
#define SFP_TX_CDR_LOL_BITMAP       (0x02)
#define SFP_TX_CDR_LOL_MASK         0x1
#define SFP_TX_CDR_LOL_BIT_OFFSET   1
#define SFP_RX_CDR_LOL_OFFSET               375
#define SFP_RX_CDR_LOL_BITMAP       (0x01)
#define SFP_RX_CDR_LOL_MASK         0x1
#define SFP_RX_CDR_LOL_BIT_OFFSET   0
#define SFP_TX_FAULT_OFFSET                 366
#define SFP_TX_FAULT_BITMAP         (0x04)
#define SFP_TX_FAULT_MASK           0x1
#define SFP_TX_FAULT_BIT_OFFSET     2

#define QSFP_POWER_CONTROL_OFFSET                93
#define QSFP_POWER_CONTROL_POWER_OVERRIDE_BIT    (1<<0)
#define QSFP_POWER_CONTROL_POWER_SET_BIT         (1<<1)
#define QSFP_POWER_CONTROL_ENABLE_CLASS_5_7_BIT  (1<<2)
#define QSFP_POWER_CONTROL_ENABLE_CLASS_8_BIT    (1<<3)
#define QSFP_TEMPERATURE_OFFSET                  22
#define QSFP_VOLTAGE_OFFSET                      26
#define QSFP_BIAS_OFFSET                         42
#define QSFP_TX_POWER_OFFSET                     50
#define QSFP_RX_POWER_OFFSET                     34
#define QSFP_LANE_NUM                            4
#define QSFP_TX_LOS_OFFSET                       3
#define QSFP_TX_LOS_BITMAP              (0xF0)
#define QSFP_TX_LOS_MASK                0xF
#define QSFP_TX_LOS_BIT_OFFSET          4
#define QSFP_RX_LOS_OFFSET                       3
#define QSFP_RX_LOS_BITMAP              (0x0F)
#define QSFP_RX_LOS_MASK                0xF
#define QSFP_RX_LOS_BIT_OFFSET          0
#define QSFP_TX_DISABLE_OFFSET                   86
#define QSFP_TX_DISABLE_BITMAP          (0x0F)
#define QSFP_TX_DISABLE_MASK            0xF
#define QSFP_TX_DISABLE_BIT_OFFSET      0
#define QSFP_RX_DISABLE_OFFSET                   625
#define QSFP_RX_DISABLE_BITMAP          (0xF0)
#define QSFP_RX_DISABLE_MASK            0xF
#define QSFP_RX_DISABLE_BIT_OFFSET      4
#define QSFP_TX_CDR_LOL_OFFSET                   5
#define QSFP_TX_CDR_LOL_BITMAP          (0xF0)
#define QSFP_TX_CDR_LOL_MASK            0xF
#define QSFP_TX_CDR_LOL_BIT_OFFSET      4
#define QSFP_RX_CDR_LOL_OFFSET                   5
#define QSFP_RX_CDR_LOL_BITMAP          (0x0F)
#define QSFP_RX_CDR_LOL_MASK            0xF
#define QSFP_RX_CDR_LOL_BIT_OFFSET      0
#define QSFP_TX_FAULT_OFFSET                     4
#define QSFP_TX_FAULT_BITMAP            (0x0F)
#define QSFP_TX_FAULT_MASK              0xF
#define QSFP_TX_FAULT_BIT_OFFSET        0
#define QSFP_EXT_IDENTIFIER_OFFSET                  129
#define QSFP_EXT_IDENTIFIER_POWER_CLASS_8_BITMAP    (0x20)
#define QSFP_EXT_IDENTIFIER_POWER_CLASS_1_4_BITMAP  (0xC0)
#define QSFP_EXT_IDENTIFIER_POWER_CLASS_5_7_BITMAP  (0x03)

#define QSFP_DD_MODULE_GLOBAL_CONTROL_OFFSET                 26
#define QSFP_DD_MODULE_GLOBAL_CONTROL_FORCE_LOW_POWER_BIT    (1<<4)
#define QSFP_DD_MODULE_GLOBAL_CONTROL_LOW_POWER_BIT          (1<<6)
#define QSFP_DD_TEMPERATURE_OFFSET                           14
#define QSFP_DD_VOLTAGE_OFFSET                               16
#define QSFP_DD_BIAS_OFFSET                                  2346
#define QSFP_DD_TX_POWER_OFFSET                              2330
#define QSFP_DD_RX_POWER_OFFSET                              2362
#define QSFP_DD_LANE_NUM                                     8
#define QSFP_DD_LANE_NUM_OFFSET                              88
#define QSFP_DD_LANE_NUM_MEDIALANE_MASK         0xF
#define QSFP_DD_LANE_NUM_MEDIALANE_BIT_OFFSET   0
#define QSFP_DD_TX_LOS_OFFSET                                2312
#define QSFP_DD_TX_LOS_BITMAP           (0xFF)
#define QSFP_DD_TX_LOS_MASK             0xFF
#define QSFP_DD_TX_LOS_BIT_OFFSET       0
#define QSFP_DD_RX_LOS_OFFSET                                2323
#define QSFP_DD_RX_LOS_BITMAP           (0xFF)
#define QSFP_DD_RX_LOS_MASK             0xFF
#define QSFP_DD_RX_LOS_BIT_OFFSET       0
#define QSFP_DD_TX_DISABLE_OFFSET                            2178
#define QSFP_DD_TX_DISABLE_BITMAP       (0xFF)
#define QSFP_DD_TX_DISABLE_MASK         0xFF
#define QSFP_DD_TX_DISABLE_BIT_OFFSET   0
#define QSFP_DD_RX_DISABLE_OFFSET                            2186
#define QSFP_DD_RX_DISABLE_BITMAP       (0xFF)
#define QSFP_DD_RX_DISABLE_MASK         0xFF
#define QSFP_DD_RX_DISABLE_BIT_OFFSET   0
#define QSFP_DD_TX_CDR_LOL_OFFSET                            2313
#define QSFP_DD_TX_CDR_LOL_BITMAP       (0xFF)
#define QSFP_DD_TX_CDR_LOL_MASK         0xFF
#define QSFP_DD_TX_CDR_LOL_BIT_OFFSET   0
#define QSFP_DD_RX_CDR_LOL_OFFSET                            2324
#define QSFP_DD_RX_CDR_LOL_BITMAP       (0xFF)
#define QSFP_DD_RX_CDR_LOL_MASK         0xFF
#define QSFP_DD_RX_CDR_LOL_BIT_OFFSET   0
#define QSFP_DD_TX_FAULT_OFFSET                              2311
#define QSFP_DD_TX_FAULT_BITMAP         (0xFF)
#define QSFP_DD_TX_FAULT_MASK           0xFF
#define QSFP_DD_TX_FAULT_BIT_OFFSET     0
#define QSFP_DD_MODULE_STATUS_OFFSET                         3
#define QSFP_DD_MODULE_STATUS_BITMAP                         (0x0E)
#define QSFP_DD_DATAPATH_STATUS_OFFSET                       2304
#define QSFP_DD_DATAPATH_STATUS_WIDTH                        4

struct transceiver_drivers_t{
    unsigned int (*get_eth_diagnostic) (unsigned int transceiver_index, char *buf);
    int (*get_eth_temp) (unsigned int transceiver_index, long long *temp);
    unsigned int (*get_eth_led_status) (unsigned int transceiver_index);
    unsigned int (*set_eth_led_status) (unsigned int transceiver_index, unsigned int led_value);
    unsigned int (*get_eth_power_on) (unsigned int transceiver_index);
    unsigned int (*set_eth_power_on) (unsigned int transceiver_index, unsigned int pwr_value);
    unsigned int (*get_eth_reset) (unsigned int transceiver_index);
    unsigned int (*set_eth_reset) (unsigned int transceiver_index, unsigned int rst_value);
    int (*get_eth_lpmode) (unsigned int transceiver_index, unsigned int *lpmode_value);
    int (*set_eth_lpmode) (unsigned int transceiver_index, unsigned int lpmode_value);
    unsigned int (*get_eth_present) (unsigned int transceiver_index);
    unsigned int (*get_eth_present_history) (unsigned int transceiver_index);
    unsigned int (*get_eth_interrupt) (unsigned int transceiver_index);
    int (*get_eth_eeprom) (unsigned int transceiver_index, char *buf, loff_t off, size_t len);
    int (*set_eth_eeprom) (unsigned int transceiver_index, char *buf, loff_t off, size_t len);
    int (*get_eth_tx_los) (unsigned int transceiver_index, unsigned char *tx_los);
    int (*get_eth_rx_los) (unsigned int transceiver_index, unsigned char *rx_los);
    int (*get_eth_tx_disable) (unsigned int transceiver_index, unsigned char *tx_disable);
    int (*set_eth_tx_disable) (unsigned int transceiver_index, unsigned int tx_disable);
    int (*get_eth_rx_disable) (unsigned int transceiver_index, unsigned char *rx_disable);
    int (*get_eth_tx_cdr_lol) (unsigned int transceiver_index, unsigned char *tx_cdr_lol);
    int (*get_eth_rx_cdr_lol) (unsigned int transceiver_index, unsigned char *rx_cdr_lol);
    int (*get_eth_tx_fault) (unsigned int transceiver_index, unsigned char *tx_fault);
    int (*get_eth_module_status) (unsigned int transceiver_index, unsigned char *module_status);
    int (*get_eth_datapath_status) (unsigned int transceiver_index, unsigned int *datapath_status);
    int (*get_eth_bus_status) (unsigned int transceiver_index, unsigned char *bus_status);
    void (*get_loglevel) (long *lev);
    void (*set_loglevel) (long lev);
    int (*get_disable_iic_access) (char *buf, long iic_value);
    int (*set_disable_iic_access) (char *buf, long iic_value);
    ssize_t (*debug_help) (char *buf);
    ssize_t (*debug_help_hisonic) (char *buf);
    ssize_t (*debug) (const char *buf, int count);
};

unsigned int drv_get_eth_reset(unsigned int transceiver_index);
unsigned int drv_set_eth_reset(unsigned int transceiver_index, unsigned int rst_value);
unsigned int drv_get_eth_led_status(unsigned int transceiver_index);
unsigned int drv_set_eth_led_status(unsigned int transceiver_index, unsigned int led_value);
unsigned int drv_get_eth_present(unsigned int transceiver_index);
unsigned int drv_get_eth_present_history(unsigned int transceiver_index);
int drv_get_eth_eeprom(unsigned int transceiver_index, char *buf, loff_t off, size_t len);
int drv_set_eth_eeprom(unsigned int transceiver_index, char *buf, loff_t off, size_t len);
void drv_get_loglevel(long *lev);
void drv_set_loglevel(long lev);
int drv_get_disable_iic_access(char *buf, long iic_value);
int drv_set_disable_iic_access(char *buf, long iic_value);
ssize_t drv_debug_help(char *buf);
ssize_t drv_debug_help_hisonic(char *buf);
ssize_t drv_debug(const char *buf, int count);

void s3ip_transceiver_drivers_register(struct transceiver_drivers_t *p_func);
void s3ip_transceiver_drivers_unregister(void);

#endif /* SWITCH_TRANSCEIVER_DRIVER_H */
