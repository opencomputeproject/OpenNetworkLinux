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
        .cpld_config = &banyan_cpld_config,
        .mux_rst_gpio = BANYAN_MUX_RST_GPIO,
    },
    {
        .platform_id = PLATFORM_END,
    },

};
#endif /*__IO_CONFIG*/
