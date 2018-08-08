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
 * LED Management
 *
 ***********************************************************/
#ifndef __ONLP_LED_H__
#define __ONLP_LED_H__

#include <onlp/onlp.h>
#include <onlp/oids.h>

/* <auto.start.enum(tag:led).define> */
/** onlp_led_caps */
typedef enum onlp_led_caps_e {
    ONLP_LED_CAPS_OFF = (1 << 0),
    ONLP_LED_CAPS_AUTO = (1 << 1),
    ONLP_LED_CAPS_AUTO_BLINKING = (1 << 2),
    ONLP_LED_CAPS_CHAR = (1 << 3),
    ONLP_LED_CAPS_RED = (1 << 4),
    ONLP_LED_CAPS_RED_BLINKING = (1 << 5),
    ONLP_LED_CAPS_ORANGE = (1 << 6),
    ONLP_LED_CAPS_ORANGE_BLINKING = (1 << 7),
    ONLP_LED_CAPS_YELLOW = (1 << 8),
    ONLP_LED_CAPS_YELLOW_BLINKING = (1 << 9),
    ONLP_LED_CAPS_GREEN = (1 << 10),
    ONLP_LED_CAPS_GREEN_BLINKING = (1 << 11),
    ONLP_LED_CAPS_BLUE = (1 << 12),
    ONLP_LED_CAPS_BLUE_BLINKING = (1 << 13),
    ONLP_LED_CAPS_PURPLE = (1 << 14),
    ONLP_LED_CAPS_PURPLE_BLINKING = (1 << 15),
} onlp_led_caps_t;

/** onlp_led_mode */
typedef enum onlp_led_mode_e {
    ONLP_LED_MODE_OFF,
    ONLP_LED_MODE_AUTO,
    ONLP_LED_MODE_AUTO_BLINKING,
    ONLP_LED_MODE_CHAR,
    ONLP_LED_MODE_RED,
    ONLP_LED_MODE_RED_BLINKING,
    ONLP_LED_MODE_ORANGE,
    ONLP_LED_MODE_ORANGE_BLINKING,
    ONLP_LED_MODE_YELLOW,
    ONLP_LED_MODE_YELLOW_BLINKING,
    ONLP_LED_MODE_GREEN,
    ONLP_LED_MODE_GREEN_BLINKING,
    ONLP_LED_MODE_BLUE,
    ONLP_LED_MODE_BLUE_BLINKING,
    ONLP_LED_MODE_PURPLE,
    ONLP_LED_MODE_PURPLE_BLINKING,
    ONLP_LED_MODE_LAST = ONLP_LED_MODE_PURPLE_BLINKING,
    ONLP_LED_MODE_COUNT,
    ONLP_LED_MODE_INVALID = -1,
} onlp_led_mode_t;
/* <auto.end.enum(tag:led).define> */


/**
 * LED information structure.
 */
typedef struct onlp_led_info_s {
    /** Header */
    onlp_oid_hdr_t hdr;

    /** Capabilities */
    uint32_t caps;

    /** Current mode, if capable. */
    onlp_led_mode_t mode;

    /** Current char, if capable. */
    char character;

} onlp_led_info_t;

/**
 * @brief Software initialization of the LED module.
 */
int onlp_led_sw_init(void);

/**
 * @brief Hardware initialization of the LED module.
 */
int onlp_led_hw_init(uint32_t flags);

/**
 * @brief Deinitialize the led software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_led_sw_denit(void);

/**
 * @brief Get the LED header.
 * @param id The LED OID
 * @param[out] rv Receives the header.
 */
int onlp_led_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv);


/**
 * @brief Get LED information.
 * @param id The LED OID
 * @param[out] rv Receives the information structure.
 */
int onlp_led_info_get(onlp_oid_t id, onlp_led_info_t* rv);

/**
 * @brief Set the LED color
 * @param id The LED OID
 * @param color The color.
 * @note Only relevant if the LED supports the color capability.
 */
int onlp_led_mode_set(onlp_oid_t id, onlp_led_mode_t mode);


/**
 * @brief Set the LED char
 * @param id The LED OID
 * @param c The character.
 * @note Only relevant if the LED supports the char capability.
 */
int onlp_led_char_set(onlp_oid_t id, char c);

/**
 * @brief Convert an LED info structure to user JSON.
 * @param info The LED info structure.
 * @param [out] cj Receives the JSON object.
 * @param flags The JSON format flags.
 */
int onlp_led_info_to_user_json(onlp_led_info_t* info, cJSON** cj, uint32_t flags);

/**
 * @brief Convert an LED info structure to JSON.
 * @param info The LED info structure.
 * @param [out] cj Receives the JSON object.
 * @param flags The JSON format flags.
 */
int onlp_led_info_to_json(onlp_led_info_t* info, cJSON** cj, uint32_t flags);

/**
 * @brief Convert a JSON object to an LED info structure.
 * @param cj The JSON oibject.
 * @param [out] info Receives the LED info structure.
 */
int onlp_led_info_from_json(cJSON* cj, onlp_led_info_t* info);

/******************************************************************************
 *
 * Enumeration Support Definitions.
 *
 * Please do not add additional code beyond this point.
 *
 *****************************************************************************/
/* <auto.start.enum(tag:led).supportheader> */
/** Enum names. */
const char* onlp_led_caps_name(onlp_led_caps_t e);

/** Enum values. */
int onlp_led_caps_value(const char* str, onlp_led_caps_t* e, int substr);

/** Enum descriptions. */
const char* onlp_led_caps_desc(onlp_led_caps_t e);

/** Enum validator. */
int onlp_led_caps_valid(onlp_led_caps_t e);

/** validator */
#define ONLP_LED_CAPS_VALID(_e) \
    (onlp_led_caps_valid((_e)))

/** onlp_led_caps_map table. */
extern aim_map_si_t onlp_led_caps_map[];
/** onlp_led_caps_desc_map table. */
extern aim_map_si_t onlp_led_caps_desc_map[];

/** Strings macro. */
#define ONLP_LED_MODE_STRINGS \
{\
    "OFF", \
    "AUTO", \
    "AUTO_BLINKING", \
    "CHAR", \
    "RED", \
    "RED_BLINKING", \
    "ORANGE", \
    "ORANGE_BLINKING", \
    "YELLOW", \
    "YELLOW_BLINKING", \
    "GREEN", \
    "GREEN_BLINKING", \
    "BLUE", \
    "BLUE_BLINKING", \
    "PURPLE", \
    "PURPLE_BLINKING", \
}
/** Enum names. */
const char* onlp_led_mode_name(onlp_led_mode_t e);

/** Enum values. */
int onlp_led_mode_value(const char* str, onlp_led_mode_t* e, int substr);

/** Enum descriptions. */
const char* onlp_led_mode_desc(onlp_led_mode_t e);

/** validator */
#define ONLP_LED_MODE_VALID(_e) \
    ( (0 <= (_e)) && ((_e) <= ONLP_LED_MODE_PURPLE_BLINKING))

/** onlp_led_mode_map table. */
extern aim_map_si_t onlp_led_mode_map[];
/** onlp_led_mode_desc_map table. */
extern aim_map_si_t onlp_led_mode_desc_map[];
/* <auto.end.enum(tag:led).supportheader> */

#endif /* __ONLP_LED_H__ */
