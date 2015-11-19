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
#include "onlpie_log.h"


/*
 * This is the first function called by the ONLP framework.
 *
 * It should return the name of your platform driver.
 *
 * If the name of your platform driver is the same as the
 * current platform then this driver will be used.
 *
 * If the name of the driver is different from the current
 * platform, or the driver is capable of supporting multiple
 * platform variants, see onlp_sysi_platform_set() below.
 */
const char*
onlp_sysi_platform_get(void)
{
    return "onlp-example-platform-driver";
}

/*
 * This function will be called if onlp_sysi_platform_get()
 * returns a platform name that is not equal to the current platform.
 *
 * If you are compatible with the given platform then return ONLP_STATUS_OK.
 * If you can are not compatible return ONLP_STATUS_E_UNSUPPORTED.
 * - This is fatal and will abort platform initialization.
 */

int
onlp_sysi_platform_set(const char* name)
{
    /*
     * For the purposes of this example we
     * accept all platforms.
     */
    return ONLP_STATUS_OK;
}

/*
 * This is the first function the ONLP framework will call
 * after it has validated the the platform is supported using the mechanisms
 * described above.
 *
 * If this function does not return ONL_STATUS_OK
 * then platform initialization is aborted.
 */
int
onlp_sysi_init(void)
{
    AIM_LOG_MSG("%s", __func__);
    return ONLP_STATUS_OK;
}


/*
 * This function is called to return the physical base address
 * of the ONIE boot rom.
 *
 * The ONLP framework will mmap() and parse the ONIE TLV structure
 * from the given data.
 *
 * If you platform does not support a mappable address for the ONIE
 * eeprom then you should not provide this function at all.
 *
 * For the purposes of this example we will provide it but
 * return UNSUPPORTED (which is all the default implementation does).
 *
 */
int
onlp_sysi_onie_data_phys_addr_get(void** pa)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * If you cannot provide a base address you must provide the ONLP
 * framework the raw ONIE data through whatever means necessary.
 *
 * This function will be called as a backup in the event that
 * onlp_sysi_onie_data_phys_addr_get() fails.
 */
int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    /*
     * This represents the example ONIE data.
     */
    static uint8_t onie_data[] = {
        'T', 'l', 'v','I','n','f','o', 0,
        0x1, 0x0, 0x0,
        0x21, 0x8, 'O', 'N', 'L', 'P', 'I', 'E', 0, 0,
        0x22, 0x3, 'O', 'N', 'L',
        0xFE, 0x4, 0x4b, 0x1b, 0x1d, 0xde,
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

/*
 * IF the ONLP frame calles onlp_sysi_onie_data_get(),
 * if will call this function to free the data when it
 * is finished with it.
 *
 * This function is optional, and depends on the data
 * you return in onlp_sysi_onie_data_get().
 */
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
    /*
     *This function would normally implement things like the
     * the thermal planning.
     */
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* This example supports 2 Thermal sensors on the chassis */
    *e++ = ONLP_THERMAL_ID_CREATE(1);
    *e++ = ONLP_THERMAL_ID_CREATE(2);

    /* 3 PSUs */
    *e++ = ONLP_PSU_ID_CREATE(1);
    *e++ = ONLP_PSU_ID_CREATE(2);
    *e++ = ONLP_PSU_ID_CREATE(3);

    /* 2 Fans */
    *e++ = ONLP_FAN_ID_CREATE(1);
    *e++ = ONLP_FAN_ID_CREATE(2);

    /* 2 LEDs */
    *e++ = ONLP_LED_ID_CREATE(1);
    *e++ = ONLP_LED_ID_CREATE(2);

    return 0;
}

