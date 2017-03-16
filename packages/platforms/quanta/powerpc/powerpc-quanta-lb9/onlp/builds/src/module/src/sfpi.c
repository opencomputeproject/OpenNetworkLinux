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
 * SFPI Interface for the Quanta LB9
 *
 * This code was lifted wholesale and minimally ported
 * from the previous implementation that preceded the ONLP interfaces.
 *
 * It should be ported in the future to use the services provided
 * by the ONLP infrastructure directly.
 *
 ***********************************************************/
#include <powerpc_quanta_lb9/powerpc_quanta_lb9_config.h>
#include <onlp/platformi/sfpi.h>
#include <onlplib/sfp.h>
#include <onlplib/gpio.h>
#include "powerpc_quanta_lb9_log.h"


#include <unistd.h>
#include <fcntl.h>
#include <time.h>

/**
 * This table maps the presence gpio, reset gpio, and eeprom file
 * for each SFP port.
 */
typedef struct sfpmap_s {
    int sport;
    int mod_abs_gpio_number;
    int reset_gpio_number;
    const char*  mod_abs_gpio;
    const char* reset_gpio;
    const char* eeprom;
    const char* dom;
} sfpmap_t;

static sfpmap_t sfpmap__[] =
    {
        {
            48,
            112, 120,
            "/sys/class/gpio/gpio112/value",
            "/sys/class/gpio/gpio120/value",
            "/sys/devices/e0000000.soc8541/e0003000.i2c/i2c-0/i2c-1/1-0050/eeprom",
            "/sys/devices/e0000000.soc8541/e0003000.i2c/i2c-0/i2c-1/1-0051/eeprom"
        },
        {
            49,
            113, 121,
            "/sys/class/gpio/gpio113/value",
            "/sys/class/gpio/gpio121/value",
            "/sys/devices/e0000000.soc8541/e0003000.i2c/i2c-0/i2c-2/2-0050/eeprom",
            "/sys/devices/e0000000.soc8541/e0003000.i2c/i2c-0/i2c-2/2-0051/eeprom"
        },
        {
            50,
            114, 122,
            "/sys/class/gpio/gpio114/value",
            "/sys/class/gpio/gpio122/value",
            "/sys/devices/e0000000.soc8541/e0003000.i2c/i2c-0/i2c-3/3-0050/eeprom",
            "/sys/devices/e0000000.soc8541/e0003000.i2c/i2c-0/i2c-3/3-0051/eeprom"
        },
        {
            51,
            115, 123,
            "/sys/class/gpio/gpio115/value",
            "/sys/class/gpio/gpio123/value",
            "/sys/devices/e0000000.soc8541/e0003000.i2c/i2c-0/i2c-4/4-0050/eeprom",
            "/sys/devices/e0000000.soc8541/e0003000.i2c/i2c-0/i2c-4/4-0051/eeprom"
        },
    };

#define SFP_GET(_port) (sfpmap__+ (_port - 48))

int
onlp_sfpi_init(void)
{
    /**
     * Initialize the SFP presence and reset GPIOS.
     */
    int i;
    int rv;
    for(i = 0; i < AIM_ARRAYSIZE(sfpmap__); i++) {
        if( (rv = onlp_gpio_export(sfpmap__[i].mod_abs_gpio_number, ONLP_GPIO_DIRECTION_IN)) < 0) {
            AIM_LOG_ERROR("Failed to initialize MOD_ABS gpio %d",
                          sfpmap__[i].mod_abs_gpio_number);
            return -1;
        }
        if( (rv = onlp_gpio_export(sfpmap__[i].reset_gpio_number, ONLP_GPIO_DIRECTION_HIGH)) < 0) {
            AIM_LOG_ERROR("Failed to initialize RESET gpio %d",
                          sfpmap__[i].reset_gpio_number);
            return -1;
        }
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;

    for(p = 48; p < 52; p++) {
        AIM_BITMAP_SET(bmap, p);
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    sfpmap_t* sfp = SFP_GET(port);

    return onlplib_sfp_is_present_file(sfp->mod_abs_gpio,
                                       /* Present */ "0\n",
                                       /* Absent */  "1\n");
}

int
onlp_sfpi_eeprom_read(int port, int dev_addr, uint8_t data[256])
{
    sfpmap_t* sfp = SFP_GET(port);

    if (dev_addr == SFP_IDPROM_ADDR) {
        return onlplib_sfp_eeprom_read_file(sfp->eeprom, data);
    } else if (dev_addr == SFP_DOM_ADDR) {
        return onlplib_sfp_eeprom_read_file(sfp->dom, data);
    }
    return ONLP_STATUS_E_PARAM;
}
