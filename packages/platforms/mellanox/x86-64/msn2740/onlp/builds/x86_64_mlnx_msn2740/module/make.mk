###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_mlnx_msn2740_INCLUDES := -I $(THIS_DIR)inc
x86_64_mlnx_msn2740_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_mlnx_msn2740_DEPENDMODULE_ENTRIES := init:x86_64_mlnx_msn2740 ucli:x86_64_mlnx_msn2740
