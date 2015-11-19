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
#include <onlp_snmp/onlp_snmp_config.h>
#include <onlp_snmp/onlp_snmp_sensor_oids.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <OS/os_time.h>

#include <onlp/thermal.h>
#include <onlp/fan.h>
#include <onlp/psu.h>
#include <onlp/led.h>

#include "onlp_snmp_log.h"

/**
 * Individual Sensor Control structure.
 */
typedef struct onlp_snmp_sensor_s {
    int sensor_id; /* sensor identification */
    char name[ONLP_SNMP_CONFIG_MAX_NAME_LENGTH];
    char desc[ONLP_SNMP_CONFIG_MAX_DESC_LENGTH];
    int sensor_type;
    union sensor_info {
        onlp_thermal_info_t ti;
        onlp_fan_info_t     fi;
        onlp_psu_info_t     pi;
        onlp_led_info_t     li;
        uint32_t            mi; /* this is for misc value */
    } sensor_info;
    bool info_valid;             /* true if sensor_info is valid */
    uint64_t last_update_time;   /* last time called */
} onlp_snmp_sensor_t;


/**
 * NET SNMP Handler
 */
typedef void (*onlp_snmp_handler_fn)(netsnmp_request_info *req,
                                     uint32_t index,
                                     onlp_snmp_sensor_t *ss);

/* index into sensor_handler_fn array */
#define UPDATE_HANDLER_IDX 0

/*
 * Sensor Value Update period
 */
static uint32_t update_period__ = ONLP_SNMP_CONFIG_UPDATE_PERIOD;

#define SENSOR_NEEDS_UPDATE(_current_time, _sensor)                     \
    ((_current_time - _sensor->last_update_time) > update_period__ * 1000 * 1000)

#define SENSOR_SET_VALIDITY(_rv, _current_time, _sensor)        \
    do {                                                        \
        if (_rv < 0) {                                          \
            _sensor->info_valid = false;                        \
        } else {                                                \
            _sensor->info_valid = true;                          \
            _sensor->last_update_time = _current_time;           \
        }                                                        \
    } while(0)



typedef struct onlp_snmp_sensor_ctrl_s {
    char name[20];

    /* Handle sensor OIDs */
    uint32_t handler_cnt;
    onlp_snmp_handler_fn *handlers;

    uint32_t sensor_cnt;
    /*
     * Base index starts from 1, thus we add 1
     * Each sensor has a callback to get its value
     */
    onlp_snmp_sensor_t *sensor_list[ONLP_SNMP_CONFIG_DEV_MAX_INDEX+1];

} onlp_snmp_sensor_ctrl_t;

static onlp_snmp_sensor_ctrl_t sensor_ctrls__[ONLP_SNMP_SENSOR_TYPE_MAX+1];

static onlp_snmp_sensor_t*
get_sensor_reg__(onlp_snmp_sensor_ctrl_t *ss_type, int index)
{
    return ss_type->sensor_list[index];
}

static onlp_snmp_sensor_ctrl_t*
get_sensor_ctrl__(int sensor_type)
{
    return sensor_ctrls__ + sensor_type;
}




/**
 * Thermal Sensor Handlers
 */

static void
temp_update_handler__(netsnmp_request_info *req,
                      uint32_t index,
                      onlp_snmp_sensor_t *ss)
{
    uint64_t current = os_time_monotonic();

    if (SENSOR_NEEDS_UPDATE(current, ss)) {
        onlp_thermal_info_t *ti = &ss->sensor_info.ti;
        onlp_oid_t oid = (onlp_oid_t) ss->sensor_id;

        int rv = onlp_thermal_info_get(oid, ti);
        SENSOR_SET_VALIDITY(rv, current, ss);
    }

    /* else use the last update info */
}

static void
temp_index_handler__(netsnmp_request_info *req,
                     uint32_t index,
                     onlp_snmp_sensor_t *ss)
{
    snmp_set_var_typed_integer(req->requestvb,
                               ASN_INTEGER,
                               index);
}

static void
temp_devname_handler__(netsnmp_request_info *req,
                       uint32_t index,
                       onlp_snmp_sensor_t *ss)
{
    char device_name[ONLP_SNMP_CONFIG_MAX_NAME_LENGTH+ONLP_SNMP_CONFIG_MAX_DESC_LENGTH];

    snprintf(device_name,  sizeof(device_name),
             "%s %s%s", "Thermal", ss->name, ss->desc);

    snmp_set_var_typed_value(req->requestvb,
                             ASN_OCTET_STR,
                             (u_char *) device_name,
                             strlen(device_name));
}

static void
temp_status_handler__(netsnmp_request_info *req,
                      uint32_t index,
                      onlp_snmp_sensor_t *ss)
{
    int value;
    onlp_thermal_info_t *ti = &ss->sensor_info.ti;

    if (!ss->info_valid) {
        return;
    }

    value = ONLP_SNMP_SENSOR_STATUS_MISSING;
    if (ti->status & ONLP_THERMAL_STATUS_PRESENT) {
        value = ONLP_SNMP_SENSOR_STATUS_GOOD;
        if (ti->status & ONLP_THERMAL_STATUS_FAILED) {
            value = ONLP_SNMP_SENSOR_STATUS_FAILED;
        }
    }

    snmp_set_var_typed_integer(req->requestvb,
                               ASN_INTEGER,
                               value);
}

static void
temp_value_handler__(netsnmp_request_info *req,
                     uint32_t index,
                     onlp_snmp_sensor_t *ss)
{
    int value;
    onlp_thermal_info_t *ti = &ss->sensor_info.ti;

    if (!ss->info_valid) {
        return;
    }

    if (!(ti->status & ONLP_THERMAL_STATUS_PRESENT)) {
        return;
    }

    value = ti->mcelsius;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}

static onlp_snmp_handler_fn temp_handler_fn__[] = {
    temp_update_handler__,
    temp_index_handler__,
    temp_devname_handler__,
    temp_status_handler__,
    temp_value_handler__,
};






/**
 * Fan Sensor Handlers
 */
static void
fan_update_handler__(netsnmp_request_info *req,
                     uint32_t index,
                     onlp_snmp_sensor_t *ss)
{
    uint64_t current = os_time_monotonic();

    if (SENSOR_NEEDS_UPDATE(current, ss)) {
        onlp_fan_info_t *fi = &ss->sensor_info.fi;
        onlp_oid_t oid = (onlp_oid_t) ss->sensor_id;

        int rv = onlp_fan_info_get(oid, fi);
        SENSOR_SET_VALIDITY(rv, current, ss);
    }

    /* else use the last update info */
}

static void
fan_index_handler__(netsnmp_request_info *req,
                    uint32_t index,
                    onlp_snmp_sensor_t *ss)
{
    snmp_set_var_typed_integer(req->requestvb,
                               ASN_INTEGER,
                               index);
}

static void
fan_devname_handler__(netsnmp_request_info *req,
                      uint32_t index,
                      onlp_snmp_sensor_t *ss)
{
    char device_name[ONLP_SNMP_CONFIG_MAX_NAME_LENGTH+ONLP_SNMP_CONFIG_MAX_DESC_LENGTH];
    snprintf(device_name,  sizeof(device_name),
             "%s %s%s", "Fan", ss->name, ss->desc);

    snmp_set_var_typed_value(req->requestvb,
                             ASN_OCTET_STR,
                             (u_char *) device_name,
                             strlen(device_name));
}


static void
fan_status_handler__(netsnmp_request_info *req,
                     uint32_t index,
                     onlp_snmp_sensor_t *ss)
{
    int value;
    onlp_fan_info_t *fi = &ss->sensor_info.fi;

    if (!ss->info_valid) {
        return;
    }

    value = ONLP_SNMP_SENSOR_STATUS_MISSING;
    if (fi->status & ONLP_FAN_STATUS_PRESENT) {
        value = ONLP_SNMP_SENSOR_STATUS_GOOD;
        if (fi->status & ONLP_FAN_STATUS_FAILED) {
            value = ONLP_SNMP_SENSOR_STATUS_FAILED;
        }
    }

    snmp_set_var_typed_integer(req->requestvb,
                               ASN_INTEGER,
                               value);
}

static void
fan_flow_type_handler__(netsnmp_request_info *req,
                        uint32_t index,
                        onlp_snmp_sensor_t *ss)
{
    int name_index;
    onlp_fan_info_t *fi = &ss->sensor_info.fi;

    if (!ss->info_valid) {
        return;
    }

    name_index = ONLP_SNMP_FAN_FLOW_TYPE_UNKNOWN;

    if (fi->status & ONLP_FAN_STATUS_PRESENT) {
        if (fi->status & ONLP_FAN_STATUS_B2F) {
            name_index = ONLP_SNMP_FAN_FLOW_TYPE_B2F;
        } else if (fi->status & ONLP_FAN_STATUS_F2B) {
            name_index = ONLP_SNMP_FAN_FLOW_TYPE_F2B;
        } else {
            /* Unknown */
        }
    }

    const char* s = onlp_snmp_fan_flow_type_name(name_index);
    snmp_set_var_typed_value(req->requestvb,
                             ASN_OCTET_STR,
                             (u_char*)s, strlen(s));
}

static void
fan_rpm_handler__(netsnmp_request_info *req,
                  uint32_t index,
                  onlp_snmp_sensor_t *ss)
{
    int value;
    onlp_fan_info_t *fi = &ss->sensor_info.fi;

    if (!ss->info_valid) {
        return;
    }

    if (!(fi->status & ONLP_FAN_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present*/
        return;
    }

    value = fi->rpm;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}

static void
fan_pct_handler__(netsnmp_request_info *req,
                  uint32_t index,
                  onlp_snmp_sensor_t *ss)
{
    int value;
    onlp_fan_info_t *fi = &ss->sensor_info.fi;

    if (!ss->info_valid) {
        return;
    }

    if (!(fi->status & ONLP_FAN_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present*/
        return;
    }
    value = fi->percentage;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}

static void
fan_model_handler__(netsnmp_request_info *req,
                    uint32_t index,
                    onlp_snmp_sensor_t *ss)
{
    onlp_fan_info_t *fi = &ss->sensor_info.fi;

    if (!ss->info_valid) {
        return;
    }

    if (!(fi->status & ONLP_FAN_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present*/
        return;
    }

    int len = strlen(fi->model);
    if (len == 0) {
        return;
    }

    snmp_set_var_typed_value(req->requestvb,
                             ASN_OCTET_STR,
                             (u_char *) fi->model,
                             len);
}

static void
fan_serial_handler__(netsnmp_request_info *req,
                     uint32_t index,
                     onlp_snmp_sensor_t *ss)
{
    onlp_fan_info_t *fi = &ss->sensor_info.fi;

    if (!ss->info_valid) {
        return;
    }

    if (!(fi->status & ONLP_FAN_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present*/
        return;
    }

    int len = strlen(fi->serial);
    if (len == 0) {
        return;
    }

    snmp_set_var_typed_value(req->requestvb,
                             ASN_OCTET_STR,
                             (u_char *) fi->serial,
                             len);
}

static onlp_snmp_handler_fn fan_handler_fn__[] = {
    fan_update_handler__,
    fan_index_handler__,
    fan_devname_handler__,
    fan_status_handler__,
    fan_flow_type_handler__,
    fan_rpm_handler__,
    fan_pct_handler__,
    fan_model_handler__,
    fan_serial_handler__
};



/**
 * PSU Handlers
 */
static void
psu_update_handler__(netsnmp_request_info *req,
                     uint32_t index,
                     onlp_snmp_sensor_t *ss)
{
    uint64_t current = os_time_monotonic();

    if (SENSOR_NEEDS_UPDATE(current, ss)) {
        onlp_psu_info_t *pi = &ss->sensor_info.pi;
        onlp_oid_t oid = (onlp_oid_t) ss->sensor_id;

        int rv = onlp_psu_info_get(oid, pi);
        SENSOR_SET_VALIDITY(rv, current, ss);
    }

    /* else use the last update info */
}

static void
psu_index_handler__(netsnmp_request_info *req,
                    uint32_t index,
                    onlp_snmp_sensor_t *ss)
{
    snmp_set_var_typed_integer(req->requestvb,
                               ASN_INTEGER,
                               index);
}

static void
psu_devname_handler__(netsnmp_request_info *req,
                      uint32_t index,
                      onlp_snmp_sensor_t *ss)
{
    char device_name[ONLP_SNMP_CONFIG_MAX_NAME_LENGTH+ONLP_SNMP_CONFIG_MAX_DESC_LENGTH];
    snprintf(device_name,  sizeof(device_name),
             "%s %s%s", "PSU", ss->name, ss->desc);

    snmp_set_var_typed_value(req->requestvb,
                             ASN_OCTET_STR,
                             (u_char *) device_name,
                             strlen(device_name));
}

static void
psu_status_handler__(netsnmp_request_info *req,
                     uint32_t index,
                     onlp_snmp_sensor_t *ss )
{
    int value;
    onlp_psu_info_t *pi = &ss->sensor_info.pi;

    if (!ss->info_valid) {
        return;
    }

    value = ONLP_SNMP_SENSOR_STATUS_MISSING;
    if (pi->status & ONLP_PSU_STATUS_PRESENT) {
        value = ONLP_SNMP_SENSOR_STATUS_GOOD;

        /* failed or good is always reported */
        if (pi->status & ONLP_PSU_STATUS_FAILED) {
            value = ONLP_SNMP_SENSOR_STATUS_FAILED;
        }

        /* if additional unplugged status is reported */
        if (pi->status & ONLP_PSU_STATUS_UNPLUGGED) {
            value = ONLP_SNMP_SENSOR_STATUS_FAILED;
        }

    }

    snmp_set_var_typed_integer(req->requestvb,
                               ASN_INTEGER,
                               value);
}

static void
psu_current_type_handler__(netsnmp_request_info *req,
                           uint32_t index,
                           onlp_snmp_sensor_t *ss )
{
    int name_index;
    onlp_psu_info_t *pi = &ss->sensor_info.pi;

    if (!ss->info_valid) {
        return;
    }

    if (!(pi->status & ONLP_PSU_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present*/
        return;
    }

    name_index = ONLP_SNMP_PSU_TYPE_UNKNOWN;
    /* These values are mutual exclusive */
    if (pi->caps & ONLP_PSU_CAPS_AC) {
        name_index = ONLP_SNMP_PSU_TYPE_AC;
    } else if (pi->caps & ONLP_PSU_CAPS_DC12) {
        name_index = ONLP_SNMP_PSU_TYPE_DC12;
    } else if (pi->caps & ONLP_PSU_CAPS_DC48) {
        name_index = ONLP_SNMP_PSU_TYPE_DC48;
    } else {
        /* Unknown type */
    }

    const char* s = onlp_snmp_psu_type_name(name_index);
    snmp_set_var_typed_value(req->requestvb,
                             ASN_OCTET_STR,
                             (u_char *) s, strlen(s));
}

static void
psu_model_handler__(netsnmp_request_info *req,
                    uint32_t index,
                    onlp_snmp_sensor_t *ss)
{

    onlp_psu_info_t *pi = &ss->sensor_info.pi;

    if (!ss->info_valid) {
        return;
    }

    if (!(pi->status & ONLP_PSU_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present*/
        return;
    }

    int len = strlen(pi->model);
    if (len == 0) {
        return;
    }

    snmp_set_var_typed_value(req->requestvb,
                             ASN_OCTET_STR,
                             (u_char *) pi->model,
                             len);
}

static void
psu_serial_handler__(netsnmp_request_info *req,
                     uint32_t index,
                     onlp_snmp_sensor_t *ss)
{

    onlp_psu_info_t *pi = &ss->sensor_info.pi;

    if (!ss->info_valid) {
        return;
    }

    if (!(pi->status & ONLP_PSU_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present*/
        return;
    }

    int len = strlen(pi->serial);
    if (len == 0) {
        return;
    }

    snmp_set_var_typed_value(req->requestvb,
                             ASN_OCTET_STR,
                             (u_char *) pi->serial,
                             len);
}

static void
psu_vin_handler__(netsnmp_request_info *req,
                  uint32_t index,
                  onlp_snmp_sensor_t *ss )
{
    int value;
    onlp_psu_info_t *pi = &ss->sensor_info.pi;

    if (!ss->info_valid) {
        return;
    }

    if (!(pi->status & ONLP_PSU_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present*/
        return;
    }

    value = pi->mvin;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}

static void
psu_vout_handler__(netsnmp_request_info *req,
                   uint32_t index,
                   onlp_snmp_sensor_t *ss )
{
    int value;
    onlp_psu_info_t *pi = &ss->sensor_info.pi;

    if (!ss->info_valid) {
        return;
    }

    if (!(pi->status & ONLP_PSU_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present */
        return;
    }
    value = pi->mvout;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}

static void
psu_iin_handler__(netsnmp_request_info *req,
                  uint32_t index,
                  onlp_snmp_sensor_t *ss )
{
    int value;
    onlp_psu_info_t *pi = &ss->sensor_info.pi;

    if (!ss->info_valid) {
        return;
    }

    if (!(pi->status & ONLP_PSU_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present*/
        return;
    }
    value = pi->miin;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}

static void
psu_iout_handler__(netsnmp_request_info *req,
                   uint32_t index,
                   onlp_snmp_sensor_t *ss )
{
    int value;
    onlp_psu_info_t *pi = &ss->sensor_info.pi;

    if (!ss->info_valid) {
        return;
    }

    if (!(pi->status & ONLP_PSU_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present*/
        return;
    }
    value = pi->miout;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}

static void
psu_pin_handler__(netsnmp_request_info *req,
                  uint32_t index,
                  onlp_snmp_sensor_t *ss )
{
    int value;
    onlp_psu_info_t *pi = &ss->sensor_info.pi;

    if (!ss->info_valid) {
        return;
    }

    if (!(pi->status & ONLP_PSU_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present*/
        return;
    }
    value = pi->mpin;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}

static void
psu_pout_handler__(netsnmp_request_info *req,
                   uint32_t index,
                   onlp_snmp_sensor_t *ss )
{
    int value;
    onlp_psu_info_t *pi = &ss->sensor_info.pi;

    if (!ss->info_valid) {
        return;
    }

    if (!(pi->status & ONLP_PSU_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present*/
        return;
    }
    value = pi->mpout;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}


static onlp_snmp_handler_fn psu_handler_fn__[] = {
    psu_update_handler__,
    psu_index_handler__,
    psu_devname_handler__,
    psu_status_handler__,
    psu_current_type_handler__,
    psu_model_handler__,
    psu_vin_handler__,
    psu_vout_handler__,
    psu_iin_handler__,
    psu_iout_handler__,
    psu_pin_handler__,
    psu_pout_handler__,
    psu_serial_handler__
};


/**
 * LED Handlers
 */
static void
led_update_handler__(netsnmp_request_info *req,
                     uint32_t index,
                     onlp_snmp_sensor_t *ss)
{
    uint64_t current = os_time_monotonic();

    if (SENSOR_NEEDS_UPDATE(current, ss)) {
        onlp_led_info_t *li = &ss->sensor_info.li;
        onlp_oid_t oid = (onlp_oid_t) ss->sensor_id;

        int rv = onlp_led_info_get(oid, li);
        SENSOR_SET_VALIDITY(rv, current, ss);
    }

    /* else use the last update info */
}

static void
led_index_handler__(netsnmp_request_info *req,
                    uint32_t index,
                    onlp_snmp_sensor_t *ss)
{
    snmp_set_var_typed_integer(req->requestvb,
                               ASN_INTEGER,
                               index);
}

static void
led_devname_handler__(netsnmp_request_info *req,
                      uint32_t index,
                      onlp_snmp_sensor_t *ss)
{
    char device_name[ONLP_SNMP_CONFIG_MAX_NAME_LENGTH+ONLP_SNMP_CONFIG_MAX_DESC_LENGTH];
    snprintf(device_name,  sizeof(device_name),
             "%s %s%s", "Led", ss->name, ss->desc);

    snmp_set_var_typed_value(req->requestvb,
                             ASN_OCTET_STR,
                             (u_char *) device_name,
                             strlen(device_name));
}

static void
led_status_handler__(netsnmp_request_info *req,
                     uint32_t index,
                     onlp_snmp_sensor_t *ss)
{
    int value;
    onlp_led_info_t *li = &ss->sensor_info.li;

    if (!ss->info_valid) {
        return;
    }

    value = li->status;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}

static void
led_value_handler__(netsnmp_request_info *req,
                    uint32_t index,
                    onlp_snmp_sensor_t *ss)
{
    int value;
    onlp_led_info_t *li = &ss->sensor_info.li;

    if (!ss->info_valid) {
        return;
    }

    if (!(li->status & ONLP_LED_STATUS_PRESENT)) {
        /* Simply return if failed to get or not present*/
        return;
    }
    value = li->mode;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}

static onlp_snmp_handler_fn led_handler_fn__[] = {
    led_update_handler__,
    led_index_handler__,
    led_devname_handler__,
    led_status_handler__,
    led_value_handler__,
};


/**
 * Misc Handlers.
 * Placeholder for unknown types
 */
static void
misc_update_handler__(netsnmp_request_info *req,
                      uint32_t index,
                      onlp_snmp_sensor_t *ss)
{
    ss->info_valid = 1;
}

static void
misc_index_handler__(netsnmp_request_info *req,
                     uint32_t index,
                     onlp_snmp_sensor_t *ss)
{
    snmp_set_var_typed_integer(req->requestvb,
                               ASN_INTEGER,
                               index);
}

static void
misc_devname_handler__(netsnmp_request_info *req,
                       uint32_t index,
                       onlp_snmp_sensor_t *ss)
{
    char device_name[ONLP_SNMP_CONFIG_MAX_NAME_LENGTH+ONLP_SNMP_CONFIG_MAX_DESC_LENGTH];
    snprintf(device_name,  sizeof(device_name),
             "%s %s%s", "Misc", ss->name, ss->desc);

    snmp_set_var_typed_value(req->requestvb,
                             ASN_OCTET_STR,
                             (u_char *) device_name,
                             strlen(device_name));
}

static void
misc_status_handler__(netsnmp_request_info *req,
                      uint32_t index,
                      onlp_snmp_sensor_t *ss)
{
    int value = 0;
    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}

static void
misc_value_handler__(netsnmp_request_info *req,
                     uint32_t index,
                     onlp_snmp_sensor_t *ss)
{
    int value = 0;
    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}

static onlp_snmp_handler_fn misc_handler_fn__[] = {
    misc_update_handler__,
    misc_index_handler__,
    misc_devname_handler__,
    misc_status_handler__,
    misc_value_handler__,
};


/*
 * OID HANDLER for all sensor types
 * This is registered to NETSNMP using agentX
 */
static int
onlp_snmp_sensor_handler__(netsnmp_mib_handler *handler,
                           netsnmp_handler_registration *reg,
                           netsnmp_agent_request_info *agent_req,
                           netsnmp_request_info *req)
{

    int ret = SNMP_ERR_NOERROR;
    onlp_snmp_sensor_t *sensor;

    int sensor_type = req->requestvb->name[req->requestvb->name_length - OID_SENSOR_TYPE_INDEX];
    onlp_snmp_sensor_ctrl_t *ss_type = get_sensor_ctrl__(sensor_type);

    /* This is for index / device / value handler */
    int column = req->requestvb->name[req->requestvb->name_length - OID_SENSOR_COL_INDEX];

    /* This is device index */
    int index = req->requestvb->name[req->requestvb->name_length - OID_SENSOR_DEV_INDEX];

    if (agent_req->mode != MODE_GET && agent_req->mode != MODE_GETNEXT) {
        /* If happen, just return */
        return ret;
    }

    if(!onlp_snmp_sensor_type_valid(sensor_type)) {
        /* If happen, just return */
        return ret;
    }

    if (column >= ss_type->handler_cnt) {
        /* If happen, just return */
        return ret;
    }

    /* index start from 1 and equal to sensor_cnt */
    if (index < ONLP_SNMP_CONFIG_DEV_BASE_INDEX || index > ss_type->sensor_cnt) {
        /* If happen, just return */
        return ret;
    }

    sensor = get_sensor_reg__(ss_type, index);
    if (!sensor) {
        /* If happen, just return */
        return ret;
    }

    if (!ss_type->handlers[UPDATE_HANDLER_IDX]) {
        snmp_log(LOG_ALERT,
                 "No update handler for type=%d column=%d, index=%d",
                 sensor_type, column, index);
        return ret;
    }
    ss_type->handlers[UPDATE_HANDLER_IDX](req, index, sensor);

    /* We select index/devname/value to handle for each device */
    if (ss_type->handlers[column]) {
        (*ss_type->handlers[column])(req, index, sensor);
    }

    return ret;
}




/* Register OID handler for a sensor type  */
static int
reg_snmp_sensor_helper__(int sensor_type,
                         oid *reg_oid,
                         size_t oid_len,
                         int dev_idx)
{
    int ret = MIB_REGISTRATION_FAILED;

    Netsnmp_Node_Handler *handler = onlp_snmp_sensor_handler__;
    onlp_snmp_sensor_ctrl_t *ss_type = get_sensor_ctrl__(sensor_type);
    uint32_t col_cnt = ss_type->handler_cnt;
    char *table_name = ss_type->name;

    /* Use this to increase the column index in oid */
    u_long *oid_col = &reg_oid[oid_len - OID_SENSOR_COL_INDEX];

    /* Use this to increase the dev index of oid */
    reg_oid[oid_len - OID_SENSOR_DEV_INDEX] = dev_idx;

    snmp_log(LOG_DEBUG, "oid registrations: %s for dev_idx=%d",
             table_name, dev_idx);

    /*
     * Caller makes sure that this loop is run
     * since *oid_col starts as 1st col
     */
    for (; *oid_col < col_cnt; (*oid_col)++) {
        netsnmp_handler_registration *reg;

        if (!ss_type->handlers[*oid_col])
            continue;

        reg = netsnmp_create_handler_registration(table_name,
                                                  handler,
                                                  reg_oid,
                                                  oid_len,
                                                  HANDLER_CAN_RONLY);

        /* Ofad enables verbose/trace to see this */
        snmp_log(LOG_INFO, "registering handler for %s column %ld, index %d",
                 table_name, *oid_col, dev_idx);

        /* If reg is null, this returns error */
        ret = netsnmp_register_instance(reg);
        if(ret) {
            break;
        }
    }

    return ret;

}

/*
 * Register a sensor
 * Caller must make sure that 1 sensor registered only once
 * If it calls this twice, it will get 2 oid entries
 * for the same sensor
 *
 * We want to keep this snmp code as simple as possible
 */
static int
onlp_snmp_sensor_reg__(int sensor_type,
                       onlp_snmp_sensor_t *sensor)
{
    oid otemp[] = { ONLP_SNMP_SENSOR_TEMP_OID };
    oid ofan[]  = { ONLP_SNMP_SENSOR_FAN_OID };
    oid opsu[]  = { ONLP_SNMP_SENSOR_PSU_OID };
    oid oled[]  = { ONLP_SNMP_SENSOR_LED_OID };
    oid omisc[] = { ONLP_SNMP_SENSOR_MISC_OID };
    oid *o;
    u_long o_len;
    int ret = MIB_REGISTRATION_FAILED;

    onlp_snmp_sensor_ctrl_t *ss_type = get_sensor_ctrl__(sensor_type);

    /* We start with Base 1 */
    AIM_TRUE_OR_DIE(onlp_snmp_sensor_type_valid(sensor_type));
    AIM_TRUE_OR_DIE(sensor);
    AIM_TRUE_OR_DIE(ss_type);

    switch(sensor_type)
        {
        case ONLP_SNMP_SENSOR_TYPE_TEMP:
            o = otemp;
            o_len = OID_LENGTH(otemp);

            /* Not init yet, init oid table */
            if (!ss_type->handlers) {
                ss_type->handler_cnt = sizeof(temp_handler_fn__) / sizeof(temp_handler_fn__[0]);
                ss_type->handlers    = temp_handler_fn__;
                snprintf(ss_type->name, sizeof(ss_type->name), "%s", "temp_table");
            }
            break;

        case ONLP_SNMP_SENSOR_TYPE_FAN:
            o = ofan;
            o_len = OID_LENGTH(ofan);

            /* Not init yet, init oid table */
            if (!ss_type->handlers) {
                ss_type->handler_cnt = sizeof(fan_handler_fn__) / sizeof(fan_handler_fn__[0]);
                ss_type->handlers    = fan_handler_fn__;
                snprintf(ss_type->name, sizeof(ss_type->name), "%s", "fan_table");
            }
            break;

        case ONLP_SNMP_SENSOR_TYPE_PSU:
            o = opsu;
            o_len = OID_LENGTH(opsu);

            /* Not init yet, init oid table */
            if (!ss_type->handlers) {
                ss_type->handler_cnt = sizeof(psu_handler_fn__) / sizeof(psu_handler_fn__[0]);
                ss_type->handlers    = psu_handler_fn__;
                snprintf(ss_type->name, sizeof(ss_type->name), "%s", "psu_table");

            }
            break;

        case ONLP_SNMP_SENSOR_TYPE_LED:
            o = oled;
            o_len = OID_LENGTH(oled);

            /* Not init yet, init oid table */
            if (!ss_type->handlers) {
                ss_type->handler_cnt = sizeof(led_handler_fn__) / sizeof(led_handler_fn__[0]);
                ss_type->handlers    = led_handler_fn__;
                snprintf(ss_type->name, sizeof(ss_type->name), "%s", "led_table");
            }
            break;

        case ONLP_SNMP_SENSOR_TYPE_MISC:
            o = omisc;
            o_len = OID_LENGTH(omisc);

            /* Not init yet, init oid table */
            if (!ss_type->handlers) {
                ss_type->handler_cnt = sizeof(misc_handler_fn__) / sizeof(misc_handler_fn__[0]);
                ss_type->handlers    = misc_handler_fn__;
                snprintf(ss_type->name, sizeof(ss_type->name), "%s", "misc_table");
            }
            break;

        default:
            AIM_DIE("Invalid sensor value.");
            break;
    }

    /*
     * sensor_cnt original is 0
     * When sensor_cnt == ONLP_SNMP_CONFIG_DEV_MAX_INDEX
     * We stop adding
     */
    if (ss_type->sensor_cnt < ONLP_SNMP_CONFIG_DEV_MAX_INDEX) {
        /* Device index equal to ss_type->sensor_cnt */
        ss_type->sensor_cnt++;

        /* This entry must be null */
        AIM_TRUE_OR_DIE(!ss_type->sensor_list[ss_type->sensor_cnt]);

        snmp_log(LOG_INFO, "init type=%d, index=%d, id=%d",
                 sensor_type, ss_type->sensor_cnt, sensor->sensor_id);

        onlp_snmp_sensor_t *ss = AIM_MALLOC(sizeof(onlp_snmp_sensor_t));
        AIM_TRUE_OR_DIE(ss);
        AIM_MEMCPY(ss, sensor, sizeof(*sensor));
        ss->sensor_type = sensor_type;
        ss->info_valid = 0;
        ss->last_update_time = 0;

        /* Assign sensor to the list */
        ss_type->sensor_list[ss_type->sensor_cnt] = ss;

    } else {
        snmp_log(LOG_ALERT,
                 "Failed to register sensor type=%d id=%d, resource limited",
                 sensor_type, sensor->sensor_id);
        return ret;
    }

    AIM_TRUE_OR_DIE(o_len == ONLP_SNMP_SENSOR_OID_LENGTH,
                    "invalid oid length=%d", o_len);

    ret = reg_snmp_sensor_helper__(sensor_type, o, o_len,
                                   ss_type->sensor_cnt);
    if (ret) {
        snmp_log(LOG_ALERT,
                 "Failed to register sensor type=%d id=%d, MIB_ERROR=%d",
                 sensor_type, sensor->sensor_id, ret);
    }

    return ret;
}


static int
onlp_snmp_sensor_register_oid__(onlp_oid_t oid, void* cookie)
{
    onlp_oid_hdr_t hdr;
    onlp_snmp_sensor_t s;

    onlp_oid_hdr_get(oid, &hdr);

    AIM_MEMSET(&s, 0x0, sizeof(onlp_snmp_sensor_t));
    switch(ONLP_OID_TYPE_GET(oid))
        {
        case ONLP_OID_TYPE_THERMAL:
#if ONLP_SNMP_CONFIG_INCLUDE_THERMALS == 1
            s.sensor_id = oid;
            sprintf(s.name, "%d - ", ONLP_OID_ID_GET(oid));
            aim_strlcpy(s.desc, hdr.description, sizeof(s.desc));
            if(onlp_snmp_sensor_reg__(ONLP_SNMP_SENSOR_TYPE_TEMP, &s) < 0) {
                AIM_LOG_ERROR("onlp_snmp_sensor_reg for OID 0x%x failed.", oid);
            }
#endif
            break;

        case ONLP_OID_TYPE_FAN:
#if ONLP_SNMP_CONFIG_INCLUDE_FANS == 1
            s.sensor_id = oid;
            sprintf(s.name, "%d - ", ONLP_OID_ID_GET(oid));
            aim_strlcpy(s.desc, hdr.description, sizeof(s.desc));
            if(onlp_snmp_sensor_reg__(ONLP_SNMP_SENSOR_TYPE_FAN, &s) < 0) {
                AIM_LOG_ERROR("onlp_snmp_sensor_reg for OID 0x%x failed.", oid);
            }
#endif
            break;

        case ONLP_OID_TYPE_PSU:
#if ONLP_SNMP_CONFIG_INCLUDE_PSUS == 1
            /* Register Sensors for VIN,VOUT,IIN,IOUT,PIN,POUT */
            s.sensor_id = oid;
            sprintf(s.name, "%d - ", ONLP_OID_ID_GET(oid));
            aim_strlcpy(s.desc, hdr.description, sizeof(s.desc));
            if(onlp_snmp_sensor_reg__(ONLP_SNMP_SENSOR_TYPE_PSU, &s) < 0) {
                AIM_LOG_ERROR("onlp_snmp_sensor_reg for OID 0x%x failed.", oid);
            }
#endif
            break;

        default:
            AIM_LOG_INFO("snmp type %s id %d unsupported",
                         onlp_oid_type_name(ONLP_OID_TYPE_GET(oid)),
                         ONLP_OID_ID_GET(oid));
            break;
    }

    return 0;
}


/**
 * Register Sensors
 */
void onlp_snmp_sensors_init(void)
{
    int rv;
    AIM_LOG_MSG("%s", __FUNCTION__);

    /* Register all sensor OIDs */
    rv = onlp_oid_iterate(ONLP_OID_SYS, 0, onlp_snmp_sensor_register_oid__, NULL);
    if (rv != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("%s error %d", __FUNCTION__, rv);
    } else {
        AIM_LOG_MSG("%s succeeded.", __FUNCTION__);
    }
}

int
onlp_snmp_sensors_client(int enable, void* cookie)
{
    onlp_snmp_sensors_init();
    return 0;
}
