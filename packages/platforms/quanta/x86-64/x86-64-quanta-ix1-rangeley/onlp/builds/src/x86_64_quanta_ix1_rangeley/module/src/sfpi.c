/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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
 * SFPI Interface for the Quanta IX1
 *
 ***********************************************************/
#include <x86_64_quanta_ix1_rangeley/x86_64_quanta_ix1_rangeley_config.h>
#include <x86_64_quanta_ix1_rangeley/x86_64_quanta_ix1_rangeley_gpio_table.h>
#include <onlp/platformi/sfpi.h>
#include <onlplib/sfp.h>
#include <onlplib/gpio.h>
#include "x86_64_quanta_ix1_rangeley_log.h"

#include <unistd.h>
#include <fcntl.h>

/**
 * This table maps the presence gpio, reset gpio, and eeprom file
 * for each SFP port.
 */
typedef struct sfpmap_s {
    int port;
    const char* present_cpld;
    const char* reset_gpio;
    const char* eeprom;
    const char* dom;
} sfpmap_t;

static sfpmap_t sfpmap__[] =
    {
        {  1, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-1/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-32/32-0050/eeprom", NULL },
        {  2, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-2/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-33/33-0050/eeprom", NULL },
        {  3, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-3/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-34/34-0050/eeprom", NULL },
        {  4, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-4/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-35/35-0050/eeprom", NULL },
        {  5, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-5/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-36/36-0050/eeprom", NULL },
        {  6, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-6/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-37/37-0050/eeprom", NULL },
        {  7, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-7/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-38/38-0050/eeprom", NULL },
        {  8, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-8/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-39/39-0050/eeprom", NULL },
        {  9, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-9/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-40/40-0050/eeprom", NULL },
        { 10, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-10/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-41/41-0050/eeprom", NULL },
        { 11, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-11/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-42/42-0050/eeprom", NULL },
        { 12, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-12/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-43/43-0050/eeprom", NULL },
        { 13, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-13/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-44/44-0050/eeprom", NULL },
        { 14, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-14/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-45/45-0050/eeprom", NULL },
        { 15, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-15/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-46/46-0050/eeprom", NULL },
        { 16, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-qsfp28/port-16/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-47/47-0050/eeprom", NULL },
        { 17, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-17/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-48/48-0050/eeprom", NULL },
        { 18, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-18/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-49/49-0050/eeprom", NULL },
        { 19, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-19/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-50/50-0050/eeprom", NULL },
        { 20, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-20/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-51/51-0050/eeprom", NULL },
        { 21, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-21/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-52/52-0050/eeprom", NULL },
        { 22, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-22/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-53/53-0050/eeprom", NULL },
        { 23, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-23/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-54/54-0050/eeprom", NULL },
        { 24, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-24/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-55/55-0050/eeprom", NULL },
        { 25, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-25/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-56/56-0050/eeprom", NULL },
        { 26, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-26/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-57/57-0050/eeprom", NULL },
        { 27, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-27/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-58/58-0050/eeprom", NULL },
        { 28, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-28/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-59/59-0050/eeprom", NULL },
        { 29, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-29/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-60/60-0050/eeprom", NULL },
        { 30, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-30/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-61/61-0050/eeprom", NULL },
        { 31, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-31/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-62/62-0050/eeprom", NULL },
        { 32, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0039/cpld-qsfp28/port-32/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-63/63-0050/eeprom", NULL },
  };

int
onlp_sfpi_init(void)
{
    int value = -1, ret;

    onlp_gpio_export(QUANTA_IX1_ZQSFP_EN_GPIO_P3V3_PW_EN, ONLP_GPIO_DIRECTION_IN);
    ret = onlp_gpio_get(QUANTA_IX1_ZQSFP_EN_GPIO_P3V3_PW_EN, &value);
    if(ret == ONLP_STATUS_OK && value != 1) {
        onlp_gpio_export(QUANTA_IX1_ZQSFP_EN_GPIO_P3V3_PW_EN, ONLP_GPIO_DIRECTION_OUT);
        ret = onlp_gpio_set(QUANTA_IX1_ZQSFP_EN_GPIO_P3V3_PW_EN, 1);
        sleep(1);
    }

    return ret;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;

    for(p = 1; p < 33; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

#define SFP_GET(_port) (sfpmap__ + _port - 1)

int
onlp_sfpi_is_present(int port)
{
    sfpmap_t* sfp = SFP_GET(port);

    return onlplib_sfp_is_present_file(sfp->present_cpld, /* Present */ "1\n", /* Absent */  "0\n");
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    sfpmap_t* sfp = SFP_GET(port);
    return onlplib_sfp_eeprom_read_file(sfp->eeprom, data);
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    sfpmap_t* sfp = SFP_GET(port);
    return onlplib_sfp_eeprom_read_file(sfp->dom, data);
}

