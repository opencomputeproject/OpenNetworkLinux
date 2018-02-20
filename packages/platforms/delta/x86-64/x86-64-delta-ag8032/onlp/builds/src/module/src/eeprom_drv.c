
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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "24cXX.h"
#include "eeprom_drv.h"

int eeprom_read (int bus, uint8_t addr, int offset, uint8_t *buff, int len)
{
	struct eeprom e;
	char bus_str[128];
	int i;
	int r;

	snprintf (bus_str, sizeof(bus_str), "/dev/i2c-%d", bus);

	/////////////////////////////////////////////////////////////////////
	// Try 16 bit offset first
	do {
		if (eeprom_open (bus_str, addr, EEPROM_TYPE_16BIT_ADDR, &e)) {
			return -1;
		}

		for (i = 0 ; i < len ; i ++) {
			r = eeprom_read_byte(&e, (__u16)(offset + i));
			if (r < 0) {
				goto start_for_8bit;
			}
			buff[i] = (uint8_t)r;
		}

		eeprom_close (&e);
		return 0;
	} while (0);

start_for_8bit:

	eeprom_close (&e);

	/////////////////////////////////////////////////////////////////////
	// Try 8 bit offset length
	do {
		if (eeprom_open (bus_str, addr, EEPROM_TYPE_8BIT_ADDR, &e)) {
			return -1;
		}

		for (i = 0 ; i < len ; i ++) {
			r = eeprom_read_byte(&e, (__u16)(offset + i));
			if (r < 0) {
				goto error_out;
			}
			buff[i] = (uint8_t)r;
		}

		eeprom_close (&e);
		return 0;
	} while (0);

error_out:

	eeprom_close (&e);

	return -1;
}

