/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <powerpc_quanta_ly2/powerpc_quanta_ly2_config.h>

#if POWERPC_QUANTA_LY2_R0_CONFIG_INCLUDE_UCLI == 1

#include <uCli/ucli.h>
#include <uCli/ucli_argparse.h>
#include <uCli/ucli_handler_macros.h>

static ucli_status_t
powerpc_quanta_ly2_ucli_ucli__config__(ucli_context_t* uc)
{
    UCLI_HANDLER_MACRO_MODULE_CONFIG(powerpc_quanta_ly2)
}

/* <auto.ucli.handlers.start> */
/* <auto.ucli.handlers.end> */

static ucli_module_t
powerpc_quanta_ly2_ucli_module__ =
    {
        "powerpc_quanta_ly2_ucli",
        NULL,
        powerpc_quanta_ly2_ucli_ucli_handlers__,
        NULL,
        NULL,
    };

ucli_node_t*
powerpc_quanta_ly2_ucli_node_create(void)
{
    ucli_node_t* n;
    ucli_module_init(&powerpc_quanta_ly2_ucli_module__);
    n = ucli_node_create("powerpc_quanta_ly2", NULL, &powerpc_quanta_ly2_ucli_module__);
    ucli_node_subnode_add(n, ucli_module_log_node_create("powerpc_quanta_ly2"));
    return n;
}

#else
void*
powerpc_quanta_ly2_ucli_node_create(void)
{
    return NULL;
}
#endif

