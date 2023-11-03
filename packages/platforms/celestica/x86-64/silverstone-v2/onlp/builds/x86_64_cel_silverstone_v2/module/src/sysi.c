#include <unistd.h>
#include <fcntl.h>

#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/sysi.h>
#include <x86_64_cel_silverstone_v2/x86_64_cel_silverstone_v2_config.h>

#include "x86_64_cel_silverstone_v2_int.h"
#include "x86_64_cel_silverstone_v2_log.h"
#include "platform_common.h"
//Below include add for support Cache system
#include <sys/stat.h>
#include <time.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "fancontrol.h"
#include "ledcontrol.h"


static char arr_cplddev_name[NUM_OF_CPLD][10] =
{
        "version"
};

const char *onlp_sysi_platform_get(void)
{
    return "x86-64-cel-silverstone-v2-r0";
}

int onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_info_get(onlp_platform_info_t *pi)
{
    int i, v[NUM_OF_CPLD] = {0};
    char r_data[10] = {0};
    char fullpath[PREFIX_PATH_LEN] = {0};

    for (i = 0; i < NUM_OF_CPLD; i++)
    {
        memset(fullpath, 0, PREFIX_PATH_LEN);
        sprintf(fullpath, "%s%s", SYS_CPLD_PATH, arr_cplddev_name[i]);
        if (read_device_node_string(fullpath, r_data, sizeof(r_data), 0) != 0)
        {
            DEBUG_PRINT("%s(%d): read %s error\n", __FUNCTION__, __LINE__, fullpath);
            return ONLP_STATUS_E_INTERNAL;
        }
        v[i] = strtol(r_data, NULL, 0);
    }
    pi->cpld_versions = aim_fstrdup("CPLD_B=0x%02x", v[0]);
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
    if (is_cache_exist() < 1)
    {
        create_cache();
    }
    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_manage_fans(void)
{
    int bmc_flag;
	bmc_flag = check_bmc_status();
	static uint8_t start = 0;
	
    if(bmc_flag == -1)
		return bmc_flag;
	if(bmc_flag){
        /* set initial PWM when start up*/
        if (0 == start)
        {
            set_all_fans(PWM_START);
            start = 1;
        }
        DEBUG_PRINT("Enter fan update\n");
        update_fan();
	}
	else{
	    if (is_cache_exist() < 1)
		{
			create_cache();
		}

	}
    
    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_manage_leds(void)
{
    int bmc_flag;
	bmc_flag = check_bmc_status();
	
    if(bmc_flag == -1)
		return bmc_flag;
	if(bmc_flag){
       update_led();
	}
	else{
	    if (is_cache_exist() < 1)
		{
			create_cache();
		}

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
