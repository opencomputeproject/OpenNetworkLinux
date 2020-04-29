###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_mlnx_msn3510_INCLUDES := -I $(THIS_DIR)inc
x86_64_mlnx_msn3510_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_mlnx_msn3510_DEPENDMODULE_ENTRIES := init:x86_64_mlnx_msn3510 ucli:x86_64_mlnx_msn3510

