###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_mlnx_msn2410_INCLUDES := -I $(THIS_DIR)inc
x86_64_mlnx_msn2410_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_mlnx_msn2410_DEPENDMODULE_ENTRIES := init:x86_64_mlnx_msn2410 ucli:x86_64_mlnx_msn2410

