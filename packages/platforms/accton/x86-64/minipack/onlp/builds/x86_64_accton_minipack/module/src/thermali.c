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

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

#define THERMAL_CPU_CORE_PATH_FORMAT "/sys/class/hwmon/hwmon0/temp1_input"
#define THERMAL_PATH_FORMAT "/sys/bus/platform/devices/minipack_psensor/temp%d_input"

struct {
    onlp_oid_t id;
    onlp_oid_desc_t des;
} thermal_des [] = {
  {},
  {THERMAL_CPU_CORE, "CPU Core"},
  {THERMAL_1_ON_MAIN_BROAD, "TMP75-1"},
  {THERMAL_2_ON_MAIN_BROAD, "TMP75-2"},
  {THERMAL_3_ON_MAIN_BROAD, "TMP75-3"},
  {THERMAL_4_ON_MAIN_BROAD, "TMP75-4"},
  {THERMAL_5_ON_MAIN_BROAD, "TMP75-5"},
  {THERMAL_6_ON_MAIN_BROAD, "TMP75-6"},
  {THERMAL_7_ON_MAIN_BROAD, "TMP75-7"},
  {THERMAL_8_ON_MAIN_BROAD, "TMP75-8"},
  {THERMAL_9_ON_MAIN_BROAD, "TMP75-9"},
  {THERMAL_10_ON_MAIN_BROAD, "TMP75-10"},
  {THERMAL_11_ON_MAIN_BROAD, "TMP75-11"},
  {THERMAL_12_ON_MAIN_BROAD, "TMP75-12"},
  {THERMAL_PIM1_1,         "PIM1-LM75_48"},
  {THERMAL_PIM1_2,         "PIM1-LM75_4B"},
  {THERMAL_PIM1_3,         "PIM1-adm1278"},
  {THERMAL_PIM2_1,         "PIM2-LM75_48"},
  {THERMAL_PIM2_2,         "PIM2-LM75_4B"},
  {THERMAL_PIM2_3,         "PIM2-adm1278"},
  {THERMAL_PIM3_1,         "PIM3-LM75_48"},
  {THERMAL_PIM3_2,         "PIM3-LM75_4B"},
  {THERMAL_PIM3_3,         "PIM3-adm1278"},
  {THERMAL_PIM4_1,         "PIM4-LM75_48"},
  {THERMAL_PIM4_2,         "PIM4-LM75_4B"},
  {THERMAL_PIM4_3,         "PIM4-adm1278"},
  {THERMAL_PIM5_1,         "PIM5-LM75_48"},
  {THERMAL_PIM5_2,         "PIM5-LM75_4B"},
  {THERMAL_PIM5_3,         "PIM5-adm1278"},  
  {THERMAL_PIM6_1,         "PIM6-LM75_48"},
  {THERMAL_PIM6_2,         "PIM6-LM75_4B"},
  {THERMAL_PIM6_3,         "PIM6-adm1278"},
  {THERMAL_PIM7_1,         "PIM7-LM75_48"},
  {THERMAL_PIM7_2,         "PIM7-LM75_4B"},
  {THERMAL_PIM7_3,         "PIM7-adm1278"},
  {THERMAL_PIM8_1,         "PIM8-LM75_48"},
  {THERMAL_PIM8_2,         "PIM8-LM75_4B"},
  {THERMAL_PIM8_3,         "PIM8-adm1278"},
};


/* Static values */
static onlp_thermal_info_t linfo[ONLP_THERMAL_ID_MAX] = {{{0}}};

static bool _is_set(onlp_thermal_info_t *info)
{
    if (info[1].hdr.id){
        return true;
    }
    return false;
}
/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
    int i;
    if (!_is_set(linfo)){
        for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++){
            linfo[i].hdr.id = ONLP_THERMAL_ID_CREATE(thermal_des[i].id);
            AIM_SNPRINTF(linfo[i].hdr.description,
                sizeof(linfo[i].hdr.description), "%s", thermal_des[i].des);

                linfo[i].status = ONLP_THERMAL_STATUS_PRESENT;
                linfo[i].caps = ONLP_THERMAL_CAPS_ALL;
                linfo[i].mcelsius = 0;
                linfo[i].thresholds.warning = ONLP_THERMAL_THRESHOLD_WARNING_DEFAULT;
                linfo[i].thresholds.error = ONLP_THERMAL_THRESHOLD_ERROR_DEFAULT;
                linfo[i].thresholds.shutdown = ONLP_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT;
        }
    }
    return ONLP_STATUS_OK;
}


#define IS_PIM_TID(_id)  (_id >= THERMAL_PIM1_1 && _id <= THERMAL_PIM8_3)? true : false
#define NO_TEMP_OF_PIM   (3)

extern int onlp_read_pim_present(uint32_t *pbmp);

static int update_for_pim_presence(
    int pim, onlp_thermal_info_t* info, uint32_t *pim_bmap)
{
    int ret;
    ret = onlp_read_pim_present(pim_bmap);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    if (*pim_bmap & BIT(pim)) {
        info->status |= ONLP_THERMAL_STATUS_PRESENT;
    } else {
        info->status &= ~ONLP_THERMAL_STATUS_PRESENT;
    }
    return ONLP_STATUS_OK;
}

/* temp1_input sysfs of PIM are graped from the rear,
 * if any PIM absent after this one, temp_idx should add that missed count.
 */
static int recalc_temp_idx(
     uint32_t pim_idx, uint32_t pim_bmap, uint32_t *temp_idx)
{
    int i;
    for (i = PLATFOTM_NUM_OF_PIM-1; i > pim_idx; i--){
        if(!(pim_bmap & BIT(i))) {
            *temp_idx += NO_TEMP_OF_PIM;
        }
    }
    return ONLP_STATUS_OK;   
}

/*
 * Retrieve the information structure for the given thermal OID.
 *
 * If the OID is invalid, return ONLP_E_STATUS_INVALID.
 * If an unexpected error occurs, return ONLP_E_STATUS_INTERNAL.
 * Otherwise, return ONLP_STATUS_OK with the OID's information.
 *
 * Note -- it is expected that you fill out the information
 * structure even if the sensor described by the OID is not present.
 */
int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    int   tid;
    char  path[64] = {0};
    VALIDATE(id);

    tid = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[tid];

    /* get path */
    if (THERMAL_CPU_CORE == tid) {
        sprintf(path, THERMAL_CPU_CORE_PATH_FORMAT);
        if (onlp_file_read_int(&info->mcelsius, path) < 0) {
            AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
            return ONLP_STATUS_E_INTERNAL;
        }
    } else if (!IS_PIM_TID(tid)){
        sprintf(path, THERMAL_PATH_FORMAT, tid - THERMAL_1_ON_MAIN_BROAD + 1);
        if (onlp_file_read_int(&info->mcelsius, path) < 0) {
            AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
            return ONLP_STATUS_E_INTERNAL;
        }
    } else if (IS_PIM_TID(tid)){
        uint32_t pim_bmap, temp_idx, pim_idx;
        pim_idx = (tid - THERMAL_PIM1_1)/NO_TEMP_OF_PIM;
        update_for_pim_presence(pim_idx, info, &pim_bmap);
        if (!(info->status & ONLP_THERMAL_STATUS_PRESENT)){
            return ONLP_STATUS_OK;
        }

        /*If PIM removed, sensor nodes are removed as well, it needs to shift idx*/
        temp_idx = tid - THERMAL_1_ON_MAIN_BROAD + 1;
        recalc_temp_idx(pim_idx, pim_bmap, &temp_idx);
        sprintf(path, THERMAL_PATH_FORMAT, temp_idx);
        if (onlp_file_read_int(&info->mcelsius, path) < 0) {
            AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", path);
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    return ONLP_STATUS_OK;
}


