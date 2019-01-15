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
 * @brief Validate a thermal oid.
 * @param id The thermal id.
 */
int onlp_thermali_id_validate(onlp_oid_id_t id);

/**
 * @brief Retrieve the thermal's oid header.
 * @param id The thermal oid.
 * @param[out] rv Receives the header.
 */
int onlp_thermali_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* rv);

/**
 * @brief Get the information for the given thermal OID.
 * @param id The Thermal OID
 * @param[out] rv Receives the thermal information.
 */
int onlp_thermali_info_get(onlp_oid_id_t id, onlp_thermal_info_t* rv);

/**
 * This macro should be used in your implementation to declare
 * your static chassis thermal sensors.
 */
#define ONLP_THERMAL_INFO_ENTRY_INIT(_id, _desc, _parent)       \
    {                                                           \
        {                                                       \
            .id = ONLP_THERMAL_ID_CREATE(_id),                  \
                .description = _desc,                           \
                .poid = ONLP_OID_CHASSIS,                       \
                .status = ONLP_OID_STATUS_FLAG_PRESENT,         \
         },                                                     \
            .caps = ONLP_THERMAL_CAPS_GET_TEMPERATURE,          \
    }

/**
 * This macro should be used to statically initialize a chassis
 * thermal info structure.
 */
#define ONLP_CHASSIS_THERMAL_INFO_ENTRY_INIT(_id, _desc)        \
    ONLP_THERMAL_INFO_ENTRY_INIT(_id, _desc, ONLP_OID_CHASSIS)

/**
 * This macro should be used to statically initialize a PSU
 * thermal info structure.
 */
#define ONLP_PSU_THERMAL_INFO_ENTRY_INIT(_id, _desc, _psu_id) \
    ONLP_THERMAL_INFO_ENTRY_INIT(_id, _desc, ONLP_PSU_ID_CREATE(_psu_id))

#endif /* __ONLP_THERMALI_H__ */
/* @} */
