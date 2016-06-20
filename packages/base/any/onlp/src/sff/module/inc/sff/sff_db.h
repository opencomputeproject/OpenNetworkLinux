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
#ifndef __SFF_DB_H__
#define __SFF_DB_H__

#include <sff/sff_config.h>
#include <sff/sff.h>
#include <AIM/aim_pvs.h>

typedef struct {
    sff_eeprom_t se;
} sff_db_entry_t;

/**
 * @brief Get the database entry table.
 * @param entries Receives the table pointer.
 * @param count Receives the size of the table.
 */
int sff_db_get(sff_db_entry_t** entries, int* count);

/**
 * @brief Return any entry with the given module type.
 * @param se Receives the information struct.
 * @param type The type to retreive.
 */
int sff_db_get_type(sff_eeprom_t* se, sff_module_type_t type);


/**
 * @brief Output the given SFF information to a database entry.
 * @param info The source information.
 * @param pvs The output pvs.;
 * @note This is used mainly for generating new entries for the SFF db from a running system.
 */
int sff_db_entry_struct(sff_eeprom_t* se, aim_pvs_t* pvs);

#endif /* __SFF_DB_H__ */

