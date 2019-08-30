/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *        Copyright 2018 Alpha Networks Incorporation.
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
 * LED Management
 *
 ***********************************************************/
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/fani.h>
#include <sys/mman.h>
#include <stdio.h>
#include "platform_lib.h"

#include <onlplib/mmap.h>
//#include "onlpie_int.h"

#define CPLD_PSU_LED_ADDRESS_OFFSET     0x0A //PSU0 [1:0], PSU1 [3:2]
#define CPLD_SYS_LED_ADDRESS_OFFSET     0x0B //system [2:0], power [4:3], fan [6:5]

#define POWER_LED_OFF                     0x00
#define POWER_LED_GREEN_SOLID             0x01
#define POWER_LED_AMBER_BLINKING          0x02

#define PSU_LED_OFF                     0x00
#define PSU_LED_GREEN_SOLID             0x01
#define PSU_LED_AMBER_BLINKING          0x02

#define SYSTEM_LED_OFF                     0x00
#define SYSTEM_LED_GREEN_SOLID             0x02
#define SYSTEM_LED_GREEN_BLINKING          0x04
#define SYSTEM_LED_AMBER_SOLID             0x06
#define SYSTEM_LED_AMBER_BLINKING          0x07

#define FAN_LED_OFF                     0x00
#define FAN_LED_GREEN_SOLID             0x01
#define FAN_LED_AMBER_BLINKING          0x02

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

struct led_id_mode
{
    enum onlp_led_id led_id;
    onlp_led_mode_t mode;
    int hw_led_light_mode;
};

static struct led_id_mode led_id_mode_data[] = {
    { LED_POWER, ONLP_LED_MODE_OFF, POWER_LED_OFF },
    { LED_POWER, ONLP_LED_MODE_GREEN, POWER_LED_GREEN_SOLID },
    { LED_POWER, ONLP_LED_MODE_ORANGE_BLINKING, POWER_LED_AMBER_BLINKING },
    { LED_POWER, ONLP_LED_MODE_ON, POWER_LED_GREEN_SOLID },

    { LED_PSU1, ONLP_LED_MODE_OFF, PSU_LED_OFF },
    { LED_PSU1, ONLP_LED_MODE_GREEN, PSU_LED_GREEN_SOLID },
    { LED_PSU1, ONLP_LED_MODE_ORANGE_BLINKING, PSU_LED_AMBER_BLINKING },
    { LED_PSU1, ONLP_LED_MODE_ON, PSU_LED_GREEN_SOLID },

    { LED_PSU2, ONLP_LED_MODE_OFF, PSU_LED_OFF },
    { LED_PSU2, ONLP_LED_MODE_GREEN, PSU_LED_GREEN_SOLID },
    { LED_PSU2, ONLP_LED_MODE_ORANGE_BLINKING, PSU_LED_AMBER_BLINKING },
    { LED_PSU2, ONLP_LED_MODE_ON, PSU_LED_GREEN_SOLID },

    { LED_SYSTEM, ONLP_LED_MODE_OFF, SYSTEM_LED_OFF },
    { LED_SYSTEM, ONLP_LED_MODE_GREEN, SYSTEM_LED_GREEN_SOLID },
    { LED_SYSTEM, ONLP_LED_MODE_GREEN_BLINKING, SYSTEM_LED_GREEN_BLINKING },
    { LED_SYSTEM, ONLP_LED_MODE_ORANGE, SYSTEM_LED_AMBER_SOLID },
    { LED_SYSTEM, ONLP_LED_MODE_ORANGE_BLINKING, SYSTEM_LED_AMBER_BLINKING },
    { LED_SYSTEM, ONLP_LED_MODE_ON, SYSTEM_LED_GREEN_SOLID },

    { LED_FAN, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_FAN, ONLP_LED_MODE_GREEN, FAN_LED_GREEN_SOLID },
    { LED_FAN, ONLP_LED_MODE_ORANGE_BLINKING, FAN_LED_AMBER_BLINKING },
    { LED_FAN, ONLP_LED_MODE_ON, FAN_LED_GREEN_SOLID },

};

typedef union
{
    unsigned char val;
    struct
    {
        unsigned system :3;
        unsigned char power :2;
        unsigned char fan_err :2;
        unsigned char :1;  /* reserved */
    }bit;

}_CPLD_SYSTEM_LED_REG_T;

typedef union
{
    unsigned char val;
    struct
    {
        unsigned char psu0 :2;
        unsigned char psu1 :2;
        unsigned char :4;  /* reserved */
    }bit;

}_CPLD_PSU_LED_REG_T;

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_POWER), "Chassis LED 1 (POWER LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN| ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU1), "Chassis LED 2 (PSU1 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN| ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU2), "Chassis LED 3 (PSU2 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN| ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_SYSTEM), "Chassis LED 4 (SYSTEM LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "Chassis LED 5 (FAN LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN| ONLP_LED_CAPS_ORANGE_BLINKING,
    },
};

static int convert_hw_led_light_mode_to_onlp(enum onlp_led_id id, int hw_mode)
{
    int i, nsize = sizeof(led_id_mode_data) / sizeof(led_id_mode_data[0]);
    for (i = 0; i < nsize; i++)
    {
        if ((led_id_mode_data[i].led_id == id) &&
            (led_id_mode_data[i].hw_led_light_mode == hw_mode))
        {
            DIAG_PRINT("%s, id:%d, hw_mode:%d mode:%d", __FUNCTION__, id, hw_mode, led_id_mode_data[i].mode);
            return (int)led_id_mode_data[i].mode;
        }
    }

    return -1;
}

static int convert_onlp_led_light_mode_to_hw(enum onlp_led_id id, onlp_led_mode_t mode)
{
    int i, nsize = sizeof(led_id_mode_data) / sizeof(led_id_mode_data[0]);
    for (i = 0; i < nsize; i++)
    {
        if ((led_id_mode_data[i].led_id == id) &&
            (led_id_mode_data[i].mode == mode))
        {
            DIAG_PRINT("%s, id:%d, mode:%d hw_mode:%d", __FUNCTION__, id, mode, led_id_mode_data[i].hw_led_light_mode);
            return led_id_mode_data[i].hw_led_light_mode;
        }
    }

    return -1;
}

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
#if 0
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_SERVICE), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_STACKING), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PWR), ONLP_LED_MODE_OFF);

    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN1), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN2), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN3), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN4), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN5), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN6), ONLP_LED_MODE_OFF);

    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN1), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN2), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN3), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN4), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN5), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN6), ONLP_LED_MODE_OFF);
#endif
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t *info)
{
    DIAG_PRINT("%s, id=%d", __FUNCTION__, id);
    _CPLD_SYSTEM_LED_REG_T system_led_reg;
    _CPLD_PSU_LED_REG_T psu_led_reg;
    char data = 0;
    int ret = 0, local_id = 0;

    VALIDATE(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    local_id = ONLP_OID_ID_GET(id);

    switch (local_id)
    {
        case LED_POWER:
        case LED_SYSTEM:
        case LED_FAN:
            ret = bmc_i2c_read_byte(BMC_CPLD_I2C_BUS_ID, BMC_CPLD_I2C_ADDR, CPLD_SYS_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            system_led_reg.val = data;
            break;
            
        case LED_PSU1:
        case LED_PSU2:
            ret = bmc_i2c_read_byte(BMC_CPLD_I2C_BUS_ID, BMC_CPLD_I2C_ADDR, CPLD_PSU_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            psu_led_reg.val = data;
            break;         
            
        default :
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, local_id);
            return ONLP_STATUS_E_PARAM;
    }

    /* Get LED status */
    switch (local_id)
    {
        case LED_POWER:
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, system_led_reg.bit.power);
            break;
        case LED_PSU1:
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, psu_led_reg.bit.psu0);
            break;
        case LED_PSU2:
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, psu_led_reg.bit.psu1);
            break;
        case LED_SYSTEM:
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, system_led_reg.bit.system);
            break;
        case LED_FAN:
            //debug
            //printf("[%s]  fan_led_reg.val:0x%x \n", __FUNCTION__,  system_led_reg.val);
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, system_led_reg.bit.fan_err);
            break;

        default:
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, local_id);
            return ONLP_STATUS_E_PARAM;
    }

    /* Set the on/off status */
    if (info->mode != ONLP_LED_MODE_OFF)
    {
        info->status |= ONLP_LED_STATUS_ON;
    }

//debug
//printf("[%s] local_id:%d info->mode:%d info->status:%d\n", __FUNCTION__, local_id, info->mode, info->status);

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
    DIAG_PRINT("%s, id=%d, on_or_off=%d", __FUNCTION__, id, on_or_off);
    VALIDATE(id);

    if (!on_or_off)
    {
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
    }

    return onlp_ledi_mode_set(id, ONLP_LED_MODE_ON);
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
    DIAG_PRINT("%s, id=%d, mode=%d", __FUNCTION__, id, mode);
    _CPLD_SYSTEM_LED_REG_T system_led_reg;
    _CPLD_PSU_LED_REG_T psu_led_reg;
    char data = 0;
    int ret = 0, local_id = 0, hw_led_mode = 0;

    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    switch (local_id)
    {
        case LED_POWER:
        case LED_SYSTEM:
        case LED_FAN:
            ret = bmc_i2c_read_byte(BMC_CPLD_I2C_BUS_ID, BMC_CPLD_I2C_ADDR, CPLD_SYS_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            system_led_reg.val = data;
            break;
            
        case LED_PSU1:
        case LED_PSU2:
            ret = bmc_i2c_read_byte(BMC_CPLD_I2C_BUS_ID, BMC_CPLD_I2C_ADDR, CPLD_PSU_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            psu_led_reg.val = data;
            break;

        default :
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, local_id);
            return ONLP_STATUS_E_PARAM;
    }

    hw_led_mode = convert_onlp_led_light_mode_to_hw(local_id, mode);
    if (hw_led_mode < 0)
        return ONLP_STATUS_E_PARAM;

    /* Set LED light mode */
    switch (local_id)
    {
        case LED_POWER:
            system_led_reg.bit.power = hw_led_mode;
            break;
        case LED_PSU1:
            psu_led_reg.bit.psu0 = hw_led_mode;
            break;
        case LED_PSU2:
            psu_led_reg.bit.psu1 = hw_led_mode;
            break;
        case LED_SYSTEM:
            system_led_reg.bit.system = hw_led_mode;
            break;
        case LED_FAN:
            system_led_reg.bit.fan_err = hw_led_mode;
            break;
        default:
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, local_id);
            return ONLP_STATUS_E_PARAM;
    }

    switch (local_id)
    {
        case LED_POWER:
        case LED_SYSTEM:
        case LED_FAN:            
            //printf("[debug]service_led_reg.val:0x%x\n", service_led_reg.val);
            ret = bmc_i2c_write_byte(BMC_CPLD_I2C_BUS_ID, BMC_CPLD_I2C_ADDR, CPLD_SYS_LED_ADDRESS_OFFSET, system_led_reg.val);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            break;
        case LED_PSU1:
        case LED_PSU2:
            ret = bmc_i2c_write_byte(BMC_CPLD_I2C_BUS_ID, BMC_CPLD_I2C_ADDR, CPLD_PSU_LED_ADDRESS_OFFSET, psu_led_reg.val);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            break;

        default :
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, local_id);
            return ONLP_STATUS_E_PARAM;
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

