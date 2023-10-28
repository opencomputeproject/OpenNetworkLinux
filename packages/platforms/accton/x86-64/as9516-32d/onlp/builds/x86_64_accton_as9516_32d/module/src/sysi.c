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
 *
 *
 ***********************************************************/
#include <unistd.h>
#include <fcntl.h>

#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "platform_lib.h"
#include "x86_64_accton_as9516_32d_int.h"
#include "x86_64_accton_as9516_32d_log.h"

typedef struct cpld_version
{
    char *attr_name;
    int   major_version;
    int   sub_version;
    char *description;
} cpld_version_t;

static int sys_ver = 0;
static int sys_subver = 0;
static int fan_ver = 0;

static void cpld_ver_call_back(void *p)
{

    int char_val[2];
    int i;
    const char *ptr = (const char *)p;

    if (ptr == NULL)
    {
        AIM_LOG_ERROR("NULL POINTER PASSED to call back function\n");

        return;
    }

    /* get the two chars of return string and convert to integer */
    for(i = 0 ; i < 2 ; i++)
    {
            /* ASCII convert*/
            if (ptr[50+i] == 97)
                char_val[i] = 10;
            else if(ptr[50+i] == 98)
                char_val[i] = 11; 
            else if (ptr[50+i] == 99)
                char_val[i] = 12;
            else if(ptr[50+i] == 100)
                char_val[i] = 13; 
            else if (ptr[50+i] == 101)
                char_val[i] = 14;
            else if(ptr[50+i] == 102)
                char_val[i] = 15; 
            else
                char_val[i] = (int)(ptr[50+i]) - 48;
    }
    sys_ver =  (int)(char_val[0]) * 16 + (int)(char_val[1]);

    return;
}

static void cpld_subver_call_back(void *p)
{

    int char_val[2];
    int i;
    const char *ptr = (const char *)p;

    if (ptr == NULL)
    {
        AIM_LOG_ERROR("NULL POINTER PASSED to call back function\n");

        return;
    }

    /* get the two chars of return string and convert to integer */
    for(i = 0 ; i < 2 ; i++)
    {
            /* ASCII convert*/
            if (ptr[50+i] == 97)
                char_val[i] = 10;
            else if(ptr[50+i] == 98)
                char_val[i] = 11; 
            else if (ptr[50+i] == 99)
                char_val[i] = 12;
            else if(ptr[50+i] == 100)
                char_val[i] = 13; 
            else if (ptr[50+i] == 101)
                char_val[i] = 14;
            else if(ptr[50+i] == 102)
                char_val[i] = 15; 
            else
                char_val[i] = (int)(ptr[50+i]) - 48;
    }
    sys_subver =  (int)(char_val[0]) * 16 + (int)(char_val[1]);

    return;
}

static void fan_cpld_ver_call_back(void *p)
{

    int char_val[2];
    int i;
    const char *ptr = (const char *)p;

    if (ptr == NULL)
    {
        AIM_LOG_ERROR("NULL POINTER PASSED to call back function\n");

        return;
    }

    /* get the two chars of return string and convert to integer */
    for(i = 0 ; i < 2 ; i++)
    {
            /* ASCII convert*/
            if (ptr[50+i] == 97)
                char_val[i] = 10;
            else if(ptr[50+i] == 98)
                char_val[i] = 11; 
            else if (ptr[50+i] == 99)
                char_val[i] = 12;
            else if(ptr[50+i] == 100)
                char_val[i] = 13; 
            else if (ptr[50+i] == 101)
                char_val[i] = 14;
            else if(ptr[50+i] == 102)
                char_val[i] = 15; 
            else
                char_val[i] = (int)(ptr[50+i]) - 48;
    }
    fan_ver =  (int)(char_val[0]) * 16 + (int)(char_val[1]);

    return;
}

const char*
onlp_sysi_platform_get(void)
{
    if (fpga_pltfm_init(0) != 0)
    {
        return NULL;
    }

    return "x86-64-accton-as9516-32d-r0";
}

int
onlp_sysi_init(void)
{

    bmc_curl_init();

    return ONLP_STATUS_OK;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);
    int fail_cnt, rd_size, wr_size, i;
    uint8_t pca9548_chan;
    uint8_t byte_buf[128];
    uint8_t fpga_id;
    uint8_t bus, i2c_addr, mux_i2c_addr, mux_chn;

    fail_cnt=0;
    /* 1. need to back up. read pca9548 0x74 chan */
    fpga_id=0;
    bus=32;
    mux_i2c_addr=0x88;
    mux_chn=0;
    i2c_addr=0x74;
    rd_size=1;

    if((fpga_proc_i2c_read(fpga_id, bus, mux_i2c_addr, mux_chn, i2c_addr, rd_size, byte_buf)) != 0)
    {
        fail_cnt++;

        goto exit1;
    }
    else
    {
        pca9548_chan=byte_buf[0];
    }
    /* 2. change pca9548 0x74 to chan 6 */
    mux_i2c_addr=0x88;
    mux_chn=0;
    i2c_addr = 0x74;
    wr_size = 1;
    byte_buf[0]=0x40;
    if((fpga_proc_i2c_write(fpga_id, bus, mux_i2c_addr, mux_chn, i2c_addr, wr_size, byte_buf)) != 0)
    {
        fail_cnt++;

        goto exit1;
    }
    /* 3. set offset and get data from eeprom */
    bus=32;
    mux_i2c_addr=0x88;
    mux_chn=0;
    i2c_addr =0x51;
    wr_size =2;
    byte_buf[0]=0x0;
    byte_buf[1]=0x0;
    if((fpga_proc_i2c_write(fpga_id, bus, mux_i2c_addr, mux_chn, i2c_addr, wr_size, byte_buf)) != 0)
    {
        fail_cnt++;

        goto exit2;
    }

    bus=32;
    mux_i2c_addr=0x88;
    mux_chn=0;
    i2c_addr = 0x51;
    rd_size = 1;
    for(i=0; i<256; i++)
    {
        if((fpga_proc_i2c_read(fpga_id, bus, mux_i2c_addr, mux_chn, i2c_addr, rd_size, byte_buf)) != 0)
        {
            fail_cnt++;

            goto exit2; 
        }

        *(rdata+i)=byte_buf[0];
    }

exit2:
    /* restore the default chan */
    fpga_id=0;
    bus=32;
    mux_i2c_addr=0x88;
    mux_chn=0;
    i2c_addr =0x74;
    wr_size =1;
    byte_buf[0]=pca9548_chan;
    if((fpga_proc_i2c_write(fpga_id, bus, mux_i2c_addr, mux_chn, i2c_addr, wr_size, byte_buf)) != 0)
    {
        fail_cnt++;
        goto exit1;
    }

exit1:

    if(fail_cnt==0)
    {
        *size=256;
        *data = rdata;
        return ONLP_STATUS_OK;
    }

    aim_free(rdata);
    *size = 0;

    return ONLP_STATUS_E_INTERNAL;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 7 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++)
    {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 2 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++)
    {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++)
    {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 6 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
    {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int curlid;
    char url[256] = {0};
    CURLMcode mc;
    int still_running = 1;

    cpld_version_t cplds[] = { { "sys_cpld_ver", 0, 0, "SYSTEM CPLD"},
                               { "fan_cpld_ver", 0, 0, "FAN CPLD"} };
    
    snprintf(url, sizeof(url),"%s""0x31/0x1", BMC_CPLD_CURL_PREFIX);
    curlid = CURL_SYS_CPLD_VER;
    curl_easy_setopt(curl[curlid], CURLOPT_URL, url);
    curl_easy_setopt(curl[curlid], CURLOPT_WRITEFUNCTION, cpld_ver_call_back);
    curl_multi_add_handle(multi_curl, curl[curlid]);

    snprintf(url, sizeof(url),"%s""0x31/0x2", BMC_CPLD_CURL_PREFIX);
    curlid = CURL_SYS_CPLD_SUBVER;
    curl_easy_setopt(curl[curlid], CURLOPT_URL, url);
    curl_easy_setopt(curl[curlid], CURLOPT_WRITEFUNCTION, cpld_subver_call_back);
    curl_multi_add_handle(multi_curl, curl[curlid]);

    snprintf(url, sizeof(url),"%s""0x08/0x66/0x01", BMC_FAN_CPLD_CURL_PREFIX);
    curlid = CURL_FAN_CPLD;
    curl_easy_setopt(curl[curlid], CURLOPT_URL, url);
    curl_easy_setopt(curl[curlid], CURLOPT_WRITEFUNCTION, fan_cpld_ver_call_back);
    curl_multi_add_handle(multi_curl, curl[curlid]);

    while(still_running) {
         mc = curl_multi_perform(multi_curl, &still_running);
        if(mc != CURLM_OK)
        {
            AIM_LOG_ERROR("multi_curl failed, code %d.\n", mc);
        }
    }

    /* Read CPLD version */
    cplds[0].major_version=sys_ver;
    cplds[0].sub_version=sys_subver;
    cplds[1].major_version=fan_ver;

    pi->cpld_versions = aim_fstrdup("%s:%x.%x, %s:0x%x", 
                                    cplds[0].description, cplds[0].major_version, cplds[0].sub_version,
                                    cplds[1].description, cplds[1].major_version);

    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

int
onlp_sysi_platform_manage_fans(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
