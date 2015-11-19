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
#ifndef __ONLP_SNMP_SENSORS_H__
#define __ONLP_SNMP_SENSORS_H__

/**
 * This handler is compatible with the snmp_subagent client registration
 * system.
 */
int onlp_snmp_sensors_client(int enable, void* cookie);

#endif /* __ONLP_SNMP_SENSORS_H__ */
