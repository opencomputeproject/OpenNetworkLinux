#ifndef __PLATFORM_LIB_H__
#define __PLATFORM_LIB_H__

#include "x86_64_dni_wb2448_log.h"

typedef unsigned int    UINT4;
typedef unsigned short  UINT2;
typedef unsigned char   UINT1;
typedef int             INT4;
typedef short			INT2;
typedef char            INT1;

#define OS_MAX_MSG_SIZE		100

INT4 ifnOS_LINUX_BmcI2CGet(UINT1 u1Bus, UINT1 u1Dev, UINT4 u4Addr, UINT1 u1AddrLen, UINT4 *pu4RetData, UINT1 u1DataLen);
INT4 ifnOS_LINUX_BmcI2CSet(UINT1 u1Bus, UINT1 u1Dev, UINT4 u4Addr, UINT1 u1AddrLen, UINT4 u4Data, UINT1 u1DataLen);
INT4 ifnOS_LINUX_BmcI2CProbe(UINT1 u1Bus, UINT1 u1Dev);
INT4 ifnBmcFanSpeedGet(INT1 *pi1FanName, UINT4 *pu4RetData);
INT4 ifnBmcFanSpeedSet(UINT4 u4FanNumber, UINT4 u4Percentage);
UINT4 MMI_XTOI (const UINT1* str);

#endif  /* __PLATFORM_LIB_H__ */

