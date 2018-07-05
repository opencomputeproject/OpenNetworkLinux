###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_delta_ag7648c_INCLUDES := -I $(THIS_DIR)inc
x86_64_delta_ag7648c_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_delta_ag7648c_DEPENDMODULE_ENTRIES := init:x86_64_delta_ag7648c ucli:x86_64_delta_ag7648c

