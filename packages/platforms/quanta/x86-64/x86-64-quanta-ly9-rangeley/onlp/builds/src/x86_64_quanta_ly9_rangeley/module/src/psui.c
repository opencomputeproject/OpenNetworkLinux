/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <x86_64_quanta_ly9_rangeley/x86_64_quanta_ly9_rangeley_config.h>
#include <x86_64_quanta_ly9_rangeley/x86_64_quanta_ly9_rangeley_gpio_table.h>
#include <onlp/platformi/psui.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlplib/gpio.h>
#include "x86_64_quanta_ly9_rangeley_int.h"
#include "x86_64_quanta_ly9_rangeley_log.h"
#include <AIM/aim_string.h>

struct psu_info_s psu_info[] = {
	{}, /* Not used */
	{ .path = "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-24/24-006f", .present = QUANTA_LY9_PSU_GPIO_PSU1_PRSNT_N, .busno = 24, .addr = 0x6f},
	{ .path = "/sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-25/25-0069", .present = QUANTA_LY9_PSU_GPIO_PSU2_PRSNT_N, .busno = 25, .addr = 0x69},
};

int
onlp_psui_init(void)
{
    return 0;
}

static onlp_psu_info_t psus__[] = {
    { }, /* Not used */
    {
        {
            PSU_OID_PSU1,
            "Quanta LY9 RPSU-1",
            0,
            {
                FAN_OID_FAN9,
            },
        }
    },
    {
        {
            PSU_OID_PSU2,
            "Quanta LY9 RPSU-2",
            0,
            {
                FAN_OID_FAN10,
            },
        }
    },
};

#define PMBUS_MFR_MODEL			0x9A
#define PMBUS_MFR_SERIAL		0x9E
#define PMBUS_MFR_MODEL_LEN		20
#define PMBUS_MFR_SERIAL_LEN	19

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int rv;
    int pid = ONLP_OID_ID_GET(id);
    *info = psus__[pid];
    const char* dir = psu_info[pid].path;
    uint8_t buffer[ONLP_CONFIG_INFO_STR_MAX];
	int value = -1;

	rv = onlp_gpio_get(psu_info[pid].present, &value);
	if(rv < 0) {
        return rv;
    }
	else if(value == 1) {
        info->status &= ~1;
        return 0;
	}

    if(onlp_file_read_int(&info->mvin, "%s*in1_input", dir) == 0 && info->mvin >= 0) {
        info->caps |= ONLP_PSU_CAPS_VIN;
    }

    /* PSU is present and powered. */
    info->status |= 1;

    memset(buffer, 0, sizeof(buffer));
    rv = onlp_i2c_block_read(psu_info[pid].busno, psu_info[pid].addr, PMBUS_MFR_MODEL, PMBUS_MFR_MODEL_LEN, buffer, ONLP_I2C_F_FORCE);
    buffer[buffer[0] + 1] = 0x00;
    if(rv >= 0){
        aim_strlcpy(info->model, (char *) (buffer+1), (buffer[0] + 1));
    }
    else{
        strcpy(info->model, "Missing");
    }

    memset(buffer, 0, sizeof(buffer));
    rv = onlp_i2c_block_read(psu_info[pid].busno, psu_info[pid].addr, PMBUS_MFR_SERIAL, PMBUS_MFR_SERIAL_LEN, buffer, ONLP_I2C_F_FORCE);
    buffer[buffer[0] + 1] = 0x00;
    if(rv >= 0){
        aim_strlcpy(info->serial, (char *) (buffer+1), (buffer[0] + 1));
    }
    else{
        strcpy(info->serial, "Missing");
    }

    info->caps |= ONLP_PSU_CAPS_AC;

    if(onlp_file_read_int(&info->miin, "%s*curr1_input", dir) == 0 && info->miin >= 0) {
        info->caps |= ONLP_PSU_CAPS_IIN;
    }
    if(onlp_file_read_int(&info->miout, "%s*/curr2_input", dir) == 0 && info->miout >= 0) {
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }
    if(onlp_file_read_int(&info->mvout, "%s*in2_input", dir) == 0 && info->mvout >= 0) {
        info->caps |= ONLP_PSU_CAPS_VOUT;
    }
    if(onlp_file_read_int(&info->mpin, "%s*power1_input", dir) == 0 && info->mpin >= 0) {
        info->caps |= ONLP_PSU_CAPS_PIN;
        /* The pmbus driver reports power in micro-units */
        info->mpin /= 1000;
    }
    if(onlp_file_read_int(&info->mpout, "%s*power2_input", dir) == 0 && info->mpout >= 0) {
        info->caps |= ONLP_PSU_CAPS_POUT;
        /* the pmbus driver reports power in micro-units */
        info->mpout /= 1000;
    }
    return ONLP_STATUS_OK;
}
