/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <powerpc_accton_as5610_52x/powerpc_accton_as5610_52x_config.h>

#include "powerpc_accton_as5610_52x_log.h"

static int
datatypes_init__(void)
{
#define POWERPC_ACCTON_AS5610_52X_ENUMERATION_ENTRY(_enum_name, _desc)     AIM_DATATYPE_MAP_REGISTER(_enum_name, _enum_name##_map, _desc,                               AIM_LOG_INTERNAL);
#include <powerpc_accton_as5610_52x/powerpc_accton_as5610_52x.x>
    return 0;
}

void __powerpc_accton_as5610_52x_module_init__(void)
{
    AIM_LOG_STRUCT_REGISTER();
    datatypes_init__();
}

int __onlp_platform_version__ = 1;
