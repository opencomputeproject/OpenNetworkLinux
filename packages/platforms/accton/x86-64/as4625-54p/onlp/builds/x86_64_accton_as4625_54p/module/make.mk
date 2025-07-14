###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as4625_54p_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as4625_54p_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as4625_54p_DEPENDMODULE_ENTRIES := init:x86_64_accton_as4625_54p ucli:x86_64_accton_as4625_54p

