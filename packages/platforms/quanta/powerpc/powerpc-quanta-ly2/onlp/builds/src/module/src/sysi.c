/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <onlp/platformi/sysi.h>
#include "powerpc_quanta_ly2_int.h"
#include "powerpc_quanta_ly2_log.h"
#include <quanta_sys_eeprom/eeprom.h>

const char*
onlp_sysi_platform_get(void)
{
    return "powerpc-quanta-ly2-r0";
}

int
onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}

#define QUANTA_SYS_EEPROM_PATH \
"/sys/bus/i2c/devices/2-0054/eeprom"

int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{
    int rv;
    quanta_sys_eeprom_t e;
    rv = quanta_sys_eeprom_parse_file(QUANTA_SYS_EEPROM_PATH, &e);
    if(rv >= 0) {
        quanta_sys_eeprom_to_onie(&e, onie);
        onie->platform_name = aim_strdup("powerpc-quanta-ly2-r0");
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
    *e++ = THERMAL_OID_THERMAL4;
    *e++ = THERMAL_OID_THERMAL5;

    /*
     * 4 Fans
     */
    *e++ = FAN_OID_FAN1;
    *e++ = FAN_OID_FAN2;
    *e++ = FAN_OID_FAN3;
    *e++ = FAN_OID_FAN4;

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
