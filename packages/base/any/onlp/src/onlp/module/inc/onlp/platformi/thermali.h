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
 * @brief Thermal Sensor Platform Implementation.
 * @addtogroup thermali
 * @{
 *
 ***********************************************************/
#ifndef __ONLP_THERMALI_H__
#define __ONLP_THERMALI_H__

#include <onlp/thermal.h>

/**
 * @brief Software initialization of the Thermal module.
 */
int onlp_thermali_sw_init(void);

/**
 * @brief Hardware initialization of the Thermal module.
 * @param flags The hardware initialization flags.
 */
int onlp_thermali_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the thermal software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_thermali_sw_denit(void);

/**
 * @brief Retrieve the thermal's oid header.
 * @param id The thermal oid.
 * @param[out] rv Receives the header.
 */
int onlp_thermali_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv);

/**
 * @brief Get the information for the given thermal OID.
 * @param id The Thermal OID
 * @param[out] rv Receives the thermal information.
 */
int onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* rv);

#endif /* __ONLP_THERMALI_H__ */
/* @} */
