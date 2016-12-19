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
#include "onlp_snmp_log.h"

#include <OS/os_time.h>
#include <cjson/cJSON.h>
#include <cjson_util/cjson_util.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <onlp/sys.h>

static void
platform_string_register(int index, const char* desc, char* value)
{
    oid tree[] = { 1, 3, 6, 1, 4, 1, 42623, 1, 1, 1, 1, 1};
    tree[11] = index;

    if(!value || !value[0]) {
        return;
    }

    char* s = aim_strdup(value);

    netsnmp_handler_registration *reg =
        netsnmp_create_handler_registration(
                                            desc, NULL,
                                            tree, OID_LENGTH(tree),
                                            HANDLER_CAN_RONLY);
    netsnmp_watcher_info *winfo =
        netsnmp_create_watcher_info(
                                    s, strlen(s),
                                    ASN_OCTET_STR, WATCHER_FIXED_SIZE);
    netsnmp_register_watched_scalar( reg, winfo );
}

static void
platform_int_register(int index, char* desc, int value)
{
    oid tree[] = { 1, 3, 6, 1, 4, 1, 42623, 1, 1, 1, 1, 1};
    tree[11] = index;

    int* v = aim_zmalloc(sizeof(value));
    *v = value;

    netsnmp_register_int_instance(desc,
                                  tree,
                                  OID_LENGTH(tree),
                                  v, NULL);
}

static void
resource_int_register(int index, const char* desc,
                      Netsnmp_Node_Handler *handler)
{
    oid tree[] = { 1, 3, 6, 1, 4, 1, 42623, 1, 3, 1, 1 };
    tree[10] = index;

    netsnmp_handler_registration *reg =
        netsnmp_create_handler_registration(desc, handler,
                                            tree, OID_LENGTH(tree),
                                            HANDLER_CAN_RONLY);
    if (netsnmp_register_scalar(reg) != MIB_REGISTERED_OK) {
        AIM_LOG_ERROR("registering handler for %s failed", desc);
    }
}


/* resource objects refreshed with this period; units in seconds */
#define RESOURCE_UPDATE_PERIOD 5

/* resource objects */
typedef struct {
    uint32_t utilization_percent;
    uint32_t idle_percent;
} resources_t;

static resources_t resources;
static uint64_t resource_update_time;

void resource_update(void)
{
    uint64_t now = os_time_monotonic();
    if (now - resource_update_time > RESOURCE_UPDATE_PERIOD * 1000 * 1000) {
        resource_update_time = now;
        AIM_LOG_INFO("update resource objects");

        /* invoke mpstat collection script for json output */
        FILE *fp = popen("/usr/bin/onl-snmp-mpstat", "r");
        if (fp == NULL) {
            AIM_LOG_ERROR("failed invoking onl-snmp-mpstat");
            return;
        }

        /* parse json output */
        char line[1024];
        while (fgets(line, sizeof(line), fp) != NULL) {
            cJSON *root = cJSON_Parse(line);
            int result;
            int rv = cjson_util_lookup_int(root, &result, "all.%%idle");
            if (rv == 0) {
                /* save it */
                resources.idle_percent = result;
                resources.utilization_percent = 100 - result;
            }
            cJSON_Delete(root);
        }
    }
}

static int
utilization_handler(netsnmp_mib_handler *handler,
             netsnmp_handler_registration *reginfo,
             netsnmp_agent_request_info *reqinfo,
             netsnmp_request_info *requests)
{
    if (MODE_GET == reqinfo->mode) {
        resource_update();
        snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE,
                                 (u_char *) &resources.utilization_percent,
                                 sizeof(resources.utilization_percent));
    } else {
        netsnmp_assert("bad mode in RO handler");
    }

    if (handler->next && handler->next->access_method) {
        return netsnmp_call_next_handler(handler, reginfo, reqinfo, requests);
    }

    return SNMP_ERR_NOERROR;
}

static int
idle_handler(netsnmp_mib_handler *handler,
             netsnmp_handler_registration *reginfo,
             netsnmp_agent_request_info *reqinfo,
             netsnmp_request_info *requests)
{
    if (MODE_GET == reqinfo->mode) {
        resource_update();
        snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE,
                                 (u_char *) &resources.idle_percent,
                                 sizeof(resources.idle_percent));
    } else {
        netsnmp_assert("bad mode in RO handler");
    }

    if (handler->next && handler->next->access_method) {
        return netsnmp_call_next_handler(handler, reginfo, reqinfo, requests);
    }

    return SNMP_ERR_NOERROR;
}

void
onlp_snmp_platform_init(void)
{
    /**
     * This is the base of the platform:general:system tree
     */
    onlp_sys_info_t si;
    if(onlp_sys_info_get(&si) >= 0) {

#define REGISTER_STR(_index, _field)                                    \
        do {                                                            \
            platform_string_register(_index, #_field, (char*)si.onie_info._field); \
        } while(0)

#define REGISTER_INT(_index, _field)                                    \
        do {                                                            \
            platform_int_register(_index, #_field, si.onie_info._field); \
        } while(0)

        REGISTER_STR(1, product_name);
        REGISTER_STR(2, part_number);
        REGISTER_STR(3, serial_number);
        char* mstring = aim_fstrdup("%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
                                    si.onie_info.mac[0], si.onie_info.mac[1], si.onie_info.mac[2],
                                    si.onie_info.mac[3], si.onie_info.mac[4], si.onie_info.mac[5]);
        platform_string_register(4, "mac", mstring);
        aim_free(mstring);

        REGISTER_INT(5,  mac_range);
        REGISTER_STR(6,  manufacturer);
        REGISTER_STR(7,  manufacture_date);
        REGISTER_STR(8,  vendor);
        REGISTER_STR(9,  platform_name);
        REGISTER_INT(10, device_version);
        REGISTER_STR(11, label_revision);
        REGISTER_STR(12, country_code);
        REGISTER_STR(13, diag_version);
        REGISTER_STR(14, service_tag);
        REGISTER_STR(15, onie_version);
    }

    resource_int_register(1, "CpuAllPercentUtilization", utilization_handler);
    resource_int_register(2, "CpuAllPercentIdle", idle_handler);
}

