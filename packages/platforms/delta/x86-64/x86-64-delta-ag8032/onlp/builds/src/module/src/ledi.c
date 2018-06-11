/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
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
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "platform_lib.h"

static int _fan_tray_present (void *e);

static cpld_reg_t _fan_tray_present_reg[] = {
	[PLAT_LED_ID_5] = CPLD_REG (CPLD_CPUPLD, 0x11, 0, 1),
	[PLAT_LED_ID_6] = CPLD_REG (CPLD_CPUPLD, 0x11, 1, 1),
	[PLAT_LED_ID_7] = CPLD_REG (CPLD_CPUPLD, 0x11, 2, 1),
};

static plat_led_t plat_leds[] = {
	/////////////////////////////////////////////////////////////
	[PLAT_LED_ID_1] = {
		.name = "sys",
		.hw = CPLD_REG (CPLD_CPUPLD, 0x0e, 2, 2),
		.mode = {
			PLAT_LED_MODE(ONLP_LED_MODE_RED,	0),
			PLAT_LED_MODE(ONLP_LED_MODE_GREEN,	1),
			PLAT_LED_MODE(ONLP_LED_MODE_GREEN_BLINKING,	2),
			PLAT_LED_MODE(ONLP_LED_MODE_RED_BLINKING,	3),
			PLAT_LED_MODE_END,
		},
		PLAT_LED_INTERNAL_DEF,
	},
	/////////////////////////////////////////////////////////////
	[PLAT_LED_ID_2] = {
		.name = "fans",
		.hw = CPLD_REG (CPLD_CPUPLD, 0x0c, 0, 2),
		.mode = {
			PLAT_LED_MODE(ONLP_LED_MODE_OFF,	0),
			PLAT_LED_MODE(ONLP_LED_MODE_GREEN,	1),
			PLAT_LED_MODE(ONLP_LED_MODE_ORANGE,	3),
			PLAT_LED_MODE_END,
		},
		PLAT_LED_INTERNAL_DEF,
	},
	/////////////////////////////////////////////////////////////
	[PLAT_LED_ID_4] = {
		.name = "pwr2",
		.hw = CPLD_REG (CPLD_CPUPLD, 0x0c, 4, 2),
		.mode = {
			PLAT_LED_MODE(ONLP_LED_MODE_OFF,	0),
			PLAT_LED_MODE(ONLP_LED_MODE_GREEN,	1),
			PLAT_LED_MODE(ONLP_LED_MODE_ORANGE,	2),
			PLAT_LED_MODE_END,
		},
		PLAT_LED_INTERNAL_DEF,
	},
	/////////////////////////////////////////////////////////////
	[PLAT_LED_ID_3] = {
		.name = "pwr1",
		.hw = CPLD_REG (CPLD_CPUPLD, 0x0c, 6, 2),
		.mode = {
			PLAT_LED_MODE(ONLP_LED_MODE_OFF,	0),
			PLAT_LED_MODE(ONLP_LED_MODE_GREEN,	1),
			PLAT_LED_MODE(ONLP_LED_MODE_ORANGE,	2),
			PLAT_LED_MODE_END,
		},
		PLAT_LED_INTERNAL_DEF,
	},
	/////////////////////////////////////////////////////////////
	[PLAT_LED_ID_5] = {
		.name = "fan1",
		.hw = CPLD_REG (CPLD_CPUPLD, 0x0d, 0, 2),
		.present = _fan_tray_present,
		.present_data = &_fan_tray_present_reg[PLAT_LED_ID_5],
		.mode = {
			PLAT_LED_MODE(ONLP_LED_MODE_OFF,	0),
			PLAT_LED_MODE(ONLP_LED_MODE_GREEN,	1),
			PLAT_LED_MODE(ONLP_LED_MODE_ORANGE,	2),
			PLAT_LED_MODE_END,
		},
		PLAT_LED_INTERNAL_DEF,
	},
	/////////////////////////////////////////////////////////////
	[PLAT_LED_ID_6] = {
		.name = "fan2",
		.hw = CPLD_REG (CPLD_CPUPLD, 0x0d, 2, 2),
		.present = _fan_tray_present,
		.present_data = &_fan_tray_present_reg[PLAT_LED_ID_6],
		.mode = {
			PLAT_LED_MODE(ONLP_LED_MODE_OFF,	0),
			PLAT_LED_MODE(ONLP_LED_MODE_GREEN,	1),
			PLAT_LED_MODE(ONLP_LED_MODE_ORANGE,	2),
			PLAT_LED_MODE_END,
		},
		PLAT_LED_INTERNAL_DEF,
	},
	/////////////////////////////////////////////////////////////
	[PLAT_LED_ID_7] = {
		.name = "fan3",
		.hw = CPLD_REG (CPLD_CPUPLD, 0x0d, 4, 2),
		.present = _fan_tray_present,
		.present_data = &_fan_tray_present_reg[PLAT_LED_ID_7],
		.mode = {
			PLAT_LED_MODE(ONLP_LED_MODE_OFF,	0),
			PLAT_LED_MODE(ONLP_LED_MODE_GREEN,	1),
			PLAT_LED_MODE(ONLP_LED_MODE_ORANGE,	2),
			PLAT_LED_MODE_END,
		},
		PLAT_LED_INTERNAL_DEF,
	},
};
#define plat_leds_size	(sizeof(plat_leds)/sizeof(plat_leds[0]))

static int plat_led_is_valid (int id)
{
	if (id > 0 && id < plat_leds_size) {
		if (plat_leds[id].name)
			return 1;
	}
	return 0;
}

static int __hw_to_onlp_val (int id, int hv)
{
	plat_led_t *led = &plat_leds[id];
	led_mode_t *mod = &led->mode[0];

	while (mod->hw_val >= 0) {
		if (mod->hw_val == hv)
			return mod->onlp_val;
		mod ++;
	}
	return -1;
}

static int __onlp_to_hw_val (int id, int ov)
{
	plat_led_t *led = &plat_leds[id];
	led_mode_t *mod = &led->mode[0];

	while (mod->onlp_val >= 0) {
		if (mod->onlp_val == ov)
			return mod->hw_val;
		mod ++;
	}
	return -1;
}

static uint32_t _onlp_cap_create (led_mode_t *mod)
{
	uint32_t cap = 0;

	while (mod->onlp_val >= 0) {
		switch (mod->onlp_val) {
		case ONLP_LED_MODE_OFF:				cap |= ONLP_LED_CAPS_ON_OFF; break;
		case ONLP_LED_MODE_ON:				cap |= ONLP_LED_CAPS_ON_OFF; break;
		case ONLP_LED_MODE_RED:				cap |= ONLP_LED_CAPS_RED; break;
		case ONLP_LED_MODE_RED_BLINKING:	cap |= ONLP_LED_CAPS_RED_BLINKING; break;
		case ONLP_LED_MODE_ORANGE:			cap |= ONLP_LED_CAPS_ORANGE; break;
		case ONLP_LED_MODE_ORANGE_BLINKING:	cap |= ONLP_LED_CAPS_ORANGE_BLINKING; break;
		case ONLP_LED_MODE_YELLOW:			cap |= ONLP_LED_CAPS_YELLOW; break;
		case ONLP_LED_MODE_YELLOW_BLINKING: cap |= ONLP_LED_CAPS_YELLOW_BLINKING; break;
		case ONLP_LED_MODE_GREEN:			cap |= ONLP_LED_CAPS_GREEN; break;
		case ONLP_LED_MODE_GREEN_BLINKING:	cap |= ONLP_LED_CAPS_GREEN_BLINKING; break;
		case ONLP_LED_MODE_BLUE:			cap |= ONLP_LED_CAPS_BLUE; break;
		case ONLP_LED_MODE_BLUE_BLINKING:	cap |= ONLP_LED_CAPS_BLUE_BLINKING; break;
		case ONLP_LED_MODE_PURPLE:			cap |= ONLP_LED_CAPS_PURPLE;	break;
		case ONLP_LED_MODE_PURPLE_BLINKING:	cap |= ONLP_LED_CAPS_PURPLE_BLINKING; break;
		case ONLP_LED_MODE_AUTO:			cap |= ONLP_LED_CAPS_AUTO; break;
		case ONLP_LED_MODE_AUTO_BLINKING:	cap |= ONLP_LED_CAPS_AUTO_BLINKING; break;
		}
		mod ++;
	}
	return cap;
}

static int _fan_tray_present (void *e)
{
	plat_led_t *led = e;
	return cpld_reg_get (led->present_data) == 0 ? 1 : 0;
}

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{	
    int lid;
	plat_led_t *led = &plat_leds[id];
	led_mode_t *mod = &led->mode[0];
	int present = 1;


	if (!ONLP_OID_IS_LED(id))
		return ONLP_STATUS_E_INVALID;

	lid = ONLP_OID_ID_GET(id);

	if (!plat_led_is_valid (lid))
		return ONLP_STATUS_E_INVALID;

    /* Set the onlp_oid_hdr_t and capabilities */
	led = &plat_leds[lid];
	mod = &led->mode[0];

	memset (info, 0, sizeof(*info));
	info->hdr.id = id;
	if (led->name)
		snprintf (info->hdr.description, sizeof(info->hdr.description), "%s", led->name);
	
	info->caps = _onlp_cap_create (mod);

	if (led->present) {
		present = led->present(led) ? 1 : 0;
	}

	if (present) {
		int mode;

		if (led->hw_val_run < 0)
			led->hw_val_run = cpld_reg_get (&led->hw);

		mode = __hw_to_onlp_val (lid, led->hw_val_run);

		info->status |= ONLP_LED_STATUS_PRESENT;

		if (mode < 0) {
			info->mode = ONLP_LED_MODE_OFF;
			info->status |= ONLP_LED_STATUS_FAILED;
		} else {
			info->mode = mode;
			info->status |= ONLP_LED_STATUS_ON;
		}

		switch (info->mode) {
		case ONLP_LED_MODE_OFF:
			info->status &= ~ONLP_LED_STATUS_ON;
			break;
		default:
			break;
		}
	} else {
		info->mode = ONLP_LED_MODE_OFF;
		info->status &= ~ONLP_LED_STATUS_ON;
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
	int lid;
	plat_led_t *led = &plat_leds[id];
	int hw_val;

	if (!ONLP_OID_IS_LED(id))
		return ONLP_STATUS_E_INVALID;

	lid = ONLP_OID_ID_GET(id);

	if (!plat_led_is_valid (lid))
		return ONLP_STATUS_E_INVALID;

	led = &plat_leds[lid];


	hw_val = __onlp_to_hw_val (lid, mode);
	if (hw_val < 0)
		return ONLP_STATUS_E_UNSUPPORTED;
	
	if (led->hw_val_run == hw_val)
		return ONLP_STATUS_OK;

    if (cpld_reg_set (&led->hw, (uint8_t)hw_val)){
        return ONLP_STATUS_E_INTERNAL;	
    }

	led->hw_val_run = hw_val;
 
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


