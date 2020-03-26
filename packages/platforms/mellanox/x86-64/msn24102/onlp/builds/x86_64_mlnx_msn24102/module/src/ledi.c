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
 *
 *
 ***********************************************************/
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include <onlp/platformi/ledi.h>
#include "platform_lib.h"
#include <mlnx_common/mlnx_common.h>

static char* file_names[] =  /* must map with onlp_led_id */
{
    "reserved",
    "status",
    "fan1",
    "fan2",
    "fan3",
    "fan4",
    "psu"
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_SYSTEM), "Chassis LED 1 (SYSTEM LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING |
        ONLP_LED_CAPS_RED | ONLP_LED_CAPS_RED_BLINKING | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN1), "Chassis LED 2 (FAN1 (1-2) )", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING |
        ONLP_LED_CAPS_RED | ONLP_LED_CAPS_RED_BLINKING | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN2), "Chassis LED 3 (FAN2 (3-4)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING |
        ONLP_LED_CAPS_RED | ONLP_LED_CAPS_RED_BLINKING | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN3), "Chassis LED 4 (FAN3 (5-6) )", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING |
        ONLP_LED_CAPS_RED | ONLP_LED_CAPS_RED_BLINKING | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN4), "Chassis LED 5 (FAN4 (7-8) )", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING |
        ONLP_LED_CAPS_RED | ONLP_LED_CAPS_RED_BLINKING | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU), "Chassis LED 6 (PSU LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING |
        ONLP_LED_CAPS_RED | ONLP_LED_CAPS_RED_BLINKING | ONLP_LED_CAPS_AUTO,
    }
};

int
onlp_ledi_init(void)
{
    mlnx_platform_info_t* mlnx_platform_info = get_platform_info();
    mlnx_platform_info->linfo = linfo;
    mlnx_platform_info->led_fnames = file_names;
    return ONLP_STATUS_OK;
}
