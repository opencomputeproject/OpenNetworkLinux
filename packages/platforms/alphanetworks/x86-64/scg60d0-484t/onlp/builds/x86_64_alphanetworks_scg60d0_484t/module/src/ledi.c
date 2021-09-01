/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 Accton Technology Corporation.
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
#include <onlplib/file.h>
#include <onlp/platformi/ledi.h>
#include "platform_lib.h"

#define LED_FORMAT          "/sys/bus/platform/devices/scg60d0_led/%s"
#define LED_GPIO_FORMAT     "/sys/class/gpio/gpio%d/value"


/* LED_POWER (map to driver) */
#define PWR_LED_MODE_AMBER_BLINKING         0x0
#define PWR_LED_MODE_GREEN                  0x1

/* LED_PSU1 (map to driver) */
#define PSU1_LED_MODE_AMBER_BLINKING        0x0
#define PSU1_LED_MODE_GREEN                 0x1

/* LED_PSU2 (map to driver) */
#define PSU2_LED_MODE_AMBER_BLINKING        0x0
#define PSU2_LED_MODE_GREEN                 0x1

/* LED_SYSTEM (user define , not value) */
#define SYS_LED_MODE_OFF                    0x0
#define SYS_LED_MODE_GREEN                  0x1
#define SYS_LED_MODE_AMBER                  0x3

/* LED_FAN (user define , not value) */
#define FAN_LED_MODE_OFF                    0x0
#define FAN_LED_MODE_GREEN                  0x1
#define FAN_LED_MODE_AMBER                  0x3

/* GPIO LED AMBER (GPIO Value) */
#define GPIO_LED_AMBER_MODE_OFF             0x0
#define GPIO_LED_AMBER_MODE_ON              0x1

/* GPIO LED GREEN (GPIO Value) */
#define GPIO_LED_GREEN_MODE_OFF             0x0
#define GPIO_LED_GREEN_MODE_ON              0x1

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)


typedef struct led_light_mode_map {
    enum onlp_led_id id;
    int driver_led_mode;
    enum onlp_led_mode_e onlp_led_mode;
} led_light_mode_map_t;

led_light_mode_map_t led_map[] = {
    {LED_POWER,   PWR_LED_MODE_GREEN,           ONLP_LED_MODE_GREEN},
    {LED_POWER,   PWR_LED_MODE_AMBER_BLINKING,  ONLP_LED_MODE_ORANGE_BLINKING},

    {LED_PSU1,    PSU1_LED_MODE_GREEN,          ONLP_LED_MODE_GREEN},
    {LED_PSU1,    PSU1_LED_MODE_AMBER_BLINKING, ONLP_LED_MODE_ORANGE_BLINKING},

    {LED_PSU2,    PSU2_LED_MODE_GREEN,          ONLP_LED_MODE_GREEN},
    {LED_PSU2,    PSU2_LED_MODE_AMBER_BLINKING, ONLP_LED_MODE_ORANGE_BLINKING},

    {LED_SYSTEM,  SYS_LED_MODE_OFF,             ONLP_LED_MODE_OFF},
    {LED_SYSTEM,  SYS_LED_MODE_GREEN,           ONLP_LED_MODE_GREEN},
    {LED_SYSTEM,  SYS_LED_MODE_AMBER,           ONLP_LED_MODE_ORANGE},

    {LED_FAN,     FAN_LED_MODE_OFF,             ONLP_LED_MODE_OFF},
    {LED_FAN,     FAN_LED_MODE_GREEN,           ONLP_LED_MODE_GREEN},
    {LED_FAN,     FAN_LED_MODE_AMBER,           ONLP_LED_MODE_ORANGE},
};

/* For GPIO LED */
typedef struct gpio_led_light_driver_mode_map {
    enum onlp_led_id id;
    int driver_led_mode;
    int amber_value;
    int green_value;
} gpio_led_light_driver_mode_map_t;

gpio_led_light_driver_mode_map_t gpio_led_map[] = {
    {LED_SYSTEM,  SYS_LED_MODE_OFF,             GPIO_LED_AMBER_MODE_OFF, GPIO_LED_GREEN_MODE_OFF},
    {LED_SYSTEM,  SYS_LED_MODE_GREEN,           GPIO_LED_AMBER_MODE_OFF, GPIO_LED_GREEN_MODE_ON},
    {LED_SYSTEM,  SYS_LED_MODE_AMBER,           GPIO_LED_AMBER_MODE_ON,  GPIO_LED_GREEN_MODE_OFF},

    {LED_FAN,     FAN_LED_MODE_OFF,             GPIO_LED_AMBER_MODE_OFF, GPIO_LED_GREEN_MODE_OFF},
    {LED_FAN,     FAN_LED_MODE_GREEN,           GPIO_LED_AMBER_MODE_OFF, GPIO_LED_GREEN_MODE_ON},
    {LED_FAN,     FAN_LED_MODE_AMBER,           GPIO_LED_AMBER_MODE_ON,  GPIO_LED_GREEN_MODE_OFF},
};


static char *leds[] =  /* must map with onlp_led_id (platform_lib.h) */
{
    "reserved",
    "led_pwr",    /* LED_POWER  : 0: amber blinking , 1: green solid */
    "led_psu1",   /* LED_PSU1   : 0: amber blinking , 1: green solid */
    "led_psu2",   /* LED_PSU2   : 0: amber blinking , 1: green solid */
    "",           /* LED_SYSTEM */
    "",           /* LED_FAN    */
};

struct led_gpio_id
{
    uint8_t ledid;
    uint16_t gpioid_color1;
	uint16_t gpioid_color2;
};

struct led_gpio_id gpio_leds[] = {
    {  },  /* Not used */
    {  },  /* Not used */
    {  },  /* Not used */
    {  },  /* Not used */
    { LED_SYSTEM, 496, 497 },  /* 496:AMBER. 497:GREEN */
    { LED_FAN,    498, 499 },  /* 498:AMBER. 499:GREEN */
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_POWER), "Chassis LED 1 (POWER LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU1), "Chassis LED 2 (PSU1 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU2), "Chassis LED 3 (PSU2 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_SYSTEM), "Chassis LED 4 (SYSTEM LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "Chassis LED 5 (FAN LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
    },
};

static int driver_to_onlp_led_mode(enum onlp_led_id id, int driver_led_mode)
{
    int i, nsize = sizeof(led_map)/sizeof(led_map[0]);

    for (i = 0; i < nsize; i++)
    {
        if (id == led_map[i].id && driver_led_mode == led_map[i].driver_led_mode)
        {
            DIAG_PRINT("%s, id:%d, driver_led_mode:%d to onlp_led_mode:%d", 
                __FUNCTION__, id, driver_led_mode, led_map[i].onlp_led_mode);
            return led_map[i].onlp_led_mode;
        }
    }

    return 0;
}

static int onlp_to_driver_led_mode(enum onlp_led_id id, onlp_led_mode_t onlp_led_mode)
{
    int i, nsize = sizeof(led_map)/sizeof(led_map[0]);

    for(i = 0; i < nsize; i++)
    {
        if (id == led_map[i].id && onlp_led_mode == led_map[i].onlp_led_mode)
        {
            DIAG_PRINT("%s, id:%d, onlp_led_mode:%d to driver_led_mode:%d", 
                __FUNCTION__, id, onlp_led_mode, led_map[i].driver_led_mode);
            return led_map[i].driver_led_mode;
        }
    }

    return 0;
}

/* For GPIO LED Value,  return array index of gpio_led_map */
static int driver_to_gpio_led(enum onlp_led_id id, int driver_led_mode)
{
    int i, nsize = sizeof(gpio_led_map)/sizeof(gpio_led_map[0]);

    for (i = 0; i < nsize; i++)
    {
        if (id == gpio_led_map[i].id && driver_led_mode == gpio_led_map[i].driver_led_mode)
        {
            DIAG_PRINT("%s, id:%d, driver_led_mode:%d to gpio_led_map[%d]", 
                __FUNCTION__, id, driver_led_mode, i);
            return i;
        }
    }

    return 0;
}

/* For GPIO LED value to get driver_led_mode */
static int gpio_led_value_to_driver_led_mode(enum onlp_led_id id, int amber_value, int green_value)
{
    int i, nsize = sizeof(gpio_led_map)/sizeof(gpio_led_map[0]);

    for (i = 0; i < nsize; i++)
    {
        if (id == gpio_led_map[i].id && amber_value == gpio_led_map[i].amber_value
                                     && green_value == gpio_led_map[i].green_value)
        {
            DIAG_PRINT("%s, id:%d, amber_value:%d and green_value:%d to driver_led_mode:%d", 
                __FUNCTION__, id, amber_value, green_value, gpio_led_map[i].driver_led_mode);
            return gpio_led_map[i].driver_led_mode;
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
    DIAG_PRINT("%s", __FUNCTION__);
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int     lid, value;
	int     amber_value = 0, green_value = 0;   /* for GPIO LED */
    int     driver_led_mode = 0;
    VALIDATE(id);

    lid = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    /* Get LED mode */
    switch (lid)
    {
        case LED_POWER:
        case LED_PSU1:
        case LED_PSU2:
            if (onlp_file_read_int(&value, LED_FORMAT, leds[lid]) < 0) 
            {
                AIM_LOG_ERROR("Unable to read status from file "LED_FORMAT, leds[lid]);
                return ONLP_STATUS_E_INTERNAL;
            }

            info->mode = driver_to_onlp_led_mode(lid, value);
            break;
        case LED_SYSTEM:
        case LED_FAN:
            if (onlp_file_read_int(&amber_value, LED_GPIO_FORMAT, gpio_leds[lid].gpioid_color1) < 0) 
            {
                AIM_LOG_ERROR("Unable to read status from file "LED_GPIO_FORMAT, gpio_leds[lid].gpioid_color1);
                return ONLP_STATUS_E_INTERNAL;
            }
            if (onlp_file_read_int(&green_value, LED_GPIO_FORMAT, gpio_leds[lid].gpioid_color2) < 0) 
            {
                AIM_LOG_ERROR("Unable to read status from file "LED_GPIO_FORMAT, gpio_leds[lid].gpioid_color2);
                return ONLP_STATUS_E_INTERNAL;
            }
            
            driver_led_mode = gpio_led_value_to_driver_led_mode(lid, amber_value, green_value);
            info->mode = driver_to_onlp_led_mode(lid, driver_led_mode);

            break;
        default:
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, lid);
            return ONLP_STATUS_E_PARAM;
    }

    /* Set the on/off status */
    if (info->mode != ONLP_LED_MODE_OFF) 
    {
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

    if (!on_or_off) 
    {
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
    }
    else
    {
        /*Just pick a color to light*/
        int rv, i;
        onlp_led_info_t info;
        uint32_t caps;

        rv = onlp_ledi_info_get(id, &info);
        if (rv < 0)
            return rv;

        caps = info.caps;
        /*Bit scan*/
        for (i = 1; i < sizeof(caps)*8; i++) 
        {
            if( caps & (1<<i)) {
                return onlp_ledi_mode_set(id, i);
            }
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
    int     lid;
    int     driver_led_mode = 0;
    int     gpio_led_index = 0;
    VALIDATE(id);

    lid = ONLP_OID_ID_GET(id);

    /* Set LED light mode */
    switch (lid)
    {
        case LED_POWER:
        case LED_PSU1:
        case LED_PSU2:
            /* LED control by BMC , not support to set LED */
            return ONLP_STATUS_E_UNSUPPORTED;
        case LED_SYSTEM:
        case LED_FAN:
            driver_led_mode = onlp_to_driver_led_mode(lid , mode);
            gpio_led_index = driver_to_gpio_led(lid, driver_led_mode);
            
			if (onlp_file_write_int(gpio_led_map[gpio_led_index].amber_value, LED_GPIO_FORMAT, gpio_leds[lid].gpioid_color1) < 0)
    		{
        		AIM_LOG_ERROR("Unable to read status from file "LED_GPIO_FORMAT, gpio_leds[lid].gpioid_color1);
				return ONLP_STATUS_E_INTERNAL;
    		}

			if (onlp_file_write_int(gpio_led_map[gpio_led_index].green_value, LED_GPIO_FORMAT, gpio_leds[lid].gpioid_color2) < 0)
    		{
        		AIM_LOG_ERROR("Unable to read status from file "LED_GPIO_FORMAT, gpio_leds[lid].gpioid_color2);
				return ONLP_STATUS_E_INTERNAL;
    		}
            break;
        default:
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, lid);
            return ONLP_STATUS_E_PARAM;
    }

    return ONLP_STATUS_OK;
}

