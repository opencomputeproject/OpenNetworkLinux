/************************************************************
 * ledi.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 ***********************************************************/
#include <onlp/platformi/ledi.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include "platform_lib.h"

#define filename    "brightness"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)


/* Reference to inv_cpld.h */
/*
typedef struct cpld_led_map_s {
	char	*name;
	int	bit_shift;
	unsigned int	bit_mask;
	unsigned int	led_off;
	unsigned int	led_on;
	unsigned int	led_blink;
	unsigned int	led_blink_slow;
} cpld_led_map_t;
*/
static cpld_led_map_t cpld_led_map[] = {
	{ NULL,      0, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ "stk_led", 0, 0x03, 0x00, 0x01, 0x02, 0x03 },
	{ "fan_led", 2, 0x0c, 0x00, 0x04, 0x08, 0x0c },
	{ "pwr_led", 4, 0x30, 0x00, 0x10, 0x20, 0x30 },
	{ "sys_led", 6, 0xc0, 0x00, 0x40, 0x80, 0xc0 }
};

static char* devfiles__[LED_MAX] =  /* must map with onlp_thermal_id */
{
    "reserved",
    INV_CPLD_PREFIX"/stk_led",
    INV_CPLD_PREFIX"/fan_led",
    INV_CPLD_PREFIX"/pwr_led",
    INV_CPLD_PREFIX"/sys_led",
    INV_PSOC_PREFIX"/fan1_led_%s",
    INV_PSOC_PREFIX"/fan2_led_%s",
};

enum led_light_mode {
	LED_MODE_OFF = 0,
	LED_MODE_GREEN,
	LED_MODE_AMBER,
	LED_MODE_RED,
	LED_MODE_BLUE,
	LED_MODE_GREEN_BLINK,
	LED_MODE_AMBER_BLINK,
	LED_MODE_RED_BLINK,
	LED_MODE_BLUE_BLINK,
	LED_MODE_AUTO,
	LED_MODE_UNKNOWN
};

typedef struct led_light_mode_map {
    enum onlp_led_id id;
    enum led_light_mode driver_led_mode;
    enum onlp_led_mode_e onlp_led_mode;
} led_light_mode_map_t;

led_light_mode_map_t led_map[] = {
	{LED_SYS, LED_MODE_BLUE,  ONLP_LED_MODE_BLUE,},
	{LED_PWR, LED_MODE_GREEN, ONLP_LED_MODE_GREEN},
	{LED_FAN, LED_MODE_GREEN, ONLP_LED_MODE_GREEN},
	{LED_STK, LED_MODE_GREEN, ONLP_LED_MODE_GREEN},
	{LED_FAN1,LED_MODE_AUTO,  ONLP_LED_MODE_AUTO},
	{LED_FAN2,LED_MODE_AUTO,  ONLP_LED_MODE_AUTO},
};

static char last_path[][10] =  /* must map with onlp_led_id */
{
    "reserved",
    "diag",
    "loc",
    "fan",
    "psu1",
    "psu2"
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[LED_MAX] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_STK), "Chassis LED (STACKING LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "Chassis LED (FAN LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_PWR), "Chassis LED (POWER LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_SYS), "Chassis LED (SYSTEM LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_BLUE,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN1), "Fan LED 1 (FAN1 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN2), "Fan LED 2 (FAN2 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
	ONLP_LED_MODE_ON, '0',
    },
};

static int onlp_to_driver_led_mode(enum onlp_led_id id, onlp_led_mode_t onlp_led_mode)
{
    int i, nsize = sizeof(led_map)/sizeof(led_map[0]);
    
    for(i = 0; i < nsize; i++)
    {
        if (id == led_map[i].id && onlp_led_mode == led_map[i].onlp_led_mode)
        {
            return led_map[i].driver_led_mode;
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
    DEBUG_PRINT("%s(%d): %s\r\n", __FUNCTION__, __LINE__, INV_PLATFORM_NAME);

    /*
     * Diag LED Off
     */
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_SYS), ONLP_LED_MODE_OFF);

    return ONLP_STATUS_OK;
}

int onlp_chassis_led_read(char *pathp, char *buf, size_t len)
{
   FILE * fp;

   fp = fopen (pathp, "r");
   if(fp == NULL) {
      perror("Error opening file");
      return(-1);
   }
   if( fgets (buf, len, fp) == NULL ) {
      perror("Error fgets operation");
   }
   fclose(fp);
   
   return(0);
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int  local_id = ONLP_OID_ID_GET(id);
    char fullpath_grn[50] = {0};
    char fullpath_red[50] = {0};
    int  gvalue = 0, rvalue = 0;
    int  led_on, led_blink;
    char *bp, buf[32] = {0};
    unsigned int led_st;
		
    VALIDATE(id);

    memset(info, 0, sizeof(onlp_led_info_t));
    *info = linfo[local_id]; /* Set the onlp_oid_hdr_t */
    		
    /* get fullpath */
    switch (local_id) {
	case LED_STK:
	case LED_FAN:
	case LED_PWR:
	case LED_SYS:
            sprintf(fullpath_grn, devfiles__[local_id]);
	    /* Set LED mode */
	    if (onlp_chassis_led_read(fullpath_grn, buf, 32) < 0) {
		DEBUG_PRINT("%s/%d\r\n", __FUNCTION__, __LINE__);
		info->mode = ONLP_LED_MODE_OFF;
        	info->status &= ~ONLP_LED_STATUS_PRESENT;
    		return ONLP_STATUS_OK;
	    }
	    bp = strstr(&buf[0], "0x");
	    if (!bp) {
		DEBUG_PRINT("%s/%d: (%s)\r\n", __FUNCTION__, __LINE__,buf);
		info->mode = ONLP_LED_MODE_OFF;
        	info->status &= ~ONLP_LED_STATUS_PRESENT;
    		return ONLP_STATUS_OK;
	    }
	    *(bp+4) = '\0';
	    led_st = strtoul(bp, NULL, 16);
	    switch (local_id) {
		case LED_STK:
		    led_st &=  cpld_led_map[LED_STK].bit_mask;
		    led_st >>= cpld_led_map[LED_STK].bit_shift;
		    led_on    = ONLP_LED_MODE_GREEN;
		    led_blink = ONLP_LED_MODE_GREEN_BLINKING;
		    break;
		case LED_FAN:
		    led_st &=  cpld_led_map[LED_FAN].bit_mask;
		    led_st >>= cpld_led_map[LED_FAN].bit_shift;
		    led_on    = ONLP_LED_MODE_GREEN;
		    led_blink = ONLP_LED_MODE_GREEN_BLINKING;
		    break;
		case LED_PWR:
		    led_st &=  cpld_led_map[LED_PWR].bit_mask;
		    led_st >>= cpld_led_map[LED_PWR].bit_shift;
		    led_on    = ONLP_LED_MODE_GREEN;
		    led_blink = ONLP_LED_MODE_GREEN_BLINKING;
		    break;
		case LED_SYS:
		    led_st &=  cpld_led_map[LED_SYS].bit_mask;
		    led_st >>= cpld_led_map[LED_SYS].bit_shift;
		    led_on    = ONLP_LED_MODE_BLUE;
		    led_blink = ONLP_LED_MODE_BLUE_BLINKING;
		    break;
	    }

	    info->status |= ONLP_LED_STATUS_PRESENT;
	    if (led_st == 0) {
		info->mode = ONLP_LED_MODE_OFF;
	    }
	    else
	    if (led_st == 1) {
		info->mode = led_on;
		info->status |= ONLP_LED_STATUS_ON;
	    }
	    else
	    if (led_st == 2 || led_st == 3) {
		info->mode = led_blink;
		info->status |= ONLP_LED_STATUS_ON;
	    }
	    else {
		info->mode = ONLP_LED_MODE_OFF;
		info->status &= ~ONLP_LED_STATUS_PRESENT;
	    }
    	    return ONLP_STATUS_OK;
	    break;
	case LED_FAN1:
	case LED_FAN2:
            sprintf(fullpath_grn, devfiles__[local_id], "grn");
            sprintf(fullpath_red, devfiles__[local_id], "red");

	    /* Set LED mode */
	    if (onlp_file_read_int(&gvalue, fullpath_grn) != 0) {
		DEBUG_PRINT("%s(%d)\r\n", __FUNCTION__, __LINE__);
		gvalue = 0;
	    }
	    if (onlp_file_read_int(&rvalue, fullpath_red) != 0) {
		DEBUG_PRINT("%s(%d)\r\n", __FUNCTION__, __LINE__);
		rvalue = 0;
	    }

	    if (gvalue == 1 && rvalue == 0) {
		info->mode = ONLP_LED_MODE_GREEN;
		info->status |= ONLP_LED_STATUS_PRESENT;
		info->status |= ONLP_LED_STATUS_ON;
	    }
	    else
	    if (gvalue == 0 && rvalue == 1) {
		info->mode = ONLP_LED_MODE_RED;
		info->status |= ONLP_LED_STATUS_PRESENT;
		info->status |= ONLP_LED_STATUS_ON;
	    }
	    else {
		info->mode = ONLP_LED_MODE_OFF;
		info->status &= ~ONLP_LED_STATUS_PRESENT;
		info->status &= ~ONLP_LED_STATUS_ON;
	    }
	    break;
	default:
	    DEBUG_PRINT("%s(%d) Invalid led id %d\r\n", __FUNCTION__, __LINE__, local_id);
	    info->mode = ONLP_LED_MODE_OFF;
	    info->status |= ONLP_LED_STATUS_FAILED;
            info->status &= ~ONLP_LED_STATUS_PRESENT;
    	    return ONLP_STATUS_OK;
	    break;
    }
		
    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];
    if (gvalue == 1 && rvalue == 0) {
	info->mode = ONLP_LED_MODE_GREEN;
	info->status |= ONLP_LED_STATUS_ON;
    }
    else
    if (gvalue == 0 && rvalue == 1) {
	info->mode = ONLP_LED_MODE_RED;
	info->status |= ONLP_LED_STATUS_ON;
    }
    else
    if (gvalue == 1 && rvalue == 1) {
	info->mode = ONLP_LED_MODE_ORANGE;
	info->status |= ONLP_LED_STATUS_ON;
    }
    else
    if (gvalue == 0 && rvalue == 0) {
	info->mode = ONLP_LED_MODE_OFF;
	info->status |= ONLP_LED_STATUS_ON;
    }
    else {
	info->mode = ONLP_LED_MODE_OFF;
        info->status &= ~ONLP_LED_STATUS_PRESENT;
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
    int  local_id;
    char fullpath[50] = {0};		

    VALIDATE(id);
	
    local_id = ONLP_OID_ID_GET(id);
    switch (local_id) {
	case LED_SYS:
	case LED_PWR:
	case LED_FAN:
	case LED_STK:
	    sprintf(fullpath, "%s%s/%s", INV_CPLD_PREFIX, last_path[local_id], filename);	
	    break;
	case LED_FAN1:
	case LED_FAN2:
	    sprintf(fullpath, "%s%s/%s", INV_PSOC_PREFIX, last_path[local_id], filename);	
	    break;
	default:
	    DEBUG_PRINT("%s(%d) Invalid led id %d\r\n", __FUNCTION__, __LINE__, local_id);
	    return ONLP_STATUS_E_INTERNAL;
    }
    
    if (onlp_file_write_int(onlp_to_driver_led_mode(local_id, mode), fullpath, NULL) != 0)
    {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}
