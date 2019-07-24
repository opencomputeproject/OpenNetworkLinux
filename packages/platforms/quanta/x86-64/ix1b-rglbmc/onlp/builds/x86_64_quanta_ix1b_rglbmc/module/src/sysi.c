/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <onlp/platformi/sysi.h>
#include "x86_64_quanta_ix1b_rglbmc_int.h"
#include "x86_64_quanta_ix1b_rglbmc_log.h"
#include <quanta_sys_eeprom/eeprom.h>
#include <x86_64_quanta_ix1b_rglbmc/x86_64_quanta_ix1b_rglbmc_gpio_table.h>
#include <onlplib/gpio.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/ledi.h>

struct led_control_s led_control;

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-quanta-ix1b-rglbmc-r0";
}

int
onlp_sysi_init(void)
{
    /* Initial value */
    led_control.PMCnt = 0;
    led_control.psu1_present = 0;
    led_control.psu2_present = 0;
    led_control.psu1_power_good = 0;
    led_control.psu2_power_good = 0;

    /* Config GPIO */
    /* LED Output */
    onlp_gpio_export(QUANTA_IX1B_CPU_BOARD_SYS_P1, ONLP_GPIO_DIRECTION_OUT);
    onlp_gpio_export(QUANTA_IX1B_CPU_BOARD_SYS_P2, ONLP_GPIO_DIRECTION_OUT);
    onlp_gpio_export(QUANTA_IX1B_PSU_GPIO_PSU1_GREEN_R, ONLP_GPIO_DIRECTION_OUT);
    onlp_gpio_export(QUANTA_IX1B_PSU_GPIO_PSU1_RED_R, ONLP_GPIO_DIRECTION_OUT);
    onlp_gpio_export(QUANTA_IX1B_PSU_GPIO_PSU2_GREEN_R, ONLP_GPIO_DIRECTION_OUT);
    onlp_gpio_export(QUANTA_IX1B_PSU_GPIO_PSU2_RED_R, ONLP_GPIO_DIRECTION_OUT);
    onlp_gpio_export(QUANTA_IX1B_FAN_FAIL_LED_1, ONLP_GPIO_DIRECTION_OUT);
    onlp_gpio_export(QUANTA_IX1B_FAN_FAIL_LED_2, ONLP_GPIO_DIRECTION_OUT);
    onlp_gpio_export(QUANTA_IX1B_FAN_FAIL_LED_3, ONLP_GPIO_DIRECTION_OUT);
    onlp_gpio_export(QUANTA_IX1B_FAN_FAIL_LED_4, ONLP_GPIO_DIRECTION_OUT);
    onlp_gpio_export(QUANTA_IX1B_PSU_GPIO_FAN_GREEN_R, ONLP_GPIO_DIRECTION_OUT);
    onlp_gpio_export(QUANTA_IX1B_PSU_GPIO_FAN_RED_R, ONLP_GPIO_DIRECTION_OUT);

    /* PSU Input */
    onlp_gpio_export(QUANTA_IX1B_PSU_GPIO_PSU1_PRSNT_N, ONLP_GPIO_DIRECTION_IN);
    onlp_gpio_export(QUANTA_IX1B_PSU_GPIO_PSU1_PWRGD, ONLP_GPIO_DIRECTION_IN);
    onlp_gpio_export(QUANTA_IX1B_PSU_GPIO_PSU2_PRSNT_N, ONLP_GPIO_DIRECTION_IN);
    onlp_gpio_export(QUANTA_IX1B_PSU_GPIO_PSU2_PWRGD, ONLP_GPIO_DIRECTION_IN);

    /* FAN Input */
    onlp_gpio_export(QUANTA_IX1B_FAN_PRSNT_N_1, ONLP_GPIO_DIRECTION_IN);
    onlp_gpio_export(QUANTA_IX1B_FAN_PRSNT_N_2, ONLP_GPIO_DIRECTION_IN);
    onlp_gpio_export(QUANTA_IX1B_FAN_PRSNT_N_3, ONLP_GPIO_DIRECTION_IN);
    onlp_gpio_export(QUANTA_IX1B_FAN_PRSNT_N_4, ONLP_GPIO_DIRECTION_IN);

    /* Set LED to green */
    onlp_ledi_mode_set(LED_OID_SYSTEM, ONLP_LED_MODE_GREEN);
    led_control.psu_status_changed = 1;
    led_control.fan_status_changed = 1;
    onlp_sysi_platform_manage_leds();

    return ONLP_STATUS_OK;
}

#define QUANTA_SYS_EEPROM_PATH \
"/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-26/26-0054/eeprom"

int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{
    int rv;

    rv = onlp_onie_decode_file(onie, QUANTA_SYS_EEPROM_PATH);
    if(rv >= 0) {
        onie->platform_name = aim_strdup("x86-64-quanta-ix1b-rglbmc-r0");
        rv = quanta_onie_sys_eeprom_custom_format(onie);
    }
    return rv;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /*
     * 2 PSUs
     */
    *e++ = PSU_OID_PSU1;
    *e++ = PSU_OID_PSU2;

    /*
     * 8 LEDs
     */
    *e++ = LED_OID_SYSTEM;
    *e++ = LED_OID_FAN;
    *e++ = LED_OID_PSU_1;
    *e++ = LED_OID_PSU_2;
    *e++ = LED_OID_FAN_FAIL_1;
    *e++ = LED_OID_FAN_FAIL_2;
    *e++ = LED_OID_FAN_FAIL_3;
    *e++ = LED_OID_FAN_FAIL_4;

    return 0;
}

int
update_rpsu_fan_status(void){
    int last_status, rv, value = -1/*, tmp*/;

    last_status = led_control.psu1_present;
    rv = onlp_gpio_get(QUANTA_IX1B_PSU_GPIO_PSU1_PRSNT_N, &value);
    if(rv < 0) {
        AIM_LOG_ERROR("GPIO %d read Error!", QUANTA_IX1B_PSU_GPIO_PSU1_PRSNT_N);
        return rv;
    }
    led_control.psu1_present = (value ? 0 : 1);
    if(last_status != led_control.psu1_present)
        led_control.psu_status_changed = 1;

    last_status = led_control.psu1_power_good;
    rv = onlp_gpio_get(QUANTA_IX1B_PSU_GPIO_PSU1_PWRGD, &value);
    if(rv < 0) {
        AIM_LOG_ERROR("GPIO %d read Error!", QUANTA_IX1B_PSU_GPIO_PSU1_PWRGD);
        return rv;
    }
    led_control.psu1_power_good = (value ? 1 : 0);
    if(last_status != led_control.psu1_power_good)
        led_control.psu_status_changed = 1;

    last_status = led_control.psu2_present;
    rv = onlp_gpio_get(QUANTA_IX1B_PSU_GPIO_PSU2_PRSNT_N, &value);
    if(rv < 0) {
        AIM_LOG_ERROR("GPIO %d read Error!", QUANTA_IX1B_PSU_GPIO_PSU2_PRSNT_N);
        return rv;
    }
    led_control.psu2_present = (value ? 0 : 1);
    if(last_status != led_control.psu2_present)
        led_control.psu_status_changed = 1;

    last_status = led_control.psu2_power_good;
    rv = onlp_gpio_get(QUANTA_IX1B_PSU_GPIO_PSU2_PWRGD, &value);
    if(rv < 0) {
        AIM_LOG_ERROR("GPIO %d read Error!", QUANTA_IX1B_PSU_GPIO_PSU2_PWRGD);
        return rv;
    }
    led_control.psu2_power_good = (value ? 1 : 0);
    if(last_status != led_control.psu2_power_good)
        led_control.psu_status_changed = 1;

    last_status = led_control.fan1_present;
    rv = onlp_gpio_get(QUANTA_IX1B_FAN_PRSNT_N_1, &value);
    if(rv < 0) {
        AIM_LOG_ERROR("GPIO %d read Error!", QUANTA_IX1B_FAN_PRSNT_N_1);
        return rv;
    }
    led_control.fan1_present = (value ? 0 : 1);
    if(last_status != led_control.fan1_present)
        led_control.fan_status_changed = 1;

    last_status = led_control.fan2_present;
    rv = onlp_gpio_get(QUANTA_IX1B_FAN_PRSNT_N_2, &value);
    if(rv < 0) {
        AIM_LOG_ERROR("GPIO %d read Error!", QUANTA_IX1B_FAN_PRSNT_N_2);
        return rv;
    }
    led_control.fan2_present = (value ? 0 : 1);
    if(last_status != led_control.fan2_present)
        led_control.fan_status_changed = 1;

    last_status = led_control.fan3_present;
    rv = onlp_gpio_get(QUANTA_IX1B_FAN_PRSNT_N_3, &value);
    if(rv < 0) {
        AIM_LOG_ERROR("GPIO %d read Error!", QUANTA_IX1B_FAN_PRSNT_N_3);
        return rv;
    }
    led_control.fan3_present = (value ? 0 : 1);
    if(last_status != led_control.fan3_present)
        led_control.fan_status_changed = 1;

    last_status = led_control.fan4_present;
    rv = onlp_gpio_get(QUANTA_IX1B_FAN_PRSNT_N_4, &value);
    if(rv < 0) {
        AIM_LOG_ERROR("GPIO %d read Error!", QUANTA_IX1B_FAN_PRSNT_N_4);
        return rv;
    }
    led_control.fan4_present = (value ? 0 : 1);
    if(last_status != led_control.fan4_present)
        led_control.fan_status_changed = 1;

    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    int rv;

    led_control.PMCnt++;
    if(led_control.PMCnt>300)
        led_control.PMCnt = 0;
    if(led_control.PMCnt % 5 == 1){/* Each 10 seconds detect one time */

        rv = update_rpsu_fan_status();
        if(rv < 0){
            printf("onlp_sysi_platform_manage_leds error\n");
            return ONLP_STATUS_E_INVALID;
        }

        if(led_control.psu_status_changed){
            if(led_control.psu1_present && led_control.psu1_power_good) {
                onlp_ledi_mode_set(LED_ID_PSU_1, ONLP_LED_MODE_GREEN);
            }
            else if(!led_control.psu1_present){
                onlp_ledi_mode_set(LED_ID_PSU_1, ONLP_LED_MODE_OFF);
            }
            else{
                onlp_ledi_mode_set(LED_ID_PSU_1, ONLP_LED_MODE_RED);
            }

            if(led_control.psu2_present && led_control.psu2_power_good) {
                onlp_ledi_mode_set(LED_ID_PSU_2, ONLP_LED_MODE_GREEN);
            }
            else if(!led_control.psu2_present){
                onlp_ledi_mode_set(LED_ID_PSU_2, ONLP_LED_MODE_OFF);
            }
            else{
                onlp_ledi_mode_set(LED_ID_PSU_2, ONLP_LED_MODE_RED);
            }
            led_control.psu_status_changed = 0;
        }

        if(led_control.fan_status_changed){
            if(led_control.fan1_present && led_control.fan2_present && led_control.fan3_present && led_control.fan4_present){
                 onlp_ledi_mode_set(LED_ID_FAN, ONLP_LED_MODE_GREEN);
            }
            else{
                 onlp_ledi_mode_set(LED_ID_FAN, ONLP_LED_MODE_RED);
            }
            led_control.fan_status_changed = 0;
        }
    }

    return ONLP_STATUS_OK;
}
