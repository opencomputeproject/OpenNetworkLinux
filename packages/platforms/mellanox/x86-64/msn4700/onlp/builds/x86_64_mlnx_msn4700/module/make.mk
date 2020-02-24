###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_mlnx_msn4700_INCLUDES := -I $(THIS_DIR)inc
x86_64_mlnx_msn4700_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_mlnx_msn4700_DEPENDMODULE_ENTRIES := init:x86_64_mlnx_msn4700 ucli:x86_64_mlnx_msn4700

