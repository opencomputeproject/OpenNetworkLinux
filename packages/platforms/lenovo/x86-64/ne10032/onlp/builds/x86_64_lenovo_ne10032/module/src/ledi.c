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

#define CPLD_I2C_BUS_ID                 8
#define CPLD_I2C_ADDR                   0x5F  /* System CPLD Physical Address in the I2C */
#define CPLD_SYS_LED_ADDRESS_OFFSET     0x08
#define CPLD_FAN1_LED_ADDRESS_OFFSET    0x09 //Fan LED Register 0
#define CPLD_FAN2_LED_ADDRESS_OFFSET    0x09
#define CPLD_FAN3_LED_ADDRESS_OFFSET    0x0A //Fan LED Register 1
#define CPLD_FAN4_LED_ADDRESS_OFFSET    0x0A
#define CPLD_FAN5_LED_ADDRESS_OFFSET    0x0B //Fan LED Register 2
#define CPLD_FAN6_LED_ADDRESS_OFFSET    0x0B
#define CPLD_FAN1_REAR_LED_ADDRESS_OFFSET    0x0E //Fan Rear LED Register 0
#define CPLD_FAN2_REAR_LED_ADDRESS_OFFSET    0x0E
#define CPLD_FAN3_REAR_LED_ADDRESS_OFFSET    0x0F //Fan Rear LED Register 1
#define CPLD_FAN4_REAR_LED_ADDRESS_OFFSET    0x0F
#define CPLD_FAN5_REAR_LED_ADDRESS_OFFSET    0x010 //Fan Rear LED Register 2
#define CPLD_FAN6_REAR_LED_ADDRESS_OFFSET    0x010
#define CPLD_SERVICE_LED_ADDRESS_OFFSET 0x11 //Service Blue LED Register


#define STACKING_LED_OFF                  0x00
#define STACKING_LED_AMBER_SOLID          0x01
#define STACKING_LED_AMBER_BLINKING       0x03
#define STACKING_LED_GREEN_SOLID          0x05
#define STACKING_LED_GREEN_BLINKING       0x07

#define PWR_LED_OFF                     0x00
#define PWR_LED_AMBER_SOLID             0x01
#define PWR_LED_AMBER_BLINKING          0x03
#define PWR_LED_GREEN_SOLID             0x05
#define PWR_LED_GREEN_BLINKING          0x07

#define SERVICE_LED_OFF                 0x00
#define SERVICE_LED_BLUE_SOLID          0x01
#define SERVICE_LED_BLUE_BLINKING       0x03

#define FAN_LED_OFF                     0x00
#define FAN_LED_AMBER_SOLID             0x01
#define FAN_LED_AMBER_BLINKING          0x03
#define FAN_LED_GREEN_SOLID             0x05
#define FAN_LED_GREEN_BLINKING          0x07

#define REAR_FAN_LED_GREEN_SOLID        0x01
#define REAR_FAN_LED_GREEN_BLINKING     0x03

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
    { LED_SERVICE, ONLP_LED_MODE_OFF, SERVICE_LED_OFF },
    { LED_SERVICE, ONLP_LED_MODE_BLUE, SERVICE_LED_BLUE_SOLID },
    { LED_SERVICE, ONLP_LED_MODE_BLUE_BLINKING, SERVICE_LED_BLUE_BLINKING },
    { LED_SERVICE, ONLP_LED_MODE_ON, SERVICE_LED_BLUE_SOLID },

    { LED_STACKING, ONLP_LED_MODE_OFF, STACKING_LED_OFF },
    { LED_STACKING, ONLP_LED_MODE_GREEN, STACKING_LED_GREEN_SOLID },
    { LED_STACKING, ONLP_LED_MODE_GREEN_BLINKING, STACKING_LED_GREEN_BLINKING },
    { LED_STACKING, ONLP_LED_MODE_ORANGE, STACKING_LED_AMBER_SOLID },
    { LED_STACKING, ONLP_LED_MODE_ORANGE_BLINKING, STACKING_LED_AMBER_BLINKING },
    { LED_STACKING, ONLP_LED_MODE_ON, STACKING_LED_GREEN_SOLID },

    { LED_PWR, ONLP_LED_MODE_OFF, PWR_LED_OFF },
    { LED_PWR, ONLP_LED_MODE_GREEN, PWR_LED_GREEN_SOLID },
    { LED_PWR, ONLP_LED_MODE_GREEN_BLINKING, PWR_LED_GREEN_BLINKING },
    { LED_PWR, ONLP_LED_MODE_ORANGE, PWR_LED_AMBER_SOLID },
    { LED_PWR, ONLP_LED_MODE_ORANGE_BLINKING, PWR_LED_AMBER_BLINKING },
    { LED_PWR, ONLP_LED_MODE_ON, PWR_LED_GREEN_SOLID },

    { LED_FAN1, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_FAN1, ONLP_LED_MODE_GREEN, FAN_LED_GREEN_SOLID },
    { LED_FAN1, ONLP_LED_MODE_GREEN_BLINKING, FAN_LED_GREEN_BLINKING },
    { LED_FAN1, ONLP_LED_MODE_ORANGE, FAN_LED_AMBER_SOLID },
    { LED_FAN1, ONLP_LED_MODE_ORANGE_BLINKING, FAN_LED_AMBER_BLINKING },
    { LED_FAN1, ONLP_LED_MODE_ON, FAN_LED_GREEN_SOLID },

    { LED_FAN2, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_FAN2, ONLP_LED_MODE_GREEN, FAN_LED_GREEN_SOLID },
    { LED_FAN2, ONLP_LED_MODE_GREEN_BLINKING, FAN_LED_GREEN_BLINKING },
    { LED_FAN2, ONLP_LED_MODE_ORANGE, FAN_LED_AMBER_SOLID },
    { LED_FAN2, ONLP_LED_MODE_ORANGE_BLINKING, FAN_LED_AMBER_BLINKING },
    { LED_FAN2, ONLP_LED_MODE_ON, FAN_LED_GREEN_SOLID },

    { LED_FAN3, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_FAN3, ONLP_LED_MODE_GREEN, FAN_LED_GREEN_SOLID },
    { LED_FAN3, ONLP_LED_MODE_GREEN_BLINKING, FAN_LED_GREEN_BLINKING },
    { LED_FAN3, ONLP_LED_MODE_ORANGE, FAN_LED_AMBER_SOLID },
    { LED_FAN3, ONLP_LED_MODE_ORANGE_BLINKING, FAN_LED_AMBER_BLINKING },
    { LED_FAN3, ONLP_LED_MODE_ON, FAN_LED_GREEN_SOLID },

    { LED_FAN4, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_FAN4, ONLP_LED_MODE_GREEN, FAN_LED_GREEN_SOLID },
    { LED_FAN4, ONLP_LED_MODE_GREEN_BLINKING, FAN_LED_GREEN_BLINKING },
    { LED_FAN4, ONLP_LED_MODE_ORANGE, FAN_LED_AMBER_SOLID },
    { LED_FAN4, ONLP_LED_MODE_ORANGE_BLINKING, FAN_LED_AMBER_BLINKING },
    { LED_FAN4, ONLP_LED_MODE_ON, FAN_LED_GREEN_SOLID },

    { LED_FAN5, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_FAN5, ONLP_LED_MODE_GREEN, FAN_LED_GREEN_SOLID },
    { LED_FAN5, ONLP_LED_MODE_GREEN_BLINKING, FAN_LED_GREEN_BLINKING },
    { LED_FAN5, ONLP_LED_MODE_ORANGE, FAN_LED_AMBER_SOLID },
    { LED_FAN5, ONLP_LED_MODE_ORANGE_BLINKING, FAN_LED_AMBER_BLINKING },
    { LED_FAN5, ONLP_LED_MODE_ON, FAN_LED_GREEN_SOLID },

    { LED_FAN6, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_FAN6, ONLP_LED_MODE_GREEN, FAN_LED_GREEN_SOLID },
    { LED_FAN6, ONLP_LED_MODE_GREEN_BLINKING, FAN_LED_GREEN_BLINKING },
    { LED_FAN6, ONLP_LED_MODE_ORANGE, FAN_LED_AMBER_SOLID },
    { LED_FAN6, ONLP_LED_MODE_ORANGE_BLINKING, FAN_LED_AMBER_BLINKING },
    { LED_FAN6, ONLP_LED_MODE_ON, FAN_LED_GREEN_SOLID },

    { LED_REAR_FAN1, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_REAR_FAN1, ONLP_LED_MODE_GREEN, REAR_FAN_LED_GREEN_SOLID },
    { LED_REAR_FAN1, ONLP_LED_MODE_GREEN_BLINKING, REAR_FAN_LED_GREEN_BLINKING },
    { LED_REAR_FAN1, ONLP_LED_MODE_ON, REAR_FAN_LED_GREEN_SOLID },

    { LED_REAR_FAN2, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_REAR_FAN2, ONLP_LED_MODE_GREEN, REAR_FAN_LED_GREEN_SOLID },
    { LED_REAR_FAN2, ONLP_LED_MODE_GREEN_BLINKING, REAR_FAN_LED_GREEN_BLINKING },
    { LED_REAR_FAN2, ONLP_LED_MODE_ON, REAR_FAN_LED_GREEN_SOLID },

    { LED_REAR_FAN3, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_REAR_FAN3, ONLP_LED_MODE_GREEN, REAR_FAN_LED_GREEN_SOLID },
    { LED_REAR_FAN3, ONLP_LED_MODE_GREEN_BLINKING, REAR_FAN_LED_GREEN_BLINKING },
    { LED_REAR_FAN3, ONLP_LED_MODE_ON, REAR_FAN_LED_GREEN_SOLID },

    { LED_REAR_FAN4, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_REAR_FAN4, ONLP_LED_MODE_GREEN, REAR_FAN_LED_GREEN_SOLID },
    { LED_REAR_FAN4, ONLP_LED_MODE_GREEN_BLINKING, REAR_FAN_LED_GREEN_BLINKING },
    { LED_REAR_FAN4, ONLP_LED_MODE_ON, REAR_FAN_LED_GREEN_SOLID },

    { LED_REAR_FAN5, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_REAR_FAN5, ONLP_LED_MODE_GREEN, REAR_FAN_LED_GREEN_SOLID },
    { LED_REAR_FAN5, ONLP_LED_MODE_GREEN_BLINKING, REAR_FAN_LED_GREEN_BLINKING },
    { LED_REAR_FAN5, ONLP_LED_MODE_ON, REAR_FAN_LED_GREEN_SOLID },

    { LED_REAR_FAN6, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_REAR_FAN6, ONLP_LED_MODE_GREEN, REAR_FAN_LED_GREEN_SOLID },
    { LED_REAR_FAN6, ONLP_LED_MODE_GREEN_BLINKING, REAR_FAN_LED_GREEN_BLINKING },
    { LED_REAR_FAN6, ONLP_LED_MODE_ON, REAR_FAN_LED_GREEN_SOLID },
};


typedef union
{
    unsigned char val;
    struct
    {
        unsigned char power :3;
        unsigned char :1;  /* reserved */
        unsigned char stacking :3;
        unsigned char :1;  /* reserved */
    }bit;

}_CPLD_SYSTEM_LED_REG_T;

typedef union
{
    unsigned char val;
    struct
    {
        unsigned char locator :2;
        unsigned char :6;  /* reserved */
    }bit;

}_CPLD_LOCATOR_LED_REG_T;

typedef union
{
    unsigned char val;
    struct
    {
        unsigned char fan_a :3;  /* fan_a : FAN1/FAN3/FAN5, fan_b : FAN2/FAN4/FAN6 */
        unsigned char :1;  /* reserved */
        unsigned char fan_b :3;
        unsigned char :1;  /* reserved */
    }bit;

}_CPLD_FAN_LED_REG_T;

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_SERVICE), "Chassis LED 1 (SERVICE LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_BLUE | ONLP_LED_CAPS_BLUE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_STACKING), "Chassis LED 2 (STACKING LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PWR), "Chassis LED 3 (POWER LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN1), "Chassis LED 4 (FAN1 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN2), "Chassis LED 5 (FAN2 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN3), "Chassis LED 6 (FAN3 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN4), "Chassis LED 7 (FAN4 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN5), "Chassis LED 8 (FAN5 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN6), "Chassis LED 9 (FAN6 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN1), "FAN Rear LED 1", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN2), "FAN Rear LED 2", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN3), "FAN Rear LED 3", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN4), "FAN Rear LED 4", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN5), "FAN Rear LED 5", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN6), "FAN Rear LED 6", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
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
    _CPLD_LOCATOR_LED_REG_T service_led_reg;
    _CPLD_SYSTEM_LED_REG_T system_led_reg;
    _CPLD_FAN_LED_REG_T fan_led_reg;
    char data = 0;
    int ret = 0, local_id = 0;

    VALIDATE(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    local_id = ONLP_OID_ID_GET(id);

    switch (local_id)
    {
        case LED_SERVICE:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_SERVICE_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            service_led_reg.val = data;
            break;
        case LED_PWR:
        case LED_STACKING:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_SYS_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            system_led_reg.val = data;
            break;
        case LED_FAN1:
        case LED_FAN2:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN1_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
            break;
        case LED_FAN3:
        case LED_FAN4:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN3_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
            break;
        case LED_FAN5:
        case LED_FAN6:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN5_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
            break;

        case LED_REAR_FAN1:
        case LED_REAR_FAN2:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN1_REAR_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
            break;
        case LED_REAR_FAN3:
        case LED_REAR_FAN4:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN3_REAR_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
            break;
        case LED_REAR_FAN5:
        case LED_REAR_FAN6:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN5_REAR_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
            break;
        default :
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, local_id);
            return ONLP_STATUS_E_PARAM;
    }

    /* Get LED status */
    switch (local_id)
    {
        case LED_SERVICE:
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, service_led_reg.bit.locator);
            break;
        case LED_PWR:
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, system_led_reg.bit.power);
            break;
        case LED_STACKING:
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, system_led_reg.bit.stacking);
            break;
        case LED_FAN1:
        case LED_FAN3:
        case LED_FAN5:
        case LED_REAR_FAN1:
        case LED_REAR_FAN3:
        case LED_REAR_FAN5:
            //debug
            //printf("[%s]  fan_led_reg.val:0x%x \n", __FUNCTION__,  fan_led_reg.val);
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, fan_led_reg.bit.fan_a);
            break;
        case LED_FAN2:
        case LED_FAN4:
        case LED_FAN6:
        case LED_REAR_FAN2:
        case LED_REAR_FAN4:
        case LED_REAR_FAN6:
            //debug
            //printf("[%s]  fan_led_reg.val:0x%x \n", __FUNCTION__,  fan_led_reg.val);
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, fan_led_reg.bit.fan_b);
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
    _CPLD_LOCATOR_LED_REG_T service_led_reg;
    _CPLD_SYSTEM_LED_REG_T system_led_reg;
    _CPLD_FAN_LED_REG_T fan_led_reg;
    char data = 0;
    int ret = 0, local_id = 0, hw_led_mode = 0;

    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    switch (local_id)
    {
        case LED_SERVICE:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_SERVICE_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            service_led_reg.val = data;
            break;
        case LED_PWR:
        case LED_STACKING:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_SYS_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            system_led_reg.val = data;
            break;
        case LED_FAN1:
        case LED_FAN2:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN1_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
            break;
        case LED_FAN3:
        case LED_FAN4:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN3_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
            break;
        case LED_FAN5:
        case LED_FAN6:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN5_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
            break;
        case LED_REAR_FAN1:
        case LED_REAR_FAN2:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN1_REAR_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
            break;
        case LED_REAR_FAN3:
        case LED_REAR_FAN4:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN3_REAR_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
            break;
        case LED_REAR_FAN5:
        case LED_REAR_FAN6:
            ret = i2c_read_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN5_REAR_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
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
        case LED_SERVICE:
            service_led_reg.bit.locator = hw_led_mode;
            break;
        case LED_PWR:
            system_led_reg.bit.power = hw_led_mode;
            break;
        case LED_STACKING:
            system_led_reg.bit.stacking = hw_led_mode;
            break;
        case LED_FAN1:
        case LED_FAN3:
        case LED_FAN5:
        case LED_REAR_FAN1:
        case LED_REAR_FAN3:
        case LED_REAR_FAN5:
            fan_led_reg.bit.fan_a = hw_led_mode;
            break;
        case LED_FAN2:
        case LED_FAN4:
        case LED_FAN6:
        case LED_REAR_FAN2:
        case LED_REAR_FAN4:
        case LED_REAR_FAN6:
            fan_led_reg.bit.fan_b = hw_led_mode;
            break;
        default:
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, local_id);
            return ONLP_STATUS_E_PARAM;
    }

    switch (local_id)
    {
        case LED_SERVICE:
            //printf("[debug]service_led_reg.val:0x%x\n", service_led_reg.val);
            ret = i2c_write_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_SERVICE_LED_ADDRESS_OFFSET, service_led_reg.val);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            break;
        case LED_PWR:
        case LED_STACKING:
            ret = i2c_write_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_SYS_LED_ADDRESS_OFFSET, system_led_reg.val);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            break;
        case LED_FAN1:
        case LED_FAN2:
            ret = i2c_write_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN1_LED_ADDRESS_OFFSET, fan_led_reg.val);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            break;
        case LED_FAN3:
        case LED_FAN4:
            ret = i2c_write_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN3_LED_ADDRESS_OFFSET, fan_led_reg.val);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            break;
        case LED_FAN5:
        case LED_FAN6:
            ret = i2c_write_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN5_LED_ADDRESS_OFFSET, fan_led_reg.val);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            break;
        case LED_REAR_FAN1:
        case LED_REAR_FAN2:
            ret = i2c_write_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN1_REAR_LED_ADDRESS_OFFSET, fan_led_reg.val);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
            break;
        case LED_REAR_FAN3:
        case LED_REAR_FAN4:
            ret = i2c_write_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN3_REAR_LED_ADDRESS_OFFSET, fan_led_reg.val);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
            break;
        case LED_REAR_FAN5:
        case LED_REAR_FAN6:
            ret = i2c_write_byte(CPLD_I2C_BUS_ID, CPLD_I2C_ADDR, CPLD_FAN5_REAR_LED_ADDRESS_OFFSET, fan_led_reg.val);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            fan_led_reg.val = data;
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

