###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_cel_redstone_xp_INCLUDES := -I $(THIS_DIR)inc
x86_64_cel_redstone_xp_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_cel_redstone_xp_DEPENDMODULE_ENTRIES := init:x86_64_cel_redstone_xp ucli:x86_64_cel_redstone_xp

