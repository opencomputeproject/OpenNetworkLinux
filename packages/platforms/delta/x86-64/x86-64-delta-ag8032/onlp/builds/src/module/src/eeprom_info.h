
/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 (C) Delta Networks, Inc.
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

#ifndef __EEPROM_INFO__
#define __EEPROM_INFO__

#include "onlp/onlp_config.h"

#define FAN_ATTR_VALUE_MAX_SIZE		ONLP_CONFIG_INFO_STR_MAX

#define FAN_ATTR_ID_PPID			0x00
#define FAN_ATTR_ID_DPN_REV			0x14
#define FAN_ATTR_ID_SERV_TAG		0x17
#define FAN_ATTR_ID_PART_NUM		0x1e
#define FAN_ATTR_ID_PART_NUM_REV	0x28
#define FAN_ATTR_ID_SN				0x23
#define FAN_ATTR_ID_MFG_TEST_RES	0x2b
#define FAN_ATTR_ID_NUM_OF_FAN		0x2d
#define FAN_ATTR_ID_FAN_TYPE		0x2f
#define FAN_ATTR_ID_FAN_REV			0x2e

extern int fan_eeprom_parse (uint8_t *eeprom, int attr, char *v);

#define PSU_ATTR_ID_PPID			0x00
#define PSU_ATTR_ID_DPN_REV			0x14
#define PSU_ATTR_ID_SERV_TAG		0x17
#define PSU_ATTR_ID_PART_NUM		0x22
#define PSU_ATTR_ID_PART_NUM_REV	0x28
#define PSU_ATTR_ID_SN				0x23
#define PSU_ATTR_ID_MFG_TEST_RES	0x2b
#define PSU_ATTR_ID_PSU_TYPE		0x2d
#define PSU_ATTR_ID_FAB_REV			0x2e

extern int psu_eeprom_parse (uint8_t *eeprom, int attr, char *v);

#endif // __EEPROM_INFO__

