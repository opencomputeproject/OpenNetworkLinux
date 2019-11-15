//port info

//i2c_eeprom_config_cypress
#ifndef __PORT_INFO_CONFIG_CEDAR
#define __PORT_INFO_CONFIG_CEDAR

struct port_info_map_t cedar_port_info_map[] = {

    {0, QSFP_TYPE, "0"},
    {1, QSFP_TYPE, "1"},
    {2, QSFP_TYPE, "2"},
    {3, QSFP_TYPE, "3"},
    {4, QSFP_TYPE, "4"},
    {5, QSFP_TYPE, "5"},
    {6, QSFP_TYPE, "6"},
    {7, QSFP_TYPE, "7"},

    {8, QSFP_TYPE, "8"},
    {9, QSFP_TYPE, "9"},
    {10, QSFP_TYPE,"10"},
    {11, QSFP_TYPE, "11"},
    {12, QSFP_TYPE, "12"},
    {13, QSFP_TYPE, "13"},
    {14, QSFP_TYPE, "14"},
    {15, QSFP_TYPE, "15"},

    {16, QSFP_TYPE, "16"},
    {17, QSFP_TYPE, "17"},
    {18, QSFP_TYPE, "18"},
    {19, QSFP_TYPE, "19"},
    {20, QSFP_TYPE, "20"},
    {21, QSFP_TYPE, "21"},
    {22, QSFP_TYPE, "22"},
    {23, QSFP_TYPE, "23"},

    {24, QSFP_TYPE, "24"},
    {25, QSFP_TYPE, "25"},
    {26, QSFP_TYPE, "26"},
    {27, QSFP_TYPE, "27"},
    {28, QSFP_TYPE, "28"},
    {29, QSFP_TYPE, "29"},
    {30, QSFP_TYPE, "30"},
    {31, QSFP_TYPE, "31"},
};

struct port_info_table_t cedar_port_info_table =
{
    .map =  cedar_port_info_map,
    .size = ARRAY_SIZE(cedar_port_info_map),
};

#endif /*__PORT_INFO_CONFIG_CEDAR*/
