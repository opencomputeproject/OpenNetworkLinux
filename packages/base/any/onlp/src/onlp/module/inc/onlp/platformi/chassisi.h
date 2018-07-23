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
 * @brief Chassis Platform Interface.
 * @addtogroup chassisi
 * @{
 *
 ***********************************************************/
#ifndef __ONLP_CHASSISI_H__
#define __ONLP_CHASSISI_H__

#include <onlp/chassis.h>

/**
 * @brief Software initializaiton of the Chassis module.
 */
int onlp_chassisi_sw_init(void);

/**
 * @brief Hardware initializaiton of the Chassis module.
 * @param flags The hardware initialization flags.
 */
int onlp_chassisi_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the chassis software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_chassisi_sw_denit(void);


/**
 * @brief Get the chassis hdr structure.
 * @param oid The Chassis OID.
 * @param[out] hdr Receives the header.
 */
int onlp_chassisi_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr);

/**
 * @brief Get the chassis info structure.
 * @param oid The Chassis OID.
 * @param[out] info Receives the chassis information.
 */
int onlp_chassisi_info_get(onlp_oid_t oid, onlp_chassis_info_t* info);

#endif /* __ONLP_CHASSISI_H__ */
/* @} */
