//"port info

//i2c_eeprom_config_cypress
#ifndef __PORT_INFO_CONFIG_BANYAN
#define __PORT_INFO_CONFIG_BANYAN

struct port_info_map_t banyan_port_info_map[] = {
    {0, QSFP_DD_TYPE, "port0"},
    {1, QSFP_DD_TYPE, "port1"},
    {2, QSFP_DD_TYPE, "port2"},
    {3, QSFP_DD_TYPE, "port3"},
    {4, QSFP_DD_TYPE, "port4"},
    {5, QSFP_DD_TYPE, "port5"},
    {6, QSFP_DD_TYPE, "port6"},
    {7, QSFP_DD_TYPE, "port7"},
    {8, QSFP_DD_TYPE, "port8"},
    {9, QSFP_DD_TYPE, "port9"},
    {10, QSFP_DD_TYPE, "port10"},
    {11, QSFP_DD_TYPE, "port11"},
    {12, QSFP_DD_TYPE, "port12"},
    {13, QSFP_DD_TYPE, "port13"},
    {14, QSFP_DD_TYPE, "port14"},
    {15, QSFP_DD_TYPE, "port15"},
    {16, QSFP_DD_TYPE, "port16"},
    {17, QSFP_DD_TYPE, "port17"},
    {18, QSFP_DD_TYPE, "port18"},
    {19, QSFP_DD_TYPE, "port19"},
    {20, QSFP_DD_TYPE, "port20"},
    {21, QSFP_DD_TYPE, "port21"},
    {22, QSFP_DD_TYPE, "port22"},
    {23, QSFP_DD_TYPE, "port23"},
    {24, QSFP_DD_TYPE, "port24"},
    {25, QSFP_DD_TYPE, "port25"},
    {26, QSFP_DD_TYPE, "port26"},
    {27, QSFP_DD_TYPE, "port27"},
    {28, QSFP_DD_TYPE, "port28"},
    {29, QSFP_DD_TYPE, "port29"},
    {30, QSFP_DD_TYPE, "port30"},
    {31, QSFP_DD_TYPE, "port31"},

};

struct port_info_table_t banyan_port_info_table = {
    .map =  banyan_port_info_map,
    .size = ARRAY_SIZE(banyan_port_info_map),
};
#endif /*__PORT_INFO_CONFIG_BANYAN*/

