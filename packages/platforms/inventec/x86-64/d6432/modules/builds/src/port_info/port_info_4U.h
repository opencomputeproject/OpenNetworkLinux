#ifndef __PORT_INFO_CONFIG_4U
#define __PORT_INFO_CONFIG_4U

struct port_info_map_t port_info_4U_map1[] = {
    
    {1, SFP_TYPE, "port2"},
    {0, SFP_TYPE, "port1"},
    {3, SFP_TYPE, "port4"},
    {2, SFP_TYPE, "port3"},
    {5, SFP_TYPE, "port6"},
    {4, SFP_TYPE, "port5"},
    {7, SFP_TYPE, "port8"},
    {6, SFP_TYPE, "port7"},
    {9, SFP_TYPE, "port10"},
    {8, SFP_TYPE, "port9"},
    {11, SFP_TYPE, "port12"},
    {10, SFP_TYPE, "port11"},
    {13, SFP_TYPE, "port14"},
    {12, SFP_TYPE, "port13"},
    {15, SFP_TYPE, "port16"},
    {14, SFP_TYPE, "port15"},
    {17, SFP_TYPE, "port18"},
    {16, SFP_TYPE, "port17"},
    {19, SFP_TYPE, "port20"},
    {18, SFP_TYPE, "port19"},
    {21, SFP_TYPE, "port22"},
    {20, SFP_TYPE, "port21"},
    {23, SFP_TYPE, "port24"},
    {22, SFP_TYPE, "port23"},
    {25, SFP_TYPE, "port26"},
    {24, SFP_TYPE, "port25"},
    {27, SFP_TYPE, "port28"},
    {26, SFP_TYPE, "port27"},
    {29, SFP_TYPE, "port30"},
    {28, SFP_TYPE, "port29"},
    {31, SFP_TYPE, "port32"},
    {30, SFP_TYPE, "port31"},
};

struct port_info_map_t port_info_4U_map2[] = {

    {1, SFP_TYPE, "port2"},
    {0, SFP_TYPE, "port1"},
    {3, SFP_TYPE, "port4"},
    {2, SFP_TYPE, "port3"},
    {5, SFP_TYPE, "port6"},
    {4, SFP_TYPE, "port5"},
    {7, SFP_TYPE, "port8"},
    {6, SFP_TYPE, "port7"},
};
struct port_info_table_t port_info_4U_table1 = {
    .map =  port_info_4U_map1,
    .size = ARRAY_SIZE(port_info_4U_map1),
};

struct port_info_table_t port_info_4U_table2 = {
    .map =  port_info_4U_map2,
    .size = ARRAY_SIZE(port_info_4U_map2),
};
#endif /*__PORT_INFO_CONFIG_4U*/
