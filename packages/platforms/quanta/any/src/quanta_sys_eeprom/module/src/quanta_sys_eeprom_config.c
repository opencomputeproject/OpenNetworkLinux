/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <quanta_sys_eeprom/quanta_sys_eeprom_config.h>

/* <auto.start.cdefs(QUANTA_SYS_EEPROM_CONFIG_HEADER).source> */
#define __quanta_sys_eeprom_config_STRINGIFY_NAME(_x) #_x
#define __quanta_sys_eeprom_config_STRINGIFY_VALUE(_x) __quanta_sys_eeprom_config_STRINGIFY_NAME(_x)
quanta_sys_eeprom_config_settings_t quanta_sys_eeprom_config_settings[] =
{
#ifdef QUANTA_SYS_EEPROM_CONFIG_INCLUDE_LOGGING
    { __quanta_sys_eeprom_config_STRINGIFY_NAME(QUANTA_SYS_EEPROM_CONFIG_INCLUDE_LOGGING), __quanta_sys_eeprom_config_STRINGIFY_VALUE(QUANTA_SYS_EEPROM_CONFIG_INCLUDE_LOGGING) },
#else
{ QUANTA_SYS_EEPROM_CONFIG_INCLUDE_LOGGING(__quanta_sys_eeprom_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef QUANTA_SYS_EEPROM_CONFIG_LOG_OPTIONS_DEFAULT
    { __quanta_sys_eeprom_config_STRINGIFY_NAME(QUANTA_SYS_EEPROM_CONFIG_LOG_OPTIONS_DEFAULT), __quanta_sys_eeprom_config_STRINGIFY_VALUE(QUANTA_SYS_EEPROM_CONFIG_LOG_OPTIONS_DEFAULT) },
#else
{ QUANTA_SYS_EEPROM_CONFIG_LOG_OPTIONS_DEFAULT(__quanta_sys_eeprom_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef QUANTA_SYS_EEPROM_CONFIG_LOG_BITS_DEFAULT
    { __quanta_sys_eeprom_config_STRINGIFY_NAME(QUANTA_SYS_EEPROM_CONFIG_LOG_BITS_DEFAULT), __quanta_sys_eeprom_config_STRINGIFY_VALUE(QUANTA_SYS_EEPROM_CONFIG_LOG_BITS_DEFAULT) },
#else
{ QUANTA_SYS_EEPROM_CONFIG_LOG_BITS_DEFAULT(__quanta_sys_eeprom_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef QUANTA_SYS_EEPROM_CONFIG_LOG_CUSTOM_BITS_DEFAULT
    { __quanta_sys_eeprom_config_STRINGIFY_NAME(QUANTA_SYS_EEPROM_CONFIG_LOG_CUSTOM_BITS_DEFAULT), __quanta_sys_eeprom_config_STRINGIFY_VALUE(QUANTA_SYS_EEPROM_CONFIG_LOG_CUSTOM_BITS_DEFAULT) },
#else
{ QUANTA_SYS_EEPROM_CONFIG_LOG_CUSTOM_BITS_DEFAULT(__quanta_sys_eeprom_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef QUANTA_SYS_EEPROM_CONFIG_PORTING_STDLIB
    { __quanta_sys_eeprom_config_STRINGIFY_NAME(QUANTA_SYS_EEPROM_CONFIG_PORTING_STDLIB), __quanta_sys_eeprom_config_STRINGIFY_VALUE(QUANTA_SYS_EEPROM_CONFIG_PORTING_STDLIB) },
#else
{ QUANTA_SYS_EEPROM_CONFIG_PORTING_STDLIB(__quanta_sys_eeprom_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef QUANTA_SYS_EEPROM_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
    { __quanta_sys_eeprom_config_STRINGIFY_NAME(QUANTA_SYS_EEPROM_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS), __quanta_sys_eeprom_config_STRINGIFY_VALUE(QUANTA_SYS_EEPROM_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS) },
#else
{ QUANTA_SYS_EEPROM_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS(__quanta_sys_eeprom_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef QUANTA_SYS_EEPROM_CONFIG_INCLUDE_UCLI
    { __quanta_sys_eeprom_config_STRINGIFY_NAME(QUANTA_SYS_EEPROM_CONFIG_INCLUDE_UCLI), __quanta_sys_eeprom_config_STRINGIFY_VALUE(QUANTA_SYS_EEPROM_CONFIG_INCLUDE_UCLI) },
#else
{ QUANTA_SYS_EEPROM_CONFIG_INCLUDE_UCLI(__quanta_sys_eeprom_config_STRINGIFY_NAME), "__undefined__" },
#endif
    { NULL, NULL }
};
#undef __quanta_sys_eeprom_config_STRINGIFY_VALUE
#undef __quanta_sys_eeprom_config_STRINGIFY_NAME

const char*
quanta_sys_eeprom_config_lookup(const char* setting)
{
    int i;
    for(i = 0; quanta_sys_eeprom_config_settings[i].name; i++) {
        if(!strcmp(quanta_sys_eeprom_config_settings[i].name, setting)) {
            return quanta_sys_eeprom_config_settings[i].value;
        }
    }
    return NULL;
}

int
quanta_sys_eeprom_config_show(struct aim_pvs_s* pvs)
{
    int i;
    for(i = 0; quanta_sys_eeprom_config_settings[i].name; i++) {
        aim_printf(pvs, "%s = %s\n", quanta_sys_eeprom_config_settings[i].name, quanta_sys_eeprom_config_settings[i].value);
    }
    return i;
}

/* <auto.end.cdefs(QUANTA_SYS_EEPROM_CONFIG_HEADER).source> */

