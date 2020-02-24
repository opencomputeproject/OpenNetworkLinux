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
#define BMC_BUS_4          0x03
#define CMD_SETDATA        0x03
#define CMD_GETDATA        0x02
#define BMC_ERR              -6

#define SYSTEM_CPLD_ADDR   0x31
#define CPUPLD_ADDR        0x3d
#define PORT_CPLD0_ADDR    0x32

#define PAC9528_1_ADDR     0x71
#define PAC9528_MUX_REG    0x00

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
    cpu_cpld,
    system_cpld,
    port_cpld0,
};

struct cpld_platform_data {
    int reg_addr;
    struct i2c_client *client;
};

struct cpld_platform_data agc7008s_cpu_cpld_platform_data[] = {
    [cpu_cpld] = {
        .reg_addr = CPUPLD_ADDR,
    },
};

struct cpld_platform_data agc7008s_system_cpld_platform_data[] = {
    [system_cpld] = {
        .reg_addr = SYSTEM_CPLD_ADDR,
    },
};

struct cpld_platform_data agc7008s_port_cpld0_platform_data[] = {   
    [port_cpld0] = {
        .reg_addr = PORT_CPLD0_ADDR,
    },
};

enum cpld_attributes {
    CPU_CPLD_REG_ADDR,
    CPU_CPLD_REG_VALUE,
    SYSTEM_CPLD_REG_ADDR,
    SYSTEM_CPLD_REG_VALUE,
    PORT_CPLD0_REG_ADDR,
    PORT_CPLD0_REG_VALUE,
//CPU_CPLD
    CPU_CPLD_VER,
    CPU_PCB_VER,
    THERMAL_INT,
    TPS53622_OVER_THERMAL,
    CPU_THERMAL_TRIP,
    RTC_TEST,
    RTC_RST,
    CPLD_TPM_RST,
    FORCE_RST,
    BACKUP_BIOS_WP,
    MAIN_BIOS_WP,
    BIOS_CHIP_SEL,
    BIOS_CHIP_SEL_CTRL,
    MAIN_BOARD_INT,
    TPM_INT,
    LPC_INT,
    USB_OVER_CURRENT,
    MAIN_BOARD_INT_MASK,
    TPM_INT_MASK,
    LPC_INT_MASK,
    USB_OVER_CURRENT_MASK,
//SYSTEM_CPLD
    SYSTEM_CPLD_VER,
    MB_PWR_RECYCLE,
    CPU_PWR_RECYCLE,
    MB_PWR_GOOD,
    MAC_RST,
    PHY_1G_RST,
    PHY_10G_RST,
    PHY_25G_RST,
    PHY_100G_RST,
    CPLD0_RST,
    PCA9548_0_RST,
    PCA9548_0_1_RST,
    PCA9548_0_2_RST,
    PCA9548_2_RST,
    PCA9546_3_RST,
    PCA9546_4_RST,
    GPS_UART_RST,
    TOD_UART_RST,
    GPS_MOD_RST,
    ZL30364_RST,
    CHIP_81000_RST,
    NIC_I210_RST,
    USB_HUB_RST,
    BUTTON_INT,
    WATCHDOG_INT,
    MAC_INT,
    CHIP_81000_INT,
    CHIP_54140_INT,
    GBIC_MOD_PRST,
    GBIC_MOD_INT,
    PSU1_INT,
    PSU2_INT,
    BUTTON_MASK,
    WATCHDOG_MASK,
    MAC_MASK,
    CHIP_81000_MASK,
    CHIP_54140_MASK,
    GBIC_MOD_PRST_MASK,
    GBIC_MOD_INT_MASK,
    PSU1_MASK,
    PSU2_MASK,
    PSU1_ON,
    PSU2_ON,
    PSU1_PRST,
    PSU2_PRST,
    PSU1_SMBALERT,
    PSU2_SMBALERT,
    PSU1_PWR_GOOD,
    PSU2_PWR_GOOD,
    PSU1_AC_OK,
    PSU2_AC_OK,
    UART_1588_SEL,
    BF3_1588,
    BF2_1588,
    BF1_1588,
    ISP_1014A_EN,
    ISP_FPGA_EN,
    UART_SEL,
    I2C_BUS_STATUS,
    BMC_FLASH1,
    BMC_FLASH2,
    PWR_LED_CTRL,
    FAN_LED,
    SYS_LED,
    PWR_LED,
    TOD_1PPS_RX,
    TOD_1PPS_TX,
    TOD_RX,
    TOD_TX,
    GPS_MOD_EN,
    HEATER_EN,
    MAC_CORE_FREQ,
    WATCHDOG_TIMER,
    WATCHDOG_EN,
    WATCHDOG_TIMER_CLEAR,
    MAC_VOL,
//PORT_CPLD0
    PORT_CPLD0_VER,
    SFPP_TX_FAULT,
    SFPP_TX_FAULT_MASK,
    SFPP_TX_DISABLE,
    SFPP_PRST,
    SFPP_PRST_MASK,
    SFPP_RX_LOSS,
    SFPP_RX_LOSS_MASK,
    SFP_TX_FAULT,
    SFP_TX_FAULT_MASK,
    SFP_TX_DISABLE,
    SFP_PRST,
    SFP_PRST_MASK,
    SFP_RX_LOSS,
    SFP_RX_LOSS_MASK,
};

struct cpld_attribute_data attribute_data[] = {
    [CPU_CPLD_REG_ADDR] = {
    },
    [CPU_CPLD_REG_VALUE] = {
    },
    [SYSTEM_CPLD_REG_ADDR] = {
    },
    [SYSTEM_CPLD_REG_VALUE] = {
    },
    [PORT_CPLD0_REG_ADDR] = {
    },
    [PORT_CPLD0_REG_VALUE] = {
    },
//CPU_CPLD
    [CPU_CPLD_VER] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x01,       .mask = 0x0F,
        .note = "CPLD Version, controlled by CPLD editor"
    },
    [CPU_PCB_VER] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x02,       .mask = 0x0F,
        .note = "PCB Version, controlled by CPLD editor"
    },
    [THERMAL_INT] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x0b,       .mask = 1 << 6,
        .note = "\"0\": TMP75 is over temp\n\"1\": normal"
    },
    [TPS53622_OVER_THERMAL] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x0b,       .mask = 1 << 3,
        .note = "\"0\": TPS53622 is over temp\n\"1\": normal"
    },
    [CPU_THERMAL_TRIP] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x0b,       .mask = 1 << 1,
        .note = "\"0\": CPU is over temp\n\"1\": normal"
    },
    [RTC_TEST] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x11,       .mask = 1 << 7,
        .note = "\"0\": Battery is missing/week\n\"1\": normal"
    },
    [RTC_RST] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x11,       .mask = 1 << 6,
        .note = "\"0\": Battery voltage is invaild\n\"1\": vaild"
    },
    [CPLD_TPM_RST] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x11,       .mask = 1 << 1,
        .note = "\"0\": Reset\n\"1\": normal"
    },
    [FORCE_RST] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x11,       .mask = 1 << 0,
        .note = "\"0\": Reset\n\"1\": normal"
    },
    [BACKUP_BIOS_WP] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x13,       .mask = 1 << 3,
        .note = "\"0\": Enable write protect\n\"1\": disable write protect"
    },
    [MAIN_BIOS_WP] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x13,       .mask = 1 << 2,
        .note = "\"0\": Enable write protect\n\"1\": disable write protect"
    },
    [BIOS_CHIP_SEL] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x13,       .mask = 1 << 1,
        .note = "\"0\": Main BIOS\n\"1\": backup BIOS"
    },
    [BIOS_CHIP_SEL_CTRL] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x13,       .mask = 1 << 0,
        .note = "\"0\": Chip select control by boot sequence\n\"1\": Chip select control by i2c register"
    },
    [MAIN_BOARD_INT] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x1b,       .mask = 1 << 5,
        .note = "\"0\": Interrupt\n\"1\": normal"
    },
    [TPM_INT] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x1b,       .mask = 1 << 4,
        .note = "\"0\": Interrupt\n\"1\": normal"
    },
    [LPC_INT] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x1b,       .mask = 1 << 1,
        .note = "\"0\": Interrupt\n\"1\": normal"
    },
    [USB_OVER_CURRENT] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x1b,       .mask = 1 << 0,
        .note = "\"0\": Over current\n\"1\": normal"
    },
    [MAIN_BOARD_INT_MASK] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x1c,       .mask = 1 << 5,
        .note = "\"0\": No mask\n\"1\": normal"
    },
    [TPM_INT_MASK] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x1c,       .mask = 1 << 4,
        .note = "\"0\": No mask\n\"1\": normal"
    },
    [LPC_INT_MASK] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x1c,       .mask = 1 << 1,
        .note = "\"0\": No mask\n\"1\": normal"
    },
    [USB_OVER_CURRENT_MASK] = {
        .bus  = BUS1,       .addr = CPUPLD_ADDR,
        .reg  = 0x1c,       .mask = 1 << 0,
        .note = "\"0\": No mask\n\"1\": normal"
    },
//SYSTEM_CPLD
    [SYSTEM_CPLD_VER] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x00,       .mask = 0x0F,
        .note = "CPLD Version, controlled by CPLD editor"
    },
    [MB_PWR_RECYCLE] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x01,       .mask = 1 << 3,
        .note = "\"0\": power recycle\n\"1\": normal"
    },
    [CPU_PWR_RECYCLE] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x01,       .mask = 1 << 2,
        .note = "\"0\": power recycle\n\"1\": normal"
    },
    [MB_PWR_GOOD] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x01,       .mask = 1 << 1,
        .note = "\"0\": power not ready\n\"1\": power good"
    },
    [MAC_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x02,       .mask = 1 << 4,
        .note = "\"0\": mac reset\n\"1\": Normal operation"
    },
    [PHY_1G_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x03,       .mask = 1 << 0,
        .note = "\"0\" = Normal operation\n\"1\" = 1G PHY reset"
    },
    [PHY_10G_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x03,       .mask = 1 << 1,
        .note = "\"0\" = 10G PHY reset\n\"1\" = Normal operation"
    },
    [PHY_25G_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x03,       .mask = 1 << 2,
        .note = "\"0\" = 25G PHY reset\n\"1\" = Normal operation"
    },
    [PHY_100G_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x03,       .mask = 1 << 3,
        .note = "\"0\" = 100G PHY reset\n\"1\" = Normal operation"
    },
    [CPLD0_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x04,       .mask = 1 << 0,
        .note = "\"0\" = Port CPLD 0 reset\n\"1\" = Normal operation"
    },
    [PCA9548_0_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x05,       .mask = 1 << 2,
        .note = "\"0\" = PCA9548#0 reset\n\"1\" = Normal operation"
    },
    [PCA9548_0_1_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x05,       .mask = 1 << 3,
        .note = "\"0\" = PCA9548#0_1 reset\n\"1\" = Normal operation"
    },
    [PCA9548_0_2_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x05,       .mask = 1 << 4,
        .note = "\"0\" = PCA9548#0_1 reset\n\"1\" = Normal operation"
    },
    [PCA9548_2_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x05,       .mask = 1 << 5,
        .note = "\"0\" = PCA9548#2 reset\n\"1\" = Normal operation"
    },
    [PCA9546_3_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x05,       .mask = 1 << 6,
        .note = "\"0\" = PCA9546#3 reset\n\"1\" = Normal operation"
    },
    [PCA9546_4_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x05,       .mask = 1 << 7,
        .note = "\"0\" = PCA9546#4 reset\n\"1\" = Normal operation"
    },
    [GPS_UART_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x07,       .mask = 1 << 7,
        .note = "\"0\" = Normal operation\n\"1\" = GPS UART reset"
    },
    [TOD_UART_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x07,       .mask = 1 << 6,
        .note = "\"0\" = Normal operation\n\"1\" = ToD UART reset"
    },
    [GPS_MOD_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x07,       .mask = 1 << 2,
        .note = "\"0\" = Normal operation\n\"1\" = GPS Module reset"
    },
    [ZL30364_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x07,       .mask = 1 << 1,
        .note = "\"0\" = ZL30364 reset\n\"1\" = Normal operation"
    },
    [CHIP_81000_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x07,       .mask = 1 << 0,
        .note = "\"0\" = Normal operation\n\"1\" = 81000 chip reset"
    },
    [NIC_I210_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x08,       .mask = 1 << 1,
        .note = "\"0\" = MGMT NIC I210 reset\n\"1\" = Normal operation"
    },
    [USB_HUB_RST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x08,       .mask = 1 << 0,
        .note = "\"0\" = USB HUB reset\n\"1\" = Normal operation"
    },
    [BUTTON_INT] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x10,       .mask = 1 << 1,
        .note = "\"0\": Interrupt occurs\n\"1\": Interrupt doesn't occur"
    },
    [WATCHDOG_INT] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x10,       .mask = 1 << 0,
        .note = "\"0\": Interrupt occurs\n\"1\": Interrupt doesn't occur"
    },
    [MAC_INT] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x11,       .mask = 1 << 0,
        .note = "\"0\": Interrupt occurs\n\"1\": Interrupt doesn't occur"
    },
    [CHIP_81000_INT] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x12,       .mask = 1 << 1,
        .note = "\"0\": Interrupt occurs\n\"1\": Interrupt doesn't occur"
    },
    [CHIP_54140_INT] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x12,       .mask = 1 << 0,
        .note = "\"0\": Interrupt occurs\n\"1\": Interrupt doesn't occur"
    },
    [GBIC_MOD_PRST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x13,       .mask = 1 << 1,
        .note = "\"0\": present\n\"1\": Not present"
    },
    [GBIC_MOD_INT] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x13,       .mask = 1 << 0,
        .note = "\"0\": Interrupt occurs\n\"1\": Interrupt doesn't occur"
    },
    [PSU1_INT] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x14,       .mask = 1 << 1,
        .note = "\"0\": Interrupt occurs\n\"1\": Interrupt doesn't occur"
    },
    [PSU2_INT] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x14,       .mask = 1 << 0,
        .note = "\"0\": Interrupt occurs\n\"1\": Interrupt doesn't occur"
    },
    [BUTTON_MASK] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x18,       .mask = 1 << 1,
        .note = "\"0\": No mask\n\"1\": Mask"
    },
    [WATCHDOG_MASK] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x18,       .mask = 1 << 0,
        .note = "\"0\": No mask\n\"1\": Mask"
    },
    [MAC_MASK] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x19,       .mask = 1 << 0,
        .note = "\"0\": No mask\n\"1\": Mask"
    },
    [CHIP_81000_MASK] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x1a,       .mask = 1 << 1,
        .note = "\"0\": No mask\n\"1\": Mask"
    },
    [CHIP_54140_MASK] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x1a,       .mask = 1 << 0,
        .note = "\"0\": No mask\n\"1\": Mask"
    },
    [GBIC_MOD_PRST_MASK] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x1b,       .mask = 1 << 1,
        .note = "\"0\": No mask\n\"1\": Mask"
    },
    [GBIC_MOD_INT_MASK] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x1b,       .mask = 1 << 0,
        .note = "\"0\": No mask\n\"1\": Mask"
    },
    [PSU1_MASK] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x1c,       .mask = 1 << 1,
        .note = "\"0\": No mask\n\"1\": Mask"
    },
    [PSU2_MASK] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x1c,       .mask = 1 << 0,
        .note = "\"0\": No mask\n\"1\": Mask"
    },
    [PSU1_ON] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x20,       .mask = 1 << 1,
        .note = "\"0\": Turn off\n\"1\": Turn on"
    },
    [PSU2_ON] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x20,       .mask = 1 << 0,
        .note = "\"0\": Turn off\n\"1\": Turn on"
    },
    [PSU1_PRST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x21,       .mask = 1 << 1,
        .note = "\"0\": Present\n\"1\": Not present"
    },
    [PSU2_PRST] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x21,       .mask = 1 << 0,
        .note = "\"0\": Present\n\"1\": Not present"
    },
    [PSU1_SMBALERT] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x22,       .mask = 1 << 1,
        .note = "\"0\": Critical events or warning events\n\"1\": OK"
    },
    [PSU2_SMBALERT] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x22,       .mask = 1 << 0,
        .note = "\"0\": Critical events or warning events\n\"1\": OK"
    },
    [PSU1_PWR_GOOD] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x23,       .mask = 1 << 1,
        .note = "\"0\" = Power good fail\n\"1\" = Power good OK"
    },
    [PSU2_PWR_GOOD] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x23,       .mask = 1 << 0,
        .note = "\"0\" = Power good fail\n\"1\" = Power good OK"
    },
    [PSU1_AC_OK] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x24,       .mask = 1 << 1,
        .note = "\"0\" = AC line failure or AC removed\n\"1\" = AC input voltage OK"
    },
    [PSU2_AC_OK] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x24,       .mask = 1 << 0,
        .note = "\"0\" = AC line failure or AC removed\n\"1\" = AC input voltage OK"
    },
    [UART_1588_SEL] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x25,       .mask = 1 << 7,
        .note = "\"0\" = ToD UART\n\"1\" = GPS UART"
    },
    [BF3_1588] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x25,       .mask = 1 << 6,
        .note = "\"0\" = Input 0\n\"1\" = Input 1"
    },
    [BF2_1588] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x25,       .mask = 1 << 5,
        .note = "\"0\" = Input 0\n\"1\" = Input 1"
    },
    [BF1_1588] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x25,       .mask = 1 << 4,
        .note = "\"0\" = Input 0\n\"1\" = Input 1"
    },
    [ISP_1014A_EN] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x25,       .mask = 1 << 3,
        .note = "\"0\" = Enable\n\"1\" = Disable"
    },
    [ISP_FPGA_EN] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x25,       .mask = 1 << 2,
        .note = "\"0\" = Enable\n\"1\" = Disable"
    },
    [UART_SEL] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x25,       .mask = 1 << 1,
        .note = "\"0\" = BMC UART\n\"1\" = CPU UART"
    },
    [I2C_BUS_STATUS] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x25,       .mask = 1 << 0,
        .note = "\"0\" = Free\n\"1\" = Busy"
    },
    [BMC_FLASH1] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x26,       .mask = 1 << 1,
        .note = "\"0\" = Boot flash disable\n\"1\" = Boot flash enable"
    },
    [BMC_FLASH2] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x26,       .mask = 1 << 0,
        .note = "\"0\" = Boot flash disable\n\"1\" = Boot flash enable"
    },
    [PWR_LED_CTRL] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x27,       .mask = 1 << 0,
        .note = "\"0\" = Control by CPLD\n\"1\" = Control by bit"
    },
    [FAN_LED] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x28,       .mask = 0x30,
        .note = "\"00\" = Off\n\"01\" = Solid Green.\n\"10\" = Solid Red.\n\"11\" = Blinking Red.\n"
    },
    [SYS_LED] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x28,       .mask = 0x0c,
        .note = "\"00\" = Off\n\"01\" = Solid Green.\n\"10\" = Solid Red.\n\"11\" = Blinking Green.\n"
    },
    [PWR_LED] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x28,       .mask = 0x03,
        .note = "\"00\" = Off\n\"01\" = Solid Green.\n\"10\" = Solid Red.\n"
    },
    [TOD_1PPS_RX] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x2a,       .mask = 1 << 5,
        .note = "\"0\" = Receive\n\"1\" = Normal operation"
    },
    [TOD_1PPS_TX] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x2a,       .mask = 1 << 4,
        .note = "\"0\" = Normal operation\n\"1\" = Transmit"
    },
    [TOD_RX] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x2a,       .mask = 1 << 3,
        .note = "\"0\" = Receive\n\"1\" = Normal operation"
    },
    [TOD_TX] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x2a,       .mask = 1 << 2,
        .note = "\"0\" = Normal operation\n\"1\" = Transmit"
    },
    [GPS_MOD_EN] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x2a,       .mask = 1 << 1,
        .note = "\"0\" = Enable\n\"1\" = Disable"
    },
    [HEATER_EN] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x2a,       .mask = 1 << 0,
        .note = "\"0\" = Enable\n\"1\" = Disable"
    },
    [MAC_CORE_FREQ] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x2b,       .mask = 0xF0,
        .note = "\"0000\" = 600MHz\n\"1001\" = 325MHz"
    },
    [WATCHDOG_TIMER] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x30,       .mask = 0xFF,
        .note = "Watchdog timer default value is 15 sec,the watchdog timer can be set 1 to 255 sec"
    },
    [WATCHDOG_EN] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x31,       .mask = 1 << 0,
        .note = "\"0\" = Disable\n\"1\" = Enable"
    },
    [WATCHDOG_TIMER_CLEAR] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x32,       .mask = 1 << 0,
        .note = "\"0\" = No action\n\"1\" = Clear watchdog timer to default value"
    },
    [MAC_VOL] = {
        .bus  = BUS4,       .addr = SYSTEM_CPLD_ADDR,
        .reg  = 0x31,       .mask = 0x07,
        .note = "\"000\" = Initial state\n\"001\" = 1.00V \n\"010\" = 0.95V\n\"100\" = 1.04V\n"
    },
//PORT_CPLD0
    [PORT_CPLD0_VER] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x00,       .mask = 0x0F,
        .note = "CPLD Version, controlled by CPLD editor"
    },
    [SFPP_TX_FAULT] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x08,       .mask = 0xFF,
        .note = "TX Fault of SFP+ Port 9 to Port 16"
    },
    [SFPP_TX_FAULT_MASK] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x09,       .mask = 0xFF,
        .note = "TX Fault Mask of SFP+ Port 9 to Port 16"
    },
    [SFPP_TX_DISABLE] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x0a,       .mask = 0xFF,
        .note = "TX Disable of SFP+ Port 9 to Port 16"
    },
    [SFPP_PRST] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x0b,       .mask = 0xFF,
        .note = "Present of SFP+ Port 9 to Port 16"
    },
    [SFPP_PRST_MASK] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x0c,       .mask = 0xFF,
        .note = "Present Mask of SFP+ Port 9 to Port 16"
    },
    [SFPP_RX_LOSS] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x0d,       .mask = 0xFF,
        .note = "RX Loss of SFP+ Port 9 to Port 16"
    },
    [SFPP_RX_LOSS_MASK] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x0e,       .mask = 0xFF,
        .note = "RX Loss Mask of SFP+ Port 9 to Port 16"
    },
    [SFP_TX_FAULT] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x0f,       .mask = 0x0F,
        .note = "TX Fault of SFP Port 5 to Port 8"
    },
    [SFP_TX_FAULT_MASK] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x10,       .mask = 0x0F,
        .note = "TX Fault Mask of SFP Port 5 to Port 8"
    },
    [SFP_TX_DISABLE] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x11,       .mask = 0x0F,
        .note = "TX Disable of SFP Port 5 to Port 8"
    },
    [SFP_PRST] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x12,       .mask = 0x0F,
        .note = "Present of SFP Port 5 to Port 8"
    },
    [SFP_PRST_MASK] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x13,       .mask = 0x0F,
        .note = "Present Mask of SFP Port 5 to Port 8"
    },
    [SFP_RX_LOSS] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x14,       .mask = 0x0F,
        .note = "RX Loss of SFP Port 5 to Port 8"
    },
    [SFP_RX_LOSS_MASK] = {
        .bus  = BUS4,       .addr = PORT_CPLD0_ADDR,
        .reg  = 0x15,       .mask = 0x0F,
        .note = "RX Loss Mask of SFP Port 5 to Port 8"
    },
};
