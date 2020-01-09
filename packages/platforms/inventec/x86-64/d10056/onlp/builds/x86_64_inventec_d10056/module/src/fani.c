/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2054 Big Switch Networks, Inc.
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
 * Fan Platform Implementation Defaults.
 *
 ***********************************************************/
#include <onlp/platformi/fani.h>
#include <onlplib/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#define SLOW_PWM 100
#define NORMAL_PWM 175
#define MAX_PWM 255
#define STEP_SIZE 100
#define FAN_ON_MAIN_BOARD_COUNT 4
#define LOCAL_ID_TO_PSU_ID(id) (id- ONLP_FAN_PSU_1 + 1)

#define FAN_CAPS ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE | ONLP_FAN_CAPS_GET_DIR
#define PSU_FAN_CAPS ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE

static int _fani_status_failed_check(uint32_t* status, int id);
static int _fani_status_present_check(uint32_t* status, onlp_fan_dir_t *dir, int id);
extern int inv_psui_status_get(onlp_oid_id_t id, uint32_t* rv);

#define MAKE_FAN_INFO_NODE_ON_FAN_BOARD(id)                                 \
    { 										                                \
        { 									                                \
            ONLP_FAN_ID_CREATE(ONLP_FAN_##id), "Fan "#id, ONLP_OID_CHASSIS,	\
            { 									                            \
                ONLP_LED_ID_CREATE(ONLP_LED_FAN##id),				        \
            }, 0  									                        \
        },									                                \
        0, FAN_CAPS    						                                \
    }

#define MAKE_FAN_INFO_NODE_ON_PSU(psu_id) 					                \
    {                                                                       \
        {                                                                   \
	        ONLP_FAN_ID_CREATE(ONLP_FAN_PSU_##psu_id), "PSU-"#psu_id" Fan", \
	        ONLP_PSU_ID_CREATE(ONLP_PSU_##psu_id), {0}, 0                   \
        },                                                                  \
        0, PSU_FAN_CAPS						                                    \
    }


/* Static values */
static onlp_fan_info_t __onlp_fan_info[ ] = {
    {},
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(1),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(2),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(3),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(4),
    MAKE_FAN_INFO_NODE_ON_PSU(1),
    MAKE_FAN_INFO_NODE_ON_PSU(2),
};

int inv_fani_status_get(onlp_oid_id_t id, uint32_t* rv)
{
    int result = ONLP_STATUS_OK;
    onlp_fan_info_t* info;
    onlp_oid_hdr_t *hdr;
    uint32_t psu_status;
    info = &__onlp_fan_info[id];
    hdr  = &info->hdr;
    if( (id>=ONLP_FAN_1) && (id<=ONLP_FAN_4) ) {
        result= _fani_status_present_check(&(hdr->status), &(info->dir), id);
        if(result!=ONLP_STATUS_OK) {
            return result;
        }
        if(ONLP_OID_STATUS_FLAG_IS_SET(hdr,PRESENT)) {
            result = _fani_status_failed_check(&(hdr->status), id);
        }
    } else if( (id>=ONLP_FAN_PSU_1) && (id<=ONLP_FAN_PSU_2) ) {
        result=inv_psui_status_get( ONLP_OID_ID_GET(hdr->poid), &psu_status) ;
        if(result!=ONLP_STATUS_OK) {
            return result;
        }
        if(psu_status & ONLP_OID_STATUS_FLAG_PRESENT) {
            hdr->status= ADD_STATE(hdr->status,ONLP_OID_STATUS_FLAG_PRESENT);
            result = _fani_status_failed_check(&(hdr->status), id);
        } else {
            hdr->status = 0;
        }
    } else {
        result = ONLP_STATUS_E_INVALID;
    }

    *rv = hdr->status;
    return result;
}

int
onlp_fani_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, 1, ONLP_FAN_MAX-1);
}

int
onlp_fani_info_get(onlp_oid_id_t id, onlp_fan_info_t* info)
{
    int rv = ONLP_STATUS_OK;
    int lrpm, rrpm, pwm, psu_id;
    onlp_oid_hdr_t *hdr;
    pwm = 0;

    *info = __onlp_fan_info[id];
    hdr = &info->hdr;
    rv = onlp_fani_hdr_get(id , hdr);
    if(rv == ONLP_STATUS_OK) {

        if(ONLP_OID_STATUS_FLAG_IS_SET(hdr,PRESENT)) {
            char*  info_path=hwmon_path(INV_HWMON_BASE);

            switch(id) {
            case ONLP_FAN_1:
            case ONLP_FAN_2:
            case ONLP_FAN_3:
            case ONLP_FAN_4:
                ONLP_TRY(onlp_file_read_int(&lrpm, "%sfan%d_input", info_path, id*2-1));
                ONLP_TRY(onlp_file_read_int(&rrpm, "%sfan%d_input", info_path, id*2));
                ONLP_TRY(onlp_file_read_int(&pwm, "%spwm%d", info_path, id));
                if(lrpm <=0 && rrpm <=0) {
                    info->rpm = 0;
                } else if(lrpm <= 0) {
                    info->rpm = rrpm;
                } else if(rrpm <= 0) {
                    info->rpm = lrpm;
                } else {
                    info->rpm = (lrpm+rrpm)/2;
                }
                _fani_status_present_check(&hdr->status,&info->dir,id);
                break;
            case ONLP_FAN_PSU_1:
            case ONLP_FAN_PSU_2:
                psu_id = LOCAL_ID_TO_PSU_ID(id);
                ONLP_TRY(onlp_file_read_int(&info->rpm, "%srpm_psu%d", info_path, psu_id));
                ONLP_TRY(onlp_file_read_int(&pwm, "%spwm_psu%d", info_path, psu_id));
                break;
            default:
                rv = ONLP_STATUS_E_INVALID;
                break;
            }
            if(rv == ONLP_STATUS_OK) {
                if(info->rpm>0){
                    info->percentage =(pwm*100)/MAX_PWM;
                }else{
                    info->percentage =0;
                }
            }
        } else {
            info->rpm = 0;
            info->percentage = 0;
            info->dir=ONLP_FAN_DIR_UNKNOWN;
        }
        snprintf(info->model, ONLP_CONFIG_INFO_STR_MAX, "N/A");
        snprintf(info->serial, ONLP_CONFIG_INFO_STR_MAX, "N/A");
    }
    return rv;
}
static int _fani_status_failed_check(uint32_t* status, int id)
{
    int rv;
    int lrpm, rrpm, rpm, pwm, psu_id;
    char*  info_path=hwmon_path(INV_HWMON_BASE);

    switch(id) {
    case ONLP_FAN_1:
    case ONLP_FAN_2:
    case ONLP_FAN_3:
    case ONLP_FAN_4:
        ONLP_TRY(onlp_file_read_int(&lrpm, "%sfan%d_input", info_path, id*2-1));
        ONLP_TRY(onlp_file_read_int(&rrpm, "%sfan%d_input", info_path, id*2));
        ONLP_TRY(onlp_file_read_int(&pwm,"%spwm%d", info_path, id));
        if( lrpm <= 0 || rrpm <=0 || pwm <=0 || pwm > MAX_PWM) {
            *status = ADD_STATE(*status,ONLP_OID_STATUS_FLAG_FAILED);

        } else {
            *status = REMOVE_STATE(*status,ONLP_OID_STATUS_FLAG_FAILED);
        }
        break;
    case ONLP_FAN_PSU_1:
    case ONLP_FAN_PSU_2:
        psu_id = LOCAL_ID_TO_PSU_ID(id);
        ONLP_TRY(onlp_file_read_int(&rpm, "%srpm_psu%d", info_path, psu_id));
        ONLP_TRY(onlp_file_read_int(&pwm, "%spwm_psu%d", info_path, psu_id));
        if( rpm <= 0 || pwm <=0 || pwm > MAX_PWM) {
            uint32_t psu_state;
            rv= inv_psui_status_get(psu_id, &psu_state);
            if(rv!=ONLP_STATUS_OK) {
                return rv;
            }
            if(psu_state==ONLP_OID_STATUS_FLAG_PRESENT){
                *status =ADD_STATE(*status,ONLP_OID_STATUS_FLAG_FAILED);
            }else{
                *status =ADD_STATE(*status, ONLP_OID_STATUS_FLAG_UNPLUGGED);
            }
        } else {
            *status = REMOVE_STATE(*status,ONLP_OID_STATUS_FLAG_FAILED);
        }
        break;
    default:
        rv = ONLP_STATUS_E_INVALID;
        break;
    }
    return rv;
}

static int _fani_status_present_check(uint32_t* status,  onlp_fan_dir_t *dir, int id)
{
    int rv = ONLP_STATUS_OK;
    int gpi;
    int info_idx;
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX];
    char*  info_path=hwmon_path(INV_HWMON_BASE);

    if(id >= ONLP_FAN_1 && id <= ONLP_FAN_4) {
        info_idx = id - ONLP_FAN_1;
        ONLP_TRY( onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, "%sfan_gpi", info_path) );
    } else {
        rv = ONLP_STATUS_E_INVALID;
    }
    if( rv == ONLP_STATUS_OK ) {
        sscanf( buf, "0x%x\n", &gpi);
        /* B[0-3] installed(0)/uninstalled(1)
           B[4-7] FRtype(0)/RFtype(1) */
        bool uninstall= (gpi>>info_idx) & 1;
        if (!uninstall) {
            *status = ADD_STATE(*status,ONLP_OID_STATUS_FLAG_PRESENT) ;
            bool dir_B2F=(gpi>>(info_idx+4)) & 1;
            if (dir_B2F) {
                *dir = ONLP_FAN_DIR_B2F;
            } else {
                *dir = ONLP_FAN_DIR_F2B;
            }
        } else {
            *dir=ONLP_FAN_DIR_UNKNOWN;
            *status = 0;
        }
    }
    return rv;
}

/**
 * @brief Retrieve the fan's OID hdr.
 * @param id The fan OID.
 * @param rv [out] Receives the OID header.
 */
int onlp_fani_hdr_get(onlp_oid_id_t id, onlp_oid_hdr_t* hdr)
{
    int result = ONLP_STATUS_OK;
    onlp_fan_info_t* info;

    info = &__onlp_fan_info[id];
    *hdr = info->hdr;
    result = inv_fani_status_get(id, &(hdr->status));

    return result;
}


/*
 * This function sets the speed of the given fan in RPM.
 *
 * This function will only be called if the fan supprots the RPM_SET
 * capability.
 *
 * It is optional if you have no fans at all with this feature.
 */
int
onlp_fani_rpm_set(onlp_oid_id_t id, int rpm)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * This function sets the fan speed of the given OID as a percentage.
 *
 * This will only be called if the OID has the PERCENTAGE_SET
 * capability.
 *
 * It is optional if you have no fans at all with this feature.
 */
int
onlp_fani_percentage_set(onlp_oid_id_t id, int p)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}


/*
 * This function sets the fan direction of the given OID.
 *
 * This function is only relevant if the fan OID supports both direction
 * capabilities.
 *
 * This function is optional unless the functionality is available.
 */
int
onlp_fani_dir_set(onlp_oid_id_t id, onlp_fan_dir_t dir)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * Generic fan ioctl. Optional.
 */
int
onlp_fani_ioctl(onlp_oid_id_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

