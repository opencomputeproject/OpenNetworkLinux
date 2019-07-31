/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2015 Accton Technology Corporation.
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
#include <onlp/platformi/ledi.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>

#include <onlplib/mmap.h>

//#include "onlpie_int.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/* LED related data
 */
enum led_light_mode { /*must be the same with the definition @ kernel driver */
    LED_MODE_OFF = 0,
    LED_MODE_GREEN,
    LED_MODE_AMBER,
    LED_MODE_RED,
    LED_MODE_GREEN_BLINK,
    LED_MODE_AMBER_BLINK,
    LED_MODE_RED_BLINK,
    LED_MODE_AUTO,
};

int led_light_map_mode[][2] =
{
    {LED_MODE_OFF,         ONLP_LED_MODE_OFF},
    {LED_MODE_GREEN,       ONLP_LED_MODE_GREEN},
    {LED_MODE_AMBER,       ONLP_LED_MODE_ORANGE},
    {LED_MODE_RED,         ONLP_LED_MODE_RED},
    {LED_MODE_GREEN_BLINK, ONLP_LED_MODE_GREEN_BLINKING},
    {LED_MODE_AMBER_BLINK, ONLP_LED_MODE_ORANGE_BLINKING},
    {LED_MODE_RED_BLINK,   ONLP_LED_MODE_RED_BLINKING},
    {LED_MODE_AUTO,        ONLP_LED_MODE_AUTO},
};


#define prefix_path "/sys/class/leds/accton_as5812_54x_led::"
#define filename    "brightness"

enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_DIAG,
    LED_FAN,
    LED_LOC,
    LED_PSU1,
    LED_PSU2,
    LED_FAN1,
    LED_FAN2,
    LED_FAN3,
    LED_FAN4,
    LED_FAN5,
};

static char last_path[][10] =  /* must map with onlp_led_id */
{
    "reserved",
    "diag",
    "fan",
    "loc",
    "psu1",
    "psu2",
    "fan1",
    "fan2",
    "fan3",
    "fan4",
    "fan5",
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_DIAG), "Chassis LED 1 (DIAG LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "Chassis LED 2 (FAN LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_LOC), "Chassis LED 3 (LOC LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU1), "Chassis LED 4 (PSU1 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU2), "Chassis LED 5 (PSU2 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN1), "Chassis LED 6 (FAN1 LED)", 0 },
        ONLP_LED_STATUS_PRESENT, 0,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN2), "Chassis LED 7 (FAN2 LED)", 0 },
        ONLP_LED_STATUS_PRESENT, 0,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN3), "Chassis LED 8 (FAN3 LED)", 0 },
        ONLP_LED_STATUS_PRESENT, 0,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN4), "Chassis LED 9 (FAN4 LED)", 0 },
        ONLP_LED_STATUS_PRESENT, 0,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN5), "Chassis LED 10 (FAN5 LED)", 0 },
        ONLP_LED_STATUS_PRESENT, 0,
    },
};


static int conver_led_light_mode_to_onl(int led_ligth_mode)
{
    int i, nsize = sizeof(led_light_map_mode)/sizeof(led_light_map_mode[0]);
    for(i=0; i<nsize; i++)
    {
        if (led_ligth_mode == led_light_map_mode[i][0])
        {
            return led_light_map_mode[i][1];
        }
    }
    return 0;
}


static int conver_led_light_mode_to_driver(int led_ligth_mode)
{
    int i, nsize = sizeof(led_light_map_mode)/sizeof(led_light_map_mode[0]);
    for(i=0; i<nsize; i++)
    {
        if (led_ligth_mode == led_light_map_mode[i][1])
        {
            return led_light_map_mode[i][0];
        }
    }
    return 0;
}

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    /*
     * Turn off the LOCATION and DIAG LEDs at startup
     */
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_LOC), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_DIAG), ONLP_LED_MODE_OFF);
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{

    int  fd, len, nbytes=1, local_id;
	char data[2] = {0};
    char fullpath[PATH_MAX] = {0};

    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    /* get fullpath */
    if (strchr(last_path[local_id], '/') != NULL)
	{
        sprintf(fullpath, "%s%s", prefix_path, last_path[local_id]);
	}
	else
	{
        sprintf(fullpath, "%s%s/%s", prefix_path, last_path[local_id], filename);
    }

	/* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    /* Set current mode */
    if ((fd = open(fullpath, O_RDONLY)) == -1)
    {
        return ONLP_STATUS_E_INTERNAL;
    }

    if ((len = read(fd, data, nbytes)) <= 0)
    {
        close(fd);
        return ONLP_STATUS_E_INTERNAL;
    }

    /* If the read byte count is less, the format is different and calc will be wrong*/
    if (close(fd) == -1)
    {
      return ONLP_STATUS_E_INTERNAL;
    }

    info->mode = conver_led_light_mode_to_onl(atoi(data));

    /* Set the on/off status */
    if (info->mode != ONLP_LED_MODE_OFF) {
        info->status |= ONLP_LED_STATUS_ON;
    }

    return ONLP_STATUS_OK;
}

/*
 * Turn an LED on or off.
 *
 * This function will only be called if the LED OID supports the ONOFF
 * capability.
 *
 * What 'on' means in terms of colors or modes for multimode LEDs is
 * up to the platform to decide. This is intended as baseline toggle mechanism.
 */
int
onlp_ledi_set(onlp_oid_t id, int on_or_off)
{
    VALIDATE(id);

    if (!on_or_off) {
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
    }

    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * This function puts the LED into the given mode. It is a more functional
 * interface for multimode LEDs.
 *
 * Only modes reported in the LED's capabilities will be attempted.
 */
int
onlp_ledi_mode_set(onlp_oid_t id, onlp_led_mode_t mode)
{
    int  fd, len, driver_mode, nbytes=1, local_id;
    char data[2] = {0};
    char fullpath[PATH_MAX] = {0};

    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    /* get fullpath */
    if (strchr(last_path[local_id], '/') != NULL)
	{
        sprintf(fullpath, "%s%s", prefix_path, last_path[local_id]);
	}
	else
	{
        sprintf(fullpath, "%s%s/%s", prefix_path, last_path[local_id], filename);
    }

	driver_mode = conver_led_light_mode_to_driver(mode);
    sprintf(data, "%d", driver_mode);

    /* Create output file descriptor */
    fd = open(fullpath, O_WRONLY, 0644);
    if(fd == -1){
        return ONLP_STATUS_E_INTERNAL;
    }

    len = write (fd, data, (ssize_t) nbytes);
    if(len != nbytes){
        close(fd);
        return ONLP_STATUS_E_INTERNAL;
    }

    close(fd);
    return ONLP_STATUS_OK;
}

/*
 * Generic LED ioctl interface.
 */
int
onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
