###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as4222_28pe_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as4222_28pe_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as4222_28pe_DEPENDMODULE_ENTRIES := init:x86_64_accton_as4222_28pe ucli:x86_64_accton_as4222_28pe

