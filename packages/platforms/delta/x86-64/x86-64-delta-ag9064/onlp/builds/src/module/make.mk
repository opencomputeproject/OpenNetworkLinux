###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_delta_ag9064_INCLUDES := -I $(THIS_DIR)inc
x86_64_delta_ag9064_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_delta_ag9064_DEPENDMODULE_ENTRIES := init:x86_64_delta_ag9064 ucli:x86_64_delta_ag9064

