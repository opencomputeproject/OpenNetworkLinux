/************************************************************
 * <bsn.cl fy=2015 v=onl>
 *
 *           Copyright 2015-2017 Big Switch Networks, Inc.
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

#include <AIM/aim_sem.h>
#include <AIM/aim_time.h>

#include <pthread.h>
#include <unistd.h>

#include <onlp/thermal.h>
#include <onlp/fan.h>
#include <onlp/psu.h>

#include "onlp_snmp_log.h"


typedef struct sensor_info_s {
    bool valid;  /* for snmp table maintenance */
    union {
        onlp_thermal_info_t ti;
        onlp_fan_info_t     fi;
        onlp_psu_info_t     pi;
    } data;
} sensor_info_t;

/* for front-back buffers */
#define NUM_SENSOR_INFO (2)

/**
 * Individual Sensor Control structure.
 * The valid field in the sensor_info_t structure above
 * is used snmp table maintenance:
 * - A table row is added when previously invalid sensor is now valid.
 * - A table row is deleted when previously valid sensor is now invalid
 */
typedef struct onlp_snmp_sensor_s {
    list_links_t links;  /* for tracking sensors of the same type */
    int sensor_id;       /* onlp_oid_t for this sensor */
    char name[ONLP_SNMP_CONFIG_MAX_NAME_LENGTH];
    char desc[ONLP_SNMP_CONFIG_MAX_DESC_LENGTH];
    onlp_snmp_sensor_type_t sensor_type;
    uint32_t index;      /* snmp table column */
    sensor_info_t sensor_info[NUM_SENSOR_INFO];
} onlp_snmp_sensor_t;

static int curr_info;
static int
next_info(void)
{
    return (curr_info+1) % NUM_SENSOR_INFO;
}
static sensor_info_t *
get_curr_info(onlp_snmp_sensor_t *ss)
{
    return &ss->sensor_info[curr_info];
}
static sensor_info_t *
get_next_info(onlp_snmp_sensor_t *ss)
{
    return &ss->sensor_info[next_info()];
}
static void
swap_curr_next_info(void)
{
    curr_info = next_info();
}

/* timestamp used to trigger sensor update */
static uint64_t last_sensor_update_time;

/* true if table restructuring is to happen;
 * set after all tables updated;
 * cleared after all tables restructured */
static bool restructure_trigger;

/* updates happen in this pthread */
static pthread_t update_thread_handle;

/**
 * NET SNMP handler
 */
typedef void (*onlp_snmp_handler_fn)(netsnmp_request_info *req,
                                     uint32_t index,
                                     onlp_snmp_sensor_t *ss);
/**
 * Update handler
 */
typedef int (*update_handler_fn)(onlp_snmp_sensor_t *ss);


/*
 * Sensor control block, one for each sensor type
 */
typedef struct onlp_snmp_sensor_ctrl_s {
    char name[20];
    list_head_t sensors;
} onlp_snmp_sensor_ctrl_t;

static onlp_snmp_sensor_ctrl_t sensor_ctrls__[ONLP_SNMP_SENSOR_TYPE_MAX+1];


static onlp_snmp_sensor_ctrl_t*
get_sensor_ctrl__(int sensor_type)
{
    return &sensor_ctrls__[sensor_type];
}


/* for accessing netsnmp table info */
static netsnmp_tdata *sensor_table__[ONLP_SNMP_SENSOR_TYPE_MAX+1];


static void *
delete_table_row__(netsnmp_tdata *table, uint32_t index)
{
    /* the search oid is the index */
    oid o[] = { index };
    netsnmp_tdata_row *row = netsnmp_tdata_row_get_byoid(table,
                                                         o, OID_LENGTH(o));
    void *data = netsnmp_tdata_remove_and_delete_row(table, row);
    return data;
}

/* returns 0 if row is successfully populated, -1 if not */
static int
add_table_row__(netsnmp_tdata *table, onlp_snmp_sensor_t *ss)
{
    netsnmp_tdata_row *row = netsnmp_tdata_create_row();
    netsnmp_variable_list *varlist;
    int rv;

    if (row == NULL) {
        AIM_LOG_ERROR("failed to allocate table row");
        return -1;
    }

    /* assign sensor info to row's private data pointer for later retrieval by
     * the table's oid handlers */
    row->data = ss;

    varlist = netsnmp_tdata_row_add_index(row, ASN_INTEGER, &ss->index,
                                          sizeof(ss->index));
    AIM_ASSERT(varlist != NULL);
    rv = netsnmp_tdata_add_row(table, row);
    AIM_ASSERT(rv == SNMPERR_SUCCESS);

    return 0;
}

static int
table_handler__(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reg_info,
                netsnmp_agent_request_info *req_info,
                netsnmp_request_info *requests,
                onlp_snmp_handler_fn table_handler_fns[])
{
    netsnmp_request_info *req;

    if (req_info->mode != MODE_GET && req_info->mode != MODE_GETNEXT) {
        return SNMP_ERR_NOERROR;
    }

    for (req = requests; req; req = req->next) {
        onlp_snmp_sensor_t *ss =
            (onlp_snmp_sensor_t *) netsnmp_tdata_extract_entry(req);
        netsnmp_table_request_info *table_info =
            netsnmp_extract_table_info(req);
        if (ss == NULL) {
            netsnmp_set_request_error(req_info, req, SNMP_NOSUCHINSTANCE);
            continue;
        }

        if (table_handler_fns[table_info->colnum]) {
            (*table_handler_fns[table_info->colnum])(req, table_info->colnum,
                                                     ss);
        } else {
            netsnmp_set_request_error(req_info, req, SNMP_NOSUCHINSTANCE);
            continue;
        }
    }

    if (handler->next && handler->next->access_method) {
        return netsnmp_call_next_handler(handler, reg_info, req_info, requests);
    }

    return SNMP_ERR_NOERROR;
}


typedef int (*table_handler_fn)(netsnmp_mib_handler *,
                                netsnmp_handler_registration *,
                                netsnmp_agent_request_info *,
                                netsnmp_request_info *);

static netsnmp_tdata *
register_table__(char table_name[], oid table_oid[], size_t table_oid_len,
                 unsigned int min_col, unsigned int max_col,
                 table_handler_fn handler_fn)
{
    netsnmp_tdata *table = netsnmp_tdata_create_table(table_name, 0);
    if (table == NULL) {
        AIM_LOG_ERROR("failed to create table %s", table_name);
        return NULL;
    }

    netsnmp_table_registration_info *table_info =
        SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info);
    if (table_info == NULL) {
        AIM_LOG_ERROR("failed to create table registration info for %s",
                      table_name);
        return NULL;
    }

    netsnmp_table_helper_add_indexes(table_info, ASN_INTEGER, 0);

    table_info->min_column = min_col;
    table_info->max_column = max_col;

    netsnmp_handler_registration *reg =
        netsnmp_create_handler_registration(table_name, handler_fn,
                                            table_oid, table_oid_len,
                                            HANDLER_CAN_RONLY);
    if (reg == NULL) {
        AIM_LOG_ERROR("failed to create handler registration for %s",
                      table_name);
        return NULL;
    }

    /* use lower priority to override default handler registered at
     * DEFAULT_MIB_PRIORITY on this OID */
    reg->priority = DEFAULT_MIB_PRIORITY - 1;

    if (netsnmp_tdata_register(reg, table, table_info) !=
        MIB_REGISTERED_OK) {
        AIM_LOG_ERROR("failed to register table %s", table_name);
        return NULL;
    }

    return table;
}


/**
 * Thermal Sensor Handlers
 */

static int
temp_update_handler__(onlp_snmp_sensor_t *ss)
{
    onlp_thermal_info_t *ti = &get_next_info(ss)->data.ti;
    onlp_oid_t oid = (onlp_oid_t) ss->sensor_id;

    return onlp_thermal_info_get(oid, ti);
}

static void
temp_index_handler__(netsnmp_request_info *req,
                     uint32_t index,
                     onlp_snmp_sensor_t *ss)
{
    snmp_set_var_typed_integer(req->requestvb,
                               ASN_INTEGER,
                               ss->index);
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
    sensor_info_t *si = get_curr_info(ss);
    onlp_thermal_info_t *ti = &si->data.ti;

    if (!si->valid) {
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
    sensor_info_t *si = get_curr_info(ss);
    onlp_thermal_info_t *ti = &si->data.ti;

    if (!si->valid) {
        return;
    }

    value = (ti->status & ONLP_THERMAL_STATUS_PRESENT)? ti->mcelsius: 0;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}

static onlp_snmp_handler_fn temp_handler_fn__[] = {
    NULL,
    temp_index_handler__,
    temp_devname_handler__,
    temp_status_handler__,
    temp_value_handler__,
};

static int
temp_table_handler__(netsnmp_mib_handler *handler,
                     netsnmp_handler_registration *reg,
                     netsnmp_agent_request_info *agent_req,
                     netsnmp_request_info *requests)
{
    return table_handler__(handler, reg, agent_req, requests,
                           temp_handler_fn__);
}


/**
 * Fan Sensor Handlers
 */
static int
fan_update_handler__(onlp_snmp_sensor_t *ss)
{
    onlp_fan_info_t *fi = &get_next_info(ss)->data.fi;
    onlp_oid_t oid = (onlp_oid_t) ss->sensor_id;

    return onlp_fan_info_get(oid, fi);
}

static void
fan_index_handler__(netsnmp_request_info *req,
                    uint32_t index,
                    onlp_snmp_sensor_t *ss)
{
    snmp_set_var_typed_integer(req->requestvb,
                               ASN_INTEGER,
                               ss->index);
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
    sensor_info_t *si = get_curr_info(ss);
    onlp_fan_info_t *fi = &si->data.fi;

    if (!si->valid) {
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
    sensor_info_t *si = get_curr_info(ss);
    onlp_fan_info_t *fi = &si->data.fi;

    if (!si->valid) {
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
    sensor_info_t *si = get_curr_info(ss);
    onlp_fan_info_t *fi = &si->data.fi;

    if (!si->valid) {
        return;
    }

    value = (fi->status & ONLP_FAN_STATUS_PRESENT)? fi->rpm: 0;

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
    sensor_info_t *si = get_curr_info(ss);
    onlp_fan_info_t *fi = &si->data.fi;

    if (!si->valid) {
        return;
    }

    value = (fi->status & ONLP_FAN_STATUS_PRESENT)? fi->percentage: 0;

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
    sensor_info_t *si = get_curr_info(ss);
    onlp_fan_info_t *fi = &si->data.fi;

    if (!si->valid) {
        return;
    }

    int len = (fi->status & ONLP_FAN_STATUS_PRESENT)? strlen(fi->model): 0;

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
    sensor_info_t *si = get_curr_info(ss);
    onlp_fan_info_t *fi = &si->data.fi;

    if (!si->valid) {
        return;
    }

    int len = (fi->status & ONLP_FAN_STATUS_PRESENT)? strlen(fi->serial): 0;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_OCTET_STR,
                             (u_char *) fi->serial,
                             len);
}

static onlp_snmp_handler_fn fan_handler_fn__[] = {
    NULL,
    fan_index_handler__,
    fan_devname_handler__,
    fan_status_handler__,
    fan_flow_type_handler__,
    fan_rpm_handler__,
    fan_pct_handler__,
    fan_model_handler__,
    fan_serial_handler__
};

static int
fan_table_handler__(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reg,
                    netsnmp_agent_request_info *agent_req,
                    netsnmp_request_info *requests)
{
    return table_handler__(handler, reg, agent_req, requests,
                           fan_handler_fn__);
}


/**
 * PSU Handlers
 */
static int
psu_update_handler__(onlp_snmp_sensor_t *ss)
{
    onlp_psu_info_t *pi = &get_next_info(ss)->data.pi;
    onlp_oid_t oid = (onlp_oid_t) ss->sensor_id;

    return onlp_psu_info_get(oid, pi);
}

static void
psu_index_handler__(netsnmp_request_info *req,
                    uint32_t index,
                    onlp_snmp_sensor_t *ss)
{
    snmp_set_var_typed_integer(req->requestvb,
                               ASN_INTEGER,
                               ss->index);
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
    sensor_info_t *si = get_curr_info(ss);
    onlp_psu_info_t *pi = &si->data.pi;

    if (!si->valid) {
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
    sensor_info_t *si = get_curr_info(ss);
    onlp_psu_info_t *pi = &si->data.pi;

    if (!si->valid) {
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
    sensor_info_t *si = get_curr_info(ss);
    onlp_psu_info_t *pi = &si->data.pi;

    if (!si->valid) {
        return;
    }

    int len = (pi->status & ONLP_PSU_STATUS_PRESENT)? strlen(pi->model): 0;

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
    sensor_info_t *si = get_curr_info(ss);
    onlp_psu_info_t *pi = &si->data.pi;

    if (!si->valid) {
        return;
    }

    int len = (pi->status & ONLP_PSU_STATUS_PRESENT)? strlen(pi->serial): 0;

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
    sensor_info_t *si = get_curr_info(ss);
    onlp_psu_info_t *pi = &si->data.pi;

    if (!si->valid) {
        return;
    }

    value = (pi->status & ONLP_PSU_STATUS_PRESENT)? pi->mvin: 0;

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
    sensor_info_t *si = get_curr_info(ss);
    onlp_psu_info_t *pi = &si->data.pi;

    if (!si->valid) {
        return;
    }

    value = (pi->status & ONLP_PSU_STATUS_PRESENT)? pi->mvout: 0;

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
    sensor_info_t *si = get_curr_info(ss);
    onlp_psu_info_t *pi = &si->data.pi;

    if (!si->valid) {
        return;
    }

    value = (pi->status & ONLP_PSU_STATUS_PRESENT)? pi->miin: 0;

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
    sensor_info_t *si = get_curr_info(ss);
    onlp_psu_info_t *pi = &si->data.pi;

    if (!si->valid) {
        return;
    }

    value = (pi->status & ONLP_PSU_STATUS_PRESENT)? pi->miout: 0;

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
    sensor_info_t *si = get_curr_info(ss);
    onlp_psu_info_t *pi = &si->data.pi;

    if (!si->valid) {
        return;
    }

    value = (pi->status & ONLP_PSU_STATUS_PRESENT)? pi->mpin: 0;

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
    sensor_info_t *si = get_curr_info(ss);
    onlp_psu_info_t *pi = &si->data.pi;

    if (!si->valid) {
        return;
    }

    value = (pi->status & ONLP_PSU_STATUS_PRESENT)? pi->mpout: 0;

    snmp_set_var_typed_value(req->requestvb,
                             ASN_GAUGE,
                             (u_char *) &value,
                             sizeof(value));
}


static onlp_snmp_handler_fn psu_handler_fn__[] = {
    NULL,
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

static int
psu_table_handler__(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reg,
                    netsnmp_agent_request_info *agent_req,
                    netsnmp_request_info *requests)
{
    return table_handler__(handler, reg, agent_req, requests,
                           psu_handler_fn__);
}


/*
 * All update handlers
 */
static update_handler_fn all_update_handler_fns__[] = {
    NULL,
    temp_update_handler__,
    fan_update_handler__,
    psu_update_handler__,
};


/*
 * Add a sensor to the appropriate type-specific control structure.
 * Updates next sensor info, not current sensor info.
 */
static void
add_sensor__(int sensor_type, onlp_snmp_sensor_t *new_sensor)
{
    onlp_snmp_sensor_ctrl_t *ctrl = get_sensor_ctrl__(sensor_type);
    list_links_t *curr;
    onlp_snmp_sensor_t *ss;

    /* We start with Base 1 */
    AIM_TRUE_OR_DIE(onlp_snmp_sensor_type_valid(sensor_type));
    AIM_TRUE_OR_DIE(new_sensor);
    AIM_TRUE_OR_DIE(ctrl);

    /* check if the sensor already exists */
    LIST_FOREACH(&ctrl->sensors, curr) {
        ss = container_of(curr, links, onlp_snmp_sensor_t);
        if (new_sensor->sensor_id == ss->sensor_id) {
            /* no need to add sensor */
            AIM_LOG_TRACE("skipping existing sensor %08x", ss->sensor_id);
            get_next_info(ss)->valid = true;
            return;
        }
    }

    ss = AIM_MALLOC(sizeof(onlp_snmp_sensor_t));
    AIM_TRUE_OR_DIE(ss);
    AIM_MEMCPY(ss, new_sensor, sizeof(*new_sensor));
    ss->sensor_type = sensor_type;
    get_next_info(ss)->valid = true;

    /* finally add sensor */
    list_push(&ctrl->sensors, &ss->links);
}


static int
collect_sensors__(onlp_oid_t oid, void* cookie)
{
    onlp_oid_hdr_t hdr;
    onlp_snmp_sensor_t s;

    onlp_oid_hdr_get(oid, &hdr);
    AIM_LOG_MSG("collect: %{onlp_oid}", oid);

    AIM_MEMSET(&s, 0x0, sizeof(onlp_snmp_sensor_t));
    switch(ONLP_OID_TYPE_GET(oid))
        {
        case ONLP_OID_TYPE_THERMAL:
#if ONLP_SNMP_CONFIG_INCLUDE_THERMALS == 1
            s.sensor_id = oid;
            s.index = ONLP_OID_ID_GET(oid);
            sprintf(s.name, "%d - ", ONLP_OID_ID_GET(oid));
            aim_strlcpy(s.desc, hdr.description, sizeof(s.desc));
            add_sensor__(ONLP_SNMP_SENSOR_TYPE_TEMP, &s);
#endif
            break;

        case ONLP_OID_TYPE_FAN:
#if ONLP_SNMP_CONFIG_INCLUDE_FANS == 1
            s.sensor_id = oid;
            s.index = ONLP_OID_ID_GET(oid);
            sprintf(s.name, "%d - ", ONLP_OID_ID_GET(oid));
            aim_strlcpy(s.desc, hdr.description, sizeof(s.desc));
            add_sensor__(ONLP_SNMP_SENSOR_TYPE_FAN, &s);
#endif
            break;

        case ONLP_OID_TYPE_PSU:
#if ONLP_SNMP_CONFIG_INCLUDE_PSUS == 1
            /* Register Sensors for VIN,VOUT,IIN,IOUT,PIN,POUT */
            s.sensor_id = oid;
            s.index = ONLP_OID_ID_GET(oid);
            sprintf(s.name, "%d - ", ONLP_OID_ID_GET(oid));
            aim_strlcpy(s.desc, hdr.description, sizeof(s.desc));
            add_sensor__(ONLP_SNMP_SENSOR_TYPE_PSU, &s);
#endif
            break;

        default:
            AIM_LOG_VERBOSE("snmp type %s id %d unsupported",
                            onlp_oid_type_name(ONLP_OID_TYPE_GET(oid)),
                            ONLP_OID_ID_GET(oid));
            break;
    }

    return 0;
}


/*
 * sensor table is updated in two parts:
 * 1. sensor update, performed in separate thread by calling update_tables__.
 *    once update is complete, front-back buffers will be switched
 *    and flag set to indicate table restructuring can occur
 * 2. sensor table restructuring, performed in snmp callback
 *    by calling restructure_tables__.
 */

static void
update_tables__(void)
{
    int i;
    onlp_snmp_sensor_ctrl_t *ctrl;
    list_links_t *curr;
    onlp_snmp_sensor_t *ss;

    uint64_t now = aim_time_monotonic();
    if (now - last_sensor_update_time <
        (ONLP_SNMP_CONFIG_UPDATE_PERIOD * 1000 * 1000)) {
        return;
    }

    if (restructure_trigger) {
        AIM_LOG_INFO("restructure has not happened, skip sensor update");
        return;
    }

    last_sensor_update_time = now;
    AIM_LOG_TRACE("update sensor objects");

    /* for each table: mark next_info invalid */
    for (i = ONLP_SNMP_SENSOR_TYPE_TEMP; i <= ONLP_SNMP_SENSOR_TYPE_MAX; i++) {
        ctrl = get_sensor_ctrl__(i);
        LIST_FOREACH(&ctrl->sensors, curr) {
            ss = container_of(curr, links, onlp_snmp_sensor_t);
            get_next_info(ss)->valid = false;
        }
    }

    /* discover new sensors for all tables,
     * writing validity into next_info for all sensors */
    onlp_oid_iterate(ONLP_OID_SYS, 0, collect_sensors__, NULL);

    /* for each table: update all sensor info */
    for (i = ONLP_SNMP_SENSOR_TYPE_TEMP; i <= ONLP_SNMP_SENSOR_TYPE_MAX; i++) {
        ctrl = get_sensor_ctrl__(i);
        LIST_FOREACH(&ctrl->sensors, curr) {
            ss = container_of(curr, links, onlp_snmp_sensor_t);
            if (get_next_info(ss)->valid) {
                AIM_LOG_INFO("update sensor %s%s", ss->name, ss->desc);
                /* invoke update handler */
                if ((*all_update_handler_fns__[i])(ss) != ONLP_STATUS_OK) {
                    AIM_LOG_ERROR("failed to update %s%s", ss->name, ss->desc);
                    get_next_info(ss)->valid = false;
                }
            }
        }
    }

    /* swap front and back buffers */
    swap_curr_next_info();

    /* trigger table restructuring */
    AIM_LOG_TRACE("trigger restructure");
    restructure_trigger = true;
}

/*
 * adds or removes rows from sensor table.
 * registered with snmp_alarm_register.
 * table restructuring then happens within alarm handler,
 * thus avoiding crashes when table is changed while handling snmp requests.
 */
static void
restructure_tables__(unsigned int reg, void *clientarg)
{
    int i;
    onlp_snmp_sensor_ctrl_t *ctrl;
    list_links_t *curr;
    list_links_t *next;
    onlp_snmp_sensor_t *ss;
    bool previously_valid;
    bool now_valid;

    if (!restructure_trigger) {
        return;
    }

    AIM_LOG_INFO("restructuring tables");

    /* for each table: add or delete rows as necessary */
    for (i = ONLP_SNMP_SENSOR_TYPE_TEMP; i <= ONLP_SNMP_SENSOR_TYPE_MAX; i++) {
        ctrl = get_sensor_ctrl__(i);
        LIST_FOREACH_SAFE(&ctrl->sensors, curr, next) {
            ss = container_of(curr, links, onlp_snmp_sensor_t);
            previously_valid = get_next_info(ss)->valid;
            now_valid = get_curr_info(ss)->valid;
            if (!previously_valid && now_valid) {
                snmp_log(LOG_INFO, "Adding %s%s, id=%08x",
                         ss->name, ss->desc, ss->sensor_id);
                AIM_LOG_INFO("add row %d to %s for %s%s",
                                ss->index, ctrl->name, ss->name, ss->desc);
                add_table_row__(sensor_table__[i], ss);
            } else if (previously_valid && !now_valid) {
                snmp_log(LOG_INFO, "Deleting %s%s, id=%08x",
                         ss->name, ss->desc, ss->sensor_id);
                AIM_LOG_INFO("delete row %d from %s for %s%s",
                                ss->index, ctrl->name, ss->name, ss->desc);
                delete_table_row__(sensor_table__[i], ss->index);
                list_remove(curr);
                aim_free(ss);
            }
        }
    }

    AIM_LOG_INFO("restructuring complete");

    restructure_trigger = false;
}


typedef struct table_cfg_s {
    onlp_snmp_sensor_type_t type;
    char name[32];
    unsigned int min_col;
    unsigned int max_col;
    table_handler_fn handler;
} table_cfg_t;

static void
init_all_tables__(void)
{
    int i;

    /* initialize control blocks */
    for (i = ONLP_SNMP_SENSOR_TYPE_TEMP; i <= ONLP_SNMP_SENSOR_TYPE_MAX; i++) {
        onlp_snmp_sensor_ctrl_t *ctrl = get_sensor_ctrl__(i);
        aim_strlcpy(ctrl->name, onlp_snmp_sensor_type_name(i),
                    sizeof(ctrl->name));
        list_init(&ctrl->sensors);
    }

    /* register oids with netsnmp */
    table_cfg_t cfgs[] = {
        {
            .type = ONLP_SNMP_SENSOR_TYPE_TEMP,
            .name = "onlTempTable",
            .min_col = 1,
            .max_col = AIM_ARRAYSIZE(temp_handler_fn__)-1,
            .handler = temp_table_handler__,
        },
        {
            .type = ONLP_SNMP_SENSOR_TYPE_FAN,
            .name = "onlFanTable",
            .min_col = 1,
            .max_col = AIM_ARRAYSIZE(fan_handler_fn__)-1,
            .handler = fan_table_handler__,
        },
        {
            .type = ONLP_SNMP_SENSOR_TYPE_PSU,
            .name = "onlPsuTable",
            .min_col = 1,
            .max_col = AIM_ARRAYSIZE(psu_handler_fn__)-1,
            .handler = psu_table_handler__,
        },
    };

    for (i = 0; i < AIM_ARRAYSIZE(cfgs); i++) {
        table_cfg_t *cfg = &cfgs[i];
        oid o[] = { ONLP_SNMP_SENSOR_OID, cfg->type };
        sensor_table__[cfg->type] =
            register_table__(cfg->name, o, OID_LENGTH(o),
                             cfg->min_col, cfg->max_col, cfg->handler);
        AIM_TRUE_OR_DIE(sensor_table__[cfg->type]);
    }
}


/* populates initial stats and sets up periodic timer */
static void
setup_alarm__(void)
{
    /* initial stats population: update and restructure */
    update_tables__();
    restructure_tables__(0, NULL);
    /* registration of periodic timer for table restructuring */
    snmp_alarm_register(1, SA_REPEAT, restructure_tables__, NULL);
}


int
onlp_snmp_sensors_init(void)
{
    init_all_tables__();
    setup_alarm__();
    return 0;
}

#define MIN(a,b) ((a)<(b)? (a): (b))
static unsigned int
us_to_next_update(void)
{
    uint64_t deltat = aim_time_monotonic() - last_sensor_update_time;
    uint64_t period = ONLP_SNMP_CONFIG_UPDATE_PERIOD * 1000 * 1000;
    return MIN(period - deltat, period);
}

static void *
do_update(void *arg)
{
    for (;;) {
        update_tables__();
        usleep(us_to_next_update());
    }

    return NULL;
}

int
onlp_snmp_sensor_update_start(void)
{
    if (pthread_create(&update_thread_handle, NULL, do_update, NULL) < 0) {
        AIM_LOG_ERROR("update thread creation failed");
        return -1;
    }
    return 0;
}

int
onlp_snmp_sensor_update_stop(void)
{
    pthread_join(update_thread_handle, NULL);
    return 0;
}
