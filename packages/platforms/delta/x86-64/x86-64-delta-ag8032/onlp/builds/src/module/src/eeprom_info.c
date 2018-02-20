
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
#include "tlvinfo.h"
#include "eeprom_info.h"

static struct tlv_code_desc fan_tray_tlv_code_list[] = {
    { FAN_ATTR_ID_PPID ,	"PPID",			TLV_TYPE_STRING},
    { FAN_ATTR_ID_DPN_REV,	"DPN Rev",		TLV_TYPE_STRING},
    { FAN_ATTR_ID_SERV_TAG,	"Service Tag",		TLV_TYPE_STRING},
    { FAN_ATTR_ID_PART_NUM,	"Part Number",		TLV_TYPE_STRING},
    { FAN_ATTR_ID_PART_NUM_REV,"Part Number Rev",	TLV_TYPE_STRING},
    { FAN_ATTR_ID_SN,		"Serial Number",	TLV_TYPE_STRING},
    { FAN_ATTR_ID_MFG_TEST_RES,"MFG Test Results",	TLV_TYPE_STRING},
    { FAN_ATTR_ID_NUM_OF_FAN,	"Number of Fans",	TLV_TYPE_STRING},
    { FAN_ATTR_ID_FAN_TYPE,	"Fan Type",		TLV_TYPE_STRING},
    { FAN_ATTR_ID_FAN_REV,	"Fan Rev",		TLV_TYPE_STRING},

    { TLV_CODE_CRC_32	         , "CRC-32",		TLV_TYPE_UINT32},

    { -1, NULL, -1}
};

static uint8_t fan_info_start[] = {0x00};

static struct tlv_code_desc psu_tray_tlv_code_list[] = {
    { PSU_ATTR_ID_PPID ,	"PPID",			TLV_TYPE_STRING},
    { PSU_ATTR_ID_DPN_REV,	"DPN Rev",		TLV_TYPE_STRING},
    { PSU_ATTR_ID_SERV_TAG,	"Service Tag",		TLV_TYPE_STRING},
    { PSU_ATTR_ID_PART_NUM,	"Part Number",		TLV_TYPE_STRING},
    { PSU_ATTR_ID_PART_NUM_REV,"Part Number Rev",	TLV_TYPE_STRING},
    { PSU_ATTR_ID_SN,		"Serial Number",	TLV_TYPE_STRING},
    { PSU_ATTR_ID_MFG_TEST_RES,"MFG Test Results",	TLV_TYPE_STRING},
    { PSU_ATTR_ID_PSU_TYPE,	"PSU Type",		TLV_TYPE_STRING},
    { PSU_ATTR_ID_FAB_REV,	"Fab Rev",		TLV_TYPE_STRING},

    { TLV_CODE_CRC_32	         , "CRC-32",		TLV_TYPE_UINT32},

    { -1, NULL, -1}
};
static uint8_t psu_info_start[] = { 0xa0, 0x00};

int fan_eeprom_parse (uint8_t *eeprom, int attr, char *v)
{
	int ret;
	int i;

	for (i = 0 ; i < sizeof(fan_info_start) ; i ++) {
		ret = tlvinfo_decode_tlv (fan_tray_tlv_code_list, 
				&eeprom[fan_info_start[i]], attr, v);
		if (ret)
			return 0;
	}

	return -1;
}

int psu_eeprom_parse (uint8_t *eeprom, int attr, char *v)
{
	int ret;
	int i;

	for (i = 0 ; i < sizeof(psu_info_start) ; i ++) {
		ret = tlvinfo_decode_tlv (psu_tray_tlv_code_list, 
				&eeprom[psu_info_start[i]], attr, v);
		if (ret)
			return 0;
	}

	return -1;
}


