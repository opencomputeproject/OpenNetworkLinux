/**************************************************************
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
 **************************************************************
 *
 * Common thermal support routines.
 *
 ************************************************************/
#ifndef __ONLPLIB_THERMAL_H__
#define __ONLPLIB_THERMAL_H__
#include <onlplib/onlplib_config.h>
#include <onlp/thermal.h>
/**
 * @brief Read the mcelsius value from the given file.
 * @param fname Filename
 * @param info Thermal info structure.
 */
int onlplib_thermal_read_file(const char* fname, onlp_thermal_info_t* info);

#endif /* __ONLPLIB_THERMAL_H__ */
