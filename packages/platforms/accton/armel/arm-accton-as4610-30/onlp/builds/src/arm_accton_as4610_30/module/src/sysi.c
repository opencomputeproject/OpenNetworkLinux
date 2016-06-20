/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2015 Accton Technology Corporation.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
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

#include "arm_accton_as4610_30_int.h"
#include "arm_accton_as4610_30_log.h"

#include "platform_lib.h"

#define OPEN_READ_FILE(fd,fullpath,data,nbytes,len) \
    DEBUG_PRINT("[Debug][%s][%d][openfile: %s]\n", __FUNCTION__, __LINE__, fullpath); \
    if ((fd = open(fullpath, O_RDONLY)) == -1)  \
       return ONLP_STATUS_E_INTERNAL;           \
    if ((len = read(fd, r_data, nbytes)) <= 0){ \
        close(fd);                              \
        return ONLP_STATUS_E_INTERNAL;}         \
    DEBUG_PRINT("[Debug][%s][%d][read data: %s]\n", __FUNCTION__, __LINE__, r_data); \
    if (close(fd) == -1)                        \
        return ONLP_STATUS_E_INTERNAL

const char*
onlp_sysi_platform_get(void)
{
    return "arm-accton-as4610-30-r0";
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int   v;
    int   fd, len, nbytes = 10;
    char  r_data[10]   = {0};
    char  fullpath[65] = {0};

    sprintf(fullpath, "/sys/bus/i2c/devices/0-0030/version");
    OPEN_READ_FILE(fd, fullpath, r_data, nbytes, len);
    v = atoi(r_data);

    pi->cpld_versions = aim_fstrdup("%d", v);

    return 0;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);

    if(onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK) {
        if(*size == 256) {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }
    aim_free(rdata);
    *size = 0;
    return ONLP_STATUS_E_INTERNAL;
}

void
onlp_sysi_onie_data_free(uint8_t* data)
{
    aim_free(data);
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 1 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* LEDs on the chassis */
    for (i = 1; i <= chassis_led_count(); i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 1 Fans on the chassis */
    for (i = 1; i <= chassis_fan_count(); i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_manage_fans(void)
{
    int rc, oldP, newP = 0;
    onlp_thermal_info_t ti;
    int  fd, len;
    char  buf[10] = {0};
    char path[70] = "/sys/devices/platform/as4610_fan/fan_duty_cycle_percentage";

    if (chassis_fan_count() == 0) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    /* Get fan duty cycle */
    fd = open(path, O_RDWR, 0644);
    if (fd == -1){
        return ONLP_STATUS_E_INTERNAL;
    }

    len = read(fd, buf, sizeof(buf));
    if (len <= 0){
        close(fd);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (close(fd) == -1) {
        return ONLP_STATUS_E_INTERNAL;
    }
    
    DEBUG_PRINT("[Debug][%s][%d][read data: %s]\n", __FUNCTION__, __LINE__, buf);
    oldP = atoi(buf);

    /* Get temperature */
    rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(1), &ti);
    if (rc != ONLP_STATUS_OK) {
        return rc;
    }

    /* Bring fan speed to high if current speed is unexpected
     */
    if (oldP != FAN_PERCENTAGE_LOW && oldP != FAN_PERCENTAGE_HIGH) {
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_PERCENTAGE_HIGH);
        return ONLP_STATUS_OK;
    }

    if (oldP == FAN_PERCENTAGE_LOW && ti.mcelsius >= 61000) {
        newP = FAN_PERCENTAGE_HIGH;
            
    }
    else if (oldP == FAN_PERCENTAGE_HIGH && ti.mcelsius <= 49000) {
        newP = FAN_PERCENTAGE_LOW;
    }

    if (newP) {
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), newP);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

