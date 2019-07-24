/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
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
#include <onlp/platformi/sfpi.h>

#include <fcntl.h> /* For O_RDWR && open */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "platform_lib.h"

static int
as6700_32x_sfp_node_read_int(char *node_path, int *value, int data_len)
{
    int ret = 0;
    char buf[8] = {0};
    *value = 0;

    memset(buf, '\0', sizeof(buf));

    ret = deviceNodeReadString(node_path, buf, sizeof(buf), data_len);

    if (ret == 0) {
        *value = atoi(buf);
    }

    return ret;
}

static int set_active_port(int front_port)
{
    return deviceNodeWriteInt(SFP_EEPROM_NODE(sfp_active_port), front_port, 0);
}

static int set_equalizer_type(int type)
{
    return deviceNodeWriteInt(SFP_EEPROM_NODE(sfp_equalizer), type, 0);
}

/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
int
onlp_sfpi_init(void)
{
    /* Called at initialization time */
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    /*
     * Ports {0, CHASSIS_SFP_COUNT}
     */
    int p;

    for(p = 0; p < CHASSIS_SFP_COUNT; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    /*
     * Return 1 if present.
     * Return 0 if not present.
     * Return < 0 if error.
     */
    int present = 0, front_port=0;

    front_port=SFP_MAP_API_2_FRONT_PORT(port);
    if (set_active_port(front_port) < 0) {
        AIM_LOG_ERROR("Unable to set active port to port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (as6700_32x_sfp_node_read_int(SFP_EEPROM_NODE(sfp_is_present), &present, 1) != 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t bytes[4];
    FILE* fp = fopen(SFP_EEPROM_NODE(sfp_is_present_all), "r");
    
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the sfp_is_present_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }
    int count = fscanf(fp, "%x %x %x %x",
                       bytes+0,
                       bytes+1,
                       bytes+2,
                       bytes+3
                       );
    fclose(fp);
    if(count != AIM_ARRAYSIZE(bytes)) {
        /* Likely a CPLD read timeout. */
        AIM_LOG_ERROR("Unable to read all fields from the sfp_is_present_all device file.");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Convert to 64 bit integer in port order */
    int i = 0;
    uint32_t presence_all = 0 ;
    for(i = AIM_ARRAYSIZE(bytes)-1; i >= 0; i--) {
        presence_all <<= 8;
        presence_all |= bytes[i];
    }

    /* Populate bitmap */
    for(i = 0; presence_all; i++) {
        AIM_BITMAP_MOD(dst, i, (presence_all & 1));
        presence_all >>= 1;
    }

    return ONLP_STATUS_OK;
}


int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
    int front_port=0;

    memset(data, 0, 256);

    front_port=SFP_MAP_API_2_FRONT_PORT(port);
    if (set_active_port(front_port) < 0) {
        AIM_LOG_ERROR("Unable to set active port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (deviceNodeReadBinary(SFP_EEPROM_NODE(sfp_eeprom), (char*)data, 256, 256) != 0) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_post_insert(int port, sff_info_t* info)
{
    /*
     * IF any platform-programming must be performed based
     * on the actual SFP that was inserted (for example, custom equalizer settings)
     * then it should be performed by this function.
     *
     * If not custom programming must be performed, then do nothing.
     * This function is optional.
     */
    int front_port=0;
    sfp_tranceiver_cable_type_t cable_type = SFP_TRANCEIVER_CABLE_TYPE_FIBER;

    switch(info->media_type)
        {
        case SFF_MEDIA_TYPE_FIBER:
            cable_type = SFP_TRANCEIVER_CABLE_TYPE_FIBER;
            break;

        case SFF_MEDIA_TYPE_COPPER:
            switch(info->length)
                {
                case 1:
                case 2:
                    AIM_LOG_MSG("Port %d EQ 1M", port);
                    cable_type = SFP_TRANCEIVER_CABLE_TYPE_COPPER_1M;
                    break;
                case 3:
                case 4:
                    AIM_LOG_MSG("Port %d EQ 3M", port);
                    cable_type = SFP_TRANCEIVER_CABLE_TYPE_COPPER_3M; break;
                case 5:
                case 6:
                    AIM_LOG_MSG("Port %d EQ 5M", port);
                    cable_type = SFP_TRANCEIVER_CABLE_TYPE_COPPER_5M; break;
                case 7:
                    AIM_LOG_MSG("Port %d EQ 7M", port);
                    cable_type = SFP_TRANCEIVER_CABLE_TYPE_COPPER_7M; break;
                default:
                    AIM_LOG_MSG("Port %d EQ 7M (MAX)", port);
                    /* Nothing beyond 7M is supported. Best we can do is use the 7M settings. */
                    cable_type = SFP_TRANCEIVER_CABLE_TYPE_COPPER_7M; break;
    }
            break;

        default:
            AIM_LOG_WARN("port(%d) media_type(%d) length(%d) is not supported\r\n",
                         port, info->media_type, info->length);
            return ONLP_STATUS_E_INTERNAL;
        }


    front_port=SFP_MAP_API_2_FRONT_PORT(port);
    if (set_active_port(front_port) < 0) {
        AIM_LOG_ERROR("Unable to set active port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (set_equalizer_type(cable_type) < 0) {
        AIM_LOG_ERROR("Unable to set port(%d) media_type(%d) length(%d)\r\n",
                    port, info->media_type, info->length);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

