/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <onlp_snmp/onlp_snmp_config.h>

#if ONLP_SNMP_CONFIG_INCLUDE_UCLI == 1

#include <uCli/ucli.h>
#include <uCli/ucli_argparse.h>
#include <uCli/ucli_handler_macros.h>

static ucli_status_t
onlp_snmp_ucli_ucli__config__(ucli_context_t* uc)
{
    UCLI_HANDLER_MACRO_MODULE_CONFIG(onlp_snmp)
}

/* <auto.ucli.handlers.start> */
/* <auto.ucli.handlers.end> */

static ucli_module_t
onlp_snmp_ucli_module__ =
    {
        "onlp_snmp_ucli",
        NULL,
        onlp_snmp_ucli_ucli_handlers__,
        NULL,
        NULL,
    };

ucli_node_t*
onlp_snmp_ucli_node_create(void)
{
    ucli_node_t* n;
    ucli_module_init(&onlp_snmp_ucli_module__);
    n = ucli_node_create("onlp_snmp", NULL, &onlp_snmp_ucli_module__);
    ucli_node_subnode_add(n, ucli_module_log_node_create("onlp_snmp"));
    return n;
}

#else
void*
onlp_snmp_ucli_node_create(void)
{
    return NULL;
}
#endif

