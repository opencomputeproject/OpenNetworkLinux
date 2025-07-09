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

#define THERMAL_PATH_FORMAT "/sys/switch/sensor/temp%d/temp_input"
#define THERMAL_PSU_FORMAT "/sys/switch/psu/psu%d/temp%d/temp_input"
 
 
/* Static values */
static onlp_thermal_info_t linfo[] =
{
    { }, /* Not used */
    /*
    { { ONLP_THERMAL_ID_CREATE(THERMAL_MAC_PIPE), "MAC_PIPE", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },   
    */
    { { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_0), "CPU_CORE_0", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_1), "CPU_CORE_1", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_2), "CPU_CORE_2", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_3), "CPU_CORE_3", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_PACKAGE), "CPU_PACKAGE", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_TMP75_48), "TMP75_48", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_TMP75_49), "TMP75_49", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_TMP75_4A), "TMP75_4A", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_TMP75_4B), "TMP75_4B", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_TMP431_4C), "TMP431_4C_1", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_2_TMP431_4C), "TMP431_4C_2", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_TMP75_4E), "TMP75_4E", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_TMP75_4D), "TMP75_4D", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_TMP275_4E), "TMP275_4E", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_VCORE_MAC), "VCORE_MAC", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_VDDT0V9_R_MAC), "VDDT0V9_R_MAC", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_VDD1V5_1V8_MAC), "VDD1V5_1V8_MAC", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_VCC3V3_QSFP_AB), "VCC3V3_QSFP_AB", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_VCC3V3_QSFP_CD), "VCC3V3_QSFP_CD", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }, 
    { { ONLP_THERMAL_ID_CREATE(THERMAL_PSU1_TEMP_1), "PSU1_TEMP_1", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },     
    { { ONLP_THERMAL_ID_CREATE(THERMAL_PSU1_TEMP_2), "PSU1_TEMP_2", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },  
    { { ONLP_THERMAL_ID_CREATE(THERMAL_PSU2_TEMP_1), "PSU2_TEMP_1", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },  
    { { ONLP_THERMAL_ID_CREATE(THERMAL_PSU2_TEMP_2), "PSU2_TEMP_2", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },              
};

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
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
    char  path[128] = {0};
    VALIDATE(id);
    
    tid = ONLP_OID_ID_GET(id);
    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[tid];
    if(tid<THERMAL_PSU1_TEMP_1){
        sprintf(path, THERMAL_PATH_FORMAT, tid+1);//remove THERMAL_MAC_PIPE,  
        if (0 > onlp_file_read_int(&info->mcelsius, path ))
                return ONLP_STATUS_E_INTERNAL;
    }
    else{
        switch(tid){
            case THERMAL_PSU1_TEMP_1:
                sprintf(path, THERMAL_PSU_FORMAT, 1,1);
                break;
            case THERMAL_PSU1_TEMP_2:
                sprintf(path, THERMAL_PSU_FORMAT, 1,2);
                break;
            case THERMAL_PSU2_TEMP_1:
                sprintf(path, THERMAL_PSU_FORMAT, 2,1);
                break;
            case THERMAL_PSU2_TEMP_2:
                sprintf(path, THERMAL_PSU_FORMAT, 2,2);
                break;  
            default:
                return ONLP_STATUS_E_INTERNAL;           
        }
        if (0 > onlp_file_read_int(&info->mcelsius, path ))
                return ONLP_STATUS_E_INTERNAL;
        
    }

    return ONLP_STATUS_OK;
}
