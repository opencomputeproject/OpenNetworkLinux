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
 *
 *
 ***********************************************************/

#include <onlp/onlp_config.h>

#if ONLP_CONFIG_INCLUDE_UCLI == 1

#include <uCli/ucli.h>
#include <uCli/ucli_argparse.h>
#include <uCli/ucli_handler_macros.h>
#include <onlp/onlp.h>
#include <onlp/platform.h>
#include <onlp/attribute.h>
#include <onlp/stdattrs.h>
#include <onlp/fan.h>
#include <onlp/sfp.h>
#include <onlplib/onie.h>
#include <onlp/oids.h>
#include <cjson_util/cjson_util_format.h>
#include <AIM/aim_sleep.h>

#include <onlp/chassis.h>
#include <onlp/module.h>
#include <onlp/thermal.h>
#include <onlp/fan.h>
#include <onlp/led.h>
#include <onlp/generic.h>
#include <onlp/psu.h>

/**
 * Needed for commands which invoke daemonized restart.
 */
char** onlp_ucli_argv = NULL;

#define ONLP_CMD_STATUS(_rv)                                    \
    do {                                                        \
        if(ONLP_FAILURE(_rv)) {                                 \
            ucli_printf(uc, "failed: %{onlp_status}\n", _rv);   \
        }                                                       \
        return 0;                                               \
    } while(0)

static ucli_status_t
onlp_ucli__chassis__onie__show__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "show", -1,
                      "$summary#Show the chassis onie information.");
    onlp_attribute_onie_info_show(ONLP_OID_CHASSIS, &uc->pvs);
    return 0;
}

static ucli_status_t
onlp_ucli__chassis__onie__json__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "json", -1,
                      "$summary#Show the chassis onie information.");
    onlp_attribute_onie_info_show_json(ONLP_OID_CHASSIS, &uc->pvs);
    return 0;
}
static ucli_status_t
onlp_ucli__chassis__asset__show__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "show", 0,
                      "$summary#Show the chassis asset information.");
    onlp_attribute_asset_info_show(ONLP_OID_CHASSIS, &uc->pvs);
    return 0;
}

static ucli_status_t
onlp_ucli__chassis__asset__json__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "json", 0,
                      "$summary#Show the chassis asset information.");
    onlp_attribute_asset_info_show_json(ONLP_OID_CHASSIS, &uc->pvs);
    return 0;
}

static ucli_status_t
onlp_ucli__chassis__env__show__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "env", 0,
                      "$summary#Show the chassis environment information.");
    onlp_chassis_environment_show(&uc->pvs, ONLP_OID_JSON_FLAG_TO_USER_JSON);
    return 0;
}

static ucli_status_t
onlp_ucli__chassis__debug__show__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "show", 0,
                      "Dump all chassis OIDs.");
    onlp_chassis_debug_show(&uc->pvs);
    return 0;
}

static ucli_status_t
onlp_ucli__debug__oid__verify__json__(ucli_context_t* uc)
{
    onlp_oid_t oid;
    UCLI_COMMAND_INFO(uc, "json", 1, "");
    UCLI_ARGPARSE_OR_RETURN(uc, "{onlp_oid}", &oid);
    ucli_printf(uc, "%{onlp_oid}: %{onlp_status}\n",
                oid, onlp_oid_json_verify(oid));
    return 0;
}

static ucli_status_t
onlp_ucli__debug__oid__from__json__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc, "json", 1, "");
    int rv;
    cJSON* cj;
    char* filename;

    UCLI_ARGPARSE_OR_RETURN(uc, "s", &filename);
    if(cjson_util_parse_file(filename, &cj) < 0) {
        ucli_printf(uc, "Could not parse json file %s\n", filename);
        return -1;
    }
    biglist_t* all_oids = NULL;
    if(ONLP_SUCCESS(rv = onlp_oid_from_json(cj, NULL, &all_oids,
                                            ONLP_OID_JSON_FLAG_RECURSIVE))) {
        biglist_t* ble;
        onlp_oid_hdr_t* hdr;
        BIGLIST_FOREACH_DATA(ble, all_oids, onlp_oid_hdr_t*, hdr) {
            cJSON* object = NULL;
            ucli_printf(uc, "%{onlp_oid_hdr}\n", hdr);
            if(ONLP_SUCCESS(onlp_oid_info_to_json(hdr, &object, 0))) {
                cjson_util_yaml_pvs(&uc->pvs, object);
            }
            cJSON_Delete(object);
        }
    }
    else {
        ucli_printf(uc, "onlp_oid_from_json returned %{onlp_status}\n", rv);
        return 0;
    }
    return 0;
}

static ucli_status_t
onlp_ucli__debug__oid__to__json__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "json", 2, "");

    int rv;
    cJSON* cj = NULL;
    onlp_oid_t oid;
    int choice;

    UCLI_ARGPARSE_OR_RETURN(uc, "{onlp_oid}{choice}", &oid, &choice, "type", 3, "debug", "debug-all", "user");
    switch(choice)
        {
        case 0:
            {
                rv = onlp_oid_to_json(oid, &cj, 0);
                break;
            }
        case 1:
            {
                rv = onlp_oid_to_json(oid, &cj, ONLP_OID_JSON_FLAG_RECURSIVE);
                break;
            }
        case 2:
            {
                rv = onlp_oid_to_user_json(oid, &cj,
                                           ONLP_OID_JSON_FLAG_RECURSIVE);
                break;
            }
        default: rv = ONLP_STATUS_E_PARAM; break;
        }

    if(ONLP_SUCCESS(rv)) {
        cjson_util_json_pvs(&uc->pvs, cj);
        cJSON_Delete(cj);
    }
    else {
        ucli_printf(uc, "oid to json failed: %{onlp_status}", rv);
    }
    return 0;
}


static ucli_status_t
onlp_ucli__oid__hdr__json__id__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "id", 1, "");
    int rv;
    onlp_oid_t oid;

    UCLI_ARGPARSE_OR_RETURN(uc, "{onlp_oid}", &oid);

    onlp_oid_hdr_t hdr;
    if(ONLP_SUCCESS(rv = onlp_oid_hdr_get(oid, &hdr))) {
        cJSON* cj;
        if(ONLP_SUCCESS(rv = onlp_oid_hdr_to_json(&hdr, &cj, 0))) {
            cjson_util_json_pvs(&uc->pvs, cj);
            cJSON_Delete(cj);
        }
        else {
            ucli_printf(uc, "onlp_oid_hdr_to_json failed: %{onlp_status}", rv);
        }
    }
    else {
        ucli_printf(uc, "onlp_oid_hdr_get failed: %{onlp_status}", rv);
    }
    return UCLI_STATUS_OK;
}

static ucli_status_t
onlp_ucli__oid__hdr__json__file__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "file", 1, "");

    int rv;
    char* f;
    UCLI_ARGPARSE_OR_RETURN(uc, "s", &f);

    cJSON* cj;
    if(cjson_util_parse_file(f, &cj) < 0) {
        ucli_printf(uc, "failed to open or parse '%s'\n", f);
        return UCLI_STATUS_E_ERROR;
    }
    onlp_oid_hdr_t hdr;
    if(ONLP_FAILURE(rv = onlp_oid_hdr_from_json(cj, &hdr))) {
        ucli_printf(uc, "hdr from json failed: %{onlp_status}\n", rv);
        return UCLI_STATUS_E_ERROR;
    }
    cJSON_Delete(cj);

    if(ONLP_FAILURE(rv = onlp_oid_hdr_to_json(&hdr, &cj, 0))) {
        ucli_printf(uc, "hdr to json failed: %{onlp_status}\n", rv);
        return UCLI_STATUS_E_ERROR;
    }
    cjson_util_json_pvs(&uc->pvs, cj);
    cJSON_Delete(cj);
    return 0;
}



static ucli_status_t
onlp_ucli__sfp__inventory__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "inventory", 0,
                      "$summary#Show the SFP inventory.");
    onlp_sfp_inventory_show(&uc->pvs);
    return 0;
}

static ucli_status_t
onlp_ucli__sfp__dev__read__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "read", 4,
                      "$summary#Read bytes from the given addresses.");
    int rv;
    onlp_oid_t port;
    int devaddr;
    int addr; int size;
    uint8_t* data;

    UCLI_ARGPARSE_OR_RETURN(uc, "{onlp_oid_type}iii",
                            &port, ONLP_OID_TYPE_SFP, &devaddr, &addr, &size);
    if(ONLP_SUCCESS(rv = onlp_sfp_dev_alloc_read(port, devaddr, addr, size, &data))) {
        ucli_printf(uc, "%{data}\n", data, size);
        aim_free(data);
    }
    else {
        ucli_printf(uc, "%{onlp_status}\n", rv);
    }
    return 0;
}

static ucli_status_t
onlp_ucli__sfp__bitmaps__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "bitmaps", 0,
                      "$summary#Show SFP Presence and RX_LOS bitmaps.");

    int rv;
    onlp_sfp_bitmap_t bmap;

    ucli_printf(uc, "Presence: ");
    if(ONLP_SUCCESS(rv = onlp_sfp_presence_bitmap_get(&bmap))) {
        ucli_printf(uc, "%{aim_bitmap}\n", &bmap);
    }
    else {
        ucli_printf(uc, "%{onlp_status}\n", rv);
    }

    ucli_printf(uc, "RX_LOS: ");
    if(ONLP_SUCCESS(rv = onlp_sfp_rx_los_bitmap_get(&bmap))) {
        ucli_printf(uc, "%{aim_bitmap}\n", &bmap);
    }
    else {
        ucli_printf(uc, "%{onlp_status}\n", rv);
    }
    return 0;
}


static ucli_status_t
onlp_ucli__fan__rpm__get(ucli_context_t* uc)
{

    UCLI_COMMAND_INFO(uc,
                      "get", 1,
                      "$summary#Show the RPM for the given fan.");

    int rv;
    onlp_oid_t oid;
    onlp_fan_info_t info;

    UCLI_ARGPARSE_OR_RETURN(uc, "{onlp_oid_type}", &oid, ONLP_OID_TYPE_FAN);

    if(ONLP_SUCCESS(rv = onlp_fan_info_get(oid, &info))) {
        if(info.caps & ONLP_FAN_CAPS_GET_RPM) {
            ucli_printf(uc, "%{onlp_oid} rpm = %d\n", oid, info.rpm);
        }
        else {
            ucli_printf(uc, "%{onlp_oid} does not support rpm.\n", oid);
        }
    }
    else {
        ucli_printf(uc, "%{onlp_status}\n", rv);
    }

    return 0;
}

static ucli_status_t
onlp_ucli__fan__rpm__set(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "set", 2,
                      "$summary#Set the RPM for the given fan.");

    int rpm;
    onlp_oid_t oid;

    UCLI_ARGPARSE_OR_RETURN(uc, "{onlp_oid_type}i", &oid, ONLP_OID_TYPE_FAN, &rpm);
    ucli_printf(uc, "%{onlp_status}\n",
                onlp_fan_rpm_set(oid, rpm));
    return 0;
}

static ucli_status_t
onlp_ucli__fan__percentage__get(ucli_context_t* uc)
{

    UCLI_COMMAND_INFO(uc,
                      "get", 1,
                      "$summary#Show the speed percentage for the given fan.");

    int rv;
    onlp_oid_t oid;
    onlp_fan_info_t info;

    UCLI_ARGPARSE_OR_RETURN(uc, "{onlp_oid_type}", &oid, ONLP_OID_TYPE_FAN);

    if(ONLP_SUCCESS(rv = onlp_fan_info_get(oid, &info))) {
        if(info.caps & ONLP_FAN_CAPS_GET_PERCENTAGE) {
            ucli_printf(uc, "%{onlp_oid} percentage = %d\n", oid, info.percentage);
        }
        else {
            ucli_printf(uc, "%{onlp_oid} does not support percentage.\n", oid);
        }
    }
    else {
        ucli_printf(uc, "%{onlp_status}\n", rv);
    }

    return 0;
}

static ucli_status_t
onlp_ucli__fan__percentage__set(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "set", 2,
                      "$summary#Set the speed percentage for the given fan.");

    int p;
    onlp_oid_t oid;

    UCLI_ARGPARSE_OR_RETURN(uc, "{onlp_oid_type}i", &oid, ONLP_OID_TYPE_FAN, &p);
    ucli_printf(uc, "%{onlp_status}\n",
                onlp_fan_percentage_set(oid, p));
    return 0;
}

static ucli_status_t
onlp_ucli__platform__manager__run__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "run", 1,
                      "$summary#Run the platform manager for the given number of seconds."
                      "$args#<seconds>");
    int seconds;
    UCLI_ARGPARSE_OR_RETURN(uc, "i", &seconds);
    ucli_printf(uc, "Running the platform manager for %d seconds...\n", seconds);
    onlp_platform_manager_start(0);
    aim_sleep_usecs(seconds*1000000);
    ucli_printf(uc, "Stopping the platform manager...");
    onlp_platform_manager_stop(1);
    ucli_printf(uc, "done\n");
    return UCLI_STATUS_OK;
}

static ucli_status_t
onlp_ucli__platform__manager__daemon__(ucli_context_t* uc)
{
    UCLI_COMMAND_INFO(uc,
                      "daemon", 3,
                      "$summary#Start the platform management daemon."
                      "$args#<agent-name> <log-file> <pid-file>");

    char *name, *pid, *log;

    if(onlp_ucli_argv == NULL) {
        /*
         * This is not an interactive command. It must be invoked
         * via the command line tool.
         */
        ucli_printf(uc, "This command cannot be invoked interactively..");
        return UCLI_STATUS_E_ERROR;
    }

    UCLI_ARGPARSE_OR_RETURN(uc, "sss", &name, &log, &pid);
    onlp_platform_manager_daemon(name, log, pid, onlp_ucli_argv);

    /* We should never get here. */
    ucli_printf(uc, "platform manager daemon has failed.");
    return UCLI_STATUS_E_INTERNAL;
}


/* <auto.ucli.handlers.start> */
/******************************************************************************
 *
 * These handler table(s) were autogenerated from the symbols in this
 * source file.
 *
 *****************************************************************************/
ucli_node_t* onlp_ucli__node__ = NULL;
ucli_node_t* onlp_ucli__oid__node__ = NULL;
ucli_node_t* onlp_ucli__oid__hdr__node__ = NULL;
ucli_node_t* onlp_ucli__oid__hdr__json__node__ = NULL;
static ucli_command_handler_f onlp_ucli__oid__hdr__json__json__handlers__[] = 
{
    onlp_ucli__oid__hdr__json__id__,
    onlp_ucli__oid__hdr__json__file__,
    NULL
};
static ucli_module_t onlp_ucli__oid__hdr__json__json__module__ = 
{
    "json",
    NULL,
    onlp_ucli__oid__hdr__json__json__handlers__,
    NULL,
    NULL
};
ucli_node_t* onlp_ucli__sfp__node__ = NULL;
ucli_node_t* onlp_ucli__sfp__dev__node__ = NULL;
static ucli_command_handler_f onlp_ucli__sfp__dev__dev__handlers__[] = 
{
    onlp_ucli__sfp__dev__read__,
    NULL
};
static ucli_module_t onlp_ucli__sfp__dev__dev__module__ = 
{
    "dev",
    NULL,
    onlp_ucli__sfp__dev__dev__handlers__,
    NULL,
    NULL
};
static ucli_command_handler_f onlp_ucli__sfp__sfp__handlers__[] = 
{
    onlp_ucli__sfp__inventory__,
    onlp_ucli__sfp__bitmaps__,
    NULL
};
static ucli_module_t onlp_ucli__sfp__sfp__module__ = 
{
    "sfp",
    NULL,
    onlp_ucli__sfp__sfp__handlers__,
    NULL,
    NULL
};
ucli_node_t* onlp_ucli__platform__node__ = NULL;
ucli_node_t* onlp_ucli__platform__manager__node__ = NULL;
static ucli_command_handler_f onlp_ucli__platform__manager__manager__handlers__[] = 
{
    onlp_ucli__platform__manager__run__,
    onlp_ucli__platform__manager__daemon__,
    NULL
};
static ucli_module_t onlp_ucli__platform__manager__manager__module__ = 
{
    "manager",
    NULL,
    onlp_ucli__platform__manager__manager__handlers__,
    NULL,
    NULL
};
ucli_node_t* onlp_ucli__chassis__node__ = NULL;
ucli_node_t* onlp_ucli__chassis__debug__node__ = NULL;
static ucli_command_handler_f onlp_ucli__chassis__debug__debug__handlers__[] = 
{
    onlp_ucli__chassis__debug__show__,
    NULL
};
static ucli_module_t onlp_ucli__chassis__debug__debug__module__ = 
{
    "debug",
    NULL,
    onlp_ucli__chassis__debug__debug__handlers__,
    NULL,
    NULL
};
ucli_node_t* onlp_ucli__chassis__onie__node__ = NULL;
static ucli_command_handler_f onlp_ucli__chassis__onie__onie__handlers__[] = 
{
    onlp_ucli__chassis__onie__show__,
    onlp_ucli__chassis__onie__json__,
    NULL
};
static ucli_module_t onlp_ucli__chassis__onie__onie__module__ = 
{
    "onie",
    NULL,
    onlp_ucli__chassis__onie__onie__handlers__,
    NULL,
    NULL
};
ucli_node_t* onlp_ucli__chassis__asset__node__ = NULL;
static ucli_command_handler_f onlp_ucli__chassis__asset__asset__handlers__[] = 
{
    onlp_ucli__chassis__asset__show__,
    onlp_ucli__chassis__asset__json__,
    NULL
};
static ucli_module_t onlp_ucli__chassis__asset__asset__module__ = 
{
    "asset",
    NULL,
    onlp_ucli__chassis__asset__asset__handlers__,
    NULL,
    NULL
};
ucli_node_t* onlp_ucli__chassis__env__node__ = NULL;
static ucli_command_handler_f onlp_ucli__chassis__env__env__handlers__[] = 
{
    onlp_ucli__chassis__env__show__,
    NULL
};
static ucli_module_t onlp_ucli__chassis__env__env__module__ = 
{
    "env",
    NULL,
    onlp_ucli__chassis__env__env__handlers__,
    NULL,
    NULL
};
ucli_node_t* onlp_ucli__fan__node__ = NULL;
ucli_node_t* onlp_ucli__fan__percentage__node__ = NULL;
static ucli_command_handler_f onlp_ucli__fan__percentage__percentage__handlers__[] = 
{
    onlp_ucli__fan__percentage__get,
    onlp_ucli__fan__percentage__set,
    NULL
};
static ucli_module_t onlp_ucli__fan__percentage__percentage__module__ = 
{
    "percentage",
    NULL,
    onlp_ucli__fan__percentage__percentage__handlers__,
    NULL,
    NULL
};
ucli_node_t* onlp_ucli__fan__rpm__node__ = NULL;
static ucli_command_handler_f onlp_ucli__fan__rpm__rpm__handlers__[] = 
{
    onlp_ucli__fan__rpm__get,
    onlp_ucli__fan__rpm__set,
    NULL
};
static ucli_module_t onlp_ucli__fan__rpm__rpm__module__ = 
{
    "rpm",
    NULL,
    onlp_ucli__fan__rpm__rpm__handlers__,
    NULL,
    NULL
};
ucli_node_t* onlp_ucli__debug__node__ = NULL;
ucli_node_t* onlp_ucli__debug__oid__node__ = NULL;
ucli_node_t* onlp_ucli__debug__oid__verify__node__ = NULL;
static ucli_command_handler_f onlp_ucli__debug__oid__verify__verify__handlers__[] = 
{
    onlp_ucli__debug__oid__verify__json__,
    NULL
};
static ucli_module_t onlp_ucli__debug__oid__verify__verify__module__ = 
{
    "verify",
    NULL,
    onlp_ucli__debug__oid__verify__verify__handlers__,
    NULL,
    NULL
};
ucli_node_t* onlp_ucli__debug__oid__from__node__ = NULL;
static ucli_command_handler_f onlp_ucli__debug__oid__from__from__handlers__[] = 
{
    onlp_ucli__debug__oid__from__json__,
    NULL
};
static ucli_module_t onlp_ucli__debug__oid__from__from__module__ = 
{
    "from",
    NULL,
    onlp_ucli__debug__oid__from__from__handlers__,
    NULL,
    NULL
};
ucli_node_t* onlp_ucli__debug__oid__to__node__ = NULL;
static ucli_command_handler_f onlp_ucli__debug__oid__to__to__handlers__[] = 
{
    onlp_ucli__debug__oid__to__json__,
    NULL
};
static ucli_module_t onlp_ucli__debug__oid__to__to__module__ = 
{
    "to",
    NULL,
    onlp_ucli__debug__oid__to__to__handlers__,
    NULL,
    NULL
};
static ucli_node_t* __ucli_auto_init__(void)
{
    if(onlp_ucli__node__ == NULL) onlp_ucli__node__ = ucli_node_create("onlp", NULL, NULL);
    if(onlp_ucli__oid__node__ == NULL) onlp_ucli__oid__node__ = ucli_node_create("oid", NULL, NULL);
    if(onlp_ucli__oid__hdr__node__ == NULL) onlp_ucli__oid__hdr__node__ = ucli_node_create("hdr", NULL, NULL);
    if(onlp_ucli__oid__hdr__json__node__ == NULL) onlp_ucli__oid__hdr__json__node__ = ucli_node_create("json", NULL, NULL);
    ucli_module_init(&onlp_ucli__oid__hdr__json__json__module__);
    if(onlp_ucli__sfp__node__ == NULL) onlp_ucli__sfp__node__ = ucli_node_create("sfp", NULL, NULL);
    if(onlp_ucli__sfp__dev__node__ == NULL) onlp_ucli__sfp__dev__node__ = ucli_node_create("dev", NULL, NULL);
    ucli_module_init(&onlp_ucli__sfp__dev__dev__module__);
    ucli_module_init(&onlp_ucli__sfp__sfp__module__);
    if(onlp_ucli__platform__node__ == NULL) onlp_ucli__platform__node__ = ucli_node_create("platform", NULL, NULL);
    if(onlp_ucli__platform__manager__node__ == NULL) onlp_ucli__platform__manager__node__ = ucli_node_create("manager", NULL, NULL);
    ucli_module_init(&onlp_ucli__platform__manager__manager__module__);
    if(onlp_ucli__chassis__node__ == NULL) onlp_ucli__chassis__node__ = ucli_node_create("chassis", NULL, NULL);
    if(onlp_ucli__chassis__debug__node__ == NULL) onlp_ucli__chassis__debug__node__ = ucli_node_create("debug", NULL, NULL);
    ucli_module_init(&onlp_ucli__chassis__debug__debug__module__);
    if(onlp_ucli__chassis__onie__node__ == NULL) onlp_ucli__chassis__onie__node__ = ucli_node_create("onie", NULL, NULL);
    ucli_module_init(&onlp_ucli__chassis__onie__onie__module__);
    if(onlp_ucli__chassis__asset__node__ == NULL) onlp_ucli__chassis__asset__node__ = ucli_node_create("asset", NULL, NULL);
    ucli_module_init(&onlp_ucli__chassis__asset__asset__module__);
    if(onlp_ucli__chassis__env__node__ == NULL) onlp_ucli__chassis__env__node__ = ucli_node_create("env", NULL, NULL);
    ucli_module_init(&onlp_ucli__chassis__env__env__module__);
    if(onlp_ucli__fan__node__ == NULL) onlp_ucli__fan__node__ = ucli_node_create("fan", NULL, NULL);
    if(onlp_ucli__fan__percentage__node__ == NULL) onlp_ucli__fan__percentage__node__ = ucli_node_create("percentage", NULL, NULL);
    ucli_module_init(&onlp_ucli__fan__percentage__percentage__module__);
    if(onlp_ucli__fan__rpm__node__ == NULL) onlp_ucli__fan__rpm__node__ = ucli_node_create("rpm", NULL, NULL);
    ucli_module_init(&onlp_ucli__fan__rpm__rpm__module__);
    if(onlp_ucli__debug__node__ == NULL) onlp_ucli__debug__node__ = ucli_node_create("debug", NULL, NULL);
    if(onlp_ucli__debug__oid__node__ == NULL) onlp_ucli__debug__oid__node__ = ucli_node_create("oid", NULL, NULL);
    if(onlp_ucli__debug__oid__verify__node__ == NULL) onlp_ucli__debug__oid__verify__node__ = ucli_node_create("verify", NULL, NULL);
    ucli_module_init(&onlp_ucli__debug__oid__verify__verify__module__);
    if(onlp_ucli__debug__oid__from__node__ == NULL) onlp_ucli__debug__oid__from__node__ = ucli_node_create("from", NULL, NULL);
    ucli_module_init(&onlp_ucli__debug__oid__from__from__module__);
    if(onlp_ucli__debug__oid__to__node__ == NULL) onlp_ucli__debug__oid__to__node__ = ucli_node_create("to", NULL, NULL);
    ucli_module_init(&onlp_ucli__debug__oid__to__to__module__);
    ucli_node_subnode_add(onlp_ucli__node__, onlp_ucli__oid__node__);
    ucli_node_subnode_add(onlp_ucli__oid__node__, onlp_ucli__oid__hdr__node__);
    ucli_node_subnode_add(onlp_ucli__oid__hdr__node__, onlp_ucli__oid__hdr__json__node__);
    ucli_node_module_add(onlp_ucli__oid__hdr__json__node__, &onlp_ucli__oid__hdr__json__json__module__);
    ucli_node_subnode_add(onlp_ucli__node__, onlp_ucli__sfp__node__);
    ucli_node_subnode_add(onlp_ucli__sfp__node__, onlp_ucli__sfp__dev__node__);
    ucli_node_module_add(onlp_ucli__sfp__dev__node__, &onlp_ucli__sfp__dev__dev__module__);
    ucli_node_module_add(onlp_ucli__sfp__node__, &onlp_ucli__sfp__sfp__module__);
    ucli_node_subnode_add(onlp_ucli__node__, onlp_ucli__platform__node__);
    ucli_node_subnode_add(onlp_ucli__platform__node__, onlp_ucli__platform__manager__node__);
    ucli_node_module_add(onlp_ucli__platform__manager__node__, &onlp_ucli__platform__manager__manager__module__);
    ucli_node_subnode_add(onlp_ucli__node__, onlp_ucli__chassis__node__);
    ucli_node_subnode_add(onlp_ucli__chassis__node__, onlp_ucli__chassis__debug__node__);
    ucli_node_module_add(onlp_ucli__chassis__debug__node__, &onlp_ucli__chassis__debug__debug__module__);
    ucli_node_subnode_add(onlp_ucli__chassis__node__, onlp_ucli__chassis__onie__node__);
    ucli_node_module_add(onlp_ucli__chassis__onie__node__, &onlp_ucli__chassis__onie__onie__module__);
    ucli_node_subnode_add(onlp_ucli__chassis__node__, onlp_ucli__chassis__asset__node__);
    ucli_node_module_add(onlp_ucli__chassis__asset__node__, &onlp_ucli__chassis__asset__asset__module__);
    ucli_node_subnode_add(onlp_ucli__chassis__node__, onlp_ucli__chassis__env__node__);
    ucli_node_module_add(onlp_ucli__chassis__env__node__, &onlp_ucli__chassis__env__env__module__);
    ucli_node_subnode_add(onlp_ucli__node__, onlp_ucli__fan__node__);
    ucli_node_subnode_add(onlp_ucli__fan__node__, onlp_ucli__fan__percentage__node__);
    ucli_node_module_add(onlp_ucli__fan__percentage__node__, &onlp_ucli__fan__percentage__percentage__module__);
    ucli_node_subnode_add(onlp_ucli__fan__node__, onlp_ucli__fan__rpm__node__);
    ucli_node_module_add(onlp_ucli__fan__rpm__node__, &onlp_ucli__fan__rpm__rpm__module__);
    ucli_node_subnode_add(onlp_ucli__node__, onlp_ucli__debug__node__);
    ucli_node_subnode_add(onlp_ucli__debug__node__, onlp_ucli__debug__oid__node__);
    ucli_node_subnode_add(onlp_ucli__debug__oid__node__, onlp_ucli__debug__oid__verify__node__);
    ucli_node_module_add(onlp_ucli__debug__oid__verify__node__, &onlp_ucli__debug__oid__verify__verify__module__);
    ucli_node_subnode_add(onlp_ucli__debug__oid__node__, onlp_ucli__debug__oid__from__node__);
    ucli_node_module_add(onlp_ucli__debug__oid__from__node__, &onlp_ucli__debug__oid__from__from__module__);
    ucli_node_subnode_add(onlp_ucli__debug__oid__node__, onlp_ucli__debug__oid__to__node__);
    ucli_node_module_add(onlp_ucli__debug__oid__to__node__, &onlp_ucli__debug__oid__to__to__module__);
    return onlp_ucli__node__;
}
/******************************************************************************/
/* <auto.ucli.handlers.end> */

ucli_node_t*
onlp_ucli_node_create(void)
{
    static ucli_node_t* n = NULL;
    if(n) {
        return NULL;
    }
    n = __ucli_auto_init__();
    return n;
}

#else
void*
onlp_ucli_node_create(void)
{
    return NULL;
}
#endif
