#include <onlp/platformi/ledi.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "platform.h"

#if 0
enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_SYSTEM,
    LED_ALARM,
    LED_PSU,
    LED_FAN,
    LED_FAN1,
    LED_FAN2,
    LED_FAN3,
};
#endif

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t led_info[] =
{
    { },
    {
        { ONLP_LED_ID_CREATE(LED_SYSTEM_ID), "System LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_YELLOW_BLINKING | ONLP_LED_CAPS_GREEN |
        ONLP_LED_CAPS_GREEN_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_ALARM_ID), "Alarm LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_YELLOW_BLINKING | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU_ID), "PSU LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_AUTO | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_ID), "FAN LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN1_ID), "Chassis FAN1 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW |  ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN2_ID), "Chassis FAN2 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW |  ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN3_ID), "Chassis FAN3 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW |  ONLP_LED_CAPS_GREEN,
    }
};

int
onlp_ledi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info_p)
{
    int led_id;
    uint8_t led_color = 0;
    uint8_t ctrl_status = 0;
    uint8_t blink_status = 0;
    uint8_t result = 0, i = 0;

    led_id = ONLP_OID_ID_GET(id);
    *info_p = led_info[led_id];

    result = get_led_status(led_id);

    if(result != 0xFF)
        info_p->status |= ONLP_LED_STATUS_ON;

    switch(led_id){
        case LED_SYSTEM_ID:
            led_color = (result >> 4)&0x3;
            blink_status = result & 0x3;
            info_p->mode = ONLP_LED_MODE_OFF;
            if(led_color == 0 && (blink_status == 2 || blink_status == 1)){
                info_p->mode |= ONLP_LED_MODE_BLINKING;
                break;
            }
            if(led_color == 1){
                info_p->mode |= ONLP_LED_MODE_YELLOW;
            }
            if(led_color == 2){
                info_p->mode |= ONLP_LED_MODE_GREEN;
            }
            if(led_color == 3 || blink_status == 3){
                info_p->mode = ONLP_LED_MODE_OFF;
                break;
            }

            if(blink_status == 1 || blink_status == 2){
                int current_mode = info_p->mode;
                info_p->mode = current_mode+1;
            }
            break;
        case LED_ALARM_ID:
            led_color = (result >> 4)&0x3;
            blink_status = result & 0x3;
            if(led_color == 0 || led_color == 3){
                info_p->mode = ONLP_LED_MODE_OFF;
                break;
            }
            if(led_color == 1 && (blink_status == 0 || blink_status == 3)){
                info_p->mode |= ONLP_LED_MODE_GREEN;
            }
            if(led_color == 2){
                if(blink_status == 0){
                    info_p->mode |= ONLP_LED_MODE_YELLOW;
		}
                else if(blink_status == 1 || blink_status == 2){
                    info_p->mode |= ONLP_LED_MODE_YELLOW_BLINKING;
		}
            }

            break;
        case LED_PSU_ID:
           ctrl_status = (result >> 4) & 0x1;
           led_color = result & 0x3;
            if(ctrl_status){
                info_p->mode |= ONLP_LED_MODE_AUTO;
                break;
            }
            if(led_color == 0x02){
                info_p->mode |= ONLP_LED_MODE_GREEN;
            }else if(led_color == 0x01){
                info_p->mode |= ONLP_LED_MODE_YELLOW;
            }else if(led_color == 0x00){
                info_p->mode = ONLP_LED_MODE_OFF;
            }
            break;
        case LED_FAN_ID:
            /* front panel fan LED is automatically control by 3 fans LED */
            info_p->mode = ONLP_LED_MODE_GREEN;
            for(i = 0; i < CHASSIS_FAN_COUNT; i++){
                result = get_led_status(++led_id);
                led_color = result & 0x3;
                if(led_color == 0 || led_color == 3){
                    info_p->mode = ONLP_LED_MODE_OFF;
                }
                else if(led_color == 1 && info_p->mode != ONLP_LED_MODE_OFF){
                    info_p->mode = ONLP_LED_MODE_GREEN;
                }
                else if(led_color == 2){
                    info_p->mode = ONLP_LED_MODE_YELLOW;
                    break;
                }
            }
            break;
        case LED_FAN1_ID:
        case LED_FAN2_ID:
        case LED_FAN3_ID:
            led_color = result & 0x3;

            if(led_color == 0 || led_color == 3){
                info_p->mode = ONLP_LED_MODE_OFF;
            }
            if(led_color == 1){
                info_p->mode |= ONLP_LED_MODE_GREEN;
            }
            if(led_color == 2){
                info_p->mode |= ONLP_LED_MODE_YELLOW;
            }
            break;
        }
    return ONLP_STATUS_OK;
}

