/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <mlnx_common/mlnx_common_config.h>

#if MLNX_COMMON_CONFIG_INCLUDE_UCLI == 1

#include <uCli/ucli.h>
#include <uCli/ucli_argparse.h>
#include <uCli/ucli_handler_macros.h>

static ucli_status_t
mlnx_common_ucli_ucli__config__(ucli_context_t* uc)
{
    UCLI_HANDLER_MACRO_MODULE_CONFIG(mlnx_common)
}

/* <auto.ucli.handlers.start> */
/* <auto.ucli.handlers.end> */

static ucli_module_t
mlnx_common_ucli_module__ =
    {
        "mlnx_common_ucli",
        NULL,
        mlnx_common_ucli_ucli_handlers__,
        NULL,
        NULL,
    };

ucli_node_t*
mlnx_common_ucli_node_create(void)
{
    ucli_node_t* n;
    ucli_module_init(&mlnx_common_ucli_module__);
    n = ucli_node_create("mlnx_common", NULL, &mlnx_common_ucli_module__);
    ucli_node_subnode_add(n, ucli_module_log_node_create("mlnx_common"));
    return n;
}

#else
void*
mlnx_common_ucli_node_create(void)
{
    return NULL;
}
#endif
