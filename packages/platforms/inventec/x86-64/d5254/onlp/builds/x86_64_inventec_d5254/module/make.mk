###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_inventec_d5254_INCLUDES := -I $(THIS_DIR)inc
x86_64_inventec_d5254_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_inventec_d5254_DEPENDMODULE_ENTRIES := init:x86_64_inventec_d5254 ucli:x86_64_inventec_d5254

