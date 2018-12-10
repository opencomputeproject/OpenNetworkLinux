#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c/pca954x.h>
#include <linux/i2c-mux.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/hwmon-sysfs.h>
#include <linux/hwmon.h>
#include <linux/err.h>
#include <linux/ipmi.h>
#include <linux/ipmi_smi.h>

#define CPUPLD_ADDR 0x31
#define SWPLD1_ADDR 0x6a
#define SWPLD2_ADDR 0x75
#define SWPLD3_ADDR 0x73
#define DEFAULT_CPLD 0

#define MUX_VAL_SERDES_SWPLD3 0xFF
#define MUX_VAL_IDEEPROM      0xFC
#define MUX_VAL_PCA9547       0xFD

#define MUX_VAL_FAN1_EEPROM 0x00
#define MUX_VAL_FAN2_EEPROM 0x01
#define MUX_VAL_FAN3_EEPROM 0x02
#define MUX_VAL_FAN4_EEPROM 0x03
#define MUX_VAL_FAN_CTL     0x05
#define MUX_VAL_FAN_TMP75   0x06
#define MUX_VAL_FAN_IO_CTL  0x07

#define MUX_VAL_PSU1  0x00
#define MUX_VAL_PSU2  0x02

#define DEF_DEV_NUM      1
#define BUS0_DEV_NUM     3
#define BUS0_BASE_NUM    1
#define BUS0_MUX_REG  0x14

#define BUS2_QSFP_DEV_NUM     6
#define BUS2_QSFP_BASE_NUM   41
#define BUS2_QSFP_MUX_REG  0x20
#define BUS2_SFP_DEV_NUM    48
#define BUS2_SFP_BASE_NUM   51
#define BUS2_SFP_MUX_REG  0x21
#define BUS5_DEV_NUM     7
#define BUS5_BASE_NUM   21
#define BUS5_MUX_REG  0x67

#define BUS6_DEV_NUM     2
#define BUS6_BASE_NUM   31
#define BUS6_MUX_REG  0x1f

/* on SWPLD3 */
#define SWPLD3_SFP_CH1_EN     0x00
#define SWPLD3_SFP_CH2_EN     0x01
#define SWPLD3_SFP_CH3_EN     0x02
#define SWPLD3_SFP_CH4_EN     0x03
#define SWPLD3_SFP_CH5_EN     0x04
#define SWPLD3_SFP_CH6_EN     0x05
#define SWPLD3_QSFP_CH_EN     0x06
#define SWPLD3_SFP_CH_DISABLE 0x07

#define SWPLD3_SFP_PORT_9   9
#define SWPLD3_SFP_PORT_19 19
#define SWPLD3_SFP_PORT_29 29
#define SWPLD3_SFP_PORT_39 39
#define SWPLD3_SFP_PORT_48 48

/* on SWPLD2 */
#define SFP_PRESENCE_1 0x30
#define SFP_PRESENCE_2 0x31
#define SFP_PRESENCE_3 0x32
#define SFP_PRESENCE_4 0x33
#define SFP_PRESENCE_5 0x34
#define SFP_PRESENCE_6 0x35
#define SFP_RXLOS_1 0x36
#define SFP_RXLOS_2 0x37
#define SFP_RXLOS_3 0x38
#define SFP_RXLOS_4 0x39
#define SFP_RXLOS_5 0x3A
#define SFP_RXLOS_6 0x3B
#define SFP_TXDIS_1 0x3C
#define SFP_TXDIS_2 0x3D
#define SFP_TXDIS_3 0x3E
#define SFP_TXDIS_4 0x3F
#define SFP_TXDIS_5 0x40
#define SFP_TXDIS_6 0x41
#define SFP_TXFAULT_1 0x42
#define SFP_TXFAULT_2 0x43
#define SFP_TXFAULT_3 0x44
#define SFP_TXFAULT_4 0x45
#define SFP_TXFAULT_5 0x46
#define SFP_TXFAULT_6 0x47

/* on SWPLD1 */
#define QSFP_PRESENCE  0x63
#define QSFP_LPMODE    0x62
#define QSFP_RESET     0x3c

#define SWPLD1_QSFP_MODSEL_REG 0x64
#define SWPLD1_QSFP_MODSEL_VAL 0x3f

/* BMC IMPI CMD */
#define IPMI_MAX_INTF (4)
#define DELTA_NETFN 0x38
#define CMD_SETDATA 0x03
#define CMD_GETDATA 0x02
#define CMD_DIAGMODE 0x1a
#define BMC_DIAG_STATE 0x00
#define BMC_ERR (-6)
#define BMC_NOT_EXIST 0xc1
#define BMC_SWPLD_BUS 2


/* Check cpld read results */
#define VALIDATED_READ(_buf, _rv, _read, _invert)   \
    do {                                            \
        _rv = _read;                                \
        if (_rv < 0) {                              \
            return sprintf(_buf, "READ ERROR\n");   \
        }                                           \
        if (_invert) {                              \
            _rv = ~_rv;                             \
        }                                           \
        _rv &= 0xFF;                                \
    } while(0)                                      \

struct mutex dni_lock;

extern int dni_bmc_cmd(char set_cmd, char *cmd_data, int cmd_data_len);
extern int dni_create_user(void);
extern unsigned char dni_log2(unsigned char num);
extern int dni_bmc_exist_check(void);
extern void device_release(struct device *dev);
extern void msg_handler(struct ipmi_recv_msg *recv_msg,void* handler_data);
extern void dummy_smi_free(struct ipmi_smi_msg *msg);
extern void dummy_recv_free(struct ipmi_recv_msg *msg);

static ipmi_user_t ipmi_mh_user = NULL;
static struct ipmi_user_hndl ipmi_hndlrs = { .ipmi_recv_hndl = msg_handler, };
static atomic_t dummy_count = ATOMIC_INIT(0);

static struct ipmi_smi_msg halt_smi_msg = {
    .done = dummy_smi_free
};

static struct ipmi_recv_msg halt_recv_msg = {
    .done = dummy_recv_free
};

void device_release(struct device *dev)
{
    return;
}
EXPORT_SYMBOL(device_release);

void msg_handler(struct ipmi_recv_msg *recv_msg, void* handler_data)
{
    struct completion *comp = recv_msg->user_msg_data;
    if (comp)
         complete(comp);
    else
        ipmi_free_recv_msg(recv_msg);
    return;
}
EXPORT_SYMBOL(msg_handler);

void dummy_smi_free(struct ipmi_smi_msg *msg)
{
    atomic_dec(&dummy_count);
}
EXPORT_SYMBOL(dummy_smi_free);

void dummy_recv_free(struct ipmi_recv_msg *msg)
{
    atomic_dec(&dummy_count);
}
EXPORT_SYMBOL(dummy_recv_free);

int dni_bmc_exist_check(void)
{
    uint8_t set_cmd;
    uint8_t cmd_data[1] = {0};
    int cmd_data_len, rv = 0;

    set_cmd = CMD_DIAGMODE;
    cmd_data[0] = BMC_DIAG_STATE;
    cmd_data_len = sizeof(cmd_data);
    rv = dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);

    return rv;
}
EXPORT_SYMBOL(dni_bmc_exist_check);

unsigned char dni_log2 (unsigned char num){
    unsigned char num_log2 = 0;
    while(num > 0){
        num = num >> 1;
        num_log2 += 1;
    }
    return num_log2 -1;
}
EXPORT_SYMBOL(dni_log2);

enum{
    BUS0 = 0,
    BUS1,
    BUS2,
    BUS3,
    BUS4,
    BUS5,
    BUS6,
    BUS7,
    BUS8,
    BUS9,
    BUS10,
    BUS11,
    BUS12,
    BUS13,
    BUS14,
};

#define agc7648sv1_i2c_device_num(NUM){    \
    .name = "delta-agc7648sv1-i2c-device", \
    .id   = NUM,                           \
    .dev  = {                              \
        .platform_data = &agc7648sv1_i2c_device_platform_data[NUM], \
        .release       = device_release,   \
    },                                     \
}

static struct cpld_attribute_data {
    uint8_t bus;
    uint8_t addr;
    uint8_t reg;
    uint8_t mask;
    char note[350];
};

enum cpld_type {
    system_cpld,
};

enum swpld1_type {
    swpld1,
};

enum swpld2_type {
    swpld2,
};

enum swpld3_type {
    swpld3,
};

struct cpld_platform_data {
    int reg_addr;
    struct i2c_client *client;
};

enum cpld_attributes {
//CPLDs address and value
    CPLD_REG_ADDR,
    CPLD_REG_VALUE,
    SWPLD1_REG_ADDR,
    SWPLD1_REG_VALUE,
    SWPLD2_REG_ADDR,
    SWPLD2_REG_VALUE,
    SWPLD3_REG_ADDR,
    SWPLD3_REG_VALUE,
//CPLD
    CPU_BOARD_ID1,
    CPU_BOARD_ID2,
    BOARD_VER,
    CPULD_VER,
    CPU_SYS_PWR_OK,
    PLAT_RST,
    CPLD_VR_HOT,
    CPU_OVER_TMP,
    DDR_OVER_TMP,
    CPU_PWR_RST,
    CPU_HARD_RST,
    CPLD_RST,
    MB_PWR_ENABLE,
    MB_PWR_PGD,
    MB_RST_DONE,
    MB_RST,
    EEPROM_WP,
    PSU_FAN_EVENT,
    CPU_I2C_MUX_EN,
    CPU_I2C_MUX_SEL,
//SWPLD1
    BOARD_ID,
    BCM88375_RST,
    B54616S_RST,
    PSU1_EN,
    PSU2_EN,
    PSU1_PWR_FAN_OK,
    PSU2_PWR_FAN_OK,
    PSU2_PRESENT,
    PSU1_PRESENT,
    PSU2_PWR_INT,
    PSU1_PWR_INT,
    BCM88375_INT,
    BCM54616S_IRQ,
    LED_SYS,
    LED_PWR,
    LED_FAN,
    PSU_I2C_SEL,
    FAN1_LED,
    FAN2_LED,
    FAN3_LED,
    FAN4_LED,
    FAN_I2C_SEL,
//SWPLD3
    QSFP_I2C_SEL,
    SFP_CHAN_EN,
    SFP_SEL,
};

enum agc7648sv1_sfp_sysfs_attributes 
{
    SFP_SELECT_PORT,
    SFP_IS_PRESENT,
    SFP_IS_PRESENT_ALL,
    SFP_LP_MODE,
    SFP_RESET,
    SFP_RX_LOS,
    SFP_RX_LOS_ALL,
    SFP_TX_DISABLE,
    SFP_TX_FAULT
};

static struct cpld_attribute_data attribute_data[] = {
//CPLDs address and value
    [CPLD_REG_ADDR] = {
    },
    [CPLD_REG_VALUE] = {
    },
    [SWPLD1_REG_ADDR] = {
    },
    [SWPLD1_REG_VALUE] = {
    },
    [SWPLD2_REG_ADDR] = {
    },
    [SWPLD2_REG_VALUE] = {
    },
    [SWPLD3_REG_ADDR] = {
    },
//CPLD
    [CPU_BOARD_ID1] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x00,       .mask = 0xff,
        .note = "Configured by PLD editor.\n0x15 : BROADWELL D-1527"
    },
    [CPU_BOARD_ID2] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x01,       .mask = 0xff,
        .note = "Configured by PLD editor.\n0x27 : BROADWELL D-1527"
    },
    [BOARD_VER] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x02,       .mask = 0xff,
        .note = "Controlled by external resistors.\n0x00: EVT1\n0x01: EVT2\n0x02: EVT3\n0x03: EVT4\n0x10: DVT1\n0x11: DVT2\n0x20: PVT1"
    },
    [CPULD_VER] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x03,       .mask = 0xff,
        .note = "CPLD Version, controlled by CPLD editor."
    },
    [CPU_SYS_PWR_OK] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x06,       .mask = 1 << 2,
        .note = "Indicate CPU that System Power is OK.\n\"1\" = System Power is OK\n\"0\" = System Power is not OK"
    },
    [PLAT_RST] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x09,       .mask = 1 << 4,
        .note = "Indicate Platform Reset.\n\"1\" = Platform Reset\n\"0\" = Platform Not Reset"
    },
    [CPLD_VR_HOT] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x0b,       .mask = 1 << 3,
        .note = "Indicate Power Rail Over temperature\n\"1\" = Not over temperature\n\"0\" = Over temperature"
    },
    [CPU_OVER_TMP] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x0b,       .mask = 1 << 1,
        .note = "CPU Disomic temperature sensor\n\"1\" = Not over temperature.\n\"0\" = Over temperature."
    },
    [DDR_OVER_TMP] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x0b,       .mask = 1 << 0,
        .note = "DDR over temperature sensor\n\"1\" = Not over temperature.\n\"0\" = Over temperature."
    },
    [CPU_PWR_RST] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x11,       .mask = 1 << 4,
        .note = "Software execute the CPU Power On reset\n\"0\" = Reset\n\"1\" = Normal operation"
    },
    [CPU_HARD_RST] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x11,       .mask = 1 << 2,
        .note = "Software execute the CPU Hard reset\n\"0\" = Reset\n\"1\" = Normal operation"
    },
    [CPLD_RST] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x11,       .mask = 1 << 0,
        .note = "Software reset the CPLD system\n\"0\" = Reset\n\"1\" = Normal operation"
    },
    [MB_PWR_ENABLE] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x12,       .mask = 1 << 3,
        .note = "Software Enable Main board Power\n\"0\" = Disable.\n\"1\" = Enable."
    },
    [MB_PWR_PGD] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x12,       .mask = 1 << 2,
        .note = "Indicate the Main board all power good\n\"0\" = Power rail is failed\n\"1\" = Power rail is good"
    },
    [MB_RST_DONE] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x12,       .mask = 1 << 1,
        .note = "Main board reset done.\n\"0\" = Reset\n\"1\" = Normal operation"
    },
    [MB_RST] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x12,       .mask = 1 << 0,
        .note = "Software reset Main board\n\"0\" = Reset\n\"1\" = Normal operation"
    },
    [EEPROM_WP] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x14,       .mask = 1 << 3,
        .note = "ID EEPROM Write Protect\n\"1\" = enables the lock-down mechanism.\n\"0\" = overrides the lock-down function enabling blocks to be erased or programmed using software commands."
    },
    [PSU_FAN_EVENT] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x15,       .mask = 1 << 1,
        .note = "Indicate the PSU Fan interrupt occurs or not.\n\"0\" = Interrupt occurs\n\"1\" = Interrupt doesn't occur"
    },
    [CPU_I2C_MUX_EN] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x14,       .mask = 1 << 2,
        .note = "CPU I2C MUX Enable\n\"0\" = Enable CPU I2C MUX\n\"1\" = Disable CPU I2C MUX"
    },
    [CPU_I2C_MUX_SEL] = {
        .bus  = BUS3,       .addr = CPUPLD_ADDR,
        .reg  = 0x14,       .mask = 0x03,
        .note = "CPU I2C MUX Selection\n\"0x00\" = CPUBD devices\n\"0x01\" = SWBD devices\n\"0x02\" = SWPLDs\n\"0x03\" = QSFP-DD module devices"
    },
//SWPLD1
    [BOARD_ID] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x00,       .mask = 0xf0,
        .note = "SW Board ID\n\"0000\": AGC7648."
    },
    [BCM88375_RST] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x04,       .mask = 1 << 6,
        .note = "software Reset for MAC\n\"0\" = Reset.\n\"1\" = Normal Operation."
    },
    [B54616S_RST] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x04,       .mask = 1 << 5,
        .note = "Software Reset for PHY\n\"0\" = Reset.\n\"1\" = Normal Operation."
    },
    [PSU1_EN] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x08,       .mask = 1 << 7,
        .note = "Enable/Disable the Power Supply 1\n\"0\" = Enabled.\n\"1\" = Disabled."
    },
    [PSU2_EN] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x08,       .mask = 1 << 6,
        .note = "Enable/Disable the Power Supply 2\n\"0\" = Enabled.\n\"1\" = Disabled."
    },
    [PSU1_PWR_FAN_OK] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x0b,       .mask = 1 << 7,
        .note = "Indicate the Power Supply 1 power good\n\"1\" = Power rail is good\n\"0\" = Power rail is failed"
    },
    [PSU2_PWR_FAN_OK] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x0b,       .mask = 1 << 6,
        .note = "Indicate the Power Supply 2 power good\n\"1\" = Power rail is good\n\"0\" = Power rail is failed"
    },
    [PSU2_PRESENT] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x0d,       .mask = 1 << 1,
        .note = "Indicate PSU2 present or not.\n\"0\" = YES.\n\"1\" = NO."
    },
    [PSU1_PRESENT] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x0d,       .mask = 1 << 0,
        .note = "Indicate PSU1 present or not.\n\"0\" = YES.\n\"1\" = NO."
    },
    [PSU2_PWR_INT] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x0e,       .mask = 1 << 5,
        .note = "Indicate the PSU2 interrupt occurs or not.\n\"0\" = Interrupt occurs\n\"1\" = Interrupt doesn't occur"
    },
    [PSU1_PWR_INT] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x0e,       .mask = 1 << 4,
        .note = "Indicate the PSU1 interrupt occurs or not.\n\"0\" = Interrupt occurs\n\"1\" = Interrupt doesn't occur"
    },
    [BCM88375_INT] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x0e,       .mask = 1 << 3,
        .note = "Indicate BCM88375 Interrupt occurs or not.\n\"0\" = Interrupt occurs\n\"1\" = Interrupt doesn't occur"
    },
    [BCM54616S_IRQ] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x16,       .mask = 1 << 7,
        .note = "Indicate the BCM54616S interrupt occurs or not.\n\"0\" = Interrupt occurs\n\"1\" = Interrupt doesn't occur"
    },
    [LED_SYS] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x1c,       .mask = 0xf0,
        .note = "\"0x00\"= Off (No Power)\n\"0x01\"= Solid Amber(System Fault)\n\"0x02\"= Solid Green(System Normal Operation)\n\"0x05\"= Blinking Green(1/4S)(System Booting)\n\"0x06\"= Blinking Amber(1/4S)\n\"0x09\"= Blinking Green(1/2S)\n\"0x0A\"= Blinking Amber(1/2S)\n\"0x0D\"= Blinking Green(1S)\n\"0x0E\"= Blinking Amber(1S)"
    },
    [LED_PWR] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x1c,       .mask = 0x0c,
        .note = "\"0x00\"= Off (No Power)\n\"0x01\"= Solid Green(PSU Normal Operation)\n\"0x02\"= Solid Amber(POST in progress)\n\"0x03\"= (Not define)"
    },
    [LED_FAN] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x1c,       .mask = 0x03,
        .note = "\"0x00\"= Off (No Power)\n\"0x01\"= Solid Green(Fan Normal Operation)\n\"0x02\"= Solid Amber(Fan not present)\n\"0x03\"= Blinking Amber(Fan failed)"
    },
    [PSU_I2C_SEL] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x1f,       .mask = 0x03,
        .note = "FAN I2C channel selection\n\"0x00\" = PS1 EEPROM\n\"0x01\" = PS1 HOT SWAP IC\n\"0x02\" = PS2 EEPROM\n\"0x03\" = PS2 HOT SWAP IC\n"
    },
    [FAN1_LED] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x65,       .mask = 0xc0,
        .note = "Indicate the FAN Tray 1 LED status\n\"0x00\" = Off\n\"0x01\" = Solid Green.\n\"0x02\" = Solid Red.\n\"0x03\" = Off\n"
    },
    [FAN2_LED] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x65,       .mask = 0x30,
        .note = "Indicate the FAN Tray 2 LED status\n\"0x00\" = Off\n\"0x01\" = Solid Green.\n\"0x02\" = Solid Red.\n\"0x03\" = Off\n"
    },
    [FAN3_LED] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x65,       .mask = 0x0c,
        .note = "Indicate the FAN Tray 3 LED status\n\"0x00\" = Off\n\"0x01\" = Solid Green.\n\"0x02\" = Solid Red.\n\"0x03\" = Off\n"
    },
    [FAN4_LED] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x65,       .mask = 0x03,
        .note = "Indicate the FAN Tray 4 LED status\n\"0x00\" = Off\n\"0x01\" = Solid Green.\n\"0x02\" = Solid Red.\n\"0x03\" = Off\n"
    },
    [FAN_I2C_SEL] = {
        .bus  = BUS7,       .addr = SWPLD1_ADDR,
        .reg  = 0x67,       .mask = 0x07,
        .note = "FAN I2C channel selection\n\"0x00\" = FAN TRAY 1 EEPROM\n\"0x01\" = FAN TRAY 2 EEPROM\n\"0x02\" = FAN TRAY 3 EEPROM\n\"0x03\" = FAN TRAY 4 EEPROM\n\"0x04\" = Reserved;(Do not use)\n\"0x05\" = FAN Control IC (EMC2305)\n\"0x06\" = FAN Thermal Sensor (TMP75A)\n\"0x07\" = FAN IO Control (PCA9555DB)"
    },
//SWPLD3
    [QSFP_I2C_SEL] = {
        .bus  = BUS7,       .addr = SWPLD3_ADDR,
        .reg  = 0x20,       .mask = 0x07,
        .note = "QSFP28 I2C channel selection.\n\"0x00\" : QSFP28 Port 0\n\"0x01\" : QSFP28 Port 1\n\"0x02\" : QSFP28 Port 2\n\"0x03\" : QSFP28 Port 3\n\"0x04\" : QSFP28 Port 4\n\"0x05\" : QSFP28 Port 5"
    },
    [SFP_CHAN_EN] = {
        .bus  = BUS7,       .addr = SWPLD3_ADDR,
        .reg  = 0x21,       .mask = 0x70,
        .note = "SFP+ I2C Nth channel and QSFP channel enable index\n\"0x00\": means SFP+ N=0 and 1th channel enable.\n\"0x01\": means SFP+ N=1 and 2th channel enable.\n ...\n\"0x05\": means SFP+ N=5 and 6th channel enable.\n\"0x06\": means QSFP channel enable.\n\"0x07\": Disable all channels."
    },
    [SFP_SEL] = {
        .bus  = BUS7,       .addr = SWPLD3_ADDR,
        .reg  = 0x21,       .mask = 0x07,
        .note = "SFP+ I2C Mth selection. (From PORT1 ~ PORT48)\n\"0x00\": means M=0.\n\"0x01\": means M=1.\n...\n\"0x07\": means M=7\nSFP I2C Channel Number = 8 * N + M +1"
    },
};

struct i2c_device_platform_data {
    int parent;
    struct i2c_board_info info;
    struct i2c_client *client;
};

struct i2c_client * i2c_client_9547;

static struct cpld_platform_data agc7648sv1_cpld_platform_data[] = {
    [system_cpld] = {
        .reg_addr = CPUPLD_ADDR,
    },
};

static struct cpld_platform_data agc7648sv1_swpld1_platform_data[] = {
    [swpld1] = {
        .reg_addr = SWPLD1_ADDR,
    },
};

static struct cpld_platform_data agc7648sv1_swpld2_platform_data[] = {
    [swpld2] = {
        .reg_addr = SWPLD2_ADDR,
    },
};

static struct cpld_platform_data agc7648sv1_swpld3_platform_data[] = {
    [swpld3] = {
        .reg_addr = SWPLD3_ADDR,
    },
};

// pca9548 - add 8 bus 
static struct pca954x_platform_mode pca954x_mode[] = 
{
    { 
        .adap_id = 4,
        .deselect_on_exit = 1,
    },
    { 
        .adap_id = 5,
        .deselect_on_exit = 1,
    },
    { 
        .adap_id = 6,
        .deselect_on_exit = 1,
    },
    { 
        .adap_id = 7,
        .deselect_on_exit = 1,
    },
    { 
        .adap_id = 8,
        .deselect_on_exit = 1,
    },
    {   
        .adap_id = 9,
        .deselect_on_exit = 1,
    },
    { 
        .adap_id = 10,
        .deselect_on_exit = 1,
    },
    { 
        .adap_id = 11,
        .deselect_on_exit = 1,
    },
};

static struct pca954x_platform_data pca954x_data = 
{
    .modes = pca954x_mode, 
    .num_modes = ARRAY_SIZE(pca954x_mode),
};

static struct i2c_board_info __initdata i2c_info_pca9547[] =
{
    {
        I2C_BOARD_INFO("pca9547", 0x71),
        .platform_data = &pca954x_data,
    },
};

/* ---------------- IPMI - start ------------- */
int dni_create_user(void)
{
    int rv, i;

    for (i = 0, rv = 1; i < IPMI_MAX_INTF && rv; i++)
    {
        rv = ipmi_create_user(i, &ipmi_hndlrs, NULL, &ipmi_mh_user);
    }
    if(rv == 0)
    {
        printk("Enable IPMI protocol.\n");
        return rv;
    }
}
EXPORT_SYMBOL(dni_create_user);

int dni_bmc_cmd(char set_cmd, char *cmd_data, int cmd_data_len)
{
    int rv;
    struct ipmi_system_interface_addr addr;
    struct kernel_ipmi_msg msg;
    struct completion comp;

    addr.addr_type = IPMI_SYSTEM_INTERFACE_ADDR_TYPE;
    addr.channel = IPMI_BMC_CHANNEL;
    addr.lun = 0;

    msg.netfn = DELTA_NETFN;
    msg.cmd = set_cmd;
    msg.data_len = cmd_data_len;
    msg.data = cmd_data;

    init_completion(&comp);
    rv = ipmi_request_supply_msgs(ipmi_mh_user, (struct ipmi_addr*)&addr, 0, &msg, &comp, &halt_smi_msg, &halt_recv_msg, 0);
    if(rv)
        return BMC_ERR;

    wait_for_completion(&comp);

    switch(msg.cmd)
    {
        case CMD_GETDATA:
            if(rv == 0)
                return halt_recv_msg.msg.data[1];
            else
            {
                printk(KERN_ERR "IPMI get error!\n");
                return BMC_ERR;
            }
            break;
        case CMD_SETDATA:
            if(rv == 0)
                return rv;
            else
            {
                printk(KERN_ERR "IPMI set error!\n");
                return BMC_ERR;
            }
        case CMD_DIAGMODE:
            if(rv == 0 && (halt_recv_msg.msg.data[0] != BMC_NOT_EXIST))
                return halt_recv_msg.msg.data[1];
            else
			{
                printk(KERN_ERR "BMC is not exist!\n");
                return BMC_ERR;
            }
    }

    ipmi_free_recv_msg(&halt_recv_msg);

    return rv;
}
EXPORT_SYMBOL(dni_bmc_cmd);
/* ---------------- IPMI - stop ------------- */

/* ---------------- I2C device - start ------------- */
static struct i2c_device_platform_data agc7648sv1_i2c_device_platform_data[] = {
    {
        // id eeprom
        .parent = 1,
        .info = { I2C_BOARD_INFO("24c02", 0x53) },
        .client = NULL,
    },
    {
        // tmp75
        .parent = 8,
        .info = { I2C_BOARD_INFO("tmp75", 0x4b) },
        .client = NULL,
    },
    {
        // tmp431
        .parent = 8,
        .info = { I2C_BOARD_INFO("tmp431", 0x4c) },
        .client = NULL,
    },
    {
        // tmp432
        .parent = 8,
        .info = { I2C_BOARD_INFO("tmp432", 0x4d) },
        .client = NULL,
    },
    {
        // tmp75
        .parent = 8,
        .info = { I2C_BOARD_INFO("tmp75", 0x4e) },
        .client = NULL,
    },
    {
        // tmp75 cpu
        .parent = 8,
        .info = { I2C_BOARD_INFO("tmp75", 0x4f) },
        .client = NULL,
    },
    {
        // fan control 1
        .parent = 25,
        .info = { I2C_BOARD_INFO("emc2305", 0x2c) },
        .client = NULL,
    },
    {
        // fan control 2
        .parent = 25,
        .info = { I2C_BOARD_INFO("emc2305", 0x2d) },
        .client = NULL,
    },
    {
        // tmp75 fan
        .parent = 26,
        .info = { I2C_BOARD_INFO("tmp75", 0x4f) },
        .client = NULL,
    },
    {
        // fan IO CTRL
        .parent = 27,
        .info = { I2C_BOARD_INFO("pca9555", 0x27) },
        .client = NULL,
    },
    {
        // PSU 1 eeprom
        .parent = 31,
        .info = { I2C_BOARD_INFO("24c02", 0x50) },
        .client = NULL,
    },
    {
        // PSU 2 eeprom
        .parent = 32,
        .info = { I2C_BOARD_INFO("24c02", 0x50) },
        .client = NULL,
    },
    {
        // PSU 1 control
        .parent = 31,
        .info = { I2C_BOARD_INFO("dni_agc7648sv1_psu", 0x58) },
        .client = NULL,
    },
    {
        // PSU 2 control
        .parent = 32,
        .info = { I2C_BOARD_INFO("dni_agc7648sv1_psu", 0x58) },
        .client = NULL,
    },
    {
        // qsfp 1 (0x50)
        .parent = 41,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        // qsfp 2 (0x50)
        .parent = 42,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        // qsfp 3 (0x50)
        .parent = 43,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        // qsfp 4 (0x50)
        .parent = 44,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        // qsfp 5 (0x50)
        .parent = 45,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        // qsfp 6 (0x50)
        .parent = 46,
        .info = { .type = "optoe1", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 1 (0x50)
        .parent = 51,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 2 (0x50)
        .parent = 52,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 3 (0x50)
        .parent = 53,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 4 (0x50)
        .parent = 54,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 5 (0x50)
        .parent = 55,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 6 (0x50)
        .parent = 56,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 7 (0x50)
        .parent = 57,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 8 (0x50)
        .parent = 58,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 9 (0x50)
        .parent = 59,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 10 (0x50)
        .parent = 60,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 11 (0x50)
        .parent = 61,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 12 (0x50)
        .parent = 62,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 13 (0x50)
        .parent = 63,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 14 (0x50)
        .parent = 64,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 15 (0x50)
        .parent = 65,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 16 (0x50)
        .parent = 66,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 17 (0x50)
        .parent = 67,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 18 (0x50)
        .parent = 68,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 19 (0x50)
        .parent = 69,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 20 (0x50)
        .parent = 70,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 21 (0x50)
        .parent = 71,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 22 (0x50)
        .parent = 72,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 23 (0x50)
        .parent = 73,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 24 (0x50)
        .parent = 74,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 25 (0x50)
        .parent = 75,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 26 (0x50)
        .parent = 76,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 27 (0x50)
        .parent = 77,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 28 (0x50)
        .parent = 78,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 29 (0x50)
        .parent = 79,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 30 (0x50)
        .parent = 80,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 31 (0x50)
        .parent = 81,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 32 (0x50)
        .parent = 82,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 33 (0x50)
        .parent = 83,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 34 (0x50)
        .parent = 84,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 35 (0x50)
        .parent = 85,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 36 (0x50)
        .parent = 86,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 37 (0x50)
        .parent = 87,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 38 (0x50)
        .parent = 88,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 39 (0x50)
        .parent = 89,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 40 (0x50)
        .parent = 90,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 41 (0x50)
        .parent = 91,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 42 (0x50)
        .parent = 92,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 43 (0x50)
        .parent = 93,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 44 (0x50)
        .parent = 94,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 45 (0x50)
        .parent = 95,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 46 (0x50)
        .parent = 96,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 47 (0x50)
        .parent = 97,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
    {
        // sfp 48 (0x50)
        .parent = 98,
        .info = { .type = "optoe2", .addr = 0x50 },
        .client = NULL,
    },
};

static struct platform_device agc7648sv1_i2c_device[] = {
    agc7648sv1_i2c_device_num(0),
    agc7648sv1_i2c_device_num(1),
    agc7648sv1_i2c_device_num(2),
    agc7648sv1_i2c_device_num(3),
    agc7648sv1_i2c_device_num(4),
    agc7648sv1_i2c_device_num(5),
    agc7648sv1_i2c_device_num(6),
    agc7648sv1_i2c_device_num(7),
    agc7648sv1_i2c_device_num(8),
    agc7648sv1_i2c_device_num(9),
    agc7648sv1_i2c_device_num(10),
    agc7648sv1_i2c_device_num(11),
    agc7648sv1_i2c_device_num(12),
    agc7648sv1_i2c_device_num(13),
    agc7648sv1_i2c_device_num(14),
    agc7648sv1_i2c_device_num(15),
    agc7648sv1_i2c_device_num(16),
    agc7648sv1_i2c_device_num(17),
    agc7648sv1_i2c_device_num(18),
    agc7648sv1_i2c_device_num(19),
    agc7648sv1_i2c_device_num(20),
    agc7648sv1_i2c_device_num(21),
    agc7648sv1_i2c_device_num(22),
    agc7648sv1_i2c_device_num(23),
    agc7648sv1_i2c_device_num(24),
    agc7648sv1_i2c_device_num(25),
    agc7648sv1_i2c_device_num(26),
    agc7648sv1_i2c_device_num(27),
    agc7648sv1_i2c_device_num(28),
    agc7648sv1_i2c_device_num(29),
    agc7648sv1_i2c_device_num(30),
    agc7648sv1_i2c_device_num(31),
    agc7648sv1_i2c_device_num(32),
    agc7648sv1_i2c_device_num(33),
    agc7648sv1_i2c_device_num(34),
    agc7648sv1_i2c_device_num(35),
    agc7648sv1_i2c_device_num(36),
    agc7648sv1_i2c_device_num(37),
    agc7648sv1_i2c_device_num(38),
    agc7648sv1_i2c_device_num(39),
    agc7648sv1_i2c_device_num(40),
    agc7648sv1_i2c_device_num(41),
    agc7648sv1_i2c_device_num(42),
    agc7648sv1_i2c_device_num(43),
    agc7648sv1_i2c_device_num(44),
    agc7648sv1_i2c_device_num(45),
    agc7648sv1_i2c_device_num(46),
    agc7648sv1_i2c_device_num(47),
    agc7648sv1_i2c_device_num(48),
    agc7648sv1_i2c_device_num(49),
    agc7648sv1_i2c_device_num(50),
    agc7648sv1_i2c_device_num(51),
    agc7648sv1_i2c_device_num(52),
    agc7648sv1_i2c_device_num(53),
    agc7648sv1_i2c_device_num(54),
    agc7648sv1_i2c_device_num(55),
    agc7648sv1_i2c_device_num(56),
    agc7648sv1_i2c_device_num(57),
    agc7648sv1_i2c_device_num(58),
    agc7648sv1_i2c_device_num(59),
    agc7648sv1_i2c_device_num(60),
    agc7648sv1_i2c_device_num(61),
    agc7648sv1_i2c_device_num(62),
    agc7648sv1_i2c_device_num(63),
    agc7648sv1_i2c_device_num(64),
    agc7648sv1_i2c_device_num(65),
    agc7648sv1_i2c_device_num(66),
    agc7648sv1_i2c_device_num(67),
};
/* ---------------- I2C device - end ------------- */

/* ---------------- I2C driver - start ------------- */
static int __init i2c_device_probe(struct platform_device *pdev)
{
    struct i2c_device_platform_data *pdata;
    struct i2c_adapter *parent;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "Missing platform data\n");
        return -ENODEV;
    }

    parent = i2c_get_adapter(pdata->parent);
    if (!parent) {
        dev_err(&pdev->dev, "Parent adapter (%d) not found\n",
            pdata->parent);
        return -ENODEV;
    }

    pdata->client = i2c_new_device(parent, &pdata->info);
    if (!pdata->client) {
        dev_err(&pdev->dev, "Failed to create i2c client %s at %d\n",
            pdata->info.type, pdata->parent);
        return -ENODEV;
    }

    return 0; 
}

static int __exit i2c_deivce_remove(struct platform_device *pdev)
{
    struct i2c_adapter *parent;
    struct i2c_device_platform_data *pdata;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "Missing platform data\n");
        return -ENODEV;
    }

    if (pdata->client) {
        parent = (pdata->client)->adapter;
        i2c_unregister_device(pdata->client);
        i2c_put_adapter(parent);
    }

    return 0;
}
static struct platform_driver i2c_device_driver = {
    .probe  = i2c_device_probe,
    .remove = __exit_p(i2c_deivce_remove),
    .driver = {
        .owner = THIS_MODULE,
        .name  = "delta-agc7648sv1-i2c-device",
    }
};
/* ---------------- I2C driver - end ------------- */

/* ---------------- SFP attribute read/write - start -------- */
long sfp_port_data = 0;
static struct kobject *kobj_cpld;
static struct kobject *kobj_swpld1;
static struct kobject *kobj_swpld2;
static struct kobject *kobj_swpld3;

static ssize_t for_status(struct device *dev, struct device_attribute *dev_attr, char *buf){
    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);
    struct device *i2cdev_1 = kobj_to_dev(kobj_swpld1);
    struct device *i2cdev_2 = kobj_to_dev(kobj_swpld2);
    struct cpld_platform_data *pdata1 = i2cdev_1->platform_data;
    struct cpld_platform_data *pdata2 = i2cdev_2->platform_data;
    long port_t = 0;
    u8 reg_t = 0x00;
    int values[7] = {'\0'};
    int bit_t = 0x00;
    int i;
    mutex_lock(&dni_lock);

    switch (attr->index) {
        case SFP_IS_PRESENT:
            port_t = sfp_port_data;
            if (port_t > 0 && port_t < 9) {          /* SFP Port 1-8 */
                reg_t = SFP_PRESENCE_1;
            } else if (port_t > 8 && port_t < 17) {  /* SFP Port 9-16 */
                reg_t = SFP_PRESENCE_2;
            } else if (port_t > 16 && port_t < 25) { /* SFP Port 17-24 */
                reg_t = SFP_PRESENCE_3;
            } else if (port_t > 24 && port_t < 33) { /* SFP Port 25-32 */
                reg_t = SFP_PRESENCE_4;
            } else if (port_t > 32 && port_t < 41) { /* SFP Port 33-40 */
                reg_t = SFP_PRESENCE_5;
            } else if (port_t > 40 && port_t < 49) { /* SFP Port 41-48 */
                reg_t = SFP_PRESENCE_6;
            } else if (port_t > 48 && port_t < 55) { /* QSFP Port 1-6 */
                reg_t = QSFP_PRESENCE;
            } else {
                values[0] = 1; /* return 1, module NOT present */
                mutex_unlock(&dni_lock);
                return sprintf(buf, "%d\n", values[0]);
            }

            if (port_t > 48 && port_t < 55) { /* QSFP */
                VALIDATED_READ(buf, values[0], i2c_smbus_read_byte_data(pdata1[swpld1].client, reg_t), 0);
                mutex_unlock(&dni_lock);
                port_t = port_t - 1;
                bit_t = 1 << (port_t % 8);
                values[0] = values[0] & bit_t;
                values[0] = values[0] / bit_t;
            }
            else { /* SFP */
                VALIDATED_READ(buf, values[0], i2c_smbus_read_byte_data(pdata2[swpld2].client, reg_t), 0);
                mutex_unlock(&dni_lock);
                port_t = port_t - 1;
                bit_t = 1 << (port_t % 8);
                values[0] = values[0] & bit_t;
                values[0] = values[0] / bit_t;
            }

            /* sfp_is_present value
             * return 0 is module present
             * return 1 is module NOT present */
            mutex_unlock(&dni_lock);
            return sprintf(buf, "%d\n", values[0]);

        case SFP_IS_PRESENT_ALL:
            /* Report the SFP/QSFP ALL PRESENCE status
             * This data information form SWPLD2(SFP) and SWPLD1(QSFP). */

            /* SFP_PRESENT Ports 1-8 */
            VALIDATED_READ(buf, values[0],
                i2c_smbus_read_byte_data(pdata2[swpld2].client, SFP_PRESENCE_1), 0);
            /* SFP_PRESENT Ports 9-16 */
            VALIDATED_READ(buf, values[1],
                i2c_smbus_read_byte_data(pdata2[swpld2].client, SFP_PRESENCE_2), 0);
            /* SFP_PRESENT Ports 17-24 */
            VALIDATED_READ(buf, values[2],
                i2c_smbus_read_byte_data(pdata2[swpld2].client, SFP_PRESENCE_3), 0);
            /* SFP_PRESENT Ports 25-32 */
            VALIDATED_READ(buf, values[3],
                i2c_smbus_read_byte_data(pdata2[swpld2].client, SFP_PRESENCE_4), 0);
            /* SFP_PRESENT Ports 33-40 */
            VALIDATED_READ(buf, values[4],
                i2c_smbus_read_byte_data(pdata2[swpld2].client, SFP_PRESENCE_5), 0);
            /* SFP_PRESENT Ports 41-48 */
            VALIDATED_READ(buf, values[5],
                i2c_smbus_read_byte_data(pdata2[swpld2].client, SFP_PRESENCE_6), 0);
            /* QSFP_PRESENT Ports 49-54 */
            VALIDATED_READ(buf, values[6],
                i2c_smbus_read_byte_data(pdata1[swpld1].client, QSFP_PRESENCE), 0);

            values[6] = values[6] & 0x3F;

            /* sfp_is_present_all value
             * return 0 is module present
             * return 1 is module NOT present */
            mutex_unlock(&dni_lock);
            return sprintf(buf, "%02X %02X %02X %02X %02X %02X %02X\n", values[6], values[5], values[4],values[3], values[2], values[1], values[0]);

        case SFP_LP_MODE:
            port_t = sfp_port_data;
            if (port_t > 48 && port_t < 55) { /* QSFP Port 49-54 */
                reg_t = QSFP_LPMODE;
            } else {
                values[0] = 0; /* return 0, module is NOT in LP mode */
                mutex_unlock(&dni_lock);
                return sprintf(buf, "%d\n", values[0]);
            }

            if (port_t > 48 && port_t < 55) { /* QSFP Port 49-54 */
                VALIDATED_READ(buf, values[0], i2c_smbus_read_byte_data(pdata1[swpld1].client, reg_t), 0);
            } else { /* In agc7648sv1 only QSFP support control LP MODE */
                values[0] = 0;
                mutex_unlock(&dni_lock);
                return sprintf(buf, "%d\n", values[0]);
            }
            port_t = port_t - 1;
            bit_t = 1 << (port_t % 8);
            values[0] = values[0] & bit_t;
            values[0] = values[0] / bit_t;

            /* sfp_lp_mode value
             * return 0 is module NOT in LP mode
             * return 1 is module in LP mode */
            mutex_unlock(&dni_lock);
            return sprintf(buf, "%d\n", values[0]);

         case SFP_RESET:
            port_t = sfp_port_data;
            if (port_t > 48 && port_t < 55) { /* QSFP Port 49-54 */
                reg_t = QSFP_RESET;
            } else {
                values[0] = 1; /* return 1, module NOT in reset mode */
                mutex_unlock(&dni_lock);
                return sprintf(buf, "%d\n", values[0]);
            }

            if (port_t > 48 && port_t < 55) { /* QSFP Port 49-54 */
                VALIDATED_READ(buf, values[0], i2c_smbus_read_byte_data(pdata1[swpld1].client, reg_t), 0);
            } else { /* In agc7648sv1 only QSFP support control RESET MODE */
                values[0] = 1;
                mutex_unlock(&dni_lock);
                return sprintf(buf, "%d\n", values[0]);
            }
            port_t = port_t - 1;
            bit_t = 1 << (port_t % 8);
            values[0] = values[0] & bit_t;
            values[0] = values[0] / bit_t;

            /* sfp_reset value
             * return 0 is module Reset
             * return 1 is module Normal */
            mutex_unlock(&dni_lock);
            return sprintf(buf, "%d\n", values[0]);

        case SFP_RX_LOS:
            port_t = sfp_port_data;
            if (port_t > 0 && port_t < 9) {          /* SFP Port 1-8 */
                reg_t = SFP_RXLOS_1;
            } else if (port_t > 8 && port_t < 17) {  /* SFP Port 9-16 */
                reg_t = SFP_RXLOS_2;
            } else if (port_t > 16 && port_t < 25) { /* SFP Port 17-24 */
                reg_t = SFP_RXLOS_3;
            } else if (port_t > 24 && port_t < 33) { /* SFP Port 25-32 */
                reg_t = SFP_RXLOS_4;
            } else if (port_t > 32 && port_t < 41) { /* SFP Port 33-40 */
                reg_t = SFP_RXLOS_5;
            } else if (port_t > 40 && port_t < 49) { /* SFP Port 41-48 */
                reg_t = SFP_RXLOS_6;
            } else {
                values[0] = 1; /* return 1, module Error */
                mutex_unlock(&dni_lock);
                return sprintf(buf, "%d\n", values[0]);
            }

            if (port_t > 0 && port_t < 49) { /* SFP */
                VALIDATED_READ(buf, values[0], i2c_smbus_read_byte_data(pdata2[swpld2].client, reg_t), 0);
            } else { /* In agc7648sv1 only SFP support control RX_LOS MODE */
                values[0] = 1;
                mutex_unlock(&dni_lock);
                return sprintf(buf, "%d\n", values[0]);
            }
            port_t = port_t - 1;
            bit_t = 1 << (port_t % 8);
            values[0] = values[0] & bit_t;
            values[0] = values[0] / bit_t;

            /* sfp_rx_los value
             * return 0 is module Normal Operation
             * return 1 is module Error */
            mutex_unlock(&dni_lock);
            return sprintf(buf, "%d\n", values[0]);

        case SFP_RX_LOS_ALL:
            /* Report the SFP ALL RXLOS status
             * This data information form SWPLD2. */

            /* SFP_RXLOS Ports 1-8 */
            VALIDATED_READ(buf, values[0],
                i2c_smbus_read_byte_data(pdata2[swpld2].client, SFP_RXLOS_1), 0);
            /* SFP_RXLOS Ports 9-16 */
            VALIDATED_READ(buf, values[1],
                i2c_smbus_read_byte_data(pdata2[swpld2].client, SFP_RXLOS_2), 0);
            /* SFP_RXLOS Ports 17-24 */
            VALIDATED_READ(buf, values[2],
                i2c_smbus_read_byte_data(pdata2[swpld2].client, SFP_RXLOS_3), 0);
            /* SFP_RXLOS Ports 25-32 */
            VALIDATED_READ(buf, values[3],
                i2c_smbus_read_byte_data(pdata2[swpld2].client, SFP_RXLOS_4), 0);
            /* SFP_RXLOS Ports 33-40 */
            VALIDATED_READ(buf, values[4],
                i2c_smbus_read_byte_data(pdata2[swpld2].client, SFP_RXLOS_5), 0);
            /* SFP_RXLOS Ports 41-48 */
            VALIDATED_READ(buf, values[5],
                i2c_smbus_read_byte_data(pdata2[swpld2].client, SFP_RXLOS_6), 0);

            /* sfp_rx_los_all value
             * return 0 is module Normal Operation
             * return 1 is module Error */
            mutex_unlock(&dni_lock);
            return sprintf(buf, "%02X %02X %02X %02X %02X %02X\n", values[5], values[4],values[3], values[2], values[1], values[0]);

        case SFP_TX_DISABLE:
            port_t = sfp_port_data;
            if (port_t > 0 && port_t < 9) {          /* SFP Port 1-8 */
                reg_t = SFP_TXDIS_1;
            } else if (port_t > 8 && port_t < 17) {  /* SFP Port 9-16 */
                reg_t = SFP_TXDIS_2;
            } else if (port_t > 16 && port_t < 25) { /* SFP Port 17-24 */
                reg_t = SFP_TXDIS_3;
            } else if (port_t > 24 && port_t < 33) { /* SFP Port 25-32 */
                reg_t = SFP_TXDIS_4;
            } else if (port_t > 32 && port_t < 41) { /* SFP Port 33-40 */
                reg_t = SFP_TXDIS_5;
            } else if (port_t > 40 && port_t < 49) { /* SFP Port 41-48 */
                reg_t = SFP_TXDIS_6;
            } else {
                values[0] = 1; /* return 1, module Transmitter Disabled */
                mutex_unlock(&dni_lock);
                return sprintf(buf, "%d\n", values[0]);
            }

            if (port_t > 0 && port_t < 49) { /* SFP */
                VALIDATED_READ(buf, values[0], i2c_smbus_read_byte_data(pdata2[swpld2].client, reg_t), 0);
            } else { /* In agc7648sv1 only SFP support control TX_DISABLE MODE */
                values[0] = 1;
                mutex_unlock(&dni_lock);
                return sprintf(buf, "%d\n", values[0]);
            }
            port_t = port_t - 1;
            bit_t = 1 << (port_t % 8);
            values[0] = values[0] & bit_t;
            values[0] = values[0] / bit_t;

            /* sfp_tx_disable value
             * return 0 is module Enable Transmitter on
             * return 1 is module Transmitter Disabled */
            mutex_unlock(&dni_lock);
            return sprintf(buf, "%d\n", values[0]);

        case SFP_TX_FAULT:
            port_t = sfp_port_data;
            if (port_t > 0 && port_t < 9) {          /* SFP Port 1-8 */
                reg_t = SFP_TXFAULT_1;
            } else if (port_t > 8 && port_t < 17) {  /* SFP Port 9-16 */
                reg_t = SFP_TXFAULT_2;
            } else if (port_t > 16 && port_t < 25) { /* SFP Port 17-24 */
                reg_t = SFP_TXFAULT_3;
            } else if (port_t > 24 && port_t < 33) { /* SFP Port 25-32 */
                reg_t = SFP_TXFAULT_4;
            } else if (port_t > 32 && port_t < 41) { /* SFP Port 33-40 */
                reg_t = SFP_TXFAULT_5;
            } else if (port_t > 40 && port_t < 49) { /* SFP Port 41-48 */
                reg_t = SFP_TXFAULT_6;
            } else {
                values[0] = 1; /* return 1, module is Fault */
                mutex_unlock(&dni_lock);
                return sprintf(buf, "%d\n", values[0]);
            }

            if (port_t > 0 && port_t < 49) { /* SFP */
                VALIDATED_READ(buf, values[0], i2c_smbus_read_byte_data(pdata2[swpld2].client, reg_t), 0);
            } else { /* In agc7648sv1 only SFP support control TX_FAULT MODE */
                values[0] = 1;
                mutex_unlock(&dni_lock);
                return sprintf(buf, "%d\n", values[0]);
            }
            port_t = port_t - 1;
            bit_t = 1 << (port_t % 8);
            values[0] = values[0] & bit_t;
            values[0] = values[0] / bit_t;

            /* sfp_tx_fault value
             * return 0 is module Normal
             * return 1 is module Fault */
            mutex_unlock(&dni_lock);
            return sprintf(buf, "%d\n", values[0]);

        default:
            mutex_unlock(&dni_lock);
            return sprintf(buf, "%d not found", attr->index);
    }
}

static ssize_t get_port_data(struct device *dev, struct device_attribute *dev_attr, char *buf)
{
    return sprintf(buf, "%ld\n", sfp_port_data);
}

static ssize_t set_port_data(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)
{
    long data;
    int error;
    
    error = kstrtol(buf, 10, &data);
    if(error)
        return error;

    if(data < 1 || data > 54) /* valid port is 1-54 */
    {
        printk(KERN_ALERT "select port out of range (1-54)\n");
        return count;
    }
    else
        sfp_port_data = data;

    return count;
}

static ssize_t set_lpmode_data(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)
{
    struct device *i2cdev = kobj_to_dev(kobj_swpld1);
    struct cpld_platform_data *pdata = i2cdev->platform_data;
    long data;
    int error;
    long port_t = 0;
    int bit_t = 0x00;
    int values = 0x00;
    u8 reg_t = 0x00;

    error = kstrtol(buf, 10, &data);
    if (error)
        return error;
    mutex_lock(&dni_lock);
    port_t = sfp_port_data;
    if (port_t > 48 && port_t < 55) { /* QSFP Port 49-54 */
        reg_t = QSFP_LPMODE;
    } else {
        values = 0; /* return 0, module NOT in LP mode */
        mutex_unlock(&dni_lock);
        return sprintf(buf, "%d\n", values);
    }

    values = i2c_smbus_read_byte_data(pdata[swpld1].client, reg_t);
    if (values < 0){
        mutex_unlock(&dni_lock);
        return -EIO;
    }
    /* Indicate the module is in LP mode or not
     * 0 = Disable
     * 1 = Enable */
    port_t = port_t - 1;
    if (data == 0)
    {
        bit_t = ~(1 << (port_t % 8));
        values = values & bit_t;
    }
    else if (data == 1){
        bit_t = (1 << (port_t % 8));
        values = values | bit_t;
    }
    else
    {
        mutex_unlock(&dni_lock);
        return -EINVAL;
    }
    if (i2c_smbus_write_byte_data(pdata[swpld1].client, reg_t, (u8)values) < 0)
    {
        mutex_unlock(&dni_lock);
        return -EIO;
    }
    mutex_unlock(&dni_lock);
    return count;
}

static ssize_t set_reset_data(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)
{
    struct device *i2cdev = kobj_to_dev(kobj_swpld1);
    struct cpld_platform_data *pdata = i2cdev->platform_data;
    long data;
    int error;
    long port_t = 0;
    int bit_t = 0x00;
    int values = 0x00;
    u8 reg_t = 0x00;

    error = kstrtol(buf, 10, &data);
    if (error)
        return error;

    mutex_lock(&dni_lock);
    port_t = sfp_port_data;

    if (port_t > 48 && port_t < 55) { /* QSFP Port 49-54 */
        reg_t = QSFP_RESET;
    } else {
        values = 1; /* return 1, module NOT in reset mode */
        mutex_unlock(&dni_lock);
        return sprintf(buf, "%d\n", values);
    }

    values = i2c_smbus_read_byte_data(pdata[swpld1].client, reg_t);
    if (values < 0){
        mutex_unlock(&dni_lock);
        return -EIO;
    }
    /* Indicate the module is in reset mode or not
     * 0 = Reset
     * 1 = Normal */
    port_t = port_t - 1;
    if (data == 0)
    {
        bit_t = ~(1 << (port_t % 8));
        values = values & bit_t;
    }
    else if (data == 1)
    {
        bit_t = (1 << (port_t % 8));
        values = values | bit_t;
    }
    else
    {
        mutex_unlock(&dni_lock);
        return -EINVAL;
    }
    if (i2c_smbus_write_byte_data(pdata[swpld1].client, reg_t, (u8)values) < 0)
    {
        mutex_unlock(&dni_lock);
        return -EIO;
    }
    mutex_unlock(&dni_lock);
    return count;
}

static ssize_t set_tx_disable(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)
{
    struct device *i2cdev = kobj_to_dev(kobj_swpld2);
    struct cpld_platform_data *pdata = i2cdev->platform_data;
    long data;
    int error;
    long port_t = 0;
    int bit_t = 0x00;
    int values = 0x00;
    u8 reg_t = 0x00;

    error = kstrtol(buf, 10, &data);
    if (error)
        return error;

    mutex_lock(&dni_lock);
    port_t = sfp_port_data;

	if (port_t > 0 && port_t < 9) {          /* SFP Port 1-8 */
		reg_t = SFP_TXDIS_1;
	} else if (port_t > 8 && port_t < 17) {  /* SFP Port 9-16 */
		reg_t = SFP_TXDIS_2;
	} else if (port_t > 16 && port_t < 25) { /* SFP Port 17-24 */
		reg_t = SFP_TXDIS_3;
	} else if (port_t > 24 && port_t < 33) { /* SFP Port 25-32 */
		reg_t = SFP_TXDIS_4;
	} else if (port_t > 32 && port_t < 41) { /* SFP Port 33-40 */
		reg_t = SFP_TXDIS_5;
	} else if (port_t > 40 && port_t < 49) { /* SFP Port 41-48 */
		reg_t = SFP_TXDIS_6;
    } else {
        values = 1; /* return 1, module NOT in reset mode */
        mutex_unlock(&dni_lock);
        return sprintf(buf, "%d\n", values);
    }

    values = i2c_smbus_read_byte_data(pdata[swpld2].client, reg_t);
    if (values < 0){
        mutex_unlock(&dni_lock);
        return -EIO;
    }
    /* Indicate the module is Enable Transmitter on or not
     * 0 = Enable
     * 1 = Disable */
    port_t = port_t - 1;
    if (data == 0)
    {
        bit_t = ~(1 << (port_t % 8));
        values = values & bit_t;
    }
    else if (data == 1)
    {
        bit_t = (1 << (port_t % 8));
        values = values | bit_t;
    }
    else
    {
        mutex_unlock(&dni_lock);
        return -EINVAL;
    }
    if (i2c_smbus_write_byte_data(pdata[swpld2].client, reg_t, (u8)values) < 0)
    {
        mutex_unlock(&dni_lock);
        return -EIO;
    }
    mutex_unlock(&dni_lock);
    return count;
}
/* ---------------- SFP attribute read/write - end -------- */

/* ---------------- CPLD - start ------------- */
unsigned char cpupld_reg_addr;
unsigned char swpld1_reg_addr;
unsigned char swpld2_reg_addr;
unsigned char swpld3_reg_addr;

/*    CPLD  -- device   */
static struct platform_device cpld_device = {
    .name = "delta-agc7648sv1-cpld",
    .id   = 0,
    .dev = {
        .platform_data  = agc7648sv1_cpld_platform_data,
        .release        = device_release
    },
};

static struct platform_device swpld1_device = {
    .name = "delta-agc7648sv1-swpld1",
    .id   = 0,
    .dev  = {
        .platform_data = agc7648sv1_swpld1_platform_data,
        .release       = device_release
    },
};

static struct platform_device swpld2_device = {
    .name = "delta-agc7648sv1-swpld2",
    .id   = 0,
    .dev  = {
        .platform_data  = agc7648sv1_swpld2_platform_data,
        .release        = device_release
    },
};

static struct platform_device swpld3_device = {
    .name               = "delta-agc7648sv1-swpld3",
    .id                 = 0,
    .dev                = {
                .platform_data   = agc7648sv1_swpld3_platform_data,
                .release         = device_release
    },
};

static ssize_t get_cpld_reg(struct device *dev, struct device_attribute *dev_attr, char *buf) 
{
    int ret;
    int mask;
    int value;
    char note[450];
    unsigned char reg;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);
    struct cpld_platform_data *pdata = dev->platform_data;
    
    mutex_lock(&dni_lock);
    switch (attr->index) {
        case CPLD_REG_ADDR:
            mutex_unlock(&dni_lock);
            return sprintf(buf, "0x%02x\n", cpupld_reg_addr);
        case SWPLD1_REG_ADDR:
            mutex_unlock(&dni_lock);
            return sprintf(buf, "0x%02x\n", swpld1_reg_addr);
        case SWPLD2_REG_ADDR:
            mutex_unlock(&dni_lock);
            return sprintf(buf, "0x%02x\n", swpld2_reg_addr);
        case SWPLD3_REG_ADDR:
            mutex_unlock(&dni_lock);
            return sprintf(buf, "0x%02x\n", swpld3_reg_addr);
        case CPLD_REG_VALUE:
            ret = i2c_smbus_read_byte_data(pdata[system_cpld].client, cpupld_reg_addr);
            mutex_unlock(&dni_lock);
            return sprintf(buf, "0x%02x\n", ret);
        case SWPLD1_REG_VALUE:
            ret = i2c_smbus_read_byte_data(pdata[swpld1].client, swpld1_reg_addr);
            mutex_unlock(&dni_lock);
            return sprintf(buf, "0x%02x\n", ret);
        case SWPLD2_REG_VALUE:
            ret = i2c_smbus_read_byte_data(pdata[swpld2].client, swpld2_reg_addr);
            mutex_unlock(&dni_lock);
            return sprintf(buf, "0x%02x\n", ret);
        case SWPLD3_REG_VALUE:
            ret = i2c_smbus_read_byte_data(pdata[swpld3].client, swpld3_reg_addr);
            mutex_unlock(&dni_lock);
            return sprintf(buf, "0x%02x\n", ret);
        case CPU_BOARD_ID1 ... SFP_SEL:
            reg   = attribute_data[attr->index].reg;
            mask  = attribute_data[attr->index].mask;
            value = i2c_smbus_read_byte_data(pdata[DEFAULT_CPLD].client, reg);
            sprintf(note, "\n%s\n",attribute_data[attr->index].note);
            value = value & mask;
            break;
        default:
            mutex_unlock(&dni_lock);
            return sprintf(buf, "%d not found", attr->index);
    }
    
    switch (mask) {
        case 0xff:
            mutex_unlock(&dni_lock);
            return sprintf(buf, "0x%02x%s", value, note);
        case 0x0f:
        case 0x07:
        case 0x03:
            break;
        case 0x0c:
            value = value >> 2;
            break;
        case 0xf0:
        case 0x70:
        case 0x30:
            value = value >> 4;
            break;
        case 0xe0:
            value = value >> 5;
            break;
        case 0xc0:
            value = value >> 6;
            break;
        default :
            value = value >> dni_log2(mask);
            mutex_unlock(&dni_lock);
            return sprintf(buf, "%d%s", value, note);
    }
    mutex_unlock(&dni_lock);
    return sprintf(buf, "0x%02x%s", value, note);
}

static ssize_t set_cpld_reg(struct device *dev, struct device_attribute *dev_attr,
            const char *buf, size_t count)
{
    int err;
    int value;
    int set_data;
    unsigned long set_data_ul;
    unsigned char reg;
    unsigned char mask;  
    unsigned char mask_out;      

    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);
    struct cpld_platform_data *pdata = dev->platform_data;

    err = kstrtoul(buf, 0, &set_data_ul);
    if (err){
        return err;
    }
    
    set_data = (int)set_data_ul;
    if (set_data > 0xff){
        printk(KERN_ALERT "address out of range (0x00-0xFF)\n");
        return count;
    }

    mutex_lock(&dni_lock);
    
    switch (attr->index) {
        case CPLD_REG_ADDR:
            cpupld_reg_addr = set_data;
            mutex_unlock(&dni_lock);
            return count;
        case SWPLD1_REG_ADDR:
            swpld1_reg_addr = set_data;
            mutex_unlock(&dni_lock);
            return count;
        case SWPLD2_REG_ADDR:
            swpld2_reg_addr = set_data;
            mutex_unlock(&dni_lock);
            return count;
        case SWPLD3_REG_ADDR:
            swpld3_reg_addr = set_data;
            mutex_unlock(&dni_lock);
            return count;
        case CPLD_REG_VALUE:
            i2c_smbus_write_byte_data(pdata[system_cpld].client, cpupld_reg_addr, set_data);
            mutex_unlock(&dni_lock);
            return count;
        case SWPLD1_REG_VALUE:
            i2c_smbus_write_byte_data(pdata[swpld1].client, swpld1_reg_addr, set_data);
            mutex_unlock(&dni_lock);
            return count;
        case SWPLD2_REG_VALUE:
            i2c_smbus_write_byte_data(pdata[swpld2].client, swpld2_reg_addr, set_data);
            mutex_unlock(&dni_lock);
            return count;
        case SWPLD3_REG_VALUE:
            i2c_smbus_write_byte_data(pdata[swpld3].client, swpld3_reg_addr, set_data);
            mutex_unlock(&dni_lock);
            return count;
         case CPU_BOARD_ID1 ... SFP_SEL:
            reg   = attribute_data[attr->index].reg;
            mask  = attribute_data[attr->index].mask;
            value = i2c_smbus_read_byte_data(pdata[DEFAULT_CPLD].client, reg);
            mask_out = value & ~(mask);
            break;
        default:
            mutex_unlock(&dni_lock);
            return sprintf(buf, "%d not found", attr->index);
    }

    switch (mask) {
        case 0x03:
        case 0x07:
        case 0x0f:
        case 0xff:
            set_data = mask_out | (set_data & mask);
            break;
        case 0x0c:
            set_data = set_data << 2;
            set_data = mask_out | (set_data & mask);
            break;
        case 0xf0:
        case 0x70:
        case 0x30:
            set_data = set_data << 4;
            set_data = mask_out | (set_data & mask);
            break;
        case 0xe0:
            set_data = set_data << 5;
            set_data = mask_out | (set_data & mask);
            break;
        case 0xc0:
            set_data = set_data << 6;
            set_data = mask_out | (set_data & mask);
            break;      
        default :
            set_data = mask_out | (set_data << dni_log2(mask) );        
    }   

    switch (attr->index) {
        case CPU_BOARD_ID1 ... SFP_SEL:
            i2c_smbus_write_byte_data(pdata[DEFAULT_CPLD].client, reg, set_data);
            mutex_unlock(&dni_lock);
            break;
        default:
            mutex_unlock(&dni_lock);
            return sprintf(buf, "cpld not found"); 
    }

    return count;
}

//address and value
static SENSOR_DEVICE_ATTR(cpld_reg_addr,    S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, CPLD_REG_ADDR);
static SENSOR_DEVICE_ATTR(cpld_reg_value,   S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, CPLD_REG_VALUE);
static SENSOR_DEVICE_ATTR(swpld1_reg_addr,  S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, SWPLD1_REG_ADDR);
static SENSOR_DEVICE_ATTR(swpld1_reg_value, S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, SWPLD1_REG_VALUE);
static SENSOR_DEVICE_ATTR(swpld2_reg_addr,  S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, SWPLD2_REG_ADDR);
static SENSOR_DEVICE_ATTR(swpld2_reg_value, S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, SWPLD2_REG_VALUE);
static SENSOR_DEVICE_ATTR(swpld3_reg_addr,  S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, SWPLD3_REG_ADDR);
static SENSOR_DEVICE_ATTR(swpld3_reg_value, S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, SWPLD3_REG_VALUE);

//CPLD
static SENSOR_DEVICE_ATTR(cpu_board_id1,       S_IRUGO,           get_cpld_reg, NULL,         CPU_BOARD_ID1);
static SENSOR_DEVICE_ATTR(cpu_board_id2,       S_IRUGO,           get_cpld_reg, NULL,         CPU_BOARD_ID2);
static SENSOR_DEVICE_ATTR(board_ver,           S_IRUGO,           get_cpld_reg, NULL,         BOARD_VER);
static SENSOR_DEVICE_ATTR(cpuld_ver,           S_IRUGO,           get_cpld_reg, NULL,         CPULD_VER);
static SENSOR_DEVICE_ATTR(cpu_sys_pwr_ok,      S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, CPU_SYS_PWR_OK);
static SENSOR_DEVICE_ATTR(plat_rst,            S_IRUGO,           get_cpld_reg, NULL,         PLAT_RST);
static SENSOR_DEVICE_ATTR(cpld_vr_hot,         S_IRUGO,           get_cpld_reg, NULL,         CPLD_VR_HOT);
static SENSOR_DEVICE_ATTR(cpu_over_tmp,        S_IRUGO,           get_cpld_reg, NULL,         CPU_OVER_TMP);
static SENSOR_DEVICE_ATTR(ddr_over_tmp,        S_IRUGO,           get_cpld_reg, NULL,         DDR_OVER_TMP);
static SENSOR_DEVICE_ATTR(cpu_pwr_rst,         S_IRUGO,           get_cpld_reg, NULL,         CPU_PWR_RST);
static SENSOR_DEVICE_ATTR(cpu_hard_rst,        S_IRUGO,           get_cpld_reg, NULL,         CPU_HARD_RST);
static SENSOR_DEVICE_ATTR(cpld_rst,            S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, CPLD_RST);
static SENSOR_DEVICE_ATTR(mb_pwr_enable,       S_IRUGO,           get_cpld_reg, NULL,         MB_PWR_ENABLE);
static SENSOR_DEVICE_ATTR(mb_pwr_pgd,          S_IRUGO,           get_cpld_reg, NULL,         MB_PWR_PGD);
static SENSOR_DEVICE_ATTR(mb_rst_done,         S_IRUGO,           get_cpld_reg, NULL,         MB_RST_DONE);
static SENSOR_DEVICE_ATTR(mb_rst,              S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, MB_RST);
static SENSOR_DEVICE_ATTR(eeprom_wp,           S_IRUGO,           get_cpld_reg, NULL,         EEPROM_WP);
static SENSOR_DEVICE_ATTR(psu_fan_event,       S_IRUGO,           get_cpld_reg, NULL,         PSU_FAN_EVENT);
static SENSOR_DEVICE_ATTR(cpu_i2c_mux_en,      S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, CPU_I2C_MUX_EN);
static SENSOR_DEVICE_ATTR(cpu_i2c_mux_sel,     S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, CPU_I2C_MUX_SEL);

//SWPLD1
static SENSOR_DEVICE_ATTR(board_id,        S_IRUGO,           get_cpld_reg, NULL,         BOARD_ID);
static SENSOR_DEVICE_ATTR(bcm88375_rst,    S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, BCM88375_RST);
static SENSOR_DEVICE_ATTR(b54616s_rst,     S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, B54616S_RST);
static SENSOR_DEVICE_ATTR(psu1_en,         S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, PSU1_EN);
static SENSOR_DEVICE_ATTR(psu2_en,         S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, PSU2_EN);
static SENSOR_DEVICE_ATTR(psu1_pwr_fan_ok, S_IRUGO,           get_cpld_reg, NULL,         PSU1_PWR_FAN_OK);
static SENSOR_DEVICE_ATTR(psu2_pwr_fan_ok, S_IRUGO,           get_cpld_reg, NULL,         PSU2_PWR_FAN_OK);
static SENSOR_DEVICE_ATTR(psu2_present,    S_IRUGO,           get_cpld_reg, NULL,         PSU2_PRESENT);
static SENSOR_DEVICE_ATTR(psu1_present,    S_IRUGO,           get_cpld_reg, NULL,         PSU1_PRESENT);
static SENSOR_DEVICE_ATTR(psu2_pwr_int,    S_IRUGO,           get_cpld_reg, NULL,         PSU2_PWR_INT);
static SENSOR_DEVICE_ATTR(psu1_pwr_int,    S_IRUGO,           get_cpld_reg, NULL,         PSU1_PWR_INT);
static SENSOR_DEVICE_ATTR(bcm88375_int,    S_IRUGO,           get_cpld_reg, NULL,         BCM88375_INT);
static SENSOR_DEVICE_ATTR(bcm54616s_irq,   S_IRUGO,           get_cpld_reg, NULL,         BCM54616S_IRQ);
static SENSOR_DEVICE_ATTR(led_sys,         S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, LED_SYS);
static SENSOR_DEVICE_ATTR(led_pwr,         S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, LED_PWR);
static SENSOR_DEVICE_ATTR(led_fan,         S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, LED_FAN);
static SENSOR_DEVICE_ATTR(psu_i2c_sel,     S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, PSU_I2C_SEL);
static SENSOR_DEVICE_ATTR(fan1_led,        S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, FAN1_LED);
static SENSOR_DEVICE_ATTR(fan2_led,        S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, FAN2_LED);
static SENSOR_DEVICE_ATTR(fan3_led,        S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, FAN3_LED);
static SENSOR_DEVICE_ATTR(fan4_led,        S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, FAN4_LED);
static SENSOR_DEVICE_ATTR(fan_i2c_sel,     S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, FAN_I2C_SEL);

//SWPLD3
static SENSOR_DEVICE_ATTR(qsfp_i2c_sel,    S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, QSFP_I2C_SEL);
static SENSOR_DEVICE_ATTR(sfp_chan_en,     S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, SFP_CHAN_EN);
static SENSOR_DEVICE_ATTR(sfp_sel,         S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, SFP_SEL);

//SFP, QSFP
static SENSOR_DEVICE_ATTR(sfp_select_port,     S_IRUGO | S_IWUSR, get_port_data,   set_port_data,     SFP_SELECT_PORT);
static SENSOR_DEVICE_ATTR(sfp_is_present,      S_IRUGO,           for_status,      NULL,              SFP_IS_PRESENT);
static SENSOR_DEVICE_ATTR(sfp_is_present_all,  S_IRUGO,           for_status,      NULL,              SFP_IS_PRESENT_ALL);
static SENSOR_DEVICE_ATTR(sfp_lp_mode,         S_IWUSR | S_IRUGO, for_status,      set_lpmode_data,   SFP_LP_MODE);
static SENSOR_DEVICE_ATTR(sfp_reset,           S_IWUSR | S_IRUGO, for_status,      set_reset_data,    SFP_RESET);
static SENSOR_DEVICE_ATTR(sfp_rx_los,          S_IRUGO,           for_status,      NULL,              SFP_RX_LOS);
static SENSOR_DEVICE_ATTR(sfp_rx_los_all,      S_IRUGO,           for_status,      NULL,              SFP_RX_LOS_ALL);
static SENSOR_DEVICE_ATTR(sfp_tx_disable,      S_IWUSR | S_IRUGO, for_status,      set_tx_disable,    SFP_TX_DISABLE);
static SENSOR_DEVICE_ATTR(sfp_tx_fault,        S_IRUGO,           for_status,      NULL,              SFP_TX_FAULT);

static struct attribute *cpld_attrs[] = {
    &sensor_dev_attr_cpld_reg_value.dev_attr.attr,
    &sensor_dev_attr_cpld_reg_addr.dev_attr.attr,
    &sensor_dev_attr_cpu_board_id1.dev_attr.attr,
    &sensor_dev_attr_cpu_board_id2.dev_attr.attr,
    &sensor_dev_attr_board_ver.dev_attr.attr,
    &sensor_dev_attr_cpuld_ver.dev_attr.attr,
    &sensor_dev_attr_cpu_sys_pwr_ok.dev_attr.attr,
    &sensor_dev_attr_plat_rst.dev_attr.attr,
    &sensor_dev_attr_cpld_vr_hot.dev_attr.attr,
    &sensor_dev_attr_cpu_over_tmp.dev_attr.attr,
    &sensor_dev_attr_ddr_over_tmp.dev_attr.attr,
    &sensor_dev_attr_cpu_pwr_rst.dev_attr.attr,
    &sensor_dev_attr_cpu_hard_rst.dev_attr.attr,
    &sensor_dev_attr_cpld_rst.dev_attr.attr,
    &sensor_dev_attr_mb_pwr_enable.dev_attr.attr,
    &sensor_dev_attr_mb_pwr_pgd.dev_attr.attr,
    &sensor_dev_attr_mb_rst_done.dev_attr.attr,
    &sensor_dev_attr_mb_rst.dev_attr.attr,
    &sensor_dev_attr_eeprom_wp.dev_attr.attr,
    &sensor_dev_attr_psu_fan_event.dev_attr.attr,
    &sensor_dev_attr_cpu_i2c_mux_en.dev_attr.attr,
    &sensor_dev_attr_cpu_i2c_mux_sel.dev_attr.attr,
    NULL,
};

static struct attribute *swpld1_attrs[] = {
    //SWPLD1
    &sensor_dev_attr_swpld1_reg_value.dev_attr.attr,
    &sensor_dev_attr_swpld1_reg_addr.dev_attr.attr, 
    &sensor_dev_attr_board_id.dev_attr.attr,
    &sensor_dev_attr_bcm88375_rst.dev_attr.attr,
    &sensor_dev_attr_b54616s_rst.dev_attr.attr,
    &sensor_dev_attr_psu1_en.dev_attr.attr,
    &sensor_dev_attr_psu2_en.dev_attr.attr,
    &sensor_dev_attr_psu1_pwr_fan_ok.dev_attr.attr,
    &sensor_dev_attr_psu2_pwr_fan_ok.dev_attr.attr,
    &sensor_dev_attr_psu2_present.dev_attr.attr,
    &sensor_dev_attr_psu1_present.dev_attr.attr,
    &sensor_dev_attr_psu2_pwr_int.dev_attr.attr,
    &sensor_dev_attr_psu1_pwr_int.dev_attr.attr,
    &sensor_dev_attr_bcm88375_int.dev_attr.attr,
    &sensor_dev_attr_bcm54616s_irq.dev_attr.attr,
    &sensor_dev_attr_led_sys.dev_attr.attr,
    &sensor_dev_attr_led_pwr.dev_attr.attr,
    &sensor_dev_attr_led_fan.dev_attr.attr,
    &sensor_dev_attr_psu_i2c_sel.dev_attr.attr,
    &sensor_dev_attr_fan1_led.dev_attr.attr,
    &sensor_dev_attr_fan2_led.dev_attr.attr,
    &sensor_dev_attr_fan3_led.dev_attr.attr,
    &sensor_dev_attr_fan4_led.dev_attr.attr,
    &sensor_dev_attr_fan_i2c_sel.dev_attr.attr,
    //SFP, QSFP
    &sensor_dev_attr_sfp_select_port.dev_attr.attr,
    &sensor_dev_attr_sfp_is_present.dev_attr.attr,
    &sensor_dev_attr_sfp_is_present_all.dev_attr.attr,
    &sensor_dev_attr_sfp_lp_mode.dev_attr.attr,
    &sensor_dev_attr_sfp_reset.dev_attr.attr,
    &sensor_dev_attr_sfp_rx_los.dev_attr.attr,
    &sensor_dev_attr_sfp_rx_los_all.dev_attr.attr,
    &sensor_dev_attr_sfp_tx_disable.dev_attr.attr,
    &sensor_dev_attr_sfp_tx_fault.dev_attr.attr,
    NULL,
};

static struct attribute *swpld2_attrs[] = {
    &sensor_dev_attr_swpld2_reg_value.dev_attr.attr,
    &sensor_dev_attr_swpld2_reg_addr.dev_attr.attr,
    NULL,
};

static struct attribute *swpld3_attrs[] = {
    &sensor_dev_attr_swpld3_reg_value.dev_attr.attr,
    &sensor_dev_attr_swpld3_reg_addr.dev_attr.attr, 
    &sensor_dev_attr_qsfp_i2c_sel.dev_attr.attr,
    &sensor_dev_attr_sfp_chan_en.dev_attr.attr,
    &sensor_dev_attr_sfp_sel.dev_attr.attr,
    NULL,
};

static struct attribute_group cpld_attr_grp = {
    .attrs = cpld_attrs,
};

static struct attribute_group swpld1_attr_grp = {
    .attrs = swpld1_attrs,
};

static struct attribute_group swpld2_attr_grp = {
    .attrs = swpld2_attrs,
};

static struct attribute_group swpld3_attr_grp = {
    .attrs = swpld3_attrs,
};

static int __init cpld_probe(struct platform_device *pdev)
{
    struct cpld_platform_data *pdata;
    struct i2c_adapter *parent;
    int ret;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "CPLD platform data not found\n");
        return -ENODEV;
    }

    parent = i2c_get_adapter(BUS0);
    if (!parent) {
        printk(KERN_WARNING "Parent adapter (%d) not found\n", BUS0);
        return -ENODEV;
    }

    pdata[system_cpld].client = i2c_new_dummy(parent, pdata[system_cpld].reg_addr);
    if (!pdata[system_cpld].client) {
        printk(KERN_WARNING "Fail to create dummy i2c client for addr %d\n", pdata[system_cpld].reg_addr);
        goto error;
    }

    kobj_cpld = &pdev->dev.kobj;
    ret = sysfs_create_group(&pdev->dev.kobj, &cpld_attr_grp);
    if (ret) {
        printk(KERN_WARNING "Fail to create cpld attribute group");
        goto error;
    }

    return 0;

error:
    kobject_put(kobj_cpld); 
    i2c_unregister_device(pdata[system_cpld].client);
    i2c_put_adapter(parent);

    return -ENODEV;
}

static int __init swpld1_probe(struct platform_device *pdev)
{
    struct cpld_platform_data *pdata;
    struct i2c_adapter *parent;
    int ret;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "SWPLD1 platform data not found\n");
        return -ENODEV;
    }
    parent = i2c_get_adapter(BUS7);
    if (!parent) {
        printk(KERN_WARNING "Parent adapter (%d) not found\n", BUS7);
        return -ENODEV;
    }

    pdata[swpld1].client = i2c_new_dummy(parent, pdata[swpld1].reg_addr);
    if (!pdata[swpld1].client) {
        printk(KERN_WARNING "Fail to create dummy i2c client for addr %d\n", pdata[swpld1].reg_addr);
        goto error;
    }

    kobj_swpld1 = &pdev->dev.kobj;
    ret = sysfs_create_group(&pdev->dev.kobj, &swpld1_attr_grp);
    if (ret) {
        printk(KERN_WARNING "Fail to create swpld attribute group");
        goto error;
    }
    return 0;

error:
    kobject_put(kobj_swpld1); 
    i2c_unregister_device(pdata[swpld1].client);
    i2c_put_adapter(parent);
    return -ENODEV;
}

static int __init swpld2_probe(struct platform_device *pdev)
{
    struct cpld_platform_data *pdata;
    struct i2c_adapter *parent;
    int ret;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "SWPLD2 platform data not found\n");
        return -ENODEV;
    }

    parent = i2c_get_adapter(BUS7);
    if (!parent) {
        printk(KERN_WARNING "Parent adapter (%d) not found\n", BUS7);
        return -ENODEV;
    }

    pdata[swpld2].client = i2c_new_dummy(parent, pdata[swpld2].reg_addr);
    if (!pdata[swpld2].client) {
        printk(KERN_WARNING "Fail to create dummy i2c client for addr %d\n", pdata[swpld2].reg_addr);
        goto error;
    }

    kobj_swpld2 = &pdev->dev.kobj;
    ret = sysfs_create_group(&pdev->dev.kobj, &swpld2_attr_grp);
    if (ret) {
        printk(KERN_WARNING "Fail to create swpld attribute group");
        goto error;
    }

    return 0;

error:
    kobject_put(kobj_swpld2); 
    i2c_unregister_device(pdata[swpld2].client);
    i2c_put_adapter(parent);

    return -ENODEV;
}

static int __init swpld3_probe(struct platform_device *pdev)
{
    struct cpld_platform_data *pdata;
    struct i2c_adapter *parent;
    int ret;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "SWPLD3 platform data not found\n");
        return -ENODEV;
    }

    parent = i2c_get_adapter(BUS7);
    if (!parent) {
        printk(KERN_WARNING "Parent adapter (%d) not found\n", BUS7);
        return -ENODEV;
    }

    pdata[swpld3].client = i2c_new_dummy(parent, pdata[swpld3].reg_addr);
    if (!pdata[swpld3].client) {
        printk(KERN_WARNING "Fail to create dummy i2c client for addr %d\n", pdata[swpld3].reg_addr);
        goto error;
    }

    kobj_swpld3 = &pdev->dev.kobj;
    ret = sysfs_create_group(&pdev->dev.kobj, &swpld3_attr_grp);
    if (ret) {
        printk(KERN_WARNING "Fail to create swpld attribute group");
        goto error;
    }

    return 0;

error:
    kobject_put(kobj_swpld3); 
    i2c_unregister_device(pdata[swpld3].client);
    i2c_put_adapter(parent);

    return -ENODEV;
}

static int __exit cpld_remove(struct platform_device *pdev)
{
    struct i2c_adapter *parent = NULL;
    struct cpld_platform_data *pdata = pdev->dev.platform_data;
    sysfs_remove_group(&pdev->dev.kobj, &cpld_attr_grp);

    if (!pdata) {
        dev_err(&pdev->dev, "Missing platform data\n");
    }
    else {
        if (pdata[system_cpld].client) {
            if (!parent) {
                parent = (pdata[system_cpld].client)->adapter;
            }
        i2c_unregister_device(pdata[system_cpld].client);
        }
    }
    i2c_put_adapter(parent);

    return 0;
}

static int __exit swpld1_remove(struct platform_device *pdev)
{
    struct i2c_adapter *parent = NULL;
    struct cpld_platform_data *pdata = pdev->dev.platform_data;
    sysfs_remove_group(&pdev->dev.kobj, &swpld1_attr_grp);

    if (!pdata) {
        dev_err(&pdev->dev, "Missing platform data\n");
    }
    else {
        if (pdata[swpld1].client) {
            if (!parent) {
                parent = (pdata[swpld1].client)->adapter;
            }
        i2c_unregister_device(pdata[swpld1].client);
        }
    }
    i2c_put_adapter(parent);
    return 0;
}

static int __exit swpld2_remove(struct platform_device *pdev)
{
    struct i2c_adapter *parent = NULL;
    struct cpld_platform_data *pdata = pdev->dev.platform_data;
    sysfs_remove_group(&pdev->dev.kobj, &swpld2_attr_grp);

    if (!pdata) {
        dev_err(&pdev->dev, "Missing platform data\n");
    }
    else {
        if (pdata[swpld2].client) {
            if (!parent) {
                parent = (pdata[swpld2].client)->adapter;
            }
        i2c_unregister_device(pdata[swpld2].client);
        }
    }
    i2c_put_adapter(parent);
    return 0;
}

static int __exit swpld3_remove(struct platform_device *pdev)
{
    struct i2c_adapter *parent = NULL;
    struct cpld_platform_data *pdata = pdev->dev.platform_data;
    sysfs_remove_group(&pdev->dev.kobj, &swpld3_attr_grp);

    if (!pdata) {
        dev_err(&pdev->dev, "Missing platform data\n");
    }
    else {
        if (pdata[swpld3].client) {
            if (!parent) {
                parent = (pdata[swpld3].client)->adapter;
            }
        i2c_unregister_device(pdata[swpld3].client);
        }
    }
    i2c_put_adapter(parent);
    return 0;
}

static struct platform_driver cpld_driver = {
    .probe  = cpld_probe,
    .remove = __exit_p(cpld_remove),
    .driver = {
        .owner = THIS_MODULE,
        .name  = "delta-agc7648sv1-cpld",
    },
};

static struct platform_driver swpld1_driver = {
    .probe  = swpld1_probe,
    .remove = __exit_p(swpld1_remove),
    .driver = {
        .owner = THIS_MODULE,
        .name  = "delta-agc7648sv1-swpld1",
    },
};

static struct platform_driver swpld2_driver = {
    .probe  = swpld2_probe,
    .remove = __exit_p(swpld2_remove),
    .driver = {
        .owner = THIS_MODULE,
        .name  = "delta-agc7648sv1-swpld2",
    },
};

static struct platform_driver swpld3_driver = {
    .probe  = swpld3_probe,
    .remove = __exit_p(swpld3_remove),
    .driver = {
        .owner = THIS_MODULE,
        .name  = "delta-agc7648sv1-swpld3",
    },
};
/* ---------------- CPLD - end ------------- */

/* ---------------- MUX - start ------------- */
struct cpld_mux_platform_data {
    int parent;
    int base_nr;
    struct i2c_client *cpld;
    int reg_addr;
};

struct cpld_mux {
    struct i2c_adapter *parent;
    struct i2c_adapter **child;
    struct cpld_mux_platform_data data;
};

static struct cpld_mux_platform_data agc7648sv1_cpld_mux_platform_data[] = {
    {
        .parent         = BUS0,
        .base_nr        = BUS0_BASE_NUM,
        .cpld           = NULL,
        .reg_addr       = BUS0_MUX_REG, 
    },
};

static struct cpld_mux_platform_data agc7648sv1_swpld1_mux_platform_data[] = {
    {
        .parent         = BUS5,
        .base_nr        = BUS5_BASE_NUM,
        .cpld           = NULL,
        .reg_addr       = BUS5_MUX_REG,
    },
    {
        .parent         = BUS6,
        .base_nr        = BUS6_BASE_NUM,
        .cpld           = NULL,
        .reg_addr       = BUS6_MUX_REG,
    },
};

static struct cpld_mux_platform_data agc7648sv1_swpld3_mux_platform_data[] = {
    {
        .parent         = BUS2,
        .base_nr        = BUS2_QSFP_BASE_NUM,
        .cpld           = NULL,
        .reg_addr       = BUS2_QSFP_MUX_REG,
    },
    {
        .parent         = BUS2,
        .base_nr        = BUS2_SFP_BASE_NUM,
        .cpld           = NULL,
        .reg_addr       = BUS2_SFP_MUX_REG,
    },
};

static struct platform_device cpld_mux_device[] = 
{
    {
        .name           = "delta-agc7648sv1-cpld-mux",
        .id             = 0,
        .dev            = {
                .platform_data   = &agc7648sv1_cpld_mux_platform_data[0],
                .release         = device_release,
        },
    },
};

static struct platform_device swpld1_mux_device[] = 
{
    {
        .name = "delta-agc7648sv1-swpld1-mux",
        .id   = 0,
        .dev  = {
            .platform_data = &agc7648sv1_swpld1_mux_platform_data[0],
            .release       = device_release,
        },
    },
    {
        .name = "delta-agc7648sv1-swpld1-mux",
        .id   = 1,
        .dev  = {
            .platform_data = &agc7648sv1_swpld1_mux_platform_data[1],
            .release       = device_release,
        },
    },
};

static struct platform_device swpld3_mux_device[] =
{
    {
        .name = "delta-agc7648sv1-swpld3-mux",
        .id   = 0,
        .dev  = {
            .platform_data  = &agc7648sv1_swpld3_mux_platform_data[0],
            .release        = device_release,
        },
    },
    {
        .name = "delta-agc7648sv1-swpld3-mux",
        .id   = 1,
        .dev  = {
            .platform_data  = &agc7648sv1_swpld3_mux_platform_data[1],
            .release        = device_release,
        },
    },
};

static int cpld_reg_write_byte(struct i2c_client *client, u8 regaddr, u8 val)
{
    union i2c_smbus_data data;

    data.byte = val;
    return client->adapter->algo->smbus_xfer(client->adapter, client->addr,
                                             client->flags,
                                             I2C_SMBUS_WRITE,
                                             regaddr, I2C_SMBUS_BYTE_DATA, &data);
}

static int cpld_mux_select(struct i2c_mux_core *muxc, u32 chan)
{
    struct cpld_mux  *mux = i2c_mux_priv(muxc);
    u8 cpld_mux_val = 0;
    
    if ( mux->data.base_nr == BUS0_BASE_NUM ){
        switch (chan) {
            case 0:
                cpld_mux_val = MUX_VAL_IDEEPROM;
                break;
            case 1:
                cpld_mux_val = MUX_VAL_SERDES_SWPLD3;
                break;
            case 2:
            default:
                cpld_mux_val = MUX_VAL_PCA9547;
                break;
        }
    }
    else
    {
        printk(KERN_ERR "CPLD mux select error\n");
        return 0;
    }
    return cpld_reg_write_byte(mux->data.cpld, mux->data.reg_addr, (u8)(cpld_mux_val & 0xff));
}

static int swpld1_mux_select(struct i2c_mux_core *muxc, u32 chan)
{
    struct cpld_mux  *mux = i2c_mux_priv(muxc);
    u8 swpld1_mux_val = 0;
    u8 bmc_swpld1_mux_val = 0;
    int ret;
    uint8_t cmd_data[4] = {0};
    uint8_t set_cmd;
    int cmd_data_len;

    ret = dni_bmc_exist_check();
    if(ret == 0) //BMC monitor on
    {
        if ( mux->data.base_nr == BUS5_BASE_NUM ){
            switch (chan) {
                case 0:
                    bmc_swpld1_mux_val = MUX_VAL_FAN_TMP75;
                    break;
                case 1:
                    bmc_swpld1_mux_val = MUX_VAL_FAN_IO_CTL;
                    break;
                case 2:
                    bmc_swpld1_mux_val = (MUX_VAL_FAN1_EEPROM + 0x08);
                    break;
                case 3:
                    bmc_swpld1_mux_val = (MUX_VAL_FAN2_EEPROM + 0x09);
                    break;
                case 4:
                    bmc_swpld1_mux_val = (MUX_VAL_FAN3_EEPROM + 0x09);
                    break;
                case 5:
                    bmc_swpld1_mux_val = (MUX_VAL_FAN4_EEPROM + 0x09);
                    break;
                case 6:
                    bmc_swpld1_mux_val = (MUX_VAL_FAN_CTL + 0x08);
                    break;
                default:
                    bmc_swpld1_mux_val = (MUX_VAL_FAN_CTL + 0x08);
                    break;
            }

            set_cmd = CMD_SETDATA;
            cmd_data[0] = BMC_SWPLD_BUS;
            cmd_data[1] = SWPLD1_ADDR;
            cmd_data[2] = BUS5_MUX_REG;
            cmd_data[3] = bmc_swpld1_mux_val;
            cmd_data_len = sizeof(cmd_data);
        }
        else if ( mux->data.base_nr == BUS6_BASE_NUM ){
            switch (chan) {
                case 0:
                    bmc_swpld1_mux_val = MUX_VAL_PSU1;
                    break;
                case 1:
                    bmc_swpld1_mux_val = MUX_VAL_PSU2;
                    break;
                default:
                    bmc_swpld1_mux_val = MUX_VAL_PSU1;
                    break;
            }

            set_cmd = CMD_SETDATA;
            cmd_data[0] = BMC_SWPLD_BUS;
            cmd_data[1] = SWPLD1_ADDR;
            cmd_data[2] = BUS6_MUX_REG;
            cmd_data[3] = bmc_swpld1_mux_val;
            cmd_data_len = sizeof(cmd_data);
        }
        else
        {
            printk(KERN_ERR "SWPLD1 mux select error\n");
            return 0;
        }
        return dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
    }
    else //BMC monitor off or BMC is not exist
    {
        if ( mux->data.base_nr == BUS5_BASE_NUM ){
            switch (chan) {
                case 0:
                    swpld1_mux_val = MUX_VAL_FAN1_EEPROM;
                    break;
                case 1:
                    swpld1_mux_val = MUX_VAL_FAN2_EEPROM;
                    break;
                case 2:
                    swpld1_mux_val = MUX_VAL_FAN3_EEPROM;
                    break;
                case 3:
                    swpld1_mux_val = MUX_VAL_FAN4_EEPROM;
                    break;
                case 4:
                    swpld1_mux_val = MUX_VAL_FAN_CTL;
                    break;
                case 5:
                    swpld1_mux_val = MUX_VAL_FAN_TMP75;
                    break;
                case 6:
                    swpld1_mux_val = MUX_VAL_FAN_IO_CTL;
                    break;
                default:
                    swpld1_mux_val = MUX_VAL_FAN_CTL;
                    break;
            }
        }
        else if ( mux->data.base_nr == BUS6_BASE_NUM ){
            switch (chan) {
                case 0:
                    swpld1_mux_val = MUX_VAL_PSU1;
                    break;
                case 1:
                    swpld1_mux_val = MUX_VAL_PSU2;
                    break;
                default:
                    swpld1_mux_val = MUX_VAL_PSU1;
                    break;
            }
        }
        else
        {
            printk(KERN_ERR "SWPLD1 mux select error\n");
            return 0;
        }
        return cpld_reg_write_byte(mux->data.cpld, mux->data.reg_addr, (u8)(swpld1_mux_val & 0xff));
    }
}

static int swpld3_mux_select(struct i2c_mux_core *muxc, u32 chan)
{
    struct cpld_mux  *mux = i2c_mux_priv(muxc);
    struct device *i2cdev = kobj_to_dev(kobj_swpld1);
    struct cpld_platform_data *pdata = i2cdev->platform_data;
    u8 swpld3_mux_val = 0;
    u8 swpld3_qsfp_ch_en = 0;
    u8 swpld1_qsfp_modsel_val = 0;
    int ret;
    uint8_t cmd_data[4] = {0};
    uint8_t set_cmd;
    int cmd_data_len;

    ret = dni_bmc_exist_check();

    if ( mux->data.base_nr == BUS2_QSFP_BASE_NUM ){
        /* Set QSFP module respond */
        swpld1_qsfp_modsel_val = SWPLD1_QSFP_MODSEL_VAL & (~(1 << chan));
        if (ret == 0) //BMC monitor on
        {
            set_cmd = CMD_SETDATA;
            cmd_data[0] = BMC_SWPLD_BUS;
            cmd_data[1] = SWPLD1_ADDR;
            cmd_data[2] = SWPLD1_QSFP_MODSEL_REG;
            cmd_data[3] = swpld1_qsfp_modsel_val;
            cmd_data_len = sizeof(cmd_data);
			dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
        }
        else //BMC monitor off or BMC is not exist
        {
            if (cpld_reg_write_byte(pdata[swpld1].client, SWPLD1_QSFP_MODSEL_REG, swpld1_qsfp_modsel_val) < 0)
                return -EIO;
        }

        /* QSFP channel enable */
        swpld3_qsfp_ch_en |= SWPLD3_QSFP_CH_EN << 4;
        if (cpld_reg_write_byte(mux->data.cpld, BUS2_SFP_MUX_REG, swpld3_qsfp_ch_en) < 0)
            return -EIO;

        /* QSFP channel selection */
        swpld3_mux_val = chan;
    }
    else if ( mux->data.base_nr == BUS2_SFP_BASE_NUM ){
        /* Disable all QSFP modules respond */
        swpld1_qsfp_modsel_val |= SWPLD1_QSFP_MODSEL_VAL;
        if (ret == 0) //BMC monitor on
        {
            set_cmd = CMD_SETDATA;
            cmd_data[0] = BMC_SWPLD_BUS;
            cmd_data[1] = SWPLD1_ADDR;
            cmd_data[2] = SWPLD1_QSFP_MODSEL_REG;
            cmd_data[3] = swpld1_qsfp_modsel_val;
            cmd_data_len = sizeof(cmd_data);
			dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
        }
        else //BMC monitor off or BMC is not exist
        {
            if (cpld_reg_write_byte(pdata[swpld1].client, SWPLD1_QSFP_MODSEL_REG, swpld1_qsfp_modsel_val) < 0)
                return -EIO;
        }

        /* SFP port 51-59, 9 ports, chan 0-8 */
        if ( chan < SWPLD3_SFP_PORT_9 ){
            swpld3_qsfp_ch_en |= SWPLD3_SFP_CH1_EN << 4;
            swpld3_mux_val = swpld3_qsfp_ch_en | (chan + 1);
        }
        /* SFP port 60-69, 10 ports, chan 9-18 */
        else if ( chan >= SWPLD3_SFP_PORT_9 && chan < SWPLD3_SFP_PORT_19 ){
            swpld3_qsfp_ch_en |= SWPLD3_SFP_CH2_EN << 4;
            swpld3_mux_val = swpld3_qsfp_ch_en | (chan - SWPLD3_SFP_PORT_9);
        }
        /* SFP port 70-79, 10 ports, chan 19-28 */
        else if ( chan >= SWPLD3_SFP_PORT_19 && chan < SWPLD3_SFP_PORT_29 ){
            swpld3_qsfp_ch_en |= SWPLD3_SFP_CH3_EN << 4;
            swpld3_mux_val = swpld3_qsfp_ch_en | (chan - SWPLD3_SFP_PORT_19);
        }
        /* SFP port 80-89, 10 ports, chan 29-38 */
        else if ( chan >= SWPLD3_SFP_PORT_29 && chan < SWPLD3_SFP_PORT_39 ){
            swpld3_qsfp_ch_en |= SWPLD3_SFP_CH4_EN << 4;
            swpld3_mux_val = swpld3_qsfp_ch_en | (chan - SWPLD3_SFP_PORT_29);
        }
        /* SFP port 90-98, 9 ports, chan 39-47 */
        else if ( chan >= SWPLD3_SFP_PORT_39 && chan < SWPLD3_SFP_PORT_48 ){
            swpld3_qsfp_ch_en |= SWPLD3_SFP_CH5_EN << 4;
            swpld3_mux_val = swpld3_qsfp_ch_en | (chan - SWPLD3_SFP_PORT_39);
        }
        else {
            swpld3_qsfp_ch_en |= SWPLD3_SFP_CH_DISABLE << 4;
            swpld3_mux_val = swpld3_qsfp_ch_en;
        }
    }
    else {
        printk(KERN_ERR "SWPLD3 mux select error\n");
        return 0;
    }
    return cpld_reg_write_byte(mux->data.cpld, mux->data.reg_addr, (u8)(swpld3_mux_val & 0xff));
}

static int __init cpld_mux_probe(struct platform_device *pdev)
{
    struct i2c_mux_core *muxc;
    struct cpld_mux *mux;
    struct cpld_mux_platform_data *pdata;
    struct i2c_adapter *parent;
    int i, ret, dev_num;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "CPLD platform data not found\n");
        return -ENODEV;
    }
    mux = kzalloc(sizeof(*mux), GFP_KERNEL);
    if (!mux) {
        printk(KERN_ERR "Failed to allocate memory for mux\n");
        return -ENOMEM;
    }
    mux->data = *pdata;
    parent = i2c_get_adapter(pdata->parent);
    if (!parent) {
        kfree(mux);
        dev_err(&pdev->dev, "Parent adapter (%d) not found\n", pdata->parent);
        return -ENODEV;
    }
    /* Judge bus number to decide how many devices*/
    switch (pdata->parent) {
        case BUS0:
            dev_num = BUS0_DEV_NUM;
            break;
        default :
            dev_num = DEF_DEV_NUM;
            break;
    }
    muxc = i2c_mux_alloc(parent, &pdev->dev, dev_num, 0, 0, cpld_mux_select, NULL);
    if (!muxc) {
        ret = -ENOMEM;
        goto alloc_failed;
    }
    muxc->priv = mux;
    platform_set_drvdata(pdev, muxc);

    for (i = 0; i < dev_num; i++)
    {
        int nr = pdata->base_nr + i;
        unsigned int class = 0;
        ret = i2c_mux_add_adapter(muxc, nr, i, class);
        if (ret) {
            dev_err(&pdev->dev, "Failed to add adapter %d\n", i);
            goto add_adapter_failed;
        }
    }
    dev_info(&pdev->dev, "%d port mux on %s adapter\n", dev_num, parent->name);
    return 0;

add_adapter_failed:
    i2c_mux_del_adapters(muxc);
alloc_failed:
    kfree(mux);
    i2c_put_adapter(parent);
    return ret;
}

static int __init swpld1_mux_probe(struct platform_device *pdev)
{
    struct i2c_mux_core *muxc;
    struct cpld_mux *mux;
    struct cpld_mux_platform_data *pdata;
    struct i2c_adapter *parent;
    int i, ret, dev_num;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "SWPLD1 platform data not found\n");
        return -ENODEV;
    }
    mux = kzalloc(sizeof(*mux), GFP_KERNEL);
    if (!mux) {
        printk(KERN_ERR "Failed to allocate memory for mux\n");
        return -ENOMEM;
    }
    mux->data = *pdata;
    parent = i2c_get_adapter(pdata->parent);
    if (!parent) {
        kfree(mux);
        dev_err(&pdev->dev, "Parent adapter (%d) not found\n", pdata->parent);
        return -ENODEV;
    }
    /* Judge bus number to decide how many devices*/
    switch (pdata->parent) {
        case BUS5:
            dev_num = BUS5_DEV_NUM;
            break;
        case BUS6:
            dev_num = BUS6_DEV_NUM;
            break;
        default :
            dev_num = DEF_DEV_NUM;
            break;
    }

    muxc = i2c_mux_alloc(parent, &pdev->dev, dev_num, 0, 0, swpld1_mux_select, NULL);
    if (!muxc) {
        ret = -ENOMEM;
        goto alloc_failed;
    }
    muxc->priv = mux;
    platform_set_drvdata(pdev, muxc);
    for (i = 0; i < dev_num; i++)
    {
        int nr = pdata->base_nr + i;
        unsigned int class = 0;
        ret = i2c_mux_add_adapter(muxc, nr, i, class);
        if (ret) {
            dev_err(&pdev->dev, "Failed to add adapter %d\n", i);
            goto add_adapter_failed;
        }
    }
    dev_info(&pdev->dev, "%d port mux on %s adapter\n", dev_num, parent->name);
    return 0;

add_adapter_failed:
    i2c_mux_del_adapters(muxc);
alloc_failed:
    kfree(mux);
    i2c_put_adapter(parent);

    return ret;
}

static int __init swpld3_mux_probe(struct platform_device *pdev)
{
    struct i2c_mux_core *muxc;
    struct cpld_mux *mux;
    struct cpld_mux_platform_data *pdata;
    struct i2c_adapter *parent;
    int i, ret, dev_num;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        dev_err(&pdev->dev, "SWPLD3 platform data not found\n");
        return -ENODEV;
    }
    mux = kzalloc(sizeof(*mux), GFP_KERNEL);
    if (!mux) {
        printk(KERN_ERR "Failed to allocate memory for mux\n");
        return -ENOMEM;
    }
    mux->data = *pdata;
    parent = i2c_get_adapter(pdata->parent);
    if (!parent) {
        kfree(mux);
        dev_err(&pdev->dev, "Parent adapter (%d) not found\n", pdata->parent);
        return -ENODEV;
    }
    /* Judge bus number to decide how many devices*/
    switch (pdata->base_nr) {
        case BUS2_QSFP_BASE_NUM:
            dev_num = BUS2_QSFP_DEV_NUM;
            break;
        case BUS2_SFP_BASE_NUM:
            dev_num = BUS2_SFP_DEV_NUM;
            break;
        default :
            dev_num = DEF_DEV_NUM;
            break;
    }

    muxc = i2c_mux_alloc(parent, &pdev->dev, dev_num, 0, 0, swpld3_mux_select, NULL);
    if (!muxc) {
        ret = -ENOMEM;
        goto alloc_failed;
    }
    muxc->priv = mux;
    platform_set_drvdata(pdev, muxc);
    for (i = 0; i < dev_num; i++)
    {
        int nr = pdata->base_nr + i;
        unsigned int class = 0;
        ret = i2c_mux_add_adapter(muxc, nr, i, class);
        if (ret) {
            dev_err(&pdev->dev, "Failed to add adapter %d\n", i);
            goto add_adapter_failed;
        }
    }
    dev_info(&pdev->dev, "%d port mux on %s adapter\n", dev_num, parent->name);
    return 0;

add_adapter_failed:
    i2c_mux_del_adapters(muxc);
alloc_failed:
    kfree(mux);
    i2c_put_adapter(parent);

    return ret;
}

static int __exit cpld_mux_remove(struct platform_device *pdev)
{
    struct i2c_mux_core *muxc  = platform_get_drvdata(pdev);
    struct i2c_adapter *parent = muxc->parent;

    i2c_mux_del_adapters(muxc);
    i2c_put_adapter(parent);

    return 0;
}

static int __exit swpld1_mux_remove(struct platform_device *pdev)
{
    struct i2c_mux_core *muxc  = platform_get_drvdata(pdev);
    struct i2c_adapter *parent = muxc->parent;

    i2c_mux_del_adapters(muxc);
    i2c_put_adapter(parent);

    return 0;
}

static int __exit swpld3_mux_remove(struct platform_device *pdev)
{
    struct i2c_mux_core *muxc  = platform_get_drvdata(pdev);
    struct i2c_adapter *parent = muxc->parent;

    i2c_mux_del_adapters(muxc);
    i2c_put_adapter(parent);

    return 0;
}

static struct platform_driver cpld_mux_driver = {
    .probe  = cpld_mux_probe,
    .remove = __exit_p(cpld_mux_remove), /* TODO */
    .driver = {
        .owner = THIS_MODULE,
        .name  = "delta-agc7648sv1-cpld-mux",
    },
};

static struct platform_driver swpld1_mux_driver = {
    .probe  = swpld1_mux_probe,
    .remove = __exit_p(swpld1_mux_remove), /* TODO */
    .driver = {
        .owner = THIS_MODULE,
        .name  = "delta-agc7648sv1-swpld1-mux",
    },
};

static struct platform_driver swpld3_mux_driver = {
    .probe  = swpld3_mux_probe,
    .remove = __exit_p(swpld3_mux_remove), /* TODO */
    .driver = {
        .owner = THIS_MODULE,
        .name  = "delta-agc7648sv1-swpld3-mux",
    },
};
/* ---------------- MUX - end ------------- */

/* ---------------- module initialization ------------- */
static int __init delta_agc7648sv1_platform_init(void)
{
//    struct i2c_client *client;
    struct i2c_adapter *adapter;
    struct cpld_mux_platform_data *cpld_mux_pdata;
    struct cpld_platform_data     *cpld_pdata;
    struct cpld_mux_platform_data *swpld1_mux_pdata;
    struct cpld_platform_data     *swpld1_pdata;
    struct cpld_mux_platform_data *swpld3_mux_pdata;
    struct cpld_platform_data     *swpld3_pdata;
    int ret,i = 0;
    
    mutex_init(&dni_lock);
    printk("agc7648sv1_platform module initialization\n");

    ret = dni_create_user();
    if (ret != 0) {
        printk(KERN_WARNING "Fail to create IPMI user\n");
    }

    ret = platform_driver_register(&cpld_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register cpupld driver\n");
        goto error_cpld_driver;
    }

    // set the SWPLD prob and remove
    ret = platform_driver_register(&swpld1_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register swpld driver\n");
        goto error_swpld1_driver;
    }

    // set the SWPLD prob and remove
    ret = platform_driver_register(&swpld2_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register swpld driver\n");
        goto error_swpld2_driver;
    }

    // set the SWPLD prob and remove
    ret = platform_driver_register(&swpld3_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register swpld driver\n");
        goto error_swpld3_driver;
    }

    // register the mux prob which call the SWPLD
    ret = platform_driver_register(&cpld_mux_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register swpld mux driver\n");
        goto error_cpld_mux_driver;
    }

    // register the mux prob which call the SWPLD
    ret = platform_driver_register(&swpld1_mux_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register swpld1 mux driver\n");
        goto error_swpld1_mux_driver;
    }

    // register the mux prob which call the SWPLD3
    ret = platform_driver_register(&swpld3_mux_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register swpld3 mux driver\n");
        goto error_swpld3_mux_driver;
    }

    // register the i2c devices    
    ret = platform_driver_register(&i2c_device_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register i2c device driver\n");
        goto error_i2c_device_driver;
    }

    // register the CPUPLD
    ret = platform_device_register(&cpld_device);
    if (ret) {
        printk(KERN_WARNING "Fail to create cpupld device\n");
        goto error_cpld_device;
    }

    // link the CPLD and the Mux
    cpld_pdata = agc7648sv1_cpld_platform_data;
    for (i = 0; i < ARRAY_SIZE(cpld_mux_device); i++)
    {
        cpld_mux_pdata = cpld_mux_device[i].dev.platform_data;
        cpld_mux_pdata->cpld = cpld_pdata[system_cpld].client;
        ret = platform_device_register(&cpld_mux_device[i]);
        if (ret) {
            printk(KERN_WARNING "Fail to create swpld mux %d\n", i);
            goto error_cpld_mux;
        }
    }

    adapter = i2c_get_adapter(BUS3);
    i2c_client_9547 = i2c_new_device(adapter, &i2c_info_pca9547[0]);
    i2c_put_adapter(adapter);

    // register the SWPLD1
    ret = platform_device_register(&swpld1_device);
    if (ret) {
        printk(KERN_WARNING "Fail to create swpld1 device\n");
        goto error_swpld1_device;
    }

    // register the SWPLD2
    ret = platform_device_register(&swpld2_device);
    if (ret) {
        printk(KERN_WARNING "Fail to create swpld2 device\n");
        goto error_swpld2_device;
    }

    // register the SWPLD3
    ret = platform_device_register(&swpld3_device);
    if (ret) {
        printk(KERN_WARNING "Fail to create swpld3 device\n");
        goto error_swpld3_device;
    }

    // link the SWPLD1 and the Mux
    swpld1_pdata = agc7648sv1_swpld1_platform_data;
    for (i = 0; i < ARRAY_SIZE(swpld1_mux_device); i++)
    {
        swpld1_mux_pdata = swpld1_mux_device[i].dev.platform_data;
        swpld1_mux_pdata->cpld = swpld1_pdata[swpld1].client;
        ret = platform_device_register(&swpld1_mux_device[i]);
        if (ret) {
            printk(KERN_WARNING "Fail to create swpld1 mux %d\n", i);
            goto error_agc7648sv1_swpld1_mux;
        }
    }

    // link the SWPLD3 and the Mux
    swpld3_pdata = agc7648sv1_swpld3_platform_data;
    for (i = 0; i < ARRAY_SIZE(swpld3_mux_device); i++)
    {
        swpld3_mux_pdata = swpld3_mux_device[i].dev.platform_data;
        swpld3_mux_pdata->cpld = swpld3_pdata[swpld3].client;
        ret = platform_device_register(&swpld3_mux_device[i]);
        if (ret) {
            printk(KERN_WARNING "Fail to create swpld3 mux %d\n", i);
            goto error_agc7648sv1_swpld3_mux;
        }
    }

    for (i = 0; i < ARRAY_SIZE(agc7648sv1_i2c_device); i++)
    {
        ret = platform_device_register(&agc7648sv1_i2c_device[i]);
        if (ret) 
        {
            printk(KERN_WARNING "Fail to create i2c device %d\n", i);
            goto error_agc7648sv1_i2c_device;
        }
    }
    if (ret)
        goto error_cpld_mux;
    return 0;

error_agc7648sv1_i2c_device:
    i--;
    for (; i >= 0; i--) {
        platform_device_unregister(&agc7648sv1_i2c_device[i]);
    }
    i = ARRAY_SIZE(swpld3_mux_device);
error_agc7648sv1_swpld3_mux:
    i--;
    for (; i >= 0; i--) {
        platform_device_unregister(&swpld3_mux_device[i]);
    }
    i = ARRAY_SIZE(swpld1_mux_device);
error_agc7648sv1_swpld1_mux:
    i--;
    for (; i >= 0; i--) {
        platform_device_unregister(&swpld1_mux_device[i]);
    }
    platform_device_unregister(&swpld3_device);
error_swpld3_device:
    platform_device_unregister(&swpld2_device);
error_swpld2_device:
    platform_device_unregister(&swpld1_device);
error_swpld1_device:
    i2c_unregister_device(i2c_client_9547);
    i = ARRAY_SIZE(cpld_mux_device);
error_cpld_mux:
    i--;
    for (; i >= 0; i--) {
        platform_device_unregister(&cpld_mux_device[i]);
    }
    platform_device_unregister(&cpld_device);
error_cpld_device:
    platform_driver_unregister(&i2c_device_driver);
error_i2c_device_driver:
    platform_driver_unregister(&swpld3_mux_driver);
error_swpld3_mux_driver:
    platform_driver_unregister(&swpld1_mux_driver);
error_swpld1_mux_driver:
    platform_driver_unregister(&cpld_mux_driver);
error_cpld_mux_driver:
    platform_driver_unregister(&swpld3_driver);
error_swpld3_driver:
    platform_driver_unregister(&swpld2_driver);
error_swpld2_driver:
    platform_driver_unregister(&swpld1_driver);
error_swpld1_driver:
    platform_driver_unregister(&cpld_driver);
error_cpld_driver:
    return ret;
}

static void __exit delta_agc7648sv1_platform_exit(void)
{
    int i = 0;

    for (i = 0; i < ARRAY_SIZE(agc7648sv1_i2c_device); i++) {
        platform_device_unregister(&agc7648sv1_i2c_device[i]);
    }   

    for (i = 0; i < ARRAY_SIZE(swpld3_mux_device); i++) {
        platform_device_unregister(&swpld3_mux_device[i]);
    }

    for (i = 0; i < ARRAY_SIZE(swpld1_mux_device); i++) {
        platform_device_unregister(&swpld1_mux_device[i]);
    }

    platform_device_unregister(&swpld1_device);
    platform_driver_unregister(&swpld1_driver);

    platform_device_unregister(&swpld2_device);
    platform_driver_unregister(&swpld2_driver);

    platform_device_unregister(&swpld3_device);
    platform_driver_unregister(&swpld3_driver);

    i2c_unregister_device(i2c_client_9547);

    for (i = 0; i < ARRAY_SIZE(cpld_mux_device); i++) {
        platform_device_unregister(&cpld_mux_device[i]);
    }

    platform_driver_unregister(&i2c_device_driver);
    platform_driver_unregister(&swpld3_mux_driver);
    platform_driver_unregister(&swpld1_mux_driver);
    platform_driver_unregister(&cpld_mux_driver);
    platform_device_unregister(&cpld_device);
    platform_driver_unregister(&cpld_driver);    
}

module_init(delta_agc7648sv1_platform_init);
module_exit(delta_agc7648sv1_platform_exit);

MODULE_DESCRIPTION("DELTA agc7648sv1 Platform Support");
MODULE_AUTHOR("Stanley Chi <stanley.chi@deltaww.com>");
MODULE_LICENSE("GPL");
