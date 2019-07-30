/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2016 NXP Semiconductor, Inc.
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

#ifndef __ARM64_NXP_LAYERSCAPE_INT_H__
#define __ARM64_NXP_LAYERSCAPE_INT_H__

#include <arm64_nxp_layerscape/arm64_nxp_layerscape_config.h>

/* <auto.start.enum(ALL).header> */
/** platform_id */
typedef enum platform_id_e {
    PLATFORM_ID_ARM64_NXP_LS2088ARDB_R0,
    PLATFORM_ID_ARM64_NXP_LS1043ARDB_R0,
    PLATFORM_ID_ARM64_NXP_LS1046ARDB_R0,
    PLATFORM_ID_ARM64_NXP_LS1088ARDB_R0,
    PLATFORM_ID_LAST = PLATFORM_ID_ARM64_NXP_LS1088ARDB_R0,
    PLATFORM_ID_COUNT,
    PLATFORM_ID_INVALID = -1,
} platform_id_t;

/** Strings macro. */
#define PLATFORM_ID_STRINGS \
{\
    "ARM64_NXP_LS2088ARDB_R0", \
    "ARM64_NXP_LS1043ARDB_R0", \
    "ARM64_NXP_LS1046ARDB_R0", \
    "ARM64_NXP_LS1088ARDB_R0", \
}
/** Enum names. */
const char* platform_id_name(platform_id_t e);

/** Enum values. */
int platform_id_value(const char* str, platform_id_t* e, int substr);

/** Enum descriptions. */
const char* platform_id_desc(platform_id_t e);

/** validator */
#define PLATFORM_ID_VALID(_e) \
    ( (0 <= (_e)) && ((_e) <= PLATFORM_ID_ARM64_NXP_LS1088ARDB_R0))

/** platform_id_map table. */
extern aim_map_si_t platform_id_map[];
/** platform_id_desc_map table. */
extern aim_map_si_t platform_id_desc_map[];
/* <auto.end.enum(ALL).header> */

#endif /* __ARM64_NXP_LAYERSCAPE_INT_H__ */
