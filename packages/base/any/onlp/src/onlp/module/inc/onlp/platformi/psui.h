/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
 ********************************************************//**
 *
 * @file
 * @brief Power Supply Management Implementation.
 * @addtogroup psui
 * @{
 *
 ***********************************************************/
#ifndef __ONLP_PSUI_H__
#define __ONLP_PSUI_H__

#include <onlp/psu.h>

/**
 * @brief Software initialization of the PSU module.
 */
int onlp_psui_sw_init(void);

/**
 * @brief Hardware initialization of the PSU module.
 * @param flags The hardware initialization flags.
 */
int onlp_psui_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the psu software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_psui_sw_denit(void);

/**
 * @brief Get the PSU's oid header.
 * @param id The PSU OID.
 * @param[out] rv Receives the header.
 */
int onlp_psui_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* rv);

/**
 * @brief Get the information structure for the given PSU
 * @param id The PSU OID
 * @param[out] rv Receives the PSU information.
 */
int onlp_psui_info_get(onlp_oid_id_t id, onlp_psu_info_t* rv);


/**
 * @brief Initialize a static PSU info structure.
 */
#define ONLP_CHASSIS_PSU_INFO_ENTRY_INIT(_id, _desc)    \
    {                                                   \
        {                                               \
            .id = _id,                                  \
            .description = _desc,                   \
            .poid = ONLP_OID_CHASSIS,               \
        },                                      \
    }

#endif /* __ONLP_PSUI_H__ */
/* @} */
