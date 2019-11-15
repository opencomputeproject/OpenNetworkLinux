//port info

//i2c_eeprom_config_cypress
#ifndef __PORT_INFO_CONFIG_MAPLE
#define __PORT_INFO_CONFIG_MAPLE

struct port_info_map_t maple_port_info_map[] = {
    {0, SFP_TYPE, "port0"},
    {1, SFP_TYPE, "port1"},
    {2, SFP_TYPE, "port2"},
    {3, SFP_TYPE, "port3"},
    {4, SFP_TYPE, "port4"},
    {5, SFP_TYPE, "port5"},
    {6, SFP_TYPE, "port6"},
    {7, SFP_TYPE, "port7"},
    {8, SFP_TYPE, "port8"},
    {9, SFP_TYPE, "port9"},
    {10, SFP_TYPE, "port10"},
    {11, SFP_TYPE, "port11"},
    {12, SFP_TYPE, "port12"},
    {13, SFP_TYPE, "port13"},
    {14, SFP_TYPE, "port14"},
    {15, SFP_TYPE, "port15"},
    {16, SFP_TYPE, "port16"},
    {17, SFP_TYPE, "port17"},
    {18, SFP_TYPE, "port18"},
    {19, SFP_TYPE, "port19"},
    {20, SFP_TYPE, "port20"},
    {21, SFP_TYPE, "port21"},
    {22, SFP_TYPE, "port22"},
    {23, SFP_TYPE, "port23"},
    {24, SFP_TYPE, "port24"},
    {25, SFP_TYPE, "port25"},
    {26, SFP_TYPE, "port26"},
    {27, SFP_TYPE, "port27"},
    {28, SFP_TYPE, "port28"},
    {29, SFP_TYPE, "port29"},
    {30, SFP_TYPE, "port30"},
    {31, SFP_TYPE, "port31"},
    {32, SFP_TYPE, "port32"},
    {33, SFP_TYPE, "port33"},
    {34, SFP_TYPE, "port34"},
    {35, SFP_TYPE, "port35"},
    {36, SFP_TYPE, "port36"},
    {37, SFP_TYPE, "port37"},
    {38, SFP_TYPE, "port38"},
    {39, SFP_TYPE, "port39"},
    {40, SFP_TYPE, "port40"},
    {41, SFP_TYPE, "port41"},
    {42, SFP_TYPE, "port42"},
    {43, SFP_TYPE, "port43"},
    {44, SFP_TYPE, "port44"},
    {45, SFP_TYPE, "port45"},
    {46, SFP_TYPE, "port46"},
    {47, SFP_TYPE, "port47"},
    {48, QSFP_TYPE, "port48"},
    {49, QSFP_TYPE, "port49"},
    {50, QSFP_TYPE, "port50"},
    {51, QSFP_TYPE, "port51"},
    {52, QSFP_TYPE, "port52"},
    {53, QSFP_TYPE, "port53"},
    {54, QSFP_TYPE, "port54"},
    {55, QSFP_TYPE, "port55"},

};

struct port_info_table_t maple_port_info_table =
{
    .map =  maple_port_info_map,
    .size = ARRAY_SIZE(maple_port_info_map),
};

#endif /*__PORT_INFO_CONFIG_MAPLE*/
