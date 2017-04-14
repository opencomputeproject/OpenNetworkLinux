/************************************************************
 * <bsn.cl fy=2017 v=onl>
 *
 *        Copyright 2017 Delta Networks, Inc.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#ifndef __PLATFORM_LIB_H__
#define __PLATFORM_LIB_H__

#include "x86_64_delta_wb2448_log.h"

typedef unsigned int    UINT4;
typedef unsigned short  UINT2;
typedef unsigned char   UINT1;
typedef int             INT4;
typedef short           INT2;
typedef char            INT1;

#define OS_MAX_MSG_SIZE 100

INT4 ifnOS_LINUX_BmcI2CGet(UINT1 u1Bus, UINT1 u1Dev, UINT4 u4Addr, UINT1 u1AddrLen, UINT4 *pu4RetData, UINT1 u1DataLen);
INT4 ifnOS_LINUX_BmcI2CSet(UINT1 u1Bus, UINT1 u1Dev, UINT4 u4Addr, UINT1 u1AddrLen, UINT4 u4Data, UINT1 u1DataLen);
INT4 ifnOS_LINUX_BmcI2CProbe(UINT1 u1Bus, UINT1 u1Dev);
INT4 ifnBmcFanSpeedGet(INT1 *pi1FanName, UINT4 *pu4RetData);
INT4 ifnBmcFanSpeedSet(UINT4 u4FanNumber, UINT4 u4Percentage);
UINT4 MMI_XTOI (const UINT1* str);

#endif  /* __PLATFORM_LIB_H__ */

