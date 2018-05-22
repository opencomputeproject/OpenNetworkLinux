###############################################################################
#
# x86_64_accton_wedge100s_32x Unit Test Makefile.
#
###############################################################################
UMODULE := x86_64_accton_wedge100s_32x
UMODULE_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/utest.mk
