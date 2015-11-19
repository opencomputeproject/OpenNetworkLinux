###############################################################################
#
# sff Unit Test Makefile.
#
###############################################################################
UMODULE := sff
UMODULE_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/utest.mk
