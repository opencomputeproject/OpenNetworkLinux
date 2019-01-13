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
 ************************************************************
 *
 * LED Platform Implementation.
 *
 ***********************************************************/
#ifndef __ONLP_LEDI_H__
#define __ONLP_LEDI_H__

#include <onlp/led.h>

/**
 * @brief Software initialization of the LED module.
 */
int onlp_ledi_sw_init(void);

/**
 * @brief Hardware initialization of the LED module.
 * @param flags The hardware initialization flags.
 */
int onlp_ledi_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the led software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_ledi_sw_denit(void);

/**
 * @brief Get the LED header.
 * @param id The LED OID
 * @param[out] rv  Receives the header.
 */
int onlp_ledi_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* rv);

/**
 * @brief Get the information for the given LED
 * @param id The LED OID
 * @param[out] rv  Receives the LED information.
 */
int onlp_ledi_info_get(onlp_oid_id_t id, onlp_led_info_t* rv);

/**
 * @brief Get the caps for the given LED
 * @param id The LED ID
 * @param[out] rv Receives the caps.
 */
int onlp_ledi_caps_get(onlp_oid_id_t id, uint32_t* rv);

/**
 * @brief Set the LED mode.
 * @param id The LED OID
 * @param mode The new mode.
 * @notes Only called if the mode is advertised in the LED capabilities.
 */
int onlp_ledi_mode_set(onlp_oid_id_t id, onlp_led_mode_t mode);

/**
 * @brief Set the LED character.
 * @param id The LED OID
 * @param c The character..
 * @notes Only called if the char capability is set.
 */
int onlp_ledi_char_set(onlp_oid_id_t id, char c);

#define ONLP_LED_INFO_ENTRY_INIT(_id, _desc, _parent, _caps)           \
    {                                                           \
        {                                                       \
            .id = ONLP_LED_ID_CREATE(_id),                  \
            .description = _desc,                           \
            .poid = ONLP_OID_CHASSIS,                       \
            .status = ONLP_OID_STATUS_FLAG_PRESENT,         \
         },                                              \
         .caps = _caps,                               \
     }

#define ONLP_CHASSIS_LED_INFO_ENTRY_INIT(_id, _desc, _caps) \
    ONLP_LED_INFO_ENTRY_INIT(_id, _desc, ONLP_OID_CHASSIS, _caps)

#define ONLP_PSU_LED_INFO_ENTRY_INIT(_id, _desc, _psu_id, _caps) \
    ONLP_LED_INFO_ENTRY_INIT(_id, _desc, ONLP_PSU_ID_CREATE(_psu_id), _caps)

#define ONLP_FAN_LED_INFO_ENTRY_INIT(_id, _desc, _fan_id, _caps) \
    ONLP_LED_INFO_ENTRY_INIT(_id, _desc, ONLP_FAN_ID_CREATE(_fan_id), _caps)

#endif /* __ONLP_LED_H__ */
