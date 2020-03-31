###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_mlnx_msn27002_INCLUDES := -I $(THIS_DIR)inc
x86_64_mlnx_msn27002_DEPENDMODULE_ENTRIES := init:x86_64_mlnx_msn27002 ucli:x86_64_mlnx_msn27002

