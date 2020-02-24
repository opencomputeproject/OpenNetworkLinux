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
 * Fan Management.
 *
 ***********************************************************/
#include <onlp/fan.h>
#include <onlp/platformi/fani.h>
#include <onlp/oids.h>
#include "onlp_int.h"
#include "onlp_locks.h"
#include "onlp_log.h"
#include "onlp_json.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

#define VALIDATENR(_id)                         \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return;                             \
        }                                       \
    } while(0)


static int
onlp_fan_init_locked__(void)
{
    return onlp_fani_init();
}
ONLP_LOCKED_API0(onlp_fan_init)


#if ONLP_CONFIG_INCLUDE_PLATFORM_OVERRIDES == 1

static int
onlp_fani_info_from_json__(cJSON* data, onlp_fan_info_t* fip, int errorcheck)
{
    int rv;

    if(data == NULL) {
        return (errorcheck) ? ONLP_STATUS_E_PARAM : 0;
    }

    rv = cjson_util_lookup_int(data, (int*) &fip->status, "status");
    if(rv < 0 && errorcheck) return rv;

    rv = cjson_util_lookup_int(data, (int*) &fip->caps, "caps");
    if(rv < 0 && errorcheck) return rv;

    rv = cjson_util_lookup_int(data, (int*) &fip->rpm, "rpm");
    if(rv < 0 && errorcheck) return rv;

    rv = cjson_util_lookup_int(data, (int*) &fip->percentage, "percentage");
    if(rv < 0 && errorcheck) return rv;

    rv = cjson_util_lookup_int(data, (int*) &fip->mode, "mode");
    if(rv < 0 && errorcheck) return rv;

    return 0;
}

#endif

static int
onlp_fan_info_get_locked__(onlp_oid_t oid, onlp_fan_info_t* fip)
{
    int rv;

    VALIDATE(oid);

    /* Get the information struct from the platform */
    rv = onlp_fani_info_get(oid, fip);

    if(rv >= 0) {

#if ONLP_CONFIG_INCLUDE_PLATFORM_OVERRIDES == 1
        /*
         * Optional override from the config file.
         * This is usually just for testing.
         */
        int id = ONLP_OID_ID_GET(oid);
        cJSON* entry = NULL;


        cjson_util_lookup(onlp_json_get(0), &entry, "overrides.fan.%d", id);
        onlp_fani_info_from_json__(entry, fip, 0);
#endif

        if(fip->percentage && fip->rpm == 0) {
            /* Approximate RPM based on a 10,000 RPM Maximum */
            fip->rpm = fip->percentage * 100;
        }
    }

    return rv;
}
ONLP_LOCKED_API2(onlp_fan_info_get, onlp_oid_t, oid, onlp_fan_info_t*, fip);

static int
onlp_fan_status_get_locked__(onlp_oid_t oid, uint32_t* status)
{
    int rv = onlp_fani_status_get(oid, status);
    if(ONLP_SUCCESS(rv)) {
        return rv;
    }
    if(ONLP_UNSUPPORTED(rv)) {
        onlp_fan_info_t fi;
        rv = onlp_fani_info_get(oid, &fi);
        *status = fi.status;
    }
    return rv;
}
ONLP_LOCKED_API2(onlp_fan_status_get, onlp_oid_t, oid, uint32_t*, status);

static int
onlp_fan_hdr_get_locked__(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    int rv = onlp_fani_hdr_get(oid, hdr);
    if(ONLP_SUCCESS(rv)) {
        return rv;
    }
    if(ONLP_UNSUPPORTED(rv)) {
        onlp_fan_info_t fi;
        rv = onlp_fani_info_get(oid, &fi);
        memcpy(hdr, &fi.hdr, sizeof(fi.hdr));
    }
    return rv;
}
ONLP_LOCKED_API2(onlp_fan_hdr_get, onlp_oid_t, oid, onlp_oid_hdr_t*, hdr);

static int
onlp_fan_present__(onlp_oid_t id, onlp_fan_info_t* info)
{
    int rv;
    VALIDATE(id);

    /* Info retrieval required. */
    rv = onlp_fani_info_get(id, info);
    if(rv < 0) {
        return rv;
    }
    /* The fan must be present. */
    if((info->status & 0x1) == 0) {
        return ONLP_STATUS_E_MISSING;
    }
    return ONLP_STATUS_OK;
}
#define ONLP_FAN_PRESENT_OR_RETURN(_id, _info)          \
    do {                                                \
        int _rv = onlp_fan_present__(_id, _info);       \
        if(_rv < 0) {                                   \
            return _rv;                                 \
        }                                               \
    } while(0)


static int
onlp_fan_rpm_set_locked__(onlp_oid_t id, int rpm)
{
    onlp_fan_info_t info;
    ONLP_FAN_PRESENT_OR_RETURN(id, &info);
    if(info.caps & ONLP_FAN_CAPS_SET_RPM) {
        return onlp_fani_rpm_set(id, rpm);
    }
    else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}
ONLP_LOCKED_API2(onlp_fan_rpm_set, onlp_oid_t, id, int, rpm);

static int
onlp_fan_percentage_set_locked__(onlp_oid_t id, int p)
{
    onlp_fan_info_t info;
    ONLP_FAN_PRESENT_OR_RETURN(id, &info);
    if(info.caps & ONLP_FAN_CAPS_SET_PERCENTAGE) {
        return onlp_fani_percentage_set(id, p);
    }
    else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}
ONLP_LOCKED_API2(onlp_fan_percentage_set, onlp_oid_t, id, int, p);

static int
onlp_fan_mode_set_locked__(onlp_oid_t id, onlp_fan_mode_t mode)
{
    onlp_fan_info_t info;
    ONLP_FAN_PRESENT_OR_RETURN(id, &info);
    return onlp_fani_mode_set(id, mode);
}
ONLP_LOCKED_API2(onlp_fan_mode_set, onlp_oid_t, id, onlp_fan_mode_t, mode);

static int
onlp_fan_dir_set_locked__(onlp_oid_t id, onlp_fan_dir_t dir)
{
    onlp_fan_info_t info;
    ONLP_FAN_PRESENT_OR_RETURN(id, &info);
    if( (info.caps & ONLP_FAN_CAPS_B2F) &&
        (info.caps & ONLP_FAN_CAPS_F2B) ) {
        return onlp_fani_dir_set(id, dir);
    }
    else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}
ONLP_LOCKED_API2(onlp_fan_dir_set, onlp_oid_t, id, onlp_fan_dir_t, dir);


/************************************************************
 *
 * Debug and Show Functions
 *
 ***********************************************************/

void
onlp_fan_dump(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags)
{
    int rv;
    iof_t iof;
    onlp_fan_info_t info;

    VALIDATENR(id);

    onlp_oid_dump_iof_init_default(&iof, pvs);
    iof_push(&iof, "fan @ %d", ONLP_OID_ID_GET(id));
    rv = onlp_fan_info_get(id, &info);
    if(rv < 0) {
        onlp_oid_info_get_error(&iof, rv);
    }
    else {
        onlp_oid_show_description(&iof, &info.hdr);
        if(info.status & 1) {
            /* Present */
            iof_iprintf(&iof, "Status: %{onlp_fan_status_flags}", info.status);
            iof_iprintf(&iof, "Caps:   %{onlp_fan_caps_flags}", info.caps);
            iof_iprintf(&iof, "RPM:    %d", info.rpm);
            iof_iprintf(&iof, "Per:    %d", info.percentage);
            iof_iprintf(&iof, "Model:  %s", info.model[0] ? info.model : "NULL");
            iof_iprintf(&iof, "SN:     %s", info.serial[0] ? info.serial : "NULL");
        }
        else {
            iof_iprintf(&iof, "Not present.");
        }
    }
    iof_pop(&iof);
}

void
onlp_fan_show(onlp_oid_t oid, aim_pvs_t* pvs, uint32_t flags)
{
    int rv;
    iof_t iof;
    onlp_fan_info_t fi;
    int yaml;

    onlp_oid_show_iof_init_default(&iof, pvs, flags);

    rv = onlp_fan_info_get(oid, &fi);

    yaml = flags & ONLP_OID_SHOW_YAML;

    if(yaml) {
        iof_push(&iof, "- ");
        iof_iprintf(&iof, "Name: Fan %d", ONLP_OID_ID_GET(oid));
    }
    else {
        iof_push(&iof, "Fan %d", ONLP_OID_ID_GET(oid));
    }

    if(rv < 0) {
        if(yaml) {
            iof_iprintf(&iof, "State: Error");
            iof_iprintf(&iof, "Error: %{onlp_status}", rv);
        } else {
            onlp_oid_info_get_error(&iof, rv);
        }
    }
    else {
        onlp_oid_show_description(&iof, &fi.hdr);
        if(fi.status & 0x1) {
            /* Present */
            iof_iprintf(&iof, "State: Present");
            if(fi.status & ONLP_FAN_STATUS_FAILED) {
                iof_iprintf(&iof, "Status: Failed");
            }
            else {
                iof_iprintf(&iof, "Status: Running");
                if(fi.model[0]) {
                    iof_iprintf(&iof, "Model: %{pstr}", fi.model, '?');
                }
                if(fi.serial[0]) {
                    iof_iprintf(&iof, "SN: %{pstr}", fi.serial, '?');
                }
                if(fi.caps & ONLP_FAN_CAPS_GET_RPM) {
                    iof_iprintf(&iof, "RPM: %d", fi.rpm);
                }
                if(fi.caps & ONLP_FAN_CAPS_GET_PERCENTAGE) {
                    iof_iprintf(&iof, "Speed: %d%%", fi.percentage);
                }
                if(fi.status & ONLP_FAN_STATUS_B2F) {
                    iof_iprintf(&iof, "Airflow: Back-to-Front");
                }
                if(fi.status & ONLP_FAN_STATUS_F2B) {
                    iof_iprintf(&iof, "Airflow: Front-to-Back");
                }
            }
        }
        else {
            /* Not present */
            onlp_oid_show_state_missing(&iof);
        }
    }
    iof_pop(&iof);
}
