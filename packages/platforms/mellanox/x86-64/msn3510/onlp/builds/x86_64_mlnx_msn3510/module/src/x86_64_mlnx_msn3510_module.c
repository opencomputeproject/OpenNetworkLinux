/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <x86_64_mlnx_msn3510/x86_64_mlnx_msn3510_config.h>

#include "x86_64_mlnx_msn3510_log.h"

static int
datatypes_init__(void)
{
#define x86_64_mlnx_msn3510_ENUMERATION_ENTRY(_enum_name, _desc)     AIM_DATATYPE_MAP_REGISTER(_enum_name, _enum_name##_map, _desc,                               AIM_LOG_INTERNAL);
#include <x86_64_mlnx_msn3510/x86_64_mlnx_msn3510.x>
    return 0;
}

void __x86_64_mlnx_msn3510_module_init__(void)
{
    AIM_LOG_STRUCT_REGISTER();
    datatypes_init__();
}

int __onlp_platform_version__ = 1;
