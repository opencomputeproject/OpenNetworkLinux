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
 * ONLP Platform Object Identifiers.
 *
 ***********************************************************/
#ifndef __ONLP_OID_H__
#define __ONLP_OID_H__

#include <onlp/onlp_config.h>

#include <stdint.h>
#include <AIM/aim_pvs.h>
#include <IOF/iof.h>

/**
 * System peripherals are identified by a 32bit OID.
 *
 * The First byte is the object-class identifier:
 *    Thermal sensor object
 *    Fan object
 *    PSU object
 *    LED object
 *    MODULE object
 *    etc..
 * The remaining bytes are the object id.
 */

typedef uint32_t onlp_oid_t;

/* <auto.start.enum(tag:oid).define> */
/** onlp_oid_dump */
typedef enum onlp_oid_dump_e {
    ONLP_OID_DUMP_RECURSE = (1 << 0),
    ONLP_OID_DUMP_EVEN_IF_ABSENT = (1 << 1),
} onlp_oid_dump_t;

/** onlp_oid_show */
typedef enum onlp_oid_show_e {
    ONLP_OID_SHOW_RECURSE = (1 << 0),
    ONLP_OID_SHOW_EXTENDED = (1 << 1),
    ONLP_OID_SHOW_YAML = (1 << 2),
} onlp_oid_show_t;

/** onlp_oid_type */
typedef enum onlp_oid_type_e {
    ONLP_OID_TYPE_SYS = 1,
    ONLP_OID_TYPE_THERMAL = 2,
    ONLP_OID_TYPE_FAN = 3,
    ONLP_OID_TYPE_PSU = 4,
    ONLP_OID_TYPE_LED = 5,
    ONLP_OID_TYPE_MODULE = 6,
    ONLP_OID_TYPE_RTC = 7,
} onlp_oid_type_t;
/* <auto.end.enum(tag:oid).define> */


/**
 * Get the or set the type of an OID
 */
#define ONLP_OID_TYPE_GET(_id) ( ( (_id) >> 24) )
#define ONLP_OID_TYPE_CREATE(_type, _id) ( ( (_type) << 24) | (_id))
#define ONLP_OID_IS_TYPE(_type,_id) (ONLP_OID_TYPE_GET((_id)) == _type)
#define ONLP_OID_ID_GET(_id) (_id & 0xFFFFFF)
#define ONLP_THERMAL_ID_CREATE(_id) ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_THERMAL, _id)
#define ONLP_FAN_ID_CREATE(_id)     ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_FAN, _id)
#define ONLP_PSU_ID_CREATE(_id)     ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_PSU, _id)
#define ONLP_LED_ID_CREATE(_id)     ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_LED, _id)
#define ONLP_MODULE_ID_CREATE(_id)  ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_MODULE, _id)

#define ONLP_OID_IS_THERMAL(_id) ONLP_OID_IS_TYPE(ONLP_OID_TYPE_THERMAL,_id)
#define ONLP_OID_IS_FAN(_id)     ONLP_OID_IS_TYPE(ONLP_OID_TYPE_FAN,_id)
#define ONLP_OID_IS_PSU(_id)     ONLP_OID_IS_TYPE(ONLP_OID_TYPE_PSU,_id)
#define ONLP_OID_IS_LED(_id)     ONLP_OID_IS_TYPE(ONLP_OID_TYPE_LED,_id)
#define ONLP_OID_IS_MODULE(_id)  ONLP_OID_IS_TYPE(ONLP_OID_TYPE_MODULE,_id)


/**
 * There is only one SYS OID. This value should be used.
 */
#define ONLP_OID_SYS ONLP_OID_TYPE_CREATE(ONLP_OID_TYPE_SYS, 1)

/**
 * All OIDs have user-level description strings:
 */
#define ONLP_OID_DESC_SIZE 128

typedef char onlp_oid_desc_t[ONLP_OID_DESC_SIZE];

/* fixme */
#define ONLP_OID_TABLE_SIZE 128

typedef onlp_oid_t onlp_oid_table_t[ONLP_OID_TABLE_SIZE];
#define ONLP_OID_TABLE_SIZE_BYTES (sizeof(onlp_oid_t)*ONLP_OID_TABLE_SIZE)
#define ONLP_OID_TABLE_COPY(_dst, _src) memcpy(_dst, _src, ONLP_OID_TABLE_SIZE_BYTES)
#define ONLP_OID_TABLE_CLEAR(_table) memset(_table, 0, ONLP_OID_TABLE_SIZE_BYTES)


/**
 * This macro can be used to construct your OID hdr tables
 */
#define ONLP_OID_THERMAL_ENTRY(_id, _desc, _parent_type, _parent_id) \
    { ONLP_THERMAL_ID_CREATE(_id), _desc, ONLP_OID_TYPE_CREATE(_parent_type, _parent_id) }

/**
 * All OID objects contain this header as the first member.
 */
typedef struct onlp_oid_hdr_s {
    /** The OID */
    onlp_oid_t id;
    /** The description of this object. */
    onlp_oid_desc_t description;
    /** The parent OID of this object. */
    onlp_oid_t poid;
    /** The children of this OID */
    onlp_oid_table_t coids;
} onlp_oid_hdr_t;


void onlp_oid_dump(onlp_oid_t oid, aim_pvs_t* pvs, uint32_t flags);
void onlp_oid_table_dump(onlp_oid_table_t table, aim_pvs_t* pvs,
                         uint32_t flags);

void onlp_oid_show(onlp_oid_t oid, aim_pvs_t* pvs, uint32_t flags);
void onlp_oid_table_show(onlp_oid_table_t table, aim_pvs_t* pvs,
                         uint32_t flags);

/**
 * @brief Iterate over all OIDS in the given table that match the given expression.
 * @param _table The OID table
 * @param _oidp    OID pointer iterator
 * @param _expr    OID Expression which must be true
 */
#define ONLP_OID_TABLE_ITER_EXPR(_table, _oidp, _expr)                 \
    for(_oidp = _table; _oidp < (_table+ONLP_OID_TABLE_SIZE); _oidp++) \
        if( (*_oidp) && (_expr) )

/**
 * @brief Iterate over all OIDs in the given table.
 * @param _table The OID table
 * @param _oidp  OID pointer iterator
 */
#define ONLP_OID_TABLE_ITER(_table, _oidp) ONLP_OID_TABLE_ITER_EXPR(_table, _oidp, 1)

/**
 * @brief Iterate over all OIDs in the given table of the given type.
 * @param _table The OID table
 * @param _oidp  OID pointer iteration.
 * @param _type  The OID Type
 */

#define ONLP_OID_TABLE_ITER_TYPE(_table, _oidp, _type)                  \
    ONLP_OID_TABLE_ITER_EXPR(_table, _oidp, ONLP_OID_IS_TYPE(ONLP_OID_TYPE_##_type, *_oidp))


/**
 * Iterator
 */
typedef int (*onlp_oid_iterate_f)(onlp_oid_t oid, void* cookie);

/**
 * @brief Iterate over all platform OIDs.
 * @param oid The root OID.
 * @param type The OID type filter (optional)
 * @param itf The iterator function.
 * @param cookie The cookie.
 */
int onlp_oid_iterate(onlp_oid_t oid, onlp_oid_type_t type,
                     onlp_oid_iterate_f itf, void* cookie);

/**
 * @brief Get the OID header for a given OID.
 * @param oid The oid
 * @param hdr [out] Receives the header
 */
int onlp_oid_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr);






/******************************************************************************
 *
 * Enumeration Support Definitions.
 *
 * Please do not add additional code beyond this point.
 *
 *****************************************************************************/
/* <auto.start.enum(tag:oid).supportheader> */
/** Enum names. */
const char* onlp_oid_dump_name(onlp_oid_dump_t e);

/** Enum values. */
int onlp_oid_dump_value(const char* str, onlp_oid_dump_t* e, int substr);

/** Enum descriptions. */
const char* onlp_oid_dump_desc(onlp_oid_dump_t e);

/** Enum validator. */
int onlp_oid_dump_valid(onlp_oid_dump_t e);

/** validator */
#define ONLP_OID_DUMP_VALID(_e) \
    (onlp_oid_dump_valid((_e)))

/** onlp_oid_dump_map table. */
extern aim_map_si_t onlp_oid_dump_map[];
/** onlp_oid_dump_desc_map table. */
extern aim_map_si_t onlp_oid_dump_desc_map[];

/** Enum names. */
const char* onlp_oid_show_name(onlp_oid_show_t e);

/** Enum values. */
int onlp_oid_show_value(const char* str, onlp_oid_show_t* e, int substr);

/** Enum descriptions. */
const char* onlp_oid_show_desc(onlp_oid_show_t e);

/** Enum validator. */
int onlp_oid_show_valid(onlp_oid_show_t e);

/** validator */
#define ONLP_OID_SHOW_VALID(_e) \
    (onlp_oid_show_valid((_e)))

/** onlp_oid_show_map table. */
extern aim_map_si_t onlp_oid_show_map[];
/** onlp_oid_show_desc_map table. */
extern aim_map_si_t onlp_oid_show_desc_map[];

/** Enum names. */
const char* onlp_oid_type_name(onlp_oid_type_t e);

/** Enum values. */
int onlp_oid_type_value(const char* str, onlp_oid_type_t* e, int substr);

/** Enum descriptions. */
const char* onlp_oid_type_desc(onlp_oid_type_t e);

/** Enum validator. */
int onlp_oid_type_valid(onlp_oid_type_t e);

/** validator */
#define ONLP_OID_TYPE_VALID(_e) \
    (onlp_oid_type_valid((_e)))

/** onlp_oid_type_map table. */
extern aim_map_si_t onlp_oid_type_map[];
/** onlp_oid_type_desc_map table. */
extern aim_map_si_t onlp_oid_type_desc_map[];
/* <auto.end.enum(tag:oid).supportheader> */

#endif /* __ONLP_OID_H__ */
