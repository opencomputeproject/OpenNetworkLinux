/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <onlp_snmp/onlp_snmp_config.h>
#include <onlp/onlp.h>
#include "onlp_snmp_log.h"

static int
datatypes_init__(void)
{
#define ONLP_SNMP_ENUMERATION_ENTRY(_enum_name, _desc)     AIM_DATATYPE_MAP_REGISTER(_enum_name, _enum_name##_map, _desc,                               AIM_LOG_INTERNAL);
#include <onlp_snmp/onlp_snmp.x>
    return 0;
}

void __onlp_snmp_module_init__(void)
{
    AIM_LOG_STRUCT_REGISTER();
    datatypes_init__();
    onlp_init();
}

/**
 * Used when integrated with the snmp_subagent module.
 */

#include <onlp_snmp/onlp_snmp_sensors.h>

#include <dependmodules.x>

#ifdef DEPENDMODULE_INCLUDE_SNMP_SUBAGENT

#include <snmp_subagent/snmp_subagent.h>

int onlp_snmp_snmp_subagent_register(void)
{
    return snmp_subagent_client_register("onlp_snmp_sensors",
                                         onlp_snmp_sensors_client,
                                         NULL);
}

int onlp_snmp_snmp_subagent_unregister(void)
{
    return snmp_subagent_client_unregister("onlp_snmp_sensors");
}

#endif /* DEPENDMODULE_INCLUDE_SNMP_SUBAGENT */




