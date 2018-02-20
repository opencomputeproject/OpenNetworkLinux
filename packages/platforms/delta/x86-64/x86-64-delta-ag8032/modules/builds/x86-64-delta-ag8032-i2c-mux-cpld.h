
/*
 *
 * Copyright (C) 2017 Delta Networks, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __DNI_I2C_MUX_CPLD_H__
#define __DNI_I2C_MUX_CPLD_H__

struct i2c_mux_cpld_platform_data
{
	u8 cpld_bus;
	u8 cpld_addr;
	u8 cpld_reg;

	u8 parent_bus;

	u8 base_nr;

	const u8 *values;
	int n_values;
	bool idle_in_use;
	u8  idle;

	void *ctrl_adap;
};

#endif // __DNI_I2C_MUX_CPLD_H__

