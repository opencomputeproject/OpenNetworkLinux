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
 *
 *
 ***********************************************************/
#ifndef __ONLP_GPIO_H__
#define __ONLP_GPIO_H__

#include <onlplib/onlplib_config.h>

typedef enum onlp_gpio_direction_e {
    ONLP_GPIO_DIRECTION_NONE,
    ONLP_GPIO_DIRECTION_IN,
    ONLP_GPIO_DIRECTION_OUT,
    ONLP_GPIO_DIRECTION_LOW,
    ONLP_GPIO_DIRECTION_HIGH,
} onlp_gpio_direction_t;

/**
 * @brief Export the given GPIO and set its direction.
 * @param gpio The gpio number.
 * @param dir The gpio direction.
 */
int onlp_gpio_export(int gpio, onlp_gpio_direction_t dir);

/**
 * @brief Set the given GPIO value.
 * @param gpio The gpio number.
 * @param v The value to set.
 * @param ... Additional GPIO and Value parameters.
 */
int onlp_gpio_set(int gpio, int v);

/**
 * @brief Get the given GPIO value.
 * @param gpio The gpio number.
 * @returns Returns the value or negative on error.
 */
int onlp_gpio_get(int gpio, int* rv);


#endif /* __ONLP_GPIO_H__ */
