#include <onlp/onlp.h>
#include <onlp/oids.h>
#include <onlp/module.h>
#include <onlp/platformi/modulei.h>
#include "onlp_int.h"
#include "onlp_locks.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_MODULE(_id)) {          \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

#define VALIDATENR(_id)                         \
    do {                                        \
        if(!ONLP_OID_IS_MODULE(_id)) {          \
            return;                             \
        }                                       \
    } while(0)

static int
onlp_module_init_locked__(void)
{
    return onlp_modulei_init();
}
ONLP_LOCKED_API0(onlp_module_init);

static int
onlp_module_info_get_locked__(onlp_oid_t id,  onlp_module_info_t* info)
{
    VALIDATE(id);
    return onlp_modulei_info_get(id, info);
}
ONLP_LOCKED_API2(onlp_module_info_get, onlp_oid_t, id, onlp_module_info_t*, info);

static int
onlp_module_status_get_locked__(onlp_oid_t id, uint32_t* status)
{
    int rv = onlp_modulei_status_get(id, status);
    if(ONLP_SUCCESS(rv)) {
        return rv;
    }
    if(ONLP_UNSUPPORTED(rv)) {
        onlp_module_info_t pi;
        rv = onlp_modulei_info_get(id, &pi);
        *status = pi.status;
    }
    return rv;
}
ONLP_LOCKED_API2(onlp_module_status_get, onlp_oid_t, id, uint32_t*, status);

static int
onlp_module_hdr_get_locked__(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    int rv = onlp_modulei_hdr_get(id, hdr);
    if(ONLP_SUCCESS(rv)) {
        return rv;
    }
    if(ONLP_UNSUPPORTED(rv)) {
        onlp_module_info_t pi;
        rv = onlp_modulei_info_get(id, &pi);
        memcpy(hdr, &pi.hdr, sizeof(pi.hdr));
    }
    return rv;
}
ONLP_LOCKED_API2(onlp_module_hdr_get, onlp_oid_t, id, onlp_oid_hdr_t*, hdr);

/************************************************************
 *
 * Debug and Show Functions
 *
 ***********************************************************/

void
onlp_module_dump(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags)
{
    int rv;
    iof_t iof;
    onlp_module_info_t info;

    VALIDATENR(id);
    onlp_oid_dump_iof_init_default(&iof, pvs);

    iof_push(&iof, "module @ %d", ONLP_OID_ID_GET(id));
    rv = onlp_module_info_get(id, &info);
    if(rv < 0) {
        onlp_oid_info_get_error(&iof, rv);
    } else {
        iof_iprintf(&iof, "Description: %s", info.hdr.description);
        if(info.status & 1) {
            iof_iprintf(&iof, "Present.");
            if(flags & ONLP_OID_DUMP_RECURSE) {
                onlp_oid_table_dump(info.hdr.coids, &iof.inherit, flags);
            }
        } else {
            iof_iprintf(&iof, "Not present.");
        }
    }
    iof_pop(&iof);
}

void
onlp_module_show(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags)
{
    int rv;
    iof_t iof;
    onlp_module_info_t mi;
    int yaml;

    onlp_oid_show_iof_init_default(&iof, pvs, flags);
    iof_pop(&iof);
    rv = onlp_module_info_get(id, &mi);

    yaml = flags & ONLP_OID_SHOW_YAML;

    if (yaml) {
        iof_push(&iof, "- ");
        iof_iprintf(&iof, "Name: Module %d", ONLP_OID_ID_GET(id));
    } else {
        iof_push(&iof, "Module %d", ONLP_OID_ID_GET(id));
    }

    if (rv < 0) {
        if(yaml) {
            iof_iprintf(&iof, "State: Error");
            iof_iprintf(&iof, "Error: %{onlp_status}", rv);
        }
        else {
            onlp_oid_info_get_error(&iof, rv);
        }
    } else {
        onlp_oid_show_description(&iof, &mi.hdr);
       if(mi.status & 0x1) {
            /* Present */
            iof_iprintf(&iof, "State: Present");
       } else {
            /* Not present */
            onlp_oid_show_state_missing(&iof);
       }
    }
    iof_pop(&iof);
}
