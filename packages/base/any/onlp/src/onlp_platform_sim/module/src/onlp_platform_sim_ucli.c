/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <onlp_platform_sim/onlp_platform_sim_config.h>

#if ONLP_PLATFORM_SIM_CONFIG_INCLUDE_UCLI == 1

#include <uCli/ucli.h>
#include <uCli/ucli_argparse.h>
#include <uCli/ucli_handler_macros.h>

static ucli_status_t
onlp_platform_sim_ucli_ucli__config__(ucli_context_t* uc)
{
    UCLI_HANDLER_MACRO_MODULE_CONFIG(onlp_platform_sim)
}

/* <auto.ucli.handlers.start> */
/* <auto.ucli.handlers.end> */

static ucli_module_t
onlp_platform_sim_ucli_module__ =
    {
        "onlp_platform_sim_ucli",
        NULL,
        onlp_platform_sim_ucli_ucli_handlers__,
        NULL,
        NULL,
    };

ucli_node_t*
onlp_platform_sim_ucli_node_create(void)
{
    ucli_node_t* n;
    ucli_module_init(&onlp_platform_sim_ucli_module__);
    n = ucli_node_create("onlp_platform_sim", NULL, &onlp_platform_sim_ucli_module__);
    ucli_node_subnode_add(n, ucli_module_log_node_create("onlp_platform_sim"));
    return n;
}

#else
void*
onlp_platform_sim_ucli_node_create(void)
{
    return NULL;
}
#endif

