/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
 ***********************************************************/
#include <onlp/platformi/sfpi.h>
#include <x86_64_delta_wb2448/x86_64_delta_wb2448_config.h>
#include "x86_64_delta_wb2448_log.h"

#define SFP_PORT_BASE	49

static int sfp_count__ = X86_64_DELTA_WB2448_CONFIG_SFP_COUNT;

int
onlp_sfpi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;
    
    for(p = 0; p < sfp_count__; p++) 
    {
        AIM_BITMAP_SET(bmap, p + SFP_PORT_BASE);
    }
    
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    return 0;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    AIM_BITMAP_CLR_ALL(dst);
    return ONLP_STATUS_OK;
}

int 
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    return ONLP_STATUS_OK;
}

/*
 * This function reads the SFPs idrom and returns in
 * in the data buffer provided.
 */
int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    return ONLP_STATUS_E_MISSING;
}

int 
onlp_sfpi_ioctl(int port, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

