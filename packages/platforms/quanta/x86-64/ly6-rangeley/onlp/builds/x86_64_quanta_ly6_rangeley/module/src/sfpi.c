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
 * SFPI Interface for the Quanta LY6
 *
 ***********************************************************/
#include <x86_64_quanta_ly6_rangeley/x86_64_quanta_ly6_rangeley_config.h>
#include <x86_64_quanta_ly6_rangeley/x86_64_quanta_ly6_rangeley_gpio_table.h>
#include <onlp/platformi/sfpi.h>
#include <onlplib/sfp.h>
#include <onlplib/gpio.h>
#include "x86_64_quanta_ly6_rangeley_log.h"

#include <unistd.h>
#include <fcntl.h>

/**
 * This table maps the presence gpio, reset gpio, and eeprom file
 * for each SFP port.
 */
typedef struct sfpmap_s {
    int port;
    int present_gpio;
    const char* reset_gpio;
    const char* eeprom;
    const char* dom;
} sfpmap_t;

static sfpmap_t sfpmap__[] =
    {
        /* CPLD will not always be stable, use eeprom read instead */
        {  1, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(1)  */ /* 168 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-32/32-0050/eeprom", NULL },
        {  2, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(2)  */ /* 172 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-33/33-0050/eeprom", NULL },
        {  3, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(3)  */ /* 176 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-34/34-0050/eeprom", NULL },
        {  4, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(4)  */ /* 180 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-35/35-0050/eeprom", NULL },
        {  5, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(5)  */ /* 184 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-36/36-0050/eeprom", NULL },
        {  6, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(6)  */ /* 188 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-37/37-0050/eeprom", NULL },
        {  7, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(7)  */ /* 192 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-38/38-0050/eeprom", NULL },
        {  8, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(8)  */ /* 196 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/i2c-39/39-0050/eeprom", NULL },
        {  9, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(9)  */ /* 208 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-40/40-0050/eeprom", NULL },
        { 10, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(10) */ /* 212 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-41/41-0050/eeprom", NULL },
        { 11, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(11) */ /* 216 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-42/42-0050/eeprom", NULL },
        { 12, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(12) */ /* 220 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-43/43-0050/eeprom", NULL },
        { 13, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(13) */ /* 224 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-44/44-0050/eeprom", NULL },
        { 14, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(14) */ /* 228 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-45/45-0050/eeprom", NULL },
        { 15, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(15) */ /* 232 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-46/46-0050/eeprom", NULL },
        { 16, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(16) */ /* 236 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/i2c-47/47-0050/eeprom", NULL },
        { 17, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(17) */ /* 248 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/i2c-48/48-0050/eeprom", NULL },
        { 18, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(18) */ /* 252 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/i2c-49/49-0050/eeprom", NULL },
        { 19, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(19) */ /* 256 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/i2c-50/50-0050/eeprom", NULL },
        { 20, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(20) */ /* 260 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/i2c-51/51-0050/eeprom", NULL },
        { 21, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(21) */ /* 264 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/i2c-52/52-0050/eeprom", NULL },
        { 22, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(22) */ /* 268 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/i2c-53/53-0050/eeprom", NULL },
        { 23, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(23) */ /* 272 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/i2c-54/54-0050/eeprom", NULL },
        { 24, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(24) */ /* 276 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/i2c-55/55-0050/eeprom", NULL },
        { 25, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(25) */ /* 288 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-19/i2c-56/56-0050/eeprom", NULL },
        { 26, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(26) */ /* 292 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-19/i2c-57/57-0050/eeprom", NULL },
        { 27, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(27) */ /* 296 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-19/i2c-58/58-0050/eeprom", NULL },
        { 28, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(28) */ /* 300 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-19/i2c-59/59-0050/eeprom", NULL },
        { 29, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(29) */ /* 304 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-19/i2c-60/60-0050/eeprom", NULL },
        { 30, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(30) */ /* 308 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-19/i2c-61/61-0050/eeprom", NULL },
        { 31, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(31) */ /* 312 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-19/i2c-62/62-0050/eeprom", NULL },
        { 32, -1 /* QUANTA_LY6_CPLD_QSFP_PRSNT(32) */ /* 316 */, NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-19/i2c-63/63-0050/eeprom", NULL },
  };

int
onlp_sfpi_init(void)
{
    int value = -1, ret;

    onlp_gpio_export(QUANTA_LY6_QSFP_EN_GPIO_P3V3_PW_EN, ONLP_GPIO_DIRECTION_IN);
    ret = onlp_gpio_get(QUANTA_LY6_QSFP_EN_GPIO_P3V3_PW_EN, &value);
    if(ret == ONLP_STATUS_OK && value != 1) {
        onlp_gpio_export(QUANTA_LY6_QSFP_EN_GPIO_P3V3_PW_EN, ONLP_GPIO_DIRECTION_OUT);
        ret = onlp_gpio_set(QUANTA_LY6_QSFP_EN_GPIO_P3V3_PW_EN, 1);
        sleep(1);
    }

    return ret;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;

    for(p = 0; p < 32; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

#define SFP_GET(_port) (sfpmap__ + _port)

int
onlp_sfpi_is_present(int port)
{
	int value = 0;
    sfpmap_t* sfp = SFP_GET(port);
    if(sfp->present_gpio > 0) {
		if(onlp_gpio_get(sfp->present_gpio, &value) == ONLP_STATUS_OK)
			return (value == 0);
		else
			return ONLP_STATUS_E_MISSING;
    }
    else {
        /**
         * If we can open and read a byte from the EEPROM file
         * then we consider it present.
         */
        int fd = open(sfp->eeprom, O_RDONLY);
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

