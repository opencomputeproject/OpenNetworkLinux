###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_amx3200_INCLUDES := -I $(THIS_DIR)inc
x86_64_amx3200_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_amx3200_DEPENDMODULE_ENTRIES := init:x86_64_amx3200 ucli:x86_64_amx3200

