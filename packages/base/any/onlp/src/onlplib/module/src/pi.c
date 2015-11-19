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

#include <onlplib/pi.h>
#include <IOF/iof.h>

void
onlp_platform_info_show(onlp_platform_info_t* pi, aim_pvs_t* pvs)
{
    iof_t iof;
    iof_init(&iof, pvs);
    if(pi->cpld_versions) {
        iof_iprintf(&iof, "CPLD Versions: %s", pi->cpld_versions);
    }
    if(pi->other_versions) {
        iof_iprintf(&iof, "Other Versions: %s", pi->other_versions);
    }
}

void
onlp_platform_info_show_json(onlp_platform_info_t* pi, aim_pvs_t* pvs)
{
    aim_printf(pvs, "{\n");

#define STROUT(_name, _member, _comma)                            \
    do {                                                          \
        aim_printf(pvs, "    \"%s\" : ", #_name);                 \
        if(pi-> _member) {                                        \
            aim_printf(pvs, "\"%s\"%s\n", pi->_member, _comma);   \
        }                                                         \
        else {                                                    \
            aim_printf(pvs, "null%s\n", _comma );                 \
        }                                                         \
    } while(0)

    STROUT(CPLD Versions, cpld_versions, ",");
    STROUT(Other Versions, other_versions, "");

    aim_printf(pvs, "}\n");
}
