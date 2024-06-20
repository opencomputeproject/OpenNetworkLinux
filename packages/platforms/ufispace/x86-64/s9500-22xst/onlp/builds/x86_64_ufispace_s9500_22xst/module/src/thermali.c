/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
#include <onlp/platformi/thermali.h>
#include <onlplib/file.h>
#include "x86_64_ufispace_s9500_22xst_log.h"
#include "platform_lib.h"
      
static onlp_thermal_info_t thermal_info[] = {    
    { }, /* Not used */
    { { THERMAL_OID_CPU, "Temp_CPU", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {92000, 97000, 102000}
    },    
    { { THERMAL_OID_MAC, "Temp_MAC", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {96000, 101000, 106000}
    },
    { { THERMAL_OID_BMC, "Temp_BMC", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {80000, 85000, 89000}
    },
    { { THERMAL_OID_100G_CAGE, "Temp_100GCage", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {79000, 82000, 85000}
    },
    { { THERMAL_OID_DDR4, "Temp_DDR4", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {85000, 90000, 92000}
    },
    { { THERMAL_OID_FANCARD1, "Temp_FANCARD1", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {80000, 85000, 89000}
    },
    { { THERMAL_OID_FANCARD2, "Temp_FANCARD2", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {80000, 85000, 89000}
    },
    { { THERMAL_OID_PSU0, "PSU 1 - Thermal Sensor", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {86000, 90000, 95000}
    },
    { { THERMAL_OID_PSU1, "PSU 2 - Thermal Sensor", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {86000, 90000, 95000}
    },
    { { THERMAL_OID_CPU_PKG, "CPU Package", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU1, "CPU Thermal 1", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU2, "CPU Thermal 2", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU3, "CPU Thermal 3", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU4, "CPU Thermal 4", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU_BOARD, "CPU Board", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_AMB, "Ambient Thermal", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {76000, 80000, 84000}
    },
    { { THERMAL_OID_PHY1, "Temp_PHY1", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {86000, 90000, 95000}
    },
    { { THERMAL_OID_HEATER, "Temp_Heater", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {73000, 75000, 78000}
    },
};

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{   
    lock_init();
    return ONLP_STATUS_OK;
}

static int
cpu_thermal_info_get(onlp_thermal_info_t* info, int id)
{
    int rv;
    
    rv = onlp_file_read_int(&info->mcelsius,
                            SYS_CPU_CORETEMP_PREFIX "temp%d_input", (id - THERMAL_ID_CPU_PKG) + 1);    
    
    if(rv != ONLP_STATUS_OK) {

        rv = onlp_file_read_int(&info->mcelsius,
                            SYS_CPU_CORETEMP_PREFIX2 "temp%d_input", (id - THERMAL_ID_CPU_PKG) + 1); 

        if(rv != ONLP_STATUS_OK) {
            return rv;
        }
    }

    if(rv == ONLP_STATUS_E_MISSING) {
        info->status &= ~1;
        return 0;
    }
    
    return ONLP_STATUS_OK;
}

int 
psu_thermal_info_get(onlp_thermal_info_t* info, int id)
{
    int rv;

    if ( bmc_enable ) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
	
    rv = psu_thermal_get(info, id);
    if(rv == ONLP_STATUS_E_INTERNAL) {
        return rv;
    }
    
    return ONLP_STATUS_OK;
}

static int
cpu_board_thermal_info_get(onlp_thermal_info_t* info)
{
    int rv;
    
    rv = onlp_file_read_int(&info->mcelsius,
                      SYS_CPU_BOARD_TEMP_PREFIX "temp1_input");    
	
    if (rv != ONLP_STATUS_OK) {
        rv = onlp_file_read_int(&info->mcelsius,
                      SYS_CPU_BOARD_TEMP_PREFIX2 "temp1_input");
        
        if (rv != ONLP_STATUS_OK) {
            return rv;
        }
    }

    if (rv == ONLP_STATUS_E_MISSING) {
        info->status &= ~1;
        return 0;
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
    int sensor_id, rc;
    sensor_id = ONLP_OID_ID_GET(id);
    
    *info = thermal_info[sensor_id];
    info->caps |= ONLP_THERMAL_CAPS_GET_TEMPERATURE;

    switch (sensor_id) {        
        case THERMAL_ID_CPU_PKG:
        case THERMAL_ID_CPU1:
        case THERMAL_ID_CPU2:
        case THERMAL_ID_CPU3:
        case THERMAL_ID_CPU4:          
            rc = cpu_thermal_info_get(info, sensor_id);
            break;        
        case THERMAL_ID_CPU_BOARD:
            rc = cpu_board_thermal_info_get(info);
            break;
        case THERMAL_ID_CPU:
        case THERMAL_ID_MAC:
        case THERMAL_ID_BMC:
        case THERMAL_ID_100G_CAGE:
        case THERMAL_ID_DDR4:        
        case THERMAL_ID_FANCARD1:
        case THERMAL_ID_FANCARD2:
        case THERMAL_ID_PSU0:    
        case THERMAL_ID_PSU1:
        case THERMAL_ID_AMB:  
        case THERMAL_ID_PHY1:  
        case THERMAL_ID_HEATER:     
            rc = bmc_thermal_info_get(info, sensor_id);
            break;    
        default:            
            return ONLP_STATUS_E_INTERNAL;
            break;
    }

    return rc;
}
