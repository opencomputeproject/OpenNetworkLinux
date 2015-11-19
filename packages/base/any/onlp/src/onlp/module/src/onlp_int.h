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
#ifndef __ONLP_INT_H__
#define __ONLP_INT_H__

#include <onlp/onlp_config.h>
#include <onlp/onlp.h>
#include <IOF/iof.h>
#include <onlp/oids.h>
#include <cjson/cJSON.h>
#include "onlp_json.h"

/** Default IOF initializations for dump() and show() routines */
void onlp_oid_show_iof_init_default(iof_t* iof, aim_pvs_t* pvs, uint32_t flags);
void onlp_oid_dump_iof_init_default(iof_t* iof, aim_pvs_t* pvs);

/** Default error message when the status of an OID cannot be retreived */
void onlp_oid_info_get_error(iof_t* iof, int error);

/** Standard OID description output */
void onlp_oid_show_description(iof_t* iof, onlp_oid_hdr_t* hdr);
/** Standard message when an OID is missing. */
void onlp_oid_show_state_missing(iof_t* iof);

#endif /* __ONLP_INT_H__ */
