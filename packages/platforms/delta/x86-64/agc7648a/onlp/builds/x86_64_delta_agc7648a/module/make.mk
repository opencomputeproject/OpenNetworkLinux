###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_delta_agc7648a_INCLUDES := -I $(THIS_DIR)inc
x86_64_delta_agc7648a_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_delta_agc7648a_DEPENDMODULE_ENTRIES := init:x86_64_delta_agc7648a ucli:x86_64_delta_agc7648a

