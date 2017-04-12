/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 (C) Delta Networks, Inc.
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
#include <unistd.h>
#include <fcntl.h>

#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "x86_64_delta_ag9032v1_int.h"
#include "x86_64_delta_ag9032v1_log.h"
#include "platform_lib.h"

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-delta-ag9032v1-r0";
}

int
onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);
    if(onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK) {
        if(*size == 256) {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }
    aim_free(rdata);
    *size = 0;
    return ONLP_STATUS_E_UNSUPPORTED;
}

void
onlp_sysi_onie_data_free(uint8_t* data)
{
    aim_free(data);
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int cpld_version = 0;
    int swpld_version = 0;

    cpld_version = onlp_i2c_readb(I2C_BUS_2, CPUCPLD, CPUPLD_VERSION_ADDR, DEFAULT_FLAG); 
    swpld_version = dni_lock_swpld_read_attribute(SWPLD_VERSION_ADDR);
    pi->cpld_versions = aim_fstrdup("%d , SWPLD_Versions: %d", cpld_version, swpld_version);
    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}


int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 6 Thermal sensors on the chassis */
    for (i = 1; i <= NUM_OF_THERMAL_ON_BOARDS; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 9 LEDs on the chassis */
    for (i = 1; i <= NUM_OF_LED_ON_BOARDS; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 10 Fans on the chassis */
    for (i = 1; i <= NUM_OF_FAN_ON_FAN_BOARD; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= NUM_OF_PSU_ON_PSU_BOARD; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_manage_fans(void)
{
     int i = 0;
     int highest_temp = 0;
     onlp_thermal_info_t thermal[8];
     int new_duty_percentage;
    /* Get current temperature
     */
    if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), &thermal[0]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_CPU_BOARD), &thermal[1]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_FAN_BOARD), &thermal[2]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_SW_BOARD), &thermal[3]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_SW_BOARD), &thermal[4]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_SW_BOARD), &thermal[5]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), &thermal[6]) != ONLP_STATUS_OK ||
        onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), &thermal[7]) != ONLP_STATUS_OK )
    {
        AIM_LOG_ERROR("Unable to read thermal status");
        return ONLP_STATUS_E_INTERNAL;
    }
    for (i = 0; i < 8; i++)
    {
        if (thermal[i].mcelsius > highest_temp)
        {
            highest_temp = thermal[i].mcelsius;
        }
    }

    highest_temp = highest_temp/1000;

    if (highest_temp > 0 && highest_temp <= 30)
    {
        new_duty_percentage = SPEED_25_PERCENTAGE;
    } 
    else if (highest_temp > 30 && highest_temp <= 40)
    {
       new_duty_percentage = SPEED_50_PERCENTAGE;
    }
    else if (highest_temp > 40 && highest_temp <= 50)
    {
        new_duty_percentage = SPEED_75_PERCENTAGE;
    }
    else
    {
        new_duty_percentage = SPEED_100_PERCENTAGE;
    }
    /* Set speed on fan 1-10*/
    for(i = 1 ; i <= 10; i++)
    {
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), new_duty_percentage);
    }
    
    /*Set fans' speed of PSU 1, 2 
     */
    if(highest_temp >= 0 && highest_temp <= 55)
    {
	new_duty_percentage = SPEED_50_PERCENTAGE;
    }
    else if(highest_temp > 55)
    {
        new_duty_percentage = SPEED_100_PERCENTAGE;
    }
    else
    {
	new_duty_percentage = SPEED_100_PERCENTAGE;
	AIM_LOG_ERROR("Unable to get thermal temperature");
    }
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1_ON_PSU1) , new_duty_percentage);
    onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(FAN_1_ON_PSU2) , new_duty_percentage);
    
    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    /* Set front lights: fan, power supply 1, 2 
     */
    uint8_t addr, present_bit = 0x00, bit = 0x00;

    addr = dni_lock_swpld_read_attribute(LED_REG);
    /* Turn the fan led on or off */
    if((addr & 0x3) == 0 || (addr & 0x3) == 0x3 )
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN), TURN_OFF);
    else
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_FAN), TURN_ON);

    /* Set front light of PSU 1 */
    addr = dni_lock_swpld_read_attribute(LED_REG);
    
    if((addr & 0xC0) == 0 || (addr & 0xC0) == 0xC0 )
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR1), TURN_OFF);
    else
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR1), TURN_ON);

    /* Set front light of PSU 2 */
    addr = dni_lock_swpld_read_attribute(LED_REG);
    if((addr & 0x30) == 0 || (addr & 0x30) == 0x30 )
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR2), TURN_OFF);
    else
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_FRONT_PWR2), TURN_ON);
    
    /* Rear light fan tray 1-5 */
    dev_info_t dev_info;
    dev_info.bus = I2C_BUS_3;
    dev_info.addr = FAN_IO_CTL;
    dev_info.offset = 0x00;
    dev_info.flags = DEFAULT_FLAG;

    mux_info_t mux_info;
    mux_info.offset = SWPLD_PSU_FAN_I2C_MUX_REG;
    mux_info.flags = DEFAULT_FLAG;
    mux_info.channel = 0x07;
 
    /* Turn on or off the fan trays' leds */
    present_bit = dni_i2c_lock_read(&mux_info, &dev_info);
    if((present_bit & ((bit+1)<<4)) == 0)
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1), TURN_OFF);
    else
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1), TURN_ON);
    if((present_bit & ((bit+1)<<3)) == 0)
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2), TURN_OFF);
    else
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2), TURN_ON);
    if((present_bit & ((bit+1)<<2)) == 0)
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3), TURN_OFF);
    else
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3), TURN_ON);
    if((present_bit & ((bit+1)<<1)) == 0)
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4), TURN_OFF);
    else
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4), TURN_ON);

    if((present_bit & ((bit+1)<<0)) == 0)
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_5), TURN_OFF);
    else
        onlp_ledi_set(ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_5), TURN_ON);

    return ONLP_STATUS_OK;
}

