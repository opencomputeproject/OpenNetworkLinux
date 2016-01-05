###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
quanta_sys_eeprom_INCLUDES := -I $(THIS_DIR)inc
quanta_sys_eeprom_INTERNAL_INCLUDES := -I $(THIS_DIR)src
quanta_sys_eeprom_DEPENDMODULE_ENTRIES := init:quanta_sys_eeprom ucli:quanta_sys_eeprom

