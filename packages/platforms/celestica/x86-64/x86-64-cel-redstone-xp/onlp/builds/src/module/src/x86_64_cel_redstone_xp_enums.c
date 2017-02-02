/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <x86_64_cel_redstone_xp/x86_64_cel_redstone_xp_config.h>
#include "x86_64_cel_redstone_xp_int.h"

/* <auto.start.enum(ALL).source> */
aim_map_si_t cpld1_reg_map[] =
{
    { "VERSION", CPLD1_REG_VERSION },
    { "SCRATCH", CPLD1_REG_SCRATCH },
    { "RESET_CONTROL", CPLD1_REG_RESET_CONTROL },
    { "RESET_SOURCE", CPLD1_REG_RESET_SOURCE },
    { "BOARD_TYPE", CPLD1_REG_BOARD_TYPE },
    { "INT_PORT_STATUS", CPLD1_REG_INT_PORT_STATUS },
    { "INT0_SOURCE_STATUS", CPLD1_REG_INT0_SOURCE_STATUS },
    { "INT0_SOURCE_INT", CPLD1_REG_INT0_SOURCE_INT },
    { "INT0_SOURCE_MASK", CPLD1_REG_INT0_SOURCE_MASK },
    { "POWER_SUPPLY_STATUS", CPLD1_REG_POWER_SUPPLY_STATUS },
    { "POWER_GOOD_STATUS", CPLD1_REG_POWER_GOOD_STATUS },
    { "BPP_CONTROL", CPLD1_REG_BPP_CONTROL },
    { "WRITE_PROTECT_CONTROL", CPLD1_REG_WRITE_PROTECT_CONTROL },
    { "MISC_STATUS_CONTROL", CPLD1_REG_MISC_STATUS_CONTROL },
    { "INFO_RAM_ADDR_HIGH", CPLD1_REG_INFO_RAM_ADDR_HIGH },
    { "INFO_RAM_ADDR_LOW", CPLD1_REG_INFO_RAM_ADDR_LOW },
    { "INFO_RAM_READ_DATA", CPLD1_REG_INFO_RAM_READ_DATA },
    { "INFO_RAM_WRITE_DATA", CPLD1_REG_INFO_RAM_WRITE_DATA },
    { NULL, 0 }
};

aim_map_si_t cpld1_reg_desc_map[] =
{
    { "None", CPLD1_REG_VERSION },
    { "None", CPLD1_REG_SCRATCH },
    { "None", CPLD1_REG_RESET_CONTROL },
    { "None", CPLD1_REG_RESET_SOURCE },
    { "None", CPLD1_REG_BOARD_TYPE },
    { "None", CPLD1_REG_INT_PORT_STATUS },
    { "None", CPLD1_REG_INT0_SOURCE_STATUS },
    { "None", CPLD1_REG_INT0_SOURCE_INT },
    { "None", CPLD1_REG_INT0_SOURCE_MASK },
    { "None", CPLD1_REG_POWER_SUPPLY_STATUS },
    { "None", CPLD1_REG_POWER_GOOD_STATUS },
    { "None", CPLD1_REG_BPP_CONTROL },
    { "None", CPLD1_REG_WRITE_PROTECT_CONTROL },
    { "None", CPLD1_REG_MISC_STATUS_CONTROL },
    { "None", CPLD1_REG_INFO_RAM_ADDR_HIGH },
    { "None", CPLD1_REG_INFO_RAM_ADDR_LOW },
    { "None", CPLD1_REG_INFO_RAM_READ_DATA },
    { "None", CPLD1_REG_INFO_RAM_WRITE_DATA },
    { NULL, 0 }
};

const char*
cpld1_reg_name(cpld1_reg_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, cpld1_reg_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'cpld1_reg'";
    }
}

int
cpld1_reg_value(const char* str, cpld1_reg_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, cpld1_reg_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
cpld1_reg_desc(cpld1_reg_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, cpld1_reg_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'cpld1_reg'";
    }
}

int
cpld1_reg_valid(cpld1_reg_t e)
{
    return aim_map_si_i(NULL, e, cpld1_reg_map, 0) ? 1 : 0;
}


aim_map_si_t cpld2_reg_map[] =
{
    { "VERSION", CPLD2_REG_VERSION },
    { "SCRATCH", CPLD2_REG_SCRATCH },
    { "I2C_PORT_ID", CPLD2_REG_I2C_PORT_ID },
    { "I2C_OP_CODE", CPLD2_REG_I2C_OP_CODE },
    { "I2C_DEV_ADDR", CPLD2_REG_I2C_DEV_ADDR },
    { "I2C_CMD_BYTE0", CPLD2_REG_I2C_CMD_BYTE0 },
    { "I2C_CMD_BYTE1", CPLD2_REG_I2C_CMD_BYTE1 },
    { "I2C_CMD_BYTE2", CPLD2_REG_I2C_CMD_BYTE2 },
    { "I2C_STATUS_RESET", CPLD2_REG_I2C_STATUS_RESET },
    { "I2C_WRITE_DATA_BYTE0", CPLD2_REG_I2C_WRITE_DATA_BYTE0 },
    { "I2C_WRITE_DATA_BYTE1", CPLD2_REG_I2C_WRITE_DATA_BYTE1 },
    { "I2C_WRITE_DATA_BYTE2", CPLD2_REG_I2C_WRITE_DATA_BYTE2 },
    { "I2C_WRITE_DATA_BYTE3", CPLD2_REG_I2C_WRITE_DATA_BYTE3 },
    { "I2C_WRITE_DATA_BYTE4", CPLD2_REG_I2C_WRITE_DATA_BYTE4 },
    { "I2C_WRITE_DATA_BYTE5", CPLD2_REG_I2C_WRITE_DATA_BYTE5 },
    { "I2C_WRITE_DATA_BYTE6", CPLD2_REG_I2C_WRITE_DATA_BYTE6 },
    { "I2C_WRITE_DATA_BYTE7", CPLD2_REG_I2C_WRITE_DATA_BYTE7 },
    { "I2C_READ_DATA_BYTE0", CPLD2_REG_I2C_READ_DATA_BYTE0 },
    { "I2C_READ_DATA_BYTE1", CPLD2_REG_I2C_READ_DATA_BYTE1 },
    { "I2C_READ_DATA_BYTE2", CPLD2_REG_I2C_READ_DATA_BYTE2 },
    { "I2C_READ_DATA_BYTE3", CPLD2_REG_I2C_READ_DATA_BYTE3 },
    { "I2C_READ_DATA_BYTE4", CPLD2_REG_I2C_READ_DATA_BYTE4 },
    { "I2C_READ_DATA_BYTE5", CPLD2_REG_I2C_READ_DATA_BYTE5 },
    { "I2C_READ_DATA_BYTE6", CPLD2_REG_I2C_READ_DATA_BYTE6 },
    { "I2C_READ_DATA_BYTE7", CPLD2_REG_I2C_READ_DATA_BYTE7 },
    { "SFP_1_8_RX_LOS", CPLD2_REG_SFP_1_8_RX_LOS },
    { "SFP_9_16_RX_LOS", CPLD2_REG_SFP_9_16_RX_LOS },
    { "SFP_17_18_RX_LOS", CPLD2_REG_SFP_17_18_RX_LOS },
    { "SFP_1_8_RX_LOS_INT", CPLD2_REG_SFP_1_8_RX_LOS_INT },
    { "SFP_9_16_RX_LOS_INT", CPLD2_REG_SFP_9_16_RX_LOS_INT },
    { "SFP_17_18_RX_LOS_INT", CPLD2_REG_SFP_17_18_RX_LOS_INT },
    { "SFP_1_8_RX_LOS_MASK", CPLD2_REG_SFP_1_8_RX_LOS_MASK },
    { "SFP_9_16_RX_LOS_MASK", CPLD2_REG_SFP_9_16_RX_LOS_MASK },
    { "SFP_17_18_RX_LOS_MASK", CPLD2_REG_SFP_17_18_RX_LOS_MASK },
    { "SFP_1_8_TX_DISABLE", CPLD2_REG_SFP_1_8_TX_DISABLE },
    { "SFP_9_16_TX_DISABLE", CPLD2_REG_SFP_9_16_TX_DISABLE },
    { "SFP_17_18_TX_DISABLE", CPLD2_REG_SFP_17_18_TX_DISABLE },
    { "SFP_1_8_RS_CONTROL", CPLD2_REG_SFP_1_8_RS_CONTROL },
    { "SFP_9_16_RS_CONTROL", CPLD2_REG_SFP_9_16_RS_CONTROL },
    { "SFP_17_18_RS_CONTROL", CPLD2_REG_SFP_17_18_RS_CONTROL },
    { "SFP_1_8_TX_FAULT", CPLD2_REG_SFP_1_8_TX_FAULT },
    { "SFP_9_16_TX_FAULT", CPLD2_REG_SFP_9_16_TX_FAULT },
    { "SFP_17_18_TX_FAULT", CPLD2_REG_SFP_17_18_TX_FAULT },
    { "SFP_1_8_ABS_STATUS", CPLD2_REG_SFP_1_8_ABS_STATUS },
    { "SFP_9_16_ABS_STATUS", CPLD2_REG_SFP_9_16_ABS_STATUS },
    { "SFP_17_18_ABS_STATUS", CPLD2_REG_SFP_17_18_ABS_STATUS },
    { NULL, 0 }
};

aim_map_si_t cpld2_reg_desc_map[] =
{
    { "None", CPLD2_REG_VERSION },
    { "None", CPLD2_REG_SCRATCH },
    { "None", CPLD2_REG_I2C_PORT_ID },
    { "None", CPLD2_REG_I2C_OP_CODE },
    { "None", CPLD2_REG_I2C_DEV_ADDR },
    { "None", CPLD2_REG_I2C_CMD_BYTE0 },
    { "None", CPLD2_REG_I2C_CMD_BYTE1 },
    { "None", CPLD2_REG_I2C_CMD_BYTE2 },
    { "None", CPLD2_REG_I2C_STATUS_RESET },
    { "None", CPLD2_REG_I2C_WRITE_DATA_BYTE0 },
    { "None", CPLD2_REG_I2C_WRITE_DATA_BYTE1 },
    { "None", CPLD2_REG_I2C_WRITE_DATA_BYTE2 },
    { "None", CPLD2_REG_I2C_WRITE_DATA_BYTE3 },
    { "None", CPLD2_REG_I2C_WRITE_DATA_BYTE4 },
    { "None", CPLD2_REG_I2C_WRITE_DATA_BYTE5 },
    { "None", CPLD2_REG_I2C_WRITE_DATA_BYTE6 },
    { "None", CPLD2_REG_I2C_WRITE_DATA_BYTE7 },
    { "None", CPLD2_REG_I2C_READ_DATA_BYTE0 },
    { "None", CPLD2_REG_I2C_READ_DATA_BYTE1 },
    { "None", CPLD2_REG_I2C_READ_DATA_BYTE2 },
    { "None", CPLD2_REG_I2C_READ_DATA_BYTE3 },
    { "None", CPLD2_REG_I2C_READ_DATA_BYTE4 },
    { "None", CPLD2_REG_I2C_READ_DATA_BYTE5 },
    { "None", CPLD2_REG_I2C_READ_DATA_BYTE6 },
    { "None", CPLD2_REG_I2C_READ_DATA_BYTE7 },
    { "None", CPLD2_REG_SFP_1_8_RX_LOS },
    { "None", CPLD2_REG_SFP_9_16_RX_LOS },
    { "None", CPLD2_REG_SFP_17_18_RX_LOS },
    { "None", CPLD2_REG_SFP_1_8_RX_LOS_INT },
    { "None", CPLD2_REG_SFP_9_16_RX_LOS_INT },
    { "None", CPLD2_REG_SFP_17_18_RX_LOS_INT },
    { "None", CPLD2_REG_SFP_1_8_RX_LOS_MASK },
    { "None", CPLD2_REG_SFP_9_16_RX_LOS_MASK },
    { "None", CPLD2_REG_SFP_17_18_RX_LOS_MASK },
    { "None", CPLD2_REG_SFP_1_8_TX_DISABLE },
    { "None", CPLD2_REG_SFP_9_16_TX_DISABLE },
    { "None", CPLD2_REG_SFP_17_18_TX_DISABLE },
    { "None", CPLD2_REG_SFP_1_8_RS_CONTROL },
    { "None", CPLD2_REG_SFP_9_16_RS_CONTROL },
    { "None", CPLD2_REG_SFP_17_18_RS_CONTROL },
    { "None", CPLD2_REG_SFP_1_8_TX_FAULT },
    { "None", CPLD2_REG_SFP_9_16_TX_FAULT },
    { "None", CPLD2_REG_SFP_17_18_TX_FAULT },
    { "None", CPLD2_REG_SFP_1_8_ABS_STATUS },
    { "None", CPLD2_REG_SFP_9_16_ABS_STATUS },
    { "None", CPLD2_REG_SFP_17_18_ABS_STATUS },
    { NULL, 0 }
};

const char*
cpld2_reg_name(cpld2_reg_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, cpld2_reg_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'cpld2_reg'";
    }
}

int
cpld2_reg_value(const char* str, cpld2_reg_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, cpld2_reg_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
cpld2_reg_desc(cpld2_reg_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, cpld2_reg_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'cpld2_reg'";
    }
}

int
cpld2_reg_valid(cpld2_reg_t e)
{
    return aim_map_si_i(NULL, e, cpld2_reg_map, 0) ? 1 : 0;
}


aim_map_si_t cpld3_reg_map[] =
{
    { "VERSION", CPLD3_REG_VERSION },
    { "SCRATCH", CPLD3_REG_SCRATCH },
    { "I2C_PORT_ID", CPLD3_REG_I2C_PORT_ID },
    { "I2C_OP_CODE", CPLD3_REG_I2C_OP_CODE },
    { "I2C_DEV_ADDR", CPLD3_REG_I2C_DEV_ADDR },
    { "I2C_CMD_BYTE0", CPLD3_REG_I2C_CMD_BYTE0 },
    { "I2C_CMD_BYTE1", CPLD3_REG_I2C_CMD_BYTE1 },
    { "I2C_CMD_BYTE2", CPLD3_REG_I2C_CMD_BYTE2 },
    { "I2C_STATUS_RESET", CPLD3_REG_I2C_STATUS_RESET },
    { "I2C_WRITE_DATA_BYTE0", CPLD3_REG_I2C_WRITE_DATA_BYTE0 },
    { "I2C_WRITE_DATA_BYTE1", CPLD3_REG_I2C_WRITE_DATA_BYTE1 },
    { "I2C_WRITE_DATA_BYTE2", CPLD3_REG_I2C_WRITE_DATA_BYTE2 },
    { "I2C_WRITE_DATA_BYTE3", CPLD3_REG_I2C_WRITE_DATA_BYTE3 },
    { "I2C_WRITE_DATA_BYTE4", CPLD3_REG_I2C_WRITE_DATA_BYTE4 },
    { "I2C_WRITE_DATA_BYTE5", CPLD3_REG_I2C_WRITE_DATA_BYTE5 },
    { "I2C_WRITE_DATA_BYTE6", CPLD3_REG_I2C_WRITE_DATA_BYTE6 },
    { "I2C_WRITE_DATA_BYTE7", CPLD3_REG_I2C_WRITE_DATA_BYTE7 },
    { "I2C_READ_DATA_BYTE0", CPLD3_REG_I2C_READ_DATA_BYTE0 },
    { "I2C_READ_DATA_BYTE1", CPLD3_REG_I2C_READ_DATA_BYTE1 },
    { "I2C_READ_DATA_BYTE2", CPLD3_REG_I2C_READ_DATA_BYTE2 },
    { "I2C_READ_DATA_BYTE3", CPLD3_REG_I2C_READ_DATA_BYTE3 },
    { "I2C_READ_DATA_BYTE4", CPLD3_REG_I2C_READ_DATA_BYTE4 },
    { "I2C_READ_DATA_BYTE5", CPLD3_REG_I2C_READ_DATA_BYTE5 },
    { "I2C_READ_DATA_BYTE6", CPLD3_REG_I2C_READ_DATA_BYTE6 },
    { "I2C_READ_DATA_BYTE7", CPLD3_REG_I2C_READ_DATA_BYTE7 },
    { "SFP_19_26_RX_LOS", CPLD3_REG_SFP_19_26_RX_LOS },
    { "SFP_27_34_RX_LOS", CPLD3_REG_SFP_27_34_RX_LOS },
    { "SFP_35_36_RX_LOS", CPLD3_REG_SFP_35_36_RX_LOS },
    { "SFP_19_26_RX_LOS_INT", CPLD3_REG_SFP_19_26_RX_LOS_INT },
    { "SFP_27_34_RX_LOS_INT", CPLD3_REG_SFP_27_34_RX_LOS_INT },
    { "SFP_35_36_RX_LOS_INT", CPLD3_REG_SFP_35_36_RX_LOS_INT },
    { "SFP_19_26_RX_LOS_MASK", CPLD3_REG_SFP_19_26_RX_LOS_MASK },
    { "SFP_27_34_RX_LOS_MASK", CPLD3_REG_SFP_27_34_RX_LOS_MASK },
    { "SFP_35_36_RX_LOS_MASK", CPLD3_REG_SFP_35_36_RX_LOS_MASK },
    { "SFP_19_26_TX_DISABLE", CPLD3_REG_SFP_19_26_TX_DISABLE },
    { "SFP_27_34_TX_DISABLE", CPLD3_REG_SFP_27_34_TX_DISABLE },
    { "SFP_35_36_TX_DISABLE", CPLD3_REG_SFP_35_36_TX_DISABLE },
    { "SFP_19_26_RS_CONTROL", CPLD3_REG_SFP_19_26_RS_CONTROL },
    { "SFP_27_34_RS_CONTROL", CPLD3_REG_SFP_27_34_RS_CONTROL },
    { "SFP_35_36_RS_CONTROL", CPLD3_REG_SFP_35_36_RS_CONTROL },
    { "SFP_19_26_TX_FAULT", CPLD3_REG_SFP_19_26_TX_FAULT },
    { "SFP_27_34_TX_FAULT", CPLD3_REG_SFP_27_34_TX_FAULT },
    { "SFP_35_36_TX_FAULT", CPLD3_REG_SFP_35_36_TX_FAULT },
    { "SFP_19_26_ABS_STATUS", CPLD3_REG_SFP_19_26_ABS_STATUS },
    { "SFP_27_34_ABS_STATUS", CPLD3_REG_SFP_27_34_ABS_STATUS },
    { "SFP_35_36_ABS_STATUS", CPLD3_REG_SFP_35_36_ABS_STATUS },
    { NULL, 0 }
};

aim_map_si_t cpld3_reg_desc_map[] =
{
    { "None", CPLD3_REG_VERSION },
    { "None", CPLD3_REG_SCRATCH },
    { "None", CPLD3_REG_I2C_PORT_ID },
    { "None", CPLD3_REG_I2C_OP_CODE },
    { "None", CPLD3_REG_I2C_DEV_ADDR },
    { "None", CPLD3_REG_I2C_CMD_BYTE0 },
    { "None", CPLD3_REG_I2C_CMD_BYTE1 },
    { "None", CPLD3_REG_I2C_CMD_BYTE2 },
    { "None", CPLD3_REG_I2C_STATUS_RESET },
    { "None", CPLD3_REG_I2C_WRITE_DATA_BYTE0 },
    { "None", CPLD3_REG_I2C_WRITE_DATA_BYTE1 },
    { "None", CPLD3_REG_I2C_WRITE_DATA_BYTE2 },
    { "None", CPLD3_REG_I2C_WRITE_DATA_BYTE3 },
    { "None", CPLD3_REG_I2C_WRITE_DATA_BYTE4 },
    { "None", CPLD3_REG_I2C_WRITE_DATA_BYTE5 },
    { "None", CPLD3_REG_I2C_WRITE_DATA_BYTE6 },
    { "None", CPLD3_REG_I2C_WRITE_DATA_BYTE7 },
    { "None", CPLD3_REG_I2C_READ_DATA_BYTE0 },
    { "None", CPLD3_REG_I2C_READ_DATA_BYTE1 },
    { "None", CPLD3_REG_I2C_READ_DATA_BYTE2 },
    { "None", CPLD3_REG_I2C_READ_DATA_BYTE3 },
    { "None", CPLD3_REG_I2C_READ_DATA_BYTE4 },
    { "None", CPLD3_REG_I2C_READ_DATA_BYTE5 },
    { "None", CPLD3_REG_I2C_READ_DATA_BYTE6 },
    { "None", CPLD3_REG_I2C_READ_DATA_BYTE7 },
    { "None", CPLD3_REG_SFP_19_26_RX_LOS },
    { "None", CPLD3_REG_SFP_27_34_RX_LOS },
    { "None", CPLD3_REG_SFP_35_36_RX_LOS },
    { "None", CPLD3_REG_SFP_19_26_RX_LOS_INT },
    { "None", CPLD3_REG_SFP_27_34_RX_LOS_INT },
    { "None", CPLD3_REG_SFP_35_36_RX_LOS_INT },
    { "None", CPLD3_REG_SFP_19_26_RX_LOS_MASK },
    { "None", CPLD3_REG_SFP_27_34_RX_LOS_MASK },
    { "None", CPLD3_REG_SFP_35_36_RX_LOS_MASK },
    { "None", CPLD3_REG_SFP_19_26_TX_DISABLE },
    { "None", CPLD3_REG_SFP_27_34_TX_DISABLE },
    { "None", CPLD3_REG_SFP_35_36_TX_DISABLE },
    { "None", CPLD3_REG_SFP_19_26_RS_CONTROL },
    { "None", CPLD3_REG_SFP_27_34_RS_CONTROL },
    { "None", CPLD3_REG_SFP_35_36_RS_CONTROL },
    { "None", CPLD3_REG_SFP_19_26_TX_FAULT },
    { "None", CPLD3_REG_SFP_27_34_TX_FAULT },
    { "None", CPLD3_REG_SFP_35_36_TX_FAULT },
    { "None", CPLD3_REG_SFP_19_26_ABS_STATUS },
    { "None", CPLD3_REG_SFP_27_34_ABS_STATUS },
    { "None", CPLD3_REG_SFP_35_36_ABS_STATUS },
    { NULL, 0 }
};

const char*
cpld3_reg_name(cpld3_reg_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, cpld3_reg_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'cpld3_reg'";
    }
}

int
cpld3_reg_value(const char* str, cpld3_reg_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, cpld3_reg_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
cpld3_reg_desc(cpld3_reg_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, cpld3_reg_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'cpld3_reg'";
    }
}

int
cpld3_reg_valid(cpld3_reg_t e)
{
    return aim_map_si_i(NULL, e, cpld3_reg_map, 0) ? 1 : 0;
}


aim_map_si_t cpld4_reg_map[] =
{
    { "VERSION", CPLD4_REG_VERSION },
    { "SCRATCH", CPLD4_REG_SCRATCH },
    { "RESET_CONTROL", CPLD4_REG_RESET_CONTROL },
    { "LED_CONTROL", CPLD4_REG_LED_CONTROL },
    { "MISC_STATUS_CONTROL", CPLD4_REG_MISC_STATUS_CONTROL },
    { "INT_PORT_STATUS", CPLD4_REG_INT_PORT_STATUS },
    { "INT0_SOURCE_STATUS", CPLD4_REG_INT0_SOURCE_STATUS },
    { "INT1_SOURCE_STATUS", CPLD4_REG_INT1_SOURCE_STATUS },
    { "INT2_SOURCE_STATUS", CPLD4_REG_INT2_SOURCE_STATUS },
    { "INT0_SOURCE_INT", CPLD4_REG_INT0_SOURCE_INT },
    { "INT1_SOURCE_INT", CPLD4_REG_INT1_SOURCE_INT },
    { "INT2_SOURCE_INT", CPLD4_REG_INT2_SOURCE_INT },
    { "INT0_SOURCE_MASK", CPLD4_REG_INT0_SOURCE_MASK },
    { "INT1_SOURCE_MASK", CPLD4_REG_INT1_SOURCE_MASK },
    { "INT2_SOURCE_MASK", CPLD4_REG_INT2_SOURCE_MASK },
    { "I2C_PORT_ID", CPLD4_REG_I2C_PORT_ID },
    { "I2C_OP_CODE", CPLD4_REG_I2C_OP_CODE },
    { "I2C_DEV_ADDR", CPLD4_REG_I2C_DEV_ADDR },
    { "I2C_COMMAND_BYTE0", CPLD4_REG_I2C_COMMAND_BYTE0 },
    { "I2C_COMMAND_BYTE1", CPLD4_REG_I2C_COMMAND_BYTE1 },
    { "I2C_COMMAND_BYTE2", CPLD4_REG_I2C_COMMAND_BYTE2 },
    { "I2C_STATUS_RESET", CPLD4_REG_I2C_STATUS_RESET },
    { "I2C_WRITE_DATA_BYTE0", CPLD4_REG_I2C_WRITE_DATA_BYTE0 },
    { "I2C_WRITE_DATA_BYTE1", CPLD4_REG_I2C_WRITE_DATA_BYTE1 },
    { "I2C_WRITE_DATA_BYTE2", CPLD4_REG_I2C_WRITE_DATA_BYTE2 },
    { "I2C_WRITE_DATA_BYTE3", CPLD4_REG_I2C_WRITE_DATA_BYTE3 },
    { "I2C_WRITE_DATA_BYTE4", CPLD4_REG_I2C_WRITE_DATA_BYTE4 },
    { "I2C_WRITE_DATA_BYTE5", CPLD4_REG_I2C_WRITE_DATA_BYTE5 },
    { "I2C_WRITE_DATA_BYTE6", CPLD4_REG_I2C_WRITE_DATA_BYTE6 },
    { "I2C_WRITE_DATA_BYTE7", CPLD4_REG_I2C_WRITE_DATA_BYTE7 },
    { "I2C_READ_DATA_BYTE0", CPLD4_REG_I2C_READ_DATA_BYTE0 },
    { "I2C_READ_DATA_BYTE1", CPLD4_REG_I2C_READ_DATA_BYTE1 },
    { "I2C_READ_DATA_BYTE2", CPLD4_REG_I2C_READ_DATA_BYTE2 },
    { "I2C_READ_DATA_BYTE3", CPLD4_REG_I2C_READ_DATA_BYTE3 },
    { "I2C_READ_DATA_BYTE4", CPLD4_REG_I2C_READ_DATA_BYTE4 },
    { "I2C_READ_DATA_BYTE5", CPLD4_REG_I2C_READ_DATA_BYTE5 },
    { "I2C_READ_DATA_BYTE6", CPLD4_REG_I2C_READ_DATA_BYTE6 },
    { "I2C_READ_DATA_BYTE7", CPLD4_REG_I2C_READ_DATA_BYTE7 },
    { "QSFP_RESET_CONTROL", CPLD4_REG_QSFP_RESET_CONTROL },
    { "QSFP_LPMOD_CONTROL", CPLD4_REG_QSFP_LPMOD_CONTROL },
    { "QSFP_ABS_STATUS", CPLD4_REG_QSFP_ABS_STATUS },
    { "QSFP_INT_STATUS", CPLD4_REG_QSFP_INT_STATUS },
    { "QSFP_I2C_READY", CPLD4_REG_QSFP_I2C_READY },
    { NULL, 0 }
};

aim_map_si_t cpld4_reg_desc_map[] =
{
    { "None", CPLD4_REG_VERSION },
    { "None", CPLD4_REG_SCRATCH },
    { "None", CPLD4_REG_RESET_CONTROL },
    { "None", CPLD4_REG_LED_CONTROL },
    { "None", CPLD4_REG_MISC_STATUS_CONTROL },
    { "None", CPLD4_REG_INT_PORT_STATUS },
    { "None", CPLD4_REG_INT0_SOURCE_STATUS },
    { "None", CPLD4_REG_INT1_SOURCE_STATUS },
    { "None", CPLD4_REG_INT2_SOURCE_STATUS },
    { "None", CPLD4_REG_INT0_SOURCE_INT },
    { "None", CPLD4_REG_INT1_SOURCE_INT },
    { "None", CPLD4_REG_INT2_SOURCE_INT },
    { "None", CPLD4_REG_INT0_SOURCE_MASK },
    { "None", CPLD4_REG_INT1_SOURCE_MASK },
    { "None", CPLD4_REG_INT2_SOURCE_MASK },
    { "None", CPLD4_REG_I2C_PORT_ID },
    { "None", CPLD4_REG_I2C_OP_CODE },
    { "None", CPLD4_REG_I2C_DEV_ADDR },
    { "None", CPLD4_REG_I2C_COMMAND_BYTE0 },
    { "None", CPLD4_REG_I2C_COMMAND_BYTE1 },
    { "None", CPLD4_REG_I2C_COMMAND_BYTE2 },
    { "None", CPLD4_REG_I2C_STATUS_RESET },
    { "None", CPLD4_REG_I2C_WRITE_DATA_BYTE0 },
    { "None", CPLD4_REG_I2C_WRITE_DATA_BYTE1 },
    { "None", CPLD4_REG_I2C_WRITE_DATA_BYTE2 },
    { "None", CPLD4_REG_I2C_WRITE_DATA_BYTE3 },
    { "None", CPLD4_REG_I2C_WRITE_DATA_BYTE4 },
    { "None", CPLD4_REG_I2C_WRITE_DATA_BYTE5 },
    { "None", CPLD4_REG_I2C_WRITE_DATA_BYTE6 },
    { "None", CPLD4_REG_I2C_WRITE_DATA_BYTE7 },
    { "None", CPLD4_REG_I2C_READ_DATA_BYTE0 },
    { "None", CPLD4_REG_I2C_READ_DATA_BYTE1 },
    { "None", CPLD4_REG_I2C_READ_DATA_BYTE2 },
    { "None", CPLD4_REG_I2C_READ_DATA_BYTE3 },
    { "None", CPLD4_REG_I2C_READ_DATA_BYTE4 },
    { "None", CPLD4_REG_I2C_READ_DATA_BYTE5 },
    { "None", CPLD4_REG_I2C_READ_DATA_BYTE6 },
    { "None", CPLD4_REG_I2C_READ_DATA_BYTE7 },
    { "None", CPLD4_REG_QSFP_RESET_CONTROL },
    { "None", CPLD4_REG_QSFP_LPMOD_CONTROL },
    { "None", CPLD4_REG_QSFP_ABS_STATUS },
    { "None", CPLD4_REG_QSFP_INT_STATUS },
    { "None", CPLD4_REG_QSFP_I2C_READY },
    { NULL, 0 }
};

const char*
cpld4_reg_name(cpld4_reg_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, cpld4_reg_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'cpld4_reg'";
    }
}

int
cpld4_reg_value(const char* str, cpld4_reg_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, cpld4_reg_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
cpld4_reg_desc(cpld4_reg_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, cpld4_reg_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'cpld4_reg'";
    }
}

int
cpld4_reg_valid(cpld4_reg_t e)
{
    return aim_map_si_i(NULL, e, cpld4_reg_map, 0) ? 1 : 0;
}


aim_map_si_t cpld5_reg_map[] =
{
    { "VERSION", CPLD5_REG_VERSION },
    { "SCRATCH", CPLD5_REG_SCRATCH },
    { "I2C_PORT_ID", CPLD5_REG_I2C_PORT_ID },
    { "I2C_OP_CODE", CPLD5_REG_I2C_OP_CODE },
    { "I2C_DEV_ADDR", CPLD5_REG_I2C_DEV_ADDR },
    { "I2C_CMD_BYTE0", CPLD5_REG_I2C_CMD_BYTE0 },
    { "I2C_CMD_BYTE1", CPLD5_REG_I2C_CMD_BYTE1 },
    { "I2C_CMD_BYTE2", CPLD5_REG_I2C_CMD_BYTE2 },
    { "I2C_STATUS_RESET", CPLD5_REG_I2C_STATUS_RESET },
    { "I2C_WRITE_DATA_BYTE0", CPLD5_REG_I2C_WRITE_DATA_BYTE0 },
    { "I2C_WRITE_DATA_BYTE1", CPLD5_REG_I2C_WRITE_DATA_BYTE1 },
    { "I2C_WRITE_DATA_BYTE2", CPLD5_REG_I2C_WRITE_DATA_BYTE2 },
    { "I2C_WRITE_DATA_BYTE3", CPLD5_REG_I2C_WRITE_DATA_BYTE3 },
    { "I2C_WRITE_DATA_BYTE4", CPLD5_REG_I2C_WRITE_DATA_BYTE4 },
    { "I2C_WRITE_DATA_BYTE5", CPLD5_REG_I2C_WRITE_DATA_BYTE5 },
    { "I2C_WRITE_DATA_BYTE6", CPLD5_REG_I2C_WRITE_DATA_BYTE6 },
    { "I2C_WRITE_DATA_BYTE7", CPLD5_REG_I2C_WRITE_DATA_BYTE7 },
    { "I2C_READ_DATA_BYTE0", CPLD5_REG_I2C_READ_DATA_BYTE0 },
    { "I2C_READ_DATA_BYTE1", CPLD5_REG_I2C_READ_DATA_BYTE1 },
    { "I2C_READ_DATA_BYTE2", CPLD5_REG_I2C_READ_DATA_BYTE2 },
    { "I2C_READ_DATA_BYTE3", CPLD5_REG_I2C_READ_DATA_BYTE3 },
    { "I2C_READ_DATA_BYTE4", CPLD5_REG_I2C_READ_DATA_BYTE4 },
    { "I2C_READ_DATA_BYTE5", CPLD5_REG_I2C_READ_DATA_BYTE5 },
    { "I2C_READ_DATA_BYTE6", CPLD5_REG_I2C_READ_DATA_BYTE6 },
    { "I2C_READ_DATA_BYTE7", CPLD5_REG_I2C_READ_DATA_BYTE7 },
    { "SFP_37_44_RX_LOS", CPLD5_REG_SFP_37_44_RX_LOS },
    { "SFP_45_48_RX_LOS", CPLD5_REG_SFP_45_48_RX_LOS },
    { "SFP_37_44_RX_LOS_INT", CPLD5_REG_SFP_37_44_RX_LOS_INT },
    { "SFP_45_48_RX_LOS_INT", CPLD5_REG_SFP_45_48_RX_LOS_INT },
    { "SFP_37_44_RX_LOS_MASK", CPLD5_REG_SFP_37_44_RX_LOS_MASK },
    { "SFP_45_48_RX_LOS_MASK", CPLD5_REG_SFP_45_48_RX_LOS_MASK },
    { "SFP_37_44_TX_DISABLE", CPLD5_REG_SFP_37_44_TX_DISABLE },
    { "SFP_45_48_TX_DISABLE", CPLD5_REG_SFP_45_48_TX_DISABLE },
    { "SFP_37_44_RS_CONTROL", CPLD5_REG_SFP_37_44_RS_CONTROL },
    { "SFP_45_48_RS_CONTROL", CPLD5_REG_SFP_45_48_RS_CONTROL },
    { "SFP_37_44_TX_FAULT", CPLD5_REG_SFP_37_44_TX_FAULT },
    { "SFP_45_48_TX_FAULT", CPLD5_REG_SFP_45_48_TX_FAULT },
    { "SFP_37_44_ABS_STATUS", CPLD5_REG_SFP_37_44_ABS_STATUS },
    { "SFP_45_48_ABS_STATUS", CPLD5_REG_SFP_45_48_ABS_STATUS },
    { NULL, 0 }
};

aim_map_si_t cpld5_reg_desc_map[] =
{
    { "None", CPLD5_REG_VERSION },
    { "None", CPLD5_REG_SCRATCH },
    { "None", CPLD5_REG_I2C_PORT_ID },
    { "None", CPLD5_REG_I2C_OP_CODE },
    { "None", CPLD5_REG_I2C_DEV_ADDR },
    { "None", CPLD5_REG_I2C_CMD_BYTE0 },
    { "None", CPLD5_REG_I2C_CMD_BYTE1 },
    { "None", CPLD5_REG_I2C_CMD_BYTE2 },
    { "None", CPLD5_REG_I2C_STATUS_RESET },
    { "None", CPLD5_REG_I2C_WRITE_DATA_BYTE0 },
    { "None", CPLD5_REG_I2C_WRITE_DATA_BYTE1 },
    { "None", CPLD5_REG_I2C_WRITE_DATA_BYTE2 },
    { "None", CPLD5_REG_I2C_WRITE_DATA_BYTE3 },
    { "None", CPLD5_REG_I2C_WRITE_DATA_BYTE4 },
    { "None", CPLD5_REG_I2C_WRITE_DATA_BYTE5 },
    { "None", CPLD5_REG_I2C_WRITE_DATA_BYTE6 },
    { "None", CPLD5_REG_I2C_WRITE_DATA_BYTE7 },
    { "None", CPLD5_REG_I2C_READ_DATA_BYTE0 },
    { "None", CPLD5_REG_I2C_READ_DATA_BYTE1 },
    { "None", CPLD5_REG_I2C_READ_DATA_BYTE2 },
    { "None", CPLD5_REG_I2C_READ_DATA_BYTE3 },
    { "None", CPLD5_REG_I2C_READ_DATA_BYTE4 },
    { "None", CPLD5_REG_I2C_READ_DATA_BYTE5 },
    { "None", CPLD5_REG_I2C_READ_DATA_BYTE6 },
    { "None", CPLD5_REG_I2C_READ_DATA_BYTE7 },
    { "None", CPLD5_REG_SFP_37_44_RX_LOS },
    { "None", CPLD5_REG_SFP_45_48_RX_LOS },
    { "None", CPLD5_REG_SFP_37_44_RX_LOS_INT },
    { "None", CPLD5_REG_SFP_45_48_RX_LOS_INT },
    { "None", CPLD5_REG_SFP_37_44_RX_LOS_MASK },
    { "None", CPLD5_REG_SFP_45_48_RX_LOS_MASK },
    { "None", CPLD5_REG_SFP_37_44_TX_DISABLE },
    { "None", CPLD5_REG_SFP_45_48_TX_DISABLE },
    { "None", CPLD5_REG_SFP_37_44_RS_CONTROL },
    { "None", CPLD5_REG_SFP_45_48_RS_CONTROL },
    { "None", CPLD5_REG_SFP_37_44_TX_FAULT },
    { "None", CPLD5_REG_SFP_45_48_TX_FAULT },
    { "None", CPLD5_REG_SFP_37_44_ABS_STATUS },
    { "None", CPLD5_REG_SFP_45_48_ABS_STATUS },
    { NULL, 0 }
};

const char*
cpld5_reg_name(cpld5_reg_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, cpld5_reg_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'cpld5_reg'";
    }
}

int
cpld5_reg_value(const char* str, cpld5_reg_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, cpld5_reg_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
cpld5_reg_desc(cpld5_reg_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, cpld5_reg_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'cpld5_reg'";
    }
}

int
cpld5_reg_valid(cpld5_reg_t e)
{
    return aim_map_si_i(NULL, e, cpld5_reg_map, 0) ? 1 : 0;
}

/* <auto.end.enum(ALL).source> */

