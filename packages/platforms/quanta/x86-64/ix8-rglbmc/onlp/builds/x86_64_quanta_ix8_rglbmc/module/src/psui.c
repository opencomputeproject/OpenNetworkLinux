/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <onlp/platformi/psui.h>
#include <onlplib/file.h>
#include "x86_64_quanta_ix8_rglbmc_int.h"
#include "x86_64_quanta_ix8_rglbmc_log.h"

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

int asctoint(int in)
{
	if (in < 58) return (in - 48);
	else return (in - 87);
}

static int
sys_psu_info_get__(onlp_psu_info_t* info, int id)
{
	int psu_present = 0;
	int err = 0, strlen = 0;
	char *tempstr = NULL;

	int fan_id = 0;
	int pin_id = 0, pout_id = 0;
	int vin_id = 0, vout_id = 0;
	int cin_id = 0, cout_id = 0;
	int temp1_id = 0, temp2_id = 0, temp3_id = 0;

	if (id == PSU_OID_PSU101)
	{
		sprintf(info->hdr.description, "Quanta IX8 RPSU-1");

		fan_id = QUANTA_IX8_PSU1_FAN_ID;
		pin_id = QUANTA_IX8_PSU1_PIN_ID, pout_id = QUANTA_IX8_PSU1_POUT_ID;
		vin_id = QUANTA_IX8_PSU1_VIN_ID, vout_id = QUANTA_IX8_PSU1_VOUT_ID;
		cin_id = QUANTA_IX8_PSU1_CIN_ID, cout_id = QUANTA_IX8_PSU1_COUT_ID;
		temp1_id = QUANTA_IX8_PSU1_TEMP1_ID, temp2_id = QUANTA_IX8_PSU1_TEMP2_ID, temp3_id = QUANTA_IX8_PSU1_TEMP3_ID;
	}
	else if (id == PSU_OID_PSU102)
	{
		sprintf(info->hdr.description, "Quanta IX8 RPSU-2");

		fan_id = QUANTA_IX8_PSU2_FAN_ID;
		pin_id = QUANTA_IX8_PSU2_PIN_ID, pout_id = QUANTA_IX8_PSU2_POUT_ID;
		vin_id = QUANTA_IX8_PSU2_VIN_ID, vout_id = QUANTA_IX8_PSU2_VOUT_ID;
		cin_id = QUANTA_IX8_PSU2_CIN_ID, cout_id = QUANTA_IX8_PSU2_COUT_ID;
		temp1_id = QUANTA_IX8_PSU2_TEMP1_ID, temp2_id = QUANTA_IX8_PSU2_TEMP2_ID, temp3_id = QUANTA_IX8_PSU2_TEMP3_ID;
	}
	else
	{
		return ONLP_STATUS_E_INVALID;
	}

	info->hdr.id = id;
	info->hdr.poid = 0;
	info->hdr.coids[0] = ONLP_FAN_ID_CREATE(fan_id);
	info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(temp1_id);
	info->hdr.coids[2] = ONLP_THERMAL_ID_CREATE(temp2_id);
	info->hdr.coids[3] = ONLP_THERMAL_ID_CREATE(temp3_id);
	info->status = ONLP_PSU_STATUS_PRESENT;

	/* get present */
	err = onlp_file_read_int(&psu_present, SYS_HWMON_PREFIX "/power%d_present", pout_id);
	if ((err != ONLP_STATUS_OK) || (psu_present == 0)) {
		info->status = ONLP_PSU_STATUS_UNPLUGGED;
		return ONLP_STATUS_OK;
	}

	/* get pin */
	err = onlp_file_read_int(&info->mpin, SYS_HWMON_PREFIX "/power%d_input", pin_id);
	if (err == ONLP_STATUS_OK) info->caps |= ONLP_PSU_CAPS_PIN;

	/* get pout */
	err = onlp_file_read_int(&info->mpout, SYS_HWMON_PREFIX "/power%d_input", pout_id);
	if (err == ONLP_STATUS_OK) info->caps |= ONLP_PSU_CAPS_POUT;

	if (info->mpout == 0) info->status = ONLP_PSU_STATUS_FAILED;

	/* get vin */
	err = onlp_file_read_int(&info->mvin, SYS_HWMON_PREFIX "/in%d_input", vin_id);
	if (err == ONLP_STATUS_OK) info->caps |= ONLP_PSU_CAPS_VIN;

	/* get vout */
	err = onlp_file_read_int(&info->mvout, SYS_HWMON_PREFIX "/in%d_input", vout_id);
	if (err == ONLP_STATUS_OK) info->caps |= ONLP_PSU_CAPS_VOUT;

	/* get cin */
	err = onlp_file_read_int(&info->miin, SYS_HWMON_PREFIX "/curr%d_input", cin_id);
	if (err == ONLP_STATUS_OK) info->caps |= ONLP_PSU_CAPS_IIN;

	/* get cout */
	err = onlp_file_read_int(&info->miout, SYS_HWMON_PREFIX "/curr%d_input", cout_id);
	if (err == ONLP_STATUS_OK) info->caps |= ONLP_PSU_CAPS_IOUT;

	/* get model */
	strlen = onlp_file_read_str(&tempstr, SYS_HWMON_PREFIX "/power%d_model", pout_id);
	if ((strlen > ONLP_STATUS_OK) && (strlen < ONLP_CONFIG_INFO_STR_MAX)) memcpy(&info->model, tempstr, strlen);
	if (tempstr) {
		aim_free(tempstr);
		tempstr = NULL;
	}

	/* get sn */
	strlen = onlp_file_read_str(&tempstr, SYS_HWMON_PREFIX "/power%d_sn", pout_id);
	if ((strlen > ONLP_STATUS_OK) && (strlen < ONLP_CONFIG_INFO_STR_MAX)) memcpy(&info->serial, tempstr, strlen);
	if (tempstr) {
		aim_free(tempstr);
		tempstr = NULL;
	}

	return ONLP_STATUS_OK;
}

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* rv)
{
	/* init info */
	onlp_psu_info_t psu__ =
	{
		{ 0, "", 0, { 0, } },
		"","",
		0,0,0,0,0,0,0,0
	};

	*rv = psu__;

	return sys_psu_info_get__(rv, id);
}
