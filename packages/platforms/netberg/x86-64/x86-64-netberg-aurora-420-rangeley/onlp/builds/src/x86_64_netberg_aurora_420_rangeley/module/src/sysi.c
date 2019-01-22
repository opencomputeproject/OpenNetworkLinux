/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <onlp/platformi/sysi.h>
#include <onlplib/file.h>
#include "x86_64_netberg_aurora_420_rangeley_int.h"
#include "x86_64_netberg_aurora_420_rangeley_log.h"

/*
 * This is the first function called by the ONLP framework.
 *
 * It should return the name of your platform driver.
 *
 * If the name of your platform driver is the same as the
 * current platform then this driver will be used.
 *
 * If the name of the driver is different from the current
 * platform, or the driver is capable of supporting multiple
 * platform variants, see onlp_sysi_platform_set() below.
 */
const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-netberg-aurora-420-rangeley-r0";
}

/*
 * This is the first function the ONLP framework will call
 * after it has validated the the platform is supported using the mechanisms
 * described above.
 *
 * If this function does not return ONL_STATUS_OK
 * then platform initialization is aborted.
 */
int
onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{
    int rv;
    uint8_t data[256];
    int len;

    memset(data, 0, sizeof(data));
    rv = onlp_file_read(data, sizeof(data), &len, SYS_HWMON2_PREFIX "/eeprom");
    if (rv == ONLP_STATUS_OK)
    {
        rv = onlp_onie_decode(onie, (uint8_t*)data, sizeof(data));
        if(rv >= 0)
        {
            onie->platform_name = aim_strdup("x86-64-netberg-aurora-420-rangeley-r0");
        }
    }
    return rv;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));
    int i;
    int n_thermal=7, n_fan=10, n_led=4;

     /* 2 PSUs */
    *e++ = ONLP_PSU_ID_CREATE(1);
    *e++ = ONLP_PSU_ID_CREATE(2);

    /* LEDs Item */
    for (i=1; i<=n_led; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

     /* THERMALs Item */
    for (i=1; i<=n_thermal; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* Fans Item */
    for (i=1; i<=n_fan; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

