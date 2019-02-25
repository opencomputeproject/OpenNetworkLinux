###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_inventec_d5052_INCLUDES := -I $(THIS_DIR)inc
x86_64_inventec_d5052_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_inventec_d5052_DEPENDMODULE_ENTRIES := init:x86_64_inventec_d5052 ucli:x86_64_inventec_d5052

