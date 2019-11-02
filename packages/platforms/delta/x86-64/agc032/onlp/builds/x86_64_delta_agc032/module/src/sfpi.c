/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2019 Delta Electronics, Inc.
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
#include "vendor_driver_pool.h"
#include "vendor_i2c_device_list.h"
/*
    We assume ONLP user will check the present status before calling sfpi api.
 */

int
onlp_sfpi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    //AIM_LOG_ERROR("Function: %s \n", __FUNCTION__);

    uint8_t index = 0;

    AIM_BITMAP_CLR_ALL(bmap);

    for(index = 0; index < sfp_list_size; index++)
    {
        if(vendor_find_copper_by_name(sfp_dev_list[index].dev_name) == 0)
            AIM_BITMAP_SET(bmap, index);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    //AIM_LOG_ERROR("Function: %s, instance: %d \n", __FUNCTION__, ONLP_OID_ID_GET(port));

    int rv = 0;
    int present = 0;

    rv = vendor_get_present_status(&sfp_present_list[port], &present);
    if(rv < 0) return ONLP_STATUS_E_INTERNAL;

    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    //AIM_LOG_ERROR("Function: %s \n", __FUNCTION__);

    int rv = 0, index = 0;

    for(index = 0; index < sfp_list_size; index++)
    {
        if(vendor_find_copper_by_name(sfp_dev_list[index].dev_name) == 0)
        {
            rv = onlp_sfpi_is_present(index);
            if(rv < 0) return ONLP_STATUS_E_INTERNAL;

            AIM_BITMAP_MOD(dst, index, rv);
        }
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_read(int port, uint8_t devaddr, uint8_t addr,
                   uint8_t* rdata, int size)
{
    //AIM_LOG_ERROR("Function: %s, instance: %d \n", __FUNCTION__, ONLP_OID_ID_GET(port));

    int id = port, fail = 0;
    void *busDrv = (void *)vendor_find_driver_by_name(sfp_dev_list[id].bus_drv_name);
    sfp_dev_driver_t *sfp =
        (sfp_dev_driver_t *)vendor_find_driver_by_name(sfp_dev_list[id].dev_drv_name);

    /*
    rv = onlp_sfpi_is_present(port);

    if(rv < 0)
    {
        AIM_LOG_ERROR("onlp_sfpi_dev_read: sfp is unpresent.");
        return ONLP_STATUS_E_INTERNAL;
    }
    */

    if(size != 256)
    {
        AIM_LOG_ERROR("onlp_sfpi_dev_read: unsupport size, the size must be 256.");
        return ONLP_STATUS_E_INTERNAL;
    }

    vendor_dev_do_oc(sfp_o_list[id]);
    if(sfp->eeprom_load(
        busDrv,
        sfp_dev_list[id].bus,
        sfp_dev_list[id].addr,
        rdata) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("sfp->eeprom_load failed.");
        fail = 1;
    }
    vendor_dev_do_oc(sfp_c_list[id]);

    if(fail == 1) return ONLP_STATUS_E_INTERNAL;

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    //AIM_LOG_ERROR("Function: %s, instance: %d \n", __FUNCTION__, ONLP_OID_ID_GET(port));

    int id = port, fail = 0;;
    uint8_t data = 0;
    void *busDrv = (void *)vendor_find_driver_by_name(sfp_dev_list[id].bus_drv_name);
    sfp_dev_driver_t *sfp =
        (sfp_dev_driver_t *)vendor_find_driver_by_name(sfp_dev_list[id].dev_drv_name);

    vendor_dev_do_oc(sfp_o_list[id]);
    if(sfp->eeprom_readb(
        busDrv,
        sfp_dev_list[id].bus,
        sfp_dev_list[id].addr,
        addr,
        &data) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("sfp->eeprom_readb failed.");
        fail = 1;
    }
    vendor_dev_do_oc(sfp_c_list[id]);

    if(fail == 1) return ONLP_STATUS_E_INTERNAL;

    return data;
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    //AIM_LOG_ERROR("Function: %s, instance: %d \n", __FUNCTION__, ONLP_OID_ID_GET(port));

    int id = port, fail = 0;
    void *busDrv = (void *)vendor_find_driver_by_name(sfp_dev_list[id].bus_drv_name);
    sfp_dev_driver_t *sfp =
        (sfp_dev_driver_t *)vendor_find_driver_by_name(sfp_dev_list[id].dev_drv_name);

    vendor_dev_do_oc(sfp_o_list[id]);
    if(sfp->eeprom_writeb(
        busDrv,
        sfp_dev_list[id].bus,
        sfp_dev_list[id].addr,
        addr,
        value) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("sfp->eeprom_writeb failed.");
        fail = 1;
    }
    vendor_dev_do_oc(sfp_c_list[id]);

    if(fail == 1) return ONLP_STATUS_E_INTERNAL;

    return ONLP_STATUS_OK;
}

int onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv)
{
    //AIM_LOG_ERROR("Function: %s, instance: %d \n", __FUNCTION__, ONLP_OID_ID_GET(port));

    int id = port;
    sfp_dev_driver_t *sfp =
        (sfp_dev_driver_t *)vendor_find_driver_by_name(sfp_dev_list[id].dev_drv_name);

    if(sfp->control_is_support(control, (uint8_t *)rv) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("sfp->control_is_support failed.");
    }

    return ONLP_STATUS_OK;
}

int onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int *value)
{
    //AIM_LOG_ERROR("Function: %s, instance: %d, control: %d\n", __FUNCTION__, ONLP_OID_ID_GET(port), control);

    int rv = 0, id = port, cpld_idx = 0, fail = 0;
    uint8_t curr_data = 0;
    int *eeprom_data = calloc(1, sizeof(int));

    void *busDrv = (void *)vendor_find_driver_by_name(sfp_dev_list[id].bus_drv_name);
    cpld_dev_driver_t *cpld =
        (cpld_dev_driver_t *)vendor_find_driver_by_name("CPLD");
    sfp_dev_driver_t *sfp =
        (sfp_dev_driver_t *)vendor_find_driver_by_name(sfp_dev_list[id].dev_drv_name);

    switch (control)
    {
        case ONLP_SFP_CONTROL_RESET:
        case ONLP_SFP_CONTROL_RESET_STATE:
            if(sfp_reset_list[id].type == 0)
            {
                /*NOT SUPPORT */
                return ONLP_STATUS_OK;
            }
            cpld_idx = vendor_find_cpld_idx(sfp_reset_list[id].addr);
            if(cpld_idx < 0) return ONLP_STATUS_E_INTERNAL;

            vendor_dev_do_oc(cpld_o_list[cpld_idx]);
            rv = cpld->readb(
                busDrv,
                sfp_reset_list[id].bus,
                sfp_reset_list[id].addr,
                sfp_reset_list[id].offset,
                &curr_data);
            vendor_dev_do_oc(cpld_c_list[cpld_idx]);

            if(rv < 0) return ONLP_STATUS_E_INTERNAL;

            *value = ((curr_data & sfp_reset_list[id].mask) == sfp_reset_list[id].match) ? 1 : 0;
            return 0;

        case ONLP_SFP_CONTROL_LP_MODE:
            if(sfp_lpmode_list[id].type == 0)
            {
                /*NOT SUPPORT */
                return ONLP_STATUS_OK;
            }
            cpld_idx = vendor_find_cpld_idx(sfp_lpmode_list[id].addr);
            if(cpld_idx < 0) return ONLP_STATUS_E_INTERNAL;

            vendor_dev_do_oc(cpld_o_list[cpld_idx]);
            rv = cpld->readb(
                busDrv,
                sfp_lpmode_list[id].bus,
                sfp_lpmode_list[id].addr,
                sfp_lpmode_list[id].offset,
                &curr_data);
            vendor_dev_do_oc(cpld_c_list[cpld_idx]);

            if(rv < 0) return ONLP_STATUS_E_INTERNAL;

            *value = ((curr_data & sfp_lpmode_list[id].mask) == sfp_lpmode_list[id].match) ? 1 : 0;
            return 0;

        case ONLP_SFP_CONTROL_RX_LOS:
            free(eeprom_data);
            eeprom_data = calloc(4, sizeof(int));
        case ONLP_SFP_CONTROL_TX_FAULT:
            free(eeprom_data);
            eeprom_data = calloc(4, sizeof(int));
        case ONLP_SFP_CONTROL_TX_DISABLE:
        case ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL:
        case ONLP_SFP_CONTROL_POWER_OVERRIDE:
            vendor_dev_do_oc(sfp_o_list[id]);
            if(sfp->control_get(
                busDrv,
                sfp_dev_list[id].bus,
                sfp_dev_list[id].addr,
                control,
                eeprom_data) != ONLP_STATUS_OK)
            {
                AIM_LOG_ERROR("sfp->onlp_sfpi_control_get failed.");
                fail = 1;
            }
            vendor_dev_do_oc(sfp_c_list[id]);

            *value = eeprom_data[0];
            free(eeprom_data);

            break;
        default:
            AIM_LOG_ERROR("Unsupported control.");
            return ONLP_STATUS_E_INTERNAL;
    }

    if(fail == 1) return ONLP_STATUS_E_INTERNAL;

    return ONLP_STATUS_OK;
}


int onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    //AIM_LOG_ERROR("Function: %s, instance: %d \n", __FUNCTION__, ONLP_OID_ID_GET(port));

    int rv = 0, id = port, cpld_idx = 0, fail = 0;
    uint8_t curr_data = 0;
    void *busDrv = (void *)vendor_find_driver_by_name(sfp_dev_list[id].bus_drv_name);
    cpld_dev_driver_t *cpld =
        (cpld_dev_driver_t *)vendor_find_driver_by_name("CPLD");
    sfp_dev_driver_t *sfp =
        (sfp_dev_driver_t *)vendor_find_driver_by_name(sfp_dev_list[id].dev_drv_name);

    switch (control)
    {
        case ONLP_SFP_CONTROL_RESET:
        case ONLP_SFP_CONTROL_RESET_STATE:
            if(sfp_reset_list[id].type == 0)
            {
                /*NOT SUPPORT */
                return ONLP_STATUS_OK;
            }
            cpld_idx = vendor_find_cpld_idx(sfp_reset_list[id].addr);
            if(cpld_idx < 0) return ONLP_STATUS_E_INTERNAL;

            vendor_dev_do_oc(cpld_o_list[cpld_idx]);
            rv = cpld->readb(
                busDrv,
                sfp_reset_list[id].bus,
                sfp_reset_list[id].addr,
                sfp_reset_list[id].offset,
                &curr_data);

            curr_data &= ~sfp_reset_list[id].mask;
            if (value == 1 )
                curr_data |= (sfp_reset_list[id].match & sfp_reset_list[id].mask);
            else
                curr_data |= (~sfp_reset_list[id].match & sfp_reset_list[id].mask);

            curr_data &= 0xff;

            rv = cpld->writeb(
                busDrv,
                sfp_reset_list[id].bus,
                sfp_reset_list[id].addr,
                sfp_reset_list[id].offset,
                curr_data);
            vendor_dev_do_oc(cpld_c_list[cpld_idx]);

            if(rv < 0) return ONLP_STATUS_E_INTERNAL;

            return 0;

        case ONLP_SFP_CONTROL_LP_MODE:
            if(sfp_lpmode_list[id].type == 0)
            {
                /*NOT SUPPORT */
                return ONLP_STATUS_OK;
            }
            cpld_idx = vendor_find_cpld_idx(sfp_lpmode_list[id].addr);
            if(cpld_idx < 0) return ONLP_STATUS_E_INTERNAL;

            vendor_dev_do_oc(cpld_o_list[cpld_idx]);
            rv = cpld->readb(
                busDrv,
                sfp_lpmode_list[id].bus,
                sfp_lpmode_list[id].addr,
                sfp_lpmode_list[id].offset,
                &curr_data);

            curr_data &= ~sfp_lpmode_list[id].mask;
            if (value == 1 )
                curr_data |= (sfp_lpmode_list[id].match & sfp_lpmode_list[id].mask);
            else
                curr_data |= (~sfp_lpmode_list[id].match & sfp_lpmode_list[id].mask);

            curr_data &= 0xff;

            rv = cpld->writeb(
                busDrv,
                sfp_reset_list[id].bus,
                sfp_reset_list[id].addr,
                sfp_reset_list[id].offset,
                curr_data);
            vendor_dev_do_oc(cpld_c_list[cpld_idx]);

            if(rv < 0) return ONLP_STATUS_E_INTERNAL;

            return 0;

        case ONLP_SFP_CONTROL_TX_DISABLE:
        case ONLP_SFP_CONTROL_TX_DISABLE_CHANNEL:
        case ONLP_SFP_CONTROL_POWER_OVERRIDE:
            vendor_dev_do_oc(sfp_o_list[id]);
            if(sfp->control_set(
                busDrv,
                sfp_dev_list[id].bus,
                sfp_dev_list[id].addr,
                control,
                value) != ONLP_STATUS_OK)
            {
                AIM_LOG_ERROR("sfp->onlp_sfpi_control_set failed.");
                fail = 1;
            }
            vendor_dev_do_oc(sfp_c_list[id]);

            break;
        default:
            AIM_LOG_ERROR("Unsupported control.");
            return ONLP_STATUS_E_INTERNAL;
    }

    if(fail == 1) return ONLP_STATUS_E_INTERNAL;

    return ONLP_STATUS_OK;
}

int onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    //AIM_LOG_ERROR("Function: %s\n", __FUNCTION__);

    uint8_t index = 0;
    int rv = 0, rx_los = 0;
    int present = 0;

    for(index = 0; index < sfp_list_size; index++)
    {
        if(vendor_find_copper_by_name(sfp_dev_list[index].dev_name) == 0)
        {
            present = onlp_sfpi_is_present(index);
            if(present){
                rv = onlp_sfpi_control_get(index, ONLP_SFP_CONTROL_RX_LOS, &rx_los);
                if(rv < 0) return ONLP_STATUS_E_INTERNAL;
                AIM_BITMAP_MOD(dst, index, rx_los);
            }
        }
    }

    return ONLP_STATUS_OK;
}


int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_post_insert(int port, sff_info_t* info)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}


int
onlp_sfpi_ioctl(int port, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    //AIM_LOG_ERROR("Function: %s, instance: %d \n", __FUNCTION__, ONLP_OID_ID_GET(port));

    int id = port, fail = 0;
    void *busDrv = (void *)vendor_find_driver_by_name(sfp_dev_list[id].bus_drv_name);
    sfp_dev_driver_t *sfp =
        (sfp_dev_driver_t *)vendor_find_driver_by_name(sfp_dev_list[id].dev_drv_name);

    /*
    rv = onlp_sfpi_is_present(port);

    if(rv < 0)
    {
        AIM_LOG_ERROR("onlp_sfpi_dev_readb: sfp is unpresent.");
        return ONLP_STATUS_E_INTERNAL;
    }
    */

    vendor_dev_do_oc(sfp_o_list[id]);
    if(sfp->eeprom_load(
        busDrv,
        sfp_dev_list[id].bus,
        sfp_dev_list[id].addr,
        data) != ONLP_STATUS_OK)
    {
        AIM_LOG_ERROR("sfp->eeprom_load failed.");
        fail = 1;
    }
    vendor_dev_do_oc(sfp_c_list[id]);

    if(fail == 1) return ONLP_STATUS_E_INTERNAL;

    return ONLP_STATUS_OK;

}


