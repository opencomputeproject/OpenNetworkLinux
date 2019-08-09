/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <powerpc_quanta_lb9/powerpc_quanta_lb9_config.h>
#include <onlplib/file.h>
#include "powerpc_quanta_lb9_int.h"
#include "powerpc_quanta_lb9_log.h"
#include "system.h"

int
powerpc_quanta_lb9_system_airflow_get(void)
{
    int i;
    int f2b = 0;
    int b2f = 0;

    for(i = 1; i < 5; i++) {
        int rpm = 0;
        onlp_file_read_int(&rpm, SYS_CONTROLLER_PREFIX_F2B "*fan%d_input", i);
        f2b += rpm;
    }
    for(i = 1; i < 5; i++) {
        int rpm = 0;
        onlp_file_read_int(&rpm, SYS_CONTROLLER_PREFIX_B2F "*fan%d_input", i);
        b2f += rpm;
    }

    if(f2b && !b2f) {
        return 0;
    }
    else if(b2f && !f2b) {
        return 1;
    }
    else {
        AIM_LOG_ERROR("Cannot determine active airflow controller.");
        return -1;
    }
    return 0;
}

char*
powerpc_quanta_lb9_system_fan_dir(void)
{
    /*
     * Determine the correct HW monitor path based on
     * current system settings.
     */
    int airflow = powerpc_quanta_lb9_system_airflow_get();
    switch(airflow)
        {
        case 0: return SYS_CONTROLLER_PREFIX_F2B; break;
        case 1: return SYS_CONTROLLER_PREFIX_B2F; break;
        }

    /* Error message has already been reported. */
    return NULL;
}

char*
powerpc_quanta_lb8_r9_system_psu_dir(int pid)
{
    switch(pid)
        {
        case PSU_ID_PSU1: return SYS_PSU1_PREFIX; break;
        case PSU_ID_PSU2: return SYS_PSU2_PREFIX; break;
        }

    AIM_LOG_ERROR("Invalid PSU id %d", pid);
    return NULL;
}
