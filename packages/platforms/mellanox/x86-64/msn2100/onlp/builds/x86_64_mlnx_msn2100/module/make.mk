###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_mlnx_msn2100_INCLUDES := -I $(THIS_DIR)inc
x86_64_mlnx_msn2100_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_mlnx_msn2100_DEPENDMODULE_ENTRIES := init:x86_64_mlnx_msn2100 ucli:x86_64_mlnx_msn2100

