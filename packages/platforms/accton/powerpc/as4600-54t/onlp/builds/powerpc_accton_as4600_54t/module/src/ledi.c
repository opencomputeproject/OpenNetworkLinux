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
#include <onlplib/mmap.h>
#include "powerpc_accton_as4600_54t_int.h"

/* LED related data
 */
#define CPLD_BASE_ADDRESS                 0xEA000000
#define CPLD_SYSTEM_LED_CONTROL_OFFSET_1  0x4  /* DIAG/PSU1/PSU2 LED*/
#define CPLD_SYSTEM_LED_CONTROL_OFFSET_2  0x5  /* FAN1/FAN2/MODULE1/MODULE2 LED */

#define CPLD_LED_DIAG_REG_MASK      0xC0
#define CPLD_LED_DIAG_GREEN         0x40
#define CPLD_LED_DIAG_AMBER         0x80
#define CPLD_LED_DIAG_OFF           0xC0

#define CPLD_LED_PSU_1_REG_MASK      0x0C
#define CPLD_LED_PSU_1_GREEN         0x04
#define CPLD_LED_PSU_1_AMBER         0x08
#define CPLD_LED_PSU_1_OFF           0x0C

#define CPLD_LED_PSU_2_REG_MASK      0x03
#define CPLD_LED_PSU_2_GREEN         0x01
#define CPLD_LED_PSU_2_AMBER         0x02
#define CPLD_LED_PSU_2_OFF           0x03

#define CPLD_LED_FAN_1_REG_MASK       0xC0
#define CPLD_LED_FAN_1_GREEN          0x40
#define CPLD_LED_FAN_1_AMBER          0x80
#define CPLD_LED_FAN_1_OFF            0xC0

#define CPLD_LED_FAN_2_REG_MASK       0x30
#define CPLD_LED_FAN_2_GREEN          0x10
#define CPLD_LED_FAN_2_AMBER          0x20
#define CPLD_LED_FAN_2_OFF            0x30

#define CPLD_LED_MODULE_1_REG_MASK       0x0C
#define CPLD_LED_MODULE_1_GREEN          0x04
#define CPLD_LED_MODULE_1_AMBER          0x08
#define CPLD_LED_MODULE_1_OFF            0x0C

#define CPLD_LED_MODULE_2_REG_MASK       0x03
#define CPLD_LED_MODULE_2_GREEN          0x01
#define CPLD_LED_MODULE_2_AMBER          0x02
#define CPLD_LED_MODULE_2_OFF            0x03


typedef struct onlp_led_cpld
{
    onlp_led_id_t  lid;
    unsigned char  cpld_offset;
    unsigned char  cpld_mask;
    unsigned char  cpld_val; /* The CPLD mask for the given mode*/
    int            mode;
} onlp_led_cpld_t;

const onlp_led_cpld_t led_data[] = {
{LED_DIAG, CPLD_SYSTEM_LED_CONTROL_OFFSET_1, CPLD_LED_DIAG_REG_MASK, CPLD_LED_DIAG_OFF, ONLP_LED_MODE_OFF},
{LED_DIAG, CPLD_SYSTEM_LED_CONTROL_OFFSET_1, CPLD_LED_DIAG_REG_MASK, CPLD_LED_DIAG_GREEN, ONLP_LED_MODE_GREEN},
{LED_DIAG, CPLD_SYSTEM_LED_CONTROL_OFFSET_1, CPLD_LED_DIAG_REG_MASK, CPLD_LED_DIAG_AMBER, ONLP_LED_MODE_ORANGE},
{LED_PSU_1, CPLD_SYSTEM_LED_CONTROL_OFFSET_1, CPLD_LED_PSU_1_REG_MASK, CPLD_LED_PSU_1_OFF, ONLP_LED_MODE_OFF},
{LED_PSU_1, CPLD_SYSTEM_LED_CONTROL_OFFSET_1, CPLD_LED_PSU_1_REG_MASK, CPLD_LED_PSU_1_GREEN, ONLP_LED_MODE_GREEN},
{LED_PSU_1, CPLD_SYSTEM_LED_CONTROL_OFFSET_1, CPLD_LED_PSU_1_REG_MASK, CPLD_LED_PSU_1_AMBER, ONLP_LED_MODE_ORANGE},
{LED_PSU_2, CPLD_SYSTEM_LED_CONTROL_OFFSET_1, CPLD_LED_PSU_2_REG_MASK, CPLD_LED_PSU_2_OFF, ONLP_LED_MODE_OFF},
{LED_PSU_2, CPLD_SYSTEM_LED_CONTROL_OFFSET_1, CPLD_LED_PSU_2_REG_MASK, CPLD_LED_PSU_2_GREEN, ONLP_LED_MODE_GREEN},
{LED_PSU_2, CPLD_SYSTEM_LED_CONTROL_OFFSET_1, CPLD_LED_PSU_2_REG_MASK, CPLD_LED_PSU_2_AMBER, ONLP_LED_MODE_ORANGE},
{LED_FAN_1, CPLD_SYSTEM_LED_CONTROL_OFFSET_2, CPLD_LED_FAN_1_REG_MASK, CPLD_LED_FAN_1_OFF, ONLP_LED_MODE_OFF},
{LED_FAN_1, CPLD_SYSTEM_LED_CONTROL_OFFSET_2, CPLD_LED_FAN_1_REG_MASK, CPLD_LED_FAN_1_GREEN, ONLP_LED_MODE_GREEN},
{LED_FAN_1, CPLD_SYSTEM_LED_CONTROL_OFFSET_2, CPLD_LED_FAN_1_REG_MASK, CPLD_LED_FAN_1_AMBER, ONLP_LED_MODE_ORANGE},
{LED_FAN_2, CPLD_SYSTEM_LED_CONTROL_OFFSET_2, CPLD_LED_FAN_2_REG_MASK, CPLD_LED_FAN_2_OFF, ONLP_LED_MODE_OFF},
{LED_FAN_2, CPLD_SYSTEM_LED_CONTROL_OFFSET_2, CPLD_LED_FAN_2_REG_MASK, CPLD_LED_FAN_2_GREEN, ONLP_LED_MODE_GREEN},
{LED_FAN_2, CPLD_SYSTEM_LED_CONTROL_OFFSET_2, CPLD_LED_FAN_2_REG_MASK, CPLD_LED_FAN_2_AMBER, ONLP_LED_MODE_ORANGE},
{LED_MODULE_1, CPLD_SYSTEM_LED_CONTROL_OFFSET_2, CPLD_LED_MODULE_1_REG_MASK, CPLD_LED_MODULE_1_OFF, ONLP_LED_MODE_OFF},
{LED_MODULE_1, CPLD_SYSTEM_LED_CONTROL_OFFSET_2, CPLD_LED_MODULE_1_REG_MASK, CPLD_LED_MODULE_1_GREEN, ONLP_LED_MODE_GREEN},
{LED_MODULE_1, CPLD_SYSTEM_LED_CONTROL_OFFSET_2, CPLD_LED_MODULE_1_REG_MASK, CPLD_LED_MODULE_1_AMBER, ONLP_LED_MODE_ORANGE},
{LED_MODULE_2, CPLD_SYSTEM_LED_CONTROL_OFFSET_2, CPLD_LED_MODULE_2_REG_MASK, CPLD_LED_MODULE_2_OFF, ONLP_LED_MODE_OFF},
{LED_MODULE_2, CPLD_SYSTEM_LED_CONTROL_OFFSET_2, CPLD_LED_MODULE_2_REG_MASK, CPLD_LED_MODULE_2_GREEN, ONLP_LED_MODE_GREEN},
{LED_MODULE_2, CPLD_SYSTEM_LED_CONTROL_OFFSET_2, CPLD_LED_MODULE_2_REG_MASK, CPLD_LED_MODULE_2_AMBER, ONLP_LED_MODE_ORANGE}
};

static volatile uint8_t* cpld_base__ = NULL;

static int
led_cpld_val_to_light_mode(enum onlp_led_id lid, unsigned char reg_val)
{
    int i;

    for (i = 0; i < sizeof(led_data)/sizeof(led_data[0]); i++)
    {
        if (lid != led_data[i].lid)
            continue;

        if ((led_data[i].cpld_mask & reg_val) == led_data[i].cpld_val)
            return led_data[i].mode;
    }

    return ONLP_LED_MODE_OFF;
}

static unsigned char
led_light_mode_to_cpld_val(onlp_led_id_t lid, onlp_led_mode_t mode, unsigned char orig_val)
{
    int i;

    for (i = 0; i < sizeof(led_data)/sizeof(led_data[0]); i++)
    {
        if (lid != led_data[i].lid || mode != led_data[i].mode)
            continue;

        orig_val = led_data[i].cpld_val | (orig_val & (~led_data[i].cpld_mask));
    }

    return orig_val;
}

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    /*
     * Map the CPLD address
     */
    cpld_base__ = onlp_mmap(CPLD_BASE_ADDRESS, getpagesize(), __FILE__);
    if(cpld_base__ == NULL || cpld_base__ == MAP_FAILED) {
        return ONLP_STATUS_E_INTERNAL;
    }


    /*
     * Diag LED Off
     */
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_DIAG), ONLP_LED_MODE_OFF);

    /*
     * M1 and M2 LEDs off
     */
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(6), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(7), ONLP_LED_MODE_OFF);

    return ONLP_STATUS_OK;
}

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
    {
        { }, /* Not used */
        {
            { ONLP_LED_ID_CREATE(1), "Chassis LED 1 (DIAG LED)", 0 },
            ONLP_LED_STATUS_PRESENT,
            ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
            ONLP_LED_MODE_OFF
        },
        {
            { ONLP_LED_ID_CREATE(2), "Chassis LED 2 (PSU1 LED)", 0 },
            ONLP_LED_STATUS_PRESENT,
            ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
            ONLP_LED_MODE_OFF
        },
        {
            { ONLP_LED_ID_CREATE(3), "Chassis LED 3 (PSU2 LED)", 0 },
            ONLP_LED_STATUS_PRESENT,
            ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
            ONLP_LED_MODE_OFF
        },
        {
            { ONLP_LED_ID_CREATE(4), "Chassis LED 4 (FAN1 LED)", 0 },
            ONLP_LED_STATUS_PRESENT,
            ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
            ONLP_LED_MODE_OFF
        },
        {
            { ONLP_LED_ID_CREATE(5), "Chassis LED 5 (FAN2 LED)", 0 },
            ONLP_LED_STATUS_PRESENT,
            ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
            ONLP_LED_MODE_OFF
        },
        {
            { ONLP_LED_ID_CREATE(6), "Chassis LED 6 (MODULE1 LED)", 0 },
            ONLP_LED_STATUS_PRESENT,
            ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
            ONLP_LED_MODE_OFF
        },
        {
            { ONLP_LED_ID_CREATE(7), "Chassis LED 7 (MODULE2 LED)", 0 },
            ONLP_LED_STATUS_PRESENT,
            ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_ORANGE,
            ONLP_LED_MODE_OFF
        },
    };

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int i;
    unsigned char data = 0, reg = 0;

    *info = linfo[ONLP_OID_ID_GET(id)];

    for (i = 0; i < sizeof(led_data)/sizeof(led_data[0]); i++)
    {
        if (ONLP_OID_ID_GET(id) != led_data[i].lid)
            continue;

        reg = led_data[i].cpld_offset;
    }

    /* Set the mode () */
    info->mode = 0;

    data = cpld_base__[reg];
    info->mode |= led_cpld_val_to_light_mode(ONLP_OID_ID_GET(id), data);

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
    return ONLP_STATUS_OK;
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
    int i = 0;
    unsigned char data = 0, reg;

    for (i = 0; i < sizeof(led_data)/sizeof(led_data[0]); i++)
    {
        if (ONLP_OID_ID_GET(id) != led_data[i].lid)
            continue;

        reg = led_data[i].cpld_offset;
        break;
    }

    data = cpld_base__[reg];
    cpld_base__[reg] = led_light_mode_to_cpld_val(ONLP_OID_ID_GET(id), mode, data);

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


