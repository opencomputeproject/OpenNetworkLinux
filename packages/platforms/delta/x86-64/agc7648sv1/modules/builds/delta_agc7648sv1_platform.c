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

#define SWPLD3_QSFP_MUX_DISABLE    (0x01 << 4)
#define SWPLD3_QSFP_SFP_CH_DISABLE  0xFF

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

struct cpld_attribute_data {
    uint8_t bus;
    uint8_t addr;
    uint8_t reg;
    uint8_t mask;
    char note[350];
};

enum cpld_type {
    system_cpld,
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
};

static struct cpld_attribute_data attribute_data[] = {
//CPLDs address and value
    [CPLD_REG_ADDR] = {
    },
    [CPLD_REG_VALUE] = {
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
};

struct i2c_device_platform_data {
    int parent;
    struct i2c_board_info info;
    struct i2c_client *client;
};

static struct cpld_platform_data agc7648sv1_cpld_platform_data[] = {
    [system_cpld] = {
        .reg_addr = CPUPLD_ADDR,
    },
};

static struct cpld_platform_data agc7648sv1_swpld3_platform_data[] = {
    [swpld3] = {
        .reg_addr = SWPLD3_ADDR,
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

/* ---------------- CPLD - start ------------- */
static struct kobject *kobj_cpld;
unsigned char cpupld_reg_addr;

/*    CPLD  -- device   */
static struct platform_device cpld_device = {
    .name = "delta-agc7648sv1-cpld",
    .id   = 0,
    .dev = {
        .platform_data  = agc7648sv1_cpld_platform_data,
        .release        = device_release
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
        case CPLD_REG_VALUE:
            ret = i2c_smbus_read_byte_data(pdata[system_cpld].client, cpupld_reg_addr);
            mutex_unlock(&dni_lock);
            return sprintf(buf, "0x%02x\n", ret);
        case CPU_BOARD_ID1 ... CPU_I2C_MUX_SEL:
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
        case CPLD_REG_VALUE:
            i2c_smbus_write_byte_data(pdata[system_cpld].client, cpupld_reg_addr, set_data);
            mutex_unlock(&dni_lock);
            return count;
         case CPU_BOARD_ID1 ... CPU_I2C_MUX_SEL:
            reg   = attribute_data[attr->index].reg;
            mask  = attribute_data[attr->index].mask;
            value = i2c_smbus_read_byte_data(pdata[DEFAULT_CPLD].client, reg);
            mask_out = value & ~(mask);
            break;
        default:
            mutex_unlock(&dni_lock);
            return sprintf((char *)buf, "%d not found", attr->index);
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
        case CPU_BOARD_ID1 ... CPU_I2C_MUX_SEL:
            i2c_smbus_write_byte_data(pdata[DEFAULT_CPLD].client, reg, set_data);
            mutex_unlock(&dni_lock);
            break;
        default:
            mutex_unlock(&dni_lock);
            return sprintf((char *)buf, "cpld not found");
    }

    return count;
}

//address and value
static SENSOR_DEVICE_ATTR(cpld_reg_addr,    S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, CPLD_REG_ADDR);
static SENSOR_DEVICE_ATTR(cpld_reg_value,   S_IRUGO | S_IWUSR, get_cpld_reg, set_cpld_reg, CPLD_REG_VALUE);

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

static struct attribute_group cpld_attr_grp = {
    .attrs = cpld_attrs,
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

static struct platform_driver cpld_driver = {
    .probe  = cpld_probe,
    .remove = __exit_p(cpld_remove),
    .driver = {
        .owner = THIS_MODULE,
        .name  = "delta-agc7648sv1-cpld",
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

static int swpld3_mux_select(struct i2c_mux_core *muxc, u32 chan)
{
    struct cpld_mux  *mux = i2c_mux_priv(muxc);
    u8 swpld3_mux_val = 0;
    u8 swpld3_qsfp_ch_en = 0;
    u8 swpld1_qsfp_modsel_val = 0;
    int ret;
    uint8_t cmd_data[4] = {0};
    uint8_t set_cmd;
    int cmd_data_len;

    if ( mux->data.base_nr == BUS2_QSFP_BASE_NUM ) {
        /* Set QSFP module respond */
        swpld1_qsfp_modsel_val = SWPLD1_QSFP_MODSEL_VAL & (~(1 << chan));
        set_cmd = CMD_SETDATA;
        cmd_data[0] = BMC_SWPLD_BUS;
        cmd_data[1] = SWPLD1_ADDR;
        cmd_data[2] = SWPLD1_QSFP_MODSEL_REG;
        cmd_data[3] = swpld1_qsfp_modsel_val;
        cmd_data_len = sizeof(cmd_data);
        ret = dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
        if (ret != 0) {
            printk("SWPLD3 mux select error! bmc_bus = %d, swpld_addr = 0x%x, reg = 0x%x, cmd_val = 0x%x\n",
                    cmd_data[0], cmd_data[1], cmd_data[2], cmd_data[3]);
            return -EIO;
        }

        /* QSFP channel enable */
        swpld3_qsfp_ch_en |= SWPLD3_QSFP_CH_EN << 4;
        set_cmd = CMD_SETDATA;
        cmd_data[0] = BMC_SWPLD_BUS;
        cmd_data[1] = SWPLD3_ADDR;
        cmd_data[2] = BUS2_SFP_MUX_REG;
        cmd_data[3] = swpld3_qsfp_ch_en;
        cmd_data_len = sizeof(cmd_data);
        ret = dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
        if (ret != 0) {
            printk("SWPLD3 mux select error! bmc_bus = %d, swpld_addr = 0x%x, reg = 0x%x, cmd_val = 0x%x\n",
                    cmd_data[0], cmd_data[1], cmd_data[2], cmd_data[3]);
            return -EIO;
        }

        /* QSFP channel selection */
        swpld3_mux_val = chan;
        set_cmd = CMD_SETDATA;
        cmd_data[0] = BMC_SWPLD_BUS;
        cmd_data[1] = SWPLD3_ADDR;
        cmd_data[2] = BUS2_QSFP_MUX_REG;
        cmd_data[3] = swpld3_mux_val;
        cmd_data_len = sizeof(cmd_data);
        ret = dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
        if (ret != 0) {
            printk("SWPLD3 mux select error! bmc_bus = %d, swpld_addr = 0x%x, reg = 0x%x, cmd_val = 0x%x\n",
                    cmd_data[0], cmd_data[1], cmd_data[2], cmd_data[3]);
            return -EIO;
        }
    }
    else if ( mux->data.base_nr == BUS2_SFP_BASE_NUM ) {
        /* Disable all QSFP modules respond */
        swpld1_qsfp_modsel_val |= SWPLD1_QSFP_MODSEL_VAL;
        set_cmd = CMD_SETDATA;
        cmd_data[0] = BMC_SWPLD_BUS;
        cmd_data[1] = SWPLD1_ADDR;
        cmd_data[2] = SWPLD1_QSFP_MODSEL_REG;
        cmd_data[3] = swpld1_qsfp_modsel_val;
        cmd_data_len = sizeof(cmd_data);
        ret = dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
        if (ret != 0) {
            printk("SWPLD3 mux select error! bmc_bus = %d, swpld_addr = 0x%x, reg = 0x%x, cmd_val = 0x%x\n",
                    cmd_data[0], cmd_data[1], cmd_data[2], cmd_data[3]);
            return -EIO;
        }

        /* SFP port 51-59, 9 ports, chan 0-8 */
        if ( chan < SWPLD3_SFP_PORT_9 ){
            swpld3_qsfp_ch_en |= SWPLD3_SFP_CH1_EN << 4;
            swpld3_mux_val = swpld3_qsfp_ch_en | (chan + 1);
        }
        /* SFP port 60-69, 10 ports, chan 9-18 */
        else if ( chan >= SWPLD3_SFP_PORT_9 && chan < SWPLD3_SFP_PORT_19 ) {
            swpld3_qsfp_ch_en |= SWPLD3_SFP_CH2_EN << 4;
            swpld3_mux_val = swpld3_qsfp_ch_en | (chan - SWPLD3_SFP_PORT_9);
        }
        /* SFP port 70-79, 10 ports, chan 19-28 */
        else if ( chan >= SWPLD3_SFP_PORT_19 && chan < SWPLD3_SFP_PORT_29 ) {
            swpld3_qsfp_ch_en |= SWPLD3_SFP_CH3_EN << 4;
            swpld3_mux_val = swpld3_qsfp_ch_en | (chan - SWPLD3_SFP_PORT_19);
        }
        /* SFP port 80-89, 10 ports, chan 29-38 */
        else if ( chan >= SWPLD3_SFP_PORT_29 && chan < SWPLD3_SFP_PORT_39 ) {
            swpld3_qsfp_ch_en |= SWPLD3_SFP_CH4_EN << 4;
            swpld3_mux_val = swpld3_qsfp_ch_en | (chan - SWPLD3_SFP_PORT_29);
        }
        /* SFP port 90-98, 9 ports, chan 39-47 */
        else if ( chan >= SWPLD3_SFP_PORT_39 && chan < SWPLD3_SFP_PORT_48 ) {
            swpld3_qsfp_ch_en |= SWPLD3_SFP_CH5_EN << 4;
            swpld3_mux_val = swpld3_qsfp_ch_en | (chan - SWPLD3_SFP_PORT_39);
        }
        else {
            swpld3_qsfp_ch_en |= SWPLD3_SFP_CH_DISABLE << 4;
            swpld3_mux_val = swpld3_qsfp_ch_en;
        }
        set_cmd = CMD_SETDATA;
        cmd_data[0] = BMC_SWPLD_BUS;
        cmd_data[1] = SWPLD3_ADDR;
        cmd_data[2] = BUS2_SFP_MUX_REG;
        cmd_data[3] = swpld3_mux_val;
        cmd_data_len = sizeof(cmd_data);
        ret = dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
        if (ret != 0) {
            printk("SWPLD3 mux select error! bmc_bus = %d, swpld_addr = 0x%x, reg = 0x%x, cmd_val = 0x%x\n",
                    cmd_data[0], cmd_data[1], cmd_data[2], cmd_data[3]);
            return -EIO;
        }
    }
    else {
        printk(KERN_ERR "SWPLD3 mux select error\n");
        return 0;
    }
}

static int swpld3_mux_deselect(struct i2c_mux_core *muxc, u32 chan)
{
    u8 swpld3_qsfp_mux_disable = 0;
    u8 swpld3_qsfp_sfp_ch_disable = 0;
    int ret;
    uint8_t cmd_data[4] = {0};
    uint8_t set_cmd;
    int cmd_data_len;

    /* Disable QSFP mux */
    swpld3_qsfp_mux_disable |= SWPLD3_QSFP_MUX_DISABLE;
    set_cmd = CMD_SETDATA;
    cmd_data[0] = BMC_SWPLD_BUS;
    cmd_data[1] = SWPLD3_ADDR;
    cmd_data[2] = BUS2_QSFP_MUX_REG;
    cmd_data[3] = swpld3_qsfp_mux_disable;
    cmd_data_len = sizeof(cmd_data);
    ret = dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
    if (ret != 0) {
        printk("SWPLD3 mux deselect error! bmc_bus = %d, swpld_addr = 0x%x, reg = 0x%x, cmd_val = 0x%x\n",
                cmd_data[0], cmd_data[1], cmd_data[2], cmd_data[3]);
        return -EIO;
    }

    /* SFP/QSFP channel disable */
    swpld3_qsfp_sfp_ch_disable |= SWPLD3_QSFP_SFP_CH_DISABLE;
    set_cmd = CMD_SETDATA;
    cmd_data[0] = BMC_SWPLD_BUS;
    cmd_data[1] = SWPLD3_ADDR;
    cmd_data[2] = BUS2_SFP_MUX_REG;
    cmd_data[3] = swpld3_qsfp_sfp_ch_disable;
    cmd_data_len = sizeof(cmd_data);
    ret = dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
    if (ret != 0) {
        printk("SWPLD3 mux deselect error! bmc_bus = %d, swpld_addr = 0x%x, reg = 0x%x, cmd_val = 0x%x\n",
                cmd_data[0], cmd_data[1], cmd_data[2], cmd_data[3]);
        return -EIO;
    }
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

    muxc = i2c_mux_alloc(parent, &pdev->dev, dev_num, 0, 0, swpld3_mux_select, swpld3_mux_deselect);
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
    struct cpld_mux_platform_data *cpld_mux_pdata;
    struct cpld_platform_data     *cpld_pdata;
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

    // register the mux prob which call the SWPLD
    ret = platform_driver_register(&cpld_mux_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register swpld mux driver\n");
        goto error_cpld_mux_driver;
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
    platform_driver_unregister(&cpld_mux_driver);
error_cpld_mux_driver:
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

    for (i = 0; i < ARRAY_SIZE(cpld_mux_device); i++) {
        platform_device_unregister(&cpld_mux_device[i]);
    }

    platform_driver_unregister(&i2c_device_driver);
    platform_driver_unregister(&swpld3_mux_driver);
    platform_driver_unregister(&cpld_mux_driver);
    platform_device_unregister(&cpld_device);
    platform_driver_unregister(&cpld_driver);    
}

module_init(delta_agc7648sv1_platform_init);
module_exit(delta_agc7648sv1_platform_exit);

MODULE_DESCRIPTION("DELTA agc7648sv1 Platform Support");
MODULE_AUTHOR("Jeff Chen <jeff.sj.chen@deltaww.com>");
MODULE_LICENSE("GPL");
