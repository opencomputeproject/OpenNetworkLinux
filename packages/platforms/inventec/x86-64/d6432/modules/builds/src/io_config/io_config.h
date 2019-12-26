#ifndef __IO_CONFIG
#define __IO_CONFIG

#include "io_config_banyan.h"
#include "io_config_maple.h"
//#include "io_config_cedar.h"
//#include "io_config_4U.h"

struct platform_io_info_t platform_io_info_tbl[] = {
    {
        .platform_id = PLATFORM_MAPLE,
        .i2c_ch_map = &maple_ioexp_i2c_ch_map,
        .config_map = &maple_ioexp_config_map,
        .input_map = &maple_ioexp_input_map,
        .input_port_num = maple_ioexp_input_port_num,
        .output = maple_ioexp_output_tbl,
        //.port_num =  MAPLE_IO_PORT_NUM,
        //.sfp_port_num = MAPLE_SFP_PORT_NUM,
        .cpld_config = &maple_cpld_config,
        .mux_rst_gpio = MAPLE_MUX_RST_GPIO,
    },
    {
        .platform_id = PLATFORM_BANYAN,
        .i2c_ch_map = &banyan_ioexp_i2c_ch_map,
        .config_map = &banyan_ioexp_config_map,
        .input_map = &banyan_ioexp_input_map,
        .input_port_num = banyan_ioexp_input_port_num,
        .output = banyan_ioexp_output_tbl,
        //.port_num =  MAPLE_IO_PORT_NUM,
        //.sfp_port_num = MAPLE_SFP_PORT_NUM,
        .cpld_config = &banyan_cpld_config,
        .mux_rst_gpio = BANYAN_MUX_RST_GPIO,
    },
    {
        .platform_id = PLATFORM_END,
    },
#if 0
    {
        .platform_id = PLATFORM_BANYAN,
        .io_port_num = BANYAN_IO_PORT_NUM,
        .sfp_port_num = 0,
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
#if 0
    {
        .platform_id = PLATFORM_CEDAR,
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
#endif    
    {
        .platform_id = PLATFORM_4U,
        .io_port_num = banyan_4U_IO_PORT_NUM,
        .io_i2c_tbl =  banyan_4U_io_i2c_tbl,
        .sfp_port_num = 0,
        .func_map = banyan_4U_ioexp_func_map,
        .io_config = banyan_4U_ioexp_config,
        .pca95xx_type_tbl = banyan_4U_pca95xx_type_tbl,
        .input_change = &banyan_4U_input_change_table,
        .cpld_io_info = &banyan_4U_cpld_io_info,
        //.cpld_io_info = NULL,
        .mux_reset_gpio = banyan_4U_MUX_RESET_GPIO,
        .io_sff_type = IO_SFP_QSFP_TYPE,
    },
#endif    

};
#endif /*__IO_CONFIG*/
