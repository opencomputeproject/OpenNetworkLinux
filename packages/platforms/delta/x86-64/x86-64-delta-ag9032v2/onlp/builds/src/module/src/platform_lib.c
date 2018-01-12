/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017 Delta Networks, Inc.
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
 *
 *
 ***********************************************************/
#include "platform_lib.h"
#include <onlp/onlp.h>
#include <time.h>

int dni_get_bmc_data(char *device_name, UINT4 *num, UINT4 multiplier)
{
    FILE *fpRead;
    char Buf[ 10 ]={0};
    char ipmi_command[120] = {0};
    int lenth=10;
    float num_f;

    sprintf(ipmi_command, "ipmitool sdr get %s |grep 'Sensor Reading'| awk -F':' '{print $2}'| awk -F' ' '{ print $1}'",      device_name);
    fpRead = popen(ipmi_command, "r");

    if(fpRead == NULL){
        pclose(fpRead);
                return ONLP_STATUS_E_GENERIC;
    }
    fgets(Buf, lenth , fpRead);
    num_f = atof( Buf );
    *num = num_f * multiplier;
    pclose(fpRead);
    return ONLP_STATUS_OK;
}

int
dni_fanpresent_info_get(int *r_data)
{
    int rv = ONLP_STATUS_OK;
    char cmd[30] = {0};
    char str_data[100] = {0};
    FILE *fptr = NULL;

    sprintf(cmd, "ipmitool raw 0x38 0x0e");
    fptr = popen(cmd, "r");
    if(fptr != NULL)
    {
        if(fgets(str_data, sizeof(str_data), fptr) != NULL)
        {
            *r_data = strtol(str_data, NULL, 16);
        }
        else
        {
           rv = ONLP_STATUS_E_INVALID;
        }
        pclose(fptr);
    }
    else
    {
        pclose(fptr);
        rv = ONLP_STATUS_E_INVALID;
    }

    return rv;
}

int hex_to_int(char hex_input){
        int first = hex_input / 16 - 3;
        int second = hex_input % 16;
        int result = first*10 + second;
        if(result > 9) result--;
        return result;
}

int hex_to_ascii(char hex_high, char hex_low){
        int high = hex_to_int(hex_high) * 16;
        int low = hex_to_int(hex_low);
        return high+low;
}

int
dni_psui_eeprom_info_get(char *r_data, int psu_id, int psu_reg)
{
    int i             = 0;
    int rv            = ONLP_STATUS_OK;
    FILE *fptr        = NULL;
    char buf;
    char cmd[35]      = {0};
    char str_data[50] = {0};

    sprintf(cmd, "ipmitool raw 0x38 0x12 %d %d", psu_id, psu_reg);
    fptr = popen(cmd, "r");
  
    if(fptr != NULL)
    {
        while( (buf = fgetc(fptr)) != EOF) {
            if( buf != ' '){
                str_data[i] = buf;
                i++;
            }
        }
        if(i == 0){
            pclose(fptr);
            rv = ONLP_STATUS_E_INVALID;
        }
        else{
            /*  "str_data"  :psu model name or serial number in hex code
             *   str_data(hex) ex:0e 44 50 53 2d 38 30 30 41 42 2d 31 36 20 44
             */
            for(i = 1; i < PSU_NUM_LENGTH; i++)
            {
                r_data[i] = hex_to_ascii(str_data[2*i], str_data[2*i+1]);
            }
            pclose(fptr);            
        }
    }
    else
    {
        pclose(fptr);
        rv = ONLP_STATUS_E_INVALID;
    }
    return rv;
}