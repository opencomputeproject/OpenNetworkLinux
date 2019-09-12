###############################################################################
#
# x86_64_accton_as7726_32x Unit Test Makefile.
#
###############################################################################
UMODULE := x86_64_accton_as7726_32x
UMODULE_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/utest.mk
