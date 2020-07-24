/************************************************************
 * ledi.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
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

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/* LED related data
 */

/* CAPS*/
#define SYS_LED_CAPS ONLP_LED_CAPS_ON_OFF|ONLP_LED_CAPS_GREEN|ONLP_LED_CAPS_GREEN_BLINKING|  \
    ONLP_LED_CAPS_RED|ONLP_LED_CAPS_RED_BLINKING|ONLP_LED_CAPS_ORANGE
#define FAN_LED_CAPS ONLP_LED_CAPS_RED|ONLP_LED_CAPS_GREEN

#define LED_ID_TO_FAN_ID(id) (id-ONLP_LED_SYS)
#define FAN_TO_BLADE_ID(fan_id) fan_id*2

typedef enum sys_led_mode_e {
    SYS_LED_MODE_OFF = 0,
    SYS_LED_MODE_ON = 1,
    SYS_LED_MODE_1_HZ = 2,
    SYS_LED_MODE_2_HZ = 3
} sys_led_mode_t;

static char* __led_path_list[ONLP_LED_MAX] = {
    "reserved",
    INV_LED_PREFIX"/stacking_led",
    INV_LED_PREFIX"/fan_led",
    INV_LED_PREFIX"/power_led",
    INV_LED_PREFIX"/service_led",
    INV_LED_PREFIX"/fanmodule1_led",
    INV_LED_PREFIX"/fanmodule2_led",
    INV_LED_PREFIX"/fanmodule3_led",
    INV_LED_PREFIX"/fanmodule4_led",
    INV_LED_PREFIX"/fanmodule5_led",
};

/*
 * Get the information for the given LED OID.
 */

#define MAKE_LED_INFO_NODE_ON_FAN(fan_id)	\
    {						\
        { ONLP_LED_ID_CREATE(ONLP_LED_FAN##fan_id), \
          "FAN LED "#fan_id, 0,			\
        },					\
        0,	FAN_LED_CAPS,					\
    }

static onlp_led_info_t __onlp_led_info[] = {
    {},/* Not used */
    {
        { ONLP_LED_ID_CREATE(ONLP_LED_STK), "Chassis LED (STACKING LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
        ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(ONLP_LED_FAN), "Chassis LED (FAN LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING ,
        ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(ONLP_LED_PWR), "Chassis LED (POWER LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
        ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(ONLP_LED_SYS), "Chassis LED (SYSTEM LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_BLUE| ONLP_LED_CAPS_BLUE_BLINKING,
        ONLP_LED_MODE_ON, '0',
    },
    MAKE_LED_INFO_NODE_ON_FAN(1),
    MAKE_LED_INFO_NODE_ON_FAN(2),
    MAKE_LED_INFO_NODE_ON_FAN(3),
    MAKE_LED_INFO_NODE_ON_FAN(4),
    MAKE_LED_INFO_NODE_ON_FAN(5),
};


/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    return ONLP_STATUS_OK;
}


int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int  led_id = ONLP_OID_ID_GET(id);
    int fan_id=LED_ID_TO_FAN_ID(led_id);
    int  len,led_state;
    char buf[ONLP_CONFIG_INFO_STR_MAX] = {0};
    uint32_t fan_status;
    int rv;

    VALIDATE(id);

    memset(info, 0, sizeof(onlp_led_info_t));
    *info = __onlp_led_info[led_id];

    switch (led_id) {
    case ONLP_LED_STK:
        if( onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, __led_path_list[led_id]) == ONLP_STATUS_OK ) {
            led_state=(int)(buf[0]-'0');
            if(led_state==SYS_LED_MODE_OFF) {
                info->mode = ONLP_LED_MODE_OFF;
            } else if(led_state==SYS_LED_MODE_ON) {
                info->mode = ONLP_LED_MODE_ON;
            } else if(led_state==SYS_LED_MODE_1_HZ) {
                info->mode =ONLP_LED_MODE_GREEN_BLINKING;
            } else {
                AIM_LOG_ERROR("[ONLP][LED] Not defined LED behavior detected on led@%d\n", led_id);
                return ONLP_STATUS_E_INTERNAL;
            }
        }
        info->status &= ONLP_LED_STATUS_PRESENT;
        return ONLP_STATUS_OK;
        break;
    case ONLP_LED_SYS:
        if( onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, __led_path_list[led_id]) == ONLP_STATUS_OK ) {
            led_state=(int)(buf[0]-'0');
            if(led_state==SYS_LED_MODE_OFF) {
                info->mode = ONLP_LED_MODE_OFF;
            } else if(led_state==SYS_LED_MODE_ON) {
                info->mode = ONLP_LED_MODE_ON;
            } else if(led_state==SYS_LED_MODE_1_HZ) {
                info->mode =ONLP_LED_MODE_BLUE_BLINKING;
            } else {
                AIM_LOG_ERROR("[ONLP][LED] Not defined LED behavior detected on led@%d\n", led_id);
                return ONLP_STATUS_E_INTERNAL;
            }
        }
        info->status &= ONLP_LED_STATUS_PRESENT;
        return ONLP_STATUS_OK;
        break;
    case ONLP_LED_FAN:
    case ONLP_LED_PWR:
        if( onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, __led_path_list[led_id]) == ONLP_STATUS_OK ) {
            led_state=(int)(buf[0]-'0');
            if(led_state==SYS_LED_MODE_OFF) {
                info->mode = ONLP_LED_MODE_OFF;
            } else if(led_state==SYS_LED_MODE_ON) {
                info->mode = ONLP_LED_MODE_ON;
            } else if( (led_state==SYS_LED_MODE_1_HZ)||(led_state==SYS_LED_MODE_2_HZ) ) {
                info->mode =ONLP_LED_MODE_GREEN_BLINKING;
            } else {
                AIM_LOG_ERROR("[ONLP][LED] Not defined LED behavior detected on led@%d\n", led_id);
                return ONLP_STATUS_E_INTERNAL;
            }
        }
        info->status &= ONLP_LED_STATUS_PRESENT;
        return ONLP_STATUS_OK;
        break;
    case ONLP_LED_FAN1:
    case ONLP_LED_FAN2:
    case ONLP_LED_FAN3:
    case ONLP_LED_FAN4:
    case ONLP_LED_FAN5:
        rv = onlp_fani_status_get(ONLP_FAN_ID_CREATE(FAN_TO_BLADE_ID(fan_id)), &fan_status);
        if(rv==ONLP_STATUS_OK) {
            rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, INV_LED_PREFIX"fanmodule%d_led", fan_id);
        } else {
            AIM_LOG_ERROR("[ONLP][LED] Read error in line %s\n",__LINE__);
        }

        if(rv == ONLP_STATUS_OK ) {
            if(fan_status & ONLP_FAN_STATUS_PRESENT) {
                info->status = ADD_STATE(info->status,ONLP_LED_STATUS_PRESENT);
                switch(buf[0]) {
                case '0':
                    info->mode = ONLP_LED_MODE_OFF;
                    break;
                case '1':
                    info->mode = ONLP_LED_MODE_GREEN;
                    break;
                case '2':
                    info->mode = ONLP_LED_MODE_RED;
                    break;
                default:
                    rv=ONLP_STATUS_E_INVALID;
                    break;
                }
            } else {
                info->status = 0 ;
            }
        }
        break;
    default:
        return ONLP_STATUS_E_INVALID;
        break;
    }
    return ONLP_STATUS_OK;
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
onlp_ledi_set(onlp_oid_t id, int on_or_off)
{
    /*the led behavior is handled by another driver on D6332. Therefore, we're not supporting this attribute*/
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * This function puts the LED into the given mode. It is a more functional
 * interface for multimode LEDs.
 *
 * Only modes reported in the LED's capabilities will be attempted.
 */
int
onlp_ledi_mode_set(onlp_oid_t id, onlp_led_mode_t mode)
{
    /*the led behavior is handled by another driver on D6332. Therefore, we're not supporting this attribute*/
    return ONLP_STATUS_E_UNSUPPORTED;
}
