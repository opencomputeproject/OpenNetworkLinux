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
#include <unistd.h>
#include <onlplib/mmap.h>
#include <onlp/platformi/thermali.h>
#include "powerpc_accton_as4600_54t_log.h"
#include "platform_lib.h"

/*-- i2c --*/
/* slave address */
#define I2C_SLAVE_ADDR_ADT7473   0x2E
#define I2C_SLAVE_ADDR_PCA9548   0x70
#define I2C_SLAVE_ADDR_LMP75     0x49
#define I2C_SLAVE_ADDR_PSU_1_PMBUS   0x1A
#define I2C_SLAVE_ADDR_PSU_2_PMBUS   0x19

#define I2C_PCA9548_PORT_7_BIT_MASK  0x80

/* chip register */
#define I2C_THERMAL_1_REG_ADT7473_LOCAL       0x26
#define I2C_THERMAL_2_REG_ADT7473_REMOTE_1    0x25
#define I2C_THERMAL_REG_ADT7473_CONFIGURE_REG_5    0x7C
#define I2C_THERMAL_REG_ADT7473_EXTENDED_RESOLUTION_1      0x76
#define I2C_THERMAL_REG_ADT7473_EXTENDED_RESOLUTION_2      0x77
#define I2C_THERMAL_REG_AC_PSU_CHICONY_TEMPERATURE         0x8D
#define I2C_THERMAL_REG_AC_PSU_CHICONY_STATUS              0x79

#define I2C_THERMAL_ADT7473_TEMPERATURE_FORMAT_BIT_MASK   0x1
#define I2C_THERMAL_REG_AC_PSU_CHICONY_STATUS_BIT_MASK    (1<<2)

#define THERMAL_ADT7473_EXT_RESOLUTION_BIT_MASK      0x3 /* capture 2 bit values*/
#define THERMAL_1_ADT7473_EXT_RESOLUTION_SHIFT   4   /* bit 5:4 */
#define THERMAL_2_ADT7473_EXT_RESOLUTION_SHIFT   2   /* bit 3:2 */

#define THERMAL_LMP75_RESOLUTION_BIT_MASK   0x3 /* capture 2 bit values*/
#define THERMAL_3_LMP75_RESOLUTION_SHIFT    6   /* bit 7:6 */

#define TEMPERATURE_MULTIPLIER       1000
#define TEMPERATURE_EXT_RESOLUTION    250  /* resolution is 0.25C */

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
    /* There are two format for temperature data in the chip ADT7473.
     * The offset 64 format. The range of temperature is -64C ~ 191C. (Default)
     * The two's complement format. The range of temperature is -128C ~ 127C.
     * Due to the temperature range of TMP75 is -128C ~ 127C, we change the format of ADT7473 to two's complement format.
     */
    unsigned int bus_id = 1;
    unsigned char i2c_addr = 0, i2c_port=0, temp_format=0;
    int rc=0;

    i2c_addr = I2C_SLAVE_ADDR_ADT7473;
    rc = I2C_nRead(bus_id, i2c_addr, I2C_THERMAL_REG_ADT7473_CONFIGURE_REG_5, 1, &temp_format);
    if(0 != rc)
    {
        return ONLP_STATUS_E_INTERNAL;
    }

    temp_format |= I2C_THERMAL_ADT7473_TEMPERATURE_FORMAT_BIT_MASK;
    rc = I2C_nWrite(bus_id, i2c_addr, I2C_THERMAL_REG_ADT7473_CONFIGURE_REG_5, 1, &temp_format);
    if(0 != rc)
    {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Open the I2C 0 channel 7 for LMP75.
     */
    bus_id = 0;
    i2c_addr = I2C_SLAVE_ADDR_PCA9548;
    i2c_port = (unsigned char)I2C_PCA9548_PORT_7_BIT_MASK;

    /* enable channel 7*/
    rc = I2C_nWrite(bus_id, i2c_addr, 0, 1, &i2c_port);
    if(0 != rc)
    {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* wait for system config.
     */
    sleep(1);

    return ONLP_STATUS_OK;
}


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
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* rv)
{
    /* Static values */
    onlp_thermal_info_t info[] = {
        { }, /* Not used */
        { { ONLP_THERMAL_ID_CREATE(1), "Chassis Thermal Sensor 1 (Nearby FAN 2)", 0}, 0x0, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
        { { ONLP_THERMAL_ID_CREATE(2), "Chassis Thermal Sensor 2 (CPU)", 0}, 0x0, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
        { { ONLP_THERMAL_ID_CREATE(3), "Chassis Thermal Sensor 3 (Nearby BCM MAC(56540) chip)", 0}, 0x0, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
        { { ONLP_THERMAL_ID_CREATE(4), "PSU-1 Thermal Sensor 1", 0}, 0x0, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 },
        { { ONLP_THERMAL_ID_CREATE(5), "PSU-2 Thermal Sensor 1", 0}, 0x0, ONLP_THERMAL_CAPS_GET_TEMPERATURE, 0 }
    };
    unsigned int bus_id = 1,tid = ONLP_OID_ID_GET(id);
    unsigned char i2c_addr, temp, temp_ext2, data[2]={0};
    unsigned short  psu_temp=0;
    int fractional_part=0, integer_part=0;
    int rc=0;

    *rv = info[ONLP_OID_ID_GET(id)];

    switch (tid)
    {
        case 1:  /* thermal sensor 1*/
        case 2:
            /* update the present. (always present.)
             */
            rv->status |= ONLP_THERMAL_STATUS_PRESENT;

            /* Get the information from I2C
             */
            i2c_addr = I2C_SLAVE_ADDR_ADT7473;

            /* Get the temperature.
             */
            rc = I2C_nRead(bus_id, i2c_addr, I2C_THERMAL_REG_ADT7473_EXTENDED_RESOLUTION_2, 1, &temp_ext2);
            if(0 != rc)
            {
                return ONLP_STATUS_E_INTERNAL;
            }

            if ( tid == 1)
            {
                rc = I2C_nRead(bus_id, i2c_addr, I2C_THERMAL_1_REG_ADT7473_LOCAL, 1, &temp);
                if(0 != rc)
                {
                    return ONLP_STATUS_E_INVALID;
                }

                fractional_part = (temp_ext2 >> THERMAL_1_ADT7473_EXT_RESOLUTION_SHIFT) & THERMAL_ADT7473_EXT_RESOLUTION_BIT_MASK;
            }
            else
            {
                rc = I2C_nRead(bus_id, i2c_addr, I2C_THERMAL_2_REG_ADT7473_REMOTE_1, 1, &temp);
                if(0 != rc)
                {
                    return ONLP_STATUS_E_INTERNAL;
                }

                fractional_part = (temp_ext2 >> THERMAL_2_ADT7473_EXT_RESOLUTION_SHIFT) & THERMAL_ADT7473_EXT_RESOLUTION_BIT_MASK;
            }
            /* update the temperature
             */
            integer_part = two_complement_to_int((unsigned short)temp, 8, 0xff);
            rv->mcelsius = (integer_part * TEMPERATURE_MULTIPLIER) + (fractional_part * TEMPERATURE_EXT_RESOLUTION);

            /* update status
             * Datasheet: temperature reading of -128 C (0x80) indicates a diode fault (open or short) on that channel.
             */
            if (temp == 0x80)
            {
                rv->status |= ONLP_THERMAL_STATUS_FAILED;
            }
            else
            {
                rv->status &= ~ONLP_THERMAL_STATUS_FAILED;
            }
            break;

        case 3:
            rv->status |= ONLP_THERMAL_STATUS_PRESENT;

            /*
             * Open the I2C 0 channel 7 for LMP75.
             */
            bus_id = 0;
            i2c_addr = I2C_SLAVE_ADDR_PCA9548;
            unsigned char i2c_port = (unsigned char)I2C_PCA9548_PORT_7_BIT_MASK;

            /* enable channel 7 */
            rc = I2C_nWrite(bus_id, i2c_addr, 0, 1, &i2c_port);
            if(0 != rc) {
                return ONLP_STATUS_E_INTERNAL;
            }

            /*
             * Get the information from I2C
             */
            bus_id = 0;
            i2c_addr = I2C_SLAVE_ADDR_LMP75;

            /* read 2 bytes
             */
            rc = I2C_nRead(bus_id, i2c_addr, 0, 2, data);

            /* update the temperature
             */
            temp = data[0];
            fractional_part = (data[1] >> THERMAL_3_LMP75_RESOLUTION_SHIFT) & THERMAL_LMP75_RESOLUTION_BIT_MASK;

            integer_part = two_complement_to_int((unsigned short)temp, 8, 0xff);
            rv->mcelsius = (integer_part * TEMPERATURE_MULTIPLIER) + (fractional_part * TEMPERATURE_EXT_RESOLUTION);

            /* This chip does not support status, we can't verify the fail case.
             */

            break;

        case 4:  /* PSU 1 */
        case 5:  /* PSU 2 */
            rv->status |= ONLP_THERMAL_STATUS_PRESENT;  /* always present. */

            /* Get the information from I2C
             */
            bus_id = 1;
            if (tid == 4)
            {
                i2c_addr = I2C_SLAVE_ADDR_PSU_1_PMBUS;
            }
            else
            {
                i2c_addr = I2C_SLAVE_ADDR_PSU_2_PMBUS;
            }

            rc = I2C_nRead(bus_id, i2c_addr, I2C_THERMAL_REG_AC_PSU_CHICONY_TEMPERATURE, 2, data);
            if(0 != rc)
            {
                return ONLP_STATUS_E_INTERNAL;
            }

            /* update the temperature
             */
            psu_temp = (data[1] << 8) + data[0];
            rv->mcelsius = parse_literal_format(psu_temp, TEMPERATURE_MULTIPLIER);

            rc = I2C_nRead(bus_id, i2c_addr, I2C_THERMAL_REG_AC_PSU_CHICONY_STATUS, 2, data);
            if(0 != rc)
            {
                return ONLP_STATUS_E_INTERNAL;
            }

            /* update status
             */
            if ((data[0] & I2C_THERMAL_REG_AC_PSU_CHICONY_STATUS_BIT_MASK) == I2C_THERMAL_REG_AC_PSU_CHICONY_STATUS_BIT_MASK)
            {
                rv->status |= ONLP_THERMAL_STATUS_FAILED;
            }
            else
            {
                rv->status &= ~ONLP_THERMAL_STATUS_FAILED;
            }

            break;

        default:
            break;
    }

    return ONLP_STATUS_OK;
}

