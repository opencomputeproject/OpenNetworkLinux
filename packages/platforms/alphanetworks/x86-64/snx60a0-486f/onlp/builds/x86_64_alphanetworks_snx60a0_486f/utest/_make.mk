###############################################################################
#
# x86_64_alphanetworks_snx60a0_486f Unit Test Makefile.
#
###############################################################################
UMODULE := x86_64_alphanetworks_snx60a0_486f
UMODULE_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/utest.mk
