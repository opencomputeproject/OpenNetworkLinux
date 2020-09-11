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
#include <onlp/oids.h>
#include "onlp_log.h"
#include "onlp_int.h"
#include <AIM/aim.h>
#include <AIM/aim_printf.h>

#include <onlp/thermal.h>
#include <onlp/fan.h>
#include <onlp/led.h>
#include <onlp/psu.h>
#include <onlp/sys.h>
#include <onlp/module.h>

#define OID_TYPE_SHOWDUMP_DEFINE(_TYPE, _type)                          \
    static void                                                         \
    oid_type_##_TYPE##_dump__(onlp_oid_t oid, aim_pvs_t* pvs, uint32_t flags) \
    {                                                                   \
        onlp_##_type##_dump(oid, pvs, flags);                           \
    }                                                                   \
    static void                                                         \
    oid_type_##_TYPE##_show__(onlp_oid_t oid, aim_pvs_t* pvs, uint32_t flags) \
    {                                                                   \
        onlp_##_type##_show(oid, pvs, flags);                           \
    }

#define OID_TYPE_SHOWDUMP_DEFINE_EMPTY(_TYPE, _type)                    \
    static void                                                         \
    oid_type_##_TYPE##_dump__(onlp_oid_t oid, aim_pvs_t* pvs, uint32_t flags) \
    {                                                                   \
    }                                                                   \
    static void                                                         \
    oid_type_##_TYPE##_show__(onlp_oid_t oid, aim_pvs_t* pvs, uint32_t flags) \
    {                                                                   \
    }


OID_TYPE_SHOWDUMP_DEFINE(SYS, sys);
OID_TYPE_SHOWDUMP_DEFINE(THERMAL, thermal);
OID_TYPE_SHOWDUMP_DEFINE(FAN, fan);
OID_TYPE_SHOWDUMP_DEFINE(PSU, psu);
OID_TYPE_SHOWDUMP_DEFINE(LED, led);
OID_TYPE_SHOWDUMP_DEFINE(MODULE, module);
OID_TYPE_SHOWDUMP_DEFINE_EMPTY(RTC, rtc);

static void
oid_type_unknown_dump__(onlp_oid_t oid, aim_pvs_t* pvs, uint32_t flags)
{
    iof_t iof;
    onlp_oid_dump_iof_init_default(&iof, pvs);
    iof_push(&iof, "invalid oid @ 0x%x", oid);
    iof_iprintf(&iof, "type = %d", ONLP_OID_TYPE_GET(oid));
    iof_iprintf(&iof, "  id = %d", ONLP_OID_ID_GET(oid));
    iof_pop(&iof);
}

static int
oid_type_SYS_hdr_get__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    return onlp_sys_hdr_get(hdr);
}

static int
oid_type_THERMAL_hdr_get__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    return onlp_thermal_hdr_get(oid, hdr);
}

static int
oid_type_FAN_hdr_get__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    return onlp_fan_hdr_get(oid, hdr);
}

static int
oid_type_LED_hdr_get__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    return onlp_led_hdr_get(oid, hdr);
}

static int
oid_type_PSU_hdr_get__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    return onlp_psu_hdr_get(oid, hdr);
}

static int
oid_type_RTC_hdr_get__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    /* Not implemented yet */
    AIM_LOG_MSG("RTC_coids_get: 0x%x", oid);
    return ONLP_STATUS_E_INVALID;
}


static int
oid_type_MODULE_hdr_get__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    return onlp_module_hdr_get(oid, hdr);
}

static void
onlp_oid_showdump__(onlp_oid_t oid,
                    /* show=1, dump=0 */
                    int show,
                    aim_pvs_t* pvs,
                    uint32_t flags)
{
    if(oid == 0) {
        oid = ONLP_OID_SYS;
    }

    switch(ONLP_OID_TYPE_GET(oid)) {
        /* {dump || show} */
#define ONLP_OID_TYPE_ENTRY(_name, _value)                              \
        case ONLP_OID_TYPE_##_name:                                     \
            if(show) {                                                  \
                oid_type_##_name##_show__(oid, pvs, flags);             \
            }                                                           \
            else {                                                      \
                oid_type_##_name##_dump__(oid, pvs, flags);             \
            }                                                           \
            return;

#include <onlp/onlp.x>

        /* Intentional compile time error if an OID decode is missing. */
    }
    oid_type_unknown_dump__(oid, pvs, flags);
}

void
onlp_oid_dump(onlp_oid_t oid, aim_pvs_t* pvs, uint32_t flags)
{
    onlp_oid_showdump__(oid, 0, pvs, flags);
}

void
onlp_oid_table_dump(onlp_oid_table_t table, aim_pvs_t* pvs, uint32_t flags)
{
    onlp_oid_t* oidp;
    ONLP_OID_TABLE_ITER(table, oidp) {
        onlp_oid_dump(*oidp, pvs, flags);
    }
}

void
onlp_oid_show(onlp_oid_t oid, aim_pvs_t* pvs, uint32_t flags)
{
    onlp_oid_showdump__(oid, 1, pvs, flags);
}

void
onlp_oid_table_show(onlp_oid_table_t table, aim_pvs_t* pvs, uint32_t flags)
{
    onlp_oid_t* oidp;
    ONLP_OID_TABLE_ITER(table, oidp) {
        onlp_oid_show(*oidp, pvs, flags);
    }
}

int
onlp_oid_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    switch(ONLP_OID_TYPE_GET(oid))
        {
#define ONLP_OID_TYPE_ENTRY(_name, _value)                              \
            case ONLP_OID_TYPE_##_name:                                 \
                return oid_type_##_name##_hdr_get__(oid, hdr);
#include <onlp/onlp.x>
            /* Intentional compile time error if an OID handler is missing. */
        }
    return ONLP_STATUS_E_INVALID;
}

int
onlp_oid_iterate(onlp_oid_t oid, onlp_oid_type_t type,
                 onlp_oid_iterate_f itf, void* cookie)
{
    int rv;
    onlp_oid_hdr_t hdr;
    onlp_oid_t* oidp;

    if(oid == 0) {
        oid = ONLP_OID_SYS;
    }

    rv = onlp_oid_hdr_get(oid, &hdr);
    if(rv < 0) {
        return rv;
    }

    ONLP_OID_TABLE_ITER(hdr.coids, oidp) {
        if(type == 0 || ONLP_OID_IS_TYPE(type, *oidp)) {
            int rv = itf(*oidp, cookie);
            if(rv < 0) {
                return rv;
            }
            rv = onlp_oid_iterate(*oidp, type, itf, cookie);
            if(rv < 0) {
                return rv;
            }
        }
    }
    return ONLP_STATUS_OK;
}
