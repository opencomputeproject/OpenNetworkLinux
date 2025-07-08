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
 *
 *
 ***********************************************************/
#include <onlp/platformi/sfpi.h>
#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include "x86_64_accton_as4625_54t_int.h"
#include "x86_64_accton_as4625_54t_log.h"

#define PORT_EEPROM_FORMAT      "/sys/bus/i2c/devices/%d-0050/eeprom"
#define MODULE_PRESENT_FORMAT   "/sys/bus/i2c/devices/0-0064/module_present_%d"
#define MODULE_RXLOS_FORMAT     "/sys/bus/i2c/devices/0-0064/module_rx_los_%d"
#define MODULE_TXFAULT_FORMAT   "/sys/bus/i2c/devices/0-0064/module_tx_fault_%d"
#define MODULE_TXDISABLE_FORMAT "/sys/bus/i2c/devices/0-0064/module_tx_disable_%d"
#define MODULE_RESET_FORMAT     "/sys/bus/i2c/devices/0-0064/module_reset_%d"
#define MODULE_LPMODE_FORMAT    "/sys/bus/i2c/devices/0-0064/module_lpmode_%d"
#define MODULE_PRESENT_ALL_ATTR "/sys/bus/i2c/devices/0-0064/module_present_all"
#define MODULE_RXLOS_ALL_ATTR   "/sys/bus/i2c/devices/0-0064/module_rx_los_all"
/* QSFP device address of eeprom */
#define PORT_EEPROM_DEVADDR             0x50
/* QSFP tx disable offset */
#define QSFP_EEPROM_OFFSET_TXDIS        0x56

int port_bus_index[] = { 10, 11, 12, 13, 14, 15 };
#define PORT_BUS_INDEX(port) (port_bus_index[port-48])

#define VALIDATE_SFP(_port) \
	do { \
		if (_port < 48 || _port > 53) \
			return ONLP_STATUS_E_UNSUPPORTED; \
	} while(0)


/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
int
onlp_sfpi_init(void)
{
	/* Called at initialization time */
	return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
	/*
	 * Ports {48, 53}
	 */
	int p;

	for (p = 48; p < 54; p++) {
		AIM_BITMAP_SET(bmap, p);
	}

	return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
	/*
	 * Return 1 if present.
	 * Return 0 if not present.
	 * Return < 0 if error.
	 */
	int present;
	VALIDATE_SFP(port);

	if (onlp_file_read_int(&present, MODULE_PRESENT_FORMAT, port+1) < 0) {
		AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", 
			port);
		return ONLP_STATUS_E_INTERNAL;
	}

	return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
	uint32_t byte;
	FILE* fp;

	/* Read present status of port 48 ~ 53 */
	int count  = 0;

	fp = fopen(MODULE_PRESENT_ALL_ATTR, "r");
	if(fp == NULL) {
		AIM_LOG_ERROR("Unable to open the module_present_all device file");
		return ONLP_STATUS_E_INTERNAL;
	}

	count = fscanf(fp, "%x", &byte);
	fclose(fp);

	if(count != 1) {
		/* Likely a CPLD read timeout. */
		AIM_LOG_ERROR("Unable to read the module_present_all device file");
		return ONLP_STATUS_E_INTERNAL;
	}

	/* Mask out non-existant SFP ports */
	byte &= 0x3F;

	/* Convert to 64 bit integer in port order */
	int i = 0;
	uint64_t presence_all = byte;

	/* Populate bitmap */
	presence_all <<= 48;

	for(i = 0; presence_all; i++) {
		AIM_BITMAP_MOD(dst, i, (presence_all & 1));
		presence_all >>= 1;
	}

	return ONLP_STATUS_OK;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
	uint32_t byte;
	FILE* fp;

	uint32_t byte1;
	FILE* fp1;

	/* Read rxlos status of port 48 ~ 53 */
	int count = 0;

	/* Read present status of port 48 ~ 53 */
	int count1 = 0;

	fp = fopen(MODULE_RXLOS_ALL_ATTR, "r");
	if(fp == NULL) {
		AIM_LOG_ERROR("Unable to open the module_rx_los_all device file");
		return ONLP_STATUS_E_INTERNAL;
	}

	count = fscanf(fp, "%x", &byte);
	fclose(fp);

	if(count != 1) {
		/* Likely a CPLD read timeout. */
		AIM_LOG_ERROR("Unable to read the module_rx_los_all device file");
		return ONLP_STATUS_E_INTERNAL;
	}

	fp1 = fopen(MODULE_PRESENT_ALL_ATTR, "r");
	if(fp1 == NULL) {
		AIM_LOG_ERROR("Unable to open the module_present_all device file");
		return ONLP_STATUS_E_INTERNAL;
	}

	count1 = fscanf(fp1, "%x", &byte1);
	fclose(fp1);

	if(count1 != 1) {
		/* Likely a CPLD read timeout. */
		AIM_LOG_ERROR("Unable to read the module_present_all device file");
		return ONLP_STATUS_E_INTERNAL;
	}

	/* Mask out non-existant SFP ports */
	byte &= 0x3F;
	byte &= byte1;

	/* Convert to 64 bit integer in port order */
	AIM_BITMAP_CLR_ALL(dst);
	int i = 0;
	uint64_t rx_los_all = byte;

	/* Populate bitmap */
	rx_los_all <<= 48;
	for(i = 0; rx_los_all; i++) {
		AIM_BITMAP_MOD(dst, i, (rx_los_all & 1));
		rx_los_all >>= 1;
	}

	return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
	/*
	 * Read the SFP eeprom into data[]
	 *
	 * Return MISSING if SFP is missing.
	 * Return OK if eeprom is read
	 */
	int size = 0;
	VALIDATE_SFP(port);

	memset(data, 0, 256);

	if (onlp_file_read(data, 256, &size, PORT_EEPROM_FORMAT,
			PORT_BUS_INDEX(port)) != ONLP_STATUS_OK) {
		AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
		return ONLP_STATUS_E_INTERNAL;
	}

	if (size != 256) {
		AIM_LOG_ERROR("Unable to read eeprom from port(%d), size is different!\r\n"
			, port);
		return ONLP_STATUS_E_INTERNAL;
	}

	return ONLP_STATUS_OK;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
	FILE* fp;
	char file[64] = {0};

	if(port < 48)
		return ONLP_STATUS_E_UNSUPPORTED;
	sprintf(file, PORT_EEPROM_FORMAT, PORT_BUS_INDEX(port));
	fp = fopen(file, "r");
	if(fp == NULL) {
		AIM_LOG_ERROR("Unable to open the eeprom device file of port(%d)"
			, port);
		return ONLP_STATUS_E_INTERNAL;
	}

	if (fseek(fp, 256, SEEK_CUR) != 0) {
		fclose(fp);
		AIM_LOG_ERROR("Unable to set the file position indicator of port(%d)"
			, port);
		return ONLP_STATUS_E_INTERNAL;
	}

	int ret = fread(data, 1, 256, fp);
	fclose(fp);
	if (ret != 256) {
		AIM_LOG_ERROR("Unable to read the module_eeprom device file of port(%d)"
			, port);
		return ONLP_STATUS_E_INTERNAL;
	}

	return ONLP_STATUS_OK;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
	switch(control) {
	case ONLP_SFP_CONTROL_TX_DISABLE:{
		if (port >= 48 && port <= 53) {
			if (onlp_file_write_int(value, MODULE_TXDISABLE_FORMAT
				, (port+1)) < 0) {
				AIM_LOG_ERROR("Unable to set tx_disable status to port(%d)\r\n"
					, port);
				return ONLP_STATUS_E_INTERNAL;
			}

			return ONLP_STATUS_OK;
		} else {
			return ONLP_STATUS_E_INTERNAL;
		}

		return ONLP_STATUS_E_INTERNAL;
	}

	default:
		break;
	}

	return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{

	switch(control) {
	case ONLP_SFP_CONTROL_RX_LOS: {
		VALIDATE_SFP(port);
		if (onlp_file_read_int(value, MODULE_RXLOS_FORMAT, (port+1)) 
			< 0) {
			AIM_LOG_ERROR("Unable to read rx_loss status from port(%d)\r\n"
				, port);
			return ONLP_STATUS_E_INTERNAL;
		}

		return ONLP_STATUS_OK;
	}

	case ONLP_SFP_CONTROL_TX_FAULT: {
		VALIDATE_SFP(port);
		if (onlp_file_read_int(value, MODULE_TXFAULT_FORMAT, (port+1))
			 < 0) {
			AIM_LOG_ERROR("Unable to read tx_fault status from port(%d)\r\n"
				, port);
			return ONLP_STATUS_E_INTERNAL;
		}

		return ONLP_STATUS_OK;
	}

	case ONLP_SFP_CONTROL_TX_DISABLE: {
		if (port >= 48 && port <= 53)
		{
			if (onlp_file_read_int(value, MODULE_TXDISABLE_FORMAT, 
				(port+1)) < 0) {
				AIM_LOG_ERROR("Unable to read tx_disabled status from port(%d)\r\n"
					, port);
				return ONLP_STATUS_E_INTERNAL;
			}

			return ONLP_STATUS_OK;
		} else {
			return ONLP_STATUS_E_INTERNAL;
		}


		return ONLP_STATUS_E_INTERNAL;
	}
	default:
		break;
	}

	return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_denit(void)
{
	return ONLP_STATUS_OK;
}
