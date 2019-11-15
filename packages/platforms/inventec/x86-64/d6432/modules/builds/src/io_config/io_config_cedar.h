//port info
#ifndef __IO_CONFIG_CEDAR_H
#define __IO_CONFIG_CEDAR_H

#define CPLD_IOEXP_INTR_N (12) /*GPIO 12*/
#define CPLD_I2C_CH (2)
#define CPLD_I2C_ADDR (0x33)
#define CEDAR_MUX_RESET_GPIO (69)

/*io expander layout*/
int cedar_io_i2c_tbl[] =
{
    4,
    5,
    6,
    7,
    END_OF_TABLE,    
};
struct pca95xx_type_t cedar_pca95xx_type_tbl[] = 
{
    {.ch = 4, .addr = 0x20, .type = PCA9555_TYPE},
    {.ch = 4, .addr = 0x21, .type = PCA9554_TYPE},
	{.ch = 4, .addr = 0x22, .type = PCA9555_TYPE},
	{.ch = 5, .addr = 0x20, .type = PCA9555_TYPE},
	{.ch = 5, .addr = 0x21, .type = PCA9554_TYPE},
	{.ch = 5, .addr = 0x22, .type = PCA9555_TYPE},
    {.ch = 6, .addr = 0x20, .type = PCA9555_TYPE},
	{.ch = 6, .addr = 0x21, .type = PCA9554_TYPE},
	{.ch = 6, .addr = 0x22, .type = PCA9555_TYPE},
    {.ch = 7, .addr = 0x20, .type = PCA9555_TYPE},
	{.ch = 7, .addr = 0x21, .type = PCA9554_TYPE},
	{.ch = 7, .addr = 0x22, .type = PCA9555_TYPE},
   	{.ch = END_OF_TABLE} /*end of table*/

};
struct ioexp_data_t cedar_ioexp_prsL[] =
{
    {.ch = 4, .addr = 0x22, .port_min = 0, .bit_min = 8, .bit_max = 15},
    {.ch = 5, .addr = 0x22, .port_min = 8, .bit_min = 8, .bit_max = 15},
    {.ch = 6, .addr = 0x22, .port_min = 16, .bit_min = 8, .bit_max = 15},
    {.ch = 7, .addr = 0x22, .port_min = 24, .bit_min = 8, .bit_max = 15},
   	{.ch = END_OF_TABLE} /*end of table*/
};
struct ioexp_data_t cedar_ioexp_intL[] =
{
    {.ch = 4,  .addr = 0x22, .port_min = 0, .bit_min = 0, .bit_max = 7},
    {.ch = 5,  .addr = 0x22, .port_min = 8, .bit_min = 0, .bit_max = 7},
    {.ch = 6,  .addr = 0x22, .port_min = 16, .bit_min = 0, .bit_max = 7},
    {.ch = 7,  .addr = 0x22, .port_min = 24, .bit_min = 0, .bit_max = 7},
   	{.ch = END_OF_TABLE} /*end of table*/
};
struct ioexp_data_t cedar_ioexp_lpmode[] =
{
    {.ch = 4,  .addr = 0x21, .port_min = 0, .bit_min = 0, .bit_max = 7},
    {.ch = 5,  .addr = 0x21, .port_min = 8, .bit_min = 0, .bit_max = 7},
    {.ch = 6,  .addr = 0x21, .port_min = 16, .bit_min = 0, .bit_max = 7},
    {.ch = 7,  .addr = 0x21, .port_min = 24, .bit_min = 0, .bit_max = 7},
   	{.ch = END_OF_TABLE} /*end of table*/
};
struct ioexp_data_t cedar_ioexp_modesel[] =
{
    {.ch = 4,  .addr = 0x20, .port_min = 0, .bit_min = 0, .bit_max = 7},
    {.ch = 5,  .addr = 0x20, .port_min = 8, .bit_min = 0, .bit_max = 7},
    {.ch = 6,  .addr = 0x20, .port_min = 16, .bit_min = 0, .bit_max = 7},
    {.ch = 7,  .addr = 0x20, .port_min = 24, .bit_min = 0, .bit_max = 7},
   	{.ch = END_OF_TABLE} /*end of table*/
};
struct ioexp_data_t cedar_ioexp_reset[] =
{
    {.ch = 4,  .addr = 0x20, .port_min = 0, .bit_min = 8, .bit_max = 15},
    {.ch = 5,  .addr = 0x20, .port_min = 8, .bit_min = 8, .bit_max = 15},
    {.ch = 6,  .addr = 0x20, .port_min = 16, .bit_min = 8, .bit_max = 15},
    {.ch = 7,  .addr = 0x20, .port_min = 24, .bit_min = 8, .bit_max = 15},
   	{.ch = END_OF_TABLE} /*end of table*/
};

struct ioexp_config_t cedar_ioexp_config[] =
{
    /*qsfp*/
    {.ch = 4, .addr = 0x20, .val = 0x0000},
    {.ch = 4, .addr = 0x21, .val = 0x00},
    {.ch = 4, .addr = 0x22, .val = 0xffff},
    {.ch = 5, .addr = 0x20, .val = 0x0000},
    {.ch = 5, .addr = 0x21, .val = 0x00},
    {.ch = 5, .addr = 0x22, .val = 0xffff},
    {.ch = 6, .addr = 0x20, .val = 0x0000},
    {.ch = 6, .addr = 0x21, .val = 0x00},
    {.ch = 6, .addr = 0x22, .val = 0xffff},
    {.ch = 7, .addr = 0x20, .val = 0x0000},
    {.ch = 7, .addr = 0x21, .val = 0x00},
    {.ch = 7, .addr = 0x22, .val = 0xffff},
   	{.ch = END_OF_TABLE} /*end of table*/
};

int cedar_ioexp_port_2_port[] =
{
    0,1,2,3,4,5,6,7,
    8,9,10,11,12,13,14,15,
    16,17,18,19,20,21,22,23,
    24,25,26,27,28,29,30,31

};
#define CEDAR_IO_PORT_NUM (ARRAY_SIZE(cedar_ioexp_port_2_port))
int cedar_port_2_ioexp_port[] =
{    
    0,1,2,3,4,5,6,7,
    8,9,10,11,12,13,14,15,
    16,17,18,19,20,21,22,23,
    24,25,26,27,28,29,30,31
};
struct ioexp_func_t cedar_ioexp_func_map[IO_IDX_NUM] = 
{
    [IO_PRESENT] = {
     .map = cedar_ioexp_prsL,
    .ioPort_2_port = cedar_ioexp_port_2_port,
    .port_2_ioPort = cedar_port_2_ioexp_port,
    },
    [IO_RX_LOS] = {
    .map = NULL,
    .ioPort_2_port = cedar_ioexp_port_2_port,
    .port_2_ioPort = cedar_port_2_ioexp_port,
    },
    [IO_TX_FAULT] = {
    .map = NULL,
    .ioPort_2_port = cedar_ioexp_port_2_port,
    .port_2_ioPort = cedar_port_2_ioexp_port,
    },
    [IO_INTL] = {
    .map = cedar_ioexp_intL,
    .ioPort_2_port = cedar_ioexp_port_2_port,
    .port_2_ioPort = cedar_port_2_ioexp_port,
    },
    [IO_RESET] = {
    .map = cedar_ioexp_reset,
    .ioPort_2_port = cedar_ioexp_port_2_port,
    .port_2_ioPort = cedar_port_2_ioexp_port,
    },
    [IO_LPMODE] = {
    .map = cedar_ioexp_lpmode,
    .ioPort_2_port = cedar_ioexp_port_2_port,
    .port_2_ioPort = cedar_port_2_ioexp_port,
    },
    [IO_MODE_SEL] = {
    .map = cedar_ioexp_modesel,
    .ioPort_2_port = cedar_ioexp_port_2_port,
    .port_2_ioPort = cedar_port_2_ioexp_port,
    },
    [IO_TXDISABLE] = {
    .map = NULL,
    .ioPort_2_port = cedar_ioexp_port_2_port,
    .port_2_ioPort = cedar_port_2_ioexp_port,
    },
};

int cedar_input_change_idx[] =
{
	0, 
	8,
	16, 
	24,
};
struct input_change_table_t cedar_input_change_table = 
{
    .table = cedar_input_change_idx,
    .size = ARRAY_SIZE(cedar_input_change_idx),
};
/*<TBD>*/
struct cpld_io_info_t cedar_cpld_io_info = 
{    
    .ch = 2,
    .addr = 0x33,
    .io_int_status_reg = 0x35,
    .io_int_enable_reg = 0x37,
    .int_gpio = 12,
};
#endif /*__IO_CONFIG_CEDAR_H*/
