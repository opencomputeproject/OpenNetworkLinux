###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_delta_ag5648_INCLUDES := -I $(THIS_DIR)inc
x86_64_delta_ag5648_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_delta_ag5648_DEPENDMODULE_ENTRIES := init:x86_64_delta_ag5648 ucli:x86_64_delta_ag5648

