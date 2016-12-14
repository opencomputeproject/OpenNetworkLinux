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

void
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
}

