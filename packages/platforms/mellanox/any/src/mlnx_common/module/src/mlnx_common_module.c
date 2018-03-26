/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <mlnx_common/mlnx_common_config.h>

#include "mlnx_common_log.h"

static int
datatypes_init__(void)
{
#define MLNX_COMMON_ENUMERATION_ENTRY(_enum_name, _desc)     AIM_DATATYPE_MAP_REGISTER(_enum_name, _enum_name##_map, _desc,                               AIM_LOG_INTERNAL);
#include <mlnx_common/mlnx_common.x>
    return 0;
}

void __mlnx_common_module_init__(void)
{
    AIM_LOG_STRUCT_REGISTER();
    datatypes_init__();
}
