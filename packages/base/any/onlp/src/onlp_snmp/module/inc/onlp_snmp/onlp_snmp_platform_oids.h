/************************************************************
 * <bsn.cl fy=2015 v=onl>
 * 
 *           Copyright 2015 Big Switch Networks, Inc.          
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
#ifndef __ONLP_SNMP_PLATFORM_OIDS_H__
#define __ONLP_SNMP_PLATFORM_OIDS_H__

/**
 * See:
 *      ONLP-PLATFORM-MIBS.txt
 */

/**
 * These are all currently defined as part of the Big Switch enterprise tree.
 *
 * In the future we should apply for an ONL Private Enterprise Number and
 * redefine these appropriately.
 */

#define ONLP_SNMP_PLATFORM_INFORMATION_OID          1,3,6,1,4,1,37538,2,1000
#define ONLP_SNMP_PLATFORM_GENERAL_OID ONLP_SNMP_PLATFORM_INFORMATION_OID,1
#define ONLP_SNMP_PLATFORM_SYSTEM_OID ONLP_SNMP_PLATFORM_GENERAL_OID,1


/**
 * TODO
 */


#endif /* __ONLP_SNMP_PLATFORM_OIDS_H__ */




