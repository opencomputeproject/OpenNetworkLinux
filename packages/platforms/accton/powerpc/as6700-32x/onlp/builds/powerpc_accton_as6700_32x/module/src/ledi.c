/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
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
 ***********************************************************/
#include <onlp/platformi/ledi.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

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
/*
enum led_light_mode {
    LED_MODE_OFF = 0,
    LED_MODE_AMBER,
    LED_MODE_GREEN,
    LED_MODE_AMBER_BLINK,
    LED_MODE_GREEN_BLINK,
    LED_MODE_AUTO, 
};*/
enum led_light_mode {
    /* system led mode */
    LED_MODE_SYSTEM_OFF = 0,
    LED_MODE_SYSTEM_GREEN_BLINK,
    LED_MODE_SYSTEM_GREEN,
    LED_MODE_SYSTEM_OFF_2,

    /* alarm led mode */
    LED_MODE_ALARM_OFF = 0,
    LED_MODE_ALARM_RED_BLINK,
    LED_MODE_ALARM_GREEN,
    LED_MODE_ALARM_OFF_2,

    /* fan led mode */
    LED_MODE_FAN_GREEN = 0,
    LED_MODE_FAN_RED
};


int led_light_map_mode[][2] =
{
    {LED_MODE_SYSTEM_OFF,         ONLP_LED_MODE_OFF}, 			
    {LED_MODE_SYSTEM_GREEN_BLINK, ONLP_LED_MODE_GREEN_BLINKING},
    {LED_MODE_SYSTEM_GREEN,       ONLP_LED_MODE_GREEN},
    {LED_MODE_SYSTEM_OFF_2,       0},

    {LED_MODE_ALARM_OFF,         ONLP_LED_MODE_OFF}, 			
    {LED_MODE_ALARM_GREEN,       ONLP_LED_MODE_GREEN},
    {LED_MODE_ALARM_RED_BLINK, ONLP_LED_MODE_RED_BLINKING},	
    {LED_MODE_ALARM_OFF_2,       0},
		
    {LED_MODE_FAN_RED,       ONLP_LED_MODE_RED},
    {LED_MODE_FAN_GREEN,       ONLP_LED_MODE_GREEN},	
};


#define led_prefix_path "/sys/class/leds/accton_as6700_32x_led::"
#define led_filename   "brightness"

typedef enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_SYSTEM,
    LED_ALARM,
    LED_FAN1,
    LED_FAN2,
    LED_FAN3,
    LED_FAN4,
    LED_FAN5,			
} onlp_led_id_t;

static char onlp_led_node_subname[][10] =  /* must map with onlp_led_id */
{
    "reserved",
    "system",
    "alarm",
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
        { ONLP_LED_ID_CREATE(LED_SYSTEM), "system", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_ALARM), "alarm", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED_BLINKING | ONLP_LED_CAPS_GREEN,
    },
   {
        { ONLP_LED_ID_CREATE(LED_FAN1), "fan1", 0 },
        ONLP_LED_STATUS_PRESENT, 0,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN2), "fan2", 0 },
        ONLP_LED_STATUS_PRESENT, 0,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN3), "fan3", 0 },
        ONLP_LED_STATUS_PRESENT, 0,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN4), "fan4", 0 },
        ONLP_LED_STATUS_PRESENT, 0,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN5), "fan5", 0 },
        ONLP_LED_STATUS_PRESENT, 0,
    },
};


static int conver_led_light_mode_to_onl(uint32_t id, int led_ligth_mode)
{
    int i, nsize,offset;

    switch(id)
    {
        case LED_SYSTEM:
            nsize= 4;
            offset=0;
            break;
        case LED_ALARM:
            nsize= 8;
            offset=4;
            break;
        case LED_FAN1:
        case LED_FAN2:
        case LED_FAN3:
        case LED_FAN4:
        case LED_FAN5:
            nsize= 10;
            offset=8;
            break;
        default:
            return 0;
    }
    /*nsize= sizeof(led_light_map_mode)/sizeof(led_light_map_mode[0]);*/
    for(i=offset; i<nsize; i++)
    {
	    if (led_ligth_mode == led_light_map_mode[i][0])
		{
            return led_light_map_mode[i][1];  	  
	    }
    }
	return 0;
}


static int conver_led_light_mode_to_driver(uint32_t id, int led_ligth_mode)
{
    int i, nsize,offset;

    switch(id)
    {
        case LED_SYSTEM:
            nsize= 4;
            offset=0;
            break;
        case LED_ALARM:
            nsize= 8;
            offset=4;
            break;
        case LED_FAN1:
        case LED_FAN2:
        case LED_FAN3:
        case LED_FAN4:
        case LED_FAN5:
            nsize= 10;
            offset=8;
            break;
        default:
            return 0;
    }

    /*nsize= sizeof(led_light_map_mode)/sizeof(led_light_map_mode[0]);*/
    for(i=offset; i<nsize; i++)
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
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{

    int  fd, len, nbytes=1;
    char data[2]      = {0};
    char fullpath[50] = {0};
		
    VALIDATE(id);

    sprintf(fullpath, "%s%s/%s", led_prefix_path, onlp_led_node_subname[ONLP_OID_ID_GET(id)], led_filename);	

    /* Set the onlp_oid_hdr_t and capabilities */		
    *info = linfo[ONLP_OID_ID_GET(id)];

    /* Set current mode */
    if ((fd = open(fullpath, O_RDONLY)) == -1)
    {
        return ONLP_STATUS_E_INTERNAL;
    }
	
    if ((len = read(fd, data, nbytes)) < 0)
    {
        close(fd);
        return ONLP_STATUS_E_INTERNAL;
    }
	
    /* If the read byte count is less, the format is different and calc will be wrong*/
    if ((close(fd) == -1) || (len != nbytes))
    {
        return ONLP_STATUS_E_INTERNAL;
    }

    info->mode = conver_led_light_mode_to_onl(ONLP_OID_ID_GET(id),atoi(data));

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
    int  fd, len, driver_mode, nbytes=1;
    char data[2]      = {0};	
    char fullpath[50] = {0};		

    VALIDATE(id);
	
    sprintf(fullpath, "%s%s/%s", led_prefix_path, onlp_led_node_subname[ONLP_OID_ID_GET(id)], led_filename);		

    driver_mode = conver_led_light_mode_to_driver(ONLP_OID_ID_GET(id),mode);
    sprintf(data, "%d", driver_mode);
	
    /* Create output file descriptor */
    fd = open(fullpath, O_WRONLY | O_CREAT, 0644);
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

