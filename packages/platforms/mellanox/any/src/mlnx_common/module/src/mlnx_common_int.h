/**************************************************************************//**
 *
 * mlnx_common Internal Header
 *
 *****************************************************************************/
#ifndef __MLNX_COMMON_INT_H__
#define __MLNX_COMMON_INT_H__

#include <mlnx_common/mlnx_common_config.h>
#include <mlnx_common/mlnx_common.h>

#define MAX_NUM_OF_CPLD				3
#define PREFIX_PATH_ON_CPLD_DEV		"/bsp/cpld"

mlnx_platform_info_t* get_platform_info(void);

#endif /* __MLNX_COMMON_INT_H__ */
