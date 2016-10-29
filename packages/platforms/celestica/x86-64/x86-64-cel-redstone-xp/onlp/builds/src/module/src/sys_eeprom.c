#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "sys_eeprom.h"
#include "i2c_chips.h"
#include "i2c_dev.h"
#include "redstone_cpld.h"

static inline u_int8_t is_valid_tlvinfo_header(tlvinfo_header_t *hdr)
{
    int max_size = TLV_TOTAL_LEN_MAX;

    return((strcmp(hdr->signature, TLV_INFO_ID_STRING) == 0) &&
                            (hdr->version == TLV_INFO_VERSION) &&
                            (be16_to_cpu(hdr->totallen) <= max_size));
}

int
read_sys_eeprom(void *eeprom_data, int offset, int len)
{
  int ret = 0;
  int i = 0;
  u_int8_t *c;

  int addr = SYS_EEPROM_OFFSET + offset;
  unsigned char dev_id = 5;

  c = eeprom_data;
  if (eeprom_enable(dev_id) < 0) {
    printf("ERROR: Cannot open I2C device\n");
    return -1;
  }

  for (i = 0; i < len; i++) {
    unsigned char buf;
    ret = eeprom_read_byte(CONFIG_SYS_I2C_EEPROM_ADDR, addr, &buf);
    *c = buf;
    c++;
    addr++;
  }

  eeprom_disable(dev_id);
  return ret;
}

int
read_eeprom(u_int8_t *eeprom, int *size)
{
    int ret;
    tlvinfo_header_t *eeprom_hdr = (tlvinfo_header_t *) eeprom;
    u_int8_t *buf = eeprom + sizeof(tlvinfo_header_t);

    /* Read the header */
    ret = read_sys_eeprom((void *)eeprom_hdr, 0, sizeof(tlvinfo_header_t));

    /* If the header was successfully read, read the TLVs */
    if ((ret == 0) && is_valid_tlvinfo_header(eeprom_hdr))
        ret = read_sys_eeprom((void *)buf, sizeof(tlvinfo_header_t),
                                            be16_to_cpu(eeprom_hdr->totallen));

    return ret;
}
