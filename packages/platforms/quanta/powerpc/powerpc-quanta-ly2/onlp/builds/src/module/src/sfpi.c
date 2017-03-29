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
 * SFPI Interface for the Quanta LY2
 *
 ***********************************************************/
#include <powerpc_quanta_ly2/powerpc_quanta_ly2_config.h>
#include <onlp/platformi/sfpi.h>
#include <onlplib/sfp.h>
#include "powerpc_quanta_ly2_log.h"

#include <unistd.h>
#include <fcntl.h>

/**
 * This table maps the presence gpio, reset gpio, and eeprom file
 * for each SFP port.
 */
typedef struct sfpmap_s {
    int port;
    const char*  mod_present_gpio;
    const char* reset_gpio;
    const char* eeprom;
    const char* dom;
} sfpmap_t;

static sfpmap_t sfpmap__[] =
    {
        {  1, "/sys/class/gpio/gpio168/value", NULL, "/sys/bus/i2c/devices/14-0050/eeprom", "/sys/bus/i2c/devices/14-0051/eeprom" },
        {  2, "/sys/class/gpio/gpio169/value", NULL, "/sys/bus/i2c/devices/15-0050/eeprom", "/sys/bus/i2c/devices/15-0051/eeprom" },
        {  3, "/sys/class/gpio/gpio170/value", NULL, "/sys/bus/i2c/devices/16-0050/eeprom", "/sys/bus/i2c/devices/16-0051/eeprom" },
        {  4, "/sys/class/gpio/gpio171/value", NULL, "/sys/bus/i2c/devices/17-0050/eeprom", "/sys/bus/i2c/devices/17-0051/eeprom" },
        {  5, "/sys/class/gpio/gpio172/value", NULL, "/sys/bus/i2c/devices/18-0050/eeprom", "/sys/bus/i2c/devices/18-0051/eeprom" },
        {  6, "/sys/class/gpio/gpio173/value", NULL, "/sys/bus/i2c/devices/19-0050/eeprom", "/sys/bus/i2c/devices/19-0051/eeprom" },
        {  7, "/sys/class/gpio/gpio174/value", NULL, "/sys/bus/i2c/devices/20-0050/eeprom", "/sys/bus/i2c/devices/20-0051/eeprom" },
        {  8, "/sys/class/gpio/gpio175/value", NULL, "/sys/bus/i2c/devices/21-0050/eeprom", "/sys/bus/i2c/devices/21-0051/eeprom" },
        {  9, "/sys/class/gpio/gpio176/value", NULL, "/sys/bus/i2c/devices/22-0050/eeprom", "/sys/bus/i2c/devices/22-0051/eeprom" },
        { 10, "/sys/class/gpio/gpio177/value", NULL, "/sys/bus/i2c/devices/23-0050/eeprom", "/sys/bus/i2c/devices/23-0051/eeprom" },
        { 11, "/sys/class/gpio/gpio178/value", NULL, "/sys/bus/i2c/devices/24-0050/eeprom", "/sys/bus/i2c/devices/24-0051/eeprom" },
        { 12, "/sys/class/gpio/gpio179/value", NULL, "/sys/bus/i2c/devices/25-0050/eeprom", "/sys/bus/i2c/devices/25-0051/eeprom" },
        { 13, "/sys/class/gpio/gpio180/value", NULL, "/sys/bus/i2c/devices/26-0050/eeprom", "/sys/bus/i2c/devices/26-0051/eeprom" },
        { 14, "/sys/class/gpio/gpio181/value", NULL, "/sys/bus/i2c/devices/27-0050/eeprom", "/sys/bus/i2c/devices/27-0051/eeprom" },
        { 15, "/sys/class/gpio/gpio182/value", NULL, "/sys/bus/i2c/devices/28-0050/eeprom", "/sys/bus/i2c/devices/28-0051/eeprom" },
        { 16, "/sys/class/gpio/gpio183/value", NULL, "/sys/bus/i2c/devices/29-0050/eeprom", "/sys/bus/i2c/devices/29-0051/eeprom" },
        { 17, "/sys/class/gpio/gpio144/value", NULL, "/sys/bus/i2c/devices/30-0050/eeprom", "/sys/bus/i2c/devices/30-0051/eeprom" },
        { 18, "/sys/class/gpio/gpio145/value", NULL, "/sys/bus/i2c/devices/31-0050/eeprom", "/sys/bus/i2c/devices/31-0051/eeprom" },
        { 19, "/sys/class/gpio/gpio146/value", NULL, "/sys/bus/i2c/devices/32-0050/eeprom", "/sys/bus/i2c/devices/32-0051/eeprom" },
        { 20, "/sys/class/gpio/gpio147/value", NULL, "/sys/bus/i2c/devices/33-0050/eeprom", "/sys/bus/i2c/devices/33-0051/eeprom" },
        { 21, "/sys/class/gpio/gpio148/value", NULL, "/sys/bus/i2c/devices/34-0050/eeprom", "/sys/bus/i2c/devices/34-0051/eeprom" },
        { 22, "/sys/class/gpio/gpio149/value", NULL, "/sys/bus/i2c/devices/35-0050/eeprom", "/sys/bus/i2c/devices/35-0051/eeprom" },
        { 23, "/sys/class/gpio/gpio150/value", NULL, "/sys/bus/i2c/devices/36-0050/eeprom", "/sys/bus/i2c/devices/35-0051/eeprom" },
        { 24, "/sys/class/gpio/gpio151/value", NULL, "/sys/bus/i2c/devices/37-0050/eeprom", "/sys/bus/i2c/devices/37-0051/eeprom" },
        { 25, "/sys/class/gpio/gpio152/value", NULL, "/sys/bus/i2c/devices/38-0050/eeprom", "/sys/bus/i2c/devices/38-0051/eeprom" },
        { 26, "/sys/class/gpio/gpio153/value", NULL, "/sys/bus/i2c/devices/39-0050/eeprom", "/sys/bus/i2c/devices/39-0051/eeprom" },
        { 27, "/sys/class/gpio/gpio154/value", NULL, "/sys/bus/i2c/devices/40-0050/eeprom", "/sys/bus/i2c/devices/40-0051/eeprom" },
        { 28, "/sys/class/gpio/gpio155/value", NULL, "/sys/bus/i2c/devices/41-0050/eeprom", "/sys/bus/i2c/devices/41-0051/eeprom" },
        { 29, "/sys/class/gpio/gpio156/value", NULL, "/sys/bus/i2c/devices/42-0050/eeprom", "/sys/bus/i2c/devices/42-0051/eeprom" },
        { 30, "/sys/class/gpio/gpio157/value", NULL, "/sys/bus/i2c/devices/43-0050/eeprom", "/sys/bus/i2c/devices/43-0051/eeprom" },
        { 31, "/sys/class/gpio/gpio158/value", NULL, "/sys/bus/i2c/devices/44-0050/eeprom", "/sys/bus/i2c/devices/44-0051/eeprom" },
        { 32, "/sys/class/gpio/gpio159/value", NULL, "/sys/bus/i2c/devices/45-0050/eeprom", "/sys/bus/i2c/devices/45-0051/eeprom" },
        { 33, "/sys/class/gpio/gpio120/value", NULL, "/sys/bus/i2c/devices/46-0050/eeprom", "/sys/bus/i2c/devices/46-0051/eeprom" },
        { 34, "/sys/class/gpio/gpio121/value", NULL, "/sys/bus/i2c/devices/47-0050/eeprom", "/sys/bus/i2c/devices/47-0051/eeprom" },
        { 35, "/sys/class/gpio/gpio122/value", NULL, "/sys/bus/i2c/devices/48-0050/eeprom", "/sys/bus/i2c/devices/48-0051/eeprom" },
        { 36, "/sys/class/gpio/gpio123/value", NULL, "/sys/bus/i2c/devices/49-0050/eeprom", "/sys/bus/i2c/devices/49-0051/eeprom" },
        { 37, "/sys/class/gpio/gpio124/value", NULL, "/sys/bus/i2c/devices/50-0050/eeprom", "/sys/bus/i2c/devices/50-0051/eeprom" },
        { 38, "/sys/class/gpio/gpio125/value", NULL, "/sys/bus/i2c/devices/51-0050/eeprom", "/sys/bus/i2c/devices/51-0051/eeprom" },
        { 39, "/sys/class/gpio/gpio126/value", NULL, "/sys/bus/i2c/devices/52-0050/eeprom", "/sys/bus/i2c/devices/52-0051/eeprom" },
        { 40, "/sys/class/gpio/gpio127/value", NULL, "/sys/bus/i2c/devices/53-0050/eeprom", "/sys/bus/i2c/devices/53-0051/eeprom" },
        { 41, "/sys/class/gpio/gpio128/value", NULL, "/sys/bus/i2c/devices/54-0050/eeprom", "/sys/bus/i2c/devices/54-0051/eeprom" },
        { 42, "/sys/class/gpio/gpio129/value", NULL, "/sys/bus/i2c/devices/55-0050/eeprom", "/sys/bus/i2c/devices/55-0051/eeprom" },
        { 43, "/sys/class/gpio/gpio130/value", NULL, "/sys/bus/i2c/devices/56-0050/eeprom", "/sys/bus/i2c/devices/56-0051/eeprom" },
        { 44, "/sys/class/gpio/gpio131/value", NULL, "/sys/bus/i2c/devices/57-0050/eeprom", "/sys/bus/i2c/devices/57-0051/eeprom" },
        { 45, "/sys/class/gpio/gpio132/value", NULL, "/sys/bus/i2c/devices/58-0050/eeprom", "/sys/bus/i2c/devices/58-0051/eeprom" },
        { 46, "/sys/class/gpio/gpio133/value", NULL, "/sys/bus/i2c/devices/59-0050/eeprom", "/sys/bus/i2c/devices/59-0051/eeprom" },
        { 47, "/sys/class/gpio/gpio134/value", NULL, "/sys/bus/i2c/devices/60-0050/eeprom", "/sys/bus/i2c/devices/60-0051/eeprom" },
        { 48, "/sys/class/gpio/gpio135/value", NULL, "/sys/bus/i2c/devices/61-0050/eeprom", "/sys/bus/i2c/devices/61-0051/eeprom" },
        { 49,  NULL, NULL, "/sys/bus/i2c/devices/i2c-10/10-0050/eeprom", NULL },
        { 50,  NULL, NULL, "/sys/bus/i2c/devices/i2c-11/11-0050/eeprom", NULL },
        { 51,  NULL, NULL, "/sys/bus/i2c/devices/i2c-12/12-0050/eeprom", NULL },
        { 52,  NULL, NULL, "/sys/bus/i2c/devices/i2c-13/13-0050/eeprom", NULL },
  };


int
onlp_sfpi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;

    for(p = 0; p < 52; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

#define SFP_GET(_port) (sfpmap__ + _port)

int
onlp_sfpi_is_present(int port)
{
    sfpmap_t* sfp = SFP_GET(port);
    if(sfp->mod_present_gpio) {
        return onlplib_sfp_is_present_file(sfp->mod_present_gpio,
                                           /* Present */ "1\n",
                                           /* Absent */  "0\n");
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

