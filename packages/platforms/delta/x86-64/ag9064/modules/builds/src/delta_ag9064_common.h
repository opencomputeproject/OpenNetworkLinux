#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/completion.h>
#include <linux/ipmi.h>
#include <linux/ipmi_smi.h>
#include <linux/hwmon-sysfs.h>
#include <linux/hwmon.h>

#define CMD_DATA_SIZE         4
#define ATTRIBUTE_NOTE_SIZE 200

#define IPMI_MAX_INTF         4
#define DELTA_NETFN        0x38
#define BMC_BUS_5          0x04
#define CMD_SETDATA        0x03
#define CMD_GETDATA        0x02
#define BMC_ERR              -6

#define CPUPLD_ADDR        0x31
#define SWPLD1_ADDR        0x35
#define SWPLD2_ADDR        0x34
#define SWPLD3_ADDR        0x33
#define SWPLD4_ADDR        0x32
#define QSFP_PORT_MUX_REG  0x13

#define DEFAULT_NUM           1
#define BUS9_DEV_NUM         64
#define BUS9_BASE_NUM        20

extern int dni_bmc_cmd(char set_cmd, char *cmd_data, int cmd_data_len);
extern int dni_create_user(void);
extern unsigned char dni_log2(unsigned char num);

extern void device_release(struct device *dev);
extern void msg_handler(struct ipmi_recv_msg *recv_msg, void* handler_data);
extern void dummy_smi_free(struct ipmi_smi_msg *msg);
extern void dummy_recv_free(struct ipmi_recv_msg *msg);

ipmi_user_t ipmi_mh_user = NULL;
struct ipmi_user_hndl ipmi_hndlrs = { .ipmi_recv_hndl = msg_handler, };
atomic_t dummy_count = ATOMIC_INIT(0);

struct ipmi_smi_msg halt_smi_msg = {
    .done = dummy_smi_free
};

struct ipmi_recv_msg halt_recv_msg = {
    .done = dummy_recv_free
};

enum {
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

struct cpld_attribute_data {
    uint8_t bus;
    uint8_t addr;
    uint8_t reg;
    uint8_t mask;
    char note[ATTRIBUTE_NOTE_SIZE];
};


enum cpld_type {
    system_cpld,
    swpld1,
    swpld2,
    swpld3,
    swpld4,
};

struct cpld_platform_data {
    int reg_addr;
    struct i2c_client *client;
};

struct cpld_platform_data ag9064_cpld_platform_data[] = {
    [system_cpld] = {
        .reg_addr = CPUPLD_ADDR,
    },
};

struct cpld_platform_data ag9064_swpld1_platform_data[] = {
    [swpld1] = {
        .reg_addr = SWPLD1_ADDR,
    },
};

struct cpld_platform_data ag9064_swpld2_platform_data[] = {
    [swpld2] = {
        .reg_addr = SWPLD2_ADDR,
    },
};

struct cpld_platform_data ag9064_swpld3_platform_data[] = {
    [swpld3] = {
        .reg_addr = SWPLD3_ADDR,
    },
};

struct cpld_platform_data ag9064_swpld4_platform_data[] = {
    [swpld4] = {
        .reg_addr = SWPLD4_ADDR,
    },
};

enum cpld_attributes {
    CPLD_REG_ADDR,
    CPLD_REG_VALUE,
    SWPLD1_REG_ADDR,
    SWPLD1_REG_VALUE,
    SWPLD2_REG_ADDR,
    SWPLD2_REG_VALUE,
    SWPLD3_REG_ADDR,
    SWPLD3_REG_VALUE,
    SWPLD4_REG_ADDR,
    SWPLD4_REG_VALUE,
 //CPLD
    CPLD_VER,
    CPU_BOARD_VER,
    CPU_ID,
    MB_ID,
    MB_VER,
    CPU0_PWR_OK,
    PWR_RAIL_OVER_TEMP,
    CPU_DISOMIC_OVER_TEMP,
    DDR_OVER_TEMP,
    CPLD_PWR_ON_RST,
    CPLD_HARD_RST,
    CPLD_RST,
    MB_RST,
    PSU_FAN_INT,
    OP_MODULE_INT,
//SWPLD1
    SWPLD1_MAJOR_VER,
    SWPLD1_MINOR_VER,
    SWPLD1_SCRTCH_REG,
    PSU1_PWR_OK,
    PSU1_INT,
    PSU2_PWR_OK,
    PSU2_INT,
    PSU1_GREEN_LED,
    PSU1_RED_LED,
    PSU2_GREEN_LED,
    PSU2_RED_LED,
    PSU_LED_MODE,
//SWPLD2
    SWPLD2_MAJOR_VER,
    SWPLD2_MINOR_VER,
    SWPLD2_SCRTCH_REG,
    FAN_LED,
    SYS_LED,
    FAN_MOD1_LED,
    FAN_MOD2_LED,
    FAN_MOD3_LED,
    FAN_MOD4_LED,
//SWPLD3
    SWPLD3_MAJOR_VER,
    SWPLD3_MINOR_VER,
    SWPLD3_SCRTCH_REG,
    SB_VER,
    PLATFORM_TYPE,
//SWPLD4
    SW_BOARD_ID1,
    SW_BOARD_ID2,
    SWBD_VER,
    SWPLD4_VER,
    PSU_FAN_EVENT,
    CPU_THERMAL_INT,
    FAN_INT,
    CPLD_SPI_WP,
    RJ45_CONSOLE_SEL,
    SYSTEM_INT,
    CPLD_MB_RST_DONE,
    MB_PWR_OK,
    FAN_EEPROM_WP,
};

struct cpld_attribute_data attribute_data[] = {
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
    [SWPLD3_REG_VALUE] = {
    },
    [SWPLD4_REG_ADDR] = {
    },
    [SWPLD4_REG_VALUE] = {
    },
//CPLD
    [CPLD_VER] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x01,       .mask = 0xFF,
        .note = "CPLD Version, controlled by CPLD editor"
    },
    [CPU_BOARD_VER] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x02,       .mask = 0xF0,
        .note = "\"0x0\": proto A1\n\"0x1\": proto A2\n\"0x2\": PR (Production)"
    },
    [CPU_ID] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x02,       .mask = 0x0F,
        .note = "\"0x0\": C2558 ECC\n\"0x1\": Rangeley ECC\n\"0x2\": BROADWELL-DE ECC"
    },
    [MB_ID] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x03,       .mask = 0xFF,
        .note = "\"0x00\": proto A1\n\"0x01\": proto A2\n\"0x02\": PR (Production)"
    },
    [MB_VER] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x04,       .mask = 0x0F,
        .note = "\"0x0\": proto-A\n\"0x1\": proto-B"
    },
    [CPU0_PWR_OK] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x08,       .mask = 1 << 3,
        .note = "\"1\" = Power rail is good\n\"0\" = Power rail is failed"
    },
    [PWR_RAIL_OVER_TEMP] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x0b,       .mask = 1 << 3,
        .note = "\"1\" = Not over temperature\n\"0\" = Over temperature"
    },
    [CPU_DISOMIC_OVER_TEMP] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x0b,       .mask = 1 << 1,
        .note = "\"1\" = Not over temperature\n\"0\" = Over temperature"
    },
    [DDR_OVER_TEMP] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x0b,       .mask = 1 << 0,
        .note = "\"1\" = Not over temperature\n\"0\" = Over temperature"
    },
    [CPLD_PWR_ON_RST] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x11,       .mask = 1 << 4,
        .note = "\"0\" = Reset\n\"1\" = Normal operation"
    },
    [CPLD_HARD_RST] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x11,       .mask = 1 << 2,
        .note = "\"0\" = Reset\n\"1\" = Normal operation"
    },
    [CPLD_RST] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x11,       .mask = 1 << 0,
        .note = "\"0\" = Reset\n\"1\" = Normal operation"
    },
    [MB_RST] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x12,       .mask = 1 << 0,
        .note = "\"0\" = Reset\n\"1\" = Normal operation"
    },
    [PSU_FAN_INT] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x15,       .mask = 1 << 1,
        .note = "\"0\" = Interrupt occurs\n\"1\" = Interrupt doesn't occur"
    },
    [OP_MODULE_INT] = {
        .bus  = BUS0,       .addr = CPUPLD_ADDR,
        .reg  = 0x15,       .mask = 1 << 0,
        .note = "\"0\" = Interrupt occurs\n\"1\" = Interrupt doesn't occur"
    },
//SWPLD1
    [SWPLD1_MAJOR_VER] = {
        .bus  = BUS0,       .addr = SWPLD1_ADDR,
        .reg  = 0x00,       .mask = 0xF0,
        .note = "CPLD Major Version, controlled by CPLD editor."
    },
    [SWPLD1_MINOR_VER] = {
        .bus  = BUS0,       .addr = SWPLD1_ADDR,
        .reg  = 0x00,       .mask = 0x0F,
        .note = "CPLD Minor Version, controlled by CPLD editor."
    },
    [SWPLD1_SCRTCH_REG] = {
        .bus  = BUS0,       .addr = SWPLD1_ADDR,
        .reg  = 0x01,       .mask = 0xFF,
        .note = "CPLD read/write test register, to provide a way to test CPLD access."
    },
    [PSU1_PWR_OK] = {
        .bus  = BUS0,       .addr = SWPLD1_ADDR,
        .reg  = 0x02,       .mask = 1 << 6,
        .note = "\"0\" = Power rail is good\n\"1\" = Power rail is failed"
    },
    [PSU1_INT] = {
        .bus  = BUS0,       .addr = SWPLD1_ADDR,
        .reg  = 0x02,       .mask = 1 << 5,
        .note = "\"0\" = Interrupt doesn't occur\n\"1\" = Interrupt occurs"
    },
    [PSU2_PWR_OK] = {
        .bus  = BUS0,       .addr = SWPLD1_ADDR,
        .reg  = 0x02,       .mask = 1 << 2,
        .note = "\"0\" = Power rail is good\n\"1\" = Power rail is failed"
    },
    [PSU2_INT] = {
        .bus  = BUS0,       .addr = SWPLD1_ADDR,
        .reg  = 0x02,       .mask = 1 << 1,
        .note = "\"0\" = Interrupt doesn't occur\n\"1\" = Interrupt occurs"
    },
    [PSU1_GREEN_LED] = {
        .bus  = BUS0,       .addr = SWPLD1_ADDR,
        .reg  = 0x13,       .mask = 1 << 7,
        .note = "\"0\": Solid Green - Power Supply 1 is supplied to the switch & operating normally\n\"1\": OFF"
    },
    [PSU1_RED_LED] = {
        .bus  = BUS0,       .addr = SWPLD1_ADDR,
        .reg  = 0x13,       .mask = 1 << 6,
        .note = "\"0\": Solid Red - Power Supply 1 is supplied to the switch & operating normally\n\"1\": OFF"
    },
    [PSU2_GREEN_LED] = {
        .bus  = BUS0,       .addr = SWPLD1_ADDR,
        .reg  = 0x13,       .mask = 1 << 5,
        .note = "\"0\": Solid Green - Power Supply 2 is supplied to the switch & operating normally\n\"1\": OFF"
    },
    [PSU2_RED_LED] = {
        .bus  = BUS0,       .addr = SWPLD1_ADDR,
        .reg  = 0x13,       .mask = 1 << 4,
        .note = "\"0\": Solid Red - Power Supply 2 is supplied to the switch & operating normally\n\"1\": OFF"
    },
    [PSU_LED_MODE] = {
        .bus  = BUS0,       .addr = SWPLD1_ADDR,
        .reg  = 0x13,       .mask = 1 << 0,
        .note = "\"0\": PSU LED can be changed manually\n\"1\": PSU LED can't be changed manually"
    },
//SWPLD2
    [SWPLD2_MAJOR_VER] = {
        .bus  = BUS0,       .addr = SWPLD2_ADDR,
        .reg  = 0x00,       .mask = 0xF0,
        .note = "CPLD Major Version, controlled by CPLD editor."
    },
    [SWPLD2_MINOR_VER] = {
        .bus  = BUS0,       .addr = SWPLD2_ADDR,
        .reg  = 0x00,       .mask = 0x0F,
        .note = "CPLD Minor Version, controlled by CPLD editor."
    },
    [SWPLD2_SCRTCH_REG] = {
        .bus  = BUS0,       .addr = SWPLD2_ADDR,
        .reg  = 0x01,       .mask = 0xFF,
        .note = "CPLD read/write test register, to provide a way to test CPLD access."
    },
    [FAN_LED] = {
        .bus  = BUS0,       .addr = SWPLD2_ADDR,
        .reg  = 0x02,       .mask = 0xC0,
        .note = "\"00\"/\"11\": OFF\n\"01\": Solid Green - FANs are operating normally\n\"10\": Solid Amber - FANs are Error"
    },
    [SYS_LED] = {
        .bus  = BUS0,       .addr = SWPLD2_ADDR,
        .reg  = 0x02,       .mask = 0x30,
        .note = "\"00\": Off\n\"01\": Solid Green - Normal operation\n\"10\": Blinking Green - Booting Progress\n\"11\": Solid Red - System Fail"
    },
    [FAN_MOD1_LED] = {
        .bus  = BUS0,       .addr = SWPLD2_ADDR,
        .reg  = 0x1b,       .mask = 1 << 7,
        .note = "\"0\" = Amber\n\"1\" = Green"
    },
    [FAN_MOD2_LED] = {
        .bus  = BUS0,       .addr = SWPLD2_ADDR,
        .reg  = 0x1b,       .mask = 1 << 6,
        .note = "\"0\" = Amber\n\"1\" = Green"
    },
    [FAN_MOD3_LED] = {
        .bus  = BUS0,       .addr = SWPLD2_ADDR,
        .reg  = 0x1b,       .mask = 1 << 5,
        .note = "\"0\" = Amber\n\"1\" = Green"
    },
    [FAN_MOD4_LED] = {
        .bus  = BUS0,       .addr = SWPLD2_ADDR,
        .reg  = 0x1b,       .mask = 1 << 4,
        .note = "\"0\" = Amber\n\"1\" = Green"
    },
//SWPLD3
    [SWPLD3_MAJOR_VER] = {
        .bus  = BUS0,       .addr = SWPLD3_ADDR,
        .reg  = 0x00,       .mask = 0xF0,
        .note = "CPLD Major Version, controlled by CPLD editor."
    },
    [SWPLD3_MINOR_VER] = {
        .bus  = BUS0,       .addr = SWPLD3_ADDR,
        .reg  = 0x00,       .mask = 0x0F,
        .note = "CPLD Minor Version, controlled by CPLD editor."
    },
    [SWPLD3_SCRTCH_REG] = {
        .bus  = BUS0,       .addr = SWPLD3_ADDR,
        .reg  = 0x01,       .mask = 0xFF,
        .note = "CPLD read/write test register, to provide a way to test CPLD access."
    },
    [SB_VER] = {
        .bus  = BUS0,       .addr = SWPLD3_ADDR,
        .reg  = 0x02,       .mask = 0xF0,
        .note = "\"0x0\": proto-A\n\"0x1\": proto-B"
    },
    [PLATFORM_TYPE] = {
        .bus  = BUS0,       .addr = SWPLD3_ADDR,
        .reg  = 0x02,       .mask = 0x0F,
        .note = "\"0x0\": 64X100G_2U\n\"0x1\"~\"0xF\" Reserved"
    },
//SWPLD4
    [SW_BOARD_ID1] = {
        .bus  = BUS0,       .addr = SWPLD4_ADDR,
        .reg  = 0x00,       .mask = 0xFF,
        .note = "0x00"
    },
    [SW_BOARD_ID2] = {
        .bus  = BUS0,       .addr = SWPLD4_ADDR,
        .reg  = 0x01,       .mask = 0xFF,
        .note = "Configured by PLD Editor\n0x03: AG9064"
    },
    [SWBD_VER] = {
        .bus  = BUS0,       .addr = SWPLD4_ADDR,
        .reg  = 0x02,       .mask = 0xFF,
        .note = "Configured by external resistor\n0x01: EVT1\n0x02: EVT2\n0x03: EVT3\n0x04: EVT4\n0x10: DVT\n0x20: PVT"
    },
    [SWPLD4_VER] = {
        .bus  = BUS0,       .addr = SWPLD4_ADDR,
        .reg  = 0x03,       .mask = 0xFF,
        .note = ""
    },
    [PSU_FAN_EVENT] = {
        .bus  = BUS0,       .addr = SWPLD4_ADDR,
        .reg  = 0x04,       .mask = 1 << 4,
        .note = "\"0\" = Interrupt occurs\n\"1\" = Interrupt doesn't occur"
    },
    [CPU_THERMAL_INT] = {
        .bus  = BUS0,       .addr = SWPLD4_ADDR,
        .reg  = 0x04,       .mask = 1 << 1,
        .note = "\"0\" = Interrupt occurs\n\"1\" = Interrupt doesn't occur"
    },
    [FAN_INT] = {
        .bus  = BUS0,       .addr = SWPLD4_ADDR,
        .reg  = 0x04,       .mask = 1 << 0,
        .note = "\"0\" = Interrupt occurs\n\"1\" = Interrupt doesn't occur"
    },
    [CPLD_SPI_WP] = {
        .bus  = BUS0,       .addr = SWPLD4_ADDR,
        .reg  = 0x06,       .mask = 1 << 3,
        .note = "\"0\" = SPI write operation is disabled\n\"1\" = SPI write operation is enabled"
    },
    [RJ45_CONSOLE_SEL] = {
        .bus  = BUS0,       .addr = SWPLD4_ADDR,
        .reg  = 0x06,       .mask = 1 << 2,
        .note = "\"0\" = Use BCM UART\n\"1\" = Use CPU UART"
    },
    [SYSTEM_INT] = {
        .bus  = BUS0,       .addr = SWPLD4_ADDR,
        .reg  = 0x07,       .mask = 1 << 2,
        .note = "\"0\" = Interrupt is asserted\n\"1\" = Interrupt is deasserted"
    },
    [CPLD_MB_RST_DONE] = {
        .bus  = BUS0,       .addr = SWPLD4_ADDR,
        .reg  = 0x07,       .mask = 1 << 1,
        .note = "\"0\" = Is done\n\"1\" = Is not done"
    },
    [MB_PWR_OK] = {
        .bus  = BUS0,       .addr = SWPLD4_ADDR,
        .reg  = 0x07,       .mask = 1 << 0,
        .note = "\"0\" = Power is failed\n\"1\" = Power is good"
    },
    [FAN_EEPROM_WP] = {
        .bus  = BUS0,       .addr = SWPLD4_ADDR,
        .reg  = 0x15,       .mask = 1 << 2,
        .note = "\"1\" = enables the lock-down mechanism.\n\"0\" = overrides the lock-down function enabling blocks to be erased or programmed using software commands."
    },
};