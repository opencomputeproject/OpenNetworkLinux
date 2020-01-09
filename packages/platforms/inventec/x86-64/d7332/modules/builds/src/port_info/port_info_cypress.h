#ifndef __PORT_INFO_CONFIG_CYPRESS
#define __PORT_INFO_CONFIG_CYPRESS

struct port_info_map_t cypress_port_info_map[] = {
    {0, SFP_TYPE, "0"},
    {1, SFP_TYPE, "1"},
    {2, SFP_TYPE, "2"},
    {3, SFP_TYPE, "3"},
    {4, SFP_TYPE, "4"},
    {5, SFP_TYPE, "5"},
    {6, SFP_TYPE, "6"},
    {7, SFP_TYPE, "7"},

    {8, SFP_TYPE, "8"},
    {9, SFP_TYPE, "9"},
    {10, SFP_TYPE,"10"},
    {11, SFP_TYPE, "11"},
    {12, SFP_TYPE, "12"},
    {13, SFP_TYPE, "13"},
    {14, SFP_TYPE, "14"},
    {15, SFP_TYPE, "15"},

    {16, SFP_TYPE, "16"},
    {17, SFP_TYPE, "17"},
    {18, SFP_TYPE, "18"},
    {19, SFP_TYPE, "19"},
    {20, SFP_TYPE, "20"},
    {21, SFP_TYPE, "21"},
    {22, SFP_TYPE, "22"},
    {23, SFP_TYPE, "23"},

    {24, SFP_TYPE, "24"},
    {25, SFP_TYPE, "25"},
    {26, SFP_TYPE, "26"},
    {27, SFP_TYPE, "27"},
    {28, SFP_TYPE, "28"},
    {29, SFP_TYPE, "29"},
    {30, SFP_TYPE, "30"},
    {31, SFP_TYPE, "31"},

    {32, SFP_TYPE, "32"},
    {33, SFP_TYPE, "33"},
    {34, SFP_TYPE, "34"},
    {35, SFP_TYPE, "35"},
    {36, SFP_TYPE, "36"},
    {37, SFP_TYPE, "37"},
    {38, SFP_TYPE, "38"},
    {39, SFP_TYPE, "39"},

    {40, SFP_TYPE, "40"},
    {41, SFP_TYPE, "41"},
    {42, SFP_TYPE, "42"},
    {43, SFP_TYPE, "43"},
    {44, SFP_TYPE, "44"},
    {45, SFP_TYPE, "45"},
    {46, SFP_TYPE, "46"},
    {47, SFP_TYPE, "47"},

    {48, QSFP_TYPE, "48"},
    {49, QSFP_TYPE, "49"},
    {50, QSFP_TYPE, "50"},
    {51, QSFP_TYPE, "51"},
    {52, QSFP_TYPE, "52"},
    {53, QSFP_TYPE, "53"},
};

struct port_info_table_t cypress_port_info_table = {
    .map =  cypress_port_info_map,
    .size = ARRAY_SIZE(cypress_port_info_map),
};

#endif /*__PORT_INFO_CONFIG_CYPRESS*/
