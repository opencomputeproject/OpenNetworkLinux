/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <mlnx_common/mlnx_common_config.h>

/* <auto.start.cdefs(MLNX_COMMON_CONFIG_HEADER).source> */
#define __mlnx_common_config_STRINGIFY_NAME(_x) #_x
#define __mlnx_common_config_STRINGIFY_VALUE(_x) __mlnx_common_config_STRINGIFY_NAME(_x)
mlnx_common_config_settings_t mlnx_common_config_settings[] =
{
#ifdef MLNX_COMMON_CONFIG_INCLUDE_LOGGING
    { __mlnx_common_config_STRINGIFY_NAME(MLNX_COMMON_CONFIG_INCLUDE_LOGGING), __mlnx_common_config_STRINGIFY_VALUE(MLNX_COMMON_CONFIG_INCLUDE_LOGGING) },
#else
{ MLNX_COMMON_CONFIG_INCLUDE_LOGGING(__mlnx_common_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef MLNX_COMMON_CONFIG_LOG_OPTIONS_DEFAULT
    { __mlnx_common_config_STRINGIFY_NAME(MLNX_COMMON_CONFIG_LOG_OPTIONS_DEFAULT), __mlnx_common_config_STRINGIFY_VALUE(MLNX_COMMON_CONFIG_LOG_OPTIONS_DEFAULT) },
#else
{ MLNX_COMMON_CONFIG_LOG_OPTIONS_DEFAULT(__mlnx_common_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef MLNX_COMMON_CONFIG_LOG_BITS_DEFAULT
    { __mlnx_common_config_STRINGIFY_NAME(MLNX_COMMON_CONFIG_LOG_BITS_DEFAULT), __mlnx_common_config_STRINGIFY_VALUE(MLNX_COMMON_CONFIG_LOG_BITS_DEFAULT) },
#else
{ MLNX_COMMON_CONFIG_LOG_BITS_DEFAULT(__mlnx_common_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef MLNX_COMMON_CONFIG_LOG_CUSTOM_BITS_DEFAULT
    { __mlnx_common_config_STRINGIFY_NAME(MLNX_COMMON_CONFIG_LOG_CUSTOM_BITS_DEFAULT), __mlnx_common_config_STRINGIFY_VALUE(MLNX_COMMON_CONFIG_LOG_CUSTOM_BITS_DEFAULT) },
#else
{ MLNX_COMMON_CONFIG_LOG_CUSTOM_BITS_DEFAULT(__mlnx_common_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef MLNX_COMMON_CONFIG_PORTING_STDLIB
    { __mlnx_common_config_STRINGIFY_NAME(MLNX_COMMON_CONFIG_PORTING_STDLIB), __mlnx_common_config_STRINGIFY_VALUE(MLNX_COMMON_CONFIG_PORTING_STDLIB) },
#else
{ MLNX_COMMON_CONFIG_PORTING_STDLIB(__mlnx_common_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef MLNX_COMMON_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
    { __mlnx_common_config_STRINGIFY_NAME(MLNX_COMMON_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS), __mlnx_common_config_STRINGIFY_VALUE(MLNX_COMMON_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS) },
#else
{ MLNX_COMMON_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS(__mlnx_common_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef MLNX_COMMON_CONFIG_INCLUDE_UCLI
    { __mlnx_common_config_STRINGIFY_NAME(MLNX_COMMON_CONFIG_INCLUDE_UCLI), __mlnx_common_config_STRINGIFY_VALUE(MLNX_COMMON_CONFIG_INCLUDE_UCLI) },
#else
{ MLNX_COMMON_CONFIG_INCLUDE_UCLI(__mlnx_common_config_STRINGIFY_NAME), "__undefined__" },
#endif
    { NULL, NULL }
};
#undef __mlnx_common_config_STRINGIFY_VALUE
#undef __mlnx_common_config_STRINGIFY_NAME

const char*
mlnx_common_config_lookup(const char* setting)
{
    int i;
    for(i = 0; mlnx_common_config_settings[i].name; i++) {
        if(strcmp(mlnx_common_config_settings[i].name, setting)) {
            return mlnx_common_config_settings[i].value;
        }
    }
    return NULL;
}

int
mlnx_common_config_show(struct aim_pvs_s* pvs)
{
    int i;
    for(i = 0; mlnx_common_config_settings[i].name; i++) {
        aim_printf(pvs, "%s = %s\n", mlnx_common_config_settings[i].name, mlnx_common_config_settings[i].value);
    }
    return i;
}

/* <auto.end.cdefs(MLNX_COMMON_CONFIG_HEADER).source> */
