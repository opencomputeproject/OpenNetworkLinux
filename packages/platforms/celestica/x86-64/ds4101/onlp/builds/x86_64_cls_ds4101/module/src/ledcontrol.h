#ifndef _LEDCONTROL_H_
#define _LEDCONTROL_H_
#include "fancontrol.h"

enum {
    led_grn,
    led_amb_1hz,
    led_amb_4hz,
    led_amb,
};

int update_led(void);
#endif
