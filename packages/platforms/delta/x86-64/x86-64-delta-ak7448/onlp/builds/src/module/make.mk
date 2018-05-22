###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_delta_ak7448_INCLUDES := -I $(THIS_DIR)inc
x86_64_delta_ak7448_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_delta_ak7448_DEPENDMODULE_ENTRIES := init:x86_64_delta_ak7448 ucli:x86_64_delta_ak7448

