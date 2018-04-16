###############################################################################
#
# mlnx_common Unit Test Makefile.
#
###############################################################################
UMODULE := mlnx_common
UMODULE_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/utest.mk
