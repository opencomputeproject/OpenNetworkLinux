/* SPDX-License-Identifier: GPL-2.0+
 *
 * fpga-xcvr.h
 *
 * Pradchaya Phucharoen <pphuchar@celestica.com>
 * Nicholas Wu <nicwu@celestica.com>
 *
 * Copyright (C) 2022-2024 Celestica Corp.
 */

#ifndef _FPGA_XCVR_H_
#define _FPGA_XCVR_H_

enum PORT_TYPE {
		NONE = 0,
		SFP,
		QSFP
};

/*
 * port_info - optical port info
 * @index: front panel port index starting from 1
 * @typr: port type, see *PORT_TYPE*
 */
struct port_info {
		const char *name;
		unsigned int index;
		enum PORT_TYPE type;
};

/*
 * cls_xcvr_platform_data - port xcvr private data
 * @port_reg_size: register range of each port
 * @num_ports: number of front panel ports
 * @devices: list of front panel port info
 */
struct cls_xcvr_platform_data {
		unsigned int port_reg_size;
		int num_ports;
		struct port_info *devices;
};

#endif /* _LINUX_I2C_CLS_H */
