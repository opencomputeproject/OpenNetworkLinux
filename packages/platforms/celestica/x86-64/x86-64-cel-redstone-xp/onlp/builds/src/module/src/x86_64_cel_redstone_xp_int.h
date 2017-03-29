/**************************************************************************//**
 *
 * x86_64_cel_redstone_xp Internal Header
 *
 *****************************************************************************/
#ifndef __X86_64_CEL_REDSTONE_XP_INT_H__
#define __X86_64_CEL_REDSTONE_XP_INT_H__

#include <x86_64_cel_redstone_xp/x86_64_cel_redstone_xp_config.h>

/* <auto.start.enum(ALL).header> */
/** cpld1_reg */
typedef enum cpld1_reg_e {
    CPLD1_REG_VERSION = 256,
    CPLD1_REG_SCRATCH = 257,
    CPLD1_REG_RESET_CONTROL = 258,
    CPLD1_REG_RESET_SOURCE = 259,
    CPLD1_REG_BOARD_TYPE = 260,
    CPLD1_REG_INT_PORT_STATUS = 272,
    CPLD1_REG_INT0_SOURCE_STATUS = 273,
    CPLD1_REG_INT0_SOURCE_INT = 274,
    CPLD1_REG_INT0_SOURCE_MASK = 275,
    CPLD1_REG_POWER_SUPPLY_STATUS = 288,
    CPLD1_REG_POWER_GOOD_STATUS = 289,
    CPLD1_REG_BPP_CONTROL = 290,
    CPLD1_REG_WRITE_PROTECT_CONTROL = 291,
    CPLD1_REG_MISC_STATUS_CONTROL = 292,
    CPLD1_REG_INFO_RAM_ADDR_HIGH = 304,
    CPLD1_REG_INFO_RAM_ADDR_LOW = 305,
    CPLD1_REG_INFO_RAM_READ_DATA = 306,
    CPLD1_REG_INFO_RAM_WRITE_DATA = 307,
} cpld1_reg_t;

/** Enum names. */
const char* cpld1_reg_name(cpld1_reg_t e);

/** Enum values. */
int cpld1_reg_value(const char* str, cpld1_reg_t* e, int substr);

/** Enum descriptions. */
const char* cpld1_reg_desc(cpld1_reg_t e);

/** Enum validator. */
int cpld1_reg_valid(cpld1_reg_t e);

/** validator */
#define CPLD1_REG_VALID(_e) \
    (cpld1_reg_valid((_e)))

/** cpld1_reg_map table. */
extern aim_map_si_t cpld1_reg_map[];
/** cpld1_reg_desc_map table. */
extern aim_map_si_t cpld1_reg_desc_map[];

/** cpld2_reg */
typedef enum cpld2_reg_e {
    CPLD2_REG_VERSION = 512,
    CPLD2_REG_SCRATCH = 513,
    CPLD2_REG_I2C_PORT_ID = 528,
    CPLD2_REG_I2C_OP_CODE = 529,
    CPLD2_REG_I2C_DEV_ADDR = 530,
    CPLD2_REG_I2C_CMD_BYTE0 = 531,
    CPLD2_REG_I2C_CMD_BYTE1 = 532,
    CPLD2_REG_I2C_CMD_BYTE2 = 533,
    CPLD2_REG_I2C_STATUS_RESET = 534,
    CPLD2_REG_I2C_WRITE_DATA_BYTE0 = 544,
    CPLD2_REG_I2C_WRITE_DATA_BYTE1 = 545,
    CPLD2_REG_I2C_WRITE_DATA_BYTE2 = 546,
    CPLD2_REG_I2C_WRITE_DATA_BYTE3 = 547,
    CPLD2_REG_I2C_WRITE_DATA_BYTE4 = 548,
    CPLD2_REG_I2C_WRITE_DATA_BYTE5 = 549,
    CPLD2_REG_I2C_WRITE_DATA_BYTE6 = 550,
    CPLD2_REG_I2C_WRITE_DATA_BYTE7 = 551,
    CPLD2_REG_I2C_READ_DATA_BYTE0 = 560,
    CPLD2_REG_I2C_READ_DATA_BYTE1 = 561,
    CPLD2_REG_I2C_READ_DATA_BYTE2 = 562,
    CPLD2_REG_I2C_READ_DATA_BYTE3 = 563,
    CPLD2_REG_I2C_READ_DATA_BYTE4 = 564,
    CPLD2_REG_I2C_READ_DATA_BYTE5 = 565,
    CPLD2_REG_I2C_READ_DATA_BYTE6 = 566,
    CPLD2_REG_I2C_READ_DATA_BYTE7 = 567,
    CPLD2_REG_SFP_1_8_RX_LOS = 576,
    CPLD2_REG_SFP_9_16_RX_LOS = 577,
    CPLD2_REG_SFP_17_18_RX_LOS = 578,
    CPLD2_REG_SFP_1_8_RX_LOS_INT = 579,
    CPLD2_REG_SFP_9_16_RX_LOS_INT = 580,
    CPLD2_REG_SFP_17_18_RX_LOS_INT = 581,
    CPLD2_REG_SFP_1_8_RX_LOS_MASK = 582,
    CPLD2_REG_SFP_9_16_RX_LOS_MASK = 583,
    CPLD2_REG_SFP_17_18_RX_LOS_MASK = 584,
    CPLD2_REG_SFP_1_8_TX_DISABLE = 592,
    CPLD2_REG_SFP_9_16_TX_DISABLE = 593,
    CPLD2_REG_SFP_17_18_TX_DISABLE = 594,
    CPLD2_REG_SFP_1_8_RS_CONTROL = 595,
    CPLD2_REG_SFP_9_16_RS_CONTROL = 596,
    CPLD2_REG_SFP_17_18_RS_CONTROL = 597,
    CPLD2_REG_SFP_1_8_TX_FAULT = 598,
    CPLD2_REG_SFP_9_16_TX_FAULT = 599,
    CPLD2_REG_SFP_17_18_TX_FAULT = 600,
    CPLD2_REG_SFP_1_8_ABS_STATUS = 601,
    CPLD2_REG_SFP_9_16_ABS_STATUS = 602,
    CPLD2_REG_SFP_17_18_ABS_STATUS = 603,
} cpld2_reg_t;

/** Enum names. */
const char* cpld2_reg_name(cpld2_reg_t e);

/** Enum values. */
int cpld2_reg_value(const char* str, cpld2_reg_t* e, int substr);

/** Enum descriptions. */
const char* cpld2_reg_desc(cpld2_reg_t e);

/** Enum validator. */
int cpld2_reg_valid(cpld2_reg_t e);

/** validator */
#define CPLD2_REG_VALID(_e) \
    (cpld2_reg_valid((_e)))

/** cpld2_reg_map table. */
extern aim_map_si_t cpld2_reg_map[];
/** cpld2_reg_desc_map table. */
extern aim_map_si_t cpld2_reg_desc_map[];

/** cpld3_reg */
typedef enum cpld3_reg_e {
    CPLD3_REG_VERSION = 640,
    CPLD3_REG_SCRATCH = 641,
    CPLD3_REG_I2C_PORT_ID = 656,
    CPLD3_REG_I2C_OP_CODE = 657,
    CPLD3_REG_I2C_DEV_ADDR = 658,
    CPLD3_REG_I2C_CMD_BYTE0 = 659,
    CPLD3_REG_I2C_CMD_BYTE1 = 660,
    CPLD3_REG_I2C_CMD_BYTE2 = 661,
    CPLD3_REG_I2C_STATUS_RESET = 662,
    CPLD3_REG_I2C_WRITE_DATA_BYTE0 = 672,
    CPLD3_REG_I2C_WRITE_DATA_BYTE1 = 673,
    CPLD3_REG_I2C_WRITE_DATA_BYTE2 = 674,
    CPLD3_REG_I2C_WRITE_DATA_BYTE3 = 675,
    CPLD3_REG_I2C_WRITE_DATA_BYTE4 = 676,
    CPLD3_REG_I2C_WRITE_DATA_BYTE5 = 677,
    CPLD3_REG_I2C_WRITE_DATA_BYTE6 = 678,
    CPLD3_REG_I2C_WRITE_DATA_BYTE7 = 679,
    CPLD3_REG_I2C_READ_DATA_BYTE0 = 688,
    CPLD3_REG_I2C_READ_DATA_BYTE1 = 689,
    CPLD3_REG_I2C_READ_DATA_BYTE2 = 690,
    CPLD3_REG_I2C_READ_DATA_BYTE3 = 691,
    CPLD3_REG_I2C_READ_DATA_BYTE4 = 692,
    CPLD3_REG_I2C_READ_DATA_BYTE5 = 693,
    CPLD3_REG_I2C_READ_DATA_BYTE6 = 694,
    CPLD3_REG_I2C_READ_DATA_BYTE7 = 695,
    CPLD3_REG_SFP_19_26_RX_LOS = 704,
    CPLD3_REG_SFP_27_34_RX_LOS = 705,
    CPLD3_REG_SFP_35_36_RX_LOS = 706,
    CPLD3_REG_SFP_19_26_RX_LOS_INT = 707,
    CPLD3_REG_SFP_27_34_RX_LOS_INT = 708,
    CPLD3_REG_SFP_35_36_RX_LOS_INT = 709,
    CPLD3_REG_SFP_19_26_RX_LOS_MASK = 710,
    CPLD3_REG_SFP_27_34_RX_LOS_MASK = 711,
    CPLD3_REG_SFP_35_36_RX_LOS_MASK = 712,
    CPLD3_REG_SFP_19_26_TX_DISABLE = 720,
    CPLD3_REG_SFP_27_34_TX_DISABLE = 721,
    CPLD3_REG_SFP_35_36_TX_DISABLE = 722,
    CPLD3_REG_SFP_19_26_RS_CONTROL = 723,
    CPLD3_REG_SFP_27_34_RS_CONTROL = 724,
    CPLD3_REG_SFP_35_36_RS_CONTROL = 725,
    CPLD3_REG_SFP_19_26_TX_FAULT = 726,
    CPLD3_REG_SFP_27_34_TX_FAULT = 727,
    CPLD3_REG_SFP_35_36_TX_FAULT = 728,
    CPLD3_REG_SFP_19_26_ABS_STATUS = 729,
    CPLD3_REG_SFP_27_34_ABS_STATUS = 730,
    CPLD3_REG_SFP_35_36_ABS_STATUS = 731,
} cpld3_reg_t;

/** Enum names. */
const char* cpld3_reg_name(cpld3_reg_t e);

/** Enum values. */
int cpld3_reg_value(const char* str, cpld3_reg_t* e, int substr);

/** Enum descriptions. */
const char* cpld3_reg_desc(cpld3_reg_t e);

/** Enum validator. */
int cpld3_reg_valid(cpld3_reg_t e);

/** validator */
#define CPLD3_REG_VALID(_e) \
    (cpld3_reg_valid((_e)))

/** cpld3_reg_map table. */
extern aim_map_si_t cpld3_reg_map[];
/** cpld3_reg_desc_map table. */
extern aim_map_si_t cpld3_reg_desc_map[];

/** cpld4_reg */
typedef enum cpld4_reg_e {
    CPLD4_REG_VERSION = 768,
    CPLD4_REG_SCRATCH = 769,
    CPLD4_REG_RESET_CONTROL = 770,
    CPLD4_REG_LED_CONTROL = 771,
    CPLD4_REG_MISC_STATUS_CONTROL = 772,
    CPLD4_REG_INT_PORT_STATUS = 773,
    CPLD4_REG_INT0_SOURCE_STATUS = 774,
    CPLD4_REG_INT1_SOURCE_STATUS = 775,
    CPLD4_REG_INT2_SOURCE_STATUS = 776,
    CPLD4_REG_INT0_SOURCE_INT = 777,
    CPLD4_REG_INT1_SOURCE_INT = 778,
    CPLD4_REG_INT2_SOURCE_INT = 779,
    CPLD4_REG_INT0_SOURCE_MASK = 780,
    CPLD4_REG_INT1_SOURCE_MASK = 781,
    CPLD4_REG_INT2_SOURCE_MASK = 782,
    CPLD4_REG_I2C_PORT_ID = 784,
    CPLD4_REG_I2C_OP_CODE = 785,
    CPLD4_REG_I2C_DEV_ADDR = 786,
    CPLD4_REG_I2C_COMMAND_BYTE0 = 787,
    CPLD4_REG_I2C_COMMAND_BYTE1 = 788,
    CPLD4_REG_I2C_COMMAND_BYTE2 = 789,
    CPLD4_REG_I2C_STATUS_RESET = 790,
    CPLD4_REG_I2C_WRITE_DATA_BYTE0 = 800,
    CPLD4_REG_I2C_WRITE_DATA_BYTE1 = 801,
    CPLD4_REG_I2C_WRITE_DATA_BYTE2 = 802,
    CPLD4_REG_I2C_WRITE_DATA_BYTE3 = 803,
    CPLD4_REG_I2C_WRITE_DATA_BYTE4 = 804,
    CPLD4_REG_I2C_WRITE_DATA_BYTE5 = 805,
    CPLD4_REG_I2C_WRITE_DATA_BYTE6 = 806,
    CPLD4_REG_I2C_WRITE_DATA_BYTE7 = 807,
    CPLD4_REG_I2C_READ_DATA_BYTE0 = 816,
    CPLD4_REG_I2C_READ_DATA_BYTE1 = 817,
    CPLD4_REG_I2C_READ_DATA_BYTE2 = 818,
    CPLD4_REG_I2C_READ_DATA_BYTE3 = 819,
    CPLD4_REG_I2C_READ_DATA_BYTE4 = 820,
    CPLD4_REG_I2C_READ_DATA_BYTE5 = 821,
    CPLD4_REG_I2C_READ_DATA_BYTE6 = 822,
    CPLD4_REG_I2C_READ_DATA_BYTE7 = 823,
    CPLD4_REG_QSFP_RESET_CONTROL = 864,
    CPLD4_REG_QSFP_LPMOD_CONTROL = 865,
    CPLD4_REG_QSFP_ABS_STATUS = 866,
    CPLD4_REG_QSFP_INT_STATUS = 867,
    CPLD4_REG_QSFP_I2C_READY = 868,
} cpld4_reg_t;

/** Enum names. */
const char* cpld4_reg_name(cpld4_reg_t e);

/** Enum values. */
int cpld4_reg_value(const char* str, cpld4_reg_t* e, int substr);

/** Enum descriptions. */
const char* cpld4_reg_desc(cpld4_reg_t e);

/** Enum validator. */
int cpld4_reg_valid(cpld4_reg_t e);

/** validator */
#define CPLD4_REG_VALID(_e) \
    (cpld4_reg_valid((_e)))

/** cpld4_reg_map table. */
extern aim_map_si_t cpld4_reg_map[];
/** cpld4_reg_desc_map table. */
extern aim_map_si_t cpld4_reg_desc_map[];

/** cpld5_reg */
typedef enum cpld5_reg_e {
    CPLD5_REG_VERSION = 896,
    CPLD5_REG_SCRATCH = 897,
    CPLD5_REG_I2C_PORT_ID = 912,
    CPLD5_REG_I2C_OP_CODE = 913,
    CPLD5_REG_I2C_DEV_ADDR = 914,
    CPLD5_REG_I2C_CMD_BYTE0 = 915,
    CPLD5_REG_I2C_CMD_BYTE1 = 916,
    CPLD5_REG_I2C_CMD_BYTE2 = 917,
    CPLD5_REG_I2C_STATUS_RESET = 918,
    CPLD5_REG_I2C_WRITE_DATA_BYTE0 = 928,
    CPLD5_REG_I2C_WRITE_DATA_BYTE1 = 929,
    CPLD5_REG_I2C_WRITE_DATA_BYTE2 = 930,
    CPLD5_REG_I2C_WRITE_DATA_BYTE3 = 931,
    CPLD5_REG_I2C_WRITE_DATA_BYTE4 = 932,
    CPLD5_REG_I2C_WRITE_DATA_BYTE5 = 933,
    CPLD5_REG_I2C_WRITE_DATA_BYTE6 = 934,
    CPLD5_REG_I2C_WRITE_DATA_BYTE7 = 935,
    CPLD5_REG_I2C_READ_DATA_BYTE0 = 944,
    CPLD5_REG_I2C_READ_DATA_BYTE1 = 945,
    CPLD5_REG_I2C_READ_DATA_BYTE2 = 946,
    CPLD5_REG_I2C_READ_DATA_BYTE3 = 947,
    CPLD5_REG_I2C_READ_DATA_BYTE4 = 948,
    CPLD5_REG_I2C_READ_DATA_BYTE5 = 949,
    CPLD5_REG_I2C_READ_DATA_BYTE6 = 950,
    CPLD5_REG_I2C_READ_DATA_BYTE7 = 951,
    CPLD5_REG_SFP_37_44_RX_LOS = 960,
    CPLD5_REG_SFP_45_48_RX_LOS = 961,
    CPLD5_REG_SFP_37_44_RX_LOS_INT = 962,
    CPLD5_REG_SFP_45_48_RX_LOS_INT = 963,
    CPLD5_REG_SFP_37_44_RX_LOS_MASK = 964,
    CPLD5_REG_SFP_45_48_RX_LOS_MASK = 965,
    CPLD5_REG_SFP_37_44_TX_DISABLE = 976,
    CPLD5_REG_SFP_45_48_TX_DISABLE = 977,
    CPLD5_REG_SFP_37_44_RS_CONTROL = 978,
    CPLD5_REG_SFP_45_48_RS_CONTROL = 979,
    CPLD5_REG_SFP_37_44_TX_FAULT = 980,
    CPLD5_REG_SFP_45_48_TX_FAULT = 981,
    CPLD5_REG_SFP_37_44_ABS_STATUS = 982,
    CPLD5_REG_SFP_45_48_ABS_STATUS = 983,
} cpld5_reg_t;

/** Enum names. */
const char* cpld5_reg_name(cpld5_reg_t e);

/** Enum values. */
int cpld5_reg_value(const char* str, cpld5_reg_t* e, int substr);

/** Enum descriptions. */
const char* cpld5_reg_desc(cpld5_reg_t e);

/** Enum validator. */
int cpld5_reg_valid(cpld5_reg_t e);

/** validator */
#define CPLD5_REG_VALID(_e) \
    (cpld5_reg_valid((_e)))

/** cpld5_reg_map table. */
extern aim_map_si_t cpld5_reg_map[];
/** cpld5_reg_desc_map table. */
extern aim_map_si_t cpld5_reg_desc_map[];
/* <auto.end.enum(ALL).header> */

#define CEL_REDSTONE_MAX_PORT   54
#define ALL_SFP_I2C_ADDRESS     (0xA0 >> 1)
#define ALL_SFP_DIAG_I2C_ADDRESS (0xA2 >> 1)
#define SFP_XFP_LOS_ADDR 110
#define SFP_XFP_LOS_SIZE 1
#define QSFP_LOS_ADDR 3

#endif /* __X86_64_CEL_REDSTONE_XP_INT_H__ */
