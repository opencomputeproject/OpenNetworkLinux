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
 * @brief Fan Platform Implementation.
 * @addtogroup fani
 * @{
 *
 ***********************************************************/
#ifndef __ONLP_FANI_H__
#define __ONLP_FANI_H__

#include <onlp/fan.h>

/**
 * @brief Software initialization of the Fan module.
 */
int onlp_fani_sw_init(void);

/**
 * @brief Hardware initialization of the Fan module.
 * @param flags The hardware initialization flags.
 */
int onlp_fani_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the fan software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_fani_sw_denit(void);

/**
 * @brief Retrieve the fan's OID hdr.
 * @param id The fan id.
 * @param[out] hdr Receives the OID header.
 */
int onlp_fani_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr);

/**
 * @brief Get the information structure for the given fan OID.
 * @param id The fan id
 * @param[out] rv Receives the fan information.
 */
int onlp_fani_info_get(onlp_oid_id_t id, onlp_fan_info_t* rv);

/**
 * @brief Get the fan capabilities.
 * @param id The fan id.
 * @param[out] rv The fan capabilities
 */
int onlp_fani_caps_get(onlp_oid_id_t id, uint32_t* rv);

/**
 * @brief Set the fan speed in RPM.
 * @param id The fan OID
 * @param rpm The new RPM
 * @note This is only relevant if the RPM capability is set.
 */
int onlp_fani_rpm_set(onlp_oid_id_t id, int rpm);


/**
 * @brief Set the fan speed in percentage.
 * @param id The fan OID.
 * @param p The new fan speed percentage.
 * @note This is only relevant if the PERCENTAGE capability is set.
 */
int onlp_fani_percentage_set(onlp_oid_id_t id, int p);

/**
 * @brief Set the fan direction (if supported).
 * @param id The fan OID
 * @param dir The direction.
 */
int onlp_fani_dir_set(onlp_oid_id_t id, onlp_fan_dir_t dir);


#endif /* __ONLP_FANI_H__ */
/* @} */
