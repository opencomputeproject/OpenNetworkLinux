/*
 * A hwmon driver for the accton_i2c_cpld
 *
 * Copyright (C) 2016 Accton Technology Corporation.
 * Brandon Chuang <brandon_chuang@accton.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

extern int accton_i2c_cpld_read(unsigned short cpld_addr, u8 reg);
extern int accton_i2c_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);

#define AS4610_CPLD_SLAVE_ADDR 0x30
#define AS4610_CPLD_PID_OFFSET 0x01	 /* Product ID offset */

enum as4610_product_id_e {
	PID_AS4610_30T,
	PID_AS4610_30P,
	PID_AS4610_54T,
	PID_AS4610_54P,
	PID_RESERVED,
	PID_AS4610_54T_B,
	PID_UNKNOWN
};

static inline int as4610_product_id(void)
{
	int pid = accton_i2c_cpld_read(AS4610_CPLD_SLAVE_ADDR, AS4610_CPLD_PID_OFFSET);
	pid &= 0xF;

	if (pid < PID_AS4610_30T || pid > PID_AS4610_54T_B || pid == PID_RESERVED) {
		return PID_UNKNOWN;
	}

	return pid;
}

static inline int as4610_is_poe_system(void)
{
	int pid = as4610_product_id();
	return (pid == PID_AS4610_30P || pid == PID_AS4610_54P);
}

static inline int as4610_number_of_system_fan(void)
{
	int nFan = 0;
	int pid = as4610_product_id();

	switch (pid) {
	case PID_AS4610_30P:
	case PID_AS4610_54P:
		nFan = 1;
		break;
	case PID_AS4610_54T_B:
		nFan = 2;
		break;
	default:
		nFan = 0;
		break;
	}

	return nFan;
}

