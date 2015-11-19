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

#ifndef __ONLP_JSON_H__
#define __ONLP_JSON_H__

#include <onlp/onlp_config.h>
#include "onlp_int.h"
#include <cjson_util/cjson_util.h>

/**
 * @brief Initialize the JSON configuration data.
 * @param fname JSON configuration filename.
 */
void onlp_json_init(const char* fname);

/**
 * @brief Get the JSON configuration root.
 * @param reload Option to reload the config file first.
 */
cJSON* onlp_json_get(int reload);

void onlp_json_denit(void);


#endif /* __ONLP_JSON_H__ */
