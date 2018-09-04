###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as7316_26xb_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as7316_26xb_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as7316_26xb_DEPENDMODULE_ENTRIES := init:x86_64_accton_as7316_26xb ucli:x86_64_accton_as7316_26xb

