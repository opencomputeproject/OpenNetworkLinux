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

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-quanta-ly8-rangeley-r0";
}

int
onlp_sysi_init(void)
{
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
