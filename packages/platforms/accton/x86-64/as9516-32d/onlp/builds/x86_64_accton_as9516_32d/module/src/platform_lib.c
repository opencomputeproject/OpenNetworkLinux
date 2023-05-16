/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2017 Accton Technology Corporation.
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

int bmc_curl_init(void)
{
    int i;

    for(i = 0; i<HANDLECOUNT; i++)
    {
        curl[i] = curl_easy_init();
        if (curl[i])
        {
            curl_easy_setopt(curl[i], CURLOPT_USERPWD, "root:0penBmc");
            curl_easy_setopt(curl[i], CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl[i], CURLOPT_SSL_VERIFYHOST, 0L);

        }
        else
        {
            AIM_LOG_ERROR("Unable to init curl[%d]\r\n", i);
            return -1;
        }
    }
    /* init a multi stack */
    multi_curl = curl_multi_init();

    return 0;
}

int bmc_curl_deinit(void)
{
    int i;

    for(i = 0; i<HANDLECOUNT; i++)
    {
        curl_multi_remove_handle(multi_curl, curl[i]);
        curl_easy_cleanup(curl[i]);
    }

    curl_multi_cleanup(multi_curl);

    return 0;
}

int fpga_proc_i2c_read(int fpga_id, uint8_t bus, uint8_t mux_i2c_addr,
                            uint8_t mux_chn, uint8_t i2c_addr, int rd_size, uint8_t byte_buf[])
{
    int ret;

    if (mux_i2c_addr >= 0x80)
    {
        ret = bf_fpga_i2c_read(fpga_id, bus, 0, i2c_addr, byte_buf, rd_size);
    } 
    else
    {
        ret = bf_fpga_i2c_read_mux(fpga_id, bus, 0, mux_i2c_addr, mux_chn, i2c_addr, byte_buf, rd_size);
    }

    if (ret)
    {
        printf("Error: read failed for bus %hhd addr 0x%02x\n", bus, i2c_addr);

        return -1;
    }

    usleep(10);

    return 0;
}

int fpga_proc_i2c_write(int fpga_id, uint8_t bus, uint8_t mux_i2c_addr, 
                            uint8_t mux_chn, uint8_t i2c_addr, int wr_size, uint8_t byte_buf[]) 
{
    int ret;

    if (mux_i2c_addr >= 0x80)
    {
        ret = bf_fpga_i2c_write(fpga_id, bus, 0, i2c_addr, byte_buf, wr_size);
    } 
    else 
    {
        ret = bf_fpga_i2c_write_mux(fpga_id, bus, 0, mux_i2c_addr, mux_chn, i2c_addr
                                    , byte_buf, wr_size);
    }

    if (ret)
    {
        printf("Error: write failed for bus %hhd addr 0x%02x \n", bus, i2c_addr);

        return -1;
    }

    usleep(10);

    return 0;
}

int fpga_proc_i2c_addr_read(int fpga_id, uint8_t bus, uint8_t mux_i2c_addr, uint8_t mux_chn, 
                                uint8_t i2c_addr, int rd_size, int wr_size, uint8_t byte_buf[])
{
    int ret;

    if (mux_i2c_addr >= 0x80)
    {
        ret = bf_fpga_i2c_addr_read(fpga_id, bus, 0, i2c_addr, byte_buf, byte_buf
                                    , wr_size, rd_size);
    }
    else 
    {
        ret = bf_fpga_i2c_addr_read_mux(fpga_id, bus, 0, mux_i2c_addr, mux_chn
                                        , i2c_addr, byte_buf, byte_buf, wr_size
                                        , rd_size);
    }

    if (ret)
    {
        printf("Error: addr read failed for bus %hhd addr 0x%02x \n", bus, i2c_addr);

        return -1;
    }

    usleep(10);

    return 0;
}



int fpga_pltfm_init(int fpga_id) 
{
    if (bf_pltfm_fpga_init((void *)(uintptr_t)fpga_id)) 
    {
        printf("Error: Not able to initialize the fpga device\n");

        return 1;
    }

    return 0;
}

void fpga_pltfm_deinit(int fpga_id) 
{
    bf_pltfm_fpga_deinit((void *)(uintptr_t)fpga_id);
}
