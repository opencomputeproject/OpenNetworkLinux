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
#include <onlp/sfp.h>
#include <onlp/platformi/sfpi.h>
#include "onlp_log.h"
#include "onlp_locks.h"
#include <onlp/oids.h>
#include "onlp_int.h"
#include <IOF/iof.h>

/**
 * All port numbers will be validated before calling the SFP driver.
 */
static onlp_sfp_bitmap_t sfpi_bitmap__;

static int sfp_oid_validate__(onlp_oid_t* oid, int* pid)
{
    if(oid == NULL) {
        return ONLP_STATUS_E_PARAM;
    }

    int port;

    if(ONLP_OID_IS_SFP(*oid)) {
        /** OID Formatted SFP Port */
        port = ONLP_OID_ID_GET(*oid);
        /** Ports start from 0, OIDS start from 1 */
        port--;
    }
    else if(ONLP_OID_TYPE_GET(*oid) == 0) {
        /** Raw port number */
        port = *oid;
        *oid = ONLP_SFP_ID_CREATE(port+1);
    }
    else {
        return ONLP_STATUS_E_PARAM;
    }

    if(AIM_BITMAP_GET(&sfpi_bitmap__, port) == 0) {
        /** Not a valid port id */
        return ONLP_STATUS_E_PARAM;
    }

    int rport;
    if(ONLP_SUCCESS(onlp_sfpi_port_map(port, &rport))) {
        port = rport;
    }

    if(pid) {
        *pid = port;
    }
    return 0;
}

#define ONLP_SFP_PORT_VALIDATE_AND_MAP(_oid, _port)                     \
    do {                                                                \
        ONLP_IF_ERROR_RETURN(sfp_oid_validate__(_oid, _port));          \
    } while(0)

void
onlp_sfp_bitmap_t_init(onlp_sfp_bitmap_t* bmap)
{
    AIM_BITMAP_INIT(bmap, 255);
    AIM_BITMAP_CLR_ALL(bmap);
}

static int
onlp_sfp_sw_init_locked__(void)
{
    onlp_sfp_bitmap_t_init(&sfpi_bitmap__);

    int rv = onlp_sfpi_sw_init();
    if(rv < 0) {
        if(rv == ONLP_STATUS_E_UNSUPPORTED) {
            /*
             * There are no SFPs on this platform.
             * Not necessarily an error condition.
             */
        }
        else {
            AIM_LOG_ERROR("Error initializing the SFPI driver: %{onlp_status}", rv);
        }
        return rv;
    }
    else {
        /* SFPI initialized. Get the bitmap */
        rv = onlp_sfpi_bitmap_get(&sfpi_bitmap__);
        if(rv < 0) {
            AIM_LOG_ERROR("onlp_sfpi_bitmap_get(): %{onlp_status}", rv);
            return rv;
        }
        return ONLP_STATUS_OK;
    }
}
ONLP_LOCKED_API0(onlp_sfp_sw_init)

static int
onlp_sfp_hw_init_locked__(uint32_t flags)
{
    return onlp_sfpi_hw_init(flags);
}
ONLP_LOCKED_API1(onlp_sfp_hw_init, uint32_t, flags);

static int
onlp_sfp_bitmap_get_locked__(onlp_sfp_bitmap_t* bmap)
{
    AIM_BITMAP_ASSIGN(bmap, &sfpi_bitmap__);
    return ONLP_STATUS_OK;
}
ONLP_LOCKED_API1(onlp_sfp_bitmap_get, onlp_sfp_bitmap_t*, bmap);


static int
onlp_sfp_sw_denit_locked__(void)
{
    return onlp_sfpi_sw_denit();
}
ONLP_LOCKED_API0(onlp_sfp_sw_denit);


static int
onlp_sfp_is_present_locked__(onlp_oid_t oid)
{
    int port;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, &port);
    return onlp_sfpi_is_present(port);
}
ONLP_LOCKED_API1(onlp_sfp_is_present, onlp_oid_t, port);

static int
onlp_sfp_type_get_locked__(onlp_oid_t oid, onlp_sfp_type_t* rtype)
{
    int port;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, &port);
    return onlp_sfpi_type_get(port, rtype);
}
ONLP_LOCKED_API2(onlp_sfp_type_get, onlp_oid_t, oid, onlp_sfp_type_t*, rtype);

static int
onlp_sfp_presence_bitmap_get_locked__(onlp_sfp_bitmap_t* dst)
{
    ONLP_PTR_VALIDATE_ZERO(dst);
    onlp_sfp_bitmap_t_init(dst);

    int rv = onlp_sfpi_presence_bitmap_get(dst);

    if(rv == ONLP_STATUS_E_UNSUPPORTED) {
        /* Generate from single-port API */
        int p;
        AIM_BITMAP_CLR_ALL(dst);
        AIM_BITMAP_ITER(&sfpi_bitmap__, p) {
            rv = onlp_sfp_is_present_locked__(p);
            if(rv < 0) {
                return rv;
            }
            if(rv > 0) {
                AIM_BITMAP_SET(dst, p);
            }
        }
        return 0;
    }

    return rv;
}
ONLP_LOCKED_API1(onlp_sfp_presence_bitmap_get, onlp_sfp_bitmap_t*, dst);

int
onlp_sfp_port_valid(onlp_oid_t oid)
{
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, NULL);
    return 0;
}

static int
onlp_sfp_post_insert_locked__(onlp_oid_t oid, sff_info_t* info)
{
    int port;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, &port);
    return onlp_sfpi_post_insert(port, info);
}
ONLP_LOCKED_API2(onlp_sfp_post_insert, onlp_oid_t, port, sff_info_t*, info);

static int
onlp_sfp_control_set_locked__(onlp_oid_t oid, onlp_sfp_control_t control, int value)
{
    int port;
    int supported;

    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, &port);

    if(!ONLP_SFP_CONTROL_VALID(control)) {
        return ONLP_STATUS_E_PARAM;
    }

    /* Does the platform advertise support for this control? */
    if( (onlp_sfpi_control_supported(port, control, &supported) >= 0) &&
        !supported) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch(control)
        {

        case ONLP_SFP_CONTROL_RX_LOS:
        case ONLP_SFP_CONTROL_TX_FAULT:
            /** These are read-only. */
            return ONLP_STATUS_E_PARAM;

        default:
            break;
        }
    return onlp_sfpi_control_set(port, control, value);
}
ONLP_LOCKED_API3(onlp_sfp_control_set, onlp_oid_t, port, onlp_sfp_control_t, control,
                 int, value);

static int
onlp_sfp_control_get_locked__(onlp_oid_t oid, onlp_sfp_control_t control, int* value)
{
    int port;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, &port);

    if(!ONLP_SFP_CONTROL_VALID(control)) {
        return ONLP_STATUS_E_PARAM;
    }

    /* Does the platform advertise support for this control? */
    int supported;
    if( (onlp_sfpi_control_supported(port, control, &supported) >= 0) &&
        !supported) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch(control)
        {
        case ONLP_SFP_CONTROL_RESET:
            /* This is a write-only control. */
            return ONLP_STATUS_E_UNSUPPORTED;

        default:
            break;
        }

    return (value) ? onlp_sfpi_control_get(port, control, value) : ONLP_STATUS_E_PARAM;
}
ONLP_LOCKED_API3(onlp_sfp_control_get, onlp_oid_t, port, onlp_sfp_control_t, control,
                 int*, value);



static int
onlp_sfp_rx_los_bitmap_get_locked__(onlp_sfp_bitmap_t* dst)
{
    ONLP_PTR_VALIDATE_ZERO(dst);
    onlp_sfp_bitmap_t_init(dst);

    int rv = onlp_sfpi_rx_los_bitmap_get(dst);

    if(rv == ONLP_STATUS_E_UNSUPPORTED) {
        /* Generate from control API */
        int p;
        AIM_BITMAP_CLR_ALL(dst);
        AIM_BITMAP_ITER(&sfpi_bitmap__, p) {
            int v;
            rv = onlp_sfp_control_get_locked__(p, ONLP_SFP_CONTROL_RX_LOS, &v);
            if(rv < 0) {
                return rv;
            }
            if(v) {
                AIM_BITMAP_SET(dst, p);
            }
        }
    }

    return rv;
}
ONLP_LOCKED_API1(onlp_sfp_rx_los_bitmap_get, onlp_sfp_bitmap_t*, dst);


int
onlp_sfp_control_flags_get(onlp_oid_t oid, uint32_t* flags)
{
    int port;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, &port);

    /**
     * These are the control bits queried and returned.
     */
    onlp_sfp_control_t controls[] =
        {
            ONLP_SFP_CONTROL_RESET_STATE,
            ONLP_SFP_CONTROL_RX_LOS,
            ONLP_SFP_CONTROL_TX_DISABLE,
            ONLP_SFP_CONTROL_LP_MODE
        };

    if(flags) {
        *flags = 0;
    }
    else {
        return ONLP_STATUS_E_PARAM;
    }

    int rv, i, v;

    for(i = 0; i < AIM_ARRAYSIZE(controls); i++) {
        rv = onlp_sfp_control_get(port, controls[i], &v);
        if(rv >= 0) {
            if(v) {
                *flags |= (1 << controls[i]);
            }
        }
        else {
            if(rv != ONLP_STATUS_E_UNSUPPORTED) {
                return rv;
            }
        }
    }
    return 0;
}

int
onlp_sfp_dev_alloc_read(onlp_oid_t port,
                        int devaddr, int addr, int count,
                        uint8_t** rvp)
{
    int rv;
    *rvp = aim_zmalloc(count);

    rv = onlp_sfp_dev_read(port, devaddr, addr, *rvp, count);
    if(ONLP_FAILURE(rv)) {
        aim_free(*rvp);
        *rvp = NULL;
    }
    return rv;
}



int
onlp_sfp_dev_read_locked__(onlp_oid_t oid, int devaddr, int addr,
                           uint8_t* dst, int len)
{
    int port;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, &port);
    return onlp_sfpi_dev_read(port, devaddr, addr, dst, len);
}
ONLP_LOCKED_API5(onlp_sfp_dev_read, onlp_oid_t, port, int, devaddr,
                 int, addr, uint8_t*, dst, int, len);

int
onlp_sfp_dev_write_locked__(onlp_oid_t oid, int devaddr, int addr,
                            uint8_t* src, int len)
{
    int port;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, &port);
    return onlp_sfpi_dev_write(port, devaddr, addr, src, len);
}
ONLP_LOCKED_API5(onlp_sfp_dev_write, onlp_oid_t, port, int, devaddr,
                 int, addr, uint8_t*, src, int, len);

int
onlp_sfp_dev_readb_locked__(onlp_oid_t oid, int devaddr, int addr)
{
    int port;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, &port);
    return onlp_sfpi_dev_readb(port, devaddr, addr);
}
ONLP_LOCKED_API3(onlp_sfp_dev_readb, onlp_oid_t, port, int, devaddr, int, addr);

int
onlp_sfp_dev_writeb_locked__(onlp_oid_t oid, int devaddr, int addr,
                             uint8_t value)
{
    int port;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, &port);
    return onlp_sfpi_dev_writeb(port, devaddr, addr, value);
}
ONLP_LOCKED_API4(onlp_sfp_dev_writeb, onlp_oid_t, port, int, devaddr, int, addr, uint8_t, value);

int
onlp_sfp_dev_readw_locked__(onlp_oid_t oid, int devaddr, int addr)
{
    int port;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, &port);
    return onlp_sfpi_dev_readw(port, devaddr, addr);
}
ONLP_LOCKED_API3(onlp_sfp_dev_readw, onlp_oid_t, port, int, devaddr, int, addr);

int
onlp_sfp_dev_writew_locked__(onlp_oid_t oid, int devaddr, int addr, uint16_t value)
{
    int port;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, &port);
    return onlp_sfpi_dev_writew(port, devaddr, addr, value);
}
ONLP_LOCKED_API4(onlp_sfp_dev_writew, onlp_oid_t, port, int, devaddr, int, addr, uint16_t, value);

static char*
sfp_control_str__(uint32_t controls)
{
    char tmp[16] = { 0 };
    char* cp = tmp;

    if(controls & ONLP_SFP_CONTROL_FLAG_RX_LOS) {
        *cp++ = 'R';
    }
    if(controls & ONLP_SFP_CONTROL_FLAG_TX_FAULT) {
        *cp++ = 'T';
    }
    if(controls & ONLP_SFP_CONTROL_FLAG_TX_DISABLE) {
        *cp++ = 'X';
    }
    if(controls & ONLP_SFP_CONTROL_FLAG_LP_MODE) {
        *cp++ = 'L';
    }
    if(controls & ONLP_SFP_CONTROL_RESET_STATE) {
        *cp++ = '~';
    }
    return aim_strdup(tmp);
}

static int
sfp_inventory_show_iter__(onlp_oid_t oid, void* cookie)
{
    int rv = 0;
    onlp_sfp_info_t info;
    char* fields[10] = { 0 };

    fields[0] = aim_dfstrdup("%d", ONLP_OID_ID_GET(oid));

    rv = onlp_sfp_info_get(oid, &info);

    /*
     * These fields get populated regardless of the
     * success or failure of the call to onlp_sfp_info_get().
     */
    if(ONLP_SFP_TYPE_VALID(info.type)) {
        fields[1] = aim_dfstrdup("%{onlp_sfp_type}", info.type);
    }
    else {
        fields[1] = aim_strdup("");
    }

    if(ONLP_SUCCESS(rv)) {

        fields[4] = sfp_control_str__(info.controls);

        if(ONLP_OID_PRESENT(&info)) {

            /** SFP Present. */
            fields[6] = aim_strdup(info.sff.vendor);
            fields[7] = aim_strdup(info.sff.model);
            fields[8] = aim_strdup(info.sff.serial);

            if(info.sff.sfp_type != SFF_SFP_TYPE_INVALID) {
                /** SFP Identified */
                fields[2] = aim_strdup(info.sff.module_type_name);
                fields[3] = aim_strdup(info.sff.media_type_name);
                fields[5] = aim_strdup(info.sff.length_desc);
            }
            else {
                fields[2] = aim_strdup("Unknown");
            }
        }
    }
    else {
        fields[2] = aim_dfstrdup("%{onlp_status}", rv);
    }

#define _NS(_string) ( (_string) ? (_string) : "")

    aim_printf((aim_pvs_t*)cookie, "%4s  %-6s  %-14s  %-6s  %-6.6s  %-5.5s  %-16.16s  %-16.16s  %16.16s\n",
               _NS(fields[0]), _NS(fields[1]), _NS(fields[2]), _NS(fields[3]),
               _NS(fields[4]), _NS(fields[5]), _NS(fields[6]), _NS(fields[7]), _NS(fields[8]));

    for(rv = 0; rv < AIM_ARRAYSIZE(fields); rv++) {
        aim_free(fields[rv]);
    }
    return 0;
}


int
onlp_sfp_inventory_show(aim_pvs_t* pvs)
{
    aim_printf(pvs, "Port  Type    Module          Media   Status  Len    Vendor            Model             S/N             \n");
    aim_printf(pvs, "----  ------  --------------  ------  ------  -----  ----------------  ----------------  ----------------\n");
    onlp_oid_iterate(ONLP_OID_CHASSIS, ONLP_OID_TYPE_FLAG_SFP,
                     sfp_inventory_show_iter__, pvs);
    return 0;
}

int
onlp_sfp_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr)
{
    int port, rv;
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, &port);

    memset(hdr, 0, sizeof(*hdr));
    ONLP_IF_ERROR_RETURN(rv = onlp_sfp_is_present(oid));

    if(rv) {
        ONLP_OID_STATUS_FLAG_SET(hdr, PRESENT);
    }
    hdr->id = oid;
    hdr->poid = ONLP_OID_CHASSIS;
    sprintf(hdr->description, "SFP %d", port);
    return rv;
}

int
onlp_sfp_info_get(onlp_oid_t oid, onlp_sfp_info_t* info)
{
    int rv;

    memset(info, 0, sizeof(*info));

    ONLP_IF_ERROR_RETURN(onlp_sfp_hdr_get(oid, &info->hdr));
    ONLP_SFP_PORT_VALIDATE_AND_MAP(&oid, NULL);

    if(ONLP_FAILURE(rv = onlp_sfp_type_get(oid, &info->type))) {
        info->type = ONLP_SFP_TYPE_INVALID;
    }

    if(ONLP_FAILURE(rv = onlp_sfp_control_flags_get(oid, &info->controls))) {
        AIM_LOG_ERROR("%{onlp_oid}: sfp_info_get: sfp_control_flags_get returned %{onlp_status}",
                      oid, rv);
        return rv;
    }

    if(ONLP_FAILURE(rv = onlp_sfp_is_present(oid))) {
        AIM_LOG_ERROR("%{onlp_oid}: sfp_info_get: is_present returned %{onlp_status}",
                      oid, rv);
        return rv;
    }

    if(rv == 0) {
        /** Module not present. */
        ONLP_OID_STATUS_FLAG_CLR(info, PRESENT);
        return 0;
    }

    /** Module present. */
    ONLP_OID_STATUS_FLAG_SET(info, PRESENT);

    /** Read the IDPROM */
    if(ONLP_FAILURE(rv = onlp_sfp_dev_read(oid, 0x50, 0, info->bytes.a0,
                                           sizeof(info->bytes.a0)))) {
        AIM_LOG_ERROR("%{onlp_oid}: sfp_info_get: sfp_dev_read(0x50) failed: %{onlp_status}",
                      oid, rv);
        return rv;
    }

    /** SFF Parsing */
    sff_eeprom_t sffe;
    sff_eeprom_parse(&sffe, info->bytes.a0);
    memcpy(&info->sff, &sffe.info, sizeof(info->sff));
    if(sffe.identified == 0) {
        info->sff.sfp_type = SFF_SFP_TYPE_INVALID;
        /* Nothing more to do */
        return 0;
    }

    /** DOM Information */
    if(ONLP_FAILURE(rv = sff_dom_spec_get(&info->sff, info->bytes.a0, &info->dom.spec))) {
        AIM_LOG_ERROR("%{onlp_oid}: sfp_info_get: sffp_dom_spec_get failed: %{onlp_status}",
                      oid, rv);
        return rv;
    }

    if(info->dom.spec == SFF_DOM_SPEC_UNSUPPORTED) {
        return 0;
    }

    if(info->dom.spec == SFF_DOM_SPEC_SFF8472) {
        /** Need the a2 data */
        if(ONLP_FAILURE(rv = onlp_sfp_dev_read(oid, 0x51, 0, info->bytes.a2,
                                               sizeof(info->bytes.a2)))) {
            AIM_LOG_ERROR("%{onlp_oid}: sfp_info_get: sfp_dev_read(0x51) failed: %{onlp_status}",
                          oid, rv);
            return rv;
        }
    }

    if(ONLP_FAILURE(rv = sff_dom_info_get(&info->dom, &info->sff,
                                          info->bytes.a0, info->bytes.a2))) {
        AIM_LOG_ERROR("%{onlp_oid}: sfp_info_get: sfp_dom_info_get failed: %{onlp_status}",
                      oid, rv);
        return rv;
    }


    return 0;
}

int
onlp_sfp_info_to_user_json(onlp_sfp_info_t* info, cJSON** cjp, uint32_t flags)
{
    int rv;
    cJSON* cj;
    rv = onlp_info_to_user_json_create(&info->hdr, &cj, flags);
    if(rv > 0) {
        if(ONLP_OID_PRESENT(info)) {
            if(info->sff.sfp_type != SFF_SFP_TYPE_INVALID) {
                cjson_util_add_string_to_object(cj, "Module", info->sff.module_type_name);
                cjson_util_add_string_to_object(cj, "Media", info->sff.media_type_name);
                cjson_util_add_string_to_object(cj, "Vendor", info->sff.vendor);
                cjson_util_add_string_to_object(cj, "Model", info->sff.model);
                cjson_util_add_string_to_object(cj, "Serial", info->sff.serial);
            }
            else {
                cjson_util_add_string_to_object(cj, "Module", "Unknown");
            }
        }
        else {
            cjson_util_add_string_to_object(cj, "Module", "Not Present");
        }
    }
    return onlp_info_to_user_json_finish(&info->hdr, cj, cjp, flags);
}

int
onlp_sfp_info_to_json(onlp_sfp_info_t* info, cJSON** cjp, uint32_t flags)
{
    int rv;
    cJSON* cj;

    if(ONLP_FAILURE(rv = onlp_info_to_json_create(&info->hdr, &cj, flags))) {
        AIM_LOG_ERROR("%{onlp_status}", rv);
        return rv;
    }

    cjson_util_add_string_to_object(cj, "type", "%{onlp_sfp_type}", info->type);

    if(ONLP_OID_PRESENT(info)) {
        sff_info_to_json(&info->sff, &cj);
        sff_dom_info_to_json(&info->dom, &cj);
    }
    return onlp_info_to_json_finish(&info->hdr, cj, cjp, flags);
}

int
onlp_sfp_info_from_json(cJSON* cj, onlp_sfp_info_t* info)
{
    return 0;
}
