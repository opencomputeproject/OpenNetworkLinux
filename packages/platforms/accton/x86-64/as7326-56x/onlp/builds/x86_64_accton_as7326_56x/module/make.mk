###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as7326_56x_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as7326_56x_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as7326_56x_DEPENDMODULE_ENTRIES := init:x86_64_accton_as7326_56x ucli:x86_64_accton_as7326_56x

