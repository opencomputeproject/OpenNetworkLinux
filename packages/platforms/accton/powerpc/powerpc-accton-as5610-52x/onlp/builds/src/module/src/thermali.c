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
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <onlp/platformi/thermali.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "platform_lib.h"
#include <onlplib/file.h>

/* i2c device info */
#define I2C_PSU_BUS_ID                             0
#define I2C_THERMAL_SENSOR_BUS_ID                  0
#define I2C_SLAVE_ADDR_PCA9548                  0x70
#define I2C_SLAVE_ADDR_MAX6581                  0x4D
#define I2C_SLAVE_ADDR_NE1617A                  0x18

#define I2C_REG_MAX6581_CONFIG                  0x41
#define I2C_MAX6581_REG_EXTENDED_RANGE_MASK     0x02

#define I2C_PSU1_SLAVE_ADDR_CFG      0x3E
#define I2C_PSU2_SLAVE_ADDR_CFG      0x3D

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static int
thermal_sensor_max6581_base_degree(void)
{
    unsigned char data = 0;

    /* Read configuration register to see if decimal place is supported
     */
    if (i2c_nRead(I2C_THERMAL_SENSOR_BUS_ID, I2C_SLAVE_ADDR_MAX6581, I2C_REG_MAX6581_CONFIG, sizeof(data), &data) != 0) {
        return 0;
    }

    return (data & I2C_MAX6581_REG_EXTENDED_RANGE_MASK) ? -64 : 0;
}

static int
thermal_sensor_max6581_info_get(onlp_thermal_info_t* info)
{
    unsigned char data = 0;
    unsigned char temp_reg[8]    = { 0x7,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,  0x8};
    unsigned char temp_reg_dp[8] = {0x57, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x58}; /* Register to read decimal place */
    int base_degree = thermal_sensor_max6581_base_degree();
    int index       = ONLP_OID_ID_GET(info->hdr.id) - MAX6581_LOCAL_SENSOR;

    /* Set the thermal status from diode fault(0x46) register */
    if (i2c_nRead(I2C_THERMAL_SENSOR_BUS_ID, I2C_SLAVE_ADDR_MAX6581, 0x46, sizeof(data), &data) != 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (data >> index & 0x1) {
        info->status |= ONLP_THERMAL_STATUS_FAILED;
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Read Main Temperature Register (High-Byte) Data*/
    if (i2c_nRead(I2C_THERMAL_SENSOR_BUS_ID, I2C_SLAVE_ADDR_MAX6581, temp_reg[index], sizeof(data), &data) != 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    info->mcelsius = (data + base_degree) * TEMPERATURE_MULTIPLIER;

    /* Read Extended-Resolution Temperature Register (Low-Byte) Data*/
    if (i2c_nRead(I2C_THERMAL_SENSOR_BUS_ID, I2C_SLAVE_ADDR_MAX6581, temp_reg_dp[index], sizeof(data), &data) != 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    info->mcelsius += (data >> 5) * 125;
    //printf("Max6581 temperature(%d) = (%d)\r\n", ONLP_OID_ID_GET(info->hdr.id), info->mcelsius);

    return ONLP_STATUS_OK;
}

static int
thermal_sensor_ne1617a_info_get(onlp_thermal_info_t* info)
{
    unsigned char data = 0;
    unsigned char temp_reg[2] = {0x0, 0x1};
    int index = ONLP_OID_ID_GET(info->hdr.id) - NE1617A_LOCAL_SENSOR;

    if (i2c_nRead(I2C_THERMAL_SENSOR_BUS_ID, I2C_SLAVE_ADDR_NE1617A, temp_reg[index], sizeof(data), &data) != 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    info->mcelsius = data * TEMPERATURE_MULTIPLIER;
    //printf("NE1617A temperature(%d) = (%d)\r\n", ONLP_OID_ID_GET(info->hdr.id), info->mcelsius);

    return ONLP_STATUS_OK;
}

static int
thermal_sensor_cpr_4011_info_get(onlp_thermal_info_t* info)
{
    int temperature = 0;
    unsigned char status = 0;
    unsigned char i2c_addr;

    if (ONLP_OID_ID_GET(info->hdr.id) == PSU1_THERMAL_SENSOR_1) {
        i2c_addr = I2C_PSU1_SLAVE_ADDR_CFG;
    }
    else { /* PSU2_THERMAL_SENSOR_1 */
        i2c_addr = I2C_PSU2_SLAVE_ADDR_CFG;
    }

    /* Get the STATUS_TEMPERATURE described in the PMBUS spec */
    if (i2c_nRead(I2C_PSU_BUS_ID, i2c_addr, 0x7D, sizeof(status), &status) != 0)
        return ONLP_STATUS_E_INTERNAL;

    if (status & 0x90) {
        info->status |= ONLP_THERMAL_STATUS_FAILED;
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Get the temperature */
    if (pmbus_read_literal_data(I2C_PSU_BUS_ID, i2c_addr, 0x8D, &temperature) != 0)
        return ONLP_STATUS_E_INTERNAL;

    info->caps |= ONLP_THERMAL_CAPS_GET_TEMPERATURE;
    info->mcelsius = temperature;

    return ONLP_STATUS_OK;
}

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
    unsigned char data = 0;

    if (as5610_52x_i2c0_pca9548_channel_set(0x80) != 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Configure max6581 to enable the functionality to support decimal place reading */
    if (i2c_nRead(I2C_THERMAL_SENSOR_BUS_ID, I2C_SLAVE_ADDR_MAX6581, I2C_REG_MAX6581_CONFIG, sizeof(data), &data) != 0) {
        as5610_52x_i2c0_pca9548_channel_set(0);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (data & I2C_MAX6581_REG_EXTENDED_RANGE_MASK) {
        as5610_52x_i2c0_pca9548_channel_set(0);
        return ONLP_STATUS_OK; /* extended range is already enabled */
    }

    data |= I2C_MAX6581_REG_EXTENDED_RANGE_MASK; /* Enable extend range bit */

    if (i2c_nWrite(I2C_THERMAL_SENSOR_BUS_ID, I2C_SLAVE_ADDR_MAX6581, I2C_REG_MAX6581_CONFIG, sizeof(data), &data) != 0) {
        as5610_52x_i2c0_pca9548_channel_set(0);
        return ONLP_STATUS_E_INTERNAL;
    }

    sleep(1);

    return ONLP_STATUS_OK;
}

/* Static values */
static onlp_thermal_info_t tinfo[] = {
{ }, /* Not used */
{ { ONLP_THERMAL_ID_CREATE(NE1617A_LOCAL_SENSOR),  "Chassis Thermal Sensor 1 (NE1617A local sensor)", 0}, 0x1,
    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
},
{ { ONLP_THERMAL_ID_CREATE(NE1617A_REMOTE_SENSOR),  "Chassis Thermal Sensor 2 (the left middle side of the system)", 0}, 0x1,
    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
},
{ { ONLP_THERMAL_ID_CREATE(MAX6581_LOCAL_SENSOR),  "Chassis Thermal Sensor 3 (MAX6581 local sensor)", 0}, 0x1,
    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
},
{ { ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_1),  "Chassis Thermal Sensor 4 (near right-upper side of CPU)", 0}, 0x1,
    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
},
{ { ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_2),  "Chassis Thermal Sensor 5 (near right side of MAX6581)", 0}, 0x1,
    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
},
{ { ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_3),  "Chassis Thermal Sensor 6 (near right side of MAC(Trident+)", 0}, 0x1,
    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
},
{ { ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_4),  "Chassis Thermal Sensor 7 (near left down side of Equalizer_U57)", 0}, 0x1,
    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
},
{ { ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_5),  "Chassis Thermal Sensor 8 (near right down side of MAC)", 0}, 0x1,
    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
},
{ { ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_6),  "Chassis Thermal Sensor 9 (near upper side of Equalizer_U49)", 0}, 0x1,
    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
},
{ { ONLP_THERMAL_ID_CREATE(MAX6581_REMOTE_SENSOR_7),  "Chassis Thermal Sensor 10 (near left down side of PCB)", 0}, 0x1,
    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
},
{ { ONLP_THERMAL_ID_CREATE(BCM56846_LOCAL_SENSOR),  "Chassis Thermal Sensor 11 (BCM56846 local sensor)", 0}, 0x1,
    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
},
{ { ONLP_THERMAL_ID_CREATE(PSU1_THERMAL_SENSOR_1),  "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(1)}, 0x1,
    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
},
{ { ONLP_THERMAL_ID_CREATE(PSU2_THERMAL_SENSOR_1), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(2)}, 0x1,
    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
}
};

/*  
 * Retrieve the information structure for the given thermal OID.
 *
 * If the OID is invalid, return ONLP_E_STATUS_INVALID.
 * If an unexpected error occurs, return ONLP_E_STATUS_INTERNAL.
 * Otherwise, return ONLP_STATUS_OK with the OID's information.
 *
 * Note -- it is expected that you fill out the information
 * structure even if the sensor described by the OID is not present.
 */
int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    int ret = ONLP_STATUS_E_UNSUPPORTED;
    VALIDATE(id);
    *info = tinfo[ONLP_OID_ID_GET(id)];  /* Set the ID/Description/Present state */

    if (ONLP_OID_ID_GET(id) == BCM56846_LOCAL_SENSOR) {
        /* Switch Thermal Sensor */
        char* fname = "/var/run/broadcom/temp0";
        int rv = onlp_file_read_int(&info->mcelsius, fname);
        if(rv >= 0) {
            /** Present and running */
            info->status |= 1;
            ret = ONLP_STATUS_OK;
        }
        else if(rv == ONLP_STATUS_E_MISSING) {
            /** No switch management process running. */
            info->status = 0;
            ret = ONLP_STATUS_OK;
        }
        else {
            /** Other error. */
            ret = ONLP_STATUS_E_INTERNAL;
        }
        return ret;
    }

    /* Read the temperature and status */
    if (ONLP_OID_ID_GET(id) >= MAX6581_LOCAL_SENSOR && ONLP_OID_ID_GET(id) <= MAX6581_REMOTE_SENSOR_7) {
        /* Set multiplexer to the channel of thermal sensor */
        if (as5610_52x_i2c0_pca9548_channel_set(0x80) != 0) {
            return ONLP_STATUS_E_INTERNAL;
        }

        ret = thermal_sensor_max6581_info_get(info);
    }

    if (ONLP_OID_ID_GET(id) == NE1617A_LOCAL_SENSOR || ONLP_OID_ID_GET(id) == NE1617A_REMOTE_SENSOR) {
        /* Set multiplexer to the channel of thermal sensor */
        if (as5610_52x_i2c0_pca9548_channel_set(0x80) != 0) {
            return ONLP_STATUS_E_INTERNAL;
        }

        ret = thermal_sensor_ne1617a_info_get(info);
    }

    if (ONLP_OID_ID_GET(id) == PSU1_THERMAL_SENSOR_1 || ONLP_OID_ID_GET(id) == PSU2_THERMAL_SENSOR_1) {
        /* Check PSU type */
        as5610_52x_psu_type_t psu_type;
        psu_type = as5610_52x_get_psu_type(ONLP_OID_ID_GET(id)-NUM_OF_CHASSIS_THERMAL_SENSOR, NULL, 0);

        if (PSU_TYPE_AC_F2B == psu_type || PSU_TYPE_AC_B2F == psu_type) {
            /* Set multiplexer to the channel of thermal sensor */
            if (as5610_52x_i2c0_pca9548_channel_set(0x6) != 0) {
                return ONLP_STATUS_E_INTERNAL;
            }

            ret = thermal_sensor_cpr_4011_info_get(info);
        }
    }

    /* Close PSU channel
     */
    as5610_52x_i2c0_pca9548_channel_set(0);

    return ret;
}

