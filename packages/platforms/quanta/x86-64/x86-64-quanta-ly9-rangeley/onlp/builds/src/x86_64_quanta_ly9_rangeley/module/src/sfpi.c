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
 * SFPI Interface for the Quanta LY9
 *
 ***********************************************************/
#include <x86_64_quanta_ly9_rangeley/x86_64_quanta_ly9_rangeley_config.h>
#include <x86_64_quanta_ly9_rangeley/x86_64_quanta_ly9_rangeley_gpio_table.h>
#include <onlp/platformi/sfpi.h>
#include <onlplib/sfp.h>
#include <onlplib/gpio.h>
#include "x86_64_quanta_ly9_rangeley_log.h"

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
        { 49, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/0-003a/cpld-qsfp/port-1/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-21/i2c-32/32-0050/eeprom", NULL },
        { 50, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/0-003a/cpld-qsfp/port-2/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-21/i2c-33/33-0050/eeprom", NULL },
        { 51, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/0-003a/cpld-qsfp/port-3/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-21/i2c-34/34-0050/eeprom", NULL },
        { 52, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/0-003a/cpld-qsfp/port-4/module_present", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-21/i2c-35/35-0050/eeprom", NULL },
  };

typedef struct qsfpmap_s {
    int port;
    int present_gpio;
    const char* reset_gpio;
    const char* eeprom;
    const char* dom;
} qsfpmap_t;

static qsfpmap_t qsfpmap__[] =
    {
        { 53, QUANTA_LY9_QSFP_QDB_GPIO_PRSNT_53_N, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-23/i2c-48/48-0050/eeprom", NULL },
        { 54, QUANTA_LY9_QSFP_QDB_GPIO_PRSNT_54_N, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-23/i2c-49/49-0050/eeprom", NULL },
  };

#define SFP_GET(_port) (sfpmap__ + _port - 49)
#define QSFP_GET(_port) (qsfpmap__ + _port - 53)

int
onlp_sfpi_init(void)
{
    int value = -1, ret, i;
    qsfpmap_t* qsfp;

    onlp_gpio_export(QUANTA_LY9_QSFP_EN_GPIO_P3V3_PW_EN, ONLP_GPIO_DIRECTION_IN);
    ret = onlp_gpio_get(QUANTA_LY9_QSFP_EN_GPIO_P3V3_PW_EN, &value);
    if(ret == ONLP_STATUS_OK && value != 0) {
        onlp_gpio_export(QUANTA_LY9_QSFP_EN_GPIO_P3V3_PW_EN, ONLP_GPIO_DIRECTION_OUT);
        ret = onlp_gpio_set(QUANTA_LY9_QSFP_EN_GPIO_P3V3_PW_EN, 0);
    }
    if(ret == ONLP_STATUS_OK) {
        onlp_gpio_export(QUANTA_LY9_QSFP_QDB_GPIO_MOD_EN_N, ONLP_GPIO_DIRECTION_IN);
        ret = onlp_gpio_get(QUANTA_LY9_QSFP_QDB_GPIO_MOD_EN_N, &value);
        if(ret == ONLP_STATUS_OK && value != 0) {
            onlp_gpio_export(QUANTA_LY9_QSFP_QDB_GPIO_MOD_EN_N, ONLP_GPIO_DIRECTION_OUT);
            ret = onlp_gpio_set(QUANTA_LY9_QSFP_QDB_GPIO_MOD_EN_N, 0);
        }
    }

    for(i = 53; i < 55; i++) {
        qsfp = QSFP_GET(i);
        onlp_gpio_export(qsfp->present_gpio, ONLP_GPIO_DIRECTION_IN);
    }

    return ret;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;

    for(p = 49; p < 55; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{

    if(port > 52){
        int value = 0;
        qsfpmap_t* qsfp = QSFP_GET(port);

        if(qsfp->present_gpio > 0) {
            if(onlp_gpio_get(qsfp->present_gpio, &value) == ONLP_STATUS_OK)
                return (value == 0);
            else
                return ONLP_STATUS_E_MISSING;
        }
        else {
            /**
             * If we can open and read a byte from the EEPROM file
             * then we consider it present.
             */
            int fd = open(qsfp->eeprom, O_RDONLY);
            if (fd < 0) {
                /* Not Present */
                return 0;
            }
            int rv;
            uint8_t byte;

            if(read(fd, &byte, 1) == 1) {
                /* Present */
                rv = 1;
            }
            else {
                /* No Present */
                rv = 0;
            }
            close(fd);
            return rv;
        }
    }
    else{
        sfpmap_t* sfp = SFP_GET(port);

        return onlplib_sfp_is_present_file(sfp->present_cpld, /* Present */ "1\n", /* Absent */  "0\n");
    }
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{

    if(port > 52){
        qsfpmap_t* qsfp = QSFP_GET(port);

        return onlplib_sfp_eeprom_read_file(qsfp->eeprom, data);
    }
    else{
        sfpmap_t* sfp = SFP_GET(port);

        return onlplib_sfp_eeprom_read_file(sfp->eeprom, data);
    }
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    if(port > 52){
        qsfpmap_t* qsfp = QSFP_GET(port);
        return onlplib_sfp_eeprom_read_file(qsfp->dom, data);
    }
    else{
        sfpmap_t* sfp = SFP_GET(port);
        return onlplib_sfp_eeprom_read_file(sfp->dom, data);
    }
}

