/*
 * Copyright
 */
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/sysi.h>
#include <x86_64_cel_redstone_xp/x86_64_cel_redstone_xp_config.h>

#include "x86_64_cel_redstone_xp_log.h"
#include "platform.h"
#include "cplds.h"
#include "sys_eeprom.h"

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-cel-redstone-xp-r0";
}

int
onlp_sysi_init(void)
{
    return cpld_io_init();
}


int
onlp_sysi_debug(aim_pvs_t* pvs, int argc, char* argv[])
{
    int c;
    for(c = 1; c <= 5; c++) {
        aim_printf(pvs, "CPLD%d:\n", c);
        cpld_dump(pvs, c);
    }
    return 0;
}

int
onlp_sysi_platform_set(const char* name)
{
    /*
     * For the purposes of this example we
     * accept all platforms.
     */
    return ONLP_STATUS_OK;
}

int
onlp_sysi_onie_data_phys_addr_get(void** pa)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(512);

    read_eeprom(rdata, size);
    if (size) {
        *data = rdata;
        return ONLP_STATUS_OK;
    }
    return ONLP_STATUS_E_INTERNAL;
}

void
onlp_sysi_onie_data_free(uint8_t* data)
{
    /*
     * We returned a static array in onlp_sysi_onie_data_get()
     * so no free operation is required.
     */
}

static int
_onlp_sysi_calc_speed(int atemp)
{
    int fan_speed = 0;

    if (atemp < 38 ) {
        fan_speed = 50;
    } else if ( atemp < 40) {
        fan_speed = 60;
    } else if ( atemp < 42) {
        fan_speed = 70;
    } else if ( atemp < 44) {
        fan_speed = 80;
    } else if ( atemp < 46) {
        fan_speed = 90;
    } else {
        fan_speed = 100;
    }

    return fan_speed;
}

/*
 * Thermal profile for Celestica Redstone XP
 *
 * Default Fan speed:   50%
 * Normal operating temperature range: < 46C
 * Fan Speed profile:
 *          46C +   : 100% Fan speed
 *          44C     : 80% Fan speed
 *          42C     : 70% Fan speed
 *          40C     : 60% Fan speed
 *          38C -   : 50% Fan speed
 */
int
onlp_sysi_platform_manage_fans(void)
{
    int i, atemp = 0, count = 0;
    int fan_speed = 0;
    static int o_speed = 0, o_psu_speed[2] = {0,0};
    onlp_thermal_info_t t_info;

    for (i = 1; i < PSU_FAN; i++) {
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i), &t_info);
        if (t_info.mcelsius) {
            count++;
            atemp += t_info.mcelsius;
        }
    }
    atemp = atemp/count;

    fan_speed = _onlp_sysi_calc_speed(atemp);
    if (o_speed != fan_speed) {
        for (i = 1; i < PSU_FAN; i++)
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), fan_speed);
        o_speed = fan_speed;
    }

    /* Control PSU FAN */
    for (i = 0; i < 2  ; i++) {
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i), &t_info);
        if (t_info.mcelsius) {
            atemp = t_info.mcelsius;
            fan_speed = _onlp_sysi_calc_speed(atemp);
            if (fan_speed != o_psu_speed[i]) {
                onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(PSU_FAN + i), fan_speed);
                o_psu_speed[i] = fan_speed;
            }
        }
    }

    return ONLP_STATUS_OK;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;

    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 2 PSUs */
    *e++ = ONLP_PSU_ID_CREATE(1);
    *e++ = ONLP_PSU_ID_CREATE(2);

    /* LEDs Item */
    for (i=1; i<=LED_COUNT; i++)
        *e++ = ONLP_LED_ID_CREATE(i);

    /* THERMALs Item */
    for (i=1; i<=THERMAL_COUNT; i++)
        *e++ = ONLP_THERMAL_ID_CREATE(i);

    /* Fans Item */
    for (i=1; i<=FAN_COUNT; i++)
        *e++ = ONLP_FAN_ID_CREATE(i);

    return ONLP_STATUS_OK;
}
