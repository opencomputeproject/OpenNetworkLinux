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
#include <onlp/onlp.h>
#include <onlp/sys.h>
#include <onlp/oids.h>
#include <onlp/sfp.h>

void
onlp_platform_dump(aim_pvs_t* pvs, uint32_t flags)
{
    /* Dump all OIDS, starting with the SYS OID */
    onlp_oid_dump(ONLP_OID_SYS, pvs, flags);
    aim_printf(pvs, "\nSFPs:\n");
    /* Dump all SFPs */
    onlp_sfp_dump(pvs);
}

void
onlp_platform_show(aim_pvs_t* pvs, uint32_t flags)
{
    onlp_oid_show(ONLP_OID_SYS, pvs, flags);
}
