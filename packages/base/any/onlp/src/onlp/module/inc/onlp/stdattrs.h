/************************************************************
 * <bsn.cl fy=2017 v=onl>
 *
 *        Copyright 2017 Big Switch Networks, Inc.
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
 ********************************************************//**
 *
 * @file
 * @brief Standard OID Attributes
 * @addtogroup stdattrs
 * @{
 *
 ***********************************************************/
#ifndef __ONLP_STDATTRS_H__
#define __ONLP_STDATTRS_H__

#include <onlp/oids.h>
#include <onlplib/onie.h>

#include <AIM/aim_pvs.h>

/**
 * @brief This structure describes general asset information
 * that may be associated with a particular OID.
 */
typedef struct onlp_asset_info_s {
    /** The OID to which this asset info belongs */
    onlp_oid_t oid;

    /** Manufacturer */
    char* manufacturer;

    /** Date */
    char* date;

    /** Part Number */
    char* part_number;

    /** Serial Number */
    char* serial_number;

    /** Hardware Revision */
    char* hardware_revision;

    /** Firmware Revision */
    char* firmware_revision;

    /** CPLD Revision */
    char* cpld_revision;

    /** Manufacture Date */
    char* manufacture_date;

    /** Description */
    char* description;

    /** Additional Information */
    char* additional;

} onlp_asset_info_t;


/**
 * @brief Show an asset structure.
 */
int onlp_asset_info_show(onlp_asset_info_t* aip, aim_pvs_t* pvs);

/**
 * @brief Free an asset structure.
 */
int onlp_asset_info_free(onlp_asset_info_t* aip);

/**
 * @brief Asset info to JSON.
 */
int onlp_asset_info_to_json(onlp_asset_info_t* aip, cJSON** rv);

/* See onlplib/onie.h for the onie_info versions */

#endif /* __ONLP_STDATTRS_H__ */
/* @} */
