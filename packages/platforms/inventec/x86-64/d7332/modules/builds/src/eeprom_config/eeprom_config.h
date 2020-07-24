#ifndef __EEPROM_CONFIG
#define __EEPROM_CONFIG

#include "eeprom_config_banyan.h"
#include "eeprom_config_maple.h"
//#include "eeprom_config_cedar.h"
#include "eeprom_config_4U.h"

struct platform_eeprom_info_t {
    int platform_id;
    struct eeprom_i2c_tbl_t *tbl;
};

struct platform_eeprom_info_t platform_eeprom_info_tbl[] = {

    {.platform_id = PLATFORM_MAPLE, .tbl = &maple_eeprom_i2c_tbl},
    {.platform_id = PLATFORM_BANYAN, .tbl = &banyan_eeprom_i2c_tbl},
    {.platform_id = PLATFORM_4U, .tbl = &banyan_4U_eeprom_i2c_tbl},
#if 0
    {.platform_id = PLATFORM_CEDAR, .tbl = &cedar_eeprom_i2c_tbl},
#endif    
    {.platform_id = PLATFORM_END },
};


#endif /*__EEPROM_CONFIG*/
