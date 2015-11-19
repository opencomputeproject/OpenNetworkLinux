/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <quanta_sys_eeprom/quanta_sys_eeprom_config.h>

#include "quanta_sys_eeprom_log.h"

static int
datatypes_init__(void)
{
#define QUANTA_SYS_EEPROM_ENUMERATION_ENTRY(_enum_name, _desc)     AIM_DATATYPE_MAP_REGISTER(_enum_name, _enum_name##_map, _desc,                               AIM_LOG_INTERNAL);
#include <quanta_sys_eeprom/quanta_sys_eeprom.x>
    return 0;
}

void __quanta_sys_eeprom_module_init__(void)
{
    AIM_LOG_STRUCT_REGISTER();
    datatypes_init__();
}

