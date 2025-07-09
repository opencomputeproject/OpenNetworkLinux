#ifndef AT24_H
#define AT24_H

ssize_t at24_read_fan_eeprom(unsigned int adapter_nr, uint8_t *eeprom, unsigned int offset, unsigned int count);

#endif /* AT24_H */