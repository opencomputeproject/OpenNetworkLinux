/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <onlp_snmp/onlp_snmp_config.h>

/* <auto.start.cdefs(ONLP_SNMP_CONFIG_HEADER).source> */
#define __onlp_snmp_config_STRINGIFY_NAME(_x) #_x
#define __onlp_snmp_config_STRINGIFY_VALUE(_x) __onlp_snmp_config_STRINGIFY_NAME(_x)
onlp_snmp_config_settings_t onlp_snmp_config_settings[] =
{
#ifdef ONLP_SNMP_CONFIG_INCLUDE_LOGGING
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_INCLUDE_LOGGING), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_INCLUDE_LOGGING) },
#else
{ ONLP_SNMP_CONFIG_INCLUDE_LOGGING(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_LOG_OPTIONS_DEFAULT
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_LOG_OPTIONS_DEFAULT), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_LOG_OPTIONS_DEFAULT) },
#else
{ ONLP_SNMP_CONFIG_LOG_OPTIONS_DEFAULT(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_LOG_BITS_DEFAULT
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_LOG_BITS_DEFAULT), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_LOG_BITS_DEFAULT) },
#else
{ ONLP_SNMP_CONFIG_LOG_BITS_DEFAULT(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_LOG_CUSTOM_BITS_DEFAULT
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_LOG_CUSTOM_BITS_DEFAULT), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_LOG_CUSTOM_BITS_DEFAULT) },
#else
{ ONLP_SNMP_CONFIG_LOG_CUSTOM_BITS_DEFAULT(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_PORTING_STDLIB
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_PORTING_STDLIB), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_PORTING_STDLIB) },
#else
{ ONLP_SNMP_CONFIG_PORTING_STDLIB(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS) },
#else
{ ONLP_SNMP_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_INCLUDE_UCLI
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_INCLUDE_UCLI), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_INCLUDE_UCLI) },
#else
{ ONLP_SNMP_CONFIG_INCLUDE_UCLI(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_MAX_NAME_LENGTH
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_MAX_NAME_LENGTH), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_MAX_NAME_LENGTH) },
#else
{ ONLP_SNMP_CONFIG_MAX_NAME_LENGTH(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_MAX_DESC_LENGTH
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_MAX_DESC_LENGTH), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_MAX_DESC_LENGTH) },
#else
{ ONLP_SNMP_CONFIG_MAX_DESC_LENGTH(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_UPDATE_PERIOD
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_UPDATE_PERIOD), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_UPDATE_PERIOD) },
#else
{ ONLP_SNMP_CONFIG_UPDATE_PERIOD(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_DEV_BASE_INDEX
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_DEV_BASE_INDEX), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_DEV_BASE_INDEX) },
#else
{ ONLP_SNMP_CONFIG_DEV_BASE_INDEX(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_INCLUDE_THERMALS
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_INCLUDE_THERMALS), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_INCLUDE_THERMALS) },
#else
{ ONLP_SNMP_CONFIG_INCLUDE_THERMALS(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_INCLUDE_FANS
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_INCLUDE_FANS), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_INCLUDE_FANS) },
#else
{ ONLP_SNMP_CONFIG_INCLUDE_FANS(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_INCLUDE_PSUS
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_INCLUDE_PSUS), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_INCLUDE_PSUS) },
#else
{ ONLP_SNMP_CONFIG_INCLUDE_PSUS(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_INCLUDE_LEDS
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_INCLUDE_LEDS), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_INCLUDE_LEDS) },
#else
{ ONLP_SNMP_CONFIG_INCLUDE_LEDS(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_INCLUDE_PLATFORM
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_INCLUDE_PLATFORM), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_INCLUDE_PLATFORM) },
#else
{ ONLP_SNMP_CONFIG_INCLUDE_PLATFORM(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_AS_SUBAGENT
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_AS_SUBAGENT), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_AS_SUBAGENT) },
#else
{ ONLP_SNMP_CONFIG_AS_SUBAGENT(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ONLP_SNMP_CONFIG_RESOURCE_UPDATE_SECONDS
    { __onlp_snmp_config_STRINGIFY_NAME(ONLP_SNMP_CONFIG_RESOURCE_UPDATE_SECONDS), __onlp_snmp_config_STRINGIFY_VALUE(ONLP_SNMP_CONFIG_RESOURCE_UPDATE_SECONDS) },
#else
{ ONLP_SNMP_CONFIG_RESOURCE_UPDATE_SECONDS(__onlp_snmp_config_STRINGIFY_NAME), "__undefined__" },
#endif
    { NULL, NULL }
};
#undef __onlp_snmp_config_STRINGIFY_VALUE
#undef __onlp_snmp_config_STRINGIFY_NAME

const char*
onlp_snmp_config_lookup(const char* setting)
{
    int i;
    for(i = 0; onlp_snmp_config_settings[i].name; i++) {
        if(!strcmp(onlp_snmp_config_settings[i].name, setting)) {
            return onlp_snmp_config_settings[i].value;
        }
    }
    return NULL;
}

int
onlp_snmp_config_show(struct aim_pvs_s* pvs)
{
    int i;
    for(i = 0; onlp_snmp_config_settings[i].name; i++) {
        aim_printf(pvs, "%s = %s\n", onlp_snmp_config_settings[i].name, onlp_snmp_config_settings[i].value);
    }
    return i;
}

/* <auto.end.cdefs(ONLP_SNMP_CONFIG_HEADER).source> */

