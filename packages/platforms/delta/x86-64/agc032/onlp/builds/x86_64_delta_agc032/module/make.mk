###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_delta_agc032_INCLUDES := -I $(THIS_DIR)inc
x86_64_delta_agc032_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_delta_agc032_DEPENDMODULE_ENTRIES := init:x86_64_delta_agc032 ucli:x86_64_delta_agc032