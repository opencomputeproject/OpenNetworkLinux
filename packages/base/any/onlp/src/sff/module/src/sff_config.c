/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <sff/sff_config.h>

/* <auto.start.cdefs(SFF_CONFIG_HEADER).source> */
#define __sff_config_STRINGIFY_NAME(_x) #_x
#define __sff_config_STRINGIFY_VALUE(_x) __sff_config_STRINGIFY_NAME(_x)
sff_config_settings_t sff_config_settings[] =
{
#ifdef SFF_CONFIG_INCLUDE_LOGGING
    { __sff_config_STRINGIFY_NAME(SFF_CONFIG_INCLUDE_LOGGING), __sff_config_STRINGIFY_VALUE(SFF_CONFIG_INCLUDE_LOGGING) },
#else
{ SFF_CONFIG_INCLUDE_LOGGING(__sff_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef SFF_CONFIG_LOG_OPTIONS_DEFAULT
    { __sff_config_STRINGIFY_NAME(SFF_CONFIG_LOG_OPTIONS_DEFAULT), __sff_config_STRINGIFY_VALUE(SFF_CONFIG_LOG_OPTIONS_DEFAULT) },
#else
{ SFF_CONFIG_LOG_OPTIONS_DEFAULT(__sff_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef SFF_CONFIG_LOG_BITS_DEFAULT
    { __sff_config_STRINGIFY_NAME(SFF_CONFIG_LOG_BITS_DEFAULT), __sff_config_STRINGIFY_VALUE(SFF_CONFIG_LOG_BITS_DEFAULT) },
#else
{ SFF_CONFIG_LOG_BITS_DEFAULT(__sff_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef SFF_CONFIG_LOG_CUSTOM_BITS_DEFAULT
    { __sff_config_STRINGIFY_NAME(SFF_CONFIG_LOG_CUSTOM_BITS_DEFAULT), __sff_config_STRINGIFY_VALUE(SFF_CONFIG_LOG_CUSTOM_BITS_DEFAULT) },
#else
{ SFF_CONFIG_LOG_CUSTOM_BITS_DEFAULT(__sff_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef SFF_CONFIG_PORTING_STDLIB
    { __sff_config_STRINGIFY_NAME(SFF_CONFIG_PORTING_STDLIB), __sff_config_STRINGIFY_VALUE(SFF_CONFIG_PORTING_STDLIB) },
#else
{ SFF_CONFIG_PORTING_STDLIB(__sff_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef SFF_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
    { __sff_config_STRINGIFY_NAME(SFF_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS), __sff_config_STRINGIFY_VALUE(SFF_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS) },
#else
{ SFF_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS(__sff_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef SFF_CONFIG_INCLUDE_UCLI
    { __sff_config_STRINGIFY_NAME(SFF_CONFIG_INCLUDE_UCLI), __sff_config_STRINGIFY_VALUE(SFF_CONFIG_INCLUDE_UCLI) },
#else
{ SFF_CONFIG_INCLUDE_UCLI(__sff_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef SFF_CONFIG_INCLUDE_SFF_TOOL
    { __sff_config_STRINGIFY_NAME(SFF_CONFIG_INCLUDE_SFF_TOOL), __sff_config_STRINGIFY_VALUE(SFF_CONFIG_INCLUDE_SFF_TOOL) },
#else
{ SFF_CONFIG_INCLUDE_SFF_TOOL(__sff_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef SFF_CONFIG_INCLUDE_EXT_CC_CHECK
    { __sff_config_STRINGIFY_NAME(SFF_CONFIG_INCLUDE_EXT_CC_CHECK), __sff_config_STRINGIFY_VALUE(SFF_CONFIG_INCLUDE_EXT_CC_CHECK) },
#else
{ SFF_CONFIG_INCLUDE_EXT_CC_CHECK(__sff_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef SFF_CONFIG_INCLUDE_DATABASE
    { __sff_config_STRINGIFY_NAME(SFF_CONFIG_INCLUDE_DATABASE), __sff_config_STRINGIFY_VALUE(SFF_CONFIG_INCLUDE_DATABASE) },
#else
{ SFF_CONFIG_INCLUDE_DATABASE(__sff_config_STRINGIFY_NAME), "__undefined__" },
#endif
    { NULL, NULL }
};
#undef __sff_config_STRINGIFY_VALUE
#undef __sff_config_STRINGIFY_NAME

const char*
sff_config_lookup(const char* setting)
{
    int i;
    for(i = 0; sff_config_settings[i].name; i++) {
        if(!strcmp(sff_config_settings[i].name, setting)) {
            return sff_config_settings[i].value;
        }
    }
    return NULL;
}

int
sff_config_show(struct aim_pvs_s* pvs)
{
    int i;
    for(i = 0; sff_config_settings[i].name; i++) {
        aim_printf(pvs, "%s = %s\n", sff_config_settings[i].name, sff_config_settings[i].value);
    }
    return i;
}

/* <auto.end.cdefs(SFF_CONFIG_HEADER).source> */

