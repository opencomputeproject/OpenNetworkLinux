###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_asgvolt64_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_asgvolt64_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_asgvolt64_DEPENDMODULE_ENTRIES := init:x86_64_accton_asgvolt64 ucli:x86_64_accton_asgvolt64

