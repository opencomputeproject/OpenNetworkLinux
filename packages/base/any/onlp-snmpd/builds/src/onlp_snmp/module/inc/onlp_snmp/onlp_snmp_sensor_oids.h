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
#ifndef __ONLP_SNMP_SENSOR_OIDS_H__
#define __ONLP_SNMP_SENSOR_OIDS_H__

/**
 * See:
 *      OCP-ONL-SENSOR-MIBS.txt
 */

#define ONLP_SNMP_SENSOR_OID          1,3,6,1,4,1,42623,1,2

#define ONLP_SNMP_SENSOR_ENTRY           1

/* Generic value */
#define ONLP_SNMP_SENSOR_COLUMN_INDEX    1
#define ONLP_SNMP_SENSOR_COLUMN_DEVNAME  2

/* This index counted from length of oid from right to left */
#define OID_SENSOR_TYPE_INDEX    4
#define OID_SENSOR_DEV_INDEX     1
#define OID_SENSOR_COL_INDEX     2

#define ONLP_SNMP_SENSOR_DEV_BASE_INDEX    1
#define ONLP_SNMP_SENSOR_DEV_MAX_INDEX     100


#define ONLP_SNMP_SENSOR_OID_SUFFIX   ONLP_SNMP_SENSOR_ENTRY,   \
        ONLP_SNMP_SENSOR_COLUMN_INDEX,                          \
        ONLP_SNMP_SENSOR_DEV_BASE_INDEX



#define ONLP_SNMP_SENSOR_OID_CREATE(_type)                              \
    ONLP_SNMP_SENSOR_OID,ONLP_SNMP_SENSOR_TYPE_##_type,ONLP_SNMP_SENSOR_OID_SUFFIX

#define ONLP_SNMP_SENSOR_TEMP_OID    ONLP_SNMP_SENSOR_OID_CREATE(TEMP)
#define ONLP_SNMP_SENSOR_FAN_OID     ONLP_SNMP_SENSOR_OID_CREATE(FAN)
#define ONLP_SNMP_SENSOR_PSU_OID     ONLP_SNMP_SENSOR_OID_CREATE(PSU)
#define ONLP_SNMP_SENSOR_LED_OID     ONLP_SNMP_SENSOR_OID_CREATE(LED)
#define ONLP_SNMP_SENSOR_MISC_OID    ONLP_SNMP_SENSOR_OID_CREATE(MISC)

/*
 * For legality check only, the sensor oid length from
 * ONLP-SENSOR-MIB file
 */
#define ONLP_SNMP_SENSOR_OID_LENGTH  13




/* <auto.start.enum(tag:mib).define> */
/** onlp_snmp_fan_flow_type */
typedef enum onlp_snmp_fan_flow_type_e {
    ONLP_SNMP_FAN_FLOW_TYPE_UNKNOWN = 0,
    ONLP_SNMP_FAN_FLOW_TYPE_B2F = 1,
    ONLP_SNMP_FAN_FLOW_TYPE_F2B = 2,
} onlp_snmp_fan_flow_type_t;

/** onlp_snmp_psu_type */
typedef enum onlp_snmp_psu_type_e {
    ONLP_SNMP_PSU_TYPE_UNKNOWN = 0,
    ONLP_SNMP_PSU_TYPE_AC = 1,
    ONLP_SNMP_PSU_TYPE_DC12 = 2,
    ONLP_SNMP_PSU_TYPE_DC48 = 3,
} onlp_snmp_psu_type_t;

/** onlp_snmp_sensor_status */
typedef enum onlp_snmp_sensor_status_e {
    ONLP_SNMP_SENSOR_STATUS_MISSING = 0,
    ONLP_SNMP_SENSOR_STATUS_GOOD = 1,
    ONLP_SNMP_SENSOR_STATUS_FAILED = 2,
    ONLP_SNMP_SENSOR_STATUS_UNPLUGGED = 3,
} onlp_snmp_sensor_status_t;

/** onlp_snmp_sensor_type */
typedef enum onlp_snmp_sensor_type_e {
    ONLP_SNMP_SENSOR_TYPE_TEMP = 1,
    ONLP_SNMP_SENSOR_TYPE_FAN = 2,
    ONLP_SNMP_SENSOR_TYPE_PSU = 3,
    ONLP_SNMP_SENSOR_TYPE_LED = 4,
    ONLP_SNMP_SENSOR_TYPE_MISC = 5,
    ONLP_SNMP_SENSOR_TYPE_MAX = 5,
} onlp_snmp_sensor_type_t;
/* <auto.end.enum(tag:mib).define> */









/* <auto.start.enum(tag:mib).supportheader> */
/** Enum names. */
const char* onlp_snmp_fan_flow_type_name(onlp_snmp_fan_flow_type_t e);

/** Enum values. */
int onlp_snmp_fan_flow_type_value(const char* str, onlp_snmp_fan_flow_type_t* e, int substr);

/** Enum descriptions. */
const char* onlp_snmp_fan_flow_type_desc(onlp_snmp_fan_flow_type_t e);

/** Enum validator. */
int onlp_snmp_fan_flow_type_valid(onlp_snmp_fan_flow_type_t e);

/** validator */
#define ONLP_SNMP_FAN_FLOW_TYPE_VALID(_e) \
    (onlp_snmp_fan_flow_type_valid((_e)))

/** onlp_snmp_fan_flow_type_map table. */
extern aim_map_si_t onlp_snmp_fan_flow_type_map[];
/** onlp_snmp_fan_flow_type_desc_map table. */
extern aim_map_si_t onlp_snmp_fan_flow_type_desc_map[];

/** Enum names. */
const char* onlp_snmp_psu_type_name(onlp_snmp_psu_type_t e);

/** Enum values. */
int onlp_snmp_psu_type_value(const char* str, onlp_snmp_psu_type_t* e, int substr);

/** Enum descriptions. */
const char* onlp_snmp_psu_type_desc(onlp_snmp_psu_type_t e);

/** Enum validator. */
int onlp_snmp_psu_type_valid(onlp_snmp_psu_type_t e);

/** validator */
#define ONLP_SNMP_PSU_TYPE_VALID(_e) \
    (onlp_snmp_psu_type_valid((_e)))

/** onlp_snmp_psu_type_map table. */
extern aim_map_si_t onlp_snmp_psu_type_map[];
/** onlp_snmp_psu_type_desc_map table. */
extern aim_map_si_t onlp_snmp_psu_type_desc_map[];

/** Enum names. */
const char* onlp_snmp_sensor_status_name(onlp_snmp_sensor_status_t e);

/** Enum values. */
int onlp_snmp_sensor_status_value(const char* str, onlp_snmp_sensor_status_t* e, int substr);

/** Enum descriptions. */
const char* onlp_snmp_sensor_status_desc(onlp_snmp_sensor_status_t e);

/** Enum validator. */
int onlp_snmp_sensor_status_valid(onlp_snmp_sensor_status_t e);

/** validator */
#define ONLP_SNMP_SENSOR_STATUS_VALID(_e) \
    (onlp_snmp_sensor_status_valid((_e)))

/** onlp_snmp_sensor_status_map table. */
extern aim_map_si_t onlp_snmp_sensor_status_map[];
/** onlp_snmp_sensor_status_desc_map table. */
extern aim_map_si_t onlp_snmp_sensor_status_desc_map[];

/** Enum names. */
const char* onlp_snmp_sensor_type_name(onlp_snmp_sensor_type_t e);

/** Enum values. */
int onlp_snmp_sensor_type_value(const char* str, onlp_snmp_sensor_type_t* e, int substr);

/** Enum descriptions. */
const char* onlp_snmp_sensor_type_desc(onlp_snmp_sensor_type_t e);

/** Enum validator. */
int onlp_snmp_sensor_type_valid(onlp_snmp_sensor_type_t e);

/** validator */
#define ONLP_SNMP_SENSOR_TYPE_VALID(_e) \
    (onlp_snmp_sensor_type_valid((_e)))

/** onlp_snmp_sensor_type_map table. */
extern aim_map_si_t onlp_snmp_sensor_type_map[];
/** onlp_snmp_sensor_type_desc_map table. */
extern aim_map_si_t onlp_snmp_sensor_type_desc_map[];
/* <auto.end.enum(tag:mib).supportheader> */


#endif /* __ONLP_SNMP_SENSOR_OIDS_H__ */

