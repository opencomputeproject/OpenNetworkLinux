/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <x86_64_cel_belgite/x86_64_cel_belgite_config.h>

#if X86_64_CEL_BELGITE_CONFIG_INCLUDE_UCLI == 1

#include <uCli/ucli.h>
#include <uCli/ucli_argparse.h>
#include <uCli/ucli_handler_macros.h>

static ucli_status_t
x86_64_cel_belgite_ucli_ucli__config__(ucli_context_t* uc)
{
    UCLI_HANDLER_MACRO_MODULE_CONFIG(x86_64_cel_belgite)
}

/* <auto.ucli.handlers.start> */
/* <auto.ucli.handlers.end> */

static ucli_module_t
x86_64_cel_belgite_ucli_module__ =
    {
        "x86_64_cel_belgite_ucli",
        NULL,
        x86_64_cel_belgite_ucli_ucli_handlers__,
        NULL,
        NULL,
    };

ucli_node_t*
x86_64_cel_belgite_ucli_node_create(void)
{
    ucli_node_t* n;
    ucli_module_init(&x86_64_cel_belgite_ucli_module__);
    n = ucli_node_create("x86_64_cel_belgite", NULL, &x86_64_cel_belgite_ucli_module__);
    ucli_node_subnode_add(n, ucli_module_log_node_create("x86_64_cel_belgite"));
    return n;
}

#else
void*
x86_64_cel_belgite_ucli_node_create(void)
{
    return NULL;
}
#endif

