//port info

//i2c_eeprom_config_cypress
#ifndef __I2C_EEPROM_CONFIG_CEDAR
#define __I2C_EEPROM_CONFIG_CEDAR

struct eeprom_config_t cedar_eeprom_config_map[] =
{
    /*port , i2c_channel*/
    {0, 12},
    {1, 13},
    {2, 14},
    {3, 15},
    {4, 16},
    {5, 17},
    {6, 18},
    {7, 19},

    {8, 20},
    {9, 21},
    {10, 22},
    {11, 23},
    {12, 24},
    {13, 25},
    {14, 26},
    {15, 27},

    {16, 28},
    {17, 29},
    {18, 30},
    {19, 31},
    {20, 32},
    {21, 33},
    {22, 34},
    {23, 35},

    {24, 36},
    {25, 37},
    {26, 38},
    {27, 39},
    {28, 40},
    {29, 41},
    {30, 42},
    {31, 43},

};
struct eeprom_i2c_tbl_t cedar_eeprom_i2c_tbl = 
{
    .map = cedar_eeprom_config_map, 
    .size = ARRAY_SIZE(cedar_eeprom_config_map)
};
#endif /*__I2C_EEPROM_CONFIG_CEDAR*/
