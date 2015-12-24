/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <powerpc_accton_as5710_54x/powerpc_accton_as5710_54x_config.h>

#if POWERPC_ACCTON_AS5710_54X_R0_CONFIG_INCLUDE_UCLI == 1

#include <uCli/ucli.h>
#include <uCli/ucli_argparse.h>
#include <uCli/ucli_handler_macros.h>

static ucli_status_t
powerpc_accton_as5710_54x_ucli_ucli__config__(ucli_context_t* uc)
{
    UCLI_HANDLER_MACRO_MODULE_CONFIG(powerpc_accton_as5710_54x)
}

/* <auto.ucli.handlers.start> */
/* <auto.ucli.handlers.end> */

static ucli_module_t
powerpc_accton_as5710_54x_ucli_module__ =
    {
        "powerpc_accton_as5710_54x_ucli",
        NULL,
        powerpc_accton_as5710_54x_ucli_ucli_handlers__,
        NULL,
        NULL,
    };

ucli_node_t*
powerpc_accton_as5710_54x_ucli_node_create(void)
{
    ucli_node_t* n;
    ucli_module_init(&powerpc_accton_as5710_54x_ucli_module__);
    n = ucli_node_create("powerpc_accton_as5710_54x", NULL, &powerpc_accton_as5710_54x_ucli_module__);
    ucli_node_subnode_add(n, ucli_module_log_node_create("powerpc_accton_as5710_54x"));
    return n;
}

#else
void*
powerpc_accton_as5710_54x_ucli_node_create(void)
{
    return NULL;
}
#endif

