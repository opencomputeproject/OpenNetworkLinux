###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as4625_30p_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as4625_30p_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as4625_30p_DEPENDMODULE_ENTRIES := init:x86_64_accton_as4625_30p ucli:x86_64_accton_as4625_30p

