###############################################################################
#
# x86_64_accton_es7632bt3 Unit Test Makefile.
#
###############################################################################
UMODULE := x86_64_accton_es7632bt3
UMODULE_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/utest.mk
