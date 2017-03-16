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
 *
 ***********************************************************/
#include <onlp/platformi/sfpi.h>
#include "onlpie_log.h"

/*
 * If your platform does not support any SFP interfaces
 * then none of these functions need to be provided.
 */

/*
 * This function will be called prior to all other onlp_sfpi_* functions.
 */
int
onlp_sfpi_init(void)
{
    AIM_LOG_MSG("%s", __func__);
    return ONLP_STATUS_OK;
}

/*
 * This function should populate the give bitmap with
 * all valid, SFP-capable port numbers.
 *
 * Only port numbers in this bitmap will be queried by the the
 * ONLP framework.
 *
 * No SFPI functions will be called with ports that are
 * not in this bitmap. You can ignore all error checking
 * on the incoming ports defined in this interface.
 */
int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    /* We simulate 4 SFP ports -- 17, 18, 19, and 20 */
    AIM_BITMAP_SET(bmap, 17);
    AIM_BITMAP_SET(bmap, 18);
    AIM_BITMAP_SET(bmap, 19);
    AIM_BITMAP_SET(bmap, 20);
    return ONLP_STATUS_OK;
}

/*
 * This function should return whether an SFP is inserted on the given
 * port.
 *
 * Returns 1 if the SFP is present.
 * Returns 0 if the SFP is not present.
 * Returns ONLP_E_* if there was an error determining the status.
 */
int
onlp_sfpi_is_present(int port)
{
    /* In this example only ports 17 and 19 have SFPs inserted */
    if(port == 17 || port == 19) {
        return 1;
    }
    return 0;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    AIM_BITMAP_CLR_ALL(dst);
    AIM_BITMAP_SET(dst, 17);
    AIM_BITMAP_SET(dst, 19);
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    AIM_BITMAP_CLR_ALL(dst);
    AIM_BITMAP_SET(dst, 19);
    return ONLP_STATUS_OK;
}

/*
 * This function reads the SFPs idrom and returns in
 * in the data buffer provided.
 */
int
onlp_sfpi_eeprom_read(int port, int dev_addr, uint8_t data[256])
{
    if(port == 17 || port == 19) {
        /* These ports have SFPs inserted in this example */
        return ONLP_STATUS_OK;
    }
    else {
        /* These ports have no SFPs inserted. */
        return ONLP_STATUS_E_MISSING;
    }
}

/*
 * Manually enable or disable the given SFP.
 *
 */
int
onlp_sfpi_enable_set(int port, int enable)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * Returns whether the SFP is currently enabled or disabled.
 */
int
onlp_sfpi_enable_get(int port, int* enable)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}


/*
 * If the platform requires any setup or equalizer modifications
 * based on the actual SFP that was inserted then that custom
 * setup should be performed here.
 *
 * After a new SFP is detected by the ONLP framework this
 * function will be called to perform the (optional) setup.
 */
int
onlp_sfpi_post_insert(int port, sff_info_t* sff_info)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * Return the current status of the SFP.
 * See onlp_sfp_status_t;
 */
int
onlp_sfpi_status_get(int port, uint32_t* status)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * De-initialize the SFPI subsystem.
 */
int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}


/*
 * This function should display human-readable debug status
 * of the given SFP.
 *
 * This function will be called for the purposes of system
 * debug and technical support logs.
 *
 * You can provide any information here that might be useful in
 * debugging functional system or link problems.
 *
 * Likely candidates for this output are the current status of
 * any equalizers, preemphasis settings, etc that are platform
 * specific and affect the performance of the SFP links but are otherwise
 * opaque and unknown to the rest of the system.
 *
 */
void
onlp_sfpi_debug(int port, aim_pvs_t* pvs)
{
    aim_printf(pvs, "Debug data for port %d goes here.", port);
}

/*
 * This is a generic ioctl interface.
 */
int
onlp_sfpi_ioctl(int port, va_list vargs)
{
    AIM_LOG_MSG("No ioctls supported.");
    return ONLP_STATUS_E_UNSUPPORTED;
}

