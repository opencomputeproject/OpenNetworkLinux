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
#include "x86_64_delta_ag7648_int.h"
#include "x86_64_delta_i2c.h"
#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)
		

#define CPLD_NAME1  "SYSCPLD"
#define CPLD_NAME2  "MASTERCPLD"
#define CPLD_NAME3  "SLAVECPLD"

#define CPLD_LED_REG_BITS                   (0X3) //the reg bits

#define CPLD_LED_FAN_TRAY_REG               (0X8)
#define CPLD_LED_FAN_TRAY0_REG_OFFSET       (0X0)
#define CPLD_LED_FAN_TRAY1_REG_OFFSET       (0X2)
#define CPLD_LED_FAN_TRAY2_REG_OFFSET       (0X4)

#define CPLD_LED_POWER_REG                  (0X6)
#define CPLD_LED_POWER_REG_OFFSET           (0X6)

#define CPLD_LED_SYS_REG                    (0X7)
#define CPLD_LED_SYS_REG_OFFSET             (0X5)
#define CPLD_LED_LOCATOR_REG_OFFSET         (0X3)

#define CPLD_LED_FAN_REG                    (0X9)
#define CPLD_LED_FAN_REG_OFFSET             (0X3)


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
		ONLP_LED_CAPS_YELLOW_BLINKING | ONLP_LED_CAPS_YELLOW ,
    },
	
	{
        { ONLP_LED_ID_CREATE(LED_FAN), "fan", 0 },
        ONLP_LED_STATUS_PRESENT, 
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW_BLINKING | 
		ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_YELLOW,
    },
  
    {
        { ONLP_LED_ID_CREATE(LED_LOCATOR), "locator", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | 
        ONLP_LED_CAPS_GREEN_BLINKING ,
    },
	
	{
        { ONLP_LED_ID_CREATE(LED_POWER), "power", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | 
        ONLP_LED_CAPS_YELLOW_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_TRAY0), "fan_tray0", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | 
        ONLP_LED_CAPS_YELLOW,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_TRAY1), "fan_tray1", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | 
        ONLP_LED_CAPS_YELLOW,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN_TRAY2), "fan_tray2", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | 
        ONLP_LED_CAPS_YELLOW,
    },
};

static int conver_led_light_mode_to_onl(uint32_t id, int led_ligth_mode)
{
    switch (id) {
    case LED_SYS:
		switch (led_ligth_mode) {
        case SYS_LED_MODE_GREEN_BLINKING:      return ONLP_LED_MODE_GREEN_BLINKING;
        case SYS_LED_MODE_GREEN:               return ONLP_LED_MODE_GREEN;
        case SYS_LED_MODE_YELLOW:      		   return ONLP_LED_MODE_YELLOW;
        case SYS_LED_MODE_YELLOW_BLINKING:     return ONLP_LED_MODE_YELLOW_BLINKING;
        default:                           	   return ONLP_LED_MODE_GREEN_BLINKING;
        }

    case LED_FAN:
	    switch (led_ligth_mode) {
        case FAN_LED_MODE_OFF:                 return ONLP_LED_MODE_OFF;
		case FAN_LED_MODE_GREEN:               return ONLP_LED_MODE_GREEN;
        case FAN_LED_MODE_YELLOW:     		   return ONLP_LED_MODE_YELLOW;
        case FAN_LED_MODE_YELLOW_BLINKING:     return ONLP_LED_MODE_YELLOW_BLINKING;
        default:                               return ONLP_LED_MODE_OFF;
        }
    case LED_LOCATOR:
        switch (led_ligth_mode) {
        case LOCATOR_LED_MODE_OFF:             return ONLP_LED_MODE_OFF;
        case LOCATOR_LED_MODE_GREEN:           return ONLP_LED_MODE_GREEN;
        case LOCATOR_LED_MODE_GREEN_BLINKING:  return ONLP_LED_MODE_GREEN_BLINKING;
        default:                               return ONLP_LED_MODE_OFF;
        }
	case LED_POWER:
        switch (led_ligth_mode) {
        case POWER_LED_MODE_OFF:               return ONLP_LED_MODE_OFF;
        case POWER_LED_MODE_GREEN:             return ONLP_LED_MODE_GREEN;
        case POWER_LED_MODE_YELLOW_BLINKING:   return ONLP_LED_MODE_YELLOW_BLINKING;
        default:                               return ONLP_LED_MODE_OFF;
        }
    case LED_FAN_TRAY0:
    case LED_FAN_TRAY1:
    case LED_FAN_TRAY2:
        switch (led_ligth_mode) {
        case FAN_TRAY_LED_MODE_OFF:             return ONLP_LED_MODE_OFF;
        case FAN_TRAY_LED_MODE_GREEN:           return ONLP_LED_MODE_GREEN;
        case FAN_TRAY_LED_MODE_YELLOW:          return ONLP_LED_MODE_YELLOW_BLINKING;
        default:                                return ONLP_LED_MODE_OFF;
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
        case ONLP_LED_MODE_YELLOW:      		return SYS_LED_MODE_YELLOW  ;
        case ONLP_LED_MODE_YELLOW_BLINKING:     return SYS_LED_MODE_YELLOW_BLINKING;
        default:                           		return SYS_LED_MODE_UNKNOWN;
        }

    case LED_FAN:
	    switch (led_ligth_mode) {
        case ONLP_LED_MODE_OFF:                 return FAN_LED_MODE_OFF;
		case ONLP_LED_MODE_GREEN:               return FAN_LED_MODE_GREEN ;
        case ONLP_LED_MODE_YELLOW:              return FAN_LED_MODE_YELLOW;
        case ONLP_LED_MODE_YELLOW_BLINKING:     return FAN_LED_MODE_YELLOW_BLINKING;
        default:                                return FAN_LED_MODE_UNKNOWN;
        }
    case LED_LOCATOR:
        switch (led_ligth_mode) {
        case ONLP_LED_MODE_OFF:                 return LOCATOR_LED_MODE_OFF;
        case ONLP_LED_MODE_GREEN:               return LOCATOR_LED_MODE_GREEN;
        case ONLP_LED_MODE_GREEN_BLINKING:      return LOCATOR_LED_MODE_GREEN_BLINKING;
        default:                                return LOCATOR_LED_MODE_UNKNOWN;
        }
	case LED_POWER:
        switch (led_ligth_mode) {
        case ONLP_LED_MODE_OFF:                 return POWER_LED_MODE_OFF;
        case ONLP_LED_MODE_GREEN:               return POWER_LED_MODE_GREEN;
        case ONLP_LED_MODE_YELLOW_BLINKING:     return POWER_LED_MODE_YELLOW_BLINKING;
        default:                                return POWER_LED_MODE_UNKNOWN;
        }
	case LED_FAN_TRAY0:
    case LED_FAN_TRAY1:
    case LED_FAN_TRAY2:
        switch (led_ligth_mode) {
        case ONLP_LED_MODE_OFF:                 return FAN_TRAY_LED_MODE_OFF;
        case ONLP_LED_MODE_GREEN:               return FAN_TRAY_LED_MODE_GREEN;
        case ONLP_LED_MODE_YELLOW_BLINKING:     return FAN_TRAY_LED_MODE_YELLOW;
        default:                                return FAN_TRAY_LED_MODE_UNKNOWN;
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
	enum ag7648_product_id pid = get_product_id();
	int lid = ONLP_OID_ID_GET(id);

	if (pid != PID_AG7648) {
		return lid;
	}
	
	switch (lid) {
	case 1:	return LED_SYS;
	case 2: return LED_FAN;
	case 3: return LED_LOCATOR;
	case 4: return LED_POWER;
    case 5: return LED_FAN_TRAY0;
    case 6: return LED_FAN_TRAY1;
    case 7: return LED_FAN_TRAY2;
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
	
    DEBUG_PRINT("id %u lid %d\n", id, lid);

    switch (lid)
    {
        case LED_POWER:
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_POWER_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            m_data = (r_data >> CPLD_LED_POWER_REG_OFFSET) & CPLD_LED_REG_BITS;
            break;
        case LED_SYS:
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_SYS_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            m_data = (r_data >> CPLD_LED_SYS_REG_OFFSET) & CPLD_LED_REG_BITS;
            break;
        case LED_LOCATOR:
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_SYS_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            m_data = (r_data >> CPLD_LED_LOCATOR_REG_OFFSET) & CPLD_LED_REG_BITS;
            break;

        case LED_FAN:
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_FAN_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            m_data = (r_data >> CPLD_LED_FAN_REG_OFFSET) & CPLD_LED_REG_BITS;
            break;
        case LED_FAN_TRAY0:
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_FAN_TRAY_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            m_data = (r_data >> CPLD_LED_FAN_TRAY0_REG_OFFSET) & CPLD_LED_REG_BITS;
            break;
        case LED_FAN_TRAY1:
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_FAN_TRAY_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            m_data = (r_data >> CPLD_LED_FAN_TRAY1_REG_OFFSET) & CPLD_LED_REG_BITS;
            break;
        case LED_FAN_TRAY2:
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_FAN_TRAY_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            m_data = (r_data >> CPLD_LED_FAN_TRAY2_REG_OFFSET) & CPLD_LED_REG_BITS;
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
    }
	
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
	int reg;
    
    int lid = onlp_ledi_oid_to_internal_id(id);
    
    VALIDATE(id);

    driver_mode = conver_onlp_led_light_mode_to_driver(lid, mode);
	
	if((driver_mode==SYS_LED_MODE_UNKNOWN)||(driver_mode==FAN_LED_MODE_UNKNOWN)||\
       (driver_mode==POWER_LED_MODE_UNKNOWN)||(driver_mode==LOCATOR_LED_MODE_UNKNOWN))
		return ONLP_STATUS_E_UNSUPPORTED;
	
    switch (lid)
    {
        case LED_POWER:
            reg = CPLD_LED_POWER_REG;
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_POWER_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            r_data &= ~(CPLD_LED_REG_BITS << CPLD_LED_POWER_REG_OFFSET);
            r_data |= (driver_mode & CPLD_LED_REG_BITS ) << CPLD_LED_POWER_REG_OFFSET;
            break;
        case LED_SYS:
            reg = CPLD_LED_SYS_REG;
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_SYS_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            r_data &= ~(CPLD_LED_REG_BITS << CPLD_LED_SYS_REG_OFFSET);
            r_data |= (driver_mode & CPLD_LED_REG_BITS ) << CPLD_LED_SYS_REG_OFFSET;
            break;
        case LED_LOCATOR:
            reg = CPLD_LED_SYS_REG;
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_SYS_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            r_data &= ~(CPLD_LED_REG_BITS << CPLD_LED_LOCATOR_REG_OFFSET);
            r_data |= (driver_mode & CPLD_LED_REG_BITS ) << CPLD_LED_LOCATOR_REG_OFFSET;
            break;

        case LED_FAN:
            reg = CPLD_LED_FAN_REG;
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_FAN_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            r_data &= ~(CPLD_LED_REG_BITS << CPLD_LED_FAN_REG_OFFSET);
            r_data |= (driver_mode & CPLD_LED_REG_BITS ) << CPLD_LED_FAN_REG_OFFSET;
            break;
        case LED_FAN_TRAY0:
            reg = CPLD_LED_FAN_TRAY_REG;
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_FAN_TRAY_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            r_data &= ~(CPLD_LED_REG_BITS << CPLD_LED_FAN_TRAY0_REG_OFFSET);
            r_data |= (driver_mode & CPLD_LED_REG_BITS ) << CPLD_LED_FAN_TRAY0_REG_OFFSET;
            break;
        case LED_FAN_TRAY1:
            reg = CPLD_LED_FAN_TRAY_REG;
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_FAN_TRAY_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            r_data &= ~(CPLD_LED_REG_BITS << CPLD_LED_FAN_TRAY1_REG_OFFSET);
            r_data |= (driver_mode & CPLD_LED_REG_BITS ) << CPLD_LED_FAN_TRAY1_REG_OFFSET;
            break;
        case LED_FAN_TRAY2:
            reg = CPLD_LED_FAN_TRAY_REG;
            r_data = i2c_devname_read_byte(CPLD_NAME2, CPLD_LED_FAN_TRAY_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            r_data &= ~(CPLD_LED_REG_BITS << CPLD_LED_FAN_TRAY2_REG_OFFSET);
            r_data |= (driver_mode & CPLD_LED_REG_BITS ) << CPLD_LED_FAN_TRAY2_REG_OFFSET;
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
    }
    
    rc=i2c_devname_write_byte(CPLD_NAME2, reg, r_data);
	
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

