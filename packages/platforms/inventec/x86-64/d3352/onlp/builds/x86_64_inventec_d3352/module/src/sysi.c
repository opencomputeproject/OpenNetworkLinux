/************************************************************
 * sysi.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 ***********************************************************/
#include <unistd.h>
#include <fcntl.h>

#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "x86_64_inventec_d3352_int.h"
#include "x86_64_inventec_d3352_log.h"

#include "platform_lib.h"

#define NUM_OF_CPLD			INV_CPLD_COUNT

#define NUM_OF_THERMAL_ON_MAIN_BROAD	(CHASSIS_THERMAL_COUNT)
#define NUM_OF_FAN_ON_MAIN_BROAD	(CHASSIS_FAN_COUNT)
#define NUM_OF_PSU_ON_MAIN_BROAD	(CHASSIS_PSU_COUNT)
#define NUM_OF_LED_ON_MAIN_BROAD	(CHASSIS_LED_COUNT)

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-inventec-d3352-r0";
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);
    if(onlp_file_read(rdata, 256, size, EEPROM_NODE(eeprom)) == ONLP_STATUS_OK) {
        if(*size == 256) {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }

    aim_free(rdata);
    *size = 0;
    return ONLP_STATUS_E_INTERNAL;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 4 Thermal sensors on the chassis */
    for (i = 1; i <= NUM_OF_THERMAL_ON_MAIN_BROAD; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 5 LEDs on the chassis */
    for (i = 1; i <= NUM_OF_LED_ON_MAIN_BROAD; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= NUM_OF_PSU_ON_MAIN_BROAD; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 4 Fans on the chassis */
    for (i = 1; i <= NUM_OF_FAN_ON_MAIN_BROAD; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}



static int _sysi_version_parsing(char* file_str, char* str_buf, char* version)
{
    int rv = ONLP_STATUS_OK;
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX*4];
    char *temp;

    rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX*4, &len, file_str);
    if( rv != ONLP_STATUS_OK ) {
        return rv;
    }

    temp = strstr(buf, str_buf);
    if(temp) {
        temp += strlen(str_buf);
        snprintf(version,ONLP_CONFIG_INFO_STR_MAX, temp);
        /*remove '\n'*/
        version[strlen(version)-1] = '\0';
    } else {
        rv = ONLP_STATUS_E_MISSING;
    }
    return rv;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int rv = ONLP_STATUS_OK;
    char cpld_str[ONLP_CONFIG_INFO_STR_MAX]= {0};
    char other_str[ONLP_CONFIG_INFO_STR_MAX]= {0};
    char version[ONLP_CONFIG_INFO_STR_MAX];

    rv = _sysi_version_parsing(INV_CPLD_PREFIX"info", "The CPLD version is ", version);
    if( rv != ONLP_STATUS_OK ) {
        return rv;
    }
    snprintf(cpld_str, ONLP_CONFIG_INFO_STR_MAX, "%s%s ", cpld_str, version);
    rv = _sysi_version_parsing(INV_PSOC_PREFIX"version", "ver: ", version);
    if( rv != ONLP_STATUS_OK ) {
        return rv;
    }
    snprintf(other_str, ONLP_CONFIG_INFO_STR_MAX, "%s%s%s "
             ,other_str, "\n\t\tpsoc: ", version);

    /*cpld version*/
    if(strlen(cpld_str) > 0) {
        pi->cpld_versions = aim_fstrdup("%s",cpld_str);
    }

    /*other version*/
    if(strlen(other_str) > 0) {
        pi->other_versions = aim_fstrdup("%s",other_str);
    }
    return rv;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}
