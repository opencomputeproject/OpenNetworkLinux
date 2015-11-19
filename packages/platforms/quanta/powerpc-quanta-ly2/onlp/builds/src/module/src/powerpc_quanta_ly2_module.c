/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <powerpc_quanta_ly2/powerpc_quanta_ly2_config.h>

#include "powerpc_quanta_ly2_log.h"

static int
datatypes_init__(void)
{
#define POWERPC_QUANTA_LY2_R0_ENUMERATION_ENTRY(_enum_name, _desc)     AIM_DATATYPE_MAP_REGISTER(_enum_name, _enum_name##_map, _desc,                               AIM_LOG_INTERNAL);
#include <powerpc_quanta_ly2/powerpc_quanta_ly2.x>
    return 0;
}

void __powerpc_quanta_ly2_module_init__(void)
{
    AIM_LOG_STRUCT_REGISTER();
    datatypes_init__();
}

int __onlp_platform_version__ = 1;
