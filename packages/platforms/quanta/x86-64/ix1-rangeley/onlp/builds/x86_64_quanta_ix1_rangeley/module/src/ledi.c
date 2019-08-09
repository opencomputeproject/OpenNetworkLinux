#include <onlp/platformi/ledi.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "x86_64_quanta_ix1_rangeley_int.h"
#include <x86_64_quanta_ix1_rangeley/x86_64_quanta_ix1_rangeley_gpio_table.h>
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
    },
    {
        { LED_OID_FAN, "Front FAN LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
    },
    {
        { LED_OID_PSU_1, "Front PSU(1) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
    },
    {
        { LED_OID_PSU_2, "Front PSU(2) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
    },
    {
        { LED_OID_FAN_FAIL_1, "FAN(1) fail LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED,
    },
    {
        { LED_OID_FAN_FAIL_2, "FAN(2) fail LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED,
    },
    {
        { LED_OID_FAN_FAIL_3, "FAN(3) fail LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED,
    },
    {
        { LED_OID_FAN_FAIL_4, "FAN(4) fail LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED,
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
        onlp_gpio_set(QUANTA_IX1_CPU_BOARD_SYS_P1, 0);
        onlp_gpio_set(QUANTA_IX1_CPU_BOARD_SYS_P2, 1);
    }
    else if(mode == ONLP_LED_MODE_ORANGE){
        onlp_gpio_set(QUANTA_IX1_CPU_BOARD_SYS_P1, 1);
        onlp_gpio_set(QUANTA_IX1_CPU_BOARD_SYS_P2, 0);
    }
    else{
        onlp_gpio_set(QUANTA_IX1_CPU_BOARD_SYS_P1, 1);
        onlp_gpio_set(QUANTA_IX1_CPU_BOARD_SYS_P2, 1);
    }
}

void
Sysfs_Set_Fan_LED(onlp_led_mode_t mode)
{
    if(mode == ONLP_LED_MODE_GREEN){
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_FAN_GREEN_R, 1);
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_FAN_RED_R, 0);
    }
    else if(mode == ONLP_LED_MODE_RED){
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_FAN_GREEN_R, 0);
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_FAN_RED_R, 1);
    }
    else{
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_FAN_GREEN_R, 0);
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_FAN_RED_R, 0);
    }
}

void
Sysfs_Set_Psu1_LED(onlp_led_mode_t mode)
{
    if(mode == ONLP_LED_MODE_GREEN){
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_PSU1_GREEN_R, 1);
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_PSU1_RED_R, 0);
    }
    else if(mode == ONLP_LED_MODE_RED){
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_PSU1_GREEN_R, 0);
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_PSU1_RED_R, 1);
    }
    else{
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_PSU1_GREEN_R, 0);
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_PSU1_RED_R, 0);
    }
}

void
Sysfs_Set_Psu2_LED(onlp_led_mode_t mode)
{
    if(mode == ONLP_LED_MODE_GREEN){
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_PSU2_GREEN_R, 1);
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_PSU2_RED_R, 0);
    }
    else if(mode == ONLP_LED_MODE_RED){
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_PSU2_GREEN_R, 0);
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_PSU2_RED_R, 1);
    }
    else{
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_PSU2_GREEN_R, 0);
        onlp_gpio_set(QUANTA_IX1_PSU_GPIO_PSU2_RED_R, 0);
    }
}

void
Sysfs_Set_Fan_Fail1_LED(onlp_led_mode_t mode)
{
    if(mode == ONLP_LED_MODE_RED){
        onlp_gpio_set(QUANTA_IX1_FAN_FAIL_LED_1, 1);
    }
    else{
        onlp_gpio_set(QUANTA_IX1_FAN_FAIL_LED_1, 0);
    }
}

void
Sysfs_Set_Fan_Fail2_LED(onlp_led_mode_t mode)
{
    if(mode == ONLP_LED_MODE_RED){
        onlp_gpio_set(QUANTA_IX1_FAN_FAIL_LED_2, 1);
    }
    else{
        onlp_gpio_set(QUANTA_IX1_FAN_FAIL_LED_2, 0);
    }
}

void
Sysfs_Set_Fan_Fail3_LED(onlp_led_mode_t mode)
{
    if(mode == ONLP_LED_MODE_RED){
        onlp_gpio_set(QUANTA_IX1_FAN_FAIL_LED_3, 1);
    }
    else{
        onlp_gpio_set(QUANTA_IX1_FAN_FAIL_LED_3, 0);
    }
}

void
Sysfs_Set_Fan_Fail4_LED(onlp_led_mode_t mode)
{
    if(mode == ONLP_LED_MODE_RED){
        onlp_gpio_set(QUANTA_IX1_FAN_FAIL_LED_4, 1);
    }
    else{
        onlp_gpio_set(QUANTA_IX1_FAN_FAIL_LED_4, 0);
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
      case LED_ID_FAN:
        Sysfs_Set_Fan_LED(mode);
        break;
      case LED_ID_PSU_1:
        Sysfs_Set_Psu1_LED(mode);
        break;
      case LED_ID_PSU_2:
        Sysfs_Set_Psu2_LED(mode);
        break;
      case LED_ID_FAN_FAIL_1:
        Sysfs_Set_Fan_Fail1_LED(mode);
        break;
      case LED_ID_FAN_FAIL_2:
        Sysfs_Set_Fan_Fail2_LED(mode);
        break;
      case LED_ID_FAN_FAIL_3:
        Sysfs_Set_Fan_Fail3_LED(mode);
        break;
      case LED_ID_FAN_FAIL_4:
        Sysfs_Set_Fan_Fail4_LED(mode);
        break;
      default:
        return ONLP_STATUS_E_INTERNAL;
        break;
    }

    return ONLP_STATUS_OK;
}
