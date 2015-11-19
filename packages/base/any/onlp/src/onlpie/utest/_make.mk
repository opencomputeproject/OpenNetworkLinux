###############################################################################
#
# onlpie Unit Test Makefile.
#
###############################################################################
UMODULE := onlpie
UMODULE_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/utest.mk
