###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as7312_54xs_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as7312_54xs_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as7312_54xs_DEPENDMODULE_ENTRIES := init:x86_64_accton_as7312_54xs ucli:x86_64_accton_as7312_54xs

