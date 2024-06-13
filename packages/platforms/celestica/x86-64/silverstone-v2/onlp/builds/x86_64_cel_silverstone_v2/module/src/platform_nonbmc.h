#ifndef _PLATFORM_SILVERSTONV2_H_
#define _PLATFORM_SILVERSTONV2_H_
#include <stdint.h>
#include <onlp/platformi/fani.h>


#define FAN1_EEPROM                     "/sys/bus/i2c/devices/i2c-53/53-0050/eeprom"
#define FAN2_EEPROM                     "/sys/bus/i2c/devices/i2c-54/54-0050/eeprom"
#define FAN3_EEPROM                     "/sys/bus/i2c/devices/i2c-55/55-0050/eeprom"
#define FAN4_EEPROM                     "/sys/bus/i2c/devices/i2c-52/52-0050/eeprom"
#define FAN5_EEPROM                     "/sys/bus/i2c/devices/i2c-49/49-0050/eeprom"
#define FAN6_EEPROM                     "/sys/bus/i2c/devices/i2c-50/50-0050/eeprom"
#define FAN7_EEPROM                     "/sys/bus/i2c/devices/i2c-51/51-0050/eeprom"

#define BITS_PER_LONG_LONG 64

#define GENMASK_ULL(h, l) \
	(((~0ULL) - (1ULL << (l)) + 1) & \
	 (~0ULL >> (BITS_PER_LONG_LONG - 1 - (h))))

#define FRU_COMM_HDR_LEN                8
#define FRU_BRDINFO_MFT_TLV_STRT        6   //Board Manufacturer byte offset in "Board Info Area Format" field. 
#define TLV_TYPE_MASK                   GENMASK_ULL(7,6)
#define TLV_LEN_MASK                    GENMASK_ULL(5,0)

//Fan
#define FAN_CPLD_I2C_BUS_STR          "8"
#define FAN_CPLD_I2C_ADDR_STR         "000d"

#define PSU1_I2C_BUS_STR               "57"
#define PSU2_I2C_BUS_STR               "58"
#define PSU1_I2C_ADDR_STR             "0058"
#define PSU2_I2C_ADDR_STR             "0059"

#define PSU1_I2C_BUS                   57
#define PSU2_I2C_BUS                   58
#define PSU1_I2C_ADDR                 0x58
#define PSU2_I2C_ADDR                 0x59
#define PSU1_EEPROM_ADDR              0x50
#define PSU2_EEPROM_ADDR              0x51
#define PSU_MODULE                    "platform_psu"
#define PSU_EEPROM_MODULE             "24c02"

#define CHASSIS_FAN_PATH              "/sys/bus/i2c/devices/i2c-"FAN_CPLD_I2C_BUS_STR"/"FAN_CPLD_I2C_BUS_STR"-"FAN_CPLD_I2C_ADDR_STR"/"
#define CHASSIS_FAN_DIRECTION(x)      "/sys/bus/i2c/devices/i2c-"FAN_CPLD_I2C_BUS_STR"/"FAN_CPLD_I2C_BUS_STR"-"FAN_CPLD_I2C_ADDR_STR"/fan"#x"_direction"
#define CHASSIS_FAN_FAULT(x)          "/sys/bus/i2c/devices/i2c-"FAN_CPLD_I2C_BUS_STR"/"FAN_CPLD_I2C_BUS_STR"-"FAN_CPLD_I2C_ADDR_STR"/fan"#x"_fault"
#define CHASSIS_FAN_SPEED(x)          "/sys/bus/i2c/devices/i2c-"FAN_CPLD_I2C_BUS_STR"/"FAN_CPLD_I2C_BUS_STR"-"FAN_CPLD_I2C_ADDR_STR"/fan"#x"_rear_speed_rpm"
#define CHASSIS_FAN_FRONT_SPEED(x)    "/sys/bus/i2c/devices/i2c-"FAN_CPLD_I2C_BUS_STR"/"FAN_CPLD_I2C_BUS_STR"-"FAN_CPLD_I2C_ADDR_STR"/fan"#x"_front_speed_rpm"
#define CHASSIS_FAN_LED(x)            "/sys/bus/i2c/devices/i2c-"FAN_CPLD_I2C_BUS_STR"/"FAN_CPLD_I2C_BUS_STR"-"FAN_CPLD_I2C_ADDR_STR"/fan"#x"_led"
#define CHASSIS_FAN_PRESENT(x)        "/sys/bus/i2c/devices/i2c-"FAN_CPLD_I2C_BUS_STR"/"FAN_CPLD_I2C_BUS_STR"-"FAN_CPLD_I2C_ADDR_STR"/fan"#x"_present"
#define CHASSIS_FAN_PWM(x)            "/sys/bus/i2c/devices/i2c-"FAN_CPLD_I2C_BUS_STR"/"FAN_CPLD_I2C_BUS_STR"-"FAN_CPLD_I2C_ADDR_STR"/pwm"#x

#define PSU1_FAN                      "/sys/bus/i2c/devices/i2c-"PSU1_I2C_BUS_STR"/"PSU1_I2C_BUS_STR"-"PSU1_I2C_ADDR_STR"/hwmon/hwmon*/fan1_input"
#define PSU2_FAN                      "/sys/bus/i2c/devices/i2c-"PSU2_I2C_BUS_STR"/"PSU2_I2C_BUS_STR"-"PSU2_I2C_ADDR_STR"/hwmon/hwmon*/fan1_input"

#define FAN_DIR_F2B                   1
#define FAN_DIR_B2F                   0

//PSU
#define PSU1_SYSFS "/sys/bus/i2c/devices/i2c-"PSU1_I2C_BUS_STR""
#define PSU2_SYSFS "/sys/bus/i2c/devices/i2c-"PSU2_I2C_BUS_STR""
#define PSU1_CIN "/sys/bus/i2c/devices/i2c-"PSU1_I2C_BUS_STR"/"PSU1_I2C_BUS_STR"-"PSU1_I2C_ADDR_STR"/hwmon/hwmon*/curr1_input"
#define PSU1_VIN "/sys/bus/i2c/devices/i2c-"PSU1_I2C_BUS_STR"/"PSU1_I2C_BUS_STR"-"PSU1_I2C_ADDR_STR"/hwmon/hwmon*/in1_input"
#define PSU1_PIN "/sys/bus/i2c/devices/i2c-"PSU1_I2C_BUS_STR"/"PSU1_I2C_BUS_STR"-"PSU1_I2C_ADDR_STR"/hwmon/hwmon*/power1_input"
#define PSU1_COUT "/sys/bus/i2c/devices/i2c-"PSU1_I2C_BUS_STR"/"PSU1_I2C_BUS_STR"-"PSU1_I2C_ADDR_STR"/hwmon/hwmon*/curr2_input"
#define PSU1_VOUT "/sys/bus/i2c/devices/i2c-"PSU1_I2C_BUS_STR"/"PSU1_I2C_BUS_STR"-"PSU1_I2C_ADDR_STR"/hwmon/hwmon*/in2_input"
#define PSU1_POUT "/sys/bus/i2c/devices/i2c-"PSU1_I2C_BUS_STR"/"PSU1_I2C_BUS_STR"-"PSU1_I2C_ADDR_STR"/hwmon/hwmon*/power2_input"
#define PSU2_CIN "/sys/bus/i2c/devices/i2c-"PSU2_I2C_BUS_STR"/"PSU2_I2C_BUS_STR"-"PSU2_I2C_ADDR_STR"/hwmon/hwmon*/curr1_input"
#define PSU2_VIN "/sys/bus/i2c/devices/i2c-"PSU2_I2C_BUS_STR"/"PSU2_I2C_BUS_STR"-"PSU2_I2C_ADDR_STR"/hwmon/hwmon*/in1_input"
#define PSU2_PIN "/sys/bus/i2c/devices/i2c-"PSU2_I2C_BUS_STR"/"PSU2_I2C_BUS_STR"-"PSU2_I2C_ADDR_STR"/hwmon/hwmon*/power1_input"
#define PSU2_COUT "/sys/bus/i2c/devices/i2c-"PSU2_I2C_BUS_STR"/"PSU2_I2C_BUS_STR"-"PSU2_I2C_ADDR_STR"/hwmon/hwmon*/curr2_input"
#define PSU2_VOUT "/sys/bus/i2c/devices/i2c-"PSU2_I2C_BUS_STR"/"PSU2_I2C_BUS_STR"-"PSU2_I2C_ADDR_STR"/hwmon/hwmon*/in2_input"
#define PSU2_POUT "/sys/bus/i2c/devices/i2c-"PSU2_I2C_BUS_STR"/"PSU2_I2C_BUS_STR"-"PSU2_I2C_ADDR_STR"/hwmon/hwmon*/power2_input"


#define PMBUS_MODEL_REG 0x9a
#define PMBUS_SERIAL_REG 0x9e


#define TEMP_SW_Internal_LSB_FPGA_REG   0x78
#define TEMP_SW_Internal_MSB_FPGA_REG   0x80
#define TEMP_SW_U52      "/sys/bus/i2c/devices/i2c-7/7-0048/hwmon/hwmon*/temp1_input"
#define TEMP_SW_U16      "/sys/bus/i2c/devices/i2c-7/7-0049/hwmon/hwmon*/temp1_input"
#define TEMP_FB_U52      "/sys/bus/i2c/devices/i2c-56/56-0048/hwmon/hwmon*/temp1_input"
#define TEMP_FB_U17      "/sys/bus/i2c/devices/i2c-56/56-0049/hwmon/hwmon*/temp1_input"
#define VDD_CORE_Temp      "/sys/bus/i2c/devices/i2c-3/3-006c/hwmon/hwmon*/temp1_input"
#define XP0R8V_Temp      "/sys/bus/i2c/devices/i2c-4/4-0070/hwmon/hwmon*/temp1_input"
#define XP3R3V_R_Temp      "/sys/bus/i2c/devices/i2c-4/4-007b/hwmon/hwmon*/temp1_input"
#define XP3R3V_L_Temp      "/sys/bus/i2c/devices/i2c-4/4-0076/hwmon/hwmon*/temp1_input"
#define CPU_THERMAL           "/sys/bus/platform/drivers/coretemp/coretemp.0/hwmon/hwmon*/temp1_input"
#define DIMMA0_THERMAL           "/sys/bus/i2c/devices/65-001a/hwmon/hwmon*/temp1_input"
#define DIMMB0_THERMAL           "/sys/bus/i2c/devices/65-0018/hwmon/hwmon*/temp1_input"
#define PSU1_THERMAL1         "/sys/bus/i2c/devices/i2c-"PSU1_I2C_BUS_STR"/"PSU1_I2C_BUS_STR"-"PSU1_I2C_ADDR_STR"/hwmon/hwmon*/temp1_input"
#define PSU1_THERMAL2         "/sys/bus/i2c/devices/i2c-"PSU1_I2C_BUS_STR"/"PSU1_I2C_BUS_STR"-"PSU1_I2C_ADDR_STR"/hwmon/hwmon*/temp2_input"
#define PSU1_THERMAL3         "/sys/bus/i2c/devices/i2c-"PSU1_I2C_BUS_STR"/"PSU1_I2C_BUS_STR"-"PSU1_I2C_ADDR_STR"/hwmon/hwmon*/temp3_input"
#define PSU2_THERMAL1         "/sys/bus/i2c/devices/i2c-"PSU2_I2C_BUS_STR"/"PSU2_I2C_BUS_STR"-"PSU2_I2C_ADDR_STR"/hwmon/hwmon*/temp1_input"
#define PSU2_THERMAL2         "/sys/bus/i2c/devices/i2c-"PSU2_I2C_BUS_STR"/"PSU2_I2C_BUS_STR"-"PSU2_I2C_ADDR_STR"/hwmon/hwmon*/temp2_input"
#define PSU2_THERMAL3         "/sys/bus/i2c/devices/i2c-"PSU2_I2C_BUS_STR"/"PSU2_I2C_BUS_STR"-"PSU2_I2C_ADDR_STR"/hwmon/hwmon*/temp3_input"


#define MAX_TEMP 75000
#define TEMP_SW_Internal_MAX_VALID    150
#define TEMP_CPU_MAX_VALID            130
#define TEMP_DIFF_RANGE               15
/* Warning */
#define TEMP_SW_U52_HI_WARN           MAX_TEMP
#define TEMP_SW_U16_HI_WARN           59000
#define TEMP_FB_U52_HI_WARN           MAX_TEMP
#define TEMP_FB_U17_HI_WARN           56000
#define VDD_CORE_Temp_HI_WARN         125000
#define XP0R8V_Temp_HI_WARN           125000
#define XP3R3V_R_Temp_HI_WARN         125000
#define XP3R3V_L_Temp_HI_WARN         125000
#define TEMP_SW_Internal_HI_WARN      110000
#define CPU_THERMAL_HI_WARN           105000
#define DIMMA0_THERMAL_HI_WARN        85000
#define DIMMB0_THERMAL_HI_WARN        85000
#define PSU1_THERMAL1_HI_WARN         70000
#define PSU1_THERMAL2_HI_WARN         116000
#define PSU1_B2F_THERMAL3_HI_WARN     78000
#define PSU1_F2B_THERMAL3_HI_WARN     92000
#define PSU2_THERMAL1_HI_WARN         70000
#define PSU2_THERMAL2_HI_WARN         116000
#define PSU2_B2F_THERMAL3_HI_WARN     78000
#define PSU2_F2B_THERMAL3_HI_WARN     92000


/* Error */
#define TEMP_SW_U52_HI_ERR        MAX_TEMP
#define TEMP_SW_U16_HI_ERR        59000
#define TEMP_FB_U52_HI_ERR        MAX_TEMP
#define TEMP_FB_U17_HI_ERR        56000
#define VDD_CORE_Temp_HI_ERR      125000
#define XP0R8V_Temp_HI_ERR        125000
#define XP3R3V_R_Temp_HI_ERR      125000
#define XP3R3V_L_Temp_HI_ERR      125000
#define TEMP_SW_Internal_HI_ERR   110000
#define CPU_THERMAL_HI_ERR        105000
#define DIMMA0_THERMAL_HI_ERR     85000
#define DIMMB0_THERMAL_HI_ERR     85000
#define PSU1_THERMAL1_HI_ERR      70000
#define PSU1_THERMAL2_HI_ERR      116000
#define PSU1_B2F_THERMAL3_HI_ERR  78000
#define PSU1_F2B_THERMAL3_HI_ERR  92000
#define PSU2_THERMAL1_HI_ERR      70000
#define PSU2_THERMAL2_HI_ERR      116000
#define PSU2_B2F_THERMAL3_HI_ERR  78000
#define PSU2_F2B_THERMAL3_HI_ERR  92000


/* Shutdown */
#define TEMP_SW_U52_HI_SHUTDOWN           MAX_TEMP
#define TEMP_SW_U16_HI_SHUTDOWN           59000
#define TEMP_FB_U52_HI_SHUTDOWN           MAX_TEMP
#define TEMP_FB_U17_HI_SHUTDOWN           56000
#define VDD_CORE_Temp_HI_SHUTDOWN         125000
#define XP0R8V_Temp_HI_SHUTDOWN           125000
#define XP3R3V_R_Temp_HI_SHUTDOWN         125000
#define XP3R3V_L_Temp_HI_SHUTDOWN         125000
#define TEMP_SW_Internal_HI_SHUTDOWN      124000
#define CPU_THERMAL_HI_SHUTDOWN           105000
#define DIMMA0_THERMAL_HI_SHUTDOWN        85000
#define DIMMB0_THERMAL_HI_SHUTDOWN        85000
#define PSU1_THERMAL1_HI_SHUTDOWN         70000
#define PSU1_THERMAL2_HI_SHUTDOWN         116000
#define PSU1_B2F_THERMAL3_HI_SHUTDOWN     78000
#define PSU1_F2B_THERMAL3_HI_SHUTDOWN     92000
#define PSU2_THERMAL1_HI_SHUTDOWN         70000
#define PSU2_THERMAL2_HI_SHUTDOWN         116000
#define PSU2_B2F_THERMAL3_HI_SHUTDOWN     78000
#define PSU2_F2B_THERMAL3_HI_SHUTDOWN     92000



//SFP
#define SFP_BUS_START 11

//CPLD
#define FAN_CPLD_BUS 8
#define FAN_CPLD_ADDR 0x0d
#define ABSENT 1
#define PRESENT 0



#endif /* _PLATFORM_SILVERSTONEV2_H_ */
