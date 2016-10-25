#include <onlp/platformi/ledi.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "i2c_chips.h"
#include "platform.h"

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t led_info[] =
{
    {
        { ONLP_LED_ID_CREATE(LED_SYSTEM), "Chassis System LED(DIAG LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_1), "Chassis FAN(1) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_2), "Chassis FAN(2) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_3), "Chassis FAN(3) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_4), "Chassis FAN(4) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_5), "Chassis FAN(5) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_6), "Chassis FAN(6) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_7), "Chassis FAN(7) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_8), "Chassis FAN(8) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU_1), "Chassis PSU(1) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU_2), "Chassis PSU(2) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    }
};

int
onlp_ledi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{

    int led_id;

    led_id = ONLP_OID_ID_GET(id) - 1;

    *info = led_info[led_id];
    info->status |= ONLP_LED_STATUS_ON;
    info->mode |= ONLP_LED_MODE_ON;

    return ONLP_STATUS_OK;
}

int
onlp_ledi_set(onlp_oid_t id, int on_or_off)
{
    if (!on_or_off)
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
   else
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_ON);
}

int
onlp_ledi_mode_set(onlp_oid_t id, onlp_led_mode_t mode)
{
    int led_id, psu;

    led_id = ONLP_OID_ID_GET(id) - 1;
    switch (led_id) {
      case LED_SYSTEM:
        if (mode == ONLP_LED_MODE_OFF)
            setSysLedOff();
        else
            setSysLedOn();
        break;
      case LED_FAN_1:
      case LED_FAN_2:
      case LED_FAN_3:
      case LED_FAN_4:
      case LED_FAN_5:
      case LED_FAN_6:
      case LED_FAN_7:
      case LED_FAN_8:
        if (mode == ONLP_LED_MODE_OFF)
            setFanLedOff(led_id);
        else
            setFanLedGreen(led_id);
        break;
      case LED_PSU_1:
      case LED_PSU_2:
        psu = led_id - LED_PSU_1;
        if (mode == ONLP_LED_MODE_OFF)
            setPsuLedOn(psu);
        else
            setPsuLedOff(psu);
        break;
      default:
        return ONLP_STATUS_E_INTERNAL;
        break;
    }
    return ONLP_STATUS_OK;
}
