#ifndef __PLTFM_CONFIG
#define __PLTFM_CONFIG

#include "port_info/port_info_banyan.h"
#include "port_info/port_info_maple.h"
//#include "port_info/port_info_cedar.h"
#include "port_info/port_info_4U.h"

struct platform_info_t {
    int id;
    char *name;
    struct port_info_table_t *tbl_1st;
    struct port_info_table_t *tbl_2nd;
    int lc_num;
};

struct platform_info_t platform_info_tbl[] = {

    {.id = PLATFORM_MAPLE, 
     .name = "MAPLE", 
     .tbl_1st = &maple_port_info_table, 
     .tbl_2nd = NULL, 
     .lc_num = 1},
    
    {.id = PLATFORM_BANYAN, 
     .name = "BANYAN",
     .tbl_1st = &banyan_port_info_table, 
     .tbl_2nd = NULL, 
     .lc_num = 1},
    {.id = PLATFORM_4U,
     .name = "BANYAN_4U",
     .tbl_1st = &port_info_4U_table1, 
     .tbl_2nd = &port_info_4U_table2, 
     .lc_num = 4},
    
    {.id = PLATFORM_END, }, /*keep this at the end of table*/
};
#endif /*__PLTFM_CONFIG*/
