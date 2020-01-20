// SPDX-License-Identifier: GPL-2.0+
/*
 * i2c-cls.h
 *
 * Pradchaya Phucharoen <pphuchar@celestica.com>
 * Copyright (C) 2019 Celestica Corp.
 */

#ifndef _LINUX_I2C_CLS_H
#define _LINUX_I2C_CLS_H

/* i2c bus speed mode */
typedef enum{
	// Preserved zero
	CLK50KHZ  = 1,
	CLK100KHZ = 2,
	CLK200KHZ = 3,
	CLK400KHZ = 4,
}bus_clk;

/*
 * struct cls_i2c_platform_data - platform data for celestica i2c driver
 * @reg_shift:		number of bytes between register
 * @port_id		i2c core id
 * @bus_clk_mode:	i2c bus clock speed mode
 * @timeout_us:		adapter timeout in us
 * @adap_id:		bus number for adapter device, -1 for dynamic allocate.
 * @num_devices:	number of devices on the bus
 * @devices:		devices conncted to the bus
 */
struct cls_i2c_platform_data {
	unsigned int reg_shift;
	unsigned int port_id;
	bus_clk bus_clk_mode;
	unsigned int timeout_us;
	int adap_id;
	u8 num_devices; /* number of devices in the devices list */
	struct i2c_board_info const *devices; /* devices connected to the bus */
};

#endif /* _LINUX_I2C_CLS_H */
