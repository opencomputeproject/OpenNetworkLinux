#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <unistd.h>
#include <fcntl.h>
#include "platform_lib.h"
#include <onlp/platformi/sfpi.h>
#include "x86_64_accton_as4630_54te_log.h"

#define PSU_MODEL_NAME_LEN 13
#define I2C_PSU_FAN_DIR_LEN    3

psu_type_t get_psu_type(int id, char *data_buf, int data_len)
{
    int len = 0;
    char *path[] = { PSU1_AC_EEPROM_PREFIX, PSU2_AC_EEPROM_PREFIX };
    char *str = NULL;
    psu_type_t ptype = PSU_TYPE_UNKNOWN;

    /* Read attribute */
    len = onlp_file_read_str(&str, "%s%s", path[id-1], "psu_model_name");
    if (!str || len <= 0 || len < PSU_MODEL_NAME_LEN) {
        AIM_FREE_IF_PTR(str);
        return PSU_TYPE_UNKNOWN;
    }

    /* Check AC model name */
    if (strncmp(str, "YM-1151F-A01R", strlen("YM-1151F-A01R")) == 0)
        ptype = PSU_TYPE_YM1151F_F2B;
    else if (strncmp(str, "YM-1151D-A03R", strlen("YM-1151D-A03R")) == 0)
        ptype = PSU_TYPE_YM1151D_F2B;
    else
        ptype = PSU_TYPE_YM1151D_B2F;

    if (len < data_len)
        aim_strlcpy(data_buf, str, len+1);

    AIM_FREE_IF_PTR(str);
    return ptype;
}

int psu_pmbus_info_get(int id, char *node, int *value)
{
    char *path[] = { PSU1_AC_PMBUS_PREFIX, PSU2_AC_PMBUS_PREFIX };
    *value = 0;

    return onlp_file_read_int(value, "%s%s", path[id-1], node);
}

int get_psu_eeprom_str(int id, char *data_buf, int data_len, char *data_name)
{
    int   len    = 0;
    char *path[] = { PSU1_AC_EEPROM_PREFIX, PSU2_AC_EEPROM_PREFIX };
    char *str = NULL;

    /* Read attribute */
    len = onlp_file_read_str(&str, "%s%s", path[id-1], data_name);
    if (!str || len <= 0) {
        AIM_FREE_IF_PTR(str);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (len > data_len) {
        AIM_FREE_IF_PTR(str);
        return ONLP_STATUS_E_INVALID;
    }

    aim_strlcpy(data_buf, str, len+1);
    AIM_FREE_IF_PTR(str);
    return ONLP_STATUS_OK;
}
