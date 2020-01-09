#include <onlp/platformi/attributei.h>
#include <onlp/stdattrs.h>
#include <onlplib/file.h>

#include "platform_lib.h"

int
onlp_attributei_onie_info_get(onlp_oid_t oid, onlp_onie_info_t* rp)
{
    if(oid != ONLP_OID_CHASSIS) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if(rp == NULL) {
        return 1;
    }

    return onlp_onie_decode_file(rp, IDPROM_PATH);
}

static int _sysi_version_parsing(char* file_str, char* str_buf, char* version)
{
    int rv = ONLP_STATUS_OK;
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX*4];
    char *temp;

    rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX*4, &len, file_str);
    if( rv != ONLP_STATUS_OK ) {
        return rv;
    }

    temp = strstr(buf, str_buf);
    if(temp) {
        temp += strlen(str_buf);
        snprintf(version,ONLP_CONFIG_INFO_STR_MAX, temp);
        /*remove '\n'*/
        version[strlen(version)-1] = '\0';
    } else {
        rv = ONLP_STATUS_E_MISSING;
    }
    return rv;
}

static int
__asset_versions(char** cpld, char** other)
{
    int rv = ONLP_STATUS_OK;
    char cpld_str[ONLP_CONFIG_INFO_STR_MAX]= {0};
    char other_str[ONLP_CONFIG_INFO_STR_MAX]= {0};
    char version[ONLP_CONFIG_INFO_STR_MAX];

    rv = _sysi_version_parsing(INV_SYSLED_PREFIX"info", "The CPLD version is ", version);
    if( rv != ONLP_STATUS_OK ) {
        return rv;
    }
    snprintf(cpld_str, ONLP_CONFIG_INFO_STR_MAX, "%s%s ", cpld_str, version);
    rv = _sysi_version_parsing(INV_HWMON_PREFIX"version", "ver: ", version);
    if( rv != ONLP_STATUS_OK ) {
        return rv;
    }
    snprintf(other_str, ONLP_CONFIG_INFO_STR_MAX, "%s%s.%s "
             ,other_str, "psoc", version);

    /*cpld version*/
    if(strlen(cpld_str) > 0) {
        *cpld = aim_fstrdup("%s",cpld_str);
    }

    /*other version*/
    if(strlen(other_str) > 0) {
        *other = aim_fstrdup("%s",other_str);
    }
    return rv;
}

int
onlp_attributei_asset_info_get(onlp_oid_t oid, onlp_asset_info_t* rp)
{
    if(oid != ONLP_OID_CHASSIS) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if(rp == NULL) {
        return ONLP_STATUS_OK;
    }

    rp->oid = oid;

    __asset_versions(&rp->firmware_revision, &rp->additional);

    return ONLP_STATUS_OK;
}
