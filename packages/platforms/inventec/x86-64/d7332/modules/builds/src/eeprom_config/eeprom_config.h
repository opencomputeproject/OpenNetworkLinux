
#ifndef __EEPROM_CONFIG
#define __EEPROM_CONFIG

#include "eeprom_config_banyan.h"
#include "eeprom_config_maple.h"
#include "eeprom_config_cedar.h"
#include "eeprom_config_cypress.h"

struct platform_eeprom_info_t {
    int platform_name;
    struct eeprom_i2c_tbl_t *tbl;
};

struct platform_eeprom_info_t platform_eeprom_info_tbl[] = {

    {.platform_name = PLATFORM_MAPLE, .tbl = &maple_eeprom_i2c_tbl},
    {.platform_name = PLATFORM_BANYAN, .tbl = &banyan_eeprom_i2c_tbl},
    {.platform_name = PLATFORM_CEDAR, .tbl = &cedar_eeprom_i2c_tbl},
    {.platform_name = PLATFORM_CYPRESS, .tbl = &cypress_eeprom_i2c_tbl},
    {.platform_name = PLATFORM_END },
};


#endif /*__EEPROM_CONFIG*/
