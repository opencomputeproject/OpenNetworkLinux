/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <quanta_sys_eeprom/quanta_sys_eeprom_config.h>

#if QUANTA_SYS_EEPROM_CONFIG_INCLUDE_UCLI == 1

#include <uCli/ucli.h>
#include <uCli/ucli_argparse.h>
#include <uCli/ucli_handler_macros.h>

static ucli_status_t
quanta_sys_eeprom_ucli_ucli__config__(ucli_context_t* uc)
{
    UCLI_HANDLER_MACRO_MODULE_CONFIG(quanta_sys_eeprom)
}

/* <auto.ucli.handlers.start> */
/* <auto.ucli.handlers.end> */

static ucli_module_t
quanta_sys_eeprom_ucli_module__ =
    {
        "quanta_sys_eeprom_ucli",
        NULL,
        quanta_sys_eeprom_ucli_ucli_handlers__,
        NULL,
        NULL,
    };

ucli_node_t*
quanta_sys_eeprom_ucli_node_create(void)
{
    ucli_node_t* n;
    ucli_module_init(&quanta_sys_eeprom_ucli_module__);
    n = ucli_node_create("quanta_sys_eeprom", NULL, &quanta_sys_eeprom_ucli_module__);
    ucli_node_subnode_add(n, ucli_module_log_node_create("quanta_sys_eeprom"));
    return n;
}

#else
void*
quanta_sys_eeprom_ucli_node_create(void)
{
    return NULL;
}
#endif

