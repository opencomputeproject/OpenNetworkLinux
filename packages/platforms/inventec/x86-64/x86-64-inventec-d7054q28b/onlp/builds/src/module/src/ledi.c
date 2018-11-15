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

/* LED related data */
enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_SYS,
    LED_FAN1,
    LED_FAN2,
    LED_FAN3,
    LED_FAN4
};
        
static char* devfiles__[CHASSIS_LED_COUNT+1] =  /* must map with onlp_thermal_id */
{
    "reserved",
    INV_CPLD_PREFIX"/%s_led",
    INV_PSOC_PREFIX"/fan_led_%s1",
    INV_PSOC_PREFIX"/fan_led_%s2",
    INV_PSOC_PREFIX"/fan_led_%s3",
    INV_PSOC_PREFIX"/fan_led_%s4",
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
	{LED_SYS, LED_MODE_OFF,   ONLP_LED_MODE_OFF},
	{LED_SYS, LED_MODE_GREEN, ONLP_LED_MODE_GREEN},
	{LED_SYS, LED_MODE_AMBER, ONLP_LED_MODE_ORANGE},
	{LED_SYS, LED_MODE_RED,   ONLP_LED_MODE_RED},
	{LED_FAN1,LED_MODE_AUTO,  ONLP_LED_MODE_AUTO},
	{LED_FAN2,LED_MODE_AUTO,  ONLP_LED_MODE_AUTO},
	{LED_FAN3,LED_MODE_AUTO,  ONLP_LED_MODE_AUTO},
	{LED_FAN4,LED_MODE_AUTO,  ONLP_LED_MODE_AUTO},
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
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_SYS), "Chassis LED (SYSTEM LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_ORANGE,
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
    {
        { ONLP_LED_ID_CREATE(LED_FAN3), "Fan LED 3 (FAN3 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN4), "Fan LED 4 (FAN4 LED)", 0 },
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
    int  local_id, gret = 0, rret = 0;
    char fullpath_grn[50] = {0};
    char fullpath_red[50] = {0};
    int  gvalue = 0, rvalue = 0;
    char buf[32] = {0};
		
    VALIDATE(id);
	
    local_id = ONLP_OID_ID_GET(id);
    		
    /* get fullpath */
    switch (local_id) {
	case LED_SYS:
            sprintf(fullpath_grn, devfiles__[local_id], "grn");
            sprintf(fullpath_red, devfiles__[local_id], "red");

	    /* Set LED mode */
	    gret = onlp_chassis_led_read(fullpath_grn, buf, 32);
	    if (buf[0] == '1' || buf[0] == '2' || buf[0] == '3' || buf[0] == '7') {
		gvalue = 1;
	    }

	    rret = onlp_chassis_led_read(fullpath_red, buf, 32);
	    if (buf[0] == '1' || buf[0] == '2' || buf[0] == '3' || buf[0] == '7') {
		rvalue = 1;
	    }
	    if (gret < 0 && rret < 0) {
		DEBUG_PRINT("%s(%d)\r\n", __FUNCTION__, __LINE__);
		gvalue = -1;
		rvalue = -1;
	    }
	    break;
	case LED_FAN1:
	case LED_FAN2:
	case LED_FAN3:
	case LED_FAN4:
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
	    break;
	default:
	    DEBUG_PRINT("%s(%d) Invalid led id %d\r\n", __FUNCTION__, __LINE__, local_id);
	    gvalue = -1;
	    rvalue = -1;
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
	info->status |= ONLP_LED_STATUS_FAILED;
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
	    sprintf(fullpath, "%s%s/%s", INV_CPLD_PREFIX, last_path[local_id], filename);	
	    break;
	case LED_FAN1:
	case LED_FAN2:
	case LED_FAN3:
	case LED_FAN4:
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
