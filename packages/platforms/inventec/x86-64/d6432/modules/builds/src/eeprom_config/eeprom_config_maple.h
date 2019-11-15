//port info

//i2c_eeprom_config_cypress
#ifndef __I2C_EEPROM_CONFIG_MAPLE
#define __I2C_EEPROM_CONFIG_MAPLE
struct eeprom_config_t maple_eeprom_config_map[] =
{
    /*port , i2c_channel*/
    {0, 23},
    {1, 22},
    {2, 25},
    {3, 24},
    {4, 27},
    {5, 26},
    {6, 29},
    {7, 28},

    {8, 31},
    {9, 30},
    {10, 33},
    {11, 32},
    {12, 35},
    {13, 34},
    {14, 37},
    {15, 36},

    {16, 39},
    {17, 38},
    {18, 41},
    {19, 40},
    {20, 43},
    {21, 42},
    {22, 45},
    {23, 44},

    {24, 47},
    {25, 46},
    {26, 49},
    {27, 48},
    {28, 51},
    {29, 50},
    {30, 53},
    {31, 52},

    {32, 55},
    {33, 54},
    {34, 57},
    {35, 56},
    {36, 59},
    {37, 58},
    {38, 61},
    {39, 60},

    {40, 63},
    {41, 62},
    {42, 65},
    {43, 64},
    {44, 67},
    {45, 66},
    {46, 69},
    {47, 68},

    {48, 15},
    {49, 14},
    {50, 17},
    {51, 16},
    {52, 19},
    {53, 18},
    {54, 21},
    {55, 20},
};
struct eeprom_i2c_tbl_t maple_eeprom_i2c_tbl = 
{
    .map = maple_eeprom_config_map, 
    .size = ARRAY_SIZE(maple_eeprom_config_map)
};
#endif /*__I2C_EEPROM_CONFIG_MAPLE*/
