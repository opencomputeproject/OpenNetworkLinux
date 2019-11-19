
#ifndef __I2C_EEPROM_CONFIG_CYPRESS
#define __I2C_EEPROM_CONFIG_CYPRESS
struct eeprom_config_t cypress_eeprom_config_map[] =
{
    /*port , i2c_channel*/
    {0, 11},
    {1, 10},
    {2, 13},
    {3, 12},
    {4, 15},
    {5, 14},
    {6, 17},
    {7, 16},
    {8, 19},
    {9, 18},
    {10, 21},
    {11, 20},
    {12, 23},
    {13, 22},
    {14, 25},
    {15, 24},
    {16, 27},
    {17, 26},
    {18, 29},
    {19, 28},
    {20, 31},
    {21, 30},
    {22, 33},
    {23, 32},
    {24, 35},
    {25, 34},
    {26, 37},
    {27, 36},
    {28, 39},
    {29, 38},
    {30, 41},
    {31, 40},
    {32, 43},
    {33, 42},
    {34, 45},
    {35, 44},
    {36, 47},
    {37, 46},
    {38, 49},
    {39, 48},
    {40, 51},
    {41, 50},
    {42, 53},
    {43, 52},
    {44, 55},
    {45, 54},
    {46, 57},
    {47, 56},
    {48, 59},
    {49, 58},
    {50, 61},
    {51, 60},
    {52, 63},
    {53, 62},

};
struct eeprom_i2c_tbl_t cypress_eeprom_i2c_tbl = 
{
    .map = cypress_eeprom_config_map, 
    .size = ARRAY_SIZE(cypress_eeprom_config_map)
};
#endif /*__I2C_EEPROM_CONFIG_MAPLE*/