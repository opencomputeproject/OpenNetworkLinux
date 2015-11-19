###############################################################################
#
# quanta_sys_eeprom Unit Test Makefile.
#
###############################################################################
UMODULE := quanta_sys_eeprom
UMODULE_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/utest.mk
