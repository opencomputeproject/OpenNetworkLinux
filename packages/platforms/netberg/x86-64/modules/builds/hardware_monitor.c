
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/log2.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/gpio.h>

#if 0
#include "hardware_monitor.h"
#else
enum platform_type {
    HURACAN = 0,
    NONE
};

#define W83795ADG_VENDOR_ID            0x5CA3
#define W83795ADG_CHIP_ID              0x79

#define W83795ADG_NUM2     2
#define W83795ADG_NUM8     8

#define W83795ADG_TEMP_COUNT     4
#define W83795ADG_FAN_COUNT     10
#define W83795ADG_FAN_SPEED_FACTOR     1350000 /* 1.35 * 10^6 */
#define W83795ADG_FAN_POLES_NUMBER     4
#define W83795ADG_VSEN_COUNT     7

#define TEMP_DECIMAL_BASE           25 /* 0.25 degree C */
#define VOL_MONITOR_UNIT            1000    /* 1000mV */

/* W83795ADG registeris */
#define W83795ADG_REG_BANK   0x00 /* Bank Select */

#define W83795ADG_REG_VENDOR_ID       0xFD /* Vender ID */
#define W83795ADG_REG_CHIP_ID            0xFE /* Chip ID */
#define W83795ADG_REG_DEVICE_ID            0xFB /* Device ID */

/* Bank 0*/
#define W83795ADG_REG_CONFIG   0x01 /* Configuration Register */
#define W83795ADG_REG_TEMP_CTRL2   0x05 /* Temperature Monitoring Control Register */
#define W83795ADG_REG_FANIN_CTRL2   0x07 /* FANIN CTRL2. FANIN Monitoring Control Register */
#define W83795ADG_REG_VSEN1   0x10 /* VSEN1 voltage readout high byte */
#define W83795ADG_REG_VSEN2   0x11 /* VSEN2 voltage readout high byte */
#define W83795ADG_REG_VSEN3   0x12 /* VSEN3 voltage readout high byte */
#define W83795ADG_REG_VSEN4   0x13 /* VSEN4 voltage readout high byte */
#define W83795ADG_REG_TR1   0x21 /* TR1 temperature Readout high byte */
#define W83795ADG_REG_TR2   0x22 /* TR2 temperature Readout high byte */

#define W83795ADG_REG_FANIN1_COUNT   0x2E /* FAN1IN tachometer readout high byte */
#define W83795ADG_REG_FANIN2_COUNT   0x2F /* FAN2IN tachometer readout high byte */
#define W83795ADG_REG_FANIN3_COUNT   0x30 /* FAN3IN tachometer readout high byte */
#define W83795ADG_REG_FANIN4_COUNT   0x31 /* FAN4IN tachometer readout high byte */
#define W83795ADG_REG_FANIN5_COUNT   0x32 /* FAN5IN tachometer readout high byte */
#define W83795ADG_REG_FANIN6_COUNT   0x33 /* FAN6IN tachometer readout high byte */
#define W83795ADG_REG_FANIN7_COUNT   0x34 /* FAN7IN tachometer readout high byte */
#define W83795ADG_REG_FANIN8_COUNT   0x35 /* FAN8IN tachometer readout high byte */
#define W83795ADG_REG_FANIN9_COUNT   0x36 /* FAN9IN tachometer readout high byte */
#define W83795ADG_REG_FANIN10_COUNT   0x37 /* FAN10IN tachometer readout high byte */

#define W83795ADG_REG_VR_LSB      0x3C  /* Monitored channel readout low byte */

/* Bank 2 */
#define W83795ADG_REG_FOMC   0x0F /* Fan Output Mode Control */
#define W83795ADG_REG_F1OV   0x10 /* Fan Output Value for FANCTL1 */
#define W83795ADG_REG_F2OV   0x11 /* Fan Output Value for FANCTL2 */

/* CPLD register */
#define CPLD_REG_GENERAL_0x00   0x00 /* Board Type and Revision Register */
#define CPLD_REG_GENERAL_0x01   0x01 /* CPLD Revision Register */
#define CPLD_REG_GENERAL_0x02   0x02 /* Power Bank Power Good Status Register */
#define CPLD_REG_GENERAL_0x03   0x03 /* Power Bank Power ABS Status Register */
#define CPLD_REG_GENERAL_0x06   0x06 /* Watchdog Control Register */

#define CPLD_REG_RESET_0x30  0x30 /* System Reset Register */
#define CPLD_REG_RESET_0x33  0x33 /* I2C Reset Register */
#define CPLD_REG_RESET_0x34  0x34 /* QSFP28 LED Clear Register */
#define CPLD_REG_RESET_0x35  0x35 /* MISC Reset Register */

#define CPLD_REG_LED_0x40  0x40 /* System LED Register */
#define CPLD_REG_LED_0x43  0x43 /* PSU LED Register */
#define CPLD_REG_LED_0x44  0x44 /* FAN LED Register */

#define CPLD_REG_LED   0x44 /* FAN LED */

#define CPLD_REG_MUX   0x4A /* I2C MUX control Register */

/* 9548 Channel Index */
#define PCA9548_CH00        0
#define PCA9548_CH01        1
#define PCA9548_CH02        2
#define PCA9548_CH03        3
#define PCA9548_CH04        4
#define PCA9548_CH05        5
#define PCA9548_CH06        6
#define PCA9548_CH07        7

/* PCA9553 */
#define PCA9553_SET_BIT(numberX, posX)          ( numberX |= ( 0x1  << posX)  )
#define PCA9553_CLEAR_BIT(numberX, posX)        ( numberX &= (~(0x1 << posX)) )
#define PCA9553_TEST_BIT(numberX, posX)         ( numberX & ( 0x1 << posX) )

/****************************************************************************************
 * Correlation between pca9553 I2C Read/Write bit Data and pca9553 port index assignment
 *
 *
 *    I2C First Data Byte                       I2C Second Data Byte
 *  ---------------------------------------   --------------------------------------
 * | 07 | 06 | 05 | 04 | 03 | 02 | 01 | 00 | |07 | 06 | 05 | 04 | 03 | 02 | 01 | 00 |
 *  ---------------------------------------  ----------------------------------------
 *  P07  P06  P05  P04  P03  P02  P01  P00    P17  P16  P15  P14  P13  P12  P11  P10
 *
 *  P[X][Y] stands for Port X (0 or 1), Bit Y (0-7)
 *
 *
 * NOTE:  We combine first data byte and second data byte into a 16-bit integer, which
 *        is used for I2C transfer. The following macro each defines the port's respective
 *        bit position within the 16-bit integer.
 *****************************************************************************************/

#define         PCA9553_BIT_P00                 0
#define         PCA9553_BIT_P01                 1
#define         PCA9553_BIT_P02                 2
#define         PCA9553_BIT_P03                 3
#define         PCA9553_BIT_P04                 4
#define         PCA9553_BIT_P05                 5
#define         PCA9553_BIT_P06                 6
#define         PCA9553_BIT_P07                 7

#define         PCA9553_BIT_P10                 0
#define         PCA9553_BIT_P11                 1
#define         PCA9553_BIT_P12                 2
#define         PCA9553_BIT_P13                 3
#define         PCA9553_BIT_P14                 4
#define         PCA9553_BIT_P15                 5
#define         PCA9553_BIT_P16                 6
#define         PCA9553_BIT_P17                 7


/******************************************************************************************
 * PCA9553 I2C bus transactions
 *
 *   - WRITE transaction, consisting of the following data sequence:
 *
 *         Address byte (bit0:0) + Command byte + Data Byte 0 + ...
 *
 *
 *
 *   - READ transaction, consissting of the following data sequence:
 *
 *         Address byte (bit0:0) + Command byte + Address byte (bit0:1) + Data Byte 0 + ...
 *              or
 *         Address byte (bit0:1) + Data Byte 0 + ...
 *
 *
 * EXPLANATION
 *      Address byte:   7-bit I2C slave address + 1-bit (Read|Write)
 *
 *      Command byte:   A pointer allowing the master device to select which PCA9535
 *                      register to interact with.
 *
 ******************************************************************************************/

/* Register-pointing command byte */
#define         PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0           0x00
#define         PCA9553_COMMAND_BYTE_REG_INPUT_PORT_1           0x01
#define         PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0          0x02
#define         PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_1          0x03
#define         PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0   0x04
#define         PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_1   0x05
#define         PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0        0x06
#define         PCA9553_COMMAND_BYTE_REG_CONFIGURATION_1        0x07

/* Model ID Definition */
typedef enum
{
    HURACAN_WITH_BMC = 0x0,
    HURACAN_WITHOUT_BMC,
    CABRERAIII_WITH_BMC,
    CABRERAIII_WITHOUT_BMC,
    SESTO_WITH_BMC,
    SESTO_WITHOUT_BMC,
    NCIIX_WITH_BMC,
    NCIIX_WITHOUT_BMC,
    ASTERION_WITH_BMC,
    ASTERION_WITHOUT_BMC,
    HURACAN_A_WITH_BMC,
    HURACAN_A_WITHOUT_BMC,

    MODEL_ID_LAST
} modelId_t;

/* QSFP */
#define QSFP_COUNT       64
#define QSFP_DATA_SIZE   256
#define SFP_COPPER_DATA_SIZE    512

#define EEPROM_DATA_SIZE   256

typedef struct
{
    unsigned char tempLow2HighThreshold[3];
    unsigned char tempHigh2LowThreshold[3];
    unsigned char fanDutySet[3];
} fanControlTable_t;

static int w83795adg_hardware_monitor_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int w83795adg_hardware_monitor_detect(struct i2c_client *client, struct i2c_board_info *info);
static int w83795adg_hardware_monitor_remove(struct i2c_client *client);
static void w83795adg_hardware_monitor_shutdown(struct i2c_client *client);

typedef struct
{
    unsigned char  portMaskBitForPCA9548_1;
    unsigned char  portMaskBitForPCA9548_2TO5;
    unsigned char  portMaskIOsForPCA9548_0;
    unsigned char  i2cAddrForPCA9535;
    short portMaskBitForTxEnPin;
} SFP_PORT_DATA_t;

#define SFP_PORT_DATA_PORT_NCIIX_1 {0x04, 0x01, 0x08, 0, 3}
#define SFP_PORT_DATA_PORT_NCIIX_2 {0x04, 0x02, 0x08, 0, 9}
#define SFP_PORT_DATA_PORT_NCIIX_3 {0x04, 0x04, 0x08, 1, 3}
#define SFP_PORT_DATA_PORT_NCIIX_4 {0x04, 0x08, 0x08, 1, 9}
#define SFP_PORT_DATA_PORT_NCIIX_5 {0x04, 0x10, 0x08, 2, 3}
#define SFP_PORT_DATA_PORT_NCIIX_6 {0x04, 0x20, 0x08, 2, 9}
#define SFP_PORT_DATA_PORT_NCIIX_7 {0x04, 0x40, 0x08, 3, 3}
#define SFP_PORT_DATA_PORT_NCIIX_8 {0x04, 0x80, 0x08, 3, 9}

#define SFP_PORT_DATA_PORT_NCIIX_9   {0x08, 0x01, 0x10, 0, 3}
#define SFP_PORT_DATA_PORT_NCIIX_10 {0x08, 0x02, 0x10, 0, 9}
#define SFP_PORT_DATA_PORT_NCIIX_11 {0x08, 0x04, 0x10, 1, 3}
#define SFP_PORT_DATA_PORT_NCIIX_12 {0x08, 0x08, 0x10, 1, 9}
#define SFP_PORT_DATA_PORT_NCIIX_13 {0x08, 0x10, 0x10, 2, 3}
#define SFP_PORT_DATA_PORT_NCIIX_14 {0x08, 0x20, 0x10, 2, 9}
#define SFP_PORT_DATA_PORT_NCIIX_15 {0x08, 0x40, 0x10, 3, 3}
#define SFP_PORT_DATA_PORT_NCIIX_16 {0x08, 0x80, 0x10, 3, 9}

#define SFP_PORT_DATA_PORT_NCIIX_17 {0x10, 0x01, 0x20, 0, 3}
#define SFP_PORT_DATA_PORT_NCIIX_18 {0x10, 0x02, 0x20, 0, 9}
#define SFP_PORT_DATA_PORT_NCIIX_19 {0x10, 0x04, 0x20, 1, 3}
#define SFP_PORT_DATA_PORT_NCIIX_20 {0x10, 0x08, 0x20, 1, 9}
#define SFP_PORT_DATA_PORT_NCIIX_21 {0x10, 0x10, 0x20, 2, 3}
#define SFP_PORT_DATA_PORT_NCIIX_22 {0x10, 0x20, 0x20, 2, 9}
#define SFP_PORT_DATA_PORT_NCIIX_23 {0x10, 0x40, 0x20, 3, 3}
#define SFP_PORT_DATA_PORT_NCIIX_24 {0x10, 0x80, 0x20, 3, 9}

#define SFP_PORT_DATA_PORT_NCIIX_25 {0x20, 0x01, 0x40, 0, 3}
#define SFP_PORT_DATA_PORT_NCIIX_26 {0x20, 0x02, 0x40, 0, 9}
#define SFP_PORT_DATA_PORT_NCIIX_27 {0x20, 0x04, 0x40, 1, 3}
#define SFP_PORT_DATA_PORT_NCIIX_28 {0x20, 0x08, 0x40, 1, 9}
#define SFP_PORT_DATA_PORT_NCIIX_29 {0x20, 0x10, 0x40, 2, 3}
#define SFP_PORT_DATA_PORT_NCIIX_30 {0x20, 0x20, 0x40, 2, 9}
#define SFP_PORT_DATA_PORT_NCIIX_31 {0x20, 0x40, 0x40, 3, 3}
#define SFP_PORT_DATA_PORT_NCIIX_32 {0x20, 0x80, 0x40, 3, 9}

#define SFP_PORT_DATA_PORT_NCIIX_33 {0x40, 0x01, 0x80, 0, 3}
#define SFP_PORT_DATA_PORT_NCIIX_34 {0x40, 0x02, 0x80, 0, 9}
#define SFP_PORT_DATA_PORT_NCIIX_35 {0x40, 0x04, 0x80, 1, 3}
#define SFP_PORT_DATA_PORT_NCIIX_36 {0x40, 0x08, 0x80, 1, 9}
#define SFP_PORT_DATA_PORT_NCIIX_37 {0x40, 0x10, 0x80, 2, 3}
#define SFP_PORT_DATA_PORT_NCIIX_38 {0x40, 0x20, 0x80, 2, 9}
#define SFP_PORT_DATA_PORT_NCIIX_39 {0x40, 0x40, 0x80, 3, 3}
#define SFP_PORT_DATA_PORT_NCIIX_40 {0x40, 0x80, 0x80, 3, 9}

#define SFP_PORT_DATA_PORT_NCIIX_41 {0x80, 0x01, 0x01, 0, 3}
#define SFP_PORT_DATA_PORT_NCIIX_42 {0x80, 0x02, 0x01, 0, 9}
#define SFP_PORT_DATA_PORT_NCIIX_43 {0x80, 0x04, 0x01, 1, 3}
#define SFP_PORT_DATA_PORT_NCIIX_44 {0x80, 0x08, 0x01, 1, 9}
#define SFP_PORT_DATA_PORT_NCIIX_45 {0x80, 0x10, 0x01, 2, 3}
#define SFP_PORT_DATA_PORT_NCIIX_46 {0x80, 0x20, 0x01, 2, 9}
#define SFP_PORT_DATA_PORT_NCIIX_47 {0x80, 0x40, 0x01, 3, 3}
#define SFP_PORT_DATA_PORT_NCIIX_48 {0x80, 0x80, 0x01, 3, 9}

#define QSFP_PORT_DATA_PORT_NCIIX_1 {0x02, 0x02, 0x08, 0, 0x0200}
#define QSFP_PORT_DATA_PORT_NCIIX_2 {0x02, 0x01, 0x08, 0, 0x0010}
#define QSFP_PORT_DATA_PORT_NCIIX_3 {0x02, 0x08, 0x08, 1, 0x0010}
#define QSFP_PORT_DATA_PORT_NCIIX_4 {0x02, 0x04, 0x08, 0, 0x4000}
#define QSFP_PORT_DATA_PORT_NCIIX_5 {0x02, 0x20, 0x08, 1, 0x4000}
#define QSFP_PORT_DATA_PORT_NCIIX_6 {0x02, 0x10, 0x08, 1, 0x0200}

SFP_PORT_DATA_t sfpPortData_78F[] = {
     SFP_PORT_DATA_PORT_NCIIX_1, SFP_PORT_DATA_PORT_NCIIX_2, SFP_PORT_DATA_PORT_NCIIX_3, SFP_PORT_DATA_PORT_NCIIX_4,
     SFP_PORT_DATA_PORT_NCIIX_5, SFP_PORT_DATA_PORT_NCIIX_6, SFP_PORT_DATA_PORT_NCIIX_7, SFP_PORT_DATA_PORT_NCIIX_8,
     SFP_PORT_DATA_PORT_NCIIX_9, SFP_PORT_DATA_PORT_NCIIX_10, SFP_PORT_DATA_PORT_NCIIX_11, SFP_PORT_DATA_PORT_NCIIX_12,
     SFP_PORT_DATA_PORT_NCIIX_13, SFP_PORT_DATA_PORT_NCIIX_14, SFP_PORT_DATA_PORT_NCIIX_15, SFP_PORT_DATA_PORT_NCIIX_16,
     SFP_PORT_DATA_PORT_NCIIX_17, SFP_PORT_DATA_PORT_NCIIX_18, SFP_PORT_DATA_PORT_NCIIX_19, SFP_PORT_DATA_PORT_NCIIX_20,
     SFP_PORT_DATA_PORT_NCIIX_21, SFP_PORT_DATA_PORT_NCIIX_22, SFP_PORT_DATA_PORT_NCIIX_23, SFP_PORT_DATA_PORT_NCIIX_24,
     SFP_PORT_DATA_PORT_NCIIX_25, SFP_PORT_DATA_PORT_NCIIX_26, SFP_PORT_DATA_PORT_NCIIX_27, SFP_PORT_DATA_PORT_NCIIX_28,
     SFP_PORT_DATA_PORT_NCIIX_29, SFP_PORT_DATA_PORT_NCIIX_30, SFP_PORT_DATA_PORT_NCIIX_31, SFP_PORT_DATA_PORT_NCIIX_32,
     SFP_PORT_DATA_PORT_NCIIX_33, SFP_PORT_DATA_PORT_NCIIX_34, SFP_PORT_DATA_PORT_NCIIX_35, SFP_PORT_DATA_PORT_NCIIX_36,
     SFP_PORT_DATA_PORT_NCIIX_37, SFP_PORT_DATA_PORT_NCIIX_38, SFP_PORT_DATA_PORT_NCIIX_39, SFP_PORT_DATA_PORT_NCIIX_40,
     SFP_PORT_DATA_PORT_NCIIX_41, SFP_PORT_DATA_PORT_NCIIX_42, SFP_PORT_DATA_PORT_NCIIX_43, SFP_PORT_DATA_PORT_NCIIX_44,
     SFP_PORT_DATA_PORT_NCIIX_45, SFP_PORT_DATA_PORT_NCIIX_46, SFP_PORT_DATA_PORT_NCIIX_47, SFP_PORT_DATA_PORT_NCIIX_48,
     QSFP_PORT_DATA_PORT_NCIIX_1, QSFP_PORT_DATA_PORT_NCIIX_2, QSFP_PORT_DATA_PORT_NCIIX_3,
     QSFP_PORT_DATA_PORT_NCIIX_4, QSFP_PORT_DATA_PORT_NCIIX_5, QSFP_PORT_DATA_PORT_NCIIX_6
};

/* CHL8325A for NC2X Platform */
#define LOOP1_VID_OVERRIDE_ENABLE_REG         0xD0
#define LOOP1_OVERRIDE_VID_SETTING_REG        0xD1

#define CHL8325_LOOP1_Enable 0x40

#define CHL8325_VID0 0x9C
#define CHL8325_VID1 0x8D
#define CHL8325_VID_DEFAULT (CHL8325_VID0)
#endif

static struct i2c_client qsfpDataA0_client;
static struct i2c_client qsfpDataA2_client;
static struct i2c_client SfpCopperData_client;

/* i2c bus 0 */
static struct i2c_client pca9535pwr_client_bus0;
static struct i2c_client cpld_client;
static struct i2c_client pca9548_client_bus0;
static struct i2c_client pca9535_client_bus0[4];
static struct i2c_client eeprom_client_bus0;
static struct i2c_client mp2953agu_client;
static struct i2c_client chl8325a_client;
static struct i2c_client psu_eeprom_client_bus0;
static struct i2c_client psu_mcu_client_bus0;

/* i2c bus 1 */
static struct i2c_client pca9548_client[4];
static struct i2c_client pca9535pwr_client[6];

static struct i2c_client eeprom_client;
static struct i2c_client psu_eeprom_client;
static struct i2c_client psu_mcu_client;

static unsigned int FanErr[W83795ADG_FAN_COUNT] = {0};
static unsigned int FanDir = 0;
static unsigned int FanDir2 = 0;
static unsigned int isBMCSupport = 0;

static unsigned int platformBuildRev = 0xffff;
static unsigned int platformHwRev = 0xffff;
static unsigned int platformModelId = 0xffff;

static char platformPsuPG = 0;
static char platformPsuABS = 0;

unsigned int SFPPortAbsStatus[QSFP_COUNT];
unsigned int SFPPortRxLosStatus[QSFP_COUNT];
unsigned int SFPPortTxFaultStatus[QSFP_COUNT];
char SFPPortDataValid[QSFP_COUNT];
char SFPPortTxDisable[QSFP_COUNT];

static struct i2c_client cpld_client_bus1;

struct i2c_bus0_hardware_monitor_data {
    struct device *hwmon_dev;
    struct attribute_group hwmon_group;
    struct mutex lock;
    struct task_struct *auto_update;
    struct completion auto_update_stop;

    char hardware_monitor_data_valid;
    unsigned long hardware_monitor_last_updated; /* In jiffies */

    unsigned int venderId;
    unsigned int chipId;
    unsigned int dviceId;

    unsigned int buildRev;
    unsigned int hwRev;
    unsigned int modelId;
    unsigned int cpldRev;
    unsigned int cpldRel;

    unsigned int macTemp;

    unsigned int remoteTempIsPositive[W83795ADG_TEMP_COUNT];
    unsigned int remoteTempInt[W83795ADG_TEMP_COUNT];
    unsigned int remoteTempDecimal[W83795ADG_TEMP_COUNT];
    unsigned int fanDuty;
    unsigned int fanSpeed[W83795ADG_FAN_COUNT];
    unsigned int vSen[W83795ADG_VSEN_COUNT];
    unsigned int vSenLsb[W83795ADG_VSEN_COUNT];

    char psuPG;
    char psuABS;

    char wdReg;
    unsigned int wdEnable;
    unsigned int wdRefreshControl;
    unsigned int wdRefreshControlFlag;
    unsigned int wdRefreshTimeSelect;
    unsigned int wdRefreshTimeSelectFlag;
    unsigned int wdTimeoutSelect;
    unsigned int wdTimeoutSelectFlag;

    unsigned int rov;
 };

struct i2c_bus1_hardware_monitor_data {
    struct device *hwmon_dev;
    struct attribute_group hwmon_group;
    struct mutex lock;
    struct task_struct *auto_update;
    struct completion auto_update_stop;

    char hardware_monitor_data_valid;
    unsigned long hardware_monitor_last_updated; /* In jiffies */

    unsigned short qsfpPortAbsStatus[4];
    char qsfpPortDataA0[QSFP_COUNT][QSFP_DATA_SIZE];
    char qsfpPortDataA2[QSFP_COUNT][QSFP_DATA_SIZE];
    char SfpCopperPortData[QSFP_COUNT][SFP_COPPER_DATA_SIZE];
    unsigned short qsfpPortDataValid[4];
    unsigned short sfpPortTxDisable[3];
    unsigned short sfpPortRateSelect[3];
    unsigned short sfpPortRxLosStatus[4];
    unsigned short sfpPortTxFaultStatus[4];

    unsigned short fanAbs[2];
    unsigned short fanDir[2];

    unsigned short systemLedStatus;
    unsigned short frontLedStatus;
    unsigned char sfpPortDataValidAst[64];
    unsigned char sfpPortAbsRxLosStatus[24];
    unsigned char qsfpPortAbsStatusAst[16];
    unsigned char sfpPortRateSelectAst[12];
    unsigned char sfpPortTxDisableAst[6];

    char qsfpPortTxDisableData[QSFP_COUNT];
    char qsfpPortTxDisableDataUpdate[QSFP_COUNT];
    struct i2c_client *sfpPortClient[QSFP_COUNT];
};

/* Addresses to scan */
static unsigned short w83795adg_normal_i2c[] = { 0x2F, 0x70, I2C_CLIENT_END };

static const struct i2c_device_id w83795adg_hardware_monitor_id[] = {
    { "HURACAN", HURACAN },
    { }
};

MODULE_DEVICE_TABLE(i2c, w83795adg_hardware_monitor_id);

static struct i2c_driver w83795adg_hardware_monitor_driver = {
    .class    = I2C_CLASS_HWMON,
    .driver = {
      .name = "w83795adg_hardware_monitor",
    },
    .probe    = w83795adg_hardware_monitor_probe,
    .remove   = w83795adg_hardware_monitor_remove,
    .shutdown   = w83795adg_hardware_monitor_shutdown,
    .id_table = w83795adg_hardware_monitor_id,
    .detect   = w83795adg_hardware_monitor_detect,
    .address_list = w83795adg_normal_i2c,
};

/* Front to Back */
static fanControlTable_t  fanControlTable[] =
{
    /* Huracan */
    {
        {77, 95, 105},  /* temperature threshold (going to up) */
        {72, 77, 95},  /* temperature threshold (going to down) */
        {0x6C, 0x9E, 0xFF} /* fan rpm : 8000, 12000, 16000 */
    },
    /* Sesto */
    {
        {85, 95, 100},  /* temperature threshold (going to up) */
        {71, 85, 95},  /* temperature threshold (going to down) */
        {0x73, 0xCC, 0xFF} /* fan rpm : 9000, 14000, 16000 */
    },
    /* NC2X */
    {
        {62, 70, 85},  /* temperature threshold (going to up) */
        {58, 66, 70},  /* temperature threshold (going to down) */
        {0x70, 0xB7, 0xFF} /* fan rpm : 8000, 13000, 16000 */
    },
    /* Asterion */
    {
        {70, 75, 80},  /* temperature threshold (going to up) */
        {60, 65, 70},  /* temperature threshold (going to down) */
        {0x8B, 0xD1, 0xFF} /* fan rpm : 12000, 18000, 22000 */
    }
};

/* Back to Front */
static fanControlTable_t  fanControlTable_B2F[] =
{
    /* Huracan */
    {
        {70, 77, 105},  /* temperature threshold (going to up) */
        {60, 70, 77},  /* temperature threshold (going to down) */
        {0x6C, 0xC7, 0xFF} /* fan rpm : 8000, 14000, 16000 */
    },
    /* Sesto */
    {
        {71, 81, 105},  /* temperature threshold (going to up) */
        {64, 81, 88},  /* temperature threshold (going to down) */
        {0x73, 0xCC, 0xFF} /* fan rpm : 9000, 14000, 16000 */
    },
    /* NC2X */
    {
        {58, 63, 80},  /* temperature threshold (going to up) */
        {54, 60, 63},  /* temperature threshold (going to down) */
        {0x6F, 0xB7, 0xFF} /* fan rpm : 8000, 13000, 16000 */
    },
    /* Asterion */
    {
        {70, 75, 80},  /* temperature threshold (going to up) */
        {60, 65, 70},  /* temperature threshold (going to down) */
        {0x8B, 0xD1, 0xFF} /* fan rpm : 12000, 18000, 22000 */
    }
};

#if 0
static int i2c_device_byte_write(const struct i2c_client *client, unsigned char command, unsigned char value)
{
    unsigned int retry = 10;
    int ret;

    while(retry>=0)
    {
        ret = i2c_smbus_write_byte_data(client, command, value);
        mdelay(10);
        if (ret >=0)
            break;
        retry--;
    }

    if (ret < 0)
        printk(KERN_INFO "%s fail : slave addr 0x%02x, command = 0x%02x, value = 0x%02x\n", __func__, client->addr, command, value);

    return ret;
}
#endif

#define BIT_INDEX(i)                (1ULL << (i))
#define SFF8436_RX_LOS_ADDR         3
#define SFF8436_TX_FAULT_ADDR       4
#define SFF8436_TX_DISABLE_ADDR     86

#define I2C_RW_RETRY_COUNT          3
#define I2C_RW_RETRY_INTERVAL       100 /* ms */

enum port_sysfs_attributes {
  PRESENT,
  RX_LOS,
  RX_LOS1,
  RX_LOS2,
  RX_LOS3,
  RX_LOS4,
  TX_DISABLE,
  TX_DISABLE1,
  TX_DISABLE2,
  TX_DISABLE3,
  TX_DISABLE4,
  TX_FAULT,
  TX_FAULT1,
  TX_FAULT2,
  TX_FAULT3,
  TX_FAULT4,
  EEPROM_A0_PAGE,
  EEPROM_A2_PAGE,
  SFP_COPPER,
  LAST_ATTRIBUTE
};

static struct mutex portStatusLock;

static int i2c_device_word_write(const struct i2c_client *client, unsigned char command, unsigned short value)
{
    unsigned int retry = 10;
    int ret;

    if (i2c_smbus_read_byte_data(client, command)<0)
        return -1;

    while(retry>=0)
    {
        ret = i2c_smbus_write_word_data(client, command, value);
        mdelay(10);
        if (ret >=0)
            break;
        retry--;
    }

    if (ret < 0)
        printk(KERN_INFO "%s fail : slave addr 0x%02x, command = 0x%02x, value = 0x%04x\n", __func__, client->addr, command, value);

    return ret;
}

int eepromDataBlockRead(struct i2c_client *client, char *buf)
{
    char data[32];
    int i, ret;

    for (i=0; i<8; i++)
    {
        memset(data, 0, 32);
        ret = i2c_smbus_read_i2c_block_data(client, (i*32), 32, data);
        if (ret < 0)
            return ret;
        memcpy(buf+(i*32), data, 32);
    }
    return ret;
}

int eepromDataByteRead(struct i2c_client *client, char *buf)
{
    unsigned int index;
    int value;

    for (index=0; index<EEPROM_DATA_SIZE; index++)
    {
        value = i2c_smbus_read_byte_data(client, index);
        if (value < 0)
            return value;
        buf[index] = (char)(value&0xff);
    }
    return 0;
}

int eepromDataByteWrite(struct i2c_client *client, u8 command, const char *data,
        int data_len)
{
    int status, retry = I2C_RW_RETRY_COUNT;

    while (retry)
    {
        status = i2c_smbus_write_byte_data(client, command, *data);
        if (unlikely(status < 0))
        {
            msleep(I2C_RW_RETRY_INTERVAL);
            retry--;
            continue;
        }
        break;
    }

    if (unlikely(status < 0))
    {
        return status;
    }

    return 1;
}

int eepromDataWordRead(struct i2c_client *client, char *buf)
{
    unsigned int index;
    int value;

    for (index=0; index<EEPROM_DATA_SIZE; index++)
    {
        value = i2c_smbus_read_word_data(client, index);
        if (value < 0)
            return value;
        buf[index*2 + 1] = (value & 0xff00) >> 8;
        buf[index*2] = value & 0x00ff;
    }
    return 0;
}

int eepromDataRead(struct i2c_client *client, char *buf)
{
    unsigned int index;
    int value;

    for (index=0; index<EEPROM_DATA_SIZE; index++)
    {
        value = i2c_smbus_read_byte(client);
        if (value < 0)
            return value;
        buf[index] = (char)(value&0xff);
    }
    return 0;
}

static int i2c_bus0_hardware_monitor_update_thread(void *p)
{
    struct i2c_client *client = p;
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    int MNTFANM, MNTFANL, TEMP;
    int MNTRTD, MNTTD;
    int i, fanErr;
    unsigned int cTemp, fanDuty, maxTemp, LastTemp = 0;
    fanControlTable_t  *fanTable;
    unsigned int fanCtrlDelay = 5;
    unsigned int fanSpeed;
    unsigned short port_status;
    int j, port;
    unsigned int configByte;

    while (!kthread_should_stop())
    {
        if (isBMCSupport == 0)
        {
            mutex_lock(&data->lock);

            /* Get Fan Speed and display status */
            fanErr = 0;
            for (i=0; i<W83795ADG_FAN_COUNT; i++)
            {
                /* Only ASTERION support 10 FAN */
                if ((i >= W83795ADG_NUM8) && (data->modelId != ASTERION_WITH_BMC) && (data->modelId != ASTERION_WITHOUT_BMC))
                {
                    FanErr[i] = 0;
                    continue;
                }

                fanSpeed = 0;
                /* Choose W83795ADG bank 0 */
                i2c_smbus_write_byte_data(client, W83795ADG_REG_BANK, 0x00);
                MNTFANM = (int) i2c_smbus_read_byte_data(client, (W83795ADG_REG_FANIN1_COUNT+i));
                MNTFANL = (int) i2c_smbus_read_byte_data(client, W83795ADG_REG_VR_LSB);
                if ( !((MNTFANM == 0xFF) && (MNTFANL == 0xF0)) )
                {
                    /* FanSpeed (RPM) = 1.35 x 10^6 / ( (12-bitCountValue) x (FanPoles/4) ) */
                    TEMP = (((MNTFANM << 4) + ((MNTFANL & 0xF0) >> 4)) * (W83795ADG_FAN_POLES_NUMBER / 4));
                    if (TEMP != 0)
                        fanSpeed = W83795ADG_FAN_SPEED_FACTOR / TEMP;
                }
                if (fanSpeed == 0)
                    fanErr = FanErr[i] = 1;
                else
                    FanErr[i] = 0;
                data->fanSpeed[i] = fanSpeed;
            }

            if ((data->modelId==HURACAN_WITH_BMC)||(data->modelId==HURACAN_WITHOUT_BMC))
            {
                if (data->hwRev == 0x00) /* Proto */
                {
                    if (fanErr == 1)
                        i2c_smbus_write_byte_data(&pca9535pwr_client_bus0, PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x80);
                    else
                        i2c_smbus_write_byte_data(&pca9535pwr_client_bus0, PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x00);
                }
                else if (data->hwRev == 0x02) /* Beta */
                {
                    if (fanErr == 1)
                        i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_LED_0x44, 0x01);
                    else
                        i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_LED_0x44, 0x00);
                }
            }

            /* Get Voltage */
            for (i=0; i<W83795ADG_VSEN_COUNT; i++)
            {
                data->vSen[i] = (unsigned int) i2c_smbus_read_byte_data(client, (W83795ADG_REG_VSEN1+i));
                data->vSenLsb[i] = (unsigned int) i2c_smbus_read_byte_data(client, W83795ADG_REG_VR_LSB);
            }

            /* Get Remote Temp */
            for (i=0; i<W83795ADG_TEMP_COUNT; i++)
            {
                /* Only ASTERION support 4 remote temperature */
                if ((i >= W83795ADG_NUM2) && (data->modelId != ASTERION_WITH_BMC) && (data->modelId != ASTERION_WITHOUT_BMC))
                    break;

                MNTRTD = (int) i2c_smbus_read_byte_data(client, (W83795ADG_REG_TR1+i));
                MNTTD = (int) i2c_smbus_read_byte_data(client, W83795ADG_REG_VR_LSB);
                /* temperature is negative */
                if ( MNTRTD & 0x80 )
                {
                    data->remoteTempIsPositive[i] = 0;
                    cTemp = (((MNTRTD << 2) + ((MNTTD & 0xC0) >> 6)) ^ 0x1FF) + 1; /* calculate 2's complement */
                    data->remoteTempDecimal[i] = (cTemp & 0x3) * TEMP_DECIMAL_BASE;
                    data->remoteTempInt[i] = cTemp >> 2;
                }
                else
                {
                    data->remoteTempIsPositive[i] = 1;
                    data->remoteTempDecimal[i] = ((MNTTD & 0xC0) >> 6) * TEMP_DECIMAL_BASE;
                    data->remoteTempInt[i] = MNTRTD;
                }
            }

            if (fanCtrlDelay == 0)
            {
                /* Get Max. Temp */
                maxTemp = data->macTemp;
                for (i=0; i<W83795ADG_TEMP_COUNT; i++)
                {
                    if ((i >= W83795ADG_NUM2) && (data->modelId != ASTERION_WITH_BMC) && (data->modelId != ASTERION_WITHOUT_BMC))
                        break;

                    if (data->remoteTempInt[i] > maxTemp)
                        maxTemp = data->remoteTempInt[i];
                }

                /* FAN Control */
                switch(platformModelId)
                {
                    default:
                    case HURACAN_WITH_BMC:
                    case HURACAN_WITHOUT_BMC:
                    case HURACAN_A_WITH_BMC:
                    case HURACAN_A_WITHOUT_BMC:
                        if (FanDir != 0)
                            fanTable = &(fanControlTable[0]);
                        else
                            fanTable = &(fanControlTable_B2F[0]);
                        break;

                    case SESTO_WITH_BMC:
                    case SESTO_WITHOUT_BMC:
                        if (FanDir != 0)
                            fanTable = &(fanControlTable[1]);
                        else
                            fanTable = &(fanControlTable_B2F[1]);
                        break;

                    case NCIIX_WITH_BMC:
                    case NCIIX_WITHOUT_BMC:
                        if (FanDir != 0)
                            fanTable = &(fanControlTable[2]);
                        else
                            fanTable = &(fanControlTable_B2F[2]);
                        break;

                    case ASTERION_WITH_BMC:
                    case ASTERION_WITHOUT_BMC:
                        if (FanDir2 != 0)
                            fanTable = &(fanControlTable[3]);
                        else
                            fanTable = &(fanControlTable_B2F[3]);
                        break;
                }

                if (fanErr)
                {
                    fanDuty = fanTable->fanDutySet[2];
                    LastTemp = 0;
                }
                else
                {
                    fanDuty = 0;
                    if (maxTemp > LastTemp) /* temp is going to up */
                    {
                        if (maxTemp < fanTable->tempLow2HighThreshold[0])
                        {
                            fanDuty = fanTable->fanDutySet[0];
                        }
                        else if (maxTemp < fanTable->tempLow2HighThreshold[1])
                        {
                            fanDuty = fanTable->fanDutySet[1];
                        }
                        else if (maxTemp < fanTable->tempLow2HighThreshold[2])
                        {
                            fanDuty = fanTable->fanDutySet[2];
                        }
                        else /* shutdown system */
                        {
                            i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x30, 0xff);
                        }
                    }
                    else if (maxTemp < LastTemp)/* temp is going to down */
                    {
                        if (maxTemp <= fanTable->tempHigh2LowThreshold[0])
                        {
                            fanDuty = fanTable->fanDutySet[0];
                        }
                        else if (maxTemp <= fanTable->tempHigh2LowThreshold[1])
                        {
                            fanDuty = fanTable->fanDutySet[1];
                        }
                        else
                        {
                            fanDuty = fanTable->fanDutySet[2];
                        }
                    }
                    LastTemp = maxTemp;
                }

                if ((fanDuty!=0)&&(data->fanDuty!=fanDuty))
                {
                    data->fanDuty = fanDuty;

                    /* Choose W83795ADG bank 0 */
                    i2c_smbus_write_byte_data(client, W83795ADG_REG_BANK, 0x00);
                    /* Disable monitoring operations */
                    configByte = i2c_smbus_read_byte_data(client, W83795ADG_REG_CONFIG);
                    configByte &= 0xfe;
                    i2c_smbus_write_byte_data(client, W83795ADG_REG_CONFIG, configByte);

                    /* Choose W83795ADG bank 2 */
                    i2c_smbus_write_byte_data(client, W83795ADG_REG_BANK, 0x02);
                    i2c_smbus_write_byte_data(client, W83795ADG_REG_F1OV, fanDuty);
                    i2c_smbus_write_byte_data(client, W83795ADG_REG_F2OV, fanDuty);

                    /* Choose W83795ADG bank 0 */
                    i2c_smbus_write_byte_data(client, W83795ADG_REG_BANK, 0x00);
                    /* Enable monitoring operations */
                    configByte |= 0x01;
                    i2c_smbus_write_byte_data(client, W83795ADG_REG_CONFIG, configByte);
                }
            }

            if (fanCtrlDelay > 0)
                fanCtrlDelay --;

            data->psuPG =  platformPsuPG = i2c_smbus_read_byte_data(&cpld_client, CPLD_REG_GENERAL_0x02);
            data->psuABS =  platformPsuABS = i2c_smbus_read_byte_data(&cpld_client, CPLD_REG_GENERAL_0x03);
            switch(platformModelId)
            {
                case NCIIX_WITH_BMC:
                case NCIIX_WITHOUT_BMC:
                    for (i=0; i<5 ; i++)
                    {
                        /* Turn on PCA9548#0 channel 3~7 on I2C-bus0 */
                        i2c_smbus_write_byte_data(&pca9548_client_bus0, 0, (1<<(PCA9548_CH03+i)));
                        for (j=0; j<4; j++)
                        {
                            port_status = i2c_smbus_read_word_data(&(pca9535_client_bus0[j]), PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0);
                            port = ((j*2)+(i*8));
                            SFPPortTxFaultStatus[port] = (PCA9553_TEST_BIT(port_status, 0)==0);
                            SFPPortAbsStatus[port] = (PCA9553_TEST_BIT(port_status, 1)==0);
                            SFPPortRxLosStatus[port] = (PCA9553_TEST_BIT(port_status, 2)==0);
                            port++;
                            SFPPortTxFaultStatus[port] = (PCA9553_TEST_BIT(port_status, 6)==0);
                            SFPPortAbsStatus[port] = (PCA9553_TEST_BIT(port_status, 7)==0);
                            SFPPortRxLosStatus[port] = (PCA9553_TEST_BIT(port_status, 8)==0);
                        }
                        i2c_smbus_write_byte_data(&pca9548_client_bus0, 0, 0x00);
                    }
                    break;

                default:
                    break;
            }

            /* Watchdog Control Register Support */
            if (data->cpldRev != 0)
            {
                if (data->wdEnable == 1) /* Watchdog Timer is enabled */
                {
                    if (data->wdRefreshControl == 0) /* Refresh Watchdog by Hardware Monitor */
                    {
                        data->wdReg = i2c_smbus_read_byte_data(&cpld_client, CPLD_REG_GENERAL_0x06);
                        data->wdReg |= 0x01; /* clear timer */
                        i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_GENERAL_0x06, data->wdReg);
                    }
                    else if (data->wdRefreshControl == 1) /* Refresh Watchdog by application */
                    {
                        if (data->wdRefreshControlFlag == 1)
                        {
                            data->wdReg = i2c_smbus_read_byte_data(&cpld_client, CPLD_REG_GENERAL_0x06);
                            data->wdReg |= 0x01; /* clear timer */
                            i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_GENERAL_0x06, data->wdReg);
                            data->wdRefreshControlFlag = 0;
                        }
                    }

                    /* Watchdog Timer timeout setting */
                    if (data->wdRefreshTimeSelectFlag == 1)
                    {
                        data->wdReg = i2c_smbus_read_byte_data(&cpld_client, CPLD_REG_GENERAL_0x06);
                        data->wdReg |= 0x01; /* clear timer */
                        data->wdReg &= (~0x38);
                        switch(data->wdRefreshTimeSelect)
                        {
                            case 1: /* 8 second delay */
                                data->wdReg |= 0x20;
                                break;

                            case 2: /* 16 second delay */
                                data->wdReg |= 0x10;
                                break;

                            case 3: /* 24 second delay */
                                data->wdReg |= 0x30;
                                break;

                            case 4: /* 32 second delay */
                                data->wdReg |= 0x08;
                                break;

                            case 5: /* 40 second delay */
                                data->wdReg |= 0x28;
                                break;

                            case 6: /* 48 second delay */
                                data->wdReg |= 0x18;
                                break;

                            case 7: /* 56 second delay */
                                data->wdReg |= 0x38;
                                break;

                            default: /* 8 second delay */
                                data->wdReg |= 0x20;
                                break;
                        }
                        i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_GENERAL_0x06, data->wdReg);
                        data->wdRefreshTimeSelectFlag = 0;
                    }

                    /* Watchdog Timeout occurrence */
                    if (data->wdTimeoutSelectFlag == 1)
                    {
                        data->wdReg = i2c_smbus_read_byte_data(&cpld_client, CPLD_REG_GENERAL_0x06);
                        data->wdReg |= 0x01; /* clear timer */
                        if (data->wdTimeoutSelect == 0) /* System reset */
                            data->wdReg &= (~0x02);
                        else /* Power cycle */
                            data->wdReg |= 0x02;
                        i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_GENERAL_0x06, data->wdReg);
                        data->wdTimeoutSelectFlag = 0;
                    }
                }
                else /* Watchdog Timer is disabled */
                {
                    data->wdReg = i2c_smbus_read_byte_data(&cpld_client, CPLD_REG_GENERAL_0x06);
                    data->wdReg |= 0x01; /* Enable WD function */
#if 0
                    data->wdReg &= (~0x02); /* default select System reset */
#else
                    data->wdReg |= 0x02; /* default select Power cycle */
                    data->wdTimeoutSelect = 1;
#endif
                    data->wdReg &= (~0x38);
                    data->wdReg |= 0x20; /* default select 8 second delay */
                    data->wdRefreshTimeSelect = 1;
                    i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_GENERAL_0x06, data->wdReg);
                    data->wdEnable = 1;
                }
            }
            mutex_unlock(&data->lock);
        }

        if (kthread_should_stop())
            break;
        msleep_interruptible(1000);
    }

    complete_all(&data->auto_update_stop);
    return 0;
}

static int i2c_bus1_hardware_monitor_update_thread(void *p)
{
    struct i2c_client *client = p;
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    int i, ret;
    unsigned short value, value2, fanErr, fanErr2;
    unsigned int step = 0;
    unsigned char qsfpPortData[QSFP_DATA_SIZE];
    unsigned char SfpCopperPortData[SFP_COPPER_DATA_SIZE];
    unsigned short port_status;
    int j, port;

    while (!kthread_should_stop())
    {
        mutex_lock(&data->lock);
        switch(platformModelId)
        {
            case HURACAN_WITH_BMC:
            case HURACAN_WITHOUT_BMC:
            case HURACAN_A_WITH_BMC:
            case HURACAN_A_WITHOUT_BMC:
                switch (step)
                {
                     case 0:
                        /* Turn on PCA9548 channel 4 on I2C-bus1 */
                        ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH04));
                        if (ret < 0)
                            break;

                        /* QSFP Port */
                        for (i=0; i<2; i++)
                            data->qsfpPortAbsStatus[i] = i2c_smbus_read_word_data(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0);

                        step = 1;
                        break;

                    case 1:
                        if ((data->qsfpPortAbsStatus[0]&0x00ff)!=0x00ff)  /* QSFP 0~7 ABS */
                        {
                            /* Turn on PCA9548 channel 0 on I2C-bus1 */
                            ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH00));
                            if (ret < 0)
                                break;

                            for (i=0; i<8; i++)
                            {
                                if (PCA9553_TEST_BIT(data->qsfpPortAbsStatus[0], i) == 0) /* present */
                                {
                                    ret = i2c_smbus_write_byte(&(pca9548_client[0]), (1<<i));
                                    if (ret>=0)
                                    {
                                        if (data->qsfpPortTxDisableDataUpdate[i] == 1)
                                        {
                                            eepromDataByteWrite(&qsfpDataA0_client, SFF8436_TX_DISABLE_ADDR, &data->qsfpPortTxDisableData[i], sizeof(char));
                                            data->qsfpPortTxDisableDataUpdate[i] = 0;
                                        }
                                        ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                        if (ret>=0)
                                        {
                                            memcpy(&(data->qsfpPortDataA0[i][0]), qsfpPortData, QSFP_DATA_SIZE);
                                            PCA9553_SET_BIT(data->qsfpPortDataValid[0], i);
                                        }
                                    }
                                    i2c_smbus_write_byte(&(pca9548_client[0]), 0x00);
                                }
                                else
                                {
                                    memset(&(data->qsfpPortDataA0[i][0]), 0, QSFP_DATA_SIZE);
                                    PCA9553_CLEAR_BIT(data->qsfpPortDataValid[0], i);
                                    data->qsfpPortTxDisableDataUpdate[i] = 1;
                                }
                            }
                        }
                        else
                        {
                            for (i=0; i<8; i++)
                            {
                                memset(&(data->qsfpPortDataA0[i][0]), 0, QSFP_DATA_SIZE);
                                PCA9553_CLEAR_BIT(data->qsfpPortDataValid[0], i);
                                data->qsfpPortTxDisableDataUpdate[i] = 1;
                            }
                        }

                        step = 2;
                        break;

                    case 2:
                        if ((data->qsfpPortAbsStatus[0]&0xff00)!=0xff00)  /* QSFP 8~15 ABS */
                        {
                            /* Turn on PCA9548 channel 1 on I2C-bus1 */
                            ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH01));
                            if (ret < 0)
                                break;

                            for (i=8; i<16; i++)
                            {
                                if (PCA9553_TEST_BIT(data->qsfpPortAbsStatus[0], i) == 0) /* present */
                                {
                                    ret = i2c_smbus_write_byte(&(pca9548_client[1]), (1<<(i-8)));
                                    if (ret>=0)
                                    {
                                        if (data->qsfpPortTxDisableDataUpdate[i] == 1)
                                        {
                                            eepromDataByteWrite(&qsfpDataA0_client, SFF8436_TX_DISABLE_ADDR, &data->qsfpPortTxDisableData[i], sizeof(char));
                                            data->qsfpPortTxDisableDataUpdate[i] = 0;
                                        }
                                        ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                        if (ret>=0)
                                        {
                                            memcpy(&(data->qsfpPortDataA0[i][0]), qsfpPortData, QSFP_DATA_SIZE);
                                            PCA9553_SET_BIT(data->qsfpPortDataValid[0], i);
                                        }
                                    }
                                    i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
                                }
                                else
                                {
                                    memset(&(data->qsfpPortDataA0[i][0]), 0, QSFP_DATA_SIZE);
                                    PCA9553_CLEAR_BIT(data->qsfpPortDataValid[0], i);
                                    data->qsfpPortTxDisableDataUpdate[i] = 1;
                                }
                            }
                        }
                        else
                        {
                            for (i=8; i<16; i++)
                            {
                                memset(&(data->qsfpPortDataA0[i][0]), 0, QSFP_DATA_SIZE);
                                PCA9553_CLEAR_BIT(data->qsfpPortDataValid[0], i);
                                data->qsfpPortTxDisableDataUpdate[i] = 1;
                            }
                        }

                        step = 3;
                        break;

                    case 3:
                        if ((data->qsfpPortAbsStatus[1]&0x00ff)!=0x00ff)  /* QSFP 16~23 ABS */
                        {
                            /* Turn on PCA9548 channel 2 on I2C-bus1 */
                            ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH02));
                            if (ret < 0)
                                break;

                            for (i=0; i<8; i++)
                            {
                                if (PCA9553_TEST_BIT(data->qsfpPortAbsStatus[1], i) == 0) /* present */
                                {
                                    ret = i2c_smbus_write_byte(&(pca9548_client[2]), (1<<i));
                                    if (ret>=0)
                                    {
                                        if (data->qsfpPortTxDisableDataUpdate[i+16] == 1)
                                        {
                                            eepromDataByteWrite(&qsfpDataA0_client, SFF8436_TX_DISABLE_ADDR, &data->qsfpPortTxDisableData[i+16], sizeof(char));
                                            data->qsfpPortTxDisableDataUpdate[i+16] = 0;
                                        }
                                        ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                        if (ret>=0)
                                        {
                                            memcpy(&(data->qsfpPortDataA0[i+16][0]), qsfpPortData, QSFP_DATA_SIZE);
                                            PCA9553_SET_BIT(data->qsfpPortDataValid[1], i);
                                        }
                                    }
                                    i2c_smbus_write_byte(&(pca9548_client[2]), 0x00);
                                }
                                else
                                {
                                    memset(&(data->qsfpPortDataA0[i+16][0]), 0, QSFP_DATA_SIZE);
                                    PCA9553_CLEAR_BIT(data->qsfpPortDataValid[1], i);
                                    data->qsfpPortTxDisableDataUpdate[i+16] = 1;
                                }
                            }
                        }
                        else
                        {
                            for (i=0; i<8; i++)
                            {
                                memset(&(data->qsfpPortDataA0[i+16][0]), 0, QSFP_DATA_SIZE);
                                PCA9553_CLEAR_BIT(data->qsfpPortDataValid[1], i);
                                data->qsfpPortTxDisableDataUpdate[i+16] = 1;
                            }
                        }

                        step = 4;
                        break;

                    case 4:
                        if ((data->qsfpPortAbsStatus[1]&0xff00)!=0xff00)  /* QSFP 24~31 ABS */
                        {
                            /* Turn on PCA9548 channel 3 on I2C-bus1 */
                            ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH03));
                            if (ret < 0)
                                break;

                            for (i=8; i<16; i++)
                            {
                                if (PCA9553_TEST_BIT(data->qsfpPortAbsStatus[1], i) == 0) /* present */
                                {
                                    ret = i2c_smbus_write_byte(&(pca9548_client[3]), (1<<(i-8)));
                                    if (ret>=0)
                                    {
                                        if (data->qsfpPortTxDisableDataUpdate[i+16] == 1)
                                        {
                                            eepromDataByteWrite(&qsfpDataA0_client, SFF8436_TX_DISABLE_ADDR, &data->qsfpPortTxDisableData[i+16], sizeof(char));
                                            data->qsfpPortTxDisableDataUpdate[i+16] = 0;
                                        }
                                        ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                        if (ret>=0)
                                        {
                                            memcpy(&(data->qsfpPortDataA0[i+16][0]), qsfpPortData, QSFP_DATA_SIZE);
                                            PCA9553_SET_BIT(data->qsfpPortDataValid[1], i);
                                        }
                                    }
                                    i2c_smbus_write_byte(&(pca9548_client[3]),  0x00);
                                }
                                else
                                {
                                    memset(&(data->qsfpPortDataA0[i+16][0]), 0, QSFP_DATA_SIZE);
                                    PCA9553_CLEAR_BIT(data->qsfpPortDataValid[1], i);
                                    data->qsfpPortTxDisableDataUpdate[i+16] = 1;
                                }
                            }
                        }
                        else
                        {
                            for (i=8; i<16; i++)
                            {
                                memset(&(data->qsfpPortDataA0[i+16][0]), 0, QSFP_DATA_SIZE);
                                PCA9553_CLEAR_BIT(data->qsfpPortDataValid[1], i);
                                data->qsfpPortTxDisableDataUpdate[i+16] = 1;
                            }
                        }

                        if (isBMCSupport == 0)
                            step = 5;
                        else
                            step = 0;
                        break;

                    case 5:
                        /* Turn on PCA9548 channel 7 on I2C-bus1 */
                        ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH07));
                        if (ret < 0)
                            break;

                        value = 0xcccc;
                        fanErr = 0;
                        for (i=0; i<W83795ADG_NUM8; i++)
                        {
                            if (FanErr[i] == 1)
                                fanErr |=  (0x1<<i);
                        }

                        if (fanErr&0x03)
                            value |= 0x0002;
                        else
                            value |= 0x0001;

                        if (fanErr&0x0c)
                            value |= 0x0020;
                        else
                            value |= 0x0010;

                        if (fanErr&0x30)
                            value |= 0x0200;
                        else
                            value |= 0x0100;

                        if (fanErr&0xc0)
                            value |= 0x2000;
                        else
                            value |= 0x1000;

                        ret = i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, value);
                        if (ret < 0)
                            break;

                        if ( (platformHwRev == 0x03) || /* PVT */
                                 (platformModelId == HURACAN_A_WITH_BMC)||(platformModelId == HURACAN_A_WITHOUT_BMC) )
                        {
                            data->frontLedStatus |= 0x00ff;
                            if (fanErr==0)
                                data->frontLedStatus &= (~0x0008); /* FAN_LED_G# */
                            else
                                data->frontLedStatus &= (~0x0004); /* FAN_LED_Y# */

                            if ((platformPsuABS&0x01)==0x00) /* PSU1 Present */
                            {
                                if (platformPsuPG&0x08) /* PSU1_PG_LDC Power Goodasserted */
                                    data->frontLedStatus &= (~0x0002); /* PSU1_LED_G# */
                                else
                                    data->frontLedStatus &= (~0x0001); /* PSU1_LED_Y# */
                            }
                            if ((platformPsuABS&0x02)==0x00) /* PSU2 Present */
                            {
                                if (platformPsuPG&0x10) /* PSU2_PG_LDC Power Goodasserted */
                                    data->frontLedStatus &= (~0x0020); /* PSU2_LED_G# */
                                else
                                    data->frontLedStatus &= (~0x0010); /* PSU2_LED_Y# */
                            }

                            switch (data->systemLedStatus)
                            {
                                default:
                                case 0: /* Booting */
                                    break;

                                case 1: /* Critical*/
                                    data->frontLedStatus &= (~0x0040); /* SYS_LED_Y# */
                                    break;

                                case 2: /* Normal */
                                    data->frontLedStatus &= (~0x0080); /* SYS_LED_G# */
                                    break;
                            }

                            i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->frontLedStatus);
                        }

                        /* FAN Status */
                        value =  i2c_smbus_read_word_data(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0);
                        data->fanAbs[0] = (value&0x4444);
                        data->fanDir[0] = (value&0x8888);
                        FanDir = data->fanDir[0];

                        step = 0;
                        break;

                    default:
                        step = 0;
                        break;
                }
                i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x33, 0x00);
                i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x33, 0xff);
                break;

            case CABRERAIII_WITH_BMC:
            case CABRERAIII_WITHOUT_BMC:
                break;

            case SESTO_WITH_BMC:
            case SESTO_WITHOUT_BMC:
                switch (step)
                {
                     case 0:
                        /* Turn on PCA9548#1 channel 0 on I2C-bus1 */
                        ret = i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH00));
                        if (ret < 0)
                            break;

                        /* SFP Port */
                        for (i=0; i<4; i++)
                            data->qsfpPortAbsStatus[i] = i2c_smbus_read_word_data(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0);

                        /* Turn on PCA9548#1 channel 1 on I2C-bus1 */
                        ret = i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH01));
                        if (ret < 0)
                            break;

                        /* SFP Port - RXLOS */
                        for (i=0; i<3; i++)
                            data->sfpPortRxLosStatus[i] = i2c_smbus_read_word_data(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0);

                        /* Turn on PCA9548#1 channel 2 on I2C-bus1 */
                        ret = i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH02));
                        if (ret < 0)
                            break;

                        /* SFP Port - TX_FAULT */
                        for (i=0; i<3; i++)
                            data->sfpPortTxFaultStatus[i] = i2c_smbus_read_word_data(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0);

                        step = 1;
                        break;

                    case 1:
                        if ((data->qsfpPortAbsStatus[0]&0x00ff)!=0x00ff)  /* SFP 0~7 ABS */
                        {
                            /* Turn on PCA9548#0 channel 0 on I2C-bus1 */
                            ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH00));
                            if (ret < 0)
                                break;

                            for (i=0; i<8; i++)
                            {
                                if (PCA9553_TEST_BIT(data->qsfpPortAbsStatus[0], i) == 0) /* present */
                                {
                                    ret = i2c_smbus_write_byte(&(pca9548_client[0]), (1<<i));
                                    if (ret>=0)
                                    {
                                        if (PCA9553_TEST_BIT(data->qsfpPortDataValid[0], i) == 0)
                                        {
                                            ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                            if (ret>=0)
                                            {
                                                memcpy(&(data->qsfpPortDataA0[i][0]), qsfpPortData, QSFP_DATA_SIZE);
                                                PCA9553_SET_BIT(data->qsfpPortDataValid[0], i);
                                            }
                                        }
                                        ret = eepromDataBlockRead(&qsfpDataA2_client,qsfpPortData);
                                        if (ret>=0)
                                            memcpy(&(data->qsfpPortDataA2[i][0]), qsfpPortData, QSFP_DATA_SIZE);
                                        ret = eepromDataWordRead(&SfpCopperData_client,SfpCopperPortData);
                                        if (ret>=0)
                                            memcpy(&(data->SfpCopperPortData[i][0]), SfpCopperPortData, SFP_COPPER_DATA_SIZE);
                                    }
                                    i2c_smbus_write_byte(&(pca9548_client[0]), 0x00);
                                }
                                else
                                {
                                    memset(&(data->qsfpPortDataA0[i][0]), 0, QSFP_DATA_SIZE);
                                    memset(&(data->qsfpPortDataA2[i][0]), 0, QSFP_DATA_SIZE);
                                    memset(&(data->SfpCopperPortData[i][0]), 0, SFP_COPPER_DATA_SIZE);
                                    PCA9553_CLEAR_BIT(data->qsfpPortDataValid[0], i);
                                }
                            }
                        }
                        else
                        {
                            for (i=0; i<8; i++)
                            {
                                memset(&(data->qsfpPortDataA0[i][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->qsfpPortDataA2[i][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->SfpCopperPortData[i][0]), 0, SFP_COPPER_DATA_SIZE);
                                PCA9553_CLEAR_BIT(data->qsfpPortDataValid[0], i);
                            }
                        }

                        step = 2;
                        break;

                    case 2:
                        if ((data->qsfpPortAbsStatus[0]&0xff00)!=0xff00)  /* SFP 8~15 ABS */
                        {
                            /* Turn on PCA9548#0 channel 1 on I2C-bus1 */
                            ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH01));
                            if (ret < 0)
                                break;

                            for (i=8; i<16; i++)
                            {
                                if (PCA9553_TEST_BIT(data->qsfpPortAbsStatus[0], i) == 0) /* present */
                                {
                                    ret = i2c_smbus_write_byte(&(pca9548_client[0]), (1<<(i-8)));
                                    if (ret>=0)
                                    {
                                        if (PCA9553_TEST_BIT(data->qsfpPortDataValid[0], i) == 0)
                                        {
                                            ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                            if (ret>=0)
                                            {
                                                memcpy(&(data->qsfpPortDataA0[i][0]), qsfpPortData, QSFP_DATA_SIZE);
                                                PCA9553_SET_BIT(data->qsfpPortDataValid[0], i);
                                            }
                                        }
                                        ret = eepromDataBlockRead(&qsfpDataA2_client,qsfpPortData);
                                        if (ret>=0)
                                            memcpy(&(data->qsfpPortDataA2[i][0]), qsfpPortData, QSFP_DATA_SIZE);
                                        ret = eepromDataWordRead(&SfpCopperData_client,SfpCopperPortData);
                                        if (ret>=0)
                                            memcpy(&(data->SfpCopperPortData[i][0]), SfpCopperPortData, SFP_COPPER_DATA_SIZE);
                                    }
                                    i2c_smbus_write_byte(&(pca9548_client[0]), 0x00);
                                }
                                else
                                {
                                    memset(&(data->qsfpPortDataA0[i][0]), 0, QSFP_DATA_SIZE);
                                    memset(&(data->qsfpPortDataA2[i][0]), 0, QSFP_DATA_SIZE);
                                    memset(&(data->SfpCopperPortData[i][0]), 0, SFP_COPPER_DATA_SIZE);
                                    PCA9553_CLEAR_BIT(data->qsfpPortDataValid[0], i);
                                }
                            }
                        }
                        else
                        {
                            for (i=8; i<16; i++)
                            {
                                memset(&(data->qsfpPortDataA0[i][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->qsfpPortDataA2[i][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->SfpCopperPortData[i][0]), 0, SFP_COPPER_DATA_SIZE);
                                PCA9553_CLEAR_BIT(data->qsfpPortDataValid[0], i);
                            }
                        }

                        step = 3;
                        break;

                    case 3:
                        if ((data->qsfpPortAbsStatus[1]&0x00ff)!=0x00ff)  /* SFP 16~23 ABS */
                        {
                            /* Turn on PCA9548#0 channel 2 on I2C-bus1 */
                            ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH02));
                            if (ret < 0)
                                break;

                            for (i=0; i<8; i++)
                            {
                                if (PCA9553_TEST_BIT(data->qsfpPortAbsStatus[1], i) == 0) /* present */
                                {
                                    ret = i2c_smbus_write_byte(&(pca9548_client[0]), (1<<i));
                                    if (ret>=0)
                                    {
                                        if (PCA9553_TEST_BIT(data->qsfpPortDataValid[1], i) == 0)
                                        {
                                            ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                            if (ret>=0)
                                            {
                                                memcpy(&(data->qsfpPortDataA0[i+16][0]), qsfpPortData, QSFP_DATA_SIZE);
                                                PCA9553_SET_BIT(data->qsfpPortDataValid[1], i);
                                            }
                                        }
                                        ret = eepromDataBlockRead(&qsfpDataA2_client,qsfpPortData);
                                        if (ret>=0)
                                            memcpy(&(data->qsfpPortDataA2[i+16][0]), qsfpPortData, QSFP_DATA_SIZE);
                                        ret = eepromDataWordRead(&SfpCopperData_client,SfpCopperPortData);
                                        if (ret>=0)
                                            memcpy(&(data->SfpCopperPortData[i+16][0]), SfpCopperPortData, SFP_COPPER_DATA_SIZE);
                                    }
                                    i2c_smbus_write_byte(&(pca9548_client[0]), 0x00);
                                }
                                else
                                {
                                    memset(&(data->qsfpPortDataA0[i+16][0]), 0, QSFP_DATA_SIZE);
                                    memset(&(data->qsfpPortDataA2[i+16][0]), 0, QSFP_DATA_SIZE);
                                    memset(&(data->SfpCopperPortData[i+16][0]), 0, SFP_COPPER_DATA_SIZE);
                                    PCA9553_CLEAR_BIT(data->qsfpPortDataValid[1], i);
                                }
                            }
                        }
                        else
                        {
                            for (i=0; i<8; i++)
                            {
                                memset(&(data->qsfpPortDataA0[i+16][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->qsfpPortDataA2[i+16][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->SfpCopperPortData[i+16][0]), 0, SFP_COPPER_DATA_SIZE);
                                PCA9553_CLEAR_BIT(data->qsfpPortDataValid[1], i);
                            }
                        }

                        step = 4;
                        break;

                    case 4:
                        if ((data->qsfpPortAbsStatus[1]&0xff00)!=0xff00)  /* SFP 24~31 ABS */
                        {
                            /* Turn on PCA9548#0 channel 3 on I2C-bus1 */
                            ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH03));
                            if (ret < 0)
                                break;

                            for (i=8; i<16; i++)
                            {
                                if (PCA9553_TEST_BIT(data->qsfpPortAbsStatus[1], i) == 0) /* present */
                                {
                                    ret = i2c_smbus_write_byte(&(pca9548_client[0]), (1<<(i-8)));
                                    if (ret>=0)
                                    {
                                        if (PCA9553_TEST_BIT(data->qsfpPortDataValid[1], i) == 0)
                                        {
                                            ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                            if (ret>=0)
                                            {
                                                memcpy(&(data->qsfpPortDataA0[i+16][0]), qsfpPortData, QSFP_DATA_SIZE);
                                                PCA9553_SET_BIT(data->qsfpPortDataValid[1], i);
                                            }
                                        }
                                        ret = eepromDataBlockRead(&qsfpDataA2_client,qsfpPortData);
                                        if (ret>=0)
                                            memcpy(&(data->qsfpPortDataA2[i+16][0]), qsfpPortData, QSFP_DATA_SIZE);
                                        ret = eepromDataWordRead(&SfpCopperData_client,SfpCopperPortData);
                                        if (ret>=0)
                                            memcpy(&(data->SfpCopperPortData[i+16][0]), SfpCopperPortData, SFP_COPPER_DATA_SIZE);
                                    }
                                    i2c_smbus_write_byte(&(pca9548_client[0]),  0x00);
                                }
                                else
                                {
                                    memset(&(data->qsfpPortDataA0[i+16][0]), 0, QSFP_DATA_SIZE);
                                    memset(&(data->qsfpPortDataA2[i+16][0]), 0, QSFP_DATA_SIZE);
                                    memset(&(data->SfpCopperPortData[i+16][0]), 0, SFP_COPPER_DATA_SIZE);
                                    PCA9553_CLEAR_BIT(data->qsfpPortDataValid[1], i);
                                }
                            }
                        }
                        else
                        {
                            for (i=8; i<16; i++)
                            {
                                memset(&(data->qsfpPortDataA0[i+16][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->qsfpPortDataA2[i+16][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->SfpCopperPortData[i+16][0]), 0, SFP_COPPER_DATA_SIZE);
                                PCA9553_CLEAR_BIT(data->qsfpPortDataValid[1], i);
                            }
                        }

                        step = 5;
                        break;

                    case 5:
                        if ((data->qsfpPortAbsStatus[2]&0x00ff)!=0x00ff)  /* SFP 32~39 ABS */
                        {
                            /* Turn on PCA9548#0 channel 4 on I2C-bus1 */
                            ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH04));
                            if (ret < 0)
                                break;

                            for (i=0; i<8; i++)
                            {
                                if (PCA9553_TEST_BIT(data->qsfpPortAbsStatus[2], i) == 0) /* present */
                                {
                                    ret = i2c_smbus_write_byte(&(pca9548_client[0]), (1<<i));
                                    if (ret>=0)
                                    {
                                        if (PCA9553_TEST_BIT(data->qsfpPortDataValid[2], i) == 0)
                                        {
                                            ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                            if (ret>=0)
                                            {
                                                memcpy(&(data->qsfpPortDataA0[i+32][0]), qsfpPortData, QSFP_DATA_SIZE);
                                                PCA9553_SET_BIT(data->qsfpPortDataValid[2], i);
                                            }
                                        }
                                        ret = eepromDataBlockRead(&qsfpDataA2_client,qsfpPortData);
                                        if (ret>=0)
                                            memcpy(&(data->qsfpPortDataA2[i+32][0]), qsfpPortData, QSFP_DATA_SIZE);
                                        ret = eepromDataWordRead(&SfpCopperData_client,SfpCopperPortData);
                                        if (ret>=0)
                                            memcpy(&(data->SfpCopperPortData[i+32][0]), SfpCopperPortData, SFP_COPPER_DATA_SIZE);
                                    }
                                    i2c_smbus_write_byte(&(pca9548_client[0]), 0x00);
                                }
                                else
                                {
                                    memset(&(data->qsfpPortDataA0[i+32][0]), 0, QSFP_DATA_SIZE);
                                    memset(&(data->qsfpPortDataA2[i+32][0]), 0, QSFP_DATA_SIZE);
                                    memset(&(data->SfpCopperPortData[i+32][0]), 0, SFP_COPPER_DATA_SIZE);
                                    PCA9553_CLEAR_BIT(data->qsfpPortDataValid[2], i);
                                }
                            }
                        }
                        else
                        {
                            for (i=0; i<8; i++)
                            {
                                memset(&(data->qsfpPortDataA0[i+32][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->qsfpPortDataA2[i+32][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->SfpCopperPortData[i+32][0]), 0, SFP_COPPER_DATA_SIZE);
                                PCA9553_CLEAR_BIT(data->qsfpPortDataValid[2], i);
                            }
                        }

                        step = 6;
                        break;

                    case 6:
                        if ((data->qsfpPortAbsStatus[2]&0xff00)!=0xff00)  /* SFP 40~47 ABS */
                        {
                            /* Turn on PCA9548#0 channel 5 on I2C-bus1 */
                            ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH05));
                            if (ret < 0)
                                break;

                            for (i=8; i<16; i++)
                            {
                                if (PCA9553_TEST_BIT(data->qsfpPortAbsStatus[2], i) == 0) /* present */
                                {
                                    ret = i2c_smbus_write_byte(&(pca9548_client[0]), (1<<(i-8)));
                                    if (ret>=0)
                                    {
                                        if (PCA9553_TEST_BIT(data->qsfpPortDataValid[2], i) == 0)
                                        {
                                            ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                            if (ret>=0)
                                            {
                                                memcpy(&(data->qsfpPortDataA0[i+32][0]), qsfpPortData, QSFP_DATA_SIZE);
                                                PCA9553_SET_BIT(data->qsfpPortDataValid[2], i);
                                            }
                                        }
                                        ret = eepromDataBlockRead(&qsfpDataA2_client,qsfpPortData);
                                        if (ret>=0)
                                            memcpy(&(data->qsfpPortDataA2[i+32][0]), qsfpPortData, QSFP_DATA_SIZE);
                                        ret = eepromDataWordRead(&SfpCopperData_client,SfpCopperPortData);
                                        if (ret>=0)
                                            memcpy(&(data->SfpCopperPortData[i+32][0]), SfpCopperPortData, SFP_COPPER_DATA_SIZE);
                                    }
                                    i2c_smbus_write_byte(&(pca9548_client[0]),  0x00);
                                }
                                else
                                {
                                    memset(&(data->qsfpPortDataA0[i+32][0]), 0, QSFP_DATA_SIZE);
                                    memset(&(data->qsfpPortDataA2[i+32][0]), 0, QSFP_DATA_SIZE);
                                    memset(&(data->SfpCopperPortData[i+32][0]), 0, SFP_COPPER_DATA_SIZE);
                                    PCA9553_CLEAR_BIT(data->qsfpPortDataValid[2], i);
                                }
                            }
                        }
                        else
                        {
                            for (i=8; i<16; i++)
                            {
                                memset(&(data->qsfpPortDataA0[i+32][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->qsfpPortDataA2[i+32][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->SfpCopperPortData[i+32][0]), 0, SFP_COPPER_DATA_SIZE);
                                PCA9553_CLEAR_BIT(data->qsfpPortDataValid[2], i);
                            }
                        }

                        step = 7;
                        break;

                    case 7:
                        if ((data->qsfpPortAbsStatus[3]&0x00ff)!=0x00ff)  /* QSFP 0~5 ABS */
                        {
                            /* Turn on PCA9548#0 channel 6 on I2C-bus1 */
                            ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH06));
                            if (ret < 0)
                                break;

                            for (i=0; i<6; i++)
                            {
                                if (PCA9553_TEST_BIT(data->qsfpPortAbsStatus[3], i) == 0) /* present */
                                {
                                    ret = i2c_smbus_write_byte(&(pca9548_client[0]), (1<<i));
                                    if (ret>=0)
                                    {
                                        if (data->qsfpPortTxDisableDataUpdate[i+48] == 1)
                                        {
                                            eepromDataByteWrite(&qsfpDataA0_client, SFF8436_TX_DISABLE_ADDR, &data->qsfpPortTxDisableData[i+48], sizeof(char));
                                            data->qsfpPortTxDisableDataUpdate[i+48] = 0;
                                        }
                                        ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                        if (ret>=0)
                                        {
                                            memcpy(&(data->qsfpPortDataA0[i+48][0]), qsfpPortData, QSFP_DATA_SIZE);
                                            PCA9553_SET_BIT(data->qsfpPortDataValid[3], i);
                                        }
                                    }
                                    i2c_smbus_write_byte(&(pca9548_client[0]), 0x00);
                                }
                                else
                                {
                                    memset(&(data->qsfpPortDataA0[i+48][0]), 0, QSFP_DATA_SIZE);
                                    PCA9553_CLEAR_BIT(data->qsfpPortDataValid[3], i);
                                    data->qsfpPortTxDisableDataUpdate[i+48] = 1;
                                }
                            }
                        }
                        else
                        {
                            for (i=0; i<6; i++)
                            {
                                memset(&(data->qsfpPortDataA0[i+48][0]), 0, QSFP_DATA_SIZE);
                                PCA9553_CLEAR_BIT(data->qsfpPortDataValid[3], i);
                                data->qsfpPortTxDisableDataUpdate[i+48] = 1;
                            }
                        }

                        if (isBMCSupport == 0)
                            step = 8;
                        else
                            step = 0;
                        break;

                    case 8:
                        /* Turn on PCA9548#0 channel 7 on I2C-bus1 */
                        ret = i2c_smbus_write_byte(client, (1<<PCA9548_CH07));
                        if (ret < 0)
                            break;
                        /* Turn on PCA9548#1 channel 0 on I2C-bus1 */
                        ret = i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH07));
                        if (ret < 0)
                            break;

                        value = 0xcccc;
                        fanErr = 0;
                        for (i=0; i<W83795ADG_NUM8; i++)
                        {
                            if (FanErr[i] == 1)
                                fanErr |=  (0x1<<i);
                        }

                        if (fanErr&0x03)
                            value |= 0x0002;
                        else
                            value |= 0x0001;

                        if (fanErr&0x0c)
                            value |= 0x0020;
                        else
                            value |= 0x0010;

                        if (fanErr&0x30)
                            value |= 0x0200;
                        else
                            value |= 0x0100;

                        if (fanErr&0xc0)
                            value |= 0x2000;
                        else
                            value |= 0x1000;

                        ret = i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, value);
                        if (ret < 0)
                            break;

                        data->frontLedStatus |= 0x00ff;
                        if (fanErr==0)
                            data->frontLedStatus &= (~0x0008); /* FAN_LED_G# */
                        else
                            data->frontLedStatus &= (~0x0004); /* FAN_LED_Y# */

                        if ((platformPsuABS&0x01)==0x00) /* PSU1 Present */
                        {
                            if (platformPsuPG&0x08) /* PSU1_PG_LDC Power Goodasserted */
                                data->frontLedStatus &= (~0x0002); /* PSU1_LED_G# */
                            else
                                data->frontLedStatus &= (~0x0001); /* PSU1_LED_Y# */
                        }
                        if ((platformPsuABS&0x02)==0x00) /* PSU2 Present */
                        {
                            if (platformPsuPG&0x10) /* PSU2_PG_LDC Power Goodasserted */
                                data->frontLedStatus &= (~0x0020); /* PSU2_LED_G# */
                            else
                                data->frontLedStatus &= (~0x0010); /* PSU2_LED_Y# */
                        }

                        switch (data->systemLedStatus)
                        {
                            default:
                            case 0: /* Booting */
                                break;

                            case 1: /* Critical*/
                                data->frontLedStatus &= (~0x0040); /* SYS_LED_Y# */
                                break;

                            case 2: /* Normal */
                                data->frontLedStatus &= (~0x0080); /* SYS_LED_G# */
                                break;
                        }

                        i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->frontLedStatus);

                        /* FAN Status */
                        value =  i2c_smbus_read_word_data(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0);
                        data->fanAbs[0] = (value&0x4444);
                        data->fanDir[0] = (value&0x8888);
                        FanDir = data->fanDir[0];

                        step = 0;
                        break;

                    default:
                        step = 0;
                        break;
                }
                i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x33, 0x00);
                i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x33, 0xff);
                break;

            case NCIIX_WITH_BMC:
            case NCIIX_WITHOUT_BMC:
                switch (step)
                {
                    case 0:
                        /* Turn on PCA9548#1 channel 0 on I2C-bus1 */
                        ret = i2c_smbus_write_byte_data(client, 0, (1<<PCA9548_CH00));
                        if (ret < 0)
                            break;

                        for (j=0; j<4; j++)
                        {
                            port_status = i2c_smbus_read_word_data(&(pca9535pwr_client[j]), PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0);
                            port = ((j*2)+40);
                            SFPPortTxFaultStatus[port] = (PCA9553_TEST_BIT(port_status, 0)==0);
                            SFPPortAbsStatus[port] = (PCA9553_TEST_BIT(port_status, 1)==0);
                            SFPPortRxLosStatus[port] = (PCA9553_TEST_BIT(port_status, 2)==0);
                            port++;
                            SFPPortTxFaultStatus[port] = (PCA9553_TEST_BIT(port_status, 6)==0);
                            SFPPortAbsStatus[port] = (PCA9553_TEST_BIT(port_status, 7)==0);
                            SFPPortRxLosStatus[port] = (PCA9553_TEST_BIT(port_status, 8)==0);
                        }
                        i2c_smbus_write_byte_data(client, 0, 0x00);
                        step = 1;
                        break;

                    case 1:
                        /* Turn on PCA9548#1 channel 1 on I2C-bus1 : LED Board */
                        ret = i2c_smbus_write_byte_data(client, 0, (1<<PCA9548_CH01));
                        if (ret < 0)
                            break;

                        fanErr = 0;
                        for (i=0; i<W83795ADG_NUM8; i++)
                        {
                            if (FanErr[i] == 1)
                                fanErr |=  (0x1<<i);
                        }

                        data->frontLedStatus |= 0x00ff;
                        if (fanErr==0)
                            data->frontLedStatus &= (~0x0008); /* FAN_LED_G# */
                        else
                            data->frontLedStatus &= (~0x0004); /* FAN_LED_Y# */

                        if ((platformPsuABS&0x01)==0x00) /* PSU1 Present */
                        {
                            if (platformPsuPG&0x08) /* PSU1_PG_LDC Power Goodasserted */
                                data->frontLedStatus &= (~0x0002); /* PSU1_LED_G# */
                            else
                                data->frontLedStatus &= (~0x0001); /* PSU1_LED_Y# */
                        }
                        if ((platformPsuABS&0x02)==0x00) /* PSU2 Present */
                        {
                            if (platformPsuPG&0x10) /* PSU2_PG_LDC Power Goodasserted */
                                data->frontLedStatus &= (~0x0020); /* PSU2_LED_G# */
                            else
                                data->frontLedStatus &= (~0x0010); /* PSU2_LED_Y# */
                        }

                        switch (data->systemLedStatus)
                        {
                            default:
                            case 0: /* Booting */
                                break;

                            case 1: /* Critical*/
                                data->frontLedStatus &= (~0x0040); /* SYS_LED_Y# */
                                break;

                            case 2: /* Normal */
                                data->frontLedStatus &= (~0x0080); /* SYS_LED_G# */
                                break;
                        }

                        i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->frontLedStatus);

                        i2c_smbus_write_byte_data(client, 0, 0x00);
                        step = 2;
                        break;

                    case 2:
                        /* Turn on PCA9548#1 channel 3 on I2C-bus1 */
                        ret = i2c_smbus_write_byte_data(client, 0, (1<<PCA9548_CH03));
                        if (ret < 0)
                            break;

                        value = i2c_smbus_read_word_data(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0);
                        SFPPortAbsStatus[48] = (PCA9553_TEST_BIT(value, 9)==0);
                        SFPPortAbsStatus[49] = (PCA9553_TEST_BIT(value, 4)==0);
                        SFPPortAbsStatus[51] = (PCA9553_TEST_BIT(value, 14)==0);
                        value = i2c_smbus_read_word_data(&(pca9535pwr_client[1]), PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0);
                        SFPPortAbsStatus[50] = (PCA9553_TEST_BIT(value, 4)==0);
                        SFPPortAbsStatus[52] = (PCA9553_TEST_BIT(value, 14)==0);
                        SFPPortAbsStatus[53] = (PCA9553_TEST_BIT(value, 9)==0);
                        i2c_smbus_write_byte_data(client, 0, 0x00);
                        step = 3;
                        break;

                    case 3:
                        /* Turn on PCA9548#1 channel 7 on I2C-bus1 : FAN Board */
                        ret = i2c_smbus_write_byte_data(client, 0, (1<<PCA9548_CH07));
                        if (ret < 0)
                            break;

                        value = 0xcccc;
                        fanErr = 0;
                        for (i=0; i<W83795ADG_NUM8; i++)
                        {
                            if (FanErr[i] == 1)
                                fanErr |=  (0x1<<i);
                        }

                        if (fanErr&0x03)
                            value |= 0x0002;
                        else
                            value |= 0x0001;

                        if (fanErr&0x0c)
                            value |= 0x0020;
                        else
                            value |= 0x0010;

                        if (fanErr&0x30)
                            value |= 0x0200;
                        else
                            value |= 0x0100;

                        if (fanErr&0xc0)
                            value |= 0x2000;
                        else
                            value |= 0x1000;

                        ret = i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, value);
                        if (ret < 0)
                            break;

                        value = i2c_smbus_read_word_data(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0);
                        data->fanAbs[0] = (value&0x4444);
                        data->fanDir[0] = (value&0x8888);
                        FanDir = data->fanDir[0];


                        i2c_smbus_write_byte_data(client, 0, 0x00);
                        step = 4;
                        break;

                    case 4:
                        for (i=0; i<54; i++)
                        {
                            if (SFPPortAbsStatus[i]) /*present*/
                            {
                                i2c_smbus_write_byte_data(&(pca9548_client[1]), 0, sfpPortData_78F[i].portMaskBitForPCA9548_1);
                                i2c_smbus_write_byte_data(&(pca9548_client[0]), 0, sfpPortData_78F[i].portMaskBitForPCA9548_2TO5);
                                if ((SFPPortDataValid[i] == 0)||(i>=48))
                                {
                                    if ((i>=48)&&(data->qsfpPortTxDisableDataUpdate[i] == 1))
                                    {
                                        eepromDataByteWrite(&qsfpDataA0_client, SFF8436_TX_DISABLE_ADDR, &data->qsfpPortTxDisableData[i], sizeof(char));
                                        data->qsfpPortTxDisableDataUpdate[i] = 0;
                                    }
                                    ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                    if (ret>=0)
                                    {
                                        memcpy(&(data->qsfpPortDataA0[i][0]), qsfpPortData, QSFP_DATA_SIZE);
                                        SFPPortDataValid[i] = 1;
                                    }
                                }
                                if (i<48)
                                {
                                    ret = eepromDataBlockRead(&qsfpDataA2_client,qsfpPortData);
                                    if (ret>=0)
                                        memcpy(&(data->qsfpPortDataA2[i][0]), qsfpPortData, QSFP_DATA_SIZE);
                                    ret = eepromDataWordRead(&SfpCopperData_client,SfpCopperPortData);
                                    if (ret>=0)
                                        memcpy(&(data->SfpCopperPortData[i][0]), SfpCopperPortData, SFP_COPPER_DATA_SIZE);
                                }
                                i2c_smbus_write_byte_data(&(pca9548_client[0]), 0, 0x00);
                                i2c_smbus_write_byte_data(&(pca9548_client[1]), 0, 0x00);
                            }
                            else
                            {
                                 memset(&(data->qsfpPortDataA0[i][0]), 0, QSFP_DATA_SIZE);
                                 memset(&(data->qsfpPortDataA2[i][0]), 0, QSFP_DATA_SIZE);
                                 memset(&(data->SfpCopperPortData[i][0]), 0, SFP_COPPER_DATA_SIZE);
                                 data->qsfpPortTxDisableDataUpdate[i] = 1;
                                 SFPPortDataValid[i] = 0;
                             }
                        }
                        step = 0;
                        break;

                    default:
                        step = 0;
                        break;
                }
                break;

            case ASTERION_WITH_BMC:
            case ASTERION_WITHOUT_BMC:
                switch (step)
                {
                    case 0:
                        /* Turn on PCA9548#0 channel 0 on I2C-bus1 */
                        ret = i2c_smbus_write_byte(client, (1 << PCA9548_CH00));
                        if (ret < 0)
                            break;

                        /* SFP Port 0~23, SFP Port - RXLOS */
                        for (i = 0; i < 10; i++)
                            data->sfpPortAbsRxLosStatus[i] = i2c_smbus_read_byte_data(&(cpld_client_bus1), (0x20 + i));

                        data->sfpPortAbsRxLosStatus[10] = i2c_smbus_read_byte_data(&(cpld_client_bus1), 0x30);
                        data->sfpPortAbsRxLosStatus[11] = i2c_smbus_read_byte_data(&(cpld_client_bus1), 0x31);

                        /* Turn on PCA9548#0 channel 1 on I2C-bus1 */
                        ret = i2c_smbus_write_byte(client, (1 << PCA9548_CH01));
                        if (ret < 0)
                            break;

                        /* SFP Port 24~47, SFP Port - RXLOS */
                        for (i = 0; i < 10; i++)
                            data->sfpPortAbsRxLosStatus[i + 12] = i2c_smbus_read_byte_data(&(cpld_client_bus1), 0x20 + i);

                        data->sfpPortAbsRxLosStatus[22] = i2c_smbus_read_byte_data(&(cpld_client_bus1), 0x30);
                        data->sfpPortAbsRxLosStatus[23] = i2c_smbus_read_byte_data(&(cpld_client_bus1), 0x31);

                        /* Turn on PCA9548#0 channel 2 on I2C-bus1 */
                        ret = i2c_smbus_write_byte(client, (1 << PCA9548_CH02));
                        if (ret < 0)
                            break;

                        /* QSFP Port 48~63 */
                        for (i = 0; i < 16; i++)
                            data->qsfpPortAbsStatusAst[i] = i2c_smbus_read_byte_data(&(cpld_client_bus1), (0x20 + i));

                        step = 1;
                        break;

                    case 1:
                        /* Turn on PCA9548#0 channel 0 on I2C-bus1 */
                        ret = i2c_smbus_write_byte(client, (1 << PCA9548_CH00));
                        if (ret < 0)
                            break;

                        for (i = 0; i < 12; i++)  /* SFP 0,2,4 ... 22 */
                        {
                            if ((data->sfpPortAbsRxLosStatus[i] & 0x02) == 0)  /* present */
                            {
                                ret = i2c_smbus_write_byte_data(&(cpld_client_bus1), CPLD_REG_MUX, (0x01 + (i * 2)));

                                if (ret >= 0)
                                {
                                    ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);

                                    if (ret >= 0)
                                    {
                                        memcpy(&(data->qsfpPortDataA0[i * 2][0]), qsfpPortData, QSFP_DATA_SIZE);
                                        data->sfpPortDataValidAst[i * 2] = 1;
                                    }
                                    ret = eepromDataBlockRead(&qsfpDataA2_client,qsfpPortData);
                                    memcpy(&(data->qsfpPortDataA2[i * 2][0]), qsfpPortData, QSFP_DATA_SIZE);
                                    ret = eepromDataWordRead(&SfpCopperData_client,SfpCopperPortData);
                                    if (ret>=0)
                                        memcpy(&(data->SfpCopperPortData[i * 2][0]), SfpCopperPortData, SFP_COPPER_DATA_SIZE);
                                }
                                i2c_smbus_write_byte_data(&(cpld_client_bus1), CPLD_REG_MUX, 0x00);
                            }
                            else
                            {
                                memset(&(data->qsfpPortDataA0[i * 2][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->qsfpPortDataA2[i * 2][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->SfpCopperPortData[i * 2][0]), 0, SFP_COPPER_DATA_SIZE);
                                data->sfpPortDataValidAst[i * 2] = 0;
                            }
                        }

                        for (i = 0; i < 12; i++)  /* SFP 1,3,5 ... 23 */
                        {
                            if ((data->sfpPortAbsRxLosStatus[i] & 0x20) == 0)  /* present */
                            {
                                ret = i2c_smbus_write_byte_data(&(cpld_client_bus1), CPLD_REG_MUX, (0x02 + (i * 2)));
                                if (ret >= 0)
                                {
                                    ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                    if (ret >= 0)
                                    {
                                        memcpy(&(data->qsfpPortDataA0[1 + (i * 2)][0]), qsfpPortData, QSFP_DATA_SIZE);
                                        data->sfpPortDataValidAst[1 + (i * 2)] = 1;
                                    }
                                    ret = eepromDataBlockRead(&qsfpDataA2_client,qsfpPortData);
                                    if (ret >= 0)
                                        memcpy(&(data->qsfpPortDataA2[1 + (i * 2)][0]), qsfpPortData, QSFP_DATA_SIZE);
                                    ret = eepromDataWordRead(&SfpCopperData_client,SfpCopperPortData);
                                    if (ret>=0)
                                        memcpy(&(data->SfpCopperPortData[1 + (i * 2)][0]), SfpCopperPortData, SFP_COPPER_DATA_SIZE);
                                }
                                i2c_smbus_write_byte_data(&(cpld_client_bus1), CPLD_REG_MUX, 0x00);
                            }
                            else
                            {
                                memset(&(data->qsfpPortDataA0[1 + (i * 2)][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->qsfpPortDataA2[1 + (i * 2)][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->SfpCopperPortData[1 + (i * 2)][0]), 0, SFP_COPPER_DATA_SIZE);
                                data->sfpPortDataValidAst[1 + (i * 2)] = 0;
                            }
                         }

                         step = 2;
                         break;

                    case 2:
                        /* Turn on PCA9548#0 channel 1 on I2C-bus1 */
                        ret = i2c_smbus_write_byte(client, (1 << PCA9548_CH01));
                        if (ret < 0)
                            break;

                        for (i = 0; i < 12; i++)  /* SFP 24,26,28 ... 46 */
                        {
                            if ((data->sfpPortAbsRxLosStatus[i + 12] & 0x02) == 0)  /* present */
                            {
                                ret = i2c_smbus_write_byte_data(&(cpld_client_bus1), CPLD_REG_MUX, (0x01 + (i * 2)));
                                if (ret >= 0)
                                {
                                    ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                    if (ret >= 0)
                                    {
                                        memcpy(&(data->qsfpPortDataA0[(i +12) * 2][0]), qsfpPortData, QSFP_DATA_SIZE);
                                        data->sfpPortDataValidAst[(i + 12) * 2] = 1;
                                    }
                                    ret = eepromDataBlockRead(&qsfpDataA2_client,qsfpPortData);
                                    if (ret >= 0)
                                        memcpy(&(data->qsfpPortDataA2[(i + 12) * 2][0]), qsfpPortData, QSFP_DATA_SIZE);
                                    ret = eepromDataWordRead(&SfpCopperData_client,SfpCopperPortData);
                                    if (ret>=0)
                                        memcpy(&(data->SfpCopperPortData[(i + 12) * 2][0]), SfpCopperPortData, SFP_COPPER_DATA_SIZE);
                                }
                                i2c_smbus_write_byte_data(&(cpld_client_bus1), CPLD_REG_MUX, 0x00);
                            }
                            else
                            {
                                memset(&(data->qsfpPortDataA0[(i + 12) * 2][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->qsfpPortDataA2[(i + 12) * 2][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->SfpCopperPortData[(i + 12) * 2][0]), 0, SFP_COPPER_DATA_SIZE);
                                data->sfpPortDataValidAst[(i + 12) * 2] = 0;
                            }
                        }

                        for (i = 0; i < 12; i++)  /* SFP 25,27,29 ... 47 */
                        {
                            if ((data->sfpPortAbsRxLosStatus[i + 12] & 0x20) == 0)  /* present */
                            {
                                ret = i2c_smbus_write_byte_data(&(cpld_client_bus1), CPLD_REG_MUX, (0x02 + (i * 2)));
                                if (ret >= 0)
                                {
                                    ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                    if (ret >= 0)
                                    {
                                        memcpy(&(data->qsfpPortDataA0[1 + ((i + 12) * 2)][0]), qsfpPortData, QSFP_DATA_SIZE);
                                        data->sfpPortDataValidAst[1 + ((i + 12) * 2)] = 1;
                                    }
                                    ret = eepromDataBlockRead(&qsfpDataA2_client,qsfpPortData);
                                    if (ret >= 0)
                                        memcpy(&(data->qsfpPortDataA2[1 + ((i + 12) * 2)][0]), qsfpPortData, QSFP_DATA_SIZE);
                                    ret = eepromDataWordRead(&SfpCopperData_client,SfpCopperPortData);
                                    if (ret>=0)
                                        memcpy(&(data->SfpCopperPortData[1 + ((i + 12) * 2)][0]), SfpCopperPortData, SFP_COPPER_DATA_SIZE);
                                }
                                i2c_smbus_write_byte_data(&(cpld_client_bus1), CPLD_REG_MUX, 0x00);
                            }
                            else
                            {
                                memset(&(data->qsfpPortDataA0[1 + ((i + 12) * 2)][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->qsfpPortDataA2[1 + ((i + 12) * 2)][0]), 0, QSFP_DATA_SIZE);
                                memset(&(data->SfpCopperPortData[1 + ((i + 12) * 2)][0]), 0, SFP_COPPER_DATA_SIZE);
                                data->sfpPortDataValidAst[1 + ((i + 12) * 2)] = 0;
                            }
                        }

                        step = 3;
                        break;

                    case 3:
                        /* Turn on PCA9548#0 channel 2 on I2C-bus1 */
                        ret = i2c_smbus_write_byte(client, (1 << PCA9548_CH02));
                        if (ret < 0)
                            break;

                        for (i = 0; i < 16; i++)  /* QSFP 48~63 */
                        {
                            if ((data->qsfpPortAbsStatusAst[i] & 0x02) == 0)  /* present */
                            {
                                ret = i2c_smbus_write_byte_data(&(cpld_client_bus1), CPLD_REG_MUX, (0x01 + i));
                                if (ret >= 0)
                                {
                                    if (data->qsfpPortTxDisableDataUpdate[48 + i] == 1)
                                    {
                                        eepromDataByteWrite(&qsfpDataA0_client, SFF8436_TX_DISABLE_ADDR, &data->qsfpPortTxDisableData[48 + i], sizeof(char));
                                        data->qsfpPortTxDisableDataUpdate[48 + i] = 0;
                                    }
                                    ret = eepromDataBlockRead(&qsfpDataA0_client,qsfpPortData);
                                    if (ret >= 0)
                                    {
                                        memcpy(&(data->qsfpPortDataA0[48 + i][0]), qsfpPortData, QSFP_DATA_SIZE);
                                        data->sfpPortDataValidAst[48 + i] = 1;
                                    }
                                }
                                i2c_smbus_write_byte_data(&(cpld_client_bus1), CPLD_REG_MUX, 0x00);
                             }
                             else
                             {
                                 memset(&(data->qsfpPortDataA0[48 + i][0]), 0, QSFP_DATA_SIZE);
                                 data->sfpPortDataValidAst[48 + i] = 0;
                                 data->qsfpPortTxDisableDataUpdate[48 + i] = 1;
                             }
                        }

                        step = 4;
                        break;

                    case 4:
                        /* Turn on PCA9548#0 channel 3 on I2C-bus1 : FAN Status */
                        ret = i2c_smbus_write_byte(client, (1 << PCA9548_CH03));
                        if (ret < 0)
                            break;

                        value = 0xcccc;
                        fanErr = 0;
                        for (i = 0; i < W83795ADG_NUM8; i++)
                        {
                            if (FanErr[i] == 1)
                                fanErr |=  (0x1 << i);
                        }

                        if (fanErr & 0x03)
                            value |= 0x0001;
                        else
                            value |= 0x0002;

                        if (fanErr & 0x0c)
                            value |= 0x0010;
                        else
                            value |= 0x0020;

                        if (fanErr & 0x30)
                            value |= 0x0100;
                        else
                            value |= 0x0200;

                        if (fanErr & 0xc0)
                            value |= 0x1000;
                        else
                            value |= 0x2000;

                        ret = i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, value);
                        if (ret < 0)
                            break;

                        /* FAN Status */
                        value = i2c_smbus_read_word_data(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0);
                        data->fanAbs[0] = (value & 0x4444);
                        data->fanDir[0] = (value & 0x8888);
                        FanDir = data->fanDir[0];

                        fanErr2 = 0;
                        for (i = W83795ADG_NUM8; i < W83795ADG_FAN_COUNT; i++)
                        {
                            if (FanErr[i] == 1)
                                fanErr2 |=  (0x1 << (i - 8));
                        }

                        if (fanErr2 & 0x03)
                            value2 = 0x0010;
                        else
                            value2 = 0x0020;

                        ret = i2c_device_word_write(&(pca9535pwr_client[1]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, value2);
                        if (ret < 0)
                            break;

                        /* FAN Status */
                        value2 = i2c_smbus_read_word_data(&(pca9535pwr_client[1]), PCA9553_COMMAND_BYTE_REG_INPUT_PORT_0);

                        data->fanAbs[1] = (value2 & 0x0040);
                        data->fanDir[1] = (value2 & 0x0080);
                        FanDir2 = data->fanDir[1];

                        /* Turn on PCA9548#0 channel 4 on I2C-bus1 : System LED */
                        ret = i2c_smbus_write_byte(client, (1 << PCA9548_CH04));
                        if (ret < 0)
                            break;

                        data->frontLedStatus |= 0x00ff;
                        if (fanErr == 0 && fanErr2 == 0)
                            data->frontLedStatus &= (~0x0010); /* FAN_LED_G# */
                        else
                            data->frontLedStatus &= (~0x0020); /* FAN_LED_Y# */

                        if ((platformPsuABS & 0x01) == 0x00) /* PSU1 Present */
                        {
                            if (platformPsuPG & 0x04) /* PSU1_PG_LDC Power Goodasserted */
                                data->frontLedStatus &= (~0x0001); /* PSU1_LED_G# */
                            else
                                data->frontLedStatus &= (~0x0002); /* PSU1_LED_Y# */
                        }
                        if ((platformPsuABS & 0x02) == 0x00) /* PSU2 Present */
                        {
                            if (platformPsuPG & 0x08) /* PSU2_PG_LDC Power Goodasserted */
                                data->frontLedStatus &= (~0x0004); /* PSU2_LED_G# */
                            else
                                data->frontLedStatus &= (~0x0008); /* PSU2_LED_Y# */
                        }

                        switch (data->systemLedStatus)
                        {
                            default:
                            case 0: /* Booting */
                                break;

                            case 1: /* Critical*/
                                data->frontLedStatus &= (~0x0080); /* SYS_LED_Y# */
                                break;

                            case 2: /* Normal */
                                data->frontLedStatus &= (~0x0040); /* SYS_LED_G# */
                                break;
                        }

                        i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->frontLedStatus);

                        step = 0;
                        break;

                    default:
                        step = 0;
                        break;
                }
                i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x33, 0x00);
                i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x33, 0xff);
                break;

            default:
                break;
        }
        mutex_unlock(&data->lock);

        if (kthread_should_stop())
            break;
        msleep_interruptible(200);
    } /* End of while (!kthread_should_stop()) */

    complete_all(&data->auto_update_stop);
    return 0;
}

static ssize_t show_chip_info(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    return sprintf(buf, "Vender ID = 0x%04X, Chip ID = 0x%04X, Device ID = 0x%04X\n", data->venderId, data->chipId, data->dviceId);
}

static ssize_t show_board_build_revision(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    return sprintf(buf, "%d\n", data->buildRev);
}

static ssize_t show_board_hardware_revision(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    return sprintf(buf, "%d\n", data->hwRev);
}

static ssize_t show_board_model_id(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    return sprintf(buf, "%d\n", data->modelId);
}

static ssize_t show_cpld_info(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    return sprintf(buf, "CPLD code Revision = 0x%02X, Release Bit = 0x%02X\n", data->cpldRev, data->cpldRel);
}

static ssize_t show_psu_pg_sen(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned int value;

    mutex_lock(&data->lock);
    value = data->psuPG;
    mutex_unlock(&data->lock);

    switch(platformModelId)
    {
        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
        {
            if (attr->index == 0)
                value &= 0x04;
            else
                value &= 0x08;
        }
            break;

        default:
        {
            if (attr->index == 0)
                value &= 0x08;
            else
                value &= 0x10;
        }
            break;
    }
    return sprintf(buf, "%d\n", value?1:0);
}

static ssize_t show_psu_abs_sen(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned int value;

    mutex_lock(&data->lock);
    value = data->psuABS;
    mutex_unlock(&data->lock);

    if (attr->index == 0)
        value &= 0x01;
    else
        value &= 0x02;
    return sprintf(buf, "%d\n", value?0:1);
}

static ssize_t show_fan_rpm(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned int fanSpeed = 0;

    if (attr->index < W83795ADG_FAN_COUNT)
        fanSpeed = data->fanSpeed[attr->index];
    return sprintf(buf, "%d\n", fanSpeed);
}

static ssize_t show_fan_duty(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned int fanDuty = 0;

    if (attr->index < W83795ADG_FAN_COUNT)
        fanDuty = ((data->fanDuty*100)/0xff);
    return sprintf(buf, "%d\n", fanDuty);
}

static ssize_t show_remote_temp(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);

    if (data->remoteTempIsPositive[attr->index]==1)
        return sprintf(buf, "%d.%d\n", data->remoteTempInt[attr->index], data->remoteTempDecimal[attr->index]);
    else
        return sprintf(buf, "-%d.%d\n", data->remoteTempInt[attr->index], data->remoteTempDecimal[attr->index]);
}

static ssize_t show_mac_temp(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);

    return sprintf(buf, "%d\n", data->macTemp);
}

static ssize_t set_mac_temp(struct device *dev, struct device_attribute *devattr, const char *buf,
          size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    long temp;

    if (kstrtol(buf, 10, &temp))
        return -EINVAL;

    temp = clamp_val(temp, 0, 120);

    mutex_lock(&data->lock);
    data->macTemp = temp;
    mutex_unlock(&data->lock);

    return count;
}

static ssize_t show_wd_refresh(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);

    return sprintf(buf, "%d\n", data->wdRefreshControlFlag);
}

static ssize_t set_wd_refresh(struct device *dev, struct device_attribute *devattr, const char *buf,
          size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    long temp;

    if (kstrtol(buf, 10, &temp))
        return -EINVAL;

    temp = clamp_val(temp, 0, 1);

    mutex_lock(&data->lock);
    data->wdRefreshControlFlag = temp;
    mutex_unlock(&data->lock);

    return count;
}

static ssize_t show_wd_refresh_control(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);

    return sprintf(buf, "%d\n", data->wdRefreshControl);
}

static ssize_t set_wd_refresh_control(struct device *dev, struct device_attribute *devattr, const char *buf,
          size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    long temp;

    if (kstrtol(buf, 10, &temp))
        return -EINVAL;

    temp = clamp_val(temp, 0, 1);

    mutex_lock(&data->lock);
    data->wdRefreshControl = temp;
    mutex_unlock(&data->lock);

    return count;
}

static ssize_t show_wd_refresh_time(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);

    return sprintf(buf, "%d\n", data->wdRefreshTimeSelect);
}

static ssize_t set_wd_refresh_time(struct device *dev, struct device_attribute *devattr, const char *buf,
          size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    long temp;

    if (kstrtol(buf, 10, &temp))
        return -EINVAL;

    temp = clamp_val(temp, 0, 10);

    mutex_lock(&data->lock);
    data->wdRefreshTimeSelect = temp;
    data->wdRefreshTimeSelectFlag = 1;
    mutex_unlock(&data->lock);

    return count;
}

static ssize_t show_wd_timeout_occurrence(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);

    return sprintf(buf, "%d\n", data->wdTimeoutSelect);
}

static ssize_t set_wd_timeout_occurrence(struct device *dev, struct device_attribute *devattr, const char *buf,
          size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    long temp;

    if (kstrtol(buf, 10, &temp))
        return -EINVAL;

    temp = clamp_val(temp, 0, 1);

    mutex_lock(&data->lock);
    data->wdTimeoutSelect = temp;
    data->wdTimeoutSelectFlag = 1;
    mutex_unlock(&data->lock);

    return count;
}

static ssize_t show_rov(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);

    return sprintf(buf, "%d\n", data->rov);
}

static ssize_t set_rov(struct device *dev, struct device_attribute *devattr, const char *buf,
          size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    long rov;

    if (kstrtol(buf, 10, &rov))
        return -EINVAL;

    rov = clamp_val(rov, 0, 16);

    mutex_lock(&data->lock);
    switch (data->modelId)
    {
        case HURACAN_WITH_BMC:
        case HURACAN_WITHOUT_BMC:
        case HURACAN_A_WITH_BMC:
        case HURACAN_A_WITHOUT_BMC:
        {
            /*
            - 4'b0000 = 1.2000V    -> 0x47
            - 4'b0001 = 1.1750V    -> 0x44
            - 4'b0010 = 1.1500V    -> 0x42
            - 4'b0011 = 1.1250V    -> 0x3f
            - 4'b0100 = 1.1000V    -> 0x3c
            - 4'b0101 = 1.0750V    -> 0x39
            - 4'b0110 = 1.0500V    -> 0x37
            - 4'b0111 = 1.0250V    -> 0x35
            - 4'b1000 = 1.0000V    -> 0x33
            - 4'b1001 = 0.9750V    -> 0x30
            - 4'b1010 = 0.9500V    -> 0x2d
            - 4'b1011 = 0.9250V    -> 0x2b
            - 4'b1100 = 0.9000V    -> 0x28
            - 4'b1101 = 0.8750V    -> 0x26
            - 4'b1110 = 0.8500V    -> 0x23
            - 4'b1111 = 0.8250V    -> 0x21
            */
            const unsigned short ROVtranslate[]= {0x47,0x44,0x42,0x3f,0x3c,0x39,0x37,0x35,0x33,0x30,0x2d,0x2b,0x28,0x26,0x23,0x21};

            rov &= 0xf;
            /* In "56960-DS111-RDS.pdf" page 58, the voltage range of BCM56960 for power supply is 0.95V to 1.025V. */
            if (rov<7) rov = 7;

            /* set rov to VOUT_COMMAND register */
            i2c_smbus_write_word_data(&mp2953agu_client, 0x21, ROVtranslate[rov]);
        }
            break;

        case SESTO_WITH_BMC:
        case SESTO_WITHOUT_BMC:
        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
        {
            /*
            - 4'b0000 = 1.2000V    -> 0x47
            - 4'b0001 = 1.1750V    -> 0x44
            - 4'b0010 = 1.1500V    -> 0x42
            - 4'b0011 = 1.1250V    -> 0x3e
            - 4'b0100 = 1.1000V    -> 0x3c
            - 4'b0101 = 1.0750V    -> 0x39
            - 4'b0110 = 1.0500V    -> 0x37
            - 4'b0111 = 1.0250V    -> 0x34
            - 4'b1000 = 1.0000V    -> 0x32
            - 4'b1001 = 0.9750V    -> 0x2f
            - 4'b1010 = 0.9500V    -> 0x2d
            - 4'b1011 = 0.9250V    -> 0x2b
            - 4'b1100 = 0.9000V    -> 0x28
            - 4'b1101 = 0.8750V    -> 0x26
            - 4'b1110 = 0.8500V    -> 0x23
            - 4'b1111 = 0.8250V    -> 0x21
            */
            const unsigned short ROVtranslate[]= {0x47,0x44,0x42,0x3e,0x3c,0x39,0x37,0x34,0x32,0x2f,0x2d,0x2b,0x28,0x26,0x23,0x21};

            rov &= 0xf;
            /* In "56960-DS111-RDS.pdf" page 58, the voltage range of BCM56960 for power supply is 0.95V to 1.025V. */
            if (rov<7) rov = 7;

            /* set rov to VOUT_COMMAND register */
            i2c_smbus_write_word_data(&mp2953agu_client, 0x21, ROVtranslate[rov]);
        }
            break;

        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
        {
            /*
            - 3'b000 = 1.025V    -> 0x9C
            - 3'b001 = 1.025V    -> 0x9C
            - 3'b010 = 0.95V     -> 0x8D
            - 3'b011 = RESV
            - 3'b100 = RESV
            - 3'b101 = RESV
            - 3'b110 = RESV
            - 3'b111 = RESV
             */
            char getValue = 0;
            char loop1_flag = 0;
            const char ROVtranslate[]= {CHL8325_VID_DEFAULT,CHL8325_VID0,CHL8325_VID1};

            rov &= 0xf;
            /* In "56750_56850-PR103-RDS.pdf" page 926, 3b011 ~ 3'b111 are reserved. */
            if (rov>2) rov = 0;

            /* Turn on PCA9548#0 channel 0 on I2C-bus0 */
            i2c_smbus_write_byte_data(&pca9548_client_bus0, 0, 0x01);

            /* Step 1. Disable LOOP1_VID */
            /* Get D0 register value */
            getValue = i2c_smbus_read_byte_data(&chl8325a_client, LOOP1_VID_OVERRIDE_ENABLE_REG);
            /* Disable CHL8325A PWM controller Loop1 */
            loop1_flag = getValue & (~CHL8325_LOOP1_Enable);
            i2c_smbus_write_byte_data(&chl8325a_client, LOOP1_VID_OVERRIDE_ENABLE_REG, loop1_flag);

            /* Step 2. Config CHL8325A PWM controller */
            i2c_smbus_write_byte_data(&chl8325a_client, LOOP1_OVERRIDE_VID_SETTING_REG, ROVtranslate[rov]);

            /* Step 3. Get D0 register value */
            getValue = i2c_smbus_read_byte_data(&chl8325a_client, LOOP1_VID_OVERRIDE_ENABLE_REG);

            /* Step 4. Config CHL8325A PWM controller Loop1 */
            loop1_flag = getValue | CHL8325_LOOP1_Enable;
            i2c_smbus_write_byte_data(&chl8325a_client, LOOP1_VID_OVERRIDE_ENABLE_REG, loop1_flag);

            i2c_smbus_write_byte(&pca9548_client_bus0, 0x00);
        }
            break;

        default:
            break;
    }
    data->rov = rov;
    mutex_unlock(&data->lock);

    return count;
}

static ssize_t show_voltage_sen(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned int MNTVSEN, MNTV;
    unsigned int voltage;

    mutex_lock(&data->lock);
    MNTVSEN = data->vSen[attr->index];
    MNTV = data->vSenLsb[attr->index];
    mutex_unlock(&data->lock);

    voltage = ((MNTVSEN << 2) + ((MNTV & 0xC0) >> 6));
    voltage *= ((2*VOL_MONITOR_UNIT)/VOL_MONITOR_UNIT);

    return sprintf(buf, "%d.%03d\n", (voltage/VOL_MONITOR_UNIT), (voltage%VOL_MONITOR_UNIT));
}

static DEVICE_ATTR(mac_temp, S_IWUSR | S_IRUGO, show_mac_temp, set_mac_temp);
static DEVICE_ATTR(chip_info, S_IRUGO, show_chip_info, NULL);
static DEVICE_ATTR(board_build_rev, S_IRUGO, show_board_build_revision, NULL);
static DEVICE_ATTR(board_hardware_rev, S_IRUGO, show_board_hardware_revision, NULL);
static DEVICE_ATTR(board_model_id, S_IRUGO, show_board_model_id, NULL);
static DEVICE_ATTR(cpld_info, S_IRUGO, show_cpld_info, NULL);
static DEVICE_ATTR(wd_refresh, S_IWUSR | S_IRUGO, show_wd_refresh, set_wd_refresh);
static DEVICE_ATTR(wd_refresh_control, S_IWUSR | S_IRUGO, show_wd_refresh_control, set_wd_refresh_control);
static DEVICE_ATTR(wd_refresh_time, S_IWUSR | S_IRUGO, show_wd_refresh_time, set_wd_refresh_time);
static DEVICE_ATTR(wd_timeout_occurrence, S_IWUSR | S_IRUGO, show_wd_timeout_occurrence, set_wd_timeout_occurrence);
static DEVICE_ATTR(rov, S_IWUSR | S_IRUGO, show_rov, set_rov);

static SENSOR_DEVICE_ATTR(psu1_pg, S_IRUGO, show_psu_pg_sen, NULL, 0);
static SENSOR_DEVICE_ATTR(psu2_pg, S_IRUGO, show_psu_pg_sen, NULL, 1);
static SENSOR_DEVICE_ATTR(psu1_abs, S_IRUGO, show_psu_abs_sen, NULL, 0);
static SENSOR_DEVICE_ATTR(psu2_abs, S_IRUGO, show_psu_abs_sen, NULL, 1);

static SENSOR_DEVICE_ATTR(fan1_rpm, S_IRUGO, show_fan_rpm, NULL, 0);
static SENSOR_DEVICE_ATTR(fan2_rpm, S_IRUGO, show_fan_rpm, NULL, 1);
static SENSOR_DEVICE_ATTR(fan3_rpm, S_IRUGO, show_fan_rpm, NULL, 2);
static SENSOR_DEVICE_ATTR(fan4_rpm, S_IRUGO, show_fan_rpm, NULL, 3);
static SENSOR_DEVICE_ATTR(fan5_rpm, S_IRUGO, show_fan_rpm, NULL, 4);
static SENSOR_DEVICE_ATTR(fan6_rpm, S_IRUGO, show_fan_rpm, NULL, 5);
static SENSOR_DEVICE_ATTR(fan7_rpm, S_IRUGO, show_fan_rpm, NULL, 6);
static SENSOR_DEVICE_ATTR(fan8_rpm, S_IRUGO, show_fan_rpm, NULL, 7);
static SENSOR_DEVICE_ATTR(fan9_rpm, S_IRUGO, show_fan_rpm, NULL, 8);
static SENSOR_DEVICE_ATTR(fan10_rpm, S_IRUGO, show_fan_rpm, NULL, 9);

static SENSOR_DEVICE_ATTR(fan1_duty, S_IRUGO, show_fan_duty, NULL, 0);
static SENSOR_DEVICE_ATTR(fan2_duty, S_IRUGO, show_fan_duty, NULL, 1);
static SENSOR_DEVICE_ATTR(fan3_duty, S_IRUGO, show_fan_duty, NULL, 2);
static SENSOR_DEVICE_ATTR(fan4_duty, S_IRUGO, show_fan_duty, NULL, 3);
static SENSOR_DEVICE_ATTR(fan5_duty, S_IRUGO, show_fan_duty, NULL, 4);
static SENSOR_DEVICE_ATTR(fan6_duty, S_IRUGO, show_fan_duty, NULL, 5);
static SENSOR_DEVICE_ATTR(fan7_duty, S_IRUGO, show_fan_duty, NULL, 6);
static SENSOR_DEVICE_ATTR(fan8_duty, S_IRUGO, show_fan_duty, NULL, 7);
static SENSOR_DEVICE_ATTR(fan9_duty, S_IRUGO, show_fan_duty, NULL, 8);
static SENSOR_DEVICE_ATTR(fan10_duty, S_IRUGO, show_fan_duty, NULL, 9);

static SENSOR_DEVICE_ATTR(remote_temp1, S_IRUGO, show_remote_temp, NULL, 0);
static SENSOR_DEVICE_ATTR(remote_temp2, S_IRUGO, show_remote_temp, NULL, 1);
static SENSOR_DEVICE_ATTR(remote_temp3, S_IRUGO, show_remote_temp, NULL, 2);
static SENSOR_DEVICE_ATTR(remote_temp4, S_IRUGO, show_remote_temp, NULL, 3);

static SENSOR_DEVICE_ATTR(vsen1, S_IRUGO, show_voltage_sen, NULL, 0);
static SENSOR_DEVICE_ATTR(vsen2, S_IRUGO, show_voltage_sen, NULL, 1);
static SENSOR_DEVICE_ATTR(vsen3, S_IRUGO, show_voltage_sen, NULL, 2);
static SENSOR_DEVICE_ATTR(vsen4, S_IRUGO, show_voltage_sen, NULL, 3);
static SENSOR_DEVICE_ATTR(vsen5, S_IRUGO, show_voltage_sen, NULL, 4);
static SENSOR_DEVICE_ATTR(vsen7, S_IRUGO, show_voltage_sen, NULL, 6);

static struct attribute *i2c_bus0_hardware_monitor_attr[] = {
    &dev_attr_mac_temp.attr,
    &dev_attr_chip_info.attr,
    &dev_attr_board_build_rev.attr,
    &dev_attr_board_hardware_rev.attr,
    &dev_attr_board_model_id.attr,
    &dev_attr_cpld_info.attr,
    &dev_attr_wd_refresh.attr,
    &dev_attr_wd_refresh_control.attr,
    &dev_attr_wd_refresh_time.attr,
    &dev_attr_wd_timeout_occurrence.attr,
    &dev_attr_rov.attr,

    &sensor_dev_attr_psu1_pg.dev_attr.attr,
    &sensor_dev_attr_psu2_pg.dev_attr.attr,
    &sensor_dev_attr_psu1_abs.dev_attr.attr,
    &sensor_dev_attr_psu2_abs.dev_attr.attr,

    &sensor_dev_attr_fan1_rpm.dev_attr.attr,
    &sensor_dev_attr_fan2_rpm.dev_attr.attr,
    &sensor_dev_attr_fan3_rpm.dev_attr.attr,
    &sensor_dev_attr_fan4_rpm.dev_attr.attr,
    &sensor_dev_attr_fan5_rpm.dev_attr.attr,
    &sensor_dev_attr_fan6_rpm.dev_attr.attr,
    &sensor_dev_attr_fan7_rpm.dev_attr.attr,
    &sensor_dev_attr_fan8_rpm.dev_attr.attr,

    &sensor_dev_attr_fan1_duty.dev_attr.attr,
    &sensor_dev_attr_fan2_duty.dev_attr.attr,
    &sensor_dev_attr_fan3_duty.dev_attr.attr,
    &sensor_dev_attr_fan4_duty.dev_attr.attr,
    &sensor_dev_attr_fan5_duty.dev_attr.attr,
    &sensor_dev_attr_fan6_duty.dev_attr.attr,
    &sensor_dev_attr_fan7_duty.dev_attr.attr,
    &sensor_dev_attr_fan8_duty.dev_attr.attr,

    &sensor_dev_attr_remote_temp1.dev_attr.attr,
    &sensor_dev_attr_remote_temp2.dev_attr.attr,

    &sensor_dev_attr_vsen1.dev_attr.attr,
    &sensor_dev_attr_vsen2.dev_attr.attr,
    &sensor_dev_attr_vsen3.dev_attr.attr,
    &sensor_dev_attr_vsen4.dev_attr.attr,

    NULL
};

static struct attribute *i2c_bus0_hardware_monitor_attr_nc2x[] = {
    &dev_attr_mac_temp.attr,
    &dev_attr_chip_info.attr,
    &dev_attr_board_build_rev.attr,
    &dev_attr_board_hardware_rev.attr,
    &dev_attr_board_model_id.attr,
    &dev_attr_cpld_info.attr,
    &dev_attr_wd_refresh.attr,
    &dev_attr_wd_refresh_control.attr,
    &dev_attr_wd_refresh_time.attr,
    &dev_attr_wd_timeout_occurrence.attr,
    &dev_attr_rov.attr,

    &sensor_dev_attr_psu1_pg.dev_attr.attr,
    &sensor_dev_attr_psu2_pg.dev_attr.attr,
    &sensor_dev_attr_psu1_abs.dev_attr.attr,
    &sensor_dev_attr_psu2_abs.dev_attr.attr,

    &sensor_dev_attr_fan1_rpm.dev_attr.attr,
    &sensor_dev_attr_fan2_rpm.dev_attr.attr,
    &sensor_dev_attr_fan3_rpm.dev_attr.attr,
    &sensor_dev_attr_fan4_rpm.dev_attr.attr,
    &sensor_dev_attr_fan5_rpm.dev_attr.attr,
    &sensor_dev_attr_fan6_rpm.dev_attr.attr,
    &sensor_dev_attr_fan7_rpm.dev_attr.attr,
    &sensor_dev_attr_fan8_rpm.dev_attr.attr,

    &sensor_dev_attr_fan1_duty.dev_attr.attr,
    &sensor_dev_attr_fan2_duty.dev_attr.attr,
    &sensor_dev_attr_fan3_duty.dev_attr.attr,
    &sensor_dev_attr_fan4_duty.dev_attr.attr,
    &sensor_dev_attr_fan5_duty.dev_attr.attr,
    &sensor_dev_attr_fan6_duty.dev_attr.attr,
    &sensor_dev_attr_fan7_duty.dev_attr.attr,
    &sensor_dev_attr_fan8_duty.dev_attr.attr,

    &sensor_dev_attr_remote_temp1.dev_attr.attr,
    &sensor_dev_attr_remote_temp2.dev_attr.attr,

    &sensor_dev_attr_vsen1.dev_attr.attr,
    &sensor_dev_attr_vsen4.dev_attr.attr,
    &sensor_dev_attr_vsen5.dev_attr.attr,
    &sensor_dev_attr_vsen7.dev_attr.attr,

    NULL
};

static struct attribute *i2c_bus0_hardware_monitor_attr_asterion[] = {
    &dev_attr_mac_temp.attr,
    &dev_attr_chip_info.attr,
    &dev_attr_board_build_rev.attr,
    &dev_attr_board_hardware_rev.attr,
    &dev_attr_board_model_id.attr,
    &dev_attr_cpld_info.attr,
    &dev_attr_wd_refresh.attr,
    &dev_attr_wd_refresh_control.attr,
    &dev_attr_wd_refresh_time.attr,
    &dev_attr_wd_timeout_occurrence.attr,
    &dev_attr_rov.attr,

    &sensor_dev_attr_psu1_pg.dev_attr.attr,
    &sensor_dev_attr_psu2_pg.dev_attr.attr,
    &sensor_dev_attr_psu1_abs.dev_attr.attr,
    &sensor_dev_attr_psu2_abs.dev_attr.attr,

    &sensor_dev_attr_fan1_rpm.dev_attr.attr,
    &sensor_dev_attr_fan2_rpm.dev_attr.attr,
    &sensor_dev_attr_fan3_rpm.dev_attr.attr,
    &sensor_dev_attr_fan4_rpm.dev_attr.attr,
    &sensor_dev_attr_fan5_rpm.dev_attr.attr,
    &sensor_dev_attr_fan6_rpm.dev_attr.attr,
    &sensor_dev_attr_fan7_rpm.dev_attr.attr,
    &sensor_dev_attr_fan8_rpm.dev_attr.attr,
    &sensor_dev_attr_fan9_rpm.dev_attr.attr,
    &sensor_dev_attr_fan10_rpm.dev_attr.attr,

    &sensor_dev_attr_fan1_duty.dev_attr.attr,
    &sensor_dev_attr_fan2_duty.dev_attr.attr,
    &sensor_dev_attr_fan3_duty.dev_attr.attr,
    &sensor_dev_attr_fan4_duty.dev_attr.attr,
    &sensor_dev_attr_fan5_duty.dev_attr.attr,
    &sensor_dev_attr_fan6_duty.dev_attr.attr,
    &sensor_dev_attr_fan7_duty.dev_attr.attr,
    &sensor_dev_attr_fan8_duty.dev_attr.attr,
    &sensor_dev_attr_fan9_duty.dev_attr.attr,
    &sensor_dev_attr_fan10_duty.dev_attr.attr,

    &sensor_dev_attr_remote_temp1.dev_attr.attr,
    &sensor_dev_attr_remote_temp2.dev_attr.attr,
    &sensor_dev_attr_remote_temp3.dev_attr.attr,
    &sensor_dev_attr_remote_temp4.dev_attr.attr,

    &sensor_dev_attr_vsen1.dev_attr.attr,
    &sensor_dev_attr_vsen2.dev_attr.attr,
    &sensor_dev_attr_vsen3.dev_attr.attr,
    &sensor_dev_attr_vsen4.dev_attr.attr,

    NULL
};


static ssize_t show_port_abs(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(&(pca9535pwr_client[0]));
    struct i2c_bus1_hardware_monitor_data *dataAst = i2c_get_clientdata(&(cpld_client_bus1));
    int rc = 0;

    switch(platformModelId)
    {
        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
            rc = ((SFPPortAbsStatus[attr->index]==1)&&(SFPPortDataValid[attr->index]==1));
            break;

        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
        {
            unsigned char qsfpPortAbsAst = 0, index = 0, bit = 0;
            unsigned char sfpPortDataValidAst = 0;

            if (attr->index < 48)
            {
                index = (attr->index / 2);
                bit = ((attr->index & 0x01) ? 5 : 1);
                mutex_lock(&dataAst->lock);
                qsfpPortAbsAst = dataAst->sfpPortAbsRxLosStatus[index];
                sfpPortDataValidAst = dataAst->sfpPortDataValidAst[attr->index];
                mutex_unlock(&dataAst->lock);
                rc = ((PCA9553_TEST_BIT(qsfpPortAbsAst, bit) ? 0 : 1)&&sfpPortDataValidAst);
            }
            else
            {
                index = (attr->index % 48);
                mutex_lock(&dataAst->lock);
                qsfpPortAbsAst = dataAst->qsfpPortAbsStatusAst[index];
                sfpPortDataValidAst = dataAst->sfpPortDataValidAst[attr->index];
                mutex_unlock(&dataAst->lock);
                rc = ((PCA9553_TEST_BIT(qsfpPortAbsAst, 1) ? 0 : 1)&&sfpPortDataValidAst);
            }
        }
            break;

        default:
        {
            unsigned short qsfpPortAbs=0, index=0, bit=0;
            unsigned short qsfpPortDataValid=0;

            index = (attr->index/16);
            bit = (attr->index%16);
            mutex_lock(&data->lock);
            qsfpPortAbs = data->qsfpPortAbsStatus[index];
            qsfpPortDataValid = data->qsfpPortDataValid[index];
            mutex_unlock(&data->lock);
            rc = ((PCA9553_TEST_BIT(qsfpPortAbs, bit)?0:1)&&(PCA9553_TEST_BIT(qsfpPortDataValid, bit)));
        }
            break;
    }

    return sprintf(buf, "%d\n", rc);
}

static ssize_t show_port_rxlos(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(&(pca9535pwr_client[0]));
    int rc = 0;

    switch(platformModelId)
    {
        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
            rc = (SFPPortRxLosStatus[attr->index]?0:1);
            break;

        case SESTO_WITH_BMC:
        case SESTO_WITHOUT_BMC:
        {
            unsigned short qsfpPortRxLos=0, index=0, bit=0;

            index = (attr->index/16);
            bit = (attr->index%16);
            mutex_lock(&data->lock);
            qsfpPortRxLos = data->sfpPortRxLosStatus[index];
            mutex_unlock(&data->lock);
            rc = (PCA9553_TEST_BIT(qsfpPortRxLos, bit)?1:0);
        }
            break;

        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
        {
            unsigned char qsfpPortRxLos = 0, index = 0, bit = 0;

            index = (attr->index / 2);
            bit = ((attr->index & 0x01) ? 4 : 0);
            mutex_lock(&data->lock);
            qsfpPortRxLos = data->sfpPortAbsRxLosStatus[index];
            mutex_unlock(&data->lock);
            rc = (PCA9553_TEST_BIT(qsfpPortRxLos, bit) ? 1 : 0);
        }
            break;

        default:
            break;
    }

    return sprintf(buf, "%d\n", rc);
}

static ssize_t show_port_data_a0(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(&qsfpDataA0_client);
    unsigned char qsfpPortData[QSFP_DATA_SIZE];
    ssize_t count = 0;

    memset(qsfpPortData, 0, QSFP_DATA_SIZE);
    mutex_lock(&data->lock);
    memcpy(qsfpPortData, &(data->qsfpPortDataA0[attr->index][0]), QSFP_DATA_SIZE);
    mutex_unlock(&data->lock);

    count = QSFP_DATA_SIZE;
    memcpy(buf, (char *)qsfpPortData, QSFP_DATA_SIZE);

    return count;
}

static ssize_t show_port_data_a2(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(&qsfpDataA2_client);
    unsigned char qsfpPortData[QSFP_DATA_SIZE];
    ssize_t count = 0;

    memset(qsfpPortData, 0, QSFP_DATA_SIZE);
    mutex_lock(&data->lock);
    memcpy(qsfpPortData, &(data->qsfpPortDataA2[attr->index][0]), QSFP_DATA_SIZE);
    mutex_unlock(&data->lock);

    count = QSFP_DATA_SIZE;
    memcpy(buf, (char *)qsfpPortData, QSFP_DATA_SIZE);

    return count;
}

static ssize_t show_port_sfp_copper(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(&SfpCopperData_client);
    unsigned char SfpCopperPortData[SFP_COPPER_DATA_SIZE];
    ssize_t count = 0;

    memset(SfpCopperPortData, 0, SFP_COPPER_DATA_SIZE);
    mutex_lock(&data->lock);
    memcpy(SfpCopperPortData, &(data->SfpCopperPortData[attr->index][0]), SFP_COPPER_DATA_SIZE);
    mutex_unlock(&data->lock);

    count = SFP_COPPER_DATA_SIZE;
    memcpy(buf, (char *)SfpCopperPortData, SFP_COPPER_DATA_SIZE);

    return count;
}

static ssize_t show_fan_abs(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned int value = 0;
    unsigned int index = 0;

    mutex_lock(&data->lock);
    if (attr->index<4)
    {
        value = (unsigned int)data->fanAbs[0];
        index = attr->index;
    }
    else
    {
        value = (unsigned int)data->fanAbs[1];
        index = (attr->index-3);
    }
    mutex_unlock(&data->lock);

    value &= (0x0004<<(index*4));
    return sprintf(buf, "%d\n", value?0:1);
}

static ssize_t show_fan_dir(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned int value = 0;
    unsigned int index = 0;

    mutex_lock(&data->lock);
    if (attr->index<4)
    {
        value = (unsigned int)data->fanDir[0];
        index = attr->index;
    }
    else
    {
        value = (unsigned int)data->fanDir[1];
        index = (attr->index-3);
    }
    mutex_unlock(&data->lock);

    value &= (0x0008<<(index*4));
    return sprintf(buf, "%d\n", value?0:1);
}

static ssize_t show_eeprom(struct device *dev, struct device_attribute *devattr, char *buf)
{
    unsigned char eepromData[EEPROM_DATA_SIZE];
    ssize_t count = 0;
    int ret = 0;

    memset(eepromData, 0, EEPROM_DATA_SIZE);
    switch(platformModelId)
    {
        case HURACAN_WITH_BMC:
        case HURACAN_WITHOUT_BMC:
        case HURACAN_A_WITH_BMC:
        case HURACAN_A_WITHOUT_BMC:
            {
                struct i2c_client *client = to_i2c_client(dev);
                struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);

                mutex_lock(&data->lock);
                /* Turn on PCA9548 channel 7 on I2C-bus1 */
                i2c_smbus_write_byte(client, 0x80);
                i2c_smbus_write_byte_data(&eeprom_client, 0x00, 0x00);
                ret = eepromDataRead(&eeprom_client, &(eepromData[0]));
                i2c_smbus_write_byte(client, 0x00);
                mutex_unlock(&data->lock);
            }
            break;

        case CABRERAIII_WITH_BMC:
        case CABRERAIII_WITHOUT_BMC:
            break;

        case SESTO_WITH_BMC:
        case SESTO_WITHOUT_BMC:
            {
                struct i2c_client *client = to_i2c_client(dev);
                struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);

                mutex_lock(&data->lock);
                /* Turn on PCA9548#1 channel 7 on I2C-bus1 */
                i2c_smbus_write_byte(&(pca9548_client[1]), 0x80);
                i2c_smbus_write_byte_data(&eeprom_client, 0x00, 0x00);
                ret = eepromDataRead(&eeprom_client, &(eepromData[0]));
                i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
                mutex_unlock(&data->lock);
            }
            break;

        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
            {
                if (platformHwRev == 0x03) /* PVT */
                {
                    struct i2c_client *client = to_i2c_client(dev);
                    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);

                    mutex_lock(&data->lock);
                    /* Turn on PCA9548#1 channel 2 on I2C-bus1 */
                    i2c_smbus_write_byte(client, 0x04);
                    i2c_smbus_write_byte_data(&eeprom_client, 0x00, 0x00);
                    ret = eepromDataRead(&eeprom_client, &(eepromData[0]));
                    i2c_smbus_write_byte(client, 0x00);
                    mutex_unlock(&data->lock);
                }
                else
                {
                    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(&eeprom_client_bus0);

                    mutex_lock(&data->lock);
                    i2c_smbus_write_byte_data(&eeprom_client_bus0, 0x00, 0x00);
                    ret = eepromDataRead(&eeprom_client_bus0, &(eepromData[0]));
                    mutex_unlock(&data->lock);
                }
            }
            break;

        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
            {
                struct i2c_client *client = to_i2c_client(dev);
                struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);

                mutex_lock(&data->lock);
                /* Turn on PCA9548 channel 5 on I2C-bus1 */
                i2c_smbus_write_byte(client, 0x20);
                i2c_smbus_write_byte_data(&eeprom_client, 0x00, 0x00);
                ret = eepromDataRead(&eeprom_client, &(eepromData[0]));
                i2c_smbus_write_byte(client, 0x00);
                mutex_unlock(&data->lock);
            }
            break;

        default:
            break;
    }
    if (ret < 0)
        memset(eepromData, 0, EEPROM_DATA_SIZE);

    count = EEPROM_DATA_SIZE;
    memcpy(buf, (char *)eepromData, EEPROM_DATA_SIZE);

    return count;
}

static ssize_t show_psu_eeprom(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned char eepromData[EEPROM_DATA_SIZE];
    ssize_t count = 0;
    int ret = 0;
    unsigned short index=0;
    unsigned int psu_present = 0;

    index = (attr->index&0x1);
    if (index&0x01) /* PSU 2 */
    {
        if ((platformPsuABS&0x02)==0x00) /* PSU2 Present */
            psu_present = 1;
    }
    else /* PSU 1 */
    {
        if ((platformPsuABS&0x01)==0x00) /* PSU1 Present */
            psu_present = 1;
    }
    memset(eepromData, 0, EEPROM_DATA_SIZE);
    if (psu_present == 1)
    {
        switch(platformModelId)
        {
            case HURACAN_WITH_BMC:
            case HURACAN_WITHOUT_BMC:
            case ASTERION_WITH_BMC:
            case ASTERION_WITHOUT_BMC:
            case HURACAN_A_WITH_BMC:
            case HURACAN_A_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x80);
                    else
                        /* Turn on PCA9548 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x40);
                    ret = eepromDataBlockRead(&psu_eeprom_client, &(eepromData[0]));
                    i2c_smbus_write_byte(client, 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case CABRERAIII_WITH_BMC:
            case CABRERAIII_WITHOUT_BMC:
                break;

            case SESTO_WITH_BMC:
            case SESTO_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548#1 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x80);
                    else
                        /* Turn on PCA9548#1 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x40);
                    ret = eepromDataBlockRead(&psu_eeprom_client, &(eepromData[0]));
                    i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case NCIIX_WITH_BMC:
            case NCIIX_WITHOUT_BMC:
                {
                    /*
                        Because the write ability of EEPROM with 0xAx address on I2C bus 0 (I801 bus) is blocked by BIOS,
                        it does not support I2C block read, it supports byte read only.
                        The PSUs will be moved to I2C bus 1 (iSMT bus) in PVT rev2.
                    */
                    if ((platformBuildRev > 0x01) && (platformHwRev == 0x03)) /* PVT rev2*/
                    {
                        mutex_lock(&data->lock);
                        if (index)
                            i2c_smbus_write_byte(client, 0x20); /* Turn on PCA9548 channel 5 on I2C-bus1 */
                        else
                            i2c_smbus_write_byte(client, 0x10); /* Turn on PCA9548 channel 4 on I2C-bus1 */
                        ret = eepromDataBlockRead(&psu_eeprom_client, &(eepromData[0]));
                        i2c_smbus_write_byte(client, 0x00);
                        mutex_unlock(&data->lock);
                    }
                    else
                    {
                        struct i2c_bus0_hardware_monitor_data *data_bus0 = i2c_get_clientdata(&psu_eeprom_client_bus0);

                        mutex_lock(&data_bus0->lock);
                        if (index)
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x04); /* Turn on PCA9548#0 channel 2 on I2C-bus0 */
                        else
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x02); /* Turn on PCA9548#0 channel 1 on I2C-bus0 */
                        ret = eepromDataByteRead(&psu_eeprom_client_bus0, &(eepromData[0]));
                        i2c_smbus_write_byte(&pca9548_client_bus0, 0x00);
                        mutex_unlock(&data_bus0->lock);
                    }
                }
                break;

            default:
                break;
        }
        if (ret < 0)
            memset(eepromData, 0, EEPROM_DATA_SIZE);
    }

    count = EEPROM_DATA_SIZE;
    memcpy(buf, (char *)eepromData, EEPROM_DATA_SIZE);

    return count;
}

static ssize_t show_psu_vout(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned short index = 0;
    unsigned int valueV = 0;
    unsigned char valueN = 0;
    ssize_t count = 0;
    unsigned int temp = 0;
    unsigned int psu_present = 0;
    unsigned char valueE = 0;
    unsigned short valueY = 0;

    index = (attr->index&0x1);
    if (index&0x01) /* PSU 2 */
    {
        if ((platformPsuABS&0x02)==0x00) /* PSU2 Present */
            psu_present = 1;
    }
    else /* PSU 1 */
    {
        if ((platformPsuABS&0x01)==0x00) /* PSU1 Present */
            psu_present = 1;
    }
    if (psu_present == 1)
    {
        switch(platformModelId)
        {
            case HURACAN_WITH_BMC:
            case HURACAN_WITHOUT_BMC:
            case ASTERION_WITH_BMC:
            case ASTERION_WITHOUT_BMC:
            case HURACAN_A_WITH_BMC:
            case HURACAN_A_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x80);
                    else
                        /* Turn on PCA9548 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x40);
                    valueN = i2c_smbus_read_byte_data(&psu_mcu_client, 0x20);
                    valueV = (unsigned int)i2c_smbus_read_word_data(&psu_mcu_client, 0x8B);
                    i2c_smbus_write_byte(client, 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case CABRERAIII_WITH_BMC:
            case CABRERAIII_WITHOUT_BMC:
                break;

            case SESTO_WITH_BMC:
            case SESTO_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548#1 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x80);
                    else
                        /* Turn on PCA9548#1 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x40);
                    valueN = i2c_smbus_read_byte_data(&psu_mcu_client, 0x20);
                    valueV = (unsigned int)i2c_smbus_read_word_data(&psu_mcu_client, 0x8B);
                    i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case NCIIX_WITH_BMC:
            case NCIIX_WITHOUT_BMC:
                {
                    if ((platformBuildRev > 0x01) && (platformHwRev == 0x03)) /* PVT rev2*/
                    {
                        mutex_lock(&data->lock);
                        if (index)
                            i2c_smbus_write_byte(client, 0x20); /* Turn on PCA9548 channel 5 on I2C-bus1 */
                        else
                            i2c_smbus_write_byte(client, 0x10); /* Turn on PCA9548 channel 4 on I2C-bus1 */
                        valueN = i2c_smbus_read_byte_data(&psu_mcu_client, 0x20);
                        valueV = (unsigned int)i2c_smbus_read_word_data(&psu_mcu_client, 0x8B);
                        i2c_smbus_write_byte(client, 0x00);
                        mutex_unlock(&data->lock);
                    }
                    else
                    {
                        struct i2c_bus0_hardware_monitor_data *data_bus0 = i2c_get_clientdata(&psu_eeprom_client_bus0);

                        mutex_lock(&data_bus0->lock);
                        if (index)
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x04); /* Turn on PCA9548#0 channel 2 on I2C-bus0 */
                        else
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x02); /* Turn on PCA9548#0 channel 1 on I2C-bus0 */
                        valueN = i2c_smbus_read_byte_data(&psu_mcu_client_bus0, 0x20);
                        valueV = (unsigned int)i2c_smbus_read_word_data(&psu_mcu_client_bus0, 0x8B);
                        i2c_smbus_write_byte(&pca9548_client_bus0, 0x00);
                        mutex_unlock(&data_bus0->lock);
                    }
                }
                break;

            default:
                break;
        }

        if (valueN == 0xff)
        {
            valueY = (valueV & 0x07ff);
            if ((valueV & 0x8000)&&(valueY))
            {
                valueV = ((~valueV) >> 11);
                valueE = valueV + 1;
                temp = (unsigned int)(1 << valueE);
                if (temp)
                    count = sprintf(buf, "%d.%04d\n", valueY >> valueE, ((valueY % temp) * 10000) / temp);
            }
            else
            {
                valueN = (((valueV) >> 11) & 0x0F);
                count = sprintf(buf, "%d\n", (valueY*(1<<valueN)));
            }
        }
        else
        {
            if (valueN & 0x10)
            {
                valueN = 0xF0 + (valueN & 0x0F);
                valueN = (~valueN) +1;
                temp = (unsigned int)(1<<valueN);
                if (temp)
                    count = sprintf(buf, "%d.%04d\n", valueV/temp, ((valueV%temp)*10000)/temp);
            }
            else
            {
                count = sprintf(buf, "%d\n", (valueV*(1<<valueN)));
            }
        }
    }
    else
    {
        count = sprintf(buf, "%d\n", 0);
    }
    return count;
}

static ssize_t show_psu_iout(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned short value = 0;
    unsigned short index = 0;
    unsigned int valueY = 0;
    unsigned char valueN = 0;
    ssize_t count = 0;
    unsigned int temp = 0;
    unsigned int psu_present = 0;

    index = (attr->index&0x1);
    if (index&0x01) /* PSU 2 */
    {
        if ((platformPsuABS&0x02)==0x00) /* PSU2 Present */
            psu_present = 1;
    }
    else /* PSU 1 */
    {
        if ((platformPsuABS&0x01)==0x00) /* PSU1 Present */
            psu_present = 1;
    }
    if (psu_present == 1)
    {
        switch(platformModelId)
        {
            case HURACAN_WITH_BMC:
            case HURACAN_WITHOUT_BMC:
            case ASTERION_WITH_BMC:
            case ASTERION_WITHOUT_BMC:
            case HURACAN_A_WITH_BMC:
            case HURACAN_A_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x80);
                    else
                        /* Turn on PCA9548 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x40);
                    value = i2c_smbus_read_word_data(&psu_mcu_client, 0x8C);
                    i2c_smbus_write_byte(client, 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case CABRERAIII_WITH_BMC:
            case CABRERAIII_WITHOUT_BMC:
                break;

            case SESTO_WITH_BMC:
            case SESTO_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548#1 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x80);
                    else
                        /* Turn on PCA9548#1 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x40);
                    value = i2c_smbus_read_word_data(&psu_mcu_client, 0x8C);
                    i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case NCIIX_WITH_BMC:
            case NCIIX_WITHOUT_BMC:
                {
                    if ((platformBuildRev > 0x01) && (platformHwRev == 0x03)) /* PVT rev2*/
                    {
                        mutex_lock(&data->lock);
                        if (index)
                            i2c_smbus_write_byte(client, 0x20); /* Turn on PCA9548 channel 5 on I2C-bus1 */
                        else
                            i2c_smbus_write_byte(client, 0x10); /* Turn on PCA9548 channel 4 on I2C-bus1 */
                        value = i2c_smbus_read_word_data(&psu_mcu_client, 0x8C);
                        i2c_smbus_write_byte(client, 0x00);
                        mutex_unlock(&data->lock);
                    }
                    else
                    {
                        struct i2c_bus0_hardware_monitor_data *data_bus0 = i2c_get_clientdata(&psu_eeprom_client_bus0);

                        mutex_lock(&data_bus0->lock);
                        if (index)
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x04); /* Turn on PCA9548#0 channel 2 on I2C-bus0 */
                        else
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x02); /* Turn on PCA9548#0 channel 1 on I2C-bus0 */
                        value = i2c_smbus_read_word_data(&psu_mcu_client_bus0, 0x8C);
                        i2c_smbus_write_byte(&pca9548_client_bus0, 0x00);
                        mutex_unlock(&data_bus0->lock);
                    }
                }
                break;

            default:
                break;
        }
        valueY = (value & 0x07FF);
        if ((value & 0x8000)&&(valueY))
        {
            valueN = 0xF0 + (((value) >> 11) & 0x0F);
            valueN = (~valueN) +1;
            temp = (unsigned int)(1<<valueN);
            if (temp)
                count = sprintf(buf, "%d.%04d\n", valueY/temp, ((valueY%temp)*10000)/temp);
        }
        else
        {
            valueN = (((value) >> 11) & 0x0F);
            count = sprintf(buf, "%d\n", (valueY*(1<<valueN)));
        }
    }
    else
    {
        count = sprintf(buf, "%d\n", 0);
    }
    return count;
}

static ssize_t show_psu_temp_1(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned short value = 0;
    unsigned short index = 0;
    unsigned int valueY = 0;
    unsigned char valueN = 0;
    ssize_t count = 0;
    unsigned int temp = 0;
    unsigned int psu_present = 0;

    index = (attr->index&0x1);
    if (index&0x01) /* PSU 2 */
    {
        if ((platformPsuABS&0x02)==0x00) /* PSU2 Present */
            psu_present = 1;
    }
    else /* PSU 1 */
    {
        if ((platformPsuABS&0x01)==0x00) /* PSU1 Present */
            psu_present = 1;
    }
    if (psu_present == 1)
    {
        switch(platformModelId)
        {
            case HURACAN_WITH_BMC:
            case HURACAN_WITHOUT_BMC:
            case ASTERION_WITH_BMC:
            case ASTERION_WITHOUT_BMC:
            case HURACAN_A_WITH_BMC:
            case HURACAN_A_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x80);
                    else
                        /* Turn on PCA9548 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x40);
                    value = i2c_smbus_read_word_data(&psu_mcu_client, 0x8D);
                    i2c_smbus_write_byte(client, 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case CABRERAIII_WITH_BMC:
            case CABRERAIII_WITHOUT_BMC:
                break;

            case SESTO_WITH_BMC:
            case SESTO_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548#1 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x80);
                    else
                        /* Turn on PCA9548#1 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x40);
                    value = i2c_smbus_read_word_data(&psu_mcu_client, 0x8D);
                    i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case NCIIX_WITH_BMC:
            case NCIIX_WITHOUT_BMC:
                {
                    if ((platformBuildRev > 0x01) && (platformHwRev == 0x03)) /* PVT rev2*/
                    {
                        mutex_lock(&data->lock);
                        if (index)
                            i2c_smbus_write_byte(client, 0x20); /* Turn on PCA9548 channel 5 on I2C-bus1 */
                        else
                            i2c_smbus_write_byte(client, 0x10); /* Turn on PCA9548 channel 4 on I2C-bus1 */
                        value = i2c_smbus_read_word_data(&psu_mcu_client, 0x8D);
                        i2c_smbus_write_byte(client, 0x00);
                        mutex_unlock(&data->lock);
                    }
                    else
                    {
                        struct i2c_bus0_hardware_monitor_data *data_bus0 = i2c_get_clientdata(&psu_eeprom_client_bus0);

                        mutex_lock(&data_bus0->lock);
                        if (index)
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x04); /* Turn on PCA9548#0 channel 2 on I2C-bus0 */
                        else
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x02); /* Turn on PCA9548#0 channel 1 on I2C-bus0 */
                        value = i2c_smbus_read_word_data(&psu_mcu_client_bus0, 0x8D);
                        i2c_smbus_write_byte(&pca9548_client_bus0, 0x00);
                        mutex_unlock(&data_bus0->lock);
                    }
                }
                break;

            default:
                break;
        }
        valueY = (value & 0x07FF);
        if ((value & 0x8000)&&(valueY))
        {
            valueN = 0xF0 + (((value) >> 11) & 0x0F);
            valueN = (~valueN) +1;
            temp = (unsigned int)(1<<valueN);
            if (temp)
                count = sprintf(buf, "%d.%04d\n", valueY/temp, ((valueY%temp)*10000)/temp);
        }
        else
        {
            valueN = (((value) >> 11) & 0x0F);
            count = sprintf(buf, "%d\n", (valueY*(1<<valueN)));
        }
    }
    else
    {
        count = sprintf(buf, "%d\n", 0);
    }
    return count;
}

static ssize_t show_psu_temp_2(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned short value = 0;
    unsigned short index = 0;
    unsigned int valueY = 0;
    unsigned char valueN = 0;
    ssize_t count = 0;
    unsigned int temp = 0;
    unsigned int psu_present = 0;

    index = (attr->index&0x1);
    if (index&0x01) /* PSU 2 */
    {
        if ((platformPsuABS&0x02)==0x00) /* PSU2 Present */
            psu_present = 1;
    }
    else /* PSU 1 */
    {
        if ((platformPsuABS&0x01)==0x00) /* PSU1 Present */
            psu_present = 1;
    }
    if (psu_present == 1)
    {
        switch(platformModelId)
        {
            case HURACAN_WITH_BMC:
            case HURACAN_WITHOUT_BMC:
            case HURACAN_A_WITH_BMC:
            case HURACAN_A_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x80);
                    else
                        /* Turn on PCA9548 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x40);
                    value = i2c_smbus_read_word_data(&psu_mcu_client, 0x8E);
                    i2c_smbus_write_byte(client, 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case CABRERAIII_WITH_BMC:
            case CABRERAIII_WITHOUT_BMC:
                break;

            case SESTO_WITH_BMC:
            case SESTO_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548#1 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x80);
                    else
                        /* Turn on PCA9548#1 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x40);
                    value = i2c_smbus_read_word_data(&psu_mcu_client, 0x8E);
                    i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case NCIIX_WITH_BMC:
            case NCIIX_WITHOUT_BMC:
                {
                    if ((platformBuildRev > 0x01) && (platformHwRev == 0x03)) /* PVT rev2*/
                    {
                        mutex_lock(&data->lock);
                        if (index)
                            i2c_smbus_write_byte(client, 0x20); /* Turn on PCA9548 channel 5 on I2C-bus1 */
                        else
                            i2c_smbus_write_byte(client, 0x10); /* Turn on PCA9548 channel 4 on I2C-bus1 */
                        value = i2c_smbus_read_word_data(&psu_mcu_client, 0x8E);
                        i2c_smbus_write_byte(client, 0x00);
                        mutex_unlock(&data->lock);
                    }
                    else
                    {
                        struct i2c_bus0_hardware_monitor_data *data_bus0 = i2c_get_clientdata(&psu_eeprom_client_bus0);

                        mutex_lock(&data_bus0->lock);
                        if (index)
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x04); /* Turn on PCA9548#0 channel 2 on I2C-bus0 */
                        else
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x02); /* Turn on PCA9548#0 channel 1 on I2C-bus0 */
                        value = i2c_smbus_read_word_data(&psu_mcu_client_bus0, 0x8E);
                        i2c_smbus_write_byte(&pca9548_client_bus0, 0x00);
                        mutex_unlock(&data_bus0->lock);
                    }
                }
                break;

            case ASTERION_WITH_BMC:
            case ASTERION_WITHOUT_BMC:
                break;

            default:
                break;
        }
        valueY = (value & 0x07FF);
        if ((value & 0x8000)&&(valueY))
        {
            valueN = 0xF0 + (((value) >> 11) & 0x0F);
            valueN = (~valueN) +1;
            temp = (unsigned int)(1<<valueN);
            if (temp)
                count = sprintf(buf, "%d.%04d\n", valueY/temp, ((valueY%temp)*10000)/temp);
        }
        else
        {
            valueN = (((value) >> 11) & 0x0F);
            count = sprintf(buf, "%d\n", (valueY*(1<<valueN)));
        }
    }
    else
    {
        count = sprintf(buf, "%d\n", 0);
    }
    return count;
}

static ssize_t show_psu_fan_speed(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned short value = 0;
    unsigned short index = 0;
    unsigned int temp = 0;
    unsigned int psu_present = 0;

    index = (attr->index&0x1);
    if (index&0x01) /* PSU 2 */
    {
        if ((platformPsuABS&0x02)==0x00) /* PSU2 Present */
            psu_present = 1;
    }
    else /* PSU 1 */
    {
        if ((platformPsuABS&0x01)==0x00) /* PSU1 Present */
            psu_present = 1;
    }
    if (psu_present == 1)
    {
        switch(platformModelId)
        {
            case HURACAN_WITH_BMC:
            case HURACAN_WITHOUT_BMC:
            case ASTERION_WITH_BMC:
            case ASTERION_WITHOUT_BMC:
            case HURACAN_A_WITH_BMC:
            case HURACAN_A_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x80);
                    else
                        /* Turn on PCA9548 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x40);
                    value = i2c_smbus_read_word_data(&psu_mcu_client, 0x90);
                    i2c_smbus_write_byte(client, 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case CABRERAIII_WITH_BMC:
            case CABRERAIII_WITHOUT_BMC:
                break;

            case SESTO_WITH_BMC:
            case SESTO_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548#1 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x80);
                    else
                        /* Turn on PCA9548#1 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x40);
                    value = i2c_smbus_read_word_data(&psu_mcu_client, 0x90);
                    i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case NCIIX_WITH_BMC:
            case NCIIX_WITHOUT_BMC:
                {
                    if ((platformBuildRev > 0x01) && (platformHwRev == 0x03)) /* PVT rev2*/
                    {
                        mutex_lock(&data->lock);
                        if (index)
                            i2c_smbus_write_byte(client, 0x20); /* Turn on PCA9548 channel 5 on I2C-bus1 */
                        else
                            i2c_smbus_write_byte(client, 0x10); /* Turn on PCA9548 channel 4 on I2C-bus1 */
                        value = i2c_smbus_read_word_data(&psu_mcu_client, 0x90);
                        i2c_smbus_write_byte(client, 0x00);
                        mutex_unlock(&data->lock);
                    }
                    else
                    {
                        struct i2c_bus0_hardware_monitor_data *data_bus0 = i2c_get_clientdata(&psu_eeprom_client_bus0);

                        mutex_lock(&data_bus0->lock);
                        if (index)
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x04); /* Turn on PCA9548#0 channel 2 on I2C-bus0 */
                        else
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x02); /* Turn on PCA9548#0 channel 1 on I2C-bus0 */
                        value = i2c_smbus_read_word_data(&psu_mcu_client_bus0, 0x90);
                        i2c_smbus_write_byte(&pca9548_client_bus0, 0x00);
                        mutex_unlock(&data_bus0->lock);
                    }
                }
                break;

            default:
                break;
        }
        temp = (unsigned int)value;
        temp = (temp & 0x07FF) * (1 << ((temp >> 11) & 0x1F));
    }
    return sprintf(buf, "%d\n", temp);
}

static ssize_t show_psu_pout(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned short value = 0;
    unsigned short index = 0;
    unsigned int valueY = 0;
    unsigned char valueN = 0;
    ssize_t count = 0;
    unsigned int temp = 0;
    unsigned int psu_present = 0;

    index = (attr->index&0x1);
    if (index&0x01) /* PSU 2 */
    {
        if ((platformPsuABS&0x02)==0x00) /* PSU2 Present */
            psu_present = 1;
    }
    else /* PSU 1 */
    {
        if ((platformPsuABS&0x01)==0x00) /* PSU1 Present */
            psu_present = 1;
    }
    if (psu_present == 1)
    {
        switch(platformModelId)
        {
            case HURACAN_WITH_BMC:
            case HURACAN_WITHOUT_BMC:
            case ASTERION_WITH_BMC:
            case ASTERION_WITHOUT_BMC:
            case HURACAN_A_WITH_BMC:
            case HURACAN_A_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x80);
                    else
                        /* Turn on PCA9548 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x40);
                    value = i2c_smbus_read_word_data(&psu_mcu_client, 0x96);
                    i2c_smbus_write_byte(client, 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case CABRERAIII_WITH_BMC:
            case CABRERAIII_WITHOUT_BMC:
                break;

            case SESTO_WITH_BMC:
            case SESTO_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548#1 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x80);
                    else
                        /* Turn on PCA9548#1 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x40);
                    value = i2c_smbus_read_word_data(&psu_mcu_client, 0x96);
                    i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case NCIIX_WITH_BMC:
            case NCIIX_WITHOUT_BMC:
                {
                    if ((platformBuildRev > 0x01) && (platformHwRev == 0x03)) /* PVT rev2*/
                    {
                        mutex_lock(&data->lock);
                        if (index)
                            i2c_smbus_write_byte(client, 0x20); /* Turn on PCA9548 channel 5 on I2C-bus1 */
                        else
                            i2c_smbus_write_byte(client, 0x10); /* Turn on PCA9548 channel 4 on I2C-bus1 */
                        value = i2c_smbus_read_word_data(&psu_mcu_client, 0x96);
                        i2c_smbus_write_byte(client, 0x00);
                        mutex_unlock(&data->lock);
                    }
                    else
                    {
                        struct i2c_bus0_hardware_monitor_data *data_bus0 = i2c_get_clientdata(&psu_eeprom_client_bus0);

                        mutex_lock(&data_bus0->lock);
                        if (index)
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x04); /* Turn on PCA9548#0 channel 2 on I2C-bus0 */
                        else
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x02); /* Turn on PCA9548#0 channel 1 on I2C-bus0 */
                        value = i2c_smbus_read_word_data(&psu_mcu_client_bus0, 0x96);
                        i2c_smbus_write_byte(&pca9548_client_bus0, 0x00);
                        mutex_unlock(&data_bus0->lock);
                    }
                }
                break;

            default:
                break;
        }
        valueY = (value & 0x07FF);
        if ((value & 0x8000)&&(valueY))
        {
            valueN = 0xF0 + (((value) >> 11) & 0x0F);
            valueN = (~valueN) +1;
            temp = (unsigned int)(1<<valueN);
            if (temp)
                count = sprintf(buf, "%d.%04d\n", valueY/temp, ((valueY%temp)*10000)/temp);
        }
        else
        {
            valueN = (((value) >> 11) & 0x0F);
            count = sprintf(buf, "%d\n", (valueY*(1<<valueN)));
        }
    }
    else
    {
        count = sprintf(buf, "%d\n", 0);
    }
    return count;
}

static ssize_t show_psu_pin(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned short value = 0;
    unsigned short index = 0;
    unsigned int valueY = 0;
    unsigned char valueN = 0;
    ssize_t count = 0;
    unsigned int temp = 0;
    unsigned int psu_present = 0;

    index = (attr->index&0x1);
    if (index&0x01) /* PSU 2 */
    {
        if ((platformPsuABS&0x02)==0x00) /* PSU2 Present */
            psu_present = 1;
    }
    else /* PSU 1 */
    {
        if ((platformPsuABS&0x01)==0x00) /* PSU1 Present */
            psu_present = 1;
    }
    if (psu_present == 1)
    {
        switch(platformModelId)
        {
            case HURACAN_WITH_BMC:
            case HURACAN_WITHOUT_BMC:
            case HURACAN_A_WITH_BMC:
            case HURACAN_A_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x80);
                    else
                        /* Turn on PCA9548 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x40);
                    value = i2c_smbus_read_word_data(&psu_mcu_client, 0x97);
                    i2c_smbus_write_byte(client, 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case CABRERAIII_WITH_BMC:
            case CABRERAIII_WITHOUT_BMC:
                break;

            case SESTO_WITH_BMC:
            case SESTO_WITHOUT_BMC:
                {
                    mutex_lock(&data->lock);
                    if (index)
                        /* Turn on PCA9548#1 channel 7 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x80);
                    else
                        /* Turn on PCA9548#1 channel 6 on I2C-bus1 */
                        i2c_smbus_write_byte(&(pca9548_client[1]), 0x40);
                    value = i2c_smbus_read_word_data(&psu_mcu_client, 0x97);
                    i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
                    mutex_unlock(&data->lock);
                }
                break;

            case NCIIX_WITH_BMC:
            case NCIIX_WITHOUT_BMC:
                {
                    if ((platformBuildRev > 0x01) && (platformHwRev == 0x03)) /* PVT rev2*/
                    {
                        mutex_lock(&data->lock);
                        if (index)
                            i2c_smbus_write_byte(client, 0x20); /* Turn on PCA9548 channel 5 on I2C-bus1 */
                        else
                            i2c_smbus_write_byte(client, 0x10); /* Turn on PCA9548 channel 4 on I2C-bus1 */
                        value = i2c_smbus_read_word_data(&psu_mcu_client, 0x97);
                        i2c_smbus_write_byte(client, 0x00);
                        mutex_unlock(&data->lock);
                    }
                    else
                    {
                        struct i2c_bus0_hardware_monitor_data *data_bus0 = i2c_get_clientdata(&psu_eeprom_client_bus0);

                        mutex_lock(&data_bus0->lock);
                        if (index)
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x04); /* Turn on PCA9548#0 channel 2 on I2C-bus0 */
                        else
                            i2c_smbus_write_byte(&pca9548_client_bus0, 0x02); /* Turn on PCA9548#0 channel 1 on I2C-bus0 */
                        value = i2c_smbus_read_word_data(&psu_mcu_client_bus0, 0x97);
                        i2c_smbus_write_byte(&pca9548_client_bus0, 0x00);
                        mutex_unlock(&data_bus0->lock);
                    }
                }
                break;

            case ASTERION_WITH_BMC:
            case ASTERION_WITHOUT_BMC:
                break;

            default:
                break;
        }
        valueY = (value & 0x07FF);
        if ((value & 0x8000)&&(valueY))
        {
            valueN = 0xF0 + (((value) >> 11) & 0x0F);
            valueN = (~valueN) +1;
            temp = (unsigned int)(1<<valueN);
            if (temp)
                count = sprintf(buf, "%d.%04d\n", valueY/temp, ((valueY%temp)*10000)/temp);
        }
        else
        {
            valueN = (((value) >> 11) & 0x0F);
            count = sprintf(buf, "%d\n", (valueY*(1<<valueN)));
        }
    }
    else
    {
        count = sprintf(buf, "%d\n", 0);
    }
    return count;
}

static ssize_t set_psu_power_off(struct device *dev, struct device_attribute *devattr, const char *buf,
          size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    long temp;

    if (kstrtol(buf, 10, &temp))
        return -EINVAL;

    temp = clamp_val(temp, 0, 1);
    if (temp == 0)
        return count;

    switch(platformModelId)
    {
        case HURACAN_WITH_BMC:
        case HURACAN_WITHOUT_BMC:
        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
        case HURACAN_A_WITH_BMC:
        case HURACAN_A_WITHOUT_BMC:
            {
                /*
                    Setting the ON_OFF_CONFIG Command (02h) to type 9 (SW : turn-on/off by operation command).
                    I2C Command:  B0      02      19    8F
                                  address command data  PEC(Packet Error Check)
                */
                unsigned short cmd_data_1 = 0x8F19;
                /*
                    Setting the Operation Command (01h) to turn-off power immediately.
                    I2C Command:  B0      01      00    FF
                                  address command data  PEC(Packet Error Check)
                */
                unsigned short cmd_data_2 = 0xFF00;

                mutex_lock(&data->lock);
                if ((platformPsuABS&0x01)==0x00) /* PSU1 Present */
                {
                    /* Turn on PCA9548#1 channel 6 on I2C-bus1 */
                    i2c_smbus_write_byte(client, 0x40);
                    i2c_smbus_write_word_data(&psu_mcu_client, 0x02, cmd_data_1);
                    i2c_smbus_write_word_data(&psu_mcu_client, 0x01, cmd_data_2);
                }
                if ((platformPsuABS&0x02)==0x00) /* PSU2 Present */
                {
                    /* Turn on PCA9548#1 channel 7 on I2C-bus1 */
                    i2c_smbus_write_byte(client, 0x80);
                    i2c_smbus_write_word_data(&psu_mcu_client, 0x02, cmd_data_1);
                    i2c_smbus_write_word_data(&psu_mcu_client, 0x01, cmd_data_2);
                }
                i2c_smbus_write_byte(client, 0x00);
                mutex_unlock(&data->lock);
            }
            break;

        case SESTO_WITH_BMC:
        case SESTO_WITHOUT_BMC:
            {
                unsigned short cmd_data_1 = 0x8F19;
                unsigned short cmd_data_2 = 0xFF00;

                mutex_lock(&data->lock);
                if ((platformPsuABS&0x01)==0x00) /* PSU1 Present */
                {
                    /* Turn on PCA9548#1 channel 6 on I2C-bus1 */
                    i2c_smbus_write_byte(&(pca9548_client[1]), 0x40);
                    i2c_smbus_write_word_data(&psu_mcu_client, 0x02, cmd_data_1);
                    i2c_smbus_write_word_data(&psu_mcu_client, 0x01, cmd_data_2);
                }
                if ((platformPsuABS&0x02)==0x00) /* PSU2 Present */
                {
                    /* Turn on PCA9548#1 channel 7 on I2C-bus1 */
                    i2c_smbus_write_byte(&(pca9548_client[1]), 0x80);
                    i2c_smbus_write_word_data(&psu_mcu_client, 0x02, cmd_data_1);
                    i2c_smbus_write_word_data(&psu_mcu_client, 0x01, cmd_data_2);
                }
                i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
                mutex_unlock(&data->lock);
            }
            break;

        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
            {
                /*
                    Setting the ON_OFF_CONFIG Command (02h) to type 9 (SW : turn-on/off by operation command).
                    I2C Command:  B2      02      19    59
                                  address command data  PEC(Packet Error Check)
                */
                unsigned short cmd_data_1 = 0x5919;
                /*
                    Setting the Operation Command (01h) to turn-off power immediately.
                    I2C Command:  B2      01      00    29
                                  address command data  PEC(Packet Error Check)
                */
                unsigned short cmd_data_2 = 0x2900;

                if ((platformBuildRev > 0x01) && (platformHwRev == 0x03)) /* PVT rev2*/
                {
                    mutex_lock(&data->lock);
                    if ((platformPsuABS & 0x01) == 0x00) /* PSU1 Present */
                    {
                        /* Turn on PCA9548#0 channel 4 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x10);
                        i2c_smbus_write_word_data(&psu_mcu_client, 0x02, cmd_data_1);
                        i2c_smbus_write_word_data(&psu_mcu_client, 0x01, cmd_data_2);
                    }
                    if ((platformPsuABS & 0x02) == 0x00) /* PSU2 Present */
                    {
                        /* Turn on PCA9548#0 channel 5 on I2C-bus1 */
                        i2c_smbus_write_byte(client, 0x20);
                        i2c_smbus_write_word_data(&psu_mcu_client, 0x02, cmd_data_1);
                        i2c_smbus_write_word_data(&psu_mcu_client, 0x01, cmd_data_2);
                    }
                    i2c_smbus_write_byte(client, 0x00);
                    mutex_unlock(&data->lock);
                }
                else
                {
                    struct i2c_bus0_hardware_monitor_data *data_bus0 = i2c_get_clientdata(&psu_eeprom_client_bus0);

                    mutex_lock(&data_bus0->lock);
                    if ((platformPsuABS & 0x01) == 0x00) /* PSU1 Present */
                    {
                        /* Turn on PCA9548#0 channel 1 on I2C-bus0 */
                        i2c_smbus_write_byte(&pca9548_client_bus0, 0x02);
                        i2c_smbus_write_word_data(&psu_mcu_client_bus0, 0x02, cmd_data_1);
                        i2c_smbus_write_word_data(&psu_mcu_client_bus0, 0x01, cmd_data_2);
                    }
                    if ((platformPsuABS & 0x02) == 0x00) /* PSU2 Present */
                    {
                        /* Turn on PCA9548#0 channel 2 on I2C-bus0 */
                        i2c_smbus_write_byte(&pca9548_client_bus0, 0x04);
                        i2c_smbus_write_word_data(&psu_mcu_client_bus0, 0x02, cmd_data_1);
                        i2c_smbus_write_word_data(&psu_mcu_client_bus0, 0x01, cmd_data_2);
                    }
                    i2c_smbus_write_byte(&pca9548_client_bus0, 0x00);
                    mutex_unlock(&data_bus0->lock);
                }
            }
            break;

        default:
            break;
    }

    return count;
}


static ssize_t set_system_led(struct device *dev, struct device_attribute *devattr, const char *buf,
          size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    long temp;

    if (kstrtol(buf, 10, &temp))
        return -EINVAL;

    temp = clamp_val(temp, 0, 2);

    mutex_lock(&data->lock);
    data->systemLedStatus = temp;
    mutex_unlock(&data->lock);

    return count;
}

static ssize_t show_fan_led(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned int temp;
    unsigned int frontLedStatus;

    mutex_lock(&data->lock);
    frontLedStatus = (unsigned int)data->frontLedStatus;
    switch(platformModelId)
    {
        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
        {
            if (!(frontLedStatus & 0x0010))
                temp = 2; /* Normal */
            else if (!(frontLedStatus & 0x0020))
                temp = 1; /* Critical */
            else
                temp = 0; /* Booting */
        }
            break;
        default:
        {
            if (!(frontLedStatus & 0x0008))
                temp = 2; /* Normal */
            else if (!(frontLedStatus & 0x0004))
                temp = 1; /* Critical */
            else
                temp = 0; /* Booting */
        }
            break;
    }
    mutex_unlock(&data->lock);

    return sprintf(buf, "%d\n", temp);
}

static ssize_t show_psu_led(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned int temp;
    unsigned int frontLedStatus;

    mutex_lock(&data->lock);
    frontLedStatus = (unsigned int)data->frontLedStatus;
    switch(platformModelId)
    {
        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
        {
            if (attr->index == 0)
            {
                if (!(frontLedStatus & 0x0001))
                    temp = 2; /* Normal */
                else if (!(frontLedStatus & 0x0002))
                    temp = 1; /* Critical */
                else
                    temp = 0; /* Booting */
            }
            else
            {
                if (!(frontLedStatus & 0x0004))
                    temp = 2; /* Normal */
                else if (!(frontLedStatus & 0x0008))
                    temp = 1; /* Critical */
                else
                    temp = 0; /* Booting */
            }
        }
            break;
        default:
        {
            if (attr->index == 0)
            {
                if (!(frontLedStatus & 0x0002))
                    temp = 2; /* Normal */
                else if (!(frontLedStatus & 0x0001))
                    temp = 1; /* Critical */
                else
                    temp = 0; /* Booting */
            }
            else
            {
                if (!(frontLedStatus & 0x0020))
                    temp = 2; /* Normal */
                else if (!(frontLedStatus & 0x0010))
                    temp = 1; /* Critical */
                else
                    temp = 0; /* Booting */
            }
        }
            break;
    }

    mutex_unlock(&data->lock);

    return sprintf(buf, "%d\n", temp);
}

static ssize_t show_port_tx_disable(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    int rc = 0;

    switch(platformModelId)
    {
        case SESTO_WITH_BMC:
        case SESTO_WITHOUT_BMC:
        {
            struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(&(pca9535pwr_client[0]));
            unsigned short index=0, bit=0;

            index = (attr->index/16);
            bit = (attr->index%16);
            mutex_lock(&data->lock);
            rc = (PCA9553_TEST_BIT(data->sfpPortTxDisable[index], bit)?1:0);
            mutex_unlock(&data->lock);
        }
            break;

        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
        {
            rc = (SFPPortTxDisable[attr->index]==1);
        }
            break;

        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
        {
            struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(&(pca9535pwr_client[0]));
            unsigned short bit = 0;

            bit = (attr->index % 8);
            mutex_lock(&data->lock);
            rc = (PCA9553_TEST_BIT(data->sfpPortTxDisableAst[attr->index / 8], bit) ? 1 : 0);
            mutex_unlock(&data->lock);
        }
            break;

        default:
            break;
    }


    return sprintf(buf, "%d\n", rc);
}

static ssize_t set_port_tx_disable(struct device *dev, struct device_attribute *devattr, const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client, pca9548Client;
    struct i2c_bus1_hardware_monitor_data *data;
    long temp;

    if (kstrtol(buf, 10, &temp))
        return -EINVAL;
    temp = clamp_val(temp, 0, 1);

    pca9548Client = pca9548_client[1];
    client = &pca9548Client;
    data = i2c_get_clientdata(client);
    switch(platformModelId)
    {
        case SESTO_WITH_BMC:
        case SESTO_WITHOUT_BMC:
        {
            unsigned short index=0, bit=0;

            index = (attr->index/16);
            bit = (attr->index%16);

            mutex_lock(&data->lock);
            if (temp==1)
                PCA9553_SET_BIT(data->sfpPortTxDisable[index], bit);
            else
                PCA9553_CLEAR_BIT(data->sfpPortTxDisable[index], bit);
            i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH05));
            i2c_device_word_write(&(pca9535pwr_client[index]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->sfpPortTxDisable[index]);
            i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
            mutex_unlock(&data->lock);
        }
            break;

        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
        {
            unsigned short value = 0;

            pca9548Client.addr = 0x70;
            SFPPortTxDisable[attr->index] = (temp&0x1);
            if ((attr->index/8) == 5) /* SFP+ 40~47 */
            {
                mutex_lock(&data->lock);
                i2c_smbus_write_byte(client, sfpPortData_78F[attr->index].portMaskIOsForPCA9548_0);
                value = i2c_smbus_read_word_data(&(pca9535pwr_client[sfpPortData_78F[attr->index].i2cAddrForPCA9535]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0);
                if (temp==1)
                    PCA9553_SET_BIT(value, sfpPortData_78F[attr->index].portMaskBitForTxEnPin);
                else
                    PCA9553_CLEAR_BIT(value, sfpPortData_78F[attr->index].portMaskBitForTxEnPin);
                i2c_device_word_write(&(pca9535pwr_client[sfpPortData_78F[attr->index].i2cAddrForPCA9535]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, value);
                i2c_smbus_write_byte(client, 0x00);
                mutex_unlock(&data->lock);
            }
            else  /* SFP+ 0~39 */
            {
                struct i2c_bus0_hardware_monitor_data *data_bus0 = i2c_get_clientdata(&pca9548_client_bus0);

                mutex_lock(&data_bus0->lock);
                i2c_smbus_write_byte(&pca9548_client_bus0, sfpPortData_78F[attr->index].portMaskIOsForPCA9548_0);
                value = i2c_smbus_read_word_data(&(pca9535_client_bus0[sfpPortData_78F[attr->index].i2cAddrForPCA9535]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0);
                if (temp==1)
                    PCA9553_SET_BIT(value, sfpPortData_78F[attr->index].portMaskBitForTxEnPin);
                else
                    PCA9553_CLEAR_BIT(value, sfpPortData_78F[attr->index].portMaskBitForTxEnPin);
                i2c_device_word_write(&(pca9535_client_bus0[sfpPortData_78F[attr->index].i2cAddrForPCA9535]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, value);
                i2c_smbus_write_byte(&pca9548_client_bus0, 0x00);
                mutex_unlock(&data_bus0->lock);
            }
        }
            break;

        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
        {
            unsigned short index = 0, bit = 0;

            pca9548Client.addr = 0x72;
            index = (attr->index / 8);
            bit = (attr->index % 8);

            mutex_lock(&data->lock);
            if (temp == 1)
                PCA9553_SET_BIT(data->sfpPortTxDisableAst[index], bit);
            else
                PCA9553_CLEAR_BIT(data->sfpPortTxDisableAst[index], bit);

            if (index < 3)
            {
                i2c_smbus_write_byte(client, (1 << PCA9548_CH00));
                i2c_smbus_write_byte_data(&(cpld_client_bus1), (0x40 + index), data->sfpPortTxDisableAst[index]);
            }
            else
            {
                i2c_smbus_write_byte(client, (1 << PCA9548_CH01));
                i2c_smbus_write_byte_data(&(cpld_client_bus1), (0x40 + (index - 3)), data->sfpPortTxDisableAst[index]);
            }

            i2c_smbus_write_byte(client, 0x00);
            mutex_unlock(&data->lock);
        }
            break;

        default:
            break;
    }

    return count;
}

static ssize_t show_port_rate_select(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    int rc = 0;

    switch(platformModelId)
    {
        case SESTO_WITH_BMC:
        case SESTO_WITHOUT_BMC:
        {
            struct i2c_client *client = to_i2c_client(dev);
            struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
            unsigned short index=0, bit=0;

            index = (attr->index/16);
            bit = (attr->index%16);
            mutex_lock(&data->lock);
            rc = (PCA9553_TEST_BIT(data->sfpPortRateSelect[index], bit)?1:0);
            mutex_unlock(&data->lock);
        }
            break;

        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
        {
            struct i2c_client *client = to_i2c_client(dev);
            struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
            unsigned short index = 0;

            index = (attr->index % 4);
            mutex_lock(&data->lock);
            rc = (PCA9553_TEST_BIT(data->sfpPortRateSelectAst[attr->index / 4], (index * 2)) ? 1 : 0);
            mutex_unlock(&data->lock);
        }
            break;

        default:
            break;
    }


    return sprintf(buf, "%d\n", rc);
}

static ssize_t set_port_rate_select(struct device *dev, struct device_attribute *devattr, const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    struct i2c_client *client = to_i2c_client(dev);
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    long temp;

    if (kstrtol(buf, 10, &temp))
        return -EINVAL;
    temp = clamp_val(temp, 0, 1);

    switch(platformModelId)
    {
        case SESTO_WITH_BMC:
        case SESTO_WITHOUT_BMC:
        {
            unsigned short index=0, bit=0;

            index = (attr->index/16);
            bit = (attr->index%16);

            mutex_lock(&data->lock);
            if (temp==1)
                PCA9553_SET_BIT(data->sfpPortRateSelect[index], bit);
            else
                PCA9553_CLEAR_BIT(data->sfpPortRateSelect[index], bit);
            i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH03));
            switch(index)
            {
                case 0:
                    i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->sfpPortRateSelect[0]);
                    break;

                case 1:
                    i2c_device_word_write(&(pca9535pwr_client[1]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->sfpPortRateSelect[1]);
                    break;

                case 2:
                    i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->sfpPortRateSelect[2]);
                    break;

                default:
                    break;
            }
            i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH04));
            switch(index)
            {
                case 0:
                    i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->sfpPortRateSelect[0]);
                    break;

                case 1:
                    i2c_device_word_write(&(pca9535pwr_client[1]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->sfpPortRateSelect[1]);
                    break;

                case 2:
                    i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->sfpPortRateSelect[2]);
                    break;

                default:
                    break;
            }
            i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
            mutex_unlock(&data->lock);
        }
            break;

        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
        {
            unsigned short index = 0, bit = 0;

            index = (attr->index / 4);
            bit = (attr->index % 4);

            mutex_lock(&data->lock);
            if (attr->index >= 0 && attr->index < 24)
            {
                /* Turn on PCA9548#0 channel 0 on I2C-bus1 - RX_RS, TX_RS */
                i2c_smbus_write_byte(client, (1 << PCA9548_CH00));

                if (temp == 1)
                {
                    PCA9553_SET_BIT(data->sfpPortRateSelectAst[index], (bit * 2));
                    PCA9553_SET_BIT(data->sfpPortRateSelectAst[index], (bit * 2 + 1));
                }
                else
                {
                    PCA9553_CLEAR_BIT(data->sfpPortRateSelectAst[index], (bit * 2));
                    PCA9553_CLEAR_BIT(data->sfpPortRateSelectAst[index], (bit * 2 + 1));
                }

                switch(index)
                {
                    case 0:
                        i2c_smbus_write_byte_data(&(cpld_client_bus1), 0x8, data->sfpPortRateSelectAst[index]);
                        break;
                    case 1:
                        i2c_smbus_write_byte_data(&(cpld_client_bus1), 0x9, data->sfpPortRateSelectAst[index]);
                        break;
                    case 2:
                        i2c_smbus_write_byte_data(&(cpld_client_bus1), 0x10, data->sfpPortRateSelectAst[index]);
                        break;
                    case 3:
                        i2c_smbus_write_byte_data(&(cpld_client_bus1), 0x11, data->sfpPortRateSelectAst[index]);
                        break;
                    case 4:
                        i2c_smbus_write_byte_data(&(cpld_client_bus1), 0x12, data->sfpPortRateSelectAst[index]);
                        break;
                    case 5:
                        i2c_smbus_write_byte_data(&(cpld_client_bus1), 0x13, data->sfpPortRateSelectAst[index]);
                        break;
                }
            }
            else
            {
                /* Turn on PCA9548#0 channel 1 on I2C-bus1 - RX_RS, TX_RS */
                i2c_smbus_write_byte(client, (1 << PCA9548_CH01));

                if (temp == 1)
                {
                    PCA9553_SET_BIT(data->sfpPortRateSelectAst[index], (bit * 2));
                    PCA9553_SET_BIT(data->sfpPortRateSelectAst[index], (bit * 2 + 1));
                }
                else
                {
                    PCA9553_CLEAR_BIT(data->sfpPortRateSelectAst[index], (bit * 2));
                    PCA9553_CLEAR_BIT(data->sfpPortRateSelectAst[index], (bit * 2 + 1));
                }

                switch(index)
                {
                    case 6:
                        i2c_smbus_write_byte_data(&(cpld_client_bus1), 0x8, data->sfpPortRateSelectAst[index]);
                        break;
                    case 7:
                        i2c_smbus_write_byte_data(&(cpld_client_bus1), 0x9, data->sfpPortRateSelectAst[index]);
                        break;
                    case 8:
                        i2c_smbus_write_byte_data(&(cpld_client_bus1), 0x10, data->sfpPortRateSelectAst[index]);
                        break;
                    case 9:
                        i2c_smbus_write_byte_data(&(cpld_client_bus1), 0x11, data->sfpPortRateSelectAst[index]);
                        break;
                    case 10:
                        i2c_smbus_write_byte_data(&(cpld_client_bus1), 0x12, data->sfpPortRateSelectAst[index]);
                        break;
                    case 11:
                        i2c_smbus_write_byte_data(&(cpld_client_bus1), 0x13, data->sfpPortRateSelectAst[index]);
                        break;
                }
            }

            i2c_smbus_write_byte(client, 0x00);
            mutex_unlock(&data->lock);
        }
            break;

        default:
            break;
    }

    return count;
}

static ssize_t show_port_tx_fault(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    int rc = 0;

    switch(platformModelId)
    {
        case SESTO_WITH_BMC:
        case SESTO_WITHOUT_BMC:
        {
            struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(&(pca9535pwr_client[0]));
            unsigned short index = 0, bit = 0;

            index = (attr->index / 16);
            bit = (attr->index % 16);
            mutex_lock(&data->lock);
            rc = (PCA9553_TEST_BIT(data->sfpPortTxFaultStatus[index], bit) ? 1 : 0);
            mutex_unlock(&data->lock);
        }
            break;

        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
        {
            rc = (SFPPortTxFaultStatus[attr->index] ? 0 : 1);
        }
            break;

        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
        {
            struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(&(pca9535pwr_client[0]));
            unsigned char qsfpPortRxLos = 0, index = 0, bit = 0;

            index = (attr->index / 2);
            bit = ((attr->index & 0x01) ? 7 : 3);
            mutex_lock(&data->lock);
            qsfpPortRxLos = data->sfpPortAbsRxLosStatus[index];
            mutex_unlock(&data->lock);
            rc = (PCA9553_TEST_BIT(qsfpPortRxLos, bit) ? 1 : 0);
        }
            break;

        default:
            break;
    }


    return sprintf(buf, "%d\n", rc);
}

static DEVICE_ATTR(eeprom, S_IRUGO, show_eeprom, NULL);
static DEVICE_ATTR(system_led, S_IWUSR, NULL, set_system_led);
static DEVICE_ATTR(fan_led, S_IRUGO, show_fan_led, NULL);
static SENSOR_DEVICE_ATTR(psu1_led, S_IRUGO, show_psu_led, NULL, 0);
static SENSOR_DEVICE_ATTR(psu2_led, S_IRUGO, show_psu_led, NULL, 1);

static SENSOR_DEVICE_ATTR(port_1_data_a0, S_IRUGO, show_port_data_a0, NULL, 0);
static SENSOR_DEVICE_ATTR(port_2_data_a0, S_IRUGO, show_port_data_a0, NULL, 1);
static SENSOR_DEVICE_ATTR(port_3_data_a0, S_IRUGO, show_port_data_a0, NULL, 2);
static SENSOR_DEVICE_ATTR(port_4_data_a0, S_IRUGO, show_port_data_a0, NULL, 3);
static SENSOR_DEVICE_ATTR(port_5_data_a0, S_IRUGO, show_port_data_a0, NULL, 4);
static SENSOR_DEVICE_ATTR(port_6_data_a0, S_IRUGO, show_port_data_a0, NULL, 5);
static SENSOR_DEVICE_ATTR(port_7_data_a0, S_IRUGO, show_port_data_a0, NULL, 6);
static SENSOR_DEVICE_ATTR(port_8_data_a0, S_IRUGO, show_port_data_a0, NULL, 7);
static SENSOR_DEVICE_ATTR(port_9_data_a0, S_IRUGO, show_port_data_a0, NULL, 8);
static SENSOR_DEVICE_ATTR(port_10_data_a0, S_IRUGO, show_port_data_a0, NULL, 9);
static SENSOR_DEVICE_ATTR(port_11_data_a0, S_IRUGO, show_port_data_a0, NULL, 10);
static SENSOR_DEVICE_ATTR(port_12_data_a0, S_IRUGO, show_port_data_a0, NULL, 11);
static SENSOR_DEVICE_ATTR(port_13_data_a0, S_IRUGO, show_port_data_a0, NULL, 12);
static SENSOR_DEVICE_ATTR(port_14_data_a0, S_IRUGO, show_port_data_a0, NULL, 13);
static SENSOR_DEVICE_ATTR(port_15_data_a0, S_IRUGO, show_port_data_a0, NULL, 14);
static SENSOR_DEVICE_ATTR(port_16_data_a0, S_IRUGO, show_port_data_a0, NULL, 15);
static SENSOR_DEVICE_ATTR(port_17_data_a0, S_IRUGO, show_port_data_a0, NULL, 16);
static SENSOR_DEVICE_ATTR(port_18_data_a0, S_IRUGO, show_port_data_a0, NULL, 17);
static SENSOR_DEVICE_ATTR(port_19_data_a0, S_IRUGO, show_port_data_a0, NULL, 18);
static SENSOR_DEVICE_ATTR(port_20_data_a0, S_IRUGO, show_port_data_a0, NULL, 19);
static SENSOR_DEVICE_ATTR(port_21_data_a0, S_IRUGO, show_port_data_a0, NULL, 20);
static SENSOR_DEVICE_ATTR(port_22_data_a0, S_IRUGO, show_port_data_a0, NULL, 21);
static SENSOR_DEVICE_ATTR(port_23_data_a0, S_IRUGO, show_port_data_a0, NULL, 22);
static SENSOR_DEVICE_ATTR(port_24_data_a0, S_IRUGO, show_port_data_a0, NULL, 23);
static SENSOR_DEVICE_ATTR(port_25_data_a0, S_IRUGO, show_port_data_a0, NULL, 24);
static SENSOR_DEVICE_ATTR(port_26_data_a0, S_IRUGO, show_port_data_a0, NULL, 25);
static SENSOR_DEVICE_ATTR(port_27_data_a0, S_IRUGO, show_port_data_a0, NULL, 26);
static SENSOR_DEVICE_ATTR(port_28_data_a0, S_IRUGO, show_port_data_a0, NULL, 27);
static SENSOR_DEVICE_ATTR(port_29_data_a0, S_IRUGO, show_port_data_a0, NULL, 28);
static SENSOR_DEVICE_ATTR(port_30_data_a0, S_IRUGO, show_port_data_a0, NULL, 29);
static SENSOR_DEVICE_ATTR(port_31_data_a0, S_IRUGO, show_port_data_a0, NULL, 30);
static SENSOR_DEVICE_ATTR(port_32_data_a0, S_IRUGO, show_port_data_a0, NULL, 31);
static SENSOR_DEVICE_ATTR(port_33_data_a0, S_IRUGO, show_port_data_a0, NULL, 32);
static SENSOR_DEVICE_ATTR(port_34_data_a0, S_IRUGO, show_port_data_a0, NULL, 33);
static SENSOR_DEVICE_ATTR(port_35_data_a0, S_IRUGO, show_port_data_a0, NULL, 34);
static SENSOR_DEVICE_ATTR(port_36_data_a0, S_IRUGO, show_port_data_a0, NULL, 35);
static SENSOR_DEVICE_ATTR(port_37_data_a0, S_IRUGO, show_port_data_a0, NULL, 36);
static SENSOR_DEVICE_ATTR(port_38_data_a0, S_IRUGO, show_port_data_a0, NULL, 37);
static SENSOR_DEVICE_ATTR(port_39_data_a0, S_IRUGO, show_port_data_a0, NULL, 38);
static SENSOR_DEVICE_ATTR(port_40_data_a0, S_IRUGO, show_port_data_a0, NULL, 39);
static SENSOR_DEVICE_ATTR(port_41_data_a0, S_IRUGO, show_port_data_a0, NULL, 40);
static SENSOR_DEVICE_ATTR(port_42_data_a0, S_IRUGO, show_port_data_a0, NULL, 41);
static SENSOR_DEVICE_ATTR(port_43_data_a0, S_IRUGO, show_port_data_a0, NULL, 42);
static SENSOR_DEVICE_ATTR(port_44_data_a0, S_IRUGO, show_port_data_a0, NULL, 43);
static SENSOR_DEVICE_ATTR(port_45_data_a0, S_IRUGO, show_port_data_a0, NULL, 44);
static SENSOR_DEVICE_ATTR(port_46_data_a0, S_IRUGO, show_port_data_a0, NULL, 45);
static SENSOR_DEVICE_ATTR(port_47_data_a0, S_IRUGO, show_port_data_a0, NULL, 46);
static SENSOR_DEVICE_ATTR(port_48_data_a0, S_IRUGO, show_port_data_a0, NULL, 47);
static SENSOR_DEVICE_ATTR(port_49_data_a0, S_IRUGO, show_port_data_a0, NULL, 48);
static SENSOR_DEVICE_ATTR(port_50_data_a0, S_IRUGO, show_port_data_a0, NULL, 49);
static SENSOR_DEVICE_ATTR(port_51_data_a0, S_IRUGO, show_port_data_a0, NULL, 50);
static SENSOR_DEVICE_ATTR(port_52_data_a0, S_IRUGO, show_port_data_a0, NULL, 51);
static SENSOR_DEVICE_ATTR(port_53_data_a0, S_IRUGO, show_port_data_a0, NULL, 52);
static SENSOR_DEVICE_ATTR(port_54_data_a0, S_IRUGO, show_port_data_a0, NULL, 53);
static SENSOR_DEVICE_ATTR(port_55_data_a0, S_IRUGO, show_port_data_a0, NULL, 54);
static SENSOR_DEVICE_ATTR(port_56_data_a0, S_IRUGO, show_port_data_a0, NULL, 55);
static SENSOR_DEVICE_ATTR(port_57_data_a0, S_IRUGO, show_port_data_a0, NULL, 56);
static SENSOR_DEVICE_ATTR(port_58_data_a0, S_IRUGO, show_port_data_a0, NULL, 57);
static SENSOR_DEVICE_ATTR(port_59_data_a0, S_IRUGO, show_port_data_a0, NULL, 58);
static SENSOR_DEVICE_ATTR(port_60_data_a0, S_IRUGO, show_port_data_a0, NULL, 59);
static SENSOR_DEVICE_ATTR(port_61_data_a0, S_IRUGO, show_port_data_a0, NULL, 60);
static SENSOR_DEVICE_ATTR(port_62_data_a0, S_IRUGO, show_port_data_a0, NULL, 61);
static SENSOR_DEVICE_ATTR(port_63_data_a0, S_IRUGO, show_port_data_a0, NULL, 62);
static SENSOR_DEVICE_ATTR(port_64_data_a0, S_IRUGO, show_port_data_a0, NULL, 63);

static SENSOR_DEVICE_ATTR(port_1_data_a2, S_IRUGO, show_port_data_a2, NULL, 0);
static SENSOR_DEVICE_ATTR(port_2_data_a2, S_IRUGO, show_port_data_a2, NULL, 1);
static SENSOR_DEVICE_ATTR(port_3_data_a2, S_IRUGO, show_port_data_a2, NULL, 2);
static SENSOR_DEVICE_ATTR(port_4_data_a2, S_IRUGO, show_port_data_a2, NULL, 3);
static SENSOR_DEVICE_ATTR(port_5_data_a2, S_IRUGO, show_port_data_a2, NULL, 4);
static SENSOR_DEVICE_ATTR(port_6_data_a2, S_IRUGO, show_port_data_a2, NULL, 5);
static SENSOR_DEVICE_ATTR(port_7_data_a2, S_IRUGO, show_port_data_a2, NULL, 6);
static SENSOR_DEVICE_ATTR(port_8_data_a2, S_IRUGO, show_port_data_a2, NULL, 7);
static SENSOR_DEVICE_ATTR(port_9_data_a2, S_IRUGO, show_port_data_a2, NULL, 8);
static SENSOR_DEVICE_ATTR(port_10_data_a2, S_IRUGO, show_port_data_a2, NULL, 9);
static SENSOR_DEVICE_ATTR(port_11_data_a2, S_IRUGO, show_port_data_a2, NULL, 10);
static SENSOR_DEVICE_ATTR(port_12_data_a2, S_IRUGO, show_port_data_a2, NULL, 11);
static SENSOR_DEVICE_ATTR(port_13_data_a2, S_IRUGO, show_port_data_a2, NULL, 12);
static SENSOR_DEVICE_ATTR(port_14_data_a2, S_IRUGO, show_port_data_a2, NULL, 13);
static SENSOR_DEVICE_ATTR(port_15_data_a2, S_IRUGO, show_port_data_a2, NULL, 14);
static SENSOR_DEVICE_ATTR(port_16_data_a2, S_IRUGO, show_port_data_a2, NULL, 15);
static SENSOR_DEVICE_ATTR(port_17_data_a2, S_IRUGO, show_port_data_a2, NULL, 16);
static SENSOR_DEVICE_ATTR(port_18_data_a2, S_IRUGO, show_port_data_a2, NULL, 17);
static SENSOR_DEVICE_ATTR(port_19_data_a2, S_IRUGO, show_port_data_a2, NULL, 18);
static SENSOR_DEVICE_ATTR(port_20_data_a2, S_IRUGO, show_port_data_a2, NULL, 19);
static SENSOR_DEVICE_ATTR(port_21_data_a2, S_IRUGO, show_port_data_a2, NULL, 20);
static SENSOR_DEVICE_ATTR(port_22_data_a2, S_IRUGO, show_port_data_a2, NULL, 21);
static SENSOR_DEVICE_ATTR(port_23_data_a2, S_IRUGO, show_port_data_a2, NULL, 22);
static SENSOR_DEVICE_ATTR(port_24_data_a2, S_IRUGO, show_port_data_a2, NULL, 23);
static SENSOR_DEVICE_ATTR(port_25_data_a2, S_IRUGO, show_port_data_a2, NULL, 24);
static SENSOR_DEVICE_ATTR(port_26_data_a2, S_IRUGO, show_port_data_a2, NULL, 25);
static SENSOR_DEVICE_ATTR(port_27_data_a2, S_IRUGO, show_port_data_a2, NULL, 26);
static SENSOR_DEVICE_ATTR(port_28_data_a2, S_IRUGO, show_port_data_a2, NULL, 27);
static SENSOR_DEVICE_ATTR(port_29_data_a2, S_IRUGO, show_port_data_a2, NULL, 28);
static SENSOR_DEVICE_ATTR(port_30_data_a2, S_IRUGO, show_port_data_a2, NULL, 29);
static SENSOR_DEVICE_ATTR(port_31_data_a2, S_IRUGO, show_port_data_a2, NULL, 30);
static SENSOR_DEVICE_ATTR(port_32_data_a2, S_IRUGO, show_port_data_a2, NULL, 31);
static SENSOR_DEVICE_ATTR(port_33_data_a2, S_IRUGO, show_port_data_a2, NULL, 32);
static SENSOR_DEVICE_ATTR(port_34_data_a2, S_IRUGO, show_port_data_a2, NULL, 33);
static SENSOR_DEVICE_ATTR(port_35_data_a2, S_IRUGO, show_port_data_a2, NULL, 34);
static SENSOR_DEVICE_ATTR(port_36_data_a2, S_IRUGO, show_port_data_a2, NULL, 35);
static SENSOR_DEVICE_ATTR(port_37_data_a2, S_IRUGO, show_port_data_a2, NULL, 36);
static SENSOR_DEVICE_ATTR(port_38_data_a2, S_IRUGO, show_port_data_a2, NULL, 37);
static SENSOR_DEVICE_ATTR(port_39_data_a2, S_IRUGO, show_port_data_a2, NULL, 38);
static SENSOR_DEVICE_ATTR(port_40_data_a2, S_IRUGO, show_port_data_a2, NULL, 39);
static SENSOR_DEVICE_ATTR(port_41_data_a2, S_IRUGO, show_port_data_a2, NULL, 40);
static SENSOR_DEVICE_ATTR(port_42_data_a2, S_IRUGO, show_port_data_a2, NULL, 41);
static SENSOR_DEVICE_ATTR(port_43_data_a2, S_IRUGO, show_port_data_a2, NULL, 42);
static SENSOR_DEVICE_ATTR(port_44_data_a2, S_IRUGO, show_port_data_a2, NULL, 43);
static SENSOR_DEVICE_ATTR(port_45_data_a2, S_IRUGO, show_port_data_a2, NULL, 44);
static SENSOR_DEVICE_ATTR(port_46_data_a2, S_IRUGO, show_port_data_a2, NULL, 45);
static SENSOR_DEVICE_ATTR(port_47_data_a2, S_IRUGO, show_port_data_a2, NULL, 46);
static SENSOR_DEVICE_ATTR(port_48_data_a2, S_IRUGO, show_port_data_a2, NULL, 47);
static SENSOR_DEVICE_ATTR(port_49_data_a2, S_IRUGO, show_port_data_a2, NULL, 48);
static SENSOR_DEVICE_ATTR(port_50_data_a2, S_IRUGO, show_port_data_a2, NULL, 49);
static SENSOR_DEVICE_ATTR(port_51_data_a2, S_IRUGO, show_port_data_a2, NULL, 50);
static SENSOR_DEVICE_ATTR(port_52_data_a2, S_IRUGO, show_port_data_a2, NULL, 51);
static SENSOR_DEVICE_ATTR(port_53_data_a2, S_IRUGO, show_port_data_a2, NULL, 52);
static SENSOR_DEVICE_ATTR(port_54_data_a2, S_IRUGO, show_port_data_a2, NULL, 53);
static SENSOR_DEVICE_ATTR(port_55_data_a2, S_IRUGO, show_port_data_a2, NULL, 54);
static SENSOR_DEVICE_ATTR(port_56_data_a2, S_IRUGO, show_port_data_a2, NULL, 55);
static SENSOR_DEVICE_ATTR(port_57_data_a2, S_IRUGO, show_port_data_a2, NULL, 56);
static SENSOR_DEVICE_ATTR(port_58_data_a2, S_IRUGO, show_port_data_a2, NULL, 57);
static SENSOR_DEVICE_ATTR(port_59_data_a2, S_IRUGO, show_port_data_a2, NULL, 58);
static SENSOR_DEVICE_ATTR(port_60_data_a2, S_IRUGO, show_port_data_a2, NULL, 59);
static SENSOR_DEVICE_ATTR(port_61_data_a2, S_IRUGO, show_port_data_a2, NULL, 60);
static SENSOR_DEVICE_ATTR(port_62_data_a2, S_IRUGO, show_port_data_a2, NULL, 61);
static SENSOR_DEVICE_ATTR(port_63_data_a2, S_IRUGO, show_port_data_a2, NULL, 62);
static SENSOR_DEVICE_ATTR(port_64_data_a2, S_IRUGO, show_port_data_a2, NULL, 63);

static SENSOR_DEVICE_ATTR(port_1_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 0);
static SENSOR_DEVICE_ATTR(port_2_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 1);
static SENSOR_DEVICE_ATTR(port_3_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 2);
static SENSOR_DEVICE_ATTR(port_4_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 3);
static SENSOR_DEVICE_ATTR(port_5_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 4);
static SENSOR_DEVICE_ATTR(port_6_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 5);
static SENSOR_DEVICE_ATTR(port_7_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 6);
static SENSOR_DEVICE_ATTR(port_8_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 7);
static SENSOR_DEVICE_ATTR(port_9_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 8);
static SENSOR_DEVICE_ATTR(port_10_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 9);
static SENSOR_DEVICE_ATTR(port_11_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 10);
static SENSOR_DEVICE_ATTR(port_12_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 11);
static SENSOR_DEVICE_ATTR(port_13_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 12);
static SENSOR_DEVICE_ATTR(port_14_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 13);
static SENSOR_DEVICE_ATTR(port_15_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 14);
static SENSOR_DEVICE_ATTR(port_16_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 15);
static SENSOR_DEVICE_ATTR(port_17_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 16);
static SENSOR_DEVICE_ATTR(port_18_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 17);
static SENSOR_DEVICE_ATTR(port_19_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 18);
static SENSOR_DEVICE_ATTR(port_20_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 19);
static SENSOR_DEVICE_ATTR(port_21_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 20);
static SENSOR_DEVICE_ATTR(port_22_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 21);
static SENSOR_DEVICE_ATTR(port_23_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 22);
static SENSOR_DEVICE_ATTR(port_24_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 23);
static SENSOR_DEVICE_ATTR(port_25_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 24);
static SENSOR_DEVICE_ATTR(port_26_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 25);
static SENSOR_DEVICE_ATTR(port_27_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 26);
static SENSOR_DEVICE_ATTR(port_28_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 27);
static SENSOR_DEVICE_ATTR(port_29_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 28);
static SENSOR_DEVICE_ATTR(port_30_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 29);
static SENSOR_DEVICE_ATTR(port_31_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 30);
static SENSOR_DEVICE_ATTR(port_32_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 31);
static SENSOR_DEVICE_ATTR(port_33_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 32);
static SENSOR_DEVICE_ATTR(port_34_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 33);
static SENSOR_DEVICE_ATTR(port_35_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 34);
static SENSOR_DEVICE_ATTR(port_36_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 35);
static SENSOR_DEVICE_ATTR(port_37_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 36);
static SENSOR_DEVICE_ATTR(port_38_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 37);
static SENSOR_DEVICE_ATTR(port_39_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 38);
static SENSOR_DEVICE_ATTR(port_40_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 39);
static SENSOR_DEVICE_ATTR(port_41_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 40);
static SENSOR_DEVICE_ATTR(port_42_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 41);
static SENSOR_DEVICE_ATTR(port_43_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 42);
static SENSOR_DEVICE_ATTR(port_44_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 43);
static SENSOR_DEVICE_ATTR(port_45_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 44);
static SENSOR_DEVICE_ATTR(port_46_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 45);
static SENSOR_DEVICE_ATTR(port_47_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 46);
static SENSOR_DEVICE_ATTR(port_48_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 47);
static SENSOR_DEVICE_ATTR(port_49_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 48);
static SENSOR_DEVICE_ATTR(port_50_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 49);
static SENSOR_DEVICE_ATTR(port_51_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 50);
static SENSOR_DEVICE_ATTR(port_52_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 51);
static SENSOR_DEVICE_ATTR(port_53_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 52);
static SENSOR_DEVICE_ATTR(port_54_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 53);
static SENSOR_DEVICE_ATTR(port_55_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 54);
static SENSOR_DEVICE_ATTR(port_56_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 55);
static SENSOR_DEVICE_ATTR(port_57_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 56);
static SENSOR_DEVICE_ATTR(port_58_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 57);
static SENSOR_DEVICE_ATTR(port_59_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 58);
static SENSOR_DEVICE_ATTR(port_60_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 59);
static SENSOR_DEVICE_ATTR(port_61_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 60);
static SENSOR_DEVICE_ATTR(port_62_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 61);
static SENSOR_DEVICE_ATTR(port_63_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 62);
static SENSOR_DEVICE_ATTR(port_64_sfp_copper, S_IRUGO, show_port_sfp_copper, NULL, 63);

static SENSOR_DEVICE_ATTR(port_1_abs, S_IRUGO, show_port_abs, NULL, 0);
static SENSOR_DEVICE_ATTR(port_2_abs, S_IRUGO, show_port_abs, NULL, 1);
static SENSOR_DEVICE_ATTR(port_3_abs, S_IRUGO, show_port_abs, NULL, 2);
static SENSOR_DEVICE_ATTR(port_4_abs, S_IRUGO, show_port_abs, NULL, 3);
static SENSOR_DEVICE_ATTR(port_5_abs, S_IRUGO, show_port_abs, NULL, 4);
static SENSOR_DEVICE_ATTR(port_6_abs, S_IRUGO, show_port_abs, NULL, 5);
static SENSOR_DEVICE_ATTR(port_7_abs, S_IRUGO, show_port_abs, NULL, 6);
static SENSOR_DEVICE_ATTR(port_8_abs, S_IRUGO, show_port_abs, NULL, 7);
static SENSOR_DEVICE_ATTR(port_9_abs, S_IRUGO, show_port_abs, NULL, 8);
static SENSOR_DEVICE_ATTR(port_10_abs, S_IRUGO, show_port_abs, NULL, 9);
static SENSOR_DEVICE_ATTR(port_11_abs, S_IRUGO, show_port_abs, NULL, 10);
static SENSOR_DEVICE_ATTR(port_12_abs, S_IRUGO, show_port_abs, NULL, 11);
static SENSOR_DEVICE_ATTR(port_13_abs, S_IRUGO, show_port_abs, NULL, 12);
static SENSOR_DEVICE_ATTR(port_14_abs, S_IRUGO, show_port_abs, NULL, 13);
static SENSOR_DEVICE_ATTR(port_15_abs, S_IRUGO, show_port_abs, NULL, 14);
static SENSOR_DEVICE_ATTR(port_16_abs, S_IRUGO, show_port_abs, NULL, 15);
static SENSOR_DEVICE_ATTR(port_17_abs, S_IRUGO, show_port_abs, NULL, 16);
static SENSOR_DEVICE_ATTR(port_18_abs, S_IRUGO, show_port_abs, NULL, 17);
static SENSOR_DEVICE_ATTR(port_19_abs, S_IRUGO, show_port_abs, NULL, 18);
static SENSOR_DEVICE_ATTR(port_20_abs, S_IRUGO, show_port_abs, NULL, 19);
static SENSOR_DEVICE_ATTR(port_21_abs, S_IRUGO, show_port_abs, NULL, 20);
static SENSOR_DEVICE_ATTR(port_22_abs, S_IRUGO, show_port_abs, NULL, 21);
static SENSOR_DEVICE_ATTR(port_23_abs, S_IRUGO, show_port_abs, NULL, 22);
static SENSOR_DEVICE_ATTR(port_24_abs, S_IRUGO, show_port_abs, NULL, 23);
static SENSOR_DEVICE_ATTR(port_25_abs, S_IRUGO, show_port_abs, NULL, 24);
static SENSOR_DEVICE_ATTR(port_26_abs, S_IRUGO, show_port_abs, NULL, 25);
static SENSOR_DEVICE_ATTR(port_27_abs, S_IRUGO, show_port_abs, NULL, 26);
static SENSOR_DEVICE_ATTR(port_28_abs, S_IRUGO, show_port_abs, NULL, 27);
static SENSOR_DEVICE_ATTR(port_29_abs, S_IRUGO, show_port_abs, NULL, 28);
static SENSOR_DEVICE_ATTR(port_30_abs, S_IRUGO, show_port_abs, NULL, 29);
static SENSOR_DEVICE_ATTR(port_31_abs, S_IRUGO, show_port_abs, NULL, 30);
static SENSOR_DEVICE_ATTR(port_32_abs, S_IRUGO, show_port_abs, NULL, 31);
static SENSOR_DEVICE_ATTR(port_33_abs, S_IRUGO, show_port_abs, NULL, 32);
static SENSOR_DEVICE_ATTR(port_34_abs, S_IRUGO, show_port_abs, NULL, 33);
static SENSOR_DEVICE_ATTR(port_35_abs, S_IRUGO, show_port_abs, NULL, 34);
static SENSOR_DEVICE_ATTR(port_36_abs, S_IRUGO, show_port_abs, NULL, 35);
static SENSOR_DEVICE_ATTR(port_37_abs, S_IRUGO, show_port_abs, NULL, 36);
static SENSOR_DEVICE_ATTR(port_38_abs, S_IRUGO, show_port_abs, NULL, 37);
static SENSOR_DEVICE_ATTR(port_39_abs, S_IRUGO, show_port_abs, NULL, 38);
static SENSOR_DEVICE_ATTR(port_40_abs, S_IRUGO, show_port_abs, NULL, 39);
static SENSOR_DEVICE_ATTR(port_41_abs, S_IRUGO, show_port_abs, NULL, 40);
static SENSOR_DEVICE_ATTR(port_42_abs, S_IRUGO, show_port_abs, NULL, 41);
static SENSOR_DEVICE_ATTR(port_43_abs, S_IRUGO, show_port_abs, NULL, 42);
static SENSOR_DEVICE_ATTR(port_44_abs, S_IRUGO, show_port_abs, NULL, 43);
static SENSOR_DEVICE_ATTR(port_45_abs, S_IRUGO, show_port_abs, NULL, 44);
static SENSOR_DEVICE_ATTR(port_46_abs, S_IRUGO, show_port_abs, NULL, 45);
static SENSOR_DEVICE_ATTR(port_47_abs, S_IRUGO, show_port_abs, NULL, 46);
static SENSOR_DEVICE_ATTR(port_48_abs, S_IRUGO, show_port_abs, NULL, 47);
static SENSOR_DEVICE_ATTR(port_49_abs, S_IRUGO, show_port_abs, NULL, 48);
static SENSOR_DEVICE_ATTR(port_50_abs, S_IRUGO, show_port_abs, NULL, 49);
static SENSOR_DEVICE_ATTR(port_51_abs, S_IRUGO, show_port_abs, NULL, 50);
static SENSOR_DEVICE_ATTR(port_52_abs, S_IRUGO, show_port_abs, NULL, 51);
static SENSOR_DEVICE_ATTR(port_53_abs, S_IRUGO, show_port_abs, NULL, 52);
static SENSOR_DEVICE_ATTR(port_54_abs, S_IRUGO, show_port_abs, NULL, 53);
static SENSOR_DEVICE_ATTR(port_55_abs, S_IRUGO, show_port_abs, NULL, 54);
static SENSOR_DEVICE_ATTR(port_56_abs, S_IRUGO, show_port_abs, NULL, 55);
static SENSOR_DEVICE_ATTR(port_57_abs, S_IRUGO, show_port_abs, NULL, 56);
static SENSOR_DEVICE_ATTR(port_58_abs, S_IRUGO, show_port_abs, NULL, 57);
static SENSOR_DEVICE_ATTR(port_59_abs, S_IRUGO, show_port_abs, NULL, 58);
static SENSOR_DEVICE_ATTR(port_60_abs, S_IRUGO, show_port_abs, NULL, 59);
static SENSOR_DEVICE_ATTR(port_61_abs, S_IRUGO, show_port_abs, NULL, 60);
static SENSOR_DEVICE_ATTR(port_62_abs, S_IRUGO, show_port_abs, NULL, 61);
static SENSOR_DEVICE_ATTR(port_63_abs, S_IRUGO, show_port_abs, NULL, 62);
static SENSOR_DEVICE_ATTR(port_64_abs, S_IRUGO, show_port_abs, NULL, 63);

static SENSOR_DEVICE_ATTR(port_1_rxlos, S_IRUGO, show_port_rxlos, NULL, 0);
static SENSOR_DEVICE_ATTR(port_2_rxlos, S_IRUGO, show_port_rxlos, NULL, 1);
static SENSOR_DEVICE_ATTR(port_3_rxlos, S_IRUGO, show_port_rxlos, NULL, 2);
static SENSOR_DEVICE_ATTR(port_4_rxlos, S_IRUGO, show_port_rxlos, NULL, 3);
static SENSOR_DEVICE_ATTR(port_5_rxlos, S_IRUGO, show_port_rxlos, NULL, 4);
static SENSOR_DEVICE_ATTR(port_6_rxlos, S_IRUGO, show_port_rxlos, NULL, 5);
static SENSOR_DEVICE_ATTR(port_7_rxlos, S_IRUGO, show_port_rxlos, NULL, 6);
static SENSOR_DEVICE_ATTR(port_8_rxlos, S_IRUGO, show_port_rxlos, NULL, 7);
static SENSOR_DEVICE_ATTR(port_9_rxlos, S_IRUGO, show_port_rxlos, NULL, 8);
static SENSOR_DEVICE_ATTR(port_10_rxlos, S_IRUGO, show_port_rxlos, NULL, 9);
static SENSOR_DEVICE_ATTR(port_11_rxlos, S_IRUGO, show_port_rxlos, NULL, 10);
static SENSOR_DEVICE_ATTR(port_12_rxlos, S_IRUGO, show_port_rxlos, NULL, 11);
static SENSOR_DEVICE_ATTR(port_13_rxlos, S_IRUGO, show_port_rxlos, NULL, 12);
static SENSOR_DEVICE_ATTR(port_14_rxlos, S_IRUGO, show_port_rxlos, NULL, 13);
static SENSOR_DEVICE_ATTR(port_15_rxlos, S_IRUGO, show_port_rxlos, NULL, 14);
static SENSOR_DEVICE_ATTR(port_16_rxlos, S_IRUGO, show_port_rxlos, NULL, 15);
static SENSOR_DEVICE_ATTR(port_17_rxlos, S_IRUGO, show_port_rxlos, NULL, 16);
static SENSOR_DEVICE_ATTR(port_18_rxlos, S_IRUGO, show_port_rxlos, NULL, 17);
static SENSOR_DEVICE_ATTR(port_19_rxlos, S_IRUGO, show_port_rxlos, NULL, 18);
static SENSOR_DEVICE_ATTR(port_20_rxlos, S_IRUGO, show_port_rxlos, NULL, 19);
static SENSOR_DEVICE_ATTR(port_21_rxlos, S_IRUGO, show_port_rxlos, NULL, 20);
static SENSOR_DEVICE_ATTR(port_22_rxlos, S_IRUGO, show_port_rxlos, NULL, 21);
static SENSOR_DEVICE_ATTR(port_23_rxlos, S_IRUGO, show_port_rxlos, NULL, 22);
static SENSOR_DEVICE_ATTR(port_24_rxlos, S_IRUGO, show_port_rxlos, NULL, 23);
static SENSOR_DEVICE_ATTR(port_25_rxlos, S_IRUGO, show_port_rxlos, NULL, 24);
static SENSOR_DEVICE_ATTR(port_26_rxlos, S_IRUGO, show_port_rxlos, NULL, 25);
static SENSOR_DEVICE_ATTR(port_27_rxlos, S_IRUGO, show_port_rxlos, NULL, 26);
static SENSOR_DEVICE_ATTR(port_28_rxlos, S_IRUGO, show_port_rxlos, NULL, 27);
static SENSOR_DEVICE_ATTR(port_29_rxlos, S_IRUGO, show_port_rxlos, NULL, 28);
static SENSOR_DEVICE_ATTR(port_30_rxlos, S_IRUGO, show_port_rxlos, NULL, 29);
static SENSOR_DEVICE_ATTR(port_31_rxlos, S_IRUGO, show_port_rxlos, NULL, 30);
static SENSOR_DEVICE_ATTR(port_32_rxlos, S_IRUGO, show_port_rxlos, NULL, 31);
static SENSOR_DEVICE_ATTR(port_33_rxlos, S_IRUGO, show_port_rxlos, NULL, 32);
static SENSOR_DEVICE_ATTR(port_34_rxlos, S_IRUGO, show_port_rxlos, NULL, 33);
static SENSOR_DEVICE_ATTR(port_35_rxlos, S_IRUGO, show_port_rxlos, NULL, 34);
static SENSOR_DEVICE_ATTR(port_36_rxlos, S_IRUGO, show_port_rxlos, NULL, 35);
static SENSOR_DEVICE_ATTR(port_37_rxlos, S_IRUGO, show_port_rxlos, NULL, 36);
static SENSOR_DEVICE_ATTR(port_38_rxlos, S_IRUGO, show_port_rxlos, NULL, 37);
static SENSOR_DEVICE_ATTR(port_39_rxlos, S_IRUGO, show_port_rxlos, NULL, 38);
static SENSOR_DEVICE_ATTR(port_40_rxlos, S_IRUGO, show_port_rxlos, NULL, 39);
static SENSOR_DEVICE_ATTR(port_41_rxlos, S_IRUGO, show_port_rxlos, NULL, 40);
static SENSOR_DEVICE_ATTR(port_42_rxlos, S_IRUGO, show_port_rxlos, NULL, 41);
static SENSOR_DEVICE_ATTR(port_43_rxlos, S_IRUGO, show_port_rxlos, NULL, 42);
static SENSOR_DEVICE_ATTR(port_44_rxlos, S_IRUGO, show_port_rxlos, NULL, 43);
static SENSOR_DEVICE_ATTR(port_45_rxlos, S_IRUGO, show_port_rxlos, NULL, 44);
static SENSOR_DEVICE_ATTR(port_46_rxlos, S_IRUGO, show_port_rxlos, NULL, 45);
static SENSOR_DEVICE_ATTR(port_47_rxlos, S_IRUGO, show_port_rxlos, NULL, 46);
static SENSOR_DEVICE_ATTR(port_48_rxlos, S_IRUGO, show_port_rxlos, NULL, 47);

static SENSOR_DEVICE_ATTR(port_1_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 0);
static SENSOR_DEVICE_ATTR(port_2_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 1);
static SENSOR_DEVICE_ATTR(port_3_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 2);
static SENSOR_DEVICE_ATTR(port_4_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 3);
static SENSOR_DEVICE_ATTR(port_5_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 4);
static SENSOR_DEVICE_ATTR(port_6_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 5);
static SENSOR_DEVICE_ATTR(port_7_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 6);
static SENSOR_DEVICE_ATTR(port_8_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 7);
static SENSOR_DEVICE_ATTR(port_9_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 8);
static SENSOR_DEVICE_ATTR(port_10_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 9);
static SENSOR_DEVICE_ATTR(port_11_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 10);
static SENSOR_DEVICE_ATTR(port_12_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 11);
static SENSOR_DEVICE_ATTR(port_13_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 12);
static SENSOR_DEVICE_ATTR(port_14_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 13);
static SENSOR_DEVICE_ATTR(port_15_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 14);
static SENSOR_DEVICE_ATTR(port_16_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 15);
static SENSOR_DEVICE_ATTR(port_17_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 16);
static SENSOR_DEVICE_ATTR(port_18_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 17);
static SENSOR_DEVICE_ATTR(port_19_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 18);
static SENSOR_DEVICE_ATTR(port_20_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 19);
static SENSOR_DEVICE_ATTR(port_21_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 20);
static SENSOR_DEVICE_ATTR(port_22_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 21);
static SENSOR_DEVICE_ATTR(port_23_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 22);
static SENSOR_DEVICE_ATTR(port_24_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 23);
static SENSOR_DEVICE_ATTR(port_25_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 24);
static SENSOR_DEVICE_ATTR(port_26_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 25);
static SENSOR_DEVICE_ATTR(port_27_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 26);
static SENSOR_DEVICE_ATTR(port_28_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 27);
static SENSOR_DEVICE_ATTR(port_29_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 28);
static SENSOR_DEVICE_ATTR(port_30_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 29);
static SENSOR_DEVICE_ATTR(port_31_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 30);
static SENSOR_DEVICE_ATTR(port_32_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 31);
static SENSOR_DEVICE_ATTR(port_33_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 32);
static SENSOR_DEVICE_ATTR(port_34_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 33);
static SENSOR_DEVICE_ATTR(port_35_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 34);
static SENSOR_DEVICE_ATTR(port_36_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 35);
static SENSOR_DEVICE_ATTR(port_37_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 36);
static SENSOR_DEVICE_ATTR(port_38_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 37);
static SENSOR_DEVICE_ATTR(port_39_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 38);
static SENSOR_DEVICE_ATTR(port_40_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 39);
static SENSOR_DEVICE_ATTR(port_41_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 40);
static SENSOR_DEVICE_ATTR(port_42_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 41);
static SENSOR_DEVICE_ATTR(port_43_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 42);
static SENSOR_DEVICE_ATTR(port_44_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 43);
static SENSOR_DEVICE_ATTR(port_45_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 44);
static SENSOR_DEVICE_ATTR(port_46_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 45);
static SENSOR_DEVICE_ATTR(port_47_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 46);
static SENSOR_DEVICE_ATTR(port_48_tx_disable, S_IWUSR | S_IRUGO, show_port_tx_disable, set_port_tx_disable, 47);

static SENSOR_DEVICE_ATTR(port_1_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 0);
static SENSOR_DEVICE_ATTR(port_2_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 1);
static SENSOR_DEVICE_ATTR(port_3_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 2);
static SENSOR_DEVICE_ATTR(port_4_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 3);
static SENSOR_DEVICE_ATTR(port_5_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 4);
static SENSOR_DEVICE_ATTR(port_6_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 5);
static SENSOR_DEVICE_ATTR(port_7_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 6);
static SENSOR_DEVICE_ATTR(port_8_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 7);
static SENSOR_DEVICE_ATTR(port_9_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 8);
static SENSOR_DEVICE_ATTR(port_10_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 9);
static SENSOR_DEVICE_ATTR(port_11_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 10);
static SENSOR_DEVICE_ATTR(port_12_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 11);
static SENSOR_DEVICE_ATTR(port_13_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 12);
static SENSOR_DEVICE_ATTR(port_14_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 13);
static SENSOR_DEVICE_ATTR(port_15_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 14);
static SENSOR_DEVICE_ATTR(port_16_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 15);
static SENSOR_DEVICE_ATTR(port_17_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 16);
static SENSOR_DEVICE_ATTR(port_18_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 17);
static SENSOR_DEVICE_ATTR(port_19_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 18);
static SENSOR_DEVICE_ATTR(port_20_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 19);
static SENSOR_DEVICE_ATTR(port_21_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 20);
static SENSOR_DEVICE_ATTR(port_22_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 21);
static SENSOR_DEVICE_ATTR(port_23_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 22);
static SENSOR_DEVICE_ATTR(port_24_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 23);
static SENSOR_DEVICE_ATTR(port_25_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 24);
static SENSOR_DEVICE_ATTR(port_26_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 25);
static SENSOR_DEVICE_ATTR(port_27_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 26);
static SENSOR_DEVICE_ATTR(port_28_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 27);
static SENSOR_DEVICE_ATTR(port_29_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 28);
static SENSOR_DEVICE_ATTR(port_30_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 29);
static SENSOR_DEVICE_ATTR(port_31_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 30);
static SENSOR_DEVICE_ATTR(port_32_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 31);
static SENSOR_DEVICE_ATTR(port_33_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 32);
static SENSOR_DEVICE_ATTR(port_34_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 33);
static SENSOR_DEVICE_ATTR(port_35_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 34);
static SENSOR_DEVICE_ATTR(port_36_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 35);
static SENSOR_DEVICE_ATTR(port_37_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 36);
static SENSOR_DEVICE_ATTR(port_38_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 37);
static SENSOR_DEVICE_ATTR(port_39_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 38);
static SENSOR_DEVICE_ATTR(port_40_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 39);
static SENSOR_DEVICE_ATTR(port_41_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 40);
static SENSOR_DEVICE_ATTR(port_42_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 41);
static SENSOR_DEVICE_ATTR(port_43_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 42);
static SENSOR_DEVICE_ATTR(port_44_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 43);
static SENSOR_DEVICE_ATTR(port_45_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 44);
static SENSOR_DEVICE_ATTR(port_46_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 45);
static SENSOR_DEVICE_ATTR(port_47_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 46);
static SENSOR_DEVICE_ATTR(port_48_rate_select, S_IWUSR | S_IRUGO, show_port_rate_select, set_port_rate_select, 47);

static SENSOR_DEVICE_ATTR(fan1_abs, S_IRUGO, show_fan_abs, NULL, 0);
static SENSOR_DEVICE_ATTR(fan2_abs, S_IRUGO, show_fan_abs, NULL, 1);
static SENSOR_DEVICE_ATTR(fan3_abs, S_IRUGO, show_fan_abs, NULL, 2);
static SENSOR_DEVICE_ATTR(fan4_abs, S_IRUGO, show_fan_abs, NULL, 3);
static SENSOR_DEVICE_ATTR(fan5_abs, S_IRUGO, show_fan_abs, NULL, 4);

static SENSOR_DEVICE_ATTR(fan1_dir, S_IRUGO, show_fan_dir, NULL, 0);
static SENSOR_DEVICE_ATTR(fan2_dir, S_IRUGO, show_fan_dir, NULL, 1);
static SENSOR_DEVICE_ATTR(fan3_dir, S_IRUGO, show_fan_dir, NULL, 2);
static SENSOR_DEVICE_ATTR(fan4_dir, S_IRUGO, show_fan_dir, NULL, 3);
static SENSOR_DEVICE_ATTR(fan5_dir, S_IRUGO, show_fan_dir, NULL, 4);

static SENSOR_DEVICE_ATTR(psu1_eeprom, S_IRUGO, show_psu_eeprom, NULL, 0);
static SENSOR_DEVICE_ATTR(psu2_eeprom, S_IRUGO, show_psu_eeprom, NULL, 1);

static SENSOR_DEVICE_ATTR(psu1_vout, S_IRUGO, show_psu_vout, NULL, 0);
static SENSOR_DEVICE_ATTR(psu1_iout, S_IRUGO, show_psu_iout, NULL, 0);
static SENSOR_DEVICE_ATTR(psu1_temp_1, S_IRUGO, show_psu_temp_1, NULL, 0);
static SENSOR_DEVICE_ATTR(psu1_temp_2, S_IRUGO, show_psu_temp_2, NULL, 0);
static SENSOR_DEVICE_ATTR(psu1_fan_speed, S_IRUGO, show_psu_fan_speed, NULL, 0);
static SENSOR_DEVICE_ATTR(psu1_pout, S_IRUGO, show_psu_pout, NULL, 0);
static SENSOR_DEVICE_ATTR(psu1_pin, S_IRUGO, show_psu_pin, NULL, 0);

static SENSOR_DEVICE_ATTR(psu2_vout, S_IRUGO, show_psu_vout, NULL, 1);
static SENSOR_DEVICE_ATTR(psu2_iout, S_IRUGO, show_psu_iout, NULL, 1);
static SENSOR_DEVICE_ATTR(psu2_temp_1, S_IRUGO, show_psu_temp_1, NULL, 1);
static SENSOR_DEVICE_ATTR(psu2_temp_2, S_IRUGO, show_psu_temp_2, NULL, 1);
static SENSOR_DEVICE_ATTR(psu2_fan_speed, S_IRUGO, show_psu_fan_speed, NULL, 1);
static SENSOR_DEVICE_ATTR(psu2_pout, S_IRUGO, show_psu_pout, NULL, 1);
static SENSOR_DEVICE_ATTR(psu2_pin, S_IRUGO, show_psu_pin, NULL, 1);

static DEVICE_ATTR(psu_power_off, S_IWUSR, NULL, set_psu_power_off);

static struct attribute *i2c_bus1_hardware_monitor_attr_huracan[] = {
    &dev_attr_eeprom.attr,
    &dev_attr_system_led.attr,
    &dev_attr_fan_led.attr,
    &sensor_dev_attr_psu1_led.dev_attr.attr,
    &sensor_dev_attr_psu2_led.dev_attr.attr,

    &sensor_dev_attr_port_1_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_2_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_3_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_4_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_5_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_6_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_7_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_8_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_9_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_10_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_11_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_12_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_13_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_14_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_15_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_16_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_17_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_18_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_19_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_20_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_21_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_22_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_23_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_24_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_25_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_26_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_27_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_28_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_29_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_30_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_31_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_32_data_a0.dev_attr.attr,

    &sensor_dev_attr_port_1_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_2_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_3_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_4_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_5_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_6_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_7_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_8_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_9_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_10_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_11_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_12_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_13_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_14_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_15_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_16_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_17_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_18_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_19_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_20_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_21_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_22_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_23_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_24_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_25_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_26_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_27_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_28_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_29_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_30_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_31_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_32_data_a2.dev_attr.attr,

    &sensor_dev_attr_port_1_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_2_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_3_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_4_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_5_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_6_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_7_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_8_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_9_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_10_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_11_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_12_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_13_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_14_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_15_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_16_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_17_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_18_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_19_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_20_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_21_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_22_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_23_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_24_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_25_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_26_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_27_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_28_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_29_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_30_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_31_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_32_sfp_copper.dev_attr.attr,

    &sensor_dev_attr_port_1_abs.dev_attr.attr,
    &sensor_dev_attr_port_2_abs.dev_attr.attr,
    &sensor_dev_attr_port_3_abs.dev_attr.attr,
    &sensor_dev_attr_port_4_abs.dev_attr.attr,
    &sensor_dev_attr_port_5_abs.dev_attr.attr,
    &sensor_dev_attr_port_6_abs.dev_attr.attr,
    &sensor_dev_attr_port_7_abs.dev_attr.attr,
    &sensor_dev_attr_port_8_abs.dev_attr.attr,
    &sensor_dev_attr_port_9_abs.dev_attr.attr,
    &sensor_dev_attr_port_10_abs.dev_attr.attr,
    &sensor_dev_attr_port_11_abs.dev_attr.attr,
    &sensor_dev_attr_port_12_abs.dev_attr.attr,
    &sensor_dev_attr_port_13_abs.dev_attr.attr,
    &sensor_dev_attr_port_14_abs.dev_attr.attr,
    &sensor_dev_attr_port_15_abs.dev_attr.attr,
    &sensor_dev_attr_port_16_abs.dev_attr.attr,
    &sensor_dev_attr_port_17_abs.dev_attr.attr,
    &sensor_dev_attr_port_18_abs.dev_attr.attr,
    &sensor_dev_attr_port_19_abs.dev_attr.attr,
    &sensor_dev_attr_port_20_abs.dev_attr.attr,
    &sensor_dev_attr_port_21_abs.dev_attr.attr,
    &sensor_dev_attr_port_22_abs.dev_attr.attr,
    &sensor_dev_attr_port_23_abs.dev_attr.attr,
    &sensor_dev_attr_port_24_abs.dev_attr.attr,
    &sensor_dev_attr_port_25_abs.dev_attr.attr,
    &sensor_dev_attr_port_26_abs.dev_attr.attr,
    &sensor_dev_attr_port_27_abs.dev_attr.attr,
    &sensor_dev_attr_port_28_abs.dev_attr.attr,
    &sensor_dev_attr_port_29_abs.dev_attr.attr,
    &sensor_dev_attr_port_30_abs.dev_attr.attr,
    &sensor_dev_attr_port_31_abs.dev_attr.attr,
    &sensor_dev_attr_port_32_abs.dev_attr.attr,

    &sensor_dev_attr_fan1_abs.dev_attr.attr,
    &sensor_dev_attr_fan2_abs.dev_attr.attr,
    &sensor_dev_attr_fan3_abs.dev_attr.attr,
    &sensor_dev_attr_fan4_abs.dev_attr.attr,

    &sensor_dev_attr_fan1_dir.dev_attr.attr,
    &sensor_dev_attr_fan2_dir.dev_attr.attr,
    &sensor_dev_attr_fan3_dir.dev_attr.attr,
    &sensor_dev_attr_fan4_dir.dev_attr.attr,

    &sensor_dev_attr_psu1_eeprom.dev_attr.attr,
    &sensor_dev_attr_psu2_eeprom.dev_attr.attr,

    &sensor_dev_attr_psu1_vout.dev_attr.attr,
    &sensor_dev_attr_psu1_iout.dev_attr.attr,
    &sensor_dev_attr_psu1_temp_1.dev_attr.attr,
    &sensor_dev_attr_psu1_temp_2.dev_attr.attr,
    &sensor_dev_attr_psu1_fan_speed.dev_attr.attr,
    &sensor_dev_attr_psu1_pout.dev_attr.attr,
    &sensor_dev_attr_psu1_pin.dev_attr.attr,

    &sensor_dev_attr_psu2_vout.dev_attr.attr,
    &sensor_dev_attr_psu2_iout.dev_attr.attr,
    &sensor_dev_attr_psu2_temp_1.dev_attr.attr,
    &sensor_dev_attr_psu2_temp_2.dev_attr.attr,
    &sensor_dev_attr_psu2_fan_speed.dev_attr.attr,
    &sensor_dev_attr_psu2_pout.dev_attr.attr,
    &sensor_dev_attr_psu2_pin.dev_attr.attr,

    &dev_attr_psu_power_off.attr,

    NULL
};

static struct attribute *i2c_bus1_hardware_monitor_attr_sesto[] = {
    &dev_attr_eeprom.attr,
    &dev_attr_system_led.attr,
    &dev_attr_fan_led.attr,
    &sensor_dev_attr_psu1_led.dev_attr.attr,
    &sensor_dev_attr_psu2_led.dev_attr.attr,

    &sensor_dev_attr_port_1_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_2_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_3_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_4_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_5_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_6_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_7_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_8_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_9_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_10_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_11_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_12_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_13_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_14_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_15_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_16_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_17_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_18_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_19_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_20_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_21_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_22_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_23_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_24_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_25_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_26_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_27_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_28_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_29_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_30_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_31_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_32_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_33_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_34_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_35_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_36_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_37_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_38_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_39_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_40_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_41_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_42_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_43_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_44_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_45_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_46_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_47_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_48_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_49_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_50_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_51_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_52_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_53_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_54_data_a0.dev_attr.attr,

    &sensor_dev_attr_port_1_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_2_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_3_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_4_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_5_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_6_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_7_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_8_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_9_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_10_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_11_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_12_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_13_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_14_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_15_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_16_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_17_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_18_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_19_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_20_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_21_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_22_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_23_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_24_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_25_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_26_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_27_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_28_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_29_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_30_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_31_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_32_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_33_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_34_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_35_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_36_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_37_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_38_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_39_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_40_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_41_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_42_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_43_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_44_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_45_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_46_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_47_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_48_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_49_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_50_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_51_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_52_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_53_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_54_data_a2.dev_attr.attr,

    &sensor_dev_attr_port_1_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_2_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_3_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_4_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_5_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_6_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_7_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_8_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_9_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_10_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_11_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_12_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_13_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_14_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_15_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_16_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_17_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_18_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_19_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_20_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_21_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_22_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_23_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_24_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_25_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_26_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_27_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_28_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_29_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_30_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_31_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_32_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_33_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_34_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_35_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_36_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_37_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_38_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_39_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_40_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_41_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_42_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_43_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_44_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_45_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_46_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_47_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_48_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_49_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_50_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_51_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_52_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_53_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_54_sfp_copper.dev_attr.attr,

    &sensor_dev_attr_port_1_abs.dev_attr.attr,
    &sensor_dev_attr_port_2_abs.dev_attr.attr,
    &sensor_dev_attr_port_3_abs.dev_attr.attr,
    &sensor_dev_attr_port_4_abs.dev_attr.attr,
    &sensor_dev_attr_port_5_abs.dev_attr.attr,
    &sensor_dev_attr_port_6_abs.dev_attr.attr,
    &sensor_dev_attr_port_7_abs.dev_attr.attr,
    &sensor_dev_attr_port_8_abs.dev_attr.attr,
    &sensor_dev_attr_port_9_abs.dev_attr.attr,
    &sensor_dev_attr_port_10_abs.dev_attr.attr,
    &sensor_dev_attr_port_11_abs.dev_attr.attr,
    &sensor_dev_attr_port_12_abs.dev_attr.attr,
    &sensor_dev_attr_port_13_abs.dev_attr.attr,
    &sensor_dev_attr_port_14_abs.dev_attr.attr,
    &sensor_dev_attr_port_15_abs.dev_attr.attr,
    &sensor_dev_attr_port_16_abs.dev_attr.attr,
    &sensor_dev_attr_port_17_abs.dev_attr.attr,
    &sensor_dev_attr_port_18_abs.dev_attr.attr,
    &sensor_dev_attr_port_19_abs.dev_attr.attr,
    &sensor_dev_attr_port_20_abs.dev_attr.attr,
    &sensor_dev_attr_port_21_abs.dev_attr.attr,
    &sensor_dev_attr_port_22_abs.dev_attr.attr,
    &sensor_dev_attr_port_23_abs.dev_attr.attr,
    &sensor_dev_attr_port_24_abs.dev_attr.attr,
    &sensor_dev_attr_port_25_abs.dev_attr.attr,
    &sensor_dev_attr_port_26_abs.dev_attr.attr,
    &sensor_dev_attr_port_27_abs.dev_attr.attr,
    &sensor_dev_attr_port_28_abs.dev_attr.attr,
    &sensor_dev_attr_port_29_abs.dev_attr.attr,
    &sensor_dev_attr_port_30_abs.dev_attr.attr,
    &sensor_dev_attr_port_31_abs.dev_attr.attr,
    &sensor_dev_attr_port_32_abs.dev_attr.attr,
    &sensor_dev_attr_port_33_abs.dev_attr.attr,
    &sensor_dev_attr_port_34_abs.dev_attr.attr,
    &sensor_dev_attr_port_35_abs.dev_attr.attr,
    &sensor_dev_attr_port_36_abs.dev_attr.attr,
    &sensor_dev_attr_port_37_abs.dev_attr.attr,
    &sensor_dev_attr_port_38_abs.dev_attr.attr,
    &sensor_dev_attr_port_39_abs.dev_attr.attr,
    &sensor_dev_attr_port_40_abs.dev_attr.attr,
    &sensor_dev_attr_port_41_abs.dev_attr.attr,
    &sensor_dev_attr_port_42_abs.dev_attr.attr,
    &sensor_dev_attr_port_43_abs.dev_attr.attr,
    &sensor_dev_attr_port_44_abs.dev_attr.attr,
    &sensor_dev_attr_port_45_abs.dev_attr.attr,
    &sensor_dev_attr_port_46_abs.dev_attr.attr,
    &sensor_dev_attr_port_47_abs.dev_attr.attr,
    &sensor_dev_attr_port_48_abs.dev_attr.attr,
    &sensor_dev_attr_port_49_abs.dev_attr.attr,
    &sensor_dev_attr_port_50_abs.dev_attr.attr,
    &sensor_dev_attr_port_51_abs.dev_attr.attr,
    &sensor_dev_attr_port_52_abs.dev_attr.attr,
    &sensor_dev_attr_port_53_abs.dev_attr.attr,
    &sensor_dev_attr_port_54_abs.dev_attr.attr,

    &sensor_dev_attr_port_1_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_2_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_3_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_4_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_5_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_6_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_7_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_8_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_9_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_10_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_11_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_12_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_13_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_14_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_15_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_16_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_17_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_18_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_19_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_20_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_21_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_22_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_23_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_24_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_25_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_26_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_27_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_28_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_29_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_30_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_31_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_32_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_33_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_34_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_35_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_36_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_37_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_38_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_39_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_40_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_41_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_42_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_43_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_44_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_45_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_46_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_47_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_48_rxlos.dev_attr.attr,

    &sensor_dev_attr_port_1_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_2_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_3_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_4_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_5_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_6_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_7_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_8_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_9_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_10_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_11_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_12_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_13_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_14_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_15_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_16_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_17_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_18_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_19_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_20_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_21_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_22_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_23_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_24_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_25_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_26_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_27_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_28_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_29_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_30_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_31_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_32_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_33_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_34_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_35_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_36_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_37_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_38_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_39_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_40_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_41_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_42_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_43_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_44_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_45_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_46_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_47_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_48_tx_disable.dev_attr.attr,

    &sensor_dev_attr_port_1_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_2_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_3_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_4_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_5_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_6_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_7_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_8_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_9_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_10_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_11_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_12_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_13_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_14_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_15_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_16_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_17_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_18_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_19_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_20_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_21_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_22_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_23_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_24_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_25_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_26_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_27_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_28_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_29_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_30_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_31_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_32_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_33_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_34_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_35_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_36_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_37_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_38_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_39_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_40_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_41_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_42_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_43_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_44_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_45_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_46_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_47_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_48_rate_select.dev_attr.attr,

    &sensor_dev_attr_fan1_abs.dev_attr.attr,
    &sensor_dev_attr_fan2_abs.dev_attr.attr,
    &sensor_dev_attr_fan3_abs.dev_attr.attr,
    &sensor_dev_attr_fan4_abs.dev_attr.attr,

    &sensor_dev_attr_fan1_dir.dev_attr.attr,
    &sensor_dev_attr_fan2_dir.dev_attr.attr,
    &sensor_dev_attr_fan3_dir.dev_attr.attr,
    &sensor_dev_attr_fan4_dir.dev_attr.attr,

    &sensor_dev_attr_psu1_eeprom.dev_attr.attr,
    &sensor_dev_attr_psu2_eeprom.dev_attr.attr,

    &sensor_dev_attr_psu1_vout.dev_attr.attr,
    &sensor_dev_attr_psu1_iout.dev_attr.attr,
    &sensor_dev_attr_psu1_temp_1.dev_attr.attr,
    &sensor_dev_attr_psu1_temp_2.dev_attr.attr,
    &sensor_dev_attr_psu1_fan_speed.dev_attr.attr,
    &sensor_dev_attr_psu1_pout.dev_attr.attr,
    &sensor_dev_attr_psu1_pin.dev_attr.attr,

    &sensor_dev_attr_psu2_vout.dev_attr.attr,
    &sensor_dev_attr_psu2_iout.dev_attr.attr,
    &sensor_dev_attr_psu2_temp_1.dev_attr.attr,
    &sensor_dev_attr_psu2_temp_2.dev_attr.attr,
    &sensor_dev_attr_psu2_fan_speed.dev_attr.attr,
    &sensor_dev_attr_psu2_pout.dev_attr.attr,
    &sensor_dev_attr_psu2_pin.dev_attr.attr,

    &dev_attr_psu_power_off.attr,

    NULL
};

static struct attribute *i2c_bus1_hardware_monitor_attr_nc2x[] = {
    &dev_attr_eeprom.attr,
    &dev_attr_system_led.attr,
    &dev_attr_fan_led.attr,
    &sensor_dev_attr_psu1_led.dev_attr.attr,
    &sensor_dev_attr_psu2_led.dev_attr.attr,

    &sensor_dev_attr_port_1_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_2_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_3_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_4_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_5_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_6_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_7_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_8_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_9_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_10_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_11_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_12_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_13_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_14_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_15_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_16_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_17_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_18_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_19_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_20_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_21_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_22_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_23_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_24_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_25_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_26_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_27_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_28_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_29_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_30_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_31_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_32_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_33_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_34_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_35_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_36_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_37_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_38_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_39_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_40_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_41_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_42_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_43_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_44_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_45_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_46_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_47_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_48_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_49_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_50_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_51_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_52_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_53_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_54_data_a0.dev_attr.attr,

    &sensor_dev_attr_port_1_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_2_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_3_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_4_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_5_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_6_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_7_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_8_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_9_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_10_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_11_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_12_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_13_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_14_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_15_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_16_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_17_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_18_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_19_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_20_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_21_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_22_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_23_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_24_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_25_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_26_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_27_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_28_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_29_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_30_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_31_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_32_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_33_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_34_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_35_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_36_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_37_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_38_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_39_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_40_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_41_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_42_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_43_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_44_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_45_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_46_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_47_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_48_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_49_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_50_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_51_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_52_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_53_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_54_data_a2.dev_attr.attr,

    &sensor_dev_attr_port_1_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_2_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_3_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_4_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_5_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_6_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_7_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_8_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_9_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_10_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_11_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_12_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_13_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_14_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_15_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_16_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_17_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_18_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_19_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_20_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_21_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_22_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_23_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_24_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_25_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_26_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_27_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_28_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_29_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_30_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_31_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_32_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_33_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_34_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_35_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_36_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_37_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_38_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_39_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_40_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_41_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_42_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_43_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_44_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_45_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_46_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_47_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_48_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_49_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_50_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_51_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_52_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_53_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_54_sfp_copper.dev_attr.attr,

    &sensor_dev_attr_port_1_abs.dev_attr.attr,
    &sensor_dev_attr_port_2_abs.dev_attr.attr,
    &sensor_dev_attr_port_3_abs.dev_attr.attr,
    &sensor_dev_attr_port_4_abs.dev_attr.attr,
    &sensor_dev_attr_port_5_abs.dev_attr.attr,
    &sensor_dev_attr_port_6_abs.dev_attr.attr,
    &sensor_dev_attr_port_7_abs.dev_attr.attr,
    &sensor_dev_attr_port_8_abs.dev_attr.attr,
    &sensor_dev_attr_port_9_abs.dev_attr.attr,
    &sensor_dev_attr_port_10_abs.dev_attr.attr,
    &sensor_dev_attr_port_11_abs.dev_attr.attr,
    &sensor_dev_attr_port_12_abs.dev_attr.attr,
    &sensor_dev_attr_port_13_abs.dev_attr.attr,
    &sensor_dev_attr_port_14_abs.dev_attr.attr,
    &sensor_dev_attr_port_15_abs.dev_attr.attr,
    &sensor_dev_attr_port_16_abs.dev_attr.attr,
    &sensor_dev_attr_port_17_abs.dev_attr.attr,
    &sensor_dev_attr_port_18_abs.dev_attr.attr,
    &sensor_dev_attr_port_19_abs.dev_attr.attr,
    &sensor_dev_attr_port_20_abs.dev_attr.attr,
    &sensor_dev_attr_port_21_abs.dev_attr.attr,
    &sensor_dev_attr_port_22_abs.dev_attr.attr,
    &sensor_dev_attr_port_23_abs.dev_attr.attr,
    &sensor_dev_attr_port_24_abs.dev_attr.attr,
    &sensor_dev_attr_port_25_abs.dev_attr.attr,
    &sensor_dev_attr_port_26_abs.dev_attr.attr,
    &sensor_dev_attr_port_27_abs.dev_attr.attr,
    &sensor_dev_attr_port_28_abs.dev_attr.attr,
    &sensor_dev_attr_port_29_abs.dev_attr.attr,
    &sensor_dev_attr_port_30_abs.dev_attr.attr,
    &sensor_dev_attr_port_31_abs.dev_attr.attr,
    &sensor_dev_attr_port_32_abs.dev_attr.attr,
    &sensor_dev_attr_port_33_abs.dev_attr.attr,
    &sensor_dev_attr_port_34_abs.dev_attr.attr,
    &sensor_dev_attr_port_35_abs.dev_attr.attr,
    &sensor_dev_attr_port_36_abs.dev_attr.attr,
    &sensor_dev_attr_port_37_abs.dev_attr.attr,
    &sensor_dev_attr_port_38_abs.dev_attr.attr,
    &sensor_dev_attr_port_39_abs.dev_attr.attr,
    &sensor_dev_attr_port_40_abs.dev_attr.attr,
    &sensor_dev_attr_port_41_abs.dev_attr.attr,
    &sensor_dev_attr_port_42_abs.dev_attr.attr,
    &sensor_dev_attr_port_43_abs.dev_attr.attr,
    &sensor_dev_attr_port_44_abs.dev_attr.attr,
    &sensor_dev_attr_port_45_abs.dev_attr.attr,
    &sensor_dev_attr_port_46_abs.dev_attr.attr,
    &sensor_dev_attr_port_47_abs.dev_attr.attr,
    &sensor_dev_attr_port_48_abs.dev_attr.attr,
    &sensor_dev_attr_port_49_abs.dev_attr.attr,
    &sensor_dev_attr_port_50_abs.dev_attr.attr,
    &sensor_dev_attr_port_51_abs.dev_attr.attr,
    &sensor_dev_attr_port_52_abs.dev_attr.attr,
    &sensor_dev_attr_port_53_abs.dev_attr.attr,
    &sensor_dev_attr_port_54_abs.dev_attr.attr,

    &sensor_dev_attr_port_1_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_2_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_3_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_4_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_5_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_6_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_7_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_8_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_9_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_10_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_11_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_12_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_13_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_14_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_15_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_16_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_17_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_18_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_19_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_20_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_21_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_22_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_23_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_24_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_25_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_26_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_27_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_28_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_29_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_30_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_31_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_32_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_33_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_34_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_35_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_36_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_37_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_38_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_39_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_40_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_41_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_42_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_43_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_44_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_45_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_46_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_47_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_48_rxlos.dev_attr.attr,

    &sensor_dev_attr_port_1_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_2_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_3_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_4_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_5_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_6_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_7_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_8_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_9_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_10_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_11_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_12_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_13_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_14_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_15_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_16_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_17_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_18_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_19_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_20_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_21_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_22_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_23_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_24_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_25_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_26_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_27_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_28_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_29_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_30_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_31_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_32_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_33_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_34_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_35_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_36_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_37_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_38_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_39_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_40_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_41_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_42_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_43_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_44_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_45_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_46_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_47_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_48_tx_disable.dev_attr.attr,

    &sensor_dev_attr_fan1_abs.dev_attr.attr,
    &sensor_dev_attr_fan2_abs.dev_attr.attr,
    &sensor_dev_attr_fan3_abs.dev_attr.attr,
    &sensor_dev_attr_fan4_abs.dev_attr.attr,

    &sensor_dev_attr_fan1_dir.dev_attr.attr,
    &sensor_dev_attr_fan2_dir.dev_attr.attr,
    &sensor_dev_attr_fan3_dir.dev_attr.attr,
    &sensor_dev_attr_fan4_dir.dev_attr.attr,

    &sensor_dev_attr_psu1_eeprom.dev_attr.attr,
    &sensor_dev_attr_psu2_eeprom.dev_attr.attr,

    &sensor_dev_attr_psu1_vout.dev_attr.attr,
    &sensor_dev_attr_psu1_iout.dev_attr.attr,
    &sensor_dev_attr_psu1_temp_1.dev_attr.attr,
    &sensor_dev_attr_psu1_temp_2.dev_attr.attr,
    &sensor_dev_attr_psu1_fan_speed.dev_attr.attr,
    &sensor_dev_attr_psu1_pout.dev_attr.attr,
    &sensor_dev_attr_psu1_pin.dev_attr.attr,

    &sensor_dev_attr_psu2_vout.dev_attr.attr,
    &sensor_dev_attr_psu2_iout.dev_attr.attr,
    &sensor_dev_attr_psu2_temp_1.dev_attr.attr,
    &sensor_dev_attr_psu2_temp_2.dev_attr.attr,
    &sensor_dev_attr_psu2_fan_speed.dev_attr.attr,
    &sensor_dev_attr_psu2_pout.dev_attr.attr,
    &sensor_dev_attr_psu2_pin.dev_attr.attr,

    &dev_attr_psu_power_off.attr,

    NULL
};

static struct attribute *i2c_bus1_hardware_monitor_attr_asterion[] = {
    &dev_attr_eeprom.attr,
    &dev_attr_system_led.attr,
    &dev_attr_fan_led.attr,
    &sensor_dev_attr_psu1_led.dev_attr.attr,
    &sensor_dev_attr_psu2_led.dev_attr.attr,

    &sensor_dev_attr_port_1_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_2_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_3_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_4_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_5_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_6_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_7_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_8_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_9_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_10_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_11_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_12_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_13_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_14_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_15_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_16_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_17_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_18_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_19_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_20_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_21_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_22_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_23_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_24_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_25_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_26_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_27_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_28_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_29_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_30_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_31_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_32_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_33_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_34_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_35_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_36_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_37_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_38_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_39_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_40_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_41_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_42_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_43_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_44_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_45_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_46_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_47_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_48_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_49_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_50_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_51_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_52_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_53_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_54_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_55_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_56_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_57_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_58_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_59_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_60_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_61_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_62_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_63_data_a0.dev_attr.attr,
    &sensor_dev_attr_port_64_data_a0.dev_attr.attr,

    &sensor_dev_attr_port_1_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_2_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_3_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_4_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_5_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_6_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_7_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_8_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_9_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_10_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_11_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_12_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_13_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_14_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_15_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_16_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_17_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_18_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_19_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_20_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_21_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_22_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_23_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_24_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_25_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_26_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_27_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_28_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_29_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_30_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_31_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_32_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_33_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_34_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_35_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_36_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_37_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_38_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_39_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_40_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_41_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_42_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_43_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_44_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_45_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_46_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_47_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_48_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_49_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_50_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_51_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_52_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_53_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_54_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_55_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_56_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_57_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_58_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_59_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_60_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_61_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_62_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_63_data_a2.dev_attr.attr,
    &sensor_dev_attr_port_64_data_a2.dev_attr.attr,

    &sensor_dev_attr_port_1_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_2_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_3_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_4_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_5_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_6_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_7_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_8_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_9_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_10_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_11_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_12_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_13_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_14_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_15_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_16_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_17_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_18_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_19_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_20_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_21_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_22_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_23_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_24_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_25_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_26_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_27_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_28_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_29_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_30_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_31_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_32_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_33_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_34_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_35_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_36_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_37_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_38_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_39_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_40_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_41_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_42_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_43_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_44_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_45_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_46_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_47_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_48_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_49_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_50_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_51_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_52_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_53_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_54_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_55_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_56_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_57_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_58_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_59_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_60_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_61_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_62_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_63_sfp_copper.dev_attr.attr,
    &sensor_dev_attr_port_64_sfp_copper.dev_attr.attr,

    &sensor_dev_attr_port_1_abs.dev_attr.attr,
    &sensor_dev_attr_port_2_abs.dev_attr.attr,
    &sensor_dev_attr_port_3_abs.dev_attr.attr,
    &sensor_dev_attr_port_4_abs.dev_attr.attr,
    &sensor_dev_attr_port_5_abs.dev_attr.attr,
    &sensor_dev_attr_port_6_abs.dev_attr.attr,
    &sensor_dev_attr_port_7_abs.dev_attr.attr,
    &sensor_dev_attr_port_8_abs.dev_attr.attr,
    &sensor_dev_attr_port_9_abs.dev_attr.attr,
    &sensor_dev_attr_port_10_abs.dev_attr.attr,
    &sensor_dev_attr_port_11_abs.dev_attr.attr,
    &sensor_dev_attr_port_12_abs.dev_attr.attr,
    &sensor_dev_attr_port_13_abs.dev_attr.attr,
    &sensor_dev_attr_port_14_abs.dev_attr.attr,
    &sensor_dev_attr_port_15_abs.dev_attr.attr,
    &sensor_dev_attr_port_16_abs.dev_attr.attr,
    &sensor_dev_attr_port_17_abs.dev_attr.attr,
    &sensor_dev_attr_port_18_abs.dev_attr.attr,
    &sensor_dev_attr_port_19_abs.dev_attr.attr,
    &sensor_dev_attr_port_20_abs.dev_attr.attr,
    &sensor_dev_attr_port_21_abs.dev_attr.attr,
    &sensor_dev_attr_port_22_abs.dev_attr.attr,
    &sensor_dev_attr_port_23_abs.dev_attr.attr,
    &sensor_dev_attr_port_24_abs.dev_attr.attr,
    &sensor_dev_attr_port_25_abs.dev_attr.attr,
    &sensor_dev_attr_port_26_abs.dev_attr.attr,
    &sensor_dev_attr_port_27_abs.dev_attr.attr,
    &sensor_dev_attr_port_28_abs.dev_attr.attr,
    &sensor_dev_attr_port_29_abs.dev_attr.attr,
    &sensor_dev_attr_port_30_abs.dev_attr.attr,
    &sensor_dev_attr_port_31_abs.dev_attr.attr,
    &sensor_dev_attr_port_32_abs.dev_attr.attr,
    &sensor_dev_attr_port_33_abs.dev_attr.attr,
    &sensor_dev_attr_port_34_abs.dev_attr.attr,
    &sensor_dev_attr_port_35_abs.dev_attr.attr,
    &sensor_dev_attr_port_36_abs.dev_attr.attr,
    &sensor_dev_attr_port_37_abs.dev_attr.attr,
    &sensor_dev_attr_port_38_abs.dev_attr.attr,
    &sensor_dev_attr_port_39_abs.dev_attr.attr,
    &sensor_dev_attr_port_40_abs.dev_attr.attr,
    &sensor_dev_attr_port_41_abs.dev_attr.attr,
    &sensor_dev_attr_port_42_abs.dev_attr.attr,
    &sensor_dev_attr_port_43_abs.dev_attr.attr,
    &sensor_dev_attr_port_44_abs.dev_attr.attr,
    &sensor_dev_attr_port_45_abs.dev_attr.attr,
    &sensor_dev_attr_port_46_abs.dev_attr.attr,
    &sensor_dev_attr_port_47_abs.dev_attr.attr,
    &sensor_dev_attr_port_48_abs.dev_attr.attr,
    &sensor_dev_attr_port_49_abs.dev_attr.attr,
    &sensor_dev_attr_port_50_abs.dev_attr.attr,
    &sensor_dev_attr_port_51_abs.dev_attr.attr,
    &sensor_dev_attr_port_52_abs.dev_attr.attr,
    &sensor_dev_attr_port_53_abs.dev_attr.attr,
    &sensor_dev_attr_port_54_abs.dev_attr.attr,
    &sensor_dev_attr_port_55_abs.dev_attr.attr,
    &sensor_dev_attr_port_56_abs.dev_attr.attr,
    &sensor_dev_attr_port_57_abs.dev_attr.attr,
    &sensor_dev_attr_port_58_abs.dev_attr.attr,
    &sensor_dev_attr_port_59_abs.dev_attr.attr,
    &sensor_dev_attr_port_60_abs.dev_attr.attr,
    &sensor_dev_attr_port_61_abs.dev_attr.attr,
    &sensor_dev_attr_port_62_abs.dev_attr.attr,
    &sensor_dev_attr_port_63_abs.dev_attr.attr,
    &sensor_dev_attr_port_64_abs.dev_attr.attr,

    &sensor_dev_attr_port_1_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_2_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_3_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_4_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_5_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_6_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_7_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_8_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_9_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_10_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_11_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_12_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_13_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_14_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_15_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_16_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_17_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_18_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_19_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_20_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_21_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_22_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_23_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_24_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_25_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_26_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_27_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_28_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_29_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_30_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_31_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_32_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_33_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_34_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_35_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_36_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_37_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_38_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_39_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_40_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_41_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_42_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_43_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_44_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_45_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_46_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_47_rxlos.dev_attr.attr,
    &sensor_dev_attr_port_48_rxlos.dev_attr.attr,

    &sensor_dev_attr_port_1_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_2_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_3_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_4_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_5_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_6_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_7_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_8_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_9_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_10_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_11_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_12_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_13_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_14_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_15_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_16_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_17_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_18_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_19_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_20_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_21_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_22_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_23_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_24_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_25_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_26_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_27_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_28_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_29_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_30_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_31_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_32_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_33_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_34_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_35_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_36_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_37_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_38_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_39_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_40_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_41_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_42_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_43_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_44_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_45_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_46_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_47_tx_disable.dev_attr.attr,
    &sensor_dev_attr_port_48_tx_disable.dev_attr.attr,

    &sensor_dev_attr_port_1_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_2_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_3_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_4_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_5_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_6_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_7_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_8_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_9_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_10_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_11_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_12_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_13_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_14_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_15_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_16_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_17_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_18_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_19_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_20_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_21_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_22_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_23_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_24_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_25_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_26_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_27_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_28_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_29_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_30_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_31_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_32_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_33_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_34_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_35_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_36_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_37_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_38_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_39_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_40_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_41_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_42_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_43_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_44_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_45_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_46_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_47_rate_select.dev_attr.attr,
    &sensor_dev_attr_port_48_rate_select.dev_attr.attr,

    &sensor_dev_attr_fan1_abs.dev_attr.attr,
    &sensor_dev_attr_fan2_abs.dev_attr.attr,
    &sensor_dev_attr_fan3_abs.dev_attr.attr,
    &sensor_dev_attr_fan4_abs.dev_attr.attr,
    &sensor_dev_attr_fan5_abs.dev_attr.attr,

    &sensor_dev_attr_fan1_dir.dev_attr.attr,
    &sensor_dev_attr_fan2_dir.dev_attr.attr,
    &sensor_dev_attr_fan3_dir.dev_attr.attr,
    &sensor_dev_attr_fan4_dir.dev_attr.attr,
    &sensor_dev_attr_fan5_dir.dev_attr.attr,

    &sensor_dev_attr_psu1_eeprom.dev_attr.attr,
    &sensor_dev_attr_psu2_eeprom.dev_attr.attr,

    &sensor_dev_attr_psu1_vout.dev_attr.attr,
    &sensor_dev_attr_psu1_iout.dev_attr.attr,
    &sensor_dev_attr_psu1_temp_1.dev_attr.attr,
    &sensor_dev_attr_psu1_fan_speed.dev_attr.attr,
    &sensor_dev_attr_psu1_pout.dev_attr.attr,

    &sensor_dev_attr_psu2_vout.dev_attr.attr,
    &sensor_dev_attr_psu2_iout.dev_attr.attr,
    &sensor_dev_attr_psu2_temp_1.dev_attr.attr,
    &sensor_dev_attr_psu2_fan_speed.dev_attr.attr,
    &sensor_dev_attr_psu2_pout.dev_attr.attr,

    &dev_attr_psu_power_off.attr,

    NULL
};

static int is_port_present(struct i2c_bus1_hardware_monitor_data *data, int port)
{
    int rc = 0;

    switch(platformModelId)
    {
        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
            rc = ((SFPPortAbsStatus[port] == 1) && (SFPPortDataValid[port] == 1));
            break;

        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
        {
            unsigned char qsfpPortAbsAst = 0, index = 0, bit = 0;
            unsigned char sfpPortDataValidAst = 0;

            if (port < 48)
            {
                index = (port / 2);
                bit = ((port & 0x01) ? 5 : 1);
                qsfpPortAbsAst = data->sfpPortAbsRxLosStatus[index];
                sfpPortDataValidAst = data->sfpPortDataValidAst[port];
                rc = ((PCA9553_TEST_BIT(qsfpPortAbsAst, bit) ? 0 : 1) && (sfpPortDataValidAst));
            }
            else
            {
                index = (port % 48);
                qsfpPortAbsAst = data->qsfpPortAbsStatusAst[index];
                sfpPortDataValidAst = data->sfpPortDataValidAst[port];
                rc = ((PCA9553_TEST_BIT(qsfpPortAbsAst, 1) ? 0 : 1) && (sfpPortDataValidAst));
            }
        }
            break;

        default:
        {
            unsigned short qsfpPortAbs = 0, index = 0, bit = 0;
            unsigned short qsfpPortDataValid = 0;

            index = (port / 16);
            bit = (port % 16);
            qsfpPortAbs = data->qsfpPortAbsStatus[index];
            qsfpPortDataValid = data->qsfpPortDataValid[index];
            rc = ((PCA9553_TEST_BIT(qsfpPortAbs, bit) ? 0 : 1) && (PCA9553_TEST_BIT(qsfpPortDataValid, bit)));
        }
            break;
    }

    return rc;
}

static ssize_t get_qsfp_port_tx_rx_status(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(&qsfpDataA0_client);
    unsigned char qsfpPortData[QSFP_DATA_SIZE];
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    int index = (client->addr - 1);
    int val = 0;

    memset(qsfpPortData, 0, QSFP_DATA_SIZE);

    mutex_lock(&data->lock);
    if (is_port_present(data, index) == 1)
        memcpy(qsfpPortData, &(data->qsfpPortDataA0[index][0]), QSFP_DATA_SIZE);
    else
    {
        qsfpPortData[SFF8436_RX_LOS_ADDR] = qsfpPortData[SFF8436_TX_FAULT_ADDR] = 0xF;
        qsfpPortData[SFF8436_TX_DISABLE_ADDR] = data->qsfpPortTxDisableData[index];
    }
    mutex_unlock(&data->lock);

    switch (attr->index)
    {
        case RX_LOS:
            val = (qsfpPortData[SFF8436_RX_LOS_ADDR] & 0xF);
            break;
        case RX_LOS1:
        case RX_LOS2:
        case RX_LOS3:
        case RX_LOS4:
            val = (qsfpPortData[SFF8436_RX_LOS_ADDR] & BIT_INDEX(attr->index - RX_LOS1));
            break;

        case TX_DISABLE:
            val = (qsfpPortData[SFF8436_TX_DISABLE_ADDR] & 0xF);
            break;
        case TX_DISABLE1:
        case TX_DISABLE2:
        case TX_DISABLE3:
        case TX_DISABLE4:
            val = (qsfpPortData[SFF8436_TX_DISABLE_ADDR] & BIT_INDEX(attr->index - TX_DISABLE1));
            break;

        case TX_FAULT:
            val = (qsfpPortData[SFF8436_TX_FAULT_ADDR] & 0xF);
            break;
        case TX_FAULT1:
        case TX_FAULT2:
        case TX_FAULT3:
        case TX_FAULT4:
            val = (qsfpPortData[SFF8436_TX_FAULT_ADDR] & BIT_INDEX(attr->index - TX_FAULT1));
            break;

        default:
            break;
    }
    return sprintf(buf, "%d\n", ((val) ? 1 : 0));
}

static ssize_t get_port_status(struct device *dev, struct device_attribute *devattr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    int status = LAST_ATTRIBUTE;
    ssize_t count = 0;

    mutex_lock(&portStatusLock);

    status = attr->index;

    /* common status */
    switch (status)
    {
        case PRESENT:
            attr->index = (client->addr - 1);
            count = show_port_abs(dev, devattr, buf);
            attr->index = status;
            mutex_unlock(&portStatusLock);
            return count;

        case EEPROM_A0_PAGE:
            attr->index = (client->addr - 1);
            count = show_port_data_a0(dev, devattr, buf);
            attr->index = status;
            mutex_unlock(&portStatusLock);
            return count;

        case EEPROM_A2_PAGE:
            attr->index = (client->addr - 1);
            count = show_port_data_a2(dev, devattr, buf);
            attr->index = status;
            mutex_unlock(&portStatusLock);
            return count;

        case SFP_COPPER:
            attr->index = (client->addr - 1);
            count = show_port_sfp_copper(dev, devattr, buf);
            attr->index = status;
            mutex_unlock(&portStatusLock);
            return count;

        default:
            break;
    }

    /* status for QSFP ports */
    if (strncmp(client->name, "qsfp", strlen("qsfp")) == 0)
    {
        count = get_qsfp_port_tx_rx_status(dev, devattr, buf);
        mutex_unlock(&portStatusLock);
        return count;
    }

    /* status for SFP+ ports */
    attr->index = (client->addr - 1);
    switch (status)
    {
        case RX_LOS:
        case RX_LOS1:
        case RX_LOS2:
        case RX_LOS3:
        case RX_LOS4:
            count = show_port_rxlos(dev, devattr, buf);
            break;

        case TX_DISABLE:
        case TX_DISABLE1:
        case TX_DISABLE2:
        case TX_DISABLE3:
        case TX_DISABLE4:
            count = show_port_tx_disable(dev, devattr, buf);
            break;

        case TX_FAULT:
        case TX_FAULT1:
        case TX_FAULT2:
        case TX_FAULT3:
        case TX_FAULT4:
            count = show_port_tx_fault(dev, devattr, buf);
            break;

        default:
            count = sprintf(buf, "0\n");
            break;
    }
    attr->index = status;
    mutex_unlock(&portStatusLock);
    return count;
}

static ssize_t set_qsfp_port_tx_status(struct device *dev, struct device_attribute *devattr, const char *buf, size_t count)
{
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(&qsfpDataA0_client);
    unsigned char qsfpPortData[QSFP_DATA_SIZE];
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    int index = (client->addr - 1);
    long disable;

    if (kstrtol(buf, 10, &disable))
        return -EINVAL;
    disable = clamp_val(disable, 0, 1);

    memset(qsfpPortData, 0, QSFP_DATA_SIZE);

    mutex_lock(&data->lock);
    memcpy(qsfpPortData, &(data->qsfpPortDataA0[index][0]), QSFP_DATA_SIZE);
    switch (attr->index)
    {
        case TX_DISABLE:
            if (disable == 1)
                data->qsfpPortTxDisableData[index] = (qsfpPortData[SFF8436_TX_DISABLE_ADDR] |0xF);
            else
                data->qsfpPortTxDisableData[index] = (qsfpPortData[SFF8436_TX_DISABLE_ADDR] & 0xF0);
            data->qsfpPortTxDisableDataUpdate[index] = 1;
            break;
        case TX_DISABLE1:
        case TX_DISABLE2:
        case TX_DISABLE3:
        case TX_DISABLE4:
            if (disable == 1)
                data->qsfpPortTxDisableData[index]  = (qsfpPortData[SFF8436_TX_DISABLE_ADDR] | (1 << (attr->index - TX_DISABLE1)));
            else
                data->qsfpPortTxDisableData[index]  = (qsfpPortData[SFF8436_TX_DISABLE_ADDR] & ~(1 << (attr->index - TX_DISABLE1)));
            data->qsfpPortTxDisableDataUpdate[index] = 1;
            break;

        default:
            break;
    }
    mutex_unlock(&data->lock);
    return count;
}

static ssize_t set_port_tx_status(struct device *dev, struct device_attribute *devattr, const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    int status = LAST_ATTRIBUTE;

    mutex_lock(&portStatusLock);

    /* status for QSFP ports */
    if (strncmp(client->name, "qsfp", strlen("qsfp")) == 0)
    {
        set_qsfp_port_tx_status(dev, devattr, buf, count);
        mutex_unlock(&portStatusLock);
        return count;
    }

    /* status for SFP+ ports */
    status = attr->index;
    attr->index = (client->addr - 1);
    switch (status)
    {
        case TX_DISABLE:
        case TX_DISABLE1:
        case TX_DISABLE2:
        case TX_DISABLE3:
        case TX_DISABLE4:
            set_port_tx_disable(dev, devattr, buf, count);
            break;

        default:
            break;
    }
    attr->index = status;
    mutex_unlock(&portStatusLock);
    return count;
}

static ssize_t set_port_sfp_copper(struct device *dev, struct device_attribute *devattr, const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);

    /* QSFP ports */
    if (strncmp(client->name, "qsfp", strlen("qsfp")) == 0)
    {
        return count;
    }
    /* SFP+ ports */
    else
    {
        struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(&SfpCopperData_client);
        int index = (client->addr - 1);
        long value;

        if ((platformModelId != NCIIX_WITH_BMC) && (platformModelId != NCIIX_WITHOUT_BMC))
            return count;

        if (kstrtol(buf, 10, &value))
            return -EINVAL;

        mutex_lock(&data->lock);
        if (SFPPortAbsStatus[index]) /*present*/
        {
            i2c_smbus_write_byte_data(&(pca9548_client[1]), 0, sfpPortData_78F[index].portMaskBitForPCA9548_1);
            i2c_smbus_write_byte_data(&(pca9548_client[0]), 0, sfpPortData_78F[index].portMaskBitForPCA9548_2TO5);
            i2c_device_word_write(&SfpCopperData_client,
                                  (unsigned char)((value >> 16) & 0xff),
                                  (unsigned short)(value & 0xffff));
            i2c_smbus_write_byte_data(&(pca9548_client[0]), 0, 0x00);
            i2c_smbus_write_byte_data(&(pca9548_client[1]), 0, 0x00);
        }
        mutex_unlock(&data->lock);
    }

    return count;
}

static SENSOR_DEVICE_ATTR(abs, S_IRUGO, get_port_status, NULL, PRESENT);
static SENSOR_DEVICE_ATTR(rxlos, S_IRUGO, get_port_status, NULL, RX_LOS);
static SENSOR_DEVICE_ATTR(rxlos1, S_IRUGO, get_port_status, NULL, RX_LOS1);
static SENSOR_DEVICE_ATTR(rxlos2, S_IRUGO, get_port_status, NULL, RX_LOS2);
static SENSOR_DEVICE_ATTR(rxlos3, S_IRUGO, get_port_status, NULL, RX_LOS3);
static SENSOR_DEVICE_ATTR(rxlos4, S_IRUGO, get_port_status, NULL, RX_LOS4);
static SENSOR_DEVICE_ATTR(tx_disable, S_IWUSR | S_IRUGO, get_port_status, set_port_tx_status, TX_DISABLE);
static SENSOR_DEVICE_ATTR(tx_disable1, S_IWUSR | S_IRUGO, get_port_status, set_port_tx_status, TX_DISABLE1);
static SENSOR_DEVICE_ATTR(tx_disable2, S_IWUSR | S_IRUGO, get_port_status, set_port_tx_status, TX_DISABLE2);
static SENSOR_DEVICE_ATTR(tx_disable3, S_IWUSR | S_IRUGO, get_port_status, set_port_tx_status, TX_DISABLE3);
static SENSOR_DEVICE_ATTR(tx_disable4, S_IWUSR | S_IRUGO, get_port_status, set_port_tx_status, TX_DISABLE4);
static SENSOR_DEVICE_ATTR(tx_fault, S_IRUGO, get_port_status, NULL, TX_FAULT);
static SENSOR_DEVICE_ATTR(tx_fault1, S_IRUGO, get_port_status, NULL, TX_FAULT1);
static SENSOR_DEVICE_ATTR(tx_fault2, S_IRUGO, get_port_status, NULL, TX_FAULT2);
static SENSOR_DEVICE_ATTR(tx_fault3, S_IRUGO, get_port_status, NULL, TX_FAULT3);
static SENSOR_DEVICE_ATTR(tx_fault4, S_IRUGO, get_port_status, NULL, TX_FAULT4);
static SENSOR_DEVICE_ATTR(data_a0, S_IRUGO, get_port_status, NULL, EEPROM_A0_PAGE);
static SENSOR_DEVICE_ATTR(data_a2, S_IRUGO, get_port_status, NULL, EEPROM_A2_PAGE);
static SENSOR_DEVICE_ATTR(sfp_copper, S_IWUSR | S_IRUGO, get_port_status, set_port_sfp_copper, SFP_COPPER);

static struct attribute *sfp_attributes[] = {
    &sensor_dev_attr_abs.dev_attr.attr,
    &sensor_dev_attr_rxlos.dev_attr.attr,
    &sensor_dev_attr_rxlos1.dev_attr.attr,
    &sensor_dev_attr_rxlos2.dev_attr.attr,
    &sensor_dev_attr_rxlos3.dev_attr.attr,
    &sensor_dev_attr_rxlos4.dev_attr.attr,
    &sensor_dev_attr_tx_disable.dev_attr.attr,
    &sensor_dev_attr_tx_disable1.dev_attr.attr,
    &sensor_dev_attr_tx_disable2.dev_attr.attr,
    &sensor_dev_attr_tx_disable3.dev_attr.attr,
    &sensor_dev_attr_tx_disable4.dev_attr.attr,
    &sensor_dev_attr_tx_fault.dev_attr.attr,
    &sensor_dev_attr_tx_fault1.dev_attr.attr,
    &sensor_dev_attr_tx_fault2.dev_attr.attr,
    &sensor_dev_attr_tx_fault3.dev_attr.attr,
    &sensor_dev_attr_tx_fault4.dev_attr.attr,
    &sensor_dev_attr_data_a0.dev_attr.attr,
    &sensor_dev_attr_data_a2.dev_attr.attr,
    &sensor_dev_attr_sfp_copper.dev_attr.attr,
    NULL
};

static const struct attribute_group sfp_group = {
    .attrs = sfp_attributes,
};

static struct i2c_client *sfpPortDeviceCreate(struct i2c_adapter *adap, int port, const char *sfpType)
{
    struct i2c_client *client = NULL;
    int status;

    client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
    if (!client)
        return NULL;

    client->adapter = adap;
    client->addr = (port + 1);
    sprintf(client->name, "%s_%03d", sfpType, (port + 1));
    client->dev.parent = &client->adapter->dev;
    dev_set_name(&client->dev, "port_%03d", (port + 1));
    status = device_register(&client->dev);
    /* Register sysfs hooks */
    status = sysfs_create_group(&client->dev.kobj, &sfp_group);
    return client;
}

static void i2c_bus0_devices_client_address_init(struct i2c_client *client)
{
    int index;

    pca9535pwr_client_bus0 = *client;
    pca9535pwr_client_bus0.addr = 0x27;

    cpld_client = *client;
    cpld_client.addr = 0x33;

    pca9548_client_bus0 = *client;
    pca9548_client_bus0.addr = 0x70;

    for (index=0; index<4; index++)
    {
        pca9535_client_bus0[index] = *client;
        pca9535_client_bus0[index].addr = (0x20+index);
    }

    eeprom_client_bus0 = *client;
    eeprom_client_bus0.addr = 0x56;

    mp2953agu_client = *client;
    mp2953agu_client.addr = 0x21;

    chl8325a_client = *client;
    chl8325a_client.addr = 0x32;

    psu_eeprom_client_bus0= *client;
    psu_eeprom_client_bus0.addr = 0x51;

    psu_mcu_client_bus0= *client;
    psu_mcu_client_bus0.addr = 0x59;
}

static void i2c_bus0_hardware_monitor_hw_default_config(struct i2c_client *client,
       const struct i2c_device_id *id)
{
    struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
    unsigned int hiByte, lowByte, configByte;
    int i;

    i2c_bus0_devices_client_address_init(client);

    mutex_lock(&data->lock);

    /* Get Board Type and Revision */
    lowByte = i2c_smbus_read_byte_data(&cpld_client, CPLD_REG_GENERAL_0x00);
    data->buildRev = (lowByte&0x03);
    data->hwRev = ((lowByte>>2)&0x03);
    data->modelId = ((lowByte>>4)&0x0f);

    platformBuildRev = data->buildRev;
    platformHwRev = data->hwRev;
    platformModelId = data->modelId;

    switch(data->modelId)
    {
        case HURACAN_WITH_BMC: /* 0000: Huracan with BMC */
        case CABRERAIII_WITH_BMC: /* 0010: Cabrera3 with BMC */
        case SESTO_WITH_BMC: /* 0100: Sesto with BMC */
        case NCIIX_WITH_BMC: /* 0110: New Cabrera-II X with BMC */
        case ASTERION_WITH_BMC: /* 1000: Asterion with BMC */
        case HURACAN_A_WITH_BMC: /* 1010: Huracan-A with BMC */
            isBMCSupport = 1;
            break;

        default:
            isBMCSupport = 0;
            break;
    }

    if (isBMCSupport == 0)
    {
        /* Choose W83795ADG bank 0 */
        i2c_smbus_write_byte_data(client, W83795ADG_REG_BANK, 0x00);
        /* Disable monitoring operations */
        configByte = i2c_smbus_read_byte_data(client, W83795ADG_REG_CONFIG);
        configByte &= 0xfe;
        i2c_smbus_write_byte_data(client, W83795ADG_REG_CONFIG, configByte);

        /* Choose W83795ADG bank 2 */
        i2c_smbus_write_byte_data(client, W83795ADG_REG_BANK, 0x02);
        lowByte = i2c_smbus_read_byte_data(client, W83795ADG_REG_VENDOR_ID);
        i2c_smbus_write_byte_data(client, W83795ADG_REG_BANK, 0x82);
        hiByte = i2c_smbus_read_byte_data(client, W83795ADG_REG_VENDOR_ID);
        /* Get vender id */
        data->venderId = (hiByte<<8) + lowByte;
        /* Get chip id */
        data->chipId= i2c_smbus_read_byte_data(client, W83795ADG_REG_CHIP_ID);
        /* Get device id */
        data->dviceId= i2c_smbus_read_byte_data(client, W83795ADG_REG_DEVICE_ID);

        /* set FANCTL8 - FANCTL1 output mode control to PWM output duty cycle mode. */
        i2c_smbus_write_byte_data(client, W83795ADG_REG_FOMC, 0x00);
        i2c_smbus_write_byte_data(client, W83795ADG_REG_F1OV, 0xff);
        i2c_smbus_write_byte_data(client, W83795ADG_REG_F2OV, 0xff);

        /* Choose W83795ADG bank 0 */
        i2c_smbus_write_byte_data(client, W83795ADG_REG_BANK, 0x00);
        /* Enable TR1~TR4 thermistor temperature monitoring */
        i2c_smbus_write_byte_data(client, W83795ADG_REG_TEMP_CTRL2, 0xff);

        /* set FANCTL2 to enable FANIN9 and FANIN10 monitoring */
        i2c_smbus_write_byte_data(client, W83795ADG_REG_FANIN_CTRL2, 0x03);

        /* Enable monitoring operations */
        configByte |= 0x01;
        i2c_smbus_write_byte_data(client, W83795ADG_REG_CONFIG, configByte);
    }

    /* CPLD Revision */
    lowByte = i2c_smbus_read_byte_data(&cpld_client, CPLD_REG_GENERAL_0x01);
    data->cpldRev = (lowByte&0x3f);
    data->cpldRel = ((lowByte>>6)&0x01);

    /* turn on all LEDs of front port */
    i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x34, 0x10);

    switch(data->modelId)
    {
        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
            /* Turn on PCA9548#0 channel 3 on I2C-bus0 */
            i2c_smbus_write_byte(&pca9548_client_bus0, (1<<PCA9548_CH03));
            /* set input-1/output-0 mode for IO expander #0 on channel 3 : SFP+ 0-1 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #1 on channel 3 : SFP+ 2-3 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #2 on channel 3 : SFP+ 4-5 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #3 on channel 3 : SFP+ 6-7 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            for (i=0; i<4; i++)
            {
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xf1c7);
            }

            /* Turn on PCA9548#0 channel 4 on I2C-bus0 */
            i2c_smbus_write_byte(&pca9548_client_bus0, (1<<PCA9548_CH04));
            /* set input-1/output-0 mode for IO expander #4 on channel 4 : SFP+ 8-9 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #5 on channel 4 : SFP+ 10-11 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #6 on channel 4 : SFP+ 12-13 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #7 on channel 4 : SFP+ 14-15 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            for (i=0; i<4; i++)
            {
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xf1c7);
            }

            /* Turn on PCA9548#0 channel 5 on I2C-bus0 */
            i2c_smbus_write_byte(&pca9548_client_bus0, (1<<PCA9548_CH05));
            /* set input-1/output-0 mode for IO expander #8 on channel 5 : SFP+ 16-17 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #9 on channel 5 : SFP+ 18-19 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #10 on channel 5 : SFP+ 20-21 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #11 on channel 5 : SFP+ 22-23 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            for (i=0; i<4; i++)
            {
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xf1c7);
            }

            /* Turn on PCA9548#0 channel 6 on I2C-bus0 */
            i2c_smbus_write_byte(&pca9548_client_bus0, (1<<PCA9548_CH06));
            /* set input-1/output-0 mode for IO expander #12 on channel 6 : SFP+ 24-25 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #13 on channel 6 : SFP+ 26-27 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #14 on channel 6 : SFP+ 28-29 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #15 on channel 6 : SFP+ 30-31 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            for (i=0; i<4; i++)
            {
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xf1c7);
            }

            /* Turn on PCA9548#0 channel 7 on I2C-bus0 */
            i2c_smbus_write_byte(&pca9548_client_bus0, (1<<PCA9548_CH07));
            /* set input-1/output-0 mode for IO expander #16 on channel 7 : SFP+ 32-33 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #17 on channel 7 : SFP+ 34-35 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #18 on channel 7 : SFP+ 36-37 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            /* set input-1/output-0 mode for IO expander #19 on channel 7 : SFP+ 38-39 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            for (i=0; i<4; i++)
            {
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535_client_bus0[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xf1c7);
            }

            /* Turn off PCA9548#0 all channels on I2C-bus0*/
            i2c_smbus_write_byte(&pca9548_client_bus0, 0x00);
            break;

        default:
            /* set default value for IO expander #0 */
            i2c_smbus_write_word_data(&pca9535pwr_client_bus0, PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
            /* set input-1/output-0 mode for IO expander #0 */
            i2c_smbus_write_word_data(&pca9535pwr_client_bus0, PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
            i2c_smbus_write_word_data(&pca9535pwr_client_bus0, PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xff7f);
            break;
    }

    mutex_unlock(&data->lock);
}

static void i2c_bus1_devices_client_address_init(struct i2c_client *client)
{
    int index;

    for (index=0; index<4; index++)
    {
        pca9548_client[index] = *client;
        pca9548_client[index].addr = (0x71+index);
    }

    for (index=0; index<6; index++)
    {
        pca9535pwr_client[index] = *client;
        pca9535pwr_client[index].addr = (0x20+index);
    }

    cpld_client_bus1 = *client;
    cpld_client_bus1.addr = 0x33;

    qsfpDataA0_client = *client;
    qsfpDataA0_client.addr = 0x50;

    qsfpDataA2_client = *client;
    qsfpDataA2_client.addr = 0x51;

    SfpCopperData_client = *client;
    SfpCopperData_client.addr = 0x56;

    switch(platformModelId)
    {
        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
            eeprom_client = *client;
            eeprom_client.addr = 0x56;

            psu_eeprom_client = *client;
            psu_eeprom_client.addr = 0x51;

            psu_mcu_client = *client;
            psu_mcu_client.addr = 0x59;
            break;

        default:
            eeprom_client = *client;
            eeprom_client.addr = 0x54;

            psu_eeprom_client = *client;
            psu_eeprom_client.addr = 0x50;

            psu_mcu_client = *client;
            psu_mcu_client.addr = 0x58;
            break;
    }
 }

static void i2c_bus1_io_expander_default_set(struct i2c_client *client)
{
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    int i;

    switch (platformModelId)
    {
        case HURACAN_WITH_BMC:
        case HURACAN_WITHOUT_BMC:
        case HURACAN_A_WITH_BMC:
        case HURACAN_A_WITHOUT_BMC:
            /* Turn on PCA9548 channel 4 on I2C-bus1 */
            i2c_smbus_write_byte(client, (1<<PCA9548_CH04));
            /* set input-1/output-0 mode for IO expander #1-4 on channel 4 */
            for (i=0; i<4; i++)
            {
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xffff);
            }

            /* Turn on PCA9548 channel 5 on I2C-bus1 */
            i2c_smbus_write_byte(client, (1<<PCA9548_CH05));

            /*LPMODE(Low Power Mode) = 0 */
            /* set input-1/output-0 mode for IO expander #1-2 on channel 5 */
            for (i=0; i<2; i++)
            {
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0x0000);
            }

            /* RST#(Module Reset) = 1 */
            /* set input-1/output-0 mode for IO expander #3-4 on channel 5 */
            for (i=2; i<4; i++)
            {
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0xffff);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0x0000);
            }

            /* MODSEL# (Module Select) = 0 */
            /* set input-1/output-0 mode for IO expander #5-6 on channel 5 */
            for (i=4; i<6; i++)
            {
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0x0000);
            }

            if (isBMCSupport == 0)
            {
                /* Turn on PCA9548 channel 6 on I2C-bus1 */
                i2c_smbus_write_byte(client, (1<<PCA9548_CH06));
                /* PSU Status */
                /* set input-1/output-0 mode for IO expander #1 on channel 6 */
                i2c_device_word_write(&(pca9535pwr_client[3]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[3]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
/* If the PSU_PWROFF pin of IO expander is output mode, the power cycling of CPLD cannot work.*/
#if 0
                i2c_device_word_write(&(pca9535pwr_client[3]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xffbb);
#else
                i2c_device_word_write(&(pca9535pwr_client[3]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xffff);
#endif

                /* Turn on PCA9548 channel 7 on I2C-bus1 */
                i2c_smbus_write_byte(client, (1<<PCA9548_CH07));
                /* FAN Status */
                /* set input-1/output-0 mode for IO expander #1 on channel 7 */
                i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0xeeee);
                i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xcccc);
                if ( (platformHwRev == 0x03) || /* PVT */
                         (platformModelId == HURACAN_A_WITH_BMC)||(platformModelId == HURACAN_A_WITHOUT_BMC) )
                {
                    /* Front Pannel LED Status */
                    /* set input-1/output-0 mode for IO expander #2 on channel 7 */
                    i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->frontLedStatus);
                    i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                    i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0x0000);
                }
            }

            /* Turn off PCA9548 all channels on I2C-bus1 */
            i2c_smbus_write_byte(client, 0x00);
            break;

        case CABRERAIII_WITH_BMC:
        case CABRERAIII_WITHOUT_BMC:
            break;

        case SESTO_WITH_BMC:
        case SESTO_WITHOUT_BMC:
             /* Turn on PCA9548#1 channel 0 on I2C-bus1 - ABS# */
            i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH00));
            /* set input-1/output-0 mode for IO expander #1-4 on channel 0 */
            for (i=0; i<4; i++)
            {
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xffff);
            }

             /* Turn on PCA9548#1 channel 1 on I2C-bus1 - RXLOS */
            i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH01));
            /* set input-1/output-0 mode for IO expander #1-3 on channel 1 */
            for (i=0; i<3; i++)
            {
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xffff);
            }

            /* Turn on PCA9548#1 channel 2 on I2C-bus1 - TXFAULT */
            i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH02));
            /* set input-1/output-0 mode for IO expander #1-3 on channel 2 */
            for (i=0; i<3; i++)
            {
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xffff);
            }

            /* Turn on PCA9548#1 channel 3 on I2C-bus1 - TX_RS  = 1, LPMODE = 0, MODSEL = 0 */
            i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH03));
            /* set input-1/output-0 mode for IO expander #1-4 on channel 3 */
            for (i=0; i<3; i++)
            {
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0xffff);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0x0000);
            }
            i2c_device_word_write(&(pca9535pwr_client[3]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
            i2c_device_word_write(&(pca9535pwr_client[3]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
            i2c_device_word_write(&(pca9535pwr_client[3]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0x0000);

            /* Turn on PCA9548#1 channel 4 on I2C-bus1 - RX _RS = 1 */
            i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH04));
            /* set input-1/output-0 mode for IO expander #1-3 on channel 4 */
            for (i=0; i<3; i++)
            {
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0xffff);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0x0000);
            }

            /* Turn on PCA9548#1 channel 5 on I2C-bus1 - TXEN = 0, RST = 1*/
            i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH05));
            /* set input-1/output-0 mode for IO expander #1-3 on channel 5 */
            for (i=0; i<3; i++)
            {
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0x0000);
            }
            /* set input-1/output-0 mode for IO expander #4 on channel 5 */
            i2c_device_word_write(&(pca9535pwr_client[3]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0xffff);
            i2c_device_word_write(&(pca9535pwr_client[3]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
            i2c_device_word_write(&(pca9535pwr_client[3]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0x0000);

            if (isBMCSupport == 0)
            {
                /* Turn on PCA9548#1 channel 6 on I2C-bus1 */
                i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH06));
                /* PSU Status */
                /* set input-1/output-0 mode for IO expander #1 on channel 6 */
                i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
/* If the PSU_PWROFF pin of IO expander is output mode, the power cycling of CPLD cannot work.*/
#if 0
                i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xffbb);
#else
                i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xffff);
#endif

                /* Turn on PCA9548#1 channel 7 on I2C-bus1 */
                i2c_smbus_write_byte(&(pca9548_client[1]), (1<<PCA9548_CH07));
                /* FAN Status */
                /* set input-1/output-0 mode for IO expander #1 on channel 7 */
                i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0xeeee);
                i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xcccc);

                /* Turn on PCA9548#0 channel 7 on I2C-bus1 */
                i2c_smbus_write_byte(client, (1<<PCA9548_CH07));
                /* Front Pannel LED Status */
                /* set input-1/output-0 mode for IO expander #2 on channel 7 */
                i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->frontLedStatus);
                i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0x0000);
                /* Turn off PCA9548#0 all channels on I2C-bus1 */
                i2c_smbus_write_byte(client, 0x00);
            }

            /* Turn off PCA9548#1 all channels on I2C-bus1 */
            i2c_smbus_write_byte(&(pca9548_client[1]), 0x00);
            break;

        case NCIIX_WITH_BMC:
        case NCIIX_WITHOUT_BMC:
            /* Turn on PCA9548#1 channel 0 on I2C-bus1 */
            i2c_smbus_write_byte(client, (1<<PCA9548_CH00));
            /* set input-1/output-0 mode for IO expander #20-23 on channel 0 : SFP+ 40-47 : TXEN = 0, RX_RS = 0, TX_RS = 0 */
            for (i=0; i<4; i++)
            {
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xf1c7);
            }

            /* Turn on PCA9548#1 channel 1 on I2C-bus1 */
            i2c_smbus_write_byte(client, (1<<PCA9548_CH01));
            /* set input-1/output-0 mode for IO expander #26 on channel 1 : LED Board */
            i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
            i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
            i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0x0000);

             /* Turn on PCA9548#1 channel 3 on I2C-bus1 */
            i2c_smbus_write_byte(client, (1<<PCA9548_CH03));
            /* set input-1/output-0 mode for IO expander #24 on channel 3 : QSFP 1, 2, 4 : MODSEL = 0, RST = 1, LPMODE = 0 */
            /* set input-1/output-0 mode for IO expander #25 on channel 3 : QSFP 3, 5, 6 : MODSEL = 0, RST = 1, LPMODE = 0 */
            for (i=0; i<2; i++)
            {
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0842);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
                i2c_device_word_write(&(pca9535pwr_client[i]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xe318);
            }

            /* Turn on PCA9548#1 channel 7 on I2C-bus1 */
            i2c_smbus_write_byte(client, (1<<PCA9548_CH07));
            /* set input-1/output-0 mode for IO expander #27 on channel 7 : FAN Board */
            i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
            i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
            i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xcccc);

            /* Turn off PCA9548 all channels on I2C-bus1 */
            i2c_smbus_write_byte(client, 0x00);
            break;

        case ASTERION_WITH_BMC:
        case ASTERION_WITHOUT_BMC:
            /* Turn on PCA9548#0 channel 3 on I2C-bus1 */
            i2c_smbus_write_byte(client, (1 << PCA9548_CH03));
            /* FAN Status */
            /* set input-1/output-0 mode for IO expander #1 on channel 3 */
            i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0xdddd);
            i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
            i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xcccc);

            i2c_device_word_write(&(pca9535pwr_client[1]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0xffdf);
            i2c_device_word_write(&(pca9535pwr_client[1]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
            i2c_device_word_write(&(pca9535pwr_client[1]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xffcf);

            /* Turn on PCA9548#0 channel 4 on I2C-bus1 */
            i2c_smbus_write_byte(client, (1 << PCA9548_CH04));
            /* Front Panel LED Status */
            /* set input-1/output-0 mode for IO expander #1 on channel 4 */

            i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, data->frontLedStatus);
            i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
            i2c_device_word_write(&(pca9535pwr_client[2]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0x0000);

            /* Turn on PCA9548#0 channel 5 on I2C-bus1 */
            i2c_smbus_write_byte(client, (1 << PCA9548_CH05));
            /* PSU Status */
            /* set input-1/output-0 mode for IO expander #1 on channel 5 */
            i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_OUTPUT_PORT_0, 0x0000);
            i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_POLARITY_INVERSION_0, 0x0000);
/* If the PSU_PWROFF pin of IO expander is output mode, the power cycling of CPLD cannot work.*/
#if 0
            i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xffbb);
#else
            i2c_device_word_write(&(pca9535pwr_client[0]), PCA9553_COMMAND_BYTE_REG_CONFIGURATION_0, 0xffff);
#endif

            /* Turn off PCA9548#0 all channels on I2C-bus1 */
            i2c_smbus_write_byte(client, 0x00);
            break;

        default:
            break;
     }
 }

static void i2c_bus1_hardware_monitor_hw_default_config(struct i2c_client *client,
       const struct i2c_device_id *id)
{
    struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
    int i;

    i2c_bus1_devices_client_address_init(client);

    mutex_lock(&data->lock);
    /* Turn off PCA9548 all channels on I2C-bus1 */
    i2c_smbus_write_byte(client, 0x00);

    data->frontLedStatus = 0x00aa;
    for (i=0; i<3; i++)
        data->sfpPortRateSelect[i] = 0xffff;
    i2c_bus1_io_expander_default_set(client);

    mutex_unlock(&data->lock);
}

/* Return 0 if detection is successful, -ENODEV otherwise */
static int w83795adg_hardware_monitor_detect(struct i2c_client *client,
        struct i2c_board_info *info)
{
    struct i2c_adapter *adapter = client->adapter;

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
    {
        printk(KERN_ERR "i2c_check_functionality fail.\n");
        return -ENODEV;
    }

    if(adapter->nr == 0x0)
    {
        unsigned int hiByte, lowByte, value;

        if  (client->addr != 0x2F)
            return -ENODEV;

        /* Choose W83795ADG bank 2 */
        i2c_smbus_write_byte_data(client, W83795ADG_REG_BANK, 0x02);
        lowByte = i2c_smbus_read_byte_data(client, W83795ADG_REG_VENDOR_ID);
        i2c_smbus_write_byte_data(client, W83795ADG_REG_BANK, 0x82);
        hiByte = i2c_smbus_read_byte_data(client, W83795ADG_REG_VENDOR_ID);
        /* Get vender id */
        value= (hiByte<<8) + lowByte;
        if (value != W83795ADG_VENDOR_ID)
        {
            printk(KERN_ERR "%s(%d): W83795ADG_REG_VENDOR_ID  fail.\n", __func__, __LINE__);
            return -ENODEV;
        }

        value = i2c_smbus_read_byte_data(client, W83795ADG_REG_CHIP_ID);
        if (value != W83795ADG_CHIP_ID)
        {
            printk(KERN_ERR "%s(%d): W83795ADG_REG_CHIP_ID  fail.\n", __func__, __LINE__);
            return -ENODEV;
        }
    }

    strlcpy(info->type, "HURACAN", I2C_NAME_SIZE);
    return 0;
}

static int w83795adg_hardware_monitor_probe(struct i2c_client *client,
       const struct i2c_device_id *id)
{
    int err = 0;

    if(client->adapter->nr == 0x0)
    {
        struct i2c_bus0_hardware_monitor_data *data = NULL;

        if  (client->addr != 0x2F)
            return -ENODEV;

        data = devm_kzalloc(&client->dev, sizeof(struct i2c_bus0_hardware_monitor_data), GFP_KERNEL);
        if (!data)
          return -ENOMEM;

        memset(data, 0, sizeof(struct i2c_bus0_hardware_monitor_data));
        mutex_init(&data->lock);
        i2c_set_clientdata(client, data);

        dev_info(&client->dev, "%s device found on bus %d\n", client->name, client->adapter->nr);

        /* Set Pre-defined HW config */
        i2c_bus0_hardware_monitor_hw_default_config(client, id);
        /* Register sysfs hooks */
        switch (platformModelId)
        {
            case NCIIX_WITH_BMC:
            case NCIIX_WITHOUT_BMC:
                data->hwmon_group.attrs = i2c_bus0_hardware_monitor_attr_nc2x;
                break;

            case ASTERION_WITH_BMC:
            case ASTERION_WITHOUT_BMC:
                data->hwmon_group.attrs = i2c_bus0_hardware_monitor_attr_asterion;
                w83795adg_normal_i2c[1] = 0x72;
                break;

            default:
                data->hwmon_group.attrs = i2c_bus0_hardware_monitor_attr;
                break;
        }
        err = sysfs_create_group(&client->dev.kobj, &data->hwmon_group);
        if (err)
        {
            printk(KERN_ERR "hwmon_group sysfs_create_group fail.\n");
        }
        else
        {
            data->hwmon_dev = hwmon_device_register(&client->dev);
            if (IS_ERR(data->hwmon_dev)) {
                printk(KERN_ERR "hwmon_device_register fail.\n");
                err = PTR_ERR(data->hwmon_dev);
                sysfs_remove_group(&client->dev.kobj, &data->hwmon_group);
            }

            init_completion(&data->auto_update_stop);
            data->auto_update = kthread_run(i2c_bus0_hardware_monitor_update_thread, client, dev_name(data->hwmon_dev));
            if (IS_ERR(data->auto_update)) {
                err = PTR_ERR(data->auto_update);
                hwmon_device_unregister(data->hwmon_dev);
                sysfs_remove_group(&client->dev.kobj, &data->hwmon_group);
            }
        }
    }
    else if(client->adapter->nr == 0x1)
    {
        struct i2c_bus1_hardware_monitor_data *data = NULL;

        data = devm_kzalloc(&client->dev, sizeof(struct i2c_bus1_hardware_monitor_data), GFP_KERNEL);
        if (!data)
            return -ENOMEM;

        memset(data, 0, sizeof(struct i2c_bus1_hardware_monitor_data));
        mutex_init(&data->lock);
        i2c_set_clientdata(client, data);

        dev_info(&client->dev, "%s device found on bus %d\n", client->name, client->adapter->nr);

        /* Set Pre-defined HW config */
        i2c_bus1_hardware_monitor_hw_default_config(client, id);
        /* Register sysfs hooks */

        switch (platformModelId)
        {
            default:
            case HURACAN_WITH_BMC:
            case HURACAN_WITHOUT_BMC:
            case HURACAN_A_WITH_BMC:
            case HURACAN_A_WITHOUT_BMC:
                data->hwmon_group.attrs = i2c_bus1_hardware_monitor_attr_huracan;
                break;

            case CABRERAIII_WITH_BMC:
            case CABRERAIII_WITHOUT_BMC:
                break;

            case SESTO_WITH_BMC:
            case SESTO_WITHOUT_BMC:
                data->hwmon_group.attrs = i2c_bus1_hardware_monitor_attr_sesto;
                break;

            case NCIIX_WITH_BMC:
            case NCIIX_WITHOUT_BMC:
                data->hwmon_group.attrs = i2c_bus1_hardware_monitor_attr_nc2x;
                break;

            case ASTERION_WITH_BMC:
            case ASTERION_WITHOUT_BMC:
                data->hwmon_group.attrs = i2c_bus1_hardware_monitor_attr_asterion;
                break;
        }
        err = sysfs_create_group(&client->dev.kobj, &data->hwmon_group);
        if (err)
        {
            printk(KERN_INFO "hwmon_group1 sysfs_create_group fail.\n");
        }
        else
        {
            data->hwmon_dev = hwmon_device_register(&client->dev);
            if (IS_ERR(data->hwmon_dev)) {
                printk(KERN_INFO "hwmon_device_register1 fail.\n");
                err = PTR_ERR(data->hwmon_dev);
                sysfs_remove_group(&client->dev.kobj, &data->hwmon_group);
            }
            else
            {
                struct i2c_adapter *adap = to_i2c_adapter(&client->dev);
                int port;

                for (port = 0; port < QSFP_COUNT; port++)
                    data->qsfpPortTxDisableDataUpdate[port] = 1;
                mutex_init(&portStatusLock);
                switch (platformModelId)
                {
                    case HURACAN_WITH_BMC:
                    case HURACAN_WITHOUT_BMC:
                    case HURACAN_A_WITH_BMC:
                    case HURACAN_A_WITHOUT_BMC:
                        /* QSFP ports */
                        for (port = 0; port < 32; port++)
                        {
                            data->sfpPortClient[port] = sfpPortDeviceCreate(adap, port, "qsfp");
                            if (!data->sfpPortClient[port])
                                return -ENOMEM;
                        }
                        break;

                    case SESTO_WITH_BMC:
                    case SESTO_WITHOUT_BMC:
                    case NCIIX_WITH_BMC:
                    case NCIIX_WITHOUT_BMC:
                        /* SFP+ ports */
                        for (port = 0; port < 48; port++)
                        {
                            data->sfpPortClient[port] = sfpPortDeviceCreate(adap, port, "sfp");
                            if (!data->sfpPortClient[port])
                                return -ENOMEM;
                        }
                        /* QSFP ports */
                        for (port = 48; port < 54; port++)
                        {
                            data->sfpPortClient[port] = sfpPortDeviceCreate(adap, port, "qsfp");
                            if (!data->sfpPortClient[port])
                                return -ENOMEM;
                        }
                        break;

                    case ASTERION_WITH_BMC:
                    case ASTERION_WITHOUT_BMC:
                        /* SFP+ ports */
                        for (port = 0; port < 48; port++)
                        {
                            data->sfpPortClient[port] = sfpPortDeviceCreate(adap, port, "sfp");
                            if (!data->sfpPortClient[port])
                                return -ENOMEM;
                        }
                        /* QSFP ports */
                        for (port = 48; port < 64; port++)
                        {
                            data->sfpPortClient[port] = sfpPortDeviceCreate(adap, port, "qsfp");
                            if (!data->sfpPortClient[port])
                                return -ENOMEM;
                        }
                        break;

                    default:
                        break;
                }

                init_completion(&data->auto_update_stop);
                data->auto_update = kthread_run(i2c_bus1_hardware_monitor_update_thread, client, dev_name(data->hwmon_dev));
                if (IS_ERR(data->auto_update)) {
                    err = PTR_ERR(data->auto_update);

                    hwmon_device_unregister(data->hwmon_dev);
                    sysfs_remove_group(&client->dev.kobj, &data->hwmon_group);
                }
            }
        }
    }

    return err;
}

static int w83795adg_hardware_monitor_remove(struct i2c_client *client)
{
    if(client->adapter->nr == 0x0)
    {
        struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
        kthread_stop(data->auto_update);
        wait_for_completion(&data->auto_update_stop);
        /* Watchdog Control Register Support */
        if (data->cpldRev != 0)
        {
            /* Disable WD function */
            i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_GENERAL_0x06, 0x00);
        }

        /* turn off all LEDs of front port */
        i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x34, 0x00);
#if 0 /* It's for Huracan Beta only,  remove it. */
        i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_LED_0x40, 0x00);
        i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_LED_0x44, 0x00);
#endif

        mutex_destroy(&client->dev.mutex);
        hwmon_device_unregister(data->hwmon_dev);
        sysfs_remove_group(&client->dev.kobj, &data->hwmon_group);
        mutex_destroy(&data->lock);
    }
    else if(client->adapter->nr == 0x1)
    {
        int port;
        struct i2c_client *c;
        struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
        kthread_stop(data->auto_update);
        wait_for_completion(&data->auto_update_stop);
        for (port = 0; port < QSFP_COUNT; port ++)
        {
            c = data->sfpPortClient[port];
            if (c)
            {
                sysfs_remove_group(&c->dev.kobj, &sfp_group);
                mutex_destroy(&c->dev.mutex);
                device_del(&c->dev);
                kfree(c);
            }
        }
        mutex_destroy(&portStatusLock);

        mutex_destroy(&client->dev.mutex);
        hwmon_device_unregister(data->hwmon_dev);
        sysfs_remove_group(&client->dev.kobj, &data->hwmon_group);
        mutex_destroy(&data->lock);
    }
    return 0;
}

static void w83795adg_hardware_monitor_shutdown(struct i2c_client *client)
{
    if(client->adapter->nr == 0x0)
    {
        struct i2c_bus0_hardware_monitor_data *data = i2c_get_clientdata(client);
        kthread_stop(data->auto_update);
        wait_for_completion(&data->auto_update_stop);
        /* Watchdog Control Register Support */
        if (data->cpldRev != 0)
        {
            /* Disable WD function */
            i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_GENERAL_0x06, 0x00);
        }

        /* turn off all LEDs of front port */
        i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x34, 0x00);
#if 0 /* It's for Huracan Beta only,  remove it. */
        i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_LED_0x40, 0x00);
        i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_LED_0x44, 0x00);
#endif
        /* reset MAC */
        switch(platformModelId)
        {
            case ASTERION_WITH_BMC:
            case ASTERION_WITHOUT_BMC:
                i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x30, 0x3e);
                i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x30, 0x3f);
                /* reset CPLD 2, 3 and 4 */
                i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x35, 0xfd); /* assert RST_CPLD2_3_4 */
                i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x35, 0xff);
                break;

            default:
                i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x30, 0x6e);
                i2c_smbus_write_byte_data(&cpld_client, CPLD_REG_RESET_0x30, 0x6f);
                break;
        }

        hwmon_device_unregister(data->hwmon_dev);
        sysfs_remove_group(&client->dev.kobj, &data->hwmon_group);
    }
    else if(client->adapter->nr == 0x1)
    {
        struct i2c_bus1_hardware_monitor_data *data = i2c_get_clientdata(client);
        kthread_stop(data->auto_update);
        wait_for_completion(&data->auto_update_stop);
        hwmon_device_unregister(data->hwmon_dev);
        sysfs_remove_group(&client->dev.kobj, &data->hwmon_group);
    }
}

module_i2c_driver(w83795adg_hardware_monitor_driver);

MODULE_AUTHOR("Raymond Huey <raymond.huey@gmail.com>");
MODULE_DESCRIPTION("W83795ADG Hardware Monitor driver");
MODULE_LICENSE("GPL");
