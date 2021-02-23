/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
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
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include <onlplib/mmap.h>
#include "platform_lib.h"
#include "arm_accton_as4610_int.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/* LED related data
 */
enum led_light_mode {
	LED_MODE_OFF = 0,
	LED_MODE_GREEN,
	LED_MODE_GREEN_BLINKING,
	LED_MODE_AMBER,
	LED_MODE_AMBER_BLINKING,
	LED_MODE_RED,
	LED_MODE_RED_BLINKING,
	LED_MODE_BLUE,
	LED_MODE_BLUE_BLINKING,	
	LED_MODE_AUTO,
	LED_MODE_AUTO_BLINKING,
	LED_MODE_UNKNOWN,
	LED_MODE_SEVEN_SEGMENT_MAX = 9,
};


#define led_prefix_path "/sys/class/leds/as4610::"
#define led_filename   "brightness"

typedef enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_7SEG_DIG1,
    LED_7SEG_DP1,
    LED_7SEG_DIG2,
    LED_7SEG_DP2,
    LED_SYS,
    LED_PRI,
    LED_PSU1,
    LED_PSU2,
    LED_STK1,
    LED_STK2,
    LED_FAN,
    LED_POE,
    LED_ALARM
} onlp_led_id_t;

static char* onlp_led_node_subname[] =  /* must map with onlp_led_id */
{
    "reserved",
    "7seg_tens",
    "7seg_tens_point",
    "7seg_digits",
    "7seg_digits_point",
    "sys",
    "pri",
    "psu1",
    "psu2",
    "stk1",
    "stk2",
    "fan",
    "poe",
    "alarm"
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_7SEG_DIG1), "7-segment digit 1", 0 },
        ONLP_LED_STATUS_PRESENT, ONLP_LED_CAPS_CHAR, ONLP_LED_MODE_ON,
    },
    {
        { ONLP_LED_ID_CREATE(LED_7SEG_DP1), "7-segment dot 1", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_7SEG_DIG2), "7-segment digit 2", 0 },
        ONLP_LED_STATUS_PRESENT, ONLP_LED_CAPS_CHAR, ONLP_LED_MODE_ON,
    },
    {
        { ONLP_LED_ID_CREATE(LED_7SEG_DP2), "7-segment dot 2", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_SYS), "sys", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN_BLINKING | 
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PRI), "pri", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN_BLINKING | 
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU1), "psu1", 0 },
        ONLP_LED_STATUS_PRESENT, ONLP_LED_CAPS_AUTO | ONLP_LED_CAPS_AUTO_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU2), "psu2", 0 },
        ONLP_LED_STATUS_PRESENT, ONLP_LED_CAPS_AUTO | ONLP_LED_CAPS_AUTO_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_STK1), "stk1", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN_BLINKING | 
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_STK2), "stk2", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN_BLINKING | 
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "fan", 0 },
        ONLP_LED_STATUS_PRESENT, ONLP_LED_CAPS_AUTO | ONLP_LED_CAPS_AUTO_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_POE), "poe", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN_BLINKING | 
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_ALARM), "alarm", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN_BLINKING | 
        ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
};

static int conver_led_light_mode_to_onl(uint32_t id, int led_ligth_mode)
{
    switch (id) {
    case LED_SYS:
    case LED_PRI:
    case LED_STK1:
    case LED_STK2:
    case LED_POE:
    case LED_ALARM:
    case LED_7SEG_DP1:
    case LED_7SEG_DP2:
        switch (led_ligth_mode) {
        case LED_MODE_OFF:              return ONLP_LED_MODE_OFF;
        case LED_MODE_GREEN:            return ONLP_LED_MODE_GREEN;
        case LED_MODE_GREEN_BLINKING:   return ONLP_LED_MODE_GREEN_BLINKING;
        case LED_MODE_AMBER:            return ONLP_LED_MODE_ORANGE;
        case LED_MODE_AMBER_BLINKING:   return ONLP_LED_MODE_ORANGE_BLINKING;
        default:                        return ONLP_LED_MODE_OFF;
        }
    case LED_PSU1:
    case LED_PSU2:
    case LED_FAN:
        switch (led_ligth_mode) {
        case LED_MODE_AUTO:             return ONLP_LED_MODE_AUTO;
        case LED_MODE_AUTO_BLINKING:    return ONLP_LED_MODE_AUTO_BLINKING;
        default:                        return ONLP_LED_MODE_OFF;
        }
    }

	return ONLP_LED_MODE_OFF;
}

static int conver_onlp_led_light_mode_to_driver(uint32_t id, int led_ligth_mode)
{
    switch (id) {
    case LED_SYS:
    case LED_PRI:
    case LED_STK1:
    case LED_STK2:
    case LED_POE:
    case LED_ALARM:
    case LED_7SEG_DP1:
    case LED_7SEG_DP2:
        switch (led_ligth_mode) {
        case ONLP_LED_MODE_OFF:             return LED_MODE_OFF;
        case ONLP_LED_MODE_GREEN:           return LED_MODE_GREEN;
        case ONLP_LED_MODE_GREEN_BLINKING:  return LED_MODE_GREEN_BLINKING;
        case ONLP_LED_MODE_ORANGE:          return LED_MODE_AMBER;
        case ONLP_LED_MODE_ORANGE_BLINKING: return LED_MODE_AMBER_BLINKING;
        default:                            return LED_MODE_OFF;
        }
    case LED_PSU1:
    case LED_PSU2:
    case LED_FAN:
        switch (led_ligth_mode) {
        case ONLP_LED_MODE_AUTO:            return LED_MODE_AUTO;
        case ONLP_LED_MODE_AUTO_BLINKING:   return LED_MODE_AUTO_BLINKING;
        default:                            return ONLP_LED_MODE_OFF;
        }
    }

	return ONLP_LED_MODE_OFF;
}

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    return ONLP_STATUS_OK;
}

/** Set the character */
int onlp_ledi_char_set(onlp_oid_t id, char c)
{
    int  fd, len, nbytes=1;
    char data[2]      = {0};
    char fullpath[PATH_MAX] = {0};		
    int lid = ONLP_OID_ID_GET(id);

    VALIDATE(id);
    
    if (!(linfo[lid].caps & ONLP_LED_CAPS_CHAR)) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
	
    sprintf(fullpath, "%s%s/%s", led_prefix_path, onlp_led_node_subname[lid], led_filename);		

    data[0] = c;
	
    /* Create output file descriptor */
    fd = open(fullpath, O_WRONLY | O_CREAT, 0644);
    if(fd == -1){
        return ONLP_STATUS_E_INTERNAL;	
    }
 
    len = write(fd, data, (ssize_t) nbytes);
    if(len != nbytes){
        close(fd);	
        return ONLP_STATUS_E_INTERNAL;	
    }		

    close(fd);
    return ONLP_STATUS_OK;
}

/** Get the current character */
int onlp_ledi_char_get(onlp_oid_t id, char* c)
{
    int  fd, len; 
    char data = 0;
    char fullpath[PATH_MAX] = {0};
    int lid = ONLP_OID_ID_GET(id);

    VALIDATE(id);
    
    if (!(linfo[lid].caps & ONLP_LED_CAPS_CHAR)) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    sprintf(fullpath, "%s%s/%s", led_prefix_path, onlp_led_node_subname[lid], led_filename);	

    /* Set current char */
    if ((fd = open(fullpath, O_RDONLY)) == -1) {
        return ONLP_STATUS_E_INTERNAL;
    }
	
    if ((len = read(fd, &data, sizeof(data))) < 0) {
        close(fd);
        return ONLP_STATUS_E_INTERNAL;
    }
	
    /* If the read byte count is less, the format is different and calc will be wrong*/
    if ((close(fd) == -1) || (len != sizeof(data))) {
        return ONLP_STATUS_E_INTERNAL;
    }

    *c = data;
    return ONLP_STATUS_OK;
}

static int
onlp_ledi_oid_to_internal_id(onlp_oid_t id)
{
	enum as4610_product_id pid = get_product_id();
	int lid = ONLP_OID_ID_GET(id);

	if (pid != PID_AS4610T_B) {
		return lid;
	}
	
	switch (lid) {
	case 1:	return LED_SYS;
	case 2: return LED_PRI;
	case 3: return LED_PSU1;
	case 4: return LED_PSU2;
	case 5: return LED_STK1;
	case 6:	return LED_STK2;
	case 7:	return LED_FAN;
	case 8:	return LED_ALARM;
	}

	return lid;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int  fd, len, nbytes=1;
    char data[2]      = {0};
    char fullpath[53] = {0};
    int lid = onlp_ledi_oid_to_internal_id(id);

    VALIDATE(id);

    /* Set the onlp_oid_hdr_t and capabilities */		
    *info = linfo[lid];

    if (linfo[lid].caps & ONLP_LED_CAPS_CHAR) {
        return onlp_ledi_char_get(id, &info->character);
    }
    else {
        snprintf(fullpath, sizeof(fullpath), "%s%s/%s", led_prefix_path, onlp_led_node_subname[lid], led_filename);

        /* Set current mode */
        if ((fd = open(fullpath, O_RDONLY)) == -1) {
            return ONLP_STATUS_E_INTERNAL;
        }
    	
        if ((len = read(fd, data, nbytes)) < 0) {
            close(fd);
            return ONLP_STATUS_E_INTERNAL;
        }
    	
        /* If the read byte count is less, the format is different and calc will be wrong*/
        if ((close(fd) == -1) || (len != nbytes)) {
            return ONLP_STATUS_E_INTERNAL;
        }

        info->mode = conver_led_light_mode_to_onl(lid, atoi(data));

        /* Set the on/off status */
        if (info->mode != ONLP_LED_MODE_OFF) {
            info->status |= ONLP_LED_STATUS_ON;
        }
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
    char data[2]            = {0};	
    char fullpath[PATH_MAX] = {0};		
    int lid = onlp_ledi_oid_to_internal_id(id);
    
    VALIDATE(id);

    if (linfo[lid].caps & ONLP_LED_CAPS_CHAR) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
	
    sprintf(fullpath, "%s%s/%s", led_prefix_path, onlp_led_node_subname[lid], led_filename);		

    driver_mode = conver_onlp_led_light_mode_to_driver(lid, mode);
    snprintf(data, sizeof(data), "%d", driver_mode);
	
    /* Create output file descriptor */
    fd = open(fullpath, O_WRONLY | O_CREAT, 0644);
    if(fd == -1){
        return ONLP_STATUS_E_INTERNAL;	
    }
 
    len = write(fd, data, (ssize_t) nbytes);
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

