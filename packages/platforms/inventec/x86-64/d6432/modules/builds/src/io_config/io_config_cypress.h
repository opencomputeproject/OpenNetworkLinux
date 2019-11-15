//port info
#ifndef __IO_CONFIG_CYPRESS_H
#define __IO_CONFIG_CYPRESS_H


#define CPLD_IOEXP_INTR_N (12) /*GPIO 12*/
#define CPLD_I2C_CH (2)
#define CPLD_I2C_ADDR (0x33)

#define CYPRESS_MUX_RESET_GPIO (69)

/*io expander layout*/
int cypress_io_i2c_tbl[] =
{
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    END_OF_TABLE,    
};
struct pca95xx_type_t cypress_pca95xx_type_tbl[] = 
{
    {.ch = 2, .addr = 0x20, .type = PCA9555_TYPE},
    {.ch = 2, .addr = 0x21, .type = PCA9555_TYPE},
	{.ch = 2, .addr = 0x22, .type = PCA9555_TYPE},
	{.ch = 3, .addr = 0x20, .type = PCA9555_TYPE},
	{.ch = 3, .addr = 0x21, .type = PCA9555_TYPE},
	{.ch = 3, .addr = 0x22, .type = PCA9555_TYPE},
    {.ch = 4, .addr = 0x20, .type = PCA9555_TYPE},
	{.ch = 4, .addr = 0x21, .type = PCA9555_TYPE},
	{.ch = 4, .addr = 0x22, .type = PCA9555_TYPE},
    {.ch = 5, .addr = 0x20, .type = PCA9555_TYPE},
	{.ch = 5, .addr = 0x21, .type = PCA9555_TYPE},
	{.ch = 5, .addr = 0x22, .type = PCA9555_TYPE},
    {.ch = 6, .addr = 0x20, .type = PCA9555_TYPE},
	{.ch = 6, .addr = 0x21, .type = PCA9555_TYPE},
	{.ch = 6, .addr = 0x22, .type = PCA9555_TYPE},
    {.ch = 7, .addr = 0x20, .type = PCA9555_TYPE},
	{.ch = 7, .addr = 0x21, .type = PCA9555_TYPE},
	{.ch = 7, .addr = 0x22, .type = PCA9555_TYPE},
    /*qsfp*/
    {.ch = 8, .addr = 0x20, .type = PCA9555_TYPE},
	{.ch = 8, .addr = 0x21, .type = PCA9555_TYPE},
	{.ch = 8, .addr = 0x22, .type = PCA9555_TYPE},
   	{.ch = END_OF_TABLE} /*end of table*/

};
struct ioexp_data_t cypress_ioexp_prsL[] =
{
    /*sfp*/
    {.ch = 2, .addr = 0x20, .port_min = 0, .bit_min = 4, .bit_max = 7},
	{.ch = 2, .addr = 0x21, .port_min = 4, .bit_min = 4, .bit_max = 7},
	{.ch = 3, .addr = 0x20, .port_min = 8, .bit_min = 4, .bit_max = 7},
	{.ch = 3, .addr = 0x21, .port_min = 12, .bit_min = 4, .bit_max = 7},
	{.ch = 4, .addr = 0x20, .port_min = 16, .bit_min = 4, .bit_max = 7},
	{.ch = 4, .addr = 0x21, .port_min = 20, .bit_min = 4, .bit_max = 7},
	{.ch = 5, .addr = 0x20, .port_min = 24, .bit_min = 4, .bit_max = 7},
	{.ch = 5, .addr = 0x21, .port_min = 28, .bit_min = 4, .bit_max = 7},
	{.ch = 6, .addr = 0x20, .port_min = 32, .bit_min = 4, .bit_max = 7},
	{.ch = 6, .addr = 0x21, .port_min = 36, .bit_min = 4, .bit_max = 7},
	{.ch = 7, .addr = 0x20, .port_min = 40, .bit_min = 4, .bit_max = 7},
	{.ch = 7, .addr = 0x21, .port_min = 44, .bit_min = 4, .bit_max = 7},
    /*qsfp*/
    {.ch = 8,  .addr = 0x22, .port_min = 48, .bit_min = 8, .bit_max = 11},
   	{.ch = END_OF_TABLE} /*end of table*/
};
struct ioexp_data_t cypress_ioexp_txfault[] =
{
    /*sfp*/
    {.ch = 2, .addr = 0x20, .port_min = 0, .bit_min = 0, .bit_max = 3},
	{.ch = 2, .addr = 0x21, .port_min = 4, .bit_min = 0, .bit_max = 3},
	{.ch = 3, .addr = 0x20, .port_min = 8, .bit_min = 0, .bit_max = 3},
	{.ch = 3, .addr = 0x21, .port_min = 12, .bit_min = 0, .bit_max = 3},
	{.ch = 4, .addr = 0x20, .port_min = 16, .bit_min = 0, .bit_max = 3},
	{.ch = 4, .addr = 0x21, .port_min = 20, .bit_min = 0, .bit_max = 3},
	{.ch = 5, .addr = 0x20, .port_min = 24, .bit_min = 0, .bit_max = 3},
	{.ch = 5, .addr = 0x21, .port_min = 28, .bit_min = 0, .bit_max = 3},
	{.ch = 6, .addr = 0x20, .port_min = 32, .bit_min = 0, .bit_max = 3},
	{.ch = 6, .addr = 0x21, .port_min = 36, .bit_min = 0, .bit_max = 3},
	{.ch = 7, .addr = 0x20, .port_min = 40, .bit_min = 0, .bit_max = 3},
	{.ch = 7, .addr = 0x21, .port_min = 44, .bit_min = 0, .bit_max = 3},
   	{.ch = END_OF_TABLE} /*end of table*/
};
struct ioexp_data_t cypress_ioexp_rxlos[] =
{
    /*sfp*/
    {.ch = 2, .addr = 0x20, .port_min = 0, .bit_min = 12, .bit_max = 15},
	{.ch = 2, .addr = 0x21, .port_min = 4, .bit_min = 12, .bit_max = 15},
	{.ch = 3, .addr = 0x20, .port_min = 8, .bit_min = 12, .bit_max = 15},
	{.ch = 3, .addr = 0x21, .port_min = 12, .bit_min = 12, .bit_max = 15},
	{.ch = 4, .addr = 0x20, .port_min = 16, .bit_min = 12, .bit_max = 15},
	{.ch = 4, .addr = 0x21, .port_min = 20, .bit_min = 12, .bit_max = 15},
	{.ch = 5, .addr = 0x20, .port_min = 24, .bit_min = 12, .bit_max = 15},
	{.ch = 5, .addr = 0x21, .port_min = 28, .bit_min = 12, .bit_max = 15},
	{.ch = 6, .addr = 0x20, .port_min = 32, .bit_min = 12, .bit_max = 15},
	{.ch = 6, .addr = 0x21, .port_min = 36, .bit_min = 12, .bit_max = 15},
	{.ch = 7, .addr = 0x20, .port_min = 40, .bit_min = 12, .bit_max = 15},
	{.ch = 7, .addr = 0x21, .port_min = 44, .bit_min = 12, .bit_max = 15},
   	{.ch = END_OF_TABLE} /*end of table*/
};
struct ioexp_data_t cypress_ioexp_lpmode[] =
{
    {.ch = 8,  .addr = 0x20, .port_min = 48, .bit_min = 8, .bit_max = 15},
   	{.ch = END_OF_TABLE} /*end of table*/
};
struct ioexp_data_t cypress_ioexp_intL[] =
{
    {.ch = 8,  .addr = 0x22, .port_min = 48, .bit_min = 0, .bit_max = 7},
   	{.ch = END_OF_TABLE} /*end of table*/
};
struct ioexp_data_t cypress_ioexp_modesel[] =
{
    {.ch = 8,  .addr = 0x21, .port_min = 48, .bit_min = 8, .bit_max = 15},
   	{.ch = END_OF_TABLE} /*end of table*/
};
struct ioexp_data_t cypress_ioexp_reset[] =
{
    {.ch = 8,  .addr = 0x20, .port_min = 48, .bit_min = 8, .bit_max = 15},
   	{.ch = END_OF_TABLE} /*end of table*/
};
struct ioexp_data_t cypress_ioexp_txdisable[] =
{
    /*sfp*/
    {.ch = 7, .addr = 0x20, .port_min = 0, .bit_min = 8, .bit_max = 11},
	{.ch = 7, .addr = 0x21, .port_min = 4, .bit_min = 8, .bit_max = 11},
	{.ch = 8, .addr = 0x20, .port_min = 8, .bit_min = 8, .bit_max = 11},
	{.ch = 8, .addr = 0x21, .port_min = 12, .bit_min = 8, .bit_max = 11},
	{.ch = 9, .addr = 0x20, .port_min = 16, .bit_min = 8, .bit_max = 11},
	{.ch = 9, .addr = 0x21, .port_min = 20, .bit_min = 8, .bit_max = 11},
	{.ch = 10, .addr = 0x20, .port_min = 24, .bit_min = 8, .bit_max = 11},
	{.ch = 10, .addr = 0x21, .port_min = 28, .bit_min = 8, .bit_max = 11},
	{.ch = 11, .addr = 0x20, .port_min = 32, .bit_min = 8, .bit_max = 11},
	{.ch = 11, .addr = 0x21, .port_min = 36, .bit_min = 8, .bit_max = 11},
	{.ch = 12, .addr = 0x20, .port_min = 40, .bit_min = 8, .bit_max = 11},
	{.ch = 12, .addr = 0x21, .port_min = 44, .bit_min = 8, .bit_max = 11},
   	{.ch = END_OF_TABLE} /*end of table*/
};
/*only config input pin for our purpose*/
struct ioexp_config_t cypress_ioexp_config[] =
{
    /*qsfp*/
    {.ch = 6, .addr = 0x20, .val = 0x0000},
    {.ch = 6, .addr = 0x21, .val = 0xff00},
    {.ch = 6, .addr = 0x22, .val = 0xffff},
    /*sfp*/
    {.ch = 7, .addr = 0x20, .val = 0xf0ff},
    {.ch = 7, .addr = 0x21, .val = 0xf0ff},
    {.ch = 7, .addr = 0x22, .val = 0x0000},
    {.ch = 8, .addr = 0x20, .val = 0xf0ff},
    {.ch = 8, .addr = 0x21, .val = 0xf0ff},
    {.ch = 8, .addr = 0x22, .val = 0x0000},
    {.ch = 9, .addr = 0x20, .val = 0xf0ff},
    {.ch = 9, .addr = 0x21, .val = 0xf0ff},
    {.ch = 9, .addr = 0x22, .val = 0x0000},
    {.ch = 10, .addr = 0x20, .val = 0xf0ff},
    {.ch = 10, .addr = 0x21, .val = 0xf0ff},
    {.ch = 10, .addr = 0x22, .val = 0x0000},
    {.ch = 11, .addr = 0x20, .val = 0xf0ff},
    {.ch = 11, .addr = 0x21, .val = 0xf0ff},
    {.ch = 11, .addr = 0x22, .val = 0x0000},
    {.ch = 12, .addr = 0x20, .val = 0xf0ff},
    {.ch = 12, .addr = 0x21, .val = 0xf0ff},
    {.ch = 12, .addr = 0x22, .val = 0x0000},
   	{.ch = END_OF_TABLE} /*end of table*/
};

int cypress_ioexp_port_2_port[] =
{
    1, 0, 3, 2 ,5, 4, 7, 6,
    9, 8, 11, 10 ,13, 12, 15, 14,
    17, 16, 19, 18 ,21, 20, 23, 22,
    25, 24, 27, 26 ,29, 28, 31, 30,
    33, 32, 35, 34 ,37, 36, 39, 38,
    41, 40, 43, 42 ,45, 44, 47, 46,
    49, 48, 51, 50 ,53, 52, 55, 54,

};
#define CYPRESS_IO_PORT_NUM (ARRAY_SIZE(cypress_ioexp_port_2_port))
int cypress_port_2_ioexp_port[] =
{
    1, 0, 3, 2 ,5, 4, 7, 6,
    9, 8, 11, 10 ,13, 12, 15, 14,
    17, 16, 19, 18 ,21, 20, 23, 22,
    25, 24, 27, 26 ,29, 28, 31, 30,
    33, 32, 35, 34 ,37, 36, 39, 38,
    41, 40, 43, 42 ,45, 44, 47, 46,
    49, 48, 51, 50 ,53, 52, 55, 54,

};

struct ioexp_func_t cypress_ioexp_func_map[IO_IDX_NUM] = 
{
    [IO_PRESENT] = {
     .map = cypress_ioexp_prsL,
    .ioPort_2_port = cypress_ioexp_port_2_port,
    .port_2_ioPort = cypress_port_2_ioexp_port,
    },
    [IO_RX_LOS] = {
    .map = cypress_ioexp_rxlos,
    .ioPort_2_port = cypress_ioexp_port_2_port,
    .port_2_ioPort = cypress_port_2_ioexp_port,
    },
    [IO_TX_FAULT] = {
    .map = cypress_ioexp_txfault,
    .ioPort_2_port = cypress_ioexp_port_2_port,
    .port_2_ioPort = cypress_port_2_ioexp_port,
    },
    [IO_INTL] = {
    .map = cypress_ioexp_intL,
    .ioPort_2_port = cypress_ioexp_port_2_port,
    .port_2_ioPort = cypress_port_2_ioexp_port,
    },
    [IO_RESET] = {
    .map = cypress_ioexp_reset,
    .ioPort_2_port = cypress_ioexp_port_2_port,
    .port_2_ioPort = cypress_port_2_ioexp_port,
    },
    [IO_LPMODE] = {
    .map = cypress_ioexp_lpmode,
    .ioPort_2_port = cypress_ioexp_port_2_port,
    .port_2_ioPort = cypress_port_2_ioexp_port,
    },
    [IO_MODE_SEL] = {
    .map = cypress_ioexp_modesel,
    .ioPort_2_port = cypress_ioexp_port_2_port,
    .port_2_ioPort = cypress_port_2_ioexp_port,
    },
    [IO_TXDISABLE] = {
    .map = cypress_ioexp_txdisable,
    .ioPort_2_port = cypress_ioexp_port_2_port,
    .port_2_ioPort = cypress_port_2_ioexp_port,
    },
};
/*cpld 0x36, 0x37 maping*/
int cypress_input_change_idx[] =
{
	0, /*port 0~3 , only record first port*/
	4,
	8,
	12,	
	16, 
    20,
	24,
	28,	
    32,
    36,
    40,
    44,
    48
};
struct input_change_table_t cypress_input_change_table = 
{
    .table = cypress_input_change_idx,
    .size = ARRAY_SIZE(cypress_input_change_idx),
};
struct cpld_io_info_t cypress_cpld_io_info = 
{    
    .ch = 2,
    .addr = 0x33,
    .io_int_status_reg = 0x35,
    .io_int_enable_reg = 0x37,
    .int_gpio = CPLD_IOEXP_INTR_N,
};

#endif /*__IO_CONFIG_CYPRESS_H*/
