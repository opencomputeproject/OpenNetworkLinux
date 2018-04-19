/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <powerpc_accton_as4600_54t/powerpc_accton_as4600_54t_config.h>

/* <auto.start.cdefs(POWERPC_ACCTON_AS4600_54T_CONFIG_HEADER).source> */
#define __powerpc_accton_as4600_54t_config_STRINGIFY_NAME(_x) #_x
#define __powerpc_accton_as4600_54t_config_STRINGIFY_VALUE(_x) __powerpc_accton_as4600_54t_config_STRINGIFY_NAME(_x)
powerpc_accton_as4600_54t_config_settings_t powerpc_accton_as4600_54t_config_settings[] =
{
#ifdef POWERPC_ACCTON_AS4600_54T_CONFIG_INCLUDE_LOGGING
    { __powerpc_accton_as4600_54t_config_STRINGIFY_NAME(POWERPC_ACCTON_AS4600_54T_CONFIG_INCLUDE_LOGGING), __powerpc_accton_as4600_54t_config_STRINGIFY_VALUE(POWERPC_ACCTON_AS4600_54T_CONFIG_INCLUDE_LOGGING) },
#else
{ POWERPC_ACCTON_AS4600_54T_CONFIG_INCLUDE_LOGGING(__powerpc_accton_as4600_54t_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef POWERPC_ACCTON_AS4600_54T_CONFIG_LOG_OPTIONS_DEFAULT
    { __powerpc_accton_as4600_54t_config_STRINGIFY_NAME(POWERPC_ACCTON_AS4600_54T_CONFIG_LOG_OPTIONS_DEFAULT), __powerpc_accton_as4600_54t_config_STRINGIFY_VALUE(POWERPC_ACCTON_AS4600_54T_CONFIG_LOG_OPTIONS_DEFAULT) },
#else
{ POWERPC_ACCTON_AS4600_54T_CONFIG_LOG_OPTIONS_DEFAULT(__powerpc_accton_as4600_54t_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef POWERPC_ACCTON_AS4600_54T_CONFIG_LOG_BITS_DEFAULT
    { __powerpc_accton_as4600_54t_config_STRINGIFY_NAME(POWERPC_ACCTON_AS4600_54T_CONFIG_LOG_BITS_DEFAULT), __powerpc_accton_as4600_54t_config_STRINGIFY_VALUE(POWERPC_ACCTON_AS4600_54T_CONFIG_LOG_BITS_DEFAULT) },
#else
{ POWERPC_ACCTON_AS4600_54T_CONFIG_LOG_BITS_DEFAULT(__powerpc_accton_as4600_54t_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef POWERPC_ACCTON_AS4600_54T_CONFIG_LOG_CUSTOM_BITS_DEFAULT
    { __powerpc_accton_as4600_54t_config_STRINGIFY_NAME(POWERPC_ACCTON_AS4600_54T_CONFIG_LOG_CUSTOM_BITS_DEFAULT), __powerpc_accton_as4600_54t_config_STRINGIFY_VALUE(POWERPC_ACCTON_AS4600_54T_CONFIG_LOG_CUSTOM_BITS_DEFAULT) },
#else
{ POWERPC_ACCTON_AS4600_54T_CONFIG_LOG_CUSTOM_BITS_DEFAULT(__powerpc_accton_as4600_54t_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_STDLIB
    { __powerpc_accton_as4600_54t_config_STRINGIFY_NAME(POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_STDLIB), __powerpc_accton_as4600_54t_config_STRINGIFY_VALUE(POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_STDLIB) },
#else
{ POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_STDLIB(__powerpc_accton_as4600_54t_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
    { __powerpc_accton_as4600_54t_config_STRINGIFY_NAME(POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS), __powerpc_accton_as4600_54t_config_STRINGIFY_VALUE(POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS) },
#else
{ POWERPC_ACCTON_AS4600_54T_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS(__powerpc_accton_as4600_54t_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef POWERPC_ACCTON_AS4600_54T_CONFIG_INCLUDE_UCLI
    { __powerpc_accton_as4600_54t_config_STRINGIFY_NAME(POWERPC_ACCTON_AS4600_54T_CONFIG_INCLUDE_UCLI), __powerpc_accton_as4600_54t_config_STRINGIFY_VALUE(POWERPC_ACCTON_AS4600_54T_CONFIG_INCLUDE_UCLI) },
#else
{ POWERPC_ACCTON_AS4600_54T_CONFIG_INCLUDE_UCLI(__powerpc_accton_as4600_54t_config_STRINGIFY_NAME), "__undefined__" },
#endif
    { NULL, NULL }
};
#undef __powerpc_accton_as4600_54t_config_STRINGIFY_VALUE
#undef __powerpc_accton_as4600_54t_config_STRINGIFY_NAME

const char*
powerpc_accton_as4600_54t_config_lookup(const char* setting)
{
    int i;
    for(i = 0; powerpc_accton_as4600_54t_config_settings[i].name; i++) {
        if(!strcmp(powerpc_accton_as4600_54t_config_settings[i].name, setting)) {
            return powerpc_accton_as4600_54t_config_settings[i].value;
        }
    }
    return NULL;
}

int
powerpc_accton_as4600_54t_config_show(struct aim_pvs_s* pvs)
{
    int i;
    for(i = 0; powerpc_accton_as4600_54t_config_settings[i].name; i++) {
        aim_printf(pvs, "%s = %s\n", powerpc_accton_as4600_54t_config_settings[i].name, powerpc_accton_as4600_54t_config_settings[i].value);
    }
    return i;
}

/* <auto.end.cdefs(POWERPC_ACCTON_AS4600_54T_CONFIG_HEADER).source> */

