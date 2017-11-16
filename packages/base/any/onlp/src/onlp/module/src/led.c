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
 * LED Management
 *
 ***********************************************************/
#include <onlp/onlp.h>
#include <onlp/oids.h>
#include <onlp/led.h>
#include <onlp/platformi/ledi.h>
#include "onlp_int.h"
#include "onlp_locks.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

#define VALIDATENR(_id)                         \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return;                             \
        }                                       \
    } while(0)


static int
onlp_led_present__(onlp_oid_t id, onlp_led_info_t* info)
{
    int rv;
    VALIDATE(id);

    /* Info retrieval required. */
    rv = onlp_ledi_info_get(id, info);
    if(rv < 0) {
        return rv;
    }
    /* The led must be present. */
    if((info->status & 0x1) == 0) {
        return ONLP_STATUS_E_MISSING;
    }
    return ONLP_STATUS_OK;
}
#define ONLP_LED_PRESENT_OR_RETURN(_id, _info)          \
    do {                                                \
        int _rv = onlp_led_present__(_id, _info);       \
        if(_rv < 0) {                                   \
            return _rv;                                 \
        }                                               \
    } while(0)

static int
onlp_led_init_locked__(void)
{
    return onlp_ledi_init();
}
ONLP_LOCKED_API0(onlp_led_init);

static int
onlp_led_info_get_locked__(onlp_oid_t id, onlp_led_info_t* info)
{
    VALIDATE(id);
    return onlp_ledi_info_get(id, info);
}
ONLP_LOCKED_API2(onlp_led_info_get, onlp_oid_t, id, onlp_led_info_t*, info);

static int
onlp_led_status_get_locked__(onlp_oid_t id, uint32_t* status)
{
    int rv = onlp_ledi_status_get(id, status);
    if(ONLP_SUCCESS(rv)) {
        return rv;
    }
    if(ONLP_UNSUPPORTED(rv)) {
        onlp_led_info_t li;
        rv = onlp_ledi_info_get(id, &li);
        *status = li.status;
    }
    return rv;
}
ONLP_LOCKED_API2(onlp_led_status_get, onlp_oid_t, id, uint32_t*, status);

static int
onlp_led_hdr_get_locked__(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    int rv = onlp_ledi_hdr_get(id, hdr);
    if(ONLP_SUCCESS(rv)) {
        return rv;
    }
    if(ONLP_UNSUPPORTED(rv)) {
        onlp_led_info_t li;
        rv = onlp_ledi_info_get(id, &li);
        memcpy(hdr, &li.hdr, sizeof(li.hdr));
    }
    return rv;
}
ONLP_LOCKED_API2(onlp_led_hdr_get, onlp_oid_t, id, onlp_oid_hdr_t*, hdr);

static int
onlp_led_set_locked__(onlp_oid_t id, int on_or_off)
{
    onlp_led_info_t info;
    ONLP_LED_PRESENT_OR_RETURN(id, &info);
    if(info.caps & ONLP_LED_CAPS_ON_OFF) {
        return onlp_ledi_set(id, on_or_off);
    }
    else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}
ONLP_LOCKED_API2(onlp_led_set, onlp_oid_t, id, int, on_or_off);

static int
onlp_led_mode_set_locked__(onlp_oid_t id, onlp_led_mode_t mode)
{
    onlp_led_info_t info;
    ONLP_LED_PRESENT_OR_RETURN(id, &info);

    /*
     * The mode enumeration values always match
     * the capability bit positions.
     */
    if(info.caps & (1 << mode)) {
        return onlp_ledi_mode_set(id, mode);
    }
    else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}
ONLP_LOCKED_API2(onlp_led_mode_set, onlp_oid_t, id, onlp_led_mode_t, mode);

static int
onlp_led_char_set_locked__(onlp_oid_t id, char c)
{
    onlp_led_info_t info;
    ONLP_LED_PRESENT_OR_RETURN(id, &info);

    /*
     * The mode enumeration values always match
     * the capability bit positions.
     */
    if(info.caps & ONLP_LED_CAPS_CHAR) {
        return onlp_ledi_char_set(id, c);
    }
    else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
}
ONLP_LOCKED_API2(onlp_led_char_set, onlp_oid_t, id, char, c);

/************************************************************
 *
 * Debug and Show Functions
 *
 ***********************************************************/

void
onlp_led_dump(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags)
{
    int rv;
    iof_t iof;
    onlp_led_info_t info;

    VALIDATENR(id);
    onlp_oid_dump_iof_init_default(&iof, pvs);
    iof_push(&iof, "led @ %d", ONLP_OID_ID_GET(id));
    rv = onlp_led_info_get(id, &info);
    if(rv < 0) {
        onlp_oid_info_get_error(&iof, rv);
    }
    else {
        onlp_oid_show_description(&iof, &info.hdr);
        if(info.status & 1) {
            /* Present */
            iof_iprintf(&iof, "Status: %{onlp_led_status_flags}", info.status);
            iof_iprintf(&iof, "Caps:   %{onlp_led_caps_flags}", info.caps);
            iof_iprintf(&iof, "Mode: %{onlp_led_mode}", info.mode);
            iof_iprintf(&iof, "Char: %c", info.character);
        }
        else {
            iof_iprintf(&iof, "Not present.");
        }
    }
    iof_pop(&iof);
}

void
onlp_led_show(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags)
{
    int rv;
    iof_t iof;
    onlp_led_info_t info;
    int yaml;

    VALIDATENR(id);
    onlp_oid_show_iof_init_default(&iof, pvs, flags);

    yaml = flags & ONLP_OID_SHOW_YAML;

    if(yaml) {
        iof_push(&iof, " -");
        iof_iprintf(&iof, "Name: LED %d", ONLP_OID_ID_GET(id));
    }
    else {
        iof_push(&iof, "LED %d", ONLP_OID_ID_GET(id));
    }

    rv = onlp_led_info_get(id, &info);
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
        onlp_oid_show_description(&iof, &info.hdr);
        if(info.status & 1) {
            /* Present */
            iof_iprintf(&iof, "State: Present");
            iof_iprintf(&iof, "Mode: %{onlp_led_mode}", info.mode);
            if(info.caps & ONLP_LED_CAPS_CHAR) {
                iof_iprintf(&iof, "Char: %c", info.character);
            }
        }
        else {
            onlp_oid_show_state_missing(&iof);
        }
    }
    iof_pop(&iof);

}

