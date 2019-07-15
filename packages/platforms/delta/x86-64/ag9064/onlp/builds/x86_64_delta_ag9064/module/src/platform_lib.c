/************************************************************
 * <bsn.cl fy=2017 v=onl>
 *
 *        Copyright 2017 Delta Networks, Inc.
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
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <AIM/aim.h>
#include <onlp/onlp.h>

#include "platform_lib.h"


int ifnOS_LINUX_BmcI2CGet(uint8_t bus, uint8_t dev, uint32_t reg, uint32_t *rdata, uint8_t datalen)
{
    int rv     = ONLP_STATUS_OK;
    int dIndex = 1;
    char *pch  = NULL;
    FILE *pFd  = NULL;
    int tmp_data[OS_MAX_MSG_SIZE]   = {0};
    char ipmi_cmd[OS_MAX_MSG_SIZE]  = {0};
    char cmd_rdata[OS_MAX_MSG_SIZE] = {0};
        
    sprintf(ipmi_cmd, "ipmitool raw 0x38 0x2 %d %d %d %d", bus, dev, reg, datalen);
    
    pFd = popen(ipmi_cmd, "r");
    
    if(pFd != NULL)
    {
        if (fgets(cmd_rdata, OS_MAX_MSG_SIZE, pFd) != NULL)
        {            
            memset(tmp_data, 0x0, sizeof(tmp_data));
            
            for(dIndex = 1; dIndex <= datalen; dIndex++)
            {
                if(dIndex == 1)
                {
                    pch = strtok(cmd_rdata," ");
                }
                else
                {
                    pch = strtok(NULL," ");
                }
                
                if(!pch)
                {            
                    AIM_LOG_ERROR("Command \"%s\": Extract Data Failed (ret: %d)", ipmi_cmd, ONLP_STATUS_E_INTERNAL);
                    return ONLP_STATUS_E_INTERNAL;
                }                    
                else
                {   /* cut newline char */
                    if( pch[strlen(pch)-1] == '\n') 
                    {
                        pch[strlen(pch)-1] = 0x0;
                    }
                }

                tmp_data[dIndex] = xtoi(pch);
            }
        
            switch (datalen) 
            {
                case 1:
                    *rdata = tmp_data[1];
                    break;
                case 2:
                    *rdata = (tmp_data[1] << 8) | (tmp_data[2]);
                    break;
                default:
                    if( (datalen > 2) && (datalen < 64) ) 
                    {
                        for(dIndex = 1; dIndex <= datalen; dIndex++) 
                        { 
                            rdata[dIndex - 1] = tmp_data[dIndex];
                        }
                    }
                    else 
                    {
                        rv = ONLP_STATUS_E_INTERNAL;
                        AIM_LOG_ERROR("Command \"%s\": data length out of range (ret: %d)", ipmi_cmd, rv);
                    }
            }
        }
        else
        {
            rv = ONLP_STATUS_E_INTERNAL;
            AIM_LOG_ERROR("Command \"%s\": Get Data Failed (ret: %d)", ipmi_cmd, rv);
        }
        
        pclose(pFd);
    }
    else
    {
        rv = ONLP_STATUS_E_INTERNAL;
        AIM_LOG_ERROR("Execute command \"%s\" failed (ret: %d)", ipmi_cmd, rv);
    }
    
    return rv;
}

int ifnOS_LINUX_BmcI2CSet(uint8_t bus, uint8_t dev, uint32_t reg, uint32_t u4Data, uint8_t datalen)
{
    int rv    = ONLP_STATUS_OK;
    FILE *pFd = NULL;
    char ipmi_cmd[OS_MAX_MSG_SIZE] = {0};
        
    switch (datalen)
    {
        case 1:
            sprintf(ipmi_cmd, "ipmitool raw 0x38 0x3 %d %d %d %d", bus, dev, reg, u4Data);
            break;
        case 2:
            sprintf(ipmi_cmd, "ipmitool raw 0x38 0x4 %d %d %d %d %d", bus, dev, reg, 
                                               ((u4Data & 0xFF00) >> 8), (u4Data & 0xFF));
            break;
        case 4: 
            sprintf(ipmi_cmd, "ipmitool raw 0x38 0x5 %d %d %d %d %d %d %d", bus, dev, reg, 
            ((u4Data & 0xFF000000) >> 24), ((u4Data & 0xFF0000) >> 16), ((u4Data & 0xFF00) >> 8), (u4Data & 0xFF));
            break;
        default:
            AIM_LOG_ERROR("ERR: Unsupported data length: %d", datalen);
    }
    
    pFd = popen(ipmi_cmd, "r");
    
    if (pFd != NULL)
    {
        pclose(pFd);
    }
    else
    {
        rv = ONLP_STATUS_E_INTERNAL;
        AIM_LOG_ERROR("Execute command \"%s\" failed (ret: %d)", ipmi_cmd, rv);
    }
    
    return rv;
}

int ifnOS_LINUX_BmcGetDataByName(char *devname, uint32_t *rdata)
{
    int rv           = ONLP_STATUS_OK;
    char *temp_data  = NULL;
    FILE *pFd        = NULL;
    uint32_t devdata = 0;
    char ipmi_cmd [OS_MAX_MSG_SIZE] = {0};
    char cmd_rdata[OS_MAX_MSG_SIZE] = {0};
        
    sprintf(ipmi_cmd, "ipmitool sdr get %s | grep 'Sensor Reading'", devname);
    
    pFd = popen(ipmi_cmd, "r");
    
    if(pFd != NULL)
    {
        if (fgets(cmd_rdata, OS_MAX_MSG_SIZE, pFd) != NULL)
        {
            temp_data = strchr(cmd_rdata, ':');
            temp_data = strtok(temp_data, "(");
            
            do 
            {
                devdata += strtol(temp_data, &temp_data, 10); 
            }while (*temp_data++);
            
            *rdata = devdata;
        }
        else
        {
            rv = ONLP_STATUS_E_INTERNAL;
            AIM_LOG_ERROR("Command \"%s\": Get Data Failed (ret: %d)", ipmi_cmd, rv);
        }
        
        pclose(pFd);
    }
    else
    {
        rv = ONLP_STATUS_E_INTERNAL;
        AIM_LOG_ERROR("Execute command \"%s\" failed (ret: %d)", ipmi_cmd, rv);
    }
        
    return rv;
}

uint32_t xtoi(const char* str)
{
    int  digit = 0;
    uint32_t x = 0;
    
    if ((*str == '0') && (*(str+1) == 'x')) str += 2;
    
    while (*str) 
    {
        if ((*str >= '0') && (*str <= '9')) 
        {
            digit = *str - '0';
        }
        else if ((*str >= 'A') && (*str <= 'F')) 
        {
            digit = 10 + *str - 'A';
        }
        else if ((*str >= 'a') && (*str <= 'f')) 
        {
            digit = 10 + *str - 'a';
        }
        else 
        {
            break;
        }
        
        x *= 16;
        x += digit;
        str++;
    }
    
    return x;
}
