#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/sysi.h>
#include <x86_64_cls_ds4101/x86_64_cls_ds4101_config.h>

//Below include add for support Cache system
#include <sys/stat.h>
#include <time.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "x86_64_cls_ds4101_int.h"
#include "x86_64_cls_ds4101_log.h"
#include "platform_comm.h"
#include "platform_wbmc.h"
#include "platform_wobmc.h"
#include "fancontrol.h"
#include "ledcontrol.h"


uint8_t BMC_Status = ABSENT;

const char *onlp_sysi_platform_get(void)
{
    return "x86-64-cls-ds4101-r0";
}

int onlp_sysi_init(void)
{
    BMC_Status = read_register(BMC_STATUS_REG) & 0x01;

    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_info_get(onlp_platform_info_t *pi)
{
    char r_data[15] = {0};
    char fullpath[PREFIX_PATH_LEN] = {0};

    memset(fullpath, 0, PREFIX_PATH_LEN);
    sprintf(fullpath, "%s%s", SYS_CPLD_PATH, "version");
    if (read_device_node_string(fullpath, r_data, sizeof(r_data), 0) != 0)
    {
        DEBUG_PRINT("%s(%d): read %s error\n", __FUNCTION__, __LINE__, fullpath);
        return ONLP_STATUS_E_INTERNAL;
    }
    pi->cpld_versions = aim_fstrdup("CPLD_B=%s", r_data);

    sprintf(fullpath, "%s%s", SYS_FPGA_PATH, "version");
    if (read_device_node_string(fullpath, r_data, sizeof(r_data), 0) != 0)
    {
        DEBUG_PRINT("%s(%d): read %s error\n", __FUNCTION__, __LINE__, fullpath);
        return ONLP_STATUS_E_INTERNAL;
    }
    pi->other_versions = aim_fstrdup("FPGA=%s", r_data);

    return 0;
}

void onlp_sysi_platform_info_free(onlp_platform_info_t *pi)
{
    aim_free(pi->cpld_versions);
}

int onlp_sysi_onie_data_get(uint8_t **data, int *size)
{
    uint8_t *rdata = aim_zmalloc(256);

    if (onlp_file_read(rdata, 256, size, PREFIX_PATH_ON_SYS_EEPROM) == ONLP_STATUS_OK)
    {
        if (*size == 256)
        {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }
    aim_free(rdata);
    rdata = NULL;
    *size = 0;
    DEBUG_PRINT("[Debug][%s][%d][Can't get onie data]\n", __FUNCTION__, __LINE__);

    return ONLP_STATUS_E_INTERNAL;
}

void onlp_sysi_onie_data_free(uint8_t *data)
{
    aim_free(data);
}

int onlp_sysi_platform_manage_init(void)
{
    uint8_t value = 0;
    int ret = 0;

    check_sys_airflow();

    /* power on the switch board if it's off because of high terperature */
    value = read_register(SWITCH_PWCTRL_REG);
    if (SWITCH_OFF == (value & 0x03)) {
        /* wait 30s for switch board cooling down */
        sleep(30);
        ret = syscpld_setting(LPC_SWITCH_PWCTRL_REG, SWITCH_ON);
        if (ret)
        {
            perror("Fail to power on switch board !!!");
        }
        system("reboot");
    }


    if (PRESENT == BMC_Status)
    {
        if (is_cache_exist() < 1)
        {
            create_cache();
        }
    }

    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_manage_fans(void)
{
    if (PRESENT != BMC_Status)
    {
        update_fan();
    }
    else
    {
        if (is_cache_exist() < 1)
            create_cache();
    }

    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_manage_leds(void)
{
    if (PRESENT != BMC_Status)
    {
       update_led();
    }

    return ONLP_STATUS_OK;
}

int onlp_sysi_oids_get(onlp_oid_t *table, int max)
{
    int i;
    onlp_oid_t *e = table;

    memset(table, 0, max * sizeof(onlp_oid_t));

    /* 2 PSUs */
    *e++ = ONLP_PSU_ID_CREATE(1);
    *e++ = ONLP_PSU_ID_CREATE(2);

    // // /* LEDs Item */
    for (i = 1; i <= LED_COUNT; i++)
        *e++ = ONLP_LED_ID_CREATE(i);

    // // /* THERMALs Item */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++)
        *e++ = ONLP_THERMAL_ID_CREATE(i);

    // /* Fans Item */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
        *e++ = ONLP_FAN_ID_CREATE(i);

    return ONLP_STATUS_OK;
}
