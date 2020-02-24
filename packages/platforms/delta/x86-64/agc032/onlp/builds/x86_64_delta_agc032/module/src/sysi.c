/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2019 Delta Electronics, Inc.
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
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <onlp/platformi/sysi.h>
#include "x86_64_delta_agc032_int.h"
#include "x86_64_delta_agc032_log.h"
#include "vendor_driver_pool.h"
#include "vendor_i2c_device_list.h"

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-delta-agc032-r0";
}

int
onlp_sysi_init(void)
{
    char rv_char[256];
    vendor_system_call_set("ipmitool raw 0x38 0x0a 0x01 > /dev/null");
    // printf("%s\n", "BMC Sensor Monitor Disabled.");
    vendor_system_call_set("ls /sys/bus/i2c/devices/0-0053/eeprom > /dev/null 2>&1");
    if (vendor_system_call_get("echo $?", rv_char) == 0) {
        vendor_system_call_set("echo -n \"0-0053\" > /sys/bus/i2c/drivers/eeprom/unbind  > /dev/null 2>&1");
        // printf("%s\n", "Unbind EEPROM 0-0053.");
    }
    vendor_driver_init();
    // printf("%s\n", "Vendor Driver Init.");
    return ONLP_STATUS_OK;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    int id = 0, rv = 0;
    uint8_t* rdata = aim_zmalloc(256);
    // uint16_t alen = 0x01;

    void *busDrv = (void *)vendor_find_driver_by_name(eeprom_dev_list[id].bus_drv_name);
    eeprom_dev_driver_t *eeprom =
        (eeprom_dev_driver_t *)vendor_find_driver_by_name(eeprom_dev_list[id].dev_drv_name);

    vendor_dev_do_oc(eeprom_o_list[id]);
    rv = eeprom->load(
        busDrv,
        eeprom_dev_list[id].bus,
        eeprom_dev_list[id].addr,
        rdata);
    vendor_dev_do_oc(eeprom_c_list[id]);

    if(rv < 0) {
        return ONLP_STATUS_E_INVALID;
    }

    *data = rdata;

    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    vendor_dev_t cpld_dev_ver_list[] =
    {
        {"CPLD-1", "I2C", "CPLD", 0x00, 0x31, 0x01},    // CPUPLD
        {"CPLD-2", "I2C", "CPLD", 0x00, 0x27, 0x00},    // PCA9555 (no version)
        {"CPLD-3", "I2C", "CPLD", 0x00, 0x32, 0x03},    // SWPLD1
        {"CPLD-4", "I2C", "CPLD", 0x00, 0x34, 0x01},    // SWPLD2
        {"CPLD-5", "I2C", "CPLD", 0x00, 0x35, 0x01},    // SWPLD3
    };

    int rv = 0, cpld_idx = 0;
    int cpld_version[cpld_list_size] ;
    uint8_t version_data = 0;

    void *busDrv = (void *)vendor_find_driver_by_name("I2C");
    cpld_dev_driver_t *cpld = (cpld_dev_driver_t *)vendor_find_driver_by_name("CPLD");

    for(cpld_idx = 0; cpld_idx < cpld_list_size; cpld_idx++)
    {
        vendor_dev_do_oc(cpld_o_list[cpld_idx]);
        rv = cpld->readb(
            busDrv,
            cpld_dev_ver_list[cpld_idx].bus,
            cpld_dev_ver_list[cpld_idx].addr,
            cpld_dev_ver_list[cpld_idx].id, // offset
            &version_data);
        vendor_dev_do_oc(cpld_c_list[cpld_idx]);

        cpld_version[cpld_idx] = (int)(version_data & 0xff);

        if(rv < 0)
        {
            return ONLP_STATUS_E_INTERNAL;
        }

    }

    pi->cpld_versions = aim_fstrdup(
        "\n\tSystem CPLD Versions: 0x%02x \n\tMISC CPLD Versions: 0x%02x \n\tPort 1-16 CPLD Versions: 0x%02x \n\tPort 17-32 CPLD Versions: 0x%02x \n",
        cpld_version[0], cpld_version[2], cpld_version[3], cpld_version[4]);


    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{
    uint8_t data[256] = {0};
    int rv = 0, id = 0;
    void *busDrv = (void *)vendor_find_driver_by_name(eeprom_dev_list[id].bus_drv_name);
    eeprom_dev_driver_t *eeprom =
        (eeprom_dev_driver_t *)vendor_find_driver_by_name(eeprom_dev_list[id].dev_drv_name);

    if(onie == NULL) return 0;

    vendor_dev_do_oc(eeprom_o_list[id]);
    rv = eeprom->load(
        busDrv,
        eeprom_dev_list[id].bus,
        eeprom_dev_list[id].addr,
        data);
    vendor_dev_do_oc(eeprom_c_list[id]);

    rv = onlp_onie_decode(onie, data, 256);

    if(rv < 0) {
        return ONLP_STATUS_E_INVALID;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    for (i = 1; i <= psu_list_size; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    for (i = 1; i <= thermal_list_size; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    for (i = 1; i <= sysled_list_size; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    for (i = 1; i <= fan_list_size; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return ONLP_STATUS_OK;
}



