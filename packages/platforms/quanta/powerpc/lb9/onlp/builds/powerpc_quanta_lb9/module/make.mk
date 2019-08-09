###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
powerpc_quanta_lb9_INCLUDES := -I $(THIS_DIR)inc
powerpc_quanta_lb9_INTERNAL_INCLUDES := -I $(THIS_DIR)src
powerpc_quanta_lb9_DEPENDMODULE_ENTRIES := init:powerpc_quanta_lb9 ucli:powerpc_quanta_lb9

