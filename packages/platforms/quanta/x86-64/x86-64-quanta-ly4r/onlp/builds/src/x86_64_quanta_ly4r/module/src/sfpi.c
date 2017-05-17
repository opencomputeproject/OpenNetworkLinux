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
 * SFPI Interface for the Quanta LY4R
 *
 ***********************************************************/
#include <x86_64_quanta_ly4r/x86_64_quanta_ly4r_config.h>
#include <x86_64_quanta_ly4r/x86_64_quanta_ly4r_gpio_table.h>
#include <onlp/platformi/sfpi.h>
#include <onlplib/sfp.h>
#include <onlplib/gpio.h>
#include "x86_64_quanta_ly4r_log.h"

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
        {  49, QUANTA_LY4R_PCA9698_GPIO_SFP_1_PRSNT_N, NULL, "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-32/32-0050/eeprom", NULL },
        {  50, QUANTA_LY4R_PCA9698_GPIO_SFP_2_PRSNT_N, NULL, "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-33/33-0050/eeprom", NULL },
        {  51, QUANTA_LY4R_PCA9698_GPIO_SFP_3_PRSNT_N, NULL, "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-34/34-0050/eeprom", NULL },
        {  52, QUANTA_LY4R_PCA9698_GPIO_SFP_4_PRSNT_N, NULL, "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-35/35-0050/eeprom", NULL },
        {  53, QUANTA_LY4R_PCA9698_GPIO_SFP_5_PRSNT_N, NULL, "/sys/devices/pci0000:00/0000:00:13.0/i2c-1/i2c-36/36-0050/eeprom", NULL },
  };

#define SFP_GET(_port) (sfpmap__ + _port - 49)

int
onlp_sfpi_init(void)
{
    int value = -1, ret, i;
    sfpmap_t* sfp;

    onlp_gpio_export(QUANTA_LY4R_PCA9698_GPIO_SFP_P3V3_PW_EN, ONLP_GPIO_DIRECTION_IN);
    ret = onlp_gpio_get(QUANTA_LY4R_PCA9698_GPIO_SFP_P3V3_PW_EN, &value);
    if(ret == ONLP_STATUS_OK && value != 0) {
        onlp_gpio_export(QUANTA_LY4R_PCA9698_GPIO_SFP_P3V3_PW_EN, ONLP_GPIO_DIRECTION_OUT);
        ret = onlp_gpio_set(QUANTA_LY4R_PCA9698_GPIO_SFP_P3V3_PW_EN, 0);
        sleep(1);
    }

    for(i = 49; i < 54; i++) {
        sfp = SFP_GET(i);
        onlp_gpio_export(sfp->present_gpio, ONLP_GPIO_DIRECTION_IN);
    }

    return ret;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;

    for(p = 49; p < 54; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

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

