/**************************************************************************//**
 *
 * mlnx_platform_common Internal Header
 *
 *****************************************************************************/
#ifndef __MLNX_PLATFORM_COMMON_INT_H__
#define __MLNX_PLATFORM_COMMON_INT_H__

#include <mlnx_platform_common/mlnx_platform_common_config.h>

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

int mpc_get_kernel_ver(void);

#endif /* __MLNX_PLATFORM_COMMON_INT_H__ */
