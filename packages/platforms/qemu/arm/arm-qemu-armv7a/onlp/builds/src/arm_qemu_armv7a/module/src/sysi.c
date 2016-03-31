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
 *
 ***********************************************************/
#include <onlp/platformi/sysi.h>
#include <onlplib/crc32.h>
#include "arm_qemu_armv7a_log.h"

const char*
onlp_sysi_platform_get(void)
{
    return "arm-qemu-armv7a-r0";
}

int
onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    static uint8_t onie_data[] = {
        'T', 'l', 'v','I','n','f','o', 0,
        0x1, 0x0, 0x0,
        0x21, 0x8, 'O', 'N', 'L', 'S', 'I', 'M', 0, 0,
        0x22, 0x3, 'O', 'N', 'L',
        0xFE, 0x4, 0x2f, 0x52, 0x8f, 0xda,
    };

    if(onie_data[9] == 0 && onie_data[10] == 0) {
        int len = sizeof(onie_data);
        len -= 11;
        onie_data[9] = (len & 0xFF00) >> 8;
        onie_data[10] = (len & 0xFF);
    }

    *data = onie_data;
    return 0;
}

void
onlp_sysi_onie_data_free(uint8_t* data)
{
    /*
     * We returned a static array in onlp_sysi_onie_data_get()
     * so no free operation is required.
     */
}

void
onlp_sysi_platform_manage(void)
{
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    memset(table, 0, max*sizeof(onlp_oid_t));
    return 0;
}
