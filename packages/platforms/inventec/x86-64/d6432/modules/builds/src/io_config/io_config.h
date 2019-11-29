
#ifndef __IO_CONFIG
#define __IO_CONFIG


#define END_OF_TABLE (0xff)

#include "io_config_banyan.h"
#include "io_config_maple.h"
#include "io_config_cedar.h"
#include "io_config_cypress.h"


typedef enum {

    IO_SFP_QSFP_TYPE,
    IO_QSFP_TYPE,
    IO_SFF_TYPE_NUM,

} io_sff_type_t;

struct platform_io_info_t {

    int platform_name;
    int io_port_num;
    int *io_i2c_tbl;
    struct ioexp_func_t *func_map;
    struct ioexp_config_t *io_config;
    struct pca95xx_type_t *pca95xx_type_tbl;
    struct input_change_table_t *input_change;
    struct cpld_io_info_t *cpld_io_info;
    int mux_reset_gpio;
    int io_sff_type;
};

struct platform_io_info_t platform_io_info_tbl[] = {
    {
        .platform_name = PLATFORM_MAPLE,
        .io_port_num = MAPLE_IO_PORT_NUM,
        .io_i2c_tbl =  maple_io_i2c_tbl,
        .func_map = maple_ioexp_func_map,
        .io_config = maple_ioexp_config,
        .pca95xx_type_tbl = maple_pca95xx_type_tbl,
        .input_change = &maple_input_change_table,
        .cpld_io_info = &maple_cpld_io_info,
        //.cpld_io_info = NULL,
        .mux_reset_gpio = MAPLE_MUX_RESET_GPIO,
        .io_sff_type = IO_SFP_QSFP_TYPE,
    },
    {
        .platform_name = PLATFORM_BANYAN,
        .io_port_num = BANYAN_IO_PORT_NUM,
        .io_i2c_tbl =  banyan_io_i2c_tbl,
        .func_map = banyan_ioexp_func_map,
        .io_config = banyan_ioexp_config,
        .pca95xx_type_tbl = banyan_pca95xx_type_tbl,
        .input_change = &banyan_input_change_table,
        .cpld_io_info = &banyan_cpld_io_info,
        //.cpld_io_info = NULL,
        .mux_reset_gpio = BANYAN_MUX_RESET_GPIO,
        .io_sff_type = IO_QSFP_TYPE,
    },
    {
        .platform_name = PLATFORM_CEDAR,
        .io_port_num = CEDAR_IO_PORT_NUM,
        .io_i2c_tbl =  cedar_io_i2c_tbl,
        .func_map = cedar_ioexp_func_map,
        .io_config = cedar_ioexp_config,
        .pca95xx_type_tbl = cedar_pca95xx_type_tbl,
        .input_change = &cedar_input_change_table,
        .cpld_io_info = &cedar_cpld_io_info,
        //.cpld_io_info = NULL,
        .mux_reset_gpio = CEDAR_MUX_RESET_GPIO,
        .io_sff_type = IO_QSFP_TYPE,
    },
    {
        .platform_name = PLATFORM_CYPRESS,
        .io_port_num = CYPRESS_IO_PORT_NUM,
        .io_i2c_tbl =  cypress_io_i2c_tbl,
        .func_map = cypress_ioexp_func_map,
        .io_config = cypress_ioexp_config,
        .pca95xx_type_tbl = cypress_pca95xx_type_tbl,
        .input_change = &cypress_input_change_table,
        .cpld_io_info = NULL,
        .mux_reset_gpio = CYPRESS_MUX_RESET_GPIO,
        .io_sff_type = IO_SFP_QSFP_TYPE,
    },
    {
        .platform_name = PLATFORM_END,
    },

};
#endif /*__IO_CONFIG*/
