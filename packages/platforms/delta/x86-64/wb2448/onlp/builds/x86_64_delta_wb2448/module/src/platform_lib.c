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

/****************************************************************************
 * FUNCTION    : ifnOS_LINUX_BmcI2CGet
 * DESCRIPTION : To read data through BMC I2C
 * INPUT       : u1Bus
 *               u1Dev
 *               u4Addr
 *               u1AddrLen
 *               u1DataLen
 * OUTPUT      : pu4RetData    
 * RETURN      : ONLP_STATUS_OK
 ****************************************************************************/
INT4 ifnOS_LINUX_BmcI2CGet(UINT1 u1Bus, UINT1 u1Dev, UINT4 u4Addr, UINT1 u1AddrLen, UINT4 *pu4RetData, UINT1 u1DataLen)
{
    INT4  i      = 0;
    INT4  i4Cnt  = 1;
    INT4  i4Ret  = ONLP_STATUS_OK;
    UINT4 u4Data = 0;
    FILE  *pFd   = NULL;
    UINT1 au1Temp[2]               = {0};
    INT1  ai1Cmd [OS_MAX_MSG_SIZE] = {0};
    INT1  ai1Data[OS_MAX_MSG_SIZE] = {0};
    
    sprintf(ai1Cmd, "ipmitool raw 0x38 0x2 %d %d %d %d", u1Bus, u1Dev, u4Addr, u1DataLen);
    
    pFd = popen(ai1Cmd, "r");
    
    if(pFd != NULL)
    {
        if (fgets(ai1Data, OS_MAX_MSG_SIZE, pFd) != NULL)
        {
            while (i < u1DataLen)
            {
                au1Temp[0] = ai1Data[i4Cnt++];
                au1Temp[1] = ai1Data[i4Cnt++];
        
                i4Cnt++;
        
                u4Data <<= (i*8);
                u4Data += (UINT1)MMI_XTOI(au1Temp);
                
                i++;
            }
        
            *pu4RetData = u4Data;
        }
        else
        {
            i4Ret = ONLP_STATUS_E_INTERNAL;
            AIM_LOG_ERROR("Command \"%s\": Get Data Failed (ret: %d)", ai1Cmd, i4Ret);
        }
        
        pclose(pFd);
    }
    else
    {
        i4Ret = ONLP_STATUS_E_INTERNAL;
        AIM_LOG_ERROR("Execute command \"%s\" failed (ret: %d)", ai1Cmd, i4Ret);
    }
        
    return i4Ret;
}

/****************************************************************************
 * FUNCTION    : ifnOS_LINUX_BmcI2CSet
 * DESCRIPTION : To write data through BMC I2C
 * INPUT       : u1Bus
 *               u1Dev
 *               u4Addr
 *               u1AddrLen
 *               u4Data
 *               u1DataLen
 * OUTPUT      : N/A    
 * RETURN      : ONLP_STATUS_OK
 ****************************************************************************/
INT4 ifnOS_LINUX_BmcI2CSet(UINT1 u1Bus, UINT1 u1Dev, UINT4 u4Addr, UINT1 u1AddrLen, UINT4 u4Data, UINT1 u1DataLen)
{
    INT4  i4Ret = ONLP_STATUS_OK;
    FILE  *pFd  = NULL;
    INT1  ai1Cmd[OS_MAX_MSG_SIZE] = {0};
        
    switch (u1DataLen)
    {
        case 1:
            sprintf(ai1Cmd, "ipmitool raw 0x38 0x3 %d %d %d %d", u1Bus, u1Dev, u4Addr, u4Data);
            break;
        case 2:
            sprintf(ai1Cmd, "ipmitool raw 0x38 0x4 %d %d %d %d %d", u1Bus, u1Dev, u4Addr, ((u4Data&0xFF00)>>8), (u4Data&0xFF));
            break;
        case 4: 
            sprintf(ai1Cmd, "ipmitool raw 0x38 0x5 %d %d %d %d %d %d %d", u1Bus, u1Dev, u4Addr, ((u4Data&0xFF000000)>>24), ((u4Data&0xFF0000)>>16), ((u4Data&0xFF00)>>8), (u4Data&0xFF));
            break;
        default:
            AIM_LOG_ERROR("ERR: Unsupported data length: %d", u1DataLen);
    }
    
    pFd = popen(ai1Cmd, "r");
    
    if (pFd != NULL)
    {
        pclose(pFd);
    }
    else
    {
        i4Ret = ONLP_STATUS_E_INTERNAL;
        AIM_LOG_ERROR("Execute command \"%s\" failed (ret: %d)", ai1Cmd, i4Ret);
    }
    
    return i4Ret;
}

/****************************************************************************
 * FUNCTION    : ifnOS_LINUX_BmcI2CProbe
 * DESCRIPTION : To probe device through BMC I2C
 * INPUT       : u1Bus
 *               u1Dev
 * OUTPUT      :     
 * RETURN      : ONLP_STATUS_OK
 ****************************************************************************/
INT4 ifnOS_LINUX_BmcI2CProbe(UINT1 u1Bus, UINT1 u1Dev)
{
    INT4  i4Ret  = ONLP_STATUS_OK;
    INT4  i4Cnt  = 1; 
    UINT4 u4Data = 0;
    UINT1 au1Temp[2]               = {0};
    INT1  ai1Cmd [OS_MAX_MSG_SIZE] = {0};
    INT1  ai1Data[OS_MAX_MSG_SIZE] = {0};
    FILE  *pFd = NULL;
    
    sprintf(ai1Cmd, "ipmitool raw 0x38 0x1 %d %d", u1Bus, u1Dev);
    
    pFd = popen(ai1Cmd, "r");
    
    if(pFd != NULL)
    {
        if (fgets(ai1Data, OS_MAX_MSG_SIZE, pFd) != NULL)
        {        
            au1Temp[0] = ai1Data[i4Cnt++];
            au1Temp[1] = ai1Data[i4Cnt++];
    
            u4Data += (UINT1)MMI_XTOI(au1Temp);
            
            if (u4Data != 0x00)
                AIM_LOG_ERROR("Probe failed (ret: %d)", i4Ret);
        }
        else
        {
            i4Ret = ONLP_STATUS_E_INTERNAL;     
            AIM_LOG_ERROR("Command \"%s\": Get Data Failed (ret: %d)", ai1Cmd, i4Ret);
        }
        
        pclose(pFd);
    }
    else
    {
        i4Ret = ONLP_STATUS_E_INTERNAL;
        AIM_LOG_ERROR("Execute command \"%s\" failed (ret: %d)", ai1Cmd, i4Ret);
    }
    
    return i4Ret;
}

/****************************************************************************
 * FUNCTION    : ifnBmcFanSpeedGet
 * DESCRIPTION : To Get Fan Speed from Ipmitool
 * INPUT       : pi1FanName
 *               pu4RetData
 * OUTPUT      :     
 * RETURN      : ONLP_STATUS_OK
 ****************************************************************************/
INT4 ifnBmcFanSpeedGet(INT1 *pi1FanName, UINT4 *pu4RetData)
{
    INT4  i4Ret  = ONLP_STATUS_OK;
    UINT4 u4Data = 0;
    INT1  ai1Cmd [OS_MAX_MSG_SIZE] = {0};
    INT1  au1Data[OS_MAX_MSG_SIZE] = {0};
    INT1  *pui1Temp = NULL;
    FILE  *pFd      = NULL;
    
    sprintf(ai1Cmd, "ipmitool sdr | grep %s", pi1FanName);
    
    pFd = popen(ai1Cmd, "r");
    
    if(pFd != NULL)
    {
        if (fgets(au1Data, OS_MAX_MSG_SIZE, pFd) != NULL)
        {
            pui1Temp = strchr(au1Data, '|');
        
            do 
            {
                u4Data += strtol(pui1Temp, &pui1Temp, 10); 
            }while (*pui1Temp++);
            
            *pu4RetData = u4Data;
        }
        else
        {
            i4Ret = ONLP_STATUS_E_INTERNAL;
            AIM_LOG_ERROR("Command \"%s\": Get Data Failed (ret: %d)", ai1Cmd, i4Ret);
        }
        
        pclose(pFd);
    }
    else
    {
        i4Ret = ONLP_STATUS_E_INTERNAL;
        AIM_LOG_ERROR("Execute command \"%s\" failed (ret: %d)", ai1Cmd, i4Ret);
    }
        
    return i4Ret;
}

/****************************************************************************
 * FUNCTION    : ifnBmcFanSpeedSet
 * DESCRIPTION : To Set Fan Speed from Ipmitool
 * INPUT       : u4FanNumber
 *               u4Percentage
 * OUTPUT      :     
 * RETURN      : ONLP_STATUS_OK
 ****************************************************************************/
INT4 ifnBmcFanSpeedSet(UINT4 u4FanNumber, UINT4 u4Percentage)
{
    INT1  ai1Cmd[OS_MAX_MSG_SIZE] = {0};
    INT4  i4Ret = ONLP_STATUS_OK;
    FILE  *pFd  = NULL;
    
    sprintf(ai1Cmd, "ipmitool raw 0x38 0xB %d %d", u4FanNumber, u4Percentage);
    
    pFd = popen(ai1Cmd, "r");
    
    if (pFd != NULL)
    {
        pclose(pFd);
    }
    else
    {
        i4Ret = ONLP_STATUS_E_INTERNAL;
        AIM_LOG_ERROR("Execute command \"%s\" failed (ret: %d)", ai1Cmd, i4Ret);
    }
    
    return i4Ret;
}

UINT4 MMI_XTOI (const UINT1* str)
{
    INT4  digit = 0;
    UINT4 x     = 0;
    
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
