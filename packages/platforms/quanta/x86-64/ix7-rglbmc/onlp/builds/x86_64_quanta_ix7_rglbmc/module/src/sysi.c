/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <onlp/platformi/sysi.h>
#include "x86_64_quanta_ix7_rglbmc_int.h"
#include "x86_64_quanta_ix7_rglbmc_log.h"
#include <quanta_sys_eeprom/eeprom.h>
#include <x86_64_quanta_ix7_rglbmc/x86_64_quanta_ix7_rglbmc_gpio_table.h>
#include <onlplib/gpio.h>
#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include <onlp/platformi/ledi.h>

#define I2C_MONITOR_ENABLE_PATH   "/sys/class/quanta/sys_control/i2c_monitor_enable"
#define I2C_MONITOR_INTERVAL_PATH "/sys/class/quanta/sys_control/i2c_monitor_interval"

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-quanta-ix7-rglbmc-r0";
}

int
onlp_sysi_init(void)
{
    /* Config GPIO */
    /* LED Output */
    onlp_gpio_export(QUANTA_IX7_CPU_BOARD_SYS_P1, ONLP_GPIO_DIRECTION_OUT);
    onlp_gpio_export(QUANTA_IX7_CPU_BOARD_SYS_P2, ONLP_GPIO_DIRECTION_OUT);

    /* Set LED to green */
    onlp_ledi_mode_set(LED_OID_SYSTEM, ONLP_LED_MODE_GREEN);

    /* Set I2C monitor enable */
    onlp_file_write_int(1, I2C_MONITOR_ENABLE_PATH);
    onlp_file_write_int(255, I2C_MONITOR_INTERVAL_PATH);

    return ONLP_STATUS_OK;
}

#define QUANTA_SYS_EEPROM_PATH \
"/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0054/eeprom"

int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{
    int rv;

    rv = onlp_onie_decode_file(onie, QUANTA_SYS_EEPROM_PATH);
    if(rv >= 0) {
        onie->platform_name = aim_strdup("x86-64-quanta-ix7-rglbmc-r0");
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
     * 9 THERMAL
     */
    *e++ = THERMAL_OID_THERMAL68;
    *e++ = THERMAL_OID_THERMAL69;
    *e++ = THERMAL_OID_THERMAL70;
    *e++ = THERMAL_OID_THERMAL71;
    *e++ = THERMAL_OID_THERMAL72;
    *e++ = THERMAL_OID_THERMAL73;
    *e++ = THERMAL_OID_THERMAL74;
    *e++ = THERMAL_OID_THERMAL75;
    *e++ = THERMAL_OID_THERMAL76;

    /*
     * 12 FAN
     */
    *e++ = FAN_OID_FAN21;
    *e++ = FAN_OID_FAN22;
    *e++ = FAN_OID_FAN23;
    *e++ = FAN_OID_FAN24;
    *e++ = FAN_OID_FAN25;
    *e++ = FAN_OID_FAN26;
    *e++ = FAN_OID_FAN27;
    *e++ = FAN_OID_FAN28;
    *e++ = FAN_OID_FAN29;
    *e++ = FAN_OID_FAN30;
    *e++ = FAN_OID_FAN31;
    *e++ = FAN_OID_FAN32;

    /*
     * 2 PSU
     */
    *e++ = PSU_OID_PSU101;
    *e++ = PSU_OID_PSU102;

    /*
     * 1 LEDs
     */
    *e++ = LED_OID_SYSTEM;

    return 0;
}
