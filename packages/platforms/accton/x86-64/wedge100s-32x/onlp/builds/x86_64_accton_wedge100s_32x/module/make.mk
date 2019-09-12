###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_wedge100s_32x_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_wedge100s_32x_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_wedge100s_32x_DEPENDMODULE_ENTRIES := init:x86_64_accton_wedge100s_32x ucli:x86_64_accton_wedge100s_32x

