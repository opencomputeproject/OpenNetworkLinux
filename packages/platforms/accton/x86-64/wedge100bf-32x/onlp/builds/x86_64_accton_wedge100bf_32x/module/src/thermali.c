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
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include "platform_lib.h"

#define THERMAL_PATH_FORMAT "/sys/bus/i2c/drivers/lm75/%s/temp1_input"
#define THERMAL_CPU_CORE_PATH_FORMAT "/sys/bus/i2c/drivers/com_e_driver/%s/temp2_input"

static char* directory[] =  /* must map with onlp_thermal_id */
{
    NULL,
    "4-0033",                  /* CPU_CORE files */
    "3-0048",
    "3-0049",
    "3-004a",
    "3-004b",
    "3-004c",
    "8-0048",
    "8-0049",
};

/* Static values */
static onlp_thermal_info_t linfo[] = {
    { }, /* Not used */
    { { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE), "CPU Core", 0},
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BOARD), "TMP75-1", 0},
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BOARD), "TMP75-2", 0},
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BOARD), "TMP75-3", 0},
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BOARD), "TMP75-4", 0},
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BOARD), "TMP75-5", 0},
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_6_ON_MAIN_BOARD), "TMP75-6", 0},
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_7_ON_MAIN_BOARD), "TMP75-7", 0},
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
};

/**
 * @brief Software initialization of the Thermal module.
 */
int onlp_thermali_sw_init(void) {
    return ONLP_STATUS_OK;
}

/**
 * @brief Hardware initialization of the Thermal module.
 * @param flags The hardware initialization flags.
 */
int onlp_thermali_hw_init(uint32_t flags) {
    return ONLP_STATUS_OK;
}

/**
 * @brief Deinitialize the thermal software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_thermali_sw_denit(void) {
    return ONLP_STATUS_OK;
}

/**
 * @brief Retrieve the thermal's oid header.
 * @param id The thermal oid.
 * @param[out] rv Receives the header.
 */
int onlp_thermali_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* rv) {
    onlp_thermal_info_t info;
    onlp_thermali_info_get(id, &info);
    *rv = info.hdr;
    return ONLP_STATUS_OK;
}

/**
 * @brief Get the information for the given thermal OID.
 * @param id The Thermal OID
 * @param[out] rv Receives the thermal information.
 */
int onlp_thermali_info_get(onlp_oid_id_t id, onlp_thermal_info_t* info) {
    int   tid;
    char  path[64] = {0};

    tid = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[tid];

    /* get path */
    if (THERMAL_CPU_CORE == tid) {
        sprintf(path, THERMAL_CPU_CORE_PATH_FORMAT, directory[tid]);
    }else {
        sprintf(path, THERMAL_PATH_FORMAT, directory[tid]);
    }

    if (bmc_file_read_int(&info->mcelsius, path, 10) < 0) {
            AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }
    info->mcelsius = info->mcelsius;
    info->hdr.status |= ONLP_OID_STATUS_FLAG_PRESENT;
    return ONLP_STATUS_OK;
}
