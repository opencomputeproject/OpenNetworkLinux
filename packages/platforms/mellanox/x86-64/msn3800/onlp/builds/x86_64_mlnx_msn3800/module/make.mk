###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_mlnx_msn3800_INCLUDES := -I $(THIS_DIR)inc
x86_64_mlnx_msn3800_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_mlnx_msn3800_DEPENDMODULE_ENTRIES := init:x86_64_mlnx_msn3800 ucli:x86_64_mlnx_msn3800

