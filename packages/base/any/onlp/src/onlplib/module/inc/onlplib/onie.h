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
 * Common ONIE Infrastructure Support.
 *
 ***********************************************************/
#ifndef __ONLP_ONIE_H__
#define __ONLP_ONIE_H__

#include <stdint.h>
#include <AIM/aim_pvs.h>
#include <IOF/iof.h>
#include <AIM/aim_list.h>

/**
 * The ONIE specification defines the format of the system
 * eeprom and the available fields that may be described there.
 *
 * This structure contains the decoded fields for application
 * and platform use.
 */

typedef struct onlp_onie_info_s {

    char* product_name;
    char* part_number;
    char* serial_number;
    uint8_t     mac[6];
    char* manufacture_date;
    uint8_t     device_version;
    char* label_revision;
    char* platform_name;
    char* onie_version;
    uint16_t    mac_range;
    char* manufacturer;
    char* country_code;
    char* vendor;
    char* diag_version;
    char* service_tag;
    uint32_t    crc;

    /**
     * Vendor Extensions list, if available.
     */
    list_head_t vx_list;

    /* Internal/debug */
    char* _hdr_id_string;
    uint8_t     _hdr_version;
    uint8_t     _hdr_length;
    uint8_t     _hdr_valid_crc;

} onlp_onie_info_t;


typedef struct onlp_onie_vx_s {
    list_links_t links;
    uint8_t data[256];
    int size;
} onlp_onie_vx_t;

/**
 * Support for parsing ONIE eeprom data into the
 * ONIE information structure is provided for all platforms.
 */

int onlp_onie_decode(onlp_onie_info_t* rv, const uint8_t* data, int size);
int onlp_onie_decode_file(onlp_onie_info_t* rv, const char* file);

/**
 * Free an ONIE info structure.
 */
void onlp_onie_info_free(onlp_onie_info_t* info);

/**
 * Show the contents of an ONIE info structure.
 */
void onlp_onie_show(onlp_onie_info_t* info, aim_pvs_t* pvs);

/**
 * Dump the contents of an ONIE info structure as JSON
 */
void onlp_onie_show_json(onlp_onie_info_t* info, aim_pvs_t* pvs);

/**
 * Read ONIE fields from a JSON file.
 */
int onlp_onie_read_json(onlp_onie_info_t* info, const char* fname);

#endif /* __ONLP_ONIE_H__ */
