###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_inventec_d6332_INCLUDES := -I $(THIS_DIR)inc
x86_64_inventec_d6332_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_inventec_d6332_DEPENDMODULE_ENTRIES := init:x86_64_inventec_d6332 ucli:x86_64_inventec_d6332

