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
#include <onlp/platformi/sfpi.h>
#include "platform_lib.h"

static int _sff_present (void *e);
static int _sff_lpmode  (void *e, int sval, int *gval, int *sup);
static int _sff_reset   (void *e, int sval, int *gval, int *sup);

static int _sff8636_txdisable (void *e, int sval, int *gval, int *sup);

static plat_sff_t plat_sffs[] = {
	[PLAT_SFF_ID_MIN] = {
		.valid = 1,
		.present = _sff_present,
		.present_cpld_reg = CPLD_REG(CPLD_SWPLD, 0x0d, 0, 1),
		.lpmode = _sff_lpmode,
		.lpmode_cpld_reg  = CPLD_REG(CPLD_SWPLD, 0x09, 0, 1),
		.reset = _sff_reset,
		.reset_cpld_reg   = CPLD_REG(CPLD_SWPLD, 0x11, 0, 1),
		.txdisable = _sff8636_txdisable,
		.bus = 6,
	},
	// total 32 port .... init in onlp_sfpi_init
	[PLAT_SFF_ID_MAX] = {
		.valid = 1,
	},
};

static int _sff_present (void *e)
{
	int val;
	plat_sff_t *sff = e;

	val = cpld_reg_get (&sff->present_cpld_reg);

    /*
     * Return 1 if present.
     * Return 0 if not present.
     * Return < 0 if error.
     */

	if (val < 0)
		return val;
	return val ? 0 : 1;
}

static int _sff_lpmode  (void *e, int sval, int *gval, int *sup)
{
	plat_sff_t *sff = e;

	if (sup) {
		if (cpld_reg_is_valid(&sff->lpmode_cpld_reg))
			*sup = 1;
	}
	if (gval)
		*gval = cpld_reg_get (&sff->lpmode_cpld_reg);

	if (sval >= 0)
		cpld_reg_set (&sff->lpmode_cpld_reg, sval ? 1 : 0);

	return ONLP_STATUS_OK;
}

static int _sff_reset   (void *e, int sval, int *gval, int *sup)
{
	plat_sff_t *sff = e;

	if (sup) {
		if (cpld_reg_is_valid(&sff->reset_cpld_reg))
			*sup = 1;
	}

	if (gval) {
		*gval = cpld_reg_get (&sff->reset_cpld_reg);
		*gval = *gval ? 0 : 1;
	}

	if (sval >= 0) {
		cpld_reg_set (&sff->reset_cpld_reg, sval ? 0 : 1);
	}

	return ONLP_STATUS_OK;
}

static int _sff8636_txdisable (void *e, int sval, int *gval, int *sup)
{
	int ret = ONLP_STATUS_OK;
	plat_sff_t *sff = e;

	if (sup) {
			*sup = 1;
	}

	if (gval) {
		if (cpld_reg_is_valid(&sff->txdisable_cpld_reg)) {
			*gval = cpld_reg_get (&sff->reset_cpld_reg);
		} else {
			// following sff8636 spec
			ret = onlp_i2c_readb (sff->bus, 0x50, 86, ONLP_I2C_F_DISABLE_READ_RETRIES);
			if (ret >= 0)
				*gval = ret;
		}
	}

	if (sval >= 0) {
		if (cpld_reg_is_valid(&sff->txdisable_cpld_reg)) {
			cpld_reg_set (&sff->reset_cpld_reg, sval ? 1 : 0);
		} else {
			// following sff8636 spec
			ret = onlp_i2c_writeb (sff->bus, 0x50, 86, sval, 0);
		}
	}

	return ret;
}

static int _sff_is_valid (int p)
{
	plat_sff_t *sff;

    if (p >= PLAT_SFF_ID_MIN && p <= PLAT_SFF_ID_MAX) {
		sff = &plat_sffs[p];
		if (sff->valid)
			return 1;
    }

	return 0;
}


/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
int
onlp_sfpi_init(void)
{
	int p;
	plat_sff_t *base = &plat_sffs[PLAT_SFF_ID_MIN];
	plat_sff_t *sff;

	for (p = PLAT_SFF_ID_MIN + 1; p <= PLAT_SFF_ID_MAX ; p ++) {
		sff = &plat_sffs[p];

		sff->valid = 1;

		// .present_cpld_reg
		sff->present = base->present;
		sff->present_cpld_reg.id    = base->present_cpld_reg.id;
		sff->present_cpld_reg.reg   = base->present_cpld_reg.reg   - (p - 1) / 8;
		sff->present_cpld_reg.field = base->present_cpld_reg.field + (p - 1) % 8;
		sff->present_cpld_reg.len   = base->present_cpld_reg.len;
		sff->present_cpld_reg.valid = 1,

		// .lpmode
		sff->lpmode = base->lpmode;
		sff->lpmode_cpld_reg.id    = base->lpmode_cpld_reg.id;
		sff->lpmode_cpld_reg.reg   = base->lpmode_cpld_reg.reg   - (p - 1) / 8;
		sff->lpmode_cpld_reg.field = base->lpmode_cpld_reg.field + (p - 1) % 8;
		sff->lpmode_cpld_reg.len   = base->lpmode_cpld_reg.len;
		sff->lpmode_cpld_reg.valid = 1,

		// .reset
		sff->reset = base->reset;
		sff->reset_cpld_reg.id    = base->reset_cpld_reg.id;
		sff->reset_cpld_reg.reg   = base->reset_cpld_reg.reg   - (p - 1) / 8;
		sff->reset_cpld_reg.field = base->reset_cpld_reg.field + (p - 1) % 8;
		sff->reset_cpld_reg.len   = base->reset_cpld_reg.len;
		sff->reset_cpld_reg.valid = 1,

		//.txdisable
		sff->txdisable = base->txdisable;

		// bus
		sff->bus = base->bus + p - 1;
	}
    /* Called at initialization time */
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
	int p;

    for (p = PLAT_SFF_ID_MIN ; p <= PLAT_SFF_ID_MAX ; p++) {
		if (_sff_is_valid (p))
        	AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
	plat_sff_t *sff;
    /*
     * Return 1 if present.
     * Return 0 if not present.
     * Return < 0 if error.
     */
	if (!_sff_is_valid (port))
		return -1;
	sff = &plat_sffs[port];

	if (sff->present == NULL) {
		// If not present, it means it is present always.
		return 1;
	}

	return sff->present (sff);
}

static int _sff_read_eeprom (int port, uint8_t devaddr, uint8_t *data)
{
    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
	plat_sff_t *sff;
	int i;
   
    memset(data, 0, 256);

	if (!_sff_is_valid(port)) {
        return ONLP_STATUS_E_INVALID;
	}

    if (onlp_sfpi_is_present(port) <= 0)
    {
        return ONLP_STATUS_E_MISSING;
    }

	sff = &plat_sffs[port];

    for (i = 0 ; i < (256/ONLPLIB_CONFIG_I2C_BLOCK_SIZE); i++) {
        if (onlp_i2c_block_read (sff->bus, devaddr,
			ONLPLIB_CONFIG_I2C_BLOCK_SIZE * i,
			ONLPLIB_CONFIG_I2C_BLOCK_SIZE,
			&data [ONLPLIB_CONFIG_I2C_BLOCK_SIZE * i],
			0) < 0)
        {
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    return ONLP_STATUS_OK;

}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
	return _sff_read_eeprom (port, 0x50, data);
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
	return _sff_read_eeprom (port, 0x51, data);
}

int onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
	plat_sff_t *sff;
   
	if (!_sff_is_valid(port)) {
        return ONLP_STATUS_E_INVALID;
	}

    if (onlp_sfpi_is_present(port) <= 0)
    {
        return ONLP_STATUS_E_MISSING;
    }

	sff = &plat_sffs[port];

	return onlp_i2c_readb (sff->bus, devaddr, addr, 0);
}

int onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
	plat_sff_t *sff;
   
	if (!_sff_is_valid(port)) {
        return ONLP_STATUS_E_INVALID;
	}

    if (onlp_sfpi_is_present(port) <= 0)
    {
        return ONLP_STATUS_E_MISSING;
    }

	sff = &plat_sffs[port];

	return onlp_i2c_writeb (sff->bus, devaddr, addr, value, 0);
}

int onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
	plat_sff_t *sff;
   
	if (!_sff_is_valid(port)) {
        return ONLP_STATUS_E_INVALID;
	}

    if (onlp_sfpi_is_present(port) <= 0)
    {
        return ONLP_STATUS_E_MISSING;
    }

	sff = &plat_sffs[port];

	return onlp_i2c_readw (sff->bus, devaddr, addr, 0);
}

int onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
	plat_sff_t *sff;
   
	if (!_sff_is_valid(port)) {
        return ONLP_STATUS_E_INVALID;
	}

    if (onlp_sfpi_is_present(port) <= 0)
    {
        return ONLP_STATUS_E_MISSING;
    }

	sff = &plat_sffs[port];

	return onlp_i2c_readw (sff->bus, devaddr, addr, 0);
}

int onlp_sfpi_presence_bitmap_get (onlp_sfp_bitmap_t* dst)
{
	return ONLP_STATUS_E_UNSUPPORTED;
}

int onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
	return ONLP_STATUS_E_UNSUPPORTED;
}

int onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv)
{
	plat_sff_t *sff;
   
	*rv = 0;
	if (control > ONLP_SFP_CONTROL_LAST)
		return ONLP_STATUS_OK;

	if (!_sff_is_valid(port)) {
        return ONLP_STATUS_E_INVALID;
	}

	sff = &plat_sffs[port];

	switch (control) {
	case ONLP_SFP_CONTROL_RESET_STATE:
	case ONLP_SFP_CONTROL_RESET:
		if (sff->reset) sff->reset (sff, -1, NULL, rv);
		break;

	case ONLP_SFP_CONTROL_RX_LOS:
		if (sff->rxlos) sff->rxlos (sff, -1, NULL, rv);
		break;

	case ONLP_SFP_CONTROL_TX_FAULT:
		if (sff->txfault) sff->txfault (sff, -1, NULL, rv);
		break;

	case ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL:
	case ONLP_SFP_CONTROL_TX_DISABLE:
		if (sff->txdisable) sff->txdisable (sff, -1, NULL, rv);
		break;

	case ONLP_SFP_CONTROL_LP_MODE:
		if (sff->lpmode) sff->lpmode (sff, -1, NULL, rv);
		break;

	case ONLP_SFP_CONTROL_POWER_OVERRIDE:
	default:
		break;
	}

	return ONLP_STATUS_OK;

}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
	plat_sff_t *sff;
   
	if (control > ONLP_SFP_CONTROL_LAST)
		return ONLP_STATUS_E_UNSUPPORTED;

	if (!_sff_is_valid(port)) {
        return ONLP_STATUS_E_INVALID;
	}

	sff = &plat_sffs[port];

	switch (control) {
	case ONLP_SFP_CONTROL_RESET:
		if (sff->reset) return sff->reset (sff, value, NULL, NULL);
		break;

	case ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL:
	case ONLP_SFP_CONTROL_TX_DISABLE:
		if (sff->txdisable) return sff->txdisable (sff, value, NULL, NULL);
		break;

	case ONLP_SFP_CONTROL_LP_MODE:
		if (sff->lpmode) return sff->lpmode (sff, value, NULL, NULL);
		break;

	default:
		break;
	}

	return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
	plat_sff_t *sff;
   
	if (control > ONLP_SFP_CONTROL_LAST)
		return ONLP_STATUS_E_UNSUPPORTED;

	if (!_sff_is_valid(port)) {
        return ONLP_STATUS_E_INVALID;
	}

	sff = &plat_sffs[port];

	switch (control) {
	case ONLP_SFP_CONTROL_RESET_STATE:
	case ONLP_SFP_CONTROL_RESET:
		if (sff->reset) return sff->reset (sff, -1, value, NULL);
		break;

	case ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL:
	case ONLP_SFP_CONTROL_TX_DISABLE:
		if (sff->txdisable) return sff->txdisable (sff, -1, value, NULL);
		break;

	case ONLP_SFP_CONTROL_LP_MODE:
		if (sff->lpmode) return sff->lpmode (sff, -1, value, NULL);
		break;

	case ONLP_SFP_CONTROL_RX_LOS:
		if (sff->rxlos) return sff->rxlos (sff, -1, value, NULL);
		break;

	case ONLP_SFP_CONTROL_TX_FAULT:
		if (sff->txfault) return sff->txfault (sff, -1, value, NULL);
		break;

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
