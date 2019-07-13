#include <onlp/platformi/ledi.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "x86_64_quanta_ly4r_int.h"
#include <x86_64_quanta_ly4r/x86_64_quanta_ly4r_gpio_table.h>
#include <onlplib/gpio.h>

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t led_info[] =
{
    { }, /* Not used */
    {
        { LED_OID_SYSTEM, "System LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN,
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

    led_id = ONLP_OID_ID_GET(id);

    *info = led_info[led_id];
    info->status |= ONLP_LED_STATUS_ON;
    info->mode |= ONLP_LED_MODE_ON;

    return ONLP_STATUS_OK;
}

void
Sysfs_Set_System_LED(onlp_led_mode_t mode)
{
    if(mode == ONLP_LED_MODE_GREEN){
        onlp_gpio_set(QUANTA_LY4R_PCA9698_BOOT_STSLED_N, 0);
        onlp_gpio_set(QUANTA_LY4R_PCA9698_SYS_STSLED, 1);
    }
    else if(mode == ONLP_LED_MODE_ORANGE){
        onlp_gpio_set(QUANTA_LY4R_PCA9698_BOOT_STSLED_N, 1);
        onlp_gpio_set(QUANTA_LY4R_PCA9698_SYS_STSLED, 0);
    }
    else{
        onlp_gpio_set(QUANTA_LY4R_PCA9698_BOOT_STSLED_N, 1);
        onlp_gpio_set(QUANTA_LY4R_PCA9698_SYS_STSLED, 1);
    }
}

int
onlp_ledi_mode_set(onlp_oid_t id, onlp_led_mode_t mode)
{
    int led_id;

    led_id = ONLP_OID_ID_GET(id);
    switch (led_id) {
      case LED_ID_SYSTEM:
        Sysfs_Set_System_LED(mode);
        break;
      default:
        return ONLP_STATUS_E_INTERNAL;
        break;
    }

    return ONLP_STATUS_OK;
}
