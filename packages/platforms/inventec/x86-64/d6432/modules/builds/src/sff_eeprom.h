//port info
#ifndef __SFF_EEPROM_H
#define __SFF_EEPROM_H

struct eeprom_config_t {
    int port;
    int i2c_ch;
};
struct eeprom_i2c_tbl_t {

    struct eeprom_config_t *map;
    int size;
};

int sff_eeprom_init(void);
void sff_eeprom_deinit(void);

int sff_eeprom_read(int port,
                    u8 slave_addr,
                    u8 offset,
                    u8 *buf,
                    size_t len);

int sff_eeprom_write(int port,
                     u8 slave_addr,
                     u8 offset,
                     const u8 *buf,
                     size_t len);
#endif /*__SFF_EEPROM_H*/
