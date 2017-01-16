/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <onlp/platformi/sysi.h>
#include "x86_64_quanta_ly8_rangeley_int.h"
#include "x86_64_quanta_ly8_rangeley_log.h"
#include <quanta_sys_eeprom/eeprom.h>
#include <x86_64_quanta_ly8_rangeley/x86_64_quanta_ly8_rangeley_gpio_table.h>
#include <quanta_lib/gpio.h>

struct led_control_s led_control;

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-quanta-ly8-rangeley-r0";
}

int
onlp_sysi_init(void)
{
    led_control.PMCnt = 0;
    led_control.psu1_mvin = 0;
    led_control.psu2_mvin = 0;
    led_control.fan1_rpm = 0;
    led_control.fan2_rpm = 0;
    led_control.fan3_rpm = 0;
    led_control.fan5_rpm = 0;
    led_control.fan6_rpm = 0;
    led_control.fan7_rpm = 0;

    return ONLP_STATUS_OK;
}

#define QUANTA_SYS_EEPROM_PATH \
"/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-27/27-0054/eeprom"

int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{
    int rv;

    rv = onlp_onie_decode_file(onie, QUANTA_SYS_EEPROM_PATH);
    if(rv >= 0) {
        onie->platform_name = aim_strdup("x86-64-quanta-ly8-rangeley-r0");
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
     * 5 Chassis Thermal Sensors
     */
    *e++ = THERMAL_OID_THERMAL1;
    *e++ = THERMAL_OID_THERMAL2;
    *e++ = THERMAL_OID_THERMAL3;
    *e++ = THERMAL_OID_THERMAL5;
    *e++ = THERMAL_OID_THERMAL6;

    /*
     * 6 Fans
     */
    *e++ = FAN_OID_FAN1;
    *e++ = FAN_OID_FAN2;
    *e++ = FAN_OID_FAN3;
    *e++ = FAN_OID_FAN5;
    *e++ = FAN_OID_FAN6;
    *e++ = FAN_OID_FAN7;

    /*
     * 2 PSUs
     */
    *e++ = PSU_OID_PSU1;
    *e++ = PSU_OID_PSU2;

    /*
     * Todo - LEDs
     */
    return 0;
}

int
onlp_sysi_platform_manage_leds(void)
{
    led_control.PMCnt++;
    if(led_control.PMCnt>300)
        led_control.PMCnt = 0;
    if(led_control.PMCnt % 5 == 1){//Each 10 seconds detect one time
        if(led_control.psu1_mvin != 0) {
            pca953x_gpio_value_set(PSU_GPIO_PSU1_GREEN_R, 1);
            pca953x_gpio_value_set(PSU_GPIO_PSU1_RED_R, 0);
        }
        else{
            pca953x_gpio_value_set(PSU_GPIO_PSU1_GREEN_R, 0);
            pca953x_gpio_value_set(PSU_GPIO_PSU1_RED_R, 1);
        }

        if(led_control.psu2_mvin != 0) {
            pca953x_gpio_value_set(PSU_GPIO_PSU2_GREEN_R, 1);
            pca953x_gpio_value_set(PSU_GPIO_PSU2_RED_R, 0);
        }
        else{
            pca953x_gpio_value_set(PSU_GPIO_PSU2_GREEN_R, 0);
            pca953x_gpio_value_set(PSU_GPIO_PSU2_RED_R, 1);
        }

        if(led_control.fan1_rpm >= X86_64_QUANTA_LY8_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD && led_control.fan5_rpm >= X86_64_QUANTA_LY8_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD) {
             pca953x_gpio_value_set(FAN_FAIL_LED_1, 0);
        }
        else{
             pca953x_gpio_value_set(FAN_FAIL_LED_1, 1);
        }

        if(led_control.fan2_rpm >= X86_64_QUANTA_LY8_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD && led_control.fan6_rpm >= X86_64_QUANTA_LY8_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD) {
             pca953x_gpio_value_set(FAN_FAIL_LED_2, 0);
        }
        else{
             pca953x_gpio_value_set(FAN_FAIL_LED_2, 1);
        }

        if(led_control.fan3_rpm >= X86_64_QUANTA_LY8_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD && led_control.fan7_rpm >= X86_64_QUANTA_LY8_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD) {
             pca953x_gpio_value_set(FAN_FAIL_LED_3, 0);
        }
        else{
             pca953x_gpio_value_set(FAN_FAIL_LED_3, 1);
        }

        if(led_control.fan1_rpm >= X86_64_QUANTA_LY8_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD && led_control.fan2_rpm >= X86_64_QUANTA_LY8_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD && led_control.fan3_rpm >= X86_64_QUANTA_LY8_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD && led_control.fan5_rpm >= X86_64_QUANTA_LY8_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD && led_control.fan6_rpm >= X86_64_QUANTA_LY8_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD && led_control.fan7_rpm >= X86_64_QUANTA_LY8_RANGELEY_CONFIG_SYSFAN_RPM_FAILURE_THRESHOLD){
             pca953x_gpio_value_set(PSU_GPIO_FAN_GREEN_R, 1);
             pca953x_gpio_value_set(PSU_GPIO_FAN_RED_R, 0);
        }
        else{
             pca953x_gpio_value_set(PSU_GPIO_FAN_GREEN_R, 0);
             pca953x_gpio_value_set(PSU_GPIO_FAN_RED_R, 1);
        }
    }

    return ONLP_STATUS_OK;
}
