###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as7816_64x_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as7816_64x_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as7816_64x_DEPENDMODULE_ENTRIES := init:x86_64_accton_as7816_64x ucli:x86_64_accton_as7816_64x

