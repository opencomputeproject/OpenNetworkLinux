###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_mlnx_msn24102_INCLUDES := -I $(THIS_DIR)inc
x86_64_mlnx_msn24102_DEPENDMODULE_ENTRIES := init:x86_64_mlnx_msn24102 ucli:x86_64_mlnx_msn24102

