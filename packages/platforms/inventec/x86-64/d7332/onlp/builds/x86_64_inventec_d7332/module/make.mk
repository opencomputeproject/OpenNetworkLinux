###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_inventec_d7332_INCLUDES := -I $(THIS_DIR)inc
x86_64_inventec_d7332_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_inventec_d7332_DEPENDMODULE_ENTRIES := init:x86_64_inventec_d7332 ucli:x86_64_inventec_d7332

