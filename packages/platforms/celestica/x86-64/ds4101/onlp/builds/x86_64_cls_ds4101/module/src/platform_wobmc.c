//////////////////////////////////////////////////////////////
//   PLATFORM FUNCTION TO INTERACT WITH SYS_CPLD AND OTHER DRIVERS    //
//////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/io.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/sysi.h>
#include "platform_comm.h"
#include "platform_wobmc.h"

extern const struct psu_reg_bit_mapper psu_mapper [PSU_COUNT + 1];
uint8_t Sys_Airflow = FAN_F2B;

/* This array index should be the same as corresponding sensor ID */
struct temp Temp_B2F[THERMAL_COUNT + 1];

struct temp Temp_F2B[THERMAL_COUNT + 1] = {
    {},
    {
        NOT_DEFINE, //CPU
        NOT_DEFINE,
        NOT_DEFINE,
        105000,
        105000,
        NOT_DEFINE,
        0,
        "TEMP_CPU",
        "/sys/bus/platform/drivers/coretemp/coretemp.0/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //DIMMA0
        NOT_DEFINE,
        NOT_DEFINE,
        85000,
        85000,
        NOT_DEFINE,
        0,
        "TEMP_DIMMA0",
        "/sys/bus/i2c/devices/71-001a/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //DIMMB0
        NOT_DEFINE,
        NOT_DEFINE,
        85000,
        85000,
        NOT_DEFINE,
        0,
        "TEMP_DIMMB0",
        "/sys/bus/i2c/devices/71-0018/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //SWITCH INTERNAL
        NOT_DEFINE,
        NOT_DEFINE,
        105000,
        105000,
        110000,
        0,
        "TEMP_SW_Internal",
        "/sys/devices/platform/fpga-sys/temp_sw_internal",
        0,
    },
    {
        NOT_DEFINE, //BMC
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "TEMP_BMC",
        "na",
        0,
    },
    {
        NOT_DEFINE, //U3
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "TEMP_BB_U3",
        "/sys/bus/i2c/devices/i2c-10/10-004e/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE,
        NOT_DEFINE, //U15
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "TEMP_SW_U15",
        "/sys/bus/i2c/devices/i2c-55/55-0048/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE,
        NOT_DEFINE, //U17
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "TEMP_SW_U17",
        "/sys/bus/i2c/devices/i2c-56/56-0049/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //U16
        NOT_DEFINE,
        NOT_DEFINE,
        59000,
        59000,
        NOT_DEFINE,
        0,
        "TEMP_SW_U16",
        "/sys/bus/i2c/devices/i2c-57/57-004a/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //U13
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "TEMP_SW_U13",
        "/sys/bus/i2c/devices/i2c-58/58-004b/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //U11
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "TEMP_SW_U11",
        "/sys/bus/i2c/devices/i2c-59/59-004c/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //U12
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "TEMP_SW_U12",
        "/sys/bus/i2c/devices/i2c-60/60-004d/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //U52
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "TEMP_FB_U52",
        "/sys/bus/i2c/devices/i2c-70/70-0048/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //U17
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "TEMP_FB_U17",
        "/sys/bus/i2c/devices/i2c-70/70-0049/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //MP5023
        NOT_DEFINE,
        NOT_DEFINE,
        135000,
        135000,
        135000,
        0,
        "MP5023_T",
        "/sys/bus/i2c/devices/i2c-8/8-0040/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //VDD_CORE
        NOT_DEFINE,
        NOT_DEFINE,
        135000,
        135000,
        135000,
        0,
        "VDD_CORE_T",
        "/sys/bus/i2c/devices/i2c-9/9-0020/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //XP3R3V_LEFT
        NOT_DEFINE,
        NOT_DEFINE,
        135000,
        135000,
        135000,
        0,
        "XP3R3V_LEFT_T",
        "/sys/bus/i2c/devices/i2c-9/9-0070/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //XP3R3V_RIGHT
        NOT_DEFINE,
        NOT_DEFINE,
        135000,
        135000,
        135000,
        0,
        "XP3R3V_RIGHT_T",
        "/sys/bus/i2c/devices/i2c-9/9-0076/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        60000,
        60000,
        NOT_DEFINE,
        0,
        "PSU1_TEMP1",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //PSU1_TEMP2
        NOT_DEFINE,
        NOT_DEFINE,
        79000,
        79000,
        NOT_DEFINE,
        0,
        "PSU1_TEMP2",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/temp2_input",
        0,
    },
    {
        NOT_DEFINE, //PSU1_TEMP3
        NOT_DEFINE,
        NOT_DEFINE,
        78000,
        78000,
        NOT_DEFINE,
        0,
        "PSU1_TEMP3",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/temp3_input",
        0,
    },
    {
        NOT_DEFINE, //PSU2_TEMP1
        NOT_DEFINE,
        NOT_DEFINE,
        60000,
        60000,
        NOT_DEFINE,
        0,
        "PSU2_TEMP1",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/temp1_input",
        0,
    },
    {
        NOT_DEFINE, //PSU2_TEMP2
        NOT_DEFINE,
        NOT_DEFINE,
        79000,
        79000,
        NOT_DEFINE,
        0,
        "PSU2_TEMP2",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/temp2_input",
        0,
    },
    {
        NOT_DEFINE, //PSU2_TEMP3
        NOT_DEFINE,
        NOT_DEFINE,
        78000,
        78000,
        NOT_DEFINE,
        0,
        "PSU2_TEMP3",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/temp3_input",
        0,
    }
};


struct psu Psu[PSU_COUNT + 1] = {
    {},
    { //PSU1
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/in1_input",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/in2_input",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/curr1_input",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/curr2_input",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/power1_input",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/power2_input",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/fan1_input",
        "PSU1",
        0,
    },
    { //PSU2
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/in1_input",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/in2_input",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/curr1_input",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/curr2_input",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/power1_input",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/power2_input",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/fan1_input",
        "PSU2",
        0,
    },
};

struct fan Fan[FAN_COUNT + 1] = {
    {},
    {
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan1_present",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan1_front_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan1_rear_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/pwm1",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan1_direction",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan1_led",
        "/sys/bus/i2c/devices/i2c-67/67-0050/eeprom",
        "FAN1_SPEED",
        0,
    },
    {
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan2_present",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan2_front_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan2_rear_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/pwm2",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan2_direction",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan2_led",
        "/sys/bus/i2c/devices/i2c-68/68-0050/eeprom",
        "FAN2_SPEED",
        0,
    },
    {
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan3_present",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan3_front_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan3_rear_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/pwm3",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan3_direction",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan3_led",
        "/sys/bus/i2c/devices/i2c-69/69-0050/eeprom",
        "FAN3_SPEED",
        0,
    },
    {
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan4_present",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan4_front_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan4_rear_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/pwm4",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan4_direction",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan4_led",
        "/sys/bus/i2c/devices/i2c-66/66-0050/eeprom",
        "FAN4_SPEED",
        0,
    },
    {
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan5_present",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan5_front_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan5_rear_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/pwm5",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan5_direction",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan5_led",
        "/sys/bus/i2c/devices/i2c-63/63-0050/eeprom",
        "FAN5_SPEED",
        0,
    },
    {
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan6_present",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan6_front_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan6_rear_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/pwm6",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan6_direction",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan6_led",
        "/sys/bus/i2c/devices/i2c-64/64-0050/eeprom",
        "FAN6_SPEED",
        0,
    },
    {
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan7_present",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan7_front_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan7_rear_speed_rpm",
        "/sys/bus/i2c/devices/i2c-11/11-000d/pwm7",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan7_direction",
        "/sys/bus/i2c/devices/i2c-11/11-000d/fan7_led",
        "/sys/bus/i2c/devices/i2c-65/65-0050/eeprom",
        "FAN7_SPEED",
        0,
    },
    {
        "na",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/fan1_input",
        "na",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/fan1_target",
        "na",
        "na",
        "/sys/bus/i2c/devices/i2c-47/47-0050/eeprom",
        "PSU1_Fan",
        0,
    },
    {
        "na",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/fan1_input",
        "na",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/fan1_target",
        "na",
        "na",
        "/sys/bus/i2c/devices/i2c-48/48-0051/eeprom",
        "PSU2_Fan",
        0,
    },
};

struct vol_curr_pwr Vol_Curr_Pwr[POWER_COUNT + 1] = {
    {},
    {
        90000,
        NOT_DEFINE,
        NOT_DEFINE,
        264000,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "PSU1_VIN",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/in1_input",
        0,
    },
    {
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        12500,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "PSU1_CIN",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/curr1_input",
        0,
    },
    {
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        2244000000,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "PSU1_PIN",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/power1_input",
        0,
    },
    {
        11400,
        NOT_DEFINE,
        NOT_DEFINE,
        12600,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "PSU1_VOUT",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/in2_input",
        0,
    },
    {
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        166000,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "PSU1_COUT",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/curr2_input",
        0,
    },
    {
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        2040000000,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "PSU1_POUT",
        "/sys/bus/i2c/devices/i2c-47/47-0058/hwmon/hwmon*/power2_input",
        0,
    },
    {
        90000,
        NOT_DEFINE,
        NOT_DEFINE,
        264000,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "PSU2_VIN",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/in1_input",
        0,
    },
    {
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        12500,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "PSU2_CIN",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/curr1_input",
        0,
    },
    {
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        2244000000,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "PSU2_PIN",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/power1_input",
        0,
    },
    {
        11400,
        NOT_DEFINE,
        NOT_DEFINE,
        12600,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "PSU2_VOUT",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/in2_input",
        0,
    },
    {
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        166000,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "PSU2_COUT",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/curr2_input",
        0,
    },
    {
        NOT_DEFINE,
        NOT_DEFINE,
        NOT_DEFINE,
        2040000000,
        NOT_DEFINE,
        NOT_DEFINE,
        0,
        "PSU2_POUT",
        "/sys/bus/i2c/devices/i2c-48/48-0059/hwmon/hwmon*/power2_input",
        0,
    },
    {
        10800,
        10200,
        NOT_DEFINE,
        13200,
        13800,
        NOT_DEFINE,
        0,
        "BB_XP12R0V",
        "/sys/bus/i2c/devices/i2c-8/8-0035/hwmon/hwmon*/in1_input",
        0,
    },
    {
        4500,
        4230,
        NOT_DEFINE,
        5490,
        5730,
        NOT_DEFINE,
        0,
        "BB_XP5R0V",
        "/sys/bus/i2c/devices/i2c-8/8-0035/hwmon/hwmon*/in2_input",
        0,
    },
    {
        2960,
        2800,
        NOT_DEFINE,
        3620,
        3800,
        NOT_DEFINE,
        0,
        "COME_XP3R3V",
        "/sys/bus/i2c/devices/i2c-8/8-0035/hwmon/hwmon*/in3_input",
        0,
    },
    {
        1640,
        1550,
        NOT_DEFINE,
        2000,
        2090,
        NOT_DEFINE,
        0,
        "COME_XP1R82V",
        "/sys/bus/i2c/devices/i2c-8/8-0035/hwmon/hwmon*/in4_input",
        0,
    },
    {
        950,
        890,
        NOT_DEFINE,
        1160,
        1210,
        NOT_DEFINE,
        0,
        "COME_XP1R05V",
        "/sys/bus/i2c/devices/i2c-8/8-0035/hwmon/hwmon*/in5_input",
        0,
    },
    {
        1530,
        1450,
        NOT_DEFINE,
        1870,
        1960,
        NOT_DEFINE,
        0,
        "COME_XP1R7V",
        "/sys/bus/i2c/devices/i2c-8/8-0035/hwmon/hwmon*/in6_input",
        0,
    },
    {
        1080,
        1020,
        NOT_DEFINE,
        1320,
        1380,
        NOT_DEFINE,
        0,
        "COME_XP1R2V",
        "/sys/bus/i2c/devices/i2c-8/8-0035/hwmon/hwmon*/in7_input",
        0,
    },
    {
        1170,
        1110,
        NOT_DEFINE,
        1430,
        1500,
        NOT_DEFINE,
        0,
        "COME_XP1R3V",
        "/sys/bus/i2c/devices/i2c-8/8-0035/hwmon/hwmon*/in8_input",
        0,
    },
    {
        1350,
        1280,
        NOT_DEFINE,
        1650,
        1730,
        NOT_DEFINE,
        0,
        "COME_XP1R5V",
        "/sys/bus/i2c/devices/i2c-8/8-0035/hwmon/hwmon*/in9_input",
        0,
    },
    {
        2240,
        2120,
        NOT_DEFINE,
        2740,
        2860,
        NOT_DEFINE,
        0,
        "COME_XP2R5V",
        "/sys/bus/i2c/devices/i2c-8/8-0035/hwmon/hwmon*/in10_input",
        0,
    },
    {
        10800,
        10200,
        NOT_DEFINE,
        13200,
        13800,
        NOT_DEFINE,
        0,
        "COME_XP12R0V",
        "/sys/bus/i2c/devices/i2c-8/8-0035/hwmon/hwmon*/in11_input",
        0,
    },
    {
        2960,
        2800,
        NOT_DEFINE,
        3620,
        3800,
        NOT_DEFINE,
        0,
        "SSD_XP3R3V",
        "/sys/bus/i2c/devices/i2c-8/8-0035/hwmon/hwmon*/in12_input",
        0,
    },
    {
        4500,
        4260,
        NOT_DEFINE,
        5490,
        5760,
        NOT_DEFINE,
        0,
        "XP5R0V_C_MON",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in1_input",
        0,
    },
    {
        NOT_DEFINE,
        NOT_DEFINE,
        //2980,
        //2800,
        NOT_DEFINE,
        3620,
        3800,
        NOT_DEFINE,
        0,
        "XP3R3V_LEFT_MON",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in2_input",
        0,
    },
    {
        2980,
        2800,
        NOT_DEFINE,
        3620,
        3800,
        NOT_DEFINE,
        0,
        "XP3R3V_RIGHT_MON",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in3_input",
        0,
    },
    {
        2980,
        2800,
        NOT_DEFINE,
        3620,
        3800,
        NOT_DEFINE,
        0,
        "XP3R3V_FPGA_MON",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in4_input",
        0,
    },
    {
        1620,
        1530,
        NOT_DEFINE,
        1980,
        2070,
        NOT_DEFINE,
        0,
        "XP1R8V_FPGA_MON",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in5_input",
        0,
    },
    {
        1080,
        1020,
        NOT_DEFINE,
        1320,
        1380,
        NOT_DEFINE,
        0,
        "XP1R2V_FPGA_MON",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in6_input",
        0,
    },
    {
        900,
        850,
        NOT_DEFINE,
        1100,
        1150,
        NOT_DEFINE,
        0,
        "XP1R0V_FPGA_MON",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in7_input",
        0,
    },
    {
        1620,
        1530,
        NOT_DEFINE,
        1980,
        2070,
        NOT_DEFINE,
        0,
        "XP1R8V_AVDD_MON",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in8_input",
        0,
    },
    {
        810,
        770,
        NOT_DEFINE,
        990,
        1040,
        NOT_DEFINE,
        0,
        "XP0R9V_TRVDD0",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in9_input",
        0,
    },
    {
        810,
        770,
        NOT_DEFINE,
        990,
        1040,
        NOT_DEFINE,
        0,
        "XP0R9V_TRVDD1",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in10_input",
        0,
    },
    {
        680,
        640,
        NOT_DEFINE,
        830,
        860,
        NOT_DEFINE,
        0,
        "XP0R75V_TRVDD0",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in11_input",
        0,
    },
    {
        680,
        640,
        NOT_DEFINE,
        830,
        860,
        NOT_DEFINE,
        0,
        "XP0R75V_TRVDD1",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in12_input",
        0,
    },
    {
        680,
        640,
        NOT_DEFINE,
        830,
        860,
        NOT_DEFINE,
        0,
        "XP0R75V_AVDD_MON",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in13_input",
        0,
    },
    {
        680,
        650,
        NOT_DEFINE,
        960,
        1010,
        NOT_DEFINE,
        0,
        "VDD_CORE_MON",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in14_input",
        0,
    },
    {
        1080,
        1020,
        NOT_DEFINE,
        1320,
        1380,
        NOT_DEFINE,
        0,
        "XP1R2V_VDDO_MON",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in15_input",
        0,
    },
    {
        1620,
        1500,
        NOT_DEFINE,
        1980,
        2040,
        NOT_DEFINE,
        0,
        "XP1R8V_VDDO_MON",
        "/sys/bus/i2c/devices/i2c-8/8-0034/hwmon/hwmon*/in16_input",
        0,
    }
};


int get_psu_info_wobmc(int id, int *mvin, int *mvout, int *mpin, int *mpout, int *miin, int *miout)
{
    int ret = 0, fail = 0;
    long value = 0;
    *mvin = *mvout = *mpin = *mpout = *miin = *miout = 0;

    if ((NULL == mvin) || (NULL == mvout) ||(NULL == mpin) || (NULL == mpout) || (NULL == miin) || (NULL == miout)) {
        printf("%s null pointer!\n", __FUNCTION__);
        return -1;
    }

    check_psu_absent();
    if (1 == Psu[id].is_absent) {
        return 0;
    }

    /* voltage in and out */
    ret = get_sysnode_value(Psu[id].vin_path, &value);
    if (ret < 0)
        fail++;
    else
        *mvin = (int)value;

    ret = get_sysnode_value(Psu[id].vout_path, &value);
    if (ret < 0)
        fail++;
    else
        *mvout = (int)value;

    /* power in and out */
    ret = get_sysnode_value(Psu[id].pin_path, &value);
    if (ret < 0)
        fail++;
    else
        *mpin = (int)value;

    *mpin = *mpin / 1000;
    ret = get_sysnode_value(Psu[id].pout_path, &value);
    if (ret < 0)
        fail++;
    else
        *mpout = (int)value;

    *mpout = *mpout / 1000;
    /* current in and out*/
    ret = get_sysnode_value(Psu[id].cin_path, &value);
    if (ret < 0)
        fail++;
    else
        *miin = (int)value;

    ret = get_sysnode_value(Psu[id].cout_path, &value);
    if (ret < 0)
        fail++;
    else
        *miout = (int)value;

    return fail;
}

int get_psu_model_sn_wobmc(int id, char *model, char *serial_number)
{
    unsigned char cblock[288] = {0};
    int ret = 0;
    int bus = 0, address = 0;
    int bank = 0;

    if (id == 1) {
        bus = PSU1_I2C_BUS;
        address = PSU1_I2C_ADDR;
    } else {
        bus = PSU2_I2C_BUS;
        address = PSU2_I2C_ADDR;
    }
    bank = PMBUS_MODEL_REG;
    ret = read_smbus_block(bus, address, bank, cblock);
    if (ret > 0)
        strcpy(model, (char *)cblock);
    else
        return ERR;

    bank = PMBUS_SERIAL_REG;
    ret = read_smbus_block(bus, address, bank, cblock);
    if (ret > 0)
        strcpy(serial_number, (char *)cblock);
    else
        return ERR;

    return 0;
}

int get_fan_info_wobmc(int id, char *model, char *serial, int *airflow)
{
    uint16_t offset = FRU_COMM_HDR_LEN + FRU_BRDINFO_MFT_TLV_STRT, skip_len = 0;
    uint32_t count = 1;
    uint8_t  ret = 0;
    char path[100] = {0}, data[100] = {0};

    check_fan_absent();
    check_psu_absent();

    if (1 == Fan[id].is_absent) {
       return 0;
    }

    strcpy(path, Fan[id].eeprom_path);

    ret = read_eeprom(path, offset, data, &count);
    if (ret < 0)
        return ret;

    skip_len = data[0] & TLV_LEN_MASK; // skip Board Manufacturer bytes
    offset += skip_len + 1;
    count = 1;
    ret = read_eeprom(path, offset, data, &count);
    if (ret < 0)
        return ret;

    skip_len = data[0] & TLV_LEN_MASK; // skip Board Product Name
    offset += skip_len + 1;
    count = 1;
    ret = read_eeprom(path, offset, data, &count);
    if (ret < 0)
        return ret;

    count = data[0] & TLV_LEN_MASK; // Board Serial Number bytes length
    offset++;
    if (count > sizeof(data)) {
        printf("read count %d is larger than buffer size %ld\n", count, sizeof(data));
        return -1;
    }
    ret = read_eeprom(path, offset, data, &count);
    if (ret < 0)
        return ret;

    strcpy(serial, data);
    memset(data, 0, sizeof(data));
    offset += count;
    count = 1;
    ret = read_eeprom(path, offset, data, &count);
    if (ret < 0)
        return ret;

    count = data[0] & TLV_LEN_MASK; // Board Part Number bytes length
    offset++;
    if (count > sizeof(data)) {
        printf("read count %d is larger than buffer size %ld\n", count, sizeof(data));
        return -1;
    }
    ret = read_eeprom(path, offset, data, &count);
    if (ret < 0)
        return ret;

    strcpy(model, data);

    /* 0-B2F,1-F2B*/
    get_sysnode_value(Fan[id].dir_path, airflow);

    return 0;
}


int get_sensor_info_wobmc(int id, int *temp, int *warn, int *error, int *shutdown)
{
    int ret = 0;

     *temp = *warn = *error = *shutdown = 0;

    if ((NULL == temp) || (NULL == warn) || (NULL == error) || (NULL == shutdown)) {
        printf("%s null pointer!\n", __FUNCTION__);
        return -1;
    }

    *warn = Temp_F2B[id].hi_warn;
    *error = Temp_F2B[id].hi_err;
    *shutdown = Temp_F2B[id].hi_shutdown;
    ret = get_sysnode_value(Temp_F2B[id].path, temp);
    if (ret < 0 || *temp == 0)
        return -1;

    return 0;
}

int get_fan_speed_wobmc(int id, int *per, int *rpm)
{
    char retstr[10]={0};
    int ret = 0;
    int max_speed;

    *rpm = *per = 0;

    if ((NULL == per) || (NULL == rpm)) {
        printf("%s null pointer!\n", __FUNCTION__);
        return -1;
    }

    ret = get_sysnode_value(Fan[id].frpm_path, rpm);
    if (ret < 0 || *rpm == 0)
        return -1;
    else {
        /**
         * Greystone only supports F2B PSU TDPS2000LB A
         * and it's speed is not linear, so obtain duty from mfr duty register 0xd1
         * while PMBUS duty register is 0x94
         */
        if (PSU1_FAN_ID == id) {
            exec_cmd("i2cget -y -f 47 0x58 0xd1", retstr);
            *per = strtol(retstr, NULL, 16);;
        } else if (PSU2_FAN_ID == id) {
            exec_cmd("i2cget -y -f 48 0x59 0xd1", retstr);
            *per = strtol(retstr, NULL, 16);;
        } else {
             max_speed = CHASSIS_FAN_FRONT_MAX;
            *per = *rpm * 100 / max_speed;
        }
    }

    return 0;
}

/**
 * @brief Set replaceable fan module LED color
 * @param id fan module ID
 * @param color green/off/amber
 * @retval ERR fail to set LED
 * @retval OK  succeed to set LED
 */
static uint8_t set_fan_led(uint8_t id, char *color)
{
    int fd, rc = 0;

    fd = open(Fan[id].led_path, O_WRONLY);
    if (fd < 0) {
        printf("Fail to open the file: %s \n", Fan[id].led_path);
        return ERR;
    }

    rc = write(fd, color, strlen(color));
    if (rc != strlen(color)) {
        printf("Write failed. count=%lu, rc=%d \n", strlen(color), rc);
        close(fd);
        return ERR;
    }

    close(fd);

    return OK;
}

/**
 * @brief Check replaceable fan module fault status
 * @retval fault Numbers of failed fans
 */
int check_fan_fault(void)
{
    int err = 0; // single fan error
    int fault = 0; // total fan error errors
    int ret = 0, i = 0;
    long speed = 0, speed_old = 0, pwm = 0;
    long low_thr, high_thr = 0;

    check_fan_absent();
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        if (1 == Fan[i].is_absent) {
            set_fan_led(i, "off");
            continue;
        }

        /* if fan rpm duty is out of tolerant scope, set as fault */
        ret = get_sysnode_value(Fan[i].pwm_path, (void *)&pwm);
        if (ret) {
            fault++;
            printf("Can't get sysnode: %s\n", Fan[i].pwm_path);
            set_fan_led(i, "amber");
            continue;
        }

        low_thr = CHASSIS_FAN_FRONT_MAX * pwm * (100 - CHASIS_FAN_TOLERANT_PER) / 25500;
        high_thr = CHASSIS_FAN_FRONT_MAX * pwm * (100 + CHASIS_FAN_TOLERANT_PER) / 25500;

        ret = get_sysnode_value(Fan[i].frpm_path, (void *)&speed);
        if (ret) {
            fault++;
            printf("Can't get sysnode: %s\n", Fan[i].frpm_path);
            set_fan_led(i, "amber");
            continue;
        }

        /* Eliminate jitter */
        if (speed < low_thr || speed > high_thr) {
            sleep(5);
            speed_old = speed;
            ret = get_sysnode_value(Fan[i].frpm_path, (void *)&speed);
            if (ret) {
                fault++;
                printf("Can't get sysnode: %s\n", Fan[i].frpm_path);
                set_fan_led(i, "amber");
                continue;
            }
        }

        if (speed < RPM_FAULT_DEF) {
            if (0 == Fan[i].is_udminspd_ffault)
                syslog(LOG_WARNING, "Fan%d front rpm %ld is less than %d, fault !!!\n",
                      i, speed, RPM_FAULT_DEF);
            Fan[i].is_udminspd_ffault = 1;
            err++;
            fault++;
        }
        else {
            Fan[i].is_udminspd_ffault = 0;
            if (speed < low_thr && speed <= speed_old) {
                if (0 == Fan[i].is_udspd_ffault)
                    syslog(LOG_WARNING, "Fan%d front rpm %ld is less than %ld at pwm %ld!!!\n",
                                    i, speed, low_thr, pwm);
                Fan[i].is_udspd_ffault = 1;
                err++;
                fault++;
            } else {
                Fan[i].is_udspd_ffault = 0;
            }

            if (speed > high_thr && speed >= speed_old) {
                if (0 == Fan[i].is_ovspd_ffault)
                    syslog(LOG_WARNING, "Fan%d front rpm %ld is larger than %ld at pwm %ld!!!\n",
                                    i, speed, high_thr, pwm);
                Fan[i].is_ovspd_ffault = 1;
                err++;
                fault++;
            }
            else {
                Fan[i].is_ovspd_ffault = 0;
            }
        }

        /* if fan rpm duty is out of tolerant scope, set as fault */
        low_thr = CHASSIS_FAN_REAR_MAX * pwm * (100 - CHASIS_FAN_TOLERANT_PER) / 25500;
        high_thr = CHASSIS_FAN_REAR_MAX * pwm * (100 + CHASIS_FAN_TOLERANT_PER) / 25500;

        ret = get_sysnode_value(Fan[i].rrpm_path, (void *)&speed);
        if (ret) {
            fault++;
            printf("Can't get sysnode: %s\n", Fan[i].rrpm_path);
            set_fan_led(i, "amber");
            continue;
        }

        /* Eliminate jitter */
        if (speed < low_thr || speed > high_thr) {
            sleep(5);
            speed_old = speed;
            ret = get_sysnode_value(Fan[i].rrpm_path, (void *)&speed);
            if (ret) {
                fault++;
                printf("Can't get sysnode: %s\n", Fan[i].rrpm_path);
                set_fan_led(i, "amber");
                continue;
            }
        }

        if (speed < RPM_FAULT_DEF) {
            if (0 == Fan[i].is_udminspd_rfault)
                syslog(LOG_WARNING, "Fan%d rear rpm %ld is less than %d, fault !!!\n",
                      i, speed, RPM_FAULT_DEF);
            Fan[i].is_udminspd_rfault = 1;
            err++;
            fault++;
        }
        else {
            Fan[i].is_udminspd_rfault = 0;
            if (speed < low_thr && speed <= speed_old) {
                if (0 == Fan[i].is_udspd_rfault)
                    syslog(LOG_WARNING, "Fan%d rear rpm %ld is less than %ld at pwm %ld!!!\n",
                                    i, speed, low_thr, pwm);
                Fan[i].is_udspd_rfault = 1;
                err++;
                fault++;
            }
            else {
                Fan[i].is_udspd_rfault = 0;
            }

            if (speed > high_thr && speed >= speed_old) {
                if (0 == Fan[i].is_ovspd_rfault)
                    syslog(LOG_WARNING, "Fan%d rear rpm %ld is larger than %ld at pwm %ld!!!\n",
                                    i, speed, high_thr, pwm);
                Fan[i].is_ovspd_rfault = 1;
                err++;
                fault++;
            }
            else {
                Fan[i].is_ovspd_rfault = 0;
            }
        }

        if (err > 0) {
            set_fan_led(i, "amber");
            err = 0;
        }
        else {
            set_fan_led(i, "green");
        }
    }

    return fault;
}

/**
 * @brief Check replaceable PSU module fault status
 * @retval fault Numbers of failed PSUs
 */
int check_psu_fault(void)
{
    uint8_t i = 0;
    uint8_t psu_status = 0;

    int present_status = 0, ac_status = 0, pow_status = 0, alert_status = 0;
    int fault = 0;

    for (i = 1; i <= PSU_COUNT; i++)
    {
        psu_status = get_psu_status(i);
        present_status = (psu_status >> psu_mapper[i].bit_present) & 0x01;
        alert_status = (psu_status >> psu_mapper[i].bit_alert) & 0x01;
        ac_status = (psu_status >> psu_mapper[i].bit_ac_sta) & 0x01;
        pow_status = (psu_status >> psu_mapper[i].bit_pow_sta) & 0x01;

        if (0 == present_status)
        {
            if (0 == ac_status || 0 == pow_status || 0 == alert_status) {
                fault++;
            }

            if (0 == alert_status) {
                if (0 == Psu[i].is_alert)
                    syslog(LOG_WARNING, "PSU%d alert is detected !!!\n", i);
                Psu[i].is_alert = 1;
            }
            else {
                Psu[i].is_alert = 0;
            }

            if (0 == ac_status) {
                if (0 == Psu[i].is_ac_fault)
                    syslog(LOG_WARNING, "PSU%d AC power is detected failed !!!\n", i);
                Psu[i].is_ac_fault = 1;
            }
            else {
                Psu[i].is_ac_fault = 0;
            }

            if (0 == pow_status) {
                if (0 == Psu[i].is_power_fault)
                    syslog(LOG_WARNING, "PSU%d AC power is detected failed !!!\n", i);
                Psu[i].is_power_fault = 1;
            }
            else {
                Psu[i].is_power_fault = 0;
            }
        }
    }

    return fault;
}

/**
 * @brief Check system airflow
 * @retvla <0 error
 */
int check_sys_airflow(void)
{
    int rv;
    onlp_sys_info_t si;
    list_links_t *cur, *next;

    rv = onlp_sys_info_get(&si);
    if(rv < 0) {
        printf("Can't get onie data\n");
        return rv;
    }

    LIST_FOREACH_SAFE(&si.onie_info.vx_list, cur, next) {
        onlp_onie_vx_t* vx = container_of(cur, links, onlp_onie_vx_t);
        if (0x2f == vx->data[0] && 0xd4 == vx->data[1] && 0xfb == vx->data[2]) {
           Sys_Airflow = FAN_F2B;
        } else if (0x2f == vx->data[0] && 0xd4 == vx->data[1] && 0xbf == vx->data[2]) {
           Sys_Airflow = FAN_B2F;
        }
    }

    return 0;
}

/**
 * @brief Check replaceable fan module airflow
 * @param sys_airflow System airflow
 * @retval ERR error
 * @retval fault The number of fans in the opposite direction to the system airflow
 */
int check_fan_airflow_fault(uint8_t sys_airflow)
{
    int ret = 0, i = 0;
    long fan_airflow = 0;
    int fault = 0;

    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        ret = get_sysnode_value(Fan[i].dir_path, (void *)&fan_airflow);
        if (ret) {
            return ERR;
        }
        if (sys_airflow != fan_airflow) {
            if (0 == Fan[i].is_airflow_fault)
                syslog(LOG_WARNING, "Fan%d airflow is fault !!!\n", i);
            Fan[i].is_airflow_fault = 1;
            fault++;
        } else {
            Fan[i].is_airflow_fault = 0;
        }
    }

    return fault;
}

/**
 * @brief Check replaceable PSU module airflow
 * @param sys_airflow System airflow
 * @retval fault The number of PSUs in the opposite direction to the system airflow
 */
int check_psu_airflow_fault(uint8_t sys_airflow)
{
    int ret = 0, i = 0, psu_airflow = -1;
    int fault = 0;
    char model[30] = {0};
    char serial_number[30] = {0};

    check_psu_absent();

    for (i = 1; i <= PSU_COUNT; i++) {
        if (Psu[i].is_absent) {
            continue;
        }

        ret = get_psu_model_sn_wobmc(i, model, serial_number);
        if (ret) {
            fault++;
            continue;
        }
        if (!strcmp(model, PSU_F2B_AIRFLOW_FIELD)) {
            psu_airflow = FAN_F2B;
        } else if (!strcmp(model, PSU_B2F_AIRFLOW_FIELD)) {
            psu_airflow = FAN_B2F;
        }

        if (sys_airflow != psu_airflow) {
            if (0 == Psu[i].is_airflow_fault)
                syslog(LOG_WARNING, "PSU%d airflow is fault !!!\n", i);
            Psu[i].is_airflow_fault = 1;
            fault++;
        } else {
            Psu[i].is_airflow_fault = 0;
        }
    }

    return fault;
}

/**
 * @brief Check replaceable fan module present status
 * @retval ERR error
 * @retval fault The number of absent fans
 */
int check_fan_absent(void)
{
    int ret = 0, fault = 0, i = 0;
    long status = 0;

    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        ret = get_sysnode_value(Fan[i].pres_path, (void *)&status);
        if (ret) {
            return ERR;
        }
        if (status == ABSENT) {
            if (0 == Fan[i].is_absent) {
                syslog(LOG_WARNING, "Fan%d is absent !!!\n", i);
            }
            Fan[i].is_absent = 1;
            Fan[i].is_airflow_fault = 0;
            Fan[i].is_udminspd_ffault = 0;
            Fan[i].is_udminspd_rfault = 0;
            Fan[i].is_udspd_ffault = 0;
            Fan[i].is_udspd_rfault = 0;
            Fan[i].is_ovspd_ffault = 0;
            Fan[i].is_ovspd_rfault = 0;
            fault++;
        }
        else {
            if (1 == Fan[i].is_absent)
                syslog(LOG_WARNING, "Fan%d is plugged in\n", i);
            Fan[i].is_absent = 0;
        }
    }

    return fault;
}

/**
 * @brief Check replaceable PSU module present status
 * @retval ERR error
 * @retval fault The number of absent PSUs
 */
int check_psu_absent(void)
{
    uint8_t psu_status = 0, present_status = 0;
    int fault = 0, i = 0;

    for (i = 1; i <= PSU_COUNT; i++) {
        psu_status = get_psu_status(i);
        present_status = (psu_status >> psu_mapper[i].bit_present) & 0x01;
        if (ABSENT == present_status) { /* absent */
            if (0 == Psu[i].is_absent)
                syslog(LOG_WARNING, "PSU%d is absent !!!\n", i);
            Fan[PSU1_FAN_ID + i - 1].is_absent = 1;
            Psu[i].is_absent = 1;
            Psu[i].is_airflow_fault = 0;
            Psu[i].is_ac_fault = 0;
            Psu[i].is_power_fault = 0;
            fault++;
        } else {
            if (1 == Psu[i].is_absent)
                syslog(LOG_WARNING, "PSU%d is plugged in\n", i);
            Fan[PSU1_FAN_ID + i - 1].is_absent = 0;
            Psu[i].is_absent = 0;
        }
    }

    return fault;
}

