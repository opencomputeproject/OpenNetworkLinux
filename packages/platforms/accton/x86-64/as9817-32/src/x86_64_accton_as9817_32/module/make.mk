###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as9817_32_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as9817_32_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as9817_32_DEPENDMODULE_ENTRIES := init:x86_64_accton_as9817_32 ucli:x86_64_accton_as9817_32
