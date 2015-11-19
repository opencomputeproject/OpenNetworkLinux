/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * 
 *        Copyright 2014, 2015 Big Switch Networks, Inc.       
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
#include <onlp/onlp.h>
#include <onlplib/gpio.h>
#include <onlplib/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include "onlplib_log.h"

#define SYS_CLASS_GPIO_PATH "/sys/class/gpio/gpio%d"

int
onlp_gpio_export(int gpio, onlp_gpio_direction_t direction)
{
    int fd;
    int rv;
    fd = onlp_file_open(O_RDONLY, 0, SYS_CLASS_GPIO_PATH, gpio);
    if(fd < 0) {
        /* Not exported yet */
        char g[32];
        ONLPLIB_SNPRINTF(g, sizeof(g)-1, "%d\n", gpio);
        rv = onlp_file_write_str(g, "/sys/class/gpio/export");
        if(rv < 0) {
            AIM_LOG_ERROR("Exporting gpio %d failed.", gpio);
            return -1;
        }
    }
    close(fd);

    const char* s;
    switch(direction)
        {
        case ONLP_GPIO_DIRECTION_NONE: s = NULL; break; /* Don't set direction */
        case ONLP_GPIO_DIRECTION_IN: s = "in\n"; break;
        case ONLP_GPIO_DIRECTION_OUT: s = "out\n"; break;
        case ONLP_GPIO_DIRECTION_LOW: s = "low\n"; break;
        case ONLP_GPIO_DIRECTION_HIGH: s = "high\n"; break;
        default:
            return ONLP_STATUS_E_PARAM;
        }

    if(s) {
        rv = onlp_file_write_str(s, SYS_CLASS_GPIO_PATH "/direction", gpio);
        if(rv < 0) {
            AIM_LOG_MSG("Failed to set gpio%d direction=%s: %{errno}",
                        gpio, direction, errno);
            return -1;
        }
    }
    return 0;
}

int
onlp_gpio_set(int gpio, int v)
{
    char* str = (v) ? "1\n" : "0\n";
    return onlp_file_write((uint8_t*)str, strlen(str),
                           SYS_CLASS_GPIO_PATH "/value", gpio);
}

int
onlp_gpio_get(int gpio, int* v)
{
    return onlp_file_read_int(v, SYS_CLASS_GPIO_PATH "/value", gpio);
}

