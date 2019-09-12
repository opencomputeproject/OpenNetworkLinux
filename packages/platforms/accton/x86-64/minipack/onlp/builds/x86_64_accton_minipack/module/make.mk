###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_minipack_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_minipack_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_minipack_DEPENDMODULE_ENTRIES := init:x86_64_accton_minipack ucli:x86_64_accton_minipack

