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
 * Thermal Sensor Management.
 *
 ************************************************************/
#include <onlp/thermal.h>
#include <onlp/platformi/thermali.h>
#include <onlp/oids.h>
#include "onlp_int.h"
#include "onlp_locks.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

#define VALIDATENR(_id)                         \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return;                             \
        }                                       \
    } while(0)


static int
onlp_thermal_init_locked__(void)
{
    return onlp_thermali_init();
}
ONLP_LOCKED_API0(onlp_thermal_init);

#if ONLP_CONFIG_INCLUDE_PLATFORM_OVERRIDES == 1

static int
onlp_thermali_info_from_json__(cJSON* data, onlp_thermal_info_t* info, int errorcheck)
{
    int rv;
    int t;

    if(data == NULL) {
        return (errorcheck) ? ONLP_STATUS_E_PARAM : 0;
    }

    rv = cjson_util_lookup_int(data, (int*) &info->status, "status");
    if(rv < 0 && errorcheck) return rv;

    rv = cjson_util_lookup_int(data, &t, "mcelsius");
    if(rv < 0 && errorcheck) return rv;
    info->mcelsius = t;

    return 0;
}

#endif

static int
onlp_thermal_info_get_locked__(onlp_oid_t oid, onlp_thermal_info_t* info)
{
    int rv;
    VALIDATE(oid);

    rv = onlp_thermali_info_get(oid, info);
    if(rv >= 0) {

#if ONLP_CONFIG_INCLUDE_PLATFORM_OVERRIDES == 1
        int id = ONLP_OID_ID_GET(oid);
        cJSON* entry = NULL;

        cjson_util_lookup(onlp_json_get(0), &entry, "overrides.thermal.%d", id);
        onlp_thermali_info_from_json__(entry, info, 0);
#endif

    }
    return rv;
}
ONLP_LOCKED_API2(onlp_thermal_info_get, onlp_oid_t, oid, onlp_thermal_info_t*, info);

static int
onlp_thermal_status_get_locked__(onlp_oid_t id, uint32_t* status)
{
    int rv = onlp_thermali_status_get(id, status);
    if(ONLP_SUCCESS(rv)) {
        return rv;
    }
    if(ONLP_UNSUPPORTED(rv)) {
        onlp_thermal_info_t ti;
        rv = onlp_thermali_info_get(id, &ti);
        *status = ti.status;
    }
    return rv;
}
ONLP_LOCKED_API2(onlp_thermal_status_get, onlp_oid_t, id, uint32_t*, status);

static int
onlp_thermal_hdr_get_locked__(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    int rv = onlp_thermali_hdr_get(id, hdr);
    if(ONLP_SUCCESS(rv)) {
        return rv;
    }
    if(ONLP_UNSUPPORTED(rv)) {
        onlp_thermal_info_t ti;
        rv = onlp_thermali_info_get(id, &ti);
        memcpy(hdr, &ti.hdr, sizeof(ti.hdr));
    }
    return rv;
}
ONLP_LOCKED_API2(onlp_thermal_hdr_get, onlp_oid_t, id, onlp_oid_hdr_t*, hdr);
int
onlp_thermal_ioctl(int code, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, code);
    rv = onlp_thermal_vioctl(code, vargs);
    va_end(vargs);
    return rv;
}

static int
onlp_thermal_vioctl_locked__(int code, va_list vargs)
{
    return onlp_thermali_ioctl(code, vargs);
}
ONLP_LOCKED_API2(onlp_thermal_vioctl, int, code, va_list, vargs);


/************************************************************
 *
 * Debug and Show Functions
 *
 ***********************************************************/
void
onlp_thermal_dump(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags)
{
    int rv;
    iof_t iof;
    onlp_thermal_info_t info;

    VALIDATENR(id);
    onlp_oid_dump_iof_init_default(&iof, pvs);

    iof_push(&iof, "thermal @ %d", ONLP_OID_ID_GET(id));
    rv = onlp_thermal_info_get(id, &info);
    if(rv < 0) {
        onlp_oid_info_get_error(&iof, rv);
    }
    else {
        onlp_oid_show_description(&iof, &info.hdr);
        if(info.status & 1) {
            /* Present */
            iof_iprintf(&iof, "Status: %{onlp_thermal_status_flags}", info.status);
            iof_iprintf(&iof, "Caps:   %{onlp_thermal_caps_flags}", info.caps);
            iof_iprintf(&iof, "Temperature: %d", info.mcelsius);
            iof_push(&iof, "thresholds");
            {
                iof_iprintf(&iof, "Warning: %d", info.thresholds.warning);
                iof_iprintf(&iof, "Error: %d", info.thresholds.error);
                iof_iprintf(&iof, "Shutdown: %d", info.thresholds.shutdown);
                iof_pop(&iof);
            }
        }
        else {
            iof_iprintf(&iof, "Not present.");
        }
    }
    iof_pop(&iof);
}

void
onlp_thermal_show(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags)
{
    int rv;
    iof_t iof;
    onlp_thermal_info_t ti;
    VALIDATENR(id);
    int yaml;

    onlp_oid_show_iof_init_default(&iof, pvs, flags);


    rv = onlp_thermal_info_get(id, &ti);

    yaml = flags & ONLP_OID_SHOW_F_YAML;

    if(yaml) {
        iof_push(&iof, "- ");
        iof_iprintf(&iof, "Name: Thermal %d", ONLP_OID_ID_GET(id));
    }
    else {
        iof_push(&iof, "Thermal %d", ONLP_OID_ID_GET(id));
    }

    if(rv < 0) {
        if(yaml) {
            iof_iprintf(&iof, "State: Error");
            iof_iprintf(&iof, "Error: %{onlp_status}", rv);
        }
        else {
            onlp_oid_info_get_error(&iof, rv);
        }
    }
    else {
        onlp_oid_show_description(&iof, &ti.hdr);
        if(ti.status & 0x1) {
            /* Present */
            if(ti.status & ONLP_THERMAL_STATUS_FAILED) {
                iof_iprintf(&iof, "Status: Failed");
            }
            else {
                iof_iprintf(&iof, "Status: Functional");
                if(ti.caps & ONLP_THERMAL_CAPS_GET_TEMPERATURE) {
                    iof_iprintf(&iof, "Temperature: %d.%d C",
                                ONLP_MILLI_NORMAL_INTEGER_TENTHS(ti.mcelsius));
                }
#if ONLP_CONFIG_INCLUDE_THERMAL_THRESHOLDS == 1

                if(ti.caps & ONLP_THERMAL_CAPS_GET_ANY_THRESHOLD) {
                    iof_push(&iof, "Thresholds:");
                    if(ti.caps & ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD) {
                        iof_iprintf(&iof, "Warning : %d.%d C",
                                    ONLP_MILLI_NORMAL_INTEGER_TENTHS(ti.thresholds.warning));
                    }
                    if(ti.caps & ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD) {
                        iof_iprintf(&iof, "Error   : %d.%d C",
                                    ONLP_MILLI_NORMAL_INTEGER_TENTHS(ti.thresholds.error));
                    }
                    if(ti.caps & ONLP_THERMAL_CAPS_GET_SHUTDOWN_THRESHOLD) {
                        iof_iprintf(&iof, "Shutdown: %d.%d C",
                                    ONLP_MILLI_NORMAL_INTEGER_TENTHS(ti.thresholds.shutdown));
                    }
                    iof_pop(&iof);
                }
#endif
            }
        }
        else {
            /* Not present */
            onlp_oid_show_state_missing(&iof);
        }
    }
    iof_pop(&iof);
}
