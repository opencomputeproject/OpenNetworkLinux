/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <onlp/platformi/ledi.h>
#include <onlplib/file.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <onlp/platformi/fani.h>

#include "platform_lib.h"

/* LED related data
 */
/* CAPS*/
#define SYS_LED_CAPS ONLP_LED_CAPS_OFF|ONLP_LED_CAPS_GREEN|ONLP_LED_CAPS_GREEN_BLINKING|  \
    ONLP_LED_CAPS_RED|ONLP_LED_CAPS_RED_BLINKING|ONLP_LED_CAPS_ORANGE
#define FAN_LED_CAPS ONLP_LED_CAPS_RED|ONLP_LED_CAPS_GREEN

#define LOCAL_ID_TO_FAN_ID(id) (id - ONLP_LED_FAN1 + 1)

typedef enum sys_led_mode_e {
    SYS_LED_MODE_OFF = 0,
    SYS_LED_MODE_0_5_HZ = 1,
    SYS_LED_MODE_1_HZ = 2,
    SYS_LED_MODE_2_HZ = 3,
    SYS_LED_MODE_4_HZ = 4,
    SYS_LED_MODE_ON = 7
} sys_led_mode_t;

/* function declarations*/
static int _sys_onlp_led_mode_convert(sys_led_mode_t grn_mode, sys_led_mode_t red_mode, onlp_led_mode_t* pmode);
static int _sys_onlp_ledi_info_get(onlp_oid_id_t id, onlp_led_info_t* info);
static int _fan_onlp_ledi_info_get(onlp_oid_id_t id, onlp_led_info_t* info);
static int _sys_onlp_ledi_mode_set(onlp_led_mode_t onlp_mode);

extern int inv_fani_status_get(onlp_oid_id_t id, uint32_t* rv);

/*
 * Get the information for the given LED OID.
 */

static onlp_led_info_t __onlp_led_info[ ] = {
    { }, /* Not used */
    ONLP_CHASSIS_LED_INFO_ENTRY_INIT(ONLP_LED_MGMT, "MGMT LED", SYS_LED_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN1, "FAN LED 1 ", ONLP_FAN_1, FAN_LED_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN2, "FAN LED 2 ", ONLP_FAN_2, FAN_LED_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN3, "FAN LED 3 ", ONLP_FAN_3, FAN_LED_CAPS),
    ONLP_FAN_LED_INFO_ENTRY_INIT(ONLP_LED_FAN4, "FAN LED 4 ", ONLP_FAN_4, FAN_LED_CAPS),
};

/* convert platform led type to onlp_led_mode type*/
static int _sys_onlp_led_mode_convert(sys_led_mode_t grn_mode, sys_led_mode_t red_mode, onlp_led_mode_t* pmode)
{
    int rv = ONLP_STATUS_OK;
    *pmode = ONLP_LED_MODE_OFF;

    switch(grn_mode) {
    case SYS_LED_MODE_0_5_HZ:
    case SYS_LED_MODE_1_HZ:
    case SYS_LED_MODE_2_HZ:
    case SYS_LED_MODE_4_HZ:
        if(red_mode==SYS_LED_MODE_OFF) {
            *pmode = ONLP_LED_MODE_GREEN_BLINKING;
        } else {
            return ONLP_STATUS_E_INVALID;
        }
        break;
    case SYS_LED_MODE_ON:
        if(red_mode==SYS_LED_MODE_ON) {
            *pmode = ONLP_LED_MODE_ORANGE;
        } else if(red_mode==SYS_LED_MODE_OFF) {
            *pmode = ONLP_LED_MODE_GREEN;
        } else {
            return ONLP_STATUS_E_INVALID;
        }
        break;
    case SYS_LED_MODE_OFF:
        if(red_mode==SYS_LED_MODE_OFF) {
            *pmode = ONLP_LED_MODE_OFF;
        } else if( (red_mode>=SYS_LED_MODE_0_5_HZ) && (red_mode<=SYS_LED_MODE_4_HZ) ) {
            *pmode =ONLP_LED_MODE_RED_BLINKING;
        } else if(red_mode==SYS_LED_MODE_ON) {
            *pmode=ONLP_LED_MODE_RED;
        }
        break;
    default:
        return ONLP_STATUS_E_INVALID;
        break;
    }
    return rv;
}


static int _sys_onlp_ledi_info_get(onlp_oid_id_t id, onlp_led_info_t* info)
{
    sys_led_mode_t grn_mode, red_mode;
    onlp_oid_hdr_t* hdr;
    *info = __onlp_led_info[id];
    hdr = &info->hdr;

    int rv;
    rv=onlp_ledi_hdr_get( id, hdr);
    if(rv!=ONLP_STATUS_OK) {
        return rv;
    }

    ONLP_TRY(onlp_file_read_int((int*)&grn_mode, INV_SYSLED_PREFIX"grn_led"));
    ONLP_TRY(onlp_file_read_int((int*)&red_mode, INV_SYSLED_PREFIX"red_led"));

    rv= _sys_onlp_led_mode_convert(grn_mode, red_mode, &info->mode);
    if(rv!=ONLP_STATUS_OK) {
        return rv;
    }

    if (info->mode != ONLP_LED_MODE_OFF) {
        hdr->status = ADD_STATE(hdr->status,ONLP_OID_STATUS_FLAG_PRESENT);
    } else {
        hdr->status = REMOVE_STATE(hdr->status,ONLP_OID_STATUS_FLAG_PRESENT);
    }

    return ONLP_STATUS_OK;
}

static int _fan_onlp_ledi_info_get(onlp_oid_id_t id, onlp_led_info_t* info)
{
    int rv = ONLP_STATUS_OK;
    int grn_mode, red_mode;
    onlp_oid_hdr_t* hdr;
    int fan_id = LOCAL_ID_TO_FAN_ID(id);

    *info = __onlp_led_info[id];
    hdr = &info->hdr;
    rv=onlp_ledi_hdr_get( id, hdr);
    if(rv!=ONLP_STATUS_OK) {
        return rv;
    }

    if(ONLP_OID_STATUS_FLAG_IS_SET(hdr,PRESENT)) {
        char* info_path=hwmon_path(INV_HWMON_BASE);
        ONLP_TRY(onlp_file_read_int((int*)&grn_mode, "%sfan_led_grn%d", info_path, fan_id));
        ONLP_TRY(onlp_file_read_int((int*)&red_mode, "%sfan_led_red%d", info_path, fan_id));
        if(grn_mode == 1 && red_mode == 0) {
            info->mode = ONLP_LED_MODE_GREEN;
        } else if(grn_mode == 0 && red_mode == 1) {
            info->mode = ONLP_LED_MODE_RED;
        } else if(grn_mode == 0 && red_mode == 0) {
            info->mode = ONLP_LED_MODE_OFF;
        } else {
            rv = ONLP_STATUS_E_INVALID;
        }
    } else {
        info->mode = ONLP_LED_MODE_INVALID;
    }
    return rv;
}

static int _sys_onlp_ledi_mode_set(onlp_led_mode_t onlp_mode)
{
    int rv = ONLP_STATUS_OK;

    if( onlp_mode == ONLP_LED_MODE_OFF) {
        rv = onlp_file_write_int(SYS_LED_MODE_OFF, INV_SYSLED_PREFIX"grn_led");
        if(rv != ONLP_STATUS_OK ) {
            return rv;
        }
        rv = onlp_file_write_int(SYS_LED_MODE_OFF, INV_SYSLED_PREFIX"red_led");
    } else {
        rv = ONLP_STATUS_E_UNSUPPORTED;
    }
    return rv;
}

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, 1, ONLP_LED_MAX-1);
}

int
onlp_ledi_info_get(onlp_oid_id_t id, onlp_led_info_t* info)
{
    int rv = ONLP_STATUS_OK;

    switch(id) {
    case ONLP_LED_MGMT:
        rv = _sys_onlp_ledi_info_get(id, info);
        break;
    case ONLP_LED_FAN1:
    case ONLP_LED_FAN2:
    case ONLP_LED_FAN3:
    case ONLP_LED_FAN4:
        rv = _fan_onlp_ledi_info_get(id, info);
        break;
    default:
        rv = ONLP_STATUS_E_INVALID;
        break;
    }
    return rv;
}

int inv_ledi_status_get(onlp_oid_id_t id, uint32_t* rv)
{
    int result = ONLP_STATUS_OK;
    onlp_led_info_t* info;
    onlp_oid_hdr_t* hdr;
    int fan_id = LOCAL_ID_TO_FAN_ID(id);
    int grn_mode, red_mode;
    onlp_led_mode_t pmode;
    uint32_t fan_status;

    info = &__onlp_led_info[id];
    hdr = &info->hdr;
    switch(id) {
    case ONLP_LED_MGMT:
        ONLP_TRY(onlp_file_read_int((int*)&grn_mode, INV_SYSLED_PREFIX"grn_led"));
        ONLP_TRY(onlp_file_read_int((int*)&red_mode, INV_SYSLED_PREFIX"red_led"));

        result = _sys_onlp_led_mode_convert(grn_mode, red_mode, &pmode);
        if(result != ONLP_STATUS_OK) {
            return result;
        }

        if( pmode != ONLP_LED_MODE_OFF) {
            hdr->status = ADD_STATE(hdr->status,ONLP_OID_STATUS_FLAG_PRESENT);
        } else {
            hdr->status = REMOVE_STATE(hdr->status,ONLP_OID_STATUS_FLAG_PRESENT);
        }

        *rv = hdr->status;
        break;
    case ONLP_LED_FAN1:
    case ONLP_LED_FAN2:
    case ONLP_LED_FAN3:
    case ONLP_LED_FAN4:
        result=inv_fani_status_get(fan_id, &fan_status);
        if(result!=ONLP_STATUS_OK) {
            return result;
        }
        if(fan_status & ONLP_OID_STATUS_FLAG_PRESENT) {
            char* info_path=hwmon_path(INV_HWMON_BASE);
            ONLP_TRY(onlp_file_read_int((int*)&grn_mode, "%sfan_led_grn%d", info_path, fan_id));
            ONLP_TRY(onlp_file_read_int((int*)&red_mode, "%sfan_led_red%d", info_path, fan_id));

            if(grn_mode == 1 || red_mode == 1) {
                hdr->status |= ONLP_OID_STATUS_FLAG_PRESENT;
            } else {
                hdr->status = 0;
            }
        } else {
            hdr->status = 0;
        }
        *rv = hdr->status;
        break;
    default:
        result = ONLP_STATUS_E_INVALID;
        break;
    }

    return result;
}

/**
 * @brief Get the LED header.
 * @param id The LED OID
 * @param rv [out] Receives the header.
 */
int onlp_ledi_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
    int result = ONLP_STATUS_OK;
    onlp_led_info_t* info;

    info = &__onlp_led_info[id];
    *hdr = info->hdr;

    result = inv_ledi_status_get(id, &hdr->status);

    return result;
}

/*
 * Turn an LED on or off.
 *
 * This function will only be called if the LED OID supports the ONOFF
 * capability.
 *
 * What 'on' means in terms of colors or modes for multimode LEDs is
 * up to the platform to decide. This is intended as baseline toggle mechanism.
 */
int
onlp_ledi_set(onlp_oid_id_t id, int on_or_off)
{
    onlp_led_mode_t mode;

    if (__onlp_led_info[id].caps & ONLP_LED_CAPS_OFF) {
        mode = on_or_off?ONLP_LED_MODE_AUTO:ONLP_LED_MODE_OFF;
        return onlp_ledi_mode_set(id, mode);
    }

    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * This function puts the LED into the given mode. It is a more functional
 * interface for multimode LEDs.
 *
 * Only modes reported in the LED's capabilities will be attempted.
 */
int
onlp_ledi_mode_set(onlp_oid_id_t id, onlp_led_mode_t mode)
{
    int rv = ONLP_STATUS_OK;

    switch(id) {
    case ONLP_LED_MGMT:
        rv = _sys_onlp_ledi_mode_set(mode);
        break;
    case ONLP_LED_FAN1:
    case ONLP_LED_FAN2:
    case ONLP_LED_FAN3:
    case ONLP_LED_FAN4:
        rv = ONLP_STATUS_E_UNSUPPORTED;
        break;
    default:
        rv = ONLP_STATUS_E_INVALID;
        break;
    }

    return rv;
}

/*
 * Generic LED ioctl interface.
 */
int
onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

