#ifndef __INV_TYPE_H
#define __INV_TYPE_H

#include <linux/kernel.h>
/*port config*/

enum sff_type {
    SFP_TYPE,
    QSFP_TYPE,
    QSFP_DD_TYPE
};
struct port_info_map_t {
    int port;
    enum sff_type type;
    char *name;
};
struct port_info_table_t {
    struct port_info_map_t *map;
    int size;
};
struct eeprom_config_t {
    int port;
    int i2c_ch;
};
struct eeprom_i2c_tbl_t {

    struct eeprom_config_t *map;
    int size;
};

#if 0
struct pin_config_t {

    int i2c_ch;
    u8 slave_addr;
    int port_ch;
    u8 bitmask;
    int in_use;
};
struct pin_map_t
{
    int port;
    int i2c_ch;
    u8 slave_addr;
    int port_ch;
    u8 bitmask;

};

#endif
struct sff_driver_t
{
    int (*prsL_get)(int port, u8 *prsL);
    int (*prsL_all_get)(unsigned long *bitmap);
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
#endif /*__INV_TYPE_H*/
