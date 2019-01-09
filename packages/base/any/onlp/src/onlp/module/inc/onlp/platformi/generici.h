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
 * @brief Generic OID Platform Implementation.
 * @addtogroup generici
 * @{
 *
 ***********************************************************/
#ifndef __ONLP_GENERICI_H__
#define __ONLP_GENERICI_H__

#include <onlp/generic.h>

/**
 * @brief Software initialization of the Generic module.
 */
int onlp_generici_sw_init(void);

/**
 * @brief Hardware initialization of the Generic module.
 * @param flags The hardware initialization flags.
 */
int onlp_generici_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the generic software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_generici_sw_denit(void);

/**
 * @brief Retrieve the generic's oid header.
 * @param id The generic oid.
 * @param[out] rv Receives the header.
 */
int onlp_generici_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* rv);

/**
 * @brief Get the information for the given generic OID.
 * @param id The Generic OID
 * @param[out] rv Receives the generic information.
 */
int onlp_generici_info_get(onlp_oid_id_t id, onlp_generic_info_t* rv);

#endif /* __ONLP_GENERICI_H__ */
/* @} */
