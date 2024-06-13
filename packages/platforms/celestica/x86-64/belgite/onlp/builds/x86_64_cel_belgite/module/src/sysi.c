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
#include <x86_64_cel_belgite/x86_64_cel_belgite_config.h>

#include "x86_64_cel_belgite_int.h"
#include "x86_64_cel_belgite_log.h"
#include "platform.h"
//Below include add for support Cache system
#include <sys/stat.h>
#include <time.h>
#include <sys/mman.h>
#include <semaphore.h>

#define INTERVAL 3
#define TEMP_CONV_FACTOR 1000
#define PWM_START 128   // 255 * 50%
#define PWM_MAX   255
#define PWM_FAULT_DEF   1000 // less than 1000rpm, regarded as fault

#define FAN1_PWM_PATH "/sys/bus/i2c/devices/i2c-2/2-0032/pwm1"
#define FAN2_PWM_PATH "/sys/bus/i2c/devices/i2c-2/2-0032/pwm2"
#define FAN3_PWM_PATH "/sys/bus/i2c/devices/i2c-2/2-0032/pwm3"

#define PSUR_STA "/sys/bus/platform/devices/sys_cpld/psuR_prs"
#define PSUL_STA "/sys/bus/platform/devices/sys_cpld/psuL_prs"

#define ERROR -1
#define OK    0
#define FAULT 1

typedef struct linear
{
    uint8_t min_temp;
    uint8_t min_pwm;
    uint8_t max_temp;
    uint8_t max_pwm;
    uint8_t hysteresis;
} linear;

linear CPULINEAR = {
    67, 102, 85, 255, 3
};

linear U4U7LINEAR = {
    32, 102, 48, 255, 3
};

linear U60LINEAR = {
    52, 102, 70, 255, 3
};

uint8_t get_temp(const char *path)
{
    uint8_t temp = 0;
    int t  = 0;
    int ret = 0;

    ret = get_sysnode_value(path, (void *)&t);
    if (ret)
    {
        printf("Can't get sysnode value\n");
        return ERROR;
    }
    temp = t / TEMP_CONV_FACTOR;

    return temp;
}

int set_fan(const char *path, uint8_t pwm)
{
    int ret = 0;

    ret = set_sysnode_value(path, pwm);

    return ret;
}

int set_all_fans(uint8_t pwm)
{
    int ret = 0;

    ret = set_fan(FAN1_PWM_PATH, pwm);
    ret = set_fan(FAN2_PWM_PATH, pwm);
    ret = set_fan(FAN3_PWM_PATH, pwm);

    return ret;
}

int check_psu_fault(void)
{
    uint8_t status = 0;
    int ret = 0;

    ret = get_sysnode_value(PSUR_STA, (void *)&status);
    if (ret)
    {
        printf("Can't get sysnode value\n");
        return ERROR;
    }
    if (1 == status) /* absent */
    {
        return FAULT;
    }
    ret = get_sysnode_value(PSUL_STA, (void *)&status);
    if (ret)
    {
        printf("Can't get sysnode value\n");
        return ERROR;
    }
    if (FAULT == status) /* absent */
    {
        return FAULT;
    }
    return OK;

}

int check_fan_fault(void)
{
    int speed = 0;
    int ret = 0;

    ret = get_sysnode_value(CHASSIS_FAN1_SPEED, (void *)&speed);
    if (ret)
    {
        printf("Can't get sysnode value\n");
        return ERROR;
    }
    if (speed < PWM_FAULT_DEF)
    {
        return FAULT;
    }
    ret = get_sysnode_value(CHASSIS_FAN2_SPEED, (void *)&speed);
    if (ret)
    {
        printf("Can't get sysnode value\n");
        return ERROR;
    }
    if (speed < PWM_FAULT_DEF)
    {
        return FAULT;
    }
    ret = get_sysnode_value(CHASSIS_FAN3_SPEED, (void *)&speed);
    if (ret)
    {
        printf("Can't get sysnode value\n");
        return ERROR;
    }
    if (speed < PWM_FAULT_DEF)
    {
        return FAULT;
    }
    return OK;

}

short upper_bound(uint8_t temp, uint8_t min_temp, uint8_t max_temp, uint8_t min_pwm, uint8_t max_pwm, uint8_t hysteresis)
{
    short pwm = 0;

    pwm = (temp - min_temp) * (max_pwm - min_pwm) / (max_temp - min_temp - hysteresis) + min_pwm;
    //DEBUG_PRINT("%d, %d, %d, %d, %d, %d, %d\n", temp, min_temp, max_temp, min_pwm, max_pwm, hysteresis, pwm);

    return pwm > 0 ? pwm : 0;
}

short lower_bound(uint8_t temp, uint8_t min_temp, uint8_t max_temp, uint8_t min_pwm, uint8_t max_pwm, uint8_t hysteresis)
{
    short pwm = 0;

    pwm = (temp - min_temp - hysteresis) * (max_pwm - min_pwm) / (max_temp - min_temp - hysteresis) + min_pwm;
    //DEBUG_PRINT("%d, %d, %d, %d, %d, %d, %d\n", temp, min_temp, max_temp, min_pwm, max_pwm, hysteresis, pwm);

    return pwm > 0 ? pwm : 0;
}

uint8_t linear_cal(uint8_t temp, uint8_t last_temp, uint8_t last_pwm, linear p)
{
    uint8_t pwm = 0;
    short ub = 0, lb = 0;

    ub = upper_bound(temp, p.min_temp, p.max_temp, p.min_pwm, p.max_pwm, p.hysteresis);
    lb = lower_bound(temp, p.min_temp, p.max_temp, p.min_pwm, p.max_pwm, p.hysteresis);

    if (temp > last_temp)
    {
        DEBUG_PRINT("temperature rises from %d to %d\n", last_temp, temp);
        if (lb < last_pwm)
        {
            pwm = last_pwm;
        }
        else
        {
            pwm = lb;
        }
    }
    else if (temp < last_temp)
    {
        DEBUG_PRINT("temperature declines from %d to %d\n", last_temp, temp);
        if (ub > last_pwm)
        {
            pwm = last_pwm;
        }
        else
        {
            pwm = ub;
        }
    }
    else
    {
        DEBUG_PRINT("temperature keeps to %d\n", temp);
        pwm = last_pwm;
    }
    if (pwm > p.max_pwm)
    {
        pwm = p.max_pwm;
    }
    else if (pwm < p.min_pwm)
    {
        pwm = p.min_pwm;
    }
    DEBUG_PRINT("last pwm: %d, new pwm: %d\n", last_pwm, pwm);

    return pwm;
}

void update_fan(void)
{
    uint8_t cpu_pwm = 0, u4_pwm = 0, u7_pwm = 0, u60_pwm = 0, pwm = 0;
    uint8_t cpu_temp = 0, u4_temp = 0, u7_temp = 0, u60_temp = 0;
    static uint8_t last_cpu_temp = 0, last_u4_temp = 0, last_u7_temp = 0, last_u60_temp = 0;
    static uint8_t last_cpu_pwm = 0, last_u4_pwm = 0, last_u7_pwm = 0, last_u60_pwm = 0;

    /* get all temperatures */
    cpu_temp = get_temp(CPU_THERMAL);
    u4_temp  = get_temp(CHASSIS_THERMAL2);
    u7_temp  = get_temp(CHASSIS_THERMAL4);
    u60_temp = get_temp(CHASSIS_THERMAL3);

    /* get all PWMs */
    cpu_pwm = linear_cal(cpu_temp, last_cpu_temp, last_cpu_pwm, CPULINEAR);
    u4_pwm  = linear_cal(u4_temp, last_u4_temp, last_u4_pwm, U4U7LINEAR);
    u7_pwm  = linear_cal(u7_temp, last_u7_temp, last_u7_pwm, U4U7LINEAR);
    u60_pwm = linear_cal(u60_temp, last_u60_temp, last_u60_pwm, U60LINEAR);

    /* get max PWM */
    pwm = cpu_pwm > u4_pwm ? cpu_pwm : u4_pwm;
    pwm = pwm > u7_pwm ? pwm : u7_pwm;
    pwm = pwm > u60_pwm ? pwm : u60_pwm;

    /* set max PWM to all fans */
    DEBUG_PRINT("final pwm: %d\n", pwm);
    set_all_fans(pwm);

    /* store all last temperatures */
    last_cpu_temp = cpu_temp;
    last_u4_temp  = u4_temp;
    last_u7_temp  = u7_temp;
    last_u60_temp = u60_temp;

    /* store all last PWMs */
    last_cpu_pwm = cpu_pwm;
    last_u4_pwm  = u4_pwm;
    last_u7_pwm  = u7_pwm;
    last_u60_pwm = u60_pwm;
}

static char arr_cplddev_name[NUM_OF_CPLD][10] =
{
        "version"
};

const char *onlp_sysi_platform_get(void)
{
    return "x86-64-cel-belgite-r0";
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
    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_manage_fans(void)
{
    uint8_t ret = 0;
    static uint8_t start = 0;

    /* set initial PWM when start up*/
    if (0 == start)
    {
        set_all_fans(PWM_START);
    start = 1;
    }

    DEBUG_PRINT("Enter fan update\n");
    ret = check_fan_fault();
    if (FAULT == ret)
    {
        set_all_fans(PWM_MAX);
        return ONLP_STATUS_OK;
    }
    ret = check_psu_fault();
    if (FAULT == ret)
    {
        set_all_fans(PWM_MAX);
        return ONLP_STATUS_OK;
    }
    update_fan();

    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_manage_leds(void)
{
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
