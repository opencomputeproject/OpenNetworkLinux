###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as9817_64_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as9817_64_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as9817_64_DEPENDMODULE_ENTRIES := init:x86_64_accton_as9817_64 ucli:x86_64_accton_as9817_64
