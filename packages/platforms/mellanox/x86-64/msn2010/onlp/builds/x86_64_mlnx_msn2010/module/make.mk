###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_mlnx_msn2010_INCLUDES := -I $(THIS_DIR)inc
x86_64_mlnx_msn2010_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_mlnx_msn2010_DEPENDMODULE_ENTRIES := init:x86_64_mlnx_msn2010 ucli:x86_64_mlnx_msn2010
