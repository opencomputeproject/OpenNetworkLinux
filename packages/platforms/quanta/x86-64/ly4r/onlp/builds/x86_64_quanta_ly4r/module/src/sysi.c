/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <onlp/platformi/sysi.h>
#include "x86_64_quanta_ly4r_int.h"
#include "x86_64_quanta_ly4r_log.h"
#include <quanta_sys_eeprom/eeprom.h>
#include <x86_64_quanta_ly4r/x86_64_quanta_ly4r_gpio_table.h>
#include <onlplib/gpio.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/ledi.h>

const char*
onlp_sysi_platform_get(void)
{
    /* Config GPIO */
    /* LED Output */
    onlp_gpio_export(QUANTA_LY4R_PCA9698_BOOT_STSLED_N, ONLP_GPIO_DIRECTION_OUT);
    onlp_gpio_export(QUANTA_LY4R_PCA9698_SYS_STSLED, ONLP_GPIO_DIRECTION_OUT);

    /* Set LED to green */
    onlp_ledi_mode_set(LED_OID_SYSTEM, ONLP_LED_MODE_GREEN);

    return "x86-64-quanta-ly4r-r0";
}

int
onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}

#define QUANTA_SYS_EEPROM_PATH \
"/sys/devices/pci0000:00/0000:00:13.0/i2c-1/1-0054/eeprom"

int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{
    int rv;

    rv = onlp_onie_decode_file(onie, QUANTA_SYS_EEPROM_PATH);
    if(rv >= 0) {
        onie->platform_name = aim_strdup("x86-64-quanta-ly4r-r0");
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
     * 1 LED
     */
    *e++ = LED_OID_SYSTEM;

    return 0;
}
