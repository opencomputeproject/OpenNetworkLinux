/****************************************************************************
 *
 *
 *
 *****************************************************************************/
#include <powerpc_nxp_t2080rdb/powerpc_nxp_t2080rdb_config.h>

#include "powerpc_nxp_t2080rdb_log.h"

static int datatypes_init__(void)
{
#define powerpc_nxp_T2080RDB_R0_ENUMERATION_ENTRY(_enum_name, _desc)     AIM_DATATYPE_MAP_REGISTER(_enum_name, _enum_name##_map, _desc, AIM_LOG_INTERNAL);
#include <powerpc_nxp_t2080rdb/powerpc_nxp_t2080rdb.x>
    return 0;
}

void __powerpc_nxp_t2080rdb_module_init__(void)
{
    AIM_LOG_STRUCT_REGISTER();
    datatypes_init__();
}

int __onlp_platform_version__ = 1;
