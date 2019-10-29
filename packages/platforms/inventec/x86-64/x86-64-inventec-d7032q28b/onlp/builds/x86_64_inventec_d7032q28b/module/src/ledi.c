/************************************************************
 * ledi.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 ***********************************************************/

#include <onlp/platformi/ledi.h>
#include <onlp/platformi/fani.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include "platform_lib.h"

#define filename    "brightness"
#define SYS_LED_CAPS ONLP_LED_CAPS_OFF|ONLP_LED_CAPS_GREEN|ONLP_LED_CAPS_GREEN_BLINKING|  \
    ONLP_LED_CAPS_RED|ONLP_LED_CAPS_RED_BLINKING|ONLP_LED_CAPS_ORANGE
#define FAN_LED_CAPS ONLP_LED_CAPS_RED|ONLP_LED_CAPS_GREEN

extern int inv_fani_status_get(onlp_oid_t id, uint32_t* rv);

typedef enum sys_led_mode_e {
    SYS_LED_MODE_OFF = 0,
    SYS_LED_MODE_0_5_HZ = 1,
    SYS_LED_MODE_1_HZ = 2,
    SYS_LED_MODE_2_HZ = 3,
    SYS_LED_MODE_ON = 7
} sys_led_mode_t;

static char* devfiles__[LED_MAX] =  /* must map with onlp_thermal_id */
{
    "reserved",
    INV_SYSLED_PREFIX"/%s_led",
    INV_HWMON_PREFIX"/fan_led_%s1",
    INV_HWMON_PREFIX"/fan_led_%s2",
    INV_HWMON_PREFIX"/fan_led_%s3",
    INV_HWMON_PREFIX"/fan_led_%s4",
};

#define MAKE_LED_INFO_NODE_ON_FAN(fan_id)   \
    {                       \
        { ONLP_LED_ID_CREATE(LED_FAN##fan_id), \
          "FAN LED "#fan_id,            \
          ONLP_FAN_ID_CREATE(FAN_##fan_id##_ON_MAIN_BOARD) \
        },                  \
        FAN_LED_CAPS,                  \
        0,                  \
    }

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[LED_MAX] = {
    { }, /* Not used */
    ONLP_CHASSIS_LED_INFO_ENTRY_INIT(LED_SYS, "MGMT LED", SYS_LED_CAPS),
    MAKE_LED_INFO_NODE_ON_FAN(1),
    MAKE_LED_INFO_NODE_ON_FAN(2),
    MAKE_LED_INFO_NODE_ON_FAN(3),
    MAKE_LED_INFO_NODE_ON_FAN(4),
};

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
static pthread_mutex_t diag_mutex;

int
onlp_ledi_init(void)
{
    pthread_mutex_init(&diag_mutex, NULL);

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

int onlp_chassis_led_write(char *pathp, char *buf)
{
    FILE * fp;

    fp = fopen (pathp, "w");
    if(fp == NULL) {
        perror("Error opening file");
        return(-1);
    }
    if( fputs (buf, fp) == 0 ) {
        perror("Error fputs operation");
    }
    fclose(fp);

    return(0);
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int  local_id, gret = 0, rret = 0, result;
    char fullpath_grn[50] = {0};
    char fullpath_red[50] = {0};
    int  gvalue = 0, rvalue = 0;
    char gbuf[32] = {0};
    char rbuf[32] = {0};
    uint32_t fan_status;


    ONLP_TRY( ONLP_OID_ID_VALIDATE_RANGE( id, LED_SYS, LED_FAN4) );
    local_id = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    /* get fullpath */
    switch (local_id) {
    case LED_SYS:
        sprintf(fullpath_grn, devfiles__[local_id], "grn");
        sprintf(fullpath_red, devfiles__[local_id], "red");

        /* Set LED mode */
        gret = onlp_chassis_led_read(fullpath_grn, gbuf, 32);
        rret = onlp_chassis_led_read(fullpath_red, rbuf, 32);
        if (gret < 0 || rret < 0) {
            info->mode = ONLP_LED_MODE_OFF;
            info->hdr.status = 0;  // not present
            break;
        }
        gvalue=(int)(gbuf[0]-'0');
        rvalue=(int)(rbuf[0]-'0');

        info->hdr.status |= ONLP_OID_STATUS_FLAG_PRESENT ;
        switch(gvalue) {
        case SYS_LED_MODE_0_5_HZ:
        case SYS_LED_MODE_1_HZ:
        case SYS_LED_MODE_2_HZ:
            if(rvalue==SYS_LED_MODE_OFF) {
                info->mode  = ONLP_LED_MODE_GREEN_BLINKING;
            } else {
                return ONLP_STATUS_E_INVALID;
            }
            break;
        case SYS_LED_MODE_ON:
            if(rvalue==SYS_LED_MODE_ON) {
                info->mode  = ONLP_LED_MODE_ORANGE;
            } else if(rvalue==SYS_LED_MODE_OFF) {
                info->mode  = ONLP_LED_MODE_GREEN;
            } else {
                return ONLP_STATUS_E_INVALID;
            }
            break;
        case SYS_LED_MODE_OFF:
            if(rvalue==SYS_LED_MODE_OFF) {
                info->mode  = ONLP_LED_MODE_OFF;
            } else if( (rvalue>=SYS_LED_MODE_0_5_HZ) && (rvalue<=SYS_LED_MODE_2_HZ) ) {
                info->mode  =ONLP_LED_MODE_RED_BLINKING;
            } else if(rvalue==SYS_LED_MODE_ON) {
                info->mode =ONLP_LED_MODE_RED;
            }
            break;
        default:
            return ONLP_STATUS_E_INVALID;
            break;
        }


        break;
    case LED_FAN1:
    case LED_FAN2:
    case LED_FAN3:
    case LED_FAN4:
        result=inv_fani_status_get(info->hdr.poid, &fan_status);
        if(result!=ONLP_STATUS_OK) {
            return result;
        }
        if( !(fan_status & ONLP_OID_STATUS_FLAG_PRESENT) ) {
            info->hdr.status=0;
            info->mode=ONLP_LED_MODE_INVALID;
            return ONLP_STATUS_OK;
        }

        sprintf(fullpath_grn, devfiles__[local_id], "grn");
        sprintf(fullpath_red, devfiles__[local_id], "red");

        /* Set LED mode */
        ONLP_TRY( onlp_file_read_int(&gvalue, fullpath_grn) );
        ONLP_TRY( onlp_file_read_int(&rvalue, fullpath_red) );

        if (gvalue == 1 && rvalue == 0) {
            info->mode = ONLP_LED_MODE_GREEN;
            info->hdr.status |= ONLP_OID_STATUS_FLAG_PRESENT ;
        } else if (gvalue == 0 && rvalue == 1) {
            info->mode = ONLP_LED_MODE_RED;
            info->hdr.status |= ONLP_OID_STATUS_FLAG_PRESENT ;
        } else {
            info->mode = ONLP_LED_MODE_OFF;
            info->hdr.status &= ~ONLP_OID_STATUS_FLAG_PRESENT ;
        }
    default:
        break;
    }

    return ONLP_STATUS_OK;
}

int onlp_ledi_status_get(onlp_oid_t id, uint32_t* rv)
{
    int result = ONLP_STATUS_OK;
    onlp_led_info_t linfo;

    result = onlp_ledi_info_get(id, &linfo);
    *rv = linfo.hdr.status;

    return result;
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

    if (!on_or_off) {
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
    }

    return ONLP_STATUS_E_UNSUPPORTED;
}

#define HWMON_DEVICE_DIAG_PATH	INV_HWMON_PREFIX"/subsystem/devices/0-0066/diag"
#define HWMON_DEVICE_CTRL_PATH	INV_SYSLED_PREFIX"/subsystem/devices/0-0055/ctl"

#define MIN_ACC_SIZE	(32)

/*
 * Store attr Section
 */
static int onlp_chassis_led_diag_enable(void)
{
    char tmp[MIN_ACC_SIZE];
    ssize_t ret;

    ret = onlp_chassis_led_read(HWMON_DEVICE_DIAG_PATH, &tmp[0], MIN_ACC_SIZE);
    if (ret <= 0) {
        return ret;
    }

    if (tmp[0] == '0') {
        pthread_mutex_lock(&diag_mutex);
        ret = onlp_chassis_led_write(HWMON_DEVICE_DIAG_PATH, "1");
        if (ret < 0) {
            pthread_mutex_unlock(&diag_mutex);
            return ret;
        }

        ret = onlp_chassis_led_write(HWMON_DEVICE_CTRL_PATH, "1");
        if (ret < 0) {
            pthread_mutex_unlock(&diag_mutex);
            return ret;
        }
        pthread_mutex_unlock(&diag_mutex);
    }

    return ret;
}

static int onlp_chassis_led_diag_disable(void)
{
    char tmp[MIN_ACC_SIZE];
    ssize_t ret;

    ret = onlp_chassis_led_read(HWMON_DEVICE_DIAG_PATH, &tmp[0], MIN_ACC_SIZE);
    if (ret <= 0) {
        return ret;
    }

    if (tmp[0] == '1') {
        pthread_mutex_lock(&diag_mutex);
        ret = onlp_chassis_led_write(HWMON_DEVICE_DIAG_PATH, "1");
        if (ret < 0) {
            pthread_mutex_unlock(&diag_mutex);
            return 1;
        }

        ret = onlp_chassis_led_write(HWMON_DEVICE_CTRL_PATH, "1");
        if (ret < 0) {
            pthread_mutex_unlock(&diag_mutex);
            return 1;
        }
        pthread_mutex_unlock(&diag_mutex);
    }
    return 1;
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
    char fullpath_grn[50] = {0};
    char fullpath_red[50] = {0};
    char sys_buf[32] = {0};
    onlp_led_info_t linfo;
    int ret = onlp_ledi_info_get(id, &linfo);
    int  local_id;

    local_id = ONLP_OID_ID_GET(id);

    switch (mode) {
    case ONLP_LED_MODE_OFF:
        if (ret == ONLP_STATUS_OK && linfo.hdr.status & ONLP_OID_STATUS_FLAG_PRESENT  && linfo.mode == ONLP_LED_MODE_OFF) {
            return ONLP_STATUS_OK;
        }

        sprintf(sys_buf, "%d", 0x0);
        sprintf(fullpath_grn, devfiles__[local_id], "grn");
        ret = onlp_chassis_led_write(fullpath_grn, sys_buf);

        sprintf(sys_buf, "%d", 0x0);
        sprintf(fullpath_red, devfiles__[local_id], "red");
        ret = onlp_chassis_led_write(fullpath_red, sys_buf);
        break;
    case ONLP_LED_MODE_RED:
        if (ret == ONLP_STATUS_OK && linfo.hdr.status & ONLP_OID_STATUS_FLAG_PRESENT && linfo.mode == ONLP_LED_MODE_RED) {
            return ONLP_STATUS_OK;
        }

        sprintf(sys_buf, "%d", 0x0);
        sprintf(fullpath_grn, devfiles__[local_id], "grn");
        ret = onlp_chassis_led_write(fullpath_grn, sys_buf);

        sprintf(sys_buf, "%d", 0x7);
        sprintf(fullpath_red, devfiles__[local_id], "red");
        ret = onlp_chassis_led_write(fullpath_red, sys_buf);
        break;
    case ONLP_LED_MODE_GREEN:
        if (ret == ONLP_STATUS_OK && linfo.hdr.status & ONLP_OID_STATUS_FLAG_PRESENT && linfo.mode == ONLP_LED_MODE_GREEN) {
            return ONLP_STATUS_OK;
        }

        sprintf(sys_buf, "%d", 0x7);
        sprintf(fullpath_grn, devfiles__[local_id], "grn");
        ret = onlp_chassis_led_write(fullpath_grn, sys_buf);

        sprintf(sys_buf, "%d", 0x0);
        sprintf(fullpath_red, devfiles__[local_id], "red");
        ret = onlp_chassis_led_write(fullpath_red, sys_buf);
        break;
    case ONLP_LED_MODE_ORANGE:
        if (ret == ONLP_STATUS_OK && linfo.hdr.status & ONLP_OID_STATUS_FLAG_PRESENT && linfo.mode == ONLP_LED_MODE_ORANGE) {
            return ONLP_STATUS_OK;
        }

        sprintf(sys_buf, "%d", 0x7);
        sprintf(fullpath_grn, devfiles__[local_id], "grn");
        ret = onlp_chassis_led_write(fullpath_grn, sys_buf);

        sprintf(sys_buf, "%d", 0x7);
        sprintf(fullpath_red, devfiles__[local_id], "red");
        ret = onlp_chassis_led_write(fullpath_red, sys_buf);
        break;
    default:
        return ONLP_STATUS_E_INTERNAL;
    }

    switch (local_id) {
    case LED_SYS:
        onlp_chassis_led_diag_enable();
        sleep(1);
        onlp_chassis_led_diag_disable();
        break;
    default:
        break;
    }

    return ONLP_STATUS_OK;
}