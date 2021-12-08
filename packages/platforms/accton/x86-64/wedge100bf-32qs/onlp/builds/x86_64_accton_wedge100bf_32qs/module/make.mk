###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_wedge100bf_32qs_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_wedge100bf_32qs_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_wedge100bf_32qs_DEPENDMODULE_ENTRIES := init:x86_64_accton_wedge100bf_32qs ucli:x86_64_accton_wedge100bf_32qs

