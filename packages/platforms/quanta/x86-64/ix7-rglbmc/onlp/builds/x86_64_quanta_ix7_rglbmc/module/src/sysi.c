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
#include <onlp/platformi/ledi.h>

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
     * 1 LEDs
     */
    *e++ = LED_OID_SYSTEM;

    return 0;
}
