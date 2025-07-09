#include <linux/kernel.h>

#define ETH_NAME_STRING         "eth"

/* fundamental unit of addressing for EEPROM */
#define OPTOE_PAGE_SIZE 128
/*
 * Single address devices (eg QSFP) have 256 pages, plus the unpaged
 * low 128 bytes.  If the device does not support paging, it is
 * only 2 'pages' long.
 */
#define OPTOE_ARCH_PAGES 255
#define ONE_ADDR_EEPROM_SIZE ((1 + OPTOE_ARCH_PAGES) * OPTOE_PAGE_SIZE)
#define ONE_ADDR_EEPROM_UNPAGED_SIZE (2 * OPTOE_PAGE_SIZE)
#define ONE_ADDR_EEPROM_PAGED_SIZE (6 * OPTOE_PAGE_SIZE)
#define CMIS_ADDR_EEPROM_SIZE ((2 + 0X11) * OPTOE_PAGE_SIZE)
/*
 * Dual address devices (eg SFP) have 256 pages, plus the unpaged
 * low 128 bytes, plus 256 bytes at 0x50.  If the device does not
 * support paging, it is 4 'pages' long.
 */
#define TWO_ADDR_EEPROM_SIZE ((3 + OPTOE_ARCH_PAGES) * OPTOE_PAGE_SIZE)
#define TWO_ADDR_EEPROM_UNPAGED_SIZE (4 * OPTOE_PAGE_SIZE)
#define TWO_ADDR_NO_0X51_SIZE (2 * OPTOE_PAGE_SIZE)

#define QSFP_DD_CONNECTOR_OFF 203
#define QSFP_CONNECTOR_OFF 130
#define QSFP_DD_TRANSMITTER_OFF 212
#define QSFP_TRANSMITTER_OFF 147
#define TRANSMITTER_CROTICA_VALUE 0xa
#define QSFP_DD_TRANSMITTER_BIT 0
#define QSFP_TRANSMITTER_BIT 4
#define CONNECTOR_TYPE 0x23


int optoe_bin_read_by_index(unsigned int index, char *buf, loff_t off, size_t len);
int optoe_bin_write_by_index(unsigned int index, char *buf, loff_t off, size_t len);
int optoe_get_disable_iic_access(char *buf, long iic_value);
int optoe_set_disable_iic_access(char *buf, long iic_value);

