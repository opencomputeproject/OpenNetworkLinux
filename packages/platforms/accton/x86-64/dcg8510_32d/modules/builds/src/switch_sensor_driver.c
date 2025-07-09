#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/i2c.h>

#include "switch_sensor_driver.h"
#include "switch_coretemp.h"
#include "switch_lm75.h"
#include "pmbus.h"
#include "sysfs_ipmi.h"

#define DRVNAME "drv_sensor_driver"
#define SWITCH_SENSOR_DRIVER_VERSION "0.0.1"

#define ipmitool_raw_get_in_input       1
#define ipmitool_raw_get_curr_input     2

unsigned int loglevel = 0;
extern unsigned int loglevel_bmc;
static struct platform_device *drv_sensor_device;

static temp_sensor_node_t temp_sensor_nodes[TEMP_TOTAL_NUM] = {
    {0,     0x00,   107000,  0,       -10000,  "MAC_PIPE_Temp",             "LSW"},
    {0,     0x00,   99000,   0,       -30000,  "CPU_core0_temp",            "core"},
    {0,     0x00,   99000,   0,       -30000,  "CPU_core1_temp",            "core"},
    {0,     0x00,   99000,   0,       -30000,  "CPU_core2_temp",            "core"},
    {0,     0x00,   99000,   0,       -30000,  "CPU_core3_temp",            "core"},
    {0,     0x00,   99000,   0,       -30000,  "CPU_package_temp",          "core"},
    {1,     0x48,   56000,   5000,    -10000,  "TMP75_48_TEMP",             "tmp75_48"},
    {1,     0x49,   56000,   5000,    -10000,  "TMP75_49_TEMP",             "tmp75_49"},
    {1,     0x4a,   80000,   5000,    -10000,  "TMP75_4A_TEMP",             "tmp75_4a"},
    {1,     0x4b,   76000,   5000,    -10000,  "TMP75_4B_TEMP",             "tmp75_4b"},
    {1,     0x4c,   76000,   5000,    -10000,  "TMP431_4C_TEMP1",           "tmp431_4c1"},
    {1,     0x4c,   100000,  5000,    -10000,  "TMP431_4C_TEMP2",           "tmp431_4c2"},
    {1,     0x4e,   73000,   5000,    -10000,  "TMP75_4E_TEMP",             "tmp75_4e"},
    {2,     0x4d,   73000,   5000,    -10000,  "TMP275_4D_TEMP",            "tmp275_4d"},
    {2,     0x4e,   72000,   5000,    -10000,  "TMP275_4E_TEMP",            "tmp275_4e"},
    {5,     0x20,   150000,  0,       -30000,  "VCORE_MAC_TEMP",            "mp2973_20_temp"},
    {5,     0x40,   150000,  0,       -30000,  "VDDT0V9_R_MAC_TEMP",        "mp2940b_40_temp"},
    {5,     0x60,   150000,  0,       -30000,  "VDD1V5_1V8_MAC_TEMP",       "mp2976_60_temp"},
    {5,     0x50,   150000,  0,       -30000,  "VCC3V3_QSFP_AB_TEMP",       "mp2976_50_temp"},
    {5,     0x5f,   150000,  0,       -30000,  "VCC3V3_QSFP_CD_TEMP",       "mp2976_5f_temp"},
};

static vr_sensor_node_t vr_sensor_vol_nodes[VOL_TOTAL_NUM] = {
    {UCD90320,          CHANNEL_FIRST,          VR_VIN,   13200, 10800,  12000,  0,  "M_V12V_PSU_VOUT",            "UCD90320",     "in1_input"},
    {UCD90320,          CHANNEL_SECORND,        VR_VIN,   13200, 10800,  12000,  0,  "M_V12V_COME_VOUT",           "UCD90320",     "in1_input"},
    {UCD90320,          CHANNEL_THIRD,          VR_VIN,   13200, 10800,  12000,  0,  "M_V12V_MB_VOUT",             "UCD90320",     "in1_input"},
    {UCD90320,          CHANNEL_FORTH,          VR_VIN,   5500,  4500,   5000,   0,  "M_V5V_SB_VOUT",              "UCD90320",     "in2_input"},
    {UCD90320,          CHANNEL_FIFTH,          VR_VIN,   3630,  2970,   3300,   0,  "M_3V3SB_PSU_VOUT",           "UCD90320",     "in3_input"},
    {UCD90320,          CHANNEL_SIXTH,          VR_VIN,   3630,  2970,   3300,   0,  "VOL_VCC3V3_SB_MON6",         "UCD90320",     "in1_input"},
    {UCD90320,          CHANNEL_SEVENTH,        VR_VIN,   3630,  2970,   3300,   0,  "M_V3V3_SB_VOUT",             "UCD90320",     "in2_input"},
    {UCD90320,          CHANNEL_EIGHTH,         VR_VIN,   3630,  2970,   3300,   0,  "M_3V3_QSFPA_VOUT",           "UCD90320",     "in3_input"},
    {UCD90320,          CHANNEL_NINTH,          VR_VIN,   3630,  2970,   3300,   0,  "M_3V3_QSFPB_VOUT",           "UCD90320",     "in1_input"},
    {UCD90320,          CHANNEL_TEHTH,          VR_VIN,   3630,  2970,   3300,   0,  "M_3V3_QSFPC_VOUT",           "UCD90320",     "in2_input"},
    {UCD90320,          CHANNEL_ELEVEN,         VR_VIN,   3630,  2970,   3300,   0,  "M_3V3_QSFPD_VOUT",           "UCD90320",     "in1_input"},
    {UCD90320,          CHANNEL_TWELVE,         VR_VIN,   1960,  1660,   1800,   0,  "M_VDDA_1V8_VOUT",            "UCD90320",     "in2_input"},
    {UCD90320,          CHANNEL_THIRTEEN,       VR_VIN,   1960,  1660,   1800,   0,  "M_VDD_1V8_VOUT",             "UCD90320",     "in3_input"},
    {UCD90320,          CHANNEL_FOURTEEN,       VR_VIN,   1630,  1380,   1500,   0,  "M_V1V5_VOUT",                "UCD90320",     "in1_input"},
    {UCD90320,          CHANNEL_FIFTEEN,        VR_VIN,   1630,  1380,   1500,   0,  "M_VDDA_1V5_VOUT",            "UCD90320",     "in2_input"},
    {UCD90320,          CHANNEL_SIXTEEN,        VR_VIN,   1300,  1100,   1200,   0,  "M_V1V2_VOUT",                "UCD90320",     "in3_input"},
    {UCD90320,          CHANNEL_SEVENTEEN,      VR_VIN,   1140,  970,    1050,   0,  "M_V1V05_VOUT",               "UCD90320",     "in1_input"},
    {UCD90320,          CHANNEL_EIGHTEEN,       VR_VIN,   1140,  970,    1050,   0,  "M_VDD_1V05_VOUT",            "UCD90320",     "in2_input"},
    {UCD90320,          CHANNEL_NINETEEN,       VR_VIN,   1090,  920,    1000,   0,  "M_VDDA_1V0_VOUT",            "UCD90320",     "in1_input"},
    {UCD90320,          CHANNEL_TWENTY,         VR_VIN,   1090,  920,    1000,   0,  "M_VDDT_1V0_VOUT",            "UCD90320",     "in2_input"},
    {UCD90320,          CHANNEL_TWENTY_ONE,     VR_VIN,   980,   830,    900,    0,  "M_VDDT_0V9_VOUT",            "UCD90320",     "in3_input"},
    {UCD90320,          CHANNEL_TWENTY_TWO,     VR_VIN,   820,   690,    750,    0,  "M_VDD_0V75_VOUT",            "UCD90320",     "in1_input"},
    {UCD90320,          CHANNEL_TWENTY_THREE,   VR_VIN,   950,   710,    12000,  0,  "M_VDD_CORE_VOUT",            "UCD90320",     "in2_input"},
    {LM25066,           CHANNEL_DEFAULT,        VR_VIN,   13200, 10800,  12000,  0,  "VCC12VIN_HOTSWAP_VIN",       "LM25066",      "in1_input"},
    {LM25066,           CHANNEL_DEFAULT,        VR_VOUT,   13200, 10800,  12000,  0,  "VCC12VIN_HOTSWAP_VOUT",      "LM25066",      "in1_input"},
    {MP2973,            CHANNEL_DEFAULT,        VR_VIN,   13200, 10800,  12000,  0,  "VCORE_MAC_VIN",              "MP2973",       "in1_input"},
    {MP2973,            CHANNEL_DEFAULT,        VR_VOUT,  950,   710,    0,      0,  "VCORE_MAC_VOUT",             "MP2973",       "in2_input"},
    {MP2940,            CHANNEL_DEFAULT,        VR_VIN,   13200, 10800,  12000,  0,  "VDDT0V9_R_MAC_VIN",          "MP2940",       "in3_input"},
    {MP2940,            CHANNEL_DEFAULT,        VR_VOUT,  885,   835,    860,    0,  "VDDT0V9_R_MAC_VOUT",         "MP2940",       "in1_input"},
    {MP2976_U1B1,       CHANNEL_DEFAULT,        VR_VIN,   13200, 10800,  12000,  0,  "VDD1V5_1V8_MAC_VIN",         "MP2976",       "in2_input"},
    {MP2976_U1B1,       CHANNEL_DEFAULT,        VR_VOUT,  1750,  1650,   1700,   0,  "VDDA1V5_MAC_VOUT",           "MP2976",       "in3_input"},
    {MP2976_U1B1,       CHANNEL_FIRST,          VR_VOUT,  1890,  1795,   1850,   0,  "VDDA1V8_MAC_VOUT",           "MP2976",       "in1_input"},
    {MP2976_U1J1,       CHANNEL_DEFAULT,        VR_VIN,   13200, 10800,  12000,  0,  "VCC3V3_QSFP_AB_VIN",         "MP2976",       "in2_input"},
    {MP2976_U1J1,       CHANNEL_DEFAULT,        VR_VOUT,  3630,  2790,   3300,   0,  "VCC3V3_QSFP_A_VOUT",         "MP2976",       "in1_input"},
    {MP2976_U1J1,       CHANNEL_FIRST,          VR_VOUT,  3630,  2790,   3300,   0,  "VCC3V3_QSFP_B_VOUT",         "MP2976",       "in2_input"},
    {MP2976_U1J2,       CHANNEL_DEFAULT,        VR_VIN,   20700, 0,      0,      0,  "VCC3V3_QSFP_CD_VIN",         "MP2976",       "in3_input"},
    {MP2976_U1J2,       CHANNEL_DEFAULT,        VR_VOUT,  37200, 0,      0,      0,  "VCC3V3_QSFP_C_VOUT",         "MP2976",       "in1_input"},
    {MP2976_U1J2,       CHANNEL_FIRST,          VR_VOUT,  37200, 0,      0,      0,  "VCC3V3_QSFP_D_VOUT",         "MP2976",       "in2_input"},
};

static vr_chip_info_t vr_chips[TOTAL_VR_CHIP_NUM] = {
    {UCD90320,      4,  0x11},
    {LM25066,       5,  0x41},
    {MP2973,        5,  0x20},
    {MP2940,        5,  0x40},
    {MP2976_U1B1,   5,  0x60},
    {MP2976_U1J1,   5,  0x50},
    {MP2976_U1J2,   5,  0x5f},
};


static vr_sensor_node_t vr_sensor_curr_nodes[CURR_TOTAL_NUM] = {
    {LM25066,       CHANNEL_DEFAULT,    VR_IIN,   23400,    0,  0,  0,  "VCC12VIN_HOTSWAP_IIN",     "LM25066",  "curr2_input"},
    {LM25066,       CHANNEL_DEFAULT,    VR_IOUT,  0,        0,  0,  0,  "VCC12VIN_HOTSWAP_IOUT",    "LM25066",  "curr3_input"},
    {MP2973,        CHANNEL_DEFAULT,    VR_IIN,   43800,    0,  0,  0,  "VCORE_MAC_IIN",            "MP2973",   "curr2_input"},
    {MP2973,        CHANNEL_DEFAULT,    VR_IOUT,  499200,   0,  0,  0,  "VCORE_MAC_IOUT",           "MP2973",   "curr3_input"},
    {MP2940,        CHANNEL_DEFAULT,    VR_IIN,   78900,    0,  0,  0,  "VDDT0V9_R_MAC_IOUT",       "MP2940",   "curr3_input"},
    {MP2976_U1B1,   CHANNEL_DEFAULT,    VR_IIN,   6000,     0,  0,  0,  "VDD1V5_1V8_MAC_IIN",       "MP2976",   "curr1_input"},
    {MP2976_U1B1,   CHANNEL_DEFAULT,    VR_IOUT,  37200,    0,  0,  0,  "VDDA1V5_MAC_IOUT",         "MP2976",   "curr2_label"},
    {MP2976_U1B1,   CHANNEL_FIRST,      VR_IOUT,  16800,    0,  0,  0,  "VDDA1V8_MAC_IOUT",         "MP2976",   "curr3_input"},
    {MP2976_U1J1,   CHANNEL_DEFAULT,    VR_IIN,   20700,    0,  0,  0,  "VCC3V3_QSFP_AB_IIN",       "MP2976",   "curr1_input"},
    {MP2976_U1J1,   CHANNEL_DEFAULT,    VR_IOUT,  37200,    0,  0,  0,  "VCC3V3_QSFP_A_IOUT",       "MP2976",   "curr2_label"},
    {MP2976_U1J1,   CHANNEL_FIRST,      VR_IOUT,  37200,    0,  0,  0,  "VCC3V3_QSFP_B_IOUT",       "MP2976",   "curr3_input"},
    {MP2976_U1J2,   CHANNEL_DEFAULT,    VR_IIN,   20700,    0,  0,  0,  "VCC3V3_QSFP_CD_IIN",       "MP2976",   "curr1_input"},
    {MP2976_U1J2,   CHANNEL_DEFAULT,    VR_IOUT,  37200,    0,  0,  0,  "VCC3V3_QSFP_C_IOUT",       "MP2976",   "curr2_label"},
    {MP2976_U1J2,   CHANNEL_FIRST,      VR_IOUT,  37200,    0,  0,  0,  "VCC3V3_QSFP_D_IOUT",       "MP2976",   "curr3_input"},
};

ssize_t drv_get_temp_alias(unsigned int index, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", temp_sensor_nodes[index].alias_string);
#else
    return sprintf(buf, "%s\n", temp_sensor_nodes[index].alias_string);
#endif
}

ssize_t drv_get_temp_type(unsigned int index, char *buf)
{
#ifdef C11_ANNEX_K
     return sprintf_s(buf, PAGE_SIZE, "%s\n", temp_sensor_nodes[index].type_string);
#else
	 return sprintf(buf, "%s\n", temp_sensor_nodes[index].type_string);
#endif
}

bool drv_get_temp_max(unsigned int index, long *val)
{
    *val = temp_sensor_nodes[index].temp_max;

    return true;
}

bool drv_set_temp_max(unsigned int index, long data)
{
    temp_sensor_nodes[index].temp_max = data;

    return true;
}

bool drv_get_temp_max_hyst(unsigned int index, long *val)
{
    *val = temp_sensor_nodes[index].temp_max_hyst;

    return true;
}

bool drv_get_temp_min(unsigned int index, long *val)
{
    *val = temp_sensor_nodes[index].temp_min;

    return true;
}

bool drv_set_temp_min(unsigned int index, long data)
{
    temp_sensor_nodes[index].temp_min = data;

    return true;
}

bool drv_get_temp_input_from_bmc(unsigned int index, long *temp_input)
{
    int retval=0;
    int thermal_id;

    switch(index)
    {
        case TMP75_48_TEMP:
        case TMP75_49_TEMP:
        case TMP75_4A_TEMP:
        case TMP75_4B_TEMP:
        case TMP431_4C_TEMP1:
        case TMP431_4C_TEMP2:
        case TMP75_4E_TEMP:
        case TMP275_4D_TEMP:
        case TMP275_4E_TEMP:
            thermal_id = index - X86_PACKAGE;
            retval = drv_get_sensor_temp_input_from_bmc(thermal_id, temp_input);
            if(retval < 0)
            {
                SENSOR_DEBUG("Get temp%d input failed.\n", index);
                return false;
            }
            break;
        case VCORE_MAC_TEMP:
        case VDDT0V9_R_MAC_TEMP:
        case VDD1V5_1V8_MAC_TEMP:
        case VCC3V3_QSFP_AB_TEMP:
        case VCC3V3_QSFP_CD_TEMP:
            thermal_id = index - 1;
            retval = drv_get_sensor_temp_input_from_bmc(thermal_id, temp_input);
            if(retval < 0)
            {
                SENSOR_DEBUG("Get temp%d input failed.\n", index);
                return false;
            }
            break;
        case X86_CORE1:
        case X86_CORE2:
        case X86_CORE3:
        case X86_CORE4:
        case X86_PACKAGE:
            retval = coretemp_get_temp_input(index, temp_input);
            if(retval < 0)
            {
                SENSOR_DEBUG("Get temp%d input failed.\n", index);
                return false;
            }
            break;
        case LSW_CORE:
            retval = 0;
            break;

        default:
            return false;
    }

    return true;
}

bool drv_get_temp_input(unsigned int index, long *temp_input)
{
    int retval=0;

    switch(index)
    {
        case TMP75_48_TEMP:
        case TMP75_49_TEMP:
        case TMP75_4A_TEMP:
        case TMP75_4B_TEMP:
        case TMP431_4C_TEMP1:
        case TMP431_4C_TEMP2:
        case TMP75_4E_TEMP:
        case TMP275_4D_TEMP:
        case TMP275_4E_TEMP:
            retval = lm75_read_temp_input(temp_sensor_nodes[index].bus_num, temp_sensor_nodes[index].dev_addr, temp_input);
            if(retval < 0)
            {
                SENSOR_DEBUG("Get temp%d input failed.\n", index);
                return false;
            }
            break;
        case X86_CORE1:
        case X86_CORE2:
        case X86_CORE3:
        case X86_CORE4:
        case X86_PACKAGE:
            retval = coretemp_get_temp_input(index, temp_input);
            if(retval < 0)
            {
                SENSOR_DEBUG("Get temp%d input failed.\n", index);
                return false;
            }
            break;
        case LSW_CORE:
            retval = 0;
            break;

        default:
            return false;
    }

    return true;
}

ssize_t drv_get_vol_alias(unsigned int index, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", vr_sensor_vol_nodes[index].alias_string);
#else
    return sprintf(buf, "%s\n", vr_sensor_vol_nodes[index].alias_string);
#endif

}

ssize_t drv_get_vol_type(unsigned int index, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", vr_sensor_vol_nodes[index].type_string);
#else
    return sprintf(buf, "%s\n", vr_sensor_vol_nodes[index].type_string);
#endif
}

bool drv_get_vol_max(unsigned int index, long *val)
{
    *val = vr_sensor_vol_nodes[index].data_max;
    return true;
}

bool drv_set_vol_max(unsigned int index, long data)
{
    vr_sensor_vol_nodes[index].data_max = data;
    return true;
}

bool drv_get_vol_min(unsigned int index, long *val)
{
    *val = vr_sensor_vol_nodes[index].data_min;
    return true;
}

bool drv_set_vol_min(unsigned int index, long data)
{
    vr_sensor_vol_nodes[index].data_min = data;
    return true;
}

bool drv_get_vol_range(unsigned int index, long *val)
{
    *val = vr_sensor_vol_nodes[index].data_range;
    return true;
}

bool drv_get_vol_nominal(unsigned int index, long *val)
{
    *val = vr_sensor_vol_nodes[index].data_nominal;
    return true;
}

bool drv_get_vol_input(unsigned int index, long *vol_input)
{
    unsigned char chip_id;
    unsigned int bus_num;
    unsigned short dev_addr;
    int retval=0;
    int i;

    chip_id = vr_sensor_vol_nodes[index].chip_id;
    for(i = 0; i < TOTAL_VR_CHIP_NUM; i++)
    {
        if(chip_id == vr_chips[i].chip_id)
        {
            bus_num = vr_chips[i].bus_num;
            dev_addr = vr_chips[i].dev_addr;
            break;
        }
    }

    if(i == TOTAL_VR_CHIP_NUM)
    {
        SENSOR_DEBUG("Get in%d chip info failed.\n", index);
        return false;
    }

    retval = pmbus_core_read_attrs(bus_num, dev_addr, vr_sensor_vol_nodes[index].node_name, vol_input);
    if(retval < 0)
    {
        SENSOR_DEBUG("Get in%d input failed.\n", index);
        return false;
    }

    return true;
}

bool drv_get_vol_input_from_bmc(unsigned int index, long *vol_input)
{
    int retval=0;

    retval = drv_get_sensor_vol_input_from_bmc(vr_sensor_vol_nodes[index], vol_input);
    if(retval < 0)
    {
        SENSOR_DEBUG("Get in%d input failed.\n", index);
        return false;
    }

    return true;
}

ssize_t drv_get_curr_alias(unsigned int index, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", vr_sensor_curr_nodes[index].alias_string);
#else
    return sprintf(buf, "%s\n", vr_sensor_curr_nodes[index].alias_string);
#endif
}

ssize_t drv_get_curr_type(unsigned int index, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", vr_sensor_curr_nodes[index].type_string);
#else
    return sprintf(buf, "%s\n", vr_sensor_curr_nodes[index].type_string);
#endif
}

bool drv_get_curr_max(unsigned int index, long *val)
{
    *val = vr_sensor_curr_nodes[index].data_max;
    return true;
}

bool drv_set_curr_max(unsigned int index, long data)
{
    vr_sensor_curr_nodes[index].data_max = data;
    return true;
}

bool drv_get_curr_min(unsigned int index, long *val)
{
    *val = vr_sensor_curr_nodes[index].data_min;
    return true;
}

bool drv_set_curr_min(unsigned int index, long data)
{
    vr_sensor_curr_nodes[index].data_min = data;
    return true;
}

bool drv_get_curr_input(unsigned int index, long *curr_input)
{
    unsigned char chip_id;
    unsigned int bus_num;
    unsigned short dev_addr;
    int retval=0;
    int i;

    chip_id = vr_sensor_curr_nodes[index].chip_id;
    for(i = 0; i < TOTAL_VR_CHIP_NUM; i++)
    {
        if(chip_id == vr_chips[i].chip_id)
        {
            bus_num = vr_chips[i].bus_num;
            dev_addr = vr_chips[i].dev_addr;
            break;
        }
    }

    if(i == TOTAL_VR_CHIP_NUM)
    {
        SENSOR_DEBUG("Get curr%d chip info failed.\n", index);
        return false;
    }

    retval = pmbus_core_read_attrs(bus_num, dev_addr, vr_sensor_curr_nodes[index].node_name, curr_input);
    if(retval < 0)
    {
        SENSOR_DEBUG("Get curr%d input failed.\n", index);
        return false;
    }

    return true;
}

bool drv_get_curr_input_from_bmc(unsigned int index, long *curr_input)
{
    int retval=0;

    retval = drv_get_sensor_curr_input_from_bmc(vr_sensor_curr_nodes[index], curr_input);

    if(retval < 0)
    {
        SENSOR_DEBUG("Get curr%d input failed.\n", index);
        return false;
    }

    return true;
}

void drv_get_loglevel(long *lev)
{
    *lev = (long)loglevel;

    return;
}

void drv_set_loglevel(long lev)
{
    loglevel = (unsigned int)lev;

    return;
}

ssize_t drv_debug_help(char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE,
        "tmp75_48           ipmitool raw 0x36 0x10 1\n"
        "tmp75_49           ipmitool raw 0x36 0x10 2\n"
        "tmp75_4a           ipmitool raw 0x36 0x10 3\n"
        "tmp75_4b           ipmitool raw 0x36 0x10 4\n"
        "tmp431_4c1         ipmitool raw 0x36 0x10 5\n"
        "tmp431_4c2         ipmitool raw 0x36 0x10 6\n"
        "tmp75_4e           ipmitool raw 0x36 0x10 7\n"
        "tmp275_4d          ipmitool raw 0x36 0x10 8\n"
        "tmp275_4e          ipmitool raw 0x36 0x10 9\n"
        "lm25066_41_temp    ipmitool raw 0x36 0x10 14\n"
        "mp2973_20_temp     ipmitool raw 0x36 0x10 15\n"
        "mp2940b_40_temp    ipmitool raw 0x36 0x10 16\n"
        "mp2976_60_temp     ipmitool raw 0x36 0x10 17\n"
        "mp2976_50_temp     ipmitool raw 0x36 0x10 18\n"
        "mp2976_5f_temp     ipmitool raw 0x36 0x10 19\n");
#else
    return sprintf(buf,
        "tmp75_48           ipmitool raw 0x36 0x10 1\n"
        "tmp75_49           ipmitool raw 0x36 0x10 2\n"
        "tmp75_4a           ipmitool raw 0x36 0x10 3\n"
        "tmp75_4b           ipmitool raw 0x36 0x10 4\n"
        "tmp431_4c1         ipmitool raw 0x36 0x10 5\n"
        "tmp431_4c2         ipmitool raw 0x36 0x10 6\n"
        "tmp75_4e           ipmitool raw 0x36 0x10 7\n"
        "tmp275_4d          ipmitool raw 0x36 0x10 8\n"
        "tmp275_4e          ipmitool raw 0x36 0x10 9\n"
        "lm25066_41_temp    ipmitool raw 0x36 0x10 14\n"
        "mp2973_20_temp     ipmitool raw 0x36 0x10 15\n"
        "mp2940b_40_temp    ipmitool raw 0x36 0x10 16\n"
        "mp2976_60_temp     ipmitool raw 0x36 0x10 17\n"
        "mp2976_50_temp     ipmitool raw 0x36 0x10 18\n"
        "mp2976_5f_temp     ipmitool raw 0x36 0x10 19\n");
#endif
}

ssize_t drv_debug_help_hisonic(char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE,
        "For LM75_COMe, use i2cget/i2cset -f -y 101 0x4b to debug.\n"
        "For LM75_64DQ_Fanout, use i2cget/i2cset -f -y 105 0x49 to debug.\n"
        "sysname                debug cmd\n"
        "1 temp                 i2cget -f -y 101 0x4b 0x0\n"
        "2 temp                 i2cget -f -y 105 0x49 0x0\n"
        "3 temp                 cat /sys/class/hwmon/hwmon*/name\n"
        "                       cat /sys/class/hwmon/hwmon4/temp*_input\n"
        "4 temp                 cat /sys/class/hwmon/hwmon*/name\n"
        "                       cat /sys/class/hwmon/hwmon4/temp*_input\n"
        "5 temp                 cat /sys/class/hwmon/hwmon*/name\n"
        "                       cat /sys/class/hwmon/hwmon4/temp*_input\n"
        "6 temp                 cat /sys/class/hwmon/hwmon*/name\n"
        "                       cat /sys/class/hwmon/hwmon4/temp*_input\n"
        "7 temp                 cat /sys/class/hwmon/hwmon*/name\n"
        "                       cat /sys/class/hwmon/hwmon4/temp*_input\n"
        "8 temp                 i2cget -f -y 105 0x49 0x0\n"
        "                       value = 1.273*LSW_LM75+18.7\n");
#else
    return sprintf(buf,
        "For LM75_COMe, use i2cget/i2cset -f -y 101 0x4b to debug.\n"
        "For LM75_64DQ_Fanout, use i2cget/i2cset -f -y 105 0x49 to debug.\n"
        "sysname                debug cmd\n"
        "1 temp                 i2cget -f -y 101 0x4b 0x0\n"
        "2 temp                 i2cget -f -y 105 0x49 0x0\n"
        "3 temp                 cat /sys/class/hwmon/hwmon*/name\n"
        "                       cat /sys/class/hwmon/hwmon4/temp*_input\n"
        "4 temp                 cat /sys/class/hwmon/hwmon*/name\n"
        "                       cat /sys/class/hwmon/hwmon4/temp*_input\n"
        "5 temp                 cat /sys/class/hwmon/hwmon*/name\n"
        "                       cat /sys/class/hwmon/hwmon4/temp*_input\n"
        "6 temp                 cat /sys/class/hwmon/hwmon*/name\n"
        "                       cat /sys/class/hwmon/hwmon4/temp*_input\n"
        "7 temp                 cat /sys/class/hwmon/hwmon*/name\n"
        "                       cat /sys/class/hwmon/hwmon4/temp*_input\n"
        "8 temp                 i2cget -f -y 105 0x49 0x0\n"
        "                       value = 1.273*LSW_LM75+18.7\n");
#endif
}

ssize_t drv_debug(const char *buf, int count)
{
    return 0;
}

// For s3ip
static struct sensor_drivers_t pfunc_bmc= {
    .get_temp_input = drv_get_temp_input_from_bmc,
    .get_temp_alias = drv_get_temp_alias,
    .get_temp_type = drv_get_temp_type,
    .get_temp_max = drv_get_temp_max,
    .set_temp_max = drv_set_temp_max,
    .get_temp_max_hyst = drv_get_temp_max_hyst,
    .get_temp_min = drv_get_temp_min,
    .set_temp_min = drv_set_temp_min,
    .get_vol_alias = drv_get_vol_alias,
    .get_vol_type = drv_get_vol_type,
    .get_vol_max = drv_get_vol_max,
    .set_vol_max = drv_set_vol_max,
    .get_vol_min = drv_get_vol_min,
    .set_vol_min = drv_set_vol_min,
    .get_vol_input = drv_get_vol_input_from_bmc,
    .get_vol_range = drv_get_vol_range,
    .get_vol_nominal = drv_get_vol_nominal,
    .get_curr_alias = drv_get_curr_alias,
    .get_curr_type = drv_get_curr_type,
    .get_curr_max = drv_get_curr_max,
    .set_curr_max = drv_set_curr_max,
    .get_curr_min = drv_get_curr_min,
    .set_curr_min = drv_set_curr_min,
    .get_curr_input = drv_get_curr_input_from_bmc,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
    .debug_help = drv_debug_help,
    .debug_help_hisonic = drv_debug_help_hisonic,
};

static struct sensor_drivers_t pfunc = {
    .get_temp_input = drv_get_temp_input,
    .get_temp_alias = drv_get_temp_alias,
    .get_temp_type = drv_get_temp_type,
    .get_temp_max = drv_get_temp_max,
    .set_temp_max = drv_set_temp_max,
    .get_temp_max_hyst = drv_get_temp_max_hyst,
    .get_temp_min = drv_get_temp_min,
    .set_temp_min = drv_set_temp_min,
    .get_vol_alias = drv_get_vol_alias,
    .get_vol_type = drv_get_vol_type,
    .get_vol_max = drv_get_vol_max,
    .set_vol_max = drv_set_vol_max,
    .get_vol_min = drv_get_vol_min,
    .set_vol_min = drv_set_vol_min,
    .get_vol_input = drv_get_vol_input,
    .get_vol_range = drv_get_vol_range,
    .get_vol_nominal = drv_get_vol_nominal,
    .get_curr_alias = drv_get_curr_alias,
    .get_curr_type = drv_get_curr_type,
    .get_curr_max = drv_get_curr_max,
    .set_curr_max = drv_set_curr_max,
    .get_curr_min = drv_get_curr_min,
    .set_curr_min = drv_set_curr_min,
    .get_curr_input = drv_get_curr_input,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
    .debug_help = drv_debug_help,
    .debug_help_hisonic = drv_debug_help_hisonic,
};

static int drv_sensor_probe(struct platform_device *pdev)
{
    if(ipmi_bmc_is_ok())
    {
        s3ip_sensor_drivers_register(&pfunc_bmc);
    }
    else
    {
        s3ip_sensor_drivers_register(&pfunc);
    }

    return 0;
}

static int drv_sensor_remove(struct platform_device *pdev)
{
    s3ip_sensor_drivers_unregister();

    return 0;
}

static struct platform_driver drv_sensor_driver = {
    .driver = {
        .name   = DRVNAME,
        .owner = THIS_MODULE,
    },
    .probe      = drv_sensor_probe,
    .remove     = drv_sensor_remove,
};

static int __init drv_sensor_init(void)
{
    int err=0;
    int retval=0;

    drv_sensor_device = platform_device_alloc(DRVNAME, 0);
    if(!drv_sensor_device)
    {
        SENSOR_DEBUG("%s(#%d): platform_device_alloc fail\n", __func__, __LINE__);
        return -ENOMEM;
    }

    retval = platform_device_add(drv_sensor_device);
    if(retval)
    {
        SENSOR_DEBUG("%s(#%d): platform_device_add failed\n", __func__, __LINE__);
        err = -retval;
        goto dev_add_failed;
    }

    retval = platform_driver_register(&drv_sensor_driver);
    if(retval)
    {
        SENSOR_DEBUG("%s(#%d): platform_driver_register failed(%d)\n", __func__, __LINE__, err);
        err = -retval;
        goto dev_reg_failed;
    }

    return 0;

dev_reg_failed:
    platform_device_unregister(drv_sensor_device);
    return err;

dev_add_failed:
    platform_device_put(drv_sensor_device);
    return err;
}

static void __exit drv_sensor_exit(void)
{
    platform_driver_unregister(&drv_sensor_driver);
    platform_device_unregister(drv_sensor_device);

    return;
}

MODULE_DESCRIPTION("S3IP Sensor Driver");
MODULE_VERSION(SWITCH_SENSOR_DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_init(drv_sensor_init);
module_exit(drv_sensor_exit);
