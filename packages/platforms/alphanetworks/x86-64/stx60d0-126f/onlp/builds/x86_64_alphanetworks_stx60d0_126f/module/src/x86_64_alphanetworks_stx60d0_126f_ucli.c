/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <x86_64_alphanetworks_stx60d0_126f/x86_64_alphanetworks_stx60d0_126f_config.h>

#if X86_64_ALPHANETWORKS_STX60D0_126F_CONFIG_INCLUDE_UCLI == 1

#include <uCli/ucli.h>
#include <uCli/ucli_argparse.h>
#include <uCli/ucli_handler_macros.h>

static ucli_status_t
x86_64_alphanetworks_stx60d0_126f_ucli_ucli__config__(ucli_context_t* uc)
{
    UCLI_HANDLER_MACRO_MODULE_CONFIG(x86_64_alphanetworks_stx60d0_126f)
}

/* <auto.ucli.handlers.start> */
/* <auto.ucli.handlers.end> */

static ucli_module_t
x86_64_alphanetworks_stx60d0_126f_ucli_module__ =
    {
        "x86_64_alphanetworks_stx60d0_126f_ucli",
        NULL,
        x86_64_alphanetworks_stx60d0_126f_ucli_ucli_handlers__,
        NULL,
        NULL,
    };

ucli_node_t*
x86_64_alphanetworks_stx60d0_126f_ucli_node_create(void)
{
    ucli_node_t* n;
    ucli_module_init(&x86_64_alphanetworks_stx60d0_126f_ucli_module__);
    n = ucli_node_create("x86_64_alphanetworks_stx60d0_126f", NULL, &x86_64_alphanetworks_stx60d0_126f_ucli_module__);
    ucli_node_subnode_add(n, ucli_module_log_node_create("x86_64_alphanetworks_stx60d0_126f"));
    return n;
}

#else
void*
x86_64_alphanetworks_stx60d0_126f_ucli_node_create(void)
{
    return NULL;
}
#endif

