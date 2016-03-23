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
#include <onlp/onlp_config.h>
#include <onlp/onlp.h>
#include "onlp_int.h"

/**
 * The OID dump() and show() routines
 * need a default IOF.
 */
void
onlp_oid_dump_iof_init_default(iof_t* iof, aim_pvs_t* pvs)
{
    if(iof_init(iof, pvs) == 0) {
        /* Default settings */
        iof->indent_factor=4;
        iof->level=1;
        iof->indent_terminator="";
    }
}
void
onlp_oid_show_iof_init_default(iof_t* iof, aim_pvs_t* pvs, uint32_t flags)
{
    if(iof_init(iof, pvs) == 0) {
        /* Default settings */
        iof->indent_factor=2;
        iof->level=1;
        iof->indent_terminator="";
        iof->pop_string = NULL;
        iof->push_string = "";
    }
}

void
onlp_oid_info_get_error(iof_t* iof, int error)
{
    iof_iprintf(iof, "Error retrieving status: %{onlp_status}", error);
}
void
onlp_oid_show_description(iof_t* iof, onlp_oid_hdr_t* hdr)
{
    iof_iprintf(iof, "Description: %s", hdr->description);
}

void
onlp_oid_show_state_missing(iof_t* iof)
{
    iof_iprintf(iof, "State: Missing");
}
