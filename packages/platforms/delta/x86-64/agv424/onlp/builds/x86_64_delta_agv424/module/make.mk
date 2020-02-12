###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_delta_agv424_INCLUDES := -I $(THIS_DIR)inc
x86_64_delta_agv424_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_delta_agv424_DEPENDMODULE_ENTRIES := init:x86_64_delta_agv424 ucli:x86_64_delta_agv424