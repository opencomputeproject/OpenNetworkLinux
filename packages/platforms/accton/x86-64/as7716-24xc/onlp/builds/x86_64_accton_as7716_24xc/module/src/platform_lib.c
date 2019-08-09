/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <sys/time.h>
#include <unistd.h>
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <AIM/aim.h>
#include "platform_lib.h"

#define PSU_MODEL_NAME_LEN 		13
#define PSU_SERIAL_NUMBER_LEN	14
#define PSU_NODE_MAX_PATH_LEN   64

int get_psu_serial_number(int id, char *serial, int serial_len)
{
	int   size = 0;
	int   ret  = ONLP_STATUS_OK; 
	char *path = (id == PSU1_ID) ? PSU1_AC_PMBUS_NODE(psu_mfr_serial) :
		                           PSU2_AC_PMBUS_NODE(psu_mfr_serial) ;

	if (serial == NULL || serial_len < PSU_SERIAL_NUMBER_LEN) {
		return ONLP_STATUS_E_PARAM;
	}

	ret = onlp_file_read((uint8_t*)serial, PSU_SERIAL_NUMBER_LEN, &size, path);
    if (ret != ONLP_STATUS_OK || size != PSU_SERIAL_NUMBER_LEN) {
		return ONLP_STATUS_E_INTERNAL;

    }

	serial[PSU_SERIAL_NUMBER_LEN] = '\0';
	return ONLP_STATUS_OK;
}

psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
	int   value = 0;
	int   ret   = ONLP_STATUS_OK; 
	char  model[PSU_MODEL_NAME_LEN + 1] = {0};
	char *path = NULL;

	if (modelname && modelname_len < PSU_MODEL_NAME_LEN) {
		return PSU_TYPE_UNKNOWN;
	}

	/* Check if the psu is present
	 */
	path = (id == PSU1_ID) ? PSU1_AC_EEPROM_NODE(psu_present) :
		                     PSU2_AC_EEPROM_NODE(psu_present) ;
    if (onlp_file_read_int(&value, path) < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

	if (!value) {
		return PSU_TYPE_UNKNOWN;
	}

	/* Read mode name */
	path = (id == PSU1_ID) ? PSU1_AC_PMBUS_NODE(psu_mfr_model) :
		                     PSU2_AC_PMBUS_NODE(psu_mfr_model) ;
	ret = onlp_file_read((uint8_t*)model, PSU_MODEL_NAME_LEN, &value, path);
    if (ret != ONLP_STATUS_OK || value != PSU_MODEL_NAME_LEN) {
		return PSU_TYPE_UNKNOWN;

    }

    if (modelname) {
		memcpy(modelname, model, sizeof(model));
    }

    if (strncmp(model, "DPS-850AB-4", strlen("DPS-850AB-4")) == 0) {
        return PSU_TYPE_AC_DPS850_F2B;
    }

    return PSU_TYPE_UNKNOWN;
}

int psu_dps850_pmbus_info_get(int id, char *node, int *value)
{
	char *prefix = NULL;

    *value = 0;

	prefix = (id == PSU1_ID) ? PSU1_AC_PMBUS_PREFIX : PSU2_AC_PMBUS_PREFIX;
    if (onlp_file_read_int(value, "%s%s", prefix, node) < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s%s)\r\n", prefix, node);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

#define DSP_ADDR				0x6a
#define DSP_READ_MAX_RETRY  	50000
#define DSP_WRITE_MAX_RETRY  	10 
#define DSP_BUSY_VALUE			1
#define DSP_NOBUSY_VALUE		0
#define NTT_I2C_CMD_WAIT_US  	100000

int dsp_exec_read(int bus, struct dsp_ctrl_s* dsp_ctrl, int *ret_val)
{
	int i = 0, ret = 0;
	uint8_t value = 0, offset = 0;

	if (ret_val) {
		*ret_val = 0;
	}

	/* Set the address to be read into register 0x10 ~ 0x13
	 */
	for (i = 0; i <= 3; i++) {
		offset = 0x10 + i;
		value  = (dsp_ctrl->value1 >> (8 * (3-i))) & 0xff;

		ret = onlp_i2c_writeb(bus, DSP_ADDR, offset, value, ONLP_I2C_F_FORCE);
		if (ret < 0) {
			DEBUG_PRINT("%s(%d): Failed to write data into bus(%d), addr(0x%x), offset(0x%x), value(0x%x), error code(%d)\r\n",
				   __FUNCTION__, __LINE__, bus, DSP_ADDR, offset, value, ret);
			return ret;
		}
	}

    // Trigger the read operation : write 0x2 to Reg0x1 of dsp controller
    offset = 0x01;
	value  = 0x02;
    ret    = onlp_i2c_writeb(bus, DSP_ADDR, offset, value, ONLP_I2C_F_FORCE);
	if (ret < 0) {
		DEBUG_PRINT("%s(%d): Failed to write data into bus(%d), addr(0x%x), offset(0x%x), value(0x%x), error code(%d)\r\n",
			   __FUNCTION__, __LINE__, bus, DSP_ADDR, offset, value, ret);
		return ret;
	}


    // Check Reg0x02 is changed to 0x0(Busy:0x01, no-busy:0x00)
    offset = 0x02;
    i 	   = DSP_READ_MAX_RETRY;
    while(i--) {
		ret = onlp_i2c_readb(bus, DSP_ADDR, offset, ONLP_I2C_F_FORCE);
		if (ret < 0) {
			DEBUG_PRINT("%s(%d): Failed to read data from bus(%d), addr(0x%x), offset(0x%x), value(0x%x), error code(%d)\r\n",
				   __FUNCTION__, __LINE__, bus, DSP_ADDR, offset, value, ret);
			return ret;
		}

		if (ret == DSP_BUSY_VALUE) {
			usleep(NTT_I2C_CMD_WAIT_US);
			continue;
		}

		break;
    }

    if (ret == DSP_BUSY_VALUE) {
        DEBUG_PRINT("%s(%d): Device is busy!! Not change to Idle\r\n", __FUNCTION__, __LINE__);
        return ONLP_STATUS_E_INTERNAL;   
    }


	/* Read the output data from register 0x20 ~ 0x23
	 */
	for (i = 0; i <= 3; i++) {
		offset = 0x20 + i;

		ret = onlp_i2c_readb(bus, DSP_ADDR, offset, ONLP_I2C_F_FORCE);
		if (ret < 0) {
			DEBUG_PRINT("%s(%d): Failed to read data from bus(%d), addr(0x%x), offset(0x%x), value(0x%x), error code(%d)\r\n",
				   __FUNCTION__, __LINE__, bus, DSP_ADDR, offset, value, ret);
			return ret;
		}

		if (ret_val) {
			*ret_val <<= 8;
			*ret_val |= (ret & 0xff);
		}
	}

	return ret;
}

int cfp_eeprom_read(int port, uint8_t data[256])
{
	int i, bus, ret = 0, value = 0;
	struct dsp_ctrl_s eeprom_cmd = {"R", 0x40008000};
	int i2c_bus[] = {41, 43, 45, 47, 49, 51, 53, 55};

	/* init dsp card */
	bus = i2c_bus[port % 8];

	/* read eeprom */
	for (i = 0; i < 256; i++) {
		ret = dsp_exec_read(bus, &eeprom_cmd, &value);

		if (ret < 0) {
			AIM_LOG_ERROR("Unable to read the eeprom of port (%d), check if it has been initialized properly by dsp_ctrl ", port);
			return ret;
		}

		DEBUG_PRINT("%s(%d): command:(%s0x%x), Value(0x%x)\r\n", __FUNCTION__, __LINE__, eeprom_cmd.type, eeprom_cmd.value1, value);
		data[i] = (value & 0xff);
		eeprom_cmd.value1++;
	}

	return 0;
}

#define EXPANSION_CARD_FORMAT	 "/sys/bus/i2c/devices/%d-006a/%s"

typedef enum card_type {
	CARD_TYPE_Q28,
	CARD_TYPE_DCO,
	CARD_TYPE_ACO,
	CARD_TYPE_NOT_PRESENT,
	CARD_TYPE_UNKNOWN
} card_type_t;

card_type_t get_card_type(int port)
{
	int slot = 0;
	int type = 0;
	int present = 0;
	int i2c_bus[] = {41, 43, 45, 47, 49, 51, 53, 55};

	/* Get card slot of this port */
	slot = (port % 16) / 2;
	
    if (onlp_file_read_int(&present, EXPANSION_CARD_FORMAT, i2c_bus[slot], "card_present") < 0) {
        AIM_LOG_ERROR("Unable to read card present status from bus(%d)\r\n", i2c_bus[slot]);
        return ONLP_STATUS_E_INTERNAL;
    }

	if (!present) {
		return CARD_TYPE_NOT_PRESENT;
	}

    if (onlp_file_read_int(&type, EXPANSION_CARD_FORMAT, i2c_bus[slot], "card_type") < 0) {
        AIM_LOG_ERROR("Unable to read card_type from bus(%d)\r\n", i2c_bus[slot]);
        return CARD_TYPE_UNKNOWN;
    }

	if (type != CARD_TYPE_Q28 && type != CARD_TYPE_DCO) {
		type = CARD_TYPE_ACO; /* workaround */
	}

	//printf("CARD_TYPE = (%d)\r\n", type);
	return type;
}

port_type_t get_port_type(int port)
{
	card_type_t card_type;
	port_type_t port_type;

	if (port < 16) {
		return PORT_TYPE_Q28;
	}

	/* Get card type to get correct port mapping table */
	card_type = get_card_type(port);

	switch (card_type) {
		case CARD_TYPE_Q28: port_type = PORT_TYPE_Q28; break;
		case CARD_TYPE_ACO: port_type = PORT_TYPE_ACO_200G; break;
		case CARD_TYPE_DCO: port_type = PORT_TYPE_DCO_200G; break;
		default: port_type = PORT_TYPE_UNKNOWN; break;
	}

	//printf("Port type = (%d)\r\n", port_type);
	return port_type;
}

#if 0
int get_port_number(void)
{
	/* Read type from expansion card */
	int port_number;
	card_type_t card_type = get_card_type();

	switch (card_type) {
		case CARD_TYPE_Q28: port_number = 32; break;
		case CARD_TYPE_ACO: /* fall through */
		case CARD_TYPE_DCO: port_number = 24; break;
		case CARD_TYPE_UNKNOWN: /* fall through */
		default: port_number = 16; break;
	}

	printf("Port Number = (%d)\r\n", port_number);
	return port_number;
}
#endif

