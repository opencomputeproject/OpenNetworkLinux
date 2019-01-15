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
 * SFPI Interface for the Quanta IX8
 *
 ***********************************************************/
#include <x86_64_quanta_ix8_rglbmc/x86_64_quanta_ix8_rglbmc_config.h>
#include <x86_64_quanta_ix8_rglbmc/x86_64_quanta_ix8_rglbmc_gpio_table.h>
#include <onlp/platformi/sfpi.h>
#include <onlplib/sfp.h>
#include <onlplib/gpio.h>
#include "x86_64_quanta_ix8_rglbmc_log.h"
#include <onlplib/file.h>
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
        {  1, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-1/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-20/i2c-32/32-0050/eeprom", NULL },
        {  2, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-2/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-20/i2c-33/33-0050/eeprom", NULL },
        {  3, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-3/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-20/i2c-34/34-0050/eeprom", NULL },
        {  4, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-4/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-20/i2c-35/35-0050/eeprom", NULL },
        {  5, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-5/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-20/i2c-36/36-0050/eeprom", NULL },
        {  6, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-6/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-20/i2c-37/37-0050/eeprom", NULL },
        {  7, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-7/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-20/i2c-38/38-0050/eeprom", NULL },
        {  8, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-8/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-20/i2c-39/39-0050/eeprom", NULL },
        {  9, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-9/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-21/i2c-40/40-0050/eeprom", NULL },
        { 10, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-10/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-21/i2c-41/41-0050/eeprom", NULL },
        { 11, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-11/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-21/i2c-42/42-0050/eeprom", NULL },
        { 12, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-12/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-21/i2c-43/43-0050/eeprom", NULL },
        { 13, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-13/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-21/i2c-44/44-0050/eeprom", NULL },
        { 14, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-14/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-21/i2c-45/45-0050/eeprom", NULL },
        { 15, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-15/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-21/i2c-46/46-0050/eeprom", NULL },
        { 16, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-16/16-0038/cpld-sfp28/port-16/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-21/i2c-47/47-0050/eeprom", NULL },
        { 17, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-17/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-22/i2c-48/48-0050/eeprom", NULL },
        { 18, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-18/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-22/i2c-49/49-0050/eeprom", NULL },
        { 19, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-19/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-22/i2c-50/50-0050/eeprom", NULL },
        { 20, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-20/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-22/i2c-51/51-0050/eeprom", NULL },
        { 21, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-21/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-22/i2c-52/52-0050/eeprom", NULL },
        { 22, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-22/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-22/i2c-53/53-0050/eeprom", NULL },
        { 23, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-23/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-22/i2c-54/54-0050/eeprom", NULL },
        { 24, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-24/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-22/i2c-55/55-0050/eeprom", NULL },
        { 25, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-25/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-23/i2c-56/56-0050/eeprom", NULL },
        { 26, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-26/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-23/i2c-57/57-0050/eeprom", NULL },
        { 27, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-27/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-23/i2c-58/58-0050/eeprom", NULL },
        { 28, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-28/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-23/i2c-59/59-0050/eeprom", NULL },
        { 29, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-29/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-23/i2c-60/60-0050/eeprom", NULL },
        { 30, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-30/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-23/i2c-61/61-0050/eeprom", NULL },
        { 31, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-31/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-23/i2c-62/62-0050/eeprom", NULL },
        { 32, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-17/17-0038/cpld-sfp28/port-32/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-23/i2c-63/63-0050/eeprom", NULL },
        { 33, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-33/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-24/i2c-64/64-0050/eeprom", NULL },
        { 34, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-34/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-24/i2c-65/65-0050/eeprom", NULL },
        { 35, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-35/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-24/i2c-66/66-0050/eeprom", NULL },
        { 36, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-36/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-24/i2c-67/67-0050/eeprom", NULL },
        { 37, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-37/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-24/i2c-68/68-0050/eeprom", NULL },
        { 38, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-38/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-24/i2c-69/69-0050/eeprom", NULL },
        { 39, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-39/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-24/i2c-70/70-0050/eeprom", NULL },
        { 40, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-40/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-24/i2c-71/71-0050/eeprom", NULL },
        { 41, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-41/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-25/i2c-72/72-0050/eeprom", NULL },
        { 42, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-42/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-25/i2c-73/73-0050/eeprom", NULL },
        { 43, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-43/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-25/i2c-74/74-0050/eeprom", NULL },
        { 44, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-44/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-25/i2c-75/75-0050/eeprom", NULL },
        { 45, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-45/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-25/i2c-76/76-0050/eeprom", NULL },
        { 46, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-46/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-25/i2c-77/77-0050/eeprom", NULL },
        { 47, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-47/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-25/i2c-78/78-0050/eeprom", NULL },
        { 48, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-18/18-0038/cpld-sfp28/port-48/%s", NULL, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-25/i2c-79/79-0050/eeprom", NULL },
  };

typedef struct qsfpmap_s {
    int port;
    int present_gpio;
    int reset_gpio;
    int lplmod_gpio;
    const char* eeprom;
    const char* dom;
} qsfpmap_t;

static qsfpmap_t qsfpmap__[] =
    {
        { 49, QUANTA_IX8_PCA9698_2_GPIO_QSFP_49_PRSNT_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_49_RESET_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_49_LPMOD_P, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-26/i2c-80/80-0050/eeprom", NULL },
        { 50, QUANTA_IX8_PCA9698_2_GPIO_QSFP_50_PRSNT_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_50_RESET_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_50_LPMOD_P, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-26/i2c-81/81-0050/eeprom", NULL },
        { 51, QUANTA_IX8_PCA9698_2_GPIO_QSFP_51_PRSNT_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_51_RESET_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_51_LPMOD_P, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-26/i2c-82/82-0050/eeprom", NULL },
        { 52, QUANTA_IX8_PCA9698_2_GPIO_QSFP_52_PRSNT_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_52_RESET_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_52_LPMOD_P, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-26/i2c-83/83-0050/eeprom", NULL },
        { 53, QUANTA_IX8_PCA9698_2_GPIO_QSFP_53_PRSNT_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_53_RESET_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_53_LPMOD_P, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-26/i2c-84/84-0050/eeprom", NULL },
        { 54, QUANTA_IX8_PCA9698_2_GPIO_QSFP_54_PRSNT_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_54_RESET_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_54_LPMOD_P, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-26/i2c-85/85-0050/eeprom", NULL },
        { 55, QUANTA_IX8_PCA9698_2_GPIO_QSFP_55_PRSNT_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_55_RESET_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_55_LPMOD_P, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-26/i2c-86/86-0050/eeprom", NULL },
        { 56, QUANTA_IX8_PCA9698_2_GPIO_QSFP_56_PRSNT_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_56_RESET_N, QUANTA_IX8_PCA9698_2_GPIO_QSFP_56_LPMOD_P, "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-26/i2c-87/87-0050/eeprom", NULL },
  };

#define SFP_GET(_port) (sfpmap__ + _port - 1)
#define QSFP_GET(_port) (qsfpmap__ + _port - 49)
#define MAX_SFP_PATH 	128
static char sfp_node_path[MAX_SFP_PATH] = {0};

static char*
sfp_get_port_path(int port, char *node_name)
{
    sfpmap_t* sfp = SFP_GET(port);

    sprintf(sfp_node_path, sfp->present_cpld,
                           node_name);
    return sfp_node_path;
}

int
onlp_sfpi_init(void)
{
    int ret, i;
    qsfpmap_t* qsfp;

    onlp_gpio_export(QUANTA_IX8_ZQSFP_EN_GPIO_P3V3_PW_EN, ONLP_GPIO_DIRECTION_OUT);
    ret = onlp_gpio_set(QUANTA_IX8_ZQSFP_EN_GPIO_P3V3_PW_EN, 1);
    sleep(1);

    for(i = 49; i < 57 ; i ++) {
        qsfp = QSFP_GET(i);
        onlp_gpio_export(qsfp->present_gpio, ONLP_GPIO_DIRECTION_IN);
        onlp_gpio_export(qsfp->reset_gpio, ONLP_GPIO_DIRECTION_OUT);
        onlp_gpio_set(qsfp->reset_gpio, 1);
        onlp_gpio_export(qsfp->lplmod_gpio, ONLP_GPIO_DIRECTION_OUT);
        onlp_gpio_set(qsfp->lplmod_gpio, 0);
    }

    return ret;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;

    for(p = 1; p < 57; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    if(port > 48){
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
        return onlplib_sfp_is_present_file(sfp_get_port_path(port, "pre_n"), /* Present */ "1\n", /* Absent */  "0\n");
    }
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    if(port > 48){
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
    if(port > 48){
        qsfpmap_t* qsfp = QSFP_GET(port);
        return onlplib_sfp_eeprom_read_file(qsfp->dom, data);
    }
    else{
        sfpmap_t* sfp = SFP_GET(port);
        return onlplib_sfp_eeprom_read_file(sfp->dom, data);
    }
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rv;

    if(port > 48){
        qsfpmap_t* qsfp = QSFP_GET(port);
        switch(control){
            case ONLP_SFP_CONTROL_RESET_STATE:
                {
                    if(onlp_gpio_set(qsfp->reset_gpio, value) == ONLP_STATUS_OK){
                        rv = ONLP_STATUS_OK;
                    }
                    else{
                        AIM_LOG_ERROR("Unable to set reset status to port(%d)\r\n", port);
                        rv = ONLP_STATUS_E_INTERNAL;
                    }
                    break;
                }

            case ONLP_SFP_CONTROL_LP_MODE:
                {
                    if(onlp_gpio_set(qsfp->lplmod_gpio, value) == ONLP_STATUS_OK){
                        rv = ONLP_STATUS_OK;
                    }
                    else{
                        AIM_LOG_ERROR("Unable to set lp_mode status to port(%d)\r\n", port);
                        rv = ONLP_STATUS_E_INTERNAL;
                    }
                    break;
                }

            default:
                rv = ONLP_STATUS_E_UNSUPPORTED;
        }
    }
    else{
        switch(control){
            case ONLP_SFP_CONTROL_TX_DISABLE:
                {
                    char* path = sfp_get_port_path(port, "tx_dis");

                    if (onlp_file_write_int(value, path) != 0) {
                        AIM_LOG_ERROR("Unable to set tx_disable status to port(%d)\r\n", port);
                        rv = ONLP_STATUS_E_INTERNAL;
                    }
                    else {
                        rv = ONLP_STATUS_OK;
                    }
                    break;
                }

            default:
                rv = ONLP_STATUS_E_UNSUPPORTED;
                break;
        }
    }

    return rv;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int rv;
    char* path = NULL;

    if(port > 48){
        qsfpmap_t* qsfp = QSFP_GET(port);

        switch(control){
            case ONLP_SFP_CONTROL_RESET_STATE:
                {
                    if(onlp_gpio_get(qsfp->reset_gpio, value) == ONLP_STATUS_OK){
                        if(*value == 0){
                            *value = 1;
                        }
                        else{
                            *value = 0;
                        }
                        rv = ONLP_STATUS_OK;
                    }
                    else{
                        AIM_LOG_ERROR("Unable to read reset status from port(%d)\r\n", port);
                        rv = ONLP_STATUS_E_INTERNAL;
                    }
                    break;
                }

            case ONLP_SFP_CONTROL_LP_MODE:
                {
                    if(onlp_gpio_get(qsfp->lplmod_gpio, value) == ONLP_STATUS_OK){
                        rv = ONLP_STATUS_OK;
                    }
                    else{
                        AIM_LOG_ERROR("Unable to read lp_mode status from port(%d)\r\n", port);
                        rv = ONLP_STATUS_E_INTERNAL;
                    }
                    break;
                }

            case ONLP_SFP_CONTROL_RX_LOS:
                {
                    *value = 0;
                    rv = ONLP_STATUS_OK;
                    break;
                }

            case ONLP_SFP_CONTROL_TX_DISABLE:
                {
                    *value = 0;
                    rv = ONLP_STATUS_OK;
                    break;
                }

            default:
                rv = ONLP_STATUS_E_UNSUPPORTED;
        }
    }
    else{
        switch(control){
            case ONLP_SFP_CONTROL_RX_LOS:
                {
                    path = sfp_get_port_path(port, "rx_los");

                    if (onlp_file_read_int(value, path) < 0) {
                        AIM_LOG_ERROR("Unable to read rx_los status from port(%d)\r\n", port);
                        rv = ONLP_STATUS_E_INTERNAL;
                    }
                    else {
                        rv = ONLP_STATUS_OK;
                    }
                    break;
                }

            case ONLP_SFP_CONTROL_TX_FAULT:
                {
                    path = sfp_get_port_path(port, "tx_fault");

                    if (onlp_file_read_int(value, path) < 0) {
                        AIM_LOG_ERROR("Unable to read tx_fault status from port(%d)\r\n", port);
                        rv = ONLP_STATUS_E_INTERNAL;
                    }
                    else {
                        rv = ONLP_STATUS_OK;
                    }
                    break;
                }

            case ONLP_SFP_CONTROL_TX_DISABLE:
                {
                    path = sfp_get_port_path(port, "tx_dis");

                    if (onlp_file_read_int(value, path) < 0) {
                        AIM_LOG_ERROR("Unable to read tx_disable status from port(%d)\r\n", port);
                        rv = ONLP_STATUS_E_INTERNAL;
                    }
                    else {
                        if(*value == 0){
                            *value = 1;
                        }
                        else{
                            *value = 0;
                        }
                        rv = ONLP_STATUS_OK;
                    }
                    break;
                }

            default:
                rv = ONLP_STATUS_E_UNSUPPORTED;
        }
    }

    return rv;
}
