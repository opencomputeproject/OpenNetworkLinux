
#ifndef __PORT_CONFIG
#define __PORT_CONFIG


#include "port_info_banyan.h"
#include "port_info_maple.h"
#include "port_info_cedar.h"
#include "port_info_cypress.h"

struct platform_port_info_t {
    int platform_name;
    struct port_info_table_t *tbl;
};

struct platform_port_info_t platform_port_info_tbl[] =
{

    {.platform_name = PLATFORM_MAPLE, .tbl = &maple_port_info_table},
    {.platform_name = PLATFORM_BANYAN, .tbl = &banyan_port_info_table},
    {.platform_name = PLATFORM_CEDAR, .tbl = &cedar_port_info_table},
    {.platform_name = PLATFORM_CYPRESS, .tbl = &cypress_port_info_table},
    {.platform_name = PLATFORM_END, }, /*keep this at the end of table*/

};


#endif /*__PORT_CONFIG*/
