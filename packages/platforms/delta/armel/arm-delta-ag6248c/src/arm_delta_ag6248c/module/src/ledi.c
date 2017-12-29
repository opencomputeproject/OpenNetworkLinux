/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc
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
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include <onlplib/mmap.h>
#include "platform_lib.h"
#include "arm_delta_ag6248c_int.h"
#include "arm_delta_i2c.h"
#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)
		
	
#define CPLD_LED_MODE_REG					(0X0A)
#define CPLD_LED_MODE_TEMP_REG				(0X0B)
#define CPLD_LED_MODE_REG_BIT(ch)			(0x3<<2*((ch)-1))
#define CPLD_LED_MODE_TEMP_REG_BIT			(0x0C)
#define CPLD_LED_MODE_MASTER_REG_BIT		(0x02)
#define CPLD_LED_MODE_REG_OFFSET(ch)		(2*((ch)-1))
#define CPLD_LED_MODE_TEMP_REG_OFFSET	    (2)
#define CPLD_LED_MODE_MASTER_REG_OFFSET	    (1)


/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_SYS), "sys", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_GREEN_BLINKING |ONLP_LED_CAPS_GREEN | 
		ONLP_LED_CAPS_RED_BLINKING | ONLP_LED_CAPS_RED ,
    },
	
	{
        { ONLP_LED_ID_CREATE(LED_FAN), "fan", 0 },
        ONLP_LED_STATUS_PRESENT,  ONLP_LED_CAPS_ON_OFF |
		ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
    },
	
    {
        { ONLP_LED_ID_CREATE(LED_PSU2), "psu2", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN_BLINKING | 
        ONLP_LED_CAPS_GREEN ,
    },
	
    {
        { ONLP_LED_ID_CREATE(LED_PSU1), "psu1", 0 },
        ONLP_LED_STATUS_PRESENT, 
		ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN_BLINKING | 
        ONLP_LED_CAPS_GREEN ,
    },
  
    {
        { ONLP_LED_ID_CREATE(LED_TEMP), "temp", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | 
        ONLP_LED_CAPS_RED ,
    },
	
	{
        { ONLP_LED_ID_CREATE(LED_MASTER), "master", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN ,
    },
};

static int conver_led_light_mode_to_onl(uint32_t id, int led_ligth_mode)
{
    switch (id) {
    case LED_SYS:
		switch (led_ligth_mode) {
        case SYS_LED_MODE_GREEN_BLINKING:      return ONLP_LED_MODE_GREEN_BLINKING;
        case SYS_LED_MODE_GREEN:               return ONLP_LED_MODE_GREEN;
        case SYS_LED_MODE_RED:      		   return ONLP_LED_MODE_RED;
        case SYS_LED_MODE_RED_BLINKING:        return ONLP_LED_MODE_RED_BLINKING;
        default:                           	   return ONLP_LED_MODE_OFF;
        }
    case LED_PSU1:
    case LED_PSU2:
		switch (led_ligth_mode) {
        case PSU_LED_MODE_OFF:                 return ONLP_LED_MODE_OFF;
        case PSU_LED_MODE_GREEN:               return ONLP_LED_MODE_GREEN;
        case PSU_LED_MODE_GREEN_BLINKING:      return ONLP_LED_MODE_GREEN_BLINKING;
        default:                           	   return ONLP_LED_MODE_OFF;
        }
    case LED_FAN:
	    switch (led_ligth_mode) {
		case FAN_LED_MODE_OFF:				   return ONLP_LED_MODE_OFF;
        case FAN_LED_MODE_GREEN:               return ONLP_LED_MODE_GREEN;
        case FAN_LED_MODE_RED:     			   return ONLP_LED_MODE_RED;
        default:                               return ONLP_LED_MODE_OFF;
        }
    case LED_TEMP:
        switch (led_ligth_mode) {
        case TEMP_LED_MODE_OFF:                return ONLP_LED_MODE_OFF;
        case TEMP_LED_MODE_GREEN:              return ONLP_LED_MODE_GREEN;
        case TEMP_LED_MODE_RED:                return ONLP_LED_MODE_RED;
        default:                               return ONLP_LED_MODE_OFF;
        }
	case LED_MASTER:
        switch (led_ligth_mode) {
        case MASTER_LED_MODE_OFF:              return ONLP_LED_MODE_OFF;
        case MASTER_LED_MODE_GREEN:            return ONLP_LED_MODE_GREEN;
        default:                               return ONLP_LED_MODE_OFF;
        }
    }

	return ONLP_LED_MODE_OFF;
}

static int conver_onlp_led_light_mode_to_driver(uint32_t id, int led_ligth_mode)
{
    switch (id) {
     case LED_SYS:
		switch (led_ligth_mode) {
        case ONLP_LED_MODE_GREEN_BLINKING:      return SYS_LED_MODE_GREEN_BLINKING;
        case ONLP_LED_MODE_GREEN:       		return SYS_LED_MODE_GREEN;
        case ONLP_LED_MODE_RED:      			return SYS_LED_MODE_RED  ;
        case ONLP_LED_MODE_RED_BLINKING:        return SYS_LED_MODE_RED_BLINKING;
        default:                           		return SYS_LED_MODE_UNKNOWN;
        }
    case LED_PSU1:
    case LED_PSU2:
		switch (led_ligth_mode) {
        case ONLP_LED_MODE_OFF:                 return PSU_LED_MODE_OFF;
        case ONLP_LED_MODE_GREEN:               return PSU_LED_MODE_GREEN;
        case ONLP_LED_MODE_GREEN_BLINKING:      return PSU_LED_MODE_GREEN_BLINKING;
        default:                                return PSU_LED_MODE_UNKNOWN;
        }
    case LED_FAN:
	    switch (led_ligth_mode) {
		case ONLP_LED_MODE_OFF:   			    return FAN_LED_MODE_OFF;	
        case ONLP_LED_MODE_GREEN:               return FAN_LED_MODE_GREEN ;
        case ONLP_LED_MODE_RED:                 return FAN_LED_MODE_RED;
        default:                                return FAN_LED_MODE_UNKNOWN;
        }
    case LED_TEMP:
        switch (led_ligth_mode) {
        case ONLP_LED_MODE_OFF:                 return TEMP_LED_MODE_OFF;
        case ONLP_LED_MODE_GREEN:               return TEMP_LED_MODE_GREEN;
        case ONLP_LED_MODE_RED:                 return TEMP_LED_MODE_RED;
        default:                                return TEMP_LED_MODE_UNKNOWN;
        }
	case LED_MASTER:
        switch (led_ligth_mode) {
        case ONLP_LED_MODE_OFF:                 return MASTER_LED_MODE_OFF;
        case ONLP_LED_MODE_GREEN:               return MASTER_LED_MODE_GREEN;
        default:                                return TEMP_LED_MODE_UNKNOWN;
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

static int
onlp_ledi_oid_to_internal_id(onlp_oid_t id)
{
	enum ag6248c_product_id pid = get_product_id();
	int lid = ONLP_OID_ID_GET(id);

	if ((pid != PID_AG6248C_48P)||(pid != PID_AG6248C_48)) {
		return lid;
	}
	
	switch (lid) {
	case 1:	return LED_SYS;
	case 2: return LED_FAN;
	case 3: return LED_PSU2;
	case 4: return LED_PSU1;
	case 5: return LED_TEMP;
	case 6: return LED_MASTER;
	}

	return lid;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{	
    int  r_data,m_data;
	
    int lid = onlp_ledi_oid_to_internal_id(id);
	
    VALIDATE(id);

    /* Set the onlp_oid_hdr_t and capabilities */		
    *info = linfo[lid];
	
	if((lid==LED_TEMP)||(lid==LED_MASTER))
		r_data=i2c_devname_read_byte("CPLD",CPLD_LED_MODE_TEMP_REG);
	else
		r_data=i2c_devname_read_byte("CPLD",CPLD_LED_MODE_REG);
	
	if(r_data<0)
		return ONLP_STATUS_E_INTERNAL;
	
	if(lid==LED_TEMP)
		m_data=(r_data & CPLD_LED_MODE_TEMP_REG_BIT);
	else if(lid==LED_MASTER)
		m_data=(r_data & CPLD_LED_MODE_MASTER_REG_BIT);
	else 
		m_data=(r_data & CPLD_LED_MODE_REG_BIT(lid));
	
	if(lid==LED_TEMP)
		m_data=(m_data>> CPLD_LED_MODE_TEMP_REG_OFFSET);
	else if(lid==LED_MASTER)
		m_data=(m_data>> CPLD_LED_MODE_MASTER_REG_OFFSET);
	else
		m_data=(m_data>>CPLD_LED_MODE_REG_OFFSET(lid));
	
    info->mode = conver_led_light_mode_to_onl(lid, m_data);

        /* Set the on/off status */
    if (info->mode != ONLP_LED_MODE_OFF) {
        info->status |= ONLP_LED_STATUS_ON;
        
    }

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
    int  r_data,driver_mode, rc;
	
    int lid = onlp_ledi_oid_to_internal_id(id);
    
    VALIDATE(id);

    driver_mode = conver_onlp_led_light_mode_to_driver(lid, mode);
	
	if((driver_mode==SYS_LED_MODE_UNKNOWN)||(driver_mode==PSU_LED_MODE_UNKNOWN)||\
		(driver_mode==FAN_LED_MODE_UNKNOWN)||(driver_mode==TEMP_LED_MODE_UNKNOWN)||\
		(driver_mode==MASTER_LED_MODE_UNKNOWN))
		return ONLP_STATUS_E_UNSUPPORTED;
	
	if((lid==LED_TEMP)||(lid==LED_MASTER))
		r_data=i2c_devname_read_byte("CPLD",CPLD_LED_MODE_TEMP_REG);
	else
		r_data=i2c_devname_read_byte("CPLD",CPLD_LED_MODE_REG);
	
	if(r_data<0)
		return ONLP_STATUS_E_INTERNAL;
	
	if(lid==LED_TEMP)
		r_data=r_data&(~CPLD_LED_MODE_TEMP_REG_BIT);
	else if(lid==LED_MASTER)
		r_data=r_data&(~CPLD_LED_MODE_MASTER_REG_BIT);
	else
		r_data=r_data&(~CPLD_LED_MODE_REG_BIT(lid));
	
	if(lid==LED_TEMP)
		driver_mode=(driver_mode<<CPLD_LED_MODE_TEMP_REG_OFFSET);
	else if(lid==LED_MASTER)
		driver_mode=(driver_mode<<CPLD_LED_MODE_MASTER_REG_OFFSET);
	else
		driver_mode=(driver_mode<<CPLD_LED_MODE_REG_OFFSET(lid));

	driver_mode=driver_mode| r_data;

	if((lid==LED_TEMP)||(lid==LED_MASTER))
		rc=i2c_devname_write_byte("CPLD", CPLD_LED_MODE_TEMP_REG, driver_mode);
	else
		rc=i2c_devname_write_byte("CPLD", CPLD_LED_MODE_REG, driver_mode);
	
    if(rc<0){
        return ONLP_STATUS_E_INTERNAL;	
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
		if (!on_or_off) {
			return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
		}

    return ONLP_STATUS_E_UNSUPPORTED;
}


/*
 * Generic LED ioctl interface.
 */
int
onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

