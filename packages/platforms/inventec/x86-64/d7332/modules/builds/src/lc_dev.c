#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include "sff.h"

extern u32 logLevel;
#if 0
typedef enum {
    LC_UNKNOWN_TYPE = 0,
    LC_100G_TYPE,
    LC_400G_TYPE,
    LC_TYPE_NUM,
} lc_type_t;

typedef enum {
    LC_LED_CTRL_OFF = 0,
    LC_LED_CTRL_GREEN_ON,
    LC_LED_CTRL_RED_ON,
    LC_LED_CTRL_AMBER_ON,
    LC_LED_CTRL_GREEN_BLINK,
    LC_LED_CTRL_RED_BLINK,
    LC_LED_CTRL_AMBER_BLINK,
    LC_LED_CTRL_NUM,
} lc_led_ctrl_t;
#endif
static const char *lc_led_str[LC_LED_CTRL_NUM] = {

    [LC_LED_CTRL_GREEN_ON] = "LC_LED_GREEN_ON",
    [LC_LED_CTRL_RED_ON] = "LC_LED_RED_ON",
};
const char *lc_type_name[LC_TYPE_NUM] = 
{
    "UNKNOWN_TYPE",
    "LC_100G",
    "LC_400G",
};

enum {
    LC_100G_ID = 0x1,
    LC_400G_ID,
};

#define LC_DEV_LOG_ERR(fmt, args...) \
    do { \
        if (logLevel & DEV_ERR_LEV) \
        { \
            printk (KERN_ERR "[LC_DEV]%s:"fmt,__FUNCTION__,  ##args); \
        } \
    } while (0)

#define LC_DEV_LOG_INFO(fmt, args...) \
    do { \
        if (logLevel & DEV_INFO_LEV) \
        { \
            printk (KERN_INFO "[LC_DEV]%s:"fmt,__FUNCTION__,  ##args); \
        } \
    } while (0)

#define LC_DEV_LOG_DBG(fmt, args...) \
    do { \
        if (logLevel & DEV_DBG_LEV) \
        { \
            printk (KERN_INFO "[LC_DEV]%s:"fmt,__FUNCTION__,  ##args); \
        } \
    } while (0)




#define PHY_NUM_PER_LC  (8) 
#define PHY_READY_BITMASK ((1L << PHY_NUM_PER_LC) - 1)

#define CARD_NUM (4)
#define PORT_GROUP_NUM (2) /*per lc_id*/
#define TMP_DEV_NUM (2)    /*per lc_id*/
#define MUX_L2_NUM (4)     /*per lc_id*/

#define SFF_DATA_VALID_MASK (0x3uL)

#define SWITCH_CPLD_ADDR (0x33)
#define CPLD1_ADDR (0x33)
#define CPLD2_ADDR  (0x66)
#define IOEXP1_ADDR (0x20)
#define IOEXP2_ADDR (0x21)
#define TMP1_ADDR (0x4a)
#define TMP2_ADDR (0x4d)
/*pca9546*/
#define MUX_L1_ADDR (0x71)
/*pca9548*/
#define MUX_L2_ADDR (0x72)
#define PCA9548_CONTROL_OFFSET (0)

/*INT gpio*/
#define LC_INT_GPIO (12)
#define LC1_SFF_INT_GPIO (44)
#define LC2_SFF_INT_GPIO (61)
#define LC3_SFF_INT_GPIO (26)
#define LC4_SFF_INT_GPIO (27)

/*io expander*/
#define IOEXP_INPUT_OFFSET (0x0)

/*switch cpld*/
#define LC1_ST_OFFSET (0x38)
#define LC2_ST_OFFSET (0x3a)
#define LC3_ST_OFFSET (0x3c)
#define LC4_ST_OFFSET (0x3e)

#define LC_TEMP_INT_N_BIT (6)
#define LC_PRESENT_N_BIT (3)
#define LC_MUX_INT_N_BIT (2)

#define SWITCH_LC1_CONTROL_OFFSET (0x39)
#define SWITCH_LC2_CONTROL_OFFSET (0x3b)
#define SWITCH_LC3_CONTROL_OFFSET (0x3d)
#define SWITCH_LC4_CONTROL_OFFSET (0x3f)

#define SWITCH_LC_RESET_N_BIT (2) /*use this bit to reset line card*/
#define SWITCH_LC_PWR_CONTROL_BIT (1)
#define SWITCH_LC_CPU_PLTRST_N_BIT (0)

/*line lc_id cpld reg offset*/
#define PCB_VER_OFFSET  (0x0)
#define LC_ID_BIT_S   (4)
#define LC_ID_BIT_NUM   (4)

#define I2C_MUX_RST_OFFSET (0x0b)

#define RST_PCA9548_D_OFFSET (0x0b)

#define RST_PCA9548_1_N_BIT (0)
#define RST_PCA9548_2_N_BIT (1)
#define RST_PCA9548_3_N_BIT (2)
#define RST_PCA9548_4_N_BIT (3)
#define RST_PCA9546_N_BIT (4)

#define CPLD_PRS_OFFSET  (0x0)
#define INT_OFFSET  (0x0)
#define OC_OFFSET   (0x21)

#define LC_ST_LED_OFFSET (0x0d)
#define RESET_OFFSET   (0x12)
#define LPMODE_OFFSET   (0x18)
#define POWER_OFFSET    (0x1f)

/*lc power status check*/
#define DC_DC_ST1_OFFSET (0x1d)
#define DC_DC_ST2_OFFSET (0x1e)

/*cpld1 lc power control*/
#define MISC_CONTROL1_OFFSET (0x27)
#define UCD_PMBUS_CNTRL_R_BIT (3)

#define MISC_CONTROL2_OFFSET (0x28)
#define LC1_PWR_CONTROL_BIT (1)

/*cpld transceiver interrupt*/
#define SFF_INT_ST_OFFSET (0x2a)
#define SFF_INT_CPLD2_PRS_BIT (0)
#define SFF_INT_CPLD2_INT_BIT (1)
#define SFF_INT_CPLD2_OC_BIT (2)
#define SFF_INT_CPLD1_PRS_BIT (3)
#define SFF_INT_CPLD1_INT_BIT (4)
#define SFF_INT_CPLD1_OC_BIT (5)
#define SFF_INT_NUM (6)

#define PORT_IDX_NUM (16)
#define U16_MASK (((1 << PORT_IDX_NUM) - 1))
/*for register <= 32 bits*/
#define inv_set_bit(bit, reg)  ((reg) | (1 << bit))
#define inv_clear_bit(bit, reg)  ((reg) & (~(1 << bit)))

/*side band signaling output default value*/
#define LC_CPLD_DEF_RESET_VAL   ((1 << PORT_IDX_NUM) - 1)
#define LC_CPLD_DEF_LPMODE_VAL   ((1 << PORT_IDX_NUM) - 1)
#define LC_CPLD_DEF_POWER_VAL   ((1 << PORT_IDX_NUM) - 1)

typedef enum {
    LC_SWITCH_CH_ID,
    LC1_SFF_CH_ID,
    LC2_SFF_CH_ID,
    LC3_SFF_CH_ID,
    LC4_SFF_CH_ID,
    LC1_TMP_CH_ID,
    LC2_TMP_CH_ID,
    LC3_TMP_CH_ID,
    LC4_TMP_CH_ID,
    LC1_MUX_L1_CH_ID,
    LC1_MUX_L2_1_CH_ID,
    LC1_MUX_L2_2_CH_ID,
    LC1_MUX_L2_3_CH_ID,
    LC1_MUX_L2_4_CH_ID,
    LC2_MUX_L1_CH_ID,
    LC2_MUX_L2_1_CH_ID,
    LC2_MUX_L2_2_CH_ID,
    LC2_MUX_L2_3_CH_ID,
    LC2_MUX_L2_4_CH_ID,
    LC3_MUX_L1_CH_ID,
    LC3_MUX_L2_1_CH_ID,
    LC3_MUX_L2_2_CH_ID,
    LC3_MUX_L2_3_CH_ID,
    LC3_MUX_L2_4_CH_ID,
    LC4_MUX_L1_CH_ID,
    LC4_MUX_L2_1_CH_ID,
    LC4_MUX_L2_2_CH_ID,
    LC4_MUX_L2_3_CH_ID,
    LC4_MUX_L2_4_CH_ID,
    LC_CH_ID_NUM,
} lc_ch_id_t;

int lcChTbl[LC_CH_ID_NUM] = {
    [LC_SWITCH_CH_ID] = 41,
    [LC1_SFF_CH_ID] = 6,
    [LC2_SFF_CH_ID] = 7,
    [LC3_SFF_CH_ID] = 8,
    [LC4_SFF_CH_ID] = 9,
    [LC1_TMP_CH_ID] = 50,
    [LC2_TMP_CH_ID] = 51,
    [LC4_TMP_CH_ID] = 52,
    [LC3_TMP_CH_ID] = 53,
    [LC1_MUX_L1_CH_ID] = 81,
    [LC1_MUX_L2_1_CH_ID] = 85,
    [LC1_MUX_L2_2_CH_ID] = 86,
    [LC1_MUX_L2_3_CH_ID] = 87,
    [LC1_MUX_L2_4_CH_ID] = 88,
    [LC2_MUX_L1_CH_ID] = 82,
    [LC2_MUX_L2_1_CH_ID] = 89,
    [LC2_MUX_L2_2_CH_ID] = 90,
    [LC2_MUX_L2_3_CH_ID] = 91,
    [LC2_MUX_L2_4_CH_ID] = 92,
    [LC3_MUX_L1_CH_ID] = 83,
    [LC3_MUX_L2_1_CH_ID] = 93,
    [LC3_MUX_L2_2_CH_ID] = 94,
    [LC3_MUX_L2_3_CH_ID] = 95,
    [LC3_MUX_L2_4_CH_ID] = 96,
    [LC4_MUX_L1_CH_ID] = 84,
    [LC4_MUX_L2_1_CH_ID] = 97,
    [LC4_MUX_L2_2_CH_ID] = 98,
    [LC4_MUX_L2_3_CH_ID] = 99,
    [LC4_MUX_L2_4_CH_ID] = 100,
};
#define LM75_REG_TEMP       (0x00)
#define LM75_REG_CONF       (0x01)
#define LM75_REG_LOW       (0x02)
#define LM75_REG_HIGH        (0x03)
#define LM75_RESOLUTION     (12)

#define TEMP_CHECK_NUM (5)

typedef enum {
    TEMP_UNKNOWN,
    TEMP_INPUT,
    TEMP_TH_L,
    TEMP_TH_H,
} temp_conf_type_t;

struct lc_client_t {

    struct mutex lock;
    struct i2c_client *client;
};

struct lc_config_t {
    int lc_ch_id;
    u8 addr;
};

typedef union {

    u16 reg[4];
    unsigned long data;
} u64_format_t;

struct io_data_t {

    u32 buf;
    u8 valid;
};
typedef enum {
    LC_ST_PRS_TYPE,
    LC_ST_OVER_TEMP_TYPE,
    LC_ST_TYPE_NUM,

} lc_st_type_t;
static const char *lc_st_name[LC_ST_TYPE_NUM] = {
    [LC_ST_PRS_TYPE] = "LC_ST_PRS_TYPE",
    [LC_ST_OVER_TEMP_TYPE] = "LC_ST_OVER_TEMP_TYPE",
};
typedef enum {
    SFF_PRS_TYPE,
    SFF_INT_TYPE,
    SFF_OC_TYPE,
    SFF_INPUT_TYPE_NUM,

} sff_input_type_t;

const char *sff_input_name[SFF_INPUT_TYPE_NUM] = {
    "SFF_PRS_TYPE",
    "SFF_INT_TYPE",
    "SFF_OC_TYPE",
};
typedef enum {
    SFF_LPMODE_TYPE,
    SFF_RESET_TYPE,
    SFF_POWER_TYPE,
    SFF_OUTPUT_TYPE_NUM,

} lc_sff_output_type_t;
const char *lc_sff_output_name[SFF_OUTPUT_TYPE_NUM] = {
    "SFF_LPMODE_TYPE",
    "SFF_RESET_TYPE",
    "SFF_POWER_TYPE",
};

u8 lc_sff_input_reg[SFF_INPUT_TYPE_NUM] = {
    [SFF_PRS_TYPE] = IOEXP_INPUT_OFFSET,/*use ioexp for temp  not PRS_OFFSET,*/
    [SFF_INT_TYPE] = INT_OFFSET,
    [SFF_OC_TYPE] = OC_OFFSET
};
u8 lc_sff_output_reg[SFF_OUTPUT_TYPE_NUM] = {
    [SFF_LPMODE_TYPE] = LPMODE_OFFSET,
    [SFF_RESET_TYPE] = RESET_OFFSET,
    [SFF_POWER_TYPE] = POWER_OFFSET,
};
u16 lc_sff_output_def[SFF_OUTPUT_TYPE_NUM] = {
    [SFF_LPMODE_TYPE] = LC_CPLD_DEF_LPMODE_VAL,
    [SFF_RESET_TYPE] = LC_CPLD_DEF_RESET_VAL,
    [SFF_POWER_TYPE] = LC_CPLD_DEF_POWER_VAL,
};
u8 lc_st_reg[CARD_NUM] = {
    LC1_ST_OFFSET,
    LC2_ST_OFFSET,
    LC3_ST_OFFSET,
    LC4_ST_OFFSET
};
u8 lc_control_reg[CARD_NUM] = {
    SWITCH_LC1_CONTROL_OFFSET,
    SWITCH_LC2_CONTROL_OFFSET,
    SWITCH_LC3_CONTROL_OFFSET,
    SWITCH_LC4_CONTROL_OFFSET,
};
int lc_st_bit[LC_ST_TYPE_NUM] = {
    [LC_ST_PRS_TYPE] = LC_PRESENT_N_BIT,
    [LC_ST_OVER_TEMP_TYPE] = LC_TEMP_INT_N_BIT,
};

const int rst_mux_l2_bit[MUX_L2_NUM] = {
    RST_PCA9548_1_N_BIT,
    RST_PCA9548_2_N_BIT,
    RST_PCA9548_3_N_BIT,
    RST_PCA9548_4_N_BIT,
};
enum {

    CPLD1_ID = 0,
    CPLD2_ID
};

struct lc_dev_obj_t {
    struct lc_config_t cpld[PORT_GROUP_NUM];
    struct lc_config_t ioexp[PORT_GROUP_NUM];
    struct lc_config_t tmp_dev[TMP_DEV_NUM];
    struct lc_config_t mux_l1;
    struct lc_config_t mux_l2[MUX_L2_NUM];
    struct io_data_t input[SFF_INPUT_TYPE_NUM];
    int sff_intr_gpio;
};
struct ldata_format_t {
    unsigned long bitmap;
    bool valid;
};
struct lc_dev_t {
    struct lc_config_t switch_cpld;
    struct lc_client_t lc_client[LC_CH_ID_NUM];
    struct lc_dev_obj_t obj[CARD_NUM];
    int lc_intr_gpio;
    struct ldata_format_t lc_st[LC_ST_TYPE_NUM];
    int lc_id_num;
};
struct lc_dev_t lcDev;


/*<TBD>*/
struct lc_config_t switch_cpld_config = {
    .lc_ch_id = LC_SWITCH_CH_ID,
    .addr = SWITCH_CPLD_ADDR,
};
const struct lc_config_t cpld_config_tbl[CARD_NUM][PORT_GROUP_NUM] = {

    [0] = {
        { .lc_ch_id = LC1_SFF_CH_ID, .addr = CPLD1_ADDR},
        { .lc_ch_id = LC1_SFF_CH_ID, .addr = CPLD2_ADDR},
    },
    [1] = {
        { .lc_ch_id = LC2_SFF_CH_ID, .addr = CPLD1_ADDR},
        { .lc_ch_id = LC2_SFF_CH_ID, .addr = CPLD2_ADDR},
    },
    [2] = {
        { .lc_ch_id = LC3_SFF_CH_ID, .addr = CPLD1_ADDR},
        { .lc_ch_id = LC3_SFF_CH_ID, .addr = CPLD2_ADDR},
    },
    [3] = {
        { .lc_ch_id = LC4_SFF_CH_ID, .addr = CPLD1_ADDR},
        { .lc_ch_id = LC4_SFF_CH_ID, .addr = CPLD2_ADDR},
    },

};
const struct lc_config_t ioexp_config_tbl[CARD_NUM][PORT_GROUP_NUM] = {

    [0] = {
        { .lc_ch_id = LC1_SFF_CH_ID, .addr = IOEXP1_ADDR},
        { .lc_ch_id = LC1_SFF_CH_ID, .addr = IOEXP2_ADDR},
    },
    [1] = {
        { .lc_ch_id = LC2_SFF_CH_ID, .addr = IOEXP1_ADDR},
        { .lc_ch_id = LC2_SFF_CH_ID, .addr = IOEXP2_ADDR},
    },
    [2] = {
        { .lc_ch_id = LC3_SFF_CH_ID, .addr = IOEXP1_ADDR},
        { .lc_ch_id = LC3_SFF_CH_ID, .addr = IOEXP2_ADDR},
    },
    [3] = {
        { .lc_ch_id = LC4_SFF_CH_ID, .addr = IOEXP1_ADDR},
        { .lc_ch_id = LC4_SFF_CH_ID, .addr = IOEXP2_ADDR},
    },

};
const struct lc_config_t tmp_dev_config_tbl[CARD_NUM][TMP_DEV_NUM] = {

    [0] = {
        { .lc_ch_id = LC1_TMP_CH_ID, .addr = TMP1_ADDR},
        { .lc_ch_id = LC1_TMP_CH_ID, .addr = TMP2_ADDR},
    },
    [1] = {
        { .lc_ch_id = LC2_TMP_CH_ID, .addr = TMP1_ADDR},
        { .lc_ch_id = LC2_TMP_CH_ID, .addr = TMP2_ADDR},
    },
    [2] = {
        { .lc_ch_id = LC3_TMP_CH_ID, .addr = TMP1_ADDR},
        { .lc_ch_id = LC3_TMP_CH_ID, .addr = TMP2_ADDR},
    },
    [3] = {
        { .lc_ch_id = LC4_TMP_CH_ID, .addr = TMP1_ADDR},
        { .lc_ch_id = LC4_TMP_CH_ID, .addr = TMP2_ADDR},
    },
};
const struct lc_config_t mux_l1_config_tbl[CARD_NUM] = {

    [0] = {
        .lc_ch_id = LC1_MUX_L1_CH_ID, .addr = MUX_L1_ADDR,
    },
    [1] = {
        .lc_ch_id = LC2_MUX_L1_CH_ID, .addr = MUX_L1_ADDR,
    },
    [2] = {
        .lc_ch_id = LC2_MUX_L1_CH_ID, .addr = MUX_L1_ADDR,
    },
    [3] = {
        .lc_ch_id = LC2_MUX_L1_CH_ID, .addr = MUX_L1_ADDR,
    },
};
const struct lc_config_t mux_l2_config_tbl[CARD_NUM][MUX_L2_NUM] = {

    [0] = {
        {.lc_ch_id = LC1_MUX_L2_1_CH_ID, .addr = MUX_L2_ADDR},
        {.lc_ch_id = LC1_MUX_L2_2_CH_ID, .addr = MUX_L2_ADDR},
        {.lc_ch_id = LC1_MUX_L2_3_CH_ID, .addr = MUX_L2_ADDR},
        {.lc_ch_id = LC1_MUX_L2_3_CH_ID, .addr = MUX_L2_ADDR},
    },
    [1] = {
        {.lc_ch_id = LC2_MUX_L2_1_CH_ID, .addr = MUX_L2_ADDR},
        {.lc_ch_id = LC2_MUX_L2_2_CH_ID, .addr = MUX_L2_ADDR},
        {.lc_ch_id = LC2_MUX_L2_3_CH_ID, .addr = MUX_L2_ADDR},
        {.lc_ch_id = LC2_MUX_L2_3_CH_ID, .addr = MUX_L2_ADDR},
    },
    [2] = {
        {.lc_ch_id = LC3_MUX_L2_1_CH_ID, .addr = MUX_L2_ADDR},
        {.lc_ch_id = LC3_MUX_L2_2_CH_ID, .addr = MUX_L2_ADDR},
        {.lc_ch_id = LC3_MUX_L2_3_CH_ID, .addr = MUX_L2_ADDR},
        {.lc_ch_id = LC3_MUX_L2_3_CH_ID, .addr = MUX_L2_ADDR},
    },
    [3] = {
        {.lc_ch_id = LC4_MUX_L2_1_CH_ID, .addr = MUX_L2_ADDR},
        {.lc_ch_id = LC4_MUX_L2_2_CH_ID, .addr = MUX_L2_ADDR},
        {.lc_ch_id = LC4_MUX_L2_3_CH_ID, .addr = MUX_L2_ADDR},
        {.lc_ch_id = LC4_MUX_L2_3_CH_ID, .addr = MUX_L2_ADDR},
    },
};
const int sff_intr_gpio[CARD_NUM] = {
    LC1_SFF_INT_GPIO,
    LC2_SFF_INT_GPIO,
    LC3_SFF_INT_GPIO,
    LC4_SFF_INT_GPIO,
};
#if 0
/*function declaration*/
int lc_dev_lc_cpld_init(int lc_id);
int lc_dev_power_set(int lc_id, bool on);
int lc_dev_power_ready(int lc_id, bool *ready);
void lc_dev_phy_ready(unsigned long bitmap, bool *ready);
int lc_dev_prs_get(unsigned long *bitmap);
int lc_dev_overtemp_get(unsigned long *bitmap);
int lc_dev_temp_get(int lc_id, char *buf, int size);
int lc_dev_reset_set(int lc_id, u8 lv);
int lc_dev_reset_get(int lc_id, u8 *lv);
int lc_dev_type_get(int lc_id, lc_type_t *type);
int lc_dev_type_get_text(int lc_id, u8 *buf, int size);
int lc_dev_mux_l1_reset(int lc_id, u8 lv);
int lc_dev_mux_l2_reset(int lc_id, int mux_l2_id, u8 lv);
int lc_dev_led_set(int lc_id, lc_led_ctrl_t ctrl);
int lc_dev_led_st_get(int lc_id, lc_led_ctrl_t *ctrl);
int lc_dev_port_led_set(int lc_id, unsigned long bitmap);
int lc_dev_port_led_get(int lc_id, unsigned long *bitmap);
int lc_dev_init(void);
void lc_dev_deinit(void);
int lc_dev_handler(void);
struct sff_io_driver_t *sff_io_drv_get_lcdev(void);
#endif
/*private function*/
static int lc_sff_lpmode_get(int lc_id, int port, u8 *val);
static int lc_sff_lpmode_set(int lc_id, int port, u8 val);
static int lc_sff_reset_get(int lc_id, int port, u8 *val);
static int lc_sff_reset_set(int lc_id, int port, u8 val);
static int lc_sff_power_get(int lc_id, int port, u8 *val);
static int lc_sff_power_set(int lc_id, int port, u8 val);
static int lc_sff_output_bitmap_set(lc_sff_output_type_t type, int lc_id, int port_grp, u16 val);
static int lc_sff_prs_get(int lc_id, unsigned long *bitmap);
static int lc_sff_intr_get(int lc_id, unsigned long *bitmap);
static int lc_sff_oc_get(int lc_id, unsigned long *bitmap);

static int lc_sff_prs_update_byGroup(int lc_id, int port_grp);
static int lc_sff_intr_update_byGroup(int lc_id, int port_grp);
static int lc_sff_oc_update_byGroup(int lc_id, int port_grp);
static int lc_sff_prs_update_1stGroup(int lc_id);
static int lc_sff_prs_update_2ndGroup(int lc_id);
static int lc_sff_intr_update_1stGroup(int lc_id);
static int lc_sff_intr_update_2ndGroup(int lc_id);
static int lc_sff_oc_update_1stGroup(int lc_id);
static int lc_sff_oc_update_2ndGroup(int lc_id);

static int lc_dummy_rx_los_get(int lc_id, unsigned long *bitmap);
static int lc_dummy_tx_fault_get(int lc_id, unsigned long *bitmap);
static int lc_dummy_tx_disable_get(int lc_id, int port, u8 *val);
static int lc_dummy_tx_disable_set(int lc_id, int port, u8 val);
static int lc_dummy_mode_sel_get(int lc_id, int port, u8 *val);
static int lc_dummy_mode_sel_set(int lc_id, int port, u8 val);

struct sff_io_driver_t lcDevSffIoDrv = {
    .prs_all_get = lc_sff_prs_get,
    .intr_all_get = lc_sff_intr_get,
    .oc_all_get = lc_sff_oc_get,
    .rx_los_all_get = lc_dummy_rx_los_get,
    .tx_fault_all_get = lc_dummy_tx_fault_get,
    .reset_set = lc_sff_reset_set,
    .reset_get = lc_sff_reset_get,
    .power_set = lc_sff_power_set,
    .power_get = lc_sff_power_get,
    .lpmode_set = lc_sff_lpmode_set,
    .lpmode_get = lc_sff_lpmode_get,
    .tx_disable_set = lc_dummy_tx_disable_set,
    .tx_disable_get = lc_dummy_tx_disable_get,
    .mode_sel_set = lc_dummy_mode_sel_set,
    .mode_sel_get = lc_dummy_mode_sel_get,

};

int (*sff_intr_funcs[SFF_INT_NUM])(int lc_id) = {

    [SFF_INT_CPLD2_PRS_BIT] = lc_sff_prs_update_2ndGroup,
    [SFF_INT_CPLD2_INT_BIT] = lc_sff_intr_update_2ndGroup,
    [SFF_INT_CPLD2_OC_BIT] = lc_sff_oc_update_2ndGroup,
    [SFF_INT_CPLD1_PRS_BIT] = lc_sff_prs_update_1stGroup,
    [SFF_INT_CPLD1_INT_BIT] = lc_sff_intr_update_1stGroup,
    [SFF_INT_CPLD1_OC_BIT] = lc_sff_oc_update_1stGroup,
};

struct sff_io_driver_t *sff_io_drv_get_lcdev(void)
{
    return &lcDevSffIoDrv;
}
static bool port_idx_valid(int idx)
{
    if (idx <  PORT_IDX_NUM && idx >= 0) {
        return true;
    } else {
        LC_DEV_LOG_ERR("invalid port_idx:%d\n", idx);
        return false;
    }
}
static bool grp_valid(int port_grp)
{
    if (port_grp < PORT_GROUP_NUM && port_grp >= 0) {
        return true;
    } else {
        LC_DEV_LOG_ERR("invalid port_grp:%d\n", port_grp);
        return false;
    }
}
static bool lc_id_valid(int lc_id)
{
    if (lc_id < CARD_NUM && lc_id >= 0) {
        return true;
    } else {
        LC_DEV_LOG_ERR("invalid lc_id:%d\n", lc_id);
        return false;
    }
}

static void config_load(void)
{
    int lc_id = 0;
    int port_grp = 0;
    int tmp_dev = 0;
    int mux_l2_i = 0;

    memcpy(&(lcDev.switch_cpld), &switch_cpld_config, sizeof(struct lc_config_t));
    lcDev.lc_intr_gpio = LC_INT_GPIO;
    for (lc_id = 0; lc_id < CARD_NUM; lc_id++) {

        lcDev.obj[lc_id].sff_intr_gpio = sff_intr_gpio[lc_id];

        for (port_grp = 0; port_grp < PORT_GROUP_NUM; port_grp++) {

            memcpy(&lcDev.obj[lc_id].cpld[port_grp], &cpld_config_tbl[lc_id][port_grp], sizeof(struct lc_config_t));
            memcpy(&lcDev.obj[lc_id].ioexp[port_grp], &ioexp_config_tbl[lc_id][port_grp], sizeof(struct lc_config_t));
        }
        for (tmp_dev = 0; tmp_dev < TMP_DEV_NUM; tmp_dev++) {
            memcpy(&lcDev.obj[lc_id].tmp_dev[tmp_dev], &tmp_dev_config_tbl[lc_id][tmp_dev], sizeof(struct lc_config_t));
        }
        /*mux config*/
        memcpy(&lcDev.obj[lc_id].mux_l1, &mux_l1_config_tbl[lc_id], sizeof(struct lc_config_t));
        for (mux_l2_i = 0; mux_l2_i < MUX_L2_NUM;  mux_l2_i++) {
            memcpy(&lcDev.obj[lc_id].mux_l2[mux_l2_i], &mux_l2_config_tbl[lc_id][mux_l2_i], sizeof(struct lc_config_t));
        }
    }
}
struct i2c_client *lc_i2c_client_create_init(int lc_ch)
{
    struct i2c_client *client = NULL;
    struct i2c_adapter *adap = NULL;

    client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
    if (!client) {
        goto exit_err;
    }
    adap = i2c_get_adapter(lc_ch);
    if (!adap) {
        LC_DEV_LOG_ERR("get adapter fail ch:%d\n", lc_ch);
        goto exit_kfree_i2c_client;
    }
    client->adapter = adap;

    return client;

exit_kfree_i2c_client:
    kfree(client);
exit_err:

    return NULL;

}
static void lc_i2c_client_destroy_all(struct lc_dev_t *self)
{
    struct i2c_client *client = NULL;
    int id = 0;
    if (!self) {
        return;
    }

    for (id = 0; id < LC_CH_ID_NUM; id++) {
        client = self->lc_client[id].client;
        if (client) {
            if (client->adapter) {
                i2c_put_adapter(client->adapter);
            }
            kfree(client);
        }
    }
}

int lc_i2c_client_create_init_all(struct lc_dev_t *self)
{
    int lc_ch_id = 0;
    struct i2c_client *client = NULL;
    int ret = 0;
    for (lc_ch_id = 0; lc_ch_id < LC_CH_ID_NUM; lc_ch_id++) {

        client = lc_i2c_client_create_init(lcChTbl[lc_ch_id]);
        if (!client) {
            ret = -EBADRQC;
            break;
        }
        self->lc_client[lc_ch_id].client = client;
        mutex_init(&(self->lc_client[lc_ch_id].lock));

    }
    if (-EBADRQC == ret) {
        lc_i2c_client_destroy_all(self);
        return ret;
    }

    return 0;
}
static bool ch_id_valid(int id)
{
    return ((id >= 0 && id < LC_CH_ID_NUM) ? true : false);
}
struct lc_client_t *lc_client_find(struct lc_config_t *config)
{
    struct lc_client_t *lc_client = NULL;
    int lc_ch_id = 0;
    u8 addr = 0;
    if (!config) {
        return NULL;
    }
    lc_ch_id = config->lc_ch_id;
    if (!ch_id_valid(lc_ch_id)) {
        LC_DEV_LOG_ERR("cant find lc_ch_id:%d\n", lc_ch_id);
        return NULL;
    }
    addr = config->addr;
    lc_client = &(lcDev.lc_client[lc_ch_id]);
    lc_client->client->addr = addr;

    return lc_client;
}

static int lc_i2c_smbus_write_byte_data(struct lc_client_t *lc_client, u8 offset, u8 buf)
{
    int ret = 0;
    
    if (!lc_client) { 
        return -EINVAL;
    }

    mutex_lock(&lc_client->lock);
    ret = i2c_smbus_write_byte_data_retry(lc_client->client, offset, buf);
    mutex_unlock(&lc_client->lock);

    return ret;
}
int lc_i2c_smbus_read_byte_data(struct lc_client_t *lc_client, u8 offset)
{
    int ret = 0;
    
    if (!lc_client) { 
        return -EINVAL;
    }

    mutex_lock(&lc_client->lock);
    ret = i2c_smbus_read_byte_data_retry(lc_client->client, offset);
    mutex_unlock(&lc_client->lock);

    return ret;
}

int lc_i2c_smbus_read_word_data(struct lc_client_t *lc_client, u8 offset)
{
    int ret = 0;
    
    if (!lc_client) { 
        return -EINVAL;
    }

    mutex_lock(&lc_client->lock);
    ret = i2c_smbus_read_word_data_retry(lc_client->client, offset);
    mutex_unlock(&lc_client->lock);

    return ret;
}

int lc_i2c_smbus_write_word_data(struct lc_client_t *lc_client, u8 offset, u16 data)
{
    int ret = 0;
    
    if (!lc_client) { 
        return -EINVAL;
    }

    mutex_lock(&lc_client->lock);
    ret = i2c_smbus_write_word_data_retry(lc_client->client, offset, data);
    mutex_unlock(&lc_client->lock);

    return ret;
}

/* description: get lc object instance
 *
 * input:
 *      lc_id: lc id number
 *return:
        0: success
 *  */
inline struct lc_dev_obj_t *lc_obj_get(int lc_id)
{
    if (!lc_id_valid(lc_id)) {
        LC_DEV_LOG_ERR("fail to get lc obj:%d\n", lc_id);
        return NULL;
    }
    return &(lcDev.obj[lc_id]);
}
int lc_sff_input_get(sff_input_type_t type, int lc_id, unsigned long *bitmap)
{
    struct lc_dev_obj_t *obj = NULL;

    if (!bitmap) {
        return -EINVAL;
    }

    if (!lc_id_valid(lc_id)) {
        return -EINVAL;
    }
    
    if (!(obj = lc_obj_get(lc_id))) {
        return -EBADRQC;
    }
    
    if (SFF_DATA_VALID_MASK != obj->input[type].valid) {
        return -EBADRQC;
    }

    *bitmap = (unsigned long)(obj->input[type].buf);
    LC_DEV_LOG_DBG("lc_id:%d %s: %lx\n", lc_id, sff_input_name[type], *bitmap);
    return 0;
}
static int lc_dummy_rx_los_get(int lc_id, unsigned long *bitmap)
{
    return -ENOSYS;
}
static int lc_dummy_tx_fault_get(int lc_id, unsigned long *bitmap)
{
    return -ENOSYS;
}

static int lc_dummy_tx_disable_get(int lc_id, int port, u8 *val)
{
    return -ENOSYS;
}

static int lc_dummy_tx_disable_set(int lc_id, int port, u8 val)
{
    return -ENOSYS;
}

static int lc_dummy_mode_sel_get(int lc_id, int port, u8 *val)
{
    return -ENOSYS;
}

static int lc_dummy_mode_sel_set(int lc_id, int port, u8 val)
{
    return -ENOSYS;
}

static int lc_sff_prs_get(int lc_id, unsigned long *bitmap)
{
    int ret = 0;
    unsigned long tmp = 0;

    if (!bitmap) {
        return -EINVAL;
    }
    if ((ret = lc_sff_input_get(SFF_PRS_TYPE, lc_id, &tmp)) < 0) {
        return ret;
    }

    *bitmap = tmp;
    return 0;
}
static int lc_sff_oc_get(int lc_id, unsigned long *bitmap)
{
    int ret = 0;
    unsigned long tmp = 0;

    if (!bitmap) {
        return -EINVAL;
    }
    if ((ret = lc_sff_input_get(SFF_OC_TYPE, lc_id, &tmp)) < 0) {
        return ret;
    }

    *bitmap = tmp;
    return 0;

}
static int lc_sff_intr_get(int lc_id, unsigned long *bitmap)
{
    return lc_sff_input_get(SFF_INT_TYPE, lc_id, bitmap);
}
/*update part of input data by port_group
 *input data contains 32 bits which is divided into 2 group of sub_data(16bit)
 * */
static void subdata_update(u32 *data, u32 sub, int grp)
{
    u32 mask = (((1 << PORT_IDX_NUM) - 1) << (grp * PORT_IDX_NUM));
    u32 shift = grp * PORT_IDX_NUM;
    (*data) = ((*data) & (~mask)) | (sub << shift);
}
static int lc_sff_input_update_byGroup(sff_input_type_t type, int lc_id, int port_grp)
{
    struct lc_client_t *lc_client = NULL;
    int ret = 0;
    struct lc_dev_obj_t *obj = NULL;
    struct lc_config_t *config = NULL;
    u32 old_input = 0;
    u32 input_change = 0;
    int port = 0;
    u8 cur_st = 0;
    
    if (!grp_valid(port_grp) ||
            !lc_id_valid(lc_id)) {
        return -EINVAL;
    }
    
    if (!(obj = lc_obj_get(lc_id))) {
        return -EBADRQC;
    }
    /*first stage , use ioexpander to handle present change*/
    if (SFF_PRS_TYPE == type) {
        config = &(obj->ioexp[port_grp]);
    } else {
        config = &(obj->cpld[port_grp]);
    }
    
    if (!(lc_client = lc_client_find(config))) {
        return -EBADRQC;
    }
    if ((ret = lc_i2c_smbus_read_word_data(lc_client, lc_sff_input_reg[type])) < 0) {
        obj->input[type].valid = 0;
        return ret;
    }
    old_input = obj->input[type].buf;
    subdata_update(&(obj->input[type].buf), ret, port_grp);
    obj->input[type].valid |= 1 << port_grp;
    LC_DEV_LOG_DBG("lc_id:%d group:%d %s\n", lc_id , port_grp, sff_input_name[type]);
    /*debugging*/
    input_change = old_input ^ (obj->input[type].buf);
    for (port = 0; port < PORT_IDX_NUM * PORT_GROUP_NUM; port++) {
        if (test_bit(port, (unsigned long *)&input_change)) {
            cur_st = (test_bit(port, (unsigned long *)&(obj->input[type].buf)) ? 1 : 0);
            LC_DEV_LOG_DBG("lc_id:%d port:%d st->%d\n", lc_id, port, cur_st);
        }
    }  
    return 0;
};
static int lc_sff_prs_update_byGroup(int lc_id, int port_grp)
{
    return lc_sff_input_update_byGroup(SFF_PRS_TYPE, lc_id, port_grp);
}

static int lc_sff_intr_update_byGroup(int lc_id, int port_grp)
{
    return lc_sff_input_update_byGroup(SFF_INT_TYPE, lc_id, port_grp);
}

static int lc_sff_oc_update_byGroup(int lc_id, int port_grp)
{
    return lc_sff_input_update_byGroup(SFF_OC_TYPE, lc_id, port_grp);
}
/*1st group*/
static int lc_sff_prs_update_1stGroup(int lc_id)
{
    return lc_sff_prs_update_byGroup(lc_id, 0);
}

static int lc_sff_intr_update_1stGroup(int lc_id)
{
    return lc_sff_intr_update_byGroup(lc_id, 0);
}

static int lc_sff_oc_update_1stGroup(int lc_id)
{
    return lc_sff_oc_update_byGroup(lc_id, 0);
}
/*2nd group*/
static int lc_sff_prs_update_2ndGroup(int lc_id)
{
    return lc_sff_prs_update_byGroup(lc_id, 1);
}

static int lc_sff_intr_update_2ndGroup(int lc_id)
{
    return lc_sff_intr_update_byGroup(lc_id, 1);
}
static int lc_sff_oc_update_2ndGroup(int lc_id)
{
    return lc_sff_oc_update_byGroup(lc_id, 1);
}

static inline void group_idx_get(int port, int *port_grp, int *idx)
{
    *port_grp = (port >> 4);
    *idx = port & 0xf;
}
static int lc_sff_output_bitmap_set(lc_sff_output_type_t type, int lc_id, int port_grp, u16 val)
{
    struct lc_client_t *lc_client = NULL;
    int ret = 0;
    u8 offset = lc_sff_output_reg[type];
    struct lc_dev_obj_t *obj = NULL;

    if (!lc_id_valid(lc_id)) {
        return -EINVAL;
    }
    
    if (!grp_valid(port_grp)) {
        return -EINVAL;
    }
    
    if (!(obj = lc_obj_get(lc_id))) {
        return -EBADRQC;
    }
    
    if (!(lc_client = lc_client_find(&(obj->cpld[port_grp])))) {
        return -EBADRQC;
    }
    
    if ((ret = lc_i2c_smbus_write_word_data(lc_client, offset, val)) < 0) {
        return ret;
    }

    return 0;
}
static int lc_sff_output_set(lc_sff_output_type_t type, int lc_id, int port, u8 val)
{
    struct lc_client_t *lc_client = NULL;
    int ret = 0;
    int port_grp = 0;
    int idx = 0;
    u16 reg = 0;
    u8 offset = lc_sff_output_reg[type];
    struct lc_dev_obj_t *obj = NULL;

    if (!lc_id_valid(lc_id)) {
        return -EINVAL;
    }
    
    if (!(obj = lc_obj_get(lc_id))) {
        return -EBADRQC;
    }
    
    group_idx_get(port, &port_grp, &idx);
    
    if (!grp_valid(port_grp) ||
        !port_idx_valid(idx)) {
        return -EINVAL;
    }
    
    if (!(lc_client = lc_client_find(&(obj->cpld[port_grp])))) {
        return -EBADRQC;
    }
    
    if((ret = lc_i2c_smbus_read_word_data(lc_client, offset)) < 0) {
        return ret;
    }

    reg = ret;

    if (val) {
        reg = inv_set_bit(idx, reg);
    } else {
        reg = inv_clear_bit(idx, reg);
    }
    /*debug*/
    LC_DEV_LOG_DBG("%s lc_id:%d port:%d val:%d\n", lc_sff_output_name[type], lc_id, port, val);
    
    if ((ret = lc_i2c_smbus_write_word_data(lc_client, offset, reg)) < 0) {
        return ret;
    }

    return 0;
}
static int lc_sff_output_get(lc_sff_output_type_t type, int lc_id, int port, u8 *val)
{
    struct lc_client_t *lc_client = NULL;
    int ret = 0;
    int port_grp = 0;
    int idx = 0;
    u16 reg = 0;
    u8 offset = lc_sff_output_reg[type];
    struct lc_dev_obj_t *obj = NULL;
    
    if (!(obj = lc_obj_get(lc_id))) {
        return -EBADRQC;
    }
    
    group_idx_get(port, &port_grp, &idx);
    
    if (!grp_valid(port_grp) ||
        !port_idx_valid(idx)) {
        return -EINVAL;
    }
    if (!(lc_client = lc_client_find(&(obj->cpld[port_grp])))) {
        return -EBADRQC;
    }
    
    if ((ret = lc_i2c_smbus_read_word_data(lc_client, offset)) < 0) {
        return ret;
    }

    reg = ret;
    if (test_bit(idx, (unsigned long *)&reg)) {
        *val = 1;
    } else {

        *val = 0;
    }

    /*debug*/
    LC_DEV_LOG_DBG("%s lc_id:%d port:%d val:%d\n", lc_sff_output_name[type], lc_id, port, *val);
    return 0;
}
static int lc_sff_lpmode_set(int lc_id, int port, u8 val)
{
    return lc_sff_output_set(SFF_LPMODE_TYPE, lc_id, port, val);
}
static int lc_sff_lpmode_get(int lc_id, int port, u8 *val)
{
    return lc_sff_output_get(SFF_LPMODE_TYPE, lc_id, port, val);
}
static int lc_sff_reset_set(int lc_id, int port, u8 val)
{
    return lc_sff_output_set(SFF_RESET_TYPE, lc_id, port, val);
}
static int lc_sff_reset_get(int lc_id, int port, u8 *val)
{
    return lc_sff_output_get(SFF_RESET_TYPE, lc_id, port, val);
}
static int lc_sff_power_set(int lc_id, int port, u8 val)
{
    return lc_sff_output_set(SFF_POWER_TYPE, lc_id, port, val);
}
static int lc_sff_power_get(int lc_id, int port, u8 *val)
{
    return lc_sff_output_get(SFF_POWER_TYPE, lc_id, port, val);
}
static int lc_sff_intr_is_asserted(int lc_id, bool *asserted)
{
    bool st = false;
    int lv = 0;
    lv = gpio_get_value(lcDev.obj[lc_id].sff_intr_gpio);

    if (!lv) {
        st = true;
        LC_DEV_LOG_DBG("asserted\n");
    }
    *asserted = st;
    return 0;
}
/*used to check if mux l1 is alive*/
bool lc_dev_mux_l1_is_alive(int lc_id)
{
    int ret = 0;
    struct lc_client_t *lc_client = NULL;
    struct lc_dev_obj_t *obj = NULL;    
    bool alive = false;  
   
    if (!p_valid(obj = lc_obj_get(lc_id))) {
        return alive;
    }
    
    if (!p_valid(lc_client = lc_client_find(&(obj->mux_l1)))) {
        return alive;
    }

    if ((ret = lc_i2c_smbus_read_byte_data(lc_client, PCA9548_CONTROL_OFFSET)) < 0) {
        return alive;
    }
    if (0 != ret) { 
        alive = true;
    }
    return alive;
}    
int lc_dev_mux_l1_reset(int lc_id, u8 lv)
{
    int ret = 0;
    struct lc_client_t *lc_client = NULL;
    struct lc_dev_obj_t *obj = NULL;
    u8 reg = 0;

    if (!(obj = lc_obj_get(lc_id))) {
        return -EBADRQC;
    }

    if (!(lc_client = lc_client_find(&(obj->cpld[CPLD1_ID])))) {
        return -EBADRQC;
    }

    if ((ret = lc_i2c_smbus_read_byte_data(lc_client, RST_PCA9548_D_OFFSET)) < 0) {
        return ret;
    }
    reg = ret;

    LC_DEV_LOG_DBG("lc_id:%d get reg:0x%x\n", lc_id, reg);
    
    if (lv) {
        set_bit(RST_PCA9546_N_BIT, (unsigned long *)&reg);
    } else {
        clear_bit(RST_PCA9546_N_BIT, (unsigned long *)&reg);
    }
    LC_DEV_LOG_DBG("lc_id:%d set reg:0x%x\n", lc_id, reg);

    if ((ret = lc_i2c_smbus_write_byte_data(lc_client, RST_PCA9548_D_OFFSET, reg)) < 0) {
        return ret;
    }

    return 0;
}
static bool mux_l2_id_valid(int mux_l2_id)
{
    if (mux_l2_id < MUX_L2_NUM && mux_l2_id >= 0) {
        return true;
    } else {
        LC_DEV_LOG_ERR("invalid mux_l2_id:%d\n", mux_l2_id);
        return false;
    }
}
int lc_dev_mux_l2_reset(int lc_id, int mux_l2_id, u8 lv)
{
    int ret = 0;
    struct lc_client_t *lc_client = NULL;
    struct lc_dev_obj_t *obj = NULL;
    u8 reg = 0;

    if (!mux_l2_id_valid(mux_l2_id)) {

        return -EINVAL;
    }

    if (!(obj = lc_obj_get(lc_id))) {
        return -EBADRQC;
    }

    if (!(lc_client = lc_client_find(&(obj->cpld[CPLD1_ID])))) {
        return -EBADRQC;
    }
    if ((ret = lc_i2c_smbus_read_byte_data(lc_client, RST_PCA9548_D_OFFSET)) < 0) {
        return ret;
    }
    reg = ret;

    LC_DEV_LOG_DBG("lc_id:%d get reg:0x%x\n", lc_id, reg);
    if (lv) {
        set_bit(rst_mux_l2_bit[mux_l2_id], (unsigned long *)&reg);
    } else {
        clear_bit(rst_mux_l2_bit[mux_l2_id], (unsigned long *)&reg);
    }
    LC_DEV_LOG_DBG("lc_id:%d set reg:0x%x\n", lc_id, reg);

    if ((ret = lc_i2c_smbus_write_byte_data(lc_client, RST_PCA9548_D_OFFSET, reg)) < 0) {
        return ret;
    }

    return 0;

}
static inline long lm75_reg_to_mc(s16 temp, u8 resolution)
{
    return ((temp >> (16 - resolution)) * 1000) >> (resolution - 8);
}

int lc_dev_temp_get(temp_conf_type_t type, int lc_id, int tmp_id, long *val)
{
    int ret = 0;
    u8 offset = 0xff;
    s16 reg = 0;
    struct lc_client_t *lc_client = NULL;
    struct lc_dev_obj_t *obj = NULL;
    
    if (!p_valid(val)) {
        return -EINVAL;
    }
    if (!lc_id_valid(lc_id)) {
        return -EINVAL;
    }
    
    if (!p_valid(obj = lc_obj_get(lc_id))) {
        return -EBADRQC;
    } 
    if (!p_valid(lc_client = lc_client_find(&(obj->tmp_dev[tmp_id])))) {
        return -EBADRQC;
    }
    
    switch (type) {
        case TEMP_INPUT:
            offset = LM75_REG_TEMP;       
            break; 
        case TEMP_TH_L:
            offset = LM75_REG_LOW;
            break;
        case TEMP_TH_H:
            offset = LM75_REG_HIGH;
            break;
        default:
            LC_DEV_LOG_ERR("UNKOWN_TYPE:%d\n", type);
            break;
    }
    if (0xff == offset) {
        return -EINVAL;
    }
    if ((ret = lc_i2c_smbus_read_word_data(lc_client, offset)) < 0) {
        return ret;
    }
    reg = ret;
    *val = lm75_reg_to_mc(reg, LM75_RESOLUTION);
    
    return 0;
}   

int lc_dev_temp_get_text(int lc_id, char *buf, int size)
{
    int ret = 0;
    long temp[2];
    int tmp_id = 0;
    int count = 0;

    for (tmp_id = 0; tmp_id < TMP_DEV_NUM; tmp_id++) { 
        
        if ((ret = lc_dev_temp_get(TEMP_INPUT, lc_id, tmp_id, &temp[tmp_id])) < 0) {
            break;
        }
        count += scnprintf(buf+count, size-count,
                           "tmp%d: %ld c\n",
                           tmp_id, temp[tmp_id]);
    }
    if (ret < 0) {
        return ret;
    } 
    
    for (tmp_id = 0; tmp_id < TMP_DEV_NUM; tmp_id++) { 
        
        if ((ret = lc_dev_temp_get(TEMP_INPUT, lc_id, tmp_id, &temp[tmp_id])) < 0) {
            break;
        }
        count += scnprintf(buf+count, size-count,
                           "tmp%d: %ld c\n",
                           tmp_id, temp[tmp_id]);
    }
    if (ret < 0) {
        return ret;
    } 
    return 0;
}    
int lc_dev_temp_th_get_text(int lc_id, char *buf, int size)
{
    int ret = 0;
    long temp_th[TMP_DEV_NUM];
    int tmp_id = 0;
    int count = 0;

    for (tmp_id = 0; tmp_id < TMP_DEV_NUM; tmp_id++) { 
        
        if ((ret = lc_dev_temp_get(TEMP_TH_H, lc_id, tmp_id, &temp_th[tmp_id])) < 0) {
            break;
        }
        count += scnprintf(buf+count, size-count,
                           "tmp%d: high thershold %ld c\n",
                           tmp_id+1, temp_th[tmp_id]);
    }
    if (ret < 0) {
        return ret;
    } 
    
    for (tmp_id = 0; tmp_id < TMP_DEV_NUM; tmp_id++) { 
        
        if ((ret = lc_dev_temp_get(TEMP_TH_L, lc_id, tmp_id, &temp_th[tmp_id])) < 0) {
            break;
        }
        count += scnprintf(buf+count, size-count,
                           "tmp%d: low thershold %ld c\n",
                           tmp_id+1, temp_th[tmp_id]);
    }
    if (ret < 0) {
        return ret;
    } 
    return 0;
}    

/* description: reset lc
 *
 * input:
 * lc_id: lc number
 * lv: reset level

 *return:
        0: success
[note]
    the reset is used to do lc reset via switch cpld reg,

 *  */
int lc_dev_reset_set(int lc_id, u8 lv)
{
    int ret = 0;
    struct lc_client_t *lc_client = NULL;
    u8 reg = 0;
   
    if (!lc_id_valid(lc_id)) {
        return -EINVAL;
    }
    
    if (!(lc_client = lc_client_find(&(lcDev.switch_cpld)))) {
        return -EBADRQC;
    }
    
    if ((ret = lc_i2c_smbus_read_byte_data(lc_client, lc_control_reg[lc_id])) < 0) {
        return ret;
    }
    reg = ret;

    LC_DEV_LOG_DBG("lc_id:%d get reg:0x%x\n", lc_id, reg);
    if (lv) {
        reg = inv_set_bit(SWITCH_LC_RESET_N_BIT, reg);
    } else {
        reg = inv_clear_bit(SWITCH_LC_RESET_N_BIT, reg);
    }
    LC_DEV_LOG_DBG("lc_id:%d set reg:0x%x\n", lc_id, reg);

    if ((ret = lc_i2c_smbus_write_byte_data(lc_client, lc_control_reg[lc_id], reg)) < 0) {
        return ret;
    }

    return 0;
}
int lc_dev_reset_get(int lc_id, u8 *lv)
{
    int ret = 0;
    struct lc_client_t *lc_client = NULL;
    u8 reg = 0;
    
    if (!lc_id_valid(lc_id)) {
        return -EINVAL;
    }
    
    if (!lv) {
        return -EINVAL;
    }

    if (!(lc_client = lc_client_find(&(lcDev.switch_cpld)))) {
        return -EBADRQC;
    }
    
    if ((ret = lc_i2c_smbus_read_byte_data(lc_client, lc_control_reg[lc_id])) < 0) {
        return ret;
    }
    reg = ret;

    LC_DEV_LOG_DBG("lc_id:%d get reg:0x%x\n", lc_id, reg);
    
    if (test_bit(SWITCH_LC_RESET_N_BIT, (unsigned long *)&reg)) {
        *lv = 1;
    } else {
        *lv = 0;
    }   

    return 0;
}
/* description: set lc power
 *
 * input:
 *      on:   true:power on  false: power off
 *return:
        0: success
 *  */
int lc_dev_power_set(int lc_id, bool on)
{
    int ret = 0;
    struct lc_client_t *lc_client = NULL;
    struct lc_dev_obj_t *obj = NULL;
    u8 reg = 0;
    if (!lc_id_valid(lc_id)) {
        return -EINVAL;
    }

    if (!(obj = lc_obj_get(lc_id))) {
        return -EBADRQC;
    }

    if (!(lc_client = lc_client_find(&(obj->cpld[CPLD1_ID])))) {
        return -EBADRQC;
    }
    /*set 12v*/
    if ((ret = lc_i2c_smbus_read_byte_data(lc_client, MISC_CONTROL2_OFFSET)) < 0) {
        return ret;
    }
    reg = ret;
    if (on) {
        reg = inv_set_bit(LC1_PWR_CONTROL_BIT, reg);
    } else {
        reg = inv_clear_bit(LC1_PWR_CONTROL_BIT, reg);
    }
    if ((ret = lc_i2c_smbus_write_byte_data(lc_client, MISC_CONTROL2_OFFSET, reg)) < 0) {
        return ret;
    }

    /*control ucd*/
    if ((ret = lc_i2c_smbus_read_byte_data(lc_client, MISC_CONTROL1_OFFSET)) < 0) {
        return ret;
    }
    reg = ret;
    if (on) {
        reg = inv_set_bit(UCD_PMBUS_CNTRL_R_BIT, reg);
    } else {
        reg = inv_clear_bit(UCD_PMBUS_CNTRL_R_BIT, reg);
    }
    if ((ret = lc_i2c_smbus_write_byte_data(lc_client, MISC_CONTROL1_OFFSET, reg)) < 0) {
        return ret;
    }

    return 0;
}


int lc_dev_led_set(int lc_id, lc_led_ctrl_t ctrl)
{
    int ret = 0;
    struct lc_client_t *lc_client = NULL;
    struct lc_dev_obj_t *obj = NULL;
    u8 reg = 0;
    
    if (!lc_id_valid(lc_id)) {
        return -EINVAL;
    }

    if (!(obj = lc_obj_get(lc_id))) {
        return -EBADRQC;
    }

    if (!(lc_client = lc_client_find(&(obj->cpld[CPLD1_ID])))) {
        return -EBADRQC;
    }
    
    reg = ctrl;
    
    if ((ret = lc_i2c_smbus_write_byte_data(lc_client, LC_ST_LED_OFFSET, reg)) < 0) {
        return ret;
    }
    if (p_valid(lc_led_str[ctrl])) {
       LC_DEV_LOG_DBG("lc_id:%d %s\n", lc_id, lc_led_str[ctrl]); 
    } 
    return 0;
}    

int lc_dev_type_get(int lc_id, lc_type_t *type)
{
    int ret = 0;
    struct lc_client_t *lc_client = NULL;
    struct lc_dev_obj_t *obj = NULL;
    u8 reg = 0;
    u8 id = 0;

    if (!lc_id_valid(lc_id)) {
        return -EINVAL;
    }

    if (!type) {
        return -EINVAL;
    }

    if (!(obj = lc_obj_get(lc_id))) {
        return -EBADRQC;
    }

    if (!(lc_client = lc_client_find(&(obj->cpld[CPLD1_ID])))) {
        return -EBADRQC;
    }

    if ((ret = lc_i2c_smbus_read_byte_data(lc_client, PCB_VER_OFFSET)) < 0) {
        return ret;
    }

    reg = ret;
    id = bits_get(reg, LC_ID_BIT_S, LC_ID_BIT_NUM);

    if (LC_100G_ID == id) {
        *type = LC_100G_TYPE;
    } else if (LC_400G_ID == id) {
        *type = LC_400G_TYPE;
    } else {
        LC_DEV_LOG_ERR("unknown lc type!\n");
        *type = LC_UNKNOWN_TYPE; 
        return -EBADRQC;
    }

    return 0;
}

int lc_dev_type_get_text(int lc_id, u8 *buf, int size)
{
    int ret = 0;
    lc_type_t type = LC_UNKNOWN_TYPE;
    
    if ((ret = lc_dev_type_get(lc_id, &type)) < 0) {
        return ret;
    }
    
     scnprintf(buf, size,
               "%s\n",
               lc_type_name[type]);

    return 0;
}   

void lc_dev_phy_ready(unsigned long bitmap, bool *ready)
{
    if (PHY_READY_BITMASK == bitmap) {
        *ready = true;
    } else {
        *ready = false;
    } 
}

int lc_dev_power_ready(int lc_id, bool *ready)
{
    int ret = 0;
    struct lc_client_t *lc_client = NULL;
    struct lc_dev_obj_t *obj = NULL;
    u8 st1 = 0;
    u8 st2 = 0;
    if (!lc_id_valid(lc_id)) {
        return -EINVAL;
    }
    if (!ready) {
        return -EINVAL;
    }
    if (!(obj = lc_obj_get(lc_id))) {
        return -EBADRQC;
    }

    if (!(lc_client = lc_client_find(&(obj->cpld[CPLD1_ID])))) {
        return -EBADRQC;
    }
    if ((ret = lc_i2c_smbus_read_byte_data(lc_client, DC_DC_ST1_OFFSET)) < 0) {
        return ret;
    }
    st1 = ret;
    if ((ret = lc_i2c_smbus_read_byte_data(lc_client, DC_DC_ST1_OFFSET)) < 0) {
        return ret;
    }
    st2 = ret;

    if (0xff == st1 &&
            0xff == st2) {
        *ready = true;
    }

    return 0;
}
static int lc_st_get(lc_st_type_t type, unsigned long *bitmap)
{
    if (!bitmap) {
        return -EINVAL;
    }

    if (!lcDev.lc_st[type].valid) {
        return -EBADRQC;
    }

    *bitmap = lcDev.lc_st[type].bitmap;
    return 0;
}
static int lc_st_reg_read(lc_st_type_t type, int lc_id, u8 *st)
{
    int ret = 0;
    struct lc_client_t *lc_client = NULL;

    if (!st) {
        return -EINVAL;
    }
    if (!lc_id_valid(lc_id)) {
        return -EINVAL;
    }

    if (!(lc_client = lc_client_find(&(lcDev.switch_cpld)))) {
        return -EBADRQC;
    }
    
    if ((ret = lc_i2c_smbus_read_byte_data(lc_client, lc_st_reg[lc_id])) < 0) {
        return ret;
    }

    if (test_bit(lc_st_bit[type], (unsigned long *)&ret)) {
        *st = true;
    } else {
        *st = false;
    }

    return 0;
}
static int lc_st_update(lc_st_type_t type)
{
    int ret = 0;
    u8 st = 0;
    u8 old_st = 0;
    int lc_id = 0;
    struct ldata_format_t *ldata = NULL;
    
    ldata = &(lcDev.lc_st[type]);
    
    for (lc_id = 0; lc_id < CARD_NUM; lc_id++) {
        if ((ret = lc_st_reg_read(type, lc_id, &st)) < 0) {
            break;
        }
        /*dbg to show transition*/
        old_st = ((test_bit(lc_id, &(ldata->bitmap))) ? 1 : 0);
        if (old_st != st) {
            LC_DEV_LOG_DBG("lc_id:%d %s %d -> %d\n", lc_id, lc_st_name[type], old_st, st);
        }
        if (st) {
            set_bit(lc_id, &(ldata->bitmap));
        } else {
            clear_bit(lc_id, &(ldata->bitmap));
        }
    }
    if (ret < 0) {
        ldata->valid = false;
        ldata->bitmap = 0;
        return ret;
    }
    ldata->valid = true;
    return 0;
}
/* description: get line card presence status
 *
 * input:
 * output:
 *      bitmap: bit0 : lc1 ~ bit3 : lc4
 *      1: present 0: absent
 *
 *return:
        0: success
 *  */
int lc_dev_prs_get(unsigned long *bitmap)
{
    int ret = 0;
    unsigned long tmp = 0;

    if ((ret = lc_st_get(LC_ST_PRS_TYPE, &tmp)) < 0) {
        return ret;
    }

    *bitmap = ~tmp;
    return 0;
}
/* description: get line card temperature INT bit
 *
 * input:
 * output:
 *      bitmap: bit0 : lc1 ~ bit3 : lc4
 *      0: interrupt 1: normal
 *
 *return:
        0: success
 *  */
int lc_dev_temp_intr_get(unsigned long *bitmap)
{
    int ret = 0;
    unsigned long tmp = 0;

    
    if (!p_valid(bitmap)) {
        return -EINVAL;
    }
    
    if ((ret = lc_st_get(LC_ST_OVER_TEMP_TYPE, &tmp)) < 0) {
        return ret;
    }

    *bitmap = tmp;
    return 0;
}

static int lc_dev_over_temp_asserted_byDev(int lc_id, int tmp_id, bool *asserted)
{
    int ret = 0;
    unsigned long intr = 0;
    long temp_th_h = 0;
    long temp = 0;
    int i = 0;
    if (!p_valid(asserted)) {
        return -EINVAL;
    }
    
    *asserted = false;
    if ((ret = lc_dev_temp_intr_get(&intr)) < 0) {
        return ret;
    }
    
    if (!test_bit(lc_id , &intr)) {

        if ((ret = lc_dev_temp_get(TEMP_TH_H, lc_id, tmp_id, &temp_th_h)) < 0) {
            return ret;
        }
        for (i = 0; i < TEMP_CHECK_NUM; i++) {
            
            if ((ret = lc_dev_temp_get(TEMP_INPUT, lc_id, tmp_id, &temp)) < 0) {
                break;
            }
            if (temp < temp_th_h) {
                break;
            }
        }
        if (i >= TEMP_CHECK_NUM) {

            *asserted = true;
        }
        
        if (ret < 0) {
            return ret;
        }
        
    }
    
    return 0;
}    

int lc_dev_over_temp_asserted(int lc_id, bool *asserted)
{
    bool dev_asserted[TMP_DEV_NUM];
    int ret = 0;
    int tmp_id = 0;

    memset(dev_asserted, 0, sizeof(dev_asserted)); 
    for (tmp_id = 0; tmp_id < TMP_DEV_NUM; tmp_id++) {
        if ((ret = lc_dev_over_temp_asserted_byDev(lc_id, tmp_id, &dev_asserted[tmp_id])) < 0) {
            break;
        } 
    }
    if (ret < 0) {
        return ret;
    }

    if (dev_asserted[0] || dev_asserted[1]) {
        *asserted = true;
    }
    return 0;
}    

static int lc_dev_over_temp_deasserted_byDev(int lc_id, int tmp_id, bool *deasserted)
{
    int ret = 0;
    unsigned long intr = 0;
    long temp_th_l = 0;
    long temp = 0;
    int i = 0;
     
    if (!p_valid(deasserted)) {
        return -EINVAL;
    }
    
    *deasserted = false;
    if ((ret = lc_dev_temp_intr_get(&intr)) < 0) {
        return ret;
    }
    
    if (test_bit(lc_id , &intr)) {
        if ((ret = lc_dev_temp_get(TEMP_TH_L, lc_id, tmp_id, &temp_th_l)) < 0) {
            return ret;
        }
        for (i = 0; i < TEMP_CHECK_NUM; i++) {
            
            if ((ret = lc_dev_temp_get(TEMP_INPUT, lc_id, tmp_id, &temp)) < 0) {
                break;
            }
            if (temp > temp_th_l) {
                break;
            }
        }
        if (i >= TEMP_CHECK_NUM) {

            *deasserted = true;
        }
        if (ret < 0) {
            return ret;
        }

    }
    
    return 0;
}    

int lc_dev_over_temp_deasserted(int lc_id, bool *deasserted)
{
    bool dev_deasserted[TMP_DEV_NUM];
    int ret = 0;
    int tmp_id = 0;
    
    memset(dev_deasserted, 0, sizeof(dev_deasserted)); 
    for (tmp_id = 0; tmp_id < TMP_DEV_NUM; tmp_id++) {
        if ((ret = lc_dev_over_temp_deasserted_byDev(lc_id, tmp_id, &dev_deasserted[tmp_id])) < 0) {
            break;
        } 
    } 
    if (ret < 0) {
        return ret;
    }
    
    if (dev_deasserted[0] && dev_deasserted[1]) {
        *deasserted = true;
    }
    return 0;
}    

static int lc_intr_is_asserted(bool *asserted)
{
    bool st = false;
    int lv = 0;
    lv = gpio_get_value(lcDev.lc_intr_gpio);

    if (!lv) {
        st = true;
        LC_DEV_LOG_DBG("asserted\n");
    }
    *asserted = st;
    return 0;
}
static int lc_sff_intr_handler_byCard(int lc_id)
{
    struct lc_client_t *lc_client = NULL;
    int ret = 0;
    u8 flag = 0;
    int bit = 0;
    lc_client = lc_client_find(&(lcDev.obj[lc_id].cpld[CPLD1_ID]));

    if (!lc_client) {
        return -EBADRQC;
    }
    if ((ret = lc_i2c_smbus_read_byte_data(lc_client, SFF_INT_ST_OFFSET)) < 0) {
        return ret;
    }

    flag = ret;

    for (bit = SFF_INT_CPLD2_PRS_BIT; bit < SFF_INT_NUM; bit++) {

        if (flag & bit_mask(bit)) {

            if ((ret = sff_intr_funcs[bit](lc_id)) < 0) {
                break;
            }
        }
    }
    if (ret < 0) {
        return ret;
    }
    return 0;
}

static int lc_intr_handler(void)
{
    int ret = 0;
    bool asserted = false;
    
    if ((ret = lc_intr_is_asserted(&asserted)) < 0) {
        return ret;
    }
    if (asserted) {
        if ((ret = lc_st_update(LC_ST_PRS_TYPE)) < 0) {
            return ret;
        }
        if ((ret = lc_st_update(LC_ST_OVER_TEMP_TYPE)) < 0) {
            return ret;
        }
    }
    return 0;
}
static int lc_sff_intr_handler(void)
{
    int lc_id = 0;
    int ret = 0;
    bool asserted = false;

    for (lc_id = 0; lc_id < CARD_NUM; lc_id++) {

        if((ret = lc_sff_intr_is_asserted(lc_id, &asserted)) < 0) {
            break;
        }
        if (asserted) {
            if((ret = lc_sff_intr_handler_byCard(lc_id)) < 0) {
                break;
            }
        }
    }
    if (ret < 0) {
        return ret;
    }
    return 0;
}
int lc_dev_handler(void)
{
    int ret = 0;

    if ((ret = lc_intr_handler()) < 0) {
        return ret;
    }

    if ((ret = lc_sff_intr_handler()) < 0) {
        return ret;
    }
    return 0;
}
static int intr_gpio_init(struct lc_dev_t *mgr)
{
    int ret = 0;
    int lc_id = 0;

    if ((ret = gpio_is_valid(mgr->lc_intr_gpio)) < 0) {

        LC_DEV_LOG_ERR("gpio:%d is invalid\n", mgr->lc_intr_gpio);
        return ret;
    }

    for (lc_id = 0; lc_id < CARD_NUM; lc_id++) {

        if ((ret = gpio_is_valid(mgr->obj[lc_id].sff_intr_gpio)) < 0) {

            LC_DEV_LOG_ERR("gpio:%d is invalid\n", mgr->obj[lc_id].sff_intr_gpio);
            break;
        }
    }
    if (ret < 0) {
        return ret;
    }
    return 0;
}
#if 0
void intr_gpio_deinit(struct lc_dev_t *mgr)
{
}
#endif
void lc_dev_deinit(void)
{
    lc_i2c_client_destroy_all(&lcDev);
}
/*need to run after lc plugged in*/
int lc_dev_lc_cpld_init(int lc_id)
{
    int ret = 0;
    int port_grp = 0;
    int mux_l2_id = 0;

    /*line card sff init*/
    for (port_grp = 0; port_grp < PORT_GROUP_NUM; port_grp++) {

        if ((ret = lc_sff_output_bitmap_set(SFF_RESET_TYPE, lc_id, port_grp, lc_sff_output_def[SFF_RESET_TYPE])) < 0) {
            break;
        }

        if ((ret = lc_sff_output_bitmap_set(SFF_LPMODE_TYPE, lc_id, port_grp, lc_sff_output_def[SFF_LPMODE_TYPE])) < 0) {
            break;
        }

        if ((ret = lc_sff_output_bitmap_set(SFF_POWER_TYPE, lc_id, port_grp, lc_sff_output_def[SFF_POWER_TYPE])) < 0) {
            break;
        }

        if ((ret = lc_sff_input_update_byGroup(SFF_PRS_TYPE, lc_id, port_grp)) < 0) {
            break;
        }
        if ((ret = lc_sff_input_update_byGroup(SFF_INT_TYPE, lc_id, port_grp)) < 0) {
            break;
        }
        if ((ret = lc_sff_input_update_byGroup(SFF_OC_TYPE, lc_id, port_grp)) < 0) {
            break;
        }
    }

    if (ret < 0) {
        return ret;
    }
    /*lc mux init*/
    for (mux_l2_id = 0; mux_l2_id < MUX_L2_NUM; mux_l2_id++) {
        if ((ret = lc_dev_mux_l2_reset(lc_id, mux_l2_id, 1)) < 0) {
            break;
        }
    }
    if (ret < 0) {
        return ret;
    }
    if ((ret = lc_dev_mux_l1_reset(lc_id, 1)) < 0) {
        return ret;
    }

    return 0;
}

int lc_dev_init(int platform_id, int io_no_init)
{
    int ret = 0;
    (void)platform_id;
    (void)io_no_init;
    config_load();
    if (lc_i2c_client_create_init_all(&lcDev) < 0) {
        goto err_exit;
    }

    if (intr_gpio_init(&lcDev) < 0) {
        goto kfree_lc_client;
    }
    LC_DEV_LOG_DBG("ok\n");
    return 0;

kfree_lc_client:
    lc_i2c_client_destroy_all(&lcDev);
err_exit:
    ret = -EBADRQC;
    return ret;

}
