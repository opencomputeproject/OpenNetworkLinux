#ifndef INV_SWPS_H
#define INV_SWPS_H

#include "transceiver.h"
#include "io_expander.h"
#include "inv_mux.h"

/* Module settings */
#define SWP_CLS_NAME          "swps"
#define SWP_DEV_PORT          "port"
#define SWP_DEV_MODCTL        "module"
#define SWP_RESET_PWD         "inventec"
#define SWP_POLLING_PERIOD    (300)  /* msec */
#define SWP_POLLING_ENABLE    (1)
#define SWP_AUTOCONFIG_ENABLE (1)

/* Module information */
#define SWP_AUTHOR            "Neil <liao.neil@inventec.com>"
#define SWP_DESC              "Inventec port and transceiver driver"
#define SWP_VERSION           "4.2.3"
#define SWP_LICENSE           "GPL"

/* Module status define */
#define SWP_STATE_NORMAL      (0)
#define SWP_STATE_I2C_DIE     (-91)

/* [Note]:
 *  Functions and mechanism for auto-detect platform type is ready,
 *  But HW and BIOS not ready! We need to wait them.
 *  So, please do not use PLATFORM_TYPE_AUTO until they are ready.
 *  (2016.06.13)
 */
#define PLATFORM_TYPE_AUTO          (100)
#define PLATFORM_TYPE_MAGNOLIA      (111)
#define PLATFORM_TYPE_MAGNOLIA_FNC  (112)
#define PLATFORM_TYPE_REDWOOD       (121)
#define PLATFORM_TYPE_REDWOOD_FSL   (122)
#define PLATFORM_TYPE_HUDSON32I_GA  (131)
#define PLATFORM_TYPE_SPRUCE        (141)
#define PLATFORM_TYPE_CYPRESS_GA1   (151) /* Up -> Down */
#define PLATFORM_TYPE_CYPRESS_GA2   (152) /* Down -> Up */
#define PLATFORM_TYPE_LAVENDER      (161)
/* Current running platfrom */
#define PLATFORM_SETTINGS           PLATFORM_TYPE_LAVENDER

/* Define platform flag and kernel version */
#if (PLATFORM_SETTINGS == PLATFORM_TYPE_MAGNOLIA)
  #define SWPS_MAGNOLIA          (1)
  #define SWPS_KERN_VER_BF_3_8   (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_MAGNOLIA_FNC)
  #define SWPS_MAGNOLIA          (1)
  #define SWPS_KERN_VER_AF_3_10  (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_REDWOOD)
  #define SWPS_REDWOOD           (1)
  #define SWPS_KERN_VER_BF_3_8   (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_REDWOOD_FSL)
  #define SWPS_REDWOOD_FSL       (1)
  #define SWPS_KERN_VER_BF_3_8   (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_HUDSON32I_GA)
  #define SWPS_HUDSON32I_GA      (1)
  #define SWPS_KERN_VER_BF_3_8   (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_SPRUCE)
  #define SWPS_SPRUCE            (1)
  #define SWPS_KERN_VER_BF_3_8   (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_CYPRESS_GA1)
  #define SWPS_CYPRESS_GA1       (1)
  #define SWPS_KERN_VER_BF_3_8   (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_CYPRESS_GA2)
  #define SWPS_CYPRESS_GA2       (1)
  #define SWPS_KERN_VER_BF_3_8   (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_LAVENDER)
  #define SWPS_LAVENDER          (1)
  #define SWPS_KERN_VER_BF_3_8   (1)
#endif


struct inv_platform_s {
    int  id;
    char name[64];
};

struct inv_ioexp_layout_s {
    int ioexp_id;
    int ioexp_type;
    struct ioexp_addr_s addr[4];
};

struct inv_port_layout_s {
    int port_id;
    int chan_id;
    int ioexp_id;
    int ioexp_offset;
    int transvr_type;
    int chipset_type;
    int lane_id[8];
};


/* ==========================================
 *   Inventec Platform Settings
 * ==========================================
 */
struct inv_platform_s platform_map[] = {
    {PLATFORM_TYPE_AUTO,         "Auto-Detect"  },
    {PLATFORM_TYPE_MAGNOLIA,     "Magnolia"     },
    {PLATFORM_TYPE_MAGNOLIA_FNC, "Magnolia_FNC" },
    {PLATFORM_TYPE_REDWOOD,      "Redwood"      },
    {PLATFORM_TYPE_REDWOOD_FSL,  "Redwood_FSL"  },
    {PLATFORM_TYPE_HUDSON32I_GA, "Hudson32i"    },
    {PLATFORM_TYPE_SPRUCE,       "Spruce"       },
    {PLATFORM_TYPE_CYPRESS_GA1,  "Cypress_GA1"  },
    {PLATFORM_TYPE_CYPRESS_GA2,  "Cypress_GA2"  },
    {PLATFORM_TYPE_LAVENDER,     "Lavender"  },
};


/* ==========================================
 *   Magnolia Layout configuration
 * ==========================================
 */
#ifdef SWPS_MAGNOLIA
unsigned magnolia_gpio_rest_mux = MUX_RST_GPIO_48_PAC9548;

struct inv_ioexp_layout_s magnolia_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_MAGINOLIA_NAB, { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, }, }, /* addr[1] = I/O Expander N B */
    },
    {1,  IOEXP_TYPE_MAGINOLIA_NAB, { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, }, }, /* addr[1] = I/O Expander N B */
    },
    {2,  IOEXP_TYPE_MAGINOLIA_NAB, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, }, }, /* addr[1] = I/O Expander N B */
    },
    {3,  IOEXP_TYPE_MAGINOLIA_NAB, { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, }, }, /* addr[1] = I/O Expander N B */
    },
    {4,  IOEXP_TYPE_MAGINOLIA_NAB, { {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, }, }, /* addr[1] = I/O Expander N B */
    },
    {5,  IOEXP_TYPE_MAGINOLIA_NAB, { {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, }, }, /* addr[1] = I/O Expander N B */
    },
    {6,  IOEXP_TYPE_MAGINOLIA_7AB, { {8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 7 A */
                                     {8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xf0, 0x33}, }, }, /* addr[1] = I/O Expander 7 B */
    },
};

struct inv_port_layout_s magnolia_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / BCM_CHIP_TYPE / LANE_ID */
    { 0,  10,  0,  0, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 16} },
    { 1,  11,  0,  1, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 15} },
    { 2,  12,  0,  2, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 14} },
    { 3,  13,  0,  3, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 13} },
    { 4,  14,  0,  4, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 24} },
    { 5,  15,  0,  5, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 23} },
    { 6,  16,  0,  6, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 22} },
    { 7,  17,  0,  7, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 21} },
    { 8,  18,  1,  0, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 28} },
    { 9,  19,  1,  1, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 27} },
    {10,  20,  1,  2, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 26} },
    {11,  21,  1,  3, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 25} },
    {12,  22,  1,  4, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 32} },
    {13,  23,  1,  5, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 31} },
    {14,  24,  1,  6, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 30} },
    {15,  25,  1,  7, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 29} },
    {16,  26,  2,  0, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 48} },
    {17,  27,  2,  1, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 47} },
    {18,  28,  2,  2, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 46} },
    {19,  29,  2,  3, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 45} },
    {20,  30,  2,  4, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 52} },
    {21,  31,  2,  5, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 51} },
    {22,  32,  2,  6, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 50} },
    {23,  33,  2,  7, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 49} },
    {24,  34,  3,  0, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 56} },
    {25,  35,  3,  1, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 55} },
    {26,  36,  3,  2, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 54} },
    {27,  37,  3,  3, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 53} },
    {28,  38,  3,  4, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 60} },
    {29,  39,  3,  5, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 59} },
    {30,  40,  3,  6, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 58} },
    {31,  41,  3,  7, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 57} },
    {32,  42,  4,  0, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 64} },
    {33,  43,  4,  1, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 63} },
    {34,  44,  4,  2, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 62} },
    {35,  45,  4,  3, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 61} },
    {36,  46,  4,  4, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 68} },
    {37,  47,  4,  5, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 67} },
    {38,  48,  4,  6, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 66} },
    {39,  49,  4,  7, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 65} },
    {40,  50,  5,  0, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 72} },
    {41,  51,  5,  1, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 71} },
    {42,  52,  5,  2, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 70} },
    {43,  53,  5,  3, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 69} },
    {44,  54,  5,  4, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 76} },
    {45,  55,  5,  5, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 75} },
    {46,  56,  5,  6, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 74} },
    {47,  57,  5,  7, TRANSVR_TYPE_SFP,  BCM_CHIP_TYPE_TRIDENT_2, { 73} },
    {48,  58,  6,  0, TRANSVR_TYPE_QSFP, BCM_CHIP_TYPE_TRIDENT_2, { 81, 82, 83, 84} },
    {49,  59,  6,  1, TRANSVR_TYPE_QSFP, BCM_CHIP_TYPE_TRIDENT_2, { 77, 78, 79, 80} },
    {50,  60,  6,  2, TRANSVR_TYPE_QSFP, BCM_CHIP_TYPE_TRIDENT_2, { 97, 98, 99,100} },
    {51,  61,  6,  3, TRANSVR_TYPE_QSFP, BCM_CHIP_TYPE_TRIDENT_2, {101,102,103,104} },
    {52,  62,  6,  4, TRANSVR_TYPE_QSFP, BCM_CHIP_TYPE_TRIDENT_2, {105,106,107,108} },
    {53,  63,  6,  5, TRANSVR_TYPE_QSFP, BCM_CHIP_TYPE_TRIDENT_2, {109,110,111,112} },
};
#endif


/* ==========================================
 *   Redwood Layout configuration
 * ==========================================
 */
#ifdef SWPS_REDWOOD
unsigned redwood_gpio_rest_mux = MUX_RST_GPIO_48_PAC9548;

struct inv_ioexp_layout_s redwood_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_REDWOOD_P01P08, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x25, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
    {1,  IOEXP_TYPE_REDWOOD_P09P16, { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x25, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
    {2,  IOEXP_TYPE_REDWOOD_P01P08, { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x24, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
    {3,  IOEXP_TYPE_REDWOOD_P09P16, { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x24, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
};

struct inv_port_layout_s redwood_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / BCM_CHIP_TYPE / LANE_ID */
    { 0,  22,  0,  0, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {  1,  2,  3,  4} },
    { 1,  23,  0,  1, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {  5,  6,  7,  8} },
    { 2,  24,  0,  2, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {  9, 10, 11, 12} },
    { 3,  25,  0,  3, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 13, 14, 15, 16} },
    { 4,  26,  0,  4, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 17, 18, 19, 20} },
    { 5,  27,  0,  5, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 21, 22, 23, 24} },
    { 6,  28,  0,  6, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 25, 26, 27, 28} },
    { 7,  29,  0,  7, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 29, 30, 31, 32} },
    { 8,  30,  1,  0, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 33, 34, 35, 36} },
    { 9,  31,  1,  1, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 37, 38, 39, 40} },
    {10,  32,  1,  2, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 41, 42, 43, 44} },
    {11,  33,  1,  3, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 45, 46, 47, 48} },
    {12,  34,  1,  4, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 49, 50, 51, 52} },
    {13,  35,  1,  5, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 53, 54, 55, 56} },
    {14,  36,  1,  6, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 57, 58, 59, 60} },
    {15,  37,  1,  7, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 61, 62, 63, 64} },
    {16,   6,  2,  0, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 65, 66, 67, 68} },
    {17,   7,  2,  1, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 69, 70, 71, 72} },
    {18,   8,  2,  2, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 73, 74, 75, 76} },
    {19,   9,  2,  3, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 77, 78, 79, 80} },
    {20,  10,  2,  4, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 81, 82, 83, 84} },
    {21,  11,  2,  5, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 85, 86, 87, 88} },
    {22,  12,  2,  6, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 89, 90, 91, 92} },
    {23,  13,  2,  7, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 93, 94, 95, 96} },
    {24,  14,  3,  0, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 97, 98, 99,100} },
    {25,  15,  3,  1, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {101,102,103,104} },
    {26,  16,  3,  2, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {105,106,107,108} },
    {27,  17,  3,  3, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {109,110,111,112} },
    {28,  18,  3,  4, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {113,114,115,116} },
    {29,  19,  3,  5, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {117,118,119,120} },
    {30,  20,  3,  6, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {121,122,123,124} },
    {31,  21,  3,  7, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {125,126,127,128} },
};
#endif


/* ==========================================
 *   Hudson32i Layout configuration
 * ==========================================
 */
#ifdef SWPS_HUDSON32I_GA
unsigned hudsin32iga_gpio_rest_mux = MUX_RST_GPIO_48_PAC9548;

struct inv_ioexp_layout_s hudson32iga_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_HUDSON32IGA_P01P08, { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander A    */
                                          {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander B    */
                                          {0, 0x24, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0x24 */
    },
    {1,  IOEXP_TYPE_HUDSON32IGA_P09P16, { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander A    */
                                          {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander B    */
                                          {0, 0x24, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0x24 */
    },
    {2,  IOEXP_TYPE_HUDSON32IGA_P01P08, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander A    */
                                          {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander B    */
                                          {0, 0x25, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0x25 */
    },
    {3,  IOEXP_TYPE_HUDSON32IGA_P09P16, { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander A    */
                                          {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander B    */
                                          {0, 0x25, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0x25 */
    },
};

struct inv_port_layout_s hudson32iga_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / BCM_CHIP_TYPE / LANE_ID */
    { 0,   6,  0,  0, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, {  1,  2,  3,  4} },
    { 1,   7,  0,  1, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, {  5,  6,  7,  8} },
    { 2,   8,  0,  2, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, {  9, 10, 11, 12} },
    { 3,   9,  0,  3, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 13, 14, 15, 16} },
    { 4,  10,  0,  4, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 17, 18, 19, 20} },
    { 5,  11,  0,  5, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 21, 22, 23, 24} },
    { 6,  12,  0,  6, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 25, 26, 27, 28} },
    { 7,  13,  0,  7, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 29, 30, 31, 32} },
    { 8,  14,  1,  0, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 33, 34, 35, 36} },
    { 9,  15,  1,  1, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 37, 38, 39, 40} },
    {10,  16,  1,  2, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 41, 42, 43, 44} },
    {11,  17,  1,  3, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 45, 46, 47, 48} },
    {12,  18,  1,  4, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 49, 50, 51, 52} },
    {13,  19,  1,  5, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 53, 54, 55, 56} },
    {14,  20,  1,  6, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 57, 58, 59, 60} },
    {15,  21,  1,  7, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 61, 62, 63, 64} },
    {16,  22,  2,  0, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 65, 66, 67, 68} },
    {17,  23,  2,  1, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 69, 70, 71, 72} },
    {18,  24,  2,  2, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 73, 74, 75, 76} },
    {19,  25,  2,  3, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 77, 78, 79, 80} },
    {20,  26,  2,  4, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 81, 82, 83, 84} },
    {21,  27,  2,  5, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 85, 86, 87, 88} },
    {22,  28,  2,  6, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 89, 90, 91, 92} },
    {23,  29,  2,  7, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 93, 94, 95, 96} },
    {24,  30,  3,  0, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 97, 98, 99,100} },
    {25,  31,  3,  1, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, {101,102,103,104} },
    {26,  32,  3,  2, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, {105,106,107,108} },
    {27,  33,  3,  3, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, {109,110,111,112} },
    {28,  34,  3,  4, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, {113,114,115,116} },
    {29,  35,  3,  5, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, {117,118,119,120} },
    {30,  36,  3,  6, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, {121,122,123,124} },
    {31,  37,  3,  7, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, {125,126,127,128} },
};
#endif


/* ==========================================
 *   Spruce Layout configuration
 * ==========================================
 */
#ifdef SWPS_SPRUCE
unsigned spruce_gpio_rest_mux = MUX_RST_GPIO_48_PAC9548;

struct inv_ioexp_layout_s spruce_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_SPRUCE_7AB,  { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 7A  */
                                   {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xf0, 0x33}, }, }, /* addr[2] = I/O Expander 7B  */
    },
};

struct inv_port_layout_s spruce_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / BCM_CHIP_TYPE / LANE_ID */
    { 0,   6,  0,  0, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 81, 82, 83, 84} },
    { 1,   7,  0,  1, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 77, 78, 79, 80} },
    { 2,   8,  0,  2, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, { 97, 98, 99,100} },
    { 3,   9,  0,  3, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, {101,102,103,104} },
    { 4,  10,  0,  4, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, {105,106,107,108} },
    { 5,  11,  0,  5, TRANSVR_TYPE_QSFP_PLUS, BCM_CHIP_TYPE_TRIDENT_2, {109,110,111,112} },
};
#endif


/* ==========================================
 *   Cypress Layout configuration (Inventec version [Up->Down])
 * ==========================================
 */
#ifdef SWPS_CYPRESS_GA1
unsigned cypress_ga1_gpio_rest_mux = MUX_RST_GPIO_69_PAC9548;

struct inv_ioexp_layout_s cypress_ga1_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_CYPRESS_NABC,  { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                     {2, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {1,  IOEXP_TYPE_CYPRESS_NABC,  { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                     {3, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {2,  IOEXP_TYPE_CYPRESS_NABC,  { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                     {4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {3,  IOEXP_TYPE_CYPRESS_NABC,  { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                     {5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {4,  IOEXP_TYPE_CYPRESS_NABC,  { {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                     {6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {5,  IOEXP_TYPE_CYPRESS_NABC,  { {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                     {7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {6,  IOEXP_TYPE_CYPRESS_7ABC,  { {8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xc0, 0xc0}, },    /* addr[0] = I/O Expander 7 A */
                                     {8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xc0, 0xc0}, {0xff, 0xc0}, },    /* addr[1] = I/O Expander 7 B */
                                     {8, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 7 C */
    },
};

struct inv_port_layout_s cypress_ga1_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / BCM_CHIP_TYPE / LANE_ID */
    { 0,  10,  0,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  1} },
    { 1,  11,  0,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  2} },
    { 2,  12,  0,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  3} },
    { 3,  13,  0,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  4} },
    { 4,  14,  0,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  5} },
    { 5,  15,  0,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  6} },
    { 6,  16,  0,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  7} },
    { 7,  17,  0,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  8} },
    { 8,  18,  1,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  9} },
    { 9,  19,  1,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 10} },
    {10,  20,  1,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 11} },
    {11,  21,  1,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 12} },
    {12,  22,  1,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 21} },
    {13,  23,  1,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 22} },
    {14,  24,  1,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 23} },
    {15,  25,  1,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 24} },
    {16,  26,  2,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 33} },
    {17,  27,  2,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 34} },
    {18,  28,  2,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 35} },
    {19,  29,  2,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 36} },
    {20,  30,  2,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 37} },
    {21,  31,  2,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 38} },
    {22,  32,  2,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 39} },
    {23,  33,  2,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 40} },
    {24,  34,  3,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 41} },
    {25,  35,  3,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 42} },
    {26,  36,  3,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 43} },
    {27,  37,  3,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 44} },
    {28,  38,  3,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 49} },
    {29,  39,  3,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 50} },
    {30,  40,  3,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 51} },
    {31,  41,  3,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 52} },
    {32,  42,  4,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 53} },
    {33,  43,  4,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 54} },
    {34,  44,  4,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 55} },
    {35,  45,  4,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 56} },
    {36,  46,  4,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 65} },
    {37,  47,  4,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 66} },
    {38,  48,  4,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 67} },
    {39,  49,  4,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 68} },
    {40,  50,  5,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 69} },
    {41,  51,  5,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 70} },
    {42,  52,  5,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 71} },
    {43,  53,  5,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 72} },
    {44,  54,  5,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 81} },
    {45,  55,  5,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 82} },
    {46,  56,  5,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 83} },
    {47,  57,  5,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 84} },
    {48,  58,  6,  0, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 97, 98, 99,100} },
    {49,  59,  6,  1, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 85, 86, 87, 88} },
    {50,  60,  6,  2, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {101,102,103,104} },
    {51,  61,  6,  3, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {105,106,107,108} },
    {52,  62,  6,  4, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {109,110,111,112} },
    {53,  63,  6,  5, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {117,118,119,120} },
};
#endif


/* ==========================================
 *   Cypress Layout configuration (Inventec version [Down->Up])
 * ==========================================
 */
#ifdef SWPS_CYPRESS_GA2
unsigned cypress_ga2_gpio_rest_mux = MUX_RST_GPIO_FORCE_HEDERA;

struct inv_ioexp_layout_s cypress_ga2_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_CYPRESS_NABC,  { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                     {2, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {1,  IOEXP_TYPE_CYPRESS_NABC,  { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                     {3, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {2,  IOEXP_TYPE_CYPRESS_NABC,  { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                     {4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {3,  IOEXP_TYPE_CYPRESS_NABC,  { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                     {5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {4,  IOEXP_TYPE_CYPRESS_NABC,  { {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                     {6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {5,  IOEXP_TYPE_CYPRESS_NABC,  { {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                     {7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {6,  IOEXP_TYPE_CYPRESS_7ABC,  { {8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xc0, 0xc0}, },    /* addr[0] = I/O Expander 7 A */
                                     {8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xc0, 0xc0}, {0xff, 0xc0}, },    /* addr[1] = I/O Expander 7 B */
                                     {8, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 7 C */
    },
};

struct inv_port_layout_s cypress_ga2_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / BCM_CHIP_TYPE / LANE_ID */
    { 0,  11,  0,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  2} },
    { 1,  10,  0,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  1} },
    { 2,  13,  0,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  4} },
    { 3,  12,  0,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  3} },
    { 4,  15,  0,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  6} },
    { 5,  14,  0,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  5} },
    { 6,  17,  0,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  8} },
    { 7,  16,  0,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  7} },
    { 8,  19,  1,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 10} },
    { 9,  18,  1,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, {  9} },
    {10,  21,  1,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 12} },
    {11,  20,  1,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 11} },
    {12,  23,  1,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 22} },
    {13,  22,  1,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 21} },
    {14,  25,  1,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 24} },
    {15,  24,  1,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 23} },
    {16,  27,  2,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 34} },
    {17,  26,  2,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 33} },
    {18,  29,  2,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 36} },
    {19,  28,  2,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 35} },
    {20,  31,  2,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 38} },
    {21,  30,  2,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 37} },
    {22,  33,  2,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 40} },
    {23,  32,  2,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 39} },
    {24,  35,  3,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 42} },
    {25,  34,  3,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 41} },
    {26,  37,  3,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 44} },
    {27,  36,  3,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 43} },
    {28,  39,  3,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 50} },
    {29,  38,  3,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 49} },
    {30,  41,  3,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 52} },
    {31,  40,  3,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 51} },
    {32,  43,  4,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 54} },
    {33,  42,  4,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 53} },
    {34,  45,  4,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 56} },
    {35,  44,  4,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 55} },
    {36,  47,  4,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 66} },
    {37,  46,  4,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 65} },
    {38,  49,  4,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 68} },
    {39,  48,  4,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 67} },
    {40,  51,  5,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 70} },
    {41,  50,  5,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 69} },
    {42,  53,  5,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 72} },
    {43,  52,  5,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 71} },
    {44,  55,  5,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 82} },
    {45,  54,  5,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 81} },
    {46,  57,  5,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 84} },
    {47,  56,  5,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TOMAHAWK, { 83} },
    {48,  59,  6,  1, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 85, 86, 87, 88} },
    {49,  58,  6,  0, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 97, 98, 99,100} },
    {50,  61,  6,  3, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {105,106,107,108} },
    {51,  60,  6,  2, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {101,102,103,104} },
    {52,  63,  6,  5, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {117,118,119,120} },
    {53,  62,  6,  4, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {109,110,111,112} },
};
#endif



/* ==========================================
 *   Redwood_fsl Layout configuration
 * ==========================================
 */
#ifdef SWPS_REDWOOD_FSL
unsigned redwood_fsl_gpio_rest_mux = MUX_RST_GPIO_48_PAC9548;

struct inv_ioexp_layout_s redwood_fsl_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_REDWOOD_P01P08, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x25, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
    {1,  IOEXP_TYPE_REDWOOD_P09P16, { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x25, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
    {2,  IOEXP_TYPE_REDWOOD_P01P08, { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x24, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
    {3,  IOEXP_TYPE_REDWOOD_P09P16, { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x24, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
};


struct inv_port_layout_s redwood_fsl_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / BCM_CHIP_TYPE / LANE_ID */
    { 0,  22,  0,  0, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {  1,  2,  3,  4} },
    { 1,  23,  0,  1, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {  5,  6,  7,  8} },
    { 2,  24,  0,  2, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {  9, 10, 11, 12} },
    { 3,  25,  0,  3, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 13, 14, 15, 16} },
    { 4,  26,  0,  4, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 17, 18, 19, 20} },
    { 5,  27,  0,  5, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 21, 22, 23, 24} },
    { 6,  28,  0,  6, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 25, 26, 27, 28} },
    { 7,  29,  0,  7, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 29, 30, 31, 32} },
    { 8,  30,  1,  0, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 33, 34, 35, 36} },
    { 9,  31,  1,  1, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 37, 38, 39, 40} },
    {10,  32,  1,  2, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 41, 42, 43, 44} },
    {11,  33,  1,  3, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 45, 46, 47, 48} },
    {12,  34,  1,  4, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 49, 50, 51, 52} },
    {13,  35,  1,  5, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 53, 54, 55, 56} },
    {14,  36,  1,  6, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 57, 58, 59, 60} },
    {15,  37,  1,  7, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 61, 62, 63, 64} },
    {16,   6,  2,  0, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 65, 66, 67, 68} },
    {17,   7,  2,  1, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 69, 70, 71, 72} },
    {18,   8,  2,  2, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 73, 74, 75, 76} },
    {19,   9,  2,  3, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 77, 78, 79, 80} },
    {20,  10,  2,  4, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 81, 82, 83, 84} },
    {21,  11,  2,  5, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 85, 86, 87, 88} },
    {22,  12,  2,  6, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 89, 90, 91, 92} },
    {23,  13,  2,  7, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 93, 94, 95, 96} },
    {24,  14,  3,  0, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, { 97, 98, 99,100} },
    {25,  15,  3,  1, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {101,102,103,104} },
    {26,  16,  3,  2, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {105,106,107,108} },
    {27,  17,  3,  3, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {109,110,111,112} },
    {28,  18,  3,  4, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {113,114,115,116} },
    {29,  19,  3,  5, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {117,118,119,120} },
    {30,  20,  3,  6, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {121,122,123,124} },
    {31,  21,  3,  7, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TOMAHAWK, {125,126,127,128} },
};
#endif


/* ==========================================
 *   Lavender Layout configuration
 * ==========================================
 */
#ifdef SWPS_LAVENDER
unsigned lavender_gpio_rest_mux = MUX_RST_GPIO_69_PAC9548;

struct inv_ioexp_layout_s lavender_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_LAVENDER_P01P08, { {1, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 1-4 A */
                                       {1, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 1-4 B */
                                       {1, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, },    /* addr[2] = I/O Expander 1-4 C */
                                     },
    },
    {1,  IOEXP_TYPE_LAVENDER_P09P16, { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 1-4 A */
                                       {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 1-4 B */
                                       {2, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, },    /* addr[2] = I/O Expander 1-4 C */
                                     },
    },
    {2,  IOEXP_TYPE_LAVENDER_P01P08, { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 1-4 A */
                                       {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 1-4 B */
                                       {3, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, },    /* addr[2] = I/O Expander 1-4 C */
                                     },
    },
    {3,  IOEXP_TYPE_LAVENDER_P09P16, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 1-4 A */
                                       {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 1-4 B */
                                       {4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, },    /* addr[2] = I/O Expander 1-4 C */
                                     },
    },
    {4,  IOEXP_TYPE_LAVENDER_P01P08, { {9, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 1-4 A */
                                       {9, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 1-4 B */
                                       {9, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, },    /* addr[2] = I/O Expander 1-4 C */
                                     },
    },
    {5,  IOEXP_TYPE_LAVENDER_P09P16, { {10, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 1-4 A */
                                       {10, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 1-4 B */
                                       {10, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, },    /* addr[2] = I/O Expander 1-4 C */
                                     },
    },
    {6,  IOEXP_TYPE_LAVENDER_P01P08, { {11, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 1-4 A */
                                       {11, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 1-4 B */
                                       {11, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, },    /* addr[2] = I/O Expander 1-4 C */
                                     },
    },
    {7,  IOEXP_TYPE_LAVENDER_P09P16, { {12, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 1-4 A */
                                       {12, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 1-4 B */
                                       {12, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, },    /* addr[2] = I/O Expander 1-4 C */
                                     },
    },
    {8,  IOEXP_TYPE_LAVENDER_P65,    { {5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xf6, 0xff}, {0xf8, 0xff}, },    /* addr[0] = I/O Expander */
                                     },
    },
};

struct inv_port_layout_s lavender_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / BCM_CHIP_TYPE / LANE_ID */
    { 0,  17,  0,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {  1,  2,  3,  4} },
    { 1,  18,  0,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {  5,  6,  7,  8} },
    { 2,  19,  0,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {  9, 10, 11, 12} },
    { 3,  20,  0,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 13, 14, 15, 16} },
    { 4,  21,  0,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 17, 18, 19, 20} },
    { 5,  22,  0,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 21, 22, 23, 24} },
    { 6,  23,  0,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 25, 26, 27, 28} },
    { 7,  24,  0,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 29, 30, 31, 32} },
    { 8,  25,  1,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 33, 34, 35, 36} },
    { 9,  26,  1,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 37, 38, 39, 40} },
    {10,  27,  1,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 41, 42, 43, 44} },
    {11,  28,  1,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 45, 46, 47, 48} },
    {12,  29,  1,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 49, 50, 51, 52} },
    {13,  30,  1,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 53, 54, 55, 56} },
    {14,  31,  1,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 57, 58, 59, 60} },
    {15,  32,  1,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 61, 62, 63, 64} },
    {16,  33,  2,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 65, 66, 67, 68} },
    {17,  34,  2,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 69, 70, 71, 72} },
    {18,  35,  2,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 73, 74, 75, 76} },
    {19,  36,  2,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 77, 78, 79, 80} },
    {20,  37,  2,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 81, 82, 83, 84} },
    {21,  38,  2,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 85, 86, 87, 88} },
    {22,  39,  2,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 89, 90, 91, 92} },
    {23,  40,  2,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 93, 94, 95, 96} },
    {24,  41,  3,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, { 97, 98, 99,100} },
    {25,  42,  3,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {101,102,103,104} },
    {26,  43,  3,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {105,106,107,108} },
    {27,  44,  3,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {109,110,111,112} },
    {28,  45,  3,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {113,114,115,116} },
    {29,  46,  3,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {117,118,119,120} },
    {30,  47,  3,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {121,122,123,124} },
    {31,  48,  3,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {125,126,127,128} },

    {32,  49,  4,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {129,130,131,132} },
    {33,  50,  4,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {133,134,135,136} },
    {34,  51,  4,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {137,138,139,140} },
    {35,  52,  4,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {141,142,143,144} },
    {36,  53,  4,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {145,146,147,148} },
    {37,  54,  4,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {149,150,151,152} },
    {38,  55,  4,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {153,154,155,156} },
    {39,  56,  4,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {157,158,159,160} },

    {40,  57,  5,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {161,162,163,164} },
    {41,  58,  5,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {165,166,167,168} },
    {42,  59,  5,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {169,170,171,172} },
    {43,  60,  5,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {173,174,175,176} },
    {44,  61,  5,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {177,178,179,180} },
    {45,  62,  5,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {181,182,183,184} },
    {46,  63,  5,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {185,186,187,188} },
    {47,  64,  5,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {189,190,191,192} },

    {48,  65,  6,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {193,194,195,196} },
    {49,  66,  6,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {197,198,199,200} },
    {50,  67,  6,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {201,202,203,204} },
    {51,  68,  6,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {205,206,207,208} },
    {52,  69,  6,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {209,210,211,212} },
    {53,  70,  6,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {213,214,215,216} },
    {54,  71,  6,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {217,218,219,220} },
    {55,  72,  6,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {221,222,223,224} },

    {56,  73,  7,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {225,226,227,228} },
    {57,  74,  7,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {229,230,231,232} },
    {58,  75,  7,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {233,234,235,236} },
    {59,  76,  7,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {237,238,239,240} },
    {60,  77,  7,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {241,242,243,244} },
    {61,  78,  7,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {245,246,247,248} },
    {62,  79,  7,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {249,250,251,252} },
    {63,  80,  7,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {253,254,255,256} },
    {64,   5,  8,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_D5264Q28B, {257,258,259,260} },
};
#endif


#endif /* INV_SWPS_H */



