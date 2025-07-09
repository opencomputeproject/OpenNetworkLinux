############################
# Makefile for s3ip driver # 
############################
obj-m += switch_mb_driver.o
obj-m += switch_fpga_driver.o
obj-m += switch_cpld_driver.o
obj-m += switch_led_driver.o
obj-m += switch_fan_driver.o
obj-m += switch_transceiver_driver.o
obj-m += switch_psu_driver.o
obj-m += switch_sensor_driver.o
obj-m += switch_wdt_driver.o
obj-m += switch_avs_driver.o
obj-m += switch_bmc_driver.o

obj-m += s3ip/switch_s3ip_switch.o
obj-m += s3ip/switch_s3ip_syseeprom.o
obj-m += s3ip/switch_s3ip_fpga.o
obj-m += s3ip/switch_s3ip_cpld.o
obj-m += s3ip/switch_s3ip_sysled.o
obj-m += s3ip/switch_s3ip_fan.o
obj-m += s3ip/switch_s3ip_transceiver.o
obj-m += s3ip/switch_s3ip_watchdog.o
obj-m += s3ip/switch_s3ip_psu.o
obj-m += s3ip/switch_s3ip_sensor.o
obj-m += s3ip/switch_s3ip_avs.o
obj-m += s3ip/switch_s3ip_bmc.o

obj-m += third_party/switch_system_fpga.o
#obj-m += third_party/switch_intel-spi.o
obj-m += third_party/switch_optoe.o
obj-m += third_party/switch_coretemp.o
obj-m += third_party/switch_i2c_cpld.o
obj-m += third_party/switch_system_cpld.o
obj-m += third_party/switch_lm75.o
obj-m += third_party/switch_at24.o
obj-m += third_party/i2c-imc.o
obj-m += third_party/sysfs_ipmi.o
#obj-m += third_party/intel-spi-platform.o
obj-m += third_party/mp2975.o
obj-m += third_party/switch_pmbus_core.o
obj-m += third_party/bel-pfe.o

subdir-ccflags-y := -I$(src)
subdir-ccflags-y += -I$(src)/third_party

