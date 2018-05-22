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

#define prefix_path "/bsp/led/led_"
#define driver_value_len 50

#define LED_MODE_OFF         "none"
#define LED_MODE_GREEN       "green"
#define LED_MODE_RED         "red"
#define LED_MODE_BLUE        "blue"
#define LED_MODE_GREEN_BLINK "green_blink"
#define LED_MODE_RED_BLINK   "red_blink"
#define LED_MODE_BLUE_BLINK  "blue_blink"
#define LED_MODE_AUTO        "cpld_control"

#define LED_BLINK_PERIOD	"100"
#define LED_ON			"1"
#define LED_OFF			"0"
#define LED_BLINK_PERIOD_LEN	3
#define LED_MODE_LEN		1

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/* LED related data
 */

typedef struct led_light_mode_map {
    enum onlp_led_id id;
    char* driver_led_mode;
    enum onlp_led_mode_e onlp_led_mode;
} led_light_mode_map_t;

led_light_mode_map_t led_map[] = {
{LED_SYSTEM, LED_MODE_OFF,         ONLP_LED_MODE_OFF},
{LED_SYSTEM, LED_MODE_GREEN,       ONLP_LED_MODE_GREEN},
{LED_SYSTEM, LED_MODE_RED,         ONLP_LED_MODE_RED},
{LED_SYSTEM, LED_MODE_RED_BLINK,   ONLP_LED_MODE_RED_BLINKING},
{LED_SYSTEM, LED_MODE_GREEN_BLINK, ONLP_LED_MODE_GREEN_BLINKING},
{LED_SYSTEM, LED_MODE_AUTO,        ONLP_LED_MODE_AUTO},

{LED_FAN, LED_MODE_OFF,         ONLP_LED_MODE_OFF},
{LED_FAN, LED_MODE_GREEN,       ONLP_LED_MODE_GREEN},
{LED_FAN, LED_MODE_RED,         ONLP_LED_MODE_RED},
{LED_FAN, LED_MODE_RED_BLINK,   ONLP_LED_MODE_RED_BLINKING},
{LED_FAN, LED_MODE_GREEN_BLINK, ONLP_LED_MODE_GREEN_BLINKING},
{LED_FAN, LED_MODE_AUTO,        ONLP_LED_MODE_AUTO},

{LED_PSU1, LED_MODE_OFF,         ONLP_LED_MODE_OFF},
{LED_PSU1, LED_MODE_GREEN,       ONLP_LED_MODE_GREEN},
{LED_PSU1, LED_MODE_RED,         ONLP_LED_MODE_RED},
{LED_PSU1, LED_MODE_RED_BLINK,   ONLP_LED_MODE_RED_BLINKING},
{LED_PSU1, LED_MODE_GREEN_BLINK, ONLP_LED_MODE_GREEN_BLINKING},
{LED_PSU1, LED_MODE_AUTO,        ONLP_LED_MODE_AUTO},

{LED_PSU2, LED_MODE_OFF,         ONLP_LED_MODE_OFF},
{LED_PSU2, LED_MODE_GREEN,       ONLP_LED_MODE_GREEN},
{LED_PSU2, LED_MODE_RED,         ONLP_LED_MODE_RED},
{LED_PSU2, LED_MODE_RED_BLINK,   ONLP_LED_MODE_RED_BLINKING},
{LED_PSU2, LED_MODE_GREEN_BLINK, ONLP_LED_MODE_GREEN_BLINKING},
{LED_PSU2, LED_MODE_AUTO,        ONLP_LED_MODE_AUTO},

{LED_UID,  LED_MODE_OFF,         ONLP_LED_MODE_OFF},
{LED_UID,  LED_MODE_BLUE,        ONLP_LED_MODE_BLUE},
{LED_UID,  LED_MODE_BLUE_BLINK,  ONLP_LED_MODE_BLUE_BLINKING},
{LED_UID,  LED_MODE_AUTO,        ONLP_LED_MODE_AUTO},
};

typedef struct led_colors {
    enum onlp_led_id id;
    const char* color1;
    const char* color2;
} led_colors_t;

static led_colors_t led_colors_map[] = {
    {LED_SYSTEM, "green", "red"},
    {LED_FAN, "green", "red"},
    {LED_PSU1, "green", "red"},
    {LED_PSU2, "green", "red"},
    {LED_UID, "blue", NULL},
};

static char file_names[][10] =  /* must map with onlp_led_id */
{
    "reserved",
    "status",
    "fan",
    "psu1",
    "psu2",
	"uid"
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
        { ONLP_LED_ID_CREATE(LED_FAN), "Chassis LED 2 (FAN LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING |
        ONLP_LED_CAPS_RED | ONLP_LED_CAPS_RED_BLINKING | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU1), "Chassis LED 3 (PSU1 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING |
        ONLP_LED_CAPS_RED | ONLP_LED_CAPS_RED_BLINKING | ONLP_LED_CAPS_AUTO,
    },
	{
		{ ONLP_LED_ID_CREATE(LED_PSU2), "Chassis LED 4 (PSU2 LED)", 0 },
		ONLP_LED_STATUS_PRESENT,
		ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING |
		ONLP_LED_CAPS_RED | ONLP_LED_CAPS_RED_BLINKING | ONLP_LED_CAPS_AUTO,
	},
	{
		{ ONLP_LED_ID_CREATE(LED_UID), "Chassis LED 5 (UID LED)", 0 },
		ONLP_LED_STATUS_PRESENT,
		ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_BLUE | ONLP_LED_CAPS_BLUE_BLINKING |
		ONLP_LED_CAPS_AUTO,
	}
};

static int driver_to_onlp_led_mode(enum onlp_led_id id, char* driver_led_mode)
{
    char *pos;
    int i, nsize = sizeof(led_map)/sizeof(led_map[0]);

    if ((pos=strchr(driver_led_mode, '\n')) != NULL)
        *pos = '\0';
    for (i = 0; i < nsize; i++)
    {
        if (id == led_map[i].id &&
            !strncmp(led_map[i].driver_led_mode, driver_led_mode, driver_value_len))
        {
            return led_map[i].onlp_led_mode;
        }
    }

    return 0;
}

static char* onlp_to_driver_led_mode(enum onlp_led_id id, onlp_led_mode_t onlp_led_mode)
{
    int i, nsize = sizeof(led_map)/sizeof(led_map[0]);

    for (i = 0; i < nsize; i++)
    {
        if (id == led_map[i].id && onlp_led_mode == led_map[i].onlp_led_mode)
        {
            return led_map[i].driver_led_mode;
        }
    }

    return LED_MODE_OFF;
}

static int led_set_mode(onlp_oid_t id, onlp_led_mode_t mode)
{
    int  local_id = ONLP_OID_ID_GET(id);
    char color[10]={0};
    int blinking = 0;

    switch (mode) {
    case ONLP_LED_MODE_RED_BLINKING:
        strcpy(color, "red");
        blinking = 1;
        break;
    case ONLP_LED_MODE_GREEN_BLINKING:
        strcpy(color, "green");
        blinking = 1;
        break;
    case ONLP_LED_MODE_BLUE_BLINKING:
        strcpy(color, "blue");
        blinking = 1;
        break;
    case ONLP_LED_MODE_YELLOW_BLINKING:
        strcpy(color, "yellow");
        blinking = 1;
        break;
    case ONLP_LED_MODE_RED:
    	strcpy(color, "red");
    	break;
    case ONLP_LED_MODE_GREEN:
    	strcpy(color, "green");
    	break;
    case ONLP_LED_MODE_BLUE:
    	strcpy(color, "blue");
    	break;
    case ONLP_LED_MODE_YELLOW:
    	strcpy(color, "yellow");
        break;
    default:
        return ONLP_STATUS_E_PARAM;
    }

    if (blinking) {
    	onlp_file_write((uint8_t*)LED_BLINK_PERIOD, LED_BLINK_PERIOD_LEN,
    			"%s%s_%s_delay_off", prefix_path, file_names[local_id], color);
    	onlp_file_write((uint8_t*)LED_BLINK_PERIOD, LED_BLINK_PERIOD_LEN,
    	    	"%s%s_%s_delay_on", prefix_path, file_names[local_id], color);
    }
    onlp_file_write((uint8_t*)LED_ON, LED_MODE_LEN,
    		"%s%s_%s", prefix_path, file_names[local_id], color);

    return ONLP_STATUS_OK;
}

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    /*
     * ONLPD calls it too early before all BSP insfrastructure is set
     */

    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int  len, local_id = 0;
    uint8_t data[driver_value_len] = {0};

    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    /* Get LED mode */
    if (onlp_get_kernel_ver() >= KERNEL_VERSION(4,9,30)) {
        char* cmd = aim_fstrdup("%s%s_state", prefix_path, file_names[local_id]);
        if(system(cmd) != 0) {
            aim_free(cmd);
            return ONLP_STATUS_E_INTERNAL;
        }
        aim_free(cmd);
    }

    if (onlp_file_read(data, sizeof(data), &len, "%s%s",
    		prefix_path, file_names[local_id]) != 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    info->mode = driver_to_onlp_led_mode(local_id, (char*)data);

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
    	if (onlp_get_kernel_ver() < KERNEL_VERSION(4,9,30))
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
    	else {
    	    int i, nsize = sizeof(led_colors_map)/sizeof(led_colors_map[0]);
    	    for (i = 0; i < nsize; i++)
    	    {
    	        if (id == led_colors_map[i].id)
    	            break;
    	    }
    	    if (led_colors_map[i].color1)
    		onlp_file_write((uint8_t*)LED_OFF, LED_MODE_LEN,
    		    		"%s%s_%s", prefix_path, file_names[id], led_colors_map[i].color1);
    	}
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
    int  local_id;
    char* driver_led_mode;
    int nbytes;

    VALIDATE(id);

    if (onlp_get_kernel_ver() < KERNEL_VERSION(4,9,30)) {
    local_id = ONLP_OID_ID_GET(id);
    	driver_led_mode = onlp_to_driver_led_mode(local_id, mode);
    	nbytes = strnlen(driver_led_mode, driver_value_len);
        if (onlp_file_write((uint8_t*)driver_led_mode, nbytes,
    	"%s%s", prefix_path, file_names[local_id]) != 0) {
        return ONLP_STATUS_E_INTERNAL;
        }
    } else {
    	if (led_set_mode(id, mode) != 0) {
    	    return ONLP_STATUS_E_INTERNAL;
    	}
    }

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

